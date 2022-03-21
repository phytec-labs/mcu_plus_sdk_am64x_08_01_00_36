/*
 *  Copyright (c) 2021, KUNBUS GmbH
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


#ifndef APPPERM_H_INC
#define APPPERM_H_INC

#include <stdbool.h>
#include <stdint.h>
#include <api/EI_API_ADP.h>

#define EI_APP_PERM_DATA_OFFSET       0x200000
#define EI_APP_PERM_DATA_OFFSET_MIN   EI_APP_PERM_DATA_OFFSET
#define EI_APP_PERM_DATA_OFFSET_MAX   (EI_APP_PERM_DATA_OFFSET_MIN + sizeof (EI_APP_PERM_SCfgData_t))

#define APP_PERM_DATA_VERSION     3

typedef struct EI_APP_PERM_SCfgHeader
{
    uint16_t magicNumber;
    uint32_t version;
    uint32_t checksum;
} EI_APP_PERM_SCfgHeader_t;

typedef struct EI_APP_PERM_SCfgData
{
    EI_APP_PERM_SCfgHeader_t tPermHdr;

    // TCP/IP object 0xF5 attribute 5
    uint32_t i32uIpAddr;                 // IP address
    uint32_t i32uIpNwMask;               // Network mask
    uint32_t i32uIpGateway;              // Gateway address
    uint32_t i32uNameServer1;            // First name server address
    uint32_t i32uNameServer2;            // Second name server address
    char szDomainName[48];               // Domain name

    // TCP/IP object 0xF5
    bool bUseDhcp;                       // attribute 3
    char szHostName[64];                 // Attribute 6
    uint8_t i8uTtlValue;                 // attribute 8
    bool bAcdActive;                     // Attribute 10, select acd
    uint8_t i8uAcdState;                 // Attribute 11, state of acd
    uint8_t ai8uAcdAddr[6];              // Attribute 11, acd remote mac
    uint8_t ai8uAcdHdr[28];              // Attribute 11, arp pdu
    uint16_t i16uEncapInactTimeout;      // attribute 13

    // Ethernet Link object 0xF6
    EI_API_ADP_UIntfConf_t intfConfig[2];  // attribute 6

    //timeSync object 0x43
    bool ptpEnable;
    bool portEnable;
    uint16_t portLogAnnounceInterval;
    int16_t portLogSyncInterval;
    uint8_t domainNumber;

    // QoS object 0x48
    EI_API_ADP_SQos_t tQoSParameter;         // QoS Object 0x48

    EI_API_ADP_SMcastConfig_t tMcastConfig; // Multicast Configuration 0xF5 Attribute 9

    bool bQuickConnectEnabled;

} EI_APP_PERM_SCfgData_t;

#ifdef __cplusplus
extern "C" {
#endif

bool EI_APP_PERM_init(EI_API_ADP_T *pAdpNode_p);
bool EI_APP_PERM_write(void);
bool EI_APP_PERM_read(void);
EI_APP_PERM_SCfgData_t *EI_APP_PERM_get(void);
bool EI_APP_PERM_factoryReset(int16_t serviceFlag_p);
void EI_APP_PERM_configCb(EI_API_CIP_NODE_T *pCipNode_p, uint16_t classId_p, uint16_t instanceId_p, uint16_t attrId_p, EI_API_CIP_ESc_t serviceCode_p, int16_t serviceFlag_p);
void EI_APP_PERM_reset(EI_API_CIP_NODE_T *pCipNode_p, uint16_t classId_p, uint16_t instanceId_p, uint16_t attrId_p, EI_API_CIP_ESc_t serviceCode_p, int16_t serviceFlag_p);
int16_t EI_APP_PERM_getResetRequired(void);
bool EI_APP_PERM_getConfigChanged(void);

#ifdef  __cplusplus 
}
#endif 

#endif // APPPERM_H_INC
