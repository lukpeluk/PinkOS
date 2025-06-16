#ifndef EVENT_IDS_H
#define EVENT_IDS_H

#define KEY_EVENT 0
#define SLEEP_EVENT 1
#define RTC_EVENT 2
#define PROCESS_DEATH_EVENT 3
#define EXCEPTION_EVENT 4
#define BROADCAST_EVENT 5


typedef struct {
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t day;
    uint8_t month;
    uint8_t year;
    uint8_t day_of_week;
} RTC_Time;

typedef struct {
    uint64_t exception_id; // Exception ID
    uint64_t error_code;   // Error code, if applicable
    uint64_t rip;          // Instruction pointer at the time of the exception
    uint64_t rsp;          // Stack pointer at the time of the exception
    uint64_t rflags;       // Flags register at the time of the exception
} Exception;


typedef struct KeyboardCondition {
    char scan_code; // The scan code of the key to filter
    char ascii;     // The ASCII character to filter
} KeyboardCondition;

typedef struct RTCCondition {
    uint8_t seconds; // Seconds to filter
    uint8_t minutes; // Minutes to filter
    uint8_t hours;   // Hours to filter
} RTCCondition;

typedef struct SleepCondition {
    uint64_t millis; // Milliseconds to filter
} SleepCondition;

typedef struct ProcessDeathCondition {
    Pid pid; // PID of the process that died
} ProcessDeathCondition;

typedef struct ExceptionCondition {
    uint64_t exception_id; // Exception ID to filter
} ExceptionCondition;



#endif
