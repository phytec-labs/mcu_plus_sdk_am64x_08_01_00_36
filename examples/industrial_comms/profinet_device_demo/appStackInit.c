/*!
 *  \example appStackInit.c
 *
 *  \brief
 *  PROFINET Device Stack Initialization, PRU Start, and Stop Functions
 *
 *  \author
 *  KUNBUS GmbH
 *
 *  \date
 *  2020-06-19
 *
 *  \copyright 
 *  Copyright (c) 2021, KUNBUS GmbH<br /><br />
 *  All rights reserved.<br />
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:<br />
 *  <ol>
 *  <li>Redistributions of source code must retain the above copyright notice, this
 *     list of conditions and the following disclaimer.</li>
 *  <li>Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.</li>
 *  <li>Neither the name of the copyright holder nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.</li>
 *  </ol>
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 *  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 *  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 *  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
 
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include <osal.h>
#include <osal_error.h>
#include <hwal.h>

#include <pru.h>
#include <pru_Profinet.h>

#include "appStackInit.h"
#include "appUserInfo.h"
#include "appHwBoardInfo.h"
#include "appHwError.h"

/*!
*  <!-- Description: -->
* \brief
* PRU initialization.
*
* \details
* Here we configure relevant PRU instance for Profinet communication.
* PRU instance will be initialized and firmware will be loaded.
*
* \remarks
* Important: Configure correct MAC address.
*
*
* <!-- Parameters and return values: -->
*
*  \param[in]  pruLogicalInstance_p         Number of PRU instance.
*
*  \return     Error code of int type. See defines with namespace OSAL_PRU_ in osal_error.h
*  \retval     OSAL_NO_ERROR           Success.
*
*/
uint32_t APP_pruInit (
    uint8_t pruLogicalInstance_p)
{
    PRU_PN_TPruLoadParameter    pruParam   = {0};
    uint32_t                    result     = 0;
    APP_HW_EError_t             status     = APP_HW_eNO_ERROR;
        
    status = APP_HW_BOARD_INFO_read();

    if (status == APP_HW_eNO_ERROR)
    {
        memset(&pruParam, 0, sizeof(pruParam));
        memmove(pruParam.ai8uMacAddr, APP_UI_getMacAddr(), PNC_MAC_ADDR_LEN);
        pruParam.i32LogicPruInstance = pruLogicalInstance_p;

        /*Initialize PRU and load Firmware*/
        result = PRU_PN_loadPru(&pruParam);
        if (OSAL_NO_ERROR != result)
        {
            OSAL_printf("\r\n[APP] ERROR: Failed to init the PRU. Error code: 0x%8.8x\n", result);
        }
    }
    else
    {
        OSAL_printf("\r\n[APP] ERROR: Failed to get board info from EEPROM.\n");
        // Return a generic stack init error.
        result = OSAL_STACK_INIT_ERROR;
    }

    return result;
}

/*!
*  <!-- Description: -->
* \brief
* Start both PRU instances.
*
* \details
* Precondition: All relevant PRUs must be initialized and the firmware is loaded to PRU.
*
* <!-- Parameters and return values: -->
*
*/
void APP_pruStart (
    void)
{
    PRU_FW_start ();
}

/*!
*  <!-- Description: -->
* \brief
* Stop both PR instances.
*
* \details
* Precondition: All relevant PRUs must be initialized and the firmware is loaded to PRU.
*
* <!-- Parameters and return values: -->
*
*/
void APP_pruStop (
    void)
{
    PRU_FW_stop ();
}

//*************************************************************************************************
