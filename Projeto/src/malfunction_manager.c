/*
Tomás Mendes - 2019232272
Joel Oliveira - 2019227468
*/

#include "malfunction_manager.h"


void malfunction_manager(){
	signal(SIGTSTP, SIG_IGN);
	//Create Message queue
	//create_mq();

	malfunction_msg msg;

	//fill the mq, just for testing
	for (int i = 0; i < 10; i++) {
		printf("messaged added to MQ\n");
		msg.car_num = i;
		//msgsnd(mqid, &msg, sizeof(malfunction_msg), 0);
	}
	
	
	sigset_t set;

	sigemptyset(&set);
	signal(SIGUSR2, malfunction_signal_handler);
	signal(SIGINT, malfunction_signal_handler);

	sigaddset(&set, SIGUSR2);
	//sigwait(&set);
	

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

	} else if (sig == SIGINT){
		cleanup();
		exit(0);
	}

}
