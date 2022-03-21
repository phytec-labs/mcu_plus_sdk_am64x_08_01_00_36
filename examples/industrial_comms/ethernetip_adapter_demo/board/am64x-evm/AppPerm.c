/*!
 *  \example AppPerm.c
 *
 *  \brief
 *  EtherNet/IP&trade; Adapter Example Application, handling of permanent data storage.
 *
 *  \author
 *  KUNBUS GmbH
 *
 *  \date
 *  2021-06-09
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
 */


#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ti_board_open_close.h>

#include <api/EI_API.h>
#include "AppPerm.h"

void EI_APP_PERM_clearFlash(void);
bool EI_APP_PERM_factoryReset(int16_t serviceFlag_p);

Flash_Handle EI_APP_PERM_flashHandle_g;
static EI_API_ADP_T *pAdp_s = NULL;
static bool bResetRequired_s = false;
static bool bConfigChanged_s = false;
static uint32_t resetTime_s = 0;
static int16_t resetServiceFlag_s;

static EI_APP_PERM_SCfgData_t tPermData_s;
static EI_APP_PERM_SCfgData_t tDefaultPermData_s =
{
    .i32uIpAddr =       0xc0a8010a,
    .i32uIpNwMask =     0xffffff00,
    .i32uIpGateway =    0xc0a80101,
    .i32uNameServer1 =  0x00000000,
    .i32uNameServer2 =  0x00000000,
    .szDomainName = "",
    .szHostName = "",
    .bUseDhcp = false,
    .i8uTtlValue = 1,
    .bAcdActive = true,
    .ai8uAcdAddr = { 0,0,0,0,0,0 },
    .ai8uAcdHdr = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
    .intfConfig[0].bit.ETHIntfActive = 1,
    .intfConfig[0].bit.ETHIntfAutoNeg = 1,
    .intfConfig[0].bit.ETHIntfFDuplex = 0,
    .intfConfig[0].bit.ETHIntf100MB = 0,
    .intfConfig[1].bit.ETHIntfActive = 1,
    .intfConfig[1].bit.ETHIntfAutoNeg = 1,
    .intfConfig[1].bit.ETHIntfFDuplex = 0,
    .intfConfig[1].bit.ETHIntf100MB = 0,
    .tQoSParameter.Q_Tag_Enable = EI_API_ADP_DEFAULT_8021Q,
    .tQoSParameter.DSCP_PTP_Event = EI_API_ADP_DEFAULT_DSCP_PTP_EVENT,
    .tQoSParameter.DSCP_PTP_General = EI_API_ADP_DEFAULT_DSCP_PTP_GENERAL,
    .tQoSParameter.DSCP_Urgent = EI_API_ADP_DEFAULT_DSCP_URGENT,
    .tQoSParameter.DSCP_Scheduled = EI_API_ADP_DEFAULT_DSCP_SCHEDULED,
    .tQoSParameter.DSCP_High = EI_API_ADP_DEFAULT_DSCP_HIGH,
    .tQoSParameter.DSCP_Low = EI_API_ADP_DEFAULT_DSCP_LOW,
    .tQoSParameter.DSCP_Explicit = EI_API_ADP_DEFAULT_DSCP_EXPLICIT,

    .i16uEncapInactTimeout = 120,
    .tMcastConfig.allocControl = 0,

#if defined(TIME_SYNC)
    .ptpEnable                  = 1,
    .portEnable                 = 1,
    .portLogAnnounceInterval    = 1,
    .portLogSyncInterval        = 1,
    .domainNumber               = 0,
#else
    .ptpEnable                  = 0,
    .portEnable                 = 1,
    .portLogAnnounceInterval    = 1,
    .portLogSyncInterval        = 1,
    .domainNumber               = 0,
#endif

#if defined(QUICK_CONNECT)
    .bQuickConnectEnabled       = true,
#else
    .bQuickConnectEnabled       = false,
#endif

};

/*!
 *  <!-- Description: -->
 *
 *  \brief
 *  Initialize handle to permanent data in Flash memory.
 *
 *  \details
 *  Initialize handle to permanent data in Flash memory.
 *
 */
bool EI_APP_PERM_init(EI_API_ADP_T *pAdpNode_p)
{
    pAdp_s = pAdpNode_p;

    EI_APP_PERM_flashHandle_g = Flash_getHandle (CONFIG_FLASH0);

    return (true);
}


/*!
 *  <!-- Description: -->
 *
 *  \brief
 *  Write the permanent data.
 *
 *  \details
 *  Write the permanent data to Flash memory.
 *
 */
bool EI_APP_PERM_write(void)
{
    int32_t  err_l;

    tPermData_s.tPermHdr.magicNumber = (('M' << 8) | 'R');
    tPermData_s.tPermHdr.version = APP_PERM_DATA_VERSION;
    tPermData_s.tPermHdr.checksum = 0;

    EI_APP_PERM_clearFlash ();

    err_l = Flash_write (EI_APP_PERM_flashHandle_g, EI_APP_PERM_DATA_OFFSET, (uint8_t *)&tPermData_s, sizeof (EI_APP_PERM_SCfgData_t));
    if (err_l < 0)
    {
        OSAL_printf ("\r\nFlash_write failed");
        goto laError;
    }

    return true;

    //-------------------------------------------------------------------------------------------------
    laError:

        exit (-1);
}

/*!
 *  <!-- Description: -->
 *
 *  \brief
 *  Reads the permanent data.
 *
 *  \details
 *  Reads the permanent data. If no data are present,
 *  a new structure is created with default data.
 *
 */
bool EI_APP_PERM_read(void)
{
    int32_t err_l;
    uint32_t checkSum;

    err_l = Flash_read (EI_APP_PERM_flashHandle_g, EI_APP_PERM_DATA_OFFSET, (uint8_t *)&tPermData_s, sizeof (EI_APP_PERM_SCfgData_t));
    if (err_l < 0)
    {
        OSAL_printf ("\r\nFlash_read failed");
        goto laError;
    }

    checkSum = 0;  // TBD calculate a proper checksum

    if (   (tPermData_s.tPermHdr.magicNumber != (('M' << 8) | 'R'))
        || (tPermData_s.tPermHdr.version != APP_PERM_DATA_VERSION)
        || (tPermData_s.tPermHdr.checksum != checkSum)
       )
    {   // Data are corrupt -> write default values
        OSAL_printf ("Data are corrupted, write default values.");
        EI_APP_PERM_factoryReset(1);
    }

    EI_API_ADP_setHostName(pAdp_s, tPermData_s.szHostName);
    EI_API_ADP_setIpTTL(pAdp_s, tPermData_s.i8uTtlValue);
    EI_API_ADP_setACD(pAdp_s, tPermData_s.bAcdActive);
    EI_API_ADP_setIntfConfig(pAdp_s, 0, tPermData_s.intfConfig[0]);
    EI_API_ADP_setIntfConfig(pAdp_s, 1, tPermData_s.intfConfig[1]);
    EI_API_ADP_setEnipAcdState(pAdp_s, tPermData_s.i8uAcdState);
    EI_API_ADP_SParam_t acdAddr = { 6, (uint8_t*)&tPermData_s.ai8uAcdAddr };
    EI_API_ADP_setEnipAcdAddr(pAdp_s, &acdAddr);
    EI_API_ADP_SParam_t acdHdr = { 28, (uint8_t*)&tPermData_s.ai8uAcdHdr };
    EI_API_ADP_setEnipAcdHdr(pAdp_s, &acdHdr);
    EI_API_ADP_setEncapInactTimeout(pAdp_s, tPermData_s.i16uEncapInactTimeout);
    EI_API_ADP_setQoS(pAdp_s, &tPermData_s.tQoSParameter);
    EI_API_ADP_setMcastConfiguration(pAdp_s, &tPermData_s.tMcastConfig);

    if(tPermData_s.tPermHdr.version == APP_PERM_DATA_VERSION)
    {
        EI_API_ADP_setPtpEnable(pAdp_s, tPermData_s.ptpEnable);
        EI_API_ADP_setPortEnable(pAdp_s, tPermData_s.portEnable);
        EI_API_ADP_setPortLogAnnounceInterval(pAdp_s, tPermData_s.portLogAnnounceInterval);
        EI_API_ADP_setPortLogSyncInterval(pAdp_s, tPermData_s.portLogSyncInterval);
        EI_API_ADP_setDomainNumber(pAdp_s, tPermData_s.domainNumber);
     }
    else
    {
        EI_API_ADP_setPtpEnable(pAdp_s, tDefaultPermData_s.ptpEnable);
        EI_API_ADP_setPortEnable(pAdp_s, tDefaultPermData_s.portEnable);
        EI_API_ADP_setPortLogAnnounceInterval(pAdp_s, tDefaultPermData_s.portLogAnnounceInterval);
        EI_API_ADP_setPortLogSyncInterval(pAdp_s, tDefaultPermData_s.portLogSyncInterval);
        EI_API_ADP_setDomainNumber(pAdp_s, tDefaultPermData_s.domainNumber);
    }

#if defined(QUICK_CONNECT)
    // Enable QuickConnect
    EI_API_ADP_setQuickConnectEnabled(pAdp_s, tPermData_s.bQuickConnectEnabled);
#endif

    EI_API_ADP_setIpConfig(pAdp_s, tPermData_s.bUseDhcp, tPermData_s.i32uIpAddr, tPermData_s.i32uIpNwMask, tPermData_s.i32uIpGateway,
                           tPermData_s.i32uNameServer1, tPermData_s.i32uNameServer2, tPermData_s.szDomainName, false);
    return true;

    //-------------------------------------------------------------------------------------------------
    laError:

        exit (-1);
}


/*!
 *  <!-- Description: -->
 *
 *  \brief
 *  Returns a pointer to the permanent data.
 *
 *  \details
 *  Returns a pointer to the permanent data area in Flash.
 *
 */
EI_APP_PERM_SCfgData_t *EI_APP_PERM_get(void)
{
    return (&tPermData_s);
}


/*!
 *  <!-- Description: -->
 *
 *  \brief
 *  Performs a factory reset.
 *
 *  \details
 *  Performs a factory reset
 *  - With serviceFlag == 1: restore default data
 *  - With serviceFlag == 2: restore default data, except communication link attributes.
 *  - Other values:          do nothing
 *
 */
bool EI_APP_PERM_factoryReset(int16_t serviceFlag_p)
{
    switch (serviceFlag_p)
    {
    case 1:
        // Restore default data.
        memcpy(&tPermData_s, &tDefaultPermData_s, sizeof(EI_APP_PERM_SCfgData_t));
        break;
    case 2:
        // Restore default data, except communication link attributes, these are:
        // TCP/IP object 0xF5, attributes 3, 5 and 6.
        // Ethernet Link object 0xF6, attribute 6.
        tPermData_s.i8uTtlValue = tDefaultPermData_s.i8uTtlValue;
        tPermData_s.bAcdActive  = tDefaultPermData_s.bAcdActive;
        tPermData_s.i8uAcdState = tDefaultPermData_s.i8uAcdState;

        memcpy(tPermData_s.ai8uAcdAddr, tDefaultPermData_s.ai8uAcdAddr, sizeof(tDefaultPermData_s.ai8uAcdAddr));
        memcpy(tPermData_s.ai8uAcdHdr,  tDefaultPermData_s.ai8uAcdHdr,  sizeof(tDefaultPermData_s.ai8uAcdHdr));

        tPermData_s.i16uEncapInactTimeout = tDefaultPermData_s.i16uEncapInactTimeout;
        memcpy(&tPermData_s.tQoSParameter, &tDefaultPermData_s.tQoSParameter, sizeof(tDefaultPermData_s.tQoSParameter));

        //timeSync attributes
        tPermData_s.ptpEnable                = tDefaultPermData_s.ptpEnable;
        tPermData_s.portEnable               = tDefaultPermData_s.portEnable;
        tPermData_s.portLogSyncInterval      = tDefaultPermData_s.portLogSyncInterval;
        tPermData_s.portLogAnnounceInterval  = tDefaultPermData_s.portLogAnnounceInterval;
        tPermData_s.domainNumber             = tDefaultPermData_s.domainNumber;


#if defined(QUICK_CONNECT)
        tPermData_s.bQuickConnectEnabled = tDefaultPermData_s.bQuickConnectEnabled;
#endif
        break;
    default:
        return false;
    }

    return EI_APP_PERM_write();
}


/*!
 *  <!-- Description: -->
 *
 *  \brief
 *  Callback function for write accesses of several attributes.
 *
 *  \details
 *  Callback function for write accesses of several attributes. Saves the new permanent data.
 *  Sets new network configuration, if necessary. Sets hostname, if necessary.
 */
void EI_APP_PERM_configCb(EI_API_CIP_NODE_T *pCipNode_p, uint16_t classId_p, uint16_t instanceId_p, uint16_t attrId_p, EI_API_CIP_ESc_t serviceCode_p, int16_t serviceFlag_p)
{
   EI_API_ADP_SQos_t qos;
   EI_API_ADP_SParam_t acdAddr;
   EI_API_ADP_SParam_t acdHdr;

   bool hwConfigEnabled;
   char domainName[49];
   char hostName[65];

   uint32_t ipAddr;                 // IP address
   uint32_t ipNwMask;               // Network mask
   uint32_t ipGateway;              // Gateway address
   uint32_t nameServer1;            // First name server address
   uint32_t nameServer2;            // Second name server address
   bool     dhcp;                   // DHCP

   // Early exit, because we are only interested if Set_Attribute was executed.
   if (serviceCode_p != EI_API_CIP_eSC_SETATTRSINGLE) return;

   if (classId_p == 0xf5)
   {
       if (attrId_p == 3 || attrId_p == 5)
       {
           EI_API_ADP_isHwSettingEnabled(pAdp_s, &hwConfigEnabled);
           if (hwConfigEnabled)
           {
               // Network configuration is controlled by hardware, do nothing.
               return;
           }

           // Attribute 3 (configuration control) or
           // Attribute 5 (interface configuration) is set.
           EI_API_ADP_getIpAddr(pAdp_s, &ipAddr);
           EI_API_ADP_getIpNwMask(pAdp_s, &ipNwMask);
           EI_API_ADP_getIpGateway(pAdp_s, &ipGateway);
           EI_API_ADP_getIpPriNameServer(pAdp_s, &nameServer1);
           EI_API_ADP_getIpSecNameServer(pAdp_s, &nameServer2);
           EI_API_ADP_getDomainName(pAdp_s, domainName);
           EI_API_ADP_getDHCP(pAdp_s, &dhcp);

           if ( (ipAddr      != tPermData_s.i32uIpAddr)      ||
                (ipNwMask    != tPermData_s.i32uIpNwMask)    ||
                (ipGateway   != tPermData_s.i32uIpGateway)   ||
                (nameServer1 != tPermData_s.i32uIpGateway)   ||
                (nameServer2 != tPermData_s.i32uNameServer1) ||
                (dhcp        != tPermData_s.bUseDhcp)        ||
                (strncmp(domainName, tPermData_s.szDomainName, sizeof(tPermData_s.szDomainName)) != 0) )
           {
               // Attribute 3 (configuration control) or
               // Attribute 5 (interface configuration) is set.
               tPermData_s.i32uIpAddr      = ipAddr;
               tPermData_s.i32uIpNwMask    = ipNwMask;
               tPermData_s.i32uIpGateway   = ipGateway;
               tPermData_s.i32uNameServer1 = nameServer1;
               tPermData_s.i32uNameServer2 = nameServer2;
               tPermData_s.bUseDhcp        = dhcp;

               strncpy(tPermData_s.szDomainName, domainName, sizeof(tPermData_s.szDomainName));

               bConfigChanged_s = true;
           }
       }
       else if (attrId_p == 6)
       {
           // TCP/IP Object 0xF5,
           // Attribute 6 (Host Name)
           EI_API_ADP_getHostName(pAdp_s, hostName);
           strncpy(tPermData_s.szHostName, hostName, sizeof(tPermData_s.szHostName));
           bConfigChanged_s = true;

       }
       else if (attrId_p == 8)
       {
           EI_API_ADP_getIpTTL(pAdp_s, &tPermData_s.i8uTtlValue);
           bConfigChanged_s = true;
       }
       else if (attrId_p == 9)
       {
           EI_API_ADP_getMcastConfiguration(pAdp_s, &tPermData_s.tMcastConfig);
           bConfigChanged_s = true;
       }
       else if (attrId_p == 10)
       {
           EI_API_ADP_getACD(pAdp_s, &tPermData_s.bAcdActive);
           bConfigChanged_s = true;
       }
       else if (attrId_p == 11)
       {
           EI_API_ADP_getEnipAcdState(pAdp_s, &tPermData_s.i8uAcdState);

           EI_API_ADP_getEnipAcdAddr(pAdp_s, &acdAddr);
           memcpy(tPermData_s.ai8uAcdAddr, acdAddr.data, sizeof(tPermData_s.ai8uAcdAddr));

           EI_API_ADP_getEnipAcdHdr(pAdp_s, &acdHdr);
           memcpy(tPermData_s.ai8uAcdHdr, acdHdr.data, sizeof(tPermData_s.ai8uAcdHdr));
           bConfigChanged_s = true;
       }
#if defined(QUICK_CONNECT)
       else if (attrId_p == 12)
       {
           // Enable/Disable QuickConnect
           EI_API_ADP_getQuickConnectEnabled(pAdp_s, &tPermData_s.bQuickConnectEnabled);
           bConfigChanged_s = true;
       }
#endif
       else if (attrId_p == 13)
       {
           EI_API_ADP_getEncapInactTimeout(pAdp_s, &tPermData_s.i16uEncapInactTimeout);
           bConfigChanged_s = true;
       }
       else
       {
           // Nothing has changed.
           bConfigChanged_s = false;
       }
   }
   else if (classId_p == 0x43)
    {
        // timeSync object has changed.
        if (attrId_p == 1)
        {
            EI_API_ADP_getPtpEnable(pAdp_s, &tPermData_s.ptpEnable);
            bConfigChanged_s = true;
        }
        else if (attrId_p == 13)
        {
            EI_API_ADP_getPortEnable(pAdp_s, &tPermData_s.portEnable);
            bConfigChanged_s = true;
        }
        else if (attrId_p == 14)
        {
            EI_API_ADP_getPortLogAnnounceInterval(pAdp_s, &tPermData_s.portLogAnnounceInterval);
            bConfigChanged_s = true;
        }
        else if (attrId_p == 15)
        {
            EI_API_ADP_getPortLogSyncInterval(pAdp_s, &tPermData_s.portLogSyncInterval);
            bConfigChanged_s = true;
        }
        else if (attrId_p == 18)
        {
            EI_API_ADP_getDomainNumber(pAdp_s, &tPermData_s.domainNumber);
             bConfigChanged_s = true;
        }

    }
   else if (classId_p == 0x48)
   {
       // QoS object has changed.
       EI_API_ADP_getQoS(pAdp_s, &qos);
       memcpy (&tPermData_s.tQoSParameter, &qos, sizeof(EI_API_ADP_SQos_t));
       bConfigChanged_s = true;
   }

   else if (classId_p == 0xf6)
   {
       // Ethernet link object has changed for instance 1 or 2.
       if (instanceId_p == 1)
       {
           EI_API_ADP_UIntfConf_t intfConf;
           EI_API_ADP_getIntfConfig(pAdp_s, 0, &intfConf);

           tPermData_s.intfConfig[0] = intfConf;
           bConfigChanged_s = true;
       }
       else if (instanceId_p == 2)
       {
           EI_API_ADP_UIntfConf_t intfConf;
           EI_API_ADP_getIntfConfig(pAdp_s, 1, &intfConf);

           tPermData_s.intfConfig[1] = intfConf;
           bConfigChanged_s = true;
       }
       else
       {
           // Nothing has changed.
           bConfigChanged_s = false;
       }
   }
   else
   {
       // Nothing has changed.
       bConfigChanged_s = false;
   }

}


/*!
 *  <!-- Description: -->
 *
 *  \brief
 *  Callback for reset service of class 0x01.
 *
 *  \details
 *  Callback for reset service of class 0x01. Sets the timestamp for a delayed reset.
 */
void EI_APP_PERM_reset(EI_API_CIP_NODE_T *pCipNode_p, uint16_t classId_p, uint16_t instanceId_p, uint16_t attrId_p, EI_API_CIP_ESc_t serviceCode_p, int16_t serviceFlag_p)
{
    if (serviceFlag_p == 1 || serviceFlag_p == 2)
    {
        // Reset with parameter 1 or 2 means factory reset.
        EI_APP_PERM_factoryReset(serviceFlag_p);
    }

    resetTime_s  = OSAL_getMsTick();
    bResetRequired_s = true;
    resetServiceFlag_s = serviceFlag_p;
}

/*!
 *  <!-- Description: -->
 *
 *  \brief
 *  Checks if a reset is required.
 *
 *  \details
 *  Checks if a reset is required and 2 seconds are expired.
 +
 */
int16_t EI_APP_PERM_getResetRequired(void)
{
    uint32_t actTime = OSAL_getMsTick();
    uint32_t difTime = 0;

    if (bResetRequired_s)
    {
        // Wait 2 seconds for reset:
        if (actTime < resetTime_s)
        {
            difTime = (0xFFFFFFFF - resetTime_s) + actTime;
        }
        else
        {
            difTime = actTime - resetTime_s;
        }

        if (difTime > 2000)
        {
            bResetRequired_s = false;
            return resetServiceFlag_s;
        }
    }
    return -1;
}

bool EI_APP_PERM_getConfigChanged(void)
{
    if (bConfigChanged_s)
    {
        bConfigChanged_s = false;
        return true;
    }
    else
    {
        return false;
    }
}

/*!
 *  <!-- Description: -->
 *
 *  \brief
 *  CLears a flash block.
 *
 *  \details
 *  CLears a flash block. This must be done before writing to this block.
 +
 */
void EI_APP_PERM_clearFlash (
    void)

{
    Flash_Attrs *pAttr;
    int32_t err;
    uint32_t iOffset;
    uint32_t block;
    uint32_t page;

    pAttr = Flash_getAttrs (CONFIG_FLASH0);

    for (iOffset = EI_APP_PERM_DATA_OFFSET_MIN; iOffset <= EI_APP_PERM_DATA_OFFSET_MAX; iOffset += pAttr->blockSize)
    {
        err = Flash_offsetToBlkPage (EI_APP_PERM_flashHandle_g, iOffset, &block, &page);
        if (err < 0)
        {
            OSAL_printf ("\r\nFlash_offsetToBlkPage failed");
            goto laError;
        }

        err = Flash_eraseBlk (EI_APP_PERM_flashHandle_g, block);
        if (err < 0)
        {
            OSAL_printf ("\r\nFlash_eraseBlk failed");
            goto laError;
        }
    }

    return;

//-------------------------------------------------------------------------------------------------
laError:

    exit (-1);

}

