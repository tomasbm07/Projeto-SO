#include "car.h"



//function to run in car thread
void *car(void){

    #ifdef DEBUG
    write_log("Thread car created!");
    #endif

    pthread_exit(NULL);
}