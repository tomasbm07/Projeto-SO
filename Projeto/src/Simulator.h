/*
Tomás Mendes - 2019232272
Joel Oliveira - 2019227468
*/

#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <semaphore.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/fcntl.h>
#include <sys/wait.h>
#include <signal.h>

#include "race_manager.h"
#include "config_file.h"
#include "write_output.h"
#include "malfunction_manager.h"


#define DEBUG  // coment this this line to remove debug information

/*Debug stuff
#ifdef DEBUG
#endif
*/
key_t shmkey;
sem_t *log_mutex, *car_mutex;
FILE *f;

void initiate_resources();
void initiate_shm();
void initiate_sems();
void destroy_resources();
void statistics(int sig);
void create_named_pipe();
void end_race();

#endif
