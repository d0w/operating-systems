#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include "ec440threads.h"

#define THREADS 10

sem_t semaphore;
int counter = 0;
int ii = 0;

void* thread_function(void* arg) {
    sem_wait(&semaphore);
    printf("Thread %d: ii value: %d\n", *((int*)arg), ii);
    ii++;
    sleep(1);
    counter = counter + ii;  
    printf("Thread %d: Counter value: %d\n", *((int*)arg), counter);
    sem_post(&semaphore);
    pthread_exit(NULL);
}

int main() {
    pthread_t threads[THREADS];
    int thread_ids[THREADS];   

    sem_init(&semaphore, 0, 1);

    int i = 0;
    for (i = 0; i < THREADS; i++) {
        thread_ids[i] = i + 1;
        pthread_create(&threads[i], NULL, thread_function, &thread_ids[i]);
    }

    i = 0;
    for (i = 0; i < THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    sem_destroy(&semaphore);
    return 0;
}