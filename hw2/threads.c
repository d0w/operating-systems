#include "ec440threads.h"

// function documentation in header file

// ------------------ GLOBAL VARIABLES ------------------
struct TCB *gThreads[MAX_THREADS] = { NULL };
pthread_t gCurrent = 0;
struct sigaction sa;


// ------------------ FUNCTIONS -------------------------

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
        next = (next + 1) % MAX_THREADS;

        // if there are no more threads open, exit
        if (count == MAX_THREADS) {
            exit(0);
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
    void *stackTop = newThread->stack + MAX_STACK_SIZE;
    void *stackPtr = stackTop - sizeof(void *);
    void (*exitPtr)(void *) = (void *) &pthread_exit;
    
    // get pointer to pthread exit
    // copy pthread exit pointer into the stack so it is called when start_routine is done
    memcpy(stackPtr, &exitPtr, sizeof(void *));


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




    // static int initialized = 1;
    // if (initialized) {
    //     initialized = 0;
    //     initializeThreading();
    // }
    
    // int ii = 1;
    // for(ii = 1; gThreads[ii] != NULL; ii++) {
    //     if(ii == MAX_THREADS - 1) {
    //         printf("max treads reached!");
    //         return -1;
    //     }
    // }

    // *thread = ii;
    // struct TCB* newthread = malloc(sizeof(struct TCB));
    // newthread->stack = malloc(MAX_STACK_SIZE);
    // gThreads[ii] = newthread;
    
    // setRegister(&(newthread->context), JB_PC, (unsigned long int)start_thunk);
    // setRegister(&(newthread->context), JB_R13, (unsigned long int)arg);
    // setRegister(&(newthread->context), JB_R12, (unsigned long int)start_routine);
    
    // //move the prethread_exit to the bottom of the stack
    // void *bottom_of_stack = newthread->stack + MAX_STACK_SIZE;
    // void *stack_pointer = bottom_of_stack - sizeof(&start_routine);
    // void (*func)(void*) = (void*) &pthread_exit;
    
    // //move the new stackpointer to the stack
    // memcpy(stack_pointer, &func, sizeof(func));
    // setRegister(&(newthread->context), JB_RSP, (unsigned long int)stack_pointer);
    
    // newthread->tid = ii;
    // newthread->status = READY;

    // return 0;
}

void pthread_exit(void *value_ptr) {
    // set status
    gThreads[gCurrent]->status = EXITED;

    // free values in TCB
    free(gThreads[gCurrent]->stack);
    free(gThreads[gCurrent]);
    gThreads[gCurrent] = NULL;

    if (gThreads[gCurrent] != NULL) {
        fprintf(stderr, "Error freeing thread: %ld", gCurrent);
    }

    // reschedule for next thread and then exit
    schedule();
    exit(0);
}


pthread_t pthread_self(void) {
    return gCurrent;
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
