/*
Tomás Mendes - 2019232272
Joel Oliveira - 2019227468
*/

#include "Simulator.h"

#define PIPE_NAME "/home/user/race_pipe"
//#define PIPE_NAME "race_pipe"
int fd_race_pipe;

int shm_id;
shm_struct* shm_info;


int main(int argc, char* argv[]) {
  int i;
  //struct sigaction sa;

  //ignorar o SIGINT no processo principal
  //deixar os outros fecharem
  //SIG_IGN = ignorar sinal
  //sa.sa_handler = signal_handler;
  signal(SIGINT, end_race);
  signal(SIGUSR2, SIG_IGN);
  signal(SIGTSTP, statistics);

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
  printf("Successfully read config file");
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
   pid_t malf_pid;
  if ( !(malf_pid = fork()) ) malfunction_manager();
  
  // create race manager process
  if (!fork()) race_manager(malf_pid);

  //sleep(1);
  //kill(0, SIGUSR2);

  // wait for both process to finish
  for (i = 0; i < 2; i++) wait(NULL);


// debug para verificar informações escritas por race_maneger -- uncomment para verificar

/*
#ifdef DEBUG
  printf("After both processes are closed\n");
  int j;
  for (i = 0; i < NR_TEAM*NR_CARS; i+=NR_CARS) {
    if (strcmp(shm_info->cars[i].team_name,"")==0)
    	break;
    printf("TEAM %s:\n", shm_info->cars[i].team_name);
    for (j = 0; j < NR_CARS; j++) {
    	if( strcmp(shm_info->cars[i + j].team_name,"") == 0)
    		break;	
      	printf("\tCar: %02d\n\t\tSpeed: %d\n\t\t Consumption: %.2f\n\t\tReliability: %d\n\t\tLaps Completed: %d\n", shm_info->cars[i + j].number, shm_info->cars[i + j].speed, shm_info->cars[i + j].consumption, shm_info->cars[i + j].reliability, shm_info->cars[i + j].laps_completed);
    }
  }
#endif
*/


  write_log("SERVER CLOSED");

  // destroy shared mem and semaphores
  destroy_resources();

  exit(0);
}

void create_named_pipe(){
	unlink(PIPE_NAME);
  	if (mkfifo(PIPE_NAME, O_CREAT | O_EXCL | 0666) < 0) {
    	perror("Erro a criar o pipe: ");
    	exit(-1);
  	}

  	if ((fd_race_pipe = open(PIPE_NAME, O_RDWR)) < 0) {
    	perror("Erro: ");
    	exit(1);
  	}
}

void get_id(int* id, key_t key, size_t size, int flag) {
  *id = shmget(key, size, flag);
  if (*id < 1) {
    write_log("Error creating shm memory!");
    destroy_resources();
    exit(1);
  }
}

void check_shmat(void* pointer){
	if (pointer < (void*)1){
		  write_log("Error attaching memory!");
    	destroy_resources();
    	exit(1);
	}
}

void initiate_shm() {
  // create shared mem key
  if ((shmkey = ftok(".", getpid())) == (key_t)-1) {
    write_log("IPC error: ftok");
    
    #ifdef DEBUG
      printf("SHM KEY = %d\n", shmkey);
    #endif
    exit(1);
  }

  get_id(&shm_id, shmkey,sizeof(shm_struct) + sizeof(car_shm_struct) * NR_TEAM * NR_CARS, IPC_CREAT | IPC_EXCL | 0700);

  shm_info = (shm_struct*)shmat(shm_id, NULL, 0);
  check_shmat(shm_info);

}
void create_sem(char * name, sem_t ** sem){
	sem_unlink(name);
	*sem = sem_open(name, O_CREAT | O_EXCL, 0700, 1);
	if (*sem == SEM_FAILED){
		write_log("Failed to create the semaphore MUTEX");
    	destroy_resources();
    	exit(1);
	}
}

void initiate_sems() {
  // create semaphores
  create_sem("LOG_MUTEX", &log_mutex);  
  create_sem("CAR_MUTEX", &car_mutex);
  
#ifdef DEBUG
char str[50];
  sprintf(str, "Semaphore initialized");
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

void statistics(int sig){
  write_log("GOT SIGTSTP - Statistics coming");
}

void end_race(){
	kill(0, SIGTERM);
	for (int i = 0; i <2; i++) wait(NULL);
	write_log("SERVER CLOSED");
	
	exit(0);
	
}
