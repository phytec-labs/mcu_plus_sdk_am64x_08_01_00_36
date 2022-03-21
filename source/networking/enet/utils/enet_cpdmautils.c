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
 * \file  enet_cpdmautils.c
 *
 * \brief This file contains the implementation of the Enet CPDMA utils functions.
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdint.h>
#include <stdarg.h>

#include <enet.h>
#include <enet_cfg.h>
#include <include/per/cpsw.h>

#include <include/core/enet_dma.h>
#include "include/enet_appmemutils.h"
#include "include/enet_appmemutils_cfg.h"
#include "include/enet_apputils.h"
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

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

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

void EnetAppUtils_setCommonRxChPrms(EnetCpdma_OpenRxChPrms *pRxChPrms)
{
    pRxChPrms->numRxPkts           = ENET_MEM_NUM_RX_PKTS;
}

void EnetAppUtils_setCommonTxChPrms(EnetCpdma_OpenTxChPrms *pTxChPrms)
{
    pTxChPrms->numTxPkts           = ENET_MEM_NUM_TX_PKTS;
}

void EnetAppUtils_openTxCh(Enet_Handle hEnet,
                           uint32_t coreKey,
                           uint32_t coreId,
                           uint32_t *pTxChNum,
                           EnetDma_TxChHandle *pTxChHandle,
                           EnetCpdma_OpenTxChPrms *pCpswTxChCfg)
{
    EnetDma_Handle hDma = Enet_getDmaHandle(hEnet);

    EnetAppUtils_assert(hDma != NULL);

    pCpswTxChCfg->hEnet = hEnet;
    /* TODO: only one channel is supported */
    pCpswTxChCfg->chNum = *pTxChNum = 0U;

    *pTxChHandle = EnetDma_openTxCh(hDma, (void *)pCpswTxChCfg);
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
}

void EnetAppUtils_openRxCh(Enet_Handle hEnet,
                           uint32_t coreKey,
                           uint32_t coreId,
                           uint32_t *pRxChNum,
                           EnetDma_RxChHandle *pRxChHandle,
                           EnetCpdma_OpenRxChPrms *pCpswRxChCfg)
{
    EnetDma_Handle hDma = Enet_getDmaHandle(hEnet);

    EnetAppUtils_assert(hDma != NULL);

    pCpswRxChCfg->hEnet = hEnet;
    /* TODO: only one channel is supported */
    pCpswRxChCfg->chNum = *pRxChNum = 0U;

    *pRxChHandle = EnetDma_openRxCh(hDma, (void *)pCpswRxChCfg);
    EnetAppUtils_assert(NULL != *pRxChHandle);
}

void EnetAppUtils_closeRxCh(Enet_Handle hEnet,
                            uint32_t coreKey,
                            uint32_t coreId,
                            EnetDma_PktQ *pFqPktInfoQ,
                            EnetDma_PktQ *pCqPktInfoQ,
                            EnetDma_RxChHandle hRxChHandle,
                            uint32_t rxChNum)
{
    int32_t status;

    EnetQueue_initQ(pFqPktInfoQ);
    EnetQueue_initQ(pCqPktInfoQ);

    status = EnetDma_closeRxCh(hRxChHandle, pFqPktInfoQ, pCqPktInfoQ);
    EnetAppUtils_assert(ENET_SOK == status);
}

int32_t EnetAppUtils_freeTxCh(Enet_Handle hEnet,
                              uint32_t coreKey,
                              uint32_t coreId,
                              uint32_t txChNum)
{
    int32_t status = ENET_SOK;
    //FIXME - AM273X openRxCh doesn't allocate channel and uses hard coded ch 0
    return status;
}

int32_t EnetAppUtils_freeRxFlow(Enet_Handle hEnet,
                                uint32_t coreKey,
                                uint32_t coreId,
                                uint32_t rxFlowIdx)
{
    int32_t status = ENET_SOK;
    //FIXME - AM273X openRxCh doesn't allocate channel and uses hard coded ch 0
    return status;
}
