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

#ifndef BFI_H
#define BFI_H

#include "commons.h"

static char* line;

static int linePosition;

/**
 * Print a line
 * If force is TRUE, flush the line buffer
 * If force is FALSE, add the character at tape[pointer] to line, if line needs to be flush, flush it
 */
static inline void printLine(int force) {
    if (force) {
        if (linePosition != 0) {
            line[linePosition] = '\0';
            fputs(line, stdout);
            linePosition = 0;
        }
    }
    else {
        line[linePosition++] = tape[pointer];

        if (tape[pointer] == '\n' || linePosition == LINE_SIZE - 1) {
            line[linePosition] = '\0';
            fputs(line, stdout);
            linePosition = 0;
        }
    }
}

/**
 * Initialize the Brainfuck interpreter.
 */
static inline void initInterpreter() {
    line = (char*) malloc(sizeof(char) * (LINE_SIZE));
    linePosition = 0;
}

/**
 * Clean up the Brainfuck interpreter.
 */
static inline void cleanupInterpreter() {
    printLine(TRUE);

    // free line
    if (line != NULL) {
        free(line);
        line = NULL;
    }
}

/**
 * Perform the operation represented by the character.
 * Ignore character if it is not an operator.
 */
static inline void doOperationBuffered(char ch) {
    // handle pointer movement (> and <)
    if (ch == ADDRESS) {
        int sum = jumps[filePointer - 1];
        pointer += sum;
        if (pointer >= TAPE_SIZE) pointer -= TAPE_SIZE;
        else if (pointer < 0) pointer += TAPE_SIZE;
    }

    // handle value update (+ and -)
    else if (ch == DATA) {
        int sum = jumps[filePointer - 1];
        tape[pointer] += sum;
    }

    // handle output (.)
    else if (ch == '.') {
        printLine(FALSE);
    }

    // handle input (,)
    else if (ch == ',') {
        printLine(TRUE);
        tape[pointer] = getchar();
    }

    // handle [-]
    else if (ch == SET_ZERO) {
        tape[pointer] = 0;
    }

    // handle [<]
    else if (ch == SCAN_ZERO_LEFT) {
        pointer = findZeroLeft(pointer);
    }

    // handle [>]
    else if (ch == SCAN_ZERO_RIGHT) {
        pointer = findZeroRight(pointer);
    }

    // handle loop opening ([)
    else if (ch == '[') {
        if (tape[pointer] == 0) {
            filePointer = jumps[filePointer - 1] + 1;
        }
    }

    // handle loop closing (])
    else if (ch == ']') {
        if (tape[pointer] != 0) {
            filePointer = jumps[filePointer - 1] + 1;
        }
    }
}

/**
 * Perform the operation represented by the character.
 * Ignore character if it is not an operator.
 */
static inline void doOperationUnbuffered(char ch) {
    // handle pointer movement (> and <)
    if (ch == ADDRESS) {
        int sum = jumps[filePointer - 1];
        pointer += sum;
        if (pointer >= TAPE_SIZE) pointer -= TAPE_SIZE;
        else if (pointer < 0) pointer += TAPE_SIZE;
    }

    // handle value update (+ and -)
    else if (ch == DATA) {
        int sum = jumps[filePointer - 1];
        tape[pointer] += sum;
    }

    // handle output (.)
    else if (ch == '.') {
        fputc(tape[pointer], stdout);
    }

    // handle input (,)
    else if (ch == ',') {
        tape[pointer] = getchar();
    }

    // handle [-]
    else if (ch == SET_ZERO) {
        tape[pointer] = 0;
    }

    // handle [<]
    else if (ch == SCAN_ZERO_LEFT) {
        pointer = findZeroLeft(pointer);
    }

    // handle [>]
    else if (ch == SCAN_ZERO_RIGHT) {
        pointer = findZeroRight(pointer);
    }

    // handle loop opening ([)
    else if (ch == '[') {
        if (tape[pointer] == 0) {
            filePointer = jumps[filePointer - 1] + 1;
        }
    }

    // handle loop closing (])
    else if (ch == ']') {
        if (tape[pointer] != 0) {
            filePointer = jumps[filePointer - 1] + 1;
        }
    }
}

#endif // BFI_H
