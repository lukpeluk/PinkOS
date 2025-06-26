#include <libs/stdpink.h>
#include <libs/events.h>
#include <libs/serialLib.h>

#define TOTAL_PAIR_PROCESSES 2

static int global_shared_value; // Shared memory

static void slowInc(int64_t *p, int64_t inc) {
  int aux = *p;
  yield(); // This makes the race condition highly probable
  aux += inc;
  *p = aux;
}

// Process function for incrementing
static void process_inc_function(char *args) {
  int n;
  int inc;
  int use_sem;
  printf("Process increment function started with args: %s\n", args);
  // Parse arguments: "n inc sem_id"
  if (sscanf(args, "%d %d %d", &n, &inc, &use_sem) != 3) {
    printf("ERROR: Invalid arguments for process_inc_function\n");
    return;
  }
  uint64_t sem_id = use_sem;

  printf("Process started with n=%d, inc=%d, use_sem=%d\n", n, inc, use_sem);

  uint64_t i;
  for (i = 0; i < n; i++) {
    if (use_sem) {
      sem_wait(sem_id);
    }
    slowInc((int64_t *)&global_shared_value,(int64_t) inc);
    if (use_sem) {
      sem_post(sem_id);
    }
  }

  log_to_serial("Process increment function completed");
}

int test_synchro_main(char *args) {
  if (args == NULL) {
    printf("Usage: test_synchro <n> <use_sem>\n");
    printf("  n: number of iterations\n");
    printf("  use_sem: 1 to use semaphores, 0 for no synchronization\n");
    return -1;
  }
  log_to_serial(args);

  int n;
  int use_sem = 0;

  if (sscanf(args, "%d %d", &n, &use_sem) != 2) {
    printf("ERROR: Invalid arguments. Usage: test_synchro <n> <use_sem>\n");
    return -1;
  }

  printf("Starting synchronization test: n=%d, use_sem=%d\n", n, use_sem);

  Pid pids[2 * TOTAL_PAIR_PROCESSES];
  global_shared_value = 0;

  uint64_t sem_id = 0;
  if (use_sem) {
    sem_id = sem_init(1);
    if (sem_id == 0) {
      printf(">!ERROR: Could not create semaphore\n");
      return -1;
    }
    use_sem = sem_id; // Use the semaphore ID for the
  }
  uint64_t i;
  
  // Create increment processes (+1)
  for (i = 0; i < TOTAL_PAIR_PROCESSES; i++) {
    char args_str[256] = {0};
    sprintf(args_str, "%d 1 %d", n, use_sem);
    
    pids[i] = newThread(process_inc_function, args_str, PRIORITY_NORMAL);
    if (pids[i] == 0) {
      printf(">!ERROR: Could not create increment process %d\n", i);
      return -1;
    }
    printf("Created increment process PID %d\n", pids[i]);
  }

  // Create decrement processes (-1)
  for (i = 0; i < TOTAL_PAIR_PROCESSES; i++) {
    char args_str[256] = {0};
    sprintf(args_str, "%d -1 %d", n, use_sem);
    
    pids[i + TOTAL_PAIR_PROCESSES] = newThread(process_inc_function, args_str, PRIORITY_NORMAL);
    if (pids[i + TOTAL_PAIR_PROCESSES] == 0) {
      printf(">!ERROR: Could not create decrement process %d\n", i);
      return -1;
    }
    printf("Created decrement process PID %d\n", pids[i + TOTAL_PAIR_PROCESSES]);
  }

  ProcessDeathCondition pdc;
  for (i = 0; i < 2 * TOTAL_PAIR_PROCESSES; i++) {
    log_to_serial("Registering process death event");
    pdc.pid = pids[i];
    waitForEvent(PROCESS_DEATH_EVENT, NULL, &pdc);
  }


  // Check final result
  printf("Final shared value: %d\n", global_shared_value);
  
  if(use_sem) {
    sem_destroy(sem_id);
  }

  if (global_shared_value == 0) {
    printf(">+SUCCESS: Synchronization worked correctly\n");
  } else {
    printf(">!ERROR: Synchronization failed, expected 0, got %d\n", global_shared_value);
  }

  log_to_serial("Test synchronization completed");

  return 0;
}
