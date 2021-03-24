#include "race_manager.h"

void race_manager(){
	int i;
	
	printf("%d\n", nr_equipas);
	for (i = 0; i < nr_equipas; i++){
		if (!fork())
			team_manager(i);
	}
	
	for (i = 0; i < nr_equipas; i++)
		wait(NULL);

	exit(0);
}
