// #include <programs.h>
// #include <stdpink.h>
// #include <graphicsLib.h>
// #include <stdint.h>

// #define GAMESCREEN_SIZE 760
// #define GAMEBOARD_SIZE 20
// #define GRAPHICS_SCALE 38


// void drawGameboard(unsigned char gameboard[20][20], int scale);

// static unsigned char gameboard[GAMEBOARD_SIZE][GAMEBOARD_SIZE]; //? No sé si va static acá
// static Point food;
// static Point snakes[2][2];   // Matriz donde cada fila es una snake y cada columna es una parte de la snake (0 cabeza, 1 cola)



void snake_main(unsigned char * args){
    return;
}
//     // initialize gameboard
//     for(int i = 0; i < 20; i++){
//         for(int j = 0; j < 20; j++){
//             gameboard[i][j] = 0;
//         }
//     }


//     drawRectangleBorder(0x0000FF, GAMESCREEN_SIZE, GAMESCREEN_SIZE, 1, (Point){4, 4});
//     while(1){
//         moveCherry();

//         // Falta la lógica de encontrar para donde se quiere mover la snake para pasarlo
//         moveSnake(1, (Point){0, 0});
//         moveSnake(2, (Point){0, 0});
//         drawGameboard(gameboard, GRAPHICS_SCALE);
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

// Point findTail(unsigned char snake){
//     // Busco arriba, abajo, izquierda y derecha de la cola (snakes[snake][1]) para ver donde está la siguiente
//     // parte de la cola, chequeando que no salgo de los bordes
//     Point tail = snakes[snake][1];
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