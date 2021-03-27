#include "team_manager.h"

void team_manager() {
  int i;
  char box_state;  // 'F' - full; 'R' - reserved; 'A' - available
  car_struct *car_stats;

  // create array for car threads
  pthread_t *car_threads = create_threads_array();

  // create car threads
  for (i = 0; i < NR_CARS; i++) {
    car_stats = malloc(sizeof(car_struct));

    set_car_stats(car_stats);

    pthread_create((car_threads + i), NULL, car, (void *)car_stats);
  }

  // something

  // wait for threads to finish
  for (i = 0; i < NR_CARS; i++) pthread_join(*(car_threads + i), NULL);

  // delete the memory used by the array
  free(car_threads);

  exit(0);
}

// allocates space for array with car threads
pthread_t *create_threads_array() {
  return (pthread_t *)malloc(sizeof(pthread_t) * NR_CARS);
}

// set the atributes of the car
void set_car_stats(car_struct *stats) { stats->num = 20; }
