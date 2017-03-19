#include "includes.h"
#include "lpw_cc430.h"
#include "port_cc430.h"


static volatile bool sIsSleep = false;

void lpw_init(void) {
    /* config P2.0 as the trigger to exit lpw */
    init_p20();
    sIsSleep = false;
}

void lpw_enterSleep(void) {
    /* set sIsSleep to true.
     * It must be done before set the register.
     */
    sIsSleep = true;

    __bis_SR_register(LPM4_bits/*LPM3_bits*/);
}

void lpw_exitSleep(void) {
    /* toggle p2.0 to trigger p2.0 interrupt */
    P2OUT ^= BIT0;
    P2OUT ^= BIT0;
    sIsSleep = false;
}

bool lpw_isSleep(void)
{
    return sIsSleep;
}


// eof...
