#include "includes.h"
#include "task_scheduler.h"
#include "soft_timer.h"


/*******************************************
 * VARIABLES
 ******************************************/

/**
 * Ring buffer.
 * Empty if head == tail.
 * Full if tail is one ahead of head
 */
static volatile ring_pos_t sHead = 0;
static volatile ring_pos_t sTail = 0;
// pool for tasks
static task_t volatile task_pool[MAX_CB_TASKS] = {0};
//static cb_task_t cir_tasks;


void task_pool_init(void)
{
    sHead = sTail = 0;
}

void reset_task_pool(void)
{
    BSP_CRITICAL_STATEMENT( task_pool_init() );
}

/**
 * post a task.
 * This function needs to be done in an atomic way.
 */
bool post_task(task_handlerFunc_t aInHandler, uint16_t aInArg)
{
    bool retVal = false;
    ring_pos_t lNextHead;
    bspIState_t s;

    BSP_ENTER_CRITICAL_SECTION(s);
    lNextHead = (sHead + 1) % MAX_CB_TASKS;
    if (lNextHead != sTail) {
        task_pool[sHead].handler = aInHandler;
        task_pool[sHead].arg = aInArg;
        sHead = lNextHead;

        retVal = true;
    }
    BSP_EXIT_CRITICAL_SECTION(s);

    return retVal;
}

void task_scheduler(void)
{
    while (1) {
        /* run tasks that are expired */
        soft_process();

        /* run tasks that are in the queue */
        while ( sHead != sTail ) { /* buffer not empty */
            task_t lTask = task_pool[sTail];
            sTail = (sTail + 1) % MAX_CB_TASKS;

            lTask.handler(lTask.arg);
        }
    }
}


// eof...
