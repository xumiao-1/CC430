#ifndef _MY_SOFT_TIMER_H_
#define _MY_SOFT_TIMER_H_

#include "bsp.h"
#include "task_scheduler.h"


void soft_initTimers(void);

/**
 * Set a timer.
 * @param aInInterval - How many ms the timer is expired.
 * @param aInHandler - Timer handler.
 * @param aInArg - Argument of the timer handler.
 * @param aInImmediate - True if the handler should be processed in the timer ISR. False otherwise.
 *
 * @return -1 if setTimer fails. A positive integer otherwise.
 */
int8_t soft_setTimer(
        uint32_t aInInterval,
        task_handlerFunc_t aInHandler,
        uint16_t aInArg);

void soft_process(void);


#endif // _MY_SOFT_TIMER_H_
// eof...
