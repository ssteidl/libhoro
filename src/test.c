#include <stdio.h>
#include <stdlib.h>

#include "Parser.h"

static void 
usage()
{
    fprintf(stderr, "test 'cronstring' \n");
}

int
main(int argc, char** argv)
{
    if(argc != 2)
    {
        usage();
        exit(EXIT_FAILURE);
    }

    CronVals cronVals;
    processCronString(argv[1], &cronVals);

    fprintf(stderr, "Hours: %d, Minutes: %d\n",
            (uint32_t)(cronVals.minute),
            (uint32_t)(cronVals.hour));
}
