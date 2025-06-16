#ifndef STDPINK_H
#define STDPINK_H

#include <syscalls/syscallCodes.h>
#include <types.h>
#include <environmentApiEndpoints.h>
#include <stdint.h>


// ====== Basic Functions ======

/**
 * Compares two memory blocks
 * @param s1 the first memory block
 * @param s2 the second memory block
 * @param n the number of bytes to compare
 * @return 0 if equal, negative if s1 < s2, positive if s1 > s2
*/
uint64_t memcmp(const void *s1, const void *s2, uint64_t n);


/**
 * Copies a memory block from source to destination
 * @param dest the destination memory block
 * @param src the source memory block
 * @param n the number of bytes to copy
 * @return a pointer to the destination memory block
*/
void * memcpy(void * dest, const void * src, uint64_t n);

/**
 * Sets a memory block to a specific value
 * @param s the memory block to set
 * @param c the value to set
 * @param n the number of bytes to set
 * @return a pointer to the memory block
*/
void * memset(void * s, int c, uint64_t n);

// ====== String Functions ======
/**
 * Compares two strings
 * @param s1 the first string
 * @param s2 the second string
 * @return 0 if equal, negative if s1 < s2, positive if s1 > s2
*/
int strcmp(const char * s1, const char * s2);

/**
 * Compares the first n characters of two strings
 * @param s1 the first string
 * @param s2 the second string
 * @param n the number of characters to compare
 * @return 0 if equal, negative if s1 < s2, positive if s1 > s2
*/
int strncmp(const char * s1, const char * s2, uint64_t n);

/**
 * Returns the length of a string
 * @param s the string
 * @return the length of the string
*/
uint64_t strlen(const char * s);

/**
 * Copies a string from source to destination
 * @param dest the destination string
 * @param src the source string
 * @return void
*/
void strcpy(char * dest, char * src);


// ====== System Functions ======
// *** Process Management ***
/**
 * Runs a program with the given arguments and priority
 * @param program the program to run
 * @param args the arguments to pass to the program
 * @param priority the priority of the process
 * @param io_files the IO files to use for the process
 * @param nohup if 1, the process runs in the background
 */
Pid runProgram(Program * program, char * args, Priority priority, IO_Files * io_files, int nohup);

/**
 * Creates a new thread with the given entry point and arguments
 * @param entrypoint the entry point of the thread
 * @param args the arguments to pass to the thread
 * @param priority the priority of the thread
 * @return the PID of the new thread
*/
Pid newThread(ProgramEntry entrypoint, char * args, Priority priority);

/**
 * Terminates the current process
 * @return void
*/
void quit();

/**
 * Terminates the process with the given PID
 * @param pid the PID of the process to terminate
 * @return void
*/
void killProcess(Pid pid);

/**
 * Changes the priority of the process with the given PID
 * @param pid the PID of the process to change
 * @param priority the new priority of the process
 * @return void
*/
void changePriority(Pid pid, Priority priority);

/**
 * Yields the CPU to allow the scheduler to choose another process
 * @return void
*/
void yield();

/**
 * Read from the standard input
 * @param buffer the buffer to store the read data
 * @param size the size of the buffer   
 * @return the number of bytes read, or -1 on EOF, -2 if does not have stdin assigned
*/
int readStdin(void * buffer, uint32_t size);

/**
 * Writes to the standard output
 * @param buffer the buffer containing the data to write
 * @param size the size of the data to write
 * @return the number of bytes written, or -1 if the file is closed for writing, -2 if the process does not have stdout assigned
*/
int writeStdout(const void * buffer, uint32_t size);

/**
 * Writes to the standard error
 * @param buffer the buffer containing the data to write
 * @param size the size of the data to write
 * @return the number of bytes written, or -1 if the file is closed for writing, -2 if does the process not have stderr assigned
*/
int writeStderr(const void * buffer, uint32_t size);

/**
 * Sets a process to waiting state
 * @param pid the PID of the process to set as waiting
 * @return void
*/
int setWaiting(Pid pid);

/**
 * Wakes up the process with the given PID
 * @param pid the PID of the process to wake up
 * @return void
*/
void wakeProcess(Pid pid);

/**
 * Returns the PID of the current process
 * @return the PID of the current process
*/
Pid getPID();

/**
 * Returns the information of the process with the given PID
 * @param pid the PID of the process to get information from
 * @return a Process struct with the information of the process
*/
Process getProcess(Pid pid);

/**
 * Returns a list of all processes in the system
 * @param count a pointer to an integer where the number of processes will be stored
 * @return a pointer to an array of Process structs with the information of all processes
*/
Process * getAllProcesses(int * count);

/**
 * Returns the PID of the main process of the group to which the current process belongs
 * @return the PID of the main process of the group
*/
Pid getProcessGroupMain();

/**
 * Returns the process with the given command
 * @param command the command of the process to get
 * @return a Process struct with the information of the process
*/
Process * getProcessByCommand(char * command);

/**
 * Returns a list of all programs in the system
 * @param count a pointer to an integer where the number of programs will be stored
 * @return a pointer to an array of Program structs with the information of all programs
*/
Program * getAllPrograms(int * count);

/**
 * Searches for a program by its command prefix
 * @param command the command prefix to search for
 * @return a pointer to the Program struct if found, NULL otherwise
*/
Program * searchProgramByPrefix(char * command);

/**
 * Installs a program in the system
 * @param program the program to install
 * @return 0 if the program was installed successfully, -1 otherwise
*/
int installProgram(Program * program);

/**
 * Uninstalls a program from the system
 * @param command the command of the program to uninstall
 * @return 0 if the program was uninstalled successfully, -1 otherwise
*/
int uninstallProgramByCommand(const char * command);

// *** Semaphore Management ***
/**
 * Initializes a semaphore with the given initial value
 * @param initial_value the initial value of the semaphore
 * @return the ID of the created semaphore, or 0 if the semaphore could not be created
*/
uint64_t sem_init(int initial_value);

/**
 * Destroys the semaphore with the given ID
 * @param id the ID of the semaphore to destroy
 * @return 0 if the semaphore was destroyed successfully, -1 if the semaphore is still in use, 1 if the semaphore was not found
*/
int sem_destroy(uint64_t id);

/**
 * Waits for the semaphore with the given ID
 * @param id the ID of the semaphore to wait for
 * @return void
*/
void sem_wait(uint64_t id);

/**
 * Posts to the semaphore with the given ID
 * @param id the ID of the semaphore to post to
 * @return void
*/
void sem_post(uint64_t id);

// *** Event Handling ***
/**
 * Subscribes to an event with the given ID
 * @param event_id the ID of the event to subscribe to
 * @param callback the callback function to call when the event is triggered
 * @param condition_data condition for receiving the event, can be NULL if no condition is needed
 * @return void
*/
void subscribeToEvent(int event_id, void (*callback)(void *), void * condition_data);

/**
 * Unsubscribes from an event with the given ID
 * @param event_id the ID of the event to unsubscribe from
 * @return void
*/
void unsubscribeFromEvent(int event_id);

/**
 * Waits for an event with the given ID
 * @param event_id the ID of the event to wait for
 * @param callback the callback function to call when the event is triggered
 * @param condition_data condition for receiving the event, can be NULL if no condition is needed
 * @return void
*/
void waitForEvent(int event_id, void * data, void * condition_data);

// *** File System Management ***
/**
 * Creates a file with the given path, type, size and permissions
 * @param path the path of the file to create
 * @param type the type of the file to create
 * @param size the size of the file to create
 * @param permissions the permissions of the file to create
 * @return the ID of the created file, or 0 if the file could not be created
*/
uint64_t mkFile(char * path, FileType type, uint32_t size, FilePermissions permissions);

/**
 * Removes the file with the given path
 * @param path the path of the file to remove
 * @return 0 if the file was removed successfully, -1 otherwise
*/
int rmFile(char * path);

/**
 * Opens a file with the given path, PID, action and type
 * @param path the path of the file to open
 * @param pid the PID of the process that is opening the file
 * @param action the action to perform on the file (read, write, etc.)
 * @param type the type of the file to open
 * @return the ID of the opened file, or 0 if the file could not be opened
*/
uint64_t openFile(char * path, Pid pid, FileAction action, FileType type);

/**
 * Closes the file with the given ID
 * @param id the ID of the file to close
 * @return 0 if the file was closed successfully, -1 otherwise
*/
int closeFile(uint64_t id, Pid pid);

/**
 * Closes a FIFO file for writing
 * @param id the ID of the FIFO file to close
 * @return 0 if the FIFO file was closed successfully, -1 otherwise
*/
int closeFifoForWriting(uint64_t id);

/**
 * Reads a raw file with the given ID into the buffer
 * @param id the ID of the file to read
 * @param buffer the buffer to read the file into
 * @param size the size of the buffer
 * @param offset the offset to start reading from
 * @return the number of bytes read, or -1 if an error occurred
*/
int readRaw(uint64_t id, void * buffer, uint32_t size, uint32_t offset);

/**
 * Writes a raw file with the given ID from the buffer
 * @param id the ID of the file to write
 * @param buffer the buffer to write the file from
 * @param size the size of the buffer
 * @param offset the offset to start writing to
 * @return the number of bytes written, or -1 if an error occurred
*/
int writeRaw(uint64_t id, void * buffer, uint32_t size, uint32_t offset);

/**
 * Reads a FIFO file with the given ID into the buffer
 * @param id the ID of the FIFO file to read
 * @param buffer the buffer to read the FIFO file into
 * @param size the size of the buffer
 * @return the number of bytes read, or -1 if an error occurred
*/
int readFifo(uint64_t id, void * buffer, uint32_t size);

/**
 * Writes a FIFO file with the given ID from the buffer
 * @param id the ID of the FIFO file to write
 * @param buffer the buffer to write the FIFO file from
 * @param size the size of the buffer
 * @return the number of bytes written, or -1 if an error occurred
*/
int writeFifo(uint64_t id, void * buffer, uint32_t size);

/**
 * Gets the file with the given ID
 * @param id the ID of the file to get
 * @return a File struct with the information of the file, or NULL if the file could not be found
*/
File getFileById(uint64_t id);

/**
 * Gets the list of files in the system
 * @param count a pointer to an integer where the number of files will be stored
 * @return a pointer to an array of File structs with the information of all files, or NULL if no files were found
*/
File * getFileList(int * count);

/**
 * Sets the permissions of the file with the given ID
 * @param id the ID of the file to set permissions for
 * @param pid the PID of the process that is setting the permissions
 * @param permissions the permissions to set for the file
 * @return 0 if the permissions were set successfully, -1 otherwise
*/
int setFilePermissions(uint64_t id, Pid pid, FilePermissions permissions);

/**
 * Gets the permissions of the file with the given ID
 * @param id the ID of the file to get permissions for
 * @return a FilePermissions struct with the permissions of the file, or NULL if the file could not be found
*/
FilePermissions getFilePermissions(uint64_t id);

/**
 * Validates if the current process has permissions to access the file with the given action
 * @param path the path of the file to validate
 * @param pid the PID of the process that is accessing the file
 * @param action the action to perform on the file (read, write, etc.)
 * @return 1 if the process has permissions, 0 otherwise
*/
int validateFileAccessPermissions(char * path, Pid pid, FileAction action);

// *** Window Management ***
/**
 * Switches to the window with the given PID
 * @param pid the PID of the window to switch to
 * @return void
*/
void switchToWindow(Pid pid);

/**
 * Gets the list of windows in the system
 * @param count a pointer to an integer where the number of windows will be stored
 * @return a pointer to an array of Pid with the PIDs of all windows, or NULL if no windows were found
 */

Pid * getWindows(int * count);

/**
 * Gets the PID of the focused window
 * @return the PID of the focused window, or 0 if no window is focused
*/
Pid getFocusedWindow();

/**
 * Checks if the window with the given PID is focused
 * @param pid the PID of the window to check
 * @return 1 if the window is focused, 0 otherwise
*/
int isFocusedWindow(Pid pid);

// ===== HERE =====

/*
 * Prints a string to the console
 * @param string the string to print
 * @return void
*/ 
void print(char * string);

/*
 * Prints a string with the given format to the console 
 * @param format the format of the string
 * @param ... the arguments to replace in the format
 * @return void
*/
void printf(char * format, ...);

/*
 * Prints a single character to the console
 * @param c the character to print
 * @return void
*/
void putChar(char c);

/*
 * Reads a character from the console
 * @return the character read
*/
char getChar();

/*
 * Clears the stdin buffer so that future reads are not yelding previously inserted chars
*/
void clearStdinBuffer();

/*
 * Reads from the console strings
 * and numbers given a certain format
 * @param format the format of the string
 * @param ... pointers to the variables to store the values
 * @return void
*/
void scanf(char * format, ...);


void seedRandom(uint64_t seed);

/*
 * Gives a random number between min and max
 * @param min the minimum value
 * @param max the maximum value
 * @return the random number
*/
uint32_t randInt(uint32_t min, uint32_t max);

void enableBackgroundAudio();
void disableBackgroundAudio();

void clear();

void sleep(uint64_t millis);


uint64_t getMillisElapsed();



#endif