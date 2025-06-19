// //! Estoy suponiendo que tengo las siguientes funciones disponibles (que despues tengo que implementar en stdpink.h):
// //! uint_64 atoi (char * str); string a entero
// //! char * num_to_string (int num); entero a string (con memoria dinámica?)
// //! char * concat(char * str1, char * str2); concatena dos strings (con memoria dinámica?)
// //! int strcpy(char * dest, char * src); copia un string a otro

// #include <libs/stdpink.h>
// #include <types.h>

// typedef enum {
//     HUNGRY,
//     EATING,
//     THINKING
// } PhilosopherState;

// typedef struct {
//     Pid pid;
//     PhilosopherState state;
// } Philosopher;
    
// void philosopher(int id, PhilosopherState *state, int *left_fork, int *right_fork) {
//     while(1){
        
//     }
// }

// void phylo_main(char *args) {
//     // Initialize the philosophers and forks
//     int num_philosophers = 5;
//     if (args != NULL && args[0] != '\0') {
//         num_philosophers = atoi(args);
//         if (num_philosophers <= 0) {
//             printf((char *)"Invalid number of philosophers: %s\n", args);
//             return;
//         }
//     }
    
//     // Create file permissions for the philosophers' shared memory
//     FilePermissions permissions = {
//         .writing_owner = getProcessGroupMain(),
//         .writing_conditions = '*',
//         .reading_owner = getProcessGroupMain(),
//         .reading_conditions = '*'
//     };

//     // Create a unique name for the shared memory based on the masters PID
//     Pid pid = getPID();
//     char philosophers_mem_name[32];
//     char *prefix = "philosophers_memory_";
    
//     char *pid_str = num_to_string(pid);
//     char *full_name = concat(prefix, pid_str);
//     strcpy(philosophers_mem_name, full_name);
//     free(full_name);
//     free(pid_str);

//     // Create the shared memory for philosophers
//     uint64_t sh_m = mkFile(philosophers_mem_name, FILE_TYPE_RAW_DATA, num_philosophers * (sizeof(Philosopher) + sizeof(uint64_t)),  permissions);
    
//     Philosopher * philosophers = malloc(num_philosophers * sizeof(Philosopher));
//     uint64_t * forks = malloc(num_philosophers * sizeof(uint64_t));
    
//     int writeRaw(uint64_t id, void * buffer, uint32_t size, uint32_t offset);

//     for (int i = 0; i < num_philosophers; i++) {
//         philosophers[i].pid = newThread(philosopher, (char *) i, PRIORITY_NORMAL);
//         philosophers[i].state = HUNGRY;
//         forks[i] = sem_init(1);
//     }
    
// }

