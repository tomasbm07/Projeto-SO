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
    sa_int.sa_mask = int_mask;
    sa_int.sa_flags = 0;
    sigaction(SIGINT, &sa_int, NULL);
    
    sa_tstp.sa_handler = statistics;
    sa_tstp.sa_flags = 0;
    sigaction(SIGTSTP, &sa_tstp, NULL);
    
    signal(SIGUSR1, SIG_IGN);
    signal(SIGUSR2, SIG_IGN);
    signal(SIGTERM, SIG_IGN);

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
    printf("--Configurações lidas do ficheiro--\n\n");
    printf("Numero de unidade de tempo /s: %dut\n", NR_UNI_PS);
    printf("Distancia de uma volta: %dm, Numero de voltas da corrida: %d\n",LAP_DIST, NR_LAP);
    printf("Numero de equipas: %d\n", NR_TEAM);
    printf("Numero de carros por equipa: %d\n", NR_CARS);
    printf("Numero de unidades de tempo entre possivel avaria: %dut\n",MALFUNCTION_UNI_NR);
    printf("Tempo min de reparacao: %dut, Tempo max dereparacao: %dut\n", MIN_REP,MAX_REP);
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
    while (wait(NULL) == -1 && errno==EINTR);

    // destroy resources and kill malfunction
    kill(cpid[0], SIGTERM); //signal malfunction to end if race ends normally, i.e, without ctrl c in the middle of the race
    end_race();

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
}


void create_sem(char * name, sem_t ** sem){
    sem_unlink(name);
    *sem = sem_open(name, O_CREAT | O_EXCL, 0700, 1);
    if (*sem == SEM_FAILED){
        write_log("Failed to create the semaphore MUTEX");
        destroy_resources();
        exit(-1);
    }
}


void initiate_sems() {
    // create semaphores
    create_sem("LOG_MUTEX", &log_mutex);  
    create_sem("CAR_MUTEX", &car_mutex);
    
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
  
    sem_close(car_mutex);
    sem_unlink("CAR_MUTEX");  

    shmdt(shm_info);
    shmctl(shm_id, IPC_RMID, NULL);
  
    close(fd_race_pipe);
    unlink(PIPE_NAME);
}


void statistics(){
    //char str[256];
    write_log("GOT SIGTSTP - Statistics coming");

    int x = 0;
    int i, j;
    car_shm_struct array[NR_TEAM*NR_CARS]; // array com uma copia dos carros

    //TODO sincronizacao para isto. pause(), copiar e enviar um sinal para a corrida recomeçar? 

    //copy the cars from shm to array
    for (i = 0; i < NR_TEAM; i++){
		for (j = 0; j < NR_CARS; j++){
			array[x++] = shm_info->cars[i * NR_CARS + j];
		}
	}

    //sort cars by track position with bubble sort with conditional varaiable :)
    bool no_swaps;
    for (i = 0; i < NR_TEAM * NR_CARS; i++){
    	no_swaps = true;
		for (j = 0; j < NR_TEAM * NR_CARS - i - 1; j++){
            //se um estiver numa volta á frente -> trocar
            if (array[j].laps_completed < array[j + 1].laps_completed){
                swap(array, j, j + 1);
                no_swaps = false;
            } 

            //se estiverem na mesma volta -> ver quem está á frente
            else if(array[j].laps_completed == array[j + 1].laps_completed){
                if (array[j].lap_distance < array[j + 1].lap_distance){
                   swap(array, j, j + 1);
                   no_swaps = false;
                }
            }
		}
		if (no_swaps) break;
	}
    
    //print statistics
    char chars[][3] = {"st", "nd", "rd", "th"};
    printf("-------------------------------------------------\n");
    for (i = 0, x = 0; i < NR_TEAM * NR_CARS; i++, x++){
        if(i == 0 || i == 1 || i == 2 || i == 3 || i == 4)
            printf("%d %s -> Car %d from team %s [lap: %d, lap_distance: %.3f]\n", x+1, chars[x <= 3 ? x : 3], array[i].number, array[i].team_name, array[i].laps_completed, array[i].lap_distance);
        else if (i == NR_TEAM * NR_CARS - 1)
            printf("%d %s -> Car %d from team %s [lap: %d, lap_distance: %.3f]\n", NR_TEAM * NR_CARS, chars[3], array[i].number, array[i].team_name, array[i].laps_completed, array[i].lap_distance);

	}
    printf(/*str, */"Total de malfuntions %d\n", shm_info->malfunctions_counter);
    //printf(str);
    printf(/*str, */"Total de paragens na box %d\n", shm_info->refill_counter);
    //printf(str);
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
	for (int i = 0; i < 2; i++) wait(NULL);
	write_log("SERVER CLOSED");
    statistics();
	destroy_resources();
	exit(0);
}
