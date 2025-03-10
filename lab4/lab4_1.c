#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>

int main(int argc, char* argv[], char *envp[]) {
    printf("ID процесса первой программы: %d\r\n", getpid());
    printf("ID родительского процесса первой программы: %d\r\n", getppid());
    printf("Аргументы, переданные в первую программу:\r\n");

    for (int i = 0; i < argc; ++i) {
        printf("Аргумент %d: %s\r\n", i + 1, argv[i]);
        sleep(1);
    }

    if (envp[0] != NULL) {
        printf("Получены переменные окружения:\n");
        for (int i = 0; envp[i] != NULL; i++) {
            printf("%s ", envp[i]);
        }
    }

    exit(13);
}