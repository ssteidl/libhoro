/**
 * December 19, 2013
 * The author disclaims copyright to this source code.
 */

#include "doorbell.h" /*Use <> so that doorbell.h can
                       *reside in a different directory from the source.*/
#include "Parser.h"
  
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
  
#define RETURN_ILLEGAL_IF(statement) if((statement)) return DBELL_ERROR_ILLEGAL_ARG

#define VALIDATE_RANGE_OR_RETURN(var, min, max)  \
    if(((var) < (min)) || ((var) > (max))) return DBELL_ERROR_OUT_OF_RANGE

#define IS_INITIALIZED(container) ((container)->initKey == INITIALIZED_KEY)

#define RETURN_IF_NOT_INITIALIZED(container) if(!IS_INITIALIZED((container))) return DBELL_ERROR_NOT_INITIALIZED


typedef enum
{
    CONTAINER_TYPE_LIST,
    CONTAINER_TYPE_MAP
}dbellContainerType_e;

typedef struct dbellContainerNode
{
    void *data;
    struct dbellContainerNode *next;
    struct dbellContainerNode *prev;
}dbellContainerNode_t;

#define INITIALIZED_KEY 0x1217
typedef struct dbellContainer
{
    dbellContainerType_e type;
    int initKey;
    dbellContainerNode_t *head;
    dbellContainerNode_t *tail;
    size_t numElements;
}dbellContainer_t;

typedef DBELL_ERROR (*dbellList_forEachFunc)(void *data, void *userp);
typedef dbellContainer_t dbellList_t;

static DBELL_ERROR
dbellList_init(dbellContainer_t *listToInit)
{
    RETURN_ILLEGAL_IF(listToInit == NULL);
    listToInit->type = CONTAINER_TYPE_LIST;
    listToInit->initKey = INITIALIZED_KEY;
    listToInit->head = NULL;
    listToInit->tail = NULL;
    listToInit->numElements = 0;
    return DBELL_SUCCESS;
}

static DBELL_ERROR
dbellList_isInitialized(const dbellList_t* list)
{
    RETURN_ILLEGAL_IF(list == NULL);
    return IS_INITIALIZED(list)? DBELL_SUCCESS : DBELL_ERROR_NOT_INITIALIZED;
}

static void
initListNode(dbellContainerNode_t *node)
{
    if(node == NULL) return;
    node->data = NULL;
    node->next = NULL;
    node->prev = NULL;
}

static void
freeListNode(dbellContainerNode_t *node)
{
    if(node != NULL)
    {
        if(node->data != NULL) free(node->data);
        free(node);
    }
}

static DBELL_ERROR
addToEmptyList(dbellContainer_t *list, const void *value, size_t size)
{
    DBELL_ERROR ret = DBELL_SUCCESS;

    list->head = (dbellContainerNode_t *)malloc(sizeof(dbellContainerNode_t));
    if(list->head == NULL)
    {
        ret = DBELL_ERROR_NO_MEM;
        goto ERR;
    }
    initListNode(list->head);

    list->head->data = malloc(size);
    if(list->head->data == NULL)
    {
        ret = DBELL_ERROR_NO_MEM;
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

static DBELL_ERROR
addToEndOfList(dbellContainer_t *list, const void *value, size_t size)
{
    DBELL_ERROR ret = DBELL_SUCCESS;

    dbellContainerNode_t *newNode = (dbellContainerNode_t*)malloc(sizeof(dbellContainerNode_t));
    if(newNode == NULL)
    {
        ret = DBELL_ERROR_NO_MEM;
        goto ERR;
    }
    initListNode(newNode);

    newNode->data = malloc(size);
    if(newNode->data == NULL)
    {
        ret = DBELL_ERROR_NO_MEM;
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

static DBELL_ERROR
dbellList_add(dbellContainer_t *list, const void *value, size_t size)
{
    DBELL_ERROR ret = DBELL_SUCCESS;

    RETURN_ILLEGAL_IF(list == NULL);
    RETURN_IF_NOT_INITIALIZED(list);
    RETURN_ILLEGAL_IF(value == NULL);

    if(list == NULL)
    {
        ret = DBELL_ERROR_ILLEGAL_ARG;
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

static DBELL_ERROR
dbellList_size(dbellContainer_t *list, size_t *sizeReturn)
{
    RETURN_ILLEGAL_IF(list == NULL);
    RETURN_IF_NOT_INITIALIZED(list);
    RETURN_ILLEGAL_IF(sizeReturn == NULL);
    *sizeReturn = list->numElements;

    return DBELL_SUCCESS;
}

static DBELL_ERROR
dbellList_remove(dbellContainer_t *list, int index)
{
    DBELL_ERROR ret = DBELL_SUCCESS;
    int count = 0;
    dbellContainerNode_t *currNode = list->head;


    RETURN_ILLEGAL_IF(list == NULL);
    RETURN_IF_NOT_INITIALIZED(list);
    RETURN_ILLEGAL_IF(list->head == NULL);
    RETURN_ILLEGAL_IF(index < 0);

    if(index > (list->numElements - 1))
    {
        ret = DBELL_ERROR_ILLEGAL_ARG;
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
        ret = DBELL_ERROR_CORRUPT;
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

static DBELL_ERROR
dbellList_forEach(const dbellContainer_t *list,
                  dbellList_forEachFunc callBack,
                  void *userp)
{

    DBELL_ERROR err = DBELL_SUCCESS;
    dbellContainerNode_t *currNode = list->head;

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
dbellList_destroyNodes(dbellContainer_t *list)
{
    dbellContainerNode_t *currNode = list->head;
    dbellContainerNode_t *tmpNode = currNode;

    if(list == NULL) return;
    if(list->numElements == 0) return;
    if(list->initKey != INITIALIZED_KEY) return;

    while(currNode != NULL)
    {
        currNode = currNode->next;
        freeListNode(tmpNode);
        tmpNode = currNode;
    }
    dbellList_init(list);
}

struct dbell_entry
{
    uint64_t id;
    
    CronVals scheduleVals;    

    dbell_time_t lastRuntime;
  
    dbell_actionFunc action;
    void *actionData;
};
typedef struct dbell_entry dbell_entry_t;


struct dbell_clock
{
    dbellList_t entries;

    dbell_time_t lastTick;
    uint64_t nextActionID;
};

DBELL_ERROR
dbell_scheduleAction(dbell_clock_t* clock, const char *scheduleString, 
                     dbell_actionFunc action, void *actionData,
                     int* oActionID)
{
    DBELL_ERROR err = DBELL_SUCCESS;
    dbell_entry_t newEntry;
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

    err = dbellList_add(&clock->entries, &newEntry, sizeof(newEntry)); 
    if(err) goto DONE;

    *oActionID = newEntry.id;
DONE:
    return err;
}

typedef struct
{
    dbell_clock_t* clock;
    dbell_time_t const* userTime;
}checkEntryData_t;

static int
checkDOMWithDOW(uint64_t dayOfMonth, uint64_t dayOfWeek, 
                dbell_time_t const* timeVals)
{
    if((dayOfMonth == DBELL_ASTERISK) &&
       (dayOfWeek == DBELL_ASTERISK))
    {
        return 1;
    }

    if((dayOfMonth != DBELL_ASTERISK) &&
       (dayOfWeek != DBELL_ASTERISK))
    {
        if((dayOfWeek & ((uint64_t)1 << timeVals->dayOfWeek)) &&
           (dayOfMonth & ((uint64_t)1 << timeVals->dayOfMonth)))
        {
            return 1;
        }
    }

    if(dayOfMonth == DBELL_ASTERISK)
    {
        if(dayOfWeek & ((uint64_t)1 << timeVals->dayOfWeek))
        {
            return 1;
        }
    }

    if(dayOfWeek == DBELL_ASTERISK)
    {
        if(dayOfMonth & ((uint64_t)1 << timeVals->dayOfMonth))
        {
            return 1;
        }
    }

    return 0;
}

static DBELL_ERROR
checkEachEntry(dbell_entry_t* entry, checkEntryData_t* checkEntryData)
{
    dbell_time_t const* lastRuntime = NULL;
    dbell_time_t const* userTime = checkEntryData->userTime;
    
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

    return DBELL_SUCCESS;
}

DBELL_ERROR
dbell_init(dbell_clock_t** oClock)
{
    RETURN_ILLEGAL_IF(oClock == NULL);

    //TODO: Add callback for memory allocation
    *oClock = malloc(sizeof(dbell_clock_t));
    if(*oClock == NULL)
    {
        return DBELL_ERROR_NO_MEM;
    }
    
    memset(&(*oClock)->lastTick, 0, sizeof((*oClock)->lastTick));
    (*oClock)->nextActionID=0;
    return dbellList_init(&(*oClock)->entries);
}

DBELL_ERROR
dbell_process(dbell_clock_t* clock, dbell_time_t const* userTime)
{

    size_t numEntries = 0;
    DBELL_ERROR ret = DBELL_SUCCESS;

    VALIDATE_RANGE_OR_RETURN(userTime->minute, 0, 59);
    VALIDATE_RANGE_OR_RETURN(userTime->hour, 0, 23);
    VALIDATE_RANGE_OR_RETURN(userTime->dayOfMonth, 1, 31);
    VALIDATE_RANGE_OR_RETURN(userTime->month, 1, 12);
    VALIDATE_RANGE_OR_RETURN(userTime->dayOfWeek, 0, 7);

    ret = dbellList_size(&clock->entries, &numEntries);
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
        dbellList_forEach((dbellContainer_t*)&clock->entries,
                          (dbellList_forEachFunc)checkEachEntry,
                          &entryCheck);

        clock->lastTick = *userTime;
    }

    return ret;
}

DBELL_ERROR
dbell_unscheduleAction(dbell_clock_t* clock, int actionID)
{
    int found = 0;
    int index = 0;
    DBELL_ERROR ret = DBELL_ERROR_UNKNOWN_ACTION;
    dbellContainerNode_t *node = NULL;

    RETURN_ILLEGAL_IF(clock == NULL);

    node = clock->entries.head;
    while(node != NULL)
    {
        dbell_entry_t* entry = (dbell_entry_t*)node->data;
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
        ret = dbellList_remove(&clock->entries, index);
    }

    return ret;
}

DBELL_ERROR
dbell_actionCount(dbell_clock_t* clock, int* oActionCount)
{
    RETURN_ILLEGAL_IF(clock == NULL);
    RETURN_ILLEGAL_IF(oActionCount == NULL);

    *oActionCount = clock->entries.numElements;
    return DBELL_SUCCESS;
}

DBELL_ERROR
dbell_destroy(dbell_clock_t* clock)
{
    RETURN_ILLEGAL_IF(clock == NULL);
    dbellList_destroyNodes(&clock->entries);
    free(clock);

    return DBELL_SUCCESS;
}
