#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>


int flag1 = 0, flag2 = 0;
int pipefd[2];

void* proc1(void* args) {
    char buffer[256];
    while (!flag1) {
        if (gethostname(buffer, sizeof(buffer)) == -1) {
            perror("Ошибка получение имени хоста:");
            continue;
        }
        if (write(pipefd[1], buffer, strlen(buffer) + 1) == -1) {
            perror("Ошибка записи:");
        }
        sleep(1);
    }
    return NULL;
}


void* proc2(void* args) {
    char buffer[256];
    while (!flag2) {
        memset(buffer, 0, sizeof(buffer));
        if (read(pipefd[0], buffer, sizeof(buffer)) > 0) {
            printf("Полученное имя хоста: %s\n", buffer);
        } else {
            perror("Ошибка чтения:");
        }
        sleep(1);
    }
    return NULL;
}


int main(int argc, char* argv[]) {
    pthread_t id1, id2;
    int rv;
    if (argc == 1) {
        rv = pipe(pipefd);
        fcntl(pipefd[0], F_SETFL, O_NONBLOCK);
        fcntl(pipefd[1], F_SETFL, O_NONBLOCK);
    }
    else {
        if (strcmp(argv[1], "1\0") == 0) {
            rv = pipe(pipefd);
        }
        else if (strcmp(argv[1], "2\0") == 0) {
            rv = pipe2(pipefd, O_NONBLOCK);
        }
        else if (strcmp(argv[1], "3\0") == 0) {
            rv = pipe(pipefd);
            fcntl(pipefd[0], F_SETFL, O_NONBLOCK);
            fcntl(pipefd[1], F_SETFL, O_NONBLOCK);
        }
    }
    pthread_create(&id1, NULL, proc1, NULL);
    pthread_create(&id2, NULL, proc2, NULL);
    getchar();
    puts("Клавиша нажата.\r\n");
    flag1 = 1;
    flag2 = 1;
    pthread_join(id1, NULL);
    pthread_join(id2, NULL);
    close(pipefd[0]);
    close(pipefd[1]);
    return 0;
}
