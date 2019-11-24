#include <stdio.h>
#include <stdlib.h>
#include "stack.h"

/**
 * Create a new stack of specified size.
 */
Stack* stackCreate(int size) {
    Stack* stack = (Stack*) malloc(sizeof(Stack));
    stack->array = (int*) malloc(sizeof(int) * size);
    stack->size = size;
    stack->tos = -1;
    return stack;
}

/**
 * Push an integer to the stack.
 */
void stackPush(Stack* stack, int value) {
    if (stack->tos == stack->size - 1) {
        printf("Stack Overflow!\n");
        exit(1);
    }
    stack->array[++stack->tos] = value;
}

/**
 * Pop an integer from the top of the stack and return it.
 */
int stackPop(Stack* stack) {
    if (stack->tos == -1) {
        printf("Stack Underflow!\n");
        exit(1);
    }
    return stack->array[stack->tos--];
}

/**
 * Peek an integer from the top of the stack and return it.
 */
int stackPeek(Stack* stack) {
    if (stack->tos == -1) {
        printf("Stack Underflow!\n");
        exit(1);
    }
    return stack->array[stack->tos];
}

/**
 * Check if the stack is empty.
 * Returns 1 if empty, otherwise false.
 */
int stackEmpty(Stack* stack) {
    if (stack->tos == -1) return 1;
    return 0;
}

/**
 * Free the stack from memory.
 */
void stackFree(Stack* stack) {
    free(stack->array);
    free(stack);
}
