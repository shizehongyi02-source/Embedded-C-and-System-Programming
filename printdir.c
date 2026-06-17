#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>

#define MAX_PATH 1024

static int is_hidden(const char *name) {
    return name[0] == '.';
}

static int process_directory(const char *dir_path) {
    DIR *dir;
    struct dirent *entry;
    struct stat file_stat;
    char full_path[MAX_PATH];
    long total_size = 0;
    int file_count = 0;

    dir = opendir(dir_path);
    if (dir == NULL) {
        fprintf(stderr, "opendir: %s: %s\n", dir_path, strerror(errno));
        return -1;
    }

    printf("%s:\n", dir_path);

    while ((entry = readdir(dir)) != NULL) {
        if (is_hidden(entry->d_name)) {
            continue;
        }

        if (strlen(dir_path) + strlen(entry->d_name) + 2 > MAX_PATH) {
            fprintf(stderr, "Path too long: %s/%s\n", dir_path, entry->d_name);
            continue;
        }
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);

        if (lstat(full_path, &file_stat) == -1) {
            fprintf(stderr, "lstat: %s: %s\n", full_path, strerror(errno));
            continue;
        }

        if (S_ISREG(file_stat.st_mode)) {
            file_count++;
            total_size += file_stat.st_size;
        }

        printf("%ld\t%s\n", file_stat.st_size, entry->d_name);
    }

    closedir(dir);

    printf("%d Files: %ld Bytes\n\n", file_count, total_size);

    return 0;
}

int main(int argc, char *argv[]) {
    if (argc == 1) {
        process_directory(".");
        return EXIT_SUCCESS;
    }

    for (int i = 1; i < argc; i++) {
        process_directory(argv[i]);
    }

    return EXIT_SUCCESS;
}
