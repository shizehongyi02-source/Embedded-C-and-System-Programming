#include <stdio.h>
#include <stdlib.h>

static size_t str_len(const char *s) {
    size_t len = 0;
    while (s[len] != '\0') {
        len++;
    }
    return len;
}

static char* str_cpy(char *dest, const char *src) {
    char *original_dest = dest;
    while (*src != '\0') {
        *dest = *src;
        dest++;
        src++;
    }
    *dest = '\0';
    return original_dest;
}

static char* str_cat(char *dest, const char *src) {
    char *original_dest = dest;
    while (*dest != '\0') {
        dest++;
    }
    while (*src != '\0') {
        *dest = *src;
        dest++;
        src++;
    }
    *dest = '\0';
    return original_dest;
}

int main(int argc, char *argv[]) {
    if (argc == 1) {
        return EXIT_SUCCESS;
    }

    size_t total_len = 0;

    for (int i = 1; i < argc; i++) {
        total_len += str_len(argv[i]);
    }

    total_len += (argc - 1);
    total_len += 1;

    char *buffer = malloc(total_len);
    if (buffer == NULL) {
        perror("malloc");
        return EXIT_FAILURE;
    }

    str_cpy(buffer, argv[1]);

    for (int i = 2; i < argc; i++) {
        str_cat(buffer, " ");
        str_cat(buffer, argv[i]);
    }

    printf("%s\n", buffer);

    free(buffer);

    printf("ConCar works\n" );

    return EXIT_SUCCESS;
}
