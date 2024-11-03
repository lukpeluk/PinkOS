#ifndef PROGRAMS_H
#define PROGRAMS_H

#include <stdint.h>


typedef void (*ProgramEntry)(char*);

typedef struct {
    char* command;
    char* name;
    ProgramEntry entry;
    uint32_t perms;
    char* help;         // This is the help command (a very brief description)
    char* description;  // All the information about the command
} Program;

Program* get_program_entry(const char* name);

char * get_name();
char * get_command();
char * get_help();
int increment_index();

void echo_main(char *args);
void forrest_gump_main(char *args);
void parrot_main(char *args);
void mona_lisa_main(char *args);
void date_main(char *args);
void time_main(char *args);
void help_main(char *args);
void man_main(char *args);
void test_main(char *args);


#endif