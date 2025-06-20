#include <programs.h>
#include <libs/stdpink.h>

void help_main(char * args){

    do
    {   
        char * command_name = get_command();
        if (strncmp(command_name, "test", 4) == 0) {
            printf((char *)">#   * %12s - %s\n", command_name, get_help());
        } else {
            printf((char *)"   * %12s - %s\n", get_command(), get_help());
        }
        sleep(20); // sleep to avoid flooding the console with output
    } while (increment_index());

}
