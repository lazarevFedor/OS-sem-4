#include <stdio.h>
#include <sys/capability.h>
int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <file>\n", argv[0]);
        return 1;
    }
    cap_t caps = cap_from_text("cap_dac_override+eip");
    if (cap_set_file(argv[1], caps) == -1) {
        perror("Failed to set capabilities");
        return 1;
    }
    cap_free(caps);
    return 0;
}