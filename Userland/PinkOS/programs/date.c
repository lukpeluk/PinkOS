#include <stdpink.h>
#include <stdint.h>

typedef struct {
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t day;
    uint8_t month;
    uint8_t year;
    uint8_t day_of_week;
} RTC_Time;

extern void syscall(uint64_t syscall, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5);

void date_main(unsigned char * args){
    RTC_Time time;
    syscall(GET_RTC_TIME_SYSCALL, &time, 0, 0, 0, 0);
    unsigned char * month;
    unsigned char * weekday;
    switch (time.month){
        case 1:
            month = "Enero";
            break;
        case 2:
            month = "Febrero";
            break;
        case 3:
            month = "Marzo";
            break;
        case 4:
            month = "Abril";
            break;
        case 5:
            month = "Mayo";
            break;
        case 6:
            month = "Junio";
            break;
        case 7:
            month = "Julio";
            break;
        case 8:
            month = "Agosto";
            break;
        case 9:
            month = "Septiembre";
            break;
        case 10:
            month = "Octubre";
            break;
        case 11:
            month = "Noviembre";
            break;
        case 12:
            month = "Diciembre";
            break;
    }
    switch (time.day_of_week){
        case 1:
            weekday = "Domingo";
            break;
        case 2:
            weekday = "Lunes";
            break;
        case 3:
            weekday = "Martes";
            break;
        case 4:
            weekday = "Miercoles";
            break;
        case 5:
            weekday = "Jueves";
            break;
        case 6:
            weekday = "Viernes";
            break;
        case 7:
            weekday = "Sabado";
            break;
    }

    printf("Hoy es %s %d de %s del 20%d\n", weekday, time.day, month, time.year);
}