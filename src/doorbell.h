#ifndef DOORBELL_H
#define DOORBELL_H

#include <stdint.h>

typedef enum
{
    DBELL_SUCCESS = 0x0,
    DBELL_ERROR_ILLEGAL_ARG = 0x1,
    DBELL_ERROR_NO_MEM = 0x2,
    DBELL_ERROR_NOT_INITIALIZED = 0x3,
    DBELL_ERROR_CORRUPT = 0x4,
    DBELL_ERROR_PARSER_MINUTE_RANGE = 0x5,
    DBELL_ERROR_PARSER_HOUR_RANGE = 0x6,
    DBELL_ERROR_PARSER_DOM_RANGE = 0x7,
    DBELL_ERROR_PARSER_MONTH_RANGE = 0x8,
    DBELL_ERROR_PARSER_DOW_RANGE = 0x9,
    DBELL_ERROR_PARSER_ILLEGAL_FIELD = 0xA,
    DBELL_ERROR_OUT_OF_RANGE = 0xB,
    DBELL_ERROR_UNKNOWN_ACTION = 0xC
}DBELL_ERROR;

//REVIEW: Does this need to be part of the public interface?
#define DBELL_ASTERISK (uint64_t)0xFFFFFFFFFFFFFFFF

/** 
 * An instance of this structure must be filled in with the current
 * time values and passed to dbell_process().  dbell_process() will 
 * use these values to check if the clock has any existing actions to
 * be called.
 *
 * @see cronprint.c 
 */
struct dbell_time
{
    int minute;
    int hour;
    int dayOfMonth;
    int month;
    int dayOfWeek;
};
typedef struct dbell_time dbell_time_t;

/**
 * Type definition for an action callback.  Actions
 * are linked to a dbell_clock_t and scheduled for
 * execution using the dbell_scheduleAction() function.
 */
typedef void (*dbell_actionFunc)(void* actionData);

/**
 * Opaque data structure used to manage actions and their
 * schedules.
 */
typedef struct dbell_clock dbell_clock_t;

/**
 * Initialize the library.
 *
 * @param[out] oClock The address of a pointer that will be made to
 * point to an initialized dbell_clock.  The dbell_clock's 
 * resources must be returned to the system using dbell_destroy
 * after the dbell_clock is no longer needed.
 */
DBELL_ERROR
dbell_init(dbell_clock_t** oClock);

/**
 * Schedule an action to be executed periodically as described in the
 * schedule string.
 *
 * @param[in] clock A clock structure to which the action will 
 * be attached.
 *
 * @param[in] scheduleString The cron based schedule string which
 * describes when the action will be executed.
 *
 * @param[in] action The action callback function.
 *
 * @param[in] Data to be passed to the action when it is executed.
 *
 * @param[out] oActionID The id of the action so that it can be
 * unscheduled if necessary. 
 */
DBELL_ERROR
dbell_scheduleAction(dbell_clock_t* clock, const char *scheduleString, 
                     dbell_actionFunc action, void *actionData,
                     int* oActionID);

/**
 * Unschedule an action.
 *
 * @param[in] clock The clock structure that contains the action.
 *
 * @param[in] actionID The actionID from dbell_scheduleAction.
 */                     
DBELL_ERROR
dbell_unscheduleAction(dbell_clock_t* clock, int actionID);

/**
 *
 */
DBELL_ERROR
dbell_actionCount(dbell_clock_t* clock, int* oActionCount);

DBELL_ERROR
dbell_process(dbell_clock_t* clock, dbell_time_t const* timeVals);

DBELL_ERROR
dbell_destroy(dbell_clock_t* clock);

#endif // DOORBELL_H
