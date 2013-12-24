/**
 * December 19, 2013
 * The author disclaims copyright to this source code.
 */

#include "horo.h" /*Use <> so that horo.h can
                       *reside in a different directory from the source.*/
#include "Parser.h"
  
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
  
#define RETURN_ILLEGAL_IF(statement) if((statement)) return HORO_ERROR_ILLEGAL_ARG

#define VALIDATE_RANGE_OR_RETURN(var, min, max)  \
    if(((var) < (min)) || ((var) > (max))) return HORO_ERROR_OUT_OF_RANGE

#define IS_INITIALIZED(container) ((container)->initKey == INITIALIZED_KEY)

#define RETURN_IF_NOT_INITIALIZED(container) if(!IS_INITIALIZED((container))) return HORO_ERROR_NOT_INITIALIZED


typedef enum
{
    CONTAINER_TYPE_LIST,
    CONTAINER_TYPE_MAP
}horoContainerType_e;

typedef struct horoContainerNode
{
    void *data;
    struct horoContainerNode *next;
    struct horoContainerNode *prev;
}horoContainerNode_t;

#define INITIALIZED_KEY 0x1217
typedef struct horoContainer
{
    horoContainerType_e type;
    int initKey;
    horoContainerNode_t *head;
    horoContainerNode_t *tail;
    size_t numElements;
}horoContainer_t;

typedef HORO_ERROR (*horoList_forEachFunc)(void *data, void *userp);
typedef horoContainer_t horoList_t;

static HORO_ERROR
horoList_init(horoContainer_t *listToInit)
{
    RETURN_ILLEGAL_IF(listToInit == NULL);
    listToInit->type = CONTAINER_TYPE_LIST;
    listToInit->initKey = INITIALIZED_KEY;
    listToInit->head = NULL;
    listToInit->tail = NULL;
    listToInit->numElements = 0;
    return HORO_SUCCESS;
}

static HORO_ERROR
horoList_isInitialized(const horoList_t* list)
{
    RETURN_ILLEGAL_IF(list == NULL);
    return IS_INITIALIZED(list)? HORO_SUCCESS : HORO_ERROR_NOT_INITIALIZED;
}

static void
initListNode(horoContainerNode_t *node)
{
    if(node == NULL) return;
    node->data = NULL;
    node->next = NULL;
    node->prev = NULL;
}

static void
freeListNode(horoContainerNode_t *node)
{
    if(node != NULL)
    {
        if(node->data != NULL) free(node->data);
        free(node);
    }
}

static HORO_ERROR
addToEmptyList(horoContainer_t *list, const void *value, size_t size)
{
    HORO_ERROR ret = HORO_SUCCESS;

    list->head = (horoContainerNode_t *)malloc(sizeof(horoContainerNode_t));
    if(list->head == NULL)
    {
        ret = HORO_ERROR_NO_MEM;
        goto ERR;
    }
    initListNode(list->head);

    list->head->data = malloc(size);
    if(list->head->data == NULL)
    {
        ret = HORO_ERROR_NO_MEM;
        goto ERR;
    }
    memcpy(list->head->data, value, size);

    list->head->next = NULL;
    list->head->prev = NULL;

    list->tail = list->head;

    ++list->numElements;
    if(0)
    {
ERR:
        freeListNode(list->head);
        list->head = NULL;
        list->tail = NULL;
    }

    return ret;
}

static HORO_ERROR
addToEndOfList(horoContainer_t *list, const void *value, size_t size)
{
    HORO_ERROR ret = HORO_SUCCESS;

    horoContainerNode_t *newNode = (horoContainerNode_t*)malloc(sizeof(horoContainerNode_t));
    if(newNode == NULL)
    {
        ret = HORO_ERROR_NO_MEM;
        goto ERR;
    }
    initListNode(newNode);

    newNode->data = malloc(size);
    if(newNode->data == NULL)
    {
        ret = HORO_ERROR_NO_MEM;
        goto ERR;
    }
    memcpy(newNode->data, value, size);

    newNode->prev = list->tail;
    list->tail = newNode;
    if(list->tail->prev)
    {
        list->tail->prev->next = list->tail;
    }

    ++list->numElements;
    if(0)
    {
ERR:
        freeListNode(newNode);
    }
    return ret;
}

static HORO_ERROR
horoList_add(horoContainer_t *list, const void *value, size_t size)
{
    HORO_ERROR ret = HORO_SUCCESS;

    RETURN_ILLEGAL_IF(list == NULL);
    RETURN_IF_NOT_INITIALIZED(list);
    RETURN_ILLEGAL_IF(value == NULL);

    if(list == NULL)
    {
        ret = HORO_ERROR_ILLEGAL_ARG;
    }

    if(0 == list->numElements)
    {
        ret = addToEmptyList(list, value, size);
    }
    else if(list->numElements > 0)
    {
        ret = addToEndOfList(list, value, size);
    }

END:

    return ret;
}

static HORO_ERROR
horoList_size(horoContainer_t *list, size_t *sizeReturn)
{
    RETURN_ILLEGAL_IF(list == NULL);
    RETURN_IF_NOT_INITIALIZED(list);
    RETURN_ILLEGAL_IF(sizeReturn == NULL);
    *sizeReturn = list->numElements;

    return HORO_SUCCESS;
}

static HORO_ERROR
horoList_remove(horoContainer_t *list, int index)
{
    HORO_ERROR ret = HORO_SUCCESS;
    int count = 0;
    horoContainerNode_t *currNode = list->head;


    RETURN_ILLEGAL_IF(list == NULL);
    RETURN_IF_NOT_INITIALIZED(list);
    RETURN_ILLEGAL_IF(list->head == NULL);
    RETURN_ILLEGAL_IF(index < 0);

    if(index > (list->numElements - 1))
    {
        ret = HORO_ERROR_ILLEGAL_ARG;
        goto DONE;
    }

    while((count != index))
    {
        if(currNode != NULL)
        {
            currNode = currNode->next;
        }
        count++;
    }

    if(currNode == NULL)
    {
        ret = HORO_ERROR_CORRUPT;
        goto DONE;
    }

    if(currNode == list->head)
    {
        list->head = currNode->next;
    }
    if(currNode == list->tail)
    {
        list->tail = currNode->prev;
    }
    if(currNode->prev != NULL)
    {
        currNode->prev->next = currNode->next;
    }
    if(currNode->next != NULL)
    {
        currNode->next->prev = currNode->prev;
    }

    freeListNode(currNode);
    --list->numElements;
DONE:
    return ret;
}

static HORO_ERROR
horoList_forEach(const horoContainer_t *list,
                  horoList_forEachFunc callBack,
                  void *userp)
{

    HORO_ERROR err = HORO_SUCCESS;
    horoContainerNode_t *currNode = list->head;

    RETURN_ILLEGAL_IF(list == NULL);
    RETURN_IF_NOT_INITIALIZED(list);

    while(currNode != NULL)
    {
        err = (*callBack)(currNode->data, userp);
        if(err) break;

        currNode = currNode->next;
    }

    return err;
}

static void
horoList_destroyNodes(horoContainer_t *list)
{
    horoContainerNode_t *currNode = list->head;
    horoContainerNode_t *tmpNode = currNode;

    if(list == NULL) return;
    if(list->numElements == 0) return;
    if(list->initKey != INITIALIZED_KEY) return;

    while(currNode != NULL)
    {
        currNode = currNode->next;
        freeListNode(tmpNode);
        tmpNode = currNode;
    }
    horoList_init(list);
}

struct horo_entry
{
    uint64_t id;
    
    CronVals scheduleVals;    

    horo_time_t lastRuntime;
  
    horo_actionFunc action;
    void *actionData;
};
typedef struct horo_entry horo_entry_t;


struct horo_clock
{
    horoList_t entries;

    horo_time_t lastTick;
    uint64_t nextActionID;
};

HORO_ERROR
horo_scheduleAction(horo_clock_t* clock, const char *scheduleString, 
                     horo_actionFunc action, void *actionData,
                     int* oActionID)
{
    HORO_ERROR err = HORO_SUCCESS;
    horo_entry_t newEntry;
    CronVals cronVals;

    RETURN_ILLEGAL_IF(scheduleString == NULL);
    RETURN_ILLEGAL_IF(action == NULL);
    RETURN_ILLEGAL_IF(oActionID == NULL);

    err = processCronString(scheduleString, &cronVals);
    if(err) goto DONE;
        
    newEntry.id = clock->nextActionID++;
    newEntry.scheduleVals.minute = cronVals.minute;
    newEntry.scheduleVals.hour = cronVals.hour;
    newEntry.scheduleVals.dayOfMonth = cronVals.dayOfMonth;
    newEntry.scheduleVals.month = cronVals.month;
    newEntry.scheduleVals.dayOfWeek = cronVals.dayOfWeek;

    memset(&newEntry.lastRuntime, 0, sizeof(newEntry.lastRuntime));
    
    newEntry.action = action;
    newEntry.actionData = actionData;

    err = horoList_add(&clock->entries, &newEntry, sizeof(newEntry)); 
    if(err) goto DONE;

    *oActionID = newEntry.id;
DONE:
    return err;
}

typedef struct
{
    horo_clock_t* clock;
    horo_time_t const* userTime;
}checkEntryData_t;

static int
checkDOMWithDOW(uint64_t dayOfMonth, uint64_t dayOfWeek, 
                horo_time_t const* timeVals)
{
    if((dayOfMonth == HORO_ASTERISK) &&
       (dayOfWeek == HORO_ASTERISK))
    {
        return 1;
    }

    if((dayOfMonth != HORO_ASTERISK) &&
       (dayOfWeek != HORO_ASTERISK))
    {
        if((dayOfWeek & ((uint64_t)1 << timeVals->dayOfWeek)) &&
           (dayOfMonth & ((uint64_t)1 << timeVals->dayOfMonth)))
        {
            return 1;
        }
    }

    if(dayOfMonth == HORO_ASTERISK)
    {
        if(dayOfWeek & ((uint64_t)1 << timeVals->dayOfWeek))
        {
            return 1;
        }
    }

    if(dayOfWeek == HORO_ASTERISK)
    {
        if(dayOfMonth & ((uint64_t)1 << timeVals->dayOfMonth))
        {
            return 1;
        }
    }

    return 0;
}

static HORO_ERROR
checkEachEntry(horo_entry_t* entry, checkEntryData_t* checkEntryData)
{
    horo_time_t const* lastRuntime = NULL;
    horo_time_t const* userTime = checkEntryData->userTime;
    
    if((entry->scheduleVals.minute & ((uint64_t)1 << userTime->minute)) &&
       (entry->scheduleVals.hour & ((uint64_t)1 << userTime->hour)) && 
       (entry->scheduleVals.month & ((uint64_t)1 << userTime->month)) &&
       checkDOMWithDOW(entry->scheduleVals.dayOfMonth,
                       entry->scheduleVals.dayOfWeek,
                       userTime))
    {
        lastRuntime = &entry->lastRuntime;
        if((lastRuntime->minute != userTime->minute) ||
           (lastRuntime->hour != userTime->hour) ||
           (lastRuntime->dayOfMonth != userTime->dayOfMonth) ||
           (lastRuntime->month != userTime->month) ||
           (lastRuntime->dayOfWeek != userTime->dayOfWeek))
        {
            entry->action(entry->actionData);
            entry->lastRuntime = *userTime;
        }
    }

    return HORO_SUCCESS;
}

HORO_ERROR
horo_init(horo_clock_t** oClock)
{
    RETURN_ILLEGAL_IF(oClock == NULL);

    //TODO: Add callback for memory allocation
    *oClock = malloc(sizeof(horo_clock_t));
    if(*oClock == NULL)
    {
        return HORO_ERROR_NO_MEM;
    }
    
    memset(&(*oClock)->lastTick, 0, sizeof((*oClock)->lastTick));
    (*oClock)->nextActionID=0;
    return horoList_init(&(*oClock)->entries);
}

HORO_ERROR
horo_process(horo_clock_t* clock, horo_time_t const* userTime)
{

    size_t numEntries = 0;
    HORO_ERROR ret = HORO_SUCCESS;

    VALIDATE_RANGE_OR_RETURN(userTime->minute, 0, 59);
    VALIDATE_RANGE_OR_RETURN(userTime->hour, 0, 23);
    VALIDATE_RANGE_OR_RETURN(userTime->dayOfMonth, 1, 31);
    VALIDATE_RANGE_OR_RETURN(userTime->month, 1, 12);
    VALIDATE_RANGE_OR_RETURN(userTime->dayOfWeek, 0, 7);

    ret = horoList_size(&clock->entries, &numEntries);
    if(ret)
    {
        return ret;
    }

    if(numEntries > 0)
    {
        checkEntryData_t entryCheck;
        entryCheck.clock = clock;
        entryCheck.userTime = userTime;

        //TODO: check return
        horoList_forEach((horoContainer_t*)&clock->entries,
                          (horoList_forEachFunc)checkEachEntry,
                          &entryCheck);

        clock->lastTick = *userTime;
    }

    return ret;
}

HORO_ERROR
horo_unscheduleAction(horo_clock_t* clock, int actionID)
{
    int found = 0;
    int index = 0;
    HORO_ERROR ret = HORO_ERROR_UNKNOWN_ACTION;
    horoContainerNode_t *node = NULL;

    RETURN_ILLEGAL_IF(clock == NULL);

    node = clock->entries.head;
    while(node != NULL)
    {
        horo_entry_t* entry = (horo_entry_t*)node->data;
        if(entry->id == actionID)
        {
            found = 1;
            break;
        }
        index++;
        node = node->next;
    }
    
    if(found)
    {
        ret = horoList_remove(&clock->entries, index);
    }

    return ret;
}

HORO_ERROR
horo_actionCount(horo_clock_t* clock, int* oActionCount)
{
    RETURN_ILLEGAL_IF(clock == NULL);
    RETURN_ILLEGAL_IF(oActionCount == NULL);

    *oActionCount = clock->entries.numElements;
    return HORO_SUCCESS;
}

HORO_ERROR
horo_destroy(horo_clock_t* clock)
{
    RETURN_ILLEGAL_IF(clock == NULL);
    horoList_destroyNodes(&clock->entries);
    free(clock);

    return HORO_SUCCESS;
}
