#ifndef PARSER_H
#define PARSER_H

#include <stdint.h>
  
struct Token
{
    char string[64];
    int length;
};
typedef struct Token Token;
    
struct Range
{
    int start;
    int stop;
    int step;
};
typedef struct Range Range;
  
struct List
{
    int listNums[32];
    int numCount;
};
typedef struct List List;
  
struct RangeList
{
    Range ranges[16];
    int numRanges;
};
typedef struct RangeList RangeList;

struct CronVals
{
    uint64_t minute;
    uint64_t  hour;
    uint64_t dayOfMonth;
    uint64_t month;
    uint64_t dayOfWeek;
};
typedef struct CronVals CronVals;
  
void processCronString(char const* string, CronVals* oCronVals);
#endif
