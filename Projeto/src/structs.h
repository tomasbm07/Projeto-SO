/*
Tomás Mendes - 2019232272
Joel Oliveira - 2019227468
*/
/*
Tomás Mendes - 2019232272
Joel Oliveira - 2019227468
*/
#ifndef STRUCTS_H
#define STRUCTS_H

//part of car struct in shm
typedef struct {
	char team_name[32]; // team of the car
	int number; // number of the car
	int speed; // defines how far it advances in a lap
	float consumption; // defines how much fuel is used every iteration
	int reliability; // defines percentage of a malfunction
	int laps_completed; // number of laps completed
	int box_stops_counter; // counter of stops 
	float lap_distance; // distance on lap; 0 - LAP_DIST; 0 = start/finish line
} car_shm_struct;


typedef struct {
	int malfunctions_counter; // number of malfunctions generated in total
	int refill_counter; // number of stops made
	int counter_cars_finished; // number of cars that have finished or quit the race
	int nr_cars;
	bool wait_statistics;
	car_shm_struct cars[];
} shm_struct;


// struct with car stats
typedef struct {
  car_shm_struct *car;
  int car_index; // car index on shm
  char state; // 'R' = race; 'S' = safety; 'B' = box; 'D' = DNF; 'F' = finished;
  float fuel; // remaining fuel
  sigset_t car_set;
} car_struct;


typedef struct{
	long car_index;
} malfunction_msg;


#endif
