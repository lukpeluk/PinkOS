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
        (unsigned char *)"echo", 
        (unsigned char *)"echo", 
        echo_main, 
        0,
        (unsigned char *)"Prints the argument to the console",
        (unsigned char *)"usage: echo <text> \nEcho v1 \n Use this program to print text to the screen"
    },
    {
        (unsigned char *)"forrestgump",
        (unsigned char *)"Forrest Gump",
        forrest_gump_main,
        0,
        (unsigned char *)"Run Forrest, run!",
        (unsigned char *)"usage: forrest_gump \nForrest Gump v1 \n Runs, and runs, and runs..."
    },
    {
        (unsigned char *)"parrot",
        (unsigned char *)"Parrot",
        parrot_main,
        0,
        (unsigned char *)"Like echo but for user input via stdin",
        (unsigned char *)"usage: parrot \nParrot v1 \n Repeats what you input via stdin, runs until forced to quit"
    },
    {
        (unsigned char *)"monalisa",
        (unsigned char *)"Mona Lisa",
        mona_lisa_main,
        DRAWING_PERMISSION | PLAY_AUDIO_PERMISSION,
        (unsigned char *)"Draws the Mona Lisa",
        (unsigned char *)"usage: monalisa <scale (range 1-15)>\nMona Lisa v2 \n Draws the Mona Lisa on the screen, animating it to the desired scale passed as argument (range 1-15). \nAlso plays the chromatic scale at 120 bpm whole notes coz it's fancy."
    },
    {
        (unsigned char *)"date",
        (unsigned char *)"Date",
        date_main,
        0,
        (unsigned char *)"Prints the current date",
        (unsigned char *)"usage: date \nDate v1 \n Prints the current date"
    },
    {
        (unsigned char *)"time",
        (unsigned char *)"Time",
        time_main,
        0,
        (unsigned char *)"Prints the current time",
        (unsigned char *)"usage: time \nTime v1 \n Prints the current time"
    },
    {
        (unsigned char *)"help",
        (unsigned char *)"Help",
        help_main,
        0,
        (unsigned char *)"Prints the help menu",
        (unsigned char *)"usage: help \nHelp v1 \n Prints the help menu"
    },
    {
        (unsigned char *)"man",
        (unsigned char *)"MAN",
        man_main,
        0,
        (unsigned char *)"Prints the manual of a program",
        (unsigned char *)"Omg, you're reading the manual of the manual.\nThis is a bit meta, don't you think?"
    },
    {
        (unsigned char *)"test",
        (unsigned char *)"Test",
        test_main,
        0,
        (unsigned char *)"Test program",
        (unsigned char *)"usage: test <args> \nTest v1 \n Use this program to test the exception handling"
    },
    {
        (unsigned char *)"ps",
        (unsigned char *)"PS",
        ps_main,
        0,
        (unsigned char *)"Prints the list of running processes",
        (unsigned char *)"usage: ps \nPS v1 \n Prints the list of running processes"
    },
    {
        (unsigned char *)"snake",
        (unsigned char *)"Snake",
        snake_main,
        DRAWING_PERMISSION | PLAY_AUDIO_PERMISSION,
        (unsigned char *)"Snake game",
        (unsigned char *)"usage: snake <players> \nSnake v3.9 \n Use this program to play the snake game. \n You can play with 1 or 2 players. \n Player 1 controls: WASD \n Player 2 controls: IJKL \n Press ESC to exit the game. \n Have fun!"
    },
    {
        (unsigned char *)"spotify",
        (unsigned char *)"Spotify",
        spotify_main,
        PLAY_AUDIO_PERMISSION,
        (unsigned char *)"Plays a song",
        (unsigned char *)"Usage: spotify [-b] <song name>\nSpotify v2.0\n\nOptions:\n-b for playing the song in loop in the backgound while you go on with your life.\n\nAvailable songs:\n\t * Pink Panther theme\n\t* Super Mario Bros theme\n\t* Fur Elise\n\t* Never Gonna Give You Up\n\nIf not in background mode, you can use space to pause/resume the song.\n"
    },
    {
        (unsigned char *)"pause",
        (unsigned char *)"Pause",
        pause_main,
        PLAY_AUDIO_PERMISSION,
        (unsigned char *)"Pauses the current song",
        (unsigned char *)"usage: pause \nPause v1 \n Pauses the current song"
    },
    {
        (unsigned char *)"resume",
        (unsigned char *)"Resume",
        resume_main,
        PLAY_AUDIO_PERMISSION,
        (unsigned char *)"Resumes the current song",
        (unsigned char *)"usage: resume \nResume v1 \n Resumes the current song"
    },
    {
        (unsigned char *)"clear",
        (unsigned char *)"Clear",
        clear_main,
        0,
        (unsigned char *)"Clears the screen",
        (unsigned char *)"usage: clear \nClear v1 \n Clears the screen (you can still scroll up if you want to see the previous output)"
    },
    {
        (unsigned char *)"ls",
        (unsigned char *)"List",
        easter_egg_main,
        DRAWING_PERMISSION | PLAY_AUDIO_PERMISSION,
        (unsigned char *)"List files",
        (unsigned char *)"usage: ls \nList v1 \n List files in the current directory"
    },
    {
        (unsigned char *)"ascii",
        (unsigned char *)"Test ascii",
        test_ascii_main,
        0,
        (unsigned char *)"Tests all ascii chars in stdout in a loop.",
        (unsigned char *)"Usage: ascii\nTest ascii v1\n Tests all ascii chars in stdout in a loop."
    },
    {
        (unsigned char *)"demo",
        (unsigned char *)"Demo",
        demo_main,
        DRAWING_PERMISSION | PLAY_AUDIO_PERMISSION, // | 0b10000000000000000000000000000000 ,
        (unsigned char *)"Demo program for showcasing permissions",
        (unsigned char *)"usage: demo \nDemo v1 \n Use this program to showcase the permission system of PinkOS in the demonstration."
    }, 
    {
        (unsigned char *)"whatsapp",
        (unsigned char *)"Whatsapp",
        whatsapp_main,
        MAKE_ETHEREAL_REQUEST_PERMISSION,
        (unsigned char *)"Mandate mensajitos con el puerto serial",
        (unsigned char *)"usage: whatsapp \nWhatsapp v1 \n Use this program to chat with your friends via serial port."
    },
    {
        (unsigned char *)"francis",
        (unsigned char *)"Francis",
        francis_main,
        MAKE_ETHEREAL_REQUEST_PERMISSION | DRAWING_PERMISSION,
        (unsigned char *)"LO AMO",
        (unsigned char *)"usage: francis \nFrancis v1 \n Use this program to see the love of your life."
    },
    {
        (unsigned char *)"pietra",
        (unsigned char *)"Pietra",
        pietra_main,
        MAKE_ETHEREAL_REQUEST_PERMISSION | DRAWING_PERMISSION,
        (unsigned char *)"Dibujar un bitmap",
        (unsigned char *)"usage: pietra \nPietra v1 \n Use this program to draw a bitmap on the screen."
    },
    {
        (unsigned char *)"chatgpt",
        (unsigned char *)"ChatGPT",
        chatgpt_main,
        MAKE_ETHEREAL_REQUEST_PERMISSION,
        (unsigned char *)"Chat with GPT-3",
        (unsigned char *)"usage: chatgpt \nChatGPT v1 \n Use this program to chat with GPT-3."
    },
    {
        (unsigned char *)"set_timezone",
        (unsigned char *)"Set Timezone",
        set_timezone_main,
        SET_TIMEZONE_PERMISSION,
        (unsigned char *)"Set the timezone",
        (unsigned char *)"usage: set_timezone <timezone> \nSet Timezone v1 \n Use this program to set the timezone of the system."
    }
    
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