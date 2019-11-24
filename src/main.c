#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>
#include "stack.h"
#include "bftoc.c"

#define VERSION "1.1"

#define NO_JUMP         INT_MAX
#define SET_ZERO        (INT_MAX - 1)
#define SCAN_ZERO_LEFT  (INT_MAX - 2)
#define SCAN_ZERO_RIGHT (INT_MAX - 3)

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
                else {
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
                else {
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

/**
 * Free all resources to prevent memory leaks.
 */
void clean() {
    free(source);
    free(jumps);
    free(tape);
}

/**
 * Execute the brainfuck source code.
 */
void execute(char* filePath) {
    // clean before exit
    atexit(clean);

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
 * Compare two strings for equality, ignoring their cases.
 */
int equalsIgnoreCase(char* str1, char* str2) {
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
    printf("    -t\n");
    printf("    -tape       Size of interpreter tape [must be equal to or above 10000]\n\n");
    printf("    -s\n");
    printf("    -stack      Size of interpreter stack [must be equal to or above 1000]\n\n");
    printf("    -v\n");
    printf("    -version    Show product version and exit\n\n");
    printf("    -i\n");
    printf("    -info       Show product information and exit\n\n");
    printf("    -h\n");
    printf("    -help       Show this help message and exit\n\n");
}

/**
 * Main entry point to the interpreter.
 */
int main(int argc, char** argv) {

    // variable to extract and store source file path from command line arguments
    char* path = NULL;

    // extract parameters and source file path from command line arguments
    for (int i = 1; i < argc; i++) {
        // check if help message is to be displayed
        if (equalsIgnoreCase(argv[i], "-h") || equalsIgnoreCase(argv[i], "-help")) {
            printf("\n");
            printHelp();
            exit(0);
        }

        // check if version is to be displayed
        else if (equalsIgnoreCase(argv[i], "-v") || equalsIgnoreCase(argv[i], "-version")) {
            printf("%s\n", VERSION);
            exit(0);
        }

        // check if informtion is to be displayed
        else if (equalsIgnoreCase(argv[i], "-i") || equalsIgnoreCase(argv[i], "-info")) {
            printf("brainfuck %s\n", VERSION);
            printf("A fast Brainfuck interpreter written in C by Pratanu Mandal\n");
            printf("https://github.com/prat-man/Brainfuck\n");
            exit(0);
        }

        // check if tape size is to be customized
        else if (equalsIgnoreCase(argv[i], "-t") || equalsIgnoreCase(argv[i], "-tape")) {
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
        else if (equalsIgnoreCase(argv[i], "-s") || equalsIgnoreCase(argv[i], "-stack")) {
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
            fprintf(stderr, "Invalid paramter: %s\n\n", argv[i]);
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

    // execute the brainfuck code
    execute(path);

    // execution is successful, return success code
    return 0;
}
