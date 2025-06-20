#include <libs/stdpink.h>

void loop_main(char *args) {
    uint64_t millis = 1000;
    if (args != NULL && args[0] != '\0') {
        uint64_t seconds = string_to_uint64(args);
        printf((char *)"Looping every %d seconds\n", (unsigned int)seconds);
        millis = seconds * 1000;
        if (millis == 0) {
            millis = 1000; 
        }
    }

    while (1) {
        Pid pid = getPID();
        printf((char *)"Loop process PID: %d\n", pid);
        sleep(millis);  
    }
}