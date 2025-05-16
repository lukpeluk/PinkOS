#include <programs.h>
#include <syscallCodes.h>
#include <permissions.h>

int strcmp(const char* a, const char* b) {
    while (*a && *b && *a == *b) {
        a++;
        b++;
    }
    return *a - *b;
}


static Program programs[] = {
    {
        (char *)"echo", 
        (char *)"echo", 
        echo_main, 
        0,
        (char *)"Prints the argument to the console",
        (char *)"usage: echo <text> \nEcho v1 \n Use this program to print text to the screen"
    },
    {
        (char *)"forrestgump",
        (char *)"Forrest Gump",
        forrest_gump_main,
        0,
        (char *)"Run Forrest, run!",
        (char *)"usage: forrest_gump \nForrest Gump v1 \n Runs, and runs, and runs..."
    },
    {
        (char *)"parrot",
        (char *)"Parrot",
        parrot_main,
        0,
        (char *)"Like echo but for user input via stdin",
        (char *)"usage: parrot \nParrot v1 \n Repeats what you input via stdin, runs until forced to quit"
    },
    {
        (char *)"monalisa",
        (char *)"Mona Lisa",
        mona_lisa_main,
        DRAWING_PERMISSION | PLAY_AUDIO_PERMISSION,
        (char *)"Draws the Mona Lisa",
        (char *)"usage: monalisa <scale (range 1-15)>\nMona Lisa v2 \n Draws the Mona Lisa on the screen, animating it to the desired scale passed as argument (range 1-15). \nAlso plays the chromatic scale at 120 bpm whole notes coz it's fancy."
    },
    {
        (char *)"date",
        (char *)"Date",
        date_main,
        0,
        (char *)"Prints the current date",
        (char *)"usage: date \nDate v1 \n Prints the current date"
    },
    {
        (char *)"time",
        (char *)"Time",
        time_main,
        0,
        (char *)"Prints the current time",
        (char *)"usage: time \nTime v1 \n Prints the current time"
    },
    {
        (char *)"help",
        (char *)"Help",
        help_main,
        0,
        (char *)"Prints the help menu",
        (char *)"usage: help \nHelp v1 \n Prints the help menu"
    },
    {
        (char *)"man",
        (char *)"MAN",
        man_main,
        0,
        (char *)"Prints the manual of a program",
        (char *)"Omg, you're reading the manual of the manual.\nThis is a bit meta, don't you think?"
    },
    {
        (char *)"test",
        (char *)"Test",
        test_main,
        0,
        (char *)"Test program",
        (char *)"usage: test <args> \nTest v1 \n Use this program to test the exception handling"
    },
    {
        (char *)"ps",
        (char *)"PS",
        ps_main,
        0,
        (char *)"Prints the list of running processes",
        (char *)"usage: ps \nPS v1 \n Prints the list of running processes"
    },
    {
        (char *)"snake",
        (char *)"Snake",
        snake_main,
        DRAWING_PERMISSION | PLAY_AUDIO_PERMISSION,
        (char *)"Snake game",
        (char *)"usage: snake <players> \nSnake v3.9 \n Use this program to play the snake game. \n You can play with 1 or 2 players. \n Player 1 controls: WASD \n Player 2 controls: IJKL \n Press ESC to exit the game. \n Have fun!"
    },
    {
        (char *)"spotify",
        (char *)"Spotify",
        spotify_main,
        PLAY_AUDIO_PERMISSION,
        (char *)"Plays a song",
        (char *)"Usage: spotify [-b] <song name>\nSpotify v2.0\n\nOptions:\n-b for playing the song in loop in the backgound while you go on with your life.\n\nAvailable songs:\n\t * Pink Panther theme\n\t* Super Mario Bros theme\n\t* Fur Elise\n\t* Never Gonna Give You Up\n\nIf not in background mode, you can use space to pause/resume the song.\n"
    },
    {
        (char *)"pause",
        (char *)"Pause",
        pause_main,
        PLAY_AUDIO_PERMISSION,
        (char *)"Pauses the current song",
        (char *)"usage: pause \nPause v1 \n Pauses the current song"
    },
    {
        (char *)"resume",
        (char *)"Resume",
        resume_main,
        PLAY_AUDIO_PERMISSION,
        (char *)"Resumes the current song",
        (char *)"usage: resume \nResume v1 \n Resumes the current song"
    },
    {
        (char *)"clear",
        (char *)"Clear",
        clear_main,
        0,
        (char *)"Clears the screen",
        (char *)"usage: clear \nClear v1 \n Clears the screen (you can still scroll up if you want to see the previous output)"
    },
    {
        (char *)"ls",
        (char *)"List",
        easter_egg_main,
        DRAWING_PERMISSION | PLAY_AUDIO_PERMISSION,
        (char *)"List files",
        (char *)"usage: ls \nList v1 \n List files in the current directory"
    },
    {
        (char *)"ascii",
        (char *)"Test ascii",
        test_ascii_main,
        0,
        (char *)"Tests all ascii chars in stdout in a loop.",
        (char *)"Usage: ascii\nTest ascii v1\n Tests all ascii chars in stdout in a loop."
    },
    {
        (char *)"demo",
        (char *)"Demo",
        demo_main,
        DRAWING_PERMISSION | PLAY_AUDIO_PERMISSION, // | 0b10000000000000000000000000000000 ,
        (char *)"Demo program for showcasing permissions",
        (char *)"usage: demo \nDemo v1 \n Use this program to showcase the permission system of PinkOS in the demonstration."
    }, 
    {
        (char *)"whatsapp",
        (char *)"Whatsapp",
        whatsapp_main,
        MAKE_ETHEREAL_REQUEST_PERMISSION,
        (char *)"Mandate mensajitos con el puerto serial",
        (char *)"usage: whatsapp \nWhatsapp v1 \n Use this program to chat with your friends via serial port."
    },
    {
        (char *)"francis",
        (char *)"Francis",
        francis_main,
        MAKE_ETHEREAL_REQUEST_PERMISSION | DRAWING_PERMISSION,
        (char *)"LO AMO",
        (char *)"usage: francis \nFrancis v1 \n Use this program to see the love of your life."
    },
    {
        (char *)"pietra",
        (char *)"Pietra",
        pietra_main,
        MAKE_ETHEREAL_REQUEST_PERMISSION | DRAWING_PERMISSION,
        (char *)"Dibujar un bitmap",
        (char *)"usage: pietra \nPietra v1 \n Use this program to draw a bitmap on the screen."
    },
    {
        (char *)"chatgpt",
        (char *)"ChatGPT",
        chatgpt_main,
        MAKE_ETHEREAL_REQUEST_PERMISSION,
        (char *)"Chat with GPT-3",
        (char *)"usage: chatgpt \nChatGPT v1 \n Use this program to chat with GPT-3."
    },
    {
        (char *)"set_timezone",
        (char *)"Set Timezone",
        set_timezone_main,
        SET_TIMEZONE_PERMISSION,
        (char *)"Set the timezone",
        (char *)"usage: set_timezone <timezone> \nSet Timezone v1 \n Use this program to set the timezone of the system."
    },
    {
        (char *)"apt_install",
        (char *)"Apt Install",
        apt_install_main,
        MAKE_ETHEREAL_REQUEST_PERMISSION | CHANGE_FONT_SIZE_PERMISSION,
        (char *)"Install a package",
        (char *)"usage: apt_install \nApt Install v1 \n Use this program to install a package."
    },
    
    // TODO: ↓↓↓↓↓↓
    // {"shutdown", "shutdown", 0, 0, "usage: shutdown", "shutdown v1 \n use this program to shutdown the system"},
    // {"reboot", "reboot", 0, 0, "usage: reboot", "reboot v1 \n use this program to reboot the system"},
};

int programs_count = sizeof(programs) / sizeof(Program);
// int programs_count = 3;


Program * get_program_entry(const char* command) {
    for (int i = 0; i < programs_count; i++) {
        if (strcmp(programs[i].command, command) == 0) {
            return &programs[i];
        }
    }
    return 0;
}

static int index_to_actual_program = 0;

char * get_name(){
    return programs[index_to_actual_program].name;
}

char * get_command(){
    return programs[index_to_actual_program].command;
}

char * get_help(){
    return programs[index_to_actual_program].help;
}

int increment_index(){
    index_to_actual_program++;
    if(index_to_actual_program >= programs_count){
        index_to_actual_program = 0;
    }
    return index_to_actual_program;
}