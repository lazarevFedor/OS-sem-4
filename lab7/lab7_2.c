#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <locale.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/ipc.h>

typedef struct {
    long mtype;
    char mtext[256];
} msg_t;

pthread_t thread;
int msgid;
int flag = 0;

void* proc(void* args) {
    msg_t message;
    while (flag == 0) {
        memset(message.mtext, 0, 256);
        ssize_t rv = msgrcv(msgid, &message, 256, 1, IPC_NOWAIT);
        if (rv == -1) {
            if (errno == ENOMSG) {
                continue;
            }
            printf("Вторая программа: ошибка получения сообщения: %s\r\n", strerror(errno));
        } else {
            printf("Получено сообщение: %s\r\n", message.mtext);
        }
    }
    pthread_exit(NULL);
}

int main() {
    key_t key = ftok("lab7_1.c", 'A');
    if (key == -1) {
        perror("ftok failed");
        return 1;
    }
    msgid = msgget(key, 0);
    if (msgid < 0) {
        msgid = msgget(key, IPC_CREAT | 0644);
    }
    pthread_create(&thread, NULL, proc, NULL);
    getchar();
    puts("Вторая программа: клавиша нажата.\r\n");
    flag = 1;
    pthread_join(thread, NULL);
    msgctl(msgid, IPC_RMID, NULL);
    return 0;
}