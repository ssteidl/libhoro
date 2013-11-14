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

static void
cronFieldFromRange(Range* range, CronField* cronField)
{
    int i = range->start;
    for(; i <= range->stop; i += range->step)
    {
        cronField->val |= (1 << i);
    }
}

static void
cronFieldFromRangeList(CronField* cronField)
{
    RangeList* rangeList = &cronField->typeVal.rangeList;

    uint64_t val = 0;
    int i = 0;
    for(; i < rangeList->numRanges; i++)
    {
        cronFieldFromRange(&rangeList->ranges[i], cronField);
    }
}


static void
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

#define RETURN_POSITION_ERROR(position) \
    do { \
    switch(position) \
    { \
    case DBELL_POSITION_MINUTE: \
        return DBELL_ERROR_PARSER_MINUTE_RANGE; \
    break; \
    case DBELL_POSITION_HOUR: \
        return DBELL_ERROR_PARSER_HOUR_RANGE; \
    break; \
    case DBELL_POSITION_DOM: \
        return DBELL_ERROR_PARSER_DOM_RANGE; \
    break; \
    case DBELL_POSITION_MONTH: \
        return DBELL_ERROR_PARSER_MONTH_RANGE; \
    break; \
    case DBELL_POSITION_DOW: \
        return DBELL_ERROR_PARSER_DOW_RANGE; \
    break; \
    default: \
        return DBELL_ERROR_OUT_OF_RANGE; \
    break; \
    } \
    }while(0)

DBELL_ERROR
setCronFieldValues(CronField *cronField, FieldPosition_e position)
{
    if(cronField->type & DBELL_FIELD_TYPE_ASTERISK)
    {
        cronField->val = DBELL_ASTERISK;
    }
    
    if(cronField->type & DBELL_FIELD_TYPE_VALUE)
    {
        if(isValidCronVal(cronField->typeVal.value))
        {
            cronField->val = (1 << cronField->typeVal.value);
        }
        else
        {
            RETURN_POSITION_ERROR(position);
        }
    }
    
    if(cronField->type & DBELL_FIELD_TYPE_RANGE)
    {
        if(isValidCronVal(cronField->typeVal.range.start) &&
           isValidCronVal(cronField->typeVal.range.stop) &&
           isValidCronVal(cronField->typeVal.range.step))
        {
            cronFieldFromRange(&cronField->typeVal.range, cronField);
        }
        else
        {
            RETURN_POSITION_ERROR(position);
        }
    }

    if(cronField->type & DBELL_FIELD_TYPE_RANGELIST)
    {
        if(cronField->typeVal.rangeList.numRanges > MAX_RANGES_IN_RANGELIST)
        {
            RETURN_POSITION_ERROR(position);
        }

        int i = 0;
        for(; i < cronField->typeVal.rangeList.numRanges; i++)
        {
            if(!isValidCronVal(cronField->typeVal.range.start) ||
               !isValidCronVal(cronField->typeVal.range.stop) ||
               !isValidCronVal(cronField->typeVal.range.step))
            {
                RETURN_POSITION_ERROR(position);
            }
        }
        cronFieldFromRangeList(cronField);
    }

    if(cronField->type & DBELL_FIELD_TYPE_LIST)
    {
        int i = 0;
        for(; i < cronField->typeVal.list.numCount; i++)
        {
            if(!isValidCronVal(cronField->typeVal.list.listNums[i]))
            {
                RETURN_POSITION_ERROR(position);
            }
        }
        cronFieldFromList(&cronField->typeVal.list, cronField);
    }

    return DBELL_SUCCESS;
}