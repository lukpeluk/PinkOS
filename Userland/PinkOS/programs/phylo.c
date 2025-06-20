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
    int id;
} Philosopher;

typedef struct {
    int num_philosophers;
    int should_exit;
    Philosopher philosophers[20]; // Max 20 philosophers
    uint64_t forks[20]; // Semaphore IDs for forks
} SharedMemory;

static uint64_t shared_memory_id = 0;
static SharedMemory *shared_data = NULL;
static uint64_t mutex_id = 0;

void philosopher_thread(char *args) {
    // Parse arguments: "id,mutex_id"
    int id;
    uint64_t local_mutex_id;
    sscanf(args, "%d,%u", &id, &local_mutex_id);
    
    while (1) {
        // Check if we should exit
        sem_wait(local_mutex_id);
        SharedMemory temp_data;
        readRaw(shared_memory_id, &temp_data, sizeof(SharedMemory), 0);
        int should_exit = temp_data.should_exit || id >= temp_data.num_philosophers;
        sem_post(local_mutex_id);
        
        if (should_exit) {
            break;
        }
        
        // THINKING phase
        sem_wait(local_mutex_id);
        readRaw(shared_memory_id, &temp_data, sizeof(SharedMemory), 0);
        temp_data.philosophers[id].state = THINKING;
        writeRaw(shared_memory_id, &temp_data, sizeof(SharedMemory), 0);
        sem_post(local_mutex_id);
        
        sleep(randInt(1000, 3000)); // Think for 1-3 seconds
        
        // Check again if we should exit
        sem_wait(local_mutex_id);
        readRaw(shared_memory_id, &temp_data, sizeof(SharedMemory), 0);
        should_exit = temp_data.should_exit || id >= temp_data.num_philosophers;
        sem_post(local_mutex_id);
        
        if (should_exit) {
            break;
        }
        
        // HUNGRY phase - try to get forks
        sem_wait(local_mutex_id);
        readRaw(shared_memory_id, &temp_data, sizeof(SharedMemory), 0);
        temp_data.philosophers[id].state = HUNGRY;
        writeRaw(shared_memory_id, &temp_data, sizeof(SharedMemory), 0);
        
        int left_fork = id;
        int right_fork = (id + 1) % temp_data.num_philosophers;
        uint64_t left_fork_sem = temp_data.forks[left_fork];
        uint64_t right_fork_sem = temp_data.forks[right_fork];
        sem_post(local_mutex_id);
        
        // Asymmetric strategy to prevent deadlock
        if (id % 2 == 0) {
            // Even philosophers: left fork first, then right
            sem_wait(left_fork_sem);
            sem_wait(right_fork_sem);
        } else {
            // Odd philosophers: right fork first, then left
            sem_wait(right_fork_sem);
            sem_wait(left_fork_sem);
        }
        
        // EATING phase
        sem_wait(local_mutex_id);
        readRaw(shared_memory_id, &temp_data, sizeof(SharedMemory), 0);
        if (!temp_data.should_exit && id < temp_data.num_philosophers) {
            temp_data.philosophers[id].state = EATING;
            writeRaw(shared_memory_id, &temp_data, sizeof(SharedMemory), 0);
        }
        sem_post(local_mutex_id);
        
        sleep(randInt(1000, 2000)); // Eat for 1-2 seconds
        
        // Release forks
        sem_post(left_fork_sem);
        sem_post(right_fork_sem);
    }
}

void display_table(SharedMemory *data) {
    // printf("Philosophers: %d\n", data->num_philosophers);
    // printf("Press 'a' to add philosopher, 'r' to remove, 'q' to quit\n\n");
    // printf("State: ");
    
    // for (int i = 0; i < data->num_philosophers; i++) {
    //     switch (data->philosophers[i].state) {
    //         case EATING:
    //             putChar('E');
    //             break;
    //         case THINKING:
    //             putChar('.');
    //             break;
    //         case HUNGRY:
    //             putChar('H');
    //             break;
    //     }
    //     putChar(' ');
    // }
    // printf("\n\n");
}

void add_philosopher(SharedMemory *data) {
    if (data->num_philosophers >= 20) {
        printf("Maximum number of philosophers reached (20)\n");
        sleep(1000);
        return;
    }
    
    int new_id = data->num_philosophers;
    data->num_philosophers++;
    
    // Initialize new philosopher
    data->philosophers[new_id].id = new_id;
    data->philosophers[new_id].state = THINKING;
    data->forks[new_id] = sem_init(1);
    
    // Create new philosopher thread with mutex_id as parameter
    char thread_args[64];
    sprintf(thread_args, "%d,%u", new_id, mutex_id);
    data->philosophers[new_id].pid = newThread((ProgramEntry)philosopher_thread, thread_args, PRIORITY_NORMAL);
    
    writeRaw(shared_memory_id, data, sizeof(SharedMemory), 0);
}

void remove_philosopher(SharedMemory *data) {
    if (data->num_philosophers <= 1) {
        printf("Cannot remove philosopher: minimum is 1\n");
        sleep(1000);
        return;
    }
    
    int last_id = data->num_philosophers - 1;
    
    // Kill the last philosopher thread
    killProcess(data->philosophers[last_id].pid);
    
    // Destroy the fork semaphore
    sem_destroy(data->forks[last_id]);
    
    data->num_philosophers--;
    writeRaw(shared_memory_id, data, sizeof(SharedMemory), 0);
}

void phylo_main(char *args) {
    // Initialize random seed
    seedRandom(getMillisElapsed());
    
    // Create mutex for shared memory access
    mutex_id = sem_init(1);
    if (mutex_id == 0) {
        printf("Failed to create mutex semaphore\n");
        return;
    }
    
    // Initialize the philosophers count
    int initial_philosophers = 5;
    if (args != NULL && args[0] != '\0') {
        initial_philosophers = string_to_int(args);
        if (initial_philosophers <= 0 || initial_philosophers > 20) {
            printf("Invalid number of philosophers. Using default (5)\n");
            initial_philosophers = 5;
            sleep(2000);
        }
    }
    
    // Create a unique shared memory name
    Pid main_pid = getPID();
    char *prefix = "phylo_mem_";
    char *pid_str = uint64_to_string(main_pid);
    char *mem_name = concat(prefix, pid_str);
    
    // Create shared memory
    shared_memory_id = mkFile(mem_name, FILE_TYPE_RAW_DATA, sizeof(SharedMemory));
    if (shared_memory_id == 0) {
        printf("Failed to create shared memory\n");
        sem_destroy(mutex_id);
        free(mem_name);
        return;
    }
    
    // Initialize shared data
    SharedMemory initial_data;
    initial_data.num_philosophers = initial_philosophers;
    initial_data.should_exit = 0;
    
    // Initialize philosophers and forks
    for (int i = 0; i < initial_philosophers; i++) {
        initial_data.philosophers[i].id = i;
        initial_data.philosophers[i].state = THINKING;
        initial_data.forks[i] = sem_init(1);
        
        // Create philosopher thread with mutex_id as parameter
        char thread_args[64];
        sprintf(thread_args, "%d,%u", i, mutex_id);
        initial_data.philosophers[i].pid = newThread((ProgramEntry)philosopher_thread, thread_args, PRIORITY_NORMAL);
    }
    
    // Write initial data to shared memory
    writeRaw(shared_memory_id, &initial_data, sizeof(SharedMemory), 0);
    
    // Main UI loop
    char input;
    SharedMemory current_data;
    
    while (1) {
        // Read current state with mutex protection
        sem_wait(mutex_id);
        readRaw(shared_memory_id, &current_data, sizeof(SharedMemory), 0);
        sem_post(mutex_id);
        
        // Display current state
        display_table(&current_data);
        
        // Check for input (non-blocking)
        if (readStdin(&input, 1) > 0) {
            switch (input) {
                case 'a':
                case 'A':
                    sem_wait(mutex_id);
                    readRaw(shared_memory_id, &current_data, sizeof(SharedMemory), 0);
                    add_philosopher(&current_data);
                    sem_post(mutex_id);
                    break;
                case 'r':
                case 'R':
                    sem_wait(mutex_id);
                    readRaw(shared_memory_id, &current_data, sizeof(SharedMemory), 0);
                    remove_philosopher(&current_data);
                    sem_post(mutex_id);
                    break;
                case 'q':
                case 'Q':
                    sem_wait(mutex_id);
                    readRaw(shared_memory_id, &current_data, sizeof(SharedMemory), 0);
                    current_data.should_exit = 1;
                    writeRaw(shared_memory_id, &current_data, sizeof(SharedMemory), 0);
                    sem_post(mutex_id);
                    
                    // Wait a bit for threads to exit
                    sleep(500);
                    
                    // Kill all philosopher threads
                    for (int i = 0; i < current_data.num_philosophers; i++) {
                        killProcess(current_data.philosophers[i].pid);
                        sem_destroy(current_data.forks[i]);
                    }
                    
                    // Clean up
                    rmFile(shared_memory_id);
                    sem_destroy(mutex_id);
                    free(mem_name);
                    return;
            }
        }
        
        sleep(10*1000);
        print("Vuelta al ciclo principal\n");
    }
}

