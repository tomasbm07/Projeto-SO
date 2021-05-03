/*
Tomás Mendes - 2019232272
Joel Oliveira - 2019227468
*/

#include "team_manager.h"

pthread_mutex_t box_mutex = PTHREAD_MUTEX_INITIALIZER;
char box_state;

void team_manager(int team_index) {
	team_index = team_index*NR_CARS;
	int i;
	char str[150];

#ifdef DEBUG
	char aux[50];
	sprintf(aux, "Team manager created (PID: %d)", getpid());
	write_log(aux);
#endif

  // fechar pipe de leitura
	close(fd_team[team_index][0]);

	for (int i = 0; i < NR_TEAM; i++) {
    	if (i == team_index) continue;
    	//fechar pipes das outras equipas
		else{
			close(fd_team[i][0]);
			close(fd_team[i][1]);
		}
  	}
  

  // int car_thread_index = 0;
	box_state = 'E';  // 'R' = Reserved; 'E' = Empty; 'F' = Full;
	srand((unsigned)team_index);
	pthread_t car_threads[NR_CARS];
	car_struct car_stats[NR_CARS];

	char pipe_str[50];
	sprintf(pipe_str, "Team %d Ready!", team_index);
	write(fd_team[team_index][1], &pipe_str, strlen(pipe_str)+1);

#ifdef DEBUG
	sprintf(str, "Team %d sent \"%s\" to unnamed pipe\n", team_index, pipe_str);
	write_log(str);
#endif

  
	for (i = 0; i < NR_CARS; i++) {
		if (strcmp(shm_info->cars[team_index + i].team_name,"")==0) break; 
		init_car_stats(&car_stats[i], team_index, i);
		pthread_create(&car_threads[i], NULL, car_worker, &car_stats[i]);
	}
  
  	// wait for threads to finish
  	for (i = 0; i < NR_CARS; i++) pthread_join(car_threads[i], NULL);

  	// destroy mutex
	pthread_mutex_destroy(&box_mutex);

  exit(0);
}

// allocates space for array with car threads
pthread_t *create_threads_array() {
	return (pthread_t *)malloc(sizeof(pthread_t) * NR_CARS);
}

// allocates space for array with car struct atributes
car_struct *create_car_structs_array() {
	return (car_struct *)malloc(sizeof(car_struct) * NR_CARS);
}

// set the atributes of the car that aren't set by race_manager
void init_car_stats(car_struct *stats, int team_index, int car_index) {
	stats->car = &shm_info->cars[team_index + car_index];

  	stats->state = 'R';
  	stats->fuel = FUEL_CAPACITY;
  	stats->lap_distance = 0;
}

//TODO: READ MESSAGE QUEUE MESSAGES AND DETERMINE WHAT TO DO
// function to run in car thread
void *car_worker(void *stats) {
  	// convert argument from void* to car_struct*
	car_struct *car_info = (car_struct *)stats;
	malfunction_msg msg;
	char states[3] = {'E', 'F', 'R'};
	char str[300];
/*
  // just for testing
#ifdef DEBUG
  	char str[300];
  	sprintf(str,
          "I am Thread %d, from process %d\n\t"
          "Team: %s | Car Number: %d | Fuel: %.2fL | Box State: %c",
          (unsigned int)pthread_self(), (unsigned int)getpid(),
          car_info->car->team_name, car_info->car->number, car_info->fuel,
          box_state);
  //write_log(str);
  	write_log("Thread car created!");

  	pthread_mutex_lock(&box_mutex);
  	//box_state = states[rand() % 3];
  	pthread_mutex_unlock(&box_mutex);
#endif
*/
	sprintf(str, "Car %d from team: %s created", car_info->car->number, car_info->car->team_name);
	write_log(str);

  	pthread_exit(NULL);
}
