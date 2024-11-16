#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/mman.h>

#define HASH_SIZE 128
// #define PAGE_SIZE 4096



typedef struct thread_local_storage {
    pthread_t tid; 
    unsigned int size; // size in bytes
    unsigned int page_num; // # pages 
    struct page **pages; // array of pointers to pages
} TLS;

struct page {
    unsigned int address; // start address of page
    int ref_count; // counter for shared pages
};

struct hash_element
{
    pthread_t tid;
    TLS *tls;
    struct hash_element *next;
};

// initializes tls system
void tls_init();

/*
This function reads length bytes from the local storage area of the currently executing thread,
starting at position offset, and writes them into the memory location pointed to by buffer. The
function returns 0 on success or -1 (error) if it is asked to read past the end of the LSA (i.e.,
offset + length > size of LSA) or if the current thread has no LSA. Finally, the function trusts
that the buffer into which data is written is large enough to hold at least length bytes. If not, the
result of the call is undefined.
*/
int tls_create(unsigned int size);

/*
This function reads length bytes, starting from the memory location pointed to by buffer, and
writes them into the local storage area of the currently executing thread, starting at position
offset. It returns 0 on success and -1 (error) if the function is asked to write more data than the
LSA can hold (i.e., offset + length > size of LSA) or if the current thread has no LSA. Finally,
the function trusts that the buffer from which data is read holds at least length bytes. If not, the
result of the call is undefined
*/
int tls_write(unsigned int offset, unsigned int length, char *buffer);

/*
This function reads length bytes from the local storage area of the currently executing thread,
starting at position offset, and writes them into the memory location pointed to by buffer. The
function returns 0 on success or -1 (error) if it is asked to read past the end of the LSA (i.e.,
offset + length > size of LSA) or if the current thread has no LSA. Finally, the function trusts
that the buffer into which data is written is large enough to hold at least length bytes. If not, the
result of the call is undefined.
*/
int tls_read(unsigned int offset, unsigned int length, char *buffer);

/*
This function frees a previously allocated local storage area of the currently executing thread. It
returns 0 on success and -1(error) when the thread does not have a local storage area.
*/
int tls_destroy();

/*
This function clones the local storage area of a target thread identified by tid. When a thread
local storage is cloned, the content is not simply copied. Instead, the storage areas of both
threads initially refer to the same memory location. Only when one thread writes to its own LSA
(using the tls_write function), then the TLS library creates a private copy of the region that
is written. Note, though, that the remaining, untouched areas still remain shared. This approach
is called CoW (copy-on-write), and it is done to save memory space and to avoid unnecessary
copy operations. The function returns 0 on success. It is an error when the target thread has no
LSA, or when the currently executing thread already has a LSA. In both cases, the function
returns -1.
*/
int tls_clone(pthread_t tid);

void tlsPageFaultHandler(int sig, siginfo_t *si, void *context);

void tls_protect(struct page *p);

void tls_unprotect(struct page *p);