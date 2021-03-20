//gcc -Wall FIcheiro_com_uma_estrutura_assim_por_alto.c -o executavel
//Includes e definicoes etc, diria

//includes for config_file.c
#include <stdio.h>
#include <stdlib.h>
#include <regex.h>

#define MAX_CONFIG_LINE 23

void file_error(void);
void read_file(char * filename);
void read_one_integer(char * string, int ** int_value, FILE * file);
void read_two_integer(char * string, int ** int_value_1, int** int_value_2, FILE * file);


/*Iniciar as cenas necessárias para a memória partilhada
	e os semaforos necessarios, a lista de threads etc */
		
//do ficheiro de configuracoes
int nr_unid_tps, dist_volta, nr_voltas, nr_equipas, unid_temp_avaria, min_reparacao, max_reparacao, deposito; 
	
	
void read_file(char * filename);

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

void file_error(){
	fprintf(stderr,"Erro na leitura do ficheiro de configuracoes.\n");
	exit(1);
}

void read_one_integer(char * string, int ** int_value, FILE * file){
	if (fgets(string, MAX_CONFIG_LINE, file)==NULL){
		file_error();
		}
	else{
		
		regex_t re;
		if ( regcomp(&re, "^[1-9][0-9]{0,9}\n$", REG_EXTENDED)!=0 ){
			printf("Erro na compilação");
			exit(1);
		}
		
		if ( regexec(&re, string, 0,NULL, 0) !=0 ) file_error();
		else sscanf(string, "%d\n", *int_value);
		
		regfree(&re);
	}
}

void read_two_integer(char * string, int ** int_value_1, int** int_value_2, FILE * file){
	if (fgets(string, MAX_CONFIG_LINE, file)==NULL){
		
		file_error();
		
		}
	else{
		regex_t re;
		if( regcomp(&re, "^[1-9][0-9]{0,9}, [1-9][0-9]{0,9}\n$", REG_EXTENDED) !=0 ){
			printf("Erro na compilação");
			exit(1);
		}
		
		if ( regexec(&re, string, 0 , NULL, 0)!=0 ) file_error();
		else sscanf(string, "%d, %d", *int_value_1, *int_value_2);
		
		regfree(&re);
	}
}
//Le o ficheiro de configuracoes
void read_file(char * filename){
	FILE * f = fopen(filename, "r");
	
	int * nr_1_input, * nr_2_input;
	nr_1_input = (int *)malloc(sizeof(int));
	nr_2_input = (int *)malloc(sizeof(int));
		
	//23 = len(2**31)*2 + len(', ') + '\0'
	char * input_line = (char *)malloc(sizeof(char)*MAX_CONFIG_LINE);
	
	read_one_integer(input_line, &nr_1_input, f);
	nr_unid_tps = *(nr_1_input);
	
	read_two_integer(input_line, &nr_1_input, &nr_2_input, f);
	dist_volta = *(nr_1_input);
	nr_voltas = *(nr_2_input);
	
	read_one_integer(input_line, &nr_1_input, f);
	if (*(nr_1_input)< 3){
		printf("Têm de existir, pelo menos, 3 equipas.\n");
		exit(0);
	}
	nr_equipas = *nr_1_input;
	
	read_one_integer(input_line, &nr_1_input, f);
	unid_temp_avaria = *nr_1_input;
	
	read_two_integer(input_line, &nr_1_input, &nr_2_input, f);
	min_reparacao = *nr_1_input;
	max_reparacao = *nr_2_input;
	
	read_one_integer(input_line, &nr_1_input, f);
	deposito = *nr_1_input;
	
	free(nr_1_input);
	free(nr_2_input);
	free(input_line);
}
