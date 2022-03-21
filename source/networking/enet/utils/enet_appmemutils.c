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

/**
 *  \file enet_appmemutils.c
 *
 *  \brief Enet DMA memory allocation utility functions.
 *
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

/* This is needed for memset/memcpy */
#include <string.h>

#include <enet.h>
#include <include/core/enet_utils.h>

#include <include/core/enet_dma.h>

#include "include/enet_appmemutils.h"
#include "include/enet_appmemutils_cfg.h"
#include "include/enet_apputils.h"

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

#if defined (ENET_SOC_HOSTPORT_DMA_TYPE_UDMA)
typedef struct EnetMem_DmaDescMem_s
{
    /*! The node element so this packet can be added to a queue
     * Note- Keep EnetQ_Node as first member always as driver uses generic Q functions
     *       and deferences to this member */
    EnetQ_Node node;

    /*! DMA descriptor element */
    EnetUdma_DmaDesc *dmaDescPtr;

    /*! DMA descriptor state, refer to CpswUtils_DescStateMemMgr */
    uint32_t dmaDescState;
}EnetMem_DmaDescMem;

/**
 *  \brief
 */
typedef struct EnetMem_RingMem_s
{
    /*! The node element so this packet can be added to a queue
     * Note- Keep EnetQ_Node as first member always as driver uses generic Q functions
     *       and deferences to this member */
    EnetQ_Node node;

    /*! Ring memory element */
    uint8_t * ringElePtr;
}EnetMem_RingMem;

#endif
/**
 *  \brief
 */
typedef struct EnetMem_EthPktMem_s
{
    /*! The node element so this packet can be added to a queue
     * Note- Keep EnetQ_Node as first member always as driver uses generic Q functions
     *       and deferences to this member */
    EnetQ_Node node;

    /*! Eth packet info structure - shared with driver */
    EnetDma_Pkt *dmaPktPtr;

    /*! Original packet size - we can't use this info from dmaPkt as app can change it */
    uint32_t orgBufSize;
}EnetMem_AppPktInfoMem;

typedef EnetQ EnetMem_DmaDescMemQ;

typedef EnetQ EnetMem_RingMemQ;

typedef EnetQ EnetMem_EthPktMemQ;

/**
 *  \brief
 */
typedef struct EnetMem_MemAllocObj_s
{
    /**< DMA packet Q */
    bool memUtilsInitFlag;

#if defined (ENET_SOC_HOSTPORT_DMA_TYPE_UDMA)
    /**< DMA packet Q */
    EnetMem_DmaDescMemQ dmaDescFreeQ;

    /**< Ring memory Q */
    EnetMem_RingMemQ ringMemQ;

#endif
    /**< Pool1 ethernet packet memory Q */
    EnetMem_EthPktMemQ ethPktMem_LargePoolQ;

    /**< Pool3 ethernet packet memory Q */
    EnetMem_EthPktMemQ ethPktMem_MediumPoolQ;

    /**< Pool3 ethernet packet memory Q */
    EnetMem_EthPktMemQ ethPktMem_SmallPoolQ;

#if defined (ENET_SOC_HOSTPORT_DMA_TYPE_UDMA)
    /* Enet UDMA DESC memories */
    EnetMem_DmaDescMem dmaDescMemArray[ENET_MEM_NUM_DESCS];

    /* RX & TX RingAcc memories */
    EnetMem_RingMem  ringMemArray[ENET_MEM_NUM_RINGS];

#endif
    /* Eth packet info memory Q - large pool */
    EnetMem_AppPktInfoMem appPktInfoMem[ENET_MEM_NUM_ALLOC_POOLS][ENET_MEM_LARGE_POOL_PACKET_NUM];

    /* Eth memuitls configuration structure */
    EnetMem_Cfg cfg;
} EnetMem_MemAllocObj;

/* Cpsw mem utils driver object */
static EnetMem_MemAllocObj gEnetMemObj = {.memUtilsInitFlag = false};

void EnetMem_initMemObjs(const EnetMem_Cfg *cfg, EnetMem_MemAllocObj *obj)
{
    uint32_t poolIdx,i;
    #if defined (ENET_SOC_HOSTPORT_DMA_TYPE_UDMA)
    EnetMem_RingMemPoolEntry *ringMem;
    EnetMem_DmaDescMemPoolEntry *dmaDescMem;
    #endif

    ENET_UTILS_COMPILETIME_ASSERT(ENET_ARRAYSIZE(cfg->pktBufPool) == ENET_ARRAYSIZE(obj->appPktInfoMem));
    for (poolIdx = 0; poolIdx < ENET_ARRAYSIZE(cfg->pktBufPool);poolIdx++)
    {
        uint8_t *pktAddr = cfg->pktBufPool[poolIdx].pktBufMem;
        EnetDma_Pkt *pktInfo = cfg->pktBufPool[poolIdx].pktInfoMem;

        for (i = 0; i < cfg->pktBufPool[poolIdx].numPkts; i++)
        {
            obj->appPktInfoMem[poolIdx][i].dmaPktPtr = pktInfo;
            obj->appPktInfoMem[poolIdx][i].dmaPktPtr->bufPtr = pktAddr;
            pktInfo++;
            EnetAppUtils_assert((uintptr_t)pktInfo <= ((uintptr_t)cfg->pktBufPool[poolIdx].pktInfoMem) + cfg->pktBufPool[poolIdx].pktInfoSize);
            pktAddr += ENET_UTILS_ALIGN(cfg->pktBufPool[poolIdx].pktSize, ENET_UTILS_CACHELINE_SIZE);
            EnetAppUtils_assert((uintptr_t)pktAddr <= ((uintptr_t)cfg->pktBufPool[poolIdx].pktBufMem) + cfg->pktBufPool[poolIdx].pktBufSize);
        }
    }
    #if defined (ENET_SOC_HOSTPORT_DMA_TYPE_UDMA)
    ringMem = cfg->ringMem.ringMemBase;
    for (i = 0; i < cfg->ringMem.numRings; i++)
    {
        EnetAppUtils_assert(i < ENET_ARRAYSIZE(obj->ringMemArray));
        obj->ringMemArray[i].ringElePtr = &ringMem->ringEle[0];
        ringMem++;
        EnetAppUtils_assert((uintptr_t)ringMem <= ((uintptr_t)cfg->ringMem.ringMemBase) + cfg->ringMem.ringMemSize);
    }
    dmaDescMem = cfg->dmaDescMem.descMemBase;
    for (i = 0; i < cfg->dmaDescMem.numDesc; i++)
    {
        EnetAppUtils_assert(i < ENET_ARRAYSIZE(obj->dmaDescMemArray));
        obj->dmaDescMemArray[i].dmaDescPtr = &dmaDescMem->dmaDesc;
        dmaDescMem++;
        EnetAppUtils_assert((uintptr_t)dmaDescMem <= ((uintptr_t)cfg->dmaDescMem.descMemBase) + cfg->dmaDescMem.descMemSize);
    }
    #endif
}

/*! Ethernet packet allocation function  */
EnetDma_Pkt *EnetMem_allocEthPkt(void *appPriv,
                                 uint32_t pktSize,
                                 uint32_t alignSize)
{
    EnetDma_Pkt *pPktInfo                     = NULL;
    EnetMem_AppPktInfoMem *pAppPktInfoMem = NULL;
    EnetMem_EthPktMemQ *selectedQ         = NULL;

    if (gEnetMemObj.memUtilsInitFlag == true)
    {
        if (gEnetMemObj.cfg.pktBufPool[ENET_MEM_POOLIDX_SMALL].pktSize  >= pktSize)
        {
            /* Allocate packet from  smallest pool*/
            selectedQ = &gEnetMemObj.ethPktMem_SmallPoolQ;
        }
        else if (gEnetMemObj.cfg.pktBufPool[ENET_MEM_POOLIDX_MEDIUM].pktSize >= pktSize)
        {
            selectedQ = &gEnetMemObj.ethPktMem_MediumPoolQ;
        }
        else if (gEnetMemObj.cfg.pktBufPool[ENET_MEM_POOLIDX_LARGE].pktSize >= pktSize)
        {
            selectedQ = &gEnetMemObj.ethPktMem_LargePoolQ;
        }
        else
        {
            selectedQ = NULL;
        }

        if (NULL != selectedQ)
        {
            pAppPktInfoMem = (EnetMem_AppPktInfoMem *)EnetQueue_deq(selectedQ);
        }

        if (NULL != pAppPktInfoMem)
        {
            pPktInfo = pAppPktInfoMem->dmaPktPtr;

            if (!ENET_UTILS_IS_ALIGNED(pPktInfo->bufPtr, alignSize))
            {
                EnetQueue_enq(selectedQ, &pAppPktInfoMem->node);
                pPktInfo = NULL;
            }
            else
            {
                pPktInfo->appPriv = (void *)appPriv;
                EnetDma_checkPktState(&pPktInfo->pktState,
                                        ENET_PKTSTATE_MODULE_MEMMGR,
                                        ENET_PKTSTATE_MEMMGR_FREE,
                                        ENET_PKTSTATE_MEMMGR_ALLOC);
                ENET_UTILS_SET_PKT_DRIVER_STATE(&pPktInfo->pktState,
                                                ENET_PKTSTATE_DMA_NOT_WITH_HW);
            }
        }
        else
        {
            pPktInfo = NULL;
        }
    }

    return pPktInfo;
}

static EnetMem_AppPktInfoMem * EnetMem_getPktInfoEntry(EnetMem_MemAllocObj *obj, EnetDma_Pkt *pPktInfo)
{
    uint32_t pktInfoIndex;
    uint32_t poolIdx;
    EnetMem_AppPktInfoMem *entry = NULL;

    for (poolIdx = 0; poolIdx < ENET_ARRAYSIZE(obj->appPktInfoMem); poolIdx++)
    {
        if ((pPktInfo >= obj->cfg.pktBufPool[poolIdx].pktInfoMem) && (pPktInfo < (EnetDma_Pkt *)((uintptr_t)obj->cfg.pktBufPool[poolIdx].pktInfoMem + obj->cfg.pktBufPool[poolIdx].pktInfoSize)))
        {
            break;
        }
    }
    if (poolIdx < ENET_ARRAYSIZE(obj->appPktInfoMem))
    {
        pktInfoIndex = pPktInfo - obj->cfg.pktBufPool[poolIdx].pktInfoMem;
        EnetAppUtils_assert(pktInfoIndex < obj->cfg.pktBufPool[poolIdx].numPkts);
        entry = &obj->appPktInfoMem[poolIdx][pktInfoIndex];
        EnetAppUtils_assert(entry->dmaPktPtr == pPktInfo);
    }
    return entry;
}

/*! Ethernet packet free function  */
void EnetMem_freeEthPkt(EnetDma_Pkt *pPktInfo)
{
    EnetMem_AppPktInfoMem *pAppPktInfoMem;

    if (gEnetMemObj.memUtilsInitFlag == true)
    {
        EnetDma_checkPktState(&pPktInfo->pktState,
                                ENET_PKTSTATE_MODULE_MEMMGR,
                                ENET_PKTSTATE_MEMMGR_ALLOC,
                                ENET_PKTSTATE_MEMMGR_FREE);
        pPktInfo->appPriv = NULL;
        pAppPktInfoMem    = EnetMem_getPktInfoEntry(&gEnetMemObj, pPktInfo);

        ENET_UTILS_COMPILETIME_ASSERT(offsetof(EnetMem_AppPktInfoMem, node) == 0);
        if(pAppPktInfoMem != NULL)
        {
            if (gEnetMemObj.cfg.pktBufPool[ENET_MEM_POOLIDX_SMALL].pktSize == pAppPktInfoMem->orgBufSize)
            {
                EnetQueue_enq(&gEnetMemObj.ethPktMem_SmallPoolQ,
                              &pAppPktInfoMem->node);
            }
            else if (gEnetMemObj.cfg.pktBufPool[ENET_MEM_POOLIDX_MEDIUM].pktSize == pAppPktInfoMem->orgBufSize)
            {
                EnetQueue_enq(&gEnetMemObj.ethPktMem_MediumPoolQ,
                              &pAppPktInfoMem->node);
            }
            else if (gEnetMemObj.cfg.pktBufPool[ENET_MEM_POOLIDX_LARGE].pktSize == pAppPktInfoMem->orgBufSize)
            {
                EnetQueue_enq(&gEnetMemObj.ethPktMem_LargePoolQ,
                              &pAppPktInfoMem->node);
            }
            else
            {
                /* packet is not allocated by me */
                EnetAppUtils_assert(false);
            }
        }
    }

    return;
}
#if defined (ENET_SOC_HOSTPORT_DMA_TYPE_UDMA)
/*! Ring memory allocation function  */
uint8_t *EnetMem_allocRingMem(void *appPriv,
                              uint32_t numRingEle,
                              uint32_t alignSize)
{
    uint8_t *ringMemPtr = NULL;
    EnetMem_RingMem *pRingMemEle;

    if (gEnetMemObj.memUtilsInitFlag == true)
    {
        EnetAppUtils_assert(numRingEle <= ENET_MEM_RING_MAX_ELEM_CNT);
        pRingMemEle = (EnetMem_RingMem *)EnetQueue_deq(&gEnetMemObj.ringMemQ);
        if (pRingMemEle != NULL)
        {
            ringMemPtr = &pRingMemEle->ringElePtr[0U];

            EnetAppUtils_assert(ENET_UTILS_IS_ALIGNED(ringMemPtr, alignSize));
        }
        else
        {
            ringMemPtr = NULL;
        }
    }

    return ringMemPtr;
}

static EnetMem_RingMem * EnetMem_getRingMemEntry(EnetMem_MemAllocObj *obj, void *ringMemPtr)
{
    EnetMem_RingMemPoolEntry *ringMemPoolEntry;
    EnetMem_RingMem *ringMem;
    uint32_t ringMemIdx;

    EnetAppUtils_assert(((uintptr_t)ringMemPtr >= (uintptr_t)obj->cfg.ringMem.ringMemBase)
                        &&
                        ((uintptr_t)ringMemPtr < ((uintptr_t)obj->cfg.ringMem.ringMemBase + obj->cfg.ringMem.ringMemSize)));
    ringMemPoolEntry = container_of(ringMemPtr, EnetMem_RingMemPoolEntry, ringEle[0]);
    ringMemIdx = ringMemPoolEntry - obj->cfg.ringMem.ringMemBase;
    EnetAppUtils_assert((ringMemIdx < ENET_ARRAYSIZE(obj->ringMemArray) && (ringMemIdx < obj->cfg.ringMem.numRings)));
    ringMem= &obj->ringMemArray[ringMemIdx];
    EnetAppUtils_assert(ringMem->ringElePtr == ringMemPtr);
    return ringMem;
}


/*! Ring memory free function  */
void EnetMem_freeRingMem(void *appPriv,
                         void *ringMemPtr,
                         uint32_t numRingEle)
{
    EnetMem_RingMem *pRingMemEle;

    if (gEnetMemObj.memUtilsInitFlag == true)
    {
        pRingMemEle = EnetMem_getRingMemEntry(&gEnetMemObj, ringMemPtr);
        /* TODO - just to get it compiling - Need to fix this after discussion with Misa/Badri */
        EnetQueue_enq(&gEnetMemObj.ringMemQ, &pRingMemEle->node);
    }

    return;
}

/*! DMA packet allocation function  */
EnetUdma_DmaDesc *EnetMem_allocDmaDesc(void *appPriv,
                                                 uint32_t alignSize)
{
    EnetUdma_DmaDesc *dmaDescPtr = NULL;
    EnetMem_DmaDescMem *pDmaDescMem;

    if (gEnetMemObj.memUtilsInitFlag == true)
    {
        pDmaDescMem = (EnetMem_DmaDescMem *)
                      EnetQueue_deq(&gEnetMemObj.dmaDescFreeQ);
        if (NULL != pDmaDescMem)
        {
            dmaDescPtr = pDmaDescMem->dmaDescPtr;
            if (!ENET_UTILS_IS_ALIGNED(dmaDescPtr, alignSize))
            {
                EnetQueue_enq(&gEnetMemObj.dmaDescFreeQ,
                              &pDmaDescMem->node);
                dmaDescPtr = NULL;
            }
            else
            {
                EnetDma_checkDescState(&pDmaDescMem->dmaDescState,
                                         ENET_DESCSTATE_MEMMGR_FREE,
                                         ENET_DESCSTATE_MEMMGR_ALLOC);
            }
        }
    }

    return dmaDescPtr;
}

static EnetMem_DmaDescMem * EnetMem_getDescMemEntry(EnetMem_MemAllocObj *obj, EnetUdma_DmaDesc *pDmaDescMem)
{
    EnetMem_DmaDescMemPoolEntry *descMemPoolEntry;
    EnetMem_DmaDescMem *descMem;
    uint32_t descMemIdx;

    EnetAppUtils_assert(((uintptr_t)pDmaDescMem >= (uintptr_t)&obj->cfg.dmaDescMem.descMemBase->dmaDesc)
                        &&
                        ((uintptr_t)pDmaDescMem < ((uintptr_t)&obj->cfg.dmaDescMem.descMemBase->dmaDesc + obj->cfg.dmaDescMem.descMemSize)));
    descMemPoolEntry = container_of(pDmaDescMem, EnetMem_DmaDescMemPoolEntry, dmaDesc);
    descMemIdx = descMemPoolEntry - obj->cfg.dmaDescMem.descMemBase;
    EnetAppUtils_assert(descMemIdx < ENET_ARRAYSIZE(obj->dmaDescMemArray) && (descMemIdx < obj->cfg.dmaDescMem.numDesc));
    descMem= &obj->dmaDescMemArray[descMemIdx];
    EnetAppUtils_assert(descMem->dmaDescPtr == pDmaDescMem);
    return descMem;
}

/*! DMA packet free function  */
void EnetMem_freeDmaDesc(void *appPriv,
                                    EnetUdma_DmaDesc *dmaDescPtr)
{
    EnetMem_DmaDescMem *pDmaDescMem;

    if (gEnetMemObj.memUtilsInitFlag == true)
    {
        pDmaDescMem = EnetMem_getDescMemEntry(&gEnetMemObj,dmaDescPtr);
        EnetDma_checkDescState(&pDmaDescMem->dmaDescState,
                                 ENET_DESCSTATE_MEMMGR_ALLOC,
                                 ENET_DESCSTATE_MEMMGR_FREE);
        EnetQueue_enq(&gEnetMemObj.dmaDescFreeQ, &pDmaDescMem->node);
    }

    return;
}
#endif

static int32_t EnetMem_initCore(const EnetMem_Cfg *cfg)
{
    uint32_t i;
    int32_t retVal = ENET_SOK;
    EnetDma_Pkt *dmaPkt;
    EnetMem_AppPktInfoMem *pAppPktInfoMem;
    uint32_t alignSize = ENETDMA_CACHELINE_ALIGNMENT;
    uint8_t *bufPtr;

    if (gEnetMemObj.memUtilsInitFlag == false)
    {
        memset(&gEnetMemObj, 0U, sizeof(EnetMem_MemAllocObj));

        gEnetMemObj.cfg = *cfg;
        EnetMem_initMemObjs(cfg, &gEnetMemObj);
#if defined (ENET_SOC_HOSTPORT_DMA_TYPE_UDMA)
        memset(gEnetMemObj.cfg.ringMem.ringMemBase , 0U, gEnetMemObj.cfg.ringMem.ringMemSize);

        /*********************** DMA packet Q ************************/
        if (ENET_SOK == retVal)
        {
            EnetQueue_initQ(&gEnetMemObj.dmaDescFreeQ);
            /* Initialize the DMA packet and enqueue into free packet Q */
            for (i = 0U; i < gEnetMemObj.cfg.dmaDescMem.numDesc; i++)
            {
                gEnetMemObj.dmaDescMemArray[i].dmaDescState = 0;
                if (!ENET_UTILS_IS_ALIGNED(gEnetMemObj.dmaDescMemArray[i].dmaDescPtr, alignSize))
                {
                    retVal = ENET_EFAIL;
                    break;
                }

                EnetQueue_enq(&gEnetMemObj.dmaDescFreeQ, &gEnetMemObj.dmaDescMemArray[i].node);
                ENET_UTILS_SET_DESC_MEMMGR_STATE(&gEnetMemObj.dmaDescMemArray[i].dmaDescState, ENET_DESCSTATE_MEMMGR_FREE);
            }
        }

        /*********************** Ring Mem Q ************************/
        if (ENET_SOK == retVal)
        {
            EnetQueue_initQ(&gEnetMemObj.ringMemQ);
            /* Enqueue ring memory element into free packet Q */
            for (i = 0U; i < ENET_MEM_NUM_RINGS; i++)
            {
                if (!ENET_UTILS_IS_ALIGNED(gEnetMemObj.ringMemArray[i].ringElePtr, alignSize))
                {
                    retVal = ENET_EFAIL;
                    break;
                }

                EnetQueue_enq(&gEnetMemObj.ringMemQ, &gEnetMemObj.ringMemArray[i].node);
            }
        }
#endif
        /****************** memory Q for Large pool ethernet packets *****************/
        if (ENET_SOK == retVal)
        {
            EnetQueue_initQ(&gEnetMemObj.ethPktMem_LargePoolQ);
            /* Enqueue ring memory element into free packet Q */
            for (i = 0U; i < gEnetMemObj.cfg.pktBufPool[ENET_MEM_POOLIDX_LARGE].numPkts; i++)
            {
                pAppPktInfoMem = &gEnetMemObj.appPktInfoMem[ENET_MEM_POOLIDX_LARGE][i];
                /* Keep record of allocated size - we use this in free */
                pAppPktInfoMem->orgBufSize = gEnetMemObj.cfg.pktBufPool[ENET_MEM_POOLIDX_LARGE].pktSize;
                bufPtr = pAppPktInfoMem->dmaPktPtr->bufPtr;
                dmaPkt                = pAppPktInfoMem->dmaPktPtr;
                EnetDma_initPktInfo(dmaPkt);
                dmaPkt->bufPtr     = bufPtr;
                dmaPkt->userBufLen = gEnetMemObj.cfg.pktBufPool[ENET_MEM_POOLIDX_LARGE].pktSize;
                dmaPkt->orgBufLen  = gEnetMemObj.cfg.pktBufPool[ENET_MEM_POOLIDX_LARGE].pktSize;

                EnetAppUtils_assert(dmaPkt->bufPtr != NULL);
                if (!ENET_UTILS_IS_ALIGNED(dmaPkt->bufPtr, alignSize))
                {
                    retVal = ENET_EFAIL;
                    break;
                }

                ENET_UTILS_SET_PKT_MEMMGR_STATE(&dmaPkt->pktState,
                                                  ENET_PKTSTATE_MEMMGR_FREE);
                EnetQueue_enq(&gEnetMemObj.ethPktMem_LargePoolQ, &pAppPktInfoMem->node);
            }
        }

        /****************** memory Q for Medium pool ethernet packets *****************/
        if (ENET_SOK == retVal)
        {
            EnetQueue_initQ(&gEnetMemObj.ethPktMem_MediumPoolQ);
            /* Enqueue ring memory element into free packet Q */
            for (i = 0U; i < gEnetMemObj.cfg.pktBufPool[ENET_MEM_POOLIDX_MEDIUM].numPkts; i++)
            {
                pAppPktInfoMem = &gEnetMemObj.appPktInfoMem[ENET_MEM_POOLIDX_MEDIUM][i];
                /* Keep record of allocated size - we use this in free */
                pAppPktInfoMem->orgBufSize = gEnetMemObj.cfg.pktBufPool[ENET_MEM_POOLIDX_MEDIUM].pktSize;
                bufPtr = pAppPktInfoMem->dmaPktPtr->bufPtr;
                dmaPkt                = pAppPktInfoMem->dmaPktPtr;
                EnetDma_initPktInfo(dmaPkt);
                dmaPkt->bufPtr     = bufPtr;
                dmaPkt->userBufLen = gEnetMemObj.cfg.pktBufPool[ENET_MEM_POOLIDX_MEDIUM].pktSize;
                dmaPkt->orgBufLen  = gEnetMemObj.cfg.pktBufPool[ENET_MEM_POOLIDX_MEDIUM].pktSize;
                EnetAppUtils_assert(dmaPkt->bufPtr != NULL);
                if (!ENET_UTILS_IS_ALIGNED(dmaPkt->bufPtr, alignSize))
                {
                    retVal = ENET_EFAIL;
                    break;
                }

                ENET_UTILS_SET_PKT_MEMMGR_STATE(&dmaPkt->pktState, ENET_PKTSTATE_MEMMGR_FREE);
                EnetQueue_enq(&gEnetMemObj.ethPktMem_MediumPoolQ, &pAppPktInfoMem->node);
            }
        }

        /****************** memory Q for Small pool ethernet packets *****************/
        if (ENET_SOK == retVal)
        {
            EnetQueue_initQ(&gEnetMemObj.ethPktMem_SmallPoolQ);
            /* Enqueue ring memory element into free packet Q */
            for (i = 0U; i < gEnetMemObj.cfg.pktBufPool[ENET_MEM_POOLIDX_SMALL].numPkts; i++)
            {
                pAppPktInfoMem = &gEnetMemObj.appPktInfoMem[ENET_MEM_POOLIDX_SMALL][i];
                /* Keep record of allocated size - we use this in free */
                pAppPktInfoMem->orgBufSize = gEnetMemObj.cfg.pktBufPool[ENET_MEM_POOLIDX_SMALL].pktSize;
                bufPtr = pAppPktInfoMem->dmaPktPtr->bufPtr;
                dmaPkt                = pAppPktInfoMem->dmaPktPtr;
                EnetDma_initPktInfo(dmaPkt);
                dmaPkt->bufPtr     = bufPtr;
                dmaPkt->userBufLen = gEnetMemObj.cfg.pktBufPool[ENET_MEM_POOLIDX_SMALL].pktSize;
                dmaPkt->orgBufLen  = gEnetMemObj.cfg.pktBufPool[ENET_MEM_POOLIDX_SMALL].pktSize;

                EnetAppUtils_assert(dmaPkt->bufPtr != NULL);
                if (!ENET_UTILS_IS_ALIGNED(dmaPkt->bufPtr, alignSize))
                {
                    retVal = ENET_EFAIL;
                    break;
                }

                ENET_UTILS_SET_PKT_MEMMGR_STATE(&dmaPkt->pktState, ENET_PKTSTATE_MEMMGR_FREE);
                EnetQueue_enq(&gEnetMemObj.ethPktMem_SmallPoolQ, &pAppPktInfoMem->node);
            }
        }

        if (ENET_SOK == retVal)
        {
            gEnetMemObj.memUtilsInitFlag = true;
        }
    }

    EnetAppUtils_assert(retVal == ENET_SOK);
    return retVal;
}

void EnetMem_deInit(void)
{
    uint32_t i;
    int32_t status = ENET_SOK;
    EnetMem_AppPktInfoMem *pAppPktInfoMem;

    if (gEnetMemObj.memUtilsInitFlag)
    {
#if defined (ENET_SOC_HOSTPORT_DMA_TYPE_UDMA)
        if (EnetQueue_getQCount(&gEnetMemObj.ringMemQ) != ENET_MEM_NUM_RINGS)
        {
            EnetAppUtils_print("RingMemQ: Before: %d, after: %d\r\n",
                               ENET_MEM_NUM_RINGS,
                               EnetQueue_getQCount(&gEnetMemObj.ringMemQ));
            status = ENET_EFAIL;
        }

        if (EnetQueue_getQCount(&gEnetMemObj.dmaDescFreeQ) != ENET_MEM_NUM_DESCS)
        {
            EnetAppUtils_print("DmaDesQ: Before: %d, after: %d\r\n",
                               ENET_MEM_NUM_DESCS,
                               EnetQueue_getQCount(&gEnetMemObj.dmaDescFreeQ));
            status = ENET_EFAIL;
        }

#endif
        if (EnetQueue_getQCount(&gEnetMemObj.ethPktMem_LargePoolQ) !=
                                 ENET_MEM_LARGE_POOL_PACKET_NUM)
        {
            EnetAppUtils_print("PktMemQ Large: Before: %d, after: %d\r\n",
                               ENET_MEM_LARGE_POOL_PACKET_NUM,
                               EnetQueue_getQCount(&gEnetMemObj.ethPktMem_LargePoolQ));
            status = ENET_EFAIL;
        }

        /* Assert if any of the Q has memory leak */
        EnetAppUtils_assert(ENET_SOK == status);

        memset(&gEnetMemObj, 0U, sizeof(EnetMem_MemAllocObj));

#if defined (ENET_SOC_HOSTPORT_DMA_TYPE_UDMA)
        for (i = 0U; i < ENET_MEM_NUM_DESCS; i++)
        {
            EnetDma_checkDescState(&(gEnetMemObj.dmaDescMemArray[i].dmaDescState),
                                    ENET_DESCSTATE_MEMMGR_FREE,
                                    ENET_DESCSTATE_MEMMGR_FREE);
        }
#endif
        for (i = 0U; i < gEnetMemObj.cfg.pktBufPool[ENET_MEM_POOLIDX_SMALL].numPkts; i++)
        {
            pAppPktInfoMem = &gEnetMemObj.appPktInfoMem[ENET_MEM_POOLIDX_SMALL][i];
            EnetDma_Pkt *dmaPkt = pAppPktInfoMem->dmaPktPtr;

            EnetDma_checkPktState(&dmaPkt->pktState,
                                    ENET_PKTSTATE_MODULE_MEMMGR,
                                    ENET_PKTSTATE_MEMMGR_FREE,
                                    ENET_PKTSTATE_MEMMGR_FREE);
        }

        for (i = 0U; i < gEnetMemObj.cfg.pktBufPool[ENET_MEM_POOLIDX_MEDIUM].numPkts; i++)
        {
            pAppPktInfoMem = &gEnetMemObj.appPktInfoMem[ENET_MEM_POOLIDX_MEDIUM][i];
            EnetDma_Pkt *dmaPkt = pAppPktInfoMem->dmaPktPtr;

            EnetDma_checkPktState(&dmaPkt->pktState,
                                    ENET_PKTSTATE_MODULE_MEMMGR,
                                    ENET_PKTSTATE_MEMMGR_FREE,
                                    ENET_PKTSTATE_MEMMGR_FREE);
        }

        for (i = 0U; i < gEnetMemObj.cfg.pktBufPool[ENET_MEM_POOLIDX_LARGE].numPkts; i++)
        {
            pAppPktInfoMem = &gEnetMemObj.appPktInfoMem[ENET_MEM_POOLIDX_LARGE][i];
            EnetDma_Pkt *dmaPkt = pAppPktInfoMem->dmaPktPtr;

            EnetDma_checkPktState(&dmaPkt->pktState,
                                    ENET_PKTSTATE_MODULE_MEMMGR,
                                    ENET_PKTSTATE_MEMMGR_FREE,
                                    ENET_PKTSTATE_MEMMGR_FREE);
        }
    }
}

/*! Initialize Cpsw memutils module
 * Note - This function should be called after Cpsw is opened as it uses CpswUtils_Q
 * functions */
int32_t EnetMem_init(void)
{
    const EnetMem_Cfg *cfg =  EnetMem_getCfg();

    return (EnetMem_initCore(cfg));

}
/* ========================================================================== */
/*                          Static Function Definitions                       */
/* ========================================================================== */

/* end of file */
