#include <programManager/programManager.h>
#include <drivers/serialDriver.h>
#include <lib.h>
#include <memoryManager/memoryManager.h>

// Program manager configuration
#define PROGRAM_BLOCK_SIZE 10  // Number of slots to add when expanding the array

// Forward declarations for program main functions can be added as needed when installing programs

// Dynamic array of programs - starts empty, programs are installed dynamically
Program* programs = 0;
static int programs_count = 0;
static int programs_capacity = 0;

// No initial programs - the system starts empty and programs are installed dynamically


// Internal function to check if string starts with prefix
int startsWith(const char* str, const char* prefix) {
    if (!str || !prefix) return 0;
    
    while (*prefix) {
        if (*str != *prefix) return 0;
        str++;
        prefix++;
    }
    return 1;
}

// GET PROGRAM
Program* getProgramByCommand(const char* command) {
    if (!command) return 0;
    
    for (int i = 0; i < programs_count; i++) {
        if (programs[i].command && strcmp(programs[i].command, command) == 0) {
            return &programs[i];
        }
    }
    
    return 0;
}

Program* getProgramByIndex(int index) {
    if (index < 0 || index >= programs_count) {
        return 0;
    }
    
    return &programs[index];
}

// LIST PROGRAMS
int getProgramsCount(void) {
    return programs_count;
}

Program* getAllPrograms(void) {
    return programs;
}

// SEARCH PROGRAM (for autocompletion)
Program* searchProgramByPrefix(const char* prefix) {
    if (!prefix) return 0;
    
    for (int i = 0; i < programs_count; i++) {
        if (programs[i].command && startsWith(programs[i].command, prefix)) {
            return &programs[i];
        }
    }
    
    return 0;
}

int getMatchingCommands(const char* prefix, char** results, int max_results) {
    if (!prefix || !results || max_results <= 0) return 0;
    
    int count = 0;
    for (int i = 0; i < programs_count && count < max_results; i++) {
        if (programs[i].command && startsWith(programs[i].command, prefix)) {
            results[count] = programs[i].command;
            count++;
        }
    }
    
    return count;
}

// INTERNAL MANAGEMENT FUNCTIONS

// Initialize the program manager with dynamic allocation
void initProgramManager(void) {
    if (programs) {
        free(programs);
    }
    
    // Start with empty program manager
    programs_count = 0;
    programs_capacity = 0; // No initial allocation - will allocate on first install
    programs = 0;
}

// Utility function to update the program count
void setProgramsCount(int count) {
    programs_count = count;
}

// INSTALL/UNINSTALL PROGRAM FUNCTIONS

// Install a new program
// Returns 1 if installed successfully (or it was already installed), 0 if installation failed
int installProgram(Program* program) {
    if (!program) return 0;
    
    // First installation - allocate initial block
    if (!programs) {
        programs_capacity = PROGRAM_BLOCK_SIZE;
        programs = (Program*)malloc(sizeof(Program) * programs_capacity);
        if (!programs) {
            programs_count = 0;
            programs_capacity = 0;
            return 0; // Memory allocation failed
        }
    }
    
    // Check if program already exists
    for (int i = 0; i < programs_count; i++) {
        if (programs[i].command && strcmp(programs[i].command, program->command) == 0) {
            return 1; // Program already exists
        }
    }
    
    // Check if we need to resize the array
    if (programs_count >= programs_capacity) {
        programs_capacity += PROGRAM_BLOCK_SIZE;
        Program* new_programs = (Program*)realloc(programs, sizeof(Program) * programs_capacity);
        if (!new_programs) {
            return 0; // Memory allocation failed
        }
        programs = new_programs;
    }
    
    // Add the new program
    programs[programs_count] = *program;
    programs_count++;
    // log_to_serial("I: Program installed: ");
    // log_to_serial(program->command);
    return 1; // Success
}

// Uninstall program by command name
int uninstallProgramByCommand(const char* command) {
    if (!command || !programs || programs_count == 0) return 0;
    
    // Find the program
    for (int i = 0; i < programs_count; i++) {
        if (programs[i].command && strcmp(programs[i].command, command) == 0) {
            // Shift all programs after this one back by one position
            for (int j = i; j < programs_count - 1; j++) {
                programs[j] = programs[j + 1];
            }
            programs_count--;
            return 1; // Success
        }
    }
    
    return 0; // Program not found
}

// Uninstall program by index
int uninstallProgramByIndex(int index) {
    if (index < 0 || index >= programs_count || !programs || programs_count == 0) return 0;
    
    // Shift all programs after this one back by one position
    for (int i = index; i < programs_count - 1; i++) {
        programs[i] = programs[i + 1];
    }
    programs_count--;
    
    return 1; // Success
}

// Cleanup program manager (free allocated memory)
void cleanupProgramManager(void) {
    if (programs) {
        free(programs);
        programs = 0;
    }
    programs_count = 0;
    programs_capacity = 0;
}