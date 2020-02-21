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
