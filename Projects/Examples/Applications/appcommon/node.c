#include <stdbool.h>
#include "bsp.h"
#include "mydef.h"
#include "node.h"
#include "rtc_cc430.h"
#include "lpw_cc430.h"
#include "task_scheduler.h"
#include "soft_timer.h"

#if defined(END_DEVICE)
#include "tmpsnr/tmpsnr.h"

#elif defined(ACCESS_POINT)
#include "wrkstn/wrkstn.h"

#else
#error ERROR: Must define a proper node type.
#endif


/**************************************************
 * PUBLIC FUNCTIONS
 *************************************************/
bool node_init(void)
{
    /* hardware-related initialization */
    BSP_Init();
    rtc_init();
    lpw_init();

    /* software-related initialization */
    task_pool_init();
    soft_initTimers();

    /* register RF msg processors */
#if defined(END_DEVICE)
    tmpsnr_registerMsgProcessor();

#elif defined(ACCESS_POINT)
    wrkstn_registerMsgProcessor();

#endif // defined

    return true;
}

void node_sleepISR(uint16_t arg)
{
    /* actions before going to sleep */
#if defined(ACCESS_POINT)
    post_task(wrkstn_taskSleep, arg);
#elif defined(END_DEVICE)
    post_task(tmpsnr_taskSleep, arg);
#endif // defined
}

void node_awakeISR(uint16_t arg)
{
    /* exit low power mode */
    lpw_exitSleep();

    /* update next wakeup schedule */
    if (gNextWkup) {
        gNextWkup += AWAKE_INTERVAL;
    } else {
        gNextWkup =  rtc_getTimeOffset() + AWAKE_INTERVAL;
    }

    /* actions after wakeup */
#if defined(ACCESS_POINT)
    post_task(wrkstn_taskMain, arg);
#elif defined(END_DEVICE)
    post_task(tmpsnr_taskMain, arg);
#endif // defined
}

//eof...
