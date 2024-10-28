#ifndef __EC440THREADS__
#define __EC440THREADS__

#include <pthread.h>
#include <setjmp.h>
#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <semaphore.h>

#define JB_RBX 0
#define JB_RBP 1
#define JB_R12 2
#define JB_R13 3
#define JB_R14 4
#define JB_R15 5
#define JB_RSP 6
#define JB_PC 7


#define MAX_THREADS 128
#define MAX_STACK_SIZE 32767
#define QUANTUM 50000 // 50000 microseconds = 50 milliseconds


enum threadStatus {
    READY,
    RUNNING,
    BLOCKED,
    EXITED
};

typedef struct TCB {
    pthread_t tid;
    jmp_buf context;
    enum threadStatus status;
    
    // allocated stack
    void *stack;

    // return value
    void *returnVal;
} TCB;

typedef struct Queue {
    TCB *thread;
    struct Queue *next;
} Queue;

typedef struct Semaphore{
    int value;
    struct Queue *head;
    struct Queue *tail;
    int initialized;
} Semaphore;


// ------------------ LIBRARY -----------------
/**
 * Sets the register value in the context
 * @param context Context to set the register in
 * @param reg Register to set
 * @param value Value to set the register to
 * @return Index in jmp_buf as defined by the OS if successful, -1 if error
 */
int setRegister(jmp_buf *context, int reg, unsigned long int value);

/**
 * Gets the register value in the context
 * @param context Context to get the register from
 * @param reg Register to get
 * @return Value of the register if successful, -1 if error
 */
unsigned long int getRegister(jmp_buf *context, int reg);

/**
 * Schedules the next thread to run
 * @return void
 */
void schedule();

/**
 * Run on first thread creation to initialize threading. 
 * Sets up the main thread and timer for the scheduler.
 */
void initializeThreading();

/**
 * Creates a thread. Sets up TCB for the scheduler to choose from.
 * @param thread Pointer to the thread
 * @param attr Attributes of the thread - always null
 * @param start_routine Function to run in the thread
 * @param arg Argument to pass to the function
 * @return exit status of the thread or -1 for errors
 */
int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg);


/**
 * Terminates calling thread. The process shall exit with an exit status of 0 after the last
thread has been terminated. The behavior shall be as if the implementation called exit() with a
zero argument at thread termination time.
 * @param value_ptr Exit status of the thread - ignored here
 * @return void
 */
void pthread_exit(void *value_ptr);


/**
 * Returns the thread ID of the calling thread.
 * @return Thread ID of the calling thread
 */
pthread_t pthread_self(void);

/**
 * Makes calling thread wait for joined thread to finish before exiting
 */
int pthread_join(pthread_t thread, void **value_ptr);

/**
 * Prevents thread from being ended until the thread is unlocked.
 */
void lock();

/**
 * Unlocks the thread so it can be ended/switched
 */
void unlock();

int sem_init(sem_t *sem, int pshared, unsigned value);

int sem_wait(sem_t *sem);

int sem_post(sem_t *sem);

int sem_destroy(sem_t *sem);

void pthread_exit_wrapper();




// JMP_BUF DEFINITION taken from bandit
/* Define the machine-dependent type `jmp_buf'.  x86-64 version.  */
// #ifndef _BITS_SETJMP_H
// #define _BITS_SETJMP_H  1

// #if !defined _SETJMP_H && !defined _PTHREAD_H
// # error "Never include <bits/setjmp.h> directly; use <setjmp.h> instead."
// #endif

// #include <bits/wordsize.h>

// #ifndef _ASM

// # if __WORDSIZE == 64
// typedef long int __jmp_buf[8];
// # elif defined  __x86_64__
// __extension__ typedef long long int __jmp_buf[8];
// # else
// typedef int __jmp_buf[6];
// # endif

// #endif

// #endif  /* bits/setjmp.h */

unsigned long int ptr_demangle(unsigned long int p);

unsigned long int ptr_mangle(unsigned long int p);

void *start_thunk();

#endif
