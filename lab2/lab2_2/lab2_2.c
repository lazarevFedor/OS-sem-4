#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <signal.h>
#include <semaphore.h>


int flag1 = 0, flag2 = 0;
int i, j;
sem_t semaphore;


void sig_handler(int signo) {
    printf("\r\nCaught SIGINT, code %d\r\n", signo);
    sem_destroy(&semaphore);
    exit(0);
}


void* proc1(void* args) {
    while (flag1 == 0) {
        sem_wait(&semaphore);
        for (i = 0; i < 10; ++i) {
            printf("%c", '1');
            fflush(stdout);
            sleep(1);
        }
        sem_post(&semaphore);
        sleep(1);
    }
    pthread_exit(NULL);
}


void* proc2(void* args) {
    while (flag2 == 0) {
        sem_wait(&semaphore);
        for (j = 0; j < 10; ++j) {
            printf("%c", '2');
            fflush(stdout);
            sleep(1);
        }
        sem_post(&semaphore);
        sleep(1);
    }
    pthread_exit(NULL);
}


int main() {
    pthread_t id1, id2;
    signal(SIGINT, sig_handler);
    sem_init(&semaphore, 0, 1);
    pthread_create(&id1, NULL, proc1, NULL);
    pthread_create(&id2, NULL, proc2, NULL);
    getchar();
    puts("Клавиша нажата.\r\n");
    flag1 = 1;
    flag2 = 1;
    sem_destroy(&semaphore);
    pthread_join(id1, NULL);
    pthread_join(id2, NULL);
}