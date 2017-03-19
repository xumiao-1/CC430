#include "includes.h"
#include "soft_timer.h"
#include "rtc_cc430.h"
#include "utils.h"



#pragma pack (1)

typedef struct {
    uint32_t expireTime;
    task_handlerFunc_t handler;
    uint16_t arg;
    //bool isImmediate;
} soft_timer_t;

typedef struct tag_timer_node {
    soft_timer_t _entry;
    struct tag_timer_node *_next;
} timer_node_t;

#pragma pack ()

#define MAX_SOFT_TIMERS (8)

static  timer_node_t * sFreeSlots = NULL;
static  timer_node_t * sUsedSlots = NULL;
static  timer_node_t sSoftTimers[MAX_SOFT_TIMERS] = {0};

void soft_initTimers(void)
{
    uint8_t i = 0;
    memset(sSoftTimers, 0, sizeof(timer_node_t) * MAX_SOFT_TIMERS);

    for ( ; i < (MAX_SOFT_TIMERS-1); i++ ) {
        sSoftTimers[i]._next = sSoftTimers + (i + 1);
    }

    sFreeSlots = sSoftTimers;
    sUsedSlots = NULL;
}

/**
 * aInInterval: ms
 */
int8_t soft_setTimer(uint32_t aInInterval,
        task_handlerFunc_t aInHandler,
        uint16_t aInArg)
{
    if ( NULL == sFreeSlots ) {
        return -1;
    }

    uint32_t lExpireTime = aInInterval * TICKS_PER_SECOND / 1000;
    BSP_CRITICAL_STATEMENT( lExpireTime += rtc_getTimeOffset() );

    /* get an empty slot from sFreeSlots */
    timer_node_t *lSlot = sFreeSlots;
    sFreeSlots = sFreeSlots->_next;

    /* fill in the slot */
    lSlot->_entry.expireTime = lExpireTime;
    lSlot->_entry.handler = aInHandler;
    lSlot->_entry.arg = aInArg;

    //bspIState_t lState;
    /* find the right position in sUsedSlots */
    //BSP_ENTER_CRITICAL_SECTION(lState);
    timer_node_t *lPreCursor = NULL;
    timer_node_t *lCursor = sUsedSlots;
    while (lCursor != NULL) {
        if (lCursor->_entry.expireTime > lExpireTime) {
            break;
        }

        lPreCursor = lCursor;
        lCursor = lCursor->_next;
    }

    if (lPreCursor != NULL) {
        lPreCursor->_next = lSlot;
        lSlot->_next = lCursor;
    } else {
        sUsedSlots = lSlot;
        lSlot->_next = lCursor;
    }
    //BSP_EXIT_CRITICAL_SECTION(lState);

    /* return the index of the slot in sSoftTimers */
    return lSlot - sSoftTimers;
}

void soft_cancelTimer(int8_t aInSlotId)
{
    if (aInSlotId < 0) {
        log(LOG_WARNING, "soft_cancelTimer: slot id is negative");
        return;
    }

    /* find the slot and free it */
    timer_node_t *lPreSlot = NULL;
    timer_node_t *lSlot = sUsedSlots;
    while (lSlot != NULL) {
        if (lSlot == &sSoftTimers[aInSlotId]) {
            break;
        }

        lPreSlot = lSlot;
        lSlot = lSlot->_next;
    }

    if (!lSlot) {
        log(LOG_WARNING, "soft_cancelTimer: the slot is already freed");
        return;
    }

    /* slot found, now free it */
    if (NULL != lPreSlot) {
        lPreSlot->_next = lSlot->_next;
    } else {
        sUsedSlots = lSlot->_next;
    }

    lSlot->_entry.handler = NULL;
    lSlot->_next = sFreeSlots;
    sFreeSlots = lSlot;
    return;
}

void soft_process(void)
{
    uint32_t lTimeStamp = 0;

    while (sUsedSlots != NULL) {
        BSP_CRITICAL_STATEMENT( lTimeStamp = rtc_getTimeOffset() );

        if (lTimeStamp < sUsedSlots->_entry.expireTime) {
            return;
        }

        /* it's the time to deal with the handler */
        if (sUsedSlots->_entry.handler) {
            sUsedSlots->_entry.handler(sUsedSlots->_entry.arg);
        }

        /* free the slot */
        timer_node_t *lSlot = sUsedSlots;
        sUsedSlots = sUsedSlots->_next;
        lSlot->_entry.handler = NULL;
        lSlot->_next = sFreeSlots;
        sFreeSlots = lSlot;
    }
}
// eof...
