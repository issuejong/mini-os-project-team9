#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "commands.h"

#define MAX_NAME 64
#define MAX_CONTENT 4096
#define MAX_INPUT 1024

typedef enum {
    NODE_DIR,
    NODE_FILE
} NodeType;

typedef struct Node {
    char name[MAX_NAME];
    NodeType type;
    char content[MAX_CONTENT];

    struct Node *parent;
    struct Node *child;
    struct Node *sibling;
} Node;

static Node *root = NULL;
static Node *current_dir = NULL;

/* =========================
   임시 파일 시스템 함수
   파일 시스템 담당자 코드 나오면 이 부분은 나중에 연결/수정
   ========================= */

static Node *create_node(const char *name, NodeType type) {
    Node *node = (Node *)malloc(sizeof(Node));

    if (node == NULL) {
        printf("memory allocation failed\n");
        exit(1);
    }

    strncpy(node->name, name, MAX_NAME - 1);
    node->name[MAX_NAME - 1] = '\0';

    node->type = type;
    node->content[0] = '\0';

    node->parent = NULL;
    node->child = NULL;
    node->sibling = NULL;

    return node;
}

static void init_file_system_if_needed(void) {
    if (root != NULL) {
        return;
    }

    root = create_node("/", NODE_DIR);
    root->parent = root;
    current_dir = root;
}

static void add_child(Node *parent, Node *child) {
    if (parent == NULL || child == NULL) {
        return;
    }

    child->parent = parent;

    if (parent->child == NULL) {
        parent->child = child;
        return;
    }

    Node *temp = parent->child;

    while (temp->sibling != NULL) {
        temp = temp->sibling;
    }

    temp->sibling = child;
}

static Node *find_child(Node *dir, const char *name) {
    if (dir == NULL || name == NULL) {
        return NULL;
    }

    Node *temp = dir->child;

    while (temp != NULL) {
        if (strcmp(temp->name, name) == 0) {
            return temp;
        }

        temp = temp->sibling;
    }

    return NULL;
}

/* =========================
   pwd
   ========================= */

static void print_path(Node *node) {
    if (node == NULL || node == root) {
        return;
    }

    print_path(node->parent);
    printf("/%s", node->name);
}

void cmd_pwd(void) {
    init_file_system_if_needed();

    if (current_dir == root) {
        printf("/\n");
        return;
    }

    print_path(current_dir);
    printf("\n");
}

/* =========================
   cd
   ========================= */

void cmd_cd(int argc, char *argv[]) {
    init_file_system_if_needed();

    if (argc < 2) {
        current_dir = root;
        return;
    }

    char *path = argv[1];

    if (strcmp(path, ".") == 0) {
        return;
    }

    if (strcmp(path, "..") == 0) {
        if (current_dir != root) {
            current_dir = current_dir->parent;
        }
        return;
    }

    Node *target = find_child(current_dir, path);

    if (target == NULL) {
        printf("cd: no such directory: %s\n", path);
        return;
    }

    if (target->type != NODE_DIR) {
        printf("cd: not a directory: %s\n", path);
        return;
    }

    current_dir = target;
}

/* =========================
   ls
   ========================= */

static int is_hidden(const char *name) {
    return name != NULL && name[0] == '.';
}

void cmd_ls(int argc, char *argv[]) {
    init_file_system_if_needed();

    int show_all = 0;
    int long_format = 0;

    if (argc >= 2) {
        if (strcmp(argv[1], "-a") == 0) {
            show_all = 1;
        } else if (strcmp(argv[1], "-l") == 0) {
            long_format = 1;
        } else if (strcmp(argv[1], "-al") == 0 || strcmp(argv[1], "-la") == 0) {
            show_all = 1;
            long_format = 1;
        } else {
            printf("ls: invalid option: %s\n", argv[1]);
            return;
        }
    }

    if (show_all) {
        if (long_format) {
            printf("d .\n");
            printf("d ..\n");
        } else {
            printf(".  ..  ");
        }
    }

    Node *temp = current_dir->child;

    while (temp != NULL) {
        if (!show_all && is_hidden(temp->name)) {
            temp = temp->sibling;
            continue;
        }

        if (long_format) {
            printf("%c %s\n", temp->type == NODE_DIR ? 'd' : '-', temp->name);
        } else {
            printf("%s  ", temp->name);
        }

        temp = temp->sibling;
    }

    if (!long_format) {
        printf("\n");
    }
}

/* =========================
   mkdir
   ========================= */

static void mkdir_single(const char *dirname) {
    if (dirname == NULL || strlen(dirname) == 0) {
        printf("mkdir: missing operand\n");
        return;
    }

    if (find_child(current_dir, dirname) != NULL) {
        printf("mkdir: cannot create directory '%s': File exists\n", dirname);
        return;
    }

    Node *new_dir = create_node(dirname, NODE_DIR);
    add_child(current_dir, new_dir);
}

static void mkdir_p(const char *path) {
    if (path == NULL || strlen(path) == 0) {
        printf("mkdir: missing operand\n");
        return;
    }

    char temp[MAX_INPUT];
    strncpy(temp, path, MAX_INPUT - 1);
    temp[MAX_INPUT - 1] = '\0';

    Node *cursor = current_dir;
    char *token = strtok(temp, "/");

    while (token != NULL) {
        Node *next = find_child(cursor, token);

        if (next == NULL) {
            next = create_node(token, NODE_DIR);
            add_child(cursor, next);
        } else if (next->type != NODE_DIR) {
            printf("mkdir: cannot create directory '%s': Not a directory\n", token);
            return;
        }

        cursor = next;
        token = strtok(NULL, "/");
    }
}

void cmd_mkdir(int argc, char *argv[]) {
    init_file_system_if_needed();

    if (argc < 2) {
        printf("mkdir: missing operand\n");
        return;
    }

    if (strcmp(argv[1], "-p") == 0) {
        if (argc < 3) {
            printf("mkdir: missing operand after '-p'\n");
            return;
        }

        for (int i = 2; i < argc; i++) {
            mkdir_p(argv[i]);
        }
    } else {
        for (int i = 1; i < argc; i++) {
            mkdir_single(argv[i]);
        }
    }
}

/* =========================
   cat
   ========================= */

static void print_with_line_number(const char *content) {
    int line = 1;

    if (content == NULL || content[0] == '\0') {
        return;
    }

    printf("%d  ", line);

    for (int i = 0; content[i] != '\0'; i++) {
        putchar(content[i]);

        if (content[i] == '\n' && content[i + 1] != '\0') {
            line++;
            printf("%d  ", line);
        }
    }

    printf("\n");
}

static void cat_read_file(const char *filename, int line_number) {
    Node *file = find_child(current_dir, filename);

    if (file == NULL) {
        printf("cat: %s: No such file\n", filename);
        return;
    }

    if (file->type != NODE_FILE) {
        printf("cat: %s: Is a directory\n", filename);
        return;
    }

    if (line_number) {
        print_with_line_number(file->content);
    } else {
        printf("%s", file->content);
    }
}

static void cat_write_file(const char *filename) {
    Node *file = find_child(current_dir, filename);

    if (file == NULL) {
        file = create_node(filename, NODE_FILE);
        add_child(current_dir, file);
    } else if (file->type != NODE_FILE) {
        printf("cat: %s: Is a directory\n", filename);
        return;
    }

    file->content[0] = '\0';

    char line[MAX_INPUT];

    printf("Enter text. Type EOF to finish.\n");

    while (1) {
        if (fgets(line, sizeof(line), stdin) == NULL) {
            break;
        }

        line[strcspn(line, "\n")] = '\0';

        if (strcmp(line, "EOF") == 0) {
            break;
        }

        if (strlen(file->content) + strlen(line) + 2 >= MAX_CONTENT) {
            printf("cat: file content limit exceeded\n");
            break;
        }

        strcat(file->content, line);
        strcat(file->content, "\n");
    }
}

void cmd_cat(int argc, char *argv[]) {
    init_file_system_if_needed();

    if (argc < 2) {
        printf("cat: missing operand\n");
        return;
    }

    if (strcmp(argv[1], ">") == 0) {
        if (argc < 3) {
            printf("cat: missing file name after '>'\n");
            return;
        }

        cat_write_file(argv[2]);
        return;
    }

    if (strcmp(argv[1], "-n") == 0) {
        if (argc < 3) {
            printf("cat: missing file name after '-n'\n");
            return;
        }

        cat_read_file(argv[2], 1);
        return;
    }

    cat_read_file(argv[1], 0);
}