// #include <programs.h>
// #include <stdpink.h>
// #include <graphicsLib.h>
// #include <keyboard.h>
// #include <stdint.h>

// #define GAMESCREEN_SIZE 760
// #define GAMEBOARD_SIZE 20
// #define GRAPHICS_SCALE 38

// #define REFRESH_RATE 500

// typedef enum {
//     UP,
//     DOWN,
//     LEFT,
//     RIGHT
// } Direction;
// typedef struct {
//     Point head;
//     Point tail;
//     Direction dir;
//     char controlls[4];
//     char type;
// } Snake;

// void drawGameboard(unsigned char gameboard[20][20], int scale);

// static uint64_t last_draw_time = 0;

// static unsigned int gameboard[GAMEBOARD_SIZE][GAMEBOARD_SIZE]; //? No sé si va static acá
// static Point food;
// static Snake snakes[2] = {
//     {
//         .head = {0, 0},
//         .tail = {0, 0},
//         .dir = RIGHT,
//         .controlls = {'w', 's', 'a', 'd'},
//         .type = 0
//     },
//     {
//         .head = {0, 0},
//         .tail = {0, 0},
//         .dir = RIGHT,
//         .controlls = {'i', 'k', 'j', 'l'},
//         .type = 1
//     }
// };

void snake_main(unsigned char * args){
}

//     // initialize gameboard
//     for(int i = 0; i < 20; i++){
//         for(int j = 0; j < 20; j++){
//             gameboard[i][j] = 0;
//         }
//     }
//     // initialize snakes
//     gameboard[5][10] = 1;
//     gameboard[6][10] = 3;
//     gameboard[7][10] = 5;
//     snakes[0].head = (Point){7, 10};
//     snakes[0].tail = (Point){5, 10};



//     gameboard[15][10] = 6;
//     gameboard[16][10] = 4;
//     gameboard[17][10] = 2;
//     snakes[1].head = (Point){15, 10};
//     snakes[1].tail = (Point){17, 10};



//     // drawRectangleBorder(0x0000FF, GAMESCREEN_SIZE, GAMESCREEN_SIZE, 1, (Point){4, 4});
//     while(1){
//         while (getMillisElapsed() - last_draw_time < REFRESH_RATE){
//             KeyboardEvent event = getKeyboardEvent();
//             if(event.event_type == 1){
//                 Direction dir = getDirection(snakes[0].controlls, event);
//                 if(dir != -1){
//                     snakes[0].dir = dir;
//                 }
//                 dir = getDirection(snakes[1].controlls, event);
//                 if(dir != -1){
//                     snakes[1].dir = dir;
//                 }
//             }
//         }

//         moveCherry();

//         // Falta la lógica de encontrar para donde se quiere mover la snake para pasarlo
//         moveSnake(1, (Point){0, 0});
//         moveSnake(2, (Point){0, 0});
//         // drawGameboard(gameboard, GRAPHICS_SCALE);
//         syscall(SLEEP_SYSCALL, 1000, 0, 0, 0, 0);
//     }
// }

// void drawGameboard(){
//     for(int i = 0; i < 20; i++){
//         for(int j = 0; j < 20; j++){
//             if(gameboard[i][j] == 1){
//                 // draw snake 1
//             } else if(gameboard[i][j] == 2){
//                 // draw snake 2
//             } else if(gameboard[i][j] == 3){
//                 // draw cherry
//             } else {
//                 // draw empty space (esto si no tenemos un "background")
//             }

//         }
//     }
// }

// // Recibe que snake mover y la posición a la que se quiere mover
// int moveSnake(unsigned char snake, Point position){
//     // Chequeo que hay en la posición a la que se quiere mover
//     int collision = checkCollision(position);
//     if(collision == 1){  // Si choco murió
//         return 1;
//     }
//     // Si no murio muevo la cabeza
//     snakes[snake][0] = position;
//     gameboard[position.x][position.y] = snake;

//     // Chequeo si comió para ver si muevo la cola
//     if(collision == 0){  // Si no hay nada en la posición
//         // Muevo la cola
//         Point tail = findTail(snake);                               // Encuentro la cola
//         gameboard[snakes[snake][1].x][snakes[snake][1].y] = 0;      // Borro la cola en el tablero
//         snakes[snake][1] = tail;                                    // Muevo la cola del array de snakes
//     }
//     return 0;
// }

// // retorna 0 si no hubo colisión, 1 si hubo colisión con la pared o cuerpo (game over), 2 si hubo colisión con la comida
// int checkCollision(Point position){
//     unsigned char obj_in_position = gameboard[position.x][position.y];
//     if(obj_in_position == 0) {  // si no hay nada en la posición
//         return 0;
//     }
//     if(obj_in_position == 3) { // si choco con la comida
//         return 2;
//     }
//     if(obj_in_position == 1 || obj_in_position == 2) { // si choco con alguna snake
//         return 1;
//     }
//     if(position.x < 0 || position.x >= 20 || position.y < 0 || position.y >= 20){ // si choco con la pared
//         return 1;
//     }
//     return -1; // si no es ninguna de las anteriores (no debería pasar) ???
// }

// Point findTail(Snake snake){
//     // Busco arriba, abajo, izquierda y derecha de la cola (snakes[snake][1]) para ver donde está la siguiente
//     // parte de la cola, chequeando que no salgo de los bordes
//     Point tail = snake.tail;
//     Point next_tail = tail;
//     if(tail.x > 0 && gameboard[tail.x - 1][tail.y] == snake){
//         next_tail = (Point){tail.x - 1, tail.y};
//     }
//     if(tail.x < GAMEBOARD_SIZE-1 && gameboard[tail.x + 1][tail.y] == snake){
//         next_tail = (Point){tail.x + 1, tail.y};
//     }
//     if(tail.y > 0 && gameboard[tail.x][tail.y - 1] == snake){
//         next_tail = (Point){tail.x, tail.y - 1};
//     }
//     if(tail.y < GAMEBOARD_SIZE-1 && gameboard[tail.x][tail.y + 1] == snake){
//         next_tail = (Point){tail.x, tail.y + 1};
//     }
//     return next_tail;

// }

// void moveCherry(){
//     // Busco una posición random en el tablero que no esté ocupada por ninguna snake
//     Point new_cherry;
//     do{
//         new_cherry = (Point){randInt(0, GAMEBOARD_SIZE), randInt(0, GAMEBOARD_SIZE)};
//     } while(gameboard[new_cherry.x][new_cherry.y] != 0);
//     food = new_cherry;
//     gameboard[new_cherry.x][new_cherry.y] = 3;
// }

// // Me devuelve la posición del menor (par o impar seugn corresponda) a los alrededores de pos (Solo arriva, abajo, izquierda y derecha)
// Point searchMinor(Point pos, int parity)
// {
//     Point minor = pos;
//     if (parity == 0)
//     {
//         if (pos.x > 0 && gameboard[pos.x - 1][pos.y] < gameboard[minor.x][minor.y])
//         {
//             minor = (Point){pos.x - 1, pos.y};
//         }
//         if (pos.x < GAMEBOARD_SIZE - 1 && gameboard[pos.x + 1][pos.y] < gameboard[minor.x][minor.y])
//         {
//             minor = (Point){pos.x + 1, pos.y};
//         }
//     }
//     else
//     {
//         if (pos.y > 0 && gameboard[pos.x][pos.y - 1] < gameboard[minor.x][minor.y])
//         {
//             minor = (Point){pos.x, pos.y - 1};
//         }
//         if (pos.y < GAMEBOARD_SIZE - 1 && gameboard[pos.x][pos.y + 1] < gameboard[minor.x][minor.y])
//         {
//             minor = (Point){pos.x, pos.y + 1};
//         }
//     }
//     return minor;
// }

// Direction getDirection(const char *controlls, KeyboardEvent event)
// {
//     for (int i = 0; i < 4; i++)
//         if (event.ascii == controlls[i])
//             return i;
    
//     return -1;
// }