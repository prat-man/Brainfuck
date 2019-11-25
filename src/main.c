#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>
#include <stddef.h>

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

// size of source file in chars
int fileSize;

// pointer to next character to be read from file
int filePointer = 0;

// jumps and flags are preinitialized for optimized execution
int* jumps = NULL;

// the brainfuck tape - 0-255 - circular
unsigned char* tape;

// pointer to current location in tape
int pointer = 0;

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
    // initialize jumps
    jumps = (int*) malloc(sizeof(int) * (fileSize));

    // create a stack for [ operators
    Stack* stack = stackCreate(STACK_SIZE);

    // find jumps to optimize code
    for (int i = 0; i < fileSize; i++) {
        // get one character
        char ch = source[i];

        // create jumps for opening and closing square brackets [ and ]
        if (ch == '[') {
            char ch2 = i + 1 < fileSize ? source[i + 1] : -1;
            char ch3 = i + 2 < fileSize ? source[i + 2] : -1;
            if (ch2 == '-' && ch3 == ']') {
                // optimize [-] to set(0)
                jumps[i] = SET_ZERO;
                i += 2;
            }
            else if (ch2 == '<' && ch3 == ']') {
                // optimize [<] to scan_left(0)
                jumps[i] = SCAN_ZERO_LEFT;
                i += 2;
            }
            else if (ch2 == '>' && ch3 == ']') {
                // optimize [>] to scan_right(0)
                jumps[i] = SCAN_ZERO_RIGHT;
                i += 2;
            }
            else {
                stackPush(stack, i);
            }
        }
        else if (ch == ']') {
            int x = stackPop(stack);
            jumps[x] = i;
            jumps[i] = x;
        }

        // compact and jump for > and <
        else if (ch == '>' || ch == '<') {
            int sum = 0;

            if (ch == '>') sum++;
            else sum--;

            int index = i;
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
            if (index == i) {
                jumps[index] = NO_JUMP;
            }
            else {
                jumps[index] = i;
                jumps[i] = sum;
            }
        }

        // compact and jump for + and -
        else if (ch == '+' || ch == '-') {
            int sum = 0;

            if (ch == '+') sum++;
            else sum--;

            int index = i;
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
            if (index == i) {
                jumps[index] = NO_JUMP;
            }
            else {
                jumps[index] = i;
                jumps[i] = sum;
            }
        }

        // everything else, no jumps
        else {
            jumps[i] = 0;
        }
    }

    // loops are unmatched
    if (!stackEmpty(stack)) {
        // free stack
        stackFree(stack);

        // display error message and exit
        fprintf(stderr, "Unmatched loops!");
        exit(1);
    }

    // free stack
    stackFree(stack);
}

/**
 * Read a single character from the source file.
 */
char readChar() {
    if (filePointer == fileSize) {
        return -1;
    }
    return source[filePointer++];
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
 * Free all resources to prevent memory leaks.
 */
void clean() {
    free(source);
    free(jumps);
    free(tape);
    cleanupTranslator();
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

int can_run_command(const char *command) {
    const char *head;

    int length = strlen(command);
    // Get the PATH environment variable
    head = getenv("PATH");
    if (head == NULL) {
        fprintf(stderr, "the PATH variable was not set, what?\n");
        return -1;
    }
    // Find the first separator
    while (*head != '\0') {
        struct stat st;
        ptrdiff_t dirlen;
        const char *tail;
        char *path;
        // Check for the next ':' if it's not found
        // then get a pointer to the null terminator
        tail = strchr(head, ':');
        if (tail == NULL)
            tail = strchr(head, '\0');
        // Get the length of the string between the head
        // and the ':'
        dirlen = tail - head;
        // Allocate space for the new string
        path = malloc(length + dirlen + 2);
        if (path == NULL)
            return -1;
        // Copy the directory path into the newly
        // allocated space
        memcpy(path, head, dirlen);
        // Append the directory separator
        path[dirlen] = '/';
        // Copy the name of the command
        memcpy(path + dirlen + 1, command, length);
        // `null' terminate please
        path[dirlen + length + 1] = '\0';
        // Check if the file exists and whether it's
        // executable
        if ((stat(path, &st) != -1) && (S_ISREG(st.st_mode) != 0)) {
            if ((st.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)) == 0) {
                fprintf(stdout, "found `%s' but it's not executable\n", path);
            } else {
                fprintf(stdout, "found at: %s\n", path);
            }
        }
        // Don't forget to free!
        free(path);
        // Point to the next directory
        head = tail + 1;
    }
    return 0;
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
    if (!can_run_command("gcc")) {
        fprintf(stderr, "gcc not found. gcc is required.\n");
        exit(1);
    }

    // build command string
    char command[strlen("gcc \"%s\" -o \"%s\"") + strlen(cFilePath) + strlen(exeFilePath)];
    sprintf(command, "gcc \"%s\" -o \"%s\"", cFilePath, exeFilePath);

    // compile the program
    int commandOut = system(command);

    // free file paths
    free(cFilePath);
    free(exeFilePath);

    // build failed
    if (commandOut != 0) {
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
 * Print help message.
 */
void printHelp() {
    printf("Description:\n");
    printf("    A fast Brainfuck interpreter written in C by Pratanu Mandal\n");
    printf("    https://github.com/prat-man/Brainfuck\n\n");

    printf("Version:\n");
    printf("    %s\n\n", VERSION);

    printf("Usage:\n");
    printf("    brainfuck [options] <source file path>\n\n");

    printf("Options:\n");
    printf("    -c\n");
    printf("    --compile     Translate to C and compile to machine code [requires gcc]\n\n");
    printf("    -x\n");
    printf("    --translate   Translate to C but do not compile\n\n");
    printf("    -t\n");
    printf("    --tape        Size of interpreter tape [must be equal to or above 10000]\n\n");
    printf("    -s\n");
    printf("    --stack       Size of interpreter stack [must be equal to or above 1000]\n\n");
    printf("    -v\n");
    printf("    --version     Show product version and exit\n\n");
    printf("    -i\n");
    printf("    --info        Show product information and exit\n\n");
    printf("    -h\n");
    printf("    --help        Show this help message and exit\n\n");
}

/**
 * Main entry point to the interpreter.
 */
int main(int argc, char** argv) {

    // by default, execute
    int compileFlag = 0, translateFlag = 0;

    // variable to extract and store source file path from command line arguments
    char* path = NULL;

    // extract parameters and source file path from command line arguments
    for (int i = 1; i < argc; i++) {
        // check if help message is to be displayed
        if (equals(argv[i], "-h") || equals(argv[i], "--help")) {
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
            printf("A fast Brainfuck interpreter written in C by Pratanu Mandal\n");
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
            if (tapeSz >= 10000) {
                TAPE_SIZE = tapeSz;
            }
            else {
                fprintf(stderr, "Invalid tape size [must be at least 10000]\n\n");
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
            if (stackSz >= 1000) {
                STACK_SIZE = stackSz;
            }
            else {
                fprintf(stderr, "Invalid stack size [must be at least 1000]\n\n");
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
