#ifndef __COMMON_INCLUDED__
#define __COMMON_INCLUDED__

#include <stdio.h>

int extn(unsigned char n);
int extw(unsigned int b);
int extl(unsigned short w);

void panic(const char *string);

unsigned short fgetw(FILE *fp);
void fputw(unsigned short s, FILE *fp);

#endif
