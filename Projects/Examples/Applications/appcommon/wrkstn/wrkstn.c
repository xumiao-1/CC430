#include "includes.h"
#include "cc430f5137.h"
#include "mrfi.h"
#include "bsp_leds.h"
#include "bsp_buttons.h"
#include "nwk_types.h"
#include "nwk_api.h"
#include "nwk_frame.h"
#include "nwk.h"
#include "rtc_cc430.h"
#include "lpw_cc430.h"
#include "uart_intfc_cc430.h"
#include "soft_timer.h"

#include "utils.h"
#include "mydef.h"
#include "wrkstn.h"
#include "node.h"


typedef void (*msg_processor_t)(pkt_app_t*);
typedef void (*uart_msg_processor_t)(uart_pkt_t*);

static linkID_t sLID[NUM_CONNECTIONS] = { 0 };
static uint8_t sNumCurrentPeers = 0;
static msg_processor_t sMsgProcessor[RF_CMD_END] = {0};
static uart_msg_processor_t sUartMsgProcessor[UART_CMD_END] = {0};

void wrkstn_processSyncReq(pkt_app_t *);
void wrkstn_processSyncRep(pkt_app_t *);
void wrkstn_processTmpData(pkt_app_t *);

void wrkstn_processUartEcho(uart_pkt_t*);
void wrkstn_processUartSyncTime(uart_pkt_t*);

extern bool rx_handler(unsigned char c); /* from uart_intfc_cc430.c */
bool wrkstn_rx_handler(uint8_t c);
bool sendTimeSyncReply(linkID_t);


void wrkstn_init_uart(void)
{
    /* use wrkstn its own rx handler. The tx handler is the default one. */
    uart_intfc_init(wrkstn_rx_handler, NULL);
}

void wrkstn_taskRunning(uint16_t aInRunningStage)
{
    node_turn_on_green_led();
    node_turn_on_red_led();

    switch ((running_stage_t)aInRunningStage) {
        case RUNNING_STAGE_RTC_WKUP: {
            /* update next wakeup time */
            BSP_CRITICAL_STATEMENT( gNextWkup += AWAKE_INTERVAL );
            uint32_t lCurTime = rtc_getTimeOffset();

            /* send time stamp to peers */
            log(LOG_DEBUG, "[wrkstn] time = %u, next wkup = %u",
                    (uint32_t)lCurTime,
                    (uint32_t)gNextWkup);

            /* awake for 3s */
            soft_setTimer(AWAKE_PERIOD, wrkstn_taskSleep, RUNNING_STAGE_SLEEP);

            /* wake up again after 10s */
//            gWkupTimerSlot = soft_setTimer(gNextWkup-lCurTime, wrkstn_taskRunning, RUNNING_STAGE_RTC_WKUP);
        } break;

        case RUNNING_STAGE_UART_WKUP: {
            if (-1 != gUartWkupTimerSlot) {
                soft_cancelTimer(gUartWkupTimerSlot);
            }
            gUartWkupTimerSlot = soft_setTimer(5, wrkstn_taskRunning, RUNNING_STAGE_UART_READ);
        } break;

        case RUNNING_STAGE_UART_READ: {
            gUartWkupTimerSlot = -1;
            post_task(wrkstn_taskReadUartMsg, 0);
        } break;
    }
}

void wrkstn_taskSleep(uint16_t aInPhase)
{
    node_turn_off_green_led();
    node_turn_off_red_led();
    lpw_enterSleep();
}

void wrkstn_registerMsgProcessor(void)
{
    sMsgProcessor[RF_CMD_SYNC_REQ] = wrkstn_processSyncReq;
    sMsgProcessor[RF_CMD_SYNC_REP] = wrkstn_processSyncRep;
    sMsgProcessor[RF_CMD_DATA_TMP] = wrkstn_processTmpData;
}

void wrkstn_registerUartMsgProcessor(void)
{
    sUartMsgProcessor[UART_CMD_ECHO] = wrkstn_processUartEcho;
    sUartMsgProcessor[UART_CMD_SYNC_TIME] = wrkstn_processUartSyncTime;
//    sUartMsgProcessor[UART_CMD_SYNC_INTV] = wrkstn_processUart;
//    sUartMsgProcessor[UART_CMD_UPLOAD_DATA] = wrkstn_processUart;
}

/**
 * Runs in ISR context. Reading the frame should be done in the
 * application thread not in the ISR thread.
 */
static uint8_t sCB(linkID_t lid) {
    if (lid) {
        post_task(wrkstn_task_readFrame, (uint16_t) lid);
    } else {
        post_task(wrkstn_task_addDevice, 0);
    }

    /* leave frame to be read by application. */
    return 0;
}

void wrkstn_taskStartup(uint16_t aInStartupStage)
{
    if (STARTUP != gConfig._phase) {
        return;
    }

    switch (aInStartupStage) {
    case STARTUP_STAGE_INIT:
        log(LOG_DEBUG, "Trying to init AP...");
        if (SMPL_SUCCESS == SMPL_Init(sCB)) {
            log(LOG_DEBUG, "Init AP done.");
            BSP_CRITICAL_STATEMENT( gNextWkup = rtc_getTimeOffset() + AWAKE_INTERVAL );
            post_task(wrkstn_taskSleep, RUNNING_STAGE_SLEEP);
//            gWkupTimerSlot = soft_setTimer(AWAKE_INTERVAL, wrkstn_taskRunning, RUNNING_STAGE_RTC_WKUP);
            node_setPhase(RUNNING);
        } else {
            node_toggle_red_led();
            soft_setTimer(250, wrkstn_taskStartup, aInStartupStage);
        }
        break;
    }
}

void wrkstn_task_readFrame(uint16_t arg) {
    /* Have we received a frame on one of the ED connections?
     * No critical section -- it doesn't really matter much if we miss a poll
     */
    pkt_app_t lPkt = {0};
    uint8_t len = 0;
    linkID_t lid = (linkID_t)arg;

    /* process all frames waiting */
    if (SMPL_SUCCESS != SMPL_Receive(lid, (uint8_t*)&lPkt, &len)) {
        log(LOG_WARNING, "failed to read a frame from link %u",
                (uint32_t)lid);
        return;
    }

    if (lPkt.hdr.cmd >= RF_CMD_END) {
        log(LOG_WARNING, "unknown msg type %u read from link %u",
                (uint32_t)lPkt.hdr.cmd,
                (uint32_t)lid);
        return;
    }

    sMsgProcessor[lPkt.hdr.cmd](&lPkt);
}

void wrkstn_taskReadUartMsg(uint16_t arg) {
    uart_pkt_t lPkt = {0};

    while (rx_peek() >= sizeof(uart_pkt_t)) {
        rx_receive((uint8_t*)&lPkt, sizeof(uart_pkt_t));

        if (lPkt._cmd >= UART_CMD_END) {
            log(LOG_ERROR, "unknown uart msg type %u", (uint32_t)lPkt._cmd);
            continue;
        }

        sUartMsgProcessor[lPkt._cmd](&lPkt);
    }
}

void wrkstn_task_addDevice(uint16_t arg) {
    /* Wait for the Join semaphore to be set by the receipt of a Join frame from a
     * device that supports an End Device.
     *
     * An external method could be used as well. A button press could be connected
     * to an ISR and the ISR could set a semaphore that is checked by a function
     * call here, or a command shell running in support of a serial connection
     * could set a semaphore that is checked by a function call.
     */
    if (sNumCurrentPeers < NUM_CONNECTIONS) {
        /* listen for a new connection */
        while (1) {
            if (SMPL_SUCCESS == SMPL_LinkListen(&sLID[sNumCurrentPeers])) {
                log(LOG_DEBUG, "End device added: sLID[%u] = %u",
                        (uint32_t)sNumCurrentPeers,
                        (uint32_t)sLID[sNumCurrentPeers]);
                break;
            }
            /* Implement fail-to-link policy here. otherwise, listen again. */
        }

        sNumCurrentPeers++;
    }
}

void wrkstn_processSyncReq(pkt_app_t *aInPkt)
{
    log(LOG_DEBUG, "enter into processSyncReq");

    if (0 == gNextWkup) {
        return;
    }

    log(LOG_DEBUG, "RF_CMD_SYNC_REQ: link %u",
            (uint32_t)(aInPkt->hdr.nodeid));

    sendTimeSyncReply((linkID_t)aInPkt->hdr.nodeid);
}

void wrkstn_processSyncRep(pkt_app_t *aInPkt)
{

}

void wrkstn_processTmpData(pkt_app_t *aInPkt)
{

}

bool sendTimeSyncReply(linkID_t aInLid)
{
    pkt_app_t lPkt = {0};
    lPkt.hdr.nodeid = (uint16_t)aInLid;
    lPkt.hdr.cmd = RF_CMD_SYNC_REP;
    lPkt.hdr.rssi = 0;

    app_msg_sync_req_t lMsg;
    lMsg.fTimeOffset = rtc_getTimeOffset();
    lMsg.fTimeWkup = gNextWkup;
    memcpy(lPkt.data, (uint8_t*)&lMsg, sizeof(lMsg));

    log(LOG_DEBUG, "Sending sync reply (link %u: %u,%u)......",
            (uint32_t)aInLid,
            (uint32_t)lMsg.fTimeOffset,
            (uint32_t)lMsg.fTimeWkup);

    smplStatus_t rc = SMPL_SendOpt(
            aInLid,
            (uint8_t*)&lPkt,
            sizeof(pkt_app_header_t) + sizeof(app_msg_sync_req_t),
            SMPL_TXOPTION_ACKREQ);
    if (SMPL_SUCCESS != rc) {
        log(LOG_DEBUG, "Sending RF_CMD_SYNC_REP failed");
    }

    return SMPL_SUCCESS == rc;
}

bool wrkstn_rx_handler(uint8_t c)
{
    /* wake up the node if necessary */
    if (lpw_isSleep()) {
        lpw_exitSleep();
    }

    /* post a task */
    if (RUNNING == gConfig._phase) {
        post_task(wrkstn_taskRunning, RUNNING_STAGE_UART_WKUP);
    }

    return rx_handler(c);
}

void wrkstn_processUartEcho(uart_pkt_t *aInPkt)
{
    log(LOG_DEBUG, "enter into wrkstn_processUartEcho");
    tx_send_wait(aInPkt->_str, sizeof(aInPkt->_str));
}

void wrkstn_processUartSyncTime(uart_pkt_t *aInPkt)
{
    uint32_t lCur = rtc_getTimeOffset();
    log(LOG_DEBUG, "enter into wrkstn_processUartSyncTime");
    log(LOG_DEBUG, "current time: %u, current next wkup: %u", lCur, gNextWkup);

    BSP_CRITICAL_STATEMENT( gNextWkup += (int32_t)(aInPkt->_time - lCur) );
    rtc_setTimeOffset(aInPkt->_time);
    log(LOG_DEBUG, "modified time: %u, modified next wkup: %u", aInPkt->_time, gNextWkup);
}
// eof...
