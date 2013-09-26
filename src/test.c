#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "doorbell.h"



typedef struct
{
    dbell_time_t expectedTimeVals;
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

static void
runTest(const char* cronString,
        int currentMinute,
        int currentHour,
        int currentDayOfMonth,
        int currentMonth,
        int currentDayOfWeek,
        int expectError)
{
    dbell_clock_t* clock;
    dbell_init(&clock);
    int alarmID;
    testData_t testData;
    memset(&testData, 0, sizeof(testData));
    testData.expectedTimeVals.minute = currentMinute;
    testData.expectedTimeVals.hour = currentHour;
    testData.expectedTimeVals.dayOfMonth = currentDayOfMonth;
    testData.expectedTimeVals.month = currentMonth;
    testData.expectedTimeVals.dayOfWeek = currentDayOfWeek;
    testData.error = 1;
    dbell_scheduleAction(clock, cronString, action, &testData, &alarmID);
    dbell_process(clock, &testData.expectedTimeVals);

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

    dbell_destroy(clock);
}

static void
testSpecialStrings()
{
    int expectError = 0;

    //@hourly
    runTest("@hourly", 0, 1, 1, 2, 0, expectError);
    runTest("@hourly", 0, 2, 1, 2, 0, expectError);
    expectError = 1;
    runTest("@hourly", 1, 2, 1, 2, 0, expectError);

    //@daily
    expectError = 0;
    runTest("@daily", 0, 0, 1, 2, 0, expectError);
    runTest("@daily", 0, 0, 1, 3, 1, expectError);
    expectError = 1;
    runTest("@daily", 1, 2, 1, 2, 0, expectError);

    //@weekly
    expectError = 0;
    runTest("@weekly", 0, 0, 1, 2, 0, expectError);
    runTest("@weekly", 0, 0, 2, 3, 0, expectError);
    expectError = 1;
    runTest("@weekly", 1, 2, 1, 2, 6, expectError);

    /* //@hourly */
    /* runTest("@hourly", 0, 1, 1, 2, 0, expectError); */
    /* runTest("@hourly", 0, 2, 1, 2, 0, expectError); */
    /* expectError = 1; */
    /* runTest("@hourly", 1, 2, 1, 2, 0, expectError); */

    /* runTest("@daily", 0, 0, 2, 1, 1); */
    /* runTest("@weekly", 0, 0, 1, 1, 0); */
}

void
testLists()
{
    int expectError = 0;
    runTest("0,1,2,3,4 * * * *", 0, 0, 1, 2, 0, expectError);
    runTest("0,1,2,3,4 * * * *", 1, 0, 1, 2, 0, expectError);
    runTest("0,1,2,3,4 * * * *", 2, 0, 1, 2, 0, expectError);
    runTest("0,1,2,3,4 * * * *", 3, 0, 1, 2, 0, expectError);

    expectError = 1;
    runTest("0,1,2,3,4 * * * *", 5, 0, 1, 1, 0, expectError);
}

int
main(int argc, char** argv)
{
    dbell_clock_t* clock;
    DBELL_ERROR err = dbell_init(&clock);
    if(err)
    {
        fprintf(stderr, "error dbell_init: %d\n", err);
        exit(EXIT_FAILURE);
    }

    /* testSpecialStrings(); */
    testLists();
}
