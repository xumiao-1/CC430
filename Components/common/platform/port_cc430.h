#ifndef _PORT_CC430_H_
#define _PORT_CC430_H_


//#pragma vector=PORT2_VECTOR
__interrupt void PORT2_ISR(void);

/**
 * P2.0 used for exit LPW
 */
void init_p20(void);


#endif // _PORT_CC430_H_
// eof...
