/*
Tomás Mendes - 2019232272
Joel Oliveira - 2019227468
*/

#include "team_manager.h"

pthread_mutex_t box_mutex = PTHREAD_MUTEX_INITIALIZER;
char box_state;
int cars_number;
pthread_t *car_threads;

void team_manager(int team_index) {
	team_index = team_index*NR_CARS;
	sigset_t set;
	int i, sig;
	char str[256];
	signal(SIGUSR1, SIG_DFL);
	sigemptyset(&set);
  	sigaddset(&set, SIGUSR2);
  	pthread_sigmask(SIG_BLOCK, &set, NULL);
  	
#ifdef DEBUG
	char aux[50];
	sprintf(aux, "\nTeam manager created (PID: %d), from Team %d", getpid(), team_index);
	write_log(aux);
#endif

  // fechar pipe de leitura
	close(fd_team[team_index][0]);

	for (int i = 0; i < NR_TEAM; i++) {
    	if (i != team_index){
    	//fechar pipes das outras equipas
			close(fd_team[i][0]);
			close(fd_team[i][1]);
		}
  	}
  
	box_state = 'E';  // 'R' = Reserved; 'E' = Empty; 'F' = Full;
	srand((unsigned)team_index);
	car_threads = create_threads_array();
	car_struct car_stats[NR_CARS];

	//create thread_cars;	
	for (i = 0; i < NR_CARS; i++) {
		if (strcmp(shm_info->cars[team_index + i].team_name,"")==0) break; 
		init_car_stats(&car_stats[i], &set, team_index, i);
		pthread_create((car_threads+i), NULL, car_worker, &car_stats[i]);
	}  	
  	cars_number = i;

  	sprintf(str, "Team %d Ready!", team_index);
	write(fd_team[team_index][1], &str, strlen(str)+1);	
	sigwait(&set, &sig);
	for (i = 0; i < cars_number; ){
		pthread_kill(car_threads[i++], SIGUSR1);
	}
  	// wait for threads to finish
  	for (i = 0; i < cars_number; i++) pthread_join(*(car_threads+i), NULL);

  	// destroy mutex
	pthread_mutex_destroy(&box_mutex);
	
	free(car_threads);
	
	close(fd_team[team_index][1]);
	
	write_log("TEam Manager finishing");
  exit(0);
}

// allocates space for array with car threads
pthread_t *create_threads_array() {
	return (pthread_t *)malloc(sizeof(pthread_t) * NR_CARS);
}

// allocates space for array with car struct atributes
car_struct *create_car_structs_array() {
	return (car_struct *)malloc(sizeof(car_struct) * NR_CARS);
}

// set the atributes of the car that aren't set by race_manager
void init_car_stats(car_struct *stats, sigset_t *set, int team_index, int car_index) {
	stats->car = &shm_info->cars[team_index + car_index];

  	stats->state = 'R';
  	stats->fuel = FUEL_CAPACITY;
  	stats->lap_distance = 0;
  	stats->set = set;
}

int randint(int min, int max){
	return (rand()%(max - min + 1)) + min;
}

//TODO: READ MESSAGE QUEUE MESSAGES AND DETERMINE WHAT TO DO
// function to run in car thread
void *car_worker(void *stats) {
  	// convert argument from void* to car_struct*
	car_struct *car_info = (car_struct *)stats;
	malfunction_msg msg;
	//char states[3] = {'E', 'F', 'R'};
	//char str[300];
	char enter_box = 'N' // Y = yes, tenta entrar. N = no , nao tenta;
	int start, steps[2] = {1,1};
	while( (start=pause()) != SIGUSR1 );
	
	while(1){
		
		//TODO iteracao, antes de começar, bloquear receção de sinais
		if (car_info->state == 'R'){
			step[0]=1;
			step[1]=1;
		}
		else if(car_info->state == 'S'){
			step[0] = 0.3;
			step[1] = 0.4;
		}
		
				
		car_info->lap_distance = (car_info->lap_distance+ step[0] * car_info->car->speed)%LAP_DIST;
		car_info->fuel -= step[1] * car_info->car->consumption;
			//sincronização ?
		if ((car_info->lap_distance - car_info->car->speed) <= 0){
			car_info->car->laps_completed++;
			if (enter_box == 'Y'){
				if (car_info->state == 'R'){
					pthread_mutex_lock(&box_mutex);
					if (box_state == 'E'){
						box_state = 'F';
						pthread_mutex_unlock(&box_mutex);
						car_info->fuel = FUEL_CAPACITY;
						car_info->lap_distance = 0;
						//sleep só aceita inteiros, tem que ser este, usa microvalores;
						usleep(1000000/NR_UNI_TEMP * randint(MIN_REP, MAX_REP));
						pthread_mutex_lock(&box_mutex);
						box_state = 'E';
					}
					pthread_mutex_unlock(&box_mutex);
				}else if (car_info->state == 'S'){
					pthread_mutex_lock(&box_mutex);
					if (box_state == 'E' || box_state == 'R'){
						box_state = 'F';
						pthread_mutex_unlock(&box_mutex);
						car_info->fuel = FUEL_CAPACITY;
						car_info->lap_distance = 0;
						car_info->state='R';
						//sleep só aceita inteiros, tem que ser este, usa microvalores;
						usleep(1000000/NR_UNI_TEMP * randint(MIN_REP, MAX_REP));
						pthread_mutex_lock(&box_mutex);
						box_state = 'E';
					}
					pthread_mutex_unlock(&box_mutex);
				}
				
				
			}
		}
	}
		
		
		
		
		
		
		sleep(1/NR_UNI_TEMP);
		//desbloquear receção de sinais (no fim da volta);
	
	}
/*
	sprintf(str, "Car %d from team: %s created", car_info->car->number, car_info->car->team_name);
	write_log(str);
*/
  	pthread_exit(NULL);
}
