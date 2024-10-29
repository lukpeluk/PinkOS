#include <stdint.h>
#include <videoDriver.h>


extern void read_rtc_time(uint8_t* time);
extern void disable_ints();
extern void enable_ints();
extern void outportb(uint16_t port, uint8_t value);
extern uint8_t inportb(uint16_t port);

extern uint8_t get_hours();
extern uint8_t get_minutes();
extern uint8_t get_seconds();
extern uint8_t get_day();
extern uint8_t get_month();
extern uint8_t get_year();
extern uint8_t get_day_of_week();

extern void rtc_acknowledge_interrupt();

static char time[20];
typedef struct {
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t day;
    uint8_t month;
    uint8_t year;
    uint8_t day_of_week;
} RTC_Time;

char * get_time(int time_zone){
    uint8_t h = get_hours();
    h = (h >> 4) * 10 + (h & 0x0F);
    h = (24 + h + time_zone) % 24;
    uint8_t m = get_minutes();
    uint8_t s = get_seconds();
    uint8_t d = get_day();
    uint8_t mo = get_month();
    uint8_t y = get_year();
    uint8_t dw = get_day_of_week();
    time[0] = 0 + (h / 10);
    time[1] = 0 + (h % 10);
    time[2] = 36;
    time[3] = 0 + (m >> 4);
    time[4] = 0 + (m & 0x0F);
    time[5] = 36;
    time[6] = 0 + (s >> 4);
    time[7] = 0 + (s & 0x0F);
    time[8] = 36;
    time[9] = 0 + (d >> 4);
    time[10] = 0 + (d & 0x0F);
    time[11] = 36;
    time[12] = 0 + (mo >> 4);
    time[13] = 0 + (mo & 0x0F);
    time[14] = 36;
    time[15] = 0 + (y >> 4);
    time[16] = 0 + (y & 0x0F);
    time[17] = 36;
    time[18] = 0 + (dw);
    time[19] = 37;


    return time;
}


void int_28() {
    drawTime();
    rtc_acknowledge_interrupt();
}
   
void drawTime(){
    char * time = get_time(-3);
    for (int i = 0; i < 20; i++){
        drawChar(time[i], 0xFFFFFF, (i + 10) * 8);
    }
}


