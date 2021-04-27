/*
Tomás Mendes - 2019232272
Joel Oliveira - 2019227468
*/

#include "race_manager.h"

#define PIPE_NAME "/home/user/race_pipe"
//#define PIPE_NAME "race_pipe"

int fd_race_pipe;

void race_manager() {
  int i, num_chars;
  fd_set read_set;
  char str[256];
  struct sigaction sa;

  sa.sa_handler = signals;
  sigaction(SIGINT, &sa, NULL);
  signal(SIGTSTP, SIG_IGN);

#ifdef DEBUG
  char aux[50];
  sprintf(aux,"Race manager created (PID: %d)", getpid());
  write_log(aux);
#endif

  //named pipe stuff
  unlink(PIPE_NAME);
  if (mkfifo(PIPE_NAME, O_CREAT | O_EXCL | 0666) < 0) {
    perror("Erro a criar o pipe: ");
    exit(-1);
  }

  if ((fd_race_pipe = open(PIPE_NAME, O_RDWR)) < 0) {
    perror("Erro: ");
    exit(1);
  }

  //unnamed pipes stuff - 1 pipe per team
  //unname pipe direction: race_manager -> team_manager
  //fd[0] = read; fd[1] = write
  fd_team = malloc(2 * NR_TEAM * sizeof(int));
  for (int i = 0; i < NR_TEAM; i++){
    pipe(&fd_team[i*2]);
    close(fd_team[i*2]); // close the read part for race_manager
  }

  
  /*
  Comandos para testar o pipe:
  echo "ADDCAR TEAM: A, CAR: 77, SPEED: 50, COMSUMPTION: 0.04, RELIABILITY: 99" > race_pipe
  echo "START RACE" > race_pipe
  */

  write_log("Named pipe 'race_pipe' is ready!\n");
  // while de leitura do pipe
  while (1) {
    FD_ZERO(&read_set);
    FD_SET(fd_race_pipe, &read_set);

    if (select(fd_race_pipe + 1, &read_set, NULL, NULL, NULL) > 0) {
      if (FD_ISSET(fd_race_pipe, &read_set)) {
        num_chars = read(fd_race_pipe, str, sizeof(str));
        str[num_chars - 1] = '\0';  // put a \0 in the end of string

        //split da string que vem do pipe
        /*
        char *aux = strtok(str, " ");
        while (aux != NULL){
          printf("%s\n", aux);
          aux = strtok(NULL, ",");
        }
        */
        
        //TODO: se der split primeiro, o srtcmp so ve "START" se o comando for "START RACE"
        if (strcmp(str, "ADDCAR") == 0){
          write_log("[Race_Pipe] Got ADDCAR");
        }
        else if (strcmp(str, "123") == 0){
          write_log("[Race_Pipe] Got 123");
        }
        else if (strcmp(str, "START RACE") == 0){
          write_log("[Race_Pipe] Got START RACE");
          write_log("Buckle Up, race is starting!");
        }
        else if (strcmp(str, "SKIP") == 0){
          write_log("[Race_Pipe] Got SKIP");
          break;
        }
        else
          printf("unknown command\n");
      }
    }
  }

  for (i = 0; i < NR_TEAM; i++)
    if (!fork()) team_manager(i * NR_CARS);

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
}

void signals(int signal){
#ifdef DEBUG
  write_log("Got SIGINT\n");
#endif
  clean_resources();
  exit(0);
}
