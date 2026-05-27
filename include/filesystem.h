#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <stdio.h>

#define MAX_NAME 64
#define MAX_CONTENT 4096
#define FS_DATA_PATH "data/fs_data.txt"

typedef enum {
    NODE_DIR,
    NODE_FILE
} NodeType;

typedef struct Node {
    char name[MAX_NAME];
    NodeType type;
    char content[MAX_CONTENT];
    char owner[MAX_NAME];

    struct Node* parent;
    struct Node* child;
    struct Node* sibling;
} Node;

extern Node* root;
extern Node* current_dir;

Node* create_node(const char* name, NodeType type);

void add_child(Node* parent, Node* child);

Node* find_child(Node* dir, const char* name);

void init_file_system_if_needed(void);

void print_path(Node* node);

void cmd_pwd_core(void);

int change_directory(const char* path);

void save_node(FILE* fp, Node* node, int depth);

void save_file_system(const char* filename);

void load_file_system(const char* filename);

#endif
