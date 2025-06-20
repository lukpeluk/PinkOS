#include <programs.h>
#include <syscalls/syscallCodes.h>
#include <permissions.h>
#include <libs/stdpink.h>

// RUNNABLE PROGRAMS
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
        "usage: monalisa <scale (range 1-15)>\nMona Lisa v2 \n Draws the ==Mona Lisa== on the screen, animating it to the desired scale passed as argument (range 1-15). \nAlso plays the chromatic scale at 120 bpm whole notes coz it's fancy."
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
        "==Omg==, you're reading the manual of the manual.\nThis is a bit meta, don't you think?"
    },
    {
        "ps",
        "PS",
        ps_main,
        MANAGE_PROCESSES_PERMISSION,
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
        "Plays a song ",
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
    {
        "whatsapp",
        "Whatsapp",
        whatsapp_main,
        MAKE_ETHEREAL_REQUEST_PERMISSION,
        "Mandate mensajitos con el puerto serial",
        "usage: whatsapp \nWhatsapp v1 \n Use this program to chat with your friends via serial port."
    },
    {
        "francis",
        "Francis",
        francis_main,
        MAKE_ETHEREAL_REQUEST_PERMISSION | DRAWING_PERMISSION,
        "LO AMO",
        "usage: francis \nFrancis v1 \n Use this program to see the love of your life."
    },
    {
        "pietra",
        "Pietra",
        pietra_main,
        MAKE_ETHEREAL_REQUEST_PERMISSION | DRAWING_PERMISSION,
        "Dibujar un bitmap",
        "usage: pietra \nPietra v1 \n Use this program to draw a bitmap on the screen."
    },
    {
        "chatgpt",
        "ChatGPT",
        chatgpt_main,
        MAKE_ETHEREAL_REQUEST_PERMISSION,
        "Chat with GPT-3",
        "usage: chatgpt \nChatGPT v1 \n Use this program to chat with GPT-3."
    },
    {
        "set_timezone",
        "Set Timezone",
        set_timezone_main,
        SET_TIMEZONE_PERMISSION,
        "Set the timezone",
        "usage: set_timezone <timezone> \nSet Timezone v1 \n Use this program to set the timezone of the system."
    },
    {
        "apt_install",
        "Apt Install",
        apt_install_main,
        MAKE_ETHEREAL_REQUEST_PERMISSION | CHANGE_FONT_SIZE_PERMISSION,
        "Install a package",
        "usage: apt_install \nApt Install v1 \n Use this program to install a package."
    },
    {
        "changetheme",
        "Change Theme",
        change_theme_main,
        0,
        "Toggle between the default and mockup color themes",
        "usage: chagetheme \nChangeTheme v1 \n Use this command to change the color theme."
    },
    {
        "wc",
        "Word Count",
        wc_main,
        0,
        "Counts the number of words in a file",
        "usage: wc \nWord Count v1 \n Use this program to count the number of words in a file."
    },
    {
        "kill",
        "Kill",
        kill_main,
        MANAGE_PROCESSES_PERMISSION,
        "Kill a process",
        "usage: kill <pid> \nKill v1 \n Use this program to kill a process by its PID."
    },
    {
        "nice",
        "Nice",
        nice_main,
        MANAGE_PROCESSES_PERMISSION,
        "Change the priority of a process",
        "usage: nice <pid> <new_priority>\nNice v1"
    },
    {
        "block",
        "Block",
        block_main,
        MANAGE_PROCESSES_PERMISSION,
        "Block a process",
        "usage: block <pid> \nBlock v1 \n Use this program to block a process by its PID. The process will not be able to run until it is unblocked."
    },
    {
        "cat",
        "Cat",
        cat_main,
        0,
        "Concatenate and print files",
        "usage: cat <file> \nCat v1 \n Use this program to concatenate and print files to the console."
    },
    {
        "loop",
        "Loop",
        loop_main,
        0,
        "Runs a program in a loop",
        "usage: loop <program> \nLoop v1 \n Use this program to run a program in a loop until it is killed."
    },
    {
        "test",
        "Test",
        test_main,
        0,
        "Test exception handling",
        "usage: test <args> \nTest v1 \n Use this program to test the exception handling"
    },
    {
        "test_prio",
        "Test Priority",
        test_priority_main,
        ROOT_PERMISSIONS & ~DRAWING_PERMISSION, // ROOT_PERMISSIONS without DRAWING_PERMISSION
        "Test the priority system",
        "usage: test_prio \nTest Priority v1 \n Use this program to test the priority system of PinkOS. It creates 3 processes with different priorities and tests the priority system."
    }
    
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