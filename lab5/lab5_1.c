#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <semaphore.h>
#include <locale.h>
#include <signal.h>
#include <time.h>
#include <fcntl.h>
#include <termios.h>
#include <stdbool.h>

#define _GNU_SOURCE

sem_t* sem;
int fd;
bool flag;
struct timespec tv;

void deleteSemaphore(const char* msg) {
    perror(msg);
    sem_close(sem);
    sem_unlink("/my_sem");
    exit(-1);
}

int kbhit(void) {
    struct termios oldt, newt;
    int ch;
    int oldf;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);
    if (ch != EOF) {
        ungetc(ch, stdin);
        return 1;
    }
    return 0;
}

int main() {
    sem = sem_open("/my_sem", O_CREAT, 0644, 1);
    if (sem == SEM_FAILED) {
        deleteSemaphore("Ошибка создания семафора");
    }
    fd = open("file.txt", O_CREAT | O_WRONLY | O_NONBLOCK | O_APPEND, 0644);
    if (fd == -1) {
        deleteSemaphore("Ошибка открытия файла");
    }
    tv.tv_sec = 1;
    tv.tv_nsec = 0;
    flag = true;
    while (flag == true) {
        sem_wait(sem);
        for (int i = 0; i < 10; ++i) {
            write(fd, "1", 1);
            printf("1");
            sleep(1);
        }
        sem_post(sem);
        if (kbhit()) {
            flag = false;
        }
    }
    if (close(fd) == -1) {
        deleteSemaphore("Ошибка закрытия файла");
    }
    sem_close(sem);
    sem_unlink("/my_sem");
    return 0;
}