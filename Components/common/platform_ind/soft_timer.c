#include <string.h>
#include "soft_timer.h"
#include "rtc_cc430.h"
#include "utils.h"



#pragma pack (1)

typedef struct {
    uint32_t expireTime;
    task_handlerFunc_t handler;
    uint16_t arg;
    bool isImmediate;
} soft_timer_t;

typedef struct tag_timer_node {
    soft_timer_t _entry;
    struct tag_timer_node *_next;
} timer_node_t;

#pragma pack ()

#define MAX_SOFT_TIMERS (8)

static timer_node_t *sUsedSlots = NULL;
static timer_node_t sSoftTimers[MAX_SOFT_TIMERS] = {0};

void soft_initTimers(void) {
    sUsedSlots = NULL;
    memset(sSoftTimers, 0, sizeof(timer_node_t) * MAX_SOFT_TIMERS);
}

/**
 * aInInterval: ms
 */
int8_t soft_setTimer(uint32_t aInInterval,
        task_handlerFunc_t aInHandler,
        uint16_t aInArg,
        bool aInImmediate)
{
    uint32_t lExpireTime = rtc_getTimeOffset() + aInInterval * TICKS_PER_SECOND / 1000;

    /* find an empty slot from sSoftTimers */
    uint8_t i = 0;
    timer_node_t *lSlot = sSoftTimers;
    while (i < MAX_SOFT_TIMERS) {
        if (NULL == lSlot->_entry.handler) {
            break;
        }
        i++;
        lSlot++;
    }

    if (MAX_SOFT_TIMERS == i) {
        return -1;
    }

    /* fill in the slot */
    lSlot->_entry.expireTime = lExpireTime;
    lSlot->_entry.handler = aInHandler;
    lSlot->_entry.arg = aInArg;
    lSlot->_entry.isImmediate = aInImmediate;

    bspIState_t lState;
    /* find the right position in sUsedSlots */
    BSP_ENTER_CRITICAL_SECTION(lState);
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
    BSP_EXIT_CRITICAL_SECTION(lState);

    /* return the index of the slot in sSoftTimers */
    return lSlot - sSoftTimers;
}

void soft_ISR(uint32_t aInCurTime)
{
    while (sUsedSlots != NULL) {
        if (sUsedSlots->_entry.expireTime > aInCurTime) {
            return;
        }

        /* it's the time to deal with the handler */
        if (sUsedSlots->_entry.isImmediate) {
            sUsedSlots->_entry.handler(sUsedSlots->_entry.arg);
        } else {
            post_task(sUsedSlots->_entry.handler, sUsedSlots->_entry.arg);
        }

        /* clear this timer */
        sUsedSlots->_entry.handler = NULL;
        sUsedSlots = sUsedSlots->_next;
    }

    return;
}
// eof...
