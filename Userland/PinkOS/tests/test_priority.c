#include <libs/stdpink.h>

#define MINOR_WAIT 500000000
#define WAIT 1000000000
#define TOTAL_PROCESSES 3

// Priority values - using stdpink Priority enum
static Priority prio[TOTAL_PROCESSES] = {PRIORITY_LOW, PRIORITY_NORMAL, PRIORITY_HIGH};

// Busy wait function
static void bussy_wait(uint64_t n) {
  uint64_t i;
  for (i = 0; i < n; i++) {
    // Just waste CPU cycles
  }
}

int endless_loop_print_main(char *args) {
    Pid pid = getPID();
    uint64_t wait_time = MINOR_WAIT;
    
    // Parse wait time from arguments if provided
    if (args && args[0] != '\0') {
        uint64_t parsed_time = string_to_uint64(args);
        if (parsed_time > 0) {
            wait_time = parsed_time;
        } else {
            printf("Invalid wait time provided, using default %d microseconds.\n", MINOR_WAIT);
        }
    }

    printf("Process %d starting endless loop with print\n", pid);
    
    // Endless loop that prints PID periodically
    while (1) {
        printf("PID: %d \n", pid);
        bussy_wait(wait_time);
    }
    
    return 0; // Never reached
}



int test_priority_main(char *args) {
  Pid pids[TOTAL_PROCESSES];
  uint64_t i;

  printf("Starting priority test...\n");

  // Create test processes
  for (i = 0; i < TOTAL_PROCESSES; i++) {
    pids[i] = newThread(endless_loop_print_main, "", PRIORITY_NORMAL);
    
    if (pids[i] == 0) {
      printf(">!ERROR: Could not create test process %d\n", i);
      return -1;
    }
    
    printf("Created test process %d with PID %d\n", i, pids[i]);
  }

  bussy_wait(WAIT);
  printf("\nCHANGING PRIORITIES...\n");

  // Change priorities
  for (i = 0; i < TOTAL_PROCESSES; i++) {
    changePriority(pids[i], prio[i]);
    printf("Changed priority of PID %d to %d\n", pids[i], prio[i]);
  }

  bussy_wait(WAIT);
  printf("\nBLOCKING...\n");

  // Block all processes
  for (i = 0; i < TOTAL_PROCESSES; i++) {
    int result = setWaiting(pids[i]);
    if (result == 0) {
      printf("Blocked PID %d\n", pids[i]);
    } else {
      printf(">!ERROR: Could not block PID %d\n", pids[i]);
    }
  }

  printf("CHANGING PRIORITIES WHILE BLOCKED...\n");

  // Change priorities while blocked
  for (i = 0; i < TOTAL_PROCESSES; i++) {
    changePriority(pids[i], PRIORITY_NORMAL);
    printf("Changed priority of blocked PID %d to PRIORITY_NORMAL\n", pids[i]);
  }

  printf("UNBLOCKING...\n");

  // Unblock all processes
  for (i = 0; i < TOTAL_PROCESSES; i++) {
    wakeProcess(pids[i]);
    printf("Unblocked PID %d\n", pids[i]);
  }

  bussy_wait(WAIT);
  printf("\nKILLING...\n");

  // Kill all test processes
  for (i = 0; i < TOTAL_PROCESSES; i++) {
    killProcess(pids[i]);
    printf("Killed PID %d\n", pids[i]);
  }

  printf(">+Priority test completed successfully\n");
  return 0;
}
