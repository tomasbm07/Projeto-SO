#include "Simulator.h"

int shm_main_id, shm_boxes_id;
shm_struct* shm_info;
shm_boxes * shm_boxes_state;
sem_t *log_mutex;

int main(int argc, char* argv[]) {
  int i;

  if (argc < 2) {
#ifdef DEBUG
    write_log("No config file was passed!");
#endif
    exit(1);
  } else if (argc > 2) {
#ifdef DEBUG
    write_log("Program only takes config filename as argument");
#endif
    exit(1);
  }

  // read config file
  read_file(argv[1]);

#ifdef DEBUG
  write_log("Successfully read config file");

  printf("--Configurações lidas do ficheiro--\n\n");
  printf("Numero de unidade de tempo /s: %dut\n", NR_UNI_PS);
  printf("Distancia de uma volta: %dm, Numero de voltas da corrida: %d\n", LAP_DIST, NR_LAP); 
  printf("Numero de equipas: %d\n", NR_TEAM);
  printf("Numero de carros por equipa: %d\n", NR_CARS);
  printf("Numero de unidades de tempo entre possivel avaria: %dut\n", MALFUNCTION_UNI_NR); 
  printf("Tempo min de reparacao: %dut, Tempo max dereparacao: %dut\n", MIN_REP, MAX_REP); 
  printf("Capacidade do deposito: %dL\n", FUEL_CAPACITY); 
  printf("--------------------------\n");
  printf("ut - unidades de tempo\n");
  printf("m - metros\n");
  printf("L - litros\n");
  printf("--------------------------\n\n\n");
#endif

  write_log("SERVER STARTED");
	
  initiate_resources();
	
	printf("\nI'm car 0 and my speed is : %f\n", shm_info->cars[0].speed);
	
  // create race manager process
  if ( !fork() ) race_manager();

  // create malfunction manager process
  if ( !fork() ) malfunction_manager();

  // wait for both process to finish
  for (i = 0; i < 2; i++) wait(NULL);
	
	
	printf("\nI'm car 0 again and now my speed is : %f\n", shm_info->cars[0].speed);
  // destroy shared mem and semaphores
  destroy_resources();

  write_log("SERVER CLOSED");
  exit(0);
}

void get_id(int *id, key_t key, size_t size, int flag){
	*id = shmget(key, size, flag);
	if (*id < 1){
		write_log("Error creating shm memory!");
		destroy_resources();
		exit(1);
	}
	
}

void initiate_shm(){
	// create shared mem key
  if( (shmkey = ftok(".", getpid()) ) == (key_t) -1 ){
    write_log("IPC error: ftok");
    exit(1);
  }

#ifdef DEBUG
  printf("Created SHM KEY = %d\n",shmkey);
#endif

  // create shared mem
  get_id(&shm_main_id, shmkey, sizeof(shm_struct) + sizeof(car_shm_struct)*NR_TEAM*NR_CARS, IPC_CREAT|IPC_EXCL|0700);
	
	// attatch mem
  shm_info = (shm_struct*) shmat(shm_main_id, NULL, 0);
	if (shm_info < (shm_struct*) 1){
		write_log("Error attaching memory!\n");
    	destroy_resources();
		exit(1);
	}
	
	get_id(&shm_boxes_id, IPC_PRIVATE, sizeof(shm_boxes) + sizeof(char)*NR_TEAM, IPC_CREAT|IPC_EXCL|0700);
	
	shm_boxes_state = (shm_boxes*) shmat(shm_boxes_id, NULL, 0);
	if (shm_boxes_state < (shm_boxes*) 1){
		write_log("Error attaching memory!\n");
    	destroy_resources();
		exit(1);
	}
	
}

void initiate_sems(){
	//create semaphores
  	sem_unlink("LOG_MUTEX");
	log_mutex = sem_open("LOG_MUTEX", O_CREAT|O_EXCL, 0700,1);
	
	if(log_mutex == SEM_FAILED){
		write_log("Failed to create the semaphore MUTEX");
    	destroy_resources();
    	exit(1);
	}
	
#ifdef DEBUG
	printf("Semaphore initialized\n");
#endif
}

void initiate_resources(){
	initiate_shm();
	
	initiate_sems();
}

void destroy_resources(void) {
  write_log("Cleaning up...");
  sem_close(log_mutex);
  sem_unlink("LOG_MUTEX");
  
  shmdt(shm_info);
  shmctl(shm_main_id, IPC_RMID, NULL);
  
  shmdt(shm_boxes_state);
  shmctl(shm_boxes_id, IPC_RMID, NULL);
}
