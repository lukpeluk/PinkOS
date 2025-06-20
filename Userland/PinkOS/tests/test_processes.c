#include <libs/stdpink.h>

enum State { RUNNING,
             BLOCKED,
             KILLED };

typedef struct P_rq {
  Pid pid;
  enum State state;
} p_rq;

// Random
static uint32_t m_z = 362436069;
static uint32_t m_w = 521288629;

static uint32_t GetUint() {
  m_z = 36969 * (m_z & 65535) + (m_z >> 16);
  m_w = 18000 * (m_w & 65535) + (m_w >> 16);
  return (m_z << 16) + m_w;
}

static uint32_t GetUniform(uint32_t max) {
  uint32_t u = GetUint();
  return (u + 1.0) * 2.328306435454494e-10 * max;
}

void endless_loop() {
  while (1)
    ;
}


int test_processes_main(char * args) {
  uint8_t rq;
  uint8_t alive = 0;
  uint8_t action;
  uint64_t max_processes;

  if (args == NULL || args[0] == '\0') {
    printf("Usage: test_processes <max_processes>\n");
    return -1;
  }

  if ((max_processes = string_to_uint64(args)) <= 0) {
    printf("Invalid max_processes value\n");
    return -1;
  }

  printf("Starting processes test with max_processes=%d\n", max_processes);

  p_rq p_rqs[max_processes];
  

  while (1) {
    alive = 0;

    // Create max_processes processes
    for (rq = 0; rq < max_processes; rq++) {
      char *args[] = {NULL};
      IO_Files io_files = {0, 0, 0}; // Default IO
      
      // Try to run endless_loop program
      p_rqs[rq].pid = newThread(endless_loop, "", PRIORITY_NORMAL);

      if (p_rqs[rq].pid == 0) {
        printf(">!ERROR creating process %d\n", rq);
        return -1;
      } else {
        p_rqs[rq].state = RUNNING;
        alive++;
        printf("Created process %d with PID %d\n", rq, p_rqs[rq].pid);
      }
    }

    printf("Created %d processes\n", alive);

    // Randomly kills, blocks or unblocks processes until every one has been killed
    while (alive > 0) {

      for (rq = 0; rq < max_processes; rq++) {
        action = GetUniform(100) % 2;

        switch (action) {
          case 0: // Kill process
            if (p_rqs[rq].state == RUNNING || p_rqs[rq].state == BLOCKED) {
              killProcess(p_rqs[rq].pid);
              printf("Killed process PID %d\n", p_rqs[rq].pid);
              p_rqs[rq].state = KILLED;
              alive--;
            }
            break;

          case 1: // Block process
            if (p_rqs[rq].state == RUNNING) {
              int block_result = setWaiting(p_rqs[rq].pid);
              if (block_result == 0) {
                printf("Blocked process PID %d\n", p_rqs[rq].pid);
                p_rqs[rq].state = BLOCKED;
              } else {
                printf(">!ERROR blocking process PID %d\n", p_rqs[rq].pid);
                return -1;
              }
            }
            break;
        }
      }

      // Randomly unblock processes
      for (rq = 0; rq < max_processes; rq++) {
        if (p_rqs[rq].state == BLOCKED && GetUniform(100) % 2) {
          wakeProcess(p_rqs[rq].pid);
          printf("Unblocked process PID %d\n", p_rqs[rq].pid);
          p_rqs[rq].state = RUNNING;
        }
      }
      
      sleep(100); // Short sleep to avoid overwhelming the system
    }

    printf(">+All processes in this iteration have been terminated\n");
    sleep(2000); // Sleep 2 seconds between test iterations
  }

  return 0;
}
