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
