/*!
 *  \file ESL_BOARD_OS_config.c
 *
 *  \brief
 *  Board OS dependent configuration for FreeRTOS AM243X Launchpad.
 *
 *  \author
 *  KUNBUS GmbH
 *
 *  \date
 *  2021-05-21
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

#include <ESL_BOARD_OS_config.h>

#include "ti_board_config.h"
#include "ti_board_open_close.h"

const ESL_GPIO_STestPinCfg_t ESL_GPIO_testPinCfg_g[] = {
#if (defined CONFIG_GPIO_TEST0_PIN)
   { .baseAddress = CONFIG_GPIO_TEST0_BASE_ADDR, .pinNumber = CONFIG_GPIO_TEST0_PIN, .pinDirection = CONFIG_GPIO_TEST0_DIR},
#endif
#if (defined CONFIG_GPIO_TEST1_PIN)
   { .baseAddress = CONFIG_GPIO_TEST1_BASE_ADDR, .pinNumber = CONFIG_GPIO_TEST1_PIN, .pinDirection = CONFIG_GPIO_TEST1_DIR},
#endif
#if (defined CONFIG_GPIO_TEST2_PIN)
   { .baseAddress = CONFIG_GPIO_TEST2_BASE_ADDR, .pinNumber = CONFIG_GPIO_TEST2_PIN, .pinDirection = CONFIG_GPIO_TEST2_DIR},
#endif
#if (defined CONFIG_GPIO_TEST3_PIN)
   { .baseAddress = CONFIG_GPIO_TEST3_BASE_ADDR, .pinNumber = CONFIG_GPIO_TEST3_PIN, .pinDirection = CONFIG_GPIO_TEST3_DIR},
#endif
};

/*!
 *  <!-- Description: -->
 *
 *  \brief
 *  Configure/control board status LED
 *
 *  <!-- Parameters and return values: -->
 *
 *  \param[in]  selectedPru_p   Pru Instance index
 *  \param[in]  runLed_p        Status LED on/off
 *  \param[in]  errLed_p        Error LED on/off
 *
 *  <!-- Example: -->
 *
 *  \par Example
 *  \code{.c}
 *
 *  // the Call
 *  ESL_Board_StatusLED(gpioHandle, EC_API_SLV_ePRUICSS_INSTANCE_ONE, true, false);
 *  \endcode
 *
 *  <!-- Group: -->
 *
 *  \ingroup ESL_OS
 *
 * */
void ESL_Board_StatusLED(void* pGpioHandle_p, uint32_t selectedPru_p, bool runLed_p, bool errLed_p)
{
    OSALUNREF_PARM(selectedPru_p);
    OSALUNREF_PARM(pGpioHandle_p);

#if !(defined FBTLPROVIDER) || (FBTLPROVIDER==0)
    if (runLed_p) /* ioexp port 0 */
    {
        LED_on(gLedHandle[CONFIG_LED_STATUS], 0);
    }
    else
    {
        LED_off(gLedHandle[CONFIG_LED_STATUS], 0);
    }

    if (errLed_p) /* gpio */
    {
        LED_on(gLedHandle[CONFIG_LED_ERROR], 0);
    }
    else
    {
        LED_off(gLedHandle[CONFIG_LED_ERROR], 0);
    }
#endif
}

//*************************************************************************************************
