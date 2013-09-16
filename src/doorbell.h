#ifndef DOORBELL_H
#define DOORBELL_H

#include <inttypes.h>

typedef enum
{
    DBELL_SUCCESS = 0x0,
    DBELL_ERROR_ILLEGAL_ARG = 0x1,
    DBELL_ERROR_NO_MEM = 0x2,
    DBELL_ERROR_NOT_INITIALIZED = 0x3,
    DBELL_ERROR_CORRUPT = 0x4,
    DBELL_ERROR_PARSER = 0x5,
    DBELL_ERROR_OUT_OF_RANGE = 0x6
}DBELL_ERROR;

#define DBELL_ASTERISK 0xFFFFFFFFFFFFFFFF

struct dbell_time
{
    int minute;
    int hour;
    int dayOfMonth;
    int month;
    int dayOfWeek;
};

typedef struct dbell_time dbell_time_t;

typedef void (*dbell_actionFunc)(void* actionData);

typedef struct dbell_clock dbell_clock_t;

DBELL_ERROR
dbell_init(dbell_clock_t** oClock);

DBELL_ERROR
dbell_scheduleAction(dbell_clock_t* clock, const char *scheduleString, 
                     dbell_actionFunc action, void *actionData,
                     int* alarmID);
                     
DBELL_ERROR
dbell_process(dbell_clock_t* clock, dbell_time_t const* cronVals);

DBELL_ERROR
dbell_destroy(dbell_clock_t* clock);

#endif // DOORBELL_H
