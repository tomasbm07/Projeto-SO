/*
Tomás Mendes - 2019232272
Joel Oliveira - 2019227468
*/

#include "team_manager.h"

void team_manager(int team_index) {
  int i, car_thread_index = 0;

  pthread_t car_threads[NR_CARS];
  car_struct car_stats[NR_CARS];

  // create car threads; just for initial testing
  for (i = 0; i < NR_CARS; i++) {
    init_car_stats(&car_stats[i], team_index, i);
    pthread_create(&car_threads[i], NULL, car_worker, &car_stats[i]);
  }

  // something

  // wait for threads to finish
  for (i = 0; i < NR_CARS; i++) pthread_join(car_threads[i], NULL);

  exit(0);
}

// allocates space for array with car threads
pthread_t *create_threads_array() {
  return (pthread_t *)malloc(sizeof(pthread_t) * NR_CARS);
}

// allocates space for array with car struct atributes
car_struct *create_car_structs_array() {
  return (car_struct *)malloc(sizeof(car_struct) * NR_CARS);
}

// set the atributes of the car
void init_car_stats(car_struct *stats, int team_index, int car_index) {
  stats->car = &shm_info->cars[team_index + car_index];

  char nome[50];
  sprintf(nome,"OMEGALUL%d", team_index/NR_CARS);

  strcpy(stats->car->team_name, nome);
  stats->car->number = team_index * NR_CARS + car_index;
  stats->car->speed = 25;
  stats->car->consumption = 0;
  stats->car->reliability = 0;
  stats->car->laps_completed = 0;
  stats->car->box_stops_counter = 0;

  stats->state = 'R';
  stats->fuel = FUEL_CAPACITY;
  stats->lap_distance = 0;
}

// function to run in car thread
void *car_worker(void *stats) {
  // convert argument from void* to car_struct*
  car_struct *car_simu = (car_struct *)stats;

  // just a test
#ifdef DEBUG
	char str[50];
  sprintf(str, "Team: %s | Car Number: %d | Fuel: %.2fL", car_simu->car->team_name,car_simu->car->number,car_simu->fuel);
  write_log(str);
  write_log("Thread car created!");
#endif

  pthread_exit(NULL);
}
