#ifndef PARSER_H
#define PARSER_H

#include "doorbell.h"

struct Token
{
    char string[64];
    int length;
};
typedef struct Token Token;
    
enum {
    DBELL_FIELD_TYPE_VALUE,
    DBELL_FIELD_TYPE_RANGE,
    DBELL_FIELD_TYPE_LIST,
    DBELL_FIELD_TYPE_RANGELIST,
    DBELL_FIELD_TYPE_ASTERISK
};

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

struct CronField
{
    union
    {
        int value;
        Range range;
        List list;
        RangeList rangeList;
        int asteriskStep;
    }typeVal;
    
    int type;
    uint64_t val;
    int hasError;
};
typedef struct CronField CronField;

struct CronVals
{
    uint64_t minute;
    uint64_t hour;
    uint64_t dayOfMonth;
    uint64_t month;
    uint64_t dayOfWeek;

    DBELL_ERROR error;
};
typedef struct CronVals CronVals;

DBELL_ERROR
validateCronVals(CronVals const* cronVals);

void
cronFieldFromList(List const* list, CronField* cronField);

void
cronFieldFromRange(Range const* range, CronField* cronField);

void
cronFieldFromRangeList(RangeList const* rangeList, CronField* cronField);

DBELL_ERROR 
processCronString(char const* string, CronVals* oCronVals);
#endif
