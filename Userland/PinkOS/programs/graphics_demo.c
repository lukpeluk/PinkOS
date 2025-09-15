#include <programs.h>
#include <libs/stdpink.h>
#include <libs/graphicsLib.h>
#include <colors.h>


// ---> LIBRERÍA Y FUNCIONES AUXILIARES DEL RUNTIME DE PICTURE <---
// ----------------------------------------------------------------

typedef enum {
    ALIGN_LEFT,
    ALIGN_CENTER,
    ALIGN_RIGHT
} Alignment;

// --- COMPONENTE ---
typedef struct Component {
    // Estilos
    uint32_t bg_color;
    uint32_t text_color;
    char* text;

    // Layout
    int width;              // Porcentajes de su parent
    int height;
    Alignment alignment;    // Respecto a su parent y considerando las dimensiones

    // Estructura de árbol
    struct Component* children; // Apunta directamente al inicio del array de hijos contiguos
    int children_count;
    struct Component* parent;
} Component;


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
void render_component(Component* comp, Point parent_pos, int parent_width, int parent_height) {
    if (!comp) return;
    
    int actual_width, actual_height;
    Point position = calculate_position_and_size(comp, parent_pos, parent_width, parent_height, &actual_width, &actual_height);
    
    // Dibujar fondo del componente
    drawRectangle(comp->bg_color, actual_width, actual_height, position);
    
    // Dibujar borde (opcional)
    drawRectangleBorder(0xFFFFFF, actual_width, actual_height, 1, position);
    
    // Renderizar texto si existe (siempre centrado)
    if (comp->text) {
        Point text_pos = position;
        int text_width = strlen(comp->text) * getCharWidth();
        text_pos.x += (actual_width - text_width) / 2;
        text_pos.y += (actual_height - getCharHeight()) / 2;
        drawString(comp->text, comp->text_color, comp->bg_color, text_pos);
    }
    
    // Renderizar hijos
    for (int i = 0; i < comp->children_count; i++) {
        render_component(&comp->children[i], position, actual_width, actual_height);
    }
}

// Función principal para renderizar todo el árbol (renderiza el componente raíz en el tamaño de la pantalla)
void render_tree(Component * root) {
    if (!root) return;
    
    // clearScreen(0x000000);
    
    // Renderizar árbol desde la raíz
    Point root_pos = {0, 0};
    render_component(root, root_pos, getScreenWidth(), getScreenHeight());
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
static Component components[10]; 
static int component_count = 0;
static ComponentRegistry registry;



// Función inicializadora que crea el árbol con "hola sr. sapo"
void initialize_component_tree() {
    // Reiniciar contador
    component_count = 0;
    
    // Siguiendo el approach de memoria contigua, el orden será:
    // [0]: title, [1]: subtitle, [2]: footer, [3]: container1, [4]: root
    // Así container1->children apunta al índice 0, y root->children apunta al índice 3
    
    // Título principal con "hola sr. sapo" - índice 0
    components[0] = (Component){
        .bg_color = ColorSchema->background,
        .text_color = ColorSchema->text, 
        .text = "hola sr. sapo",
        .width = 90, // 90% del contenedor
        .height = 60,
        .alignment = ALIGN_CENTER,
        .children = 0, // no tiene hijos
        .children_count = 0,
        .parent = &components[3] // apunta a container1
    };
    registry.title = &components[0];
    
    // Subtítulo - índice 1
    components[1] = (Component){
        .bg_color = 0x88C0D0,
        .text_color = 0x2E3440,
        .text = "Demo de Componentes",
        .width = 90, // 90% del contenedor
        .height = 40,
        .alignment = ALIGN_CENTER,
        .children = 0, // no tiene hijos
        .children_count = 0,
        .parent = &components[3] // apunta a container1
    };
    registry.subtitle = &components[1];
    
    // Pie de página - índice 2
    components[2] = (Component){
        .bg_color = 0x4C566A,
        .text_color = 0xE5E9F0,
        .text = "PinkOS UI System",
        .width = 90, // 90% del contenedor
        .height = 30,
        .alignment = ALIGN_CENTER,
        .children = 0, // no tiene hijos
        .children_count = 0,
        .parent = &components[3] // apunta a container1
    };
    registry.footer = &components[2];
    
    // Contenedor principal - índice 3
    components[3] = (Component){
        .bg_color = 0x2E3440,
        .text_color = 0xD8DEE9,
        .text = 0, // sin texto
        .width = 80, // 80% del padre
        .height = 400,
        .alignment = ALIGN_CENTER,
        .children = &components[0], // apunta al inicio de sus hijos (title, subtitle, footer)
        .children_count = 3, // title, subtitle, footer
        .parent = &components[4] // apunta a root
    };
    registry.container1 = &components[3];
    
    // Componente raíz (pantalla completa) - índice 4
    components[4] = (Component){
        .bg_color = 0x000000,
        .text_color = 0xFFFFFF,
        .text = 0, // sin texto
        .width = 100, // 100% del padre (que es la pantalla)
        .height = getScreenHeight(),
        .alignment = ALIGN_CENTER,
        .children = &components[3], // apunta a container1
        .children_count = 1,
        .parent = 0 // es la raíz
    };
    registry.root = &components[4];
    
    component_count = 5;
}



void graphics_demo_main(char * args){
    // Inicializar el árbol de componentes
    initialize_component_tree();
    
    // Renderizar el árbol
    render_tree(registry.root);
    
    // El programa permanece activo para mostrar la interfaz
    int i = 0;
    while(1) {
        if(i++ > 1000000){
            registry.title->text = "chau sr. sapo";
            render_tree(registry.root);
        }
    }
}

