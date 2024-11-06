#include <programs.h>
#include <stdpink.h>
#include <graphicsLib.h>
#include <audioLib.h>
#include <songs.h>
#include <keyboard.h>
#include <stdint.h>
#include <ascii.h>

#define GAMESCREEN_SIZE 760
#define GAMEBOARD_SIZE 20
#define GRAPHICS_SCALE 38
#define REFRESH_RATE 250
#define START_OFFSET 5

// Constantes para representar objetos en el tablero
#define EMPTY 0
#define SNAKE1 0
#define SNAKE2 1
#define FOOD 3

// Macros para verificar límites y obtener valores del tablero
#define IN_BOUNDS(x, y) ((x) >= 0 && (x) < GAMEBOARD_SIZE && (y) >= 0 && (y) < GAMEBOARD_SIZE)
#define BOARD(x, y) gameboard[(y)][(x)]                                         //! ESTO ES UNA FUCKING MATRIZ ENTONCES VA AL REVÉS X E Y
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
    Direction temp_dir;
    Point head;
    Point tail;
    unsigned char controls[4];
    char type;
    uint32_t score;
    char death;
} Snake;

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
static char num_players = 1;

// Función principal
void snake_main(unsigned char *args) {
    // Se fija el argumento para la cantidad de players
    if(args[0] == '\0'){
        print("Usage: snake <players>\n");
        return;
    }
    // Checkea si el argumento es un número
    if( args[1] != '\0' || args[0] < '1' || args[0] > '2'){
        print("Invalid number of players. Please use 1 or 2 players\n");
        return;
    }
    if (args[0] == '1') num_players = 1;
    if (args[0] == '2') num_players = 2;
    



    


    init();
    last_draw_time = getMillisElapsed();
    
    while (1) {
        while (getMillisElapsed() - last_draw_time < REFRESH_RATE) {        // Espera hasta que pase el tiempo de refresco
            unsigned char c = getChar();                                    // Obtiene la última tecla presionada

            if (c == ASCII_ESC) return;                                     // Si se presiona ESC, termina el juego


            Direction dir = getDirection(&snakes[0], c);            // Chequea si la tecla presionada es una dirección
            if (dir != -1) snakes[0].temp_dir = dir;
           
            if (num_players == 2) {
                dir = getDirection(&snakes[1], c);
                if (dir != -1) snakes[1].temp_dir = dir;
            }
        }
        // Checkeque si la nueva dirección no es la opuesta a la actual
        if (snakes[0].temp_dir != (snakes[0].dir + 2) % 4) snakes[0].dir = snakes[0].temp_dir;
        if (snakes[1].temp_dir != (snakes[1].dir + 2) % 4) snakes[1].dir = snakes[1].temp_dir;
    

        last_draw_time = getMillisElapsed();

        // Mueve cada serpiente
        for (int i = 0; i < num_players; i++) {
            if (moveSnake(&snakes[i])) {
                snakes[i].death = 1;
            }
        }
        drawRectangleBorder(0xFFFFFF, GAMESCREEN_SIZE, GAMESCREEN_SIZE, 1, (Point){0, 0});
        drawScores();
        // Dibuja el tablero
        // drawGameboard();
        cycles++;
        if (snakes[0].death || snakes[1].death) {
            play_audio(songs[3].notes, 0, songs[3].tempo);
            while (1) {
                unsigned char c = getChar();
                if (c == ASCII_ESC) return;
            }
        }
    }
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

    snakes[0] = (Snake) {.length = 3, .dir = RIGHT, .controls = {'w', 'a', 's', 'd'}, .type = SNAKE1, .death = 0};
    snakes[1] = (Snake) {.length = 3, .dir = LEFT, .controls = {'i', 'j', 'k', 'l'}, .type = SNAKE2, .death = 0};
    snakes[0].tail = (Point){START_OFFSET, 10};
    snakes[1].tail = (Point){GAMEBOARD_SIZE - START_OFFSET, 10};
    snakes[0].head = (Point){START_OFFSET + snakes[0].length - 1, 10};
    snakes[1].head = (Point){GAMEBOARD_SIZE - (START_OFFSET + snakes[1].length - 1), 10};

    drawRectangleBorder(0xFFFFFF, GAMESCREEN_SIZE, GAMESCREEN_SIZE, 1, (Point){0, 0});

    // Coloca las serpientes en el tablero
    for (int i = 0; i < snakes[0].length; i++) {
        BOARD(START_OFFSET + i , 10) = 2 * cycles + snakes[0].type + START_OFFSET;
        drawRectangle(0x00FF00, GRAPHICS_SCALE, GRAPHICS_SCALE, (Point){(START_OFFSET + i) * GRAPHICS_SCALE, 10 * GRAPHICS_SCALE});
        if (num_players == 2) {
            BOARD(GAMEBOARD_SIZE - (START_OFFSET + i), 10) = 2 * cycles + snakes[1].type + START_OFFSET;
            drawRectangle(0xFF0000, GRAPHICS_SCALE, GRAPHICS_SCALE, (Point){  (GAMEBOARD_SIZE - (START_OFFSET + i)) * GRAPHICS_SCALE, 10 * GRAPHICS_SCALE});
        }
        cycles++;
    }

    // Genera la primera comida
    moveCherry();
}

void drawScores() {
    // Calcula cuando ocupa el tablero en x
    uint64_t gameboard_width = GAMEBOARD_SIZE * GRAPHICS_SCALE;
    uint64_t space_for_scores = gameboard_width + 20; // Add some padding
    uint64_t char_width = getCharWidth();

    Point info_player_1 = {space_for_scores, 100};
    Point info_player_2 = {space_for_scores, 200};


    drawString("Player 1", 0xFFFFFF, 0x000000, info_player_1);
    drawString("Score: ", 0xFFFFFF, 0x000000, (Point){space_for_scores, info_player_1.y + 50});
    drawNumber(snakes[0].score, 0xFFFFFF, 0x000000, (Point){space_for_scores + 7 * char_width, info_player_1.y + 50});

    if (num_players == 1) {
        if (snakes[0].death) {
            incFontSize();
            drawString("GAME OVER", 0xFFFFFF, 0x000000, (Point){space_for_scores, 10});
        }
        return;
    }
    drawString("Player 2", 0xFFFFFF, 0x000000, info_player_2);
    drawString("Score: ", 0xFFFFFF, 0x000000, (Point){space_for_scores, info_player_2.y + 50});
    drawNumber(snakes[1].score, 0xFFFFFF, 0x000000, (Point){space_for_scores + 7 * char_width, info_player_2.y + 50});
    
    char is_both_dead = snakes[0].death && snakes[1].death;
    if (is_both_dead) {
        incFontSize();
        drawString("DRAW", 0xFFFFFF, 0x000000, (Point){space_for_scores, 10});
        return;
    }else if(snakes[0].death){
        incFontSize();
        drawString("SNAKE 2 WINS", 0xFF0000, 0x000000, (Point){space_for_scores, 10});
        return;
    }else if(snakes[1].death){
        incFontSize();
        drawString("SNAKE 1 WINS", 0x00FF00, 0x000000, (Point){space_for_scores, 10});
        return;
    }
    

}

int moveSnake(Snake *snake) {
    // Calcula la nueva posición de la cabeza
    Point new_head = getNewPosition(snake->head, snake->dir);
    int collision = checkCollision(new_head);

    // if (collision == 1) return 1; // Colisión fatal

    // Si colisiona quiero que quede el juego estático
    if(collision == 1) return 1;

    //! COMENTO ESTO ASI HAGO MOVER LA MF VIBORITA PRIMERO
    // Si la serpiente ha comido, aumenta la longitud y coloca la comida en otra parte
    if (collision == 2) {
        snake->score++;
        snake->length++;
        moveCherry();
    }

    // Mueve la cola solo si no ha comido
    if (collision != 2) {
        // Busco la nueva cola
        Point new_tail = getNewTail(snake);
        // Elimino LA ANTERIOR del tablero
        BOARD(snake->tail.x, snake->tail.y) = EMPTY; // Limpia la cola en el tablero
        // Elimino LA ANTERIOR del dibujo
        drawRectangle(0x000000, GRAPHICS_SCALE, GRAPHICS_SCALE, (Point){snake->tail.x * GRAPHICS_SCALE, snake->tail.y * GRAPHICS_SCALE});
        // Guardo la nueva cola
        snake->tail = new_tail;
    }

    // Mueve la cabeza
    //  Cambio la cabeza en el array
    snake->head = new_head;
    //  Agrego la cabeza en el dibujo
    drawRectangle(snake->type == SNAKE1 ? 0x00FF00 : 0xFF0000, GRAPHICS_SCALE, GRAPHICS_SCALE, (Point){new_head.x * GRAPHICS_SCALE, new_head.y * GRAPHICS_SCALE});
    //  Agrego la cabeza en el tablero
    BOARD(new_head.x, new_head.y) = 2 * cycles + snake->type + START_OFFSET;

    return 0;
}

int checkCollision(Point position) {
    if (!IN_BOUNDS(position.x, position.y)) return 1; // Colisión con el borde
    unsigned char obj_in_position = BOARD(position.x, position.y);
    if (obj_in_position == FOOD) return 2; // Colisión con comida
    if (obj_in_position != 0) {
        
        return 1; 
    }// Colisión con otra serpiente o sí misma
    return 0; // No hay colisión
}

Point getNewPosition(Point pos, Direction dir) {
    switch (dir) {
        case UP: return (Point){pos.x, pos.y - 1};
        case DOWN: return (Point){pos.x, pos.y + 1};
        case LEFT: return (Point){pos.x - 1, pos.y};
        case RIGHT: return (Point){pos.x + 1, pos.y};
    }
    return pos;
}

void moveCherry() {
    Point new_cherry;
    do {
        new_cherry = (Point){randInt(0, GAMEBOARD_SIZE-1), randInt(0, GAMEBOARD_SIZE-1)};
    } while (BOARD(new_cherry.x, new_cherry.y) != EMPTY);
    food = new_cherry;
    BOARD(new_cherry.x, new_cherry.y) = FOOD;
    drawRectangle(0xFFD700, GRAPHICS_SCALE, GRAPHICS_SCALE, (Point){new_cherry.x * GRAPHICS_SCALE, new_cherry.y * GRAPHICS_SCALE});
}

// Obtiene la dirección de la tecla presionada, si es una dirección válida, dada una snake
Direction getDirection(const Snake * snake, unsigned char key) {
    for (int i = 0; i < 4; i++)
        if (key == snake->controls[i])     // chequea que sea direcc válida y que no sea para el lado opuesto
            return i;
    return -1;                                                          // si no es una dirección válida
}

Point getNewTail(Snake *snake) {
    // Busca el menor numero cercano que sea de type de la snake (arriba, abajo, izquierda o derecha)
    Point old_tail = snake->tail;
    Point new_tail;
    int min = 1000000;
    for (int i = 0; i < 4; i++) {
        Point new_pos = getNewPosition(old_tail, i);
        if (BOARD(new_pos.x, new_pos.y) != 0 && BOARD(new_pos.x, new_pos.y) % 2 != snake->type && BOARD(new_pos.x, new_pos.y) < min && BOARD(new_pos.x, new_pos.y) != FOOD) {
            min = BOARD(new_pos.x, new_pos.y);
            new_tail = new_pos;
        }
    }
    return new_tail;
}