#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>

#define MAX_CMD_LEN 1024
#define MAX_ARGS 64

// ============================================
// 全局变量（volatile：在信号处理函数中修改）
// ============================================
static volatile sig_atomic_t g_sigchld_received = 0;

// ============================================
// 信号处理函数
// ============================================
static void sigchld_handler(int sig) {
    (void)sig;
    g_sigchld_received = 1;
}

// ============================================
// 打印进程退出状态
// ============================================
static void print_status(pid_t pid, int status) {
    if (WIFEXITED(status)) {
        fprintf(stderr, "Exit status [%d] = %d\n", pid, WEXITSTATUS(status));
    } else if (WIFSIGNALED(status)) {
        fprintf(stderr, "Signal [%d] = %d\n", pid, WTERMSIG(status));
    }
}

// ============================================
// 收集所有已终止的子进程（非阻塞）
// ============================================
static void collect_zombies(void) {
    pid_t pid;
    int status;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        print_status(pid, status);
    }
}

// ============================================
// 等待前台进程终止
// ============================================
static void wait_for_foreground(pid_t fg_pid) {
    sigset_t mask, oldmask;
    int status;

    // 阻塞 SIGCHLD
    sigemptyset(&mask);
    sigaddset(&mask, SIGCHLD);
    sigprocmask(SIG_BLOCK, &mask, &oldmask);

    // 重置标志
    g_sigchld_received = 0;

    // 等待前台进程终止
    while (1) {
        pid_t pid = waitpid(fg_pid, &status, WNOHANG);
        if (pid == fg_pid) {
            // 前台进程已终止
            print_status(fg_pid, status);
            break;
        } else if (pid == -1 && errno != ECHILD) {
            perror("waitpid");
            break;
        }

        // 如果有 SIGCHLD 信号，收集后台进程
        if (g_sigchld_received) {
            g_sigchld_received = 0;
            collect_zombies();
        }

        // 等待 SIGCHLD 信号
        sigsuspend(&oldmask);
    }

    // 恢复信号掩码
    sigprocmask(SIG_SETMASK, &oldmask, NULL);
}

// ============================================
// 解析命令行，分割成命令和参数
// ============================================
static int parse_command(char *cmdline, char *args[], int *background) {
    char *token;
    int i = 0;

    // 去除末尾换行符
    cmdline[strcspn(cmdline, "\n")] = '\0';

    // 检查是否是后台进程（以 '&' 结尾）
    *background = 0;
    size_t len = strlen(cmdline);
    if (len > 0 && cmdline[len - 1] == '&') {
        *background = 1;
        cmdline[len - 1] = '\0';
        // 去除 & 后面的空格
        while (len > 1 && cmdline[len - 2] == ' ') {
            cmdline[len - 2] = '\0';
            len--;
        }
    }

    // 使用 strtok 分割命令和参数
    token = strtok(cmdline, " \t");
    while (token != NULL && i < MAX_ARGS - 1) {
        args[i++] = token;
        token = strtok(NULL, " \t");
    }
    args[i] = NULL;

    return i;
}

// ============================================
// 执行命令
// ============================================
static void execute_command(char *args[], int background) {
    pid_t pid = fork();

    if (pid == -1) {
        perror("fork");
        return;
    }

    if (pid == 0) {
        // 子进程
        if (background) {
            // 后台进程忽略 SIGINT
            struct sigaction sa;
            sa.sa_handler = SIG_IGN;
            sigemptyset(&sa.sa_mask);
            sa.sa_flags = 0;
            sigaction(SIGINT, &sa, NULL);
        } else {
            // 前台进程恢复默认 SIGINT 处理
            struct sigaction sa;
            sa.sa_handler = SIG_DFL;
            sigemptyset(&sa.sa_mask);
            sa.sa_flags = 0;
            sigaction(SIGINT, &sa, NULL);
        }

        // 执行命令
        execvp(args[0], args);

        // 如果执行失败
        fprintf(stderr, "%s: No such file or directory\n", args[0]);
        exit(EXIT_FAILURE);
    }

    // 父进程
    if (background) {
        // 后台进程：只打印 PID，不等待
        fprintf(stderr, "Started [%d]\n", pid);
    } else {
        // 前台进程：等待终止
        wait_for_foreground(pid);
    }
}

// ============================================
// 主函数
// ============================================
int main(void) {
    char cmdline[MAX_CMD_LEN];
    char *args[MAX_ARGS];
    int background;
    struct sigaction sa;
    sigset_t mask;

    // ========== 配置信号处理 ==========
    // SIGINT：mish 忽略 SIGINT
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

    // SIGCHLD：设置自定义处理函数
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGCHLD, &sa, NULL);

    // 阻塞 SIGCHLD（在主程序中手动处理）
    sigemptyset(&mask);
    sigaddset(&mask, SIGCHLD);
    sigprocmask(SIG_BLOCK, &mask, NULL);

    // ========== 主循环 ==========
    while (1) {
        // 收集已终止的后台进程
        collect_zombies();

        // 打印提示符
        printf("mish> ");
        fflush(stdout);

        // 读取命令
        if (fgets(cmdline, sizeof(cmdline), stdin) == NULL) {
            // EOF (Ctrl+D)
            printf("\n");
            break;
        }

        // 跳过空行
        if (cmdline[0] == '\n' || cmdline[0] == '\0') {
            continue;
        }

        // 解析命令
        if (parse_command(cmdline, args, &background) == 0) {
            continue;
        }

        // 执行命令
        execute_command(args, background);
    }

    return EXIT_SUCCESS;
}
