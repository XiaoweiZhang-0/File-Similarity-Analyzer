#ifndef HASH_H
#define HASH_H
 
#define DEFUALT_HASH_SIZE 1024

struct Entry { //key-value
    char *key;
    double value;
};

struct Node { //链表节点   
    struct Entry *entry;
    struct Node *next;
};

struct HashTable { //hash结构体
    int size;
    int tableLen;//关键字数量, 为给定最大值的下一个素数
    struct Node **lists;//存放链表数组的数组起始位置
};


struct HashTable *initHash(int maxSize);

void put(struct HashTable *hashTable, char *key, double value);

struct Entry * get(struct HashTable *hashTable, char *key);

void displyHashTable(struct HashTable *hashTable);

void destroyHash(struct HashTable *hashTable);


#endif