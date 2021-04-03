/*
Tomás Mendes - 2019232272
Joel Oliveira - 2019227468
*/

#include "write_output.h"


void write_log(char *line){
    time_t my_time;
    struct tm * timeinfo; 

    time(&my_time); //gets the curent time
    timeinfo = localtime(&my_time); //converts time into struct timeinfo

    sem_wait(log_mutex);
    fprintf(stdout, "[%d:%d:%d] %s\n", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, line);
    fprintf(f, "[%d:%d:%d] %s\n", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, line);
    sem_post(log_mutex);
}
