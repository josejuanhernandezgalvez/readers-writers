#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdlib.h>

#pragma clang diagnostic ignored "-Wdeprecated-declarations"

#define READERS 4
#define WRITERS 2

#define MAX_VALUE 16
#define BUFFER_SIZE 256
int buffer[BUFFER_SIZE];

int aw = 0; // active writers
pthread_mutex_t aw_mutex; // aw is synchronized

int rr = 0; // running readers
pthread_mutex_t rr_mutex; // rr is synchronized

sem_t aw_sem; //active writer semaphore
sem_t ar_sem; //active reader semaphore

void delay(int lower, int upper) {
    usleep((random() % (upper-lower) + lower)*10000);
}

void update_rr(int value) {
    pthread_mutex_lock(&rr_mutex); // sync to modify running reader counter

    rr += value;
    printf("-- %d running readers\n", rr);
    if (value == -1 && rr == 0) {
        printf("-- active writers can run\n\n");
        sem_post(&aw_sem); // with the last running reader, active writers are enabled tod run
    }
    else if (value == 1 && rr == 1) {
        sem_wait(&aw_sem); // with the first running reader, active writers should wait
        printf("-- active writers must wait\n");
    }
    pthread_mutex_unlock(&rr_mutex);
}

void update_aw(int value) {
    pthread_mutex_lock(&aw_mutex); // writer sync to modify aw counter

    aw += value;
    printf("-- %d writers\n", aw);
    if (value == -1 && aw == 0) {
        printf("-- active readers can run\n");
        sem_post(&ar_sem); // when there are not active writers, active readers can become running readers
    }
    else if (value == 1 && aw == 1) {
        sem_wait(&ar_sem); // with the first active writer, active readers should wait
        printf("-- active readers must wait\n");
    }

    pthread_mutex_unlock(&aw_mutex);
}

void execute_reader(int id) {
    delay(1, 5);
    int value = 0;
    for (int i = 0; i < BUFFER_SIZE; ++i)
        value += buffer[i];
    printf("reader %2d buffer sum = %d\n", id, value);
    delay(1, 5);
}


void execute_writer(int id) {
    printf("writer %2d running\n", id);
    delay(1, 5);
    int index = (int) (random() % BUFFER_SIZE);
    int value = (int) (random() % MAX_VALUE);
    buffer[index] = value;
    printf("writer %2d setting buffer[%d] = %d\n", id, index, value);
    delay(1, 5);
}

void* reader(void* input) {
    int id = (int) input;
    printf("reader %2d created\n", id);
    sleep(1);
    while (1) {
        delay(30, 50);
        printf("reader %2d activated\n", id);

        sem_wait(&ar_sem);
        printf("reader %2d is running\n", id);
        sem_post(&ar_sem);

        update_rr(+1);
        execute_reader(id);
        printf("reader %2d exits\n", id);
        update_rr(-1);
    }

}

void* writer(void* input) {
    int id = (int) input;
    printf("writer %2d created\n", id);
    sleep(1);
    while (1) {
        delay(50, 100);
        printf("writer %2d activated\n", id);

        update_aw(+1);

        sem_wait(&aw_sem); // active writer waits for running readers
        execute_writer(id);
        sem_post(&aw_sem);

        printf("writer %2d exits\n", id);
        update_aw(-1);

    }
}


int main() {
    sem_init(&aw_sem, 0, 1);
    sem_init(&ar_sem, 0, 1);

    pthread_mutex_init(&rr_mutex, NULL);
    pthread_mutex_init(&aw_mutex, NULL);


    for (int i = 0; i < BUFFER_SIZE; ++i) buffer[i] = (int) (random() % MAX_VALUE);

    pthread_t readers[READERS];
    pthread_t writers[WRITERS];

    for (int i = 0; i < READERS; ++i) pthread_create(&readers[i], NULL, reader, (void *) (i + 1));
    for (int i = 0; i < WRITERS; ++i) pthread_create(&writers[i], NULL, writer, (void *) (i + 1));

    sleep(60*60);

    for (int i = 0; i < READERS; ++i) pthread_cancel(readers[i]);
    for (int i = 0; i < WRITERS; ++i) pthread_cancel(writers[i]);

}
