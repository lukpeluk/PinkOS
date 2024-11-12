#include <programs.h>
#include <syscallCodes.h>
#include <permissions.h>

int strcmp(const unsigned char* a, const unsigned char* b) {
    while (*a && *b && *a == *b) {
        a++;
        b++;
    }
    return *a - *b;
}


static Program programs[] = {
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
        "usage: forrest_gump \nForrest Gump v1 \n Runs, and runs, and runs..."
    },
    {
        "parrot",
        "Parrot",
        parrot_main,
        0,
        "Like echo but for user input via stdin",
        "usage: parrot \nParrot v1 \n Repeats what you input via stdin, runs until forced to quit"
    },
    {
        "monalisa",
        "Mona Lisa",
        mona_lisa_main,
        DRAWING_PERMISSION | PLAY_AUDIO_PERMISSION,
        "Draws the Mona Lisa",
        "usage: monalisa <scale (range 1-15)>\nMona Lisa v2 \n Draws the Mona Lisa on the screen, animating it to the desired scale passed as argument (range 1-15). \nAlso plays the chromatic scale at 120 bpm whole notes coz it's fancy."
    },
    {
        "date",
        "Date",
        date_main,
        0,
        "Prints the current date",
        "usage: date \nDate v1 \n Prints the current date"
    },
    {
        "time",
        "Time",
        time_main,
        0,
        "Prints the current time",
        "usage: time \nTime v1 \n Prints the current time"
    },
    {
        "help",
        "Help",
        help_main,
        0,
        "Prints the help menu",
        "usage: help \nHelp v1 \n Prints the help menu"
    },
    {
        "man",
        "MAN",
        man_main,
        0,
        "Prints the manual of a program",
        "Omg, you're reading the manual of the manual.\nThis is a bit meta, don't you think?"
    },
    {
        "test",
        "Test",
        test_main,
        0,
        "Test program",
        "usage: test <args> \nTest v1 \n Use this program to test the exception handling"
    },
    {
        "ps",
        "PS",
        ps_main,
        0,
        "Prints the list of running processes",
        "usage: ps \nPS v1 \n Prints the list of running processes"
    },
    {
        "snake",
        "Snake",
        snake_main,
        DRAWING_PERMISSION | PLAY_AUDIO_PERMISSION,
        "Snake game",
        "usage: snake <players> \nSnake v3.9 \n Use this program to play the snake game. \n You can play with 1 or 2 players. \n Player 1 controls: WASD \n Player 2 controls: IJKL \n Press ESC to exit the game. \n Have fun!"
    },
    {
        "spotify",
        "Spotify",
        spotify_main,
        PLAY_AUDIO_PERMISSION,
        "Plays a song",
        "Usage: spotify [-b] <song name>\nSpotify v2.0\n\nOptions:\n-b for playing the song in loop in the backgound while you go on with your life.\n\nAvailable songs:\n\t * Pink Panther theme\n\t* Super Mario Bros theme\n\t* Fur Elise\n\t* Never Gonna Give You Up\n\nIf not in background mode, you can use space to pause/resume the song.\n"
    },
    {
        "pause",
        "Pause",
        pause_main,
        PLAY_AUDIO_PERMISSION,
        "Pauses the current song",
        "usage: pause \nPause v1 \n Pauses the current song"
    },
    {
        "resume",
        "Resume",
        resume_main,
        PLAY_AUDIO_PERMISSION,
        "Resumes the current song",
        "usage: resume \nResume v1 \n Resumes the current song"
    },
    {
        "clear",
        "Clear",
        clear_main,
        0,
        "Clears the screen",
        "usage: clear \nClear v1 \n Clears the screen (you can still scroll up if you want to see the previous output)"
    },
    {
        "ls",
        "List",
        easter_egg_main,
        DRAWING_PERMISSION | PLAY_AUDIO_PERMISSION,
        "List files",
        "usage: ls \nList v1 \n List files in the current directory"
    },
    {
        "ascii",
        "Test ascii",
        test_ascii_main,
        0,
        "Tests all ascii chars in stdout in a loop.",
        "Usage: ascii\nTest ascii v1\n Tests all ascii chars in stdout in a loop."
    },
    {
        "demo",
        "Demo",
        demo_main,
        DRAWING_PERMISSION | PLAY_AUDIO_PERMISSION, // | 0b10000000000000000000000000000000 ,
        "Demo program for showcasing permissions",
        "usage: demo \nDemo v1 \n Use this program to showcase the permission system of PinkOS in the demonstration."
    }, 
    // TODO: ↓↓↓↓↓↓
    // {"shutdown", "shutdown", 0, 0, "usage: shutdown", "shutdown v1 \n use this program to shutdown the system"},
    // {"reboot", "reboot", 0, 0, "usage: reboot", "reboot v1 \n use this program to reboot the system"},
};

int programs_count = sizeof(programs) / sizeof(Program);
// int programs_count = 3;


Program * get_program_entry(const unsigned char* command) {
    for (int i = 0; i < programs_count; i++) {
        if (strcmp(programs[i].command, command) == 0) {
            return &programs[i];
        }
    }
    return 0;
}

static int index_to_actual_program = 0;

unsigned char * get_name(){
    return programs[index_to_actual_program].name;
}

unsigned char * get_command(){
    return programs[index_to_actual_program].command;
}

unsigned char * get_help(){
    return programs[index_to_actual_program].help;
}

int increment_index(){
    index_to_actual_program++;
    if(index_to_actual_program >= programs_count){
        index_to_actual_program = 0;
    }
    return index_to_actual_program;
}