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
    if (ch == '>' || ch == '<') {
        int index = jumps[filePointer - 1];

        if (index == NO_JUMP) {
            if (ch == '>') {
                pointer = (pointer + 1) % TAPE_SIZE;
            }
            else {
                if (pointer == 0) {
                    pointer = TAPE_SIZE - 1;
                }
                else {
                    pointer--;
                }
            }
        }
        else {
            int sum = jumps[index];

            if (sum > 0) {
                pointer = (pointer + sum) % TAPE_SIZE;
            }
            else if (sum < 0) {
                pointer = (pointer + sum);
                if (pointer < 0) {
                    pointer = TAPE_SIZE + pointer;
                }
            }

            filePointer = index + 1;
        }
    }

    // handle value update (+ and -)
    else if (ch == '+' || ch == '-') {
        int index = jumps[filePointer - 1];

        if (index == NO_JUMP) {
            if (ch == '+') {
                tape[pointer]++;
            }
            else {
                tape[pointer]--;
            }
        }
        else {
            int sum = jumps[index];
            tape[pointer] += sum;
            filePointer = index + 1;
        }
    }

    // handle output (.)
    else if (ch == '.') {
        printf("%c", tape[pointer]);
        fflush(stdout);
    }

    // handle input (,)
    else if (ch == ',') {
        tape[pointer] = getchar();
    }

    // handle loop opening ([)
    else if (ch == '[') {
        int flag = jumps[filePointer - 1];
        // optimize [-]
        if (flag == SET_ZERO) {
            tape[pointer] = 0;
            filePointer += 2;
        }
        // optimize [<]
        else if (flag == SCAN_ZERO_LEFT) {
            pointer = findZeroLeft(pointer);
            filePointer += 2;
        }
        // optimize [>]
        else if (flag == SCAN_ZERO_RIGHT) {
            pointer = findZeroRight(pointer);
            filePointer += 2;
        }
        // optimize jump
        else if (tape[pointer] == 0) {
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
