//Includes e definicoes etc, diria





/*Iniciar as cenas necessárias para a memória partilhada
	e os semaforos necessarios, a lista de threads etc */
	
//do ficheiro de configuracoes
int nr_unid_tps, dist_volta, nr_voltas, nr_equipa, unid_temp_avaria, min_reparacao, max_avaria, deposito; 
	
	
int main(int argc, char* argv[]){

	if (argc != 2){
		printf("informacao de erro na entrada");
		exit(0);
	}

	read_file(argv[1]);

	initiate_race();



	return 0;
}

//Le o ficheiro de configuracoes
void read_file(char * filename){
	FILE * f = fopen(filename, "r");
	
	
}
