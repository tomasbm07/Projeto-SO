#include "team_manager.h"

 //TODO alterar esta cena
// n gosta de nr_equipas * nr_carros por serem int e n constantes

void team_manager(int team_nr){
	int i;
	//create car threads
	for (i = 0; i < CARS_NR; i++) pthread_create(&car_threads[team_nr + i], NULL, car, NULL);

	//wait for threads to finish
	for (i = 0; i < CARS_NR; i++) pthread_join(car_threads[team_nr + i], NULL);
	
	exit(0);
}
