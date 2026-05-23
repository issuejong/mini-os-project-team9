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
