/*
 *  Copyright (c) Texas Instruments Incorporated 2021
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
 * \file  enet_soc_icssg_mem.c
 *
 * \brief This file contains ICSSG memories structures in AM64x SoC.
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdint.h>
#include <stdarg.h>
#include <enet_cfg.h>
#include <include/core/enet_per.h>
#include <include/per/icssg.h>
#include <priv/per/icssg_priv.h>
#include <soc/k3/icssg_soc.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/*
 * ICSSG Port buffer pool memory:
 *  - Dual-MAC: Not needed
 *  - Switch:   8 pools of 6kB each
 */
#define ICSSG_SWITCH_PORT_POOL_SIZE              (6U * 1024U)
#define ICSSG_PORT_POOL_TOTAL_SIZE               (ICSSG_SWITCH_PORT_BUFFER_POOL_NUM * ICSSG_SWITCH_PORT_POOL_SIZE)

/*
 * ICSSG Host buffer pool memory:
 *  - Dual-MAC: 8 pools of 8kB each
 *  - Switch:  16 pools of 6kB each
 */
#define ICSSG_DUALMAC_HOST_POOL_SIZE             (8U * 1024U)
#define ICSSG_DUALMAC_HOST_POOL_TOTAL_SIZE       (ICSSG_DUALMAC_HOST_BUFFER_POOL_NUM * ICSSG_DUALMAC_HOST_POOL_SIZE)

#define ICSSG_SWITCH_HOST_POOL_SIZE              (6U * 1024U)
#define ICSSG_SWITCH_HOST_POOL_TOTAL_SIZE        (ICSSG_SWITCH_HOST_BUFFER_POOL_NUM * ICSSG_SWITCH_HOST_POOL_SIZE)

#define ICSSG_HOST_POOL_TOTAL_SIZE               ((ICSSG_DUALMAC_HOST_POOL_TOTAL_SIZE > \
                                                   ICSSG_SWITCH_HOST_POOL_TOTAL_SIZE) ? \
                                                  ICSSG_DUALMAC_HOST_POOL_TOTAL_SIZE : \
                                                  ICSSG_SWITCH_HOST_POOL_TOTAL_SIZE)

/*
 * ICSSG Host egress queue memory:
 *  - Dual-MAC: 2 queues of (6kB + 2kB) each
 *  - Switch:   2 queues of (6kB + 2kB) each
 */
#define ICSSG_DUALMAC_HOST_QUEUE_SIZE            (8U * 1024U)
#define ICSSG_DUALMAC_HOST_QUEUE_TOTAL_SIZE      (ICSSG_DUALMAC_HOST_EGRESS_QUEUE_NUM * ICSSG_DUALMAC_HOST_QUEUE_SIZE)

#define ICSSG_SWITCH_HOST_QUEUE_SIZE             (8U * 1024U)
#define ICSSG_SWITCH_HOST_QUEUE_TOTAL_SIZE       (ICSSG_SWITCH_HOST_EGRESS_QUEUE_NUM * ICSSG_SWITCH_HOST_QUEUE_SIZE)

#define ICSSG_HOST_QUEUE_TOTAL_SIZE              ((ICSSG_DUALMAC_HOST_QUEUE_TOTAL_SIZE > \
                                                   ICSSG_SWITCH_HOST_QUEUE_TOTAL_SIZE) ? \
                                                  ICSSG_DUALMAC_HOST_QUEUE_TOTAL_SIZE : \
                                                  ICSSG_SWITCH_HOST_QUEUE_TOTAL_SIZE)

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

/* --------------------------------- ICSS-G --------------------------------- */

/* ICSSG0 memories */
uint8_t gEnetSoc_icssg0PortPoolMem_0[ICSSG_PORT_POOL_TOTAL_SIZE]
        __attribute__ ((aligned (ENETDMA_CACHELINE_ALIGNMENT)));

uint8_t gEnetSoc_icssg0HostPoolMem_0[ICSSG_HOST_POOL_TOTAL_SIZE]
        __attribute__ ((aligned (ENETDMA_CACHELINE_ALIGNMENT)));

uint8_t gEnetSoc_icssg0HostQueueMem_0[ICSSG_HOST_QUEUE_TOTAL_SIZE]
        __attribute__ ((aligned (ENETDMA_CACHELINE_ALIGNMENT)));

uint8_t gEnetSoc_icssg0ScratchMem_0[ICSSG_SCRATCH_BUFFER_SIZE]
        __attribute__ ((aligned (ENETDMA_CACHELINE_ALIGNMENT)));

uint8_t gEnetSoc_icssg0PortPoolMem_1[ICSSG_PORT_POOL_TOTAL_SIZE]
        __attribute__ ((aligned (ENETDMA_CACHELINE_ALIGNMENT)));

uint8_t gEnetSoc_icssg0HostPoolMem_1[ICSSG_HOST_POOL_TOTAL_SIZE]
        __attribute__ ((aligned (ENETDMA_CACHELINE_ALIGNMENT)));

uint8_t gEnetSoc_icssg0HostQueueMem_1[ICSSG_HOST_QUEUE_TOTAL_SIZE]
        __attribute__ ((aligned (ENETDMA_CACHELINE_ALIGNMENT)));

uint8_t gEnetSoc_icssg0ScratchMem_1[ICSSG_SCRATCH_BUFFER_SIZE]
        __attribute__ ((aligned (ENETDMA_CACHELINE_ALIGNMENT)));

/* ICSSG1 memories */
uint8_t gEnetSoc_icssg1PortPoolMem_0[ICSSG_PORT_POOL_TOTAL_SIZE]
        __attribute__ ((aligned (ENETDMA_CACHELINE_ALIGNMENT)));

uint8_t gEnetSoc_icssg1HostPoolMem_0[ICSSG_HOST_POOL_TOTAL_SIZE]
        __attribute__ ((aligned (ENETDMA_CACHELINE_ALIGNMENT)));

uint8_t gEnetSoc_icssg1HostQueueMem_0[ICSSG_HOST_QUEUE_TOTAL_SIZE]
        __attribute__ ((aligned (ENETDMA_CACHELINE_ALIGNMENT)));

uint8_t gEnetSoc_icssg1ScratchMem_0[ICSSG_SCRATCH_BUFFER_SIZE]
        __attribute__ ((aligned (ENETDMA_CACHELINE_ALIGNMENT)));

uint8_t gEnetSoc_icssg1PortPoolMem_1[ICSSG_PORT_POOL_TOTAL_SIZE]
        __attribute__ ((aligned (ENETDMA_CACHELINE_ALIGNMENT)));

uint8_t gEnetSoc_icssg1HostPoolMem_1[ICSSG_HOST_POOL_TOTAL_SIZE]
        __attribute__ ((aligned (ENETDMA_CACHELINE_ALIGNMENT)));

uint8_t gEnetSoc_icssg1HostQueueMem_1[ICSSG_HOST_QUEUE_TOTAL_SIZE]
        __attribute__ ((aligned (ENETDMA_CACHELINE_ALIGNMENT)));

uint8_t gEnetSoc_icssg1ScratchMem_1[ICSSG_SCRATCH_BUFFER_SIZE]
        __attribute__ ((aligned (ENETDMA_CACHELINE_ALIGNMENT)));

Icssg_FwPoolMem gEnetSoc_Icssg0_0_FwPoolMem[] =
{
    [0] =
    {
            .portBufferPoolMem   = NULL,
            .portBufferPoolSize  = 0U,
            .portBufferPoolNum   = 0U,

            .hostBufferPoolMem   = gEnetSoc_icssg0HostPoolMem_0,
            .hostBufferPoolSize  = ICSSG_DUALMAC_HOST_POOL_SIZE,
            .hostBufferPoolNum   = ICSSG_DUALMAC_HOST_BUFFER_POOL_NUM,

            .hostEgressQueueMem  = gEnetSoc_icssg0HostQueueMem_0,
            .hostEgressQueueSize = ICSSG_DUALMAC_HOST_QUEUE_SIZE,
            .hostEgressQueueNum  = ICSSG_DUALMAC_HOST_EGRESS_QUEUE_NUM,

            .scratchBufferMem    = gEnetSoc_icssg0ScratchMem_0,
            .scratchBufferSize   = ICSSG_SCRATCH_BUFFER_SIZE,
    },
};


Icssg_FwPoolMem gEnetSoc_Icssg0_1_FwPoolMem[] =
{
    [0] =
    {
        .portBufferPoolMem   = NULL,
        .portBufferPoolSize  = 0U,
        .portBufferPoolNum   = 0U,

        .hostBufferPoolMem   = gEnetSoc_icssg0HostPoolMem_1,
        .hostBufferPoolSize  = ICSSG_DUALMAC_HOST_POOL_SIZE,
        .hostBufferPoolNum   = ICSSG_DUALMAC_HOST_BUFFER_POOL_NUM,

        .hostEgressQueueMem  = gEnetSoc_icssg0HostQueueMem_1,
        .hostEgressQueueSize = ICSSG_DUALMAC_HOST_QUEUE_SIZE,
        .hostEgressQueueNum  = ICSSG_DUALMAC_HOST_EGRESS_QUEUE_NUM,

        .scratchBufferMem    = gEnetSoc_icssg0ScratchMem_1,
        .scratchBufferSize   = ICSSG_SCRATCH_BUFFER_SIZE,
    },
};

Icssg_FwPoolMem gEnetSoc_Icssg1_0_FwPoolMem[] =
{
    [0] =
    {
            .portBufferPoolMem   = NULL,
            .portBufferPoolSize  = 0U,
            .portBufferPoolNum   = 0U,

            .hostBufferPoolMem   = gEnetSoc_icssg1HostPoolMem_0,
            .hostBufferPoolSize  = ICSSG_DUALMAC_HOST_POOL_SIZE,
            .hostBufferPoolNum   = ICSSG_DUALMAC_HOST_BUFFER_POOL_NUM,

            .hostEgressQueueMem  = gEnetSoc_icssg1HostQueueMem_0,
            .hostEgressQueueSize = ICSSG_DUALMAC_HOST_QUEUE_SIZE,
            .hostEgressQueueNum  = ICSSG_DUALMAC_HOST_EGRESS_QUEUE_NUM,

            .scratchBufferMem    = gEnetSoc_icssg1ScratchMem_0,
            .scratchBufferSize   = ICSSG_SCRATCH_BUFFER_SIZE,
    },
};


Icssg_FwPoolMem gEnetSoc_Icssg1_1_FwPoolMem[] =
{
    [0] =
    {
            .portBufferPoolMem   = NULL,
            .portBufferPoolSize  = 0U,
            .portBufferPoolNum   = 0U,

            .hostBufferPoolMem   = gEnetSoc_icssg1HostPoolMem_1,
            .hostBufferPoolSize  = ICSSG_DUALMAC_HOST_POOL_SIZE,
            .hostBufferPoolNum   = ICSSG_DUALMAC_HOST_BUFFER_POOL_NUM,

            .hostEgressQueueMem  = gEnetSoc_icssg1HostQueueMem_1,
            .hostEgressQueueSize = ICSSG_DUALMAC_HOST_QUEUE_SIZE,
            .hostEgressQueueNum  = ICSSG_DUALMAC_HOST_EGRESS_QUEUE_NUM,

            .scratchBufferMem    = gEnetSoc_icssg1ScratchMem_1,
            .scratchBufferSize   = ICSSG_SCRATCH_BUFFER_SIZE,
    },
};


Icssg_FwPoolMem gEnetSoc_Icssg0_Swt_FwPoolMem[] =
{
    [0] =
    {
            .portBufferPoolMem   = gEnetSoc_icssg0PortPoolMem_0,
            .portBufferPoolSize  = ICSSG_SWITCH_PORT_POOL_SIZE,
            .portBufferPoolNum   = ICSSG_SWITCH_PORT_BUFFER_POOL_NUM,

            .hostBufferPoolMem   = gEnetSoc_icssg0HostPoolMem_0,
            .hostBufferPoolSize  = ICSSG_SWITCH_HOST_POOL_SIZE,
            .hostBufferPoolNum   = ICSSG_SWITCH_HOST_BUFFER_POOL_NUM,

            .hostEgressQueueMem  = gEnetSoc_icssg0HostQueueMem_0,
            .hostEgressQueueSize = ICSSG_SWITCH_HOST_QUEUE_SIZE,
            .hostEgressQueueNum  = ICSSG_SWITCH_HOST_EGRESS_QUEUE_NUM,

            .scratchBufferMem    = gEnetSoc_icssg0ScratchMem_0,
            .scratchBufferSize   = ICSSG_SCRATCH_BUFFER_SIZE,
        },
    [1] =
    {
            .portBufferPoolMem   = gEnetSoc_icssg0PortPoolMem_1,
            .portBufferPoolSize  = ICSSG_SWITCH_PORT_POOL_SIZE,
            .portBufferPoolNum   = ICSSG_SWITCH_PORT_BUFFER_POOL_NUM,

            .hostBufferPoolMem   = gEnetSoc_icssg0HostPoolMem_1,
            .hostBufferPoolSize  = ICSSG_SWITCH_HOST_POOL_SIZE,
            .hostBufferPoolNum   = ICSSG_SWITCH_HOST_BUFFER_POOL_NUM,

            .hostEgressQueueMem  = gEnetSoc_icssg0HostQueueMem_1,
            .hostEgressQueueSize = ICSSG_SWITCH_HOST_QUEUE_SIZE,
            .hostEgressQueueNum  = ICSSG_SWITCH_HOST_EGRESS_QUEUE_NUM,

            .scratchBufferMem    = gEnetSoc_icssg0ScratchMem_1,
            .scratchBufferSize   = ICSSG_SCRATCH_BUFFER_SIZE,
        },
};

Icssg_FwPoolMem gEnetSoc_Icssg1_Swt_FwPoolMem[] =
{
    [0] =
    {
            .portBufferPoolMem   = gEnetSoc_icssg1PortPoolMem_0,
            .portBufferPoolSize  = ICSSG_SWITCH_PORT_POOL_SIZE,
            .portBufferPoolNum   = ICSSG_SWITCH_PORT_BUFFER_POOL_NUM,

            .hostBufferPoolMem   = gEnetSoc_icssg1HostPoolMem_0,
            .hostBufferPoolSize  = ICSSG_SWITCH_HOST_POOL_SIZE,
            .hostBufferPoolNum   = ICSSG_SWITCH_HOST_BUFFER_POOL_NUM,

            .hostEgressQueueMem  = gEnetSoc_icssg1HostQueueMem_0,
            .hostEgressQueueSize = ICSSG_SWITCH_HOST_QUEUE_SIZE,
            .hostEgressQueueNum  = ICSSG_SWITCH_HOST_EGRESS_QUEUE_NUM,

            .scratchBufferMem    = gEnetSoc_icssg1ScratchMem_0,
            .scratchBufferSize   = ICSSG_SCRATCH_BUFFER_SIZE,
        },
    [1] =
    {
            .portBufferPoolMem   = gEnetSoc_icssg1PortPoolMem_1,
            .portBufferPoolSize  = ICSSG_SWITCH_PORT_POOL_SIZE,
            .portBufferPoolNum   = ICSSG_SWITCH_PORT_BUFFER_POOL_NUM,

            .hostBufferPoolMem   = gEnetSoc_icssg1HostPoolMem_1,
            .hostBufferPoolSize  = ICSSG_SWITCH_HOST_POOL_SIZE,
            .hostBufferPoolNum   = ICSSG_SWITCH_HOST_BUFFER_POOL_NUM,

            .hostEgressQueueMem  = gEnetSoc_icssg1HostQueueMem_1,
            .hostEgressQueueSize = ICSSG_SWITCH_HOST_QUEUE_SIZE,
            .hostEgressQueueNum  = ICSSG_SWITCH_HOST_EGRESS_QUEUE_NUM,

            .scratchBufferMem    = gEnetSoc_icssg1ScratchMem_1,
            .scratchBufferSize   = ICSSG_SCRATCH_BUFFER_SIZE,
        },
};


/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

/* None */

