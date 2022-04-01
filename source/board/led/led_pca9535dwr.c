/*
 *  Copyright (C) 2021 Texas Instruments Incorporated
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <board/led.h>
#include <board/led/led_pca9535dwr.h>
#include <drivers/hw_include/csl_types.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* Sub address command as per PCA9535DWR datasheet */
#define PCA9535DWR_CMD_OUTPUT_PORT_0                (0x02U)
#define PCA9535DWR_CMD_OUTPUT_PORT_1                (0x03U)
#define PCA9535DWR_CMD_CONFIG_PORT_0                (0x06U)
#define PCA9535DWR_CMD_CONFIG_PORT_1                (0x07U)

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

LED_Attrs gLedAttrs_PCA9535DWR =
{
    .numLedPerGroup = 9U,
};

LED_Fxns gLedFxns_PCA9535DWR =
{
    .openFxn    = LED_pca9535dwrOpen,
    .closeFxn   = LED_pca9535dwrClose,
    .onFxn      = LED_pca9535dwrOn,
    .offFxn     = LED_pca9535dwrOff,
    .setMaskFxn = LED_pca9535dwrSetMask,
};

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

int32_t LED_pca9535dwrOpen(LED_Config *config, const LED_Params *params)
{
    int32_t         status = SystemP_SUCCESS;
    LED_Object     *object;
    LED_Attrs      *attrs;
    uint8_t         wrData[4U];
    I2C_Transaction i2cTransaction;

    if((NULL == config) || (NULL == params))
    {
        status = SystemP_FAILURE;
    }

    if(status == SystemP_SUCCESS)
    {
        object = (LED_Object *) config->object;
        object->gpioBaseAddr = 0U;  /* Not used */
        object->gpioPinNum   = 0U;  /* Not used */
        object->i2cInstance  = params->i2cInstance;
        object->i2cAddress   = params->i2cAddress;
        object->i2cHandle      = I2C_getHandle(object->i2cInstance);
        if(NULL == object->i2cHandle)
        {
            status = SystemP_FAILURE;
        }
    }

    if (status == SystemP_SUCCESS)
    {
       object      = (LED_Object *) config->object;
       attrs       = config->attrs;
    
        /* Configure all pins as outputs */
        wrData[0U] = PCA9535DWR_CMD_CONFIG_PORT_0;
        wrData[1U] = (uint8_t) 0U;
        wrData[2U] = PCA9535DWR_CMD_CONFIG_PORT_1;
        wrData[3U] = (uint8_t) 0U;

        /* send first cmd */
        I2C_Transaction_init(&i2cTransaction);
        i2cTransaction.writeBuf     = &wrData[0U];
        i2cTransaction.writeCount   = 2U;
        i2cTransaction.slaveAddress = object->i2cAddress;
        status = I2C_transfer(object->i2cHandle, &i2cTransaction);
    }

    if (status == SystemP_SUCCESS)
    {
        /* send 2nd cmd */
        I2C_Transaction_init(&i2cTransaction);
        i2cTransaction.writeBuf     = &wrData[2U];
        i2cTransaction.writeCount   = 2U;
        i2cTransaction.slaveAddress = object->i2cAddress;
        status = I2C_transfer(object->i2cHandle, &i2cTransaction);
    }


    return (status);
}

void LED_pca9535dwrClose(LED_Config *config)
{
    int32_t         status = SystemP_SUCCESS;
    LED_Object     *object;

    if(NULL == config)
    {
        status = SystemP_FAILURE;
    }

    if(status == SystemP_SUCCESS)
    {
        object = (LED_Object *) config->object;

        /* I2C Driver will be closed outside flash */
        object->i2cHandle = NULL;
    }

    return;
}

int32_t LED_pca9535dwrOn(LED_Config *config, uint32_t index)
{
    int32_t         status = SystemP_SUCCESS;
    LED_Object     *object;
    LED_Attrs      *attrs;
    uint8_t         rdData, wrData[2U];
    I2C_Transaction i2cTransaction;

    if(NULL == config)
    {
        status = SystemP_FAILURE;
    }

    /* command byte */
    if(status == SystemP_SUCCESS)
    {
        if (index <= 5)
        {
            wrData[0U] = PCA9535DWR_CMD_OUTPUT_PORT_0;
        } 

        else 
        {
            wrData[0U] = PCA9535DWR_CMD_OUTPUT_PORT_1;
        }

        object      = (LED_Object *) config->object;
        attrs       = config->attrs;

        I2C_Transaction_init(&i2cTransaction);
        i2cTransaction.writeBuf     = &wrData[0U];
        i2cTransaction.writeCount   = 1U;
        i2cTransaction.slaveAddress = object->i2cAddress;
        status = I2C_transfer(object->i2cHandle, &i2cTransaction);
    }

    /* read current state */
    if(status == SystemP_SUCCESS)
    {
        /* Read current state */
        I2C_Transaction_init(&i2cTransaction);
        i2cTransaction.readBuf      = &rdData;
        i2cTransaction.readCount    = 1U;
        i2cTransaction.slaveAddress = object->i2cAddress;
        status = I2C_transfer(object->i2cHandle, &i2cTransaction);
    }

    if(status == SystemP_SUCCESS)
    {
        object      = (LED_Object *) config->object;
        attrs       = config->attrs;

        /* Command LED */
        if (index <= 5)
        {
            wrData[0U] = PCA9535DWR_CMD_OUTPUT_PORT_0;
            wrData[1U] = (uint8_t) ((1U << index) | rdData);
        } 

        else 
        {
            wrData[0U] = PCA9535DWR_CMD_OUTPUT_PORT_1;
            wrData[1U] = (uint8_t) ((1U << (index - 6)) | rdData);
        }

        I2C_Transaction_init(&i2cTransaction);
        i2cTransaction.writeBuf     = &wrData[0U];
        i2cTransaction.writeCount   = 2U;
        i2cTransaction.slaveAddress = object->i2cAddress;
        status = I2C_transfer(object->i2cHandle, &i2cTransaction);
    }

    return (status);
}

int32_t LED_pca9535dwrOff(LED_Config *config, uint32_t index)
{
    int32_t         status = SystemP_SUCCESS;
        LED_Object     *object;
        LED_Attrs      *attrs;
        uint8_t         rdData, wrData[2U];
        I2C_Transaction i2cTransaction;

        if(NULL == config)
        {
            status = SystemP_FAILURE;
        }

        /* command byte */
        if(status == SystemP_SUCCESS)
        {
            if (index <= 5)
            {
                wrData[0U] = PCA9535DWR_CMD_OUTPUT_PORT_0;
            }

            else
            {
                wrData[0U] = PCA9535DWR_CMD_OUTPUT_PORT_1;
            }

            object      = (LED_Object *) config->object;
            attrs       = config->attrs;

            I2C_Transaction_init(&i2cTransaction);
            i2cTransaction.writeBuf     = &wrData[0U];
            i2cTransaction.writeCount   = 1U;
            i2cTransaction.slaveAddress = object->i2cAddress;
            status = I2C_transfer(object->i2cHandle, &i2cTransaction);
        }

        /* read current state */
        if(status == SystemP_SUCCESS)
        {
            /* Read current state */
            I2C_Transaction_init(&i2cTransaction);
            i2cTransaction.readBuf      = &rdData;
            i2cTransaction.readCount    = 1U;
            i2cTransaction.slaveAddress = object->i2cAddress;
            status = I2C_transfer(object->i2cHandle, &i2cTransaction);
        }
        
        if(status == SystemP_SUCCESS)
        {
            object      = (LED_Object *) config->object;
            attrs       = config->attrs;

            /* Command LED */
            if (index <= 5)
            {
                wrData[0U] = PCA9535DWR_CMD_OUTPUT_PORT_0;
                wrData[1U] = (uint8_t) (~(1U << index) & rdData);
            }

            else
            {
                wrData[0U] = PCA9535DWR_CMD_OUTPUT_PORT_1;
                wrData[1U] = (uint8_t) (~(1U << (index - 6)) & rdData);
            }

            I2C_Transaction_init(&i2cTransaction);
            i2cTransaction.writeBuf     = &wrData[0U];
            i2cTransaction.writeCount   = 2U;
            i2cTransaction.slaveAddress = object->i2cAddress;
            status = I2C_transfer(object->i2cHandle, &i2cTransaction);
        }

        return (status);
}

int32_t LED_pca9535dwrSetMask(LED_Config *config, uint32_t mask)
{
    int32_t         status = SystemP_SUCCESS;
    LED_Object     *object;
    LED_Attrs      *attrs;
    uint8_t         wrData[4U];
    I2C_Transaction i2cTransaction;
    DebugP_log("mask value: %d \r\n", mask);

    if(NULL == config)
    {
        status = SystemP_FAILURE;
    }

    else
    {
        object      = (LED_Object *) config->object;
        attrs       = config->attrs;

        wrData[0U] = PCA9535DWR_CMD_OUTPUT_PORT_0;
        wrData[1U] = (uint8_t) (mask & 0x3FU);
        wrData[2U] = PCA9535DWR_CMD_OUTPUT_PORT_1;
        wrData[3U] = (uint8_t) ((mask & 0x1C0U) >> 6);
        DebugP_log("data0: %d \r\n", wrData[1U]);
        DebugP_log("data1: %d \r\n", wrData[3U]);
        I2C_Transaction_init(&i2cTransaction);
        i2cTransaction.writeBuf     = &wrData[0U];
        i2cTransaction.writeCount   = 2U;
        i2cTransaction.slaveAddress = object->i2cAddress;
        status = I2C_transfer(object->i2cHandle, &i2cTransaction);
    }

    if (status == SystemP_SUCCESS)
    {
        /* send 2nd cmd */
        I2C_Transaction_init(&i2cTransaction);
        i2cTransaction.writeBuf     = &wrData[2U];
        i2cTransaction.writeCount   = 2U;
        i2cTransaction.slaveAddress = object->i2cAddress;
        status = I2C_transfer(object->i2cHandle, &i2cTransaction);
    }

    return (status);
}
