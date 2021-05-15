#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <libc.h>

#pragma clang diagnostic ignored "-Wdeprecated-declarations"

#define READERS 10
#define WRITERS 8

#define MAX_VALUE 16
#define BUFFER_SIZE 256
int buffer[BUFFER_SIZE];

int aw = 0; // active writers
pthread_mutex_t aw_mutex; // aw is synchronized

int rr = 0; // running readers
pthread_mutex_t rr_mutex; // rr is synchronized

sem_t aws;
sem_t ars;

pthread_mutex_t screen = PTHREAD_MUTEX_INITIALIZER;


void make_running_reader(int id) {
    pthread_mutex_lock(&rr_mutex); // sync to modify running reader counter

    pthread_mutex_lock(&screen);
    printf("reader %2d becomes running reader\n", id);
    rr++;
    printf("-- %d running readers\n", rr);
    if (rr == 1) {
        sem_wait(&aws); // with the first running reader, active writers should wait
        printf("-- active writers should wait\n");
    }
    pthread_mutex_unlock(&screen);

    pthread_mutex_unlock(&rr_mutex);
}

void do_read(int id) {
    int value = 0;
    for (int i = 0; i < BUFFER_SIZE; ++i) {
        value += buffer[i];
        usleep(1000);
    }
    printf("reader %2d buffer sum = %d\n", id, value);
}

void terminate_running_reader(int id) {
    pthread_mutex_lock(&rr_mutex); // sync to modify running reader counter

    pthread_mutex_lock(&screen);
    printf("reader %2d terminates\n",id);
    rr--;
    printf("-- %d running readers\n", rr);
    if (rr == 0) {
        printf("-- active writers can write\n\n");
        sem_post(&aws); // with the last running reader, active writers are enabled tod run
    }
    pthread_mutex_unlock(&screen);

    pthread_mutex_unlock(&rr_mutex); // sync to modify running reader counter

}


void* reader(void* input) {
    int id = (int) input;
    printf("reader %2d created\n", id);
    sleep(1);
    while (1) {
        usleep(random() % 1000 + 100);
        printf("reader %2d awaken\n", id);

        printf("reader %2d waiting to be a running reader\n", id);
        sem_wait(&ars); // wait to become a running reader
        make_running_reader(id);
        sem_post(&ars);

        do_read(id);

        terminate_running_reader(id);

    }

}

void start_writer() {
    pthread_mutex_lock(&aw_mutex); // writer sync to modify aw counter

    pthread_mutex_lock(&screen);
    aw++;
    printf("-- %d active writers\n", aw);
    if (aw == 1) {
        sem_wait(&ars); // with the first active writer, active readers should wait
        printf("-- active readers should wait\n");
    }
    pthread_mutex_unlock(&screen);

    pthread_mutex_unlock(&aw_mutex);
}

void do_write(int id) {
    int index = (int) (random() % BUFFER_SIZE);
    int value = (int) (random() % MAX_VALUE);
    buffer[index] = value;
    printf("writer %2d setting buffer[%d] = %d\n", id, index, value);
    usleep(10000);
}

void terminate_writer(int id) {
    pthread_mutex_lock(&aw_mutex); // writer sync to modify aw counter

    pthread_mutex_lock(&screen);
    printf("writer %2d terminates\n",id);
    aw--;
    printf("-- %d active writers\n", aw);
    if (aw == 0) {
        printf("-- active readers can read\n");
        sem_post(&ars); // when there are not active writers, active readers can become running readers
    }
    pthread_mutex_unlock(&screen);

    pthread_mutex_unlock(&aw_mutex);

}


void* writer(void* input) {
    int id = (int) input;
    printf("writer %2d created\n", id);
    sleep(1);
    while (1) {
        usleep(random() % 1000 + 100);
        printf("writer %2d awaken\n", id);

        start_writer();

        sem_wait(&aws); // active writer waits for running readers
        do_write(id);
        sem_post(&aws);

        terminate_writer(id);
    }
}

int main() {
    pthread_mutex_init(&screen, NULL);
    sem_init(&aws, 0, 0);
    sem_init(&ars, 0, 0);

    pthread_mutex_init(&rr_mutex, NULL);
    pthread_mutex_init(&aw_mutex, NULL);


    for (int i = 0; i < BUFFER_SIZE; ++i) buffer[i] = (int) (random() % MAX_VALUE);

    pthread_t readers[READERS];
    pthread_t writers[READERS];

    for (int i = 0; i < READERS; ++i) pthread_create(&readers[i], NULL, reader, (void *) (i + 1));
    for (int i = 0; i < WRITERS; ++i) pthread_create(&writers[i], NULL, writer, (void *) (i + 1));

    sleep(60*60);

    for (int i = 0; i < READERS; ++i) pthread_cancel(readers[i]);
    for (int i = 0; i < WRITERS; ++i) pthread_cancel(writers[i]);

}

