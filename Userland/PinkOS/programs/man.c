#include <programs.h>
#include <libs/stdpink.h>

void man_main(char * args){
    if (args[0] == 0){
        printf((char *)"Welcome to the PinkOS manual\n");
        printf((char *)"Here you can find information about the different programs available in PinkOS\n");
        printf((char *)"To get information about a specific program use the command");
    }
    else{
        Program * program = get_program_entry(args);
        if (program == 0){
            printf((char *)"Program not found :(. Please check the name and try again\n");
        }
        else{
            printf((char *)"Name:     %s\n", program->name);
            printf((char *)"Command:  %s\n", program->command);
            printf((char *)"Description: \n%s\n", program->description);
        }
    }
}