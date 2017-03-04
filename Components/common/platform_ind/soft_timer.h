#ifndef _MY_SOFT_TIMER_H_
#define _MY_SOFT_TIMER_H_

#include "bsp.h"
#include "task_scheduler.h"


void soft_initTimers(void);

bool soft_setTimer(
        uint32_t aInInterval,
        task_handlerFunc_t aInHandler,
        uint16_t aInArg,
        bool aInImmediate);

void soft_ISR(uint32_t aInCurTime);


#endif // _MY_SOFT_TIMER_H_
// eof...
