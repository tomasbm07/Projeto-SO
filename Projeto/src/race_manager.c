/*
Tomás Mendes - 2019232272
Joel Oliveira - 2019227468
*/

#include "race_manager.h"

void race_manager() {
  int i;

  for (i = 0; i < NR_TEAM; i++)
    if ( !fork() ) team_manager( i*NR_CARS );

#ifdef DEBUG
  char str[50];
  sprintf(str, "Created %d team processes", i);
  write_log(str);
#endif

  // wait for all team processes to finish
  for (i = 0; i < NR_TEAM; i++) wait(NULL);

  exit(0);
}
