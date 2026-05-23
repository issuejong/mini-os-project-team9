#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#define MAX_NAME 50

typedef struct Node {
    char name[MAX_NAME];
    int isDirectory;

    struct Node* parent;
    struct Node* child;
    struct Node* sibling;
} Node;

// 현재 디렉토리
extern Node* currentDirectory;

// 함수 선언
Node* createNode(char* name, int isDirectory);

void addChild(Node* parent, Node* child);

Node* findChild(Node* parent, char* name);

void printPath(Node* current);

Node* changeDirectory(Node* current, char* name);

#endif
