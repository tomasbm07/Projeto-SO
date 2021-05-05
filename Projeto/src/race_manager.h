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

int **fd_team;
extern int fd_race_pipe;
void race_manager(pid_t pid);
void signals(int signal);
void clean_resources();
int check_pipe_command_regex(const char *pattern, char *string);
int car_number_exists(int number);
int minium_cars();
int max(int fd1, int other_fds[][2]);
