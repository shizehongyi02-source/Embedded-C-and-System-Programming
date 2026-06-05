#include <stdio.h>
#include <stdlib.h>

// ============================================
// 自己实现的 strlen 功能
// 计算字符串长度（不包含结尾的 '\0'）
// ============================================
static size_t str_len(const char *s) {
    size_t len = 0;
    while (s[len] != '\0') {
        len++;
    }
    return len;
}

// ============================================
// 自己实现的 strcpy 功能
// 将 src 复制到 dest（包括结尾的 '\0'）
// ============================================
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

// ============================================
// 自己实现的 strcat 功能
// 将 src 连接到 dest 的末尾
// ============================================
static char* str_cat(char *dest, const char *src) {
    char *original_dest = dest;

    // 先找到 dest 的末尾
    while (*dest != '\0') {
        dest++;
    }

    // 然后复制 src
    while (*src != '\0') {
        *dest = *src;
        dest++;
        src++;
    }
    *dest = '\0';

    return original_dest;
}

// ============================================
// 主函数
// ============================================
int main(int argc, char *argv[]) {
    // 如果没有参数，直接成功退出
    if (argc == 1) {
        return EXIT_SUCCESS;
    }

    // ========== 1. 计算总长度 ==========
    size_t total_len = 0;

    // 计算所有参数的长度之和
    for (int i = 1; i < argc; i++) {
        total_len += str_len(argv[i]);
    }

    // 添加参数之间的空格数量（argc-1 个空格）
    total_len += (argc - 1);

    // 添加结尾的 '\0'
    total_len += 1;

    // ========== 2. 动态分配内存 ==========
    char *buffer = malloc(total_len);
    if (buffer == NULL) {
        perror("malloc");
        return EXIT_FAILURE;
    }

    // ========== 3. 拼接字符串 ==========
    // 先复制第一个参数
    str_cpy(buffer, argv[1]);

    // 然后依次添加空格和后面的参数
    for (int i = 2; i < argc; i++) {
        str_cat(buffer, " ");
        str_cat(buffer, argv[i]);
    }

    // ========== 4. 输出 ==========
    printf("%s\n", buffer);

    // ========== 5. 释放内存 ==========
    free(buffer);

    printf("ConCar works\n" );

    return EXIT_SUCCESS;
}
