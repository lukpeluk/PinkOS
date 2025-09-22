#include <programs.h>
#include <libs/stdpink.h>
#include <libs/graphicsLib.h>
#include <libs/events.h>
#include <keyboard.h>
#include <colors.h>

// Forward declarations
struct Component;
static void input_key_handler(void* context, struct Component* comp, KeyboardEvent* event);


// ---> LIBRERÍA Y FUNCIONES AUXILIARES DEL RUNTIME DE PICTURE <---
// ----------------------------------------------------------------

typedef enum {
    ALIGN_LEFT,
    ALIGN_CENTER,
    ALIGN_RIGHT
} Alignment;

// Callback para on_press, recibe el contexto y el componente que fue presionado
typedef void (*OnPressCallback)(void* context, struct Component* comp);
// Callback para on_key_press, recibe el contexto, el componente que tiene el foco y el evento de teclado
typedef void (*OnKeyPressCallback)(void* context, struct Component* comp, KeyboardEvent* event);

// --- COMPONENTE ---
typedef struct Component {
    // Estilos
    uint32_t bg_color;
    uint32_t text_color;
    char* text;

    // Layout
    int width;              // Ambos porcentajes de su parent
    int height;
    Alignment alignment;    // Respecto a su parent y considerando las dimensiones

    // Estructura de árbol
    struct Component* children; // Apunta directamente al inicio del array de hijos contiguos
    int children_count;
    struct Component* parent;

    // Comportamiento
    OnPressCallback on_press;
    OnKeyPressCallback on_key_press;

    // Render
    int needs_full_redraw;      // 1 si necesita redibujarse todo, ejemplo cuando un hijo cambia de posición o cambia el color del fondo
} Component;


// --- CONTEXTO DE LA APP ---
typedef struct GuiContext {
    Component * root;
    Component * focused;
    int focused_index;
    Component ** selection_order; // Array null terminated de punteros a los componentes seleccionables y en qué orden iterar
    int selectable_count;
} GuiContext;



// ---- Renderizado ----

// Función para calcular las dimensiones y posiciones reales de los componentes
Point calculate_position_and_size(Component* comp, Point parent_pos, int parent_width, int parent_height, int* actual_width, int* actual_height) {
    Point position;
    
    // Calcular dimensiones reales
    if (comp->parent == 0) {
        // Es el root - usar dimensiones de pantalla
        *actual_width = getScreenWidth();
        *actual_height = comp->height; // El height del root ya está en pixeles
        position.x = 0;
        position.y = 0;
    } else {
        // Calcular ancho real basado en porcentaje del padre
        *actual_width = (parent_width * comp->width) / 100;
        *actual_height = comp->height;
        
        // Calcular posición basada en alineamiento
        switch(comp->alignment) {
            case ALIGN_LEFT:
                position.x = parent_pos.x;
                break;
            case ALIGN_CENTER:
                position.x = parent_pos.x + (parent_width - *actual_width) / 2;
                break;
            case ALIGN_RIGHT:
                position.x = parent_pos.x + parent_width - *actual_width;
                break;
        }
        
        // Para la posición Y, distribuir hijos uniformemente
        if (comp->parent->children_count > 0) {
            int total_height = 0;
            for (int i = 0; i < comp->parent->children_count; i++) {
                total_height += comp->parent->children[i].height;
            }
            
            int spacing = (parent_height - total_height) / (comp->parent->children_count + 1);
            
            // Encontrar índice de este componente
            int my_index = 0;
            for (int i = 0; i < comp->parent->children_count; i++) {
                if (&comp->parent->children[i] == comp) {
                    my_index = i;
                    break;
                }
            }
            
            int y_offset = spacing;
            for (int i = 0; i < my_index; i++) {
                y_offset += comp->parent->children[i].height + spacing;
            }
            
            position.y = parent_pos.y + y_offset;
        } else {
            position.y = parent_pos.y;
        }
    }
    
    return position;
}

// Función para renderizar un componente y sus hijos
void render_component(Component* comp, GuiContext * context, Point parent_pos, int parent_width, int parent_height) {
    if (!comp) return;

    int actual_width, actual_height;
    Point position = calculate_position_and_size(comp, parent_pos, parent_width, parent_height, &actual_width, &actual_height);
    
    if(comp->needs_full_redraw || comp->parent->needs_full_redraw) {
        comp->needs_full_redraw = 1; // Marcar como que me estoy redibujando para que mis hijos también lo hagan, flag se reinicia al final

        // Dibujar fondo del componente
        drawRectangle(comp->bg_color, actual_width, actual_height, position);
        
        // Dibujar borde
        int is_focused = (comp == context->focused);
        drawRectangleBorder(is_focused ? 0xa0b0ff : 0xFFFFFF, actual_width, actual_height, is_focused ? 2 : 1, position);
        
        // Renderizar texto si existe (siempre centrado)
        if (comp->text) {
            Point text_pos = position;
            int text_width = strlen(comp->text) * getCharWidth();
            text_pos.x += (actual_width - text_width) / 2;
            text_pos.y += (actual_height - getCharHeight()) / 2;
            drawString(comp->text, comp->text_color, comp->bg_color, text_pos);
        }
    }

    // Renderizar hijos
    for (int i = 0; i < comp->children_count; i++) {
        render_component(&comp->children[i], context, position, actual_width, actual_height);
    }

    comp->needs_full_redraw = 0;
}

// Función principal para renderizar todo el árbol (renderiza el componente raíz en el tamaño de la pantalla)
void render_tree(GuiContext * context) {
    if (!context || !context->root) return;
    
    // clearScreen(0x000000);
    
    // Renderizar árbol desde la raíz
    Point root_pos = {0, 0};
    render_component(context->root, context, root_pos, getScreenWidth(), getScreenHeight());
    commitChangesToBuffer();
}

void iterate_focused(GuiContext * context, int direction) {
    if (!context || !context->selection_order) return;

    int new_index = (context->focused_index + direction + context->selectable_count) % context->selectable_count; // Itera circularmente
    if(context->focused) context->focused->needs_full_redraw = 1;
    context->focused = context->selection_order[new_index];
    context->focused->needs_full_redraw = 1;
}

void unfocus(GuiContext * context) {
    if (!context) return;

    if(context->focused) context->focused->needs_full_redraw = 1;
    context->focused = 0;
    context->focused_index = -1;
}

// Indica si el input fue capturado por el componente con foco (y se debe ignorar en la app principal)
int is_input_captured(GuiContext * context) {
    if (!context) return -1;

    return (context->focused != NULL && context->focused->on_key_press != NULL); // no hay foco o el componente con foco no maneja eventos de teclado
}

// No encuentro una forma elegante de hacer esto...
// Por ahora el programa mismo tiene que encargarse de llamar a esta función desde su handler de teclado
// Esto es porque solo el programa tiene el contexto de la app
static void call_focused_key_handler(GuiContext * context, KeyboardEvent* event) {
    if (!context || !context->focused || !context->focused->on_key_press) return;

    context->focused->on_key_press((void *)context, context->focused, event);
} 

static void call_focused_press_handler(GuiContext * context) {
    if (!context || !context->focused || !context->focused->on_press) return;

    context->focused->on_press((void *)context, context->focused);
}



// ---> PROPIO DE CADA APP <---
// ----------------------------

// Struct para acceso O(1) a los componentes por nombre
typedef struct {
    Component* root;
    Component* container1;
    Component* title;
    Component* subtitle;
    Component* footer;
} ComponentRegistry;

// Pool de componentes estáticos (cantidad de componentes es estático en runtime, no hace falta malloc ni nada dinámico)
// Los componentes se guardan de forma tal que siempre los "hermanos" (hijos del mismo padre en un mismo nivel) estén consecutivos en memoria
//   -> Esto es para que el puntero children en cada Component pueda apuntar directamente a este arreglo
static Component component_array[10]; 
static Component * component_selection_order[2]; // Array null terminated de punteros a los componentes seleccionables y en qué orden iterar
static int component_count = 0;
static ComponentRegistry components;
static GuiContext gui_context;

int paused = 0; 
int subtitle_index = 0;
char subtitle_text[40] = "Placeholder...";

// Función inicializadora que crea el árbol con "hola sr. sapo"
void initialize_component_tree() {
    // Reiniciar contador
    component_count = 0;
    
    // Siguiendo el approach de memoria contigua, el orden será:
    // [0]: title, [1]: subtitle, [2]: footer, [3]: container1, [4]: root
    // Así container1->children apunta al índice 0, y root->children apunta al índice 3
    
    // Título principal con "hola sr. sapo" - índice 0
    component_array[0] = (Component){
        .bg_color = ColorSchema->background,
        .text_color = ColorSchema->text, 
        .text = "hola sr. sapo",
        .width = 90, // 90% del contenedor
        .height = 60,
        .alignment = ALIGN_CENTER,
        .children = 0, // no tiene hijos
        .children_count = 0,
        .parent = &component_array[3], // apunta a container1
        .on_press = 0,
        .on_key_press = 0,
        .needs_full_redraw = 1
    };
    components.title = &component_array[0];
    
    // Subtítulo - índice 1
    component_array[1] = (Component){
        .bg_color = 0x88C0D0,
        .text_color = 0x6B7280,
        .text = subtitle_text,
        .width = 90, // 90% del contenedor
        .height = 40,
        .alignment = ALIGN_CENTER,
        .children = 0, // no tiene hijos
        .children_count = 0,
        .parent = &component_array[3], // apunta a container1
        .on_press = 0,
        .on_key_press = input_key_handler,
        .needs_full_redraw = 1
    };
    components.subtitle = &component_array[1];
    
    // Pie de página - índice 2
    component_array[2] = (Component){
        .bg_color = 0x4C566A,
        .text_color = 0xE5E9F0,
        .text = "PinkOS UI System",
        .width = 90, // 90% del contenedor
        .height = 30,
        .alignment = ALIGN_CENTER,
        .children = 0, // no tiene hijos
        .children_count = 0,
        .parent = &component_array[3], // apunta a container1
        .on_press = 0,
        .on_key_press = 0,
        .needs_full_redraw = 1
    };
    components.footer = &component_array[2];
    
    // Contenedor principal - índice 3
    component_array[3] = (Component){
        .bg_color = 0x2E3440,
        .text_color = 0xD8DEE9,
        .text = 0, // sin texto
        .width = 80, // 80% del padre
        .height = 400,
        .alignment = ALIGN_CENTER,
        .children = &component_array[0], // apunta al inicio de sus hijos (title, subtitle, footer)
        .children_count = 3, // title, subtitle, footer
        .parent = &component_array[4], // apunta a root
        .on_press = 0,
        .on_key_press = 0,
        .needs_full_redraw = 1
    };
    components.container1 = &component_array[3];
    
    // Componente raíz (pantalla completa) - índice 4
    component_array[4] = (Component){
        .bg_color = 0x000000,
        .text_color = 0xFFFFFF,
        .text = 0, // sin texto
        .width = 100, // 100% del padre (que es la pantalla)
        .height = getScreenHeight(),
        .alignment = ALIGN_CENTER,
        .children = &component_array[3], // apunta a container1
        .children_count = 1,
        .parent = 0, // es la raíz
        .on_press = 0,
        .on_key_press = 0,
        .needs_full_redraw = 1
    };
    components.root = &component_array[4];
    
    component_count = 5;
    component_selection_order[0] = components.subtitle;
    component_selection_order[1] = 0; // null terminated

    // Inicializar contexto de GUI
    gui_context = (GuiContext){
        .root = components.root,
        .focused = 0,
        .focused_index = -1,
        .selection_order = component_selection_order,
        .selectable_count = 1
    };
}


static void input_key_handler(void* context, struct Component* comp, KeyboardEvent* event) {
    if(!event || !comp) return;

    // Si el caracter es ascii y es imprimible, agregarlo al subtitle
    if (event->ascii >= 32 && event->ascii <= 126 && subtitle_index < sizeof(subtitle_text) - 1) {
        subtitle_text[subtitle_index++] = event->ascii;
        subtitle_text[subtitle_index] = '\0'; // Null-terminate

        components.subtitle->text_color = 0x2E3440;
        components.subtitle->needs_full_redraw = 1;
    } else if (event->scan_code == 0x0E && subtitle_index > 0) { // Backspace
        subtitle_text[--subtitle_index] = '\0';
        components.subtitle->needs_full_redraw = 1;
        if (subtitle_index == 0) {
            // Texto de placeholder si está vacío
            strcpy(subtitle_text, "Placeholder...");
        }
    }
}

static void main_key_handler(KeyboardEvent * event) {
    if (event->event_type != 1 && event->event_type != 3) return; // solo eventos de tecla presionada

    call_focused_key_handler(&gui_context, event);

    // En caso de enter ejecutar el on_press
    if(event->scan_code == 0x1C) call_focused_press_handler(&gui_context);


    if (event->ascii == ' ' && !is_input_captured(&gui_context)) {
        paused = !paused;
    }

    // Right/left arrows to change alignment of container1
    if (event->scan_code == 0x4D) { // right arrow
        components.container1->alignment = (components.container1->alignment + 1) % 3;
        components.root->needs_full_redraw = 1;
    } else if (event->scan_code == 0x4B) { // left arrow
        components.container1->alignment = (components.container1->alignment + 2) % 3; // Al sumar 2 en módulo 3 es equivalente a restar 1
        components.root->needs_full_redraw = 1;
    }

    // Up and down arrows to change conteiner1 height
    if (event->scan_code == 0x48 && components.container1->height < getScreenHeight() - 20) { // up arrow
        components.container1->height += 10;
        components.root->needs_full_redraw = 1;
    } else if (event->scan_code == 0x50 && components.container1->height > 60) { // down arrow
        components.container1->height -= 10;
        components.root->needs_full_redraw = 1;
    } 

    // tab para iterar el foco
    if (event->scan_code == 0x0F) { // tab
        iterate_focused(&gui_context, 1);
    }

    // Esc para quitar el foco
    if (event->scan_code == 0x01) { // esc
        unfocus(&gui_context);
    }
}


void graphics_demo_main(char * args){
    // Inicializar el árbol de componentes
    initialize_component_tree();
    
    // Renderizar el árbol
    render_tree(&gui_context);

	subscribeToEvent(KEY_EVENT, (void (*)(void *))main_key_handler, (void *)0);

    int i = 0;
    while(1) {
        if(!paused) {
            i++;

            components.title->bg_color = 0x000000 + (5*i % 256) * 0x00010101; // Cambia el color cada 3 frames
            components.title->text = int_to_string(i);

            components.container1->width = (2*i % 40 < 20) ? 60 + (2*i % 20) : 80 - (2*i % 20); // Cambia el ancho cada 3 frames (no cada 30!)

            // Itera la alineación cada cierto tiempo
            // if(2*i % 50 == 0)
            //     components.container1->alignment = (components.container1->alignment + 1) % 3;

            components.root->needs_full_redraw = 1;
        }

    components.root->needs_full_redraw = 1;
    render_tree(&gui_context);
    }
}

