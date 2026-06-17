#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>

#define MAX_CMD_LEN 1024
#define MAX_ARGS 64

static volatile sig_atomic_t g_sigchld_received = 0;

static void sigchld_handler(int sig) {
    (void)sig;
    g_sigchld_received = 1;
}

static void print_status(pid_t pid, int status) {
    if (WIFEXITED(status)) {
        fprintf(stderr, "Exit status [%d] = %d\n", pid, WEXITSTATUS(status));
    } else if (WIFSIGNALED(status)) {
        fprintf(stderr, "Signal [%d] = %d\n", pid, WTERMSIG(status));
    }
}

static void collect_zombies(void) {
    pid_t pid;
    int status;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        print_status(pid, status);
    }
}

static void wait_for_foreground(pid_t fg_pid) {
    sigset_t mask, oldmask;
    int status;

    sigemptyset(&mask);
    sigaddset(&mask, SIGCHLD);
    sigprocmask(SIG_BLOCK, &mask, &oldmask);

    g_sigchld_received = 0;

    while (1) {
        pid_t pid = waitpid(fg_pid, &status, WNOHANG);
        if (pid == fg_pid) {
            print_status(fg_pid, status);
            break;
        } else if (pid == -1 && errno != ECHILD) {
            perror("waitpid");
            break;
        }

        if (g_sigchld_received) {
            g_sigchld_received = 0;
            collect_zombies();
        }

        sigsuspend(&oldmask);
    }

    sigprocmask(SIG_SETMASK, &oldmask, NULL);
}

static int parse_command(char *cmdline, char *args[], int *background) {
    char *token;
    int i = 0;

    cmdline[strcspn(cmdline, "\n")] = '\0';

    *background = 0;
    size_t len = strlen(cmdline);
    if (len > 0 && cmdline[len - 1] == '&') {
        *background = 1;
        cmdline[len - 1] = '\0';
        while (len > 1 && cmdline[len - 2] == ' ') {
            cmdline[len - 2] = '\0';
            len--;
        }
    }

    token = strtok(cmdline, " \t");
    while (token != NULL && i < MAX_ARGS - 1) {
        args[i++] = token;
        token = strtok(NULL, " \t");
    }
    args[i] = NULL;

    return i;
}

static void execute_command(char *args[], int background) {
    pid_t pid = fork();

    if (pid == -1) {
        perror("fork");
        return;
    }

    if (pid == 0) {
        if (background) {
            struct sigaction sa;
            sa.sa_handler = SIG_IGN;
            sigemptyset(&sa.sa_mask);
            sa.sa_flags = 0;
            sigaction(SIGINT, &sa, NULL);
        } else {
            struct sigaction sa;
            sa.sa_handler = SIG_DFL;
            sigemptyset(&sa.sa_mask);
            sa.sa_flags = 0;
            sigaction(SIGINT, &sa, NULL);
        }

        execvp(args[0], args);

        fprintf(stderr, "%s: No such file or directory\n", args[0]);
        exit(EXIT_FAILURE);
    }

    if (background) {
        fprintf(stderr, "Started [%d]\n", pid);
    } else {
        wait_for_foreground(pid);
    }
}

int main(void) {
    char cmdline[MAX_CMD_LEN];
    char *args[MAX_ARGS];
    int background;
    struct sigaction sa;
    sigset_t mask;

    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGCHLD, &sa, NULL);

    sigemptyset(&mask);
    sigaddset(&mask, SIGCHLD);
    sigprocmask(SIG_BLOCK, &mask, NULL);

    while (1) {
        collect_zombies();

        printf("mish> ");
        fflush(stdout);

        if (fgets(cmdline, sizeof(cmdline), stdin) == NULL) {
            printf("\n");
            break;
        }

        if (cmdline[0] == '\n' || cmdline[0] == '\0') {
            continue;
        }

        if (parse_command(cmdline, args, &background) == 0) {
            continue;
        }

        execute_command(args, background);
    }

    return EXIT_SUCCESS;
}
