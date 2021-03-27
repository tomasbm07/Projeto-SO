#include "car.h"




//function to run in car thread
void *car(void* stats){
    car_struct *car_stats = (car_struct*) stats;
    
    //just a test
    printf("Car Number: %d\n", car_stats->num);

    #ifdef DEBUG
    write_log("Thread car created!");
    #endif


    pthread_exit(NULL);
}