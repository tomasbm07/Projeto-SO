/*
Tomás Mendes - 2019232272
Joel Oliveira - 2019227468
*/

#include "malfunction_manager.h"


void malfunction_manager(){

#ifdef DEBUG
char str[50];
	sprintf(str,"Malfunction manager, process %d, created",(int)getpid());
	write_log(str);
#endif

	exit(0);
}
