/*
Tomás Mendes - 2019232272
Joel Oliveira - 2019227468
*/

#include "malfunction_manager.h"


void malfunction_manager(){
	//ignora sinais a serem recebidos pelo race_simulator
	signal(SIGTSTP, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	
	//Create Message queue
	create_mq();
	
	signal(SIGUSR2, generator);
	
	struct sigaction sa_malf;
	sa_malf.sa_handler = malf_term_handler;
	sigemptyset(&sa_malf.sa_mask);
	sigaddset(&sa_malf.sa_mask, SIGTERM);
	sigaddset(&sa_malf.sa_mask, SIGUSR2);
	sigaction(SIGTERM, &sa_malf, NULL);
	
	//printf("MALF PGID: %ld\n", (long)getpgid( getpid() ));
	pause();

	msgctl(mqid, IPC_RMID, 0);
	exit(0);
}


void generator(){
	#ifdef DEBUG
	write_log("[Malfunction Manager] Got SIGUSR2 -> Starting");
	#endif
	srand(time(NULL));
	malfunction_msg msg;
	int i, j, num;
	while(1){
		for (i = 0; i < NR_TEAM; i++){
			for (j = 0; j < NR_CARS; j++){
				num = rand() % 100 + 1;
				if (strcmp(shm_info->cars[i*NR_CARS + j].team_name, "") != 0) {
					if (num > shm_info->cars[i*NR_CARS + j].reliability){			
						msg.car_index = (long) (i*NR_CARS + j + 1);						
						msgsnd(mqid, &msg, 0, 0);
					}
				}
			}
		}
		usleep(MALFUNCTION_UNI_NR * 1000000/NR_UNI_PS);
	}
}


void create_mq(){
	mqid = msgget(ftok(".", 25), IPC_CREAT|0777);
  	if (mqid < 0){
    	write_log("Error creating message queue");
		msgctl(mqid, IPC_RMID, 0);
      	exit(-1);
    }
    //printf("MSQID: %d\n", mqid);
}


void malf_term_handler(int sig){
	write_log("[Malfunction Manager] Got SIGTERM");
	msgctl(mqid, IPC_RMID, 0);
	exit(0);
}
