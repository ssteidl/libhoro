#include "Parser.h"

int 
isValidCronVal(int cronVal)
{
    //Check if the value fits in a uint64_t
    return ((cronVal >= 0) && (cronVal < 64));
}

int 
isValidMinute(int minute)
{
    return ((minute >= 0) && (minute <= 59));
}

int 
isValidHour(int hour)
{
    return ((hour >= 0) && (hour <= 23));
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
