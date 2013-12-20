/**
 * December 19, 2013
 * The author disclaims copyright to this source code.
 */

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
    DBELL_FIELD_TYPE_VALUE = 1,
    DBELL_FIELD_TYPE_RANGE = 2,
    DBELL_FIELD_TYPE_LIST = 4,
    DBELL_FIELD_TYPE_RANGELIST = 8,
    DBELL_FIELD_TYPE_ASTERISK = 16
};

struct Range
{
    int start;
    int stop;
    int step;
};
typedef struct Range Range;
  
#define MAX_NUMS_IN_LIST 32
struct List
{
    int listNums[MAX_NUMS_IN_LIST];
    int numCount;
};
typedef struct List List;
  

#define MAX_RANGES_IN_RANGELIST 16
struct RangeList
{
    Range ranges[MAX_RANGES_IN_RANGELIST];
    int numRanges;
};
typedef struct RangeList RangeList;

struct CronField
{
    /*Use a struct instead of union because multiple fields can be set.*/
    struct 
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

typedef enum
{
    DBELL_POSITION_MINUTE,
    DBELL_POSITION_HOUR,
    DBELL_POSITION_DOM,
    DBELL_POSITION_MONTH,
    DBELL_POSITION_DOW
}FieldPosition_e;

DBELL_ERROR
setCronFieldValues(CronField *cronField, FieldPosition_e position);

DBELL_ERROR 
processCronString(char const* string, CronVals* oCronVals);
#endif
