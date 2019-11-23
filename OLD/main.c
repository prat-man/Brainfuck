#include <stdio.h>
#include <stdlib.h>

#define TAPE_SIZE 30000

char* source = NULL;
int filePointer = 0;
int fileSize;

unsigned char tape[TAPE_SIZE];
int pointer = 0;

void loadFile(char* filePath) {
    FILE* fp = fopen(filePath, "r");

    if (fp != NULL) {
        /* Go to the end of the file. */
        if (fseek(fp, 0L, SEEK_END) == 0) {
            /* Get the size of the file. */
            long bufsize = ftell(fp);
            if (bufsize == -1) { /* Error */ }

            /* Allocate our buffer to that size. */
            source = malloc(sizeof(char) * (bufsize + 1));

            /* Go back to the start of the file. */
            if (fseek(fp, 0L, SEEK_SET) != 0) { /* Error */ }

            /* Read the entire file into memory. */
            size_t newLen = fileSize = fread(source, sizeof(char), bufsize, fp);
            if (ferror( fp ) != 0) {
                fputs("Error reading file", stderr);
            } else {
                source[newLen++] = '\0'; /* Just to be safe. */
            }
        }

        fclose(fp);
    }
    else {
        printf("File not found!\n");
        exit(1);
    }
}

char readChar() {
    if (filePointer == fileSize) {
        return -1;
    }
    return source[filePointer++];
}

void unreadChars(int numChars) {
    if (filePointer - numChars < 0) {
        printf("Negative index for file read\n");
        exit(1);
    }
    filePointer -= numChars;
}

void doOperation(char ch) {
    if (ch == '>') {
        pointer++;
        if (pointer == TAPE_SIZE) {
            pointer = 0;
        }
    }
    else if (ch == '<') {
        if (pointer == 0) {
            pointer = TAPE_SIZE - 1;
        }
        else {
            pointer--;
        }
    }
    else if (ch == '+') {
        tape[pointer]++;
    }
    else if (ch == '-') {
        tape[pointer]--;
    }
    else if (ch == '.') {
        printf("%c", tape[pointer]);
    }
    else if (ch == ',') {
        tape[pointer] = getchar();
    }
    else if (ch == '[') {
        if (tape[pointer] == 0) {
            int balance = 1;
            char ch2;
            do {
                ch2 = readChar();
                if (ch2 == '[') {
                    balance++;
                }
                else if (ch2 == ']') {
                    balance--;
                }
            } while (ch2 != ']' || balance != 0);
        }
    }
    else if (ch == ']') {
        if (tape[pointer] != 0) {
            int balance = -1;
            char ch2;
            do {
                unreadChars(2);
                ch2 = readChar();
                if (ch2 == '[') {
                    balance++;
                }
                else if (ch2 == ']') {
                    balance--;
                }
            } while (ch2 != '[' || balance != 0);
        }
    }
}

void execute(char* filePath) {
    loadFile(filePath);

    char ch;
    while ((ch = readChar()) != -1) {
        doOperation(ch);
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: brainfuck [source file path]");
    }
    else {
        execute(argv[1]);
    }
}
