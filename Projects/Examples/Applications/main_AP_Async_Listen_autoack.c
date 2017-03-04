/*----------------------------------------------------------------------------
 *  Demo Application for SimpliciTI
 *
 *  L. Friedman
 *  Texas Instruments, Inc.
 *----------------------------------------------------------------------------
 */

/**********************************************************************************************
 Copyright 2007-2009 Texas Instruments Incorporated. All rights reserved.

 IMPORTANT: Your use of this Software is limited to those specific rights granted under
 the terms of a software license agreement between the user who downloaded the software,
 his/her employer (which must be your employer) and Texas Instruments Incorporated (the
 "License"). You may not use this Software unless you agree to abide by the terms of the
 License. The License limits your use, and you acknowledge, that the Software may not be
 modified, copied or distributed unless embedded on a Texas Instruments microcontroller
 or used solely and exclusively in conjunction with a Texas Instruments radio frequency
 transceiver, which is integrated into your product. Other than for the foregoing purpose,
 you may not use, reproduce, copy, prepare derivative works of, modify, distribute,
 perform, display or sell this Software and/or its documentation for any purpose.

 YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS"
 WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY
 WARRANTY OF MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
 IN NO EVENT SHALL TEXAS INSTRUMENTS OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT,
 NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER LEGAL EQUITABLE
 THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES INCLUDING BUT NOT LIMITED TO ANY
 INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST
 DATA, COST OF PROCUREMENT OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY
 THIRD PARTIES (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

 Should you have any questions regarding your right to use this Software,
 contact Texas Instruments Incorporated at www.TI.com.
 **************************************************************************************************/
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

#include "app_remap_led.h"

#include "task_scheduler.h"
#include "rtc_cc430.h"
//#include "lpw_cc430.h"
#include "Si705x.h"
#include "utils.h"
#include "delay.h"
#include "soft_timer.h"
#include "appcommon/mydef.h"
#include "appcommon/node.h"
#include "appcommon/wrkstn/wrkstn.h"

#ifndef APP_AUTO_ACK
#error ERROR: Must define the macro APP_AUTO_ACK for this application.
#endif


/**************************** COMMENTS ON ASYNC LISTEN APPLICATION ***********************
 Summary:
 This AP build includes implementation of an unknown number of end device peers in
 addition to AP functionality. In this scenario all End Devices establish a link to
 the AP and only to the AP. The AP acts as a data hub. All End Device peers are on
 the AP and not on other distinct ED platforms.

 There is still a limit to the number of peers supported on the AP that is defined
 by the macro NUM_CONNECTIONS. The AP will support NUM_CONNECTIONS or fewer peers
 but the exact number does not need to be known at build time.

 In this special but common scenario SimpliciTI restricts each End Device object to a
 single connection to the AP. If multiple logical connections are required these must
 be accommodated by supporting contexts in the application payload itself.

 Solution overview:
 When a new peer connection is required the AP main loop must be notified. In essence
 the main loop polls a semaphore to know whether to begin listening for a peer Link
 request from a new End Device. There are two solutions: automatic notification and
 external notification. The only difference between the automatic notification
 solution and the external notification solution is how the listen semaphore is
 set. In the external notification solution the sempahore is set by the user when the
 AP is stimulated for example by a button press or a commend over a serial link. In the
 automatic scheme the notification is accomplished as a side effect of a new End Device
 joining.

 The Rx callback must be implemented. When the callback is invoked with a non-zero
 Link ID the handler could set a semaphore that alerts the main work loop that a
 SMPL_Receive() can be executed successfully on that Link ID.

 If the callback conveys an argument (LinkID) of 0 then a new device has joined the
 network. A SMPL_LinkListen() should be executed.

 Whether the joining device supports ED objects is indirectly inferred on the joining
 device from the setting of the NUM_CONNECTIONS macro. The value of this macro should
 be non-zero only if ED objects exist on the device. This macro is always non-zero
 for ED-only devices. But Range Extenders may or may not support ED objects. The macro
 should be be set to 0 for REs that do not also support ED objects. This prevents the
 Access Point from reserving resources for a joinng device that does not support any
 End Device Objects and it prevents the AP from executing a SMPL_LinkListen(). The
 Access Point will not ever see a Link frame if the joining device does not support
 any connections.

 Each joining device must execute a SMPL_Link() after receiving the join reply from the
 Access Point. The Access Point will be listening.

 *************************** END COMMENTS ON ASYNC LISTEN APPLICATION ********************/

/************  THIS SOURCE FILE REPRESENTS THE AUTOMATIC NOTIFICATION SOLUTION ************/

/* Frequency Agility helper functions */
//static void checkChangeChannel(void);
//static void changeChannel(void);
/* work loop semaphores */
static volatile uint8_t sPeerFrameSem = 0;
static volatile uint8_t sJoinSem = 0;

#ifdef FREQUENCY_AGILITY
/*     ************** BEGIN interference detection support */

#define INTERFERNCE_THRESHOLD_DBM (-70)
#define SSIZE    25
#define IN_A_ROW  3
//static int8_t sSample[SSIZE];
//static uint8_t sChannel = 0;
#endif  /* FREQUENCY_AGILITY */

/* blink LEDs when channel changes... */
static volatile uint8_t sBlinky = 0;


/*     ************** END interference detection support                       */

#define FLASH_ADDRESS (0x05)

// copy the values from the module into a set of shadow registers in RAM
// during interrupts from the rtc.
// the shadow registers can be read safely from the main program
// provided that interrupts are disabled during the read.
// this will not cause any time to be lost because the rtc is updated by HW.
//uint32_t getRTCCounter(void) {
//    return RTCNT1 + (RTCNT2 << 8);
//}






void main(void) {
    /* init the node */
    node_init();

    /* If an on-the-fly device address is generated it must be done before the
     * call to SMPL_Init(). If the address is set here the ROM value will not
     * be used. If SMPL_Init() runs before this IOCTL is used the IOCTL call
     * will not take effect. One shot only. The IOCTL call below is conformal.
     */
#ifdef I_WANT_TO_CHANGE_DEFAULT_ROM_DEVICE_ADDRESS_PSEUDO_CODE
    {
        addr_t lAddr;

        createRandomAddress(&lAddr);
        SMPL_Ioctl(IOCTL_OBJ_ADDR, IOCTL_ACT_SET, &lAddr);
    }
#endif /* I_WANT_TO_CHANGE_DEFAULT_ROM_DEVICE_ADDRESS_PSEUDO_CODE */

    node_setPhase(STARTUP);
    post_task(wrkstn_taskStartup, STARTUP_STAGE_INIT);

//	/* test flash */
//	ExtenalFlash_Read((uint8_t*) rdbuf, FLASH_ADDRESS, strlen(buf));
//	tx_send_wait("before:\r\n", 9);
//	tx_send_wait(rdbuf, sizeof(rdbuf));
//	ExtenalFlash_Write((uint8_t*) buf, strlen(buf), FLASH_ADDRESS);
//	ExtenalFlash_Read((uint8_t*) rdbuf, FLASH_ADDRESS, strlen(buf));
//	tx_send_wait("after:\r\n", 8);
//	tx_send_wait(rdbuf, strlen(buf));

    /* main work loop */
#if 1
    while (1) {
        task_scheduler();
    }

#else
    TA0CCTL1 = CCIE;
    TA0CCTL2 = CCIE;
    TA0CCR1 = 0;
    TA0CCR2 = 0;
    TA0CTL = TASSEL_1 + MC_2;

    __enable_interrupt();
    __bis_SR_register(LPM0 + GIE); // LPM0 with interrupts enabled
#endif

}


//static void changeChannel(void) {
//#ifdef FREQUENCY_AGILITY
//	freqEntry_t freq;
//
//	if (++sChannel >= NWK_FREQ_TBL_SIZE) {
//		sChannel = 0;
//	}
//	freq.logicalChan = sChannel;
//	SMPL_Ioctl(IOCTL_OBJ_FREQ, IOCTL_ACT_SET, &freq);
//	sBlinky = 1;
//#endif
//	return;
//}

/* implement auto-channel-change policy here... */
//static void checkChangeChannel(void) {
//#ifdef FREQUENCY_AGILITY
//	int8_t dbm, inARow = 0;
//
//	uint8_t i;
//
//	memset(sSample, 0x0, SSIZE);
//	for (i = 0; i < SSIZE; ++i) {
//		/* quit if we need to service an app frame */
//		if (sPeerFrameSem || sJoinSem) {
//			return;
//		}
//		NWK_DELAY(1);
//		SMPL_Ioctl(IOCTL_OBJ_RADIO, IOCTL_ACT_RADIO_RSSI, (void *) &dbm);
//		sSample[i] = dbm;
//
//		if (dbm > INTERFERNCE_THRESHOLD_DBM) {
//			if (++inARow == IN_A_ROW) {
//				changeChannel();
//				break;
//			}
//		} else {
//			inARow = 0;
//		}
//	}
//#endif
//	return;
//}

#if 0
#pragma vector=TIMER0_A1_VECTOR
__interrupt void TIMER0_A1_ISR(void) {
    switch (__even_in_range(TA0IV, 16)) {
    case 0:                             // No interrupts
        break;

    case 2:                             // CCR1 CCIFG
        bsp_toggle_green_led();
        TA0CCR1 += 32768;
        break;

    case 4:                             // CCR2 CCIFG
        bsp_toggle_red_led();
        TA0CCR2 += 16384;
        break;

    case 6:
        break;                          // RTCAIFG
    case 8:
        break;                          // RT0PSIFG
    case 10:                            // RT1PSIFG
        break;

    case 12:
        break;                         // Reserved
    case 14:
        break;                         // Reserved
    case 16:
        break;                         // Reserved
    default:
        break;
    }
}
#endif
// eof...
