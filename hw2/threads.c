#include "ec440threads.h"

// function documentation in header file

// ------------------ GLOBAL VARIABLES ------------------
struct TCB *gThreads[MAX_THREADS] = { NULL };
pthread_t gCurrent = 0;
struct sigaction sa;


// ------------------ FUNCTIONS -------------------------

void lock() {
    // prevent thread from being switched out of
    sigset_t mask;
    sigfillset(&mask);
    sigprocmask(SIG_SETMASK, &mask, NULL);
}

void unlock() {
    // allow thread to be switched out of
    sigset_t mask;
    sigemptyset(&mask);
    sigprocmask(SIG_SETMASK, &mask, NULL);
}


// func documentation in header file
int setRegister(jmp_buf *context, int reg, unsigned long int value) {
    // set registers
    switch(reg) {
        // all registers that do not need to be mangled
        case JB_R12:
        case JB_R13:
        case JB_R14:
        case JB_R15:
            (*context)->__jmpbuf[reg] = value;
            return reg;
        
        // all registers that need to be mangled
        case JB_PC:
        case JB_RSP:
        case JB_RBP:
        case JB_RBX:
            (*context)->__jmpbuf[reg] = ptr_mangle(value);
            return reg;

    }
    return -1;
}

void schedule() {
    // save current thread

    // printf("Scheduling\n");

    // switch status to ready
    if (gThreads[gCurrent] != NULL && gThreads[gCurrent]->status == RUNNING) {
        gThreads[gCurrent]->status = READY;
    }

    // save current thread context if not exited
    if (gThreads[gCurrent] != NULL && gThreads[gCurrent]->status != EXITED) {
        if (setjmp(gThreads[gCurrent]->context) != 0) {
            // printf("Restoring context of thread %ld\n", gCurrent);
            // we are returning from long jump here so we should return
            return;
        }
        // printf("Saving context of thread %ld\n", gCurrent);
        // if setjmp is 0, we are just saving the context of the previous thread
    }


    // find next thread to run using round robin
    int next = (gCurrent + 1) % MAX_THREADS;

    // keep looping until we find a thread that is ready
    // int checkedAll = 0;
    int count = 0;
    while (gThreads[next] == NULL || gThreads[next]->status != READY) {
        if (gThreads[next] == NULL) {
            count++;
        }
        next = (next + 1) % MAX_THREADS;

        // if there are no more threads open, exit
        if (count == MAX_THREADS) {
            // exit(0);
        }
    }



    // set status to running and update gCurrent
    gThreads[next]->status = RUNNING;
    gCurrent = next;


    // run next thread
    // printf("Switching to thread %ld\n", gCurrent);
    longjmp(gThreads[next]->context, 1); 
}


void initializeThreading() {
    // set up main thread
    struct TCB *mainThread = malloc(sizeof(struct TCB));
    mainThread->tid = 0;
    mainThread->status = RUNNING;
    gThreads[0] = mainThread;
    gCurrent = 0;

    // set up timer using timer and signal handler
    // configure timer
    struct itimerval timer;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = QUANTUM;
    timer.it_value = timer.it_interval;

    // start timer
    if (setitimer(ITIMER_REAL, &timer, NULL) < 0) {
        perror("Error setting timer");
        exit(1);
    }

    // signal handler

    sa.sa_handler = &schedule;
    sa.sa_flags = SA_NODEFER;
    sigaction(SIGALRM, &sa, NULL);




}




int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg) {
    static int initialized = 0;
    if (!initialized) {
        // initialize program to start multithreading (scheduler and create TCB for main)
        // perform initializtion here
        initializeThreading();
        initialized = 1;
    }

    // find empty slot in gThreads
    int id = 1;
    for (id = 1; id < MAX_THREADS; id++) {
        if (id >= MAX_THREADS) {
            perror("Max threads reached");
            return -1;
        }
        else if (gThreads[id] == NULL) {
            break;
        }
    }

    // create TCB
    struct TCB *newThread = malloc(sizeof(struct TCB));

    // create stack, set stack pointer in TCB
    newThread->stack = malloc(MAX_STACK_SIZE);


    // set id of thread and add to thread list
    gThreads[id] = newThread;
    *thread = id;
    newThread->tid = id;


    // since an implicit call to pthreadexit is required, we need to set a ptr 
    // to the function the bottom of the stack (notation following will assume the stack grows down)
    // want to place address of pthread_exit at the top of the stack
    
    // find stack top (growing down) and go down by one function pointer
    unsigned long int *stackPtr = newThread->stack + MAX_STACK_SIZE;
    stackPtr -= 1;
    *stackPtr = (unsigned long int) &pthread_exit_wrapper;


    // set context into jmp_buf
    // set PC to start_thunk to insert args. Function Places R13 value into the first argument to function in R12
    // also need to set RSP to the stack pointer
    setRegister(&(newThread->context), JB_PC, (unsigned long int)start_thunk);
    setRegister(&(newThread->context), JB_R13, (unsigned long int)arg);
    setRegister(&(newThread->context), JB_R12, (unsigned long int)start_routine);
    setRegister(&(newThread->context), JB_RSP, (unsigned long int)(stackPtr));

    // set status to ready
    newThread->status = READY;

    // printf("Thread %d created\n", id);
    // printf("Thread %d status: %d\n", id, newThread->status);
    // printf("Thread %d stack: %p\n", id, newThread->stack);
    // printf("Thread %d context: %p\n", id, &(newThread->context));

    return 0;
}

// gets return value of thread
void pthread_exit_wrapper()
{
    unsigned long int res;
    asm("movq %%rax, %0\n":"=r"(res));
    pthread_exit((void *) res);
}

void pthread_exit(void *value_ptr) {
    lock();
    gThreads[gCurrent]->status = EXITED;

    if(value_ptr) {
        gThreads[gCurrent]->returnVal = value_ptr;
    }
    gThreads[gCurrent]->status = EXITED;
    unlock();
    schedule();
    exit(0);


    // int i;
    // int allNull = 1;
    // for (i = 0; i < MAX_THREADS; i++) {
    //     if (gThreads[i] != NULL) {
    //         allNull = 0;
    //     }
    // }
    // if (allNull) {
    //     exit(0);
    // }
    // while(1) {
    //     schedule();
    // }
    // schedule();
    // exit(0);
}

pthread_t pthread_self(void) {
    return gCurrent;
}

int pthread_join(pthread_t thread, void **value_ptr) {
    // lock to prevent race condition
    lock();
    // pthread_t current = pthread_self();

    // check if thread is valid
    if (thread < 0 || thread >= MAX_THREADS || gThreads[thread] == NULL) {
        unlock();
        return -1; // Invalid thread ID
    }


    // wait for thread to exit by checking if status is exited
    // this is essentially a blocked thread
    while (gThreads[thread]->status != EXITED) {
        // if not exited, unlock and schedule another thread
        unlock();
        schedule();
        lock();
    }


    // gThreads[current]->status = READY;

    // set returnVal to return value of thread
    if (value_ptr != NULL) {
        *value_ptr = gThreads[thread]->returnVal;
    }

    // free memory and set to NULL
    free(gThreads[thread]->stack);
    free(gThreads[thread]);
    gThreads[thread] = NULL;

    unlock();
    return 0;

}

int sem_init(sem_t *sem, int pshared, unsigned value) {
    // intialiaze semaphore with value and queue
    Semaphore *newSem = malloc(sizeof(Semaphore));
    newSem->value = value;
    newSem->head = NULL;
    newSem->tail = NULL;
    newSem->initialized = 1;
    sem->__align = (unsigned long int)newSem;
    return 0;
}

int sem_wait(sem_t *sem) {
    lock();
    // decrement value of semaphore if greater than 0
    Semaphore *curr = (Semaphore *)sem->__align;
    if (curr->value > 0) {
        curr->value--;
    } else {
        // if value is 0, dont decrement and block thread
        // Add the current thread to the waiting queue
        Queue *newItem = malloc(sizeof(Queue));
        newItem->thread = gThreads[gCurrent];
        newItem->next = NULL;
        if (curr->tail) {
            curr->tail->next = newItem;
        } else {
            curr->head = newItem;
        }
        curr->tail = newItem;
        gThreads[gCurrent]->status = BLOCKED;

        // unlock and schedule another thread
        unlock();
        schedule();
        lock();
    }
    unlock();
    return 0;
}

int sem_post(sem_t *sem) {
    lock();
    Semaphore *curr = (Semaphore *)sem->__align;

    // increment value of semaphore
    curr->value++;

    // unblock start of queue and consume it if semaphore is greater than 0
    if (curr->head && curr->value > 0) {
        Queue *unblock = curr->head;
        curr->head = curr->head->next;
        if (curr->head == NULL) {
            curr->tail = NULL;
        }
        unblock->thread->status = READY;
    }

    unlock();
    return 0;

}

int sem_destroy(sem_t *sem) {
    lock();
    Semaphore *curr = (Semaphore *)sem->__align;

    // do not destroy if not initialized by sem_init
    if (curr->initialized != 1) {
        unlock();
        return -1;
    }
    curr->initialized = 0;

    // free all items in queue
    Queue *current = curr->head;
    while(current != NULL) {
        Queue *temp = current;
        current = current->next;
        free(temp);
    }

    free(curr);
    unlock();
    return 0;
}

unsigned long int ptr_demangle(unsigned long int p)
{
    unsigned long int ret;

    asm("movq %1, %%rax;\n"
        "rorq $0x11, %%rax;"
        "xorq %%fs:0x30, %%rax;"
        "movq %%rax, %0;"
    : "=r"(ret)
    : "r"(p)
    : "%rax"
    );
    return ret;
}

unsigned long int ptr_mangle(unsigned long int p)
{
    unsigned long int ret;

    asm("movq %1, %%rax;\n"
        "xorq %%fs:0x30, %%rax;"
        "rolq $0x11, %%rax;"
        "movq %%rax, %0;"
    : "=r"(ret)
    : "r"(p)
    : "%rax"
    );
    return ret;
}

void *start_thunk() {
  asm("popq %%rbp;\n"           //clean up the function prolog
      "movq %%r13, %%rdi;\n"    //put arg in $rdi
      "pushq %%r12;\n"          //push &start_routine
      "retq;\n"                 //return to &start_routine
      :
      :
      : "%rdi"
  );
  __builtin_unreachable();
}
