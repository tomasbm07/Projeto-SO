#include "car.h"


// function to run in car thread
void *car(void *stats) {
  // convert argument from void* to car_struct*
  car_struct *car = (car_struct *)stats;

  // just a test
  printf("Car Number: %d\tFuel: %.2fL\n", car->num, car->fuel);

#ifdef DEBUG
  write_log("Thread car created!");
#endif

  pthread_exit(NULL);
}
