/**
 * December 19, 2013
 * The author disclaims copyright to this source code.
 */

#ifndef HORO_H
#define HORO_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    /** Operation was successful */
    HORO_SUCCESS = 0x0,

    /** A parameter passed into the function was NULL or out of range*/
    HORO_ERROR_ILLEGAL_ARG = 0x1,

    /** malloc returned NULL */
    HORO_ERROR_NO_MEM = 0x2,

    /** A parameter passed into the function was not initialized */
    HORO_ERROR_NOT_INITIALIZED = 0x3,

    /** An internal datastructure has been corrupted */
    HORO_ERROR_CORRUPT = 0x4,

    /** Schedule string minute field is out of range */
    HORO_ERROR_PARSER_MINUTE_RANGE = 0x5,

    /** Schedule string hour field is out of range */
    HORO_ERROR_PARSER_HOUR_RANGE = 0x6,

    /** Schedule string day of month field is out of range */
    HORO_ERROR_PARSER_DOM_RANGE = 0x7,

    /** Schedule string month field is out of range */
    HORO_ERROR_PARSER_MONTH_RANGE = 0x8,

    /** Schedule string day of week field is out of range */
    HORO_ERROR_PARSER_DOW_RANGE = 0x9,

    /** A field in the schedule string is not understood */
    HORO_ERROR_PARSER_ILLEGAL_FIELD = 0xA,

    /** Generic schedule string out of range.  This is returned
     * when a field of the schedule string would overflow an internal
     * buffer.  Any number that is greater than 2 digits in the schedule
     * string will cause this error to be returned. */
    HORO_ERROR_OUT_OF_RANGE = 0xB,

    /** Returned by horo_unscheduleAction() when the given actionID cannot
     * be found. */
    HORO_ERROR_UNKNOWN_ACTION = 0xC
}HORO_ERROR;


#define HORO_ASTERISK (uint64_t)0xFFFFFFFFFFFFFFFF

/** 
 * An instance of this structure must be filled in with the current
 * time values and passed to horo_process().  horo_process() will 
 * use these values to check if the clock has any existing actions to
 * be called.
 *
 * @see cronprint.c 
 */
struct horo_time
{
    int minute; /**< 0-59*/
    int hour; /**< 0-23*/
    int dayOfMonth; /**< 1-31*/
    int month; /**< 1-12*/
    int dayOfWeek; /**< 0-7 (0 or 7 is Sun)*/
};
typedef struct horo_time horo_time_t;

/**
 * Type definition for an action callback.  Actions
 * are linked to a horo_clock_t and scheduled for
 * execution using the horo_scheduleAction() function.
 */
typedef void (*horo_actionFunc)(void* actionData);

/**
 * Opaque data structure used to manage actions and their
 * schedules.
 */
typedef struct horo_clock horo_clock_t;

/**
 * Initialize the library.
 *
 * @param[out] oClock The address of a pointer that will be made to
 * point to an initialized horo_clock.  The horo_clock's 
 * resources must be returned to the system using horo_destroy
 * after the horo_clock is no longer needed.
 */
HORO_ERROR
horo_init(horo_clock_t** oClock);

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
HORO_ERROR
horo_scheduleAction(horo_clock_t* clock, const char *scheduleString, 
                     horo_actionFunc action, void *actionData,
                     int* oActionID);

/**
 * Unschedule an action.
 *
 * @param[in] clock The clock structure that contains the action.
 *
 * @param[in] actionID The actionID from horo_scheduleAction.
 */                     
HORO_ERROR
horo_unscheduleAction(horo_clock_t* clock, int actionID);

/**
 * The number of actions that are scheduled to be executed by 'clock'.
 *
 * @param[in] clock A clock structure to which the actions are attached.
 *
 * @param[out] oActionCount The number of actions attached to the clock.
 */
HORO_ERROR
horo_actionCount(horo_clock_t* clock, int* oActionCount);

/**
 * The asynchronous interface to libhoro. This function must be called at least
 * every minute.  If it is not called at least every minute than any action scheduled
 * for the minute that was skipped will not execute. Therefore, tasks that take longer
 * than 1 minute to execute must be executed in their own thread.
 *
 * !!NOTE: libhoro knows nothing about threads and is not thread safe.
 * horo_process() must always be called by the same thread.
 *
 * @param[in] clock A clock structure to which the actions are attached.
 *
 * @param[in] timeVals A horo_time_t structure that is used by the scheduler as the
 * current time.  This structure is generally filled in with the values returned by
 * the localtime() system function.
 * 
 * !!NOTE: If using localtime() to fill in the timeVals structure, you must add 1 to
 * the tm_mon field as shown in the example below.  This is because localtime()
 * returns 0-11 and the crontab spec expects 1-12.
 *
 * Below is an example of using horoprocess(). For a full example of using libhoro, 
 * see cronprint.c
 * EXAMPLE:
 *
 *   HORO_ERROR err = HORO_SUCCESS;
 *   horo_clock_t* clock = NULL;
 *   time_t rawTime;
 *   struct tm* timeinfo;
 *   horo_time_t horoTime;
 *
 *   //Create clock and schedule action....
 *
 *   while(1)
 *   {
 *
 *       rawTime = time(NULL);
 *
 *       timeinfo = localtime(&rawTime);
 *       horoTime.minute = timeinfo->tm_min;
 *       horoTime.hour = timeinfo->tm_hour;
 *       horoTime.dayOfMonth = timeinfo->tm_mday;
 *
 *       //!! NOTICE THE +1 !!
 *       horoTime.month = timeinfo->tm_mon + 1;
 *
 *
 *       horoTime.dayOfWeek = timeinfo->tm_wday;
 *       err = horo_process(clock, &horoTime);
 *       
 *       //Do other stuff that takes less than 1 minute.
 *   }
 */
HORO_ERROR
horo_process(horo_clock_t* clock, horo_time_t const* timeVals);

/**
 * Return clock's resources to the system.
 *
 * @param[in] clock The clock structure to be destroyed.
 */
HORO_ERROR
horo_destroy(horo_clock_t* clock);

#ifdef __cplusplus
}
#endif

#endif // HORO_H
