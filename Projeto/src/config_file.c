#include "config_file.h"


#define MAX_CONFIG_LINE 23


void file_error() {
#ifdef DEBUG
  write_log("Error reading config file");
#endif
  exit(1);
}

void check_regex(const char *pattern, char *string) {
  regex_t re;
  if (regcomp(&re, pattern, REG_EXTENDED) != 0) {
#ifdef DEBUG
    write_log("Error verifying file structure");
#endif
    exit(1);
  }

  if (regexec(&re, string, 0, NULL, 0) != 0) file_error();

  regfree(&re);
}

void read_one_integer(char *string, int **int_value, FILE *file) {
  if (fgets(string, MAX_CONFIG_LINE, file) == NULL)
    file_error();
  else {
    check_regex("^[1-9][0-9]{0,9}\n$", string);
    sscanf(string, "%d\n", *int_value);
  }
}

void read_two_integer(char *string, int **int_value_1, int **int_value_2,
                      FILE *file) {
  if (fgets(string, MAX_CONFIG_LINE, file) == NULL)
    file_error();
  else {
    check_regex("^[1-9][0-9]{0,9}, [1-9][0-9]{0,9}\n$", string);
    sscanf(string, "%d, %d", *int_value_1, *int_value_2);
  }
}

// Le o ficheiro de configuracoes
void read_file(char *filename) {
  FILE *f;

  if ((f = fopen(filename, "r")) == NULL) {
#ifdef DEBUG
    write_log("File doesn't exist!");
#endif
    exit(1);
  }

  int *nr_1_input, *nr_2_input;
  nr_1_input = (int *)malloc(sizeof(int));
  nr_2_input = (int *)malloc(sizeof(int));

  // 23 = len(2**31)*2 + len(', ') + '\0'
  char *input_line = (char *)malloc(sizeof(char) * MAX_CONFIG_LINE);

  read_one_integer(input_line, &nr_1_input, f);
  nr_unid_tps = *(nr_1_input);

  read_two_integer(input_line, &nr_1_input, &nr_2_input, f);
  dist_volta = *(nr_1_input);
  nr_voltas = *(nr_2_input);

  read_one_integer(input_line, &nr_1_input, f);
  if (*(nr_1_input) < 3) {
#ifdef DEBUG
    write_log("At least 3 teams are required for race to start!");
#endif
    exit(1);
  }
  nr_equipas = *nr_1_input;

  read_one_integer(input_line, &nr_1_input, f);
  nr_carros = *nr_1_input;

  read_one_integer(input_line, &nr_1_input, f);
  unid_temp_avaria = *nr_1_input;

  read_two_integer(input_line, &nr_1_input, &nr_2_input, f);
  min_reparacao = *nr_1_input;
  max_reparacao = *nr_2_input;

  read_one_integer(input_line, &nr_1_input, f);
  deposito = *nr_1_input;

  free(nr_1_input);
  free(nr_2_input);
  free(input_line);
}
