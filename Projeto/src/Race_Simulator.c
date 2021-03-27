#include "Race_Simulator.h"


// global variables

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

  write_log("Server started");

  // create shared mem
  init_shm();

  // initialize shared mem structs/var

  // create race manager process
  if ( !fork() ) race_manager();

  // create malfunction manager process
  if ( !fork() ) malfunction_manager();

  // wait for both process to finish
  for (i = 0; i < 2; i++) wait(NULL);

  // destroy shared mem and semaphores
  terminate_shm();

  write_log("Server closed");
  exit(0);
}

void init_shm() {}

void terminate_shm() {}
