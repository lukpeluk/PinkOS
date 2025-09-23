#include <programs.h>
#include <libs/PictureRuntimeLib.h>
#include <libs/stdpink.h>
#include <libs/events.h>
#include <keyboard.h>
#include <colors.h>

// Forward declarations
struct Component;
static void input_key_handler(void* context, struct Component* comp, KeyboardEvent* event);

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
static Component component_array[25]; 
static Component * component_selection_order[10]; // Array de punteros a los componentes seleccionables y en qué orden iterar
static ComponentRegistry components;
static GuiContext gui_context;

int paused = 0; 
int subtitle_index = 0;
char subtitle_text[40] = "Placeholder...";

// Dynamic colors for animated components
static uint32_t animated_title_color = 0x000000;
static uint32_t static_color_1 = 0x88C0D0;
static uint32_t static_color_2 = 0x6B7280;
static uint32_t static_color_3 = 0x4C566A;
static uint32_t static_color_4 = 0xE5E9F0;
static uint32_t static_color_5 = 0x2E3440;
static uint32_t static_color_6 = 0xD8DEE9;
static uint32_t static_color_7 = 0x000000;
static uint32_t static_color_8 = 0xFFFFFF;

// Función inicializadora que crea el árbol con "hola sr. sapo"
void initialize_component_tree() {

    // Título principal con "hola sr. sapo" - índice 0
    component_array[0] = COMPONENT_CONSTRUCTOR(
        .bg_color = &animated_title_color,
        .text_color = &ColorSchema->text, 
        .text = "hola sr. sapo",
        .width = 90, // 90% del contenedor
        .height = 60,
        .alignment = ALIGN_CENTER,
        .children = 0, // no tiene hijos
        .children_count = 0,
        .parent = &component_array[5], // apunta a container1
        .on_press = 0,
        .on_key_press = 0,
        .needs_full_redraw = 1
    );
    components.title = &component_array[0];
    
    // Subtítulo - índice 1
    component_array[1] = COMPONENT_CONSTRUCTOR(
        .bg_color = &static_color_1,
        .text_color = &static_color_2,
        .text = subtitle_text,
        .width = 90, // 90% del contenedor
        .height = 40,
        .alignment = ALIGN_CENTER,
        .children = 0, // no tiene hijos
        .children_count = 0,
        .parent = &component_array[5], // apunta a container1
        .on_press = 0,
        .on_key_press = input_key_handler,
        .needs_full_redraw = 1
    );
    components.subtitle = &component_array[1];
    
    // Pie de página - índice 2
    component_array[2] = COMPONENT_CONSTRUCTOR(
        .bg_color = &static_color_3,
        .text_color = &static_color_4,
        .text = "PinkOS UI System",
        .width = 90, // 90% del contenedor
        .height = 30,
        .alignment = ALIGN_CENTER,
        .children = 0, // no tiene hijos
        .children_count = 0,
        .parent = &component_array[5], // apunta a container1
        .on_press = 0,
        .on_key_press = 0,
        .needs_full_redraw = 1
    );
    components.footer = &component_array[2];

    // Componente adicional - índice 3
    component_array[3] = COMPONENT_CONSTRUCTOR(
        .bg_color = &static_color_3,
        .text_color = &static_color_4,
        .text = "PinkOS UI System",
        .width = 90, // 90% del contenedor
        .height = 30,
        .alignment = ALIGN_CENTER,
        .children = 0, // no tiene hijos
        .children_count = 0,
        .parent = &component_array[5], // apunta a container1
        .on_press = 0,
        .on_key_press = 0,
        .needs_full_redraw = 1
    );

    // Componente adicional - índice 4
    component_array[4] = COMPONENT_CONSTRUCTOR(
        .bg_color = &static_color_3,
        .text_color = &static_color_4,
        .text = "PinkOS UI System",
        .width = 90, // 90% del contenedor
        .height = 30,
        .alignment = ALIGN_CENTER,
        .children = 0, // no tiene hijos
        .children_count = 0,
        .parent = &component_array[5], // apunta a container1
        .on_press = 0,
        .on_key_press = 0,
        .needs_full_redraw = 1
    );

    // Contenedor principal - índice 5
    component_array[5] = COMPONENT_CONSTRUCTOR(
        .bg_color = &static_color_5,
        .text_color = &static_color_6,
        .text = 0, // sin texto
        .width = 80, // 80% del padre
        .height = 400,
        .alignment = ALIGN_CENTER,
        .children = &component_array[0], // apunta al inicio de sus hijos (title, subtitle, footer)
        .children_count = 5, // title, subtitle, footer y componentes adicionales
        .parent = &component_array[6], // apunta a root
        .on_press = 0,
        .on_key_press = 0,
        .needs_full_redraw = 1
    );
    components.container1 = &component_array[5];
    
    // Componente raíz (pantalla completa) - índice 6
    component_array[6] = COMPONENT_CONSTRUCTOR(
        .bg_color = &static_color_7,
        .text_color = &static_color_8,
        .text = 0, // sin texto
        .width = 100, // 100% del padre (que es la pantalla)
        .height = getScreenHeight(),
        .alignment = ALIGN_CENTER,
        .children = &component_array[5], // apunta a container1
        .children_count = 1,
        .parent = 0, // es la raíz
        .on_press = 0,
        .on_key_press = 0,
        .needs_full_redraw = 1
    );
    components.root = &component_array[6];
    
    component_selection_order[0] = components.subtitle;
    component_selection_order[1] = components.footer;
    component_selection_order[2] = 0; // null terminated

    // Inicializar contexto de GUI
    gui_context = (GuiContext){
        .root = components.root,
        .focused = 0,
        .focused_index = -1,
        .selection_order = component_selection_order,
        .selectable_count = 2
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

            animated_title_color = 0x000000 + (5*i % 256) * 0x00010101; // Cambia el color cada 3 frames
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

