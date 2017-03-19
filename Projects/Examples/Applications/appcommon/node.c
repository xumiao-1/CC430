#include "includes.h"
#include "mydef.h"
#include "utils.h"
#include "node.h"
#include "rtc_cc430.h"
#include "lpw_cc430.h"
#include "uart_intfc_cc430.h"
#include "task_scheduler.h"
#include "soft_timer.h"

#if defined(END_DEVICE)
#include "tmpsnr/tmpsnr.h"

#elif defined(ACCESS_POINT)
#include "wrkstn/wrkstn.h"

#else
#error ERROR: Must define a proper node type.
#endif




volatile int8_t gWkupTimerSlot = -1;
volatile int8_t gUartWkupTimerSlot = -1;
volatile uint32_t gNextWkup = 0;
volatile node_config_t gConfig;


/**************************************************
 * PRIVATE FUNCTIONS
 *************************************************/
void node_init_uart()
{
#if defined(ACCESS_POINT)
    wrkstn_init_uart();
#else
    uart_intfc_init(NULL, NULL); /* use default rx/tx handler */
#endif // defined(ACCESS_POINT)
}

/**************************************************
 * PUBLIC FUNCTIONS
 *************************************************/
bool node_init(void)
{
    /* hardware-related initialization */
    BSP_Init();

#if (!defined BSP_NO_UART)
    /* init uart */
    node_init_uart();
#endif // (!defined BSP_NO_UART)

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
    wrkstn_registerUartMsgProcessor();

#endif // defined

    return true;
}


void node_setPhase(phase_t aInPhase)
{
	gConfig._phase = aInPhase;
}


//eof...
