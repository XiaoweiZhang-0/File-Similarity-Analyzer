
#ifndef QUEUE_H
#define QUEUE_H

#include <pthread.h>

struct QueNode {
    struct QueNode *next;
    void *data;
};

typedef struct {
	volatile int size;
    struct QueNode *head;
    struct QueNode *tail;
	pthread_mutex_t lock;
} queue_t;



queue_t *createQue();

void display(queue_t *que);

int enqueue(queue_t *que, void *item);

void *dequeue(queue_t *que);

void destroyQue(queue_t *que);

void ** copyToArray(queue_t *que, int *len);


#endif