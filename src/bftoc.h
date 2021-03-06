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
    fprintf(cFile, "#define MEMORY_SIZE %d\n\n", MEMORY_SIZE);
    fprintf(cFile, "unsigned char memory[MEMORY_SIZE];\n");
    fprintf(cFile, "int pointer = 0;\n\n");
    fprintf(cFile, "int findZeroLeft(int position) {\n\tfor (int i = position; i >= 0; i--) {\n\t\tif (memory[i] == 0) {\n\t\t\treturn i;\n\t\t}\n\t}\n\tfor (int i = MEMORY_SIZE - 1; i > position; i--) {\n\t\tif (memory[i] == 0) {\n\t\t\treturn i;\n\t\t}\n\t}\n\treturn -1;\n}\n\n");
    fprintf(cFile, "int findZeroRight(int position) {\n\tfor (int i = position; i < MEMORY_SIZE; i++) {\n\t\tif (memory[i] == 0) {\n\t\t\treturn i;\n\t\t}\n\t}\n\tfor (int i = 0; i < position; i++) {\n\t\tif (memory[i] == 0) {\n\t\t\treturn i;\n\t\t}\n\t}\n\treturn -1;\n}\n\n");
    fprintf(cFile, "int main() {\n");
    fprintf(cFile, "\tmemset(memory, 0, MEMORY_SIZE);\n\n");
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
    if (ch == ADDRESS) {
        int sum = jumps[filePointer - 1];

        fprintf(cFile, "%spointer += %d;\n", indent, sum);
        fprintf(cFile, "%sif (pointer >= MEMORY_SIZE) pointer -= MEMORY_SIZE;\n", indent);
        fprintf(cFile, "%selse if (pointer < 0) pointer += MEMORY_SIZE;\n", indent);
    }

    // handle value update (+ and -)
    else if (ch == DATA) {
        int sum = jumps[filePointer - 1];
        fprintf(cFile, "%smemory[pointer] += %d;\n", indent, sum);
    }

    // handle output (.)
    else if (ch == '.') {
        fprintf(cFile, "%sprintf(\"%%c\", memory[pointer]);\n", indent);
        fprintf(cFile, "%sfflush(stdout);\n", indent);
    }

    // handle input (,)
    else if (ch == ',') {
        fprintf(cFile, "%smemory[pointer] = getchar();\n", indent);
    }

    // handle [-]
    else if (ch == SET_ZERO) {
        memory[pointer] = 0;
        fprintf(cFile, "%smemory[pointer] = 0;\n", indent);
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
        fprintf(cFile, "%swhile (memory[pointer] != 0) {\n", indent);
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
