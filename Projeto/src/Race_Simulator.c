#include "config_file.h"
// add includes

int main(int argc, char* argv[]) {
  if (argc != 2) {
    printf("No config file was passed!\n");
    exit(0);
  }

  read_file(argv[1]);
  printf("Configurações lidas do ficheiro:\n");
  printf("Numero de unidade de tempo /s: %dut\n", nr_unid_tps);
  printf("Distancia de uma volta: %dm, Numero de voltas da corrida: %d\n", dist_volta, nr_voltas);
  printf("Numero de equipas: %d\n", nr_equipas);
  printf("Numero de unidades de tempo entre possivel avaria: %dut\n", unid_temp_avaria);
  printf("Tempo min de reparacao: %dut, Tempo max de reparacao: %dut\n", min_reparacao, max_reparacao);
  printf("Capacidade do deposito: %dL\n", deposito);
  printf("--------------------------\n");
  printf("ut - unidades de tempo\n");
  printf("m - metros\n");
  printf("L - litros\n");
  printf("--------------------------\n");

  

  // initiate_race();
  exit(0);
}