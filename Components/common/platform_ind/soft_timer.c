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

#pragma pack ()

#define MAX_SOFT_TIMERS (8)

static volatile uint8_t sUsedTimers;
static soft_timer_t sSoftTimers[MAX_SOFT_TIMERS];

void soft_initTimers(void) {
    sUsedTimers = 0;
    memset(sSoftTimers, 0, sizeof(soft_timer_t) * MAX_SOFT_TIMERS);
}

/**
 * aInInterval: ms
 */
bool soft_setTimer(uint32_t aInInterval,
        task_handlerFunc_t aInHandler,
        uint16_t aInArg,
        bool aInImmediate)
{
    if (sUsedTimers < MAX_SOFT_TIMERS) {
        uint8_t i;
        for (i = 0; i < MAX_SOFT_TIMERS; i++) {
            if (NULL == sSoftTimers[i].handler) {
                bspIState_t lState;

                sSoftTimers[i].expireTime = rtc_getTimeOffset() + aInInterval * TICKS_PER_SECOND / 1000;
                sSoftTimers[i].handler = aInHandler;
                sSoftTimers[i].arg = aInArg;
                sSoftTimers[i].isImmediate = aInImmediate;

                BSP_ENTER_CRITICAL_SECTION(lState);
                sUsedTimers++;
                BSP_EXIT_CRITICAL_SECTION(lState);

                return true;
            }
        }
    }

    return false;
}

void soft_ISR(uint32_t aInCurTime) {
    uint8_t i;
    if (sUsedTimers > 0) {
        for (i = 0; i< MAX_SOFT_TIMERS; i++) {
            if (NULL != sSoftTimers[i].handler
                    && sSoftTimers[i].expireTime <= aInCurTime) {

                /* timer handler */
                if (sSoftTimers[i].isImmediate) {
                    sSoftTimers[i].handler(sSoftTimers[i].arg);
                } else {
                    post_task(sSoftTimers[i].handler, sSoftTimers[i].arg);
                }

                /* clear this timer */
                sSoftTimers[i].handler = NULL;
                sUsedTimers--;
            }
        }
    }
}
// eof...
