/*!
 *  \example appHwLed.c
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

#include <stdio.h>
#include <string.h>
#include "board/led.h"
#include "ti_board_open_close.h"
#include "appHwLed.h"

/*!
 *  \brief Structure to store the LED handle, attributes and current values.
 */
typedef struct
{
    LED_Handle ledHandle;
    const LED_Attrs *ledAttrs;
    uint32_t currentLedValue;
} APP_HW_LED_SObj_t;

/*!
 *  \brief We store the the LED handle, attributes and current values for all group instances here.
 */
static APP_HW_LED_SObj_t APP_HW_LED_Obj_g[APP_HW_LED_eMAX_INST] = { 0 };

/*!
 *  Function: APP_HW_LED_set
 *
 *  \brief
 *  Helper function to set a group of LEDs or a single LED to the passed value.
 *
 *  \details
 *  Helper function to set a group of LEDs or a single LED to the passed value.
 *  It checks if the LED is GPIO based or I2C based and call either the
 *  setMask or LED_on / LED_off functions.
 *
 *  \remarks
 *  This function performs set on all LEDs of a group even though only one LED's state changed.
 *  This is a blocking function, since i2c transfers are blocking.
 *  LEDs connected to a single i2c expander belong to one group. For GPIO connected LEDs there is
 *  only one LED per group.
 *
 *  \param[in]     instance_p      instance ID of the LED group to be set.
 *  \param[in]     value_p         value to be set of the LED group.
 *
 *  \return        Error code as #APP_HW_EError_t
 *  \retval        #APP_HW_eNO_ERROR          No error.
 *  \retval        #APP_HW_eFATAL_ERROR       A fatal error occurred.
 *  \retval        #APP_HW_eINVALID_PARAM     Invalid instance ID was given.
 */
static APP_HW_EError_t APP_HW_LED_set(APP_HW_LED_EGroupInst_t instance_p, uint32_t value_p)
{
    int32_t status = SystemP_SUCCESS;
    APP_HW_EError_t result = APP_HW_eNO_ERROR;

    // If there is more than 1 LED in this group then we are dealing with an i2c expander.
    // We then can use the setMask function to set all the LED's connected to it.
    // This reduces i2c traffic, since transfers are set to blocking.
    if(APP_HW_LED_Obj_g[instance_p].ledAttrs->numLedPerGroup > 1U)
    {
        status = LED_setMask(APP_HW_LED_Obj_g[instance_p].ledHandle, value_p);
    }
    else
    {
        if(value_p & 1U)
        {
            status = LED_on(APP_HW_LED_Obj_g[instance_p].ledHandle, 0U);
        }
        else
        {
            status = LED_off(APP_HW_LED_Obj_g[instance_p].ledHandle, 0U);
        }
    }

    if(status != SystemP_SUCCESS)
    {
        result = APP_HW_eFATAL_ERROR;
    }
    return result;
}

/*!
 *  Function: APP_HW_LED_setGroup
 *
 *  \brief
 *  Function sets a group of LEDs to the passed value.
 *
 *  \details
 *  Function sets a group of LEDs to the passed value. It only performs the set operation when the
 *  current state of the LED changes.
 *
 *  \remarks
 *  This function performs set on all LEDs of a group even though only one LED's state changed.
 *  This is a blocking function for i2c, since i2c transfers are blocking.
 *  LEDs connected to a single i2c expander belong to one group. For GPIO connected LEDs there is
 *  only one LED per group.
 *
 *  \param[in]     instance_p      instance ID of the LED group to be set.
 *  \param[in]     value_p         value to be set of the LED group.
 *
 *  \return        Error code as #APP_HW_EError_t
 *  \retval        #APP_HW_eNO_ERROR          No error.
 *  \retval        #APP_HW_eFATAL_ERROR       A fatal error occurred.
 *  \retval        #APP_HW_eINVALID_PARAM     Invalid instance ID was given.
 */
APP_HW_EError_t APP_HW_LED_setGroup(APP_HW_LED_EGroupInst_t instance_p, uint32_t value_p)
{
    APP_HW_EError_t result = APP_HW_eNO_ERROR;

    if(instance_p >= APP_HW_LED_eMAX_INST)
    {
        result = APP_HW_eINVALID_PARAM;
    }
    else
    {
        if((APP_HW_LED_Obj_g[instance_p].ledHandle == NULL) ||
           (APP_HW_LED_Obj_g[instance_p].ledAttrs == NULL))
        {
            result = APP_HW_eFATAL_ERROR;
        }
        else
        {
            // Has the LED state changed ?
            // This reduces i2c traffic, since transfers are set to blocking.
            if(APP_HW_LED_Obj_g[instance_p].currentLedValue != value_p)
            {
                APP_HW_LED_Obj_g[instance_p].currentLedValue = value_p;
                result = APP_HW_LED_set(instance_p, value_p);
            }
        }
    }
    return result;
 }

/*!
 *  Function: APP_HW_LED_toggleGroup
 *
 *  \brief
 *  Function toggles a group of LEDs.
 *
 *  \details
 *  Function toggles a group of LEDs. It takes the current state of the LED and inverts it.
 *
 *  \remarks
 *  This function performs inversion on all LEDs of a group.
 *  This is a blocking function for i2c, since i2c transfers are blocking.
 *  LEDs connected to a single i2c expander belong to one group. For GPIO connected LEDs there is
 *  only one LED per group.
 *
 *  \param[in]     instance_p      instance ID of the LED group to be toggled.
 *
 *  \return        Error code as #APP_HW_EError_t
 *  \retval        #APP_HW_eNO_ERROR          No error.
 *  \retval        #APP_HW_eFATAL_ERROR       A fatal error occurred.
 *  \retval        #APP_HW_eINVALID_PARAM     Invalid instance ID was given.
 */
APP_HW_EError_t APP_HW_LED_toggleGroup(APP_HW_LED_EGroupInst_t instance_p)
{
    APP_HW_EError_t result = APP_HW_eNO_ERROR;

    if(instance_p >= APP_HW_LED_eMAX_INST)
    {
        result = APP_HW_eINVALID_PARAM;
    }
    else
    {
        if((APP_HW_LED_Obj_g[instance_p].ledHandle == NULL) ||
           (APP_HW_LED_Obj_g[instance_p].ledAttrs == NULL))
        {
            result = APP_HW_eFATAL_ERROR;
        }
        else
        {
            // Take the current LED group state and invert it.
            APP_HW_LED_Obj_g[instance_p].currentLedValue = ~APP_HW_LED_Obj_g[instance_p].currentLedValue;
            result = APP_HW_LED_set(instance_p, APP_HW_LED_Obj_g[instance_p].currentLedValue);
        }
    }
    return result;
 }

/*!
 *  Function: APP_HW_LED_initGroup
 *
 *  \brief
 *  Function initializes a group of LEDs.
 *
 *  \details
 *  Function initializes a group of LEDs. It sets the global #APP_HW_LED_Obj_g to correct values for
 *  the LED group instance and also resets the LEDs.
 *
 *  \remarks
 *  LEDs connected to a single i2c expander belong to one group. For GPIO connected LEDs there is
 *  only one LED per group.
 *  This is a blocking function for i2c, since i2c transfers are blocking.
 *
 *  \param[in]     instance_p      instance ID of the LED group to be initialized.
 *
 *  \return        Error code as #APP_HW_EError_t
 *  \retval        #APP_HW_eNO_ERROR          No error.
 *  \retval        #APP_HW_eFATAL_ERROR       A fatal error occurred.
 *  \retval        #APP_HW_eINVALID_PARAM     Invalid instance ID was given.
 */
APP_HW_EError_t APP_HW_LED_initGroup(APP_HW_LED_EGroupInst_t instance_p)
{
    APP_HW_EError_t result = APP_HW_eNO_ERROR;

    if(instance_p >= APP_HW_LED_eMAX_INST)
    {
        result = APP_HW_eINVALID_PARAM;
    }
    else
    {
        memset(&APP_HW_LED_Obj_g[instance_p], 0, sizeof(APP_HW_LED_Obj_g[0]));
        APP_HW_LED_Obj_g[instance_p].ledHandle = gLedHandle[instance_p];
        APP_HW_LED_Obj_g[instance_p].ledAttrs = LED_getAttrs(instance_p);

        if((APP_HW_LED_Obj_g[instance_p].ledHandle == NULL) ||
           (APP_HW_LED_Obj_g[instance_p].ledAttrs == NULL))
        {
            result = APP_HW_eFATAL_ERROR;
        }
        else
        {
            result = APP_HW_LED_set(instance_p, 0U);
        }
    }
    return result;
}

/*!
 *  Function: APP_HW_LED_deInitGroup
 *
 *  \brief
 *  Function deinitializes a group of LEDs.
 *
 *  \details
 *  Function deinitializes a group of LEDs. It sets the global #APP_HW_LED_Obj_g for
 *  the LED group instance to 0 and also resets the LEDs.
 *
 *  \remarks
 *  LEDs connected to a single i2c expander belong to one group. For GPIO connected LEDs there is
 *  only one LED per group.
 *  This is a blocking function for i2c, since i2c transfers are blocking.
 *
 *  \param[in]     instance_p      instance ID of the LED group to be deinitialized.
 *
 *  \return        Error code as #APP_HW_EError_t
 *  \retval        #APP_HW_eNO_ERROR          No error.
 *  \retval        #APP_HW_eFATAL_ERROR       A fatal error occurred.
 *  \retval        #APP_HW_eINVALID_PARAM     Invalid instance ID was given.
 */
APP_HW_EError_t APP_HW_LED_deInitGroup(APP_HW_LED_EGroupInst_t instance_p)
{
    APP_HW_EError_t result = APP_HW_eNO_ERROR;

    if(instance_p >= APP_HW_LED_eMAX_INST)
    {
        result = APP_HW_eINVALID_PARAM;
    }
    else
    {
        result = APP_HW_LED_set(instance_p, 0U);
        memset(&APP_HW_LED_Obj_g[instance_p], 0, sizeof(APP_HW_LED_Obj_g[0]));
    }

    return result;
}
