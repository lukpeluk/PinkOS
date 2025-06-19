// #include <processManager/scheduler.h>
// #include <drivers/serialDriver.h>
// #include <drivers/pitDriver.h>
// #include <stdint.h>
// #include <stdlib.h>
// #include <types.h>
// #include <permissions.h>

// #include <tests/tests.h>

// // Forward declarations
// extern void _hlt();
// extern void sleep(uint64_t millis);

// // Test result tracking
// static int tests_passed = 0;
// static int tests_failed = 0;
// static int total_tests = 0;



// // Test process functions
// void test_process_1(char *args) {
//     log_to_serial("I: Test process 1 running");
//     for (int i = 0; i < 5; i++) {
//         sleep(100);
//         log_to_serial("I: Process 1 tick");
//     }
//     log_to_serial("I: Test process 1 ending");
// }

// void test_process_2(char *args) {
//     log_to_serial("I: Test process 2 running");
//     for (int i = 0; i < 3; i++) {
//         sleep(150);
//         log_to_serial("I: Process 2 tick");
//     }
//     log_to_serial("I: Test process 2 ending");
// }

// void test_process_long(char *args) {
//     log_to_serial("I: Long process running");
//     for (int i = 0; i < 20; i++) {
//         sleep(200);
//         log_to_serial("I: Long process tick");
//     }
//     log_to_serial("I: Long process ending");
// }

// void test_waiting_process(char *args) {
//     log_to_serial("I: Waiting process starting");
//     Pid my_pid = getCurrentProcessPID();
//     setWaiting(my_pid);
//     log_to_serial("I: Waiting process woken up!");
// }

// void test_thread_function(char *args) {
//     log_to_serial("I: Thread running with args: ");
//     if (args) {
//         log_to_serial(args);
//     }
//     for (int i = 0; i < 3; i++) {
//         sleep(100);
//         log_to_serial("I: Thread tick");
//     }
//     log_to_serial("I: Thread ending");
// }

// void test_child_process(char *args) {
//     log_to_serial("I: Child process running");
//     sleep(500);
//     log_to_serial("I: Child process ending");
// }

// void test_parent_process(char *args) {
//     log_to_serial("I: Parent process creating child");
//     Pid my_pid = getCurrentProcessPID();
    
//     Program child_program = {
//         .command = "test_child",
//         .name = "Test Child Process",
//         .entry = test_child_process,
//         .permissions = 0,
//         .help = "Test child process",
//         // .description = "A test child process"
//     };
    
//     Pid child_pid = newProcess(child_program, "child_args", PRIORITY_NORMAL, my_pid);
//     log_decimal("I: Created child with PID: ", child_pid);
    
//     sleep(1000);
//     log_to_serial("I: Parent process ending");
// }

// void test_semaphore_process_1(char *args) {
//     uint64_t sem_id = (uint64_t)args; // Semaphore ID passed as argument
//     log_to_serial("I: Semaphore process 1 waiting");
//     sem_wait(sem_id);
//     log_to_serial("I: Semaphore process 1 got the semaphore!");
//     sleep(200);
//     sem_post(sem_id);
//     log_to_serial("I: Semaphore process 1 released semaphore");
// }

// void test_semaphore_process_2(char *args) {
//     uint64_t sem_id = (uint64_t)args; // Semaphore ID passed as argument
//     log_to_serial("I: Semaphore process 2 waiting");
//     sem_wait(sem_id);
//     log_to_serial("I: Semaphore process 2 got the semaphore!");
//     sleep(200);
//     sem_post(sem_id);
//     log_to_serial("I: Semaphore process 2 released semaphore");
// }

// // Test functions
// void test_basic_process_creation() {
//     log_to_serial("I: ===== Testing Basic Process Creation =====");
    
//     Program test_program = {
//         .command = "test1",
//         .name = "Test Process 1",
//         .entry = test_process_1,
//         .permissions = 0,
//         .help = "Test process 1",
//         // .description = "A simple test process"
//     };
    
//     Pid pid = newProcess(test_program, "test_args", PRIORITY_NORMAL, 0);
//     ASSERT(pid != 0, "Process creation should succeed");
    
//     Process created_process = getProcess(pid);
//     ASSERT_EQUALS(pid, created_process.pid, "Created process PID should match");
//     ASSERT_EQUALS(PROCESS_TYPE_MAIN, created_process.type, "Process type should be MAIN");
//     ASSERT_EQUALS(PRIORITY_NORMAL, created_process.priority, "Process priority should be NORMAL");
// }

// void test_process_termination() {
//     log_to_serial("I: ===== Testing Process Termination =====");
    
//     Program test_program = {
//         .command = "test_term",
//         .name = "Test Termination Process",
//         .entry = test_process_2,
//         .permissions = 0,
//         .help = "Test termination process",
//         // .description = "A process for testing termination"
//     };
    
//     Pid pid = newProcess(test_program, "term_args", PRIORITY_NORMAL, 0);
//     ASSERT(pid != 0, "Process creation should succeed");
    
//     sleep(100); // Let the process start
    
//     int result = terminateProcess(pid);
//     ASSERT_EQUALS(0, result, "Process termination should succeed");
    
//     Process terminated_process = getProcess(pid);
//     ASSERT_EQUALS(0, terminated_process.pid, "Terminated process should not be found");
// }

// void test_priority_changes() {
//     log_to_serial("I: ===== Testing Priority Changes =====");
    
//     Program test_program = {
//         .command = "test_prio",
//         .name = "Test Priority Process",
//         .entry = test_process_long,
//         .permissions = 0,
//         .help = "Test priority process",
//         // .description = "A process for testing priority changes"
//     };
    
//     Pid pid = newProcess(test_program, "prio_args", PRIORITY_NORMAL, 0);
//     ASSERT(pid != 0, "Process creation should succeed");
    
//     Priority initial_priority = getPriority(pid);
//     ASSERT_EQUALS(PRIORITY_NORMAL, initial_priority, "Initial priority should be NORMAL");
    
//     int result = changePriority(pid, PRIORITY_HIGH);
//     ASSERT_EQUALS(0, result, "Priority change should succeed");
    
//     Priority new_priority = getPriority(pid);
//     ASSERT_EQUALS(PRIORITY_HIGH, new_priority, "Priority should be changed to HIGH");
    
//     result = changePriority(pid, PRIORITY_LOW);
//     ASSERT_EQUALS(0, result, "Priority change to LOW should succeed");
    
//     new_priority = getPriority(pid);
//     ASSERT_EQUALS(PRIORITY_LOW, new_priority, "Priority should be changed to LOW");
    
//     terminateProcess(pid);
// }

// void test_thread_creation() {
//     log_to_serial("I: ===== Testing Thread Creation =====");
    
//     Program parent_program = {
//         .command = "test_parent",
//         .name = "Test Parent Process",
//         .entry = test_parent_process,
//         .permissions = 0,
//         .help = "Test parent process",
//         // .description = "A parent process for testing threads"
//     };
    
//     Pid parent_pid = newProcess(parent_program, "parent_args", PRIORITY_NORMAL, 0);
//     ASSERT(parent_pid != 0, "Parent process creation should succeed");
    
//     sleep(100); // Let parent start
    
//     Pid thread_pid = newThread(test_thread_function, "thread_args", PRIORITY_NORMAL, parent_pid);
//     ASSERT(thread_pid != 0, "Thread creation should succeed");
    
//     Process thread_process = getProcess(thread_pid);
//     ASSERT_EQUALS(PROCESS_TYPE_THREAD, thread_process.type, "Thread type should be THREAD");
    
//     Process parent_process = getParent(thread_pid);
//     ASSERT_EQUALS(parent_pid, parent_process.pid, "Thread parent should be correct");
    
//     sleep(2000); // Let processes run
//     terminateProcess(parent_pid);
// }

// void test_parent_child_relationships() {
//     log_to_serial("I: ===== Testing Parent-Child Relationships =====");
    
//     Program parent_program = {
//         .command = "test_parent_child",
//         .name = "Test Parent-Child Process",
//         .entry = test_parent_process,
//         .permissions = 0,
//         .help = "Test parent-child process",
//         // .description = "A process for testing parent-child relationships"
//     };
    
//     Pid parent_pid = newProcess(parent_program, "parent_child_args", PRIORITY_NORMAL, 0);
//     ASSERT(parent_pid != 0, "Parent process creation should succeed");
    
//     sleep(200); // Let parent create child
    
//     // Test that terminating parent kills children
//     int result = terminateProcess(parent_pid);
//     ASSERT_EQUALS(0, result, "Parent termination should succeed");
    
//     sleep(100); // Give time for cleanup
    
//     Process parent_process = getProcess(parent_pid);
//     ASSERT_EQUALS(0, parent_process.pid, "Parent should be terminated");
// }

// void test_waiting_and_waking() {
//     log_to_serial("I: ===== Testing Process Waiting and Waking =====");
    
//     Program waiting_program = {
//         .command = "test_waiting",
//         .name = "Test Waiting Process",
//         .entry = test_waiting_process,
//         .permissions = 0,
//         .help = "Test waiting process",
//         // .description = "A process for testing waiting"
//     };
    
//     Pid waiting_pid = newProcess(waiting_program, "waiting_args", PRIORITY_NORMAL, 0);
//     ASSERT(waiting_pid != 0, "Waiting process creation should succeed");
    
//     sleep(200); // Let process start and wait
    
//     Process waiting_process_info = getProcess(waiting_pid);
//     ASSERT_EQUALS(PROCESS_STATE_WAITING, waiting_process_info.state, "Process should be waiting");
    
//     int result = wakeProcess(waiting_pid);
//     ASSERT_EQUALS(0, result, "Wake process should succeed");
    
//     sleep(100); // Give time for process to wake up
    
//     waiting_process_info = getProcess(waiting_pid);
//     ASSERT(waiting_process_info.state != PROCESS_STATE_WAITING, "Process should not be waiting anymore");
    
//     terminateProcess(waiting_pid);
// }

// void test_semaphores() {
//     log_to_serial("I: ===== Testing Semaphores =====");
    
//     // Create semaphore with initial value 1 (mutex)
//     sem_init(1);
//     uint64_t sem_id = 1; // Assuming first semaphore gets ID 1
    
//     Program sem_program_1 = {
//         .command = "test_sem1",
//         .name = "Test Semaphore Process 1",
//         .entry = test_semaphore_process_1,
//         .permissions = 0,
//         .help = "Test semaphore process 1",
//         // .description = "First process for testing semaphores"
//     };
    
//     Program sem_program_2 = {
//         .command = "test_sem2",
//         .name = "Test Semaphore Process 2",
//         .entry = test_semaphore_process_2,
//         .permissions = 0,
//         .help = "Test semaphore process 2",
//         // .description = "Second process for testing semaphores"
//     };
    
//     Pid sem_pid_1 = newProcess(sem_program_1, (char*)sem_id, PRIORITY_NORMAL, 0);
//     ASSERT(sem_pid_1 != 0, "Semaphore process 1 creation should succeed");
    
//     Pid sem_pid_2 = newProcess(sem_program_2, (char*)sem_id, PRIORITY_NORMAL, 0);
//     ASSERT(sem_pid_2 != 0, "Semaphore process 2 creation should succeed");
    
//     sleep(1000); // Let processes run and interact with semaphore
    
//     terminateProcess(sem_pid_1);
//     terminateProcess(sem_pid_2);
    
//     // // int destroy_result = sem_destroy(sem_id);
//     // // ASSERT_EQUALS(0, destroy_result, "Semaphore destruction should succeed");
// }

// void test_error_cases() {
//     log_to_serial("I: ===== Testing Error Cases =====");
    
//     // Test invalid process termination
//     int result = terminateProcess(999999); // Non-existent PID
//     ASSERT(result != 0, "Terminating non-existent process should fail");
    
//     // Test invalid priority change
//     result = changePriority(999999, PRIORITY_HIGH);
//     ASSERT_EQUALS(-1, result, "Changing priority of non-existent process should fail");
    
//     // Test invalid thread creation with non-existent parent
//     Pid invalid_thread = newThread(test_thread_function, "invalid_args", PRIORITY_NORMAL, 999999);
//     ASSERT_EQUALS(0, invalid_thread, "Creating thread with invalid parent should fail");
    
//     // Test invalid priority values
//     Program test_program = {
//         .command = "test_invalid",
//         .name = "Test Invalid Process",
//         .entry = test_process_1,
//         .permissions = 0,
//         .help = "Test invalid process",
//         // .description = "A process for testing invalid operations"
//     };
    
//     Pid valid_pid = newProcess(test_program, "invalid_test", PRIORITY_NORMAL, 0);
//     if (valid_pid != 0) {
//         result = changePriority(valid_pid, 999); // Invalid priority
//         ASSERT_EQUALS(-1, result, "Setting invalid priority should fail");
        
//         terminateProcess(valid_pid);
//     }
    
//     // Test invalid wake process
//     result = wakeProcess(999999);
//     ASSERT_EQUALS(-1, result, "Waking non-existent process should fail");
    
//     // Test destroying non-existent semaphore
//     // result = sem_destroy(999999);
//     // ASSERT(result != 0, "Destroying non-existent semaphore should fail");
// }

// void test_get_all_processes() {
//     log_to_serial("I: ===== Testing Get All Processes =====");
    
//     int initial_count;
//     Process *initial_processes = getAllProcesses(&initial_count);
    
//     // Create several test processes
//     Program test_program = {
//         .command = "test_getall",
//         .name = "Test GetAll Process",
//         .entry = test_process_long,
//         .permissions = 0,
//         .help = "Test getall process",
//         // .description = "A process for testing getAllProcesses"
//     };
    
//     Pid pid1 = newProcess(test_program, "getall1", PRIORITY_NORMAL, 0);
//     Pid pid2 = newProcess(test_program, "getall2", PRIORITY_NORMAL, 0);
//     Pid pid3 = newProcess(test_program, "getall3", PRIORITY_NORMAL, 0);
    
//     sleep(100); // Let processes start
    
//     int new_count;
//     Process *new_processes = getAllProcesses(&new_count);
    
//     ASSERT(new_count > initial_count, "Process count should increase after creating processes");
//     ASSERT(new_processes != NULL, "getAllProcesses should return valid pointer");
    
//     // Verify that our created processes are in the list
//     int found_count = 0;
//     for (int i = 0; i < new_count; i++) {
//         if (new_processes[i].pid == pid1 || new_processes[i].pid == pid2 || new_processes[i].pid == pid3) {
//             found_count++;
//         }
//     }
//     ASSERT_EQUALS(3, found_count, "All created processes should be in the list");
    
//     // Clean up
//     terminateProcess(pid1);
//     terminateProcess(pid2);
//     terminateProcess(pid3);
    
//     if (initial_processes) free(initial_processes);
//     if (new_processes) free(new_processes);
// }

// void test_initial_pid() {
//     log_to_serial("I: ===== Testing initial PID =====");
    
//     Pid current_pid = getCurrentProcessPID();
//     ASSERT(current_pid == 0, "Initial PID should be 0 (no process running)");
    
//     Process current_process = getProcess(current_pid);
//     ASSERT_EQUALS(current_pid, current_process.pid, "Current process should be retrievable even if no process is running (retrieves trash but with PID 0)");
// }

// void test_scheduling_quantum() {
//     log_to_serial("I: ===== Testing Scheduling Quantum =====");
    
//     // Create processes with different priorities to test quantum differences
//     Program high_prio_program = {
//         .command = "test_high_prio",
//         .name = "High Priority Process",
//         .entry = test_process_long,
//         .permissions = 0,
//         .help = "High priority test process",
//         // .description = "A high priority process for testing quantum"
//     };
    
//     Program low_prio_program = {
//         .command = "test_low_prio",
//         .name = "Low Priority Process",
//         .entry = test_process_long,
//         .permissions = 0,
//         .help = "Low priority test process",
//         // .description = "A low priority process for testing quantum"
//     };
    
//     Pid high_pid = newProcess(high_prio_program, "high_prio", PRIORITY_HIGH, 0);
//     Pid low_pid = newProcess(low_prio_program, "low_prio", PRIORITY_LOW, 0);
    
//     ASSERT(high_pid != 0, "High priority process creation should succeed");
//     ASSERT(low_pid != 0, "Low priority process creation should succeed");
    
//     sleep(2000); // Let processes run for a while to test scheduling
    
//     // Check that both processes are still running (or have been scheduled)
//     Process high_process = getProcess(high_pid);
//     Process low_process = getProcess(low_pid);
    
//     ASSERT(high_process.pid != 0, "High priority process should still exist");
//     ASSERT(low_process.pid != 0, "Low priority process should still exist");
    
//     terminateProcess(high_pid);
//     terminateProcess(low_pid);
// }

// void test_multiple_semaphores() {
//     log_to_serial("I: ===== Testing Multiple Semaphores =====");
    
//     // Create multiple semaphores
//     sem_init(2); // Semaphore with count 2
//     sem_init(1); // Mutex semaphore
//     sem_init(0); // Initially blocked semaphore
    
//     // Test that semaphores work independently
//     uint64_t sem1_id = 2; // Assuming IDs are assigned sequentially
//     uint64_t sem2_id = 3;
//     uint64_t sem3_id = 4;
    
//     // Test sem_wait and sem_post on different semaphores
//     sem_wait(sem1_id); // Should not block (count was 2, now 1)
//     sem_wait(sem1_id); // Should not block (count was 1, now 0)
    
//     sem_post(sem2_id); // Should increment mutex from 1 to 2
    
//     // Clean up semaphores
//     // int result1 = sem_destroy(sem1_id);
//     // int result2 = sem_destroy(sem2_id);
//     // int result3 = sem_destroy(sem3_id);
    
//     // ASSERT_EQUALS(0, result1, "Destroying semaphore 1 should succeed");
//     // ASSERT_EQUALS(0, result2, "Destroying semaphore 2 should succeed");
//     // ASSERT_EQUALS(0, result3, "Destroying semaphore 3 should succeed");
// }



// // Main test runner
// void schedulingTest(char *args) {
//     log_to_serial("I: ==========================================");
//     log_to_serial("I: Starting PinkOS Scheduler Test Suite");
//     log_to_serial("I: ==========================================");
    
//     // Initialize test counters
//     tests_passed = 0;
//     tests_failed = 0;
//     total_tests = 0;
    
//     // Run all tests
//     test_initial_pid();
//     test_basic_process_creation();
//     test_process_termination();
//     test_priority_changes();
//     test_thread_creation();
//     test_parent_child_relationships();
//     test_waiting_and_waking();
//     test_semaphores();
//     test_multiple_semaphores();
//     test_error_cases();
//     test_get_all_processes();
//     test_scheduling_quantum();
    
//     // Print test results
//     log_to_serial("I: ==========================================");
//     log_to_serial("I: Scheduler Test Suite Results:");
//     log_decimal("I: Total Tests: ", total_tests);
//     log_decimal("S: Tests Passed: ", tests_passed);
//     log_decimal("E: Tests Failed: ", tests_failed);
    
//     if (tests_failed == 0) {
//         log_to_serial("S: ALL TESTS PASSED! Scheduler is working correctly.");
//     } else {
//         log_to_serial("E: Some tests failed. Check the logs above for details.");
//     }
//     log_to_serial("I: ==========================================");
    
//     // Keep the test process alive for a bit to see results
//     sleep(2000);
// }