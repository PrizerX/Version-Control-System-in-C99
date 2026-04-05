#ifndef STACK_H
#define STACK_H

#include <stddef.h>

#define STACK_MAX_SIZE 100
#define STACK_HASH_SIZE 65

typedef struct Stack {
    char items[STACK_MAX_SIZE][STACK_HASH_SIZE];
    int top;
} Stack;

void init_stack(Stack *stack);
int isEmpty(const Stack *stack);
int push(Stack *stack, const char *hash);
int pop(Stack *stack, char *out_hash, size_t out_size);
int peek(const Stack *stack, char *out_hash, size_t out_size);

#endif