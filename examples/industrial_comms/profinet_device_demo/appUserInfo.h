/*
 *  Copyright (c) 2021, Kunbus GmbH
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice, this
 *     list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *
 *  3. Neither the name of the copyright holder nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.
 *
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

#ifndef _APPUSERINFO_H_
#define _APPUSERINFO_H_


#include <stdint.h>
#include <stdbool.h>
#include <PN_API_ETH.h>
#include <PN_API_SNMP.h>
#include <PN_API_DEV.h>

#define PNC_VENDOR_ID                   0x01c4                  // Texas Instruments
#define PNC_DEVICE_ID                   0x6401                  // AM64x Example Project on EVM
#define PNC_DEVICE_DESCRIPTION          "TI PNIOD CC-B"         // Caution! Maximal length is 25 characters
#define PNC_DEVICE_ORDER_ID             "100074"                // Default Order ID
#define PNC_DEVICE_SW_REV_PREFIX        'V'                     // 'V' - officially released version     
//   'V':  officially released version
//   'R':  Revision (of a virtual or physically modular product)
//   'P':  Prototype
//   'U':  Under Test (field test)
//   'T':  Test Device 

#define PNC_DEVICE_SERIAL_NUMBER                123456
#define PNC_DEVICE_HARDWARE_REVISION            1
#define PNC_DEVICE_SW_REV_FUNC_ENHANCEMENT      0
#define PNC_DEVICE_SW_REV_BUG_FIX               1
#define PNC_DEVICE_SW_REV_INTERNAL_CHANGE       2

#define PNC_MAC_ADDR_LEN                        6

#define PNC_REVISION_COUNTER                    0

#define PNC_IF_DESCRIPTION              "Port 0"
#define PNC_PORT_1_DESCRIPTION          "Port 1"
#define PNC_PORT_2_DESCRIPTION          "Port 2"

#define CYC_DATA_OUTPUT_LEN             1440
#define CYC_DATA_INPUT_LEN              1440


#ifdef __cplusplus
extern "C" {
#endif

    uint8_t *APP_UI_getMacAddr(void);
    void APP_UI_cbErrorHandler(uint32_t errorCode_p, bool fatal_p, uint8_t paraCnt_p, va_list argptr_p);
    void APP_UI_cbGetLocalTime(uint32_t* pTimeHigh_p, uint32_t* pTimeLow_p);
    void APP_UI_cbGetServerBootTime(uint32_t* pTime_p);
    void APP_UI_cbSavePermanentData(void);
    void APP_UI_cbSignalLinkLed(bool state_p);
    void APP_UI_cbSetStationName(const uint8_t *pName_p, uint16_t length_p);
    void APP_UI_cbSetIpAddress(uint32_t ipAddress_p, uint32_t subNetMask_p, uint32_t gateWayAddress_p);
    void APP_UI_cbFactoryReset(void);
    bool APP_UI_cbResetToFactory(PN_API_ETH_EDcpResetToFactoryQualifier_t qualifier_p);
    void APP_UI_cbWritePortDataCheck(PN_API_ETH_EPort_t port_p, const PN_API_ETH_SPortDataCheck_t *pData_p);
    void APP_UI_cbWritePortDataAdjust(PN_API_ETH_EPort_t port_p, const PN_API_ETH_SPortDataAdjust_t *pData_p);
    void APP_UI_cbSetPermanentSnmpData(const PN_API_SNMP_SPermanentData_t *pData_p);
    uint32_t APP_UI_getSerialNumber(void);
    void APP_UI_getSerialNumberString(uint8_t *pBuffer_p, uint16_t length_p);
    uint16_t APP_UI_getHardwareRevision(void);
    void APP_UI_getHardwareRevisionString(uint8_t *pBuffer_p, uint16_t length_p);
    uint8_t APP_UI_getSwRevFuncEnhancement(void);
    void APP_UI_getSwRevFuncEnhancementString(uint8_t *pBuffer_p, uint16_t length_p);
    uint8_t APP_UI_getSwRevBugFix(void);
    void APP_UI_getSwRevBugFixString(uint8_t *pBuffer_p, uint16_t length_p);
    uint8_t APP_UI_getSwRevInternalChange(void);
    void APP_UI_getSwRevInternalChangeString(uint8_t *pBuffer_p, uint16_t length_p);
    uint16_t APP_UI_getDeviceRevCounter(void);

    extern uint8_t aCycDataInput_g[];
    extern uint8_t aCycDataOutput_g[];

#ifdef __cplusplus
}
#endif

#endif  // _APPUSERINFO_H_
