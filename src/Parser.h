#ifndef PARSER_H
#define PARSER_H

#include "doorbell.h"
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

struct CronField
{
    uint64_t val;
    int isAsterisk;
    int asteriskStep;
    int hasError;
};
typedef struct CronField CronField;

typedef enum
{
    DBELL_PARSER_SUCCESS = 0x0,
    DBELL_PARSER_ERR_MINUTE_RANGE,
    DBELL_PARSER_ERR_HOUR_RANGE,
    DBELL_PARSER_ERR_DOM_RANGE,
    DBELL_PARSER_ERR_MONTH_RANGE,
    DBELL_PARSER_ERR_DOW_RANGE
}DBELL_PARSER_ERROR_e;
  
struct CronVals
{
    uint64_t minute;
    uint64_t hour;
    uint64_t dayOfMonth;
    uint64_t month;
    uint64_t dayOfWeek;

    DBELL_PARSER_ERROR_e error;
};
typedef struct CronVals CronVals;

int 
isValidCronVal(int cronVal);

int 
isValidMinute(int minute);

int 
isValidHour(int hour);

void
cronFieldFromList(List const* list, CronField* cronField);

void
cronFieldFromRange(Range const* range, CronField* cronField);

void
cronFieldFromRangeList(RangeList const* rangeList, CronField* cronField);

DBELL_ERROR 
processCronString(char const* string, CronVals* oCronVals);
#endif
