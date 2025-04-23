#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <locale.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <pthread.h>

int server_id;
volatile int sender_flag = 0;
volatile int receiver_flag = 0;
volatile int connector_flag = 0;
pthread_t sender_thread;
pthread_t receiver_thread;
pthread_t connector_thread;

void error_exit(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

void finisher() {
    sender_flag = 1;
    receiver_flag = 1;
    connector_flag = 1;
    pthread_join(sender_thread, NULL);
    pthread_join(receiver_thread, NULL);
    pthread_join(connector_thread, NULL);

    if (shutdown(server_id, SHUT_RDWR) == -1) {
        perror("Ошибка закрытия соединения с сервером");
    }
    unlink("client_socket.soc");
    close(server_id);
    printf("Клиент закончил работу.\n");
}

void sig_handler(int signo) {
    printf("Клиент получил сигнал с кодом %d\n", signo);
    sleep(1);
    if (signo == SIGINT || signo == SIGTERM || signo == SIGPIPE) {
        finisher();
        exit(EXIT_SUCCESS);
    }
}

void *sender(void *args) {
    printf("Поток передачи запросов начал работу.\n");
    char buf[256];
    while (!sender_flag) {
        memset(buf, 0, sizeof(buf));
        int sentcount = send(server_id, buf, sizeof(buf), 0);
        if (sentcount == -1) {
            perror("Ошибка посылки запроса на сервер");
        } else {
            printf("Клиент послал запрос на сервер\n");
        }
        sleep(1);
    }
    printf("Поток передачи запросов закончил работу.\n");
    pthread_exit(NULL);
}

void *receiver(void *args) {
    printf("Поток приема ответов начал работу.\n");
    char buf[256];
    while (!receiver_flag) {
        memset(buf, 0, sizeof(buf));
        int reccount = recv(server_id, buf, sizeof(buf), 0);
        if (reccount == -1) {
            sleep(1);
        } else if (reccount == 0) {
            sleep(1);
        } else {
            printf("Ответ от сервера: %s\n", buf);
        }
    }
    printf("Поток приема ответов закончил работу.\n");
    pthread_exit(NULL);
}

void *connector(void *args) {
    printf("Поток установления соединения начал работу.\n");
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    snprintf(addr.sun_path, sizeof(addr.sun_path), "server_socket.soc");

    while (!connector_flag) {
        if (connect(server_id, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
            perror("Ошибка установления соединения с сервером");
            sleep(1);
        } else {
            printf("Поток установил соединение с сервером.\n");
            if (pthread_create(&sender_thread, NULL, sender, NULL) != 0) {
                perror("Ошибка создания потока sender");
            }
            if (pthread_create(&receiver_thread, NULL, receiver, NULL) != 0) {
                perror("Ошибка создания потока receiver");
            }
            printf("Поток установления соединения закончил работу.\n");
            pthread_exit(NULL);
        }
    }
    printf("Поток установления соединения закончил работу.\n");
    pthread_exit(NULL);
}

int main() {
    setlocale(LC_ALL, "ru_RU.UTF-8");
    signal(SIGPIPE, sig_handler);
    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);

    printf("Клиент начал работу.\n");

    server_id = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_id == -1) {
        error_exit("Ошибка создания сокета для работы с сервером");
    }

    if (fcntl(server_id, F_SETFL, O_NONBLOCK) == -1) {
        error_exit("Ошибка установки неблокирующего режима для сокета");
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    snprintf(addr.sun_path, sizeof(addr.sun_path), "client_socket.soc");

    if (bind(server_id, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        error_exit("Ошибка привязки сокета к адресу");
    }
    printf("Сокет для работы с сервером привязан к адресу.\n");

    int addr_len = sizeof(addr);
    if (getsockname(server_id, (struct sockaddr *)&addr, (socklen_t *)&addr_len) == -1) {
        perror("Ошибка получения адреса сокета");
    } else {
        printf("Адрес сокета: %s\n", addr.sun_path);
    }

    if (pthread_create(&connector_thread, NULL, connector, NULL) != 0) {
        error_exit("Ошибка создания потока connector");
    }

    printf("Программа ждет нажатия на клавишу...\n");
    getchar();
    printf("Клиент: клавиша нажата.\n");
    finisher();
    return 0;
}
