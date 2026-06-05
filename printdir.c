#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>

#define MAX_PATH 1024

// ============================================
// 判断文件名是否是隐藏文件（以 '.' 开头）
// ============================================
static int is_hidden(const char *name) {
    return name[0] == '.';
}

// ============================================
// 处理单个目录，列出其内容
// 参数：dir_path - 目录路径
// 返回：0 成功，-1 失败
// ============================================
static int process_directory(const char *dir_path) {
    DIR *dir;
    struct dirent *entry;
    struct stat file_stat;
    char full_path[MAX_PATH];
    long total_size = 0;
    int file_count = 0;

    // 打开目录
    dir = opendir(dir_path);
    if (dir == NULL) {
        fprintf(stderr, "opendir: %s: %s\n", dir_path, strerror(errno));
        return -1;
    }

    // 打印目录名
    printf("%s:\n", dir_path);

    // 遍历目录条目
    while ((entry = readdir(dir)) != NULL) {
        // 忽略隐藏文件（以 '.' 开头）
        if (is_hidden(entry->d_name)) {
            continue;
        }

        // 构建完整路径
        if (strlen(dir_path) + strlen(entry->d_name) + 2 > MAX_PATH) {
            fprintf(stderr, "Path too long: %s/%s\n", dir_path, entry->d_name);
            continue;
        }
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);

        // 获取文件信息（使用 lstat 不跟随符号链接）
        if (lstat(full_path, &file_stat) == -1) {
            fprintf(stderr, "lstat: %s: %s\n", full_path, strerror(errno));
            continue;
        }

        // 只统计普通文件
        if (S_ISREG(file_stat.st_mode)) {
            file_count++;
            total_size += file_stat.st_size;
        }

        // 输出：大小\t文件名
        printf("%ld\t%s\n", file_stat.st_size, entry->d_name);
    }

    // 关闭目录
    closedir(dir);

    // 输出统计信息
    printf("%d Files: %ld Bytes\n\n", file_count, total_size);

    return 0;
}

// ============================================
// 主函数
// ============================================
int main(int argc, char *argv[]) {
    // 如果没有参数，处理当前目录
    if (argc == 1) {
        process_directory(".");
        return EXIT_SUCCESS;
    }

    // 遍历所有参数，每个参数作为目录路径
    for (int i = 1; i < argc; i++) {
        process_directory(argv[i]);
    }

    return EXIT_SUCCESS;
}
