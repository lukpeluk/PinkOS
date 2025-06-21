#ifndef PROGRAMS_H
#define PROGRAMS_H

#include <stdint.h>
#include <types.h>


typedef void (*ProgramEntry)(char*);

// typedef struct {
//     char* command;
//     char* name;
//     ProgramEntry entry;
//     uint32_t permissions;
//     char* help;         // This is the help command (a very brief description)
//     char* description;  // All the information about the command
// } Program;

Program* get_program_entry(const char* name);

char * get_name();
char * get_command();
char * get_help();
int increment_index();

void shell_main(char *args);
void echo_main(char *args);
void forrest_gump_main(char *args);
void parrot_main(char *args);
void mona_lisa_main(char *args);
void date_main(char *args);
void time_main(char *args);
void help_main(char *args);
void man_main(char *args);
void test_main(char *args);
void ps_main(char *args);
void spotify_main(char *args);
void pause_main(char *args);
void clear_main(char *args);
void resume_main(char *args);
void snake_main(char *args);
void easter_egg_main(char *args);
void pietra_main(char *args);
void test_ascii_main(char *args);
void demo_main(char *args);
void whatsapp_main(char *args);
void francis_main(char *args);
void chatgpt_main(char *args);
void set_timezone_main(char *args);
void apt_install_main(char *args);
void change_theme_main(unsigned char *args);
void wc_main(char *args);
void kill_main(char *args);
void nice_main(char *args);
void block_main(char *args);
void cat_main(char *args);
void loop_main(char *args);
void test_priority_main(char *args);
void test_processes_main(char *args);
void phylo_main(char *args);
void filter_main(char *args);
void printer_main(char *args);

#endif