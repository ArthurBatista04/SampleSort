#ifndef SEQUENTIAL_ALGORITHM_H
#define SEQUENTIAL_ALGORITHM_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include <unistd.h>

#define TRUE 1
#define FALSE 0

int cmpfunc(const void *a, const void *b);
int *init_vector(int size);

#endif