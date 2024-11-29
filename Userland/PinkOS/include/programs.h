#ifndef PROGRAMS_H
#define PROGRAMS_H

#include <stdint.h>


typedef void (*ProgramEntry)(unsigned char*);

typedef struct {
    unsigned char* command;
    unsigned char* name;
    ProgramEntry entry;
    uint32_t perms;
    unsigned char* help;         // This is the help command (a very brief description)
    unsigned char* description;  // All the information about the command
} Program;

Program* get_program_entry(const unsigned char* name);

unsigned char * get_name();
unsigned char * get_command();
unsigned char * get_help();
int increment_index();

void echo_main(unsigned char *args);
void forrest_gump_main(unsigned char *args);
void parrot_main(unsigned char *args);
void mona_lisa_main(unsigned char *args);
void date_main(unsigned char *args);
void time_main(unsigned char *args);
void help_main(unsigned char *args);
void man_main(unsigned char *args);
void test_main(unsigned char *args);
void ps_main(unsigned char *args);
void spotify_main(unsigned char *args);
void pause_main(unsigned char *args);
void clear_main(unsigned char *args);
void resume_main(unsigned char *args);
void snake_main(unsigned char *args);
void easter_egg_main(unsigned char *args);
void pietra_main(unsigned char *args);
void test_ascii_main(unsigned char *args);
void demo_main(unsigned char *args);
void whatsapp_main(unsigned char *args);

#endif