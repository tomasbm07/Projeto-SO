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
	sigaction(SIGTERM, &sa_malf, NULL);
	pause();

	exit(0);
}

void generator(){
	write_log("[Malfunction Manager] Got SIGUSR2");
	srand((unsigned) getpid());
	malfunction_msg msg;
	int i, j, num, is = 1;
	while(1){
		for (i = 0; i < NR_TEAM; i++){
			for (j = 0; j < NR_CARS; j++){
			num = rand() % 100 + 1;
				//há algum problema com a escrita das reliabities, acho.. mas amanhã vejo melhor
				if (is)printf("CAR reliability: %d\n", shm_info->cars[i*NR_CARS + j].reliability);
				if (strcmp(shm_info->cars[i*NR_CARS + j].team_name, "") != 0) {
					if (num > shm_info->cars[i].reliability){			
						msg.car_index = (long) (i*NR_CARS + j + 1);						
						msgsnd(mqid, &msg, 0, 0);
					}
				}
			}
		}
		usleep(MALFUNCTION_UNI_NR * 1000000/NR_UNI_PS);
		is = 0;
	}
}

void create_mq(){
	mqid = msgget(IPC_PRIVATE, IPC_CREAT|0777);
  	if (mqid < 0){
    	write_log("Error creating message queue");
      	exit(-1);
    }
}

void cleanup(){
	msgctl(mqid, IPC_RMID, 0);
	exit(0);
}

void malf_term_handler(int sig){
	write_log("[Malfunction Manager] Got SIGTERM");
	cleanup();
	exit(0);
}
