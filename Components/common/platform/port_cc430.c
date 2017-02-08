#include "bsp.h"
#include "port_cc430.h"



void init_p20(void) {
    P2DIR &= ~BIT0;                           // Set P2.0 to input direction
    P2REN |= BIT0;                            // Enable P2.0 internal resistance
    P2OUT |= BIT0;                            // Set P2.0 as pull-Up resistance
    P2IES |= BIT0;                            // P2.0 Hi/Lo edge
    P2IFG &= ~BIT0;                           // P2.0 IFG cleared
    P2IE |= BIT0;                             // P2.0 interrupt enabled
}

/**
 * ISR
 */
#pragma vector=PORT2_VECTOR
__interrupt void PORT2_ISR(void) {
    switch (__even_in_range(P2IV, 16)) {
    case P2IV_NONE: break;

    case P2IV_P2IFG0: {
        /* clear P2.0 IFG */
        P2IFG &= ~BIT0;
        /* exit from LPM3 */
        __bic_SR_register_on_exit(LPM3_bits);
    } break;

    case P2IV_P2IFG1: break;
    case P2IV_P2IFG2: break;
    case P2IV_P2IFG3: break;
    case P2IV_P2IFG4: break;
    case P2IV_P2IFG5: break;
    case P2IV_P2IFG6: break;
    case P2IV_P2IFG7: break;
    default: break;
    }
}


// eof...
