#ifndef _WRKSTN_H_
#define _WRKSTN_H_


void wrkstn_taskStartup(uint16_t);
void wrkstn_taskRunning(uint16_t);
void wrkstn_taskSleep(uint16_t);
void wrkstn_registerMsgProcessor(void);
void wrkstn_task_readFrame(uint16_t);
void wrkstn_task_addDevice(uint16_t);

void wrkstn_init_uart(void);
void wrkstn_registerUartMsgProcessor(void);
void wrkstn_taskReadUartMsg(uint16_t);


#endif //_WRKSTN_H_

// eof...
