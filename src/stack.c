#include "stack.h"

#include <stdio.h>
#include <string.h>

void init_stack(Stack *stack) {
    if (stack == NULL) {
        return;
    }
    stack->top = -1;
}

int isEmpty(const Stack *stack) {
    if (stack == NULL) {
        return 1;
    }
    return stack->top < 0;
}

int push(Stack *stack, const char *hash) {
    if (stack == NULL || hash == NULL) {
        return -1;
    }
    if (stack->top >= STACK_MAX_SIZE - 1) {
        fprintf(stderr, "Stack overflow: cannot push commit hash\n");
        return -1;
    }

    stack->top++;
    (void)snprintf(stack->items[stack->top], STACK_HASH_SIZE, "%s", hash);
    return 0;
}

int pop(Stack *stack, char *out_hash, size_t out_size) {
    if (stack == NULL || out_hash == NULL || out_size == 0) {
        return -1;
    }
    if (isEmpty(stack)) {
        fprintf(stderr, "Stack underflow: no commit hash to pop\n");
        return -1;
    }

    (void)snprintf(out_hash, out_size, "%s", stack->items[stack->top]);
    stack->top--;
    return 0;
}

int peek(const Stack *stack, char *out_hash, size_t out_size) {
    if (stack == NULL || out_hash == NULL || out_size == 0) {
        return -1;
    }
    if (isEmpty(stack)) {
        fprintf(stderr, "Stack underflow: no commit hash to peek\n");
        return -1;
    }

    (void)snprintf(out_hash, out_size, "%s", stack->items[stack->top]);
    return 0;
}