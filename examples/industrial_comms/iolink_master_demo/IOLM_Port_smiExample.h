/*!
* \file IOLM_Port_smiExample.h
*
* \brief
* Example application to show how the IO-Link Master Stack can be used via the SMI.
*
* \author
* KUNBUS GmbH
*
* \date
* 2021-10-06
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

#if !(defined __IOLM_PORT_SMI_EXMPL_H__)
#define __IOLM_PORT_SMI_EXMPL_H__       1

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <osal.h>

#include <IOLM_SMI.h>

#include "IOLinkPort/IOLM_Port_LEDTask.h"

/* IO-Link Master example setup */
#warning IOLM Example: Please set these following definitions according to your desired application!
/* Switch to enable the automatic setup, which is done in this example. */
#define IOLM_EXMPL_ENABLE_AUTO_SETUP        (0)
/* Number of ports used in this example state machine */
#define IOLM_EXMPL_PORTS_USED               (8)
/* Switch to enable printf output. Use with caution! */
#define IOLM_EXMPL_ENABLE_PRINTF            (0)

/* Check if printf should be enabled */
#if (defined IOLM_EXMPL_ENABLE_PRINTF) && (IOLM_EXMPL_ENABLE_PRINTF == 1)
#define IOLM_EXMPL_printf           OSAL_printf
#warning printf should not be used in production and only works with one active port!
/* Set active port count to 1, because more printf outputs will cause major performance issues */
#undef  IOLM_EXMPL_PORTS_USED
#define IOLM_EXMPL_PORTS_USED               (1)
#else
#define IOLM_EXMPL_printf(...)
#endif

void IOLM_EXMPL_init(void);
void IOLM_EXMPL_mainLoop(void);
void IOLM_EXMPL_stateMachine(void);

/* Prototype callbacks */
/* Generic channel for SMI over UART */
void IOLM_EXMPL_cbGenericCnf(IOLM_SMI_SHeader* psuHeader_p, uint8_t* pu8ArgBlock_p);
/* Master Identification */
void IOLM_EXMPL_cbMasterIdentificationCnf(uint8_t clientID_p, uint16_t error_p, uint16_t argBlockLength_p, uint8_t* pArgBlock_p);
/* Event handling(acyclic) */
void IOLM_EXMPL_PortEventInd(uint8_t port_p, uint16_t argBlockLength_p, uint8_t* pArgBlock_p);
void IOLM_EXMPL_DeviceEventInd(uint8_t port_p, uint16_t argBlockLength_p, uint8_t* pArgBlock_p);
/* Communication establishment */
void IOLM_EXMPL_cbPortConfigurationCnf(uint8_t clientID_p, uint8_t port_p, uint16_t error_p);
void IOLM_EXMPL_cbPortStatusCnf(uint8_t clientID_p, uint8_t u8Port_p, uint16_t error_p, uint16_t argBlockLength_p, uint8_t* pArgBlock_p);
/* Acyclic communication */
void IOLM_SMI_cbDeviceWriteCnf(uint8_t clientID_p, uint8_t port_p, uint16_t error_p);
void IOLM_EXMPL_cbDeviceReadCnf(uint8_t clientID_p, uint8_t port_p, uint16_t error_p, uint16_t argBlockLength_p, uint8_t* pArgBlock_p);
/* Cyclic communication */
void IOLM_EXMPL_cbPDInCnf(uint8_t clientID_p, uint8_t u8Port_p, uint16_t error_p, uint16_t argBlockLength_p, uint8_t* pArgBlock_p);
void IOLM_EXMPL_cbPDOutCnf(uint8_t clientID_p, uint8_t port_p, uint16_t error_p);

/* Prototype no callbacks */
uint8_t IOLM_EXMPL_PortErrorHandler(uint8_t port_p, uint16_t error_p);
#define PD_INPUT_LENGTH (32)
#define DEVICE_READ_DATA_LENGTH (65)
typedef enum IOLM_EXMPL_EExampleState
{
    /*---Establishment of the COMMUNICATION---*/
    IOLM_eExampleState_NotUsed,
    IOLM_eExampleState_Init,
    IOLM_eExampleState_Config,
    IOLM_eExampleState_ConfigWait,
    IOLM_eExampleState_PortStatusRequest,
    IOLM_eExampleState_PortStatusWait,
    /*--------------COMMUNICATION-------------*/
    IOLM_eExampleState_ReadVendor,
    IOLM_eExampleState_ReadVendorWait,
    IOLM_eExampleState_ReadProductName,
    IOLM_eExampleState_ReadProductNameWait,
    IOLM_eExampleState_ReadSerialnumber,
    IOLM_eExampleState_ReadSerialnumberWait,
    IOLM_eExampleState_WriteProcessDataValue,
    IOLM_eExampleState_WriteProcessDataValueWait,
    /*-----Read process data COMMUNICATION----*/
    IOLM_eExampleState_PortStatusRequestPD,
    IOLM_eExampleState_PortStatusWaitPD,
    IOLM_eExampleState_ReadProcessDataValue,
    IOLM_eExampleState_ReadProcessDataValueWait,

} IOLM_EXMPL_EExampleState_t;

typedef struct IOLM_EXMPL_SPortDataValues
{
    IOLM_EXMPL_EExampleState_t exampleState;
    uint8_t currentStackPortStatus;
    uint8_t aPDInCnfData[PD_INPUT_LENGTH];

} IOLM_EXMPL_SPortDataValues_t;

#endif
