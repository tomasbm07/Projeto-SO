#include "write_output.h"


void write_log(char *line){
    FILE *f;
    time_t my_time;
    struct tm * timeinfo; 

    time (&my_time); //gets the curent time
    timeinfo = localtime (&my_time); //converts time into struct timeinfo

    f = fopen("log.txt", "a");
    fprintf(stdout, "[%d:%d:%d] %s\n", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, line);
    fprintf(f, "[%d:%d:%d] %s\n", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, line);
    fclose(f);
}
