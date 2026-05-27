#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "commands.h"

#define MAX_NAME 64
#define MAX_CONTENT 4096
#define MAX_INPUT 1024
  
/* Juheon */
typedef enum {
    NODE_DIR,
    NODE_FILE
} NodeType;

typedef struct Node {
    char name[MAX_NAME];
    NodeType type;
    char content[MAX_CONTENT];
    char owner[MAX_NAME];

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
    strcpy(node->owner, "team9");
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

/* Jihwan */

/* 현재 디렉토리 기준으로 간단한 파일/디렉토리 이름 찾기 */
static Node *find_node_simple(const char *path) {
    if (path == NULL || strlen(path) == 0) {
        return NULL;
    }

    if (strcmp(path, ".") == 0) {
        return current_dir;
    }

    if (strcmp(path, "..") == 0) {
        if (current_dir != root) {
            return current_dir->parent;
        }
        return root;
    }

    return find_child(current_dir, path);
}

/* 부모의 child/sibling 연결에서 target을 분리 */
static int detach_child(Node *parent, Node *target) {
    if (parent == NULL || target == NULL) {
        return 0;
    }

    Node *prev = NULL;
    Node *cur = parent->child;

    while (cur != NULL) {
        if (cur == target) {
            if (prev == NULL) {
                parent->child = cur->sibling;
            } else {
                prev->sibling = cur->sibling;
            }

            cur->sibling = NULL;
            cur->parent = NULL;
            return 1;
        }

        prev = cur;
        cur = cur->sibling;
    }

    return 0;
}

/* 디렉토리 내부까지 재귀적으로 메모리 해제 */
static void free_tree(Node *node) {
    if (node == NULL) {
        return;
    }

    Node *child = node->child;

    while (child != NULL) {
        Node *next = child->sibling;
        free_tree(child);
        child = next;
    }

    free(node);
}

/* chown owner file */
void cmd_chown(const char *owner, const char *path) {
    init_file_system_if_needed();

    if (owner == NULL || path == NULL) {
        printf("chown: missing operand\n");
        return;
    }

    Node *target = find_node_simple(path);

    if (target == NULL) {
        printf("chown: cannot access '%s': No such file or directory\n", path);
        return;
    }

    strncpy(target->owner, owner, MAX_NAME - 1);
    target->owner[MAX_NAME - 1] = '\0';

    printf("owner of '%s' changed to '%s'\n", target->name, target->owner);
}

/* grep keyword file */
/* grep -n keyword file */
void cmd_grep(const char *keyword, const char *path, int showLineNumber) {
    init_file_system_if_needed();

    if (keyword == NULL || path == NULL) {
        printf("grep: missing operand\n");
        return;
    }

    Node *file = find_node_simple(path);

    if (file == NULL) {
        printf("grep: %s: No such file\n", path);
        return;
    }

    if (file->type != NODE_FILE) {
        printf("grep: %s: Is a directory\n", path);
        return;
    }

    char content_copy[MAX_CONTENT];
    strncpy(content_copy, file->content, MAX_CONTENT - 1);
    content_copy[MAX_CONTENT - 1] = '\0';

    char *line = strtok(content_copy, "\n");
    int line_number = 1;
    int found = 0;

    while (line != NULL) {
        if (strstr(line, keyword) != NULL) {
            if (showLineNumber) {
                printf("%d:%s\n", line_number, line);
            } else {
                printf("%s\n", line);
            }

            found = 1;
        }

        line = strtok(NULL, "\n");
        line_number++;
    }

    if (!found) {
        printf("grep: no match found\n");
    }
}

/* mv oldname newname */
/* mv file directory */
void cmd_mv(const char *srcPath, const char *destPath) {
    init_file_system_if_needed();

    if (srcPath == NULL || destPath == NULL) {
        printf("mv: missing operand\n");
        return;
    }

    Node *src = find_node_simple(srcPath);

    if (src == NULL) {
        printf("mv: cannot stat '%s': No such file or directory\n", srcPath);
        return;
    }

    if (src == root) {
        printf("mv: cannot move root directory\n");
        return;
    }

    Node *dest = find_node_simple(destPath);

    /*
        dest가 기존 디렉토리이면:
        src를 dest 안으로 이동
    */
    if (dest != NULL && dest->type == NODE_DIR) {
        if (find_child(dest, src->name) != NULL) {
            printf("mv: cannot move '%s': File exists in destination\n", src->name);
            return;
        }

        detach_child(src->parent, src);
        add_child(dest, src);

        printf("moved '%s' into '%s'\n", srcPath, destPath);
        return;
    }

    /*
        dest가 존재하지 않으면:
        같은 디렉토리 안에서 이름 변경
    */
    if (dest == NULL) {
        if (find_child(current_dir, destPath) != NULL) {
            printf("mv: cannot move '%s': File exists\n", destPath);
            return;
        }

        strncpy(src->name, destPath, MAX_NAME - 1);
        src->name[MAX_NAME - 1] = '\0';

        printf("renamed '%s' to '%s'\n", srcPath, destPath);
        return;
    }

    printf("mv: cannot overwrite '%s'\n", destPath);
}

/* rm file */
/* rm -rf directory */
void cmd_rm(const char *path, int recursive, int force) {
    printf("[rm] path=%s, recursive=%d, force=%d\n", path, recursive, force);
}

/* Sunbin */

/*======================
tree
======================*/

static void print_tree_node_option(Node *node, int depth, int dirOnly)
{
    if (node == NULL) {
        return;
    }

    if (dirOnly && node->type != NODE_DIR) {
        return;
    }

    for (int i = 0; i < depth; i++) {
        printf("  ");
    }

    if (node->type == NODE_DIR) {
        printf("[D] %s\n", node->name);
    } else {
        printf("[F] %s\n", node->name);
    }

    Node *child = node->child;
    while (child != NULL) {
        print_tree_node_option(child, depth + 1, dirOnly);
        child = child->sibling;
    }
}

void cmd_tree(int argc, char *argv[])
{
    int dirOnly = 0;

    if (argc > 2) {
        printf("tree: too many arguments\n");
        printf("usage: tree [-d]\n");
        return;
    }

    if (argc == 2) {
        if (strcmp(argv[1], "-d") == 0) {
            dirOnly = 1;
        } else {
            printf("tree: invalid option '%s'\n", argv[1]);
            printf("usage: tree [-d]\n");
            return;
        }
    }
    print_tree_node_option(root, 0, dirOnly);
}

/*====================
find
====================*/

static void print_find_usage(void)
{
    printf("usage:\n");
    printf("  find <name>\n");
    printf("  find -name <name>\n");
    printf("  find -type d|f\n");
    printf("  find -name <name> -type d|f\n");
}

static int match_find_condition(Node *node, const char *nameFilter, int typeFilter)
{
    if (node == NULL) {
        return 0;
    }

    if (nameFilter != NULL && strcmp(node->name, nameFilter) != 0) {
        return 0;
    }

    if (typeFilter == 1 && node->type != NODE_DIR) {
        return 0;
    }

    if (typeFilter == 2 && node->type != NODE_FILE) {
        return 0;
    }

    return 1;
}

static int find_node_advanced(Node *node, const char *parentPath, const char *nameFilter, int typeFilter)
{
    if (node == NULL) {
        return 0;
    }

    char currentPath[1024];

    if (strcmp(node->name, "/") == 0) {
        snprintf(currentPath, sizeof(currentPath), "/");
    } else if (strcmp(parentPath, "/") == 0) {
        snprintf(currentPath, sizeof(currentPath), "/%s", node->name);
    } else {
        snprintf(currentPath, sizeof(currentPath), "%s/%s", parentPath, node->name);
    }

    int found = 0;

    if (match_find_condition(node, nameFilter, typeFilter)) {
        printf("%s\n", currentPath);
        found = 1;
    }

    Node *child = node->child;
    while (child != NULL) {
        if (find_node_advanced(child, currentPath, nameFilter, typeFilter)) {
            found = 1;
        }
        child = child->sibling;
    }

    return found;
}

void cmd_find(int argc, char *argv[])
{
    const char *nameFilter = NULL;
    int typeFilter = 0;

    if (argc < 2) {
        print_find_usage();
        return;
    }

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-name") == 0) {
            if (i + 1 >= argc) {
                printf("find: option '-name' requires an argument\n");
                print_find_usage();
                return;
            }
            nameFilter = argv[i + 1];
            i++;
        } else if (strcmp(argv[i], "-type") == 0) {
            if (i + 1 >= argc) {
                printf("find: option '-type' requires an argument\n");
                print_find_usage();
                return;
            }

            if (strcmp(argv[i + 1], "d") == 0) {
                typeFilter = 1;
            } else if (strcmp(argv[i + 1], "f") == 0) {
                typeFilter = 2;
            } else {
                printf("find: invalid type '%s'\n", argv[i + 1]);
                printf("find: type must be 'd' or 'f'\n");
                return;
            }
            i++;
        } else if (argv[i][0] == '-') {
            printf("find: invalid option '%s'\n", argv[i]);
            print_find_usage();
            return;
        } else {
            if (nameFilter != NULL) {
                printf("find: too many name arguments\n");
                print_find_usage();
                return;
            }
            nameFilter = argv[i];
        }
    }

    int found = find_node_advanced(root, "/", nameFilter, typeFilter);

    if (!found) {
        printf("find: no matching result\n");
    }
}

