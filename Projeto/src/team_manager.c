/*
Tomás Mendes - 2019232272
Joel Oliveira - 2019227468
*/

#include "team_manager.h"

pthread_mutex_t box_mutex = PTHREAD_MUTEX_INITIALIZER;
char box_state;

void team_manager(int team_index) {
  int i;
  char str[150];

#ifdef DEBUG
  char aux[50];
  sprintf(aux, "Team manager created (PID: %d)", getpid());
  write_log(aux);
#endif

  // close unnecessary writing part
  close(fd_team[2 * team_index + 1]);

   for (int i = 0; i < NR_TEAM; i++) {
    if (i == team_index) continue;
    close(fd_team[2 * i]);
  }
  

  // int car_thread_index = 0;
  box_state = 'E';  // 'R' = Reserved; 'E' = Empty; 'F' = Full;
  srand((unsigned)team_index);
  pthread_t car_threads[NR_CARS];
  car_struct car_stats[NR_CARS];

  char pipe_str[50];
  read(fd_team[2 * team_index], &pipe_str, 50);
#ifdef DEBUG
  sprintf(str, "Team %d got \"%s\" from unnamed pipe\n", team_index, pipe_str);
  write_log(str);
#endif

  // Create car thread when message from pipe is correct
  if (strcmp(pipe_str, "ADDCAR") == 0){
    init_car_stats(&car_stats[0], team_index, 0);
    pthread_create(&car_threads[0], NULL, car_worker, &car_stats[0]);
  }

  /*
  for (i = 0; i < NR_CARS; i++) {
    init_car_stats(&car_stats[i], team_index, i);
    pthread_create(&car_threads[i], NULL, car_worker, &car_stats[i]);
  }
  */

  // wait for threads to finish
  for (i = 0; i < NR_CARS; i++) pthread_join(car_threads[i], NULL);

#ifdef DEBUG
  sprintf(str, "Process %d Car State: %c", getpid(), box_state);
  write_log(str);
#endif

  // destroy mutex
  pthread_mutex_destroy(&box_mutex);

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

// set the atributes of the car that aren't set by race_manager
void init_car_stats(car_struct *stats, int team_index, int car_index) {
  stats->car = &shm_info->cars[team_index + car_index];

  stats->car->laps_completed = 0;
  stats->car->box_stops_counter = 0;

  stats->state = 'R';
  stats->fuel = FUEL_CAPACITY;
  stats->lap_distance = 0;
}

// function to run in car thread
void *car_worker(void *stats) {

  // convert argument from void* to car_struct*
  car_struct *car_info = (car_struct *)stats;

  // just for testing
#ifdef DEBUG
  char str[300];
  sprintf(str,
          "I am Thread %d, from process %d\n\t"
          "Team: %s | Car Number: %d | Fuel: %.2fL | Box State: %c",
          (unsigned int)pthread_self(), (unsigned int)getpid(),
          car_info->car->team_name, car_info->car->number, car_info->fuel,
          box_state);
  //write_log(str);
  write_log("Thread car created!");

  char states[3] = {'E', 'F', 'R'};
  pthread_mutex_lock(&box_mutex);
  box_state = states[rand() % 3];
  pthread_mutex_unlock(&box_mutex);
#endif

  pthread_exit(NULL);
}
