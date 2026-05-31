#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "commands.h"
#include "filesystem.h"
#include "shell.h"

#define MAX_INPUT 1024
#define MAX_ARGS 64
#define HISTORY_MAX 100

static char command_history[HISTORY_MAX][MAX_INPUT];
static int history_count = 0;

static void add_history(const char *input)
{
    if (input == NULL || strlen(input) == 0) {
        return;
    }

    if (history_count < HISTORY_MAX) {
        strcpy(command_history[history_count], input);
        history_count++;
    } else {
        for (int i = 1; i < HISTORY_MAX; i++) {
            strcpy(command_history[i - 1], command_history[i]);
        }
        strcpy(command_history[HISTORY_MAX - 1], input);
    }
}

static void print_history_all(void)
{
    for (int i = 0; i < history_count; i++) {
        printf("%d  %s\n", i + 1, command_history[i]);
    }
}

static void print_history_recent(int n)
{
    if (n <= 0) {
        printf("history: invalid number\n");
        return;
    }

    int start = history_count - n;
    if (start < 0) {
        start = 0;
    }

    for (int i = start; i < history_count; i++) {
        printf("%d  %s\n", i + 1, command_history[i]);
    }
}

static void clear_history(void)
{
    history_count = 0;
    printf("history cleared\n");
}

static void run_history_command(int argc, char *argv[])
{
    if (argc == 1) {
        print_history_all();
    } else if (argc == 2 && strcmp(argv[1], "-c") == 0) {
        clear_history();
    } else if (argc == 2) {
        int n = atoi(argv[1]);
        print_history_recent(n);
    } else {
        printf("usage: history [n|-c]\n");
    }
}

static int parse_input(char *input, char *argv[]) {
    int argc = 0;
    char *token = strtok(input, " \t\n");

    while (token != NULL && argc < MAX_ARGS - 1) {
        argv[argc++] = token;
        token = strtok(NULL, " \t\n");
    }

    argv[argc] = NULL;
    return argc;
}

static void print_current_path(Node *node) {
    if (node == NULL || node == root) {
        return;
    }

    print_current_path(node->parent);
    printf("/%s", node->name);
}

static void print_prompt(void) {
    init_file_system_if_needed();

    printf("miniOS:");

    if (current_dir == root) {
        printf("/");
    } else {
        print_current_path(current_dir);
    }

    printf(" $ ");
}

static void run_command(int argc, char *argv[]) {
    if (argc == 0) {
        return;
    }

    if (strcmp(argv[0], "pwd") == 0) {
        cmd_pwd();
    } else if (strcmp(argv[0], "ls") == 0) {
        cmd_ls(argc, argv);
    } else if (strcmp(argv[0], "cd") == 0) {
        cmd_cd(argc, argv);
    } else if (strcmp(argv[0], "mkdir") == 0) {
        cmd_mkdir(argc, argv);
    } else if (strcmp(argv[0], "cat") == 0) {
        cmd_cat(argc, argv);
    } else if (strcmp(argv[0], "chown") == 0) {
        if (argc < 3) {
            printf("chown: missing operand\n");
            return;
        }
        cmd_chown(argv[1], argv[2]);
    } else if (strcmp(argv[0], "grep") == 0) {
        if (argc == 4 && strcmp(argv[1], "-n") == 0) {
            cmd_grep(argv[2], argv[3], 1);
        } else if (argc >= 3) {
            cmd_grep(argv[1], argv[2], 0);
        } else {
            printf("grep: missing operand\n");
        }
    } else if (strcmp(argv[0], "mv") == 0) {
        if (argc < 3) {
            printf("mv: missing operand\n");
            return;
        }
        cmd_mv(argv[1], argv[2]);
    } else if (strcmp(argv[0], "rm") == 0) {
        if (argc == 3 && strcmp(argv[1], "-rf") == 0) {
            cmd_rm(argv[2], 1, 1);
        } else if (argc >= 2) {
            cmd_rm(argv[1], 0, 0);
        } else {
            printf("rm: missing operand\n");
        }
    } else if (strcmp(argv[0], "tree") == 0) {
        cmd_tree(argc, argv);
    } else if (strcmp(argv[0], "find") == 0) {
        cmd_find(argc, argv);
    } else if (strcmp(argv[0], "history") == 0) {
        run_history_command(argc, argv);
    } else if (strcmp(argv[0], "help") == 0) {
        printf("commands: pwd, ls, cd, mkdir, cat, chown, grep, mv, rm, tree [-d], find [-name|-type], history [n|-c], help, exit\n");
    } else {
        printf("%s: command not found\n", argv[0]);
    }
}

void shell_loop(void) {
    char input[MAX_INPUT];
    char *argv[MAX_ARGS];

    struct stat st = {0};

    if (stat("data", &st) == -1) {
    #ifdef _WIN32
        mkdir("data");
    #else
        mkdir("data", 0755);
    #endif
    }

    load_file_system(FS_DATA_PATH);

    printf("Mini OS started. Type 'help' for commands.\n");

    while (1) {
        print_prompt();
        fflush(stdout);

        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("\n");
            break;
        }

        input[strcspn(input, "\n")] = '\0';

        if (strlen(input) == 0) {
            continue;
        }
        add_history(input);

        int argc = parse_input(input, argv);

        if (argc > 0 && strcmp(argv[0], "exit") == 0) {
            break;
        }

        run_command(argc, argv);
    }

    save_file_system(FS_DATA_PATH);
}
