#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include "queue.h"

queue_t * createQue() {
    queue_t *que = (queue_t *) malloc(sizeof(queue_t));
	que->size = 0;
    struct QueNode *n = (struct QueNode *) malloc(sizeof(struct QueNode));
    n->next = NULL;
    que->head = que->tail = n;
	pthread_mutex_init(&que->lock, NULL);
    return que;
}



void display(queue_t *que) {
    printf("size=%d {", que->size);
    struct QueNode *cur = que->head->next;
    while (cur) {
        printf("%s ", (char *) cur->data);
        cur = cur->next;
    }
    printf("}\n");
}

// add item to end of queue
// if the queue is full, block until space becomes available
int enqueue(queue_t *que, void *item) {
    struct QueNode *node = (struct QueNode *) malloc(sizeof(struct QueNode));
    node->data = item;
    node->next = NULL;

    if (node == NULL) {
        return -1;
    }

	pthread_mutex_lock(&que->lock);
    if (que->tail == que->head) {}
    que->tail = que->tail->next = node;
    que->size++;
	pthread_mutex_unlock(&que->lock);
	return 0;
}


void *dequeue(queue_t *que) {
    if (que->size == 0) {
        return NULL;
    }

    struct QueNode *oldHead = que->head;
    void *data = NULL;
	pthread_mutex_lock(&que->lock);
    if (que->size > 0) {
        struct QueNode *first = oldHead->next;
        data = first->data;
        que->head = first;
        que->size--;
    }
	pthread_mutex_unlock(&que->lock);
    void *retData = (void *) malloc(sizeof(void) * strlen(data));
    free(oldHead);
    return strcpy(retData, data);
}


void ** copyToArray(queue_t *que, int *len) {
    void **ary = (void **) malloc(sizeof(void *) * que->size);
    struct QueNode *cur = que->head->next;
    *len = 0;
    while (cur) {
        ary[(*len)++] = cur->data;
        cur = cur->next;
    }
    return ary;
}


void destroyQue(queue_t *que) {
	pthread_mutex_destroy(&que->lock);
}
