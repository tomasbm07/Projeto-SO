#include "config_file.h"

//do ficheiro de configuracoes
int nr_unid_tps, dist_volta, nr_voltas, nr_equipas, unid_temp_avaria, min_reparacao, max_reparacao, deposito; 

int main(int argc, char* argv[]){

	if (argc != 2){
		printf("informacao de erro na entrada");
		exit(0);
	}
	
	read_file(argv[1]);
	
	printf("%d\n%d, %d\n%d\n%d\n%d, %d\n%d\n", nr_unid_tps, dist_volta, nr_voltas, nr_equipas, unid_temp_avaria, min_reparacao, max_reparacao, deposito);
	//initiate_race();
	return 0;
}