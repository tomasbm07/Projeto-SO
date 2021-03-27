#include "team_manager.h"

void team_manager() {
  int i, car_thread_index=0;
  char box_state;  // 'F' - full; 'R' - reserved; 'A' - available
  

  // create array for car threads
  pthread_t *car_threads = create_threads_array();
  car_struct *car_stats = create_car_structs_array();
  
  // create car threads; just for initial testing
  for (i = 0; i < NR_CARS; i++) {
    init_car_stats((car_stats+i), i);
    pthread_create((car_threads + i), NULL, car, (car_stats+i));
  }

  // something

  // wait for threads to finish
  for (i = 0; i < NR_CARS; i++) pthread_join(*(car_threads + i), NULL);

  // delete the memory used by the array
  free(car_threads);
  free(car_stats);

  exit(0);
}

// allocates space for array with car threads
pthread_t *create_threads_array() {
  return (pthread_t *)malloc(sizeof(pthread_t) * NR_CARS);
}

car_struct * create_car_structs_array(){
    return (car_struct*)malloc(sizeof(car_struct) * NR_CARS);
}

// set the atributes of the car
void init_car_stats(car_struct *stats, int num) {
  stats->num = num;
  stats->speed = 0;
  stats->realiability = 0;
  stats->consuption = 0;
  stats->state = 'R';
  stats->fuel = 0;
  stats->laps_done = 0;
  stats->lap_distance = 0;
}
