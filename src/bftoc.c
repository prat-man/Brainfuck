#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "bftoc.h"

#define NO_JUMP         INT_MAX
#define SET_ZERO        (INT_MAX - 1)
#define SCAN_ZERO_LEFT  (INT_MAX - 2)
#define SCAN_ZERO_RIGHT (INT_MAX - 3)

FILE* cFile;

/**
 * Perform the operation represented by the character.
 * Ignore character if it is not an operator.
 */
static inline void doTranslate(char ch) {
    // handle pointer movement (> and <)
    if (ch == '>' || ch == '<') {
        int index = jumps[filePointer - 1];

        if (index == NO_JUMP) {
            if (ch == '>') {
                fprintf(cFile, "pointer = (pointer + 1) %% TAPE_SIZE;\n");
            }
            else {
                fprintf(cFile, "if (pointer == 0) pointer = TAPE_SIZE - 1;\n");
                fprintf(cFile, "else pointer--;\n");
            }
        }
        else {
            int sum = jumps[index];

            if (sum > 0) {
                fprintf(cFile, "pointer = (pointer + %d) %% TAPE_SIZE;\n", sum);
            }
            else if (sum < 0) {
                fprintf(cFile, "pointer = (pointer + %d);\n", sum);
                fprintf(cFile, "if (pointer < 0) pointer += TAPE_SIZE;\n");
            }

            filePointer = index + 1;
        }
    }

    // handle value update (+ and -)
    else if (ch == '+' || ch == '-') {
        int index = jumps[filePointer - 1];

        if (index == NO_JUMP) {
            if (ch == '+') {
                fprintf(cFile, "tape[pointer]++;\n");
            }
            else {
                fprintf(cFile, "tape[pointer]--;\n");
            }
        }
        else {
            int sum = jumps[index];
            filePointer = index + 1;
            fprintf(cFile, "tape[pointer] += %d;\n", sum);
        }
    }

    // handle output (.)
    else if (ch == '.') {
        fprintf(cFile, "printf(\"%%c\", tape[pointer]);\n");
        fprintf(cFile, "fflush(stdout);\n");
    }

    // handle input (,)
    else if (ch == ',') {
        fprintf(cFile, "tape[pointer] = getchar();\n");
    }

    // handle loop opening ([)
    else if (ch == '[') {
        int flag = jumps[filePointer - 1];
        // optimize [-]
        if (flag == SET_ZERO) {
            filePointer += 2;
            fprintf(cFile, "tape[pointer] = 0;\n");
        }
        // optimize [<]
        else if (flag == SCAN_ZERO_LEFT) {
            filePointer += 2;
            fprintf(cFile, "pointer = findZeroLeft(pointer);\n");
        }
        // optimize [>]
        else if (flag == SCAN_ZERO_RIGHT) {
            filePointer += 2;
            fprintf(cFile, "pointer = findZeroRight(pointer);\n");
        }
        // start loop
        else {
            fprintf(cFile, "while (tape[pointer] != 0) {\n");
        }
    }

    // handle loop closing (])
    else if (ch == ']') {
        // end loop
        fprintf(cFile, "}\n");
    }
}
