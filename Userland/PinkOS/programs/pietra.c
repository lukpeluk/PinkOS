#include <serialLib.h>
#include <programs.h>
#include <syscallCodes.h>
#include <environmentApiEndpoints.h>
#include <stdpink.h>
#include <graphicsLib.h>

#define PIETRA_WIDTH 640
#define PIETRA_HEIGHT 640

void pietra_main(unsigned char * args){

    EtherPinkResponse response;

  
    make_ethereal_request("pietra\n", &response);

    while(response.code != 2){
        // wait for response
        sleep(100);
    }

    Point position = {0};

    int screen_width = getScreenWidth();
    int screen_height = getScreenHeight();
    int scale = 1;


    uint32_t *raw_data = (uint32_t *)response.raw_data;



    position.x = (screen_width - PIETRA_WIDTH * scale) / 2;
    position.y = (screen_height - PIETRA_HEIGHT * scale) / 2;

    // Interpretar los datos como un array de uint32_t
    uint32_t *pietra = (uint32_t *)(raw_data );

    // Dibujar el bitmap
    drawBitmap((unsigned char *)pietra, PIETRA_WIDTH, PIETRA_HEIGHT, position, scale);

    printf("Bitmap dibujado con éxito.\n");
    while(1){
        sleep(500);
    }
    
}


