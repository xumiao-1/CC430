//#ifdef WRKSTN_NODE

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

#include "utils.h"
#include "../mydef.h"
#include "wrkstn.h"
#include "../node.h"


typedef void (*msg_processor_t)(pkt_app_t*);

static linkID_t sLID[NUM_CONNECTIONS] = { 0 };
static uint8_t sNumCurrentPeers = 0;
static msg_processor_t sMsgProcessor[RF_CMD_END];

//void processMessage(linkID_t aInLid, uint8_t aInMsg, uint8_t aInLen);
void wrkstn_processSyncReq(pkt_app_t *);
void wrkstn_processSyncRep(pkt_app_t *);
void wrkstn_processTmpData(pkt_app_t *);

void sendTimeSync(linkID_t);

void wrkstn_registerMsgProcessor(void)
{
    sMsgProcessor[RF_CMD_SYNC_REQ] = wrkstn_processSyncReq;
    sMsgProcessor[RF_CMD_SYNC_REP] = wrkstn_processSyncRep;
    sMsgProcessor[RF_CMD_DATA_TMP] = wrkstn_processTmpData;
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

                /* reply with my time stamp */
                sendTimeSync(sLID[sNumCurrentPeers]);
                break;
            }
            /* Implement fail-to-link policy here. otherwise, listen again. */
        }

        sNumCurrentPeers++;
    }
}

void wrkstn_processSyncReq(pkt_app_t *aInPkt)
{

}

void wrkstn_processSyncRep(pkt_app_t *aInPkt)
{

}

void wrkstn_processTmpData(pkt_app_t *aInPkt)
{

}

void sendTimeSync(linkID_t aInLid)
{
	pkt_app_t lPkt = {0};
	lPkt.hdr.nodeid = (uint16_t)aInLid;
	lPkt.hdr.cmd = RF_CMD_SYNC_REP;
	lPkt.hdr.rssi = 0;

    app_msg_t lMsg;
    lMsg.fTimeOffset = rtc_getTimeOffset();
    lMsg.fTimeWkup = gNextWkup;
    memcpy(lPkt.data, (uint8_t*)&lMsg, sizeof(lMsg));

    log(LOG_DEBUG, "Sending pkt (link %u: %u,%u)......",
            (uint32_t)aInLid,
            (uint32_t)lMsg.fTimeOffset,
            (uint32_t)lMsg.fTimeWkup);

    smplStatus_t rc = SMPL_SendOpt(
            aInLid,
            (uint8_t*)&lPkt,
            sizeof(pkt_app_header_t) + sizeof(app_msg_t),//sizeof(lPkt),//
            SMPL_TXOPTION_ACKREQ);
    if (SMPL_SUCCESS == rc) {
        log(LOG_DEBUG, "succeeded");
    } else {
        log(LOG_DEBUG, "failed");
    }
}
//#endif //WRKSTN_NODE
// eof...
