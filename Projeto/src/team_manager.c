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
	sigset_t set, usr1_mask, term_mask;
	int i, sig;
	char str[256];
	
	sigemptyset(&set);
	sigemptyset(&usr1_mask);
	sigemptyset(&term_mask);
	
  	sigaddset(&set, SIGUSR2);
  	pthread_sigmask(SIG_BLOCK, &set, NULL);
  	
  	struct sigaction sa_tterm, sa_tusr1;
  	
  	sa_tterm.sa_handler = terminate_cars_exit;
  	sa_tusr1.sa_handler = interrupt_cars;
  	
  	sa_tterm.sa_flags = 0;
  	sa_tusr1.sa_flags = 0;
  	
  	sigaddset(&term_mask, SIGTERM);
  	sigaddset(&usr1_mask, SIGUSR1);
  	
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
	race_going = 'N';
	for (i = 0; i < NR_CARS; i++) {
		if (strcmp(shm_info->cars[index_aux + i].team_name,"")==0) break; 
		init_car_stats(&car_stats[i], index_aux, i);
		pthread_create((car_threads+i), NULL, car_worker, &car_stats[i]);
	}  	
  	cars_number = i;
  	
  	sigaction(SIGTERM, &sa_tterm, NULL);
  	sigaction(SIGUSR1, &sa_tusr1, NULL);
  	
  	usleep(1000);
  	sprintf(str, "Team %d Ready!", team_index);
	write(fd_team[team_index][1], &str, strlen(str)+1);
	
	sigwait(&set, &sig);
	
	//sinalizar às thread carro o inicio da corrida;
	pthread_mutex_lock(&cond_mutex);
	race_going = 'Y';
	pthread_cond_broadcast(&cond_start);
	pthread_mutex_unlock(&cond_mutex);
	
	sigdelset(&set, SIGUSR1);
	sigdelset(&set, SIGTERM);
  	// wait for threads to finish
  	for (i = 0; i < cars_number; i++) pthread_join(*(car_threads+i), NULL);
	//TODO send signal to unanamed pipe that cars have finished
	//TODO on race manager: when all cars finish -> signal malfunction to stop

  	// destroy resources
	terminate_cars_exit(SIGTERM);
	
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
  	stats->car->lap_distance = 0;
}


int randint(int min, int max){
	return (rand()%(max - min + 1)) + min;
}


int laps_from_fuel(car_struct *car_info){
	return ((car_info->fuel*car_info->car->speed)/car_info->car->consumption) / LAP_DIST;
}

void terminate_cars_exit(int sig){
	pthread_cond_broadcast(&cond_start);
	
	for(int i = 0; i < cars_number; i++)
		pthread_kill(car_threads[i], SIGKILL);
	
	pthread_mutex_destroy(&box_mutex);
	pthread_mutex_destroy(&cond_mutex);
	pthread_cond_destroy(&cond_start);
	
	free(car_threads);
	close(fd_team[index_aux/NR_TEAM][1]);
	exit(0);
}

void interrupt_cars(int sig){
	pthread_mutex_lock(&cond_mutex);
	race_going = 'N';
	pthread_mutex_unlock(&cond_mutex);
}



void repair_car(car_struct *car_info, bool *fuel_flag){
	box_state = 'F'; // ocupar a box
	pthread_mutex_unlock(&box_mutex);
						
	car_info->state = 'B'; //mudar o estado da box para na box;
	// Re-fuel e posiçao na pista = 0 depois de sair da box 
	car_info->fuel = FUEL_CAPACITY;
	car_info->lap_distance = 0;
	car_info->state='R';
	*fuel_flag = false;

	//sleep só aceita inteiros, tem que ser este, usa microssegundos;
	usleep(1000000/NR_UNI_PS * randint(MIN_REP, MAX_REP));
						
	pthread_mutex_lock(&box_mutex);
	box_state = 'E';
}

// function to run in car thread
void *car_worker(void *stats) {
  	// convert argument from void* to car_struct*
	car_struct *car_info = (car_struct *)stats;
	
	malfunction_msg msg;
	char str[300];
	// Y = yes, tenta entrar. N = no , nao tenta;
	char enter_box = 'N';
	float multipliers[2]; //multipliers[0] = SPEED; multipliers[1] = CONSUMPTION -> speed and consumption multipliers for race and safety mode
	bool fuel_flag = true;
	mqid = msgget(ftok(".", 25), 0);
	
	//sprintf(str, "Car %d from team %s has started the race", car_info->car->number, car_info->car->team_name);
	//write_log(str);
	
	// Race loop
	while(1){
		//wait for condition variable to unlock mutex
		pthread_mutex_lock(&cond_mutex);
		while(race_going == 'N')
			pthread_cond_wait(&cond_start, &cond_mutex);
		pthread_mutex_unlock(&cond_mutex);
		// chech if there is any malfunctions on MQ -> change car state
		if(msgrcv(mqid, &msg, 0, (long)(index_aux + car_info->car_index + 1), IPC_NOWAIT) >=0){
			car_info->state = 'S';
			sprintf(str, "Car %d received a malfunction -> Safety mode ON", car_info->car->number);
			write_log(str);
		}

		//TODO iteracao, antes de começar, bloquear receção de sinais
		if (car_info->state == 'R'){ // Race mode multipliers
			multipliers[0] = 1; // speed multiplier
			multipliers[1] = 1; // consumption multiplier
		}
		else if(car_info->state == 'S'){ // Safety mode multipliers
			multipliers[0] = 0.3; // speed multiplier
			multipliers[1] = 0.4; // consumption multiplier
		}
		
		// Decrease fuel and check if is > 0
		car_info->fuel -= multipliers[1] * car_info->car->consumption;
		if (car_info->fuel <= 0){
			car_info->state = 'D';
			sprintf(str, "Car %d from team %s ran out of fuel!", car_info->car->number, car_info->car->team_name);
			write_log(str);
			pthread_exit(NULL);
		}
		
		// Increase position on lap and check if lap was completed
		car_info->car->lap_distance += multipliers[0] * (double)car_info->car->speed; // aumentar a posição na pista
		if (car_info->car->lap_distance - LAP_DIST > 0) { // se ultrapassar a distancia da volta -> distancia = distancia - LAP_DIST
			car_info->car->lap_distance = car_info->car->lap_distance - LAP_DIST;
			car_info->car->laps_completed++;
		}

		printf("Car %d -> Distance = %.3f -> Lap %d -> State = %c -> Fuel %f\n", car_info->car->number, car_info->lap_distance, car_info->car->laps_completed, car_info->state, car_info->fuel);

		//check if the car has finished the race
		if(car_info->car->laps_completed == NR_LAP){
			car_info->car->lap_distance = 0; // reset lap_distance if car has finished race
			break;
		}
		
		// Selecionar que quer entrar na box se tiver menos de 4 volta de fuel e em modo safety se tiver menos de 2
		if(laps_from_fuel(car_info) <= 2){

			if(fuel_flag){
				sprintf(str, "Car %d only has fuel for 2 laps -> Safety mode ON", car_info->car->number);
				write_log(str);
				fuel_flag = false;
			}
	
			enter_box = 'Y';
			car_info->state = 'S'; 

		} else if (laps_from_fuel(car_info) <= 4){
			enter_box =  'Y';
			pthread_mutex_lock(&box_mutex);
			if(box_state != 'R'){ // se tiver 4 voltas de combustivel, nao pode entrar se a box estiver reservada
				

				if(fuel_flag){
					sprintf(str, "Car %d only has 4 laps of fuel -> Box is empty!", car_info->car->number);
					write_log(str);
					fuel_flag = false;
				}

			} else{
				if(fuel_flag){
					sprintf(str, "Car %d can't box -> Box already reserved!", car_info->car->number);
					write_log(str);
					fuel_flag = false;
				}
			}
			pthread_mutex_unlock(&box_mutex);
		}

		// se o carro estiver em safety mode -> reservar a box
		if (car_info->state == 'S'){
			enter_box = 'Y';
			pthread_mutex_lock(&box_mutex);
			box_state = 'R';
			pthread_mutex_unlock(&box_mutex);
		}
		
		// sincronização ?
		
		// se vai passar na meta -> verificar se precisa de entrar na box
		if ((car_info->car->lap_distance - (double)car_info->car->speed*multipliers[0]) <= 0){
			//verifica se carro está a tentar entrar na box
			if (enter_box == 'Y'){
				//se está a tentar entrar e está em race_mode
				if (car_info->state == 'R'){
					pthread_mutex_lock(&box_mutex);
					if (box_state == 'E'){ // se a box estiver vazia
						repair_car(car_info, &fuel_flag);
					}
					pthread_mutex_unlock(&box_mutex);
					
				//se está a tentar entrar por estar em safety mode
				}else if (car_info->state == 'S'){
					pthread_mutex_lock(&box_mutex);
					if (box_state == 'E' || box_state == 'R'){
						repair_car(car_info, &fuel_flag);
					}
					pthread_mutex_unlock(&box_mutex);
				}
			}
		}						
		usleep(1000000/NR_UNI_PS);
		
		//sprintf(str, "Car %d postiion :: %.2f in lap:: %d !", car_info->car->number, car_info->lap_distance, car_info->car->laps_completed);
		//desbloquear receção de sinais (no fim da volta);
	}
	sprintf(str, "Car %d from team: %s has finished the race", car_info->car->number, car_info->car->team_name);
	write_log(str);
	
  	pthread_exit(NULL);
}
