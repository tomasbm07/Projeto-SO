/*
Tomás Mendes - 2019232272
Joel Oliveira - 2019227468
*/

#include "team_manager.h"

pthread_mutex_t box_mutex = PTHREAD_MUTEX_INITIALIZER, cond_mutex = PTHREAD_MUTEX_INITIALIZER, pipe_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_start = PTHREAD_COND_INITIALIZER;
char box_state;
int cars_number, index_aux;
pthread_t *car_threads;
bool race_going;

void team_manager(int team_index) {
    index_aux = team_index*NR_CARS;
    sigset_t set, usr1_mask, usr2_mask, term_mask;
    int i, sig;
    char str[256];

    sigemptyset(&usr1_mask);
    sigemptyset(&usr2_mask);
    sigemptyset(&term_mask);
    
    struct sigaction sa_tterm, sa_tusr1, sa_tusr2;
    
    sa_tterm.sa_handler = terminate_cars_exit;
    sa_tusr1.sa_handler = swap_race_state;
    sa_tusr2.sa_handler = end_car_race;
    
    sa_tterm.sa_flags = 0;
    sa_tusr1.sa_flags = 0;
    sa_tusr2.sa_flags = 0;
    
    sigaddset(&term_mask, SIGTERM);
    sigaddset(&usr1_mask, SIGUSR1);
    
    sigaction(SIGUSR2, &sa_tusr2, NULL);
    
#ifdef DEBUG
    char aux[256];
    sprintf(aux, "Team manager created (PID: %d), for Team %s\n", getpid(), shm_info->cars[team_index*NR_CARS].team_name);
    write_log(aux);
#endif
    
    box_state = 'E';  // 'R' = Reserved; 'E' = Empty; 'F' = Full;
    srand(time(NULL));
    car_threads = create_threads_array();
    car_struct car_stats[NR_CARS];

    //create thread_cars;	
    race_going = false;
    for (i = 0; i < NR_CARS; i++) {
        if (strcmp(shm_info->cars[index_aux + i].team_name,"")==0) break; 
        init_car_stats(&car_stats[i], index_aux, i);
        pthread_create((car_threads+i), NULL, car_worker, &car_stats[i]);
    }  	
    cars_number = i;
    sigemptyset(&set); 
    
    sigaddset(&set, SIGUSR2);
    sigaddset(&set, SIGTSTP);
    sigaddset(&set, SIGINT);
    
    pthread_sigmask(SIG_BLOCK, &set, NULL);
    
    sigaction(SIGTERM, &sa_tterm, NULL);
    sigaction(SIGUSR1, &sa_tusr1, NULL);
    
    usleep(1000);
    sprintf(str, "Team %s Ready!", shm_info->cars[team_index * NR_CARS].team_name);
    write(fd_team[team_index][1], &str, strlen(str)+1);
    
    sigwait(&set, &sig);
    
    //sinalizar às thread carro o inicio da corrida;
    pthread_mutex_lock(&cond_mutex);
    race_going = true;
    pthread_cond_broadcast(&cond_start);
    pthread_mutex_unlock(&cond_mutex);

      // wait for threads to finish
    for (i = 0; i < cars_number; i++) pthread_join(*(car_threads+i), NULL);
	
	kill(getppid(), SIGUSR2);
	//printf("team manager writing\n");
	//write(fd_team[index_aux/NR_TEAM], "T", 5);
	
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


float laps_from_fuel(car_struct *car_info){
    return (((double)car_info->fuel*car_info->car->speed)/car_info->car->consumption) / (double)LAP_DIST;
}


void terminate_cars_exit(int sig){
	race_going = true;
    pthread_cond_broadcast(&cond_start);
    
    for(int i = 0; i < cars_number; i++)
        pthread_kill(car_threads[i], SIGUSR2);
    
    for (int i = 0; i < cars_number; i++) pthread_join(*(car_threads+i), NULL);

    pthread_mutex_destroy(&box_mutex);
    pthread_mutex_destroy(&cond_mutex);
    pthread_cond_destroy(&cond_start);
    
    free(car_threads);
    close(fd_team[index_aux/NR_TEAM][1]);
    exit(0);
}

void swap_race_state(int sig){
    pthread_mutex_lock(&cond_mutex);
    race_going = !race_going;
    if (race_going){
        pthread_cond_broadcast(&cond_start);
    }
    pthread_mutex_unlock(&cond_mutex);
}

void end_car_race(int sig){
    printf("CAR ENDING\n");
    char to_car_pipe[10];
    sprintf(to_car_pipe, "E");
    pthread_mutex_lock(&pipe_mutex);
    write(fd_team[index_aux/NR_CARS][1], to_car_pipe, sizeof(to_car_pipe));
    pthread_mutex_unlock(&pipe_mutex);
    
    pthread_exit(NULL);
}


void repair_car(car_struct *car_info, bool *fuel_flag, bool *has_malfunction, char *to_car_pipe, sigset_t set_control_ending){
    box_state = 'F'; // ocupar a box
    pthread_mutex_unlock(&box_mutex);

    //char str[256];

    pthread_sigmask(SIG_UNBLOCK, &set_control_ending, NULL);
            
    sprintf(to_car_pipe,"B%02d", car_info->car->number);
    pthread_mutex_lock(&pipe_mutex);
    write(fd_team[index_aux/NR_CARS][1], to_car_pipe,  sizeof(to_car_pipe));
    pthread_mutex_unlock(&pipe_mutex);

    car_info->state = 'B'; //mudar o estado do carro para na box;
    
    // Re-fuel e posiçao na pista = 0 depois de sair da box 
    car_info->fuel = FUEL_CAPACITY;
    car_info->car->lap_distance = 0;
    
    *fuel_flag = false;

    // if it has malfunction -> wait repair time, else wait a few moments
    if(*has_malfunction){
        //sprintf(str, "[BOX -> Team %s] Car %d waiting to be repaired", car_info->car->team_name, car_info->car->number);
        //write_log(str);
        usleep(1000000/NR_UNI_PS * randint(MIN_REP, MAX_REP));
    }
    //sprintf(str, "[BOX -> Team %s] Car %d is just fueling", car_info->car->team_name, car_info->car->number);
    //write_log(str);
    usleep(1000000/NR_UNI_PS * 2);
       
    sprintf(to_car_pipe,"R%02d", car_info->car->number);
    pthread_mutex_lock(&pipe_mutex);
    write(fd_team[index_aux/NR_CARS][1], to_car_pipe,  sizeof(to_car_pipe));
    pthread_mutex_unlock(&pipe_mutex);

    car_info->state = 'R';
    
    *has_malfunction = false;
    (shm_info->refill_counter)++;	
    (car_info->car->box_stops_counter)++;	
    
    pthread_sigmask(SIG_BLOCK, &set_control_ending, NULL);
    
    pthread_mutex_lock(&box_mutex);
    box_state = 'E';

    //sprintf(str, "Car %d from team %s has left the box! -> State = %c", car_info->car->number, car_info->car->team_name, car_info->state);
    //write_log(str);
}

// function to run in car thread
void *car_worker(void *stats) {
    car_struct *car_info = (car_struct *)stats; // convert argument from void* to car_struct*
    malfunction_msg msg; // MQ struct
    char str[300], to_car_pipe[10]; //auxilary string to pass parameters to write_log with sprintf
    bool enter_box = false; // true -> tenta entrar. false -> nao tenta;
    bool has_malfunction = false; // flag to only log received malfunctions once and sleep repair
    bool fuel_flag = true; // flag to only log fuel warnings once
    bool crossed_start_line = true; // flag if car crossed the start/finish line. to make it easier for the box condition
    float multipliers[2]; //multipliers[0] = SPEED; multipliers[1] = CONSUMPTION -> speed and consumption multipliers for race and safety mode
    int counter = 0; // iteration counter. To print cars states every x iterations
    mqid = msgget(ftok(".", 25), 0); // MQ id
    
    sprintf(str, "Car %d from team %s has started the race", car_info->car->number, car_info->car->team_name);
    write_log(str);
    
    sigset_t set, set_control_ending;
    sigfillset(&set);
    sigemptyset(&set_control_ending);
    sigaddset(&set_control_ending, SIGUSR2);
    pthread_sigmask(SIG_BLOCK, &set, NULL);
    
    // Race loop
    while(1){
        sem_wait(statistics_mutex);

        // check if there is any malfunctions on MQ -> change car state
        if(msgrcv(mqid, &msg, 0, (long)(index_aux + car_info->car_index + 1), IPC_NOWAIT) >= 0){
            if(!has_malfunction){
                (shm_info->malfunctions_counter)++;
            }
            has_malfunction = true;
            enter_box = true;
            if (car_info->state != 'S'){
                sprintf(str, "CAR %02d GOT A MALFUNCTION", car_info->car->number);
                write_log(str);
                //comunicate change state to safety to race_manager.. sends state + car_number, for clean printing
                sprintf(to_car_pipe,"S%02d", car_info->car->number);
                pthread_mutex_lock(&pipe_mutex);
                write(fd_team[index_aux/NR_CARS][1], to_car_pipe, sizeof(to_car_pipe));
                pthread_mutex_unlock(&pipe_mutex);
                car_info->state = 'S';
            }
        }

        // se está na linha da meta -> verificar se precisa de entrar na box
        if (crossed_start_line) {
            //check if the car has finished the race
            if(car_info->car->laps_completed == NR_LAP){
                car_info->car->lap_distance = 0; // reset lap_distance if car has finished race
            
                sprintf(to_car_pipe, "F%02d", car_info->car->number);
                pthread_mutex_lock(&pipe_mutex);
                write(fd_team[index_aux/NR_CARS][1], to_car_pipe, sizeof(to_car_pipe));
                pthread_mutex_unlock(&pipe_mutex);
                
                //car_info->state = 'F';
                break;
            }
            
            crossed_start_line = false;
            
            //wait for condition variable to unlock mutex
            pthread_mutex_lock(&cond_mutex);
            while(!race_going){
                pthread_cond_wait(&cond_start, &cond_mutex);
            }
            pthread_mutex_unlock(&cond_mutex);
            
            //unblock signal sigusr2 to see if signal is in queue to end race, then block again.
            pthread_sigmask(SIG_UNBLOCK, &set_control_ending, NULL);
            pthread_sigmask(SIG_BLOCK, &set_control_ending, NULL);
            //verifica se carro está a tentar entrar na box
            if (enter_box){
                //se está a tentar entrar e está em race_mode
                if (car_info->state == 'R'){
                    pthread_mutex_lock(&box_mutex);
                    if (box_state == 'E'){ // se a box estiver vazia
                        repair_car(car_info, &fuel_flag, &has_malfunction, to_car_pipe, set_control_ending);
                        enter_box = false;
                    }
                    pthread_mutex_unlock(&box_mutex);
                    
                //se está a tentar entrar por estar em safety mode
                }else if (car_info->state == 'S'){
                    pthread_mutex_lock(&box_mutex);
                    if (box_state == 'E' || box_state == 'R'){
                        repair_car(car_info, &fuel_flag, &has_malfunction, to_car_pipe, set_control_ending);
                        enter_box = false;
                    }
                    pthread_mutex_unlock(&box_mutex);
                }
            }
        } // end if	
        
        if (car_info->state == 'R'){ // Race mode multipliers
            multipliers[0] = 1; // speed multiplier
            multipliers[1] = 1; // consumption multiplier
        }
        else if(car_info->state == 'S'){ // Safety mode multipliers
            multipliers[0] = 0.3; // speed multiplier
            multipliers[1] = 0.4; // consumption multiplier
        }
        
        // Decrease fuel and check if is < 0
        car_info->fuel -= multipliers[1] * car_info->car->consumption;
        if (car_info->fuel <= 0){            
            sprintf(to_car_pipe, "D%02d", car_info->car->number);
            pthread_mutex_lock(&pipe_mutex);
            write(fd_team[index_aux/NR_CARS][1], to_car_pipe, sizeof(to_car_pipe));
            pthread_mutex_unlock(&pipe_mutex);
            
            car_info->state = 'D';
           //sprintf(str, "Car %d from team %s ran out of fuel!", car_info->car->number, car_info->car->team_name);
           //write_log(str);
            pthread_exit(NULL);
        } else if(laps_from_fuel(car_info) <= 2){
        	if (car_info->state !='S'){
        		sprintf(str, "CAR %02d LOW ON FUEL", car_info->car->number);
            	write_log(str);
        	
            	//comunicate change state to safety to race_manager.. sends state + car_number, for clean printing
            	sprintf(to_car_pipe,"S%02d", car_info->car->number);
            	write(fd_team[index_aux/NR_CARS][1], to_car_pipe,  5);
            	car_info->state = 'S';
            }
            enter_box = true;
        } else if(laps_from_fuel(car_info)<=4){
            enter_box = true;
        }
        
        // Increase position on lap and check if lap was completed
        car_info->car->lap_distance += multipliers[0] * (double)car_info->car->speed; // aumentar a posição na pista
        if (car_info->car->lap_distance - LAP_DIST >= 0) { // se ultrapassar a distancia da volta -> distancia = distancia - LAP_DIST
            car_info->car->lap_distance -= LAP_DIST;
            car_info->car->laps_completed++;
            crossed_start_line = true;
        }

        //only print every x iterations. just so the log isn't spammed too much
        if(++counter == 20){
            sprintf(str,"Car %d | Distance = %7.2f | Lap %d | State = %c | Fuel = %04.2f(%04.2f laps)", car_info->car->number, car_info->car->lap_distance, car_info->car->laps_completed, car_info->state, car_info->fuel, laps_from_fuel(car_info));
            write_log(str);
            counter = 0;
        }

        // se o carro estiver em safety mode -> reservar a box
        if (car_info->state == 'S'){
            enter_box = true;
            pthread_mutex_lock(&box_mutex);
            box_state = 'R';
            pthread_mutex_unlock(&box_mutex);
        } 
        sem_post(statistics_mutex);
        usleep(1000000/NR_UNI_PS);

    }

    sprintf(str, "++++ Car %d from team: %s has finished the race ++++", car_info->car->number, car_info->car->team_name);
    write_log(str);
    
    pthread_exit(NULL);
}
