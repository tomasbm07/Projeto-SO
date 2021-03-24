#include "race_manager.h"

/*
void create_threads_array(){
	static pthread_t car_threads[CARS_NR*TEAM_NR];
}
*/

void race_manager(){
	int i;
	
	//create_threads_array();
	
	for (i = 0; i < TEAM_NR; i++){
		if ( !fork() ) team_manager();
	}

	#ifdef DEBUG
		char str[50];
		sprintf(str, "Created %d team processes", i);
		write_log(str);
	#endif

	//wait for all team processes to finish
	for (i = 0; i < TEAM_NR; i++) wait(NULL);
	
	exit(0);
}
