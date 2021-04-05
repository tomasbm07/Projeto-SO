/*
Tomás Mendes - 2019232272
Joel Oliveira - 2019227468
*/

#ifndef CONFIG_H
#define CONFIG_H


#include "write_output.h"

#include <stdio.h>
#include <regex.h>
#include <stdlib.h>


int NR_UNI_PS,
	LAP_DIST,
	NR_LAP,
	NR_TEAM,
	NR_CARS,
	MALFUNCTION_UNI_NR,
	MIN_REP,
	MAX_REP,
	FUEL_CAPACITY;

void file_error(void);
void read_file(char* filename);
void read_one_integer(char* string, int** value, FILE* file);
void read_two_integer(char* string, int** value_1, int** value_2, FILE* file);


#endif