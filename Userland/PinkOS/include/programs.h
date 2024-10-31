#ifndef PROGRAMS_H
#define PROGRAMS_H

typedef void (*ProgramEntry)(char*);

void* get_program_entry(const char* name);

void echo_main(char *args);


#endif