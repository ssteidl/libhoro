/**
 * December 19, 2013
 * The author disclaims copyright to this source code.
 */

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
    return (minute == HORO_ASTERISK) || ((minute >= 0) && (minute < ((uint64_t)1 << 60)));
}

static int 
isValidHour(uint64_t hour)
{
    return (hour == HORO_ASTERISK) || ((hour > 0) && (hour < ((uint64_t)1 << 24)));
}

static int
isValidDOM(uint64_t dayOfMonth)
{
    return (dayOfMonth == HORO_ASTERISK) || 
        ((dayOfMonth >= 1) && (dayOfMonth < ((uint64_t)1 << 32)));
}

static int
isValidMonth(uint64_t month)
{
    return (month == HORO_ASTERISK) || ((month >= 1) && (month < ((uint64_t)1 << 13)));
}

static int 
isValidDOW(uint64_t dayOfWeek)
{
    return (dayOfWeek == HORO_ASTERISK) || 
        ((dayOfWeek >= 0) && (dayOfWeek < ((uint64_t)1 << 8)));
}

HORO_ERROR
validateCronVals(CronVals const* cronVals)
{
    if(!isValidMinute(cronVals->minute)) return HORO_ERROR_PARSER_MINUTE_RANGE;
    if(!isValidHour(cronVals->hour)) return HORO_ERROR_PARSER_HOUR_RANGE;
    if(!isValidDOM(cronVals->dayOfMonth)) return HORO_ERROR_PARSER_DOM_RANGE;
    if(!isValidMonth(cronVals->month)) return HORO_ERROR_PARSER_MONTH_RANGE;
    if(!isValidDOW(cronVals->dayOfWeek)) return HORO_ERROR_PARSER_DOW_RANGE;

    return HORO_SUCCESS;
}

static int
maxValueFromPosition(FieldPosition_e position)
{
    switch(position) 
    { 
    case HORO_POSITION_MINUTE: 
        return 59; 
        break; 
    case HORO_POSITION_HOUR: 
        return 23; 
        break; 
    case HORO_POSITION_DOM: 
        return 31;
        break; 
    case HORO_POSITION_MONTH: 
        return 12; 
        break; 
    case HORO_POSITION_DOW: 
        return 7;
        break; 
    default: 
        return 0; 
        break; 
    }
}

static void
cronFieldFromRange(Range* range, CronField* cronField)
{
    int i = range->start;
    for(; i <= range->stop; i += range->step)
    {
        cronField->val |= ((uint64_t)1 << i);
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
        val |= ((uint64_t)1 << list->listNums[i]);
    }

    cronField->val = val;
}

static void
cronFieldFromAsteriskStep(int step, CronField* cronField, 
                          FieldPosition_e position)
{
    int i = 0;
    int maxValue = maxValueFromPosition(position);
    for(; i <= maxValue; i += step)
    {
        cronField->val |= ((uint64_t)1 << i);
    }
}

#define RETURN_POSITION_ERROR(position) \
    do { \
    switch(position) \
    { \
    case HORO_POSITION_MINUTE: \
        return HORO_ERROR_PARSER_MINUTE_RANGE; \
    break; \
    case HORO_POSITION_HOUR: \
        return HORO_ERROR_PARSER_HOUR_RANGE; \
    break; \
    case HORO_POSITION_DOM: \
        return HORO_ERROR_PARSER_DOM_RANGE; \
    break; \
    case HORO_POSITION_MONTH: \
        return HORO_ERROR_PARSER_MONTH_RANGE; \
    break; \
    case HORO_POSITION_DOW: \
        return HORO_ERROR_PARSER_DOW_RANGE; \
    break; \
    default: \
        return HORO_ERROR_OUT_OF_RANGE; \
    break; \
    } \
    }while(0)

HORO_ERROR
setCronFieldValues(CronField *cronField, FieldPosition_e position)
{
    int i = 0;

    if(cronField->type & HORO_FIELD_TYPE_ASTERISK)
    {
        if(cronField->typeVal.asteriskStep > 0)
        {
            cronFieldFromAsteriskStep(cronField->typeVal.asteriskStep, 
                                      cronField, position);
        }
        else
        {
            cronField->val = HORO_ASTERISK;
        }
    }
    
    if(cronField->type & HORO_FIELD_TYPE_VALUE)
    {
        if(isValidCronVal(cronField->typeVal.value))
        {
            cronField->val = ((uint64_t)1 << cronField->typeVal.value);
        }
        else
        {
            RETURN_POSITION_ERROR(position);
        }
    }
    
    if(cronField->type & HORO_FIELD_TYPE_RANGE)
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

    if(cronField->type & HORO_FIELD_TYPE_RANGELIST)
    {
        if(cronField->typeVal.rangeList.numRanges > MAX_RANGES_IN_RANGELIST)
        {
            RETURN_POSITION_ERROR(position);
        }

        i = 0;
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

    if(cronField->type & HORO_FIELD_TYPE_LIST)
    {
        i = 0;
        for(; i < cronField->typeVal.list.numCount; i++)
        {
            if(!isValidCronVal(cronField->typeVal.list.listNums[i]))
            {
                RETURN_POSITION_ERROR(position);
            }
        }
        cronFieldFromList(&cronField->typeVal.list, cronField);
    }

    return HORO_SUCCESS;
}
