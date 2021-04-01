#ifndef TEAM_H
#define TEAM_H

#endif

#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "structs.h"
#include "config_file.h"
#include "write_output.h"

#define DEBUG

shm_struct *shm_info; //extern ?
shm_boxes *shm_boxes_state; //extern ?
int NR_CARS, NR_TEAM; // extern ?
void team_manager();
pthread_t* create_threads_array();
car_struct* create_car_structs_array();
void init_car_stats(car_struct* stats, int team_nr, int car_nr);
void* car_worker(void* stats);


