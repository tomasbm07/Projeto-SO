#ifndef SIMULATOR_H
#define SIMULATOR_H

#endif

#include <semaphore.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/fcntl.h>
#include <sys/wait.h>

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

void initiate_resources();
void initiate_shm();
void initiate_sems();
void destroy_resources();

