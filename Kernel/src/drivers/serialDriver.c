#include <drivers/serialDriver.h>
#include <drivers/videoDriver.h>
#include <stdarg.h>

#define RAW_DATA_ADDRESS 0x600000
#define MAX_RAW_DATA_ADDRESS 0xFFFFFF // para pensar

static EtherPinkResponse * clientResponse;  // puntero al struct del cliente donde se guardará la respuesta
static uint16_t response_type = 0;          // tipo de respuesta, se define en el header
static uint16_t status = NO_DATA_YET;       // estado de la respuesta, interno, depende de la situación
static uint16_t header_code = NO_DATA_YET;         // código de respuesta del header, a devolver al cliente
                                            // existe porque necesito saber lo que mandó el header, pero si la
                                            // respuesta está en curso o hubo algún problema debo ignorarlo/sobrescribirlo

static char * current = 0;           // puntero al byte actual de raw_data que se está procesando
static uint64_t total_size = 0;      // tamaño total de los datos que se esperan recibir, se define en el header

void process_header(char c);
void finish_request(uint16_t code);

// función que se llama cuando se recibe un byte por el puerto serie
void process_serial(char c){
    // se valida que haya un cliente esperando respuesta y un byte actual donde guardar la respuesta
    if(current && clientResponse){
        // ya el caracter se agrega al buffer de datos
        *current = c;
        current++;

        // ahora, hay dos opciones: o se están recibiendo datos o se está recibiendo el header
        // depende de si status es GETTING_HEADERS o no

        if(status == GETTING_HEADERS){
            process_header(c);
            return;
        }

        // el tamaño de la respuesta solo incrementa si no era del header lo que llegó
        clientResponse->size++;

        // acá ya se cargó lo que llegó, lo único que faltaría es ver si se llegó al final de la respuesta
        // cuando se finaliza recién ahí se setea el code del header, antes va a ser o NO_DATA_YET o LOADING
        if( response_type == FIXED_SIZE && clientResponse->size >= total_size){
            // si se llegó al final de la respuesta, entonces se finaliza la request
            finish_request(header_code);
            return;
        }

        if( response_type == ASCII_STREAM && c == 0 ){
            // si es un stream de texto y llegó el byte 0, entonces se terminó la respuesta
            finish_request(header_code);
            return;
        }
    }
}

// antes de llamar a esta función, asegurarse de setear el código de respuesta final en code
void finish_request(uint16_t code){
    current = 0;
    // el resto de headers ya se guardaron, el code es un caso particular porque se puede sobrescribir o cambiar
    clientResponse->code = code;
    clientResponse = 0;
}

void make_ethereal_request(char * request, EtherPinkResponse * response){
    if(current || clientResponse){
        // si ya hay una request en curso, no se puede hacer otra
        // log_to_serial("ERROR: Ya hay una request en curso");
        response->code = SERIAL_OCCUPIED;
        response->size = 0;
        return;
    }

    // El cliente no debería usar todavía los datos porque code es NO_DATA_YET
    clientResponse = response;
    clientResponse->code = NO_DATA_YET;
    clientResponse->size = 0;
    clientResponse->raw_data = (char *)RAW_DATA_ADDRESS + HEADER_SIZE; // los datos empiezan después del header

    status = GETTING_HEADERS; // estado interno, esperando headers

    // mando por el serial el request
    current = (char *) RAW_DATA_ADDRESS;
    while(*request){
        write_serial(*request);
        request++;
    } 
}

// Manda un mensaje al puerto serie, para debug (con prefijo "LOG: ")
void log_to_serial(char * message){
    if (!message) return;

    char *log = "LOG: ";
    while(*log){
        write_serial(*log);
        log++;
    }
    while(*message){
        write_serial(*message);
        message++;
    }
    write_serial('\n'); 
}


// Manda un mensaje al puerto serie, para debug
void send_to_serial(char * message){
    if (!message) return;

    while(*message){
        write_serial(*message);
        message++;
    }
    write_serial('\n'); 
}



// Las cosas se reciben byte a byte, pero algunos elementos son de más de un byte...
// Así que se espera que vengan en little-endian
// Cuando se termina de recibir el header, se setea status a LOADING
void process_header(char c){
    // cuántos bytes leí (calculo la diferencia entre RAW_DATA_ADDRESS y current)
    uint64_t index = (uint64_t)(current - RAW_DATA_ADDRESS - 1);

    // 2 bytes iniciales son el header_code
    // los siguientes 2 bytes van en clientResponse->content_type
    // los siguientes 2 bytes en response_type y también se copian en clientResponse->response_type
    // los últimos 8 bytes son el total_size
    // Cuando se termina de cargar el total_size, se terminaron de cargar los headers, por ende debe cambiarse status a LOADING (mismo con clientResponse->code)
    if(index == 0){
        total_size = 0; 
        header_code = (uint16_t)c;
    } else if(index == 1){
        header_code |= ((uint16_t)c << 8);
    } else if(index == 2){
        clientResponse->content_type = (uint16_t)c;
    } else if(index == 3){
        clientResponse->content_type |= ((uint16_t)c << 8);
    } else if(index == 4){
        response_type = (uint16_t)c;
        clientResponse->response_type = (uint16_t)c;
    } else if(index == 5){
        response_type |= ((uint16_t)c << 8);
        clientResponse->response_type |= ((uint16_t)c << 8);
    } else if(index >= 6 && index <= 13){
        uint8_t byte_offset = index - 6;
        total_size |= ((uint64_t)c << (byte_offset * 8));
        if(index == 13){
            status = LOADING;
            clientResponse->code = LOADING;
        }
    }
}


// Funciones auxiliares para logging
void log_hex(char* prefix, uint64_t value) {
    char buffer[100]; // Buffer temporal para construir el mensaje completo
    char hexStr[20];
    char *p = buffer;
    
    // Copiar el prefijo
    while (*prefix) {
        *p++ = *prefix++;
    }
    
    // Agregar "0x"
    *p++ = '0';
    *p++ = 'x';
    
    // Convertir a hexadecimal
    itoa(value, hexStr, 16);
    char *hex_p = hexStr;
    while (*hex_p) {
        *p++ = *hex_p++;
    }
    
    *p = '\0'; // Terminar el string
    
    // Enviar todo en una sola llamada
    log_to_serial(buffer);
}

void log_decimal(char* prefix, uint64_t value) {
    char buffer[100]; // Buffer temporal para construir el mensaje completo
    char decStr[25];  // Suficiente para un uint64_t
    char *p = buffer;
    
    // Copiar el prefijo
    while (*prefix) {
        *p++ = *prefix++;
    }
    
    // Convertir a decimal
    itoa(value, decStr, 10);
    char *dec_p = decStr;
    while (*dec_p) {
        *p++ = *dec_p++;
    }
    
    *p = '\0'; // Terminar el string
    
    // Enviar todo en una sola llamada
    log_to_serial(buffer);
}

void log_string(char* prefix, char* str) {
    char buffer[100]; // Buffer temporal para construir el mensaje completo
    char *p = buffer;
    
    // Copiar el prefijo
    while (*prefix) {
        *p++ = *prefix++;
    }
    
    // Agregar la cadena
    while (*str) {
        *p++ = *str++;
    }
    
    *p = '\0'; // Terminar el string
    
    // Enviar todo en una sola llamada
    log_to_serial(buffer);
}

// Memory debugging functions
void mem_register_sector(uint64_t start_addr, uint64_t end_addr, char* tag) {
    char buffer[200]; // Buffer para construir el comando completo
    char start_hex[20], end_hex[20];
    char *p = buffer;
    
    // Construir el comando "MEMREG <start_hex> <end_hex> <tag>"
    char *cmd = "MEMREG 0x";
    while (*cmd) {
        *p++ = *cmd++;
    }
    
    // Convertir dirección de inicio a hex
    itoa(start_addr, start_hex, 16);
    char *start_p = start_hex;
    while (*start_p) {
        *p++ = *start_p++;
    }
    
    // Agregar espacio y 0x
    *p++ = ' ';
    *p++ = '0';
    *p++ = 'x';
    
    // Convertir dirección de fin a hex
    itoa(end_addr, end_hex, 16);
    char *end_p = end_hex;
    while (*end_p) {
        *p++ = *end_p++;
    }
    
    // Agregar espacio y tag
    *p++ = ' ';
    while (*tag) {
        *p++ = *tag++;
    }
    
    *p = '\0'; // Terminar el string
    
    // Enviar el comando
    send_to_serial(buffer); 
}

void mem_log_address(uint64_t address, char* name) {
    char buffer[200]; // Buffer para construir el comando completo
    char addr_hex[20];
    char *p = buffer;
    
    // Construir el comando "MEMLOG <address_hex> <name>"
    char *cmd = "MEMLOG 0x";
    while (*cmd) {
        *p++ = *cmd++;
    }
    
    // Convertir dirección a hex
    itoa(address, addr_hex, 16);
    char *addr_p = addr_hex;
    while (*addr_p) {
        *p++ = *addr_p++;
    }
    
    // Agregar espacio y nombre
    *p++ = ' ';
    while (*name) {
        *p++ = *name++;
    }
    
    *p = '\0'; // Terminar el string
    
    // Enviar el comando
    send_to_serial(buffer);
}

void mem_free_sector(uint64_t start_addr) {
    char buffer[100]; // Buffer para construir el comando completo
    char start_hex[20];
    char *p = buffer;
    
    // Construir el comando "MEMFREE <start_hex>"
    char *cmd = "MEMFREE 0x";
    while (*cmd) {
        *p++ = *cmd++;
    }
    
    // Convertir dirección de inicio a hex
    itoa(start_addr, start_hex, 16);
    char *start_p = start_hex;
    while (*start_p) {
        *p++ = *start_p++;
    }
    
    *p = '\0'; // Terminar el string
    
    // Enviar el comando
    send_to_serial(buffer); // No necesitamos respuesta para este comando
}

void mem_list_sectors() {
    // Enviar comando para listar todos los sectores registrados
    send_to_serial("MEMLIST");
}

// Enhanced logging functions

// Función auxiliar que envía mensaje sin newline al final
void send_to_serial_no_newline(char* message) {
    if (!message) return;

    while(*message) {
        write_serial(*message);
        message++;
    }
}

// Función auxiliar para convertir número a string con base específica
static void num_to_string(uint64_t num, char* buffer, int base, int uppercase, int width, char pad_char) {
    char digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";
    char upper_digits[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    char* digit_set = uppercase ? upper_digits : digits;
    
    char temp[32];
    int i = 0;
    
    if (num == 0) {
        temp[i++] = '0';
    } else {
        while (num > 0) {
            temp[i++] = digit_set[num % base];
            num /= base;
        }
    }
    
    // Aplicar padding si es necesario
    int len = i;
    int padding = width - len;
    if (padding > 0) {
        for (int j = 0; j < padding; j++) {
            buffer[j] = pad_char;
        }
        // Copiar dígitos en orden inverso después del padding
        for (int j = 0; j < len; j++) {
            buffer[padding + j] = temp[len - 1 - j];
        }
        buffer[padding + len] = '\0';
    } else {
        // Copiar dígitos en orden inverso sin padding
        for (int j = 0; j < len; j++) {
            buffer[j] = temp[len - 1 - j];
        }
        buffer[len] = '\0';
    }
}

// Función auxiliar para convertir números con signo
static void signed_num_to_string(int64_t num, char* buffer, int base, int width, char pad_char) {
    if (num < 0) {
        buffer[0] = '-';
        num_to_string(-num, buffer + 1, base, 0, width > 1 ? width - 1 : 0, pad_char);
    } else {
        num_to_string(num, buffer, base, 0, width, pad_char);
    }
}

/** console_log: 
 * Advanced logging function with printf-like formatting capabilities
 * Supports: %d (decimal), %x (hex), %X (uppercase hex), %s (string), %c (char), %p (pointer)
 * Also supports width specifiers like %10d, %08x, etc.
 */
void console_log(char* format, ...) {
    if (!format) return;
    
    va_list args;
    va_start(args, format);
    
    char buffer[1024*2]; // Buffer para construir el mensaje completo
    char* buf_ptr = buffer;
    char* fmt_ptr = format;
    
    while (*fmt_ptr && (buf_ptr - buffer) < sizeof(buffer) - 1) {
        if (*fmt_ptr == '%') {
            fmt_ptr++; // Saltar el %
            
            // Parsear width specifier y padding
            int width = 0;
            char pad_char = ' ';
            
            // Detectar padding con ceros
            if (*fmt_ptr == '0') {
                pad_char = '0';
                fmt_ptr++;
            }
            
            // Parsear width
            while (*fmt_ptr >= '0' && *fmt_ptr <= '9') {
                width = width * 10 + (*fmt_ptr - '0');
                fmt_ptr++;
            }
            
            // Procesar el especificador de formato
            switch (*fmt_ptr) {
                case 'd': {
                    int num = va_arg(args, int);
                    char temp_buf[32];
                    signed_num_to_string(num, temp_buf, 10, width, pad_char);
                    
                    char* temp_ptr = temp_buf;
                    while (*temp_ptr && (buf_ptr - buffer) < sizeof(buffer) - 1) {
                        *buf_ptr++ = *temp_ptr++;
                    }
                    break;
                }
                case 'x': {
                    unsigned int num = va_arg(args, unsigned int);
                    char temp_buf[32];
                    num_to_string(num, temp_buf, 16, 0, width, pad_char);
                    
                    char* temp_ptr = temp_buf;
                    while (*temp_ptr && (buf_ptr - buffer) < sizeof(buffer) - 1) {
                        *buf_ptr++ = *temp_ptr++;
                    }
                    break;
                }
                case 'X': {
                    unsigned int num = va_arg(args, unsigned int);
                    char temp_buf[32];
                    num_to_string(num, temp_buf, 16, 1, width, pad_char);
                    
                    char* temp_ptr = temp_buf;
                    while (*temp_ptr && (buf_ptr - buffer) < sizeof(buffer) - 1) {
                        *buf_ptr++ = *temp_ptr++;
                    }
                    break;
                }
                case 'p': {
                    void* ptr = va_arg(args, void*);
                    char temp_buf[32];
                    
                    // Agregar "0x" prefix para punteros
                    *buf_ptr++ = '0';
                    *buf_ptr++ = 'x';
                    
                    num_to_string((uint64_t)ptr, temp_buf, 16, 0, 0, '0');
                    
                    char* temp_ptr = temp_buf;
                    while (*temp_ptr && (buf_ptr - buffer) < sizeof(buffer) - 1) {
                        *buf_ptr++ = *temp_ptr++;
                    }
                    break;
                }
                case 's': {
                    char* str = va_arg(args, char*);
                    if (!str) str = "(null)";
                    
                    int str_len = 0;
                    char* count_ptr = str;
                    while (*count_ptr++) str_len++; // Calcular longitud
                    
                    // Aplicar padding si es necesario
                    int padding = width - str_len;
                    if (padding > 0) {
                        for (int i = 0; i < padding && (buf_ptr - buffer) < sizeof(buffer) - 1; i++) {
                            *buf_ptr++ = ' ';
                        }
                    }
                    
                    while (*str && (buf_ptr - buffer) < sizeof(buffer) - 1) {
                        *buf_ptr++ = *str++;
                    }
                    break;
                }
                case 'c': {
                    char c = (char)va_arg(args, int);
                    *buf_ptr++ = c;
                    break;
                }
                case '%': {
                    *buf_ptr++ = '%';
                    break;
                }
                default: {
                    // Formato desconocido, copiar literalmente
                    *buf_ptr++ = '%';
                    *buf_ptr++ = *fmt_ptr;
                    break;
                }
            }
        } else {
            *buf_ptr++ = *fmt_ptr;
        }
        fmt_ptr++;
    }
    
    *buf_ptr = '\0'; // Terminar el string
    
    va_end(args);
    
    // Enviar el mensaje completo usando log_to_serial (que añade LOG: y \n)
    log_to_serial(buffer);
}

