#include <drivers/serialDriver.h>
#include <drivers/videoDriver.h>

#define RAW_DATA_ADDRESS 0x600000
#define MAX_RAW_DATA_ADDRESS 0xFFFFFF // para pensar

static EtherPinkResponse * clientResponse;  // puntero al struct del cliente donde se guardará la respuesta
static uint16_t response_type = 0;          // tipo de respuesta, se define en el header
static uint16_t status = NO_DATA_YET;       // estado de la respuesta, interno, depende de la situación
static uint16_t header_code = NO_DATA_YET;         // código de respuesta del header, a devolver al cliente
                                            // existe porque necesito saber lo que mandó el header, pero si la
                                            // respuesta está en curso o hubo algún problema debo ignorarlo/sobrescribirlo

static char * current = 0;           // puntero al byte actual de raw_data que se está procesando
static uint64_t total_size = 0;     // tamaño total de los datos que se esperan recibir, se define en el header


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

// Manda un mensaje al puerto serie, para debug
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


// Las cosas se reciben byte a byte, pero algunos elementos son de más de un byte...
// Así que se espera que vengan en little-endian
// Cuando se termina de recibir el header, se setea status a LOADING
void process_header(char c){
    // cuántos bytes leí (calculo la diferencia entre RAW_DATA_ADDRESS y current)
    uint64_t index = current - RAW_DATA_ADDRESS - 1;

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