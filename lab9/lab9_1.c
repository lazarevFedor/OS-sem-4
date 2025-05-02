#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int main() {
    const int fd = open("/etc/shadow", O_RDONLY);
    if (fd == -1) {
        perror("open");
        return 1;
    }
    printf("File opened successfully\n");
    close(fd);
    return 0;
}