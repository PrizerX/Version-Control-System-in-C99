#ifndef QUEUE_H
#define QUEUE_H

#include <stddef.h>

#define QUEUE_MAX_SIZE 100
#define QUEUE_FILENAME_SIZE 260

typedef struct Queue {
    char items[QUEUE_MAX_SIZE][QUEUE_FILENAME_SIZE];
    int front;
    int rear;
    int count;
} Queue;

void init_queue(Queue *queue);
int isEmptyQueue(const Queue *queue);
int isFullQueue(const Queue *queue);
int enqueue(Queue *queue, const char *filename);
int dequeue(Queue *queue, char *out_filename, size_t out_size);

#endif