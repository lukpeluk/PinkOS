#include <serialLib.h>
#include <programs.h>
#include <syscallCodes.h>
#include <environmentApiEndpoints.h>
#include <stdpink.h>
#include <graphicsLib.h>

#define FRANCIS_WIDTH 300
#define FRANCIS_HEIGHT 300

uint64_t memcmp(const void *s1, const void *s2, uint64_t n) {
    const unsigned char *p1 = s1, *p2 = s2;
    while (n--) {
        if (*p1 != *p2) {
            return *p1 - *p2;
        }
        p1++;
        p2++;
    }
    return 0;
}

void francis_main(unsigned char * args){

    EtherPinkResponse response;

  
    make_ethereal_request((unsigned char *)"francis\n", &response);

    while(response.code != 2){
        // wait for response
        sleep(100);
    }

    Point position = {0};

    int screen_width = getScreenWidth();
    int screen_height = getScreenHeight();
    int scale = 3;


    uint32_t *raw_data = (uint32_t *)response.raw_data;



    position.x = (screen_width - FRANCIS_WIDTH * scale) / 2;
    position.y = (screen_height - FRANCIS_HEIGHT * scale) / 2;

    // Interpretar los datos como un array de uint32_t
    uint32_t *francis = (uint32_t *)(raw_data );

    // Dibujar el bitmap
    drawBitmap(francis, FRANCIS_WIDTH, FRANCIS_HEIGHT, position, scale);

    printf((unsigned char *)"Bitmap dibujado con éxito.\n");
    while(1){
        sleep(500);
    }
    
}


