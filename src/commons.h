#ifndef COMMONS_H
#define COMMONS_H

#define VERSION "1.1"

#define MIN_TAPE_SIZE 1000
#define MIN_STACK_SIZE 100

#define NO_JUMP         INT_MAX
#define SET_ZERO        (INT_MAX - 1)
#define SCAN_ZERO_LEFT  (INT_MAX - 2)
#define SCAN_ZERO_RIGHT (INT_MAX - 3)

#define isOperator(ch) (strchr("<>+-,.[]", ch) != NULL)

static int TAPE_SIZE;

static int STACK_SIZE;

char* programExecutablePath;

int* jumps;

int filePointer;

unsigned char* tape;

int pointer;

int findZeroLeft(int position);

int findZeroRight(int position);

#endif // COMMONS_H
