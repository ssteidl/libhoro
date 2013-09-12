#include <stdio.h>
#include <stdlib.h>

#include "doorbell.h"

static void 
usage()
{
    fprintf(stderr, "test 'cronstring' \n");
}

static void
action(void* action)
{
    fprintf(stdout, "Action occurred\n");
}

int
main(int argc, char** argv)
{
    /* if(argc != 2) */
    /* { */
    /*     usage(); */
    /*     exit(EXIT_FAILURE); */
    /* } */

    dbell_clock_t* clock;
    DBELL_ERROR err = dbell_init(&clock);
    if(err)
    {
        fprintf(stderr, "error dbell_init: %d\n", err);
        exit(EXIT_FAILURE);
    }

    int actionID = 0;
    err = dbell_scheduleAction(clock, "1-60 10-23", action, NULL, &actionID);
    if(err)
    {
        fprintf(stderr, "error dbell_scheduleAction: %d\n", err);
        exit(EXIT_FAILURE);
    }

    while(1)
    {
        dbell_process(clock);
    }
}
