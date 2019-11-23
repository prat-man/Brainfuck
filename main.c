#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "stack.h"
#include "memrchr.h"

#define TAPE_SIZE 30000
#define STACK_SIZE 1000

#define NO_JUMP         INT_MAX
#define SET_ZERO        (INT_MAX - 1)
#define SCAN_ZERO_LEFT  (INT_MAX - 2)
#define SCAN_ZERO_RIGHT (INT_MAX - 3)

char* source = NULL;
int* jumps = NULL;
int filePointer = 0;
int fileSize;

unsigned char* tape;
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
                fprintf(stderr, "Error reading source file!\n");
                exit(1);
            }

            // allocate our buffer to that size
            source = (char*) malloc(sizeof(char) * (bufsize + 1));

            // go back to the start of the file
            if (fseek(fp, 0L, SEEK_SET) != 0) {
                // display error message and exit
                fprintf(stderr, "Error reading source file!\n");
                exit(1);
            }

            // read the entire file into memory
            size_t newLen = fileSize = fread(source, sizeof(char), bufsize, fp);
            if (ferror(fp) != 0) {
                // display error message and exit
                fprintf(stderr, "Error reading source file!\n");
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
        fprintf(stderr, "Source file not found!\n");
        exit(1);
    }
}

/**
 * Initialize the jumps in the file for optimization.
 * Jumps between [ and ].
 * Compacts and jumps consecutive > and <.
 * Compacts and jumps consecutive + and -.
 * Optimizes for [-] to set(0).
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
                // optimize [<] to scanl(0)
                jumps[i] = SCAN_ZERO_LEFT;
                i += 2;
            }
            else if (ch2 == '>' && ch3 == ']') {
                // optimize [>] to scanr(0)
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
 * Unread specified number of character from the source file.
 */
/*void unreadChars(int charNums) {
    if (filePointer - charNums < 0) {
        fprintf(stderr, "File pointer underflow!\n");
        exit(1);
    }
    filePointer -= charNums;
}*/

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
inline void doOperation(char ch) {
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
        if (flag == SET_ZERO) {
            tape[pointer] = 0;
            filePointer += 2;
        }
        else if (flag == SCAN_ZERO_LEFT) {
            pointer = findZeroLeft(pointer);
            filePointer += 2;
        }
        else if (flag == SCAN_ZERO_RIGHT) {
            pointer = findZeroRight(pointer);
            filePointer += 2;
        }
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
 * Main entry point to the interpreter.
 */
int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: brainfuck [source file path]\n");
    }
    else {
        execute(argv[1]);
    }
    return 0;
}
