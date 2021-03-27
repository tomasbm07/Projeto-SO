#include "race_manager.h"

void race_manager() {
  int i;

  for (i = 0; i < NR_TEAM; i++)
    if (fork() == 0) team_manager();

#ifdef DEBUG
  char str[50];
  sprintf(str, "Created %d team processes", i);
  write_log(str);
#endif

  // wait for all team processes to finish
  for (i = 0; i < NR_TEAM; i++) wait(NULL);

  exit(0);
}
