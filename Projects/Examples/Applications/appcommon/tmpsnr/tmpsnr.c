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


#define AWAKE_INTERVAL (5000)
#define AWAKE_PERIOD   (AWAKE_INTERVAL / 2)


typedef void (*msg_processor_t)(pkt_app_t*);

static msg_processor_t sMsgProcessor[RF_CMD_END];

//void processMessage(linkID_t aInLid, uint8_t aInMsg, uint8_t aInLen);
void tmpsnr_processSyncReq(pkt_app_t *);
void tmpsnr_processSyncRep(pkt_app_t *);
void tmpsnr_processTmpData(pkt_app_t *);


void tmpsnr_taskMain(uint16_t arg)
{
    node_turn_on_green_led();
    node_turn_on_red_led();

    /* send time stamp to peers */
    log(LOG_DEBUG, "[tmpsnr] time = %u, next wkup = %u",
            (uint32_t)rtc_getTimeOffset(),
            (uint32_t)gNextWkup);

    /* awake for 3s */
    soft_setTimer(AWAKE_PERIOD, node_sleepISR, 0);

    /* wake up again after a while */
    soft_setTimer(gNextWkup-rtc_getTimeOffset(), node_awakeISR, 0);
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

    app_msg_t *lMsg = (app_msg_t*)(aInPkt->data);
    log(LOG_DEBUG, "RF_CMD_SYNC_REP: time %u, wkup %u......",
            (uint32_t)(lMsg->fTimeOffset),
            (uint32_t)(lMsg->fTimeWkup));

    /* set rtc time */
    rtc_setTimeOffset(lMsg->fTimeOffset);
    gNextWkup = lMsg->fTimeWkup;

    /* sleep */
    soft_setTimer(lMsg->fTimeWkup - lMsg->fTimeOffset, node_awakeISR, 0);
    post_task(tmpsnr_taskSleep, 0);

    return;
}

void tmpsnr_processTmpData(pkt_app_t *aInPkt)
{
    log(LOG_DEBUG, "enter into processTmpData");
}
//#endif //TMPSNR_NODE
// eof...
