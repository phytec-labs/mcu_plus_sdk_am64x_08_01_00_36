/*!
 *  \example <PN_API_SNMP>.h
 *
 *  \brief
 *  Profinet API for SNMP stack.
 *
 *  \author
 *  KUNBUS GmbH
 *
 *  \date
 *  2020-11-11
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

#ifndef _PN_API_SNMP_H_
#define _PN_API_SNMP_H_

#ifdef __cplusplus
extern "C" {
#endif

#define PN_API_SNMP_MAX_PORT_DESCRIPTION_LENGTH         20
#define PN_API_SNMP_MAX_LLDP_XDOT1_PROTO_VLAN            1
#define PN_API_SNMP_MAX_LLDP_XDOT1_VLAN_NAME             1
#define PN_API_SNMP_MAX_LLDP_XDOT1_PROTOCOL              1
#define PN_API_SNMP_MAX_SNMP_SYSTEM_VALUE_BUFFER       255

/*!
 *  \brief
 *  SNMP stack error codes.
 *  \ingroup PN_API_SNMP_ERROR_CODES
 */
typedef enum PN_API_SNMP_EError
{
    PN_API_SNMP_eOK                              = 0x00000000,
    PN_API_SNMP_eERROR_WRONG_STATE               = 0x38030501,
    PN_API_SNMP_eERROR_MISSING_PARAMETER_POINTER = 0x38030502,
    PN_API_SNMP_eERROR_PARAMETER_OUT_OF_RANGE    = 0x38030503
} PN_API_SNMP_EError_t;

typedef enum PN_API_SNMP_EBool
{
    PN_API_SNMP_eBOOL_UNKNOWN,
    PN_API_SNMP_eBOOL_TRUE,
    PN_API_SNMP_eBOOL_FALSE,
    PN_API_SNMP_eBOOL_FORCE32BIT = 0xffffffff //!< Force enum to 32 bit
} PN_API_SNMP_EBool_t;

typedef enum PN_API_SNMP_EPortConfigAdminStatus
{
    PN_API_SNMP_eAS_UNKNOWN,
    PN_API_SNMP_eAS_TX_ONLY,
    PN_API_SNMP_eAS_RX_ONLY,
    PN_API_SNMP_eAS_TX_AND_RX,
    PN_API_SNMP_eAS_DISABLED,
    PN_API_SNMP_eAS_FORCE32BIT = 0xffffffff //!< Force enum to 32 bit
} PN_API_SNMP_EPortConfigAdminStatus_t;

typedef enum PN_API_SNMP_ELldpPortConfigTlvTxEnable
{
    PN_API_SNMP_eLLDP_TX_ENABLE_NIL = 0,
    PN_API_SNMP_eLLDP_TX_ENABLE_PORT_DESC = 0x80,
    PN_API_SNMP_eLLDP_TX_ENABLE_SYS_NAME = 0x40,
    PN_API_SNMP_eLLDP_TX_ENABLE_SYS_DESC = 0x20,
    PN_API_SNMP_eLLDP_TX_ENABLE_SYS_CAP = 0x10,
    PN_API_SNMP_eLLDP_TX_ENABLE_FORCE32BIT = 0xffffffff //!< Force enum to 32 bit
} PN_API_SNMP_ELldpPortConfigTlvTxEnable_t;

typedef enum PN_API_SNMP_ELldpXdot3TlvTx
{
    PN_API_SNMP_eLLDP_TX_NIL = 0,
    PN_API_SNMP_eLLDP_TX_MAC_PHY_CONFIG_STATUS = 0x80,
    PN_API_SNMP_eLLDP_TX_POWER_VIA_MDI = 0x40,
    PN_API_SNMP_eLLDP_TX_LINK_AGGREGATION = 0x20,
    PN_API_SNMP_eLLDP_TX_MAX_FRAME_SIZE = 0x10,
    PN_API_SNMP_eLLDP_TX_FORCE32BIT = 0xffffffff //!< Force enum to 32 bit
} PN_API_SNMP_ELldpXdot3TlvTx_t;

typedef enum PN_API_SNMP_ELldpPortEnable
{
    PN_API_SNMP_eLLDP_PORT_NIL_ENABLE = 0,
    PN_API_SNMP_eLLDP_PORT_1_ENABLE = 0x80,
    PN_API_SNMP_eLLDP_PORT_2_ENABLE = 0x40,
    PN_API_SNMP_eLLDP_PORT_ENABLE_FORCE32BIT = 0xffffffff //!< Force enum to 32 bit
} PN_API_SNMP_ELldpPortEnable_t;

typedef struct PN_API_SNMP_SPort
{
    uint8_t lldpPortConfigAdminStatus;
    uint8_t lldpPortConfigNotificationEnable;
    uint8_t lldpPortConfigTLVsTxEnable;
    uint8_t lldpXPnoConfigSPDTxEnable;
    uint8_t lldpXPnoConfigPortStatusTxEnable;
    uint8_t lldpXPnoConfigAliasTxEnable;
    uint8_t lldpXPnoConfigMrpTxEnable;
    uint8_t lldpXPnoConfigPtcpTxEnable;
    uint8_t lldpXdot3PortConfigTLVsTxEnable;
    uint8_t lldpXdot1ConfigPortVlanTxEnable;
    uint8_t aLldpXdot1ConfigVlanNameTxEnable[PN_API_SNMP_MAX_LLDP_XDOT1_VLAN_NAME];
    uint8_t aLldpXdot1ConfigProtoVlanTxEnable[PN_API_SNMP_MAX_LLDP_XDOT1_PROTO_VLAN];
    uint8_t aLldpXdot1ConfigProtocolTxEnable[PN_API_SNMP_MAX_LLDP_XDOT1_PROTOCOL];
} PN_API_SNMP_SPort_t;

typedef struct PN_API_SNMP_SPermanentData
{
    uint16_t sysContactLength;
    uint8_t aSysContact[PN_API_SNMP_MAX_SNMP_SYSTEM_VALUE_BUFFER];
    uint16_t sysNameLength;
    uint8_t aSysName[PN_API_SNMP_MAX_SNMP_SYSTEM_VALUE_BUFFER];
    uint16_t sysLocationLength;
    uint8_t aSysLocation[PN_API_SNMP_MAX_SNMP_SYSTEM_VALUE_BUFFER];
    uint16_t lldpMessageTxInterval;
    uint8_t lldpMessageTxHoldMultiplier;
    uint8_t lldpReinitDelay;
    uint16_t lldpTxDelay;
    uint16_t lldpNotificationInterval;
    PN_API_SNMP_SPort_t port1;
    PN_API_SNMP_SPort_t port2;
    uint8_t lldpConfigManAddrPortsTxEnable;
} PN_API_SNMP_SPermanentData_t;

typedef void (*PN_API_SNMP_setPermanentDataCallback) (const PN_API_SNMP_SPermanentData_t *pData_p);

typedef struct PN_API_SNMP_SCallbacks
{
    PN_API_SNMP_setPermanentDataCallback cbSetPermanentData;
} PN_API_SNMP_SCallbacks_t;

typedef struct PN_API_SNMP_SConfiguration
{
    uint16_t interfaceDescriptionLength;
    uint8_t *pInterfaceDescription;
    uint16_t port1DescriptionLength;
    uint8_t *pPort1Description;
    uint16_t port2DescriptionLength;
    uint8_t *pPort2Description;
    PN_API_SNMP_SPermanentData_t *pPermanentData;
    PN_API_SNMP_SCallbacks_t callBacks;
} PN_API_SNMP_SConfiguration_t;

extern uint32_t PN_API_SNMP_applyConfiguration(const PN_API_SNMP_SConfiguration_t *pConfiguration_p);
extern uint32_t PN_API_SNMP_applyPermanentData(const PN_API_SNMP_SPermanentData_t *pData_p);

#ifdef  __cplusplus 
} 
#endif 

#endif //_PN_API_SNMP_H_
