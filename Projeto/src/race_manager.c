/*
Tomás Mendes - 2019232272
Joel Oliveira - 2019227468
*/

#include "race_manager.h"

#define PIPE_NAME "/home/user/race_pipe"

void clean_resources();

int fd_race_pipe;

void race_manager() {
  int i, num_chars;
  fd_set read_set;
  char str[256];
  struct sigaction sa;

  sa.sa_handler = signal_handler;
  sigaction(SIGINT, &sa, NULL);


  unlink(PIPE_NAME);
  if (mkfifo(PIPE_NAME, O_CREAT | O_EXCL | 0666) < 0) {
    perror("Erro a criar o pipe: ");
    exit(-1);
  }

  if ((fd_race_pipe = open(PIPE_NAME, O_RDWR)) < 0) {
    perror("Erro: ");
    exit(1);
  }

  printf("Listening to race_pipe!\n");
  // while de leitura do pipe
  while (1) {
    FD_ZERO(&read_set);
    FD_SET(fd_race_pipe, &read_set);

    if (select(fd_race_pipe + 1, &read_set, NULL, NULL, NULL) > 0) {
      if (FD_ISSET(fd_race_pipe, &read_set)) {
        num_chars = read(fd_race_pipe, str, sizeof(str));
        str[num_chars - 1] = '\0';  // put a \0 in the end of string

        if (strcmp(str, "hehexD") == 0)
          printf("got 'em bois\n");
        else if (strcmp(str, "123") == 0)
          printf("Got 123\n");
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

void signal_handler(int signal){
#ifdef DEBUG
  write_log("Got SIGINT\n");
#endif
  clean_resources();
  exit(0);
}
