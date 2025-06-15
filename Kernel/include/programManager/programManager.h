#ifndef PROGRAM_MANAGER_H
#define PROGRAM_MANAGER_H

#include <types.h>

// GET PROGRAM
Program* getProgramByCommand(const char* command);
Program* getProgramByIndex(int index);

// LIST PROGRAMS
int getProgramsCount(void);
Program* getAllPrograms(void);

// SEARCH PROGRAM (for autocompletion)
Program* searchProgramByPrefix(const char* prefix);
int getMatchingCommands(const char* prefix, char** results, int max_results);

// INTERNAL MANAGEMENT (for initialization)
void initProgramManager(void);
void setProgramsCount(int count);

// INSTALL/UNINSTALL PROGRAM
int installProgram(Program* program);
int uninstallProgramByCommand(const char* command);
int uninstallProgramByIndex(int index);
void cleanupProgramManager(void);

#endif