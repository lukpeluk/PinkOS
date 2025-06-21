#include <libs/stdpink.h>
#include <types.h>

typedef enum {
    HUNGRY,
    EATING,
    THINKING
} PhilosopherState;
typedef struct {
    Pid pid;
    PhilosopherState state;
} Philosopher;

#define MAX_PHILOSOPHERS 10 

char philosopher_states[3] = {'H', 'E', 'T'};           // array auxiliar para imprimir los estados

// informacion de filosofos
Philosopher philosophers[MAX_PHILOSOPHERS] = {0};       // array global de philosofos y sus estados
int philosophers_count;

// semaforos
uint64_t forks[MAX_PHILOSOPHERS] = {0};                 // array global de semaforos para los fork
uint64_t mutex = 0;                                         // mutex para proteger todo
    
void thinking(){
    sleep(1000); 
}

void eating(){
    sleep(1000); 
}

// Recibe el id del filosofo, un puntero a la variable del estado del filosofo,
// y los punteros a las variables que tienen los ids de los semaforos de los tenedores izquierdo y derecho
void philosopher(char *args) {
    int id;
    if(sscanf(args, "%d", &id) != 1) {
        printf("Error parsing philosopher arguments: %s\n", args);
        return;
    }

    int left = id;
    int right = (id + 1) % philosophers_count;

    while(1){
        // Piensa
        sem_wait(mutex);
        philosophers[id].state = THINKING;
        sem_post(mutex);
        thinking();
        
        // Tiene hambre
        sem_wait(mutex);
        philosophers[id].state = HUNGRY;
        sem_post(mutex);
        
        // Toma los tenedores
        id % 2 == 0 ? sem_wait(forks[left]) : sem_wait(forks[right]);
        id % 2 == 0 ? sem_wait(forks[right]) : sem_wait(forks[left]);

        // Come
        sem_wait(mutex);
        philosophers[id].state = EATING;
        sem_post(mutex);
        eating();

        //Deja los tenedores
        id % 2 == 0 ? sem_post(forks[left]) : sem_post(forks[right]);
        id % 2 == 0 ? sem_post(forks[right]) : sem_post(forks[left]);
    }
}

void phylo_main(char *args) {
     // Initialize the philosophers and forks
    philosophers_count = 5;
    // if (args != NULL && args[0] != '\0') {
    //     philosophers_count = string_to_int(args);
    //     if (philosophers_count <= 0) {
    //         printf((char *)"Invalid number of philosophers: %s\n", args);
    //         return;
    //     }
    // }

    mutex = sem_init(1);
    if(mutex == 0){
        print("Error while creating mutex semaphore\n");
        return;
    }

    for (int i = 0; i < philosophers_count; i++)
    {
        // Initialize each fork semaphore
        forks[i] = sem_init(1);
        if( forks[i] == 0){
            printf("Error while creating fork %d semaphore\n", i);
            return;
        }
        printf("Todos los semaforos inicializados\n");
    }
    

    for (int i = 0; i < philosophers_count; i++) {
        // Initialize each philosopher
        char args[16];
        sprintf(args, "%d", i);
        philosophers[i].pid = newThread(philosopher, args, PRIORITY_NORMAL);
        philosophers[i].state = THINKING;
    }
    printf("Todos los filosofos inicializados\n");

    while (1){
        printf("Philosophers states: ");
        sem_wait(mutex);
        for (int i = 0; i < philosophers_count; i++) {
            printf("%c ", philosopher_states[philosophers[i].state]);
        }
        sem_post(mutex);
        printf("\n");
        sleep(5000); 
    }
    
}

