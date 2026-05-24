#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/filesystem.h"

// 현재 디렉토리
Node* currentDirectory = NULL;

// 노드 생성
Node* createNode(char* name, int isDirectory) {
    Node* newNode = (Node*)malloc(sizeof(Node));

    strcpy(newNode->name, name);
    newNode->isDirectory = isDirectory;

    newNode->parent = NULL;
    newNode->child = NULL;
    newNode->sibling = NULL;

    return newNode;
}

// 자식 추가
void addChild(Node* parent, Node* child) {
    child->parent = parent;

    if (parent->child == NULL) {
        parent->child = child;
    }
    else {
        Node* temp = parent->child;

        while (temp->sibling != NULL) {
            temp = temp->sibling;
        }

        temp->sibling = child;
    }
}

// 자식 찾기
Node* findChild(Node* parent, char* name) {
    Node* temp = parent->child;

    while (temp != NULL) {
        if (strcmp(temp->name, name) == 0) {
            return temp;
        }

        temp = temp->sibling;
    }

    return NULL;
}

// 루트 디렉토리
Node* rootDirectory = NULL;

// 파일 시스템 초기화
void initializeFileSystem() {
    rootDirectory = createNode("/", 1);
    currentDirectory = rootDirectory;
}

// 현재 경로 출력
void printPath(Node* current) {
    if (current == NULL) {
        return;
    }

    if (current == rootDirectory) {
        printf("/");
        return;
    }

    if (current->parent != NULL) {
        printPath(current->parent);
    }

    if (current->parent == rootDirectory) {
        printf("%s", current->name);
    } else {
        printf("/%s", current->name);
    }
}

// 디렉토리 이동
Node* changeDirectory(Node* current, char* path) {

    if (current == NULL || path == NULL) {
        return current;
    }

    // 루트 이동
    if (strcmp(path, "/") == 0) {
        return rootDirectory;
    }

    // 상대경로 처리용 복사
    char tempPath[256];
    strcpy(tempPath, path);

    Node* currentNode;

    // 절대경로
    if (path[0] == '/') {
        currentNode = rootDirectory;
    }
    else {
        currentNode = current;
    }

    char* token = strtok(tempPath, "/");

    while (token != NULL) {

        // 현재 디렉토리
        if (strcmp(token, ".") == 0) {
            token = strtok(NULL, "/");
            continue;
        }

        // 상위 디렉토리
        if (strcmp(token, "..") == 0) {

            if (currentNode->parent != NULL) {
                currentNode = currentNode->parent;
            }

            token = strtok(NULL, "/");
            continue;
        }

        Node* target = findChild(currentNode, token);

        if (target == NULL) {
            printf("cd: no such directory: %s\n", token);
            return current;
        }

        if (target->isDirectory == 0) {
            printf("cd: not a directory: %s\n", token);
            return current;
        }

        currentNode = target;

        token = strtok(NULL, "/");
    }

    return currentNode;
}

// 로드 함수 기본 구조
void loadFileSystem(const char* filename) {
    FILE* fp = fopen(filename, "r");

    if (fp == NULL) {
        printf("load error\n");
        return;
    }

    fclose(fp);
}
// 노드 저장
void saveNode(FILE* fp, Node* node, int depth) {

    if (node == NULL) {
        return;
    }

    for (int i = 0; i < depth; i++) {
        fprintf(fp, "  ");
    }

    fprintf(fp, "%s %d\n", node->name, node->isDirectory);

    saveNode(fp, node->child, depth + 1);

    saveNode(fp, node->sibling, depth);
}


// 파일 시스템 저장
void saveFileSystem(const char* filename) {

    FILE* fp = fopen(filename, "w");

    if (fp == NULL) {
        printf("save error\n");
        return;
    }

    saveNode(fp, rootDirectory, 0);

    fclose(fp);

    printf("filesystem saved\n");
}
