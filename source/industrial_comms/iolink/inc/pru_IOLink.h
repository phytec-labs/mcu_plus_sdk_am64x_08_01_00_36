/*!
* \file pru_IOLink.h
*
* \brief
* PRU Integration: IOLink specific interface.
*
* \author
* KUNBUS GmbH
*
* \date
* 2021-05-20
*
* \copyright
* Copyright (c) 2021, KUNBUS GmbH<br /><br />
* All rights reserved.<br />
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:<br />
* <ol>
* <li>Redistributions of source code must retain the above copyright notice, this
* list of conditions and the following disclaimer.</li>
* <li>Redistributions in binary form must reproduce the above copyright notice,
* this list of conditions and the following disclaimer in the documentation
* and/or other materials provided with the distribution.</li>
* <li>Neither the name of the copyright holder nor the names of its
* contributors may be used to endorse or promote products derived from
* this software without specific prior written permission.</li>
* </ol>
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*/

#if !(defined __PRU_IOLINK_H__)
#define __PRU_IOLINK_H__		1

#include <stdint.h>
#include <stdbool.h>

#include <IOLM_Types.h>

#if   defined (SOC_K2G)
#define ARM_INTERRUPT_OFFSET_ICSS0 236
#define ARM_INTERRUPT_OFFSET_ICSS1 244
#else
#if defined (__aarch64__)
 /* A53 */
#define ARM_INTERRUPT_OFFSET_ICSS0 (286-20)
#define ARM_INTERRUPT_OFFSET_ICSS1 (294-20)
#define ARM_INTERRUPT_OFFSET_ICSS2 (302-20)
#else
 /* R5F */
#define ARM_INTERRUPT_OFFSET_ICSS0 (120-20)
#define ARM_INTERRUPT_OFFSET_ICSS1 (248-20)
#endif
#endif

#define MCU_getARMInterruptOffset(pruInst) \
    ( (PRUICSS_INSTANCE_ONE == (pruInst))?((int16_t)ARM_INTERRUPT_OFFSET_ICSS0):((int16_t)ARM_INTERRUPT_OFFSET_ICSS1) )

// structures

typedef enum IOLM_PL_EPortMode {
    IOLM_PL_ModeSioInactive = 0,
    IOLM_PL_ModeSdci,
    IOLM_PL_ModeSioDI,
    IOLM_PL_ModeSioDO,
} IOLM_PL_EPortMode_t;


typedef struct IOLM_PL_GpioConfig
{
    uint32_t gpioBase;
    uint32_t gpioPin;
    uint32_t ctlRegOffset;
} IOLM_PL_GpioConfig_t;

typedef struct IOLM_PL_GpioTxConfig
{
    IOLM_PL_GpioConfig_t gpio;
    uint32_t pruPin; // pru pin number
} IOLM_PL_GpioTxConfig_t;

typedef struct IOLM_PL_GpioEnConfig
{
    IOLM_PL_GpioConfig_t gpio;
    uint32_t gpioPhysAddr; // physical address for pru to access gpio
    uint32_t gpioPinMask; // mask to write on gpioPhysAddr
} IOLM_PL_GpioEnConfig_t;


typedef struct IOLM_PL_PortConfig
{
    IOLM_PL_GpioConfig_t rx;
    IOLM_PL_GpioTxConfig_t tx;
    IOLM_PL_GpioEnConfig_t txEn;
    IOLM_PL_GpioConfig_t pwrEn;
} IOLM_PL_PortConfig_t;

typedef struct IOLM_PL_EventIrq
{
    uint32_t pruIrqNum;
    uint32_t pruIrqPrio;
}IOLM_PL_EventIrq_t;

typedef struct IOLM_PL_PruIccsCfg
{
    uint32_t pruIcssInst;
    IOLM_PL_EventIrq_t eventIrq[8];
}IOLM_PL_PruIccsCfg_t;



typedef void (*IOLM_PL_PFUsetMode)(uint8_t instance_p, uint8_t portNum_p, IOLM_PL_EPortMode_t mode_p);
typedef const IOLM_PL_PortConfig_t* (*IOLM_PL_PFUgetPortCfg)(uint8_t instance_p, uint8_t portNum_p);
typedef const IOLM_PL_PruIccsCfg_t* (*IOLM_PL_PFUgetPruCfg)(uint8_t instance_p);
typedef void (*IOLM_PL_PFUsetDO)(uint8_t instance_p, uint8_t portNum_p, bool boPortTargetState_p);
typedef bool (*IOLM_PL_PFUgetDI)(uint8_t instance_p, uint8_t portNum_p);
typedef void (*IOLM_PL_PFUsetPower)(uint8_t instance_p, uint8_t portNum_p, bool powerState_p);
typedef void (*IOLM_PL_PFUSetIQ)(uint8_t instance_p, uint8_t portNum_p, bool boOutValue_p);
typedef bool (*IOLM_PL_PFUGetIQ)(uint8_t instance_p, uint8_t portNum_p);
typedef void (*IOLM_PL_PFUSetIQMode)(uint8_t instance_p, uint8_t portNum_p, IOL_EIQMode eIQMode_p);



typedef struct IOLM_PL_Callbacks
{
    IOLM_PL_PFUsetMode cbSetMode;
    IOLM_PL_PFUgetPortCfg cbGetPortCfg;
    IOLM_PL_PFUgetPruCfg cbGetPruCfg;
    IOLM_PL_PFUsetDO cbSetDo;
    IOLM_PL_PFUgetDI cbGetDi;
    IOLM_PL_PFUsetPower cbSetPower;
    IOLM_PL_PFUSetIQ cbSetIQ;
    IOLM_PL_PFUGetIQ cbGetIQ;
    IOLM_PL_PFUSetIQMode cbSetIQMode;
}IOLM_PL_Callbacks_t;



void PRU_IOLM_registerSetModeCallback(IOLM_PL_PFUsetMode pCallback_p);
void PRU_IOLM_registerGetPortCfgCallback(IOLM_PL_PFUgetPortCfg pCallback_p);
void PRU_IOLM_registerGetPruCfgCallback(IOLM_PL_PFUgetPruCfg pCallback_p);
void PRU_IOLM_registerSetDoCallback(IOLM_PL_PFUsetDO pCallback_p);
void PRU_IOLM_registerGetDiCallback(IOLM_PL_PFUgetDI pCallback_p);
void PRU_IOLM_registerSetPowerCallback(IOLM_PL_PFUsetPower pCallback_p);
void PRU_IOLM_registerSetIQCallback(IOLM_PL_PFUSetIQ pCallback_p);
void PRU_IOLM_registerGetIQCallback(IOLM_PL_PFUGetIQ pCallback_p);
void PRU_IOLM_registerSetIQModeCallback(IOLM_PL_PFUSetIQMode pCallback_p);

void PRU_IOLM_Init(uint8_t instance_p);

#endif /* __PRU_IOLINK_H__ */
