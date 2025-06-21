#include <memoryManager/memoryManager.h>

#ifdef USE_BUDDY
#include <drivers/serialDriver.h>
#include <drivers/pitDriver.h>
#include <stdint.h>
#include <stddef.h>

#define HEAP_BASE_ADDRESS 0x900000
#define HEAP_SIZE 0x200000000   

#define MEM_START HEAP_BASE_ADDRESS
#define MEM_SIZE HEAP_SIZE

#define FREE 1
#define OCCUPIED 0
#define MIN_LEVEL 5
#define MAX_ORDER 25
#define MIN_BLOCK_SIZE (1UL << MIN_LEVEL)

typedef struct block_t {
    uint64_t size;
    uint64_t user_size;
    int8_t is_free;
    struct block_t *next;
} block_t;

typedef struct {
    uint64_t max_order;
    block_t *free_blocks[MAX_ORDER + 1];
    uint64_t total_mem;
    uint64_t used_mem;
    uint64_t current_blocks;
    uint64_t total_allocated;
    uint64_t total_freed;
} buddy_manager;

typedef struct {
    uint64_t total;
    uint64_t free;
    uint64_t used;
} mem_info;

static buddy_manager buddy_man;

// ================== Debug Helper ====================

static void print_block(block_t *b) {
    if (!b) {
        // log_to_serial("print_block: NULL");
        return;
    }
    console_log("print_block: dir: %p", (uint64_t)b);
    console_log("print_block: size: %d", b->size);
    console_log("print_block: is_free: %d", b->is_free);
    console_log("print_block: next: %p", (uint64_t)b->next);

}

// ================== Helpers ====================

static uint64_t get_order(uint64_t size) {
    uint64_t order = 0;
    uint64_t block_size = MIN_BLOCK_SIZE;
    while (block_size < size && order < MAX_ORDER) {
        block_size <<= 1;
        order++;
    }
    return order;
}

static void remove_from_free_list(block_t *block, uint64_t order) {
    block_t **prev = &buddy_man.free_blocks[order];
    while (*prev) {
        if (*prev == block) {
            *prev = block->next;
            return;
        }
        prev = &((*prev)->next);
    }
}

// ================== API ====================

void initMemoryManager() {
    // log_to_serial("initMemoryManagerBuddy: Inicializando memory manager (Buddy System)");

    buddy_man.total_mem = MEM_SIZE;
    buddy_man.used_mem = 0;
    buddy_man.current_blocks = 0;
    buddy_man.total_allocated = 0;
    buddy_man.total_freed = 0;

    for (uint64_t i = 0; i <= MAX_ORDER; i++) {
        buddy_man.free_blocks[i] = NULL;
    }
    // log_to_serial("initMemoryManagerBuddy: Estructura de bloques libre inicializada");

    block_t* initial = (block_t*)MEM_START;
    initial->size = MEM_SIZE - sizeof(block_t);
    initial->user_size = 0;
    initial->is_free = FREE;
    initial->next = NULL;
    // log_to_serial("initMemoryManagerBuddy: Bloque inicial creado");

    uint64_t order = get_order(initial->size + sizeof(block_t));
    buddy_man.free_blocks[order] = initial;
    
    // log_decimal("initMemoryManagerBuddy: Orden inicial calculada", order);
    // log_decimal("initMemoryManagerBuddy: Tamaño inicial", initial->size);

    // log_to_serial("initMemoryManagerBuddy: Inicializacion completa");
}

void* malloc(uint64_t size) {
    // log_to_serial("mallocBuddy: Iniciando malloc");
    if (size == 0) {
        // log_to_serial("mallocBuddy: Tamaño 0");
        return NULL;
    }

    uint64_t requested = size;
    uint64_t requested_plus_header = size + sizeof(block_t);

    uint64_t order = get_order(requested_plus_header);
    
    // log_decimal("mallocBuddy: Orden requerida", order);
    // log_decimal("mallocBuddy: Tamaño con header", requested_plus_header);

    uint64_t current_order = order;
    while (current_order <= MAX_ORDER && buddy_man.free_blocks[current_order] == NULL) {
        current_order++;
    }
    if (current_order > MAX_ORDER) {
        // log_to_serial("mallocBuddy: No hay bloques disponibles");
        // log_decimal("mallocBuddy: Ultima orden buscada", current_order);
        return NULL;
    }

    block_t* block = buddy_man.free_blocks[current_order];
    buddy_man.free_blocks[current_order] = block->next;

    // log_to_serial("mallocBuddy: Bloque encontrado");
    print_block(block);

    while (current_order > order) {
        current_order--;

        uint64_t split_block_size = (1UL << current_order) * MIN_BLOCK_SIZE;
        block_t* buddy = (block_t*)((char*)block + split_block_size);

        buddy->size = split_block_size - sizeof(block_t);
        buddy->user_size = 0;
        buddy->is_free = FREE;
        buddy->next = buddy_man.free_blocks[current_order];
        buddy_man.free_blocks[current_order] = buddy;

        block->size = split_block_size - sizeof(block_t);
    }

    block->is_free = OCCUPIED;
    block->user_size = requested;
    block->next = NULL;

    buddy_man.current_blocks++;
    buddy_man.total_allocated += requested;

    // log_to_serial("mallocBuddy: Bloque asignado");
    return (void*)((char*)block + sizeof(block_t));
}

void free(void* address) {
    // log_to_serial("freeBuddy: Iniciando free");
    if (address == NULL) {
        // log_to_serial("freeBuddy: Error - direccion NULL");
        return;
    }
    
    block_t *block = (block_t *)((char*)address - sizeof(block_t));
    // log_to_serial("freeBuddy: bloque recibido:");
    print_block(block);

    if (block->is_free == FREE) {
        // log_to_serial("freeBuddy: Error - bloque ya estaba libre");
        return;
    }

    uint64_t freed_size = block->user_size;
    block->is_free = FREE;
    block->user_size = 0;
    
    if (buddy_man.total_allocated >= freed_size) {
        buddy_man.total_allocated -= freed_size;
    }
    buddy_man.total_freed += freed_size;
    buddy_man.current_blocks--;

    uint64_t order = get_order(block->size + sizeof(block_t));

    while (order < buddy_man.max_order) {
        uint64_t merge_block_bytes = (1UL << order) * MIN_BLOCK_SIZE;
        uintptr_t block_addr = (uintptr_t)block;
        uintptr_t buddy_addr = block_addr ^ merge_block_bytes;

        if (buddy_addr < (uintptr_t)MEM_START ||
            buddy_addr >= (uintptr_t)MEM_START + MEM_SIZE) {
            break;
        }

        block_t* buddy = (block_t*)buddy_addr;

        if (!(buddy->is_free == FREE) || (buddy->size != block->size)) {
            break;
        }

        block_t** prev = &buddy_man.free_blocks[order];
        block_t* found = NULL;
        while (*prev) {
            if (*prev == buddy) {
                found = buddy;
                break;
            }
            prev = &((*prev)->next);
        }
        if (!found) {
            break;
        }
        *prev = buddy->next;

        if (buddy_addr < block_addr) {
            block = buddy;
        }

        block->size = 2 * (block->size + sizeof(block_t)) - sizeof(block_t);

        order++;
    }

    block->next = buddy_man.free_blocks[order];
    buddy_man.free_blocks[order] = block;

    // log_to_serial("freeBuddy: Bloque liberado");
}

void* realloc(void* ptr, uint64_t new_size) {
    // log_to_serial("reallocBuddy: Iniciando realloc");
    
    if (ptr == NULL) {
        return malloc(new_size);
    }
    
    if (new_size == 0) {
        free(ptr);
        return NULL;
    }
    
    block_t *old_block = (block_t *)((char*)ptr - sizeof(block_t));
    uint64_t old_size = old_block->user_size;
    
    void* new_ptr = malloc(new_size);
    if (new_ptr == NULL) {
        // log_to_serial("reallocBuddy: Error - no se pudo asignar nueva memoria");
        return NULL;
    }
    
    uint64_t copy_size = (old_size < new_size) ? old_size : new_size;
    char *src = (char *)ptr;
    char *dst = (char *)new_ptr;
    for (uint64_t i = 0; i < copy_size; i++) {
        dst[i] = src[i];
    }
    
    free(ptr);

    // log_to_serial("reallocBuddy: Nueva memoria asignada y datos copiados");
    return new_ptr;
}

void memory_info(mem_info *info) {
    if (!info) return;
    info->total = buddy_man.total_mem;
    info->free = 0;
    info->used = buddy_man.total_allocated;
    
    for (uint64_t i = 0; i <= buddy_man.max_order; i++) {
        block_t* cur = buddy_man.free_blocks[i];
        while (cur) {
            info->free += cur->size;
            cur = cur->next;
        }
    }
}

#endif