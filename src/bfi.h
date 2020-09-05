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

/**
 * Perform the operation represented by the character.
 * Ignore character if it is not an operator.
 */
static inline void doOperation(char ch) {
    // handle pointer movement (> and <)
    if (ch == ADDRESS) {
        int sum = jumps[filePointer - 1];
        pointer += sum;
        if (pointer >= MEMORY_SIZE) pointer -= MEMORY_SIZE;
        else if (pointer < 0) pointer += MEMORY_SIZE;
    }

    // handle value update (+ and -)
    else if (ch == DATA) {
        int sum = jumps[filePointer - 1];
        memory[pointer] += sum;
    }

    // handle output (.)
    else if (ch == '.') {
        printf("%c", memory[pointer]);
        fflush(stdout);
    }

    // handle input (,)
    else if (ch == ',') {
        memory[pointer] = getchar();
    }

    // handle [-]
    else if (ch == SET_ZERO) {
        memory[pointer] = 0;
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
        if (memory[pointer] == 0) {
            filePointer = jumps[filePointer - 1] + 1;
        }
    }

    // handle loop closing (])
    else if (ch == ']') {
        if (memory[pointer] != 0) {
            filePointer = jumps[filePointer - 1] + 1;
        }
    }
}

#endif // BFI_H
