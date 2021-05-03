/*
Tomás Mendes - 2019232272
Joel Oliveira - 2019227468
*/

#include "malfunction_manager.h"


void malfunction_manager(){
	
	/*
	sigset_t set;

	sigemptyset(&set);

	signal(SIGUSR2, malfunction_signals);
	signal(SIGINT, malfunction_signals);

	sigaddset(&set, SIGUSR2);
	sigwait(&set);
	*/

	pause();


#ifdef DEBUG
	char str[50];
	sprintf(str,"Malfunction manager created (PID: %d)", getpid());
	write_log(str);
#endif

	printf("DONE");
	exit(0);
}

void malfunction_signals(int sig){
	if (sig == SIGUSR2){
		write_log("[Malfunction Manager] Got SIGUSR2");
		generator();
	} else { // SIGINT
		exit(0);
	}
}

void generator(){
	int i, reliability;

	for (i = 0; i < NR_TEAM * NR_CARS; i++){
		reliability = shm_info->cars[i].reliability;
		
		int num = rand() % reliability;
		printf("reliability = %d | num = %d", reliability, num);
		if (num == reliability - 1){
			char str[50];
			snprintf(str, sizeof(str),"Malfunction created for car %d", shm_info->cars[i].number);
			write_log(str);
		}
	}
}
