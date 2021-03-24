#include "write_output.h"

FILE *f;

void write_log(char *line){
    f = fopen("log.txt", "a");
    fprintf(f, line);
    fclose(f);
}
