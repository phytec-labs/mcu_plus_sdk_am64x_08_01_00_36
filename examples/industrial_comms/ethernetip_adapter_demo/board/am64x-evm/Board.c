/*!
 *  \example Board.c
 *
 *  \brief
 *  Handle hardware specific functionality.
 *
 *  \author
 *  KUNBUS GmbH
 *
 *  \date
 *  2021-06-09
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
 */


#include <stdio.h>
#include <string.h>

#include <board/led.h>
#include <drivers/i2c.h>

#include <api/EI_API_CIP.h>
#include "ti_board_config.h"
#include "ti_board_open_close.h"
#include "Board.h"

typedef struct
{
    LED_Handle ledHandle;
    const LED_Attrs  *attrs;
    uint8_t value;
} IDK_SLedHandle_t;

// Global variables and pointers used in this example.
static IDK_SLedHandle_t tLedHandle_g;

/*!
 *  <!-- Description: -->
 *
 *  \brief
 *  Set LEDs on AM64x EVM board which are controlled by TPIC2810
 *
 *  \details
 *  .
 *
 *  <!-- Parameters and return values: -->
 *
 *  \param[in]  pattern             LED pattern.
 *
 */
void IDK_setIndustrialLeds(uint8_t value)
{
    int32_t status;
    int32_t ledCnt;

    if(tLedHandle_g.ledHandle == NULL || tLedHandle_g.attrs == NULL)
    {
        return;
    }

    if( tLedHandle_g.value == value)
    {
        return;
    }

    tLedHandle_g.value = value;

    //for(ledCnt = 0U; ledCnt < 1; ledCnt++)
    for(ledCnt = 0U; ledCnt < tLedHandle_g.attrs->numLedPerGroup; ledCnt++)
    {
        if(value & 1)
            status = LED_on(tLedHandle_g.ledHandle, ledCnt);
        else
            status = LED_off(tLedHandle_g.ledHandle, ledCnt);
        if(SystemP_SUCCESS != status)
        {
            return;
        }
        value >>= 1;
    }
 }

/*!
 *  <!-- Description: -->
 *
 *  \brief
 *  Initialization of the board.
 *
 *  \details
 *  Initialization of the industrial LEDs on board.
 *
 *  <!-- Parameters and return values: -->
 *
 *  \return     bool                Operation result.
 *
 *  \retval     true                Success.
 *  \retval     false               Failed.
 *
 */
bool IDK_init(void)
{
    bool result = true;
    int32_t status;

    memset(&tLedHandle_g, 0, sizeof(tLedHandle_g));

    // Get LED handle from sys config
    tLedHandle_g.ledHandle = gLedHandle[CONFIG_LED0];
    if(NULL == tLedHandle_g.ledHandle)
    {
        OSAL_printf("No led handle available\n");
        result = false;
    }
    else
    {
        tLedHandle_g.attrs = LED_getAttrs(CONFIG_LED0);
        if(NULL == tLedHandle_g.attrs)
        {
            OSAL_printf("Can not get LED attributes\n");
            result = false;
        }
        else
        {
            if(tLedHandle_g.attrs->numLedPerGroup > 1U)
            {
                status = LED_setMask(tLedHandle_g.ledHandle, 0xFFU);
                if(SystemP_SUCCESS != status)
                {
                    OSAL_printf("Can not set LED Mask\n");
                    result = false;
                }
                else
                {
                    // Switch all LEDs off
                    int32_t ledCnt;
                    for(ledCnt = 0U; ledCnt < tLedHandle_g.attrs->numLedPerGroup; ledCnt++)
                        LED_off(tLedHandle_g.ledHandle, ledCnt);
                }
            }
        }
    }

    return result;
}


/*!
 *  <!-- Description: -->
 *
 *  \brief
 *  Deinitialization of the board.
 *
 *  \details
 *  Deinitialization of the industrial LEDs on board.
 *
 *  <!-- Parameters and return values: -->
 *
 *  \return     bool                Operation result.
 *
 *  \retval     true                Success.
 *  \retval     false               Failed.
 *
 */
bool IDK_deInit(void)
{
    int32_t status;

    status = LED_setMask(tLedHandle_g.ledHandle, 0x0U);

    return (status == SystemP_SUCCESS);
}
