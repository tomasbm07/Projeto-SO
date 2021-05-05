/*
Tomás Mendes - 2019232272
Joel Oliveira - 2019227468
*/

#ifndef TEAM_H
#define TEAM_H

#endif

#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "structs.h"
#include "config_file.h"
#include "write_output.h"
#include "Simulator.h"


shm_struct *shm_info;  
int NR_CARS, NR_TEAM; 

void team_manager(int team_index);
pthread_t* create_threads_array();
car_struct* create_car_structs_array();
void init_car_stats(car_struct* stats, sigset_t *set, int team_nr, int car_nr);
void* car_worker(void* stats);
void start_race(int sig);
