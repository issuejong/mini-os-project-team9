#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/filesystem.h"

Node* root = NULL;
Node* current_dir = NULL;

Node* create_node(const char* name, NodeType type) {
    Node* node = (Node*)malloc(sizeof(Node));

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

void init_file_system_if_needed(void) {
    if (root != NULL) {
        return;
    }

    root = create_node("/", NODE_DIR);
    root->parent = root;

    current_dir = root;
}

void add_child(Node* parent, Node* child) {
    if (parent == NULL || child == NULL) {
        return;
    }

    child->parent = parent;

    if (parent->child == NULL) {
        parent->child = child;
        return;
    }

    Node* temp = parent->child;

    while (temp->sibling != NULL) {
        temp = temp->sibling;
    }

    temp->sibling = child;
}

Node* find_child(Node* dir, const char* name) {
    if (dir == NULL || name == NULL) {
        return NULL;
    }

    Node* temp = dir->child;

    while (temp != NULL) {
        if (strcmp(temp->name, name) == 0) {
            return temp;
        }

        temp = temp->sibling;
    }

    return NULL;
}

void print_path(Node* node) {
    if (node == NULL || node == root) {
        return;
    }

    print_path(node->parent);

    printf("/%s", node->name);
}

void cmd_pwd_core(void) {
    init_file_system_if_needed();

    if (current_dir == root) {
        printf("/\n");
        return;
    }

    print_path(current_dir);
    printf("\n");
}

int change_directory(const char* path) {
    init_file_system_if_needed();

    if (path == NULL || strlen(path) == 0) {
        current_dir = root;
        return 1;
    }

    if (strcmp(path, "/") == 0) {
        current_dir = root;
        return 1;
    }

    char temp[1024];

    strncpy(temp, path, sizeof(temp) - 1);

    temp[sizeof(temp) - 1] = '\0';

    Node* cursor;

    if (path[0] == '/') {
        cursor = root;
    }
    else {
        cursor = current_dir;
    }

    char* token = strtok(temp, "/");

    while (token != NULL) {

        if (strcmp(token, ".") == 0) {
            token = strtok(NULL, "/");
            continue;
        }

        if (strcmp(token, "..") == 0) {

            if (cursor != root) {
                cursor = cursor->parent;
            }

            token = strtok(NULL, "/");
            continue;
        }

        Node* next = find_child(cursor, token);

        if (next == NULL) {
            printf("cd: no such directory: %s\n", token);
            return 0;
        }

        if (next->type != NODE_DIR) {
            printf("cd: not a directory: %s\n", token);
            return 0;
        }

        cursor = next;

        token = strtok(NULL, "/");
    }

    current_dir = cursor;

    return 1;
}

static void escape_content(const char* src, char* dest, int size) {
    int j = 0;

    for (int i = 0; src[i] != '\0' && j < size - 1; i++) {

        if (src[i] == '\n' && j < size - 2) {
            dest[j++] = '\\';
            dest[j++] = 'n';
        }
        else if (src[i] == '|' && j < size - 2) {
            dest[j++] = '\\';
            dest[j++] = 'p';
        }
        else if (src[i] == '\\' && j < size - 2) {
            dest[j++] = '\\';
            dest[j++] = '\\';
        }
        else {
            dest[j++] = src[i];
        }
    }

    dest[j] = '\0';
}

static void unescape_content(const char* src, char* dest, int size) {
    int j = 0;

    for (int i = 0; src[i] != '\0' && j < size - 1; i++) {

        if (src[i] == '\\' && src[i + 1] == 'n') {
            dest[j++] = '\n';
            i++;
        }
        else if (src[i] == '\\' && src[i + 1] == 'p') {
            dest[j++] = '|';
            i++;
        }
        else if (src[i] == '\\' && src[i + 1] == '\\') {
            dest[j++] = '\\';
            i++;
        }
        else {
            dest[j++] = src[i];
        }
    }

    dest[j] = '\0';
}

void save_node(FILE* fp, Node* node, int depth) {
    if (node == NULL) {
        return;
    }

    char escaped[MAX_CONTENT * 2];

    escape_content(node->content, escaped, sizeof(escaped));

    fprintf(fp,
            "%d|%d|%s|%s|%s\n",
            depth,
            node->type,
            node->name,
            node->owner,
            escaped);

    save_node(fp, node->child, depth + 1);

    save_node(fp, node->sibling, depth);
}

void save_file_system(const char* filename) {
    init_file_system_if_needed();

    FILE* fp = fopen(filename, "w");

    if (fp == NULL) {
        printf("save error\n");
        return;
    }

    save_node(fp, root, 0);

    fclose(fp);

    printf("filesystem saved\n");
}

static void free_tree(Node* node) {
    if (node == NULL) {
        return;
    }

    Node* child = node->child;

    while (child != NULL) {
        Node* next = child->sibling;

        free_tree(child);

        child = next;
    }

    free(node);
}

void load_file_system(const char* filename) {
    FILE* fp = fopen(filename, "r");

    if (fp == NULL) {
        init_file_system_if_needed();
        return;
    }

    if (root != NULL) {
        free_tree(root);

        root = NULL;
        current_dir = NULL;
    }

    Node* stack[128] = { NULL };

    char line[8192];

    while (fgets(line, sizeof(line), fp) != NULL) {

        line[strcspn(line, "\n")] = '\0';

        char* depth_str = strtok(line, "|");
        char* type_str = strtok(NULL, "|");
        char* name = strtok(NULL, "|");
        char* owner = strtok(NULL, "|");
        char* content = strtok(NULL, "");

        if (depth_str == NULL ||
            type_str == NULL ||
            name == NULL ||
            owner == NULL) {
            continue;
        }

        int depth = atoi(depth_str);

        NodeType type = (NodeType)atoi(type_str);

        Node* node = create_node(name, type);

        strncpy(node->owner, owner, MAX_NAME - 1);

        node->owner[MAX_NAME - 1] = '\0';

        if (content != NULL) {
            unescape_content(content,
                             node->content,
                             MAX_CONTENT);
        }

        if (depth == 0) {
            root = node;
            root->parent = root;
        }
        else {
            add_child(stack[depth - 1], node);
        }

        stack[depth] = node;
    }

    fclose(fp);

    if (root == NULL) {
        init_file_system_if_needed();
    }
    else {
        current_dir = root;
    }
}
