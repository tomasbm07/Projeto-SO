#include "race_manager.h"



void race_manager(){
	int i;
	
	static pthread_t car_threads[TEAM_NR*CARS_NR];
	
	for (i = 0; i < TEAM_NR; i++){
		if ( !fork() ) team_manager(i);
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
