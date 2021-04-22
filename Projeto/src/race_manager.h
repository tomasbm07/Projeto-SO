/*
Tomás Mendes - 2019232272
Joel Oliveira - 2019227468
*/

#ifndef RACE_H
#define RACE_H

#endif

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <signal.h>

#include "team_manager.h"
#include "write_output.h"

#define DEBUG

void race_manager(void);
void signals(int signal);


