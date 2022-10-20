#ifndef _FILES_H_
#define _FILES_H_
#include <stdio.h>


int getFileSize(FILE *fp);

FILE* openFile(char* fileName, char* mode);

int closeFile(FILE* fp);

#endif