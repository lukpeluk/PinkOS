#include <stdint.h>
#include <videoDriver.h>


extern void rtc_acknowledge_interrupt();
typedef struct {
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t day;
    uint8_t month;
    uint8_t year;
    uint8_t day_of_week;
} RTC_Time;

extern void get_time(RTC_Time * time);

static char time[20];

RTC_Time convert_bcd_to_binary(RTC_Time time) {
    time.seconds = (time.seconds >> 4) * 10 + (time.seconds & 0x0F);
    time.minutes = (time.minutes >> 4) * 10 + (time.minutes & 0x0F);
    time.hours = (time.hours >> 4) * 10 + (time.hours & 0x0F);
    time.day = (time.day >> 4) * 10 + (time.day & 0x0F);
    time.month = (time.month >> 4) * 10 + (time.month & 0x0F);
    time.year = (time.year >> 4) * 10 + (time.year & 0x0F);
    return time;
}

void get_time_corrected(RTC_Time * time, int time_zone) {


    int adjusted_hour = time->hours + time_zone;


    // Ajustar la hora y gestionar cambios de día/mes/año
    if (adjusted_hour < 0) {
        time->hours = 24 + adjusted_hour;
        time->day--;

        if (time->day == 0) {  // Cambiar al mes anterior
            time->month--;
            if (time->month == 0) {  // Cambiar al año anterior
                time->month = 12;
                time->year--;
            }
            // Asignar el último día del mes anterior
            time->day = 31;  // Simplificación: ajustar según el mes si es necesario
        }
        time->day_of_week = (time->day_of_week == 0) ? 6 : time->day_of_week - 1;
    } else if (adjusted_hour >= 24) {
        time->hours = adjusted_hour - 24;
        time->day++;

        // Verificar cambio de mes
        if (time->day > 31) {  // Simplificación: ajustar según el mes si es necesario
            time->day = 1;
            time->month++;
            if (time->month > 12) {
                time->month = 1;
                time->year++;
            }
        }
        time->day_of_week = (time->day_of_week + 1) % 7;
    } else {
        time->hours = adjusted_hour;
    }
}
char str[22];

char * time_to_string(int time_zone) {
    RTC_Time time;
    get_time(&time);
    time = convert_bcd_to_binary(time);
    get_time_corrected(&time, time_zone);

    uint8_t h = time.hours;
    uint8_t m = time.minutes;
    uint8_t s = time.seconds;
    uint8_t d = time.day;
    uint8_t mo = time.month;
    uint8_t y = time.year;
    uint8_t dw = time.day_of_week;

    str[0] = (h / 10);
    str[1] = (h % 10);
    str[2] = 36;
    str[3] = (m / 10);
    str[4] = (m % 10);
    str[5] = 36;
    str[6] = (s / 10);
    str[7] = (s % 10);
    str[8] = 36;
    str[9] = (d / 10);
    str[10] = (d % 10);
    str[11] = 36;
    str[12] = (mo / 10);
    str[13] = (mo % 10);
    str[14] = 36;
    str[15] = (y / 10);
    str[16] = (y % 10);
    str[17] = 36;
    str[18] = 'D' - 'A' + 10;
    str[19] = 'W' - 'A' + 10;
    str[20] = dw;
    str[21] = 37;


    return str;
}


void int_28() {
    drawTime();
    rtc_acknowledge_interrupt();
}
   
void drawTime(){
    char * time = time_to_string(-5);
    for (int i = 0; i < 22; i++){
        drawChar(time[i], 0xFFFFFF, (i + 10) * 8);
    }
}


