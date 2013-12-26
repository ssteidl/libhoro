/**
 * December 19, 2013
 * The author disclaims copyright to this source code.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "horo.h"

typedef struct
{
    horo_time_t expectedTimeVals;
    int error;
}testData_t;

static void 
usage()
{
    fprintf(stderr, "test 'cronstring' \n");
}

static void
action(void* actionData)
{
    testData_t* testData = (testData_t*)actionData;

    //Clear the error
    testData->error = 0;
}

char *errorStrings[] = {
    "Success",
    "Illegal Argument",
    "No Memory Available",
    "Not Initialized",
    "Somethings Corrupt",
    "Minute Range Error",
    "Hour Range Error",
    "Day of Month Range Error",
    "Month Range Error",
    "Day of Week Range Error",
    "Illegal Field",
    "Generic Out of Range Error",
    "Unknown Action ID"
};

/**
   Test a cron string.
 
   @param cronString The string to test.

   @param currentMinute The simulated current minute
   @param currentHour The simulated current hour
   @param currentDayOfMonth The simulated current day of month
   @param currentMonth The simulated current month
   @param currentDayOfWeek The simulated current day of week
   
   @param expectError Set this to true if it is expected that the combination of
   the cronString and the simulated time values will not fire an action.

   @param expectedScheduleError Set this to a specific HORO_ERROR if a parsing
   error is expected.

   @param expectedProcessError Set this to a specific HORO_ERROR if a process 
   error is expected.  A process error is any error that occurs when calling
   horo_parse().
 */
static void
runTest(const char* cronString,
        int currentMinute,
        int currentHour,
        int currentDayOfMonth,
        int currentMonth,
        int currentDayOfWeek,
        int expectError,
	HORO_ERROR expectedScheduleError,
	HORO_ERROR expectedProcessError)
{
    horo_clock_t* clock = NULL;
    int actionID;
    testData_t testData;
    HORO_ERROR scheduleError = HORO_SUCCESS;
    HORO_ERROR processError = HORO_SUCCESS;
    horo_init(&clock);
    memset(&testData, 0, sizeof(testData));
    testData.expectedTimeVals.minute = currentMinute;
    testData.expectedTimeVals.hour = currentHour;
    testData.expectedTimeVals.dayOfMonth = currentDayOfMonth;
    testData.expectedTimeVals.month = currentMonth;
    testData.expectedTimeVals.dayOfWeek = currentDayOfWeek;
    testData.error = 1;

    scheduleError = horo_scheduleAction(clock, cronString, 
                                         action, &testData, &actionID);

    if(scheduleError != expectedScheduleError)
    {
        fprintf(stderr, "Test failed. Expected: %s, but was: %s\n", 
                errorStrings[expectedScheduleError], errorStrings[scheduleError]);
        exit(1);
    }

    processError = horo_process(clock, &testData.expectedTimeVals);
    if(expectedProcessError != processError)
    {
        fprintf(stderr, "Test failed. Expected: %s, but was: %s\n", 
                errorStrings[expectedProcessError], errorStrings[processError]);
        exit(1);
    }

    if(testData.error != expectError)
    {
        fprintf(stderr, "Test failed.  CronString: %s\n"
                        "Expect Error: %d\n"
                        "Found Error: %d\n"
                        "User Time: %d %d %d %d %d\n", 
                cronString, expectError, testData.error,
                currentMinute, currentHour, currentDayOfMonth,
                currentMonth, currentDayOfWeek);
        exit(1);
    }

    horo_destroy(clock);
}

static void
testSpecialStrings()
{
    int expectError = 0;
    HORO_ERROR scheduleError = HORO_SUCCESS;
    HORO_ERROR processError = HORO_SUCCESS;

    //@hourly
    runTest("@hourly", 0, 1, 1, 2, 0, expectError, scheduleError, processError);
    runTest("@hourly", 0, 2, 1, 2, 0, expectError, scheduleError, processError);
    expectError = 1;
    runTest("@hourly", 1, 2, 1, 2, 0, expectError, scheduleError, processError);

    //@daily
    expectError = 0;
    runTest("@daily", 0, 0, 1, 2, 0, expectError, scheduleError, processError);
    runTest("@daily", 0, 0, 1, 3, 1, expectError, scheduleError, processError);
    expectError = 1;
    runTest("@daily", 1, 2, 1, 2, 0, expectError, scheduleError, processError);

    //@weekly
    expectError = 0;
    runTest("@weekly", 0, 0, 1, 2, 0, expectError, scheduleError, processError);
    runTest("@weekly", 0, 0, 2, 3, 0, expectError, scheduleError, processError);
    expectError = 1;
    runTest("@weekly", 1, 2, 1, 2, 6, expectError, scheduleError, processError);

    /* //@hourly */
    expectError = 0;
    runTest("@hourly", 0, 1, 1, 2, 0, expectError, scheduleError, processError);
    runTest("@hourly", 0, 2, 1, 2, 0, expectError, scheduleError, processError);
    expectError = 1;
    runTest("@hourly", 1, 2, 1, 2, 0, expectError, scheduleError, processError);
}

void
testLists()
{
    int expectError = 0;
    HORO_ERROR scheduleError = HORO_SUCCESS;
    HORO_ERROR processError = HORO_SUCCESS;
    runTest("0,1,2,3,4 * * * *", 0, 0, 1, 2, 0, expectError, scheduleError, processError);
    runTest("0,1,2,3,4 * * * *", 1, 0, 1, 2, 0, expectError, scheduleError, processError);
    runTest("0,1,2,3,4 * * * *", 2, 0, 1, 2, 0, expectError, scheduleError, processError);
    runTest("0,1,2,3,4 * * * *", 3, 0, 1, 2, 0, expectError, scheduleError, processError);

    expectError = 1;
    runTest("0,1,2,3,4 * * * *", 5, 0, 1, 1, 0, expectError, scheduleError, processError);
}

void
testRanges()
{
    int expectError = 0;
    HORO_ERROR scheduleError = HORO_SUCCESS;
    HORO_ERROR processError = HORO_SUCCESS;
    runTest("* 8-10 * * *", 7, 8, 10, 11, 6, expectError, scheduleError, processError);
    runTest("* 1-10/2 * 11 *", 7, 3, 10, 11, 6, expectError, scheduleError, processError);

    expectError = 1;
    scheduleError = HORO_ERROR_OUT_OF_RANGE; //Too many digits in 1000 only 2 digits allowed 
    processError = HORO_SUCCESS;
    runTest("* 1-1000 * * *", 7, 8, 10, 11, 6, expectError, scheduleError, processError);

    expectError = 1;
    scheduleError = HORO_SUCCESS;
    processError = HORO_SUCCESS;
    runTest("* 1-10/2 * 11 *", 7, 8, 10, 11, 6, expectError, scheduleError, processError);

    expectError = 0;
    scheduleError = HORO_SUCCESS;
    processError = HORO_SUCCESS;
    runTest("*/3 1-10/2 * 11 *", 6, 9, 10, 11, 6, expectError, scheduleError, processError);

    expectError = 1;
    scheduleError = HORO_SUCCESS;
    processError = HORO_SUCCESS;
    runTest("*/3 1-10/2 * 11 *", 7, 9, 10, 11, 6, expectError, scheduleError, processError);
}

void
testMaxVals()
{
    int expectError = 0;
    HORO_ERROR scheduleError = HORO_SUCCESS;
    HORO_ERROR processError = HORO_SUCCESS;
    runTest("* 21-23 * * *", 7, 22, 10, 11, 6, 
            expectError, scheduleError, processError);

    runTest("0-59 0-23 1-31 1-12 0-7", 59, 23, 31, 12, 7,
            expectError, scheduleError, processError);

    scheduleError = HORO_ERROR_PARSER_MINUTE_RANGE;
    expectError = 1;
    runTest("60 * * * *", 13, 8, 10, 11, 6,
            expectError, scheduleError, processError);

    scheduleError = HORO_ERROR_PARSER_HOUR_RANGE;
    expectError = 1;
    runTest("* 24 * * *", 13, 8, 10, 11, 6,
            expectError, scheduleError, processError);

    scheduleError = HORO_ERROR_PARSER_DOM_RANGE;
    expectError = 1;
    runTest("* * 32 * *", 13, 8, 10, 11, 6,
            expectError, scheduleError, processError);

    scheduleError = HORO_ERROR_PARSER_MONTH_RANGE;
    expectError = 1;
    runTest("* * * 13 *", 13, 8, 10, 11, 6,
            expectError, scheduleError, processError);

    scheduleError = HORO_ERROR_PARSER_DOW_RANGE;
    expectError = 1;
    runTest("* * * * 8", 13, 8, 10, 11, 6,
            expectError, scheduleError, processError);
}

static void
dummyAction(void* unused)
{
    return;
}

static void
testRemove()
{
    horo_clock_t* clock = NULL;
    HORO_ERROR err = HORO_SUCCESS;
    int actionID = -1;
    int count = 0;

    err = horo_init(&clock);
    if(err)
    {
        fprintf(stderr, "Error initializing clock: %s\n", errorStrings[err]);
        exit(err);
    }

    err = horo_actionCount(clock, &count);
    assert(err == HORO_SUCCESS);
    assert(count == 0);

    err = horo_scheduleAction(clock, "* * * * *", dummyAction, 
                               NULL, &actionID);
    assert(actionID == 0);
    assert(err == HORO_SUCCESS);

    err = horo_scheduleAction(clock, "* * * * *", dummyAction, 
                                NULL, &actionID);
    assert(actionID == 1);
    assert(err == HORO_SUCCESS);

    err = horo_scheduleAction(clock, "* * * * *", dummyAction, 
                                NULL, &actionID);
    assert(actionID == 2);
    assert(err == HORO_SUCCESS);

    err = horo_scheduleAction(clock, "* * * * *", dummyAction, 
                                NULL, &actionID);
    assert(actionID == 3);
    assert(err == HORO_SUCCESS);

    err = horo_scheduleAction(clock, "* * * * *", dummyAction, 
                                NULL, &actionID);
    assert(actionID == 4);
    assert(err == HORO_SUCCESS);

    err = horo_actionCount(clock, &count);
    assert(err == HORO_SUCCESS);
    assert(count == 5);

    err = horo_unscheduleAction(clock, 0);
    assert(err == HORO_SUCCESS);
    err = horo_actionCount(clock, &count);
    assert(err == HORO_SUCCESS);
    assert(count == 4);

    err = horo_unscheduleAction(clock, 4);
    assert(err == HORO_SUCCESS);
    err = horo_actionCount(clock, &count);
    assert(err == HORO_SUCCESS);
    assert(count == 3);

    err = horo_unscheduleAction(clock, 2);
    assert(err == HORO_SUCCESS);
    err = horo_unscheduleAction(clock, 1);
    assert(err == HORO_SUCCESS);
    err = horo_unscheduleAction(clock, 3);
    assert(err == HORO_SUCCESS);

    err = horo_actionCount(clock, &count);
    assert(err == HORO_SUCCESS);
    assert(count == 0);
}

int
main(int argc, char** argv)
{
    testRemove();
    testMaxVals();
    testSpecialStrings();
    testLists();
    testRanges();
}
