#ifndef CAR_H
#define CAR_H

#include "Race_Simulator.h"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>


typedef struct {
	short int state; // 0 = Corrida; 1 = Segurança; 2 = Box; 3 = Desistencia; 4 = Terminado; ? 
	float fuel;

} car_struct;


void *car();

#endif
