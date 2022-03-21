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

#ifndef _APPPERMANENTDATA_H_
#define _APPPERMANENTDATA_H_

typedef struct APP_SPermanentData
{
    uint16_t magicNumber;
    uint32_t version;
    uint32_t checksum;
    uint32_t ipAddress;
    uint32_t subNetMask;
    uint32_t gateWayAddress;
    uint16_t stationNameLength;
    uint8_t aStationName[PN_API_ETH_MAX_STATION_NAME_LENGTH];
    uint32_t helloMode;
    uint32_t helloInterval;
    uint32_t helloDelay;
    uint32_t helloRetry;
    PN_API_IM_SFields_t aImFields[PN_API_IM_MAX_CARRIERS];
    PN_API_ETH_SPermanentPortData_t permanentPortData;
    PN_API_SNMP_SPermanentData_t permanentSnmpData;
} APP_SPermanentData_t;

#ifdef __cplusplus
extern "C" {
#endif

    extern int32_t APP_initPermStorage (void);
    extern int32_t APP_savePermStorage (APP_SPermanentData_t *pData_p);
    extern APP_SPermanentData_t *APP_getPermStorage (void);
    extern void APP_factoryResetPermStorage (void);
    extern void APP_resetCommParamPermStorage (void);

#ifdef __cplusplus
}
#endif

#endif  // _APPPERMANENTDATA_H_
