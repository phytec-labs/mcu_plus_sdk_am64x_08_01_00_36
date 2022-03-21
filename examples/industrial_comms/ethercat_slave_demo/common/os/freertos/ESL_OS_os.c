/*!
 *  \file ESL_OS_os.c
 *
 *  \brief
 *  Application OS support FreeRTOS.
 *
 *  \author
 *  KUNBUS GmbH
 *
 *  \date
 *  2021-05-18
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

#include <osal.h>
#include <ESL_os.h>

typedef struct ESL_OS_I2C_SHandle
{
#if !(defined FBTLPROVIDER) || (FBTLPROVIDER==0)
    LED_Handle      ledHandle;
#endif
    uint32_t        numLedPerGroup;
} ESL_OS_I2C_SHandle_t;

StackType_t EC_SLV_APP_mainTaskStack_g[MAIN_TASK_SIZE] __attribute__((aligned(32), section(".stack"))) = {0};
StackType_t EC_SLV_APP_applLoopTaskStack_g[APPLLOOP_TASK_SIZE] __attribute__((aligned(32), section(".stack"))) = {0};

#if ((defined FBTLPROVIDER) && (1==FBTLPROVIDER)) || ((defined FBTL_REMOTE) && (1==FBTL_REMOTE))
StackType_t SYSLIB_fbtlCyclicTaskStack_g[FBTLCYCLIC_TASK_SIZE] __attribute__((aligned(32), section(".stack"))) = {0};
StackType_t SYSLIB_fbtlReceiverTaskStack_g[FBTLRECEIVER_TASK_SIZE] __attribute__((aligned(32), section(".stack"))) = {0};
#endif

#if (defined FBTL_REMOTE) && (1==FBTL_REMOTE)
StackType_t EC_SLV_APP_syncIstTaskStack_g[SYNCIST_TASK_SIZE] __attribute__((aligned(32), section(".stack"))) = {0};
StackType_t EC_SLV_APP_ledIstTaskStack_g[LEDIST_TASK_SIZE] __attribute__((aligned(32), section(".stack"))) = {0};
#endif

#if (defined DPRAM_REMOTE)
StackType_t EC_SLV_APP_appRunWrapTaskStack_g[APPRWRAP_TASK_SIZE] __attribute__((aligned(32), section(".stack"))) = {0};
#endif

/*
 * Mux and initialize the ethernet reference clock used in PHYTEC's phyCORE-AM64x design
 *
 * sysconfig doesn't support the configuration of this pin for some reason so have to
 * manually set it up like this for now, bypassing sysconfig.
 */

//Pinger carrier board
//static Pinmux_PerCfg_t My_gPinMuxMainDomainCfg[] = {
//    {
//        PIN_EXT_REFCLK1, ( PIN_MODE(5) | PIN_PULL_DISABLE )
//    },
//    {PINMUX_END, PINMUX_END}
//};
//
//void EthRefCLK_init(void)
//{
//    Pinmux_config(My_gPinMuxMainDomainCfg, PINMUX_DOMAIN_ID_MAIN);
//}

/*!
 *  <!-- Description: -->
 *
 *  \brief
 *  Initialize application OS module
 *
 *  <!-- Example: -->
 *
 *  \par Example
 *  \code{.c}
 *  #include <ESL_OS_os.h>
 *
 *  // the Call
 *  ESL_OS_init();
 *  \endcode
 *
 *  <!-- Group: -->
 *
 *  \ingroup ESL_OS
 *
 * */
void ESL_OS_init(void)
{
    System_init();
    //EthRefCLK_init();
    return;
}

/*!
 *  <!-- Description: -->
 *
 *  \brief
 *  Board OS dependent initialization
 *
 *  <!-- Parameters and return values: -->
 *
 *  \param[in]  pruInstance_p   selected PRU instance.
 *  \return     ErrorCode
 *
 *  <!-- Example: -->
 *
 *  \par Example
 *  \code{.c}
 *
 *  // required variables
 *  uint32_t retVal = 0;
 *
 *  // the Call
 *  retVal = ESL_OS_boardInit(PRUICSS_INSTANCE_ONE);
 *  \endcode
 *
 *  <!-- Group: -->
 *
 *  \ingroup ESL_OS
 *
 * */
uint32_t ESL_OS_boardInit(uint32_t pruInstance_p)
{
    uint32_t retVal = OSAL_eERR_EINVAL;

    Board_init();
    Drivers_open();
    if (SystemP_SUCCESS != Board_driversOpen())
    {
        retVal = OSAL_eERR_ENOENT;
        goto Exit;
    }

    retVal = OSAL_eERR_NOERROR;
Exit:
    return retVal;
}

/*!
 *  <!-- Description: -->
 *
 *  \brief
 *  Board OS dependent deinitialization
 *
 *  <!-- Example: -->
 *
 *  \par Example
 *  \code{.c}
 *
 *  // required variables
 *
 *  // the Call
 *  ESL_OS_boardInit();
 *  \endcode
 *
 *  <!-- Group: -->
 *
 *  \ingroup ESL_OS
 *
 * */
void ESL_OS_boardDeinit(void)
{
    Drivers_close();
    vTaskDelete(NULL);
    return;
}

/*!
 *  <!-- Description: -->
 *
 *  \brief
 *  OS Leave task
 *
 *  <!-- Example: -->
 *
 *  \par Example
 *  \code{.c}
 *
 *  // the Call
 *  ESL_OS_taskLeave();
 *  \endcode
 *
 *  <!-- Group: -->
 *
 *  \ingroup ESL_OS
 *
 * */
void ESL_OS_taskLeave(void)
{
    vTaskDelete(NULL);
}

/*!
 *  <!-- Description: -->
 *
 *  \brief
 *  OS ms clock retrieve
 *
 *  <!-- Example: -->
 *
 *  \par Example
 *  \code{.c}
 *
 *  // variables
 *  clock_t ts;
 *
 *  // the Call
 *  ts = ESL_OS_clockGet();
 *  \endcode
 *
 *  <!-- Group: -->
 *
 *  \ingroup ESL_OS
 *
 * */
clock_t ESL_OS_clockGet(void)
{
    return (ClockP_getTimeUsec()/1000000);
}

/*!
 *  <!-- Description: -->
 *
 *  \brief
 *  Calculate time delta
 *
 *  <!-- Parameters and return values: -->
 *
 *  \param[in]  reference_p		reference time
 *  \param[in]  pNow_p          current time (if NULL-&gt;now)
 *  \return     clock_t         time delta
 *
 *  \remarks
 *
 *  <!-- Example: -->
 *
 *  \par Example
 *  \code{.c}
 *  // required variables
 *  clock_t delta;
 *
 *  // the Call
 *  retVal = ESL_OS_clockDiff(ESL_OS_clockGet(), NULL);
 *  \endcode
 *
 *  <!-- References: -->
 *
 *  <!-- Group: -->
 *
 *  \ingroup ESL_OS
 *
 * */
clock_t  ESL_OS_clockDiff(clock_t reference_p, clock_t* pNow_p)
{
    clock_t delta;
    clock_t current;

    if (NULL != pNow_p)
    {
        current = pNow_p[0];
    }
    else
    {
        current = ClockP_getTimeUsec()/1000000;
    }

    delta = (current - reference_p);

    return delta;
}

static uint32_t Board_getnumLedPerGroup(void)
{
    uint32_t            ledCount;
#if !(defined FBTLPROVIDER) || (FBTLPROVIDER==0)
    const LED_Attrs    *attrs;

    attrs = LED_getAttrs(CONFIG_LED0);
    DebugP_assert(NULL != attrs);
    /* For AM64x-EVM, AM243x-EVM and AM243x-LP all LEDs are connected, so return
     * the driver attributes value of 8 */
    ledCount = attrs->numLedPerGroup;
#else
    ledCount = 0;
#endif

    return (ledCount);
}

/*!
 *  <!-- Description: -->
 *
 *  \brief
 *  Initialize I2C IOexpander LEDs (PD)
 *
 *  <!-- Parameters and return values: -->
 *
 *  \return     IO expander handle.
 *
 *  <!-- Example: -->
 *
 *  \par Example
 *  \code{.c}
 *  // required variables
 *  void* pvHandle = NULL;
 *
 *  // the Call
 *  pvHandle = ESL_OS_ioexp_leds_init();
 *  \endcode
 *
 *  <!-- Group: -->
 *
 *  \ingroup ESL_OS
 *
 * */
void* ESL_OS_ioexp_leds_init(void)
{
    ESL_OS_I2C_SHandle_t*   pHandle = NULL;
    int32_t                 status;

    pHandle = (ESL_OS_I2C_SHandle_t*)OSAL_MEMORY_calloc(1, sizeof(ESL_OS_I2C_SHandle_t));
    if (!pHandle)
    {
        goto Exit;
    }

    OSAL_MEMORY_memset(pHandle, 0, sizeof(ESL_OS_I2C_SHandle_t));
#if !(defined FBTLPROVIDER) || (FBTLPROVIDER==0)
    /* already done by drivers_open */
    pHandle->ledHandle = gLedHandle[CONFIG_LED0];
    pHandle->numLedPerGroup = Board_getnumLedPerGroup();

    if(pHandle->numLedPerGroup > 1U)
    {
        status = LED_setMask(pHandle->ledHandle, 0x0U);
    }
    else
    {
        status = LED_off(pHandle->ledHandle, 0);
    }
#endif

    if (SystemP_SUCCESS != status)
    {
        OSAL_MEMORY_free(pHandle);
        goto Exit;
    }

Exit:
    return pHandle;
}

/*!
 *  <!-- Description: -->
 *
 *  \brief
 *  Write value to I2C IOexpander LEDs (PD)
 *
 *  <!-- Parameters and return values: -->
 *
 *  \param[in]  pI2cHandle_p    I2C handle
 *  \param[in]  ledValue_p      LED register value
 *
 *  <!-- Example: -->
 *
 *  \par Example
 *  \code{.c}
 *  // required variables
 *  void* pI2cHandle = NULL;
 *
 *  // the Call
 *  ESL_OS_ioexp_leds_write(pI2cHandle, 0xa5);
 *  \endcode
 *
 *  <!-- Group: -->
 *
 *  \ingroup ESL_OS
 *
 * */
void ESL_OS_ioexp_leds_write(void* pI2cHandle_p, uint8_t ledValue_p)
{
    ESL_OS_I2C_SHandle_t*   pHandle         = (ESL_OS_I2C_SHandle_t*)pI2cHandle_p;
    int32_t                 status;

    if (!pHandle)
    {
        goto Exit;
    }

#if !(defined FBTLPROVIDER) || (FBTLPROVIDER==0)
    if(pHandle->numLedPerGroup > 1U)
    {
        status = LED_setMask(pHandle->ledHandle, ledValue_p);
    }
    else
    {
        if (0 < ledValue_p)
        {
            status = LED_on(pHandle->ledHandle, 0);
        }
        else
        {
            status = LED_off(pHandle->ledHandle, 0);
        }
    }
#endif

    if (SystemP_SUCCESS != status)
    {
        OSAL_MEMORY_free(pHandle);
        goto Exit;
    }

Exit:
    return;
}

//*************************************************************************************************
