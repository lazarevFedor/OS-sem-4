#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <locale.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/queue.h>


struct message {
    char msg[256];
    TAILQ_ENTRY(message) entries;
};


TAILQ_HEAD(message_queue, message);
struct message_queue msgqueue;


volatile int receiver_flag = 0;
volatile int processor_flag = 0;
volatile int connector_flag = 0;


int listener_id;
int client_id;
pthread_t receiver_thread;
pthread_t processor_thread;
pthread_t connector_thread;
pthread_mutex_t s_mutex;


void error_exit(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}


void finisher() {
    receiver_flag = 1;
    processor_flag = 1;
    connector_flag = 1;
    pthread_join(receiver_thread, NULL);
    pthread_join(processor_thread, NULL);
    pthread_join(connector_thread, NULL);
    if (shutdown(client_id, SHUT_RDWR) == -1) {
        perror("Ошибка закрытия соединения с рабочим сокетом");
    }
    pthread_mutex_destroy(&s_mutex);
    unlink("server_socket.soc");
    close(client_id);
    close(listener_id);
    printf("Сервер закончил работу.\n");
}


void sig_handler(int signo) {
    printf("Сервер получил сигнал с кодом %d\n", signo);
    sleep(1);
    if (signo == SIGINT || signo == SIGTERM || signo == SIGPIPE) {
        finisher();
        exit(EXIT_SUCCESS);
    }
}


int generate_message(char *buf) {
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) == -1) {
        return -1;
    }
    memset(buf, 0, 256);
    int size = snprintf(buf, 256, "Имя хоста: %s", hostname);
    return size;
}


void *receiver(void *args) {
    printf("Поток приема запросов начал работу.\n");
    char buf[256];
    while (receiver_flag == 0) {
        memset(buf, 0, sizeof(buf));
        int rec_count = recv(client_id, buf, sizeof(buf), 0);
        if (rec_count == -1) {
            perror("Ошибка приема данных из рабочего сокета");
            sleep(1);
        } else if (rec_count == 0) {
            sleep(1);
        } else {
            struct message *m = malloc(sizeof(struct message));
            if (m == NULL) {
                perror("Ошибка выделения памяти для сообщения");
                continue;
            }
            strncpy(m->msg, buf, sizeof(m->msg) - 1);
            m->msg[sizeof(m->msg) - 1] = '\0';
            pthread_mutex_lock(&s_mutex);
            TAILQ_INSERT_TAIL(&msgqueue, m, entries);
            pthread_mutex_unlock(&s_mutex);
        }
    }
    printf("Поток приема запросов закончил работу.\n");
    pthread_exit(NULL);
}


void *processor(void *args) {
    printf("Поток обработки запросов и передачи ответов начал работу.\n");
    char buf[256];
    while (processor_flag == 0) {
        pthread_mutex_lock(&s_mutex);
        if (!TAILQ_EMPTY(&msgqueue)) {
            struct message *m = TAILQ_FIRST(&msgqueue);
            TAILQ_REMOVE(&msgqueue, m, entries);
            pthread_mutex_unlock(&s_mutex);
            free(m);
            memset(buf, 0, sizeof(buf));
            int msg_size = generate_message(buf);
            if (msg_size == -1) {
                perror("Ошибка генерации сообщения");
            } else {
                if (send(client_id, buf, sizeof(buf), 0) == -1) {
                    perror("Ошибка передачи данных в рабочий сокет");
                } else {
                    printf("Сервер послал клиенту сообщение: %s\n", buf);
                }
            }
        } else {
            pthread_mutex_unlock(&s_mutex);
            sleep(1);
        }
    }
    printf("Поток обработки запросов и передачи ответов закончил работу.\n");
    pthread_exit(NULL);
}


void *connector(void *args) {
    printf("Поток ожидания соединений начал работу.\n");
    while (connector_flag == 0) {
        struct sockaddr_un addr;
        memset(&addr, 0, sizeof(addr));
        int addr_len = sizeof(addr);
        client_id = accept(listener_id, (struct sockaddr*)&addr, (socklen_t*)&addr_len);
        if (client_id == -1) {
            sleep(1);
        } else {
            printf("Поток ожидания соединений получил соединение от клиента.\n");
            printf("Адрес сокета клиента: %s\n", addr.sun_path);
            if (pthread_create(&receiver_thread, NULL, receiver, NULL) != 0) {
                perror("Ошибка создания потока receiver");
            }
            if (pthread_create(&processor_thread, NULL, processor, NULL) != 0) {
                perror("Ошибка создания потока processor");
            }
            printf("Поток ожидания соединений закончил работу.\n");
            pthread_exit(NULL);
        }
    }
    printf("Поток ожидания соединений закончил работу.\n");
    pthread_exit(NULL);
}


int main() {
    TAILQ_INIT(&msgqueue);
    signal(SIGPIPE, sig_handler);
    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);
    printf("Сервер начал работу.\n");
    listener_id = socket(AF_UNIX, SOCK_STREAM, 0);
    if (listener_id == -1) {
        error_exit("Ошибка создания слушающего сокета");
    }
    if (fcntl(listener_id, F_SETFL, O_NONBLOCK) == -1) {
        error_exit("Ошибка установки неблокирующего режима");
    }
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    snprintf(addr.sun_path, sizeof(addr.sun_path), "server_socket.soc");
    if (bind(listener_id, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        error_exit("Ошибка привязки слушающего сокета к адресу");
    }
    int optval = 1;
    if (setsockopt(listener_id, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) {
        error_exit("Ошибка задания свойств слушающему сокету");
    }
    if (listen(listener_id, 1) == -1) {
        error_exit("Ошибка перевода слушающего сокета в состояние прослушивания");
    }
    if (pthread_mutex_init(&s_mutex, NULL) != 0) {
        error_exit("Ошибка инициализации мьютекса");
    }
    if (pthread_create(&connector_thread, NULL, connector, NULL) != 0) {
        error_exit("Ошибка создания потока connector");
    }
    printf("Сервер ждет нажатия клавиши...\n");
    getchar();
    printf("Сервер: клавиша нажата.\n");
    finisher();
    return 0;
}
