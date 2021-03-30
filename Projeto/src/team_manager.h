#ifndef TEAM_H
#define TEAM_H

#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "Simulator.h"
#include "car.h"
#include "config_file.h"
#include "write_output.h"

// struct with car stats
typedef struct {
  int num;  // number of the car
  float speed;
  float realiability; 
  float consuption;
  char state;  // 'R' = race; 'S' = safety; 'B' = box; 'D' = DNF; 'F' = finished;
  float fuel;
  int laps_done;     // total of laps completed
  int lap_distance;  // distance on lap; 0 - LAP_DIST; 0 = start/finish line
  //int track_position; // 1st, 2nd, 3rd, 4th, ...
  int box_count;
} car_struct;

void team_manager();
pthread_t* create_threads_array();
car_struct* create_car_structs_array();
void init_car_stats(car_struct* stats, int num);

#endif
