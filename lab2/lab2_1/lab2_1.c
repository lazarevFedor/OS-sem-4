#include <stdio.h>
#include <unistd.h>
#include <locale.h>
#include <pthread.h>

int flag1 = 0, flag2 = 0;
int i, j;

void* proc1(void* args) {
    while (flag1 == 0) {
        for (i = 0; i < 10; ++i) {
            printf("%c", '1');
            fflush(stdout);
            sleep(1);
        }
        sleep(1);
    }
    pthread_exit(NULL);
}

void* proc2(void* args) {
    while (flag2 == 0) {
        for (j = 0; j < 10; ++j) {
            printf("%c", '2');
            fflush(stdout);
            sleep(1);
        }
        sleep(1);
    }
    pthread_exit(NULL);
}

int main() {
    pthread_t id1, id2;
    pthread_create(&id1, NULL, proc1, NULL);
    pthread_create(&id2, NULL, proc2, NULL);
    getchar();
    puts("Клавиша нажата.\r\n");
    flag1 = 1;
    flag2 = 1;
    pthread_join(id1, NULL);
    pthread_join(id2, NULL);
    return 0;
}
