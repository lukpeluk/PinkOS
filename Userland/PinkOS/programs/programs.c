#include <programs.h>
#include <syscallCodes.h>

int strcmp(const char* a, const char* b) {
    while (*a && *b && *a == *b) {
        a++;
        b++;
    }
    return *a - *b;
}


Program programs[] = {
    {
        "echo", 
        "echo", 
        echo_main, 
        0,
        "Prints the argument to the console",
        "usage: echo <text> \nEcho v1 \n Use this program to print text to the screen"
    },
    {
        "forrestgump",
        "Forrest Gump",
        forrest_gump_main,
        0,
        "Run Forrest, run!",
        "usage: forrest_gump \nForrest Gump v1 \n Simplemente corre, y corre..."
    }
    // TODO: ↓↓↓↓↓↓
    // {"help", "help", 0, 0, "usage: help <command>", "Help v1 \n use this program to get help on a command"},
    // {"clear", "clear", 0, 0, "usage: clear", "clear v1 \n use this program to clear the screen"},
    // {"date", "date", 0, 0, "usage: date", "date v1 \n use this program to print the current date"},
    // {"time", "time", 0, 0, "usage: time", "time v1 \n use this program to print the current time"},
    // {"shutdown", "shutdown", 0, 0, "usage: shutdown", "shutdown v1 \n use this program to shutdown the system"},
    // {"reboot", "reboot", 0, 0, "usage: reboot", "reboot v1 \n use this program to reboot the system"},
    // {"ps", "ps", 0, 0,  "usage: ps", "ps v1 \n use this program to list all running processes ;)"}
};

// int programs_count = sizeof(programs) / sizeof(Program);
int programs_count = 2;


Program * get_program_entry(const char* command) {
    for (int i = 0; i < programs_count; i++) {
        if (strcmp(programs[i].command, command) == 0) {
            return &programs[i];
        }
    }
    return 0;
}