#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define X 5
#define NUM_WRITERS 5
#define NUM_READERS 5

int GLOBAL_VAR = 0;
int num_resource = 0;

void * writer(void * param);
void * reader(void * param);

pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t c_writer = PTHREAD_COND_INITIALIZER;
pthread_cond_t c_reader = PTHREAD_COND_INITIALIZER;

int main(int argc, const char * argv[]) {
    time_t t;
    srand((unsigned) time(&t));

    pthread_t readers[NUM_READERS];
    pthread_t writers[NUM_WRITERS];

    for(int i = 0; i < NUM_READERS; i++) {
        pthread_create(readers + i, NULL, reader, NULL);
    }

    for(int i = 0; i < NUM_WRITERS; i++) {
        pthread_create(writers + i, NULL, writer, NULL);
    }

    for(int i = 0; i < NUM_READERS; i++) {
        pthread_join(readers[i], NULL);
    }

    for(int i = 0; i < NUM_WRITERS; i++) {
        pthread_join(writers[i], NULL);
    }

    printf("Parent thread quiting...\n");
}

void * writer(void * param) {
    for(int i = 0; i < X; i++) {
        int rand_num = rand() % 50;
        usleep(rand_num);
        pthread_mutex_lock(&m);
            while(num_resource != 0) {
                pthread_cond_wait(&c_writer, &m);
            }
            num_resource = -1;
        pthread_mutex_unlock(&m);

        // Write i //
        GLOBAL_VAR = rand_num;
        printf("WRITER : VALUE %d WRITTEN\n", rand_num);
        printf("%d READERS ARE PRESENT\n", num_resource + 1);

        num_resource = 0;
        pthread_cond_broadcast(&c_reader);
        pthread_cond_signal(&c_writer);
    }
}

void * reader(void * param) {
    for(int i = 0; i < X; i++) {
        int rand_num = rand() % 50;
        usleep(rand_num);
        pthread_mutex_lock(&m);
            while(num_resource == -1) {
                pthread_cond_wait(&c_reader, &m);
            }
            num_resource++;
        pthread_mutex_unlock(&m);

        // Read GLOBAL_VAR //
        printf("READER: VALUE %d FROM GLOBAL_VAR READ.\n", GLOBAL_VAR);
        printf("%d READERS ARE PRESENT\n", num_resource);

        pthread_mutex_lock(&m);
            num_resource--;
        pthread_mutex_unlock(&m);
        if(num_resource == 0) {
            pthread_cond_signal(&c_writer);
        }
    }
}