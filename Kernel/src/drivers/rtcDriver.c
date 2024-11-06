#include <drivers/rtcDriver.h>

extern void get_time_utc(RTC_Time * time);

static RTC_Time time;
static int time_zone = 0;


// Definimos cuántos días tiene cada mes
int days_in_month(int month, int year) {
    switch (month) {
        case 4: case 6: case 9: case 11:
            return 30;
        case 2:
            // Verificar si el año es bisiesto
            return (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)) ? 29 : 28;
        default:
            return 31;
    }
}

void adjust_time(RTC_Time *time, int displacement) {
    if (displacement == 0) return;

    int adjusted_hour = time->hours + displacement;

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
            time->day = days_in_month(time->month, time->year);
        }
        time->day_of_week = (time->day_of_week == 0) ? 6 : time->day_of_week - 1;

    } else if (adjusted_hour >= 24) {
        time->hours = adjusted_hour - 24;
        time->day++;

        // Verificar cambio de mes
        if (time->day > days_in_month(time->month, time->year)) {
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

// PUBLIC API

RTC_Time * update_time() {
    get_time_utc(&time);
    // adjust_time(&time, time_zone);   //TODO: PORQUE CHOTA NO FUNCIONA ESTO !??!?!?!
    return &time;
}

void get_time(RTC_Time * time_to_return) {
    *time_to_return = time;
}

void set_timezone(int time_zone) {
    time_zone = time_zone;
}






