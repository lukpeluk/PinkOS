#include <programs.h>
#include <libs/stdpink.h>

void help_main(char * args){

    do
    {
        printf((char *)"   * %12s - %s\n", get_command(), get_help());
        sleep(20); // sleep to avoid flooding the console with output
    } while (increment_index());

}
