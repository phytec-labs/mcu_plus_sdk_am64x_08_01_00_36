/*
 *  Copyright (c) Texas Instruments Incorporated 2020
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

/*!
 * \file  enet_udmautils.c
 *
 * \brief This file contains the implementation of the Enet UDMA utils functions.
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdint.h>
#include <stdarg.h>

#include <enet.h>
#include <enet_cfg.h>
#include <include/per/cpsw.h>
#if defined(ENET_ENABLE_ICSSG)
#include <include/per/icssg.h>
#endif
#include <include/core/enet_dma.h>
#include "include/enet_appmemutils.h"
#include "include/enet_appmemutils_cfg.h"

#include "include/enet_apputils.h"
// #include "include/enet_appboardutils.h"

#include "include/enet_appsoc.h"
#include "include/enet_apprm.h"

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/* UDMA driver objects */
static Udma_DrvObject udmaDrvObj;

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

uint32_t EnetAppUtils_getNavSSInstanceId(Enet_Type enetType)
{
    uint32_t instId;

#if defined (SOC_AM64X) || defined (SOC_AM243X)
    instId = UDMA_INST_ID_PKTDMA_0;
#else
#error
#endif

    return instId;
}

Udma_DrvHandle EnetAppUtils_udmaOpen(Enet_Type enetType,
                                     Udma_InitPrms *pInitPrms)
{
    Udma_InitPrms initPrms;
    Udma_DrvHandle hUdmaDrv;
    int32_t retVal;
    uint32_t instId;

    hUdmaDrv = &udmaDrvObj;
    memset(hUdmaDrv, 0U, sizeof(*hUdmaDrv));

    /* Use default params if not passed by caller */
    if (NULL == pInitPrms)
    {
        instId = EnetAppUtils_getNavSSInstanceId(enetType);
        /* Initialize the UDMA driver based on NAVSS instance */
        UdmaInitPrms_init(instId, &initPrms);
        retVal            = Udma_init(hUdmaDrv, &initPrms);
    }
    else
    {
        retVal = Udma_init(hUdmaDrv, pInitPrms);
    }

    /* assert if UDMA failed to open */
    EnetAppUtils_assert(UDMA_SOK == retVal);

    return hUdmaDrv;
}

void EnetAppUtils_udmaclose(Udma_DrvHandle hUdmaDrv)
{
    EnetAppUtils_assert(NULL != hUdmaDrv);
    /* App should make sure that all Rx & Tx channels & are closed */
    /* Deinitialize the UDMA driver */
    Udma_deinit(hUdmaDrv);
}

void EnetAppUtils_freePktInfoQ(EnetDma_PktQ *pPktInfoQ)
{
    EnetDma_Pkt *pktInfo;
    uint32_t pktCnt, i;

    pktCnt = EnetQueue_getQCount(pPktInfoQ);
    /* Free all retrieved packets from DMA */
    for (i = 0U; i < pktCnt; i++)
    {
        pktInfo = (EnetDma_Pkt *)EnetQueue_deq(pPktInfoQ);
        EnetMem_freeEthPkt(pktInfo);
    }
}

uint64_t EnetAppUtils_virtToPhyFxn(const void *virtAddr,
                                   void *appData)
{
    return((uint64_t)virtAddr);
}

void *EnetAppUtils_phyToVirtFxn(uint64_t phyAddr,
                                void *appData)
{
#if defined(__aarch64__)
    uint64_t temp = phyAddr;
#else
    /* R5 is 32-bit machine, need to truncate to avoid void * typecast error */
    uint32_t temp = (uint32_t)phyAddr;
#endif
    return((void *)temp);
}

int32_t EnetAppUtils_allocRxFlow(Enet_Handle hEnet,
                                 uint32_t coreKey,
                                 uint32_t coreId,
                                 uint32_t *rxFlowStartIdx,
                                 uint32_t *flowIdx)
{
    int32_t status = ENET_SOK;
    Enet_IoctlPrms prms;
    EnetRm_AllocRxFlowInArgs inArgs;
    EnetRm_AllocRxFlow rxFlowPrms;

    inArgs.coreKey = coreKey;
    inArgs.chIdx   = 0U;

    ENET_IOCTL_SET_INOUT_ARGS(&prms, &inArgs, &rxFlowPrms);
    status = Enet_ioctl(hEnet,
                        coreId,
                        ENET_RM_IOCTL_ALLOC_RX_FLOW,
                        &prms);

    if (status == ENET_SOK)
    {
        *rxFlowStartIdx = rxFlowPrms.startIdx;
        *flowIdx        = rxFlowPrms.flowIdx;
    }
    else
    {
        EnetAppUtils_print("EnetAppUtils_allocRxFlow() failed : %d\n", status);
    }

    return status;
}

int32_t EnetAppUtils_allocRxFlowForChIdx(Enet_Handle hEnet,
                                         uint32_t coreKey,
                                         uint32_t coreId,
                                         uint32_t chIdx,
                                         uint32_t *rxFlowStartIdx,
                                         uint32_t *flowIdx)
{
    int32_t status = ENET_SOK;
    Enet_IoctlPrms prms;
    EnetRm_AllocRxFlowInArgs inArgs;
    EnetRm_AllocRxFlow rxFlowPrms;

    inArgs.coreKey = coreKey;
    inArgs.chIdx   = chIdx;

    ENET_IOCTL_SET_INOUT_ARGS(&prms, &inArgs, &rxFlowPrms);
    status = Enet_ioctl(hEnet,
                        coreId,
                        ENET_RM_IOCTL_ALLOC_RX_FLOW,
                        &prms);

    if (status == ENET_SOK)
    {
        *rxFlowStartIdx = rxFlowPrms.startIdx;
        *flowIdx        = rxFlowPrms.flowIdx;
    }
    else
    {
        EnetAppUtils_print("EnetAppUtils_allocRxFlowForChIdx() failed : %d\n", status);
    }

    return status;
}

int32_t EnetAppUtils_allocTxCh(Enet_Handle hEnet,
                               uint32_t coreKey,
                               uint32_t coreId,
                               uint32_t *txPSILThreadId)
{
    int32_t status = ENET_SOK;
    Enet_IoctlPrms prms;

    /* Allocate Tx Ch */
    ENET_IOCTL_SET_INOUT_ARGS(&prms, &coreKey, txPSILThreadId);
    status = Enet_ioctl(hEnet,
                        coreId,
                        ENET_RM_IOCTL_ALLOC_TX_CH_PEERID,
                        &prms);
    if (status != ENET_SOK)
    {
        *txPSILThreadId = ENET_RM_TXCHNUM_INVALID;
        EnetAppUtils_print("EnetAppUtils_allocTxCh() failed: %d\n", status);
    }

    return status;
}

int32_t EnetAppUtils_freeRxFlow(Enet_Handle hEnet,
                                uint32_t coreKey,
                                uint32_t coreId,
                                uint32_t rxFlowIdx)
{
    int32_t status = ENET_SOK;
    Enet_IoctlPrms prms;
    EnetRm_FreeRxFlowInArgs freeRxFlowInArgs;

    /*Free Rx Flow*/
    freeRxFlowInArgs.coreKey = coreKey;
    freeRxFlowInArgs.flowIdx = rxFlowIdx;
    freeRxFlowInArgs.chIdx   = 0U;

    ENET_IOCTL_SET_IN_ARGS(&prms, &freeRxFlowInArgs);
    status = Enet_ioctl(hEnet,
                        coreId,
                        ENET_RM_IOCTL_FREE_RX_FLOW,
                        &prms);

    return status;
}

int32_t EnetAppUtils_freeRxFlowForChIdx(Enet_Handle hEnet,
                                        uint32_t coreKey,
                                        uint32_t coreId,
                                        uint32_t chIdx,
                                        uint32_t rxFlowIdx)
{
    int32_t status = ENET_SOK;
    Enet_IoctlPrms prms;
    EnetRm_FreeRxFlowInArgs freeRxFlowInArgs;

    /*Free Rx Flow*/
    freeRxFlowInArgs.coreKey = coreKey;
    freeRxFlowInArgs.flowIdx = rxFlowIdx;
    freeRxFlowInArgs.chIdx   = chIdx;

    ENET_IOCTL_SET_IN_ARGS(&prms, &freeRxFlowInArgs);
    status = Enet_ioctl(hEnet,
                        coreId,
                        ENET_RM_IOCTL_FREE_RX_FLOW,
                        &prms);

    return status;
}

int32_t EnetAppUtils_freeTxCh(Enet_Handle hEnet,
                              uint32_t coreKey,
                              uint32_t coreId,
                              uint32_t txChNum)
{
    int32_t status = ENET_SOK;
    Enet_IoctlPrms prms;
    EnetRm_FreeTxChInArgs freeTxChInArgs;

    /* Release Tx Ch */
    freeTxChInArgs.coreKey = coreKey;
    freeTxChInArgs.txChNum = txChNum;

    ENET_IOCTL_SET_IN_ARGS(&prms, &freeTxChInArgs);
    status = Enet_ioctl(hEnet,
                        coreId,
                        ENET_RM_IOCTL_FREE_TX_CH_PEERID,
                        &prms);

    return status;
}

void EnetAppUtils_setCommonRxFlowPrms(EnetUdma_OpenRxFlowPrms *pRxFlowPrms)
{
    pRxFlowPrms->numRxPkts           = ENET_MEM_NUM_RX_PKTS;
    pRxFlowPrms->disableCacheOpsFlag = false;
    pRxFlowPrms->rxFlowMtu           = ENET_MEM_LARGE_POOL_PKT_SIZE;

    pRxFlowPrms->ringMemAllocFxn = &EnetMem_allocRingMem;
    pRxFlowPrms->ringMemFreeFxn  = &EnetMem_freeRingMem;

    pRxFlowPrms->dmaDescAllocFxn = &EnetMem_allocDmaDesc;
    pRxFlowPrms->dmaDescFreeFxn  = &EnetMem_freeDmaDesc;
}

void EnetAppUtils_setCommonTxChPrms(EnetUdma_OpenTxChPrms *pTxChPrms)
{
    pTxChPrms->numTxPkts           = ENET_MEM_NUM_TX_PKTS;
    pTxChPrms->disableCacheOpsFlag = false;

    pTxChPrms->ringMemAllocFxn = &EnetMem_allocRingMem;
    pTxChPrms->ringMemFreeFxn  = &EnetMem_freeRingMem;

    pTxChPrms->dmaDescAllocFxn = &EnetMem_allocDmaDesc;
    pTxChPrms->dmaDescFreeFxn  = &EnetMem_freeDmaDesc;
}

uint32_t EnetAppUtils_getStartFlowIdx(Enet_Handle hEnet,
                                      uint32_t coreId)
{
    Enet_IoctlPrms prms;
    uint32_t p0FlowIdOffset;
    int32_t status;

    ENET_IOCTL_SET_OUT_ARGS(&prms, &p0FlowIdOffset);
    status = Enet_ioctl(hEnet,
                        coreId,
                        CPSW_HOSTPORT_GET_FLOW_ID_OFFSET,
                        &prms);

    EnetAppUtils_assert(status == ENET_SOK);
    return p0FlowIdOffset;
}

void EnetAppUtils_absFlowIdx2FlowIdxOffset(Enet_Handle hEnet,
                                           uint32_t coreId,
                                           uint32_t absRxFlowId,
                                           uint32_t *pStartFlowIdx,
                                           uint32_t *pFlowIdxOffset)
{
    uint32_t p0FlowIdOffset;

    p0FlowIdOffset = EnetAppUtils_getStartFlowIdx(hEnet, coreId);
    EnetAppUtils_assert(absRxFlowId >= p0FlowIdOffset);
    *pStartFlowIdx  = p0FlowIdOffset;
    *pFlowIdxOffset = (absRxFlowId - p0FlowIdOffset);
}

void EnetAppUtils_openTxCh(Enet_Handle hEnet,
                           uint32_t coreKey,
                           uint32_t coreId,
                           uint32_t *pTxChNum,
                           EnetDma_TxChHandle *pTxChHandle,
                           EnetUdma_OpenTxChPrms *pCpswTxChCfg)
{
    EnetDma_Handle hDma = Enet_getDmaHandle(hEnet);
    int32_t status;

    EnetAppUtils_assert(hDma != NULL);

    status = EnetAppUtils_allocTxCh(hEnet,
                                    coreKey,
                                    coreId,
                                    pTxChNum);
    EnetAppUtils_assert(ENET_SOK == status);

    pCpswTxChCfg->chNum = *pTxChNum;

    *pTxChHandle = EnetDma_openTxCh(hDma, pCpswTxChCfg);
    EnetAppUtils_assert(NULL != *pTxChHandle);
}

void EnetAppUtils_closeTxCh(Enet_Handle hEnet,
                            uint32_t coreKey,
                            uint32_t coreId,
                            EnetDma_PktQ *pFqPktInfoQ,
                            EnetDma_PktQ *pCqPktInfoQ,
                            EnetDma_TxChHandle hTxChHandle,
                            uint32_t txChNum)
{
    int32_t status;

    EnetQueue_initQ(pFqPktInfoQ);
    EnetQueue_initQ(pCqPktInfoQ);

    EnetDma_disableTxEvent(hTxChHandle);
    status = EnetDma_closeTxCh(hTxChHandle, pFqPktInfoQ, pCqPktInfoQ);
    EnetAppUtils_assert(ENET_SOK == status);

    status = EnetAppUtils_freeTxCh(hEnet,
                                   coreKey,
                                   coreId,
                                   txChNum);
    EnetAppUtils_assert(ENET_SOK == status);
}

void EnetAppUtils_openRxFlow(Enet_Type enetType,
                             uint32_t instId,
                             Enet_Handle hEnet,
                             uint32_t coreKey,
                             uint32_t coreId,
                             bool useDfltFlow,
                             uint32_t *pRxFlowStartIdx,
                             uint32_t *pRxFlowIdx,
                             uint8_t macAddr[ENET_MAC_ADDR_LEN],
                             EnetDma_RxChHandle *pRxFlowHandle,
                             EnetUdma_OpenRxFlowPrms *pRxFlowPrms)
{
    EnetDma_Handle hDma = Enet_getDmaHandle(hEnet);
    int32_t status = ENET_SOK;

    EnetAppUtils_assert(hDma != NULL);

    status = EnetAppUtils_allocRxFlow(hEnet,
                                      coreKey,
                                      coreId,
                                      pRxFlowStartIdx,
                                      pRxFlowIdx);
    EnetAppUtils_assert(status == ENET_SOK);

    pRxFlowPrms->startIdx = *pRxFlowStartIdx;
    pRxFlowPrms->flowIdx  = *pRxFlowIdx;
    pRxFlowPrms->chIdx    = 0U;

    *pRxFlowHandle = EnetDma_openRxCh(hDma, pRxFlowPrms);
    EnetAppUtils_assert(*pRxFlowHandle != NULL);

    if (useDfltFlow)
    {
        status = EnetAppUtils_allocMac(hEnet, coreKey, coreId, macAddr);
        EnetAppUtils_assert(status == ENET_SOK);

        if (Enet_isCpswFamily(enetType))
        {
            EnetAppUtils_addHostPortEntry(hEnet, coreId, macAddr);
        }
        else
        {
            // Should we add this entry to ICSSG FDB?
        }

        status = EnetAppUtils_regDfltRxFlow(hEnet,
                                            coreKey,
                                            coreId,
                                            *pRxFlowStartIdx,
                                            *pRxFlowIdx);
    }
    else
    {
        if (Enet_isCpswFamily(enetType))
        {
            status = EnetAppUtils_allocMac(hEnet, coreKey, coreId, macAddr);
            EnetAppUtils_assert(status == ENET_SOK);
            status = EnetAppUtils_regDstMacRxFlow(hEnet,
                                                  coreKey,
                                                  coreId,
                                                  *pRxFlowStartIdx,
                                                  *pRxFlowIdx,
                                                  macAddr);
        }
    }

    EnetAppUtils_assert(status == ENET_SOK);
}

void EnetAppUtils_closeRxFlow(Enet_Type enetType,
                              Enet_Handle hEnet,
                              uint32_t coreKey,
                              uint32_t coreId,
                              bool useDfltFlow,
                              EnetDma_PktQ *pFqPktInfoQ,
                              EnetDma_PktQ *pCqPktInfoQ,
                              uint32_t rxFlowStartIdx,
                              uint32_t rxFlowIdx,
                              uint8_t macAddr[ENET_MAC_ADDR_LEN],
                              EnetDma_RxChHandle hRxFlow)
{
    int32_t status = ENET_SOK;

    EnetQueue_initQ(pFqPktInfoQ);
    EnetQueue_initQ(pCqPktInfoQ);

    EnetDma_disableRxEvent(hRxFlow);

    if (useDfltFlow)
    {
        status = EnetAppUtils_unregDfltRxFlow(hEnet,
                                              coreKey,
                                              coreId,
                                              rxFlowStartIdx,
                                              rxFlowIdx);
        EnetAppUtils_assert(status == ENET_SOK);

        status = EnetAppUtils_freeMac(hEnet,
                                      coreKey,
                                      coreId,
                                      macAddr);
        EnetAppUtils_assert(status == ENET_SOK);

        if (Enet_isCpswFamily(enetType))
        {
            EnetAppUtils_delAddrEntry(hEnet, coreId, macAddr);
        }
        else
        {
            // Should we add this entry to ICSSG FDB?
        }
    }
    else
    {
        if (Enet_isCpswFamily(enetType))
        {
            status = EnetAppUtils_unregDstMacRxFlow(hEnet,
                                                    coreKey,
                                                    coreId,
                                                    rxFlowStartIdx,
                                                    rxFlowIdx,
                                                    macAddr);
            EnetAppUtils_assert(status == ENET_SOK);

            EnetAppUtils_delAddrEntry(hEnet, coreId, macAddr);

            status = EnetAppUtils_freeMac(hEnet,
                                          coreKey,
                                          coreId,
                                          macAddr);
            EnetAppUtils_assert(status == ENET_SOK);
        }
    }

    status = EnetDma_closeRxCh(hRxFlow, pFqPktInfoQ, pCqPktInfoQ);
    EnetAppUtils_assert(status == ENET_SOK);

    status = EnetAppUtils_freeRxFlow(hEnet,
                                     coreKey,
                                     coreId,
                                     rxFlowIdx);
    EnetAppUtils_assert(status == ENET_SOK);
}

void EnetAppUtils_openRxFlowForChIdx(Enet_Type enetType,
                                     Enet_Handle hEnet,
                                     uint32_t coreKey,
                                     uint32_t coreId,
                                     bool useDfltFlow,
                                     uint32_t chIdx,
                                     uint32_t *pRxFlowStartIdx,
                                     uint32_t *pRxFlowIdx,
                                     uint8_t macAddr[ENET_MAC_ADDR_LEN],
                                     EnetDma_RxChHandle *pRxFlowHandle,
                                     EnetUdma_OpenRxFlowPrms *pRxFlowPrms)
{
    EnetDma_Handle hDma = Enet_getDmaHandle(hEnet);
    int32_t status = ENET_SOK;

    EnetAppUtils_assert(hDma != NULL);

    status = EnetAppUtils_allocRxFlowForChIdx(hEnet,
                                              coreKey,
                                              coreId,
                                              chIdx,
                                              pRxFlowStartIdx,
                                              pRxFlowIdx);
    EnetAppUtils_assert(status == ENET_SOK);

    pRxFlowPrms->startIdx = *pRxFlowStartIdx;
    pRxFlowPrms->flowIdx  = *pRxFlowIdx;
    pRxFlowPrms->chIdx    = chIdx;

    *pRxFlowHandle = EnetDma_openRxCh(hDma, pRxFlowPrms);
    EnetAppUtils_assert(*pRxFlowHandle != NULL);

    if (useDfltFlow)
    {
        if (chIdx == 0U)
        {
            status = EnetAppUtils_allocMac(hEnet, coreKey, coreId, macAddr);
            EnetAppUtils_assert(status == ENET_SOK);

            if (Enet_isCpswFamily(enetType))
            {
                EnetAppUtils_addHostPortEntry(hEnet, coreId, macAddr);
            }
            else
            {
                // Should we add this entry to ICSSG FDB?
            }
        }

        status = EnetAppUtils_regDfltRxFlowForChIdx(hEnet,
                                                    coreKey,
                                                    coreId,
                                                    chIdx,
                                                    *pRxFlowStartIdx,
                                                    *pRxFlowIdx);
    }
    else
    {
        if (Enet_isCpswFamily(enetType))
        {
            EnetAppUtils_assert(chIdx == 0U);

            status = EnetAppUtils_allocMac(hEnet, coreKey, coreId, macAddr);
            EnetAppUtils_assert(status == ENET_SOK);
            status = EnetAppUtils_regDstMacRxFlow(hEnet,
                                                  coreKey,
                                                  coreId,
                                                  *pRxFlowStartIdx,
                                                  *pRxFlowIdx,
                                                  macAddr);
        }
    }

    EnetAppUtils_assert(status == ENET_SOK);
}

void EnetAppUtils_closeRxFlowForChIdx(Enet_Type enetType,
                                      Enet_Handle hEnet,
                                      uint32_t coreKey,
                                      uint32_t coreId,
                                      bool useDfltFlow,
                                      EnetDma_PktQ *pFqPktInfoQ,
                                      EnetDma_PktQ *pCqPktInfoQ,
                                      uint32_t chIdx,
                                      uint32_t rxFlowStartIdx,
                                      uint32_t rxFlowIdx,
                                      uint8_t macAddr[ENET_MAC_ADDR_LEN],
                                      EnetDma_RxChHandle hRxFlow)
{
    int32_t status = ENET_SOK;

    EnetQueue_initQ(pFqPktInfoQ);
    EnetQueue_initQ(pCqPktInfoQ);

    EnetDma_disableRxEvent(hRxFlow);

    if (useDfltFlow)
    {
        status = EnetAppUtils_unregDfltRxFlowForChIdx(hEnet,
                                                      coreKey,
                                                      coreId,
                                                      chIdx,
                                                      rxFlowStartIdx,
                                                      rxFlowIdx);
        EnetAppUtils_assert(status == ENET_SOK);

        if (chIdx == 0U)
        {
            status = EnetAppUtils_freeMac(hEnet,
                                          coreKey,
                                          coreId,
                                          macAddr);
            EnetAppUtils_assert(status == ENET_SOK);

            if (Enet_isCpswFamily(enetType))
            {
                EnetAppUtils_delAddrEntry(hEnet, coreId, macAddr);
            }
        }
    }
    else
    {
        if (Enet_isCpswFamily(enetType))
        {
            EnetAppUtils_assert(chIdx == 0U);

            status = EnetAppUtils_unregDstMacRxFlow(hEnet,
                                                    coreKey,
                                                    coreId,
                                                    rxFlowStartIdx,
                                                    rxFlowIdx,
                                                    macAddr);
            EnetAppUtils_assert(status == ENET_SOK);

            EnetAppUtils_delAddrEntry(hEnet, coreId, macAddr);

            status = EnetAppUtils_freeMac(hEnet,
                                          coreKey,
                                          coreId,
                                          macAddr);
            EnetAppUtils_assert(status == ENET_SOK);
        }
    }

    status = EnetDma_closeRxCh(hRxFlow, pFqPktInfoQ, pCqPktInfoQ);
    EnetAppUtils_assert(status == ENET_SOK);


    status = EnetAppUtils_freeRxFlowForChIdx(hEnet,
                                             coreKey,
                                             coreId,
                                             chIdx,
                                             rxFlowIdx);
    EnetAppUtils_assert(status == ENET_SOK);
}

int32_t EnetAppUtils_regDfltRxFlow(Enet_Handle hEnet,
                                   uint32_t coreKey,
                                   uint32_t coreId,
                                   uint32_t rxFlowStartIdx,
                                   uint32_t rxFlowIdx)
{
    int32_t status = ENET_SOK;
    Enet_IoctlPrms prms;
    Enet_DfltFlowInfo dfltFlow;

    dfltFlow.coreKey  = coreKey;
    dfltFlow.chIdx    = 0U;
    dfltFlow.startIdx = rxFlowStartIdx;
    dfltFlow.flowIdx  = rxFlowIdx;
    ENET_IOCTL_SET_IN_ARGS(&prms, &dfltFlow);
    status = Enet_ioctl(hEnet,
                        coreId,
                        ENET_IOCTL_REGISTER_RX_DEFAULT_FLOW,
                        &prms);

    if (status != ENET_SOK)
    {
        EnetAppUtils_print("EnetAppUtils_regDfltRxFlow() failed : %d\n", status);
    }

    return status;
}

int32_t EnetAppUtils_unregDfltRxFlow(Enet_Handle hEnet,
                                     uint32_t coreKey,
                                     uint32_t coreId,
                                     uint32_t rxFlowStartIdx,
                                     uint32_t rxFlowIdx)
{
    int32_t status = ENET_SOK;
    Enet_IoctlPrms prms;
    Enet_DfltFlowInfo dfltFlow;

    dfltFlow.coreKey  = coreKey;
    dfltFlow.chIdx    = 0U;
    dfltFlow.startIdx = rxFlowStartIdx;
    dfltFlow.flowIdx  = rxFlowIdx;
    ENET_IOCTL_SET_IN_ARGS(&prms, &dfltFlow);
    status = Enet_ioctl(hEnet,
                        coreId,
                        ENET_IOCTL_UNREGISTER_RX_DEFAULT_FLOW,
                        &prms);

    if (status != ENET_SOK)
    {
        EnetAppUtils_print("EnetAppUtils_unregDfltRxFlow() failed : %d\n", status);
    }

    return status;
}

int32_t EnetAppUtils_regDfltRxFlowForChIdx(Enet_Handle hEnet,
                                           uint32_t coreKey,
                                           uint32_t coreId,
                                           uint32_t chIdx,
                                           uint32_t rxFlowStartIdx,
                                           uint32_t rxFlowIdx)
{
    int32_t status = ENET_SOK;
    Enet_IoctlPrms prms;
    Enet_DfltFlowInfo dfltFlow;

    dfltFlow.coreKey  = coreKey;
    dfltFlow.chIdx    = chIdx;
    dfltFlow.startIdx = rxFlowStartIdx;
    dfltFlow.flowIdx  = rxFlowIdx;
    ENET_IOCTL_SET_IN_ARGS(&prms, &dfltFlow);
    status = Enet_ioctl(hEnet,
                        coreId,
                        ENET_IOCTL_REGISTER_RX_DEFAULT_FLOW,
                        &prms);

    if (status != ENET_SOK)
    {
        EnetAppUtils_print("EnetAppUtils_regDfltRxFlowForChIdx() failed : %d\n", status);
    }

    return status;
}

int32_t EnetAppUtils_unregDfltRxFlowForChIdx(Enet_Handle hEnet,
                                             uint32_t coreKey,
                                             uint32_t coreId,
                                             uint32_t chIdx,
                                             uint32_t rxFlowStartIdx,
                                             uint32_t rxFlowIdx)
{
    int32_t status = ENET_SOK;
    Enet_IoctlPrms prms;
    Enet_DfltFlowInfo dfltFlow;

    dfltFlow.coreKey  = coreKey;
    dfltFlow.chIdx    = chIdx;
    dfltFlow.startIdx = rxFlowStartIdx;
    dfltFlow.flowIdx  = rxFlowIdx;
    ENET_IOCTL_SET_IN_ARGS(&prms, &dfltFlow);
    status = Enet_ioctl(hEnet,
                        coreId,
                        ENET_IOCTL_UNREGISTER_RX_DEFAULT_FLOW,
                        &prms);

    if (status != ENET_SOK)
    {
        EnetAppUtils_print("EnetAppUtils_unregDfltRxFlowForChIdx() failed : %d\n", status);
    }

    return status;
}

int32_t EnetAppUtils_regDstMacRxFlow(Enet_Handle hEnet,
                                          uint32_t coreKey,
                                          uint32_t coreId,
                                          uint32_t rxFlowStartIdx,
                                          uint32_t rxFlowIdx,
                                          uint8_t macAddress[ENET_MAC_ADDR_LEN])
{
    int32_t status = ENET_SOK;
    Enet_IoctlPrms prms;
    Enet_MacDstFlowInfo macDstFlow;

    macDstFlow.coreKey  = coreKey;
    macDstFlow.startIdx = rxFlowStartIdx;
    macDstFlow.flowIdx  = rxFlowIdx;
    EnetUtils_copyMacAddr(macDstFlow.macAddress, macAddress);
    ENET_IOCTL_SET_IN_ARGS(&prms, &macDstFlow);
    status = Enet_ioctl(hEnet,
                        coreId,
                        ENET_IOCTL_REGISTER_DSTMAC_RX_FLOW,
                        &prms);

    if (status != ENET_SOK)
    {
        EnetAppUtils_print("EnetAppUtils_regDstMacRxFlow() failed : %d\r\n", status);
    }

    return status;
}

int32_t EnetAppUtils_unregDstMacRxFlow(Enet_Handle hEnet,
                                            uint32_t coreKey,
                                            uint32_t coreId,
                                            uint32_t rxFlowStartIdx,
                                            uint32_t rxFlowIdx,
                                            uint8_t macAddress[ENET_MAC_ADDR_LEN])
{
    int32_t status = ENET_SOK;
    Enet_IoctlPrms prms;
    Enet_MacDstFlowInfo macDstFlow;

    macDstFlow.coreKey  = coreKey;
    macDstFlow.startIdx = rxFlowStartIdx;
    macDstFlow.flowIdx  = rxFlowIdx;
    EnetUtils_copyMacAddr(macDstFlow.macAddress, macAddress);
    ENET_IOCTL_SET_IN_ARGS(&prms, &macDstFlow);
    status = Enet_ioctl(hEnet,
                        coreId,
                        ENET_IOCTL_UNREGISTER_DSTMAC_RX_FLOW,
                        &prms);

    if (status != ENET_SOK)
    {
        EnetAppUtils_print("EnetAppUtils_regDstMacRxFlow() failed : %d\r\n", status);
    }

    return status;
}

