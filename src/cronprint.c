/**
 * December 19, 2013
 * The author disclaims copyright to this source code.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "horo.h"

#ifdef _WIN32
#include <Windows.h>
static void
delay()
{
    Sleep(1);
}

#else
static void
delay()
{
    usleep(1000 * 1000);
}
#endif


static void
usage()
{
    fprintf(stderr, "cronprint <cron string> <string to print>\n");
}

void printCB(void* actionData)
{
    time_t rawTime;
    struct tm* timeinfo;

    rawTime = time(NULL);
    timeinfo = localtime(&rawTime);
    fprintf(stdout, "%s: %s\n", asctime(timeinfo), (char*)actionData);
}

int
main(int argc, char** argv)
{
    HORO_ERROR err = HORO_SUCCESS;
    horo_clock_t* clock = NULL;
    int actionID;
    time_t rawTime;
    struct tm* timeinfo;
    horo_time_t horoTime;
    int dummy;
    if(argc != 3)
    {
        usage();
        exit(EXIT_FAILURE);
    }
    
    err = horo_init(&clock);
    if(err)
    {
        fprintf(stderr, "Error in horo_init: %d\n", err);
        exit(EXIT_FAILURE);
    }

    err = horo_scheduleAction(clock, argv[1], printCB, argv[2], &actionID);
    if(err)
    {
        fprintf(stderr, "Error with horo_scheduleAction: %d\n", err);
        goto Error;
    }

    while(1)
    {

        rawTime = time(NULL);

        timeinfo = localtime(&rawTime);
        horoTime.minute = timeinfo->tm_min;
        horoTime.hour = timeinfo->tm_hour;
        horoTime.dayOfMonth = timeinfo->tm_mday;

        //!! NOTICE THE +1 !!
        horoTime.month = timeinfo->tm_mon + 1;


        horoTime.dayOfWeek = timeinfo->tm_wday;
        err = horo_process(clock, &horoTime);
        delay();
    }

    horo_destroy(clock);

    if(0)
    {
    Error:
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}
