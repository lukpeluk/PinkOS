#include <programs.h>
#include <stdpink.h>

void man_main(unsigned char * args){
    if (args[0] == 0){
        printf((unsigned char *)"Welcome to the PinkOS manual\n");
        printf((unsigned char *)"Here you can find information about the different programs available in PinkOS\n");
        printf((unsigned char *)"To get information about a specific program use the command");
    }
    else{
        Program * program = get_program_entry(args);
        if (program == 0){
            printf((unsigned char *)"Program not found :(. Please check the name and try again\n");
        }
        else{
            printf((unsigned char *)"Name:     %s\n", program->name);
            printf((unsigned char *)"Command:  %s\n", program->command);
            printf((unsigned char *)"Description: \n%s\n", program->description);
        }
    }
}