#include <semaphore.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/types.h>

int flag = 0;
const char* writer_sem_name = "/writer_sem";
const char* reader_sem_name = "/reader_sem";
const char* mem_name = "/my_memory";
char buffer[256];
void* addr;
int fd;
size_t mem_size = 4096;
sem_t* writer_sem;
sem_t* reader_sem;
pthread_t thread;

void sig_handler(int signo) {
    sem_close(writer_sem);
    sem_unlink(writer_sem_name);
    sem_close(reader_sem);
    sem_unlink(reader_sem_name);
    munmap(addr, mem_size);
    close(fd);
    shm_unlink(mem_name);
    exit(EXIT_FAILURE);
}

int generate_message() {
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) == -1) {
        return -1;
    }
    int size = snprintf(buffer, sizeof(buffer), "Первая программа, имя хоста: %s", hostname);
    if (size < 0 || size >= sizeof(buffer)) {
        return -1;
    }
    return size;
}

void* proc(void* args) {
    while (flag == 0) {
        int rv = generate_message();
        if (rv < 0) {
            perror("Первая программа: ошибка генерации сообщения");
            continue;
        }
        memcpy(addr, buffer, rv + 1);
        printf("Сгенерировано и записано сообщение: %s\r\n", buffer);
        sem_post(writer_sem);
        sem_wait(reader_sem);
        sleep(1);
    }
    pthread_exit(NULL);
}

int main() {
    signal(SIGINT, sig_handler);
    fd = shm_open(mem_name, O_CREAT | O_RDWR, 0644);
    ftruncate(fd, mem_size);
    addr = mmap(0, mem_size, PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);
    writer_sem = sem_open(writer_sem_name, O_CREAT, 0644, 0);
    reader_sem = sem_open(reader_sem_name, O_CREAT, 0644, 0);
    pthread_create(&thread, NULL, proc, NULL);
    getchar();
    puts("Первая программа: клавиша нажата.\r\n");
    flag = 1;
    pthread_join(thread, NULL);
    sem_close(writer_sem);
    sem_unlink(writer_sem_name);
    sem_close(reader_sem);
    sem_unlink(reader_sem_name);
    munmap(addr, mem_size);
    close(fd);
    shm_unlink(mem_name);
    return 0;
}