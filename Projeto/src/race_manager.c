/*
Tomás Mendes - 2019232272
Joel Oliveira - 2019227468
*/

#include "race_manager.h"

#define PIPE_NAME "/home/user/race_pipe"
//#define PIPE_NAME "race_pipe"


int fd_race_pipe;


void race_manager() {
  int i, num_chars, team;
  char str[256], aux[256];
  struct sigaction sa;

  sa.sa_handler = signals;
  //TODO: sigint é suposto ser recebido por race_simulator, nao por race_manager //
  sigaction(SIGINT, &sa, NULL);
  signal(SIGTSTP, SIG_IGN);

#ifdef DEBUG
  sprintf(aux, "Race manager created (PID: %d)", getpid());
  write_log(aux);
#endif

  // named pipe stuff
  unlink(PIPE_NAME);
  if (mkfifo(PIPE_NAME, O_CREAT | O_EXCL | 0666) < 0) {
    perror("Erro a criar o pipe: ");
    exit(-1);
  }

  if ((fd_race_pipe = open(PIPE_NAME, O_RDWR)) < 0) {
    perror("Erro: ");
    exit(1);
  }

  /*
  Comandos para testar o pipe:
  echo "ADDCAR TEAM: A, CAR: 77, SPEED: 50, COMSUMPTION: 0.04, RELIABILITY: 99"
  > race_pipe echo "START RACE" > race_pipe
  */
#ifdef DEBUG
  write_log("Named pipe 'race_pipe' is ready!\n");
#endif
  // while de leitura do pipe
  while (1) {
    num_chars = read(fd_race_pipe, str, sizeof(str));
    str[num_chars - 1] = '\0';  // put a \0 in the end of string

    if (strcmp(str, "START RACE") == 0) {
      write_log("[Race_Pipe] Got START RACE");
      write_log("Buckle Up, race is starting!");

    } else if (strcmp(str, "SKIP") == 0) {
      write_log("[Race_Pipe] Got SKIP");
      break;

    } else { 
    	if ( check_pipe_command_regex("^ADDCAR*", str) ){
    		write_log("[Race_Pipe] Got ADDCAR");
			if( check_pipe_command_regex("^ADDCAR TEAM: [a-zA-Z]{1,31}, CAR: (0?[1-9]|[1-9][0-9]), SPEED: [1-9][0-9]{0,9}, CONSUMPTION: ([0-9]*[.][0-9]{1,2}|[1-9][0-9]*), RELIABILITY: (100|[1-9][0-9]|[0-9])$", str) ) {
			// se nao for nenhum dos comandos acima, split da string do pipe
      			char **str_array = NULL;
      			int num_spaces = 0, j;
      			char *aux = strtok(str, " ");

      			while (aux) {
        			str_array = realloc(str_array, sizeof(char *) * ++num_spaces);

        			str_array[num_spaces - 1] = aux;
        			aux = strtok(NULL, ", ");
      			}
				
				if (!car_number_exists( atoi(str_array[4]) ) ) {
        			for (i = 0; i < num_spaces; ++i) printf ("str_array[%d] = %s\n", i, str_array[i]);
				
					for (i = 0; i < NR_TEAM * NR_CARS; i += NR_CARS)
						if ( strcmp(shm_info->cars[i].team_name, "")==0 || strcmp(shm_info->cars[i].team_name, str_array[2])==0)
							break;
				
					if (i >= NR_TEAM * NR_CARS)
						write_log("[ERROR] MAX NUMBER OF TEAMS ALREADY REACHED");
					else{
						for (j = 0; j < NR_CARS; j++)
							if ( strcmp(shm_info->cars[i+j].team_name, "")==0)
								break;
						if (j == NR_CARS)
							write_log("[ERROR] MAX NUMBER OF CARS, FROM TEAM REFERENCED, ALREADY REACHED");
						else{
							sprintf(shm_info->cars[i+j].team_name, "%s", str_array[2]);
							shm_info->cars[i+j].number = atoi(str_array[4]);
							shm_info->cars[i+j].speed = atoi(str_array[6]);
							shm_info->cars[i+j].consumption = atof(str_array[8]);
							shm_info->cars[i+j].reliability = atoi(str_array[10]);
						}
					}
				}else{
					write_log("[ERROR] CAR NUMBER ALREADY EXISTS");
				}
        	}else{
        		write_log("[Race_Pipe] ADDCAR SYNTAX ERROR DETECTED");
        	}
      	} else 
          write_log("[Race_Pipe] Unknown command");
      
    }
       
  }

  // unnamed pipes stuff - 1 pipe per team
  // unname pipe direction: race_manager -> team_manager
  // fd[0] = read; fd[1] = write
  fd_team = malloc(2 * NR_TEAM * sizeof(*fd_team));
  for (int i = 0; i < NR_TEAM; i++) pipe(&fd_team[i * 2]);

  for (i = 0; i < NR_TEAM; i++)
    if (!fork()) team_manager(i);

  // send a message to every team
  char teste[50];
  // sprintf(teste, "--Just a random test message--");
  sprintf(teste, "ADDCAR");
  for (i = 0; i < NR_TEAM; i++) {
    team = i;
    write(fd_team[team * 2 + 1], teste, strlen(teste) + 1);
  }

  // close unnecessary reading part
  for (int i = 0; i < NR_TEAM; i++) {
    close(fd_team[2 * i]);
  }

#ifdef DEBUG
  sprintf(str, "Created %d team processes", i);
  write_log(str);
#endif

  // wait for all team processes to finish
  for (i = 0; i < NR_TEAM; i++) wait(NULL);
  clean_resources();

  exit(0);
}

void clean_resources() {
  close(fd_race_pipe);
  unlink(PIPE_NAME);
  for (int i = 0; i < NR_TEAM; i++) close(fd_team[i * 2]);
}

void signals(int signal) {
  #ifdef DEBUG
    write_log("Got SIGINT\n");
  #endif
  clean_resources();
  exit(0);
}

int convert_to_int(char number[50]) {
    long int numl;
    int num = 0;
    char *tail;

    errno = 0;
    numl = strtol(number, &tail, 0);
    if (errno) 
        write_log("Error converting string to int");
    

    // verificar se existem carateres invalidos no resultado
    // strtol("123abc") -> 123; tail = "   " 
    while ((*tail) != 0) {
        if (!isspace(*tail)) {
            write_log("Error converting string to int");
            return -1;
        }
        tail++;
    }

    // converter de long para int
    if (numl <= INT_MAX && numl >= INT_MIN) 
      num = (int) numl;
    else
      write_log("Error converting string to int");

    return num;
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
