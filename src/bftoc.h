#ifndef BFTOC_H
#define BFTOC_H

static int TAPE_SIZE;

int* jumps;

int filePointer;

unsigned char* tape;

int pointer;

int findZeroLeft(int position);

int findZeroRight(int position);

char* cCode;

int cPointer;

FILE* cFile;

static inline void doTranslate(char ch);

#endif // BFTOC_H
