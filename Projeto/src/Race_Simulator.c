/*
Tomás Mendes - 2019232272
Joel Oliveira - 2019227468
*/

#include "Simulator.h"

//#define PIPE_NAME "/home/user/race_pipe"
#define PIPE_NAME "race_pipe"

int fd_race_pipe;
int shm_id;
shm_struct* shm_info;
pid_t cpid[2];

int main(int argc, char* argv[]) {
    struct sigaction sa_int, sa_tstp;
    sigset_t int_mask;
    
    sa_int.sa_handler = end_race;
    sigfillset(&int_mask);
    sigdelset(&int_mask, SIGTSTP);
    sa_int.sa_mask = int_mask;
    sa_int.sa_flags = 0;
    sigaction(SIGINT, &sa_int, NULL);
    
    sa_tstp.sa_handler = statistics;
    sa_tstp.sa_flags = 0;
    sigaction(SIGTSTP, &sa_tstp, NULL);

    //signals to ignore on this process
    signal(SIGUSR1, SIG_IGN);
    signal(SIGUSR2, SIG_IGN);
    signal(SIGTERM, SIG_IGN);

    printf("Race Simulator created (PID: %d)\n", getpid());

    f = fopen("log.txt", "a");

    if (argc < 2) {
        printf("No config file was passed!\n");
        exit(1);
    } else if (argc > 2) {
    #ifdef DEBUG
        printf("Program only takes config filename as argument");
    #endif
        exit(1);
    }

    // read config file
    read_file(argv[1]);

    #ifdef DEBUG
    printf("Successfully read config file\n");
    printf("\n--Configurações lidas do ficheiro--\n");
    printf("Numero de unidade de tempo /s: %dut\n", NR_UNI_PS);
    printf("Distancia de uma volta: %dm, Numero de voltas da corrida: %d\n",LAP_DIST, NR_LAP);
    printf("Numero de equipas: %d\n", NR_TEAM);
    printf("Numero de carros por equipa: %d\n", NR_CARS);
    printf("Numero de unidades de tempo entre possivel avaria: %dut\n",MALFUNCTION_UNI_NR);
    printf("Tempo min de reparacao: %dut, Tempo max de reparacao: %dut\n", MIN_REP,MAX_REP);
    printf("Capacidade do deposito: %dL\n", FUEL_CAPACITY);
    printf("--------------------------\n");
    printf("ut - unidades de tempo\n");
    printf("m - metros\n");
    printf("L - litros\n");
    printf("--------------------------\n\n\n");
    #endif

    initiate_resources();

    write_log("SERVER STARTED");

    // create malfunction manager process
    if (!(cpid[0] = fork())) malfunction_manager();
        
    // create race manager process
    if (!(cpid[1] = fork())) race_manager(cpid[0]);

    // wait for race manager and malfunction manager
    int wait_status;
    while ( (wait_status=wait(NULL))>=0 || (wait_status == -1 && errno == EINTR) );

    // destroy resources and kill malfunction
    write_log("SERVER CLOSED");
    statistics();
    destroy_resources();
    
    exit(0);
}


void create_named_pipe(){
    unlink(PIPE_NAME);
    if (mkfifo(PIPE_NAME, O_CREAT | O_EXCL | 0666) < 0) {
        write_log("ERRO: a criar o pipe: ");
        destroy_resources();
        exit(-1);
    }

    if ((fd_race_pipe = open(PIPE_NAME, O_RDWR)) < 0) {
        write_log("ERRO: A abrir o pipe");
        destroy_resources();
        exit(-1);
    }

    #ifdef DEBUG
    write_log("Named pipe 'race_pipe' is ready!\n");
    #endif
}


void get_id(int* id, key_t key, size_t size, int flag) {
    *id = shmget(key, size, flag);
    if (*id < 1) {
        write_log("Error creating shm memory!");
        destroy_resources();
        exit(-1);
    }
}


void check_shmat(void* pointer){
    if (pointer < (void*)1){
          write_log("Error attaching memory!");
        destroy_resources();
        exit(-1);
    }
}


void initiate_shm() {
    // create shared mem key
    if ((shmkey = ftok(".", getpid())) == (key_t)-1) {
        write_log("IPC error: ftok");
        destroy_resources();
        exit(-1);
    }

    get_id(&shm_id, shmkey, sizeof(shm_struct) + sizeof(car_shm_struct) * NR_TEAM * NR_CARS, IPC_CREAT | IPC_EXCL | 0700);

    shm_info = (shm_struct*)shmat(shm_id, NULL, 0);
    check_shmat(shm_info);
    
    shm_info->wait_statistics = false;
    shm_info->counter_cars_finished = 0;
    shm_info->nr_cars = 0;
    shm_info->end_counter = 0;
}


void create_sem(char * name, sem_t ** sem, int sem_value){
    sem_unlink(name);
    *sem = sem_open(name, O_CREAT | O_EXCL, 0700, sem_value);
    if (*sem == SEM_FAILED){
        write_log("Failed to create the semaphore MUTEX");
        destroy_resources();
        exit(-1);
    }
}


void initiate_sems() {
    // create semaphores
    create_sem("LOG_MUTEX", &log_mutex, 1);  
    create_sem("COUNTER_MUTEX", &counter_mutex, 1);
    create_sem("STATISTICS_MUTEX", &statistics_mutex, 1);
    create_sem("COND_STAT", &cond_sem_stat, 0);
    create_sem("COND_CAR", &cond_sem_car, 0);
    create_sem("CAR_COUNT", &sem_car_count, 0);
    
    //create_sem("STAISTICS_STOPS", &statistics_mutex, NR_TEAM*NR_CARS);
    #ifdef DEBUG
    char str[50];
    sprintf(str, "Semaphores initialized");
    write_log(str);
    #endif
}


void initiate_resources() {
    initiate_sems();
    initiate_shm(); 
    create_named_pipe();
}


void destroy_resources(void) {
    write_log("Cleaning up...");
    fclose(f);

    sem_close(log_mutex);
    sem_unlink("LOG_MUTEX");
  
    sem_close(counter_mutex);
    sem_unlink("COUNTER_MUTEX");  
	
	sem_close(statistics_mutex);
    sem_unlink("STATISTICS_MUTEX");  
    
    sem_close(cond_sem_stat);
    sem_unlink("COND_STAT");  
    
    sem_close(cond_sem_car);
    sem_unlink("COND_CAR");  
    
    sem_close(sem_car_count);
    sem_unlink("CAR_COUNT");  
    
    shmdt(shm_info);
    shmctl(shm_id, IPC_RMID, NULL);
  
    close(fd_race_pipe);
    unlink(PIPE_NAME);
}


void statistics(){
    //char str[256];
    write_log("GOT SIGTSTP - Statistics coming");

    int i, j, x = 0, value;
    car_shm_struct array[NR_TEAM*NR_CARS]; // array com uma copia dos carros

	sem_getvalue(sem_car_count, &value);
    
    sem_wait(counter_mutex);
    if (value > (shm_info->nr_cars) - (shm_info->counter_cars_finished) ){
    	for( i = 0; i < value - ( (shm_info->nr_cars) - shm_info->counter_cars_finished);i++ )
    		sem_wait(sem_car_count);
    }
	sem_post(counter_mutex);
	
    //alterar a flag
    sem_wait(statistics_mutex);
    shm_info->wait_statistics = true;
    sem_post(statistics_mutex);
  
    
    //esperar que todos os carros parem
    while(value > 0){
    	sem_wait(cond_sem_stat);
    	sem_getvalue(sem_car_count, &value);
    }

    //copy the cars from shm to array
    for (i = 0; i < NR_TEAM; i++){
        for (j = 0; j < NR_CARS; j++){
            array[x++] = shm_info->cars[i * NR_CARS + j];
        }
    }

    //sort cars by track position with bubble sort
    for (i = 0; i < NR_TEAM * NR_CARS; i++){
        for (j = 0; j < NR_TEAM * NR_CARS - 1; j++){
            if(array[j].end_position != 0){
                if (array[j + 1].end_position != 0 && array[j].end_position > array[j + 1].end_position)
                    swap(array, j, j + 1);
            }
            else{
                //se um estiver numa volta á frente -> trocar
                if (array[j].laps_completed < array[j + 1].laps_completed){
                    swap(array, j, j + 1);
                }
                //se estiverem na mesma volta e j ainda não acabou -> ver lap_distance
                else if(array[j].laps_completed == array[j + 1].laps_completed && array[j].laps_completed != NR_LAP){
                    if (array[j].lap_distance < array[j + 1].lap_distance){
                        swap(array, j, j + 1);
                    }
                }
            }
        }
    }
    
    //alterar a flag a indicar fim do procedimento
    sem_wait(statistics_mutex);
    shm_info->wait_statistics = false;
    sem_post(statistics_mutex);
    
    //permitir aos carros avançar
    sem_wait(counter_mutex);
    for(i = 0; i < (shm_info->nr_cars) - shm_info->counter_cars_finished; i++)
    	sem_post(cond_sem_car);
    sem_post(counter_mutex);
    
    //print statistics
    char chars[][3] = {"st", "nd", "rd", "th"};
    char statistics[500*(shm_info->nr_cars)];
    strcpy(statistics, "");
    
    char aux[500]="";
    
    printf("-------------------------------------------------\n");
    strcat(statistics, "----Statistics----\n");
    
    sem_wait(counter_mutex);
    for (i = 0, x = 0; i < (shm_info->nr_cars); i++, x++){
    	sem_post(counter_mutex);
    	
    	sem_wait(counter_mutex);
        if(0<=i && i<=4){
            sprintf(aux, "%d%s -> Car %02d from team %s [lap: %d, lap_distance: %7.2f, stops: %d]\n", x+1, chars[x <= 3 ? x : 3], array[i].number, array[i].team_name, array[i].laps_completed, array[i].lap_distance, array[i].box_stops_counter);
            strcat(statistics, aux);
        }
        else if (i == (shm_info->nr_cars) - 1){
            sprintf(aux, "   .\n   .\n   .\n");
            strcat(statistics, aux);

            sprintf(aux, "%d%s -> Car %02d from team %s [lap: %d, lap_distance: %7.2f, stops: %d]\n", NR_TEAM * NR_CARS, chars[3], array[i].number, array[i].team_name, array[i].laps_completed, array[i].lap_distance, array[i].box_stops_counter);
            strcat(statistics, aux);
            
        }
        sem_post(counter_mutex);
        
        sem_wait(counter_mutex);
    }
    sem_post(counter_mutex);
    
    sprintf(aux, "Total de malfuntions %d\n", shm_info->malfunctions_counter);
    strcat(statistics, aux);
    sprintf(aux, "Total de paragens na box %d\n", shm_info->refill_counter);
    strcat(statistics, aux);

    sem_wait(counter_mutex);
    sprintf(aux, "Carros em pista: %d\n", (shm_info->nr_cars) - shm_info->counter_cars_finished);
    sem_post(counter_mutex);
    strcat(statistics, aux);
    write_log(statistics);
    printf("-------------------------------------------------\n");
}


//swap 2 cars on aux array
void swap(car_shm_struct *array, int a, int b){
    car_shm_struct temp = array[a];
    array[a] = array[b];
    array[b] = temp;
}


void end_race(){
    kill(cpid[1], SIGTERM); // race_manager
    //for (int i = 0; i < 2; i++) wait(NULL);
    int wait_status;
    while ( (wait_status=wait(NULL))>=0 || (wait_status == -1 && errno == EINTR));
    write_log("SERVER CLOSED");
    statistics();
    destroy_resources();
    exit(0);
}
