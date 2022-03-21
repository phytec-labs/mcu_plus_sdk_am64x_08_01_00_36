/*!
 *  \example appLed.h
 *
 *  \brief
 *  Application LED task
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


#ifndef APP_LED
#define APP_LED

#ifdef __cplusplus
extern "C" {
#endif

/*!
 *  \brief Application LED task's scan rate.
 */
#define APP_LED_TASK_TICK_MS          10

/*!
 *  \brief Bus LED's blink frequency.
 */
#define APP_LED_BUS_BLINK_FREQ_HZ     2

/*!
 *  \brief Bus LED's toggle counter.
 */
#define APP_LED_BUS_BLINK_COUNTER     ((1000/APP_LED_BUS_BLINK_FREQ_HZ)/APP_LED_TASK_TICK_MS)

/*!
 *  \brief APP status LED (BUS, STATUS) states. (not applicable for cyclic data LEDs)
 */
typedef enum APP_ELedState
{
    APP_eLedOff = 0,
    APP_eLedOn = 1,
    APP_eLedBlink = 2,
    APP_eLedMAX
} APP_ELedState_t;

bool APP_startLedTask(void);

#ifdef  __cplusplus 
}
#endif 

#endif // APP_LED
