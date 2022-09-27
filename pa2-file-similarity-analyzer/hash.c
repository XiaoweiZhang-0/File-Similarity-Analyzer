#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "hash.h"

struct HashTable *initHash(int maxSize) {
    struct HashTable *hashTable = (struct HashTable *) malloc(sizeof(struct HashTable));
    hashTable->size = 0;
    hashTable->tableLen = maxSize;//设定最大关键字数量
    hashTable->lists = (struct Node **) malloc(sizeof(struct Node *) * maxSize);//申请链表数组空间

    //为没个关键词开辟链表起点
    for (int i = 0; i < maxSize; i++) {
        hashTable->lists[i] = (struct Node *) malloc(sizeof(struct Node));
        hashTable->lists[i]->next = NULL;
    }
    return hashTable;
}

int DJBHash(char *str) {
    int hash = 5381;
    while (*str) {
        hash += (hash << 5) + (*str++);
    }
    return (hash & 0x7FFFFFFF);
}

void put(struct HashTable *hashTable, char *key, double value) {
    struct Entry *entry = (struct Entry *) malloc(sizeof(struct Entry));
    entry->key = key;
    entry->value = value;

    int hashCode = DJBHash(key);//获取hash值
    int idx = (hashTable->tableLen - 1) & hashCode;
    struct Node *newNode = (struct Node *) malloc(sizeof(struct Node));
    newNode->entry = entry;

    //printf("%s %d\n", key, idx);
    struct Node *temp = hashTable->lists[idx]->next;
    while (temp != NULL) {
        if (strcmp(temp->entry->key, key) == 0) {
            temp->entry->value = value;
            return;
        }
        temp = temp->next;
    }
    newNode->next = hashTable->lists[idx]->next;
    hashTable->lists[idx]->next = newNode;
    hashTable->size++;
}

struct Entry * get(struct HashTable *hashTable, char *key) {
    int hashCode = DJBHash(key);
    int idx = (hashTable->tableLen - 1) & hashCode;
    //首先检查hash值是否存在
    if (hashTable->lists[idx]->next == NULL) {
        return NULL;
    }
    //存在hash值的话则搜索链表
    struct Node *temp = hashTable->lists[idx]->next;
    while (temp && strcmp(temp->entry->key, key) != 0) {
        temp = temp->next;
    }

    if (temp != NULL) {
        return temp->entry;
    } else {
        return NULL;
    }
}

void displyHashTable(struct HashTable *hashTable) {
    printf("size: %d\n", hashTable->size);
    for (int i = 0; i < hashTable->tableLen; i++) {
        struct Node *temp = hashTable->lists[i]->next;
        if (temp == NULL) {
            continue;
        }

        printf("%d: {",i);
        while (temp) {
            printf("%s:%.3lf ", temp->entry->key, temp->entry->value);
            temp = temp->next;
        }
        printf("}\n");
    }
}

void destroyHash(struct HashTable *hashTable) {
    for (int i = 0; i < hashTable->tableLen; i++) {
        struct Node *temp = hashTable->lists[i];
        while (temp) {
            struct Node *next = temp->next;
            if (temp->entry != NULL) {
                free(temp->entry->key);
                free(temp->entry);
                free(temp);
            }
            temp = next;
        }
    }
    free(*hashTable->lists);
}