#ifndef STACK_H
#define STACK_H

typedef struct Stack {
    int* array;
    int size;
    int tos;
} Stack;

Stack* stackCreate(int size);

void stackPush(Stack*, int);

int stackPop(Stack*);

int stackPeek(Stack*);

int stackEmpty(Stack*);

void stackFree(Stack*);

#endif // STACK_H
