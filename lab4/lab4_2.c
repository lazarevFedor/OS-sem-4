#include <unistd.h>
#include <stdio.h>
#include <locale.h>
#include <sys/wait.h>

int main() {
    printf("ID процесса второй программы: %d\r\n", getpid());
    printf("ID родительского процесса второй программы: %d\r\n", getppid());
    pid_t pid;
    pid = fork();

    char *const args[6] = {"16", "11", "2004", "26", "02", NULL};
    char *envp[] = {"MY_VAR=Письмо_от_родителя!", NULL};

    if (pid == -1) {
        perror("Ошибка создания дочернего процесса");
    }
    else if (pid == 0) {
        int rv = execve("lab4_1", args, envp);
        if (rv == -1) {
            perror("Ошибка исполнения дочерней программы");
        }
    }
    else if (pid > 0) {
        printf("ID дочернего процесса второй программы: %d\r\n", pid);
        int status, w;
        while ((w = waitpid(pid, &status, WNOHANG)) == 0) {
            printf("Ждем дочерний процесс\r\n");
            sleep(1);
        }
        if (w == -1) {
            perror("Ошибка waitpid");
        }
        else {
            printf("Дочерний процесс второй программы завершился с кодом %d\r\n", WEXITSTATUS(status));
        }
    }
    return 0;
}