#ifndef _MY_RTC_CC430_H_
#define _MY_RTC_CC430_H_

//#include <stdbool.h>
//#include "bsp.h"

#define TICKS_PER_SECOND (1024)


//#pragma vector=RTC_VECTOR
__interrupt void RTC_ISR(void);

void rtc_init(void);
uint32_t rtc_getTimeBase(void);
uint32_t rtc_getTimeOffset(void);
void rtc_setTimeBase(uint32_t aInBase);
void rtc_setTimeOffset(uint32_t aInOffset);


#endif //_MY_RTC_CC430_H_
// eof...
