#include <ev.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>

void timer_cb(struct ev_loop *loop, ev_timer *w, int revents) {
    alarm(1);
    printf("Сгенерирован сигнал SIGALARM\n");
}

void signal_cb(struct ev_loop *loop, ev_signal *w, int revents) {
    printf("Получен сигнал SIGALRM\n");
}

void stdin_cb(struct ev_loop *loop, ev_io *w, int revents) {
    char buf[1];
    if (read(STDIN_FILENO, buf, 1) > 0) {
        ev_break(loop, EVBREAK_ALL);
    }
}

int main() {
    struct ev_loop *loop = ev_default_loop(0);

    ev_timer timer;
    ev_timer_init(&timer, timer_cb, 5.0, 5.0);
    ev_timer_start(loop, &timer);

    ev_signal signal_watcher;
    ev_signal_init(&signal_watcher, signal_cb, SIGALRM);
    ev_signal_start(loop, &signal_watcher);

    ev_io stdin_watcher;
    ev_io_init(&stdin_watcher, stdin_cb, STDIN_FILENO, EV_READ);
    ev_io_start(loop, &stdin_watcher);

    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);

    ev_run(loop, 0);

    return 0;
}