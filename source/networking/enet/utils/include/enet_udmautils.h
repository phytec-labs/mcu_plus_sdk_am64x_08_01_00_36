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
 * \file     enet_udmautils.h
 *
 * \brief    This file contains the function prototypes of the Enet UDMA
 *           application utility functions used in the Enet examples.
 *
 * NOTE: This library is meant only for Enet examples. Customers are not
 * encouraged to use this layer as these are very specific to the examples
 * written and the API behaviour and signature can change at any time to
 * suit the examples.
 */

#ifndef ENET_UDMAUTILS_H_
#define ENET_UDMAUTILS_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <enet.h>

#include <include/core/enet_dma.h>

#if defined(__KLOCWORK__)
#include <stdlib.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

uint32_t EnetAppUtils_getNavSSInstanceId(Enet_Type enetType);

Udma_DrvHandle EnetAppUtils_udmaOpen(Enet_Type enetType,
                                     Udma_InitPrms *pInitPrms);

void EnetAppUtils_udmaclose(Udma_DrvHandle hUdmaDrv);

uint64_t EnetAppUtils_virtToPhyFxn(const void *virtAddr,
                                   void *appData);

void *EnetAppUtils_phyToVirtFxn(uint64_t phyAddr,
                                void *appData);

void EnetAppUtils_setCommonTxChPrms(EnetUdma_OpenTxChPrms *pTxChPrms);

void EnetAppUtils_setCommonRxFlowPrms(EnetUdma_OpenRxFlowPrms *pRxFlowPrms);

uint32_t EnetAppUtils_getStartFlowIdx(Enet_Handle hEnet,
                                      uint32_t coreId);

void EnetAppUtils_absFlowIdx2FlowIdxOffset(Enet_Handle hEnet,
                                           uint32_t coreId,
                                           uint32_t absRxFlowId,
                                           uint32_t *pStartFlowIdx,
                                           uint32_t *pFlowIdxOffset);

int32_t EnetAppUtils_allocRxFlow(Enet_Handle hEnet,
                                 uint32_t coreKey,
                                 uint32_t coreId,
                                 uint32_t *rxFlowStartIdx,
                                 uint32_t *rxFlowIdx);

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
                             EnetUdma_OpenRxFlowPrms *pRxFlowPrms);

void EnetAppUtils_closeRxFlow(Enet_Type enetType,
                              Enet_Handle hEnet,
                              uint32_t coreKey,
                              uint32_t coreId,
                              bool useDfltFlow,
                              EnetDma_PktQ *pFqPktInfoQ,
                              EnetDma_PktQ *pCqPktInfoQ,
                              uint32_t rxFlowStartIdx,
                              uint32_t rxFlowIdx,
                              uint8_t *macAddr,
                              EnetDma_RxChHandle hRxFlow);

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
                                     EnetUdma_OpenRxFlowPrms *pRxFlowPrms);

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
                                      uint8_t *macAddr,
                                      EnetDma_RxChHandle hRxFlow);

int32_t EnetAppUtils_freeRxFlow(Enet_Handle hEnet,
                                uint32_t coreKey,
                                uint32_t coreId,
                                uint32_t rxFlowIdx);

int32_t EnetAppUtils_allocTxCh(Enet_Handle hEnet,
                               uint32_t coreKey,
                               uint32_t coreId,
                               uint32_t *txPSILThreadId);

void EnetAppUtils_openTxCh(Enet_Handle hEnet,
                           uint32_t coreKey,
                           uint32_t coreId,
                           uint32_t *pTxChNum,
                           EnetDma_TxChHandle *pTxChHandle,
                           EnetUdma_OpenTxChPrms *pCpswTxChCfg);

void EnetAppUtils_closeTxCh(Enet_Handle hEnet,
                            uint32_t coreKey,
                            uint32_t coreId,
                            EnetDma_PktQ *pFqPktInfoQ,
                            EnetDma_PktQ *pCqPktInfoQ,
                            EnetDma_TxChHandle hTxChHandle,
                            uint32_t txChNum);

int32_t EnetAppUtils_freeTxCh(Enet_Handle hEnet,
                              uint32_t coreKey,
                              uint32_t coreId,
                              uint32_t txChNum);

void EnetAppUtils_freePktInfoQ(EnetDma_PktQ *pPktInfoQ);

int32_t EnetAppUtils_regDfltRxFlow(Enet_Handle hEnet,
                                   uint32_t coreKey,
                                   uint32_t coreId,
                                   uint32_t rxFlowStartIdx,
                                   uint32_t rxFlowIdx);

int32_t EnetAppUtils_unregDfltRxFlow(Enet_Handle hEnet,
                                             uint32_t coreKey,
                                             uint32_t coreId,
                                             uint32_t rxFlowStartIdx,
                                             uint32_t rxFlowIdx);

int32_t EnetAppUtils_regDfltRxFlowForChIdx(Enet_Handle hEnet,
                                           uint32_t coreKey,
                                           uint32_t coreId,
                                           uint32_t chIdx,
                                           uint32_t rxFlowStartIdx,
                                           uint32_t rxFlowIdx);

int32_t EnetAppUtils_unregDfltRxFlowForChIdx(Enet_Handle hEnet,
                                             uint32_t coreKey,
                                             uint32_t coreId,
                                             uint32_t chIdx,
                                             uint32_t rxFlowStartIdx,
                                             uint32_t rxFlowIdx);

int32_t EnetAppUtils_regDstMacRxFlow(Enet_Handle hEnet,
                                          uint32_t coreKey,
                                          uint32_t coreId,
                                          uint32_t rxFlowStartIdx,
                                          uint32_t rxFlowIdx,
                                          uint8_t macAddress[ENET_MAC_ADDR_LEN]);

int32_t EnetAppUtils_unregDstMacRxFlow(Enet_Handle hEnet,
                                            uint32_t coreKey,
                                            uint32_t coreId,
                                            uint32_t rxFlowStartIdx,
                                            uint32_t rxFlowIdx,
                                            uint8_t macAddress[ENET_MAC_ADDR_LEN]);

/* ========================================================================== */
/*                       Static Function Definitions                          */
/* ========================================================================== */


#ifdef __cplusplus
}
#endif

#endif  /* ENET_UDMAUTILS_H_ */
