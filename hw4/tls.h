#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>

#define HASH_SIZE 128
// #define PAGE_SIZE 4096


// local storage struct
typedef struct thread_local_storage {
    pthread_t tid; 
    unsigned int size; // size in bytes
    unsigned int page_num; // # pages 
    struct page **pages; // array of pointers to pages
} TLS;

// page struct
struct page {
    unsigned long int address; // start address of page
    int ref_count; // counter for shared pages
};

// hash element to connect tid to TLS
struct hash_element
{
    pthread_t tid;
    TLS *tls;
    struct hash_element *next;
};

/**
 * Initializes TLS system
 */
void tls_init();

/**
 * Creates new LSA for thread
 * @param size size of LSA in bytes
 * @return 0 on success, -1 on failure
 */
int tls_create(unsigned int size);

/**
 * Writes to LSA
 * @param offset offset to start writing to
 * @param length number of bytes to write
 * @param buffer buffer to write from
 * @return 0 on success, -1 on failure
 */
int tls_write(unsigned int offset, unsigned int length, char *buffer);

/**
 * Reads bytes into target buffer
 * @param offset offset to start reading from
 * @param length number of bytes to read
 * @param buffer buffer to read into
 * @return 0 on success, -1 on failure
 */
int tls_read(unsigned int offset, unsigned int length, char *buffer);

/**
 * Destorys LSA
 * @return 0 on success, -1 on failure
 */
int tls_destroy();

/**
 * Clones local storage using copy on write
 * @param tid thread id of the thread to clone
 * @return 0 on success, -1 on failure
 */
int tls_clone(pthread_t tid);

/**
 * Handles page faults
 * @param sig signal number
 * @param si signal info
 * @param context context
 * @return void
 */
void tlsPageFaultHandler(int sig, siginfo_t *si, void *context);

/**
 * Protects page
 * @param p page to protect
 * @return void
 */
void tls_protect(struct page *p);

/**
 * Unprotects page
 * @param p page to unprotect
 * @return void
 */
void tls_unprotect(struct page *p);