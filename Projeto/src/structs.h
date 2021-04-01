#ifndef STRUCTS_H
#define STRUCTS_H

typedef struct {
	char team_name[31];
	int number;
	float speed;
	float consumption;
	float reliability;
	int laps_completed;
	int boxs_stops_counter;
} car_shm_struct;

typedef struct{
	int refill_counter;
	char box_state[]; //
} shm_boxes;

typedef struct {
	int malfunctions_counter;
	car_shm_struct cars[];
} shm_struct;

// struct with car stats
typedef struct {
  car_shm_struct *car;
  char state; // 'R' = race; 'S' = safety; 'B' = box; 'D' = DNF; 'F' = finished;
  float fuel; // remaining fuel
  float lap_distance; // distance on lap; 0 - LAP_DIST; 0 = start/finish line
  //int track_position; // 1st, 2nd, 3rd, 4th, ...
} car_struct;

#endif
