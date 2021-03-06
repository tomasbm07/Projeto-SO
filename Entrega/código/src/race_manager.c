/*
Tomás Mendes - 2019232272
Joel Oliveira - 2019227468
*/

#include "race_manager.h"


pid_t *teams_pid;
pid_t malfunction_pid;

void race_manager(pid_t malf_pid) {
    malfunction_pid = malf_pid;
    bool race_started = false;
    
    int i = 0, num_chars, wait_status, car_num;
    char str[256], aux[256], from_car_pipe[10];
    fd_set read_set;

    //terminar corrida--SIGTERM
    struct sigaction sa_term, sa_usr1;
    sigset_t term_mask, usr1_mask;

    sa_term.sa_handler = terminate_teams;
    sa_usr1.sa_handler = interrupt_race;
    
    sigfillset(&term_mask);
    
    sigemptyset(&usr1_mask);
    sigaddset(&usr1_mask, SIGTERM);
    
    sa_term.sa_handler = terminate_teams;
    sa_usr1.sa_handler = interrupt_race;
      
    sigfillset(&term_mask);
    
    sigemptyset(&usr1_mask);
    sigaddset(&usr1_mask, SIGTERM);
    
    sa_term.sa_mask = term_mask;
    sa_usr1.sa_mask = usr1_mask;
    
    sa_term.sa_flags = 0;
    sa_usr1.sa_flags = 0;
    
    sigaction(SIGTERM, &sa_term, NULL);
    sigaction(SIGUSR1, &sa_usr1, NULL);
    
    signal(SIGTSTP, SIG_IGN);
    signal(SIGINT, SIG_IGN);

    #ifdef DEBUG
        sprintf(aux, "Race manager created (PID: %d)", getpid());
        write_log(aux);
    #endif
    
    while(1){
        if (!race_started){
            num_chars = read(fd_race_pipe, str, sizeof(str));
            str[num_chars - 1] = '\0';  // put a \0 in the end of string (that will be a \n)

            if (strcmp(str, "START RACE") == 0) {
                write_log("[Race_Pipe] Got START RACE");
                if ( minium_cars() ) {
                    write_log("Buckle Up, race is starting!");
                    race_started = true;
                    
                    
                    //criar array para guardar os pid dos team_manager
                    teams_pid = teams_pid_array();
                
                    // unnamed pipes stuff - 1 pipe per team
                    // unname pipe direction: race_manager <- team_manager
                    // fd[0] = read; fd[1] = write
                    fd_team = (int**)malloc(NR_TEAM * sizeof(*fd_team));
                    for (i = 0; i < NR_TEAM; i++){ 
                        fd_team[i] = (int *) malloc(2 * sizeof(**fd_team)); 
                    }
                
                    for (i = 0; i < NR_TEAM; i++){
                        pipe(fd_team[i]);
                        if ( !(teams_pid[i]=fork()) ) {
                            for(int j = 0; j <= i; j++){
                                close(fd_team[j][0]);
                            }
                            team_manager(i);
                        }
       
                        //fechar parte de escrita dos pipes
                        close(fd_team[i][1]);
                        #ifdef DEBUG
                        sprintf(str, "Created team processes");
                        write_log(str);
                        #endif
                    }
                
                    //waiting for teams to send ready message
                    for (i = 0; i < NR_TEAM; i++) {
                        read(fd_team[i][0], str, 256);
                        printf("MESSAGE received!! ----> '%s'\n", str);
                    }

                    //usleep(1000);
                    kill(cpid[0], SIGUSR2);
                    for (i = 0; i <NR_TEAM;i++)
                        kill(teams_pid[i], SIGUSR2);
                    
                    write_log("---RACE HAS STARTED---");
                    race_going = true;

                    //break;		
                }else
                    write_log("Not enough teams ready");
            } else {
                    if ( check_pipe_command_regex("^ADDCAR*", str) ) {
                        if( check_pipe_command_regex("^ADDCAR TEAM: [a-zA-Z]{1,31}, CAR: (0?[1-9]|[1-9][0-9]), SPEED: [1-9][0-9]{0,9}, CONSUMPTION: ([0-9]*[.][0-9]{1,2}|[1-9][0-9]*), RELIABILITY: (100|[1-9][0-9]|[0-9])$", str) ) {
                            // se addcar corretamente especificado -> split da string
                            char **str_array = NULL;
                            int num_spaces = 0, j;
                            char *aux = strtok(str, ":, ");
        
                            while (aux) {
                                str_array = realloc(str_array, sizeof(char *) * ++num_spaces);
        
                                str_array[num_spaces - 1] = aux;
                                aux = strtok(NULL, ":, ");
                            }
                            //se numero de carro nao existe, vai adicionar
                            if (!car_number_exists( atoi(str_array[4]) ) ) {
                                //percorre equipas até encontrar equipa com o mesmo nome ou slot vazio para nova equipa
                                for (i = 0; i < NR_TEAM * NR_CARS; i += NR_CARS)
                                    if ( strcmp(shm_info->cars[i].team_name, "") == 0 || strcmp(shm_info->cars[i].team_name, str_array[2]) == 0)
                                        break;
                        
                                if (i == NR_TEAM * NR_CARS)
                                    write_log("[ERROR] MAX NUMBER OF TEAMS ALREADY REACHED");
                                else{
                                    //percorre carros da equipa até encontrar slot vazio para novo carro
                                    for (j = 0; j < NR_CARS; j++)
                                        if ( strcmp(shm_info->cars[i+j].team_name, "") == 0 )
                                            break;
                                    if (j == NR_CARS)
                                        write_log("Max number of cars reached for this team");
                                    else{
                                        strcpy(shm_info->cars[i+j].team_name,str_array[2]);
                                        sscanf(str_array[4], "%d", &shm_info->cars[i+j].number);
                                        sscanf(str_array[6], "%d", &shm_info->cars[i+j].speed);
                                        sscanf(str_array[8], "%f", &shm_info->cars[i+j].consumption);
                                        sscanf(str_array[10], "%d", &shm_info->cars[i+j].reliability);
                                        
                                        sprintf(str, "ADDED CAR %s FROM TEAM %s", str_array[4], shm_info->cars[i+j].team_name);
                                        write_log(str);
                                    }
                                }
                        } else
                            write_log("Car number already exists");
                    }else{
                        write_log("ADDCAR Syntax error");
                    }
                } else 
                    write_log("[Race_Pipe] Unknown command");      	
            }


        } else { // after race has started
            //printf("------------------------------------------------------------------");
            FD_ZERO(&read_set);
            FD_SET(fd_race_pipe, &read_set);
            for (i = 0; i < NR_TEAM;i++)
                FD_SET(fd_team[i][0], &read_set);
            
            if (select(max(fd_race_pipe, fd_team) + 1, &read_set, NULL, NULL, NULL) > 0) {
                if (FD_ISSET(fd_race_pipe, &read_set)) {
                    num_chars = read(fd_race_pipe, str, sizeof(str));
                    str[num_chars - 1] = '\0';  // put a \0 in the end of string
                    
                    if (strcmp(str, "START RACE") == 0){
                        write_log("[Race_Pipe] Got START RACE");
                        if (!race_going){
                        race_going = true;
                            sigaction(SIGUSR1, &sa_usr1, NULL);
                            write_log("[Race_Pipe] Restarting race!");
                            for (int i = 0; i < NR_TEAM; i++)
                                kill(teams_pid[i], SIGUSR1);
                        } else {
                            write_log("[Race_Pipe] Race already going!");
                        }
                    } else if(check_pipe_command_regex("^ADDCAR*", str)){
                        write_log("[Race_Pipe] Got ADDCAR");
                        write_log("[Race_Pipe] Can't add cars once the race has started!");
                    }
                    else
                        write_log("[Race_Pipe] Unknown command");
                }

                for (i = 0; i < NR_TEAM; i++) {
                    if (FD_ISSET(fd_team[i][0], &read_set)) {
                        while( (num_chars = read(fd_team[i][0], from_car_pipe, 5)) < 0 && errno == EINTR) ;
                        if (num_chars == 0) 
                            continue;
                        else{
                            //from_car_pipe[num_chars - 1] = '\0';
                            if (check_pipe_command_regex("^S[0-9][0-9]$", from_car_pipe)){
                                sscanf(from_car_pipe, "S%d", &car_num);
                                sprintf(str, "UPDATE ==> CAR %02d GOT SWAPPED TO SAFETY MODE", car_num);
                                write_log(str);
                            }
                            if (check_pipe_command_regex("^D[0-9][0-9]$", from_car_pipe)){
                                sscanf(from_car_pipe, "D%d", &car_num);
                                sprintf(str, "UPDATE ==> CAR %02d ISN'T HABLE TO CONTINUE THE RACE", car_num);
                                write_log(str);
                                
                                sem_wait(counter_mutex);
                                shm_info->counter_cars_finished++;
                                sem_post(counter_mutex);
                                
                            }
                              if (check_pipe_command_regex("^R[0-9][0-9]$", from_car_pipe)){
                                sscanf(from_car_pipe, "R%d", &car_num);
                                sprintf(str, "UPDATE ==> CAR %02d IS BACK AT FULL SPEED", car_num);
                                write_log(str);
                            }
                            if (check_pipe_command_regex("^B[0-9][0-9]$", from_car_pipe)){
                                sscanf(from_car_pipe, "B%d", &car_num);
                                sprintf(str, "UPDATE ==> CAR %02d ENTERED THE BOX", car_num);
                                write_log(str);
                            }
                            if (check_pipe_command_regex("^F[0-9][0-9]$", from_car_pipe)){
                                int counter;
                                sem_wait(counter_mutex);
                                (shm_info->counter_cars_finished)++;
                                counter = ++(shm_info->end_counter);
                                sem_post(counter_mutex);
								
                                sscanf(from_car_pipe, "F%d", &car_num);
                                for (int k = 0; k < NR_CARS*NR_TEAM; k++){
                                    if(shm_info->cars[k].number == car_num){
                                        shm_info->cars[k].end_position = counter;
                                        break;
                                    }
                                }
                                sprintf(str, "UPDATE ==> CAR %02d FINISHED THE RACE", car_num);
                                write_log(str);
                            }                 
                        }
                    }
                }
            }
            sem_wait(counter_mutex);
            if (shm_info->counter_cars_finished == shm_info->nr_cars){
                sem_post(counter_mutex);
                sprintf(str, "-------All cars finished-------");
                write_log(str);
                break;
            }
            else
                sem_post(counter_mutex);
        }
    } // end while(1)
    
    kill(cpid[0], SIGTERM);
    
    // wait for all team processes to finish
    while ( (wait_status = wait(NULL)) >= 0 || (wait_status == -1 && errno == EINTR));    
    //close all unnamed pipes
    for (int i = 0; i < NR_TEAM; i++) close(fd_team[i][0]);
    
    exit(0);
}


void interrupt_race(int sig){
    write_log("INTERRUPTING RACE");
    if (teams_pid != NULL)
        for (int i = 0; i < NR_TEAM; i++)
            kill(teams_pid[i], SIGUSR1);
    signal(SIGUSR1, SIG_IGN);
    race_going = false;
}


void terminate_teams(int signal) {
        #ifdef DEBUG
        write_log("[Race Manager] Got SIGTERM");
        write_log("[Race Manager] Waiting for all cars to finish their lap");
        #endif
        int i, wait_status;

        if (teams_pid != NULL)
            for (i = 0; i < NR_TEAM; i++) 
                kill(teams_pid[i], SIGTERM);
        
        while ((wait_status = wait(NULL)) >= 0 || (wait_status == -1 && errno == EINTR));
        
        kill(malfunction_pid, SIGTERM); //signal malfunction_manager to stop
            
        for (i = 0; i < NR_TEAM; i++) close(fd_team[i][0]);

        free(teams_pid);

        exit(0);
}


int check_pipe_command_regex(const char *pattern, char *string) {
    regex_t re;
    if (regcomp(&re, pattern, REG_EXTENDED) != 0)
    return 0;

    if (regexec(&re, string, 0, NULL, 0) != 0){
        regfree(&re);
        return 0;
    }
    regfree(&re);
    return 1;
}


int car_number_exists(int number){
    int i;
    for (i = 0; i < NR_TEAM * NR_CARS; i ++)
        if (shm_info->cars[i].number==number)
            return 1;
    return 0;
}


int minium_cars(){
    int i;
    int count = 0;
    // loop through teams, if team doesnt have name, doesnt exist
    for (i = 0; i < NR_TEAM * NR_CARS; i+= NR_CARS)
        if ( strcmp(shm_info->cars[i].team_name, "") != 0)
            count++;
    
    return count == NR_TEAM;
}


int max(int fd1, int **other_fds){
    int max = fd1, i;
    for (i = 0; i < NR_TEAM; i++)
        if (other_fds[i][0]>max)
            max = other_fds[i][0];
    return max;
}

pid_t * teams_pid_array(){
    return (pid_t*) malloc(sizeof(pid_t)*NR_TEAM);
}

