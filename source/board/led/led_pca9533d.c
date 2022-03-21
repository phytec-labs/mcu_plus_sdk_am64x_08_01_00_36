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
#include <board/led/led_pca9533d.h>
#include <drivers/hw_include/csl_types.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* Sub address command as per PCA9533D datasheet */
#define PCA9533D_LS0_REG                (0x05U)
#define PCA9533D_LEDON_CMD              (0x01U)    //LED3 = [7:6], LED2 = [5:4], LED1 = [3:2], LED0 = [1:0]

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

LED_Attrs gLedAttrs_PCA9533D =
{
    .numLedPerGroup = 3U,
};

LED_Fxns gLedFxns_PCA9533D =
{
    .openFxn    = LED_pca9533dOpen,
    .closeFxn   = LED_pca9533dClose,
    .onFxn      = LED_pca9533dOn,
    .offFxn     = LED_pca9533dOff,
    .setMaskFxn = LED_pca9533dSetMask,
};

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

int32_t LED_pca9533dOpen(LED_Config *config, const LED_Params *params)
{
    int32_t         status = SystemP_SUCCESS;
    LED_Object     *object;

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

    return (status);
}

void LED_pca9533dClose(LED_Config *config)
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

int32_t LED_pca9533dOn(LED_Config *config, uint32_t index)
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

    else
    {
        object      = (LED_Object *) config->object;
        attrs       = config->attrs;

        /* Set LED  command */
        wrData[0U] = PCA9533D_LS0_REG;
        I2C_Transaction_init(&i2cTransaction);
        i2cTransaction.writeBuf     = &wrData[0U];
        i2cTransaction.writeCount   = 1U;
        i2cTransaction.slaveAddress = object->i2cAddress;
        status = I2C_transfer(object->i2cHandle, &i2cTransaction);
    }

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
        /* Set Mask */
        wrData[0U] = PCA9533D_LS0_REG;
        wrData[1U] = (uint8_t) ((PCA9533D_LEDON_CMD << index*2) | rdData);
        I2C_Transaction_init(&i2cTransaction);
        i2cTransaction.writeBuf     = &wrData[0U];
        i2cTransaction.writeCount   = 2U;
        i2cTransaction.slaveAddress = object->i2cAddress;
        status = I2C_transfer(object->i2cHandle, &i2cTransaction);
    }

    return (status);
}

int32_t LED_pca9533dOff(LED_Config *config, uint32_t index)
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

    else
    {
        object      = (LED_Object *) config->object;
        attrs       = config->attrs;

        /* Set LED  command */
        wrData[0U] = PCA9533D_LS0_REG;
        I2C_Transaction_init(&i2cTransaction);
        i2cTransaction.writeBuf     = &wrData[0U];
        i2cTransaction.writeCount   = 1U;
        i2cTransaction.slaveAddress = object->i2cAddress;
        status = I2C_transfer(object->i2cHandle, &i2cTransaction);
    }

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
        /* Set Mask */
        wrData[0U] = PCA9533D_LS0_REG;
        wrData[1U] = (uint8_t) (~(PCA9533D_LEDON_CMD << index*2) & rdData);
        I2C_Transaction_init(&i2cTransaction);
        i2cTransaction.writeBuf     = &wrData[0U];
        i2cTransaction.writeCount   = 2U;
        i2cTransaction.slaveAddress = object->i2cAddress;
        status = I2C_transfer(object->i2cHandle, &i2cTransaction);
    }

    return (status);
}

int32_t LED_pca9533dSetMask(LED_Config *config, uint32_t mask)
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

    else
    {
        object      = (LED_Object *) config->object;
        attrs       = config->attrs;

        /* Set LED  command */
        wrData[0U] = PCA9533D_LS0_REG;
        I2C_Transaction_init(&i2cTransaction);
        i2cTransaction.writeBuf     = &wrData[0U];
        i2cTransaction.writeCount   = 1U;
        i2cTransaction.slaveAddress = object->i2cAddress;
        status = I2C_transfer(object->i2cHandle, &i2cTransaction);
    }

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
        /* Set Mask */
        wrData[0U] = PCA9533D_LS0_REG;
        wrData[1U] = (uint8_t) (mask & 0x55U);
        I2C_Transaction_init(&i2cTransaction);
        i2cTransaction.writeBuf     = &wrData[0U];
        i2cTransaction.writeCount   = 2U;
        i2cTransaction.slaveAddress = object->i2cAddress;
        status = I2C_transfer(object->i2cHandle, &i2cTransaction);
    }

    return (status);
}
