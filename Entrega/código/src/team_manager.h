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
bool race_going;

void team_manager(int team_index);
pthread_t* create_threads_array();
car_struct* create_car_structs_array();
float laps_from_fuel(car_struct *car_info);
void init_car_stats(car_struct* stats, int team_nr, int car_nr);
void* car_worker(void* stats);
void terminate_cars_exit(int sig);
void swap_race_state(int sig);
void end_car_race(int sig);
