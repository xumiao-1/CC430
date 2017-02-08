#include "bsp.h"
#include "task_scheduler.h"


// pool for tasks
static task_t volatile task_pool[MAX_CB_TASKS];
static cb_task_t cir_tasks;


void task_pool_init(void) {
    cir_tasks.head = task_pool;
    cir_tasks.tail = cir_tasks.head;
    cir_tasks.pool_used = 0;
}

void reset_task_pool(void) {
	bspIState_t intState;

    BSP_ENTER_CRITICAL_SECTION(intState);
	task_pool_init();
    BSP_EXIT_CRITICAL_SECTION(intState);
}

bool post_task(task_handlerFunc_t handler, uint16_t arg) {
    bool retval;
    bspIState_t intState;

    BSP_ENTER_CRITICAL_SECTION(intState);
    if ( cir_tasks.pool_used < MAX_CB_TASKS ) {
        cir_tasks.head->handler = handler;
        cir_tasks.head->arg = arg;
        cir_tasks.head++;
        cir_tasks.pool_used++;
        retval = true;
        
        /* circular buffer */
        if (cir_tasks.head > &task_pool[MAX_CB_TASKS-1])
            cir_tasks.head = task_pool;
    } else {
        retval = false;
    }
    BSP_EXIT_CRITICAL_SECTION(intState);
    
    return retval;
}

void task_scheduler(void) {
    while ( cir_tasks.pool_used ) {
        task_t task;
        bspIState_t intState;

        BSP_ENTER_CRITICAL_SECTION(intState);
        task = *cir_tasks.tail;
        cir_tasks.tail++;
        cir_tasks.pool_used--;
        
        if (cir_tasks.tail > &task_pool[MAX_CB_TASKS-1]) {
            cir_tasks.tail = task_pool;
        }
        BSP_EXIT_CRITICAL_SECTION(intState);
        
        task.handler(task.arg);
    }
}


// eof...
