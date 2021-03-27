#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <semaphore.h>

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

void init_shm();
void terminate_shm();

#endif
