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
Node* changeDirectory(Node* current, char* name) {
    if (current == NULL) {
        return current;
    }

    if (strcmp(name, "/") == 0) {
        return rootDirectory;
    }

    if (strcmp(name, "..") == 0) {
        if (current->parent != NULL) {
            return current->parent;
        }
        return current;
    }

    Node* target = findChild(current, name);

    if (target == NULL) {
        printf("cd: no such directory\n");
        return current;
    }

    if (target->isDirectory == 0) {
        printf("cd: not a directory\n");
        return current;
    }

    return target;
}

// 저장 함수 기본 구조
void saveFileSystem(const char* filename) {
    FILE* fp = fopen(filename, "w");

    if (fp == NULL) {
        printf("save error\n");
        return;
    }

    fprintf(fp, "filesystem save structure\n");

    fclose(fp);
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
