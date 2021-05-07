/*
Tomás Mendes - 2019232272
Joel Oliveira - 2019227468
*/

#include "race_manager.h"

pid_t *teams_pid;
void race_manager() {
	bool flag_first_start = true;
  	int i, num_chars, teams;
  	char str[256], aux[256];
	fd_set read_set;

	//terminar corrida--sigterm
	struct sigaction sa_term, sa_usr1;
	sigset_t term_mask, usr1_mask;
	
  	sa_term.sa_handler = terminate_teams;
  	sa_usr1.sa_handler = interrupt_race;
  	
  	sigemptyset(&term_mask);
  	sigaddset(&term_mask, SIGUSR1);
  	
  	sigemptyset(&usr1_mask);
  	sigaddset(&usr1_mask, SIGTERM);
  	
  	sa_term.sa_mask = term_mask;
  	sa_usr1.sa_mask = usr1_mask;
  	sa_term.sa_flags = 0;
  	sa_usr1.sa_flags = 0;
  	sigaction(SIGTERM, &sa_term, NULL);
  	//TODO interromper corrida--sigusr1
  	sigaction(SIGUSR1, &sa_usr1, NULL);
  	
  	signal(SIGTSTP, SIG_IGN);
  	signal(SIGINT, SIG_IGN);

#ifdef DEBUG
  	sprintf(aux, "Race manager created (PID: %d)", getpid());
  	write_log(aux);
#endif
	printf("RM PGID: %ld\n", (long)getpgid( getpid() ));

	while(1){
		num_chars = read(fd_race_pipe, str, sizeof(str));
    	str[num_chars - 1] = '\0';  // put a \0 in the end of string (that will be a \n)

    	if (strcmp(str, "START RACE") == 0) {
      		write_log("[Race_Pipe] Got START RACE");
			if ( minium_cars() ) {
				write_log("Buckle Up, race is starting!");
				
				if (flag_first_start){
					flag_first_start = false;
					//criar array para guardar os pid dos team_manager
					teams_pid=teams_pid_array();
				
  					// unnamed pipes stuff - 1 pipe per team
  					// unname pipe direction: race_manager <- team_manager
  					// fd[0] = read; fd[1] = write
					fd_team = (int**)malloc(NR_TEAM * sizeof(*fd_team));
  					for (i = 0; i < NR_TEAM; i++){ 
						fd_team[i] = (int *) malloc(2 * sizeof(**fd_team)); 
					}
				
					for (i = 0; i < NR_TEAM; i++){
						pipe(fd_team[i]);
    					if ( !(teams_pid[i]=fork()) ) {
    						for(int j = 0; j <= i; j++){
    							close(fd_team[j][0]);
    						}
    						team_manager(i);
    					}
						//fechar pipe de escrita
						close(fd_team[i][1]);
						#ifdef DEBUG
						sprintf(str, "Created team processes");
						write_log(str);
						#endif
					}
				
					//waiting for teams to send ready message
  					for (i = 0; i < NR_TEAM; i++) {
    					read(fd_team[i][0], str, 256);
						printf("MESSAGE received!! ----> '%s'\n", str);
  					}
					break;		
				}
			}else
				write_log("Not enough teams ready");
    	} else { 
    			if ( check_pipe_command_regex("^ADDCAR*", str) ) {
					if( check_pipe_command_regex("^ADDCAR TEAM: [a-zA-Z]{1,31}, CAR: (0?[1-9]|[1-9][0-9]), SPEED: [1-9][0-9]{0,9}, CONSUMPTION: ([0-9]*[.][0-9]{1,2}|[1-9][0-9]*), RELIABILITY: (100|[1-9][0-9]|[0-9])$", str) ) {
					// se addcar corretamente especificado -> split da string
      					char **str_array = NULL;
      					int num_spaces = 0, j;
      					char *aux = strtok(str, ":, ");
	
      					while (aux) {
        					str_array = realloc(str_array, sizeof(char *) * ++num_spaces);
	
        					str_array[num_spaces - 1] = aux;
        					aux = strtok(NULL, ":, ");
      					}
						//se numero de carro nao existe, vai adicionar
						if (!car_number_exists( atoi(str_array[4]) ) ) {
							//percorre equipas até encontrar equipa com o mesmo nome ou slot vazio para nova equipa
							for (i = 0; i < NR_TEAM * NR_CARS; i += NR_CARS)
								if ( strcmp(shm_info->cars[i].team_name, "")==0 || strcmp(shm_info->cars[i].team_name, str_array[2])==0)
									break;
					
							if (i == NR_TEAM * NR_CARS)
								write_log("[ERROR] MAX NUMBER OF TEAMS ALREADY REACHED");
							else{
								//percorre carros da equipa até encontrar slot vazio para novo carro
								for (j = 0; j < NR_CARS; j++)
									if ( strcmp(shm_info->cars[i+j].team_name, "")==0 )
										break;
								if (j == NR_CARS)
									write_log("Max number of cars reached for this team");
								else{
									strcpy(shm_info->cars[i+j].team_name,str_array[2]);
									sscanf(str_array[4], "%d", &shm_info->cars[i+j].number);
									sscanf(str_array[6], "%d", &shm_info->cars[i+j].speed);
									sscanf(str_array[8], "%f", &shm_info->cars[i+j].consumption);
									sscanf(str_array[10], "%d", &shm_info->cars[i+j].reliability);
									
									sprintf(str, "ADDED CAR %s FROM TEAM %s", str_array[4], shm_info->cars[i+j].team_name);
									write_log(str);
								}
							}
					}else
						write_log("Car number already exists");
        		}else{
        			write_log("ADDCAR Syntax error");
        		}
        	}else 
         		write_log("[Race_Pipe] Unknown command");      	
		}  	
	}
	
	usleep(1000);
	kill(cpid[0], SIGUSR2);
	for (i = 0; i <NR_TEAM;i++)
		kill(teams_pid[i], SIGUSR2);


//TODO MULTIPLEXING ENTRE NAMED PIPE E UNNAMED PIPES DOS CARROS ALGURES AQUI PARA BAIXO
	/*while (1) {
  		FD_ZERO(&read_set);
  		FD_SET(fd_race_pipe, &read_set);

  		for (i = 0; i < NR_TEAM;)
  			FD_SET(fd_team[i++][0], &read_set);
  		
  		if (select(max(fd_race_pipe, fd_team)+1, &read_set, NULL, NULL, NULL)>0) {
    		if (FD_ISSET(fd_race_pipe, &read_set)) {

    		}
  		}
	}
	*/



  	// wait for all team processes to finish
  	for (i = 0; i < NR_TEAM; i++) wait(NULL);
  	
  	for (int i = 0; i < NR_TEAM; i++) close(fd_team[i][0]);

  	exit(0);
}

void interrupt_race(int sig){
	//TODO sinalizar team_managers de interrupção da corrida 
	write_log("INTERRUPTING RACE");
	if (teams_pid!=NULL)
		for (int i = 0; i < NR_TEAM; i++) 
			kill(teams_pid[i], SIGUSR1);
}

void terminate_teams(int signal) {
		#ifdef DEBUG
    	write_log("[Race Manager]Got SIGTERM");
		write_log("Race Manager waiting for race to end");
  		#endif
  		int i;
  		//TODO sinalizar team_manager de fim da corrida.
  		if (teams_pid!=NULL)
  			for (i = 0; i < NR_TEAM; i++) 
  				kill(teams_pid[i], SIGTERM);
  		for (i = 0; i < NR_TEAM; i++) wait(NULL);
  		for (i = 0; i < NR_TEAM; i++) close(fd_team[i][0]);
  		free(teams_pid);
  		exit(0);
}


int check_pipe_command_regex(const char *pattern, char *string) {
  	regex_t re;
  	if (regcomp(&re, pattern, REG_EXTENDED) != 0)
    	return 0;
  
  	if (regexec(&re, string, 0, NULL, 0) != 0){
  		regfree(&re);
  		return 0;
  	}
  	regfree(&re);
 	 return 1;
}


int car_number_exists(int number){
	int i;
	for (i = 0; i < NR_TEAM * NR_CARS; i ++)
		if (shm_info->cars[i].number==number)
			return 1;
	return 0;
}


int minium_cars(){
	int i;
	int count = 0;
	// loop through teams, if team doesnt have name, doesnt exist
	for (i = 0; i < NR_TEAM * NR_CARS; i+= NR_CARS)
		if ( strcmp(shm_info->cars[i].team_name, "") != 0)
			count++;
	
	return count == NR_TEAM;
}


int max(int fd1, int other_fds[][2]){
	int max = fd1, i;
	for (i = 0; i < NR_TEAM; i++)
		if (other_fds[i][0]>max)
			max = other_fds[i][0];
	return max;
}

pid_t * teams_pid_array(){
	return (pid_t*) malloc(sizeof(pid_t)*NR_TEAM);
}
