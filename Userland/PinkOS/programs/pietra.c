#include <serialLib.h>
#include <programs.h>
#include <syscallCodes.h>
#include <environmentApiEndpoints.h>
#include <stdpink.h>
#include <graphicsLib.h>
#include <colors.h>

#define FRANCIS_WIDTH 640
#define FRANCIS_HEIGHT 640


void pietra_main(char * args){

    EtherPinkResponse response;

  
    make_ethereal_request((char *)"pietra\n", &response);

    uint64_t font_size = getCharWidth();
    drawString((char *)"Esperando respuesta", ColorSchema->text, ColorSchema->background, (Point){40, 40});

    int i = 0;
    while(response.code != 2){
        // clear();
        int text_width = 19 * font_size;
        i++;
        if(i % 3 == 0){
            drawString((char *)".  ", ColorSchema->text, ColorSchema->background, (Point){40 + text_width, 40});
        }
        else if(i % 3 == 1){
            drawString((char *)".. ", ColorSchema->text, ColorSchema->background, (Point){40 + text_width, 40});
        } else if(i % 3 == 2){
            drawString((char *)"...", ColorSchema->text, ColorSchema->background, (Point){40 + text_width, 40});
        }

        // wait for response
        sleep(250);
    }
    clear();

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

    printf((char *)"Bitmap dibujado con éxito.\n");
    while(1){
        sleep(500);
    }
    
}


