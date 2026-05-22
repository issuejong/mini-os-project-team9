#include <stdio.h>
#include <string.h>
#include "commands.h"
#include "shell.h"

#define MAX_INPUT 1024
#define MAX_ARGS 64

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
    } else if (strcmp(argv[0], "help") == 0) {
        printf("commands: pwd, ls, cd, mkdir, cat, chown, grep, mv, rm, help, exit\n");
    } else {
        printf("%s: command not found\n", argv[0]);
    }
}

void shell_loop(void) {
    char input[MAX_INPUT];
    char *argv[MAX_ARGS];

    printf("Mini OS started. Type 'help' for commands.\n");

    while (1) {
        printf("miniOS:/ $ ");
        fflush(stdout);

        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("\n");
            break;
        }

        int argc = parse_input(input, argv);

        if (argc > 0 && strcmp(argv[0], "exit") == 0) {
            break;
        }

        run_command(argc, argv);
    }
}
