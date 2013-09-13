#include "doorbell.h" /*Use <> so that doorbell.h can
                       *reside in a different directory from the source.*/
#include "Parser.h"
  
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
  
#define RETURN_ILLEGAL_IF(statement) if((statement)) return DBELL_ERROR_ILLEGAL_ARG
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
        list->tail == NULL;
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
    RETURN_ILLEGAL_IF(list == NULL);
    RETURN_IF_NOT_INITIALIZED(list);
    RETURN_ILLEGAL_IF(value == NULL);

    DBELL_ERROR ret = DBELL_SUCCESS;
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
    RETURN_ILLEGAL_IF(list == NULL);
    RETURN_IF_NOT_INITIALIZED(list);
    RETURN_ILLEGAL_IF(list->head == NULL);
    RETURN_ILLEGAL_IF(index < 0);

    DBELL_ERROR ret = DBELL_SUCCESS;
    int count = 0;
    dbellContainerNode_t *currNode = list->head;

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
    RETURN_ILLEGAL_IF(list == NULL);
    RETURN_IF_NOT_INITIALIZED(list);

    DBELL_ERROR err = DBELL_SUCCESS;
    dbellContainerNode_t *currNode = list->head;
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
    if(list == NULL) return;
    if(list->numElements == 0) return;
    if(list->initKey != INITIALIZED_KEY) return;

    dbellContainerNode_t *currNode = list->head;
    dbellContainerNode_t *tmpNode = currNode;
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
    
    uint64_t minute;
    uint32_t hour;
    uint32_t dayOfMonth;
    uint16_t month;
    uint8_t dayOfWeek;

    struct tm lastRuntime;
  
    dbell_actionFunc action;
    void *actionData;
};
typedef struct dbell_entry dbell_entry_t;


struct dbell_clock
{
    dbellList_t entries;

    time_t lastTick;
    uint64_t nextActionID;
};

DBELL_ERROR
dbell_scheduleAction(dbell_clock_t* clock, const char *scheduleString, 
                     dbell_actionFunc action, void *actionData,
                     int* oActionID)
{
    RETURN_ILLEGAL_IF(scheduleString == NULL);
    RETURN_ILLEGAL_IF(action == NULL);
    RETURN_ILLEGAL_IF(oActionID == NULL);

    DBELL_ERROR ret = DBELL_SUCCESS;

    //TODO: Get rid of dynamic memory
    dbell_entry_t *newEntry = (dbell_entry_t*)malloc(sizeof(dbell_entry_t));

    if(NULL == newEntry)
    {
        ret = DBELL_ERROR_NO_MEM;
        goto DONE;
    }

    CronVals cronVals;
    //TODO: I need a return value to check here
    processCronString(scheduleString, &cronVals);

    newEntry->id = clock->nextActionID++;
    newEntry->minute = cronVals.minute;
    newEntry->hour = cronVals.hour;
    newEntry->dayOfMonth = cronVals.dayOfMonth;
    newEntry->month = cronVals.month;
    newEntry->dayOfWeek = cronVals.dayOfWeek;

    memset(&newEntry->lastRuntime, 0, sizeof(newEntry->lastRuntime));
    
    newEntry->action = action;
    newEntry->actionData = actionData;

    //TODO: Error handling
    dbellList_add(&clock->entries, newEntry, sizeof(*newEntry)); 
    
DONE:
    return ret;
}

typedef struct
{
    dbell_clock_t* clock;
    struct tm* timeinfo;
}checkEntryData_t;

static DBELL_ERROR
checkEachEntry(dbell_entry_t* entry, checkEntryData_t* checkEntryData)
{
    struct tm* timeinfo = checkEntryData->timeinfo;
    if((entry->minute & (1 << timeinfo->tm_min)) &&
       (entry->hour & (1 << timeinfo->tm_hour)))
    {
        struct tm* lastRuntime = &entry->lastRuntime;
        if(lastRuntime->tm_min != timeinfo->tm_min ||
           lastRuntime->tm_hour != timeinfo->tm_hour)
        {
            entry->action(entry->actionData);
            entry->lastRuntime = *timeinfo;
        }
    }

    return DBELL_SUCCESS;
}

DBELL_ERROR
dbell_init(dbell_clock_t** oClock)
{
    RETURN_ILLEGAL_IF(oClock == NULL);

    *oClock = malloc(sizeof(dbell_clock_t));
    if(*oClock == NULL)
    {
        return DBELL_ERROR_NO_MEM;
    }
    
    (*oClock)->lastTick=0;
    (*oClock)->nextActionID=0;
    return dbellList_init(&(*oClock)->entries);
}

DBELL_ERROR
dbell_process(dbell_clock_t* clock)
{
    size_t numEntries = 0;
    DBELL_ERROR ret = dbellList_size(&clock->entries, &numEntries);
    if(ret)
    {
        return ret;
    }

    if(numEntries > 0)
    {
        time_t currentTime;
        memset(&currentTime, 0, sizeof(currentTime));

        currentTime = time(NULL);
        struct tm* timeinfo = localtime(&currentTime);

        checkEntryData_t entryCheck;
        entryCheck.clock = clock;
        entryCheck.timeinfo = timeinfo;

        //TODO: check return
        dbellList_forEach((dbellContainer_t*)&clock->entries,
                          (dbellList_forEachFunc)checkEachEntry,
                          &entryCheck);
    }

    return ret;
}
