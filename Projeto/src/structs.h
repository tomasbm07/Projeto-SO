/*
Tomás Mendes - 2019232272
Joel Oliveira - 2019227468
*/

#ifndef STRUCTS_H
#define STRUCTS_H

//part of car struct in shm
typedef struct {
	char team_name[32];
	int number;
	int speed;
	float consumption;
	int reliability;
	int laps_completed;
	int box_stops_counter;
} car_shm_struct;

typedef struct {
	int malfunctions_counter;
	int refill_counter;
	car_shm_struct cars[];
} shm_struct;

// struct with car stats
typedef struct {
  car_shm_struct *car;
  char state; // 'R' = race; 'S' = safety; 'B' = box; 'D' = DNF; 'F' = finished;
  float fuel; // remaining fuel
  float lap_distance; // distance on lap; 0 - LAP_DIST; 0 = start/finish line
  sigset_t *set;
} car_struct;

typedef struct{
	long mtype;
    int car_num;
} malfunction_msg;


#endif
