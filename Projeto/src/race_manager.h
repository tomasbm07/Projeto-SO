#ifndef RACE_H
#define RACE_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "Race_Simulator.h"
#include "config_file.h"
#include "race_manager.h"
#include "team_manager.h"
#include "write_output.h"

void race_manager(void);

#endif
