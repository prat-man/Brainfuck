/**
 * A fast Brainfuck interpreter, translator, and compiler.
 * Copyright (C) 2019  Pratanu Mandal
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

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
