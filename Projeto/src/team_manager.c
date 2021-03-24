#include "team_manager.h"


pthread_t* create_threads_array(){
	return (pthread_t*) malloc(sizeof(pthread_t) * CARS_NR);
}

void team_manager(){
	int i;
	//create car threads
	pthread_t * car_threads = create_threads_array();
	
	for (i = 0; i < CARS_NR; i++) pthread_create((car_threads + i), NULL, car, NULL);

	//wait for threads to finish
	for (i = 0; i < CARS_NR; i++) pthread_join(*(car_threads+i), NULL);
	
	free(car_threads);
	exit(0);
}
