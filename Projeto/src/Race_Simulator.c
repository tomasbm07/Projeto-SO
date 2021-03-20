#include "config_file.h"
// add includes

int main(int argc, char* argv[]) {
  if (argc != 2) {
    printf("informacao de erro na entrada");
    exit(0);
  }

  read_file(argv[1]);

  printf("%d\n", nr_unid_tps);
  printf("%d, %d\n", dist_volta, nr_voltas);
  printf("%d\n", nr_equipas);
  printf("%d\n", unid_temp_avaria);
  printf("%d, %d\n", min_reparacao, max_reparacao);
  printf("%d\n", deposito);
  // initiate_race();
  exit(0);
}