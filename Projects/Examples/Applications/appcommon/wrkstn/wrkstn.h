//#ifdef WRKSTN_NODE

#ifndef _WRKSTN_H_
#define _WRKSTN_H_


void wrkstn_taskStartup(uint16_t);
void wrkstn_taskMain(uint16_t);
void wrkstn_taskSleep(uint16_t);
void wrkstn_registerMsgProcessor(void);
void wrkstn_task_readFrame(uint16_t);
void wrkstn_task_addDevice(uint16_t);


#endif //_WRKSTN_H_

//#endif //WRKSTN_NODE
// eof...
