#include "team_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

pthread_t car_threads[0];

void team_manager(int n_carros){
	printf("Equipa %d\n", n_carros);
	
	exit(0);
}
