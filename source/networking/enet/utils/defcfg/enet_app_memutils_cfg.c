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

#include "enet_appmemutils.h"
#include "enet_appmemutils_cfg.h"
#include "enet_apputils.h"

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */
/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

#if defined (ENET_SOC_HOSTPORT_DMA_TYPE_UDMA)
/* Enet UDMA DESC memories */
static EnetMem_DmaDescMemPoolEntry gDmaDescMemArray[ENET_MEM_NUM_DESCS]
__attribute__ ((aligned(ENETDMA_CACHELINE_ALIGNMENT),
                section(".bss:ENET_DMA_DESC_MEMPOOL")));

/* RX & TX RingAcc memories */
static EnetMem_RingMemPoolEntry gRingMemArray[ENET_MEM_NUM_RINGS]
__attribute__ ((aligned(ENETDMA_CACHELINE_ALIGNMENT),
                section(".bss:ENET_DMA_RING_MEMPOOL")));
#endif

/* Eth packet info memory Q - large pool */
static EnetDma_Pkt gAppPktInfoMem_LargePool[ENET_MEM_LARGE_POOL_PACKET_NUM];

/* Eth packet large pool memories */
static uint8_t gEthPktMem_LargePool[ENET_MEM_LARGE_POOL_PACKET_NUM][ENET_MEM_LARGE_POOL_PKT_SIZE]
__attribute__ ((aligned(ENETDMA_CACHELINE_ALIGNMENT),
                section(".bss:ENET_DMA_PKT_MEMPOOL")));

static const EnetMem_Cfg gEthMemCfg =
{
    .pktBufPool =
    {
        [ENET_MEM_POOLIDX_LARGE] =
        {
            .pktSize     = ENET_MEM_LARGE_POOL_PKT_SIZE,
            .numPkts     = ENET_MEM_LARGE_POOL_PACKET_NUM,
            .pktInfoMem  = gAppPktInfoMem_LargePool,
            .pktInfoSize = sizeof(gAppPktInfoMem_LargePool),
            .pktBufMem   = &gEthPktMem_LargePool[0][0],
            .pktBufSize  = sizeof(gEthPktMem_LargePool),
        },
    },
#if defined (ENET_SOC_HOSTPORT_DMA_TYPE_UDMA)
    .ringMem =
    {
        .numRings    =  ENET_MEM_NUM_RINGS,
        .ringMemBase =  gRingMemArray,
        .ringMemSize =  sizeof(gRingMemArray),
    },
    .dmaDescMem =
    {
        .numDesc     = ENET_MEM_NUM_DESCS,
        .descMemBase = gDmaDescMemArray,
        .descMemSize = sizeof(gDmaDescMemArray),
    },
#endif
};

const EnetMem_Cfg * EnetMem_getCfg(void)
{
    return &gEthMemCfg;
}