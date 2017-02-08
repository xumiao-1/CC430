#include "bsp.h"
#include "lpw_cc430.h"
#include "port_cc430.h"


void lpw_init(void) {
    /* config P2.0 as the trigger to exit lpw */
    init_p20();
}

void lpw_enterSleep(void) {
    __bis_SR_register(LPM3_bits);
}

void lpw_exitSleep(void) {
    /* toggle p2.0 to trigger p2.0 interrupt */
    P2OUT ^= BIT0;
    P2OUT ^= BIT0;
}


// eof...
