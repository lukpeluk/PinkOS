#ifndef PICTURE_RUNTIME_LIB_H
#define PICTURE_RUNTIME_LIB_H

#include <stdint.h>
#include <stdpink.h>
#include <libs/graphicsLib.h>
#include <keyboard.h>

// Forward declarations
struct Component;
struct GuiContext;
typedef struct Component Component;
typedef struct GuiContext GuiContext;

// Default colors for component constructor
extern uint32_t default_bg_color;
extern uint32_t default_text_color;
extern uint32_t default_border_color;

// __va_args__ permite sobrescribir los valores por defecto
#define COMPONENT_CONSTRUCTOR(...)      \
    (Component){                        \
        .bg_color = &default_bg_color,  \
        .text = 0,                      \
        .text_color = &default_text_color, \
        .text_size = 0,                 \
        .border_size = 1,               \
        .border_color = &default_border_color, \
        .width = 100,                   \
        .height = 100,                  \
        .alignment = ALIGN_CENTER,      \
        .y_position = -1,               \
        .active = 1,                    \
        .children = 0,                  \
        .children_count = 0,            \
        .parent = 0,                    \
        .on_press = 0,                  \
        .on_key_press = 0,              \
        .on_focus_gain = 0,             \
        .on_focus_lost = 0,             \
        .needs_full_redraw = 1,         \
        __VA_ARGS__                     \
    }

typedef enum {
    ALIGN_LEFT = 0,
    ALIGN_CENTER,
    ALIGN_RIGHT
} Alignment;


// Callback para on_press, recibe el contexto y el componente que fue presionado
typedef void (*OnPressCallback)(struct GuiContext* context, struct Component* comp);
// Callback para on_key_press, recibe el contexto, el componente que tiene el foco y el evento de teclado
typedef void (*OnKeyPressCallback)(struct GuiContext* context, struct Component* comp, KeyboardEvent* event);
// Callback para on_focus, recibe el contexto y el componente que recibió el foco
typedef void (*OnFocusCallback)(struct GuiContext* context, struct Component* comp);

// --- COMPONENTE ---
struct Component {
    // Estilos
    uint32_t* bg_color;
    char* text;
    uint32_t* text_color;
    int text_size;         // es relativo a la escala del sistema
    int border_size;
    uint32_t* border_color;

    // Layout
    int width;              // Porcentaje del parent
    int height;
    Alignment alignment;    // Respecto a su parent, horizontalmente
    int y_position;         // Posición vertical absoluta, -1 para auto (distribuido uniformemente)

    int active;   // Si es 0 no se renderiza ni es seleccionable

    // Estructura de árbol
    struct Component* children; // Apunta directamente al inicio del array de hijos contiguos
    int children_count;
    struct Component* parent;

    // Comportamiento
    OnPressCallback on_press;
    OnKeyPressCallback on_key_press;
    OnFocusCallback on_focus_gain;
    OnFocusCallback on_focus_lost;

    // Variables (dependen del programa, se definen en sus headers y cada uno casteará como necesite)
    void* variables; // Cada componente tiene la lista de variables, para poder sobrescribirlas en su scope. Si la tiene como NULL el valor es el del padre

    // Handler metadata (definida por los handlers y el sistema para guardar estados internos del componente, dependen del programa)
    void * metadata;

    // Render
    int needs_full_redraw;      // 1 si necesita redibujarse todo, ejemplo cuando un hijo cambia de posición o cambia el color del fondo
};


// --- CONTEXTO DE LA APP ---
struct GuiContext {
    Component * root;
    Component * focused;
    int focused_index;
    Component ** selection_order; // Array null terminated de punteros a los componentes seleccionables y en qué orden iterar
    int selectable_count;
};


void render_tree(GuiContext * context);

GuiContext initialize_gui_context(Component* root, Component** selection_order, int selectable_count);

void iterate_focused(GuiContext * context, int direction);
void unfocus(GuiContext * context);

int is_input_captured(GuiContext * context);

void call_focused_key_handler(GuiContext * context, KeyboardEvent* event);
void call_focused_press_handler(GuiContext * context);


#endif