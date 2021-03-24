#include "write_output.h"

FILE *f;

void write_log(char *line){
    f = fopen("log.txt", "a");
    fwrite(line, 1, sizeof(line), f);
    fclose(f);
}
