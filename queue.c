#include "queue.h"

#include <stdio.h>

void init_queue(Queue *queue) {
    if (queue == NULL) {
        return;
    }

    queue->front = -1;
    queue->rear = -1;
    queue->count = 0;
}

int isEmptyQueue(const Queue *queue) {
    if (queue == NULL) {
        return 1;
    }
    return queue->count == 0;
}

int isFullQueue(const Queue *queue) {
    if (queue == NULL) {
        return 0;
    }
    return queue->count >= QUEUE_MAX_SIZE;
}

int enqueue(Queue *queue, const char *filename) {
    if (queue == NULL || filename == NULL) {
        return -1;
    }

    if (isFullQueue(queue)) {
        fprintf(stderr, "Queue overflow: staging queue is full\n");
        return -1;
    }

    if (isEmptyQueue(queue)) {
        queue->front = 0;
        queue->rear = 0;
    } else {
        queue->rear = (queue->rear + 1) % QUEUE_MAX_SIZE;
    }

    (void)snprintf(queue->items[queue->rear], QUEUE_FILENAME_SIZE, "%s", filename);
    queue->count++;
    return 0;
}

int dequeue(Queue *queue, char *out_filename, size_t out_size) {
    if (queue == NULL || out_filename == NULL || out_size == 0) {
        return -1;
    }

    if (isEmptyQueue(queue)) {
        fprintf(stderr, "Queue underflow: staging queue is empty\n");
        return -1;
    }

    (void)snprintf(out_filename, out_size, "%s", queue->items[queue->front]);

    if (queue->count == 1) {
        queue->front = -1;
        queue->rear = -1;
    } else {
        queue->front = (queue->front + 1) % QUEUE_MAX_SIZE;
    }

    queue->count--;
    return 0;
}