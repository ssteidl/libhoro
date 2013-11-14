#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "doorbell.h"

static void
usage()
{
    fprintf(stderr, "cronprint <cron string> <string to print>\n");
}

void printCB(void* actionData)
{
    time_t rawTime;
    rawTime = time(NULL);
    struct tm* timeinfo;
    timeinfo = localtime(&rawTime);
    fprintf(stdout, "%s: %s\n", asctime(timeinfo), (char*)actionData);
}

int
main(int argc, char** argv)
{
    if(argc != 3)
    {
        usage();
        exit(EXIT_FAILURE);
    }

    DBELL_ERROR err = DBELL_SUCCESS;
    dbell_clock_t* clock = NULL;
    
    err = dbell_init(&clock);
    if(err)
    {
        fprintf(stderr, "Error in dbell_init: %d\n", err);
        exit(EXIT_FAILURE);
    }

    int alarmID;
    err = dbell_scheduleAction(clock, argv[1], printCB, argv[2], &alarmID);
    if(err)
    {
        fprintf(stderr, "Error with dbell_scheduleAction: %d\n", err);
        goto ERROR;
    }

    while(1)
    {
        time_t rawTime;
        rawTime = time(NULL);
        struct tm* timeinfo;
        timeinfo = localtime(&rawTime);
        dbell_time_t dbellTime;
        dbellTime.minute = timeinfo->tm_min;
        dbellTime.hour = timeinfo->tm_hour;
        dbellTime.dayOfMonth = timeinfo->tm_mday;
        dbellTime.month = timeinfo->tm_mon;
        dbellTime.dayOfWeek = timeinfo->tm_wday;
        err = dbell_process(clock, &dbellTime);
        usleep(1000);
    }

    dbell_destroy(clock);
    if(0)
    {
ERROR:
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}
