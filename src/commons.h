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

#ifndef COMMONS_H
#define COMMONS_H

#define VERSION "1.1"

#define MIN_TAPE_SIZE   1000
#define MIN_STACK_SIZE  100

#define NO_JUMP          0
#define SET_ZERO        '!'
#define SCAN_ZERO_LEFT  '@'
#define SCAN_ZERO_RIGHT '#'
#define ADDRESS         '$'
#define DATA            '%'

#define isOperator(ch) (strchr("<>+-,.[]", ch) != NULL)

static int TAPE_SIZE;

static int STACK_SIZE;

char* programExecutablePath;

int* jumps;

int filePointer;

unsigned char* tape;

int pointer;

static inline int findZeroLeft(int position);

static inline int findZeroRight(int position);

#endif // COMMONS_H
