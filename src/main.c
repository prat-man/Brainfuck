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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>

#include "stack.h"
#include "commons.h"
#include "bfi.h"
#include "bftoc.h"

// size of tape to be used by the interpreter
static int TAPE_SIZE = 30000;

// size of stack to be used by the interpreter
static int STACK_SIZE = 1000;

// source file stored in memory for fast access
char* source = NULL;

// pre-processed source file stored in memory for fast access
char* processed = NULL;

// size of source file in chars
int fileSize;

// size of pre-processed source file in chars
int processedFileSize;

// pointer to next character to be read from file
int filePointer = 0;

// jumps and flags are preinitialized for optimized execution
int* jumps = NULL;

// the brainfuck tape - 0-255 - circular
unsigned char* tape = NULL;

// pointer to current location in tape
int pointer = 0;

// loop stack
Stack* stack = NULL;

/**
 * Load source file into memory.
 */
void loadFile(char* filePath) {
    FILE* fp = fopen(filePath, "r");

    if (fp != NULL) {
        // go to the end of the file
        if (fseek(fp, 0L, SEEK_END) == 0) {
            // get the size of the file
            long bufsize = ftell(fp);
            if (bufsize == -1) {
                // display error message and exir
                fprintf(stderr, "Error reading source file: %s\n", filePath);
                exit(1);
            }

            // allocate our buffer to that size
            source = (char*) malloc(sizeof(char) * (bufsize + 1));

            // go back to the start of the file
            if (fseek(fp, 0L, SEEK_SET) != 0) {
                // display error message and exit
                fprintf(stderr, "Error reading source file: %s\n", filePath);
                exit(1);
            }

            // read the entire file into memory
            size_t newLen = fileSize = fread(source, sizeof(char), bufsize, fp);
            if (ferror(fp) != 0) {
                // display error message and exit
                fprintf(stderr, "Error reading source file: %s\n", filePath);
                exit(1);
            } else {
                // append a null character just to be safe
                source[newLen++] = '\0';
            }
        }

        // close the file
        fclose(fp);
    }
    else {
        // display error message and exit
        fprintf(stderr, "Source file not found: %s\n", filePath);
        exit(1);
    }
}

/**
 * Initialize the jumps in the file for optimization.
 * Jumps between [ and ].
 * Compacts and jumps consecutive > and <.
 * Compacts and jumps consecutive + and -.
 * Optimizes [-] to set(0).
 * Optimizes [<] to scan_left(0).
 * Optimizes [>] to scan_right(0).
 */
void initJumps() {
    // initialize pre-processed source and jumps
    processed = (char*) malloc(sizeof(char) * (fileSize));
    jumps = (int*) malloc(sizeof(int) * (fileSize));

    // create a stack for [ operators
    stack = stackCreate(STACK_SIZE);

    // find jumps to optimize code
    int i, index;
    for (i = 0, index = 0; i < fileSize; i++, index++) {
        // get one character
        char ch = source[i];

        // create jumps for opening and closing square brackets [ and ]
        if (ch == '[') {
            char ch2 = i + 1 < fileSize ? source[i + 1] : -1;
            char ch3 = i + 2 < fileSize ? source[i + 2] : -1;
            if (ch2 == '-' && ch3 == ']') {
                // optimize [-] to set(0)
                processed[index] = SET_ZERO;
                i += 2;
            }
            else if (ch2 == '<' && ch3 == ']') {
                // optimize [<] to scan_left(0)
                processed[index] = SCAN_ZERO_LEFT;
                i += 2;
            }
            else if (ch2 == '>' && ch3 == ']') {
                // optimize [>] to scan_right(0)
                processed[index] = SCAN_ZERO_RIGHT;
                i += 2;
            }
            else {
                // push opening bracket [ to stack
                processed[index] = ch;
                stackPush(stack, index);
            }
        }
        else if (ch == ']') {
            // pop opening bracket and swap indexes in jump table
            int x = stackPop(stack);
            jumps[x] = index;
            jumps[index] = x;
            processed[index] = ch;
        }

        // compact and jump for > and <
        else if (ch == '>' || ch == '<') {
            int sum = 0;

            if (ch == '>') sum++;
            else sum--;

            while (++i < fileSize) {
                if (source[i] == '>') {
                    sum++;
                }
                else if (source[i] == '<') {
                    sum--;
                }
                else if (isOperator(source[i])) {
                    break;
                }
            }
            i--;

            processed[index] = ADDRESS;
            jumps[index] = sum;
        }

        // compact and jump for + and -
        else if (ch == '+' || ch == '-') {
            int sum = 0;

            if (ch == '+') sum++;
            else sum--;

            while (++i < fileSize) {
                if (source[i] == '+') {
                    sum++;
                }
                else if (source[i] == '-') {
                    sum--;
                }
                else if (isOperator(source[i])) {
                    break;
                }
            }
            i--;

            processed[index] = DATA;
            jumps[index] = sum;
        }

        // input/output no jump
        else if (ch == ',' || ch == '.') {
            processed[index] = ch;
            jumps[index] = NO_JUMP;
        }

        // for everything else, do not include in pre-processed source
        else {
            index--;
        }
    }

    // set size of pre-processed file
    processedFileSize = index;

    // loops are unmatched
    if (!stackEmpty(stack)) {
        // free stack
        stackFree(stack);
        stack = NULL;

        // display error message and exit
        fprintf(stderr, "Unmatched loops!");
        exit(1);
    }

    // free stack
    stackFree(stack);
    stack = NULL;
}

/**
 * Read a single character from the source file.
 */
static inline char readChar() {
    if (filePointer == processedFileSize) {
        return -1;
    }
    return processed[filePointer++];
}

/**
 * Find first zero in tape at or to the left of position.
 */
int findZeroLeft(int position) {
    for (int i = position; i >= 0; i--) {
        if (tape[i] == 0) {
            return i;
        }
    }
    for (int i = TAPE_SIZE - 1; i > position; i--) {
        if (tape[i] == 0) {
            return i;
        }
    }
    return -1;
}

/**
 * Find first zero in tape at or to the right of position.
 */
int findZeroRight(int position) {
    for (int i = position; i < TAPE_SIZE; i++) {
        if (tape[i] == 0) {
            return i;
        }
    }
    for (int i = 0; i < position; i++) {
        if (tape[i] == 0) {
            return i;
        }
    }
    return -1;
}

/**
 * Generate the C file path.
 */
char* generateCFilePath(char* filePath) {
    int length = strlen(filePath);

    char* cFilePath = (char*) malloc(sizeof(char) * (length));

    strcpy(cFilePath, filePath);

    cFilePath[length - 2] = 'c';
    cFilePath[length - 1] = '\0';

    return cFilePath;
}

/**
 * Generate the executable file path.
 */
char* generateExecutableFilePath(char* filePath) {
    int length = strlen(filePath);

    char* exeFilePath = (char*) malloc(sizeof(char) * (length));

    strcpy(exeFilePath, filePath);

    exeFilePath[length - 3] = '\0';

    return exeFilePath;
}

/**
 * Execute the brainfuck source code.
 */
void execute(char* filePath) {
    // initialize tape and fill with zeros
    tape = (unsigned char*) malloc(sizeof(unsigned char) * (TAPE_SIZE));
    memset(tape, 0, TAPE_SIZE);

    // load source file
    loadFile(filePath);

    // initialize file jumps for optimization
    initJumps();

    // for each character do operation
    char ch;
    while ((ch = readChar()) != -1) {
        doOperation(ch);
    }
}

/**
 * Translate the brainfuck source code into C code.
 */
void translate(char* filePath) {
    // initialize tape and fill with zeros
    tape = (unsigned char*) malloc(sizeof(unsigned char) * (TAPE_SIZE));
    memset(tape, 0, TAPE_SIZE);

    // load source file
    loadFile(filePath);

    // initialize file jumps for optimization
    initJumps();

    // generate C file path
    cFilePath = generateCFilePath(filePath);

    // initialize the Brainfuck to C translator
    initTranslator(cFilePath);

    // write the common header for C file
    writeCHeader();

    // for each character do translation
    char ch;
    while ((ch = readChar()) != -1) {
        doTranslate(ch);
    }

    // write the common footer for C file
    writeCFooter();

    // clean up the Brainfuck to C translator
    cleanupTranslator();
}

/**
 * Execute a system call silently.
 */
int executeCommand(const char *cmd) {
    // build command string
    char command[strlen(cmd) + strlen(" 2>&1 | \"%s\" --null") + 2 * strlen(programExecutablePath)];
    sprintf(command, "%s 2>&1 | \"%s\" --null", cmd, programExecutablePath);

    // execute the command
    int out = system(command);

    // return success or error
    return out == 0;
}

/**
 * Compile the generated C code.
 */
void compile(char* filePath) {
    // generate C file path
    cFilePath = generateCFilePath(filePath);

    // generate executable file path
    exeFilePath = generateExecutableFilePath(filePath);

    // test if gcc is installed
    if (!executeCommand("gcc --version")) {
        fprintf(stderr, "The C compiler \"gcc\" was not found. GCC is required for compilation.\nIf GCC is installed please check if it is properly added to path.\n");
        exit(1);
    }

    // build command string
    char command[strlen("gcc \"%s\" -o \"%s\"") + strlen(cFilePath) + strlen(exeFilePath)];
    sprintf(command, "gcc \"%s\" -o \"%s\"", cFilePath, exeFilePath);

    // compile the program
    int commandOut = executeCommand(command);

    // free cFilePath
    free(cFilePath);
    cFilePath = NULL;

    // free exeFilePath
    free(exeFilePath);
    exeFilePath = NULL;

    // build failed
    if (!commandOut) {
        fprintf(stderr, "Failed to compile program.");
        exit(1);
    }
}

/**
 * Compare two strings for equality, case sensitive.
 */
int equals(const char* str1, const char* str2) {
    return strcmp(str1, str2) == 0;
}

/**
 * Compare two strings for equality, ignoring their cases.
 */
int equalsIgnoreCase(const char* str1, const char* str2) {
    for (int i = 0; ; i++) {
        if (str1[i] == '\0' && str2[i] == '\0') {
            break;
        }
        else if (tolower(str1[i]) != tolower(str2[i])) {
            return 0;
        }
    }
    return 1;
}

/**
 * Check if first string ends with the second string.
 */
int endsWithIgnoreCase(const char *str, const char *suffix) {
    if (!str || !suffix) {
        return 0;
    }
    size_t lenstr = strlen(str);
    size_t lensuffix = strlen(suffix);
    if (lensuffix >  lenstr) {
        return 0;
    }
    return equalsIgnoreCase(str + lenstr - lensuffix, suffix);
}

/**
 * Free all resources to prevent memory leaks.
 */
void clean() {
    // free source
    if (source != NULL) {
        free(source);
        source = NULL;
    }

    // free jumps
    if (jumps != NULL) {
        free(jumps);
        jumps = NULL;
    }

    // free tape
    if (tape != NULL) {
        free(tape);
        tape = NULL;
    }

    // free stack
    if (stack != NULL) {
        stackFree(stack);
        stack = NULL;
    }

    // clean translator
    cleanupTranslator();
}

/**
 * Print help message.
 */
void printHelp() {
    printf("Description:\n");
    printf("    A fast Brainfuck interpreter and compiler written in C by Pratanu Mandal\n");
    printf("    https://github.com/prat-man/Brainfuck\n\n");

    printf("Version:\n");
    printf("    %s\n\n", VERSION);

    printf("Usage:\n");
    printf("    brainfuck [options] <source file path>\n\n");

    printf("Options:\n");
    printf("    -c\n");
    printf("    --compile     Translate to C and compile to machine code [requires GCC]\n\n");
    printf("    -x\n");
    printf("    --translate   Translate to C but do not compile\n\n");
    printf("    -t\n");
    printf("    --tape        Size of interpreter tape [must be equal to or above %d]\n\n", MIN_TAPE_SIZE);
    printf("    -s\n");
    printf("    --stack       Size of interpreter stack [must be equal to or above %d]\n\n", MIN_STACK_SIZE);
    printf("    -v\n");
    printf("    --version     Show product version and exit\n\n");
    printf("    -i\n");
    printf("    --info        Show product information and exit\n\n");
    printf("    -h\n");
    printf("    --help        Show this help message and exit\n\n");
}

/**
 * Main entry point to the program.
 */
int main(int argc, char** argv) {

    // store program executable path for future reference
    programExecutablePath = argv[0];

    // by default, execute
    int compileFlag = 0, translateFlag = 0;

    // variable to extract and store source file path from command line arguments
    char* path = NULL;

    // extract parameters and source file path from command line arguments
    for (int i = 1; i < argc; i++) {
        // consume all
        // this is for silent command execution
        if (equals(argv[i], "--null")) {
            exit(0);
        }

        // check if help message is to be displayed
        else if (equals(argv[i], "-h") || equals(argv[i], "--help")) {
            printf("\n");
            printHelp();
            exit(0);
        }

        // check if version is to be displayed
        else if (equals(argv[i], "-v") || equals(argv[i], "--version")) {
            printf("%s\n", VERSION);
            exit(0);
        }

        // check if informtion is to be displayed
        else if (equals(argv[i], "-i") || equals(argv[i], "--info")) {
            printf("brainfuck %s\n", VERSION);
            printf("A fast Brainfuck interpreter and compiler written in C by Pratanu Mandal\n");
            printf("https://github.com/prat-man/Brainfuck\n");
            exit(0);
        }

        // check if it is to be translated to C and compiled to machine code
        else if (equals(argv[i], "-c") || equals(argv[i], "--compile")) {
            compileFlag = 1;
        }

        // check if it is to be translated to C
        else if (equals(argv[i], "-x") || equals(argv[i], "--translate")) {
            translateFlag = 1;
        }

        // check if tape size is to be customized
        else if (equals(argv[i], "-t") || equals(argv[i], "--tape")) {
            int tapeSz = 0;
            if (i + 1 < argc) {
                tapeSz = atoi(argv[++i]);
            }
            if (tapeSz >= MIN_TAPE_SIZE) {
                TAPE_SIZE = tapeSz;
            }
            else {
                fprintf(stderr, "Invalid tape size [must be at least %d]\n\n", MIN_TAPE_SIZE);
                printHelp();
                exit(1);
            }
        }

        // check if stack size is to be changed
        else if (equals(argv[i], "-s") || equals(argv[i], "--stack")) {
            int stackSz = 0;
            if (i + 1 < argc) {
                stackSz = atoi(argv[++i]);
            }
            if (stackSz >= MIN_STACK_SIZE) {
                STACK_SIZE = stackSz;
            }
            else {
                fprintf(stderr, "Invalid stack size [must be at least %d]\n\n", MIN_STACK_SIZE);
                printHelp();
                exit(1);
            }
        }

        // get the path to source file (only once)
        else if (path == NULL) {

            path = argv[i];
        }

        // unknown parameter, display error
        else {
            fprintf(stderr, "Unknown paramter: %s\n\n", argv[i]);
            printHelp();
            exit(1);
        }
    }

    // check if path to source file is present
    if (path == NULL) {
        if (argc > 1) {
            fprintf(stderr, "Path to source file not provided\n\n");
            printHelp();
            exit(1);
        }
        else {
            printf("\n");
            printHelp();
            exit(0);
        }
    }

    // check if filename is standards compliant
    if (!endsWithIgnoreCase(path, ".bf")) {
        fprintf(stderr, "Invalid file name: %s\n", path);
        fprintf(stderr, "File format not recognized [must end with \".bf\"]\n");
        exit(1);
    }

    // clean before exit
    atexit(clean);

    // compile, translate, or execute
    if (compileFlag) {
        // translate the brainfuck code to C
        translate(path);
        // compile the translated C code
        compile(path);
    }
    else if (translateFlag) {
        // translate the brainfuck code to C
        translate(path);
    }
    else {
        // execute the brainfuck code
        execute(path);
    }

    // execution is successful, return success code
    return 0;
}
