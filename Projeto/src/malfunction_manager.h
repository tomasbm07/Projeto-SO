/*
Tomás Mendes - 2019232272
Joel Oliveira - 2019227468
*/

#ifndef MALFUNCTION_H
#define MALFUNCTION_H

#endif

#include "write_output.h"
#include "config_file.h"

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

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

typedef struct{
  long mtype;
  int car_num;
} malfunction_msg;


void malfunction_manager(void);
void malfunction_signals(int sig);
void generator();
