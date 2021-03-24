#include "carro.h"
#include "config_file.h"
#include "malfunction_manager.h"
#include "race_manager.h"
#include "team_manager.h"
#include "write_output.h"

#include <semaphore.h>
// add includes

//global variables


void init_shm();
void terminate_shm();


int main(int argc, char* argv[]) {
  int i;

  if (argc != 2) {
    printf("No config file was passed!\n");
    exit(0);
  }

  read_file(argv[1]);

  /*
  printf("Configurações lidas do ficheiro:\n");
  printf("Numero de unidade de tempo /s: %dut\n", nr_unid_tps);
  printf("Distancia de uma volta: %dm, Numero de voltas da corrida: %d\n",dist_volta, nr_voltas);
  printf("Numero de equipas: %d\n", nr_equipas);
  printf("Numero de unidades de tempo entre possivel avaria: %dut\n", unid_temp_avaria); 
  printf("Tempo min de reparacao: %dut, Tempo max de reparacao: %dut\n", min_reparacao, max_reparacao); 
  printf("Capacidade dodeposito: %dL\n", deposito); 
  printf("--------------------------\n");
  printf("ut - unidades de tempo\n");
  printf("m - metros\n");
  printf("L - litros\n");
  printf("--------------------------\n\n\n");
  */
  
  //shared mem
  init_shm();
  //shared mem var
	
  //create race manager process
  if (!fork()) race_manager();

  //create malfunction manager
  if (!fork()) malfunction_manager();


  //wait for all process to finish
  for (i = 0; i < 2; i++) wait(NULL);

  char str[] = "teste123\n";

  write_log(str);


  //destroy shared mem and semaphores
	terminate_shm();
  exit(0);
}

void init_shm(){

}

void terminate_shm(){

}

