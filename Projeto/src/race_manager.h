/*
Tomás Mendes - 2019232272
Joel Oliveira - 2019227468
*/

#ifndef RACE_H
#define RACE_H

#endif

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "team_manager.h"
#include "write_output.h"

#define DEBUG

int *fd_team;

void race_manager(void);
void signals(int signal);
void clean_resources();
int convert_to_int(char number[50]);
int check_pipe_command_regex(const char *pattern, char *string);
int car_number_exists(int number);
