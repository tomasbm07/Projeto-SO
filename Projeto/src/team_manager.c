/*
Tomás Mendes - 2019232272
Joel Oliveira - 2019227468
*/

#include "team_manager.h"

pthread_mutex_t box_mutex = PTHREAD_MUTEX_INITIALIZER, cond_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_start = PTHREAD_COND_INITIALIZER;
char box_state;
char race_going;
int cars_number, index_aux;
pthread_t *car_threads;

void team_manager(int team_index) {
	index_aux = team_index*NR_CARS;
	signal(SIGTERM, clean_stuff);
	sigset_t set;
	int i, sig;
	char str[256];
	sigemptyset(&set);
  	sigaddset(&set, SIGUSR2);
  	pthread_sigmask(SIG_BLOCK, &set, NULL);
  	
#ifdef DEBUG
	char aux[50];
	sprintf(aux, "Team manager created (PID: %d), from Team %d\n", getpid(), team_index);
	write_log(aux);
#endif
  
	box_state = 'E';  // 'R' = Reserved; 'E' = Empty; 'F' = Full;
	srand((unsigned)getpid());
	car_threads = create_threads_array();
	car_struct car_stats[NR_CARS];

	//create thread_cars;	
	for (i = 0; i < NR_CARS; i++) {
		if (strcmp(shm_info->cars[index_aux + i].team_name,"")==0) break; 
		init_car_stats(&car_stats[i], index_aux, i);
		pthread_create((car_threads+i), NULL, car_worker, &car_stats[i]);
	}  	
  	cars_number = i;
  	
  	usleep(1000);
  	sprintf(str, "Team %d Ready!", team_index);
	write(fd_team[team_index][1], &str, strlen(str)+1);
	
	sigwait(&set, &sig);
	
	pthread_cond_broadcast(&cond_start);
  	// wait for threads to finish
  	for (i = 0; i < cars_number; i++) pthread_join(*(car_threads+i), NULL);

  	// destroy resources
	clean_stuff();
	
	write_log("Team Manager finishing");
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
void init_car_stats(car_struct *stats, int team_index, int car_index) {
	stats->car = &shm_info->cars[team_index + car_index];
	stats->car_index = car_index;
  	stats->state = 'R';
  	stats->fuel = FUEL_CAPACITY;
  	stats->lap_distance = 0;
}

int randint(int min, int max){
	return (rand()%(max - min + 1)) + min;
}

void clean_stuff(){
	for (int i = 0; i < cars_number; i++) pthread_join(*(car_threads+i), NULL);
	pthread_mutex_destroy(&box_mutex);
	pthread_mutex_destroy(&cond_mutex);
	pthread_cond_destroy(&cond_start);
	free(car_threads);
	close(fd_team[index_aux/NR_TEAM][1]);
	exit(0);
}


//TODO: READ MESSAGE QUEUE MESSAGES AND DETERMINE WHAT TO DO
// function to run in car thread
void *car_worker(void *stats) {
  	// convert argument from void* to car_struct*
	car_struct *car_info = (car_struct *)stats;
	
	pthread_mutex_lock(&cond_mutex);
	pthread_cond_wait(&cond_start, &cond_mutex);
	pthread_mutex_unlock(&cond_mutex);
	
	malfunction_msg msg;
	// Y = yes, tenta entrar. N = no , nao tenta;
	char enter_box = 'N';
	int counter = 0;
	char str[300];
	float multipliers[2]; //multipliers[0] = SPEED; multipliers[1] = CONSUMPTION -> speed and consumption multipliers for race and safety mode
	while(1/*(++counter) < 6*/){

		if(msgrcv(mqid, &msg, 0, (long)(index_aux + car_info->car_index + 1), IPC_NOWAIT) < 0)
			if ( errno != ENOMSG)
				printf("--------------------Car %d got malfunction------------------!\n", car_info->car->number);
		//TODO iteracao, antes de começar, bloquear receção de sinais
		if (car_info->state == 'R'){
			multipliers[0] = 1; // speed multiplier
			multipliers[1] = 1; // consumption multiplier
		}
		else if(car_info->state == 'S'){
			multipliers[0] = 0.3; // speed multiplier
			multipliers[1] = 0.4; // consumption multiplier
		}
		
		car_info->fuel -= multipliers[1] * car_info->car->consumption;

		car_info->lap_distance += multipliers[0] * car_info->car->speed; // aumentar a posição na pista
		if (car_info->lap_distance - LAP_DIST > 0) { // se ultrapassar a distancia da volta -> distancia = distancia - LAP_DIST
			car_info->lap_distance = car_info->lap_distance - LAP_DIST;
			car_info->car->laps_completed++;
		}
		//printf("Car %d -> Distance = %.3f -> Lap %d\n", car_info->car->number, car_info->lap_distance, car_info->car->laps_completed);

		if(car_info->car->laps_completed == NR_LAP){
			printf("Car %d finished!\n", car_info->car->number);
			break;
		}

		// sincronização ?
		// se vai passar na meta, incrementar voltas e verificar se vai entrar na box
		if ((car_info->lap_distance - car_info->car->speed*multipliers[0]) <= 0){
			//verifica se carro está a tentar entrar na box
			if (enter_box == 'Y'){
				//se está a tentar entrar e está em race_mode
				if (car_info->state == 'R'){
					pthread_mutex_lock(&box_mutex);
					if (box_state == 'E'){
						box_state = 'F';
						// posiçao na pista = 0 depois de sair da box
						pthread_mutex_unlock(&box_mutex);
						car_info->fuel = FUEL_CAPACITY;
						car_info->lap_distance = 0;
						//sleep só aceita inteiros, tem que ser este, usa microvalores;
						usleep(1000000/NR_UNI_PS * randint(MIN_REP, MAX_REP));
						pthread_mutex_lock(&box_mutex);
						box_state = 'E';
					}
					pthread_mutex_unlock(&box_mutex);
					
				//se está a tentar entrar por estar em safety mode
				}else if (car_info->state == 'S'){
					pthread_mutex_lock(&box_mutex);
					if (box_state == 'E' || box_state == 'R'){
						box_state = 'F';
						pthread_mutex_unlock(&box_mutex);
						car_info->fuel = FUEL_CAPACITY;
						car_info->lap_distance = 0;
						car_info->state='R';
						usleep(1000000/NR_UNI_PS * randint(MIN_REP, MAX_REP));
						pthread_mutex_lock(&box_mutex);
						box_state = 'E';
					}
					pthread_mutex_unlock(&box_mutex);
				}
			}
		}						
		usleep(1000000/NR_UNI_PS);
		
		sprintf(str, "Car %d postiion :: %.2f in lap:: %d !", car_info->car->number, car_info->lap_distance, car_info->car->laps_completed);
		//desbloquear receção de sinais (no fim da volta);
	}
	sprintf(str, "Car %d from team: %s made %d laps !", car_info->car->number, car_info->car->team_name, car_info->car->laps_completed);
	write_log(str);
	
  	pthread_exit(NULL);
}
