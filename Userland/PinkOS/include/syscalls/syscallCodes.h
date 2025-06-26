#ifndef SYSCALL_CODES_H
#define SYSCALL_CODES_H

//! #define SET_HANDLER_SYSCALL 0
//! #define USER_ENVIRONMENT_API_SYSCALL 3
//! #define SET_SYSTEM_STACK_BASE_SYSCALL 4

// 0 - 999 reserved for SYSTEM syscalls
// ========================================
// procesos (general, scheduling, semaforos), eventos, files, ventanas, manejo de memoria, programas, monitor de actividad

// 0 - 99 reserved for PROCESS syscalls
#define RUN_PROGRAM_SYSCALL 0 //! breaking
#define NEW_THREAD_SYSCALL 1
#define QUIT_SYSCALL 2
#define KILL_PROCESS_SYSCALL 3
#define CHANGE_PRIORITY_SYSCALL 4 // Cambia la prioridad de un proceso
#define DETACH_PROCESS_SYSCALL 5 // Desvincula un proceso de su shell (lo vuelve nohup)

#define YIELD_SYSCALL 10 // Renunciar a la CPU, para que el scheduler pueda elegir otro proceso
#define SET_WAITING_SYSCALL 11 // Pone al proceso en estado de espera, para que no consuma CPU
#define WAKE_PROCESS_SYSCALL 12 // Despierta un proceso que estaba en espera
#define AGARRINI_LA_PALINI_SYSCALL 12 // Dale a laburar (alias de WAKE_PROCESS_SYSCALL)

#define GET_PID_SYSCALL 20           // tu propio pid
#define GET_PROCESS_INFO_SYSCALL 21  // a partir de un pid, struct del proceso
#define GET_PROCESS_LIST_SYSCALL 22  // para ps
#define GET_GROUP_MAIN_PID_SYSCALL 23 // Te da el PID del proceso main del grupo al que perteneces

#define SEM_INIT_SYSCALL 30 // Inicializa un semáforo con el valor especificado, devuelve el id del semáforo creado
#define SEM_DESTROY_SYSCALL 31 // Destruye el semáforo con el id especificado, no se puede destruir si hay procesos esperándolo
#define SEM_WAIT_SYSCALL 32 // Deja al proceso en espera hasta que el semáforo tenga un valor positivo
#define SEM_POST_SYSCALL 33 // Desencola un proceso que estaba esperando este semáforo, si no hay nadie esperando, incrementa el valor del semáforo

#define READ_STDIN 40
#define WRITE_STDOUT 41
#define WRITE_STDERR 42

// 100 - 199 reserved for EVENT syscalls
#define SUSCRIBE_TO_EVENT_SYSCALL 100 // Registra un evento para que el proceso pueda recibir notificaciones
#define UNSUBSCRIBE_TO_EVENT_SYSCALL 101 // Desregistra un evento para que el proceso no reciba más notificaciones
#define WAIT_EVENT_SYSCALL 105

// 200 - 299 reserved for FILE SYSTEM syscalls
#define MK_FILE_SYSCALL 200 
#define RM_FILE_SYSCALL 201

#define OPEN_FILE_SYSCALL 205
#define CLOSE_FILE_SYSCALL 206
#define CLOSE_FIFO_FOR_WRITING_SYSCALL 207 // Cierra el archivo para escritura, pero lo deja abierto para lectura

#define READ_RAW_FILE_SYSCALL 210 
#define WRITE_RAW_FILE_SYSCALL 211 
#define READ_FIFO_FILE_SYSCALL 212
#define WRITE_FIFO_FILE_SYSCALL 213 

#define GET_FILE_SYSCALL 220
#define GET_FILE_LIST_SYSCALL 221

#define SET_FILE_PERMISSIONS_SYSCALL 230
#define GET_FILE_PERMISSIONS_SYSCALL 231
#define VALIDATE_FILE_ACCESS_PERMISSIONS_SYSCALL 232 // valida si el proceso actual puede hacer x acción sobre un archivo

// 300 - 399 reserved for MEMORY MANAGER syscalls
#define ALLOCATE_MEMORY_SYSCALL 300 
#define REALLOCATE_MEMORY_SYSCALL 301
#define FREE_MEMORY_SYSCALL 302 
// TODO: Ver temas de system monitor
#define GET_HEAP_SIZE_SYSCALL 302 
#define GET_HEAP_BASE_SYSCALL 303 
#define GET_AVAILABLE_MEMORY_SYSCALL 304 

// 400 - 499 reserved for WINDOW MANAGER syscalls
#define SWITCH_WINDOW_SYSCALL 400 // Cambia la ventana activa, para que los eventos de teclado y mouse se envíen a esa ventana
#define GET_WINDOW_LIST_SYSCALL 401 // Devuelve una lista de las ventanas abiertas
#define GET_FOCUSED_WINDOW_SYSCALL 402 // Devuelve el PID de la ventana activa
#define IS_FOCUSED_WINDOW_SYSCALL 403 // Devuelve 1 si el PID especificado es la ventana activa, 0 si no lo es

// 500 - 599 reserved for PROGRAM syscalls
#define GET_PROGRAM_SYSCALL 500 // Devuelve información del programa del comando que se pida
#define GET_PROGRAM_LIST_SYSCALL 501 // Devuelve una lista de los programas disponibles
#define SEARCH_PROGRAM_SYSCALL 502 // Busca si un string es el comienzo de un (unico) comando, devuelve el PID del programa si lo encuentra, 0 si no lo encuentra
#define INSTALL_PROGRAM_SYSCALL 503 // Instala un programa en el sistema, devuelve 0 si se instaló correctamente, -1 si hubo error
#define UNINSTALL_PROGRAM_SYSCALL 504 // Desinstala un programa del sistema, devuelve 0 si se desinstaló correctamente, -1 si hubo error

// 600 - 699 reserved for SYSTEM MONITOR syscalls


// 1000 - 1999 reserved for DRIVER syscalls
// ========================================

// 1000 - 1099 reserved for VIDEO DRIVER syscalls
#define DRAW_PIXEL_SYSCALL 1000
#define DRAW_RECTANGLE_SYSCALL 1001
#define DRAW_RECTANGLE_BORDER_SYSCALL 1002

#define DRAW_CHAR_SYSCALL 1010
#define DRAW_STRING_SYSCALL 1011
#define DRAW_CHAR_AT_SYSCALL 1012
#define DRAW_STRING_AT_SYSCALL 1013
#define DRAW_NUMBER_SYSCALL 1014
#define DRAW_NUMBER_AT_SYSCALL 1015
#define DRAW_HEX_SYSCALL 1016
#define DRAW_HEX_AT_SYSCALL 1017

#define DRAW_BITMAP_SYSCALL 1020

#define IS_CURSOR_IN_BOUNDARIES_SYSCALL 1030
#define SET_CURSOR_COL_SYSCALL 1031
#define SET_CURSOR_LINE_SYSCALL 1032
#define GET_CURSOR_COL_SYSCALL 1033
#define GET_CURSOR_LINE_SYSCALL 1034

#define GET_SCREEN_WIDTH_SYSCALL 1035
#define GET_SCREEN_HEIGHT_SYSCALL 1036
#define GET_CHAR_WIDTH_SYSCALL 1037
#define GET_CHAR_HEIGHT_SYSCALL 1038

#define CLEAR_SCREEN_SYSCALL 1040
#define DISABLE_REDRAW_SYSCALL 1041
#define ENABLE_REDRAW_SYSCALL 1042
#define SET_FONT_SIZE_SYSCALL 1050
#define INC_FONT_SIZE_SYSCALL 1051
#define DEC_FONT_SIZE_SYSCALL 1052
#define SET_FONT_SYSCALL 1053

// 1100 - 1199 reserved for RTC DRIVER syscalls
#define GET_RTC_TIME_SYSCALL 1100
#define SET_TIMEZONE_SYSCALL 1110

// 1200 - 1299 reserved for PIT DRIVER syscalls
#define SLEEP_SYSCALL 1200
#define GET_MILLIS_ELAPSED_SYSCALL 1205

// 1300 - 1399 reserved for keyboard DRIVER syscalls
#define GET_KEY_EVENT_SYSCALL 1300
#define CLEAR_KEYBOARD_BUFFER_SYSCALL 1310
#define IS_KEY_PRESSED_SYSCALL 1320

// 1400 - 1499 reserved for audio DRIVER syscalls
#define PLAY_AUDIO_SYSCALL 1400
#define STOP_AUDIO_SYSCALL 1401
#define PAUSE_AUDIO_SYSCALL 1410
#define RESUME_AUDIO_SYSCALL 1411

#define GET_AUDIO_STATE_SYSCALL 1420
#define LOAD_AUDIO_STATE_SYSCALL 1421

#define IS_AUDIO_PLAYING_SYSCALL 1430

// 1500 - 1599 reserved for SERIAL DRIVER syscalls
#define MAKE_ETHEREAL_REQUEST_SYSCALL 1500
#define LOG_TO_SERIAL_SYSCALL 1501
#define LOG_DECIMAL_TO_SERIAL_SYSCALL 1502
#define LOG_HEX_TO_SERIAL_SYSCALL 1503

// 1600 - 1699 reserved for MOUSE DRIVER syscalls
// (Nothing yet, but believe me, there will be)

#endif