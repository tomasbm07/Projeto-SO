#ifndef CONFIG_H
#define CONFIG_H

#include "write_output.h"
#include "Race_Simulator.h"

#include <stdio.h>
#include <regex.h>
#include <stdlib.h>


int nr_unid_tps,
	dist_volta,
	nr_voltas,
	nr_equipas,
	nr_carros,
	unid_temp_avaria,
	min_reparacao,
	max_reparacao,
	deposito;

// Functions in config_file.c
/*------------------------------------------*/
void file_error(void);
void read_file(char* filename);
void read_one_integer(char* string, int** int_value, FILE* file);
void read_two_integer(char* string, int** int_value_1, int** int_value_2, FILE* file);
/*------------------------------------------*/


#endif
