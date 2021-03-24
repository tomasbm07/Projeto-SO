#ifndef CONFIG_H
#define CONFIG_H

#include "write_output.h"
#include "Race_Simulator.h"

#include <stdio.h>
#include <regex.h>
#include <stdlib.h>


int NR_UNI_PS,
	LAP_DIST,
	LAP_NR,
	TEAM_NR,
	CARS_NR,
	MALFUNCTION_UNI_NR,
	MIN_REP,
	MAX_REP,
	FUEL_CAPACITY;

// Functions in config_file.c
/*------------------------------------------*/
void file_error(void);
void read_file(char* filename);
void read_one_integer(char* string, int** int_value, FILE* file);
void read_two_integer(char* string, int** int_value_1, int** int_value_2, FILE* file);
/*------------------------------------------*/


#endif
