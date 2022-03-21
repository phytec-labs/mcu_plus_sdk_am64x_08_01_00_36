/*!
 *  \example appHwLed.h
 *
 *  \brief
 *  AM64x and AM243x specific LED implementation
 *
 *  \author
 *  KUNBUS GmbH
 *
 *  \date
 *  2021-07-14
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


#ifndef AM64X_AM243X_HW_LED
#define AM64X_AM243X_HW_LED

#include "appHwError.h"
#include "ti_board_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 *  \brief HW LED instances of the LED groups.
 */
typedef enum APP_HW_LED_EGroupInst
{
    APP_HW_LED_eCYCLIC_INST = CONFIG_CYCLIC_LEDS,
    APP_HW_LED_eBUS_INST = CONFIG_BUS_LED,
    APP_HW_LED_eMAX_INST = CONFIG_LED_NUM_INSTANCES
} APP_HW_LED_EGroupInst_t;

APP_HW_EError_t APP_HW_LED_setGroup(APP_HW_LED_EGroupInst_t instance_p, uint32_t value_p);
APP_HW_EError_t APP_HW_LED_toggleGroup(APP_HW_LED_EGroupInst_t instance_p);
APP_HW_EError_t APP_HW_LED_initGroup(APP_HW_LED_EGroupInst_t instance_p);
APP_HW_EError_t APP_HW_LED_deInitGroup(APP_HW_LED_EGroupInst_t instance_p);

#ifdef  __cplusplus 
}
#endif 

#endif // AM64X_AM243X_HW_LED
