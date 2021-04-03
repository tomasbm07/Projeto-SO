/*
Tomás Mendes - 2019232272
Joel Oliveira - 2019227468
*/

#ifndef OUTPUT_H
#define OUTPUT_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <semaphore.h>
#include <unistd.h>
#include <pthread.h>

#include "Simulator.h"

extern FILE *f;

void write_log(char* line);

#endif
