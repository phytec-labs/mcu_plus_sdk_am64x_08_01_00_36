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
#include <string.h>
#include "icss_timeSync.h"
#include "icss_timeSync_init.h"
#include "icss_timeSync_memory_map.h"
#include "icss_timeSyncApi.h"
#include <drivers/hw_include/hw_types.h>

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */
int32_t TimeSync_config(TimeSync_ParamsHandle_t timeSyncParamsHandle)
{
    PRUICSS_HwAttrs const *pruicssHwAttrs = (PRUICSS_HwAttrs const *)(timeSyncParamsHandle->pruicssHandle->hwAttrs);
    uintptr_t iepBaseAddress = pruicssHwAttrs->iep0RegBase;
    uint8_t i;
    uint32_t regVal;

    if(timeSyncParamsHandle == NULL)
    {
        return ERROR_HANDLE_INVALID;
    }

    /* Configures domainNumber list */
    for(i = 0; i < TS_NUM_DOMAINS; i++)
    {
        HW_WR_REG8(pruicssHwAttrs->pru0DramBase + TIMESYNC_DOMAIN_NUMBER_LIST + i,
            timeSyncParamsHandle->timeSyncConfig.domainNumber[i]);
    }

    /*Update domain number in PTP TX buffers*/
    TimeSync_updateDomainNumberInPTPFrames(timeSyncParamsHandle);

    /* Configures SYNCOUT sync0 signal start time and pulse width */
    regVal = HW_RD_REG32(iepBaseAddress + CSL_ICSS_G_PR1_IEP0_SLV_CMP_CFG_REG);
    regVal |= 0x00000004;
    HW_WR_REG32(iepBaseAddress + CSL_ICSS_G_PR1_IEP0_SLV_CMP_CFG_REG, regVal);
    HW_WR_REG32(iepBaseAddress + CSL_ICSS_G_PR1_IEP0_SLV_CMP1_REG0, timeSyncParamsHandle->timeSyncConfig.syncOut_sync0Start);
    HW_WR_REG32(iepBaseAddress + CSL_ICSS_G_PR1_IEP0_SLV_SYNC_PWIDTH_REG, timeSyncParamsHandle->timeSyncConfig.syncOut_sync0PWidth);

    return 0;
}

int32_t TimeSync_getTxTimestamp(TimeSync_ParamsHandle_t timeSyncParamsHandle, \
                                ptpFrameTypes_t txFrameType, uint8_t txPort, \
                                uint32_t *nanoseconds, uint64_t *seconds)
{
    PRUICSS_HwAttrs const *pruicssHwAttrs = (PRUICSS_HwAttrs const *)(timeSyncParamsHandle->pruicssHandle->hwAttrs);
    uint32_t timeStampOffsetAddr = 0;
    uint8_t *bytePtr;
    uint64_t Nanoseconds64;
#ifdef PROFINET_PTCP
    uint64_t cycleCounter, iepCount;
    uint64_t *timeStampAddress = 0;
#endif //PROFINET_PTCP

    if(timeSyncParamsHandle->emacHandle == NULL)
    {
        return ERROR_HANDLE_INVALID;
    }

    /* Input error check */
    if((txFrameType != SYNC_FRAME) && (txFrameType != DELAY_REQ_FRAME)
            && (txFrameType != DELAY_RESP_FRAME))
    {
        return ERROR_TX_FRAMETYPE_NOTVALID;    /* not a valid frameType value */
    }

    if((txPort != ICSS_EMAC_PORT_1) && (txPort != ICSS_EMAC_PORT_2))
    {
        return ERROR_TX_PORTNUMBER_NOTVALID;    /* not a valid port number */
    }

    /* Read timestamp */
    if(txFrameType == SYNC_FRAME)
    {
        if(txPort == ICSS_EMAC_PORT_1)
        {
            timeStampOffsetAddr = TX_SYNC_TIMESTAMP_OFFSET_P1;
        }

        else
        {
            timeStampOffsetAddr = TX_SYNC_TIMESTAMP_OFFSET_P2;
        }
    }

    else if(txFrameType == DELAY_REQ_FRAME)
    {
        if(txPort == ICSS_EMAC_PORT_1)
        {
            timeStampOffsetAddr = TX_PDELAY_REQ_TIMESTAMP_OFFSET_P1;
        }

        else
        {
            timeStampOffsetAddr = TX_PDELAY_REQ_TIMESTAMP_OFFSET_P2;
        }
    }

    else /* (rxFrameType == DELAY_RESP_FRAME) */
    {
        if(txPort == ICSS_EMAC_PORT_1)
        {
            timeStampOffsetAddr = TX_PDELAY_RESP_TIMESTAMP_OFFSET_P1;
        }

        else
        {
            timeStampOffsetAddr = TX_PDELAY_RESP_TIMESTAMP_OFFSET_P2;
        }
    }

#ifdef PROFINET_PTCP
    timeStampAddress = (uint64_t *)(pruicssHwAttrs->pru1DramBase +
                                    timeStampOffsetAddr);
    iepCount = ((*timeStampAddress) & 0x00000000FFFFFFFF);
    timeStampAddress = (uint64_t *)(pruicssHwAttrs->pru1DramBase +
                                    timeStampOffsetAddr + 4);
    cycleCounter = *timeStampAddress;
    Nanoseconds64 = (cycleCounter * RTC_3125_CLK_CONST) + iepCount;
#endif //PROFINET_PTCP

    bytePtr = (uint8_t *)((uint32_t)(pruicssHwAttrs->sharedDramBase +
                                     timeStampOffsetAddr));

    /*Return timestamp */
    if(V2 == timeSyncParamsHandle->timeSyncConfig.icssVersion)
    {
        memcpy(&Nanoseconds64, bytePtr, 8);
        *nanoseconds = (uint32_t)(Nanoseconds64 % (uint64_t)SEC_TO_NS);
        *seconds = Nanoseconds64 / (uint64_t)SEC_TO_NS;
    }

    else
    {
        memcpy(nanoseconds, bytePtr, 4);
        memcpy(seconds, bytePtr + 4, 6);
    }

#ifdef PROFINET_PTCP

    if(timeSyncParamsHandle->txprotocol == 1)
    {
        timeSyncParamsHandle->txTimestamp_PTCP->Nanoseconds = Nanoseconds64 %
                1000000000;
        timeSyncParamsHandle->txTimestamp_PTCP->SecondsLow = cycleCounter / 32000;
        timeSyncParamsHandle->txTimestamp_PTCP->SecondsHigh = 0;
        timeSyncParamsHandle->txTimestamp_PTCP->Status = 0;
    }

#endif //PROFINET_PTCP
    return 0;
}

int32_t TimeSync_getRxTimestamp(TimeSync_ParamsHandle_t timeSyncParamsHandle,
                                ptpFrameTypes_t rxFrameType, uint8_t rxPort, \
                                uint32_t *nanoseconds, uint64_t *seconds)
{
    PRUICSS_HwAttrs const *pruicssHwAttrs = (PRUICSS_HwAttrs const *)(timeSyncParamsHandle->pruicssHandle->hwAttrs);
    uint32_t timeStampOffsetAddr = 0;
    uint8_t *bytePtr;
    uint64_t Nanoseconds64;

#ifdef PROFINET_PTCP
    uint64_t *timeStampAddress = 0;
    uint64_t cycleCounter, iepCount;
#endif //PROFINET_PTCP

    if(timeSyncParamsHandle->emacHandle == NULL)
    {
        return ERROR_HANDLE_INVALID;
    }

    /* Input error check */
    if((rxFrameType != SYNC_FRAME)
            && (rxFrameType != DELAY_REQ_FRAME)
            && (rxFrameType != DELAY_RESP_FRAME))
    {
        return -1;    /* not a valid frameType value */
    }

    if((rxPort != ICSS_EMAC_PORT_1)
            && (rxPort != ICSS_EMAC_PORT_2))
    {
        return -2;    /* not a valid port number */
    }

    if(rxFrameType == SYNC_FRAME)
    {
        if(rxPort == ICSS_EMAC_PORT_1)
        {
            timeStampOffsetAddr = RX_SYNC_TIMESTAMP_OFFSET_P1;
        }

        else
        {
            timeStampOffsetAddr = RX_SYNC_TIMESTAMP_OFFSET_P2;
        }
    }

    else if(rxFrameType == DELAY_REQ_FRAME)
    {
        if(rxPort == ICSS_EMAC_PORT_1)
        {
            timeStampOffsetAddr = RX_PDELAY_REQ_TIMESTAMP_OFFSET_P1;
        }

        else
        {
            timeStampOffsetAddr = RX_PDELAY_REQ_TIMESTAMP_OFFSET_P2;
        }
    }

    else /* (rxFrameType == DELAY_RESP_FRAME) */
    {
        if(rxPort == ICSS_EMAC_PORT_1)
        {
            timeStampOffsetAddr = RX_PDELAY_RESP_TIMESTAMP_OFFSET_P1;
        }

        else
        {
            timeStampOffsetAddr = RX_PDELAY_RESP_TIMESTAMP_OFFSET_P2;
        }
    }

#ifdef PROFINET_PTCP
    timeStampAddress = (uint64_t *)(pruicssHwAttrs->pru1DramBase +
                                    timeStampOffsetAddr);
    iepCount = ((*timeStampAddress) & 0x00000000FFFFFFFF);
    timeStampAddress = (uint64_t *)(pruicssHwAttrs->pru1DramBase +
                                    timeStampOffsetAddr + 4);
    cycleCounter = *timeStampAddress;
    Nanoseconds64 = (cycleCounter * RTC_3125_CLK_CONST) + iepCount;
#endif //PROFINET_PTCP

    bytePtr = (uint8_t *)((uint32_t)(pruicssHwAttrs->sharedDramBase +
                                     timeStampOffsetAddr));

    if(V2 == timeSyncParamsHandle->timeSyncConfig.icssVersion)
    {
        memcpy(&Nanoseconds64, bytePtr, 8);
        *nanoseconds = (uint32_t)(Nanoseconds64 % (uint64_t)SEC_TO_NS);
        *seconds = Nanoseconds64 / (uint64_t)SEC_TO_NS;
    }

    else
    {
        memcpy(nanoseconds, bytePtr, 4);
        memcpy(seconds, bytePtr + 4, 6);
    }

#ifdef PROFINET_PTCP

    if(timeSyncParamsHandle->rxprotocol == 1)
    {
        timeSyncParamsHandle->rxTimestamp_PTCP->Nanoseconds = Nanoseconds64 %
                1000000000;
        timeSyncParamsHandle->rxTimestamp_PTCP->SecondsLow = cycleCounter / 32000;
        timeSyncParamsHandle->rxTimestamp_PTCP->SecondsHigh = 0;
        timeSyncParamsHandle->rxTimestamp_PTCP->Status = 0;
    }

#endif //#ifdef PROFINET_PTCP

    return 0;
}


int8_t TimeSync_adjTimeSlowComp(TimeSync_ParamsHandle_t timeSyncParamsHandle,
                                int32_t adjOffset)
{
    uint32_t compensation_period;
    uintptr_t iepBaseAddress = (((PRUICSS_HwAttrs const *)(timeSyncParamsHandle->pruicssHandle->hwAttrs))->iep0RegBase);

    if(timeSyncParamsHandle->emacHandle == NULL)
    {
        return ERROR_HANDLE_INVALID;
    }

    /* set compensation interval = sync interval/drift.
       Example: Sync interval=30ms, drift=300ns, compensation interval=30ms/300ns = 5*30000000/300 = 5*100000ns. */
    compensation_period  = (uint32_t)((double)(
                                          timeSyncParamsHandle->tsRunTimeVar->ltaSyncInterval) /
                                      (double)(abs(adjOffset)));

    if(adjOffset == 0) /*No compensation required*/
    {
        /* set compensation increment = 5ns (default val)*/
        HW_WR_REG32(iepBaseAddress + CSL_ICSS_G_PR1_IEP0_SLV_GLOBAL_CFG_REG, 0x00000551);
        HW_WR_REG32(iepBaseAddress + CSL_ICSS_G_PR1_IEP0_SLV_SLOW_COMPEN_REG, 0);
    }

    else if(adjOffset < 0) /* slave is faster*/
    {
        /* set compensation increment = 10ns (default val)*/
        HW_WR_REG32(iepBaseAddress + CSL_ICSS_G_PR1_IEP0_SLV_GLOBAL_CFG_REG, 0x00000A51);
        HW_WR_REG32(iepBaseAddress + CSL_ICSS_G_PR1_IEP0_SLV_SLOW_COMPEN_REG, compensation_period);
    }

    else    /* master is faster*/
    {
        /* set compensation increment = 0ns*/
        HW_WR_REG32(iepBaseAddress + CSL_ICSS_G_PR1_IEP0_SLV_GLOBAL_CFG_REG, 0x00000051);
        HW_WR_REG32(iepBaseAddress + CSL_ICSS_G_PR1_IEP0_SLV_SLOW_COMPEN_REG, compensation_period);
    }
    return TIMESYNC_OK;
}

int32_t TimeSync_setClockTime(TimeSync_ParamsHandle_t timeSyncParamsHandle)
{

#ifdef PROFINET_PTCP
    uint8_t SendClockFactor;
    uint32_t CycleTime;
    uint64_t CurrentCycleCounter;
    PRUICSS_HwAttrs const *pruicssHwAttrs = (PRUICSS_HwAttrs const *)(timeSyncParamsHandle->pruicssHandle->hwAttrs);
#endif //PROFINET_PTCP

    uint64_t NewCycleCounter;
    uint32_t iepBaseAddress;
    uint8_t *bytePtr;

    if(timeSyncParamsHandle->emacHandle == NULL)
    {
        return ERROR_HANDLE_INVALID;
    }

#ifdef PROFINET_PTCP
    /* Configure IEP count */
    SendClockFactor = HWREG(pruicssHwAttrs->pru0DramBase + RTC_SCF_OFFSET);
    CycleTime = SendClockFactor * RTC_3125_CLK_CONST;
    HWREG(iepBaseAddress + CSL_ICSS_G_PR1_IEP0_SLV_COUNT_REG0) =
        timeSyncParamsHandle->clockTime.nanoseconds % CycleTime;

    /* Configure cycle counter */
    CurrentCycleCounter = *((uint64_t *)(pruicssHwAttrs->pru0DramBase +
                                         RTC_CYCLE_COUNTER_OFFSET));
    NewCycleCounter = timeSyncParamsHandle->clockTime.seconds * 32000 +
                      (timeSyncParamsHandle->clockTime.nanoseconds -
                       (timeSyncParamsHandle->clockTime.nanoseconds % CycleTime)) / RTC_3125_CLK_CONST;
    /* Cycle counter updated based on guideline*/
    uint64_t curr = CurrentCycleCounter & 0x00007FFF;
    uint64_t new = NewCycleCounter & 0x00007FFF;

    if(new > curr)
    {
        *((uint64_t *)(pruicssHwAttrs->pru0DramBase + RTC_CYCLE_COUNTER_OFFSET)) =
            NewCycleCounter;
    }

    if(new < curr)
    {
        *((uint64_t *)(pruicssHwAttrs->pru0DramBase + RTC_CYCLE_COUNTER_OFFSET)) =
            NewCycleCounter ^ 0x00008000;
    }

#endif //PROFINET_PTCP

    /*Calculate the absolute time in nanoseconds*/
    NewCycleCounter = timeSyncParamsHandle->clockTime.seconds *
                      (uint64_t)SEC_TO_NS + \
                      (uint64_t)timeSyncParamsHandle->clockTime.nanoseconds;
    /*Write it to IEP register*/
    iepBaseAddress = (uint32_t)(((PRUICSS_HwAttrs const *)(timeSyncParamsHandle->pruicssHandle->hwAttrs))->iep0RegBase);
    bytePtr = (uint8_t *)((uint32_t)(iepBaseAddress + CSL_ICSS_G_PR1_IEP0_SLV_COUNT_REG0));
    memcpy(bytePtr, &NewCycleCounter, 8);

    return 0;
}

int8_t TimeSync_getLatchTime(TimeSync_ParamsHandle_t timeSyncParamsHandle,
                             uint32_t *nanoseconds, uint64_t *seconds)
{
    uint32_t iepBaseAddress;
    uint8_t *bytePtr;
    uint64_t seconds_and_nanosec;

    if(timeSyncParamsHandle == NULL)
    {
        return TIME_SYNC_HANDLE_NOT_INITIALIZED;
    }

    iepBaseAddress = (uint32_t)(((PRUICSS_HwAttrs const *)(timeSyncParamsHandle->pruicssHandle->hwAttrs))->iep0RegBase);

    if(V2 == timeSyncParamsHandle->timeSyncConfig.icssVersion)
    {
        bytePtr = (uint8_t *)((uint32_t)(iepBaseAddress + CSL_ICSS_G_PR1_IEP0_SLV_CAPR6_REG0));
        memcpy(&seconds_and_nanosec, bytePtr, 8);
        *nanoseconds = (uint32_t)(seconds_and_nanosec % (uint64_t)SEC_TO_NS);
        *seconds = seconds_and_nanosec / (uint64_t)SEC_TO_NS;
    }

    /*V1 or AM3/AM4 ICSS is not supported right now since it will require special handling for seconds component*/
    if(V1 == timeSyncParamsHandle->timeSyncConfig.icssVersion)
    {
        return TIME_SYNC_FEATURE_NOT_ENABLED;
    }

    return TIME_SYNC_OK;
}

void TimeSync_getCurrentTime(TimeSync_ParamsHandle_t timeSyncParamsHandle,
                             uint32_t *nanoseconds, uint64_t *seconds)
{
    uint32_t iepBaseAddress;
    uint32_t sharedRAMbaseAddress;
    uint8_t *bytePtr;
    uint64_t seconds_and_nanosec;

    iepBaseAddress = (uint32_t)(((PRUICSS_HwAttrs const *)(timeSyncParamsHandle->pruicssHandle->hwAttrs))->iep0RegBase);
    sharedRAMbaseAddress = (uint32_t)((PRUICSS_HwAttrs const *)(timeSyncParamsHandle->pruicssHandle->hwAttrs))->sharedDramBase;

    if(V1 == timeSyncParamsHandle->timeSyncConfig.icssVersion)
    {
        bytePtr = (uint8_t *)((uint32_t)(iepBaseAddress + CSL_ICSS_G_PR1_IEP0_SLV_COUNT_REG0));
        memcpy(nanoseconds, bytePtr, 4);
        bytePtr = (uint8_t *)((uint32_t)(sharedRAMbaseAddress +
                                         TIMESYNC_SECONDS_COUNT_OFFSET));
        memcpy(seconds, bytePtr, 6);
    }

    else
    {
        bytePtr = (uint8_t *)((uint32_t)(iepBaseAddress + CSL_ICSS_G_PR1_IEP0_SLV_COUNT_REG0));
        memcpy(&seconds_and_nanosec, bytePtr, 8);
        *nanoseconds = (uint32_t)(seconds_and_nanosec % (uint64_t)SEC_TO_NS);
        *seconds = seconds_and_nanosec / (uint64_t)SEC_TO_NS;
    }


}

