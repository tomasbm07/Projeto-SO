#ifndef CAR_H
#define CAR_H

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "team_manager.h"
#include "write_output.h"

void* car(void* stats);

#endif
