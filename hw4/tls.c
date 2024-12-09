#include "tls.h"

struct hash_element* hash_table[HASH_SIZE];
int initialized = 0;
int PAGE_SIZE = 4096;

void tlsPageFaultHandler(int sig, siginfo_t *si, void *context) {
    // get address of faulting page by masking out the offset bits (last 12 bits)
    unsigned long int pageFault = ((unsigned long int) si->si_addr & ~(PAGE_SIZE - 1));
    
    // with segfault, need to check whether segfault is due to page fault or has touched forbidden memory
    int i;
    // check every TLS in hash table to see if page fault address is in any tls
    for (i = 0; i < HASH_SIZE; i++) {
        if (hash_table[i] != NULL && hash_table[i]->tls->pages != NULL) {
            int j;
            // for each page of the tls, check if the page fault address is the same as the page address
            for (j = 0; j < hash_table[i]->tls->page_num; j++) {
                if (hash_table[i]->tls->pages[j]->address == pageFault) {
                    // if so, call pthread_exit
                    pthread_exit(NULL);
                }
            }
        }
    }

    // if page fault address is not in any tls, its a normal seg fault so raise default handler
    signal(SIGSEGV, SIG_DFL);
    signal(SIGBUS, SIG_DFL);
    raise(sig);
}

void tls_init() {
    // initialize signal handler for SIGSEGV and SIGBUS
    if (initialized) {
        return;
    }
    struct sigaction sigact;

    // get page size
    PAGE_SIZE = getpagesize();

    // set up signal handler
    sigemptyset(&sigact.sa_mask); // set initial mask to empty so no signals are blocked
    sigact.sa_flags = SA_SIGINFO; // use extended signal handling
    sigact.sa_sigaction = tlsPageFaultHandler; // set handler function
}




int tls_create(unsigned int size) {
    // initialize system if not already
    if (!initialized) {
        tls_init();
    }

    pthread_t tid = pthread_self();

    // check if thread already has a local storage area
    int i;
    for (i = 0; i < HASH_SIZE; i++) {
        if (hash_table[i] != NULL && hash_table[i]->tid == tid) {
            return -1;
        }
    }

    // check if size is valid
    if (size < 1) {
        return -1;
    }

    // create new TLS. Does not need to be page aligned since this isn't an actual page yet
    TLS *tls = calloc(1, sizeof(TLS));
    if (!tls) {
        return -1;
    }
    tls->tid = tid;
    tls->size = size;
    tls->page_num = (size / PAGE_SIZE) + (size % PAGE_SIZE != 0);

    // allocate memory for pointers to pages
    tls->pages = (struct page**) calloc(tls->page_num, sizeof(struct page*));
    // for each page pointer allocate a page aligned page
    for (i = 0; i < tls->page_num; i++) {
        struct page *p = calloc(1, sizeof(struct page));
        p->address = (unsigned long int) mmap(0, PAGE_SIZE, 0, MAP_ANON | MAP_PRIVATE, 0, 0);
        p->ref_count = 1;
        tls->pages[i] = p;
    }

    // add TLS to hash table
    struct hash_element *hash = calloc(1, sizeof(struct hash_element));
    hash->tid = tid;
    hash->tls = tls;
    hash->next = NULL;

    for (i = 0; i < HASH_SIZE; i++) {
        if (hash_table[i] == NULL) {
            hash_table[i] = hash;
            return 0;
        }
    }

    return -1;
}

int tls_write(unsigned int offset, unsigned int length, char *buffer) {
    pthread_t tid = pthread_self();

    // check if lsa exists
    int i;
    for (i = 0; i < HASH_SIZE; i++) {
        if (hash_table[i] != NULL && hash_table[i]->tid == tid) {
            break;
        }
    }
    if (i == HASH_SIZE) {
        return -1;
    }

    // check if offset+length > size of lsa
    TLS *tls = hash_table[i]->tls;
    if (offset + length > tls->size) {
        return -1;
    }

    // unprotect pages to allow reading
    for (i = 0; i < tls->page_num; i++) {
        tls_unprotect(tls->pages[i]);
    }

    // begin writing
    int cnt, idx;
    for (cnt = 0, idx = offset; idx < (offset + length); cnt++, idx++) {
        struct page *p;
        unsigned int pageNumber, pageOffset;

        // find page number since pages can be in different areas of memory, then find offset within page
        pageNumber = idx / PAGE_SIZE;
        pageOffset = idx % PAGE_SIZE;

        // copy data from page to buffer
        p = tls->pages[pageNumber];

        // first check if page is shared, if so create a private copy
        if (p->ref_count > 1) {
            struct page *copy = (struct page *) calloc(1, sizeof(struct page));
            copy->address = (unsigned long int) mmap(0, PAGE_SIZE, PROT_WRITE, MAP_ANON | MAP_PRIVATE, 0, 0);

            // copy data from original page to new page and set ref count to 1 since new page belongs to new thread only now
            memcpy((void *) copy->address, (void *) p->address, PAGE_SIZE);
            copy->ref_count = 1;

            // set reference to page to the copied page
            tls->pages[pageNumber] = copy;

            // update original page and set p to the copy
            p->ref_count--;
            tls_protect(p);
            p = copy;
        }

        // write data to page
        char *dest = ((char *) p->address) + pageOffset;
        *dest = buffer[cnt];
    }

    // protect pages again
    for (i = 0; i < tls->page_num; i++) {
        tls_protect(tls->pages[i]);
    }

    
    return 0;
}


int tls_read(unsigned int offset, unsigned int length, char *buffer) {
    pthread_t tid = pthread_self();

    // check if lsa exists
    int i;
    for (i = 0; i < HASH_SIZE; i++) {
        if (hash_table[i] != NULL && hash_table[i]->tid == tid) {
            break;
        }
    }
    if (i == HASH_SIZE) {
        return -1;
    }

    // check if offset+length > size of lsa
    TLS *tls = hash_table[i]->tls;
    if (offset + length > tls->size) {
        return -1;
    }

    // unprotect pages to allow reading
    for (i = 0; i < tls->page_num; i++) {
        tls_unprotect(tls->pages[i]);
    }

    // begin reading
    int cnt, idx;
    for (cnt = 0, idx = offset; idx < (offset + length); ++cnt, ++idx) {
        struct page *p;
        unsigned int pageNumber, pageOffset;

        // find page number since pages can be in different areas of memory, then find offset within page
        pageNumber = idx / PAGE_SIZE;
        pageOffset = idx % PAGE_SIZE;

        // copy data from page to buffer
        p = tls->pages[pageNumber];
        char *src = ((char *) p->address) + pageOffset;
        buffer[cnt] = *src;
    }

    // protect pages again
    for (i = 0; i < tls->page_num; i++) {
        tls_protect(tls->pages[i]);
    }
    return 0;
}

int tls_destroy() {
    pthread_t tid = pthread_self();

    // check if thread has a local storage area
    int i;
    for (i = 0; i < HASH_SIZE; i++) {
        if (hash_table[i] != NULL && hash_table[i]->tid == tid) {
            break;
        }
    }
    // no lsa so return
    if (i == HASH_SIZE) {
        return -1; 
    }

    struct hash_element *targetHash = hash_table[i];
    int targetHashIdx = i;

    TLS *tls = hash_table[i]->tls;
    // for each page of the lsa
    for (i = 0; i < tls->page_num; i++) {
        // if page shared, decrement ref count but dont free
        if (tls->pages[i]->ref_count > 1) {
            tls->pages[i]->ref_count--;
        } else {
            // free page
            munmap((void *) tls->pages[i]->address, PAGE_SIZE);
            free(tls->pages[i]);
        }
    }
    free(tls->pages);
    free(tls);

    // remove from hash table
    free(targetHash);
    hash_table[targetHashIdx] = NULL;

    return 0;
}


int tls_clone(pthread_t tid) {
    int i;

    // see if tid's storage area exists
    for (i = 0; i < HASH_SIZE; i++) {
        if (hash_table[i] != NULL && hash_table[i]->tid == tid) {
            break;
        }
    }
    if (i == HASH_SIZE) {
        return -1;
    }

    struct hash_element *targetHash = hash_table[i];

    // check if calling thread has lsa already
    pthread_t self = pthread_self();
    for (i = 0; i < HASH_SIZE; i++) {
        if (hash_table[i] != NULL && hash_table[i]->tid == self) {
            return -1;
        }
    }

    // create new tls
    TLS *tls = calloc(1, sizeof(TLS));
    tls->tid = self;
    tls->size = targetHash->tls->size;
    tls->page_num = targetHash->tls->page_num;
    tls->pages = (struct page**) calloc(targetHash->tls->page_num, sizeof(struct page*));

    // set page references to the same as the target thread
    for (i = 0; i < tls->page_num; i++) {
        tls->pages[i] = targetHash->tls->pages[i];
        tls->pages[i]->ref_count++;
    }

    // create new hash element for calling thread
    struct hash_element *hash = calloc(1, sizeof(struct hash_element));
    hash->tid = self;
    hash->tls = tls;
    hash->next = NULL;

    // insert hash into table
    for (i = 0; i < HASH_SIZE; i++) {
        if (hash_table[i] == NULL) {
            hash_table[i] = hash;
            return 0;
        }
    }

    free(hash);
    free(tls->pages);
    free(tls);
    return -1;
}

void tls_protect(struct page *p)
{
    if (mprotect((void *) p->address, PAGE_SIZE, 0)) {
        fprintf(stderr, "tls_protect: could not protect page\n");
        exit(1);
    }
}

void tls_unprotect(struct page *p)
{
    if (mprotect((void *) p->address, PAGE_SIZE, PROT_READ | PROT_WRITE)) {
        fprintf(stderr, "tls_unprotect: could not unprotect page\n");
        exit(1);
    }
}

