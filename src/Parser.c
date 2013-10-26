#include "Parser.h"

static int 
isValidCronVal(int cronVal)
{
    //Check if the value fits in a uint64_t
    return ((cronVal >= 0) && (cronVal < 64));
}

static int 
isValidMinute(uint64_t minute)
{
    return (minute == DBELL_ASTERISK) || ((minute >= 0) && (minute <= ((uint64_t)1 << 59)));
}

static int 
isValidHour(uint64_t hour)
{
    return (hour == DBELL_ASTERISK) || ((hour >= 0) && (hour <= (1 << 23)));
}

static int
isValidDOM(uint64_t dayOfMonth)
{
    return (dayOfMonth == DBELL_ASTERISK) || ((dayOfMonth >= 1) && (dayOfMonth <= (1 << 31)));
}

static int
isValidMonth(uint64_t month)
{
    return (month == DBELL_ASTERISK) || ((month >= 1) && (month <= (1 << 12)));
}

static int 
isValidDOW(uint64_t dayOfWeek)
{
    return (dayOfWeek == DBELL_ASTERISK) || ((dayOfWeek >= 1) && (dayOfWeek <= (1 << 31)));
}

DBELL_ERROR
validateCronVals(CronVals const* cronVals)
{
    if(!isValidMinute(cronVals->minute)) return DBELL_ERROR_PARSER_MINUTE_RANGE;
    if(!isValidHour(cronVals->hour)) return DBELL_ERROR_PARSER_HOUR_RANGE;
    if(!isValidDOM(cronVals->dayOfMonth)) return DBELL_ERROR_PARSER_DOM_RANGE;
    if(!isValidMonth(cronVals->month)) return DBELL_ERROR_PARSER_MONTH_RANGE;
    if(!isValidDOW(cronVals->dayOfWeek)) return DBELL_ERROR_PARSER_DOW_RANGE;

    return DBELL_SUCCESS;
}

void
cronFieldFromRange(Range const* range, CronField* cronField)
{
    int i = range->start;
    for(; i <= range->stop; i += range->step)
    {
        cronField->val |= (1 << i);
    }
}

void
cronFieldFromRangeList(RangeList const* rangeList, CronField* cronField)
{
    uint64_t val = 0;
    int i = 0;
    for(; i < rangeList->numRanges; i++)
    {
        cronFieldFromRange(&rangeList->ranges[i], cronField);
    }
}


void
cronFieldFromList(List const* list, CronField* cronField)
{
    uint64_t val = 0;
    
    int i = 0;
    for(; i < list->numCount; i++)
    {
        val |= (1 << list->listNums[i]);
    }

    cronField->val = val;
}
