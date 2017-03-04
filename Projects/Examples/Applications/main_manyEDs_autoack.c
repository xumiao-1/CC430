/*----------------------------------------------------------------------------
 *  Demo Application for SimpliciTI
 *
 *  L. Friedman
 *  Texas Instruments, Inc.
 *---------------------------------------------------------------------------- */

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
#include <stdio.h>
#include "bsp.h"
#include "mrfi.h"
#include "nwk_types.h"
#include "nwk_api.h"
#include "bsp_leds.h"
#include "bsp_buttons.h"

#include "app_remap_led.h"

#include "Si705x.h"
#include "task_scheduler.h"
#include "utils.h"
#include "appcommon/mydef.h"
#include "appcommon/node.h"
#include "appcommon/tmpsnr/tmpsnr.h"

#ifndef APP_AUTO_ACK
#error ERROR: Must define the macro APP_AUTO_ACK for this application.
#endif



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
    post_task(tmpsnr_taskStartup, STARTUP_STAGE_INIT);

    while (1) {
        task_scheduler();
    }
}


// eof...
