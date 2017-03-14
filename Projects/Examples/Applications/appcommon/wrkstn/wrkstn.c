//#ifdef WRKSTN_NODE
#include <string.h>
#include "cc430f5137.h"
#include "bsp.h"
#include "mrfi.h"
#include "bsp_leds.h"
#include "bsp_buttons.h"
#include "nwk_types.h"
#include "nwk_api.h"
#include "nwk_frame.h"
#include "nwk.h"
#include "rtc_cc430.h"
#include "lpw_cc430.h"
#include "soft_timer.h"

#include "utils.h"
#include "../mydef.h"
#include "wrkstn.h"
#include "../node.h"


typedef void (*msg_processor_t)(pkt_app_t*);

static linkID_t sLID[NUM_CONNECTIONS] = { 0 };
static uint8_t sNumCurrentPeers = 0;
static msg_processor_t sMsgProcessor[RF_CMD_END];

void wrkstn_processSyncReq(pkt_app_t *);
void wrkstn_processSyncRep(pkt_app_t *);
void wrkstn_processTmpData(pkt_app_t *);

bool sendTimeSyncReply(linkID_t);


void wrkstn_taskMain(uint16_t arg)
{
    node_turn_on_green_led();
    node_turn_on_red_led();

    /* update next wakeup time */
    gNextWkup += AWAKE_INTERVAL;
    uint32_t lCurTime = rtc_getTimeOffset();

    /* send time stamp to peers */
    log(LOG_DEBUG, "[wrkstn] time = %u, next wkup = %u",
            (uint32_t)lCurTime,
            (uint32_t)gNextWkup);

    /* awake for 3s */
    soft_setTimer(AWAKE_PERIOD, wrkstn_taskSleep, 0);

    /* wake up again after 10s */
    if (gNextWkup <= lCurTime) {
        log(LOG_WARNING, "next wakeup time <= current time");
    } else {
        gWkupTimerSlot = soft_setTimer(gNextWkup-lCurTime, wrkstn_taskMain, 0);
    }
}

void wrkstn_taskSleep(uint16_t arg)
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
            gNextWkup = rtc_getTimeOffset() + AWAKE_INTERVAL;
            gWkupTimerSlot = soft_setTimer(AWAKE_INTERVAL, wrkstn_taskMain, 0);
            node_setPhase(RUNNING);
        } else {
            node_toggle_red_led();
            soft_setTimer(250, wrkstn_taskStartup, aInStartupStage);
        }
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
//#endif //WRKSTN_NODE
// eof...
