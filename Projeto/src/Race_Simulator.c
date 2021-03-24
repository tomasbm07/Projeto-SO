#include "Race_Simulator.h"
// add includes

// global variables

int main(int argc, char* argv[]) {
  int i;

  if (argc < 2) {
#ifdef DEBUG
    write_log("No config file was passed!");
#endif
    exit(1);
  } else if (argc >= 3) {
#ifdef DEBUG
    write_log("Program only takes config filename as argument");
#endif
    exit(1);
  }

  // read config file
  read_file(argv[1]);

#ifdef DEBUG
  write_log("Successfully read config file");
#endif

#ifdef DEBUG
  printf("Configurações lidas do ficheiro:\n");
  printf("Numero de unidade de tempo /s: %dut\n", nr_unid_tps);
  printf("Distancia de uma volta: %dm, Numero de voltas da corrida: %d\n",dist_volta, nr_voltas); 
  printf("Numero de equipas: %d\n", nr_equipas);
  printf("Numero de unidades de tempo entre possivel avaria: %dut\n", unid_temp_avaria); 
  printf("Tempo min de reparacao: %dut, Tempo max dereparacao: %dut\n", min_reparacao, max_reparacao); 
  printf("Capacidade do deposito: %dL\n", deposito); 
  printf("--------------------------\n");
  printf("ut - unidades de tempo\n");
  printf("m - metros\n");
  printf("L - litros\n");
  printf("--------------------------\n\n\n");
#endif


  // shared mem
  init_shm();
  // shared mem var

  // create race manager process
  if (!fork()) race_manager();

  // create malfunction manager process
  if (!fork()) malfunction_manager();

  // wait for both process to finish
  for (i = 0; i < 2; i++) wait(NULL);

  // destroy shared mem and semaphores
  terminate_shm();

  write_log("Program is done!");
  exit(0);
}

void init_shm() {}

void terminate_shm() {}
