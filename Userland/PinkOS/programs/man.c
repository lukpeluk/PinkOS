#include <programs.h>
#include <stdpink.h>

void man_main(char * args){
    if (args[0] == 0){
        printf("Welcome to the PinkOS manual\n");
        printf("Here you can find information about the different programs available in PinkOS\n");
        printf("To get information about a specific program use the command");
    }
    else{
        Program * program = get_program_entry(args);
        if (program == 0){
            printf("Program not found :(. Please check the name and try again\n");
        }
        else{
            printf("Name:     %s\n", program->name);
            printf("Command:  %s\n", program->command);
            printf("Description: \n%s\n", program->description);
        }
    }
}