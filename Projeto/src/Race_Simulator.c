#include "Simulator.h"

int shm_main_id, shm_boxes_id;
shm_struct* shm_info;
shm_boxes* shm_boxes_state;
sem_t *(*sem_cars), *(*sem_box);


int main(int argc, char* argv[]) {
  int i;

  f = fopen("log.txt", "a");

  if (argc < 2) {
#ifdef DEBUG
    printf("No config file was passed!");
#endif
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

#ifdef DEBUG
  int j;
  for (i = 0; i < NR_TEAM; i++) {
    printf("TEAM: %d | ", i);
    for (j = 0; j < NR_CARS; j++) {
      printf("Car %d Speed: %.2f\t", shm_info->cars[i * NR_CARS + j].number,shm_info->cars[i * NR_CARS + j].speed);
    }
    printf("\n");
  }
#endif

  // create race manager process
  if (!fork()) race_manager();

  // create malfunction manager process
  if (!fork()) malfunction_manager();

  // wait for both process to finish
  for (i = 0; i < 2; i++) wait(NULL);

#ifdef DEBUG
  printf("After both processes are closed\n");
  for (i = 0; i < NR_TEAM; i++) {
    printf("TEAM: %d | ", i);
    for (j = 0; j < NR_CARS; j++) {
      printf("Car %d Speed: %.2f\t", shm_info->cars[i * NR_CARS + j].number,shm_info->cars[i * NR_CARS + j].speed);
    }
    printf("\n");
  }
#endif

  write_log("SERVER CLOSED");

  // destroy shared mem and semaphores
  destroy_resources();

  exit(0);
}

void get_id(int* id, key_t key, size_t size, int flag) {
  *id = shmget(key, size, flag);
  if (*id < 1) {
    write_log("Error creating shm memory!");
    destroy_resources();
    exit(1);
  }
}

void check_shmat( void* pointer){
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

  get_id(&shm_main_id, shmkey,sizeof(shm_struct) + sizeof(car_shm_struct) * NR_TEAM * NR_CARS, IPC_CREAT | IPC_EXCL | 0700);

  shm_info = (shm_struct*)shmat(shm_main_id, NULL, 0);
  check_shmat(shm_info);

  get_id(&shm_boxes_id, IPC_PRIVATE, sizeof(shm_boxes) + sizeof(char) * NR_TEAM, IPC_CREAT | IPC_EXCL | 0700);

  shm_boxes_state = (shm_boxes*)shmat(shm_boxes_id, NULL, 0);
  check_shmat(shm_boxes_state);
}

void create_sem(char * name, sem_t ** sem, int pos){
	sem_unlink(name);
	*(sem+pos) = sem_open(name, O_CREAT | O_EXCL, 0700, 1);
	if (*(sem+pos) == SEM_FAILED){
		write_log("Failed to create the semaphore MUTEX");
    	destroy_resources();
    	exit(1);
	}

}

void initiate_sems() {
  // create semaphores
  create_sem("LOG_MUTEX", &log_mutex, 0);
  
  sem_cars = (sem_t**) malloc( sizeof(sem_t*) * NR_CARS * NR_TEAM);
  int i;
  char str[50];
  for (i = 0; i<NR_CARS*NR_TEAM; i++){
  		sprintf(str, "CAR%d",i);
  		create_sem(str, sem_cars, i);
  }
  
  sem_box = (sem_t**) malloc( sizeof(sem_t*) * NR_TEAM);
  for(i = 0; i<NR_TEAM;i++){
  	sprintf(str, "BOX%d",i);
  	create_sem(str, sem_box, i);
  }
  
#ifdef DEBUG

  sprintf(str, "Semaphore initialized");
  write_log(str);
#endif
}

void initiate_resources() {
	initiate_sems();
  initiate_shm();
  
}

void destroy_resources(void) {
  write_log("Cleaning up...");
  fclose(f);

  sem_close(log_mutex);
  sem_unlink("LOG_MUTEX");
  
  int i = 0;
  char str[50];
  for (i = 0; i < NR_TEAM*NR_CARS; i++){
  	sprintf(str, "CAR%d", i);
  
  	sem_close(*(sem_cars+i));
  	sem_unlink(str);
  }
  
  for (i = 0; i < NR_TEAM; i++){
  	sprintf(str, "BOX%d", i);
  
  	sem_close(*(sem_box+i));
  	sem_unlink(str);
  }

  shmdt(shm_info);
  shmctl(shm_main_id, IPC_RMID, NULL);

  shmdt(shm_boxes_state);
  shmctl(shm_boxes_id, IPC_RMID, NULL);
  
  free(sem_cars);
  free(sem_box);
}
