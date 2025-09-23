#include <libs/PictureRuntimeLib.h>
#include <syscalls/syscallCodes.h>
#include <libs/events.h>
#include <colors.h>
#include <themes.h>

extern uint64_t syscall(uint64_t syscall, uint64_t arg1, uint64_t arg2, uint64_t arg3);

// Struct para acceso O(1) a los componentes por nombre
typedef struct {
    Component* root;
    Component* title;
    Component* options_container;
    Component* start_btn;
    Component* theme_toggle_btn;
    Component* timezone_input;
} ComponentRegistry;

static Component component_array[3]; 
static Component option_components[3]; // Para los botones de opciones
static Component * component_selection_order[3];
static ComponentRegistry components;
static GuiContext gui_context;


#define TIMEZONE_USER_INPUT_MAX_LEN 3
#define TIMEZONE_USER_INPUT_START 10  // "Timezone: " es 10 caracteres
int timezone_input_index = 12;
char timezone_input_text[25] = "Timezone: -3";
int timezone_error = 0;

int pinkos_started = 0;

static void enable_timezone_error_state() {
    if(timezone_error) return;
    timezone_error = 1;

    // Mostrar error en el mismo input (to-do: capaz un toast?)
    strcpy(components.timezone_input->text, "Invalid timezone");
    components.timezone_input->text_color = &ColorSchema->error; // rojo
    components.timezone_input->needs_full_redraw = 1;
}

static void disable_timezone_error_state() {
    if(!timezone_error) return;
    timezone_error = 0;

    strcpy(components.timezone_input->text, "Timezone: -3");
    timezone_input_index = 12;
    components.timezone_input->text_color = &ColorSchema->background; // esto porque probablemente tiene el foco

    components.timezone_input->needs_full_redraw = 1;
}


static void input_key_handler(void* context, struct Component* comp, KeyboardEvent* event) {
    if(!event || !comp) return;

    if(timezone_error)
        disable_timezone_error_state();

    // Si el caracter numérico o "-" y hay espacio, agregarlo al input
    if(((event->ascii >= '0' && event->ascii <= '9') || event->ascii == '-') && timezone_input_index < TIMEZONE_USER_INPUT_MAX_LEN + TIMEZONE_USER_INPUT_START) {
        timezone_input_text[timezone_input_index++] = event->ascii;
        timezone_input_text[timezone_input_index] = '\0'; // Null-terminate
        components.timezone_input->needs_full_redraw = 1;

    } else if (event->scan_code == 0x0E && timezone_input_index > TIMEZONE_USER_INPUT_START) { // Backspace
        timezone_input_text[--timezone_input_index] = '\0';
        components.timezone_input->needs_full_redraw = 1;
    }
}

static void set_timezone_press_handler(GuiContext * context, struct Component* comp) {
    if(!comp || !context) return;

    int timezone = atoi(comp->text + TIMEZONE_USER_INPUT_START);
    if(timezone < -12 || timezone > 12) {
        enable_timezone_error_state();
        sleep(1500);
        disable_timezone_error_state();
        return;
    }

    syscall(SET_TIMEZONE_SYSCALL, timezone, 0, 0);
    comp->bg_color = &ColorSchema->success;
    sleep(1000);
    comp->bg_color = (context->focused == comp) ? &ColorSchema->text : &ColorSchema->background;
}


static void pinkos_start_press_handler(void* context, struct Component* comp) {
    pinkos_started = 1;
}

// Función para actualizar recursivamente los colores de los componentes
static void update_component_colors_recursive(Component* comp) {
    if (!comp) return;
    
    // Actualizar los colores del componente actual solo si están usando ColorSchema
    // (no queremos cambiar colores personalizados como en graphics_demo)
    // Por simplicidad, asumimos que todos los componentes en home_screen usan ColorSchema
    
    comp->bg_color = (gui_context.focused == comp) ? &ColorSchema->text : &ColorSchema->background;
    comp->text_color = (gui_context.focused == comp) ? &ColorSchema->background : &ColorSchema->text;
    comp->border_color = &ColorSchema->text;
    
    // Marcar para redibujado
    comp->needs_full_redraw = 1;
    
    // Actualizar recursivamente los hijos
    for (int i = 0; i < comp->children_count; i++) {
        update_component_colors_recursive(&comp->children[i]);
    }
}


static void toggle_color_schema_handler(void* context, struct Component* comp) {
    if (ColorSchema == &PinkOSColors) {
        ColorSchema = &PinkOSMockupColors;
    } else {
        ColorSchema = &PinkOSColors;
    }
    
    // Actualizar todos los colores de los componentes para reflejar el nuevo tema
    update_component_colors_recursive(components.root);
    
    components.root->needs_full_redraw = 1;
}

static void on_focus_handler(GuiContext* context, struct Component* comp) {
    if(!comp) return;

    if(context->focused == comp){
        comp->bg_color = &ColorSchema->text;
        comp->text_color = &ColorSchema->background;
    } else {
        comp->bg_color = &ColorSchema->background;
        comp->text_color = &ColorSchema->text;
    }
    comp->needs_full_redraw = 1;
}


static void main_key_handler(KeyboardEvent * event) {
    if ((event->event_type != 1 && event->event_type != 3) || pinkos_started) return; // solo eventos de tecla presionada, y si pinkos arrancó ya está (debería poderse desregistrar un event handler capaz)

    call_focused_key_handler(&gui_context, event);

    // En caso de enter ejecutar el on_press
    if(event->scan_code == 0x1C) call_focused_press_handler(&gui_context);

    static int is_shift_pressed = 0;
	syscall(IS_KEY_PRESSED_SYSCALL, 0x2A, 0, (uint64_t)&is_shift_pressed);

    // Flechitas arriba/abajo o tab/shift+tab para cambiar foco
    if (event->scan_code == 0x48 || (event->scan_code == 0x0F && is_shift_pressed)) {
        iterate_focused(&gui_context, -1);
    } else if (event->scan_code == 0x50 || event->scan_code == 0x0F) {
        iterate_focused(&gui_context, 1);
    } 

    // Esc para quitar el foco
    if (event->scan_code == 0x01) { // esc
        unfocus(&gui_context);
    }
}

// Función inicializadora que crea el árbol con "hola sr. sapo"
static void initialize_component_tree() {
    // 1. Definir componentes individuales, posicionándolos en el array global en el orden correcto
    // -> Al definirlos se debe indicar la cantidad de hijos y el puntero al primero
    // 2. Asignar los punteros en el struct ComponentRegistry
    // 3. Definir el orden de selección en component_selection_order
    // 4. Inicializar el contexto de la app gui_context

    // Root
    component_array[0] = COMPONENT_CONSTRUCTOR(
        .bg_color = &ColorSchema->background,
        .width = 100,
        .height = getScreenHeight(),
        .children_count = 2,
        .children = &component_array[1],
    );
    components.root = &component_array[0];

    // Título (PinkOS)
    component_array[1] = COMPONENT_CONSTRUCTOR(
        .bg_color = &ColorSchema->background,
        .text_color = &ColorSchema->text, 
        .text = "PinkOS",
        .text_size = 4,
        .border_size = 0,
        .height = 80,
        .alignment = ALIGN_CENTER,
        .y_position = 200,
        .parent = components.root,
    );
    components.title = &component_array[1];

    // Contenedor de opciones
    component_array[2] = COMPONENT_CONSTRUCTOR(
        .bg_color = &ColorSchema->background,
        .border_size = 0,
        .width = 80,
        .height = 180,
        .alignment = ALIGN_CENTER,
        .y_position = 320,
        .children_count = 3,
        .children = &option_components[0],
        .parent = components.root,
    );
    components.options_container = &component_array[2];

    // Botón Start
    option_components[0] = COMPONENT_CONSTRUCTOR(
        .bg_color = &ColorSchema->background,
        .text_color = &ColorSchema->text, 
        .text = "Start",
        .border_size = 2,
        .border_color = &ColorSchema->text,
        .width = 80,
        .height = 40,
        .alignment = ALIGN_CENTER,
        .parent = components.options_container,
        .on_press = pinkos_start_press_handler,
        .on_focus_gain = on_focus_handler,
        .on_focus_lost = on_focus_handler,
    );
    components.start_btn = &option_components[0];

    // Botón Toggle Theme
    option_components[1] = COMPONENT_CONSTRUCTOR(
        .bg_color = &ColorSchema->background,
        .text_color = &ColorSchema->text, 
        .text = "Toggle Theme",
        .border_size = 2,
        .border_color = &ColorSchema->text,
        .width = 80,
        .height = 40,
        .alignment = ALIGN_CENTER,
        .parent = components.options_container,
        .on_press = toggle_color_schema_handler,
        .on_focus_gain = on_focus_handler,
        .on_focus_lost = on_focus_handler,
    );
    components.theme_toggle_btn = &option_components[1];

    // Input Timezone
    option_components[2] = COMPONENT_CONSTRUCTOR(
        .bg_color = &ColorSchema->background,
        .text_color = &ColorSchema->text, 
        .text = timezone_input_text,
        .border_size = 2,
        .border_color = &ColorSchema->text,
        .width = 80,
        .height = 40,
        .alignment = ALIGN_CENTER,
        .parent = components.options_container,
        .on_press = set_timezone_press_handler,
        .on_key_press = input_key_handler,
        .on_focus_gain = on_focus_handler,
        .on_focus_lost = on_focus_handler,
    );
    components.timezone_input = &option_components[2];
      

    // Indicar el orden de selección
    component_selection_order[0] = components.start_btn;
    component_selection_order[1] = components.theme_toggle_btn;
    component_selection_order[2] = components.timezone_input;
    int selectable_count = 3;

    // Inicializar contexto de GUI
    gui_context = initialize_gui_context(components.root, component_selection_order, selectable_count);
}


void home_screen_main() {
    enableDoubleBuffering(); // Habilitar double buffering para evitar flickering

    initialize_component_tree();
    render_tree(&gui_context);

	subscribeToEvent(KEY_EVENT, (void (*)(void *))main_key_handler, (void *)0);

    while(!pinkos_started) {
        components.root->needs_full_redraw = 1;
        render_tree(&gui_context);
    }

    disableDoubleBuffering(); // Deshabilitar double buffering al salir porque la shell no lo usa
}