#ifndef DOORBELL_H
#define DOORBELL_H

#include <stdint.h>

typedef enum
{
    /** Operation was successful */
    DBELL_SUCCESS = 0x0,

    /** A parameter passed into the function was NULL or out of range*/
    DBELL_ERROR_ILLEGAL_ARG = 0x1,

    /** malloc returned NULL */
    DBELL_ERROR_NO_MEM = 0x2,

    /** A parameter passed into the function was not initialized */
    DBELL_ERROR_NOT_INITIALIZED = 0x3,

    /** An internal datastructure has been corrupted */
    DBELL_ERROR_CORRUPT = 0x4,

    /** Schedule string minute field is out of range */
    DBELL_ERROR_PARSER_MINUTE_RANGE = 0x5,

    /** Schedule string hour field is out of range */
    DBELL_ERROR_PARSER_HOUR_RANGE = 0x6,

    /** Schedule string day of month field is out of range */
    DBELL_ERROR_PARSER_DOM_RANGE = 0x7,

    /** Schedule string month field is out of range */
    DBELL_ERROR_PARSER_MONTH_RANGE = 0x8,

    /** Schedule string day of week field is out of range */
    DBELL_ERROR_PARSER_DOW_RANGE = 0x9,

    /** A field in the schedule string is not understood */
    DBELL_ERROR_PARSER_ILLEGAL_FIELD = 0xA,

    /** Generic schedule string out of range.  This is returned
     * when a field of the schedule string would overflow an internal
     * buffer.  Any number that is greater than 2 digits in the schedule
     * string will cause this error to be returned. */
    DBELL_ERROR_OUT_OF_RANGE = 0xB,

    /** Returned by dbell_unscheduleAction() when the given actionID cannot
     * be found. */
    DBELL_ERROR_UNKNOWN_ACTION = 0xC
}DBELL_ERROR;


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
    int minute; /**< 0-59*/
    int hour; /**< 0-23*/
    int dayOfMonth; /**< 1-31*/
    int month; /**< 1-12*/
    int dayOfWeek; /**< 0-7 (0 or 7 is Sun)*/
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
 * The number of actions that are scheduled to be executed by 'clock'.
 *
 * @param[in] clock A clock structure to which the actions are attached.
 *
 * @param[out] oActionCount The number of actions attached to the clock.
 */
DBELL_ERROR
dbell_actionCount(dbell_clock_t* clock, int* oActionCount);

/**
 * The asynchronous interface to libdoorbell. This function must be called at least
 * every minute.  If it is not called at least every minute than any action scheduled
 * for the minute that was skipped will not execute. Therefore, tasks that take longer
 * than 1 minute to execute must be executed in their own thread.
 *
 * !!NOTE: libdoorbell knows nothing about threads and is not thread safe.
 * dbell_process() must always be called by the same thread.
 *
 * @param[in] clock A clock structure to which the actions are attached.
 *
 * @param[in] timeVals A dbell_time_t structure that is used by the scheduler as the
 * current time.  This structure is generally filled in with the values returned by
 * the localtime() system function.
 * 
 * !!NOTE: If using localtime() to fill in the timeVals structure, you must add 1 to
 * the tm_mon field as shown in the example below.  This is because localtime()
 * returns 0-11 and the crontab spec expects 1-12.
 *
 * Below is an example of using dbellprocess(). For a full example of using libdoorbell, 
 * see cronprint.c
 * EXAMPLE:
 *
 *   DBELL_ERROR err = DBELL_SUCCESS;
 *   dbell_clock_t* clock = NULL;
 *   time_t rawTime;
 *   struct tm* timeinfo;
 *   dbell_time_t dbellTime;
 *
 *   //Create clock and schedule action....
 *
 *   while(1)
 *   {
 *
 *       rawTime = time(NULL);
 *
 *       timeinfo = localtime(&rawTime);
 *       dbellTime.minute = timeinfo->tm_min;
 *       dbellTime.hour = timeinfo->tm_hour;
 *       dbellTime.dayOfMonth = timeinfo->tm_mday;
 *
 *       //!! NOTICE THE +1 !!
 *       dbellTime.month = timeinfo->tm_mon + 1;
 *
 *
 *       dbellTime.dayOfWeek = timeinfo->tm_wday;
 *       err = dbell_process(clock, &dbellTime);
 *       
 *       //Do other stuff that takes less than 1 minute.
 *   }
 */
DBELL_ERROR
dbell_process(dbell_clock_t* clock, dbell_time_t const* timeVals);

/**
 * Return clock's resources to the system.
 *
 * @param[in] clock The clock structure to be destroyed.
 */
DBELL_ERROR
dbell_destroy(dbell_clock_t* clock);

#endif // DOORBELL_H
