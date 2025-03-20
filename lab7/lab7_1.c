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
int len;
int rv;
int flag = 0;
msg_t message;

int generate_message(char* buf) {
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) == -1) {
        return -1;
    }
    memset(buf, 0, 256);
    int size = sprintf(buf, "Имя хоста: %s", hostname);
    return size;
}

void* proc(void* args) {
    while (flag == 0) {
        len = generate_message(message.mtext);
        if (len < 0) {
            perror("Первая программа: ошибка генерации сообщения");
        }
        rv = msgsnd(msgid, &message, len, IPC_NOWAIT);
        if (rv == -1) {
            printf("Первая программа: ошибка передачи сообщения: %s\r\n", strerror(errno));
        }
        else if (rv == 0) {
            printf("Сгенерировано и записано сообщение: %s\r\n", message.mtext);
        }
        sleep(1);
    }
    pthread_exit(NULL);
}

int main() {
    key_t key = ftok("lab7_1.c", 'A');
    if (key == -1) {
        perror("ftok failed");
        return 1;
    }
    message.mtype = 1;
    msgid = msgget(key, 0);
    if (msgid < 0) {
        msgid = msgget(key, IPC_CREAT | 0644);
    }
    pthread_create(&thread, NULL, proc, NULL);
    getchar();
    puts("Первая программа: клавиша нажата.\r\n");
    flag = 1;
    pthread_join(thread, NULL);
    msgctl(msgid, IPC_RMID, NULL);
    return 0;
}