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

#include "car.h"
#include "config_file.h"
#include "malfunction_manager.h"
#include "race_manager.h"
#include "team_manager.h"
#include "write_output.h"

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

#endif
