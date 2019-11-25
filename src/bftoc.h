#ifndef BFTOC_H
#define BFTOC_H

#include "commons.h"

FILE* cFile;

char* cFilePath;
char* exeFilePath;

char* indent;
int indentPointer;

/**
 * Initialize the Brainfuck to C translator.
 */
static inline void initTranslator(char* filePath) {
    cFile = fopen(filePath, "w");

    if (cFile == NULL) {
        // display error message and exit
        fprintf(stderr, "Failed to write to file: %s\n", filePath);
        exit(1);
    }

    indent = (char*) malloc(sizeof(char) * (STACK_SIZE));
    indent[0] = '\t';
    indent[1] = '\0';
    indentPointer = 1;
}

/**
 * Clean up the Brainfuck to C translator.
 */
static inline void cleanupTranslator() {
    fclose(cFile);
    free(indent);
    free(cFilePath);
    free(exeFilePath);
}

/**
 * Write common header information for C file.
 */
static inline void writeCHeader() {
    fprintf(cFile, "#include<stdio.h>\n");
    fprintf(cFile, "#include<string.h>\n\n");
    fprintf(cFile, "#define TAPE_SIZE %d\n\n", TAPE_SIZE);
    fprintf(cFile, "unsigned char tape[TAPE_SIZE];\n");
    fprintf(cFile, "int pointer = 0;\n\n");
    fprintf(cFile, "int findZeroLeft(int position) {\n\tfor (int i = position; i >= 0; i--) {\n\t\tif (tape[i] == 0) {\n\t\t\treturn i;\n\t\t}\n\t}\n\tfor (int i = TAPE_SIZE - 1; i > position; i--) {\n\t\tif (tape[i] == 0) {\n\t\t\treturn i;\n\t\t}\n\t}\n\treturn -1;\n}\n\n");
    fprintf(cFile, "int findZeroRight(int position) {\n\tfor (int i = position; i < TAPE_SIZE; i++) {\n\t\tif (tape[i] == 0) {\n\t\t\treturn i;\n\t\t}\n\t}\n\tfor (int i = 0; i < position; i++) {\n\t\tif (tape[i] == 0) {\n\t\t\treturn i;\n\t\t}\n\t}\n\treturn -1;\n}\n\n");
    fprintf(cFile, "int main() {\n");
    fprintf(cFile, "\tmemset(tape, 0, TAPE_SIZE);\n\n");
}

/**
 * Write common footer information for C file.
 */
static inline void writeCFooter() {
    fprintf(cFile, "\n\treturn 0;\n}\n");
}

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
                fprintf(cFile, "%spointer = (pointer + 1) %% TAPE_SIZE;\n", indent);
            }
            else {
                fprintf(cFile, "%sif (pointer == 0) pointer = TAPE_SIZE - 1;\n", indent);
                fprintf(cFile, "%selse pointer--;\n", indent);
            }
        }
        else {
            int sum = jumps[index];

            if (sum > 0) {
                fprintf(cFile, "%spointer = (pointer + %d) %% TAPE_SIZE;\n", indent, sum);
            }
            else if (sum < 0) {
                fprintf(cFile, "%spointer = (pointer + %d);\n", indent, sum);
                fprintf(cFile, "%sif (pointer < 0) pointer += TAPE_SIZE;\n", indent);
            }

            filePointer = index + 1;
        }
    }

    // handle value update (+ and -)
    else if (ch == '+' || ch == '-') {
        int index = jumps[filePointer - 1];

        if (index == NO_JUMP) {
            if (ch == '+') {
                fprintf(cFile, "%stape[pointer]++;\n", indent);
            }
            else {
                fprintf(cFile, "%stape[pointer]--;\n", indent);
            }
        }
        else {
            int sum = jumps[index];
            filePointer = index + 1;
            fprintf(cFile, "%stape[pointer] += %d;\n", indent, sum);
        }
    }

    // handle output (.)
    else if (ch == '.') {
        fprintf(cFile, "%sprintf(\"%%c\", tape[pointer]);\n", indent);
        fprintf(cFile, "%sfflush(stdout);\n", indent);
    }

    // handle input (,)
    else if (ch == ',') {
        fprintf(cFile, "%stape[pointer] = getchar();\n", indent);
    }

    // handle loop opening ([)
    else if (ch == '[') {
        int flag = jumps[filePointer - 1];
        // optimize [-]
        if (flag == SET_ZERO) {
            filePointer += 2;
            fprintf(cFile, "%stape[pointer] = 0;\n", indent);
        }
        // optimize [<]
        else if (flag == SCAN_ZERO_LEFT) {
            filePointer += 2;
            fprintf(cFile, "%spointer = findZeroLeft(pointer);\n", indent);
        }
        // optimize [>]
        else if (flag == SCAN_ZERO_RIGHT) {
            filePointer += 2;
            fprintf(cFile, "%spointer = findZeroRight(pointer);\n", indent);
        }
        // start loop
        else {
            fprintf(cFile, "%swhile (tape[pointer] != 0) {\n", indent);
            indent[indentPointer++] = '\t';
            indent[indentPointer] = '\0';
        }
    }

    // handle loop closing (])
    else if (ch == ']') {
        // end loop
        indent[--indentPointer] = '\0';
        fprintf(cFile, "%s}\n", indent);
    }
}

#endif // BFTOC_H
