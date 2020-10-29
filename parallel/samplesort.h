#ifndef PARALLEL_ALGORITHM_H
#define PARALLEL_ALGORITHM_H
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include "mpi.h"
#define MASTER 0      /* taskid of first task */
#define FROM_MASTER 1 /* setting a message type */
#define FROM_WORKER 2 /* setting a message type */
#define TRUE 1
#define FALSE 0

int cmpfunc(const void *a, const void *b);
int *init_vector(int size);

#endif