/*!
 *  \example <PN_API_ETH>.h
 *
 *  \brief
 *  Profinet API for ethernet interface.
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

#ifndef _PN_API_ETH_H_
#define _PN_API_ETH_H_

#ifdef __cplusplus
extern "C" {
#endif

#define PN_API_ETH_MAX_STATION_NAME_LENGTH              240
#define PN_API_ETH_MAX_PORT_ID_LENGTH                    14
#define PN_API_ETH_MAX_PEERS_PER_PORT                     1

/*!
 *  \brief
 *  Ethernet Interface error codes.
 *  \ingroup PN_API_ETH_ERROR_CODES
 */
typedef enum PN_API_ETH_EError
{
    PN_API_ETH_eOK                                      = 0x00000000, /*!< error code to be documented*/
    PN_API_ETH_eERROR_WRONG_STATE                       = 0x38030201, /*!< error code to be documented*/
    PN_API_ETH_eERROR_MISSING_PARAMETER_POINTER         = 0x38030202, /*!< error code to be documented*/
    PN_API_ETH_eERROR_INVALID_PORT                      = 0x38030203  /*!< error code to be documented*/
} PN_API_ETH_EError_t;                                                
                                                                      
typedef enum PN_API_ETH_EPeerToPeerBoundary                           
{                                                                     
    PN_API_ETH_eP2PB_PASS,
    PN_API_ETH_eP2PB_BLOCK,
    PN_API_ETH_eP2PB_FORCE32BIT = 0xffffffff //!< Force enum to 32 bit
} PN_API_ETH_EPeerToPeerBoundary_t;

typedef enum PN_API_ETH_EDcpResetToFactoryQualifier
{
    PN_API_ETH_eDCP_RESET_TO_FACTORY_APPLICATION_DATA,
    PN_API_ETH_eDCP_RESET_TO_FACTORY_COMMUNICATION_PARAMETER,
    PN_API_ETH_eDCP_RESET_TO_FACTORY_ENGINEERING_PARAMETER,
    PN_API_ETH_eDCP_RESET_TO_FACTORY_ALL_PARAMETER,
    PN_API_ETH_eDCP_RESET_TO_FACTORY_DEVICE,
    PN_API_ETH_eDCP_RESET_TO_FACTORY_FIRMWARE,
    PN_API_ETH_eDCP_RESET_TO_FACTORY_FORCE32BIT = 0xffffffff //!< Force enum to 32 bit
} PN_API_ETH_EDcpResetToFactoryQualifier_t;

typedef enum PN_API_ETH_EPort
{
    PN_API_ETH_ePORT_1 = 1,
    PN_API_ETH_ePORT_2 = 2,
    PN_API_ETH_ePORT_FORCE32BIT = 0xffffffff //!< Force enum to 32 bit
} PN_API_ETH_EPort_t;

typedef enum PN_API_ETH_EPortLinkState
{
    PN_API_ETH_ePORT_LS_DOWN,
    PN_API_ETH_ePORT_LS_UP,
    PN_API_ETH_ePORT_LS_FORCE32BIT = 0xffffffff //!< Force enum to 32 bit
} PN_API_ETH_EPortLinkState_t;

typedef enum PN_API_ETH_EPortMauType
{
    PN_API_ETH_ePORT_MT_UNKNOWN,
    PN_API_ETH_ePORT_MT_10BASETXHD,
    PN_API_ETH_ePORT_MT_10BASETXFD,
    PN_API_ETH_ePORT_MT_100BASETXHD,
    PN_API_ETH_ePORT_MT_100BASETXFD,
    PN_API_ETH_ePORT_MT_FORCE32BIT = 0xffffffff //!< Force enum to 32 bit
} PN_API_ETH_EPortMauType_t;

typedef struct PN_API_ETH_SMacAddress
{
    uint8_t octet1;
    uint8_t octet2;
    uint8_t octet3;
    uint8_t octet4;
    uint8_t octet5;
    uint8_t octet6;
} PN_API_ETH_SMacAddress_t;

typedef struct PN_API_ETH_SPermanentDcpData
{
    uint32_t ipAddress;
    uint32_t subnetMask;
    uint32_t gateway;
    uint16_t stationNameLength;
    uint8_t aStationName[PN_API_ETH_MAX_STATION_NAME_LENGTH];
} PN_API_ETH_SPermanentDcpData_t;

typedef struct PN_API_ETH_SPeerToPeerBoundaries
{
    bool valid;
    PN_API_ETH_EPeerToPeerBoundary_t boundaryLLDP;
    PN_API_ETH_EPeerToPeerBoundary_t boundaryPTCP;
    PN_API_ETH_EPeerToPeerBoundary_t boundaryTime;
} PN_API_ETH_SPeerToPeerBoundaries_t;

typedef struct PN_API_ETH_SPortDataAdjust
{
    PN_API_ETH_SPeerToPeerBoundaries_t peer2PeerBoundaries;
} PN_API_ETH_SPortDataAdjust_t;

typedef struct PN_API_ETH_SPeerCheck
{
    uint16_t portIdLength;
    uint8_t aPortId[PN_API_ETH_MAX_PORT_ID_LENGTH];
    uint16_t chassisIdLength;
    uint8_t aChassisId[PN_API_ETH_MAX_STATION_NAME_LENGTH];
} PN_API_ETH_SPeerCheck_t;

typedef struct PN_API_ETH_SPortDataCheck
{
    uint8_t numberOfPeers;
    PN_API_ETH_SPeerCheck_t aPeers[PN_API_ETH_MAX_PEERS_PER_PORT];
    bool checkLineDelay;
    uint8_t lineDelayFormat;
    uint32_t lineDelay;
    bool checkMauType;
    uint16_t mauType;
    bool checkLinkState;
    uint8_t linkStateLink;
    uint8_t linkStatePort;
    bool checkSyncDiff;
    uint8_t syncMaster;
    uint8_t cableDelay;
    bool checkMauTypeMode;
    uint8_t mauTypeMode;
} PN_API_ETH_SPortDataCheck_t;

typedef struct PN_API_ETH_SPermanentPortData
{
    PN_API_ETH_SPortDataAdjust_t portDataAdjustPort1;
    PN_API_ETH_SPortDataAdjust_t portDataAdjustPort2;
    PN_API_ETH_SPortDataCheck_t portDataCheckPort1;
    PN_API_ETH_SPortDataCheck_t portDataCheckPort2;
} PN_API_ETH_SPermanentPortData_t;

typedef void (*PN_API_ETH_handleInternalErrorCallback) (uint32_t errorCode_p, bool fatal_p, uint8_t paraCnt_p, va_list argptr_p);
typedef void (*PN_API_ETH_getLocalTimeCallback) (uint32_t *pTimeHigh_p, uint32_t *pTimeLow_p);
typedef void (*PN_API_ETH_getServerBootTimeCallback) (uint32_t *pTime_p);
typedef void (*PN_API_ETH_savePermanentDataCallback) (void);
typedef void (*PN_API_ETH_signalLinkLedCallback) (bool state_p);
typedef void (*PN_API_ETH_setStationNameCallback) (const uint8_t *pName_p, uint16_t length_p);
typedef void (*PN_API_ETH_setIpAddressCallback) (uint32_t ipAddress_p, uint32_t subNetMask_p, uint32_t gateWayAddress_p);
typedef void (*PN_API_ETH_factoryResetCallback) (void);
typedef bool (*PN_API_ETH_resetToFactoryCallback) (PN_API_ETH_EDcpResetToFactoryQualifier_t qualifier_p);
typedef void (*PN_API_ETH_writePortDataCheckCallback) (PN_API_ETH_EPort_t port_p, const PN_API_ETH_SPortDataCheck_t *pData_p);
typedef void (*PN_API_ETH_writePortDataAdjustCallback) (PN_API_ETH_EPort_t port_p, const PN_API_ETH_SPortDataAdjust_t *pData_p);

typedef struct PN_API_ETH_SCallbacks
{
    PN_API_ETH_handleInternalErrorCallback cbHandleInternalError;
    PN_API_ETH_getLocalTimeCallback cbGetLocalTime;
    PN_API_ETH_getServerBootTimeCallback cbGetServerBootTime;
    PN_API_ETH_savePermanentDataCallback cbSavePermanentData;
    PN_API_ETH_signalLinkLedCallback cbSignalLinkLed;
    PN_API_ETH_setStationNameCallback cbSetStationName;
    PN_API_ETH_setIpAddressCallback cbSetIpAddress;
    PN_API_ETH_factoryResetCallback cbFactoryReset;
    PN_API_ETH_resetToFactoryCallback cbResetToFactory;
    PN_API_ETH_writePortDataCheckCallback cbWritePortDataCheck;
    PN_API_ETH_writePortDataAdjustCallback cbWritePortDataAdjust;
} PN_API_ETH_SCallbacks_t;

typedef struct PN_API_ETH_SConfiguration
{
    PN_API_ETH_SMacAddress_t macAddress;
    PN_API_ETH_SPermanentDcpData_t *pPermanentDcpData;
    PN_API_ETH_SPermanentPortData_t *pPermanentPortData;
    PN_API_ETH_SCallbacks_t callBacks;
} PN_API_ETH_SConfiguration_t;

extern uint32_t PN_API_ETH_applyConfiguration(const PN_API_ETH_SConfiguration_t *pConfiguration_p);
extern uint32_t PN_API_ETH_applyPermanentDcpData(const PN_API_ETH_SPermanentDcpData_t *pData_p);
extern uint32_t PN_API_ETH_applyPermanentPortData(const PN_API_ETH_SPermanentPortData_t *pData_p);
extern uint32_t PN_API_ETH_getPortLinkState(PN_API_ETH_EPort_t port_p, PN_API_ETH_EPortLinkState_t *pState_p);
extern uint32_t PN_API_ETH_getPortMauType(PN_API_ETH_EPort_t port_p, PN_API_ETH_EPortMauType_t *pType_p);
extern uint32_t PN_API_ETH_getStationName(const uint8_t **ppName_p, uint16_t *pLength_p);

#ifdef  __cplusplus 
} 
#endif 

#endif //_PN_API_ETH_H_
