#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "race_manager.h"
#include "team_manager.h"
#include "config_file.h"
#include "malfunction_manager.h"

void race_manager(){
	int i;
	
	printf("%d\n", nr_equipas);
	for (i = 0; i < nr_equipas; i++){
		if (!fork())
			team_manager(i);
	}
	
	if (!fork())
		malfunction_manager();
	
	for (i = 0; i <= nr_equipas; i++) wait(NULL);
	exit(0);
}
