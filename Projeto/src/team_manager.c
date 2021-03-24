#include "team_manager.h"



pthread_t car_threads[NR_EQUIPAS * 2]; //TODO alterar esta cen
// n gosta de nr_equipas * nr_carros por serem int e n constantes

void team_manager(){
	int i;

	//create car threads
	for (i = 0; i < nr_carros; i++){
    	pthread_create(&car_threads[i], NULL, car, NULL);
	}

	//wait for threads to finish
	for (i = 0; i < NR_EQUIPAS*2; i++){
		pthread_join(car_threads[i], NULL);
	}
	
	exit(0);
}
