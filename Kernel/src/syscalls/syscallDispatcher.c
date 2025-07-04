#include <stdint.h>
#include <eventHandling/eventHandlers.h>
#include <processManager/processState.h>
#include <permissions.h>
#include <syscalls/syscallCodes.h>
#include <drivers/pitDriver.h>
#include <drivers/videoDriver.h>
#include <drivers/keyboardDriver.h>
#include <drivers/rtcDriver.h>
#include <drivers/audioDriver.h>
#include <drivers/serialDriver.h>
#include <processManager/scheduler.h>
#include <windowManager/windowManager.h>
#include <eventManager/eventManager.h>
#include <programManager/programManager.h>
#include <memoryManager/memoryManager.h>
#include <lib.h>
#include <types.h>

#define VALIDATE_PERMISSIONS(permission) if (!validatePermissions(permission)) return 0;
// valida si el proceso actual o tiene permisos globales de manejo de permisos, o quiere acceder a un proceso descendiente del main de su grupo
#define VALIDATE_PROCESS_PERMISSIONS(pid_to_access) if(!validatePermissions(MANAGE_PROCESSES_PERMISSION) && !isDescendantOf((Pid)pid_to_access, getProcessGroupMain(getCurrentProcessPID()))) return 0;
// valida si el proceso actual o tiene premisos globales de manejo de archivos, o tiene permisos para el archivo y acción especificados
#define VALIDATE_FILE_PERMISSIONS(fileId, action) if (!validatePermissions(MANAGE_FILES_PERMISSION) && !validateFileAccessPermissions(fileId, getCurrentProcessPID(), action)) return 0;
// #define VALIDATE_FILE_PERMISSIONS(fileId, action) {};

uint64_t systemSyscallDispatcher(uint64_t syscall, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5);
uint64_t videoDriverSyscallDispatcher(uint64_t syscall, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5);
uint64_t keyboardDriverSyscallDispatcher(uint64_t syscall, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5);
uint64_t audioDriverSyscallDispatcher(uint64_t syscall, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5);
uint64_t rtcDriverSyscallDispatcher(uint64_t syscall, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5);
uint64_t pitDriverSyscallDispatcher(uint64_t syscall, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5);
uint64_t serialDriverSyscallDispatcher(uint64_t syscall, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5);

uint64_t syscallDispatcher(uint64_t syscall, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5) {
    if(syscall < 1000)
        return systemSyscallDispatcher(syscall, arg1, arg2, arg3, arg4, arg5);
    else if(syscall < 1100)
        return videoDriverSyscallDispatcher(syscall, arg1, arg2, arg3, arg4, arg5);
    else if(syscall < 1200)
        return rtcDriverSyscallDispatcher(syscall, arg1, arg2, arg3, arg4, arg5);
    else if(syscall < 1300)
        return pitDriverSyscallDispatcher(syscall, arg1, arg2, arg3, arg4, arg5);
    else if(syscall < 1400)
        return keyboardDriverSyscallDispatcher(syscall, arg1, arg2, arg3, arg4, arg5);
    else if(syscall < 1500)
        return audioDriverSyscallDispatcher(syscall, arg1, arg2, arg3, arg4, arg5);
    else if(syscall < 1600)
        return serialDriverSyscallDispatcher(syscall, arg1, arg2, arg3, arg4, arg5);
    else
        return 0;
}


// --- SYSTEM SYSCALLS ---
uint64_t systemSyscallDispatcher(uint64_t syscall, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5) {
    switch (syscall) {

        // ----- PROCESS MANAGEMENT -----
        case RUN_PROGRAM_SYSCALL: {
            // * Orden Syscall: Program, char * args, priority, I/O struct (stdin, stdout, stderr), uint64_t (bool) nohup -> acá te devolvemos el PID del proceso creado
            // * retorna el pid del proceso creado, 0 si error

                IO_Files io_files = {0};
                if ((void *)arg4 != NULL) {
                    io_files = *(IO_Files *)arg4; // Si se pasa un struct de I/O, lo usamos
                    // log_to_serial("runProgram: Asignando I/O al nuevo proceso");
                    // log_decimal("runProgram: stdin file id: ", io_files.stdin);
                    // log_decimal("runProgram: stdout file id: ", io_files.stdout);
                }
                    
                Program * program = getProgramByCommand((char *)arg1);
                if(program == NULL) {
                    log_to_serial("E: Program not found");
                    return 0; // Error: program not found
                }

                char * args = strdup((char *)arg2); //TODO: allocar a nombre del proceso

                Pid new_pid = newProcessWithIO(*program, args, arg3, *(uint64_t *)arg5 ? 1 : getCurrentProcessPID(), 
                                io_files.stdin, io_files.stdout, io_files.stderr);

                // log_decimal("I: runProgram: New process created with PID: ", new_pid);
                if (new_pid == 0) {
                    log_to_serial("E: syscallDispatcher: PROBLEMAS PROBLEMAS, ERROR CREANDO PROCESO");
                }

                *(uint64_t*)arg5 = new_pid;
                return new_pid;
            }
            break;
        case NEW_THREAD_SYSCALL: {
            // * Orden Syscall: ProgramEntry entrypoint, char * args, priority
            // * retorna el pid del thread creado, 0 si error

                char * args = strdup((char *)arg2); //TODO: allocar a nombre del proceso
                return newThread((ProgramEntry)arg1, args, arg3, getCurrentProcessPID());
            } 
            break;
        case QUIT_SYSCALL:
            // * sin args, no debería volverse de esta syscall
            // console_log("Quit syscall called, terminating current process with PID %d", getCurrentProcessPID());
            // Terminates the current process (no recibe nada)
            terminateProcess(getCurrentProcessPID());
            break;
        case KILL_PROCESS_SYSCALL:
            // * Orden Syscall: pid
            // * retorna 0 en caso de error
            
            // Solo alguien con permiso global de manejo de procesos puede matar un proceso arbitrario
            // Si no, solo podés matarte o a los descendientes del grupo
            VALIDATE_PROCESS_PERMISSIONS(arg1);
            return terminateProcess((Pid)arg1) == 0;
        case CHANGE_PRIORITY_SYSCALL:
            // * Orden syscall: pid del proceso, nueva prioridad
            // * retorna 0 si error

            // Changes the priority of the process with the given PID, validates permissions
            VALIDATE_PROCESS_PERMISSIONS(arg1);
            return changePriority((Pid)arg1, (Priority)arg2) == 0;
            break;

        case DETACH_PROCESS_SYSCALL:
            // * Orden syscall: pid del proceso a desvincular
            // * retorna 0 si error

            // Desvincula el proceso del shell, lo vuelve nohup
            VALIDATE_PROCESS_PERMISSIONS(arg1);  // Solo alguien de su grupo o con permisos globales puede desvincular un proceso

            // Busco el main del grupo porque un thread no puede desvincularse, se desvincula el proceso principal del grupo
            Pid process = getProcessGroupMain((Pid)arg1);

            return detachProcess(process) == 0;
            break;
        
        // SCHEDULING
        case YIELD_SYSCALL:
            // * sin args

            // Yield the CPU to allow the scheduler to choose another process
            scheduleNextProcess();
            break;
        case SET_WAITING_SYSCALL:
            // * orden syscall: pid del proceso a dejar waiting
            // * retorna 0 si error

            VALIDATE_PROCESS_PERMISSIONS(arg1);
            return setWaiting((Pid)arg1);
            break;
        case AGARRINI_LA_PALINI_SYSCALL: // Alias for WAKE_PROCESS_SYSCALL 
            // * orden syscall: pid del proceso a despertar
            // * retorna 0 si error

            VALIDATE_PROCESS_PERMISSIONS(arg1);
            return wakeProcess((Pid)arg1);
            break;
        
        // PROCESS INFO
        case GET_PID_SYSCALL:
            // * sin args
            // * retorna el pid del proceso actual

            return getCurrentProcessPID();
            break;
        case GET_PROCESS_INFO_SYSCALL:
            // * orden syscall: pid del proceso del cual se quiere la info, puntero a un struct Process donde se dejará la info
            // * si el pid del proceso devuelto es 0, es que hubo un error (de permisos o no existe el proceso de pid especificado)

            {
                VALIDATE_PROCESS_PERMISSIONS(arg1);
                Process process = getProcess((Pid)arg1);
                *(Process*) arg2 = process;
            }
            return 0;
            break;
        case GET_PROCESS_LIST_SYSCALL: {
            // * orden syscall: puntero a un Process* donde se dejará la lista de procesos, puntero a un int donde se dejará la cantidad de procesos
            // * -> Tarea de quien la llame liberar la memoria (todo: allocar a nombre del proceso)

                VALIDATE_PERMISSIONS(MANAGE_PROCESSES_PERMISSION);
                Process * processes = getAllProcesses((int *)arg2);
                if (processes == NULL) {
                    return 0;
                }
                *((Process**)arg1) = processes; // Deja la lista de procesos en el primer argumento
                return (uint64_t)processes;
            }
            break;
        case GET_GROUP_MAIN_PID_SYSCALL:
            // * No recibe nada, devuelve el PID del main del grupo al que pertenece el proceso
            return (uint64_t)getProcessGroupMain(getCurrentProcessPID());
            break;
        
        // ----- SEMAPHORES -----
        case SEM_INIT_SYSCALL:
            // * Inicializa un semáforo con el valor dado. Tarea del proceso liberarlo con sem destroy cuando no se use más
            // * retorna el id del semáforo creado, 0 si hubo error (retorna en el 2do arg)

            *(uint64_t*)arg2 = sem_init((int)arg1);
            return *(uint64_t*)arg2;
            break;
        case SEM_DESTROY_SYSCALL:
            // * Borra el semáforo con el id dado y libera la memoria. No se puede destruir si hay procesos esperandolo.
            sem_destroy(arg1);
            break;
        case SEM_WAIT_SYSCALL:
            // * Recibe el ID del semáforo a esperar
            sem_wait(arg1);
            break;
        case SEM_POST_SYSCALL:
            // * Recibe el ID del semáforo a postear
            sem_post(arg1);
            break;

        // ------ PROCESS I/O -----
        case READ_STDIN: {
            // * orden syscall: buffer donde dejar lo leído, cantidad de bytes a leer
            // * retrona: un signed int de 64 bits, con la cantidad de bytes que en efecto leyó, < 0 si EOF
            // * -> si no tenés un stdin asignado, o se borró el archivo, devuelve EOF directamente (-1 si EOF normal, -2 si el archivo no existe)
            //! como workarround a un problema raro con el retorno de las syscalls, también te deja en arg2 la cantidad de bytes leídos

                IO_Files IO_files = getIOFiles(getCurrentProcessPID());
                uint64_t result = readFifo(IO_files.stdin, (void *)arg1, *(uint32_t*)arg2);
                *(uint64_t*)arg2 = result;
                return result;
            }
            break; 

        case WRITE_STDOUT: {
            // * orden syscall: buffer con la info a escribir, cantidad de bytes a escribir
            // * retrona: un signed int de 64 bits, con la cantidad de bytes que en efecto escribió, -1 si el archivo está cerrado para escritura, -2 si el archivo no existe

                IO_Files IO_files = getIOFiles(getCurrentProcessPID());
                // console_log("Io files stdout: %d", IO_files.stdout);

                return writeFifo(IO_files.stdout, (void *)arg1, (uint32_t)arg2);
            }
            break;

        case WRITE_STDERR:{
            // * orden syscall: buffer con la info a escribir, cantidad de bytes a escribir
            // * retrona: un signed int de 64 bits, con la cantidad de bytes que en efecto escribió, -1 si el archivo está cerrado para escritura, -2 si el archivo no existe

                IO_Files IO_files = getIOFiles(getCurrentProcessPID());
                return writeFifo(IO_files.stderr, (void *)arg1, (uint32_t)arg2);
            }
            break;

        // ------ EVENT HANDLING ------
        case SUSCRIBE_TO_EVENT_SYSCALL:
            // * orden syscall: event ID, handler, condición
            registerEventSubscription((int)arg1, getCurrentProcessPID(), (void (*)(void *))arg2, (void *)arg3);
            break;
        case UNSUBSCRIBE_TO_EVENT_SYSCALL:
            // * orden syscall: event ID
            unregisterEventSubscription((int)arg1, getCurrentProcessPID());
            break;
        case WAIT_EVENT_SYSCALL:
            // * orden syscall: qué evento (ID), dónde te dejo la data, qué condición
            // Deja el proceso en waiting hasta que ocurra el evento. (cuando el evento ocurra, se despertará el proceso y tendrá en data la info del evento)
            registerEventWaiting((int)arg1, getCurrentProcessPID(), (void *)arg2, (void *)arg3);
            break;

        // ----- FILE SYSTEM -----
        case MK_FILE_SYSCALL:
            // * orden syscall: path, type, size, permissions
            // * retorna el id del archivo creado, 0 si error 

            return createFile((char *)arg1, (FileType)arg2, (uint32_t)arg3, *(FilePermissions *)arg4);
            break;
        case RM_FILE_SYSCALL:
            // * orden syscall: file ID
            // * retorna 0 si error

            VALIDATE_FILE_PERMISSIONS(arg1, FILE_REMOVE);
            return removeFile(arg1) == 0;
            break;

        case OPEN_FILE_SYSCALL:
            // * syscal args: path, file action, file type
            // * retorna 0 si error

            uint64_t fileId = openFile((char *)arg1, getCurrentProcessPID(), (FileAction)arg2, (FileType)arg3);
            VALIDATE_FILE_PERMISSIONS(fileId, (FileAction)arg2); // en teoría habría que cerrar el archivo si falla la validación, pero como open es básicamente un get id no hace falta
            return fileId;
            break;
        case CLOSE_FILE_SYSCALL:
            // * recibe file ID
            // * retorna 0 si error
            return closeFile((uint64_t)arg1, getCurrentProcessPID()) == 0;
            break;
        case CLOSE_FIFO_FOR_WRITING_SYSCALL:
            // * orden syscall: file id
            // * retorna 0 si error

            VALIDATE_FILE_PERMISSIONS(arg1, FILE_WRITE);
            return closeFifoForWriting((uint64_t)arg1) == 0;
            break;

        case READ_RAW_FILE_SYSCALL:
            // * orden syscall: file ID, buffer donde dejar lo leído, cantidad de bytes a leer, offset
            // * retrona: un signed int de 64 bits, con la cantidad de bytes que en efecto leyó, -1 si offset fuera de rango, -2 si el archivo no existe
            // * -> si no tenés permisos te devuelve 0 como si simplemente no leyó nada

            VALIDATE_FILE_PERMISSIONS(arg1, FILE_READ);
            return readRaw((uint64_t)arg1, (void *)arg2, (uint32_t)arg3, (uint32_t)arg4);
            break;
        case WRITE_RAW_FILE_SYSCALL:
            // * orden syscall: file ID, buffer con la info a escribir, cantidad de bytes a escribir, offset
            // * retrona: un signed int de 64 bits, con la cantidad de bytes que en efecto escribió, -1 si error de realocamiento, -2 si el archivo no existe
            // * -> si no tenés permisos te devuelve 0

            VALIDATE_FILE_PERMISSIONS(arg1, FILE_WRITE);
            return writeRawWithRealloc((uint64_t)arg1, (void *)arg2, (uint32_t)arg3, (uint32_t)arg4);
            break;
        case READ_FIFO_FILE_SYSCALL:
            // * orden syscall: file ID, buffer donde dejar lo leído, cantidad de bytes a leer
            // * retrona: un signed int de 64 bits, con la cantidad de bytes que en efecto leyó, -1 si EOF, -2 si el archivo no existe
            // * -> si no tenés permisos te devuelve 0
            //! como workarround a un problema raro con el retorno de las syscalls, también te deja en arg3 la cantidad de bytes leídos

            VALIDATE_FILE_PERMISSIONS(arg1, FILE_READ);
            int a =  readFifo((uint64_t)arg1, (void *)arg2, *(uint32_t*)arg3);
            *(uint32_t*)arg3 = a; // Devolvemos la cantidad de bytes leídos
            return a;
            break;
        case WRITE_FIFO_FILE_SYSCALL:
            // * orden syscall: file ID, buffer con la info a escribir, cantidad de bytes a escribir
            // * retrona: un signed int de 64 bits, con la cantidad de bytes que en efecto escribió, -1 si el archivo está cerrado para escritura, -2 si el archivo no existe
            // * -> si no tenés permisos te devuelve 0

            VALIDATE_FILE_PERMISSIONS(arg1, FILE_WRITE);
            return writeFifo((uint64_t)arg1, (void *)arg2, (uint32_t)arg3);
            break;

        case GET_FILE_SYSCALL:
            // * orden syscall: file ID, dónde dejar el archvo (puntero a File)
            // * -> el file va a tener ID 0 si es que no se encontró el archivo

            *(File*) arg2 = getFileById((uint64_t)arg1);
            break;
        case GET_FILE_LIST_SYSCALL:
            // * Devuelve una lista null terminated de IDs de todos los archivos, ordenados por path
            // * -> es tarea de quien la llama liberar la memoria

            return (uint64_t)listFiles(); 
            break;

        case SET_FILE_PERMISSIONS_SYSCALL:
            // * orden syscall: file ID, new permissions (*)
            // * solo alguien con permisos globales de archivos y el grupo del owner de escritura del archivo pueden cambiar los permisos
            // * retorna 0 si no se pudieron cambiar los permisos

            return setFilePermissions((uint64_t)arg1, validatePermissions(MANAGE_FILES_PERMISSION) ? 0 : getCurrentProcessPID(), *(FilePermissions *)arg2) == 0;
            break;
        case GET_FILE_PERMISSIONS_SYSCALL:
            // * orden syscall: file id, puntero donde retornar los permisos
            {
                FilePermissions permissions = getFilePermissions((uint64_t)arg1);
                *(FilePermissions *)arg2 = permissions;
            }
            break;
        case VALIDATE_FILE_ACCESS_PERMISSIONS_SYSCALL:
            // * orden syscall: file id, acción
            // * retorna 1 si el proceso que llamó puede hacer la acción dada en el archivo especificado
            return validateFileAccessPermissions(arg1, getCurrentProcessPID(), (FileAction)arg2);
            break;

        // TODO: MEMORY MANAGER que guarde quién registró cada cosa
        case ALLOCATE_MEMORY_SYSCALL:
            // * orden syscall: cantidad de memoria a alocar, puntero donde dejar el puntero a la memoria alocada
            // * malloc de toda la vida, devuelve un puntero a la memoria alocada o NULL si no se pudo alocar

            *(uint64_t*)arg2 = (uint64_t)malloc((uint64_t)arg1);
            return *(uint64_t*)arg2;

        // case REALLOCATE_MEMORY_SYSCALL:
        //     return reallocateMemory((uint64_t)arg1, (uint64_t)arg2, (uint64_t)arg3);
        //     break;

        case FREE_MEMORY_SYSCALL:
            // * orden syscall: puntero a la memoria a liberar
            // * libera la memoria alocada por malloc, no hace nada si el puntero es NULL

            free((void *)arg1);
            break;

        // TODO : Ver temas de system monitor

        // WINDOW MANAGER
        case SWITCH_WINDOW_SYSCALL:
            // * orden syscall: pid de la ventana a la que cambiar
            // * retrona 0 en caso de error

            VALIDATE_PERMISSIONS(MANAGE_WINDOWS_PERMISSION);
            return switchToWindow((Pid)arg1);
            break;
        case GET_WINDOW_LIST_SYSCALL:
            // * orden syscall: no recibe nada
            // * devuelve un arreglo de PIDs de las ventanas abiertas, null terminated
            // * Es tarea de quien llame a esta función liberar la memoria del arreglo devuelto

            return (uint64_t) getWindows();
            break;
        case GET_FOCUSED_WINDOW_SYSCALL:
            // * no recibe nada, devuelve el pid de la ventana que tiene el foco
            return getFocusedWindow();
            break;
        case IS_FOCUSED_WINDOW_SYSCALL:
            // * orden syscall: pid de la ventana a verificar si es la que tiene el foco
            // * devuelve 1 si es la ventana que tiene el foco, 0 si no
            {
                Pid focusedWindow = getFocusedWindow();
                return (focusedWindow == (Pid)arg1);
            }
            break;
        
        // PROGRAM MANAGER
        case GET_PROGRAM_SYSCALL:
            {
                Program* program = getProgramByCommand((char *)arg1);
                if (program == NULL) {
                    // Program not found, return 0
                    return 0;
                }
                // Return the address of the program
                *(Program *)arg2 = *program;
            }
            break;
        case GET_PROGRAM_LIST_SYSCALL:
            // Devuelve el puntero a la lista de programas y deja en arg1 la cantidad de programas
            {
                Program* programs = getAllPrograms();
                *(int *)arg1 = getProgramsCount(); // Store the count of programs in arg1
                // Return the address of the program list
                return (uint64_t)programs;
            }
            break;
        case SEARCH_PROGRAM_SYSCALL:
            {
                Program* program = searchProgramByPrefix((char *)arg1);
                if (program == NULL) {
                    // Program not found, return 0
                    return 0;
                }
                return (uint64_t)program;
            }
            break;
        case INSTALL_PROGRAM_SYSCALL:
            // * orden syscall: puntero a un struct Program con la info del programa a instalar
            // * retorna 0 si hubo error, 1 si éxito (el programa se instaló o ya estaba instalado)

            VALIDATE_PERMISSIONS(INSTALL_PROGRAM_PERMISSION);
            {
                Program * program = (Program *)arg1;
                return installProgram(program); 
            }
            break;
        case UNINSTALL_PROGRAM_SYSCALL:
            VALIDATE_PERMISSIONS(INSTALL_PROGRAM_PERMISSION);
            {
                const char * command = (const char *)arg1;
                return uninstallProgramByCommand(command);
            }
            break;

        // SYSTEM MONITOR
        default:
            break;
    }
    return 0;
}


// --- VIDEO DRIVER SYSCALLS ---
uint64_t videoDriverSyscallDispatcher(uint64_t syscall, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5) {
    if (are_interrupts_enabled()) {
        // console_log("E: INTERRUPCIONES HABILITADAS EN EL VIDEO DRIVER SYSCALL\n");
    }
    uint8_t * videoBuffer = getBufferByPID(getCurrentProcessPID());
    switch (syscall)
    {
        // BASIC SHAPES
        case DRAW_PIXEL_SYSCALL:
            VALIDATE_PERMISSIONS(DRAWING_PERMISSION);
            putPixel(videoBuffer, arg1, arg2, arg3);
            break;
        case DRAW_RECTANGLE_SYSCALL:
            VALIDATE_PERMISSIONS(DRAWING_PERMISSION);
            drawRectangle(videoBuffer, (Point *)arg1, (Point *)arg2, arg3);
            break;
        case DRAW_RECTANGLE_BORDER_SYSCALL:
            VALIDATE_PERMISSIONS(DRAWING_PERMISSION);
            drawRectangleBoder(videoBuffer, (Point *)arg1, (Point *)arg2, arg3, arg4);
            break; 

        // TEXT
        case DRAW_CHAR_SYSCALL:
            VALIDATE_PERMISSIONS(DRAWING_PERMISSION);
            drawChar(videoBuffer, arg1, arg2, arg3, 1);
            break;
        case DRAW_STRING_SYSCALL:
            VALIDATE_PERMISSIONS(DRAWING_PERMISSION);
            drawString(videoBuffer, (char *)arg1, arg2, arg3);
            break;
        case DRAW_CHAR_AT_SYSCALL:
            VALIDATE_PERMISSIONS(DRAWING_PERMISSION);
            drawCharAt(videoBuffer, arg1, arg2, arg3, (Point *) arg4);
            break;
        case DRAW_STRING_AT_SYSCALL:
            VALIDATE_PERMISSIONS(DRAWING_PERMISSION);
            drawStringAt(videoBuffer, (char *) arg1, arg2, arg3, (Point *) arg4);
            break;
        case DRAW_NUMBER_SYSCALL:
            VALIDATE_PERMISSIONS(DRAWING_PERMISSION);
            drawNumber(videoBuffer, arg1, arg2, arg3, 1);
            break;
        case DRAW_NUMBER_AT_SYSCALL:
            VALIDATE_PERMISSIONS(DRAWING_PERMISSION);
            drawNumberAt(videoBuffer, arg1, arg2, arg3, (Point *) arg4);
            break;
        case DRAW_HEX_SYSCALL:
            VALIDATE_PERMISSIONS(DRAWING_PERMISSION);
            drawHex(videoBuffer, arg1, arg2, arg3, 1);
            break;
        case DRAW_HEX_AT_SYSCALL:
            VALIDATE_PERMISSIONS(DRAWING_PERMISSION);
            drawHexAt(videoBuffer, arg1, arg2, arg3, (Point *) arg4);
            break;

        // BITMAPS
        case DRAW_BITMAP_SYSCALL:
            VALIDATE_PERMISSIONS(DRAWING_PERMISSION);
            drawBitmap(videoBuffer, (uint32_t *)arg1, arg2, arg3, (Point *)arg4, arg5);
            break;

        // CURSOR
        case IS_CURSOR_IN_BOUNDARIES_SYSCALL:
            int is_cursor_in_boundaries = isCursorInBoundaries(arg1, arg2);
            *(int *)arg3 = is_cursor_in_boundaries;
            break;

        case SET_CURSOR_COL_SYSCALL:
            VALIDATE_PERMISSIONS(DRAWING_PERMISSION);
            setCursorColumn(arg1);
            break;
        case SET_CURSOR_LINE_SYSCALL:
            VALIDATE_PERMISSIONS(DRAWING_PERMISSION);
            setCursorLine(arg1);
            break;
        case GET_CURSOR_COL_SYSCALL:
            uint32_t cursor_column =  getCursorColumn();
            *((uint32_t *)arg1) = cursor_column;
            break;
        case GET_CURSOR_LINE_SYSCALL:
            uint32_t cursor_line = getCursorLine();
            *((uint32_t *)arg1) = cursor_line;
            break;
        case GET_SCREEN_WIDTH_SYSCALL:
            uint64_t screen_width = getScreenWidth();
            *((uint64_t *)arg1) = screen_width;
            break;
        case GET_SCREEN_HEIGHT_SYSCALL:
            uint64_t screen_height = getScreenHeight();
            *(uint64_t *)arg1 = screen_height;
            break;
        case GET_CHAR_WIDTH_SYSCALL:
            uint64_t char_width = getCharWidth();
            *(uint64_t *)arg1 = char_width;
            break;
        case GET_CHAR_HEIGHT_SYSCALL:
            uint64_t char_height = getCharHeight();
            *(uint64_t *)arg1 = char_height;
            break;

        // GENERAL

        case CLEAR_SCREEN_SYSCALL:
            VALIDATE_PERMISSIONS(DRAWING_PERMISSION);
            clearScreen(videoBuffer, arg1);
            break;

        case DISABLE_REDRAW_SYSCALL:
            // * orden syscall: no recibe nada
            // * deshabilita el redraw de la pantalla, para evitar que se redibuje mientras se hacen cambios (si no el scroll parpadeaba mucho)

            setRedrawFlag(getCurrentProcessPID(), 0);
            break;
        case ENABLE_REDRAW_SYSCALL:
            // * orden syscall: no recibe nada
            // * habilita el redraw de la pantalla

            setRedrawFlag(getCurrentProcessPID(), 1);
            break;

        case SET_FONT_SIZE_SYSCALL:
            VALIDATE_PERMISSIONS(CHANGE_FONT_SIZE_PERMISSION);
            setFontSize(arg1);
            break;
        case INC_FONT_SIZE_SYSCALL:
            VALIDATE_PERMISSIONS(CHANGE_FONT_SIZE_PERMISSION);
            incFontSize();
            break;
        case DEC_FONT_SIZE_SYSCALL:
            VALIDATE_PERMISSIONS(CHANGE_FONT_SIZE_PERMISSION);
            decFontSize();
            break;
        case SET_FONT_SYSCALL:
            VALIDATE_PERMISSIONS(CHANGE_FONT_PERMISSION);
            setFont((Font)arg1);
            break;
    
        default:
            break;
    }
    return 0;
}

// --- PIT DRIVER SYSCALLS ---
uint64_t pitDriverSyscallDispatcher(uint64_t syscall, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5) {
    switch (syscall)
    {
        case SLEEP_SYSCALL:
            // sleep(arg1);
            console_log("E: Si lees esto, estas usando un sleep deprecado");
            break;
        case GET_MILLIS_ELAPSED_SYSCALL:
            uint64_t millis = milliseconds_elapsed();
            *(uint64_t *)arg1 = millis;
            break;
        default:
            break;
    }
    return 0;
}

// --- RTC DRIVER SYSCALLS ---
uint64_t rtcDriverSyscallDispatcher(uint64_t syscall, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5) {
    switch (syscall)
    {
        case GET_RTC_TIME_SYSCALL:
            get_time((RTC_Time *)arg1);
            break;
        case SET_TIMEZONE_SYSCALL:
            VALIDATE_PERMISSIONS(SET_TIMEZONE_PERMISSION);
            set_timezone(arg1);
            break;

        default:
            break;
    }
    return 0;
}

// --- KEYBOARD DRIVER SYSCALLS ---
uint64_t keyboardDriverSyscallDispatcher(uint64_t syscall, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5) {
    switch (syscall)
    {
        case GET_KEY_EVENT_SYSCALL:
            getKeyboardEvent( (KeyboardEvent *)arg1);
            break;
        case CLEAR_KEYBOARD_BUFFER_SYSCALL:
            clearKeyboardBuffer();
            break;
        case IS_KEY_PRESSED_SYSCALL:
            int is_key_pressed = isKeyPressed(arg1, arg2);
            *(int*)arg3 = is_key_pressed; 
            break;

        default:
            break;
    }
    return 0;
}

// --- AUDIO DRIVER SYSCALLS ---
uint64_t audioDriverSyscallDispatcher(uint64_t syscall, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5) {
    switch (syscall)
    {
        case PLAY_AUDIO_SYSCALL:
            VALIDATE_PERMISSIONS(PLAY_AUDIO_PERMISSION);
            play_audio((Note **)arg1, arg2, arg3);
            break;
        case STOP_AUDIO_SYSCALL:
            VALIDATE_PERMISSIONS(PLAY_AUDIO_PERMISSION);
            stop_audio();
            break;
        case PAUSE_AUDIO_SYSCALL:
            VALIDATE_PERMISSIONS(PLAY_AUDIO_PERMISSION);
            pause_audio();
            break;
        case RESUME_AUDIO_SYSCALL:
            VALIDATE_PERMISSIONS(PLAY_AUDIO_PERMISSION);
            resume_audio();
            break;
        case IS_AUDIO_PLAYING_SYSCALL:
            int is_playing = is_audio_playing();
            *(int*)arg1 = is_playing;
            break;
            
        case GET_AUDIO_STATE_SYSCALL:
            AudioState audio_state = get_audio_state();
            *(AudioState *)arg1 = audio_state;
            break;
        case LOAD_AUDIO_STATE_SYSCALL:
            VALIDATE_PERMISSIONS(PLAY_AUDIO_PERMISSION);
            load_audio_state(*(AudioState *)arg1);
            break;
        default:
            break;
    }
    return 0;
}

// --- SERIAL DRIVER SYSCALLS ---
uint64_t serialDriverSyscallDispatcher(uint64_t syscall, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5) {
    switch (syscall)
    {
        case MAKE_ETHEREAL_REQUEST_SYSCALL:
            VALIDATE_PERMISSIONS(MAKE_ETHEREAL_REQUEST_PERMISSION);
            make_ethereal_request((char *)arg1, (EtherPinkResponse *)arg2);
            break;
        case LOG_TO_SERIAL_SYSCALL:
            log_to_serial((char *)arg1);
            break;
        case LOG_DECIMAL_TO_SERIAL_SYSCALL:
            log_decimal((char*)arg1, (int)arg2);
            break;
        case LOG_HEX_TO_SERIAL_SYSCALL:
            log_hex((char*)arg1, (int)arg2);
            break;
        default:
            break;
    }
    return 0;
}