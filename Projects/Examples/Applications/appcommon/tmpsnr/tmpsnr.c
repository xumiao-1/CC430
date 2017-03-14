//#ifdef TMPSNR_NODE

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
#include "tmpsnr.h"
#include "../node.h"




typedef void (*msg_processor_t)(pkt_app_t*);

/* callback handler */
static uint8_t sCB(linkID_t);

static linkID_t sLinkID1 = 0;
static msg_processor_t sMsgProcessor[RF_CMD_END];

bool sendTimeSyncRequest(linkID_t);
void tmpsnr_processSyncReq(pkt_app_t *);
void tmpsnr_processSyncRep(pkt_app_t *);
void tmpsnr_processTmpData(pkt_app_t *);

static uint8_t sCB(linkID_t lid) {
    if (lid) {
        post_task(tmpsnr_task_readFrame, (uint16_t) lid);
    } else {
        post_task(tmpsnr_task_addDevice, 0);
    }

    /* leave frame to be read by application. */
    return 0;
}

void tmpsnr_taskStartup(uint16_t aInStartupStage)
{
    if (STARTUP != gConfig._phase) {
        return;
    }

    switch (aInStartupStage) {
    case STARTUP_STAGE_INIT:
        /* Keep trying to join (a side effect of successful initialization) until
         * successful. Toggle LEDS to indicate that joining has not occurred.
         */
        log(LOG_DEBUG, "Trying to join a network...");
        if (SMPL_SUCCESS == SMPL_Init(sCB)) {
            post_task(tmpsnr_taskStartup, STARTUP_STAGE_LINK);
            log(LOG_DEBUG, "Joined! Enter into STARTUP_STAGE_LINK");
        } else {
            node_toggle_red_led();
            soft_setTimer(250, tmpsnr_taskStartup, aInStartupStage);
        }
        break;

    case STARTUP_STAGE_LINK:
        /* Keep trying to link... */
        log(LOG_DEBUG, "Trying to link to an AP...");
        if (SMPL_SUCCESS == SMPL_Link(&sLinkID1)) {
            /* turn on RX. default is RX off. */
            SMPL_Ioctl(IOCTL_OBJ_RADIO, IOCTL_ACT_RADIO_RXON, 0);

            post_task(tmpsnr_taskStartup, STARTUP_STAGE_SYNC);
            log(LOG_DEBUG, "Linked! Enter into STARTUP_STAGE_SYNC");
        } else {
            node_toggle_green_led();
            soft_setTimer(250, tmpsnr_taskStartup, aInStartupStage);
        }
        break;

    case STARTUP_STAGE_SYNC:
        sendTimeSyncRequest(sLinkID1);
        soft_setTimer(250, tmpsnr_taskStartup, aInStartupStage);
        break;
    }
}

void tmpsnr_taskMain(uint16_t arg)
{
    node_turn_on_green_led();
    node_turn_on_red_led();

    /* update next wakeup time */
    gNextWkup += AWAKE_INTERVAL;
    uint32_t lCurTime = rtc_getTimeOffset();

    /* send time stamp to peers */
    log(LOG_DEBUG, "[tmpsnr] time = %u, next wkup = %u",
            (uint32_t)lCurTime,
            (uint32_t)gNextWkup);

    /* awake for 3s */
    soft_setTimer(AWAKE_PERIOD, tmpsnr_taskSleep, 0);

    /* wake up again after a while */
    if (gNextWkup <= lCurTime) {
        log(LOG_WARNING, "next wakeup time <= current time");
    } else {
        gWkupTimerSlot = soft_setTimer(gNextWkup-lCurTime, tmpsnr_taskMain, 0);
    }
}

void tmpsnr_taskSleep(uint16_t arg)
{
    node_turn_off_green_led();
    node_turn_off_red_led();
    lpw_enterSleep();
}

void tmpsnr_registerMsgProcessor(void)
{
    sMsgProcessor[RF_CMD_SYNC_REQ] = tmpsnr_processSyncReq;
    sMsgProcessor[RF_CMD_SYNC_REP] = tmpsnr_processSyncRep;
    sMsgProcessor[RF_CMD_DATA_TMP] = tmpsnr_processTmpData;
}

void tmpsnr_task_readFrame(uint16_t arg)
{
    log(LOG_DEBUG, "enter into readFrame");

    /* Have we received a frame on one of the ED connections?
     * No critical section -- it doesn't really matter much if we miss a poll
     */
    pkt_app_t lPkt = {0};
    uint8_t len = 0;
    linkID_t lid = (linkID_t)arg;

    /* process all frames waiting */
    if (SMPL_SUCCESS != SMPL_Receive(lid, (uint8_t*)&lPkt, &len)) {
        log(LOG_WARNING, "tmpsnr_task_readFrame: failed to read a frame from link %u",
                (uint32_t)lid);
        return;
    }

    if (lPkt.hdr.cmd >= RF_CMD_END) {
        log(LOG_WARNING, "tmpsnr_task_readFrame: unknown msg type %u read from link %u",
                (uint32_t)lPkt.hdr.cmd,
                (uint32_t)lid);
        return;
    }

    sMsgProcessor[lPkt.hdr.cmd](&lPkt);
}

void tmpsnr_task_addDevice(uint16_t arg)
{
    log(LOG_DEBUG, "link to an AP");
}


void tmpsnr_processSyncReq(pkt_app_t *aInPkt)
{
    log(LOG_DEBUG, "enter into processSyncReq");
}

void tmpsnr_processSyncRep(pkt_app_t *aInPkt)
{
    log(LOG_DEBUG, "enter into processSyncRep");

    app_msg_sync_req_t *lMsg = (app_msg_sync_req_t*)(aInPkt->data);
    log(LOG_DEBUG, "RF_CMD_SYNC_REP: time %u, wkup %u......",
            (uint32_t)(lMsg->fTimeOffset),
            (uint32_t)(lMsg->fTimeWkup));

    /* set rtc time */
    rtc_setTimeOffset(lMsg->fTimeOffset);
    gNextWkup = lMsg->fTimeWkup;

    /* sleep */
    if (STARTUP == gConfig._phase) {
        gWkupTimerSlot = soft_setTimer(gNextWkup - lMsg->fTimeOffset, tmpsnr_taskMain, 0);
		node_setPhase(RUNNING);
    }

    return;
}

void tmpsnr_processTmpData(pkt_app_t *aInPkt)
{
    log(LOG_DEBUG, "enter into processTmpData");
}

bool sendTimeSyncRequest(linkID_t aInLid)
{
	pkt_app_t lPkt = {0};
	lPkt.hdr.nodeid = (uint16_t)aInLid;
	lPkt.hdr.cmd = RF_CMD_SYNC_REQ;
	lPkt.hdr.rssi = 0;

	log(LOG_DEBUG, "Sending sync request...");

    smplStatus_t rc = SMPL_SendOpt(
            aInLid,
            (uint8_t*)&lPkt,
            sizeof(pkt_app_header_t),
            SMPL_TXOPTION_ACKREQ);
    if (SMPL_SUCCESS != rc) {
        log(LOG_DEBUG, "Sending RF_CMD_SYNC_REQ failed");
    }

    return SMPL_SUCCESS == rc;
}
//#endif //TMPSNR_NODE
// eof...
