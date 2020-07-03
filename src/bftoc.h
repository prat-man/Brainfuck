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

#ifndef BFTOC_H
#define BFTOC_H

#include "commons.h"

FILE* cFile = NULL;

char* cFilePath = NULL;
char* exeFilePath = NULL;

char* indent = NULL;
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
    // close cFile
    if (cFile != NULL) {
        fclose(cFile);
        cFile = NULL;
    }

    // free indent
    if (indent != NULL) {
        free(indent);
        indent = NULL;
    }

    // free cFilePath
    if (cFilePath != NULL) {
        free(cFilePath);
        cFilePath = NULL;
    }

    // free exeFilePath
    if (exeFilePath != NULL) {
        free(exeFilePath);
        exeFilePath = NULL;
    }
}

/**
 * Write common header information for C file.
 */
static inline void writeCHeader() {
    fprintf(cFile, "#include<stdio.h>\n");
    fprintf(cFile, "#include<string.h>\n\n");

    if (lineBuffered) {
        fprintf(cFile, "#define TAPE_SIZE %d\n", TAPE_SIZE);
        fprintf(cFile, "#define LINE_SIZE %d\n\n", LINE_SIZE);
        fprintf(cFile, "#define TRUE 1\n");
        fprintf(cFile, "#define FALSE 0\n\n");
        fprintf(cFile, "unsigned char tape[TAPE_SIZE];\n");
        fprintf(cFile, "unsigned char line[LINE_SIZE];\n\n");
    }
    else {
        fprintf(cFile, "#define TAPE_SIZE %d\n\n", TAPE_SIZE);
        fprintf(cFile, "unsigned char tape[TAPE_SIZE];\n\n");
    }

    fprintf(cFile, "int pointer = 0;\n\n");

    if (lineBuffered) {
        fprintf(cFile, "int linePosition = 0;\n\n");
    }

    fprintf(cFile, "static inline int findZeroLeft(int position) {\n\tfor (int i = position; i >= 0; i--) {\n\t\tif (tape[i] == 0) {\n\t\t\treturn i;\n\t\t}\n\t}\n\tfor (int i = TAPE_SIZE - 1; i > position; i--) {\n\t\tif (tape[i] == 0) {\n\t\t\treturn i;\n\t\t}\n\t}\n\treturn -1;\n}\n\n");
    fprintf(cFile, "static inline int findZeroRight(int position) {\n\tfor (int i = position; i < TAPE_SIZE; i++) {\n\t\tif (tape[i] == 0) {\n\t\t\treturn i;\n\t\t}\n\t}\n\tfor (int i = 0; i < position; i++) {\n\t\tif (tape[i] == 0) {\n\t\t\treturn i;\n\t\t}\n\t}\n\treturn -1;\n}\n\n");

    if (lineBuffered) {
        fprintf(cFile, "static inline void printLine(int force) {\n\tif (force) {\n\t\tif (linePosition != 0) {\n\t\t\tline[linePosition] = '\\0';\n\t\t\tfputs(line, stdout);\n\t\t\tlinePosition = 0;\n\t\t}\n\t}\n\telse {\n\t\tline[linePosition++] = tape[pointer];\n\t\t\n\t\tif (tape[pointer] == '\\n' || linePosition == LINE_SIZE - 1) {\n\t\t\tline[linePosition] = '\\0';\n\t\t\tfputs(line, stdout);\n\t\t\tlinePosition = 0;\n\t\t}\n\t}\n}\n\n");
    }

    fprintf(cFile, "int main() {\n");
    fprintf(cFile, "\tmemset(tape, 0, TAPE_SIZE);\n\n");
}

/**
 * Write common footer information for C file.
 */
static inline void writeCFooter() {
    if (lineBuffered) {
        fprintf(cFile, "%sprintLine(TRUE);\n", indent);
    }

    fprintf(cFile, "\n\treturn 0;\n}\n");
}

/**
 * Perform the operation represented by the character.
 * Ignore character if it is not an operator.
 */
static inline void doTranslate(char ch) {
    // handle pointer movement (> and <)
    if (ch == ADDRESS) {
        int sum = jumps[filePointer - 1];

        fprintf(cFile, "%spointer += %d;\n", indent, sum);
        fprintf(cFile, "%sif (pointer >= TAPE_SIZE) pointer -= TAPE_SIZE;\n", indent);
        fprintf(cFile, "%selse if (pointer < 0) pointer += TAPE_SIZE;\n", indent);
    }

    // handle value update (+ and -)
    else if (ch == DATA) {
        int sum = jumps[filePointer - 1];
        fprintf(cFile, "%stape[pointer] += %d;\n", indent, sum);
    }

    // handle output (.)
    else if (ch == '.') {
        if (lineBuffered) {
            fprintf(cFile, "%sprintLine(FALSE);\n", indent);
        }
        else {
            fprintf(cFile, "%sfputc(tape[pointer], stdout);\n", indent);
        }
    }

    // handle input (,)
    else if (ch == ',') {
        if (lineBuffered) {
            fprintf(cFile, "%sprintLine(TRUE);\n", indent);
        }
        fprintf(cFile, "%stape[pointer] = getchar();\n", indent);
    }

    // handle [-]
    else if (ch == SET_ZERO) {
        tape[pointer] = 0;
        fprintf(cFile, "%stape[pointer] = 0;\n", indent);
    }

    // handle [<]
    else if (ch == SCAN_ZERO_LEFT) {
        fprintf(cFile, "%spointer = findZeroLeft(pointer);\n", indent);
    }

    // handle [>]
    else if (ch == SCAN_ZERO_RIGHT) {
        fprintf(cFile, "%spointer = findZeroRight(pointer);\n", indent);
    }

    // handle loop opening ([)
    else if (ch == '[') {
        fprintf(cFile, "%swhile (tape[pointer] != 0) {\n", indent);
        indent[indentPointer++] = '\t';
        indent[indentPointer] = '\0';
    }

    // handle loop closing (])
    else if (ch == ']') {
        // end loop
        indent[--indentPointer] = '\0';
        fprintf(cFile, "%s}\n", indent);
    }
}

#endif // BFTOC_H
