#include <programs.h>
#include <stdpink.h>

void help_main(unsigned char * args){

    do
    {
        printf("   * %12s - %s\n", get_command(), get_help());
    } while (increment_index());

}
