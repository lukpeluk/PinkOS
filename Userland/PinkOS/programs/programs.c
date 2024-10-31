#include <programs.h>
#include <syscallCodes.h>

int strcmp(const char* a, const char* b) {
    while (*a && *b && *a == *b) {
        a++;
        b++;
    }
    return *a - *b;
}


typedef struct {
    char* name;
    ProgramEntry entry;
} Program;

Program programs[] = {
    {"echo", echo_main},
};

int programs_count = sizeof(programs) / sizeof(Program);


void* get_program_entry(const char* name) {
    for (int i = 0; i < programs_count; i++) {
        if (strcmp(programs[i].name, name) == 0) {
            return programs[i].entry;
        }
    }
    return 0;
}