/*
Tomás Mendes - 2019232272
Joel Oliveira - 2019227468
*/

#ifndef MALFUNCTION_H
#define MALFUNCTION_H


#include "write_output.h"
#include "config_file.h"

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/msg.h>

int mqid; // Message queue ID

//define the message struct for the MQ
//typedef struct malfunction_msg *msg;

void malfunction_manager(void);
void malf_term_handler(int sig);
void generator();
void cleanup();
void create_mq();

#endif
