
#include <libs/PictureRuntimeLib.h>

// Default color values for component constructor
uint32_t default_bg_color = 0xFFFFFF;
uint32_t default_text_color = 0x000000;
uint32_t default_border_color = 0xFFFFFF;

// ---- Renderizado ----

// Función para calcular las dimensiones y posiciones reales de los componentes
static Point calculate_position_and_size(Component* comp, Point parent_pos, int parent_width, int parent_height, int* actual_width, int* actual_height) {
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
        
        // Para la posición Y, usar y_position si está definido (no es -1), sino distribuir uniformemente
        if (comp->y_position != -1) {
            // Usar posición Y específica
            position.y = parent_pos.y + comp->y_position;
        } else if (comp->parent->children_count > 0) {
            // Distribuir hijos uniformemente
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
static void render_component(Component* comp, GuiContext * context, Point parent_pos, int parent_width, int parent_height) {
    if (!comp) return;

    int actual_width, actual_height;
    Point position = calculate_position_and_size(comp, parent_pos, parent_width, parent_height, &actual_width, &actual_height);
    
    if(comp->needs_full_redraw || comp->parent->needs_full_redraw) {
        comp->needs_full_redraw = 1; // Marcar como que me estoy redibujando para que mis hijos también lo hagan, flag se reinicia al final

        // Dibujar fondo del componente
        drawRectangle(*(comp->bg_color), actual_width, actual_height, position);
        
        // Dibujar borde
        int is_focused = (comp == context->focused);
        drawRectangleBorder(is_focused ? 0xa0b0ff : *(comp->border_color), actual_width, actual_height, is_focused ? 2 : comp->border_size, position);
        
        // Renderizar texto si existe (siempre centrado)
        if (comp->text) {
            Point text_pos = position;

            // Incrementa o decremento del tamaño de fuente según text_size
            // TODO: resolverlo mejor que acá si achicás demasiado rompés las cosas
            int scale = comp->text_size;
            int scale_decrease = 0;
            if(comp->text_size < 0) {
                scale = -comp->text_size;
                scale_decrease = 1;
            }

            for(int i = 0; i < scale; i++) {
                if(scale_decrease) decFontSize();
                else incFontSize();
            }

            int text_width = strlen(comp->text) * getCharWidth();
            text_pos.x += (actual_width - text_width) / 2;
            text_pos.y += (actual_height - getCharHeight()) / 2;
            drawString(comp->text, *(comp->text_color), *(comp->bg_color), text_pos);

            // Restaurar tamaño de fuente
            for(int i = 0; i < scale; i++) {
                if(scale_decrease) incFontSize();
                else decFontSize();
            }
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


// --- Helpers ---
GuiContext initialize_gui_context(Component* root, Component** selection_order, int selectable_count) {
    GuiContext context = {
        .root = root,
        .focused = 0,
        .focused_index = -1,
        .selection_order = selection_order,
        .selectable_count = selectable_count
    };
    return context;
}


// ---- Manejo de foco y eventos ----

void iterate_focused(GuiContext * context, int direction) {
    if (!context || !context->selection_order) return;

    // Maneja el caso donde no hay nada en foco y queres empezar a iterar para atrás
    if(context->focused_index == -1 && direction == -1) {
        context->focused_index = context->selectable_count; // Así al restar 1 queda en el último
    }

    Component* prev_focused = context->focused;
    int new_index = (context->focused_index + direction + context->selectable_count) % context->selectable_count; // Itera circularmente

    if(prev_focused){
        prev_focused->needs_full_redraw = 1;
    }

    context->focused = context->selection_order[new_index];
    context->focused->needs_full_redraw = 1;
    context->focused_index = new_index;

    if(context->focused->on_focus_gain) context->focused->on_focus_gain(context, context->focused);
    if(prev_focused && prev_focused->on_focus_lost) prev_focused->on_focus_lost(context, prev_focused);
}

void unfocus(GuiContext * context) {
    if (!context) return;

    if(context->focused) {
        if(context->focused->on_focus_lost) context->focused->on_focus_lost(context, context->focused);
        context->focused->needs_full_redraw = 1;
    }
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
void call_focused_key_handler(GuiContext * context, KeyboardEvent* event) {
    if (!context || !context->focused || !context->focused->on_key_press) return;

    context->focused->on_key_press((void *)context, context->focused, event);
} 

void call_focused_press_handler(GuiContext * context) {
    if (!context || !context->focused || !context->focused->on_press) return;

    context->focused->on_press((void *)context, context->focused);
}