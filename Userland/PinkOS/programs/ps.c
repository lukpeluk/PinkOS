#include <programs.h>
#include <libs/stdpink.h>

void ps_main(char *args) {
    // setWaiting(getPID());

//     //    PID TTY          TIME CMD
//     char *header_pid = (char *) "PID";
//     char *header_tty = (char *) "TTY";
//     char *header_time = (char *) "TIME";
//     char *header_cmd = (char *) "CMD";
//  //   696969 pts/0    00:00:00 ps
//     seedRandom(getMillisElapsed());
//     char *pid = (char *) "696969";
//     for (int i = 0; i < 6; i++) {
//         pid[i] = randInt(0, 9) + '0';
//     }

// //     char *tty = (char *) "pts/0";
// //     char *time = (char *) "00:00:00";
// //     char *cmd = (char *) "ps";
// //     printf((char *)"%10s %10s %10s %10s\n", header_pid, header_tty, header_time, header_cmd);
// //     printf((char *)"%10s %10s %10s %10s\n", pid, tty, time, cmd);

char * process_type_strings[] = {
    "Main",
    "Thread"
};

char * process_state_strings[] = {
    "New",
    "Running",
    "Ready",
    "Waiting",
    "Terminated"
};

char * process_priority_strings[] = {
    "Low",
    "Normal",
    "High"
};

int process_count = 0;
    
    Process * process_array =  getAllProcesses(&process_count);

    // printf("Puntero a struct de procesos: %d, cantidad de procesos %d\n", process_array, process_count);
    // Imprimir encabezados
    printf((char *)"%10s %10s %10s %10s %10s\n", "PID", "Type", "State", "Priority", "Program"); //? Esto realmente funciona asi? Si.
    
    // Imprimir información de cada proceso
    for (int i = 0; i < process_count; i++) {
        Process *process = &process_array[i];
        
        printf((char *)"%10d %10s %10s %10s %10s\n", 
               (int)process->pid,
               process_type_strings[process->type], 
               process_state_strings[process->state], 
               process_priority_strings[process->priority], 
               process->program.name);
    }

}