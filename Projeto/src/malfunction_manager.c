/*
Tomás Mendes - 2019232272
Joel Oliveira - 2019227468
*/

#include "malfunction_manager.h"


void malfunction_manager(){
	signal(SIGTSTP, SIG_IGN);

	//Create Message queue
	create_mq();

	//fill the mq, just for testing
	/*for (int i = 0; i < 10; i++) {
		printf("messaged added to MQ\n");
		msg.car_num = i;
		//msgsnd(mqid, &msg, sizeof(malfunction_msg), 0);
	}*/

	signal(SIGUSR2, malfunction_signal_handler);
	signal(SIGTERM, malfunction_signal_handler);

	pause();

#ifdef DEBUG
	char str[50];
	sprintf(str,"Malfunction manager created (PID: %d)", getpid());
	write_log(str);
#endif

	printf("DONE\n");
	exit(0);
}

void generator(){
	malfunction_msg msg;
	int i, j, num;
	char str[50];
	while(1){
		for (i = 0; i < NR_TEAM; i++){
			for (j = 0; j < NR_CARS; j++){
			num = rand() % 100 + 1;
				if (strcmp(shm_info->cars[i*NR_CARS + j].team_name, "") != 0) {
					if (num > shm_info->cars[i].reliability){			
						msg.car_index = (long) (i*NR_CARS + j + 1);						
						msgsnd(mqid, &msg, sizeof(malfunction_msg) - sizeof(long), 0);
					}
				}
			}
		usleep(MALFUNCTION_UNI_NR * 1000000/NR_UNI_PS);
		}
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

void malfunction_signal_handler(int sig){
	if (sig == SIGUSR2){
		write_log("[Malfunction Manager] Got SIGUSR2");
		//generator();

	} else if (sig == SIGTERM){
		cleanup();
		exit(0);
	}

}
