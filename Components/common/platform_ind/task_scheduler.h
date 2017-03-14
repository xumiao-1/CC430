#ifndef _APP_TASK_H_
#define _APP_TASK_H_


#include <stdbool.h>


#define MAX_CB_TASKS (16)


/** task structure definition */
typedef uint16_t ring_pos_t;
typedef void (*task_handlerFunc_t)(uint16_t arg);

#pragma pack (1)

typedef struct {
    task_handlerFunc_t handler;
    uint16_t arg;
} task_t;

//typedef struct {
//    task_t volatile * head;
//    task_t volatile * tail;
//    uint8_t volatile pool_used;
////    uint8_t volatile pool_free;
//} cb_task_t;

#pragma pack ()



void task_pool_init(void);
bool post_task(task_handlerFunc_t, uint16_t);
void task_scheduler(void);
void reset_task_pool(void);


#endif //_APP_TASK_H_
// eof...
