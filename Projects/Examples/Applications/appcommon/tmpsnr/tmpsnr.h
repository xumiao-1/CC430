//#ifdef TMPSNR_NODE

#ifndef _TMPSNR_H_
#define _TMPSNR_H_

//#include "bsp.h"
//#include "mydef.h"

void tmpsnr_registerMsgProcessor(void);
void tmpsnr_task_mytask(uint16_t);
void tmpsnr_task_readFrame(uint16_t);
void tmpsnr_task_addDevice(uint16_t);
void tmpsnr_task_sleep(uint16_t);
void tmpsnr_tmr_sleepISR(uint16_t);
void tmpsnr_tmr_awakeISR(uint16_t);


#endif //_TMPSNR_H_

//#endif //TMPSNR_NODE
// eof...
