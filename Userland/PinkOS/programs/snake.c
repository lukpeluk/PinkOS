#include <programs.h>
#include <stdpink.h>
#include <graphicsLib.h>
#include <keyboard.h>
#include <stdint.h>
#include <ascii.h>

#define GAMESCREEN_SIZE 760
#define GAMEBOARD_SIZE 20
#define GRAPHICS_SCALE 38
#define REFRESH_RATE 1000
#define START_OFFSET 5

// Constantes para representar objetos en el tablero
#define EMPTY 0
#define SNAKE1 0
#define SNAKE2 1
#define FOOD 3

// Macros para verificar límites y obtener valores del tablero
#define IN_BOUNDS(x, y) ((x) >= 0 && (x) < GAMEBOARD_SIZE && (y) >= 0 && (y) < GAMEBOARD_SIZE)
#define BOARD(x, y) gameboard[(x)][(y)]
#define IS_EMPTY(x, y) (IN_BOUNDS((x), (y)) && BOARD((x), (y)) == EMPTY)

// Tamaño máximo de la serpiente (ajústalo según tus necesidades)
#define MAX_SNAKE_LENGTH (GAMEBOARD_SIZE * GAMEBOARD_SIZE)

typedef enum {
    NONE = -1,
    UP,
    LEFT,
    DOWN,
    RIGHT
} Direction;

typedef struct {
    int length;
    Direction dir;
    Point head;
    Point tail;
    char controls[4];
    char type;
    uint32_t score;
} Snake;

void drawGameboard();
void moveCherry();
int moveSnake(Snake *snake);
int checkCollision(Point position);
Direction getDirection(const Snake *snake, unsigned char key);
Point getNewPosition(Point pos, Direction dir);
Point getNewTail(Snake *snake);
Point getNewCherryPosition();
void init();

// Variables de juego
static uint64_t last_draw_time = 0;
static unsigned int gameboard[GAMEBOARD_SIZE][GAMEBOARD_SIZE];
static Snake snakes[2];
static Point food;
static uint32_t cycles = 0;

// Función principal
void snake_main(unsigned char *args) {
    init();
    last_draw_time = getMillisElapsed();
    
    // // Hardcodeo los primeros 3 movimientos para testear
    // while(1){
        print("Primera vuelta\n");
        while(getMillisElapsed() - last_draw_time < REFRESH_RATE) {
            // Primera vuelta:
            //                  - snake 1: derecha (se mantiene)
            //                  - snake 2: arriba (cambia)
            snakes[1].dir = UP;

        }
        last_draw_time = getMillisElapsed();

        moveSnake(&snakes[0]);
        moveSnake(&snakes[1]);

        // Imprimir en stdin el tablero
        for (int i = 0; i < GAMEBOARD_SIZE; i++) {
            for (int j = 0; j < GAMEBOARD_SIZE; j++) {
                printf("%d ", BOARD(i, j));
            }
            printf("\n");
        }

        print("Segunda vuelta\n");
        while(getMillisElapsed() - last_draw_time < REFRESH_RATE) {
            // Segunda vuelta:
            //                  - snake 1: abajo (cambia)
            //                  - snake 2: derecha (cambia)
            snakes[0].dir = DOWN;
            snakes[1].dir = RIGHT;
        }
        last_draw_time = getMillisElapsed();

        moveSnake(&snakes[0]);
        moveSnake(&snakes[1]);
        // Imprimir en stdin el tablero
        for (int i = 0; i < GAMEBOARD_SIZE; i++) {
            for (int j = 0; j < GAMEBOARD_SIZE; j++) {
                printf("%d ", BOARD(i, j));
            }
            printf("\n");
        }

        print("Tercera vuelta\n");
        while(getMillisElapsed() - last_draw_time < REFRESH_RATE) {
            // Tercera vuelta:
            //                  - snake 1: izquierda (cambia)
            //                  - snake 2: abajo (cambia y muere)
            snakes[0].dir = LEFT;
            snakes[1].dir = DOWN;
        }
        last_draw_time = getMillisElapsed();

        moveSnake(&snakes[0]);
        moveSnake(&snakes[1]);
        // Imprimir en stdin el tablero
        for (int i = 0; i < GAMEBOARD_SIZE; i++) {
            for (int j = 0; j < GAMEBOARD_SIZE; j++) {
                printf("%d ", BOARD(i, j));
            }
            printf("\n");
        }
        print("Termino\n");
    // }
    
    // while (1) {
    //     while (getMillisElapsed() - last_draw_time < REFRESH_RATE) {        // Espera hasta que pase el tiempo de refresco
    //         unsigned char c = getChar();                                    // Obtiene la última tecla presionada

    //         if (c == ASCII_ESC) return;                                     // Si se presiona ESC, termina el juego


    //         Direction dir = getDirection(&snakes[0], c);            // Chequea si la tecla presionada es una dirección
    //         if (dir != -1) snakes[0].dir = dir;
           
            
    //         dir = getDirection(&snakes[1], c);
    //         if (dir != -1) snakes[1].dir = dir;
    //     }

    //     last_draw_time = getMillisElapsed();

    //     // Mueve cada serpiente
    //     for (int i = 0; i < 2; i++) {
    //         if (moveSnake(&snakes[i])) {
    //             // Si una serpiente choca, termina el juego
    //             return;
    //         }
    //     }

    //     // Dibuja el tablero
    //     // drawGameboard();
    //     cycles++;
    // }
}

// Inicialización del juego
void init() {

    cycles = 0; //? No sé si es necesario
    
    seedRandom(getMillisElapsed());

    // Inicializa el tablero vacío
    for (int i = 0; i < GAMEBOARD_SIZE; i++) {
        for (int j = 0; j < GAMEBOARD_SIZE; j++) {
            gameboard[i][j] = EMPTY;
        }
    }

    snakes[0] = (Snake) {.length = 3, .dir = RIGHT, .controls = {'w', 'a', 's', 'd'}, .type = SNAKE1};
    snakes[1] = (Snake) {.length = 3, .dir = LEFT, .controls = {'i', 'j', 'k', 'l'}, .type = SNAKE2};
    snakes[0].tail = (Point){START_OFFSET, 10};
    snakes[1].tail = (Point){GAMEBOARD_SIZE - START_OFFSET, 10};
    snakes[0].head = (Point){START_OFFSET + snakes[0].length - 1, 10};
    snakes[1].head = (Point){GAMEBOARD_SIZE - (START_OFFSET + snakes[1].length - 1), 10};

    // Coloca las serpientes en el tablero
    for (int i = 0; i < snakes[0].length; i++) {
        BOARD(START_OFFSET + i , 10) = 2 * cycles + snakes[0].type + START_OFFSET;
        drawRectangle(0x00FF00, GRAPHICS_SCALE, GRAPHICS_SCALE, (Point){10 * GRAPHICS_SCALE,(START_OFFSET + i) * GRAPHICS_SCALE});
        BOARD(GAMEBOARD_SIZE - (START_OFFSET + i), 10) = 2 * cycles + snakes[1].type + START_OFFSET;
        drawRectangle(0xFF0000, GRAPHICS_SCALE, GRAPHICS_SCALE, (Point){ 10 * GRAPHICS_SCALE, (GAMEBOARD_SIZE - (START_OFFSET + i)) * GRAPHICS_SCALE});
        cycles++;
    }

    // Genera la primera comida
    moveCherry();
}

void drawGameboard() {
    for (int i = 0; i < GAMEBOARD_SIZE; i++) {
        for (int j = 0; j < GAMEBOARD_SIZE; j++) {
            if (BOARD(i, j) == SNAKE1) {
                drawRectangle(0x00FF00, GRAPHICS_SCALE, GRAPHICS_SCALE, (Point){j * GRAPHICS_SCALE, i * GRAPHICS_SCALE});
            } else if (BOARD(i, j) == SNAKE2) {
                drawRectangle(0xFF0000, GRAPHICS_SCALE, GRAPHICS_SCALE, (Point){j * GRAPHICS_SCALE, i * GRAPHICS_SCALE});
            } else if (BOARD(i, j) == FOOD) {
                drawRectangle(0xFFD700, GRAPHICS_SCALE, GRAPHICS_SCALE, (Point){j * GRAPHICS_SCALE, i * GRAPHICS_SCALE});
            } else {
                drawRectangle(0x000000, GRAPHICS_SCALE, GRAPHICS_SCALE, (Point){j * GRAPHICS_SCALE, i * GRAPHICS_SCALE});
            }
        }
    }
}

int moveSnake(Snake *snake) {
    // Calcula la nueva posición de la cabeza
    Point new_head = getNewPosition(snake->head, snake->dir);
    int collision = checkCollision(new_head);

    // if (collision == 1) return 1; // Colisión fatal

    // Si colisiona quiero que quede el juego estático
    if(collision == 1) while(1);


    // Si la serpiente ha comido, aumenta la longitud y coloca la comida en otra parte
    if (collision == 2) {
        snake->score++;
        snake->length++;
        moveCherry();
    }

    // Mueve la cola solo si no ha comido
    if (collision != 2) {
        Point tail = getNewTail(snake);
        BOARD(tail.x, tail.y) = EMPTY; // Limpia la cola en el tablero
        drawRectangle(0x000000, GRAPHICS_SCALE, GRAPHICS_SCALE, (Point){tail.y * GRAPHICS_SCALE, tail.x * GRAPHICS_SCALE});
    }

    // Mueve la cabeza
    snake->head = new_head;
    drawRectangle(snake->type == SNAKE1 ? 0x00FF00 : 0xFF0000, GRAPHICS_SCALE, GRAPHICS_SCALE, (Point){new_head.y * GRAPHICS_SCALE, new_head.x * GRAPHICS_SCALE});

    BOARD(new_head.x, new_head.y) = 2 * cycles + snake->type + START_OFFSET;

    return 0;
}

int checkCollision(Point position) {
    if (!IN_BOUNDS(position.x, position.y)) return 1; // Colisión con el borde
    unsigned char obj_in_position = BOARD(position.x, position.y);
    if (obj_in_position == FOOD) return 2; // Colisión con comida
    if (obj_in_position != 0) return 1; // Colisión con otra serpiente o sí misma
    return 0; // No hay colisión
}

Point getNewPosition(Point pos, Direction dir) {
    switch (dir) {
        case UP: return (Point){pos.x - 1, pos.y};
        case DOWN: return (Point){pos.x + 1, pos.y};
        case LEFT: return (Point){pos.x, pos.y - 1};
        case RIGHT: return (Point){pos.x, pos.y + 1};
    }
    return pos;
}

void moveCherry() {
    Point new_cherry;
    do {
        new_cherry = (Point){randInt(0, GAMEBOARD_SIZE), randInt(0, GAMEBOARD_SIZE)};
    } while (BOARD(new_cherry.x, new_cherry.y) != EMPTY);
    food = new_cherry;
    BOARD(new_cherry.x, new_cherry.y) = FOOD;
    drawRectangle(0xFFD700, GRAPHICS_SCALE, GRAPHICS_SCALE, (Point){new_cherry.y * GRAPHICS_SCALE, new_cherry.x * GRAPHICS_SCALE});
}

// Obtiene la dirección de la tecla presionada, si es una dirección válida, dada una snake
Direction getDirection(const Snake * snake, unsigned char key) {
    for (int i = 0; i < 4; i++)
        if (key == snake->controls[i] && (i + 2) % 4 != snake->dir)     // chequea que sea direcc válida y que no sea para el lado opuesto
            return i;
    return -1;                                                          // si no es una dirección válida
}

Point getNewTail(Snake *snake) {
    // Busca el menor numero cercano que sea de type de la snake (arriba, abajo, izquierda o derecha)
    Point new_tail = snake->tail;
    int min = 1000000;
    for (int i = 0; i < 4; i++) {
        Point new_pos = getNewPosition(new_tail, i);
        if (BOARD(new_pos.x, new_pos.y) % 2 == snake->type && BOARD(new_pos.x, new_pos.y) < min) {
            min = BOARD(new_pos.x, new_pos.y);
            new_tail = new_pos;
        }
    }
    return new_tail;
    
}