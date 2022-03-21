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
 * \file  enet_soc.c
 *
 * \brief This file contains the implementation of AM64x SoC Ethernet.
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdint.h>
#include <stdarg.h>
#include <drivers/sciclient.h>
#include <drivers/hw_include/cslr_soc.h>
#include <hw_include/cslr_icss.h>
#include <hw_include/csl_cpswitch.h>
#include <enet_cfg.h>
#include <priv/mod/cpsw_ale_priv.h>
#include <priv/mod/cpsw_cpts_priv.h>
#include <priv/mod/cpsw_hostport_priv.h>
#include <priv/mod/cpsw_macport_priv.h>
#include <priv/mod/mdio_priv.h>
#include <priv/mod/cpsw_stats_priv.h>
#include <priv/mod/cpsw_clks.h>
#include <priv/core/enet_rm_priv.h>
#include <include/core/enet_utils.h>
#include <include/core/enet_osal.h>
#include <include/core/enet_soc.h>
#include <include/core/enet_per.h>
#include <include/per/cpsw.h>
#include <include/per/icssg.h>
#include <priv/per/cpsw_priv.h>
#include <priv/per/icssg_priv.h>
#include <soc/k3/cpsw_soc.h>
#include <soc/k3/icssg_soc.h>
#include <src/dma/udma/enet_udma_priv.h>

#include <src/per/firmware/icssg/dualmac/RX_PRU_SLICE0_bin.h>
#include <src/per/firmware/icssg/dualmac/RX_PRU_SLICE1_bin.h>
#include <src/per/firmware/icssg/dualmac/RTU0_SLICE0_bin.h>
#include <src/per/firmware/icssg/dualmac/RTU0_SLICE1_bin.h>
#include <src/per/firmware/icssg/dualmac/TX_PRU_SLICE0_bin.h>
#include <src/per/firmware/icssg/dualmac/TX_PRU_SLICE1_bin.h>

#include <src/per/firmware/icssg/switch/RX_PRU_SLICE0_bin.h>
#include <src/per/firmware/icssg/switch/RX_PRU_SLICE1_bin.h>
#include <src/per/firmware/icssg/switch/RTU0_SLICE0_bin.h>
#include <src/per/firmware/icssg/switch/RTU0_SLICE1_bin.h>
#include <src/per/firmware/icssg/switch/TX_PRU_SLICE0_bin.h>
#include <src/per/firmware/icssg/switch/TX_PRU_SLICE1_bin.h>

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

static uint32_t EnetSoc_getCoreDevId(void);

static uint32_t EnetSoc_getMcuEnetControl(Enet_MacPort macPort,
                                          uint32_t *modeSel);

static uint32_t EnetSoc_getMainEnetControl(Enet_MacPort macPort,
                                           uint32_t *modeSel);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/* -------------------------------- CPSW 3G --------------------------------- */

CpswSoc_Cfg gEnetSoc_cpsw3gSocCfg =
{
    /* As per the clocking specification the mcu_sysclk0 is 1000MHz with
     * fixed /3 divider. */
    .cppiClkFreqHz = 320000000LLU,
    .dmscDevId = TISCI_DEV_CPSW0,
    .intrs =
    {
        {   /* EVNT_PEND - Event pending interrupt (CPTS) */
            .intrId     = CPSW_INTR_EVNT_PEND,
            .coreIntNum = CSLR_R5FSS0_CORE0_INTR_CPSW0_EVNT_PEND_0,
            .srcIdx     = ENET_SOC_DIRECT_INTR_SRCIDX_INVALID,
        },
        {   /* STATS_PEND - Statistics pending interrupt */
            .intrId     = CPSW_INTR_STATS_PEND0,
            .coreIntNum = CSLR_R5FSS0_CORE0_INTR_CPSW0_STAT_PEND_0,
            .srcIdx     = ENET_SOC_DIRECT_INTR_SRCIDX_INVALID,
        },
        {   /* MDIO_PEND - MDIO interrupt */
            .intrId     = CPSW_INTR_MDIO_PEND,
            .coreIntNum = CSLR_R5FSS0_CORE0_INTR_CPSW0_MDIO_PEND_0,
            .srcIdx     = ENET_SOC_DIRECT_INTR_SRCIDX_INVALID,
        },
    },
    .clocks =
    {
        .cppiClkId        = TISCI_DEV_CPSW0_CPPI_CLK_CLK,
        .rgmii250MHzClkId = TISCI_DEV_CPSW0_RGMII_MHZ_250_CLK,
        .rgmii50MHzClkId  = TISCI_DEV_CPSW0_RGMII_MHZ_50_CLK,
        .rgmii5MHzClkId   = TISCI_DEV_CPSW0_RGMII_MHZ_5_CLK,
    },
    .txChPeerThreadId = CSL_PSILCFG_DMSS_CPSW2_PSILD_THREAD_OFFSET,
    .rxChPeerThreadId = CSL_PSILCFG_DMSS_CPSW2_PSILS_THREAD_OFFSET,
    .txChCount        = CSL_PSILCFG_DMSS_CPSW2_PSILD_THREAD_CNT,

    /* Note- Though CPSW supports 64 distinct flow Ids, there are only
     * 8 policer/classifier so can effectively assign only 8 flow ids in CPSW3G */
    .rxFlowCount     = 8U,
};

/* CPSW_3G MAC port template */
#define ENET_SOC_CPSW3G_MACPORT(n)                                    \
{                                                                     \
    .enetMod =                                                        \
    {                                                                 \
        .name       = "cpsw3G.macport" #n,                            \
        .physAddr   = (CSL_CPSW0_NUSS_BASE + CPSW_NU_OFFSET),         \
        .physAddr2  = (CSL_CPSW0_NUSS_BASE + CPSW_SGMII0_OFFSET(0U)), \
        .features   = (ENET_FEAT_BASE |                               \
                       CPSW_MACPORT_FEATURE_SGMII |                   \
                       CPSW_MACPORT_FEATURE_INTERVLAN),               \
        .errata     = ENET_ERRATA_NONE,                               \
        .open       = &CpswMacPort_open,                              \
        .rejoin     = &CpswMacPort_rejoin,                            \
        .ioctl      = &CpswMacPort_ioctl,                             \
        .close      = &CpswMacPort_close,                             \
    },                                                                \
    .macPort = ENET_MAC_PORT_ ## n,                                   \
}

/* CPSW_3G MAC ports */
CpswMacPort_Obj gEnetSoc_cpsw3gMacObj[] =
{
    ENET_SOC_CPSW3G_MACPORT(1),
    ENET_SOC_CPSW3G_MACPORT(2),
};


/* CPSW 3G Peripheral */
Cpsw_Obj gEnetSoc_cpsw3g =
{
    .enetPer =
    {
        .name         = "cpsw3g",
        .enetType     = ENET_CPSW_3G,
        .instId       = 0U,
        .magic        = ENET_NO_MAGIC,
        .physAddr     = (CSL_CPSW0_NUSS_BASE + CPSW_NU_OFFSET),
        .physAddr2    = (CSL_CPSW0_NUSS_BASE + CPSW_NUSS_OFFSET),
        .features     = (ENET_FEAT_BASE |
                         CPSW_FEATURE_INTERVLAN),
        .errata       = ENET_ERRATA_NONE,
        .initCfg      = &Cpsw_initCfg,
        .open         = &Cpsw_open,
        .rejoin       = &Cpsw_rejoin,
        .ioctl        = &Cpsw_ioctl,
        .poll         = &Cpsw_poll,
        .periodicTick = &Cpsw_periodicTick,
        .registerEventCb = NULL,
        .unregisterEventCb = NULL,
        .close        = &Cpsw_close,
    },

    /* Host port module object */
    .hostPortObj =
    {
        .enetMod =
        {
            .name       = "cpsw3g.hostport",
            .physAddr   = (CSL_CPSW0_NUSS_BASE + CPSW_NU_OFFSET),
            .features   = ENET_FEAT_BASE,
            .errata     = ENET_ERRATA_NONE,
            .open       = &CpswHostPort_open,
            .rejoin     = &CpswHostPort_rejoin,
            .ioctl      = &CpswHostPort_ioctl,
            .close      = &CpswHostPort_close,
        }
    },

    /* MAC port module objects */
    .macPortObj = gEnetSoc_cpsw3gMacObj,
    .macPortNum = ENET_ARRAYSIZE(gEnetSoc_cpsw3gMacObj),

    /* ALE module object */
    .aleObj =
    {
        .enetMod =
        {
            .name       = "cpsw3g.ale",
            .physAddr   = (CSL_CPSW0_NUSS_BASE + CPSW_ALE_OFFSET),
            .features   = (ENET_FEAT_BASE |
                           CPSW_ALE_FEATURE_FLOW_PRIORITY |
                           CPSW_ALE_FEATURE_IP_HDR_WHITELIST),
            .errata     = ENET_ERRATA_NONE,
            .open       = &CpswAle_open,
            .rejoin     = &CpswAle_rejoin,
            .ioctl      = &CpswAle_ioctl,
            .close      = &CpswAle_close,
        },
    },

    /* CPTS module object */
    .cptsObj =
    {
        .enetMod =
        {
            .name       = "cpsw3g.cpts",
            .physAddr   = (CSL_CPSW0_NUSS_BASE + CPSW_CPTS_OFFSET),
            .features   = ENET_FEAT_BASE,
            .errata     = CPSW_CPTS_ERRATA_GENFN_RECONFIG,
            .open       = &CpswCpts_open,
            .rejoin     = &CpswCpts_rejoin,
            .ioctl      = &CpswCpts_ioctl,
            .close      = &CpswCpts_close,
        },
        .hwPushCnt      = 8U,
    },

    /* MDIO module object */
    .mdioObj =
    {
        .enetMod =
        {
            .name       = "cpsw3g.mdio",
            .physAddr   = (CSL_CPSW0_NUSS_BASE + CPSW_MDIO_OFFSET),
            .features   = (ENET_FEAT_BASE |
                           MDIO_FEATURE_CLAUSE45),
            .errata     = ENET_ERRATA_NONE,
            .open       = &Mdio_open,
            .rejoin     = &Mdio_rejoin,
            .ioctl      = &Mdio_ioctl,
            .close      = &Mdio_close,
        },
    },

    /* Statistics module object */
    .statsObj =
    {
        .enetMod =
        {
            .name       = "cpsw3g.stats",
            .physAddr   = (CSL_CPSW0_NUSS_BASE + CPSW_NU_OFFSET),
            .features   = ENET_FEAT_BASE,
            .errata     = ENET_ERRATA_NONE,
            .open       = &CpswStats_open,
            .rejoin     = &CpswStats_rejoin,
            .ioctl      = &CpswStats_ioctl,
            .close      = &CpswStats_close,
        },
    },


    /* RM module object */
    .rmObj =
    {
        .enetMod =
        {
            .name       = "cpsw3g.rm",
            .physAddr   = 0U,
            .features   = ENET_FEAT_BASE,
            .errata     = ENET_ERRATA_NONE,
            .open       = &EnetRm_open,
            .rejoin     = &EnetRm_rejoin,
            .ioctl      = &EnetRm_ioctl,
            .close      = &EnetRm_close,
        },
    },
};

/* --------------------------------- ICSS-G --------------------------------- */

/* PRU_ICSSG 0 */
Icssg_Pruss gEnetSoc_PruIcssg0 =
{
    .instance    = ICSSG_PRUSS_ID_0,
    .initialized = false,
    .iep0InUse   = false,
};


/* ICSSG0 Dual-MAC DMA SoC data: 4 TX channels and 9 RX flows per slice */
IcssgSoc_Cfg gEnetSoc_icssg0DMacSocCfg[ICSSG_MAC_PORT_MAX] =
{
    {
        .dmscDevId        = TISCI_DEV_PRU_ICSSG0,
        .txChPeerThreadId = CSL_PSILCFG_DMSS_ICSS_G0_PSILD_THREAD_OFFSET,
        .rxChPeerThreadId = CSL_PSILCFG_DMSS_ICSS_G0_PSILS_THREAD_OFFSET,
        .txChCount        = ICSSG_DUALMAC_TX_CH_NUM,
        .rxFlowCount      = ICSSG_DUALMAC_RX_FLOW_NUM,
    },
    {
        .dmscDevId        = TISCI_DEV_PRU_ICSSG0,
        .txChPeerThreadId = CSL_PSILCFG_DMSS_ICSS_G0_PSILD_THREAD_OFFSET +
                            ICSSG_DUALMAC_TX_CH_NUM,
        .rxChPeerThreadId = CSL_PSILCFG_DMSS_ICSS_G0_PSILS_THREAD_OFFSET +
                            1U,
        .txChCount        = ICSSG_DUALMAC_TX_CH_NUM,
        .rxFlowCount      = ICSSG_DUALMAC_RX_FLOW_NUM,
    },
};

/* ICSSG0 Switch DMA SoC data: 4 TX channels and 9 RX flows */
IcssgSoc_Cfg gEnetSoc_icssg0SwtSocCfg =
{
    .dmscDevId        = TISCI_DEV_PRU_ICSSG0,
    .txChPeerThreadId = CSL_PSILCFG_DMSS_ICSS_G0_PSILD_THREAD_OFFSET,
    .rxChPeerThreadId = CSL_PSILCFG_DMSS_ICSS_G0_PSILS_THREAD_OFFSET,
    .txChCount        = ICSSG_SWITCH_TX_CH_NUM,
    .rxFlowCount      = ICSSG_SWITCH_RX_FLOW_NUM,
};

/* PRU_ICSSG 1 */
Icssg_Pruss gEnetSoc_PruIcssg1 =
{
    .instance    = ICSSG_PRUSS_ID_1,
    .initialized = false,
    .iep0InUse   = false,
};
/* ICSSG1 Dual-MAC DMA SoC data: 4 TX channels and 9 RX flows per slice */
IcssgSoc_Cfg gEnetSoc_icssg1DMacSocCfg[ICSSG_MAC_PORT_MAX] =
{
    {
        .dmscDevId        = TISCI_DEV_PRU_ICSSG1,
        .txChPeerThreadId = CSL_PSILCFG_DMSS_ICSS_G1_PSILD_THREAD_OFFSET,
        .rxChPeerThreadId = CSL_PSILCFG_DMSS_ICSS_G1_PSILS_THREAD_OFFSET,
        .txChCount        = ICSSG_DUALMAC_TX_CH_NUM,
        .rxFlowCount      = ICSSG_DUALMAC_RX_FLOW_NUM,
    },
    {
        .dmscDevId        = TISCI_DEV_PRU_ICSSG1,
        .txChPeerThreadId = CSL_PSILCFG_DMSS_ICSS_G1_PSILD_THREAD_OFFSET +
                            ICSSG_DUALMAC_TX_CH_NUM,
        .rxChPeerThreadId = CSL_PSILCFG_DMSS_ICSS_G1_PSILS_THREAD_OFFSET +
                            1U,
        .txChCount        = ICSSG_DUALMAC_TX_CH_NUM,
        .rxFlowCount      = ICSSG_DUALMAC_RX_FLOW_NUM,
    }
};

/* ICSSG1 Switch DMA SoC data: 4 TX channels and 9 RX flows */
IcssgSoc_Cfg gEnetSoc_icssg1SwtSocCfg =
{
    .dmscDevId        = TISCI_DEV_PRU_ICSSG1,
    .txChPeerThreadId = CSL_PSILCFG_DMSS_ICSS_G1_PSILD_THREAD_OFFSET,
    .rxChPeerThreadId = CSL_PSILCFG_DMSS_ICSS_G1_PSILS_THREAD_OFFSET,
    .txChCount        = ICSSG_SWITCH_TX_CH_NUM,
    .rxFlowCount      = ICSSG_SWITCH_RX_FLOW_NUM,
};

/* ICSSG0 Dual-MAC Port 1 (ENET_ICSSG_DUALMAC, 0) */
Icssg_Obj gEnetSoc_icssg0DMacp1 =
{
    .enetPer =
    {
        .name         = "icssg0-1",
        .enetType     = ENET_ICSSG_DUALMAC,
        .instId       = 0U,
        .magic        = ENET_NO_MAGIC,
        .physAddr     = CSL_PRU_ICSSG0_DRAM0_SLV_RAM_BASE,
        .physAddr2    = 0U,
        .features     = ENET_FEAT_BASE,
        .errata       = ENET_ERRATA_NONE,
        .initCfg      = &Icssg_initCfg,
        .open         = &Icssg_open,
        .close        = &Icssg_close,
        .rejoin       = &Icssg_rejoin,
        .ioctl        = &Icssg_ioctl,
        .poll         = &Icssg_poll,
        .convertTs    = &Icssg_convertTs,
        .periodicTick = &Icssg_periodicTick,
        .registerEventCb   = &Icssg_registerEventCb,
        .unregisterEventCb = &Icssg_unregisterEventCb,
    },

    /* Shared between ICSSG0 Port 1 and Port 2 objects */
    .pruss = &gEnetSoc_PruIcssg0,

    /* Dual-MAC firmware for slice 0 */
    .fw =
    {
        {
            .pru       = RX_PRU_SLICE0_b00_DMac,
            .pruSize   = sizeof(RX_PRU_SLICE0_b00_DMac),
            .rtu       = RTU0_SLICE0_b00_DMac,
            .rtuSize   = sizeof(RTU0_SLICE0_b00_DMac),
            .txpru     = TX_PRU_SLICE0_b00_DMac,
            .txpruSize = sizeof(TX_PRU_SLICE0_b00_DMac)
        },
    },

    /* TimeSync module object */
    .timeSyncObj =
    {
        .enetMod =
        {
            .name       = "icssg0-1.timesync",
            .physAddr   = CSL_PRU_ICSSG0_DRAM0_SLV_RAM_BASE + CSL_ICSS_G_PR1_IEP0_SLV_REGS_BASE,
            .physAddr2  = CSL_PRU_ICSSG0_DRAM0_SLV_RAM_BASE + CSL_ICSS_G_RAM_SLV_RAM_REGS_BASE,
            .features   = ENET_FEAT_BASE,
            .errata     = ENET_ERRATA_NONE,
            .open       = &IcssgTimeSync_open,
            .rejoin     = &IcssgTimeSync_rejoin,
            .ioctl      = &IcssgTimeSync_ioctl,
            .close      = &IcssgTimeSync_close,
        },
    },

    /* MDIO module object */
    .mdioObj =
    {
        .enetMod =
        {
            .name       = "icssg0-1.mdio",
            .physAddr   = CSL_PRU_ICSSG0_PR1_MDIO_V1P7_MDIO_BASE,
            .features   = MDIO_FEATURE_CLAUSE45,
            .errata     = ENET_ERRATA_NONE,
            .open       = &Mdio_open,
            .rejoin     = &Mdio_rejoin,
            .ioctl      = &Mdio_ioctl,
            .close      = &Mdio_close,
        },
    },

    /* Stats module object */
    .statsObj =
    {
        .enetMod =
        {
            .name       = "icssg0-1.stats",
            .physAddr   = CSL_PRU_ICSSG0_DRAM0_SLV_RAM_BASE,
            .physAddr2  = 0,
            .features   = ENET_FEAT_BASE,
            .errata     = ENET_ERRATA_NONE,
            .open       = &IcssgStats_open,
            .rejoin     = &IcssgStats_rejoin,
            .ioctl      = &IcssgStats_ioctl,
            .close      = &IcssgStats_close,
        },
    },

    /* RM module object */
    .rmObj =
    {
        .enetMod =
        {
            .name       = "icssg0-1.rm",
            .physAddr   = 0U,
            .features   = ENET_FEAT_BASE,
            .errata     = ENET_ERRATA_NONE,
            .open       = &EnetRm_open,
            .rejoin     = &EnetRm_rejoin,
            .ioctl      = &EnetRm_ioctl,
            .close      = &EnetRm_close,
        },
    },
};

/* ICSSG0 Dual-MAC Port 2 (ENET_ICSSG_DUALMAC, 1) */
Icssg_Obj gEnetSoc_icssg0DMacp2 =
{
    .enetPer =
    {
        .name         = "icssg0-2",
        .enetType     = ENET_ICSSG_DUALMAC,
        .instId       = 1U,
        .magic        = ENET_NO_MAGIC,
        .physAddr     = CSL_PRU_ICSSG0_DRAM0_SLV_RAM_BASE,
        .physAddr2    = 0U,
        .features     = ENET_FEAT_BASE,
        .errata       = ENET_ERRATA_NONE,
        .initCfg      = &Icssg_initCfg,
        .open         = &Icssg_open,
        .close        = &Icssg_close,
        .rejoin       = &Icssg_rejoin,
        .ioctl        = &Icssg_ioctl,
        .poll         = &Icssg_poll,
        .convertTs    = &Icssg_convertTs,
        .periodicTick = &Icssg_periodicTick,
        .registerEventCb   = &Icssg_registerEventCb,
        .unregisterEventCb = &Icssg_unregisterEventCb,
    },

    /* Shared between ICSSG0 Port 1 and Port 2 objects */
    .pruss = &gEnetSoc_PruIcssg0,

    /* Dual-MAC firmware for slice 1 */
    .fw =
    {
        {
            .pru       = RX_PRU_SLICE1_b00_DMac,
            .pruSize   = sizeof(RX_PRU_SLICE1_b00_DMac),
            .rtu       = RTU0_SLICE1_b00_DMac,
            .rtuSize   = sizeof(RTU0_SLICE1_b00_DMac),
            .txpru     = TX_PRU_SLICE1_b00_DMac,
            .txpruSize = sizeof(TX_PRU_SLICE1_b00_DMac)
        },
    },

    /* TimeSync module object */
    .timeSyncObj =
    {
        .enetMod =
        {
            .name       = "icssg0-2.timesync",
            .physAddr   = CSL_PRU_ICSSG0_DRAM0_SLV_RAM_BASE + CSL_ICSS_G_PR1_IEP0_SLV_REGS_BASE,
            .physAddr2  = CSL_PRU_ICSSG0_DRAM0_SLV_RAM_BASE + CSL_ICSS_G_RAM_SLV_RAM_REGS_BASE,
            .features   = ENET_FEAT_BASE,
            .errata     = ENET_ERRATA_NONE,
            .open       = &IcssgTimeSync_open,
            .rejoin     = &IcssgTimeSync_rejoin,
            .ioctl      = &IcssgTimeSync_ioctl,
            .close      = &IcssgTimeSync_close,
        },
    },

    /* MDIO module object */
    .mdioObj =
    {
        .enetMod =
        {
            .name       = "icssg0-2.mdio",
            .physAddr   = CSL_PRU_ICSSG0_PR1_MDIO_V1P7_MDIO_BASE,
            .features   = MDIO_FEATURE_CLAUSE45,
            .errata     = ENET_ERRATA_NONE,
            .open       = &Mdio_open,
            .rejoin     = &Mdio_rejoin,
            .ioctl      = &Mdio_ioctl,
            .close      = &Mdio_close,
        },
    },

    /* Stats module object */
    .statsObj =
    {
        .enetMod =
        {
            .name       = "icssg0-2.stats",
            .physAddr   = CSL_PRU_ICSSG0_DRAM0_SLV_RAM_BASE,
            .physAddr2  = 0,
            .features   = ENET_FEAT_BASE,
            .errata     = ENET_ERRATA_NONE,
            .open       = &IcssgStats_open,
            .rejoin     = &IcssgStats_rejoin,
            .ioctl      = &IcssgStats_ioctl,
            .close      = &IcssgStats_close,
        },
    },

    /* RM module object */
    .rmObj =
    {
        .enetMod =
        {
            .name       = "icssg0-2.rm",
            .physAddr   = 0U,
            .features   = ENET_FEAT_BASE,
            .errata     = ENET_ERRATA_NONE,
            .open       = &EnetRm_open,
            .rejoin     = &EnetRm_rejoin,
            .ioctl      = &EnetRm_ioctl,
            .close      = &EnetRm_close,
        },
    },
};

/* ICSSG1 Dual-MAC Port 1 (ENET_ICSSG_DUALMAC, 2) */
Icssg_Obj gEnetSoc_icssg1DMacp1 =
{
    .enetPer =
    {
        .name         = "icssg1-1",
        .enetType     = ENET_ICSSG_DUALMAC,
        .instId       = 2U,
        .magic        = ENET_NO_MAGIC,
        .physAddr     = CSL_PRU_ICSSG1_DRAM0_SLV_RAM_BASE,
        .physAddr2    = 0U,
        .features     = ENET_FEAT_BASE,
        .errata       = ENET_ERRATA_NONE,
        .initCfg      = &Icssg_initCfg,
        .open         = &Icssg_open,
        .close        = &Icssg_close,
        .rejoin       = &Icssg_rejoin,
        .ioctl        = &Icssg_ioctl,
        .poll         = &Icssg_poll,
        .convertTs    = &Icssg_convertTs,
        .periodicTick = &Icssg_periodicTick,
        .registerEventCb   = &Icssg_registerEventCb,
        .unregisterEventCb = &Icssg_unregisterEventCb,
    },

    /* Shared between ICSSG1 Port 1 and Port 2 objects */
    .pruss = &gEnetSoc_PruIcssg1,

    /* Dual-MAC firmware for slice 0 */
    .fw =
    {
        {
            .pru       = RX_PRU_SLICE0_b00_DMac,
            .pruSize   = sizeof(RX_PRU_SLICE0_b00_DMac),
            .rtu       = RTU0_SLICE0_b00_DMac,
            .rtuSize   = sizeof(RTU0_SLICE0_b00_DMac),
            .txpru     = TX_PRU_SLICE0_b00_DMac,
            .txpruSize = sizeof(TX_PRU_SLICE0_b00_DMac)
        },
    },

    /* TimeSync module object */
    .timeSyncObj =
    {
        .enetMod =
        {
            .name       = "icssg1-1.timesync",
            .physAddr   = CSL_PRU_ICSSG1_DRAM0_SLV_RAM_BASE + CSL_ICSS_G_PR1_IEP0_SLV_REGS_BASE,
            .physAddr2  = CSL_PRU_ICSSG1_DRAM0_SLV_RAM_BASE + CSL_ICSS_G_RAM_SLV_RAM_REGS_BASE,
            .features   = ENET_FEAT_BASE,
            .errata     = ENET_ERRATA_NONE,
            .open       = &IcssgTimeSync_open,
            .rejoin     = &IcssgTimeSync_rejoin,
            .ioctl      = &IcssgTimeSync_ioctl,
            .close      = &IcssgTimeSync_close,
        },
    },

    /* MDIO module object */
    .mdioObj =
    {
        .enetMod =
        {
            .name       = "icssg1-1.mdio",
            .physAddr   = CSL_PRU_ICSSG1_PR1_MDIO_V1P7_MDIO_BASE,
            .features   = MDIO_FEATURE_CLAUSE45,
            .errata     = ENET_ERRATA_NONE,
            .open       = &Mdio_open,
            .rejoin     = &Mdio_rejoin,
            .ioctl      = &Mdio_ioctl,
            .close      = &Mdio_close,
        },
    },

    /* Stats module object */
    .statsObj =
    {
        .enetMod =
        {
            .name       = "icssg1-1.stats",
            .physAddr   = CSL_PRU_ICSSG1_DRAM0_SLV_RAM_BASE,
            .physAddr2  = 0,
            .features   = ENET_FEAT_BASE,
            .errata     = ENET_ERRATA_NONE,
            .open       = &IcssgStats_open,
            .rejoin     = &IcssgStats_rejoin,
            .ioctl      = &IcssgStats_ioctl,
            .close      = &IcssgStats_close,
        },
    },

    /* RM module object */
    .rmObj =
    {
        .enetMod =
        {
            .name       = "icssg1-1.rm",
            .physAddr   = 0U,
            .features   = ENET_FEAT_BASE,
            .errata     = ENET_ERRATA_NONE,
            .open       = &EnetRm_open,
            .rejoin     = &EnetRm_rejoin,
            .ioctl      = &EnetRm_ioctl,
            .close      = &EnetRm_close,
        },
    },
};

/* ICSSG1 Dual-MAC Port 2 (ENET_ICSSG_DUALMAC, 3) */
Icssg_Obj gEnetSoc_icssg1DMacp2 =
{
    .enetPer =
    {
        .name         = "icssg1-2",
        .enetType     = ENET_ICSSG_DUALMAC,
        .instId       = 3U,
        .magic        = ENET_NO_MAGIC,
        .physAddr     = CSL_PRU_ICSSG1_DRAM0_SLV_RAM_BASE,
        .physAddr2    = 0U,
        .features     = ENET_FEAT_BASE,
        .errata       = ENET_ERRATA_NONE,
        .initCfg      = &Icssg_initCfg,
        .open         = &Icssg_open,
        .close        = &Icssg_close,
        .rejoin       = &Icssg_rejoin,
        .ioctl        = &Icssg_ioctl,
        .poll         = &Icssg_poll,
        .convertTs    = &Icssg_convertTs,
        .periodicTick = &Icssg_periodicTick,
        .registerEventCb   = &Icssg_registerEventCb,
        .unregisterEventCb = &Icssg_unregisterEventCb,
    },

    /* Shared between ICSSG1 Port 1 and Port 2 objects */
    .pruss = &gEnetSoc_PruIcssg1,

    /* Dual-MAC firmware for slice 1 */
    .fw =
    {
        {
            .pru       = RX_PRU_SLICE1_b00_DMac,
            .pruSize   = sizeof(RX_PRU_SLICE1_b00_DMac),
            .rtu       = RTU0_SLICE1_b00_DMac,
            .rtuSize   = sizeof(RTU0_SLICE1_b00_DMac),
            .txpru     = TX_PRU_SLICE1_b00_DMac,
            .txpruSize = sizeof(TX_PRU_SLICE1_b00_DMac)
        },
    },

    /* TimeSync module object */
    .timeSyncObj =
    {
        .enetMod =
        {
            .name       = "icssg1-2.timesync",
            .physAddr   = CSL_PRU_ICSSG1_DRAM0_SLV_RAM_BASE + CSL_ICSS_G_PR1_IEP0_SLV_REGS_BASE,
            .physAddr2  = CSL_PRU_ICSSG1_DRAM0_SLV_RAM_BASE + CSL_ICSS_G_RAM_SLV_RAM_REGS_BASE,
            .features   = ENET_FEAT_BASE,
            .errata     = ENET_ERRATA_NONE,
            .open       = &IcssgTimeSync_open,
            .rejoin     = &IcssgTimeSync_rejoin,
            .ioctl      = &IcssgTimeSync_ioctl,
            .close      = &IcssgTimeSync_close,
        },
    },

    /* MDIO module object */
    .mdioObj =
    {
        .enetMod =
        {
            .name       = "icssg1-2.mdio",
            .physAddr   = CSL_PRU_ICSSG1_PR1_MDIO_V1P7_MDIO_BASE,
            .features   = MDIO_FEATURE_CLAUSE45,
            .errata     = ENET_ERRATA_NONE,
            .open       = &Mdio_open,
            .rejoin     = &Mdio_rejoin,
            .ioctl      = &Mdio_ioctl,
            .close      = &Mdio_close,
        },
    },

    /* Stats module object */
    .statsObj =
    {
        .enetMod =
        {
            .name       = "icssg1-2.stats",
            .physAddr   = CSL_PRU_ICSSG1_DRAM0_SLV_RAM_BASE,
            .physAddr2  = 0,
            .features   = ENET_FEAT_BASE,
            .errata     = ENET_ERRATA_NONE,
            .open       = &IcssgStats_open,
            .rejoin     = &IcssgStats_rejoin,
            .ioctl      = &IcssgStats_ioctl,
            .close      = &IcssgStats_close,
        },
    },

    /* RM module object */
    .rmObj =
    {
        .enetMod =
        {
            .name       = "icssg1-2.rm",
            .physAddr   = 0U,
            .features   = ENET_FEAT_BASE,
            .errata     = ENET_ERRATA_NONE,
            .open       = &EnetRm_open,
            .rejoin     = &EnetRm_rejoin,
            .ioctl      = &EnetRm_ioctl,
            .close      = &EnetRm_close,
        },
    },
};

/* ICSSG0 Switch (ENET_ICSSG_SWITCH, 0) */
Icssg_Obj gEnetSoc_icssg0Swt =
{
    .enetPer =
    {
        .name         = "icssg0",
        .enetType     = ENET_ICSSG_SWITCH,
        .instId       = 0U,
        .magic        = ENET_NO_MAGIC,
        .physAddr     = CSL_PRU_ICSSG0_DRAM0_SLV_RAM_BASE,
        .physAddr2    = 0U,
        .features     = ENET_FEAT_BASE,
        .errata       = ENET_ERRATA_NONE,
        .initCfg      = &Icssg_initCfg,
        .open         = &Icssg_open,
        .close        = &Icssg_close,
        .rejoin       = &Icssg_rejoin,
        .ioctl        = &Icssg_ioctl,
        .poll         = &Icssg_poll,
        .convertTs    = &Icssg_convertTs,
        .periodicTick = &Icssg_periodicTick,
        .registerEventCb   = &Icssg_registerEventCb,
        .unregisterEventCb = &Icssg_unregisterEventCb,
    },

    .pruss = &gEnetSoc_PruIcssg0,

    /* Switch firmware for both slices */
    .fw =
    {
        {
            .pru       = RX_PRU_SLICE0_b00_Swt,
            .pruSize   = sizeof(RX_PRU_SLICE0_b00_Swt),
            .rtu       = RTU0_SLICE0_b00_Swt,
            .rtuSize   = sizeof(RTU0_SLICE0_b00_Swt),
            .txpru     = TX_PRU_SLICE0_b00_Swt,
            .txpruSize = sizeof(TX_PRU_SLICE0_b00_Swt)
        },
        {
            .pru       = RX_PRU_SLICE1_b00_Swt,
            .pruSize   = sizeof(RX_PRU_SLICE1_b00_Swt),
            .rtu       = RTU0_SLICE1_b00_Swt,
            .rtuSize   = sizeof(RTU0_SLICE1_b00_Swt),
            .txpru     = TX_PRU_SLICE1_b00_Swt,
            .txpruSize = sizeof(TX_PRU_SLICE1_b00_Swt)
        },
    },

    /* TimeSync module object */
    .timeSyncObj =
    {
        .enetMod =
        {
            .name       = "icssg0.timesync",
            .physAddr   = CSL_PRU_ICSSG0_DRAM0_SLV_RAM_BASE + CSL_ICSS_G_PR1_IEP0_SLV_REGS_BASE,
            .physAddr2  = CSL_PRU_ICSSG0_DRAM0_SLV_RAM_BASE + CSL_ICSS_G_RAM_SLV_RAM_REGS_BASE,
            .features   = ENET_FEAT_BASE,
            .errata     = ENET_ERRATA_NONE,
            .open       = &IcssgTimeSync_open,
            .rejoin     = &IcssgTimeSync_rejoin,
            .ioctl      = &IcssgTimeSync_ioctl,
            .close      = &IcssgTimeSync_close,
        },
    },

    /* MDIO module object */
    .mdioObj =
    {
        .enetMod =
        {
            .name       = "icssg0.mdio",
            .physAddr   = CSL_PRU_ICSSG0_PR1_MDIO_V1P7_MDIO_BASE,
            .features   = MDIO_FEATURE_CLAUSE45,
            .errata     = ENET_ERRATA_NONE,
            .open       = &Mdio_open,
            .rejoin     = &Mdio_rejoin,
            .ioctl      = &Mdio_ioctl,
            .close      = &Mdio_close,
        },
    },

    /* Stats module object */
    .statsObj =
    {
        .enetMod =
        {
            .name       = "icssg0.stats",
            .physAddr   = CSL_PRU_ICSSG0_DRAM0_SLV_RAM_BASE,
            .physAddr2  = 0,
            .features   = ENET_FEAT_BASE,
            .errata     = ENET_ERRATA_NONE,
            .open       = &IcssgStats_open,
            .rejoin     = &IcssgStats_rejoin,
            .ioctl      = &IcssgStats_ioctl,
            .close      = &IcssgStats_close,
        },
    },

    /* RM module object */
    .rmObj =
    {
        .enetMod =
        {
            .name       = "icssg0.rm",
            .physAddr   = 0U,
            .features   = ENET_FEAT_BASE,
            .errata     = ENET_ERRATA_NONE,
            .open       = &EnetRm_open,
            .rejoin     = &EnetRm_rejoin,
            .ioctl      = &EnetRm_ioctl,
            .close      = &EnetRm_close,
        },
    },
};

/* ICSSG1 Switch (ENET_ICSSG_SWITCH, 1) */
Icssg_Obj gEnetSoc_icssg1Swt =
{
    .enetPer =
    {
        .name         = "icssg1",
        .enetType     = ENET_ICSSG_SWITCH,
        .instId       = 1U,
        .magic        = ENET_NO_MAGIC,
        .physAddr     = CSL_PRU_ICSSG1_DRAM0_SLV_RAM_BASE,
        .physAddr2    = 0U,
        .features     = ENET_FEAT_BASE,
        .errata       = ENET_ERRATA_NONE,
        .initCfg      = &Icssg_initCfg,
        .open         = &Icssg_open,
        .close        = &Icssg_close,
        .rejoin       = &Icssg_rejoin,
        .ioctl        = &Icssg_ioctl,
        .poll         = &Icssg_poll,
        .convertTs    = &Icssg_convertTs,
        .periodicTick = &Icssg_periodicTick,
        .registerEventCb   = &Icssg_registerEventCb,
        .unregisterEventCb = &Icssg_unregisterEventCb,
    },

    .pruss = &gEnetSoc_PruIcssg1,

    /* Switch firmware for both slices */
    .fw =
    {
        {
            .pru       = RX_PRU_SLICE0_b00_Swt,
            .pruSize   = sizeof(RX_PRU_SLICE0_b00_Swt),
            .rtu       = RTU0_SLICE0_b00_Swt,
            .rtuSize   = sizeof(RTU0_SLICE0_b00_Swt),
            .txpru     = TX_PRU_SLICE0_b00_Swt,
            .txpruSize = sizeof(TX_PRU_SLICE0_b00_Swt)
        },
        {
            .pru       = RX_PRU_SLICE1_b00_Swt,
            .pruSize   = sizeof(RX_PRU_SLICE1_b00_Swt),
            .rtu       = RTU0_SLICE1_b00_Swt,
            .rtuSize   = sizeof(RTU0_SLICE1_b00_Swt),
            .txpru     = TX_PRU_SLICE1_b00_Swt,
            .txpruSize = sizeof(TX_PRU_SLICE1_b00_Swt)
        },
    },

    /* TimeSync module object */
    .timeSyncObj =
    {
        .enetMod =
        {
            .name       = "icssg1.timesync",
            .physAddr   = CSL_PRU_ICSSG1_DRAM0_SLV_RAM_BASE + CSL_ICSS_G_PR1_IEP0_SLV_REGS_BASE,
            .physAddr2  = CSL_PRU_ICSSG1_DRAM0_SLV_RAM_BASE + CSL_ICSS_G_RAM_SLV_RAM_REGS_BASE,
            .features   = ENET_FEAT_BASE,
            .errata     = ENET_ERRATA_NONE,
            .open       = &IcssgTimeSync_open,
            .rejoin     = &IcssgTimeSync_rejoin,
            .ioctl      = &IcssgTimeSync_ioctl,
            .close      = &IcssgTimeSync_close,
        },
    },

    /* MDIO module object */
    .mdioObj =
    {
        .enetMod =
        {
            .name       = "icssg1.mdio",
            .physAddr   = CSL_PRU_ICSSG1_PR1_MDIO_V1P7_MDIO_BASE,
            .features   = MDIO_FEATURE_CLAUSE45,
            .errata     = ENET_ERRATA_NONE,
            .open       = &Mdio_open,
            .rejoin     = &Mdio_rejoin,
            .ioctl      = &Mdio_ioctl,
            .close      = &Mdio_close,
        },
    },

    /* Stats module object */
    .statsObj =
    {
        .enetMod =
        {
            .name       = "icssg1.stats",
            .physAddr   = CSL_PRU_ICSSG1_DRAM0_SLV_RAM_BASE,
            .physAddr2  = 0,
            .features   = ENET_FEAT_BASE,
            .errata     = ENET_ERRATA_NONE,
            .open       = &IcssgStats_open,
            .rejoin     = &IcssgStats_rejoin,
            .ioctl      = &IcssgStats_ioctl,
            .close      = &IcssgStats_close,
        },
    },

    /* RM module object */
    .rmObj =
    {
        .enetMod =
        {
            .name       = "icssg1.rm",
            .physAddr   = 0U,
            .features   = ENET_FEAT_BASE,
            .errata     = ENET_ERRATA_NONE,
            .open       = &EnetRm_open,
            .rejoin     = &EnetRm_rejoin,
            .ioctl      = &EnetRm_ioctl,
            .close      = &EnetRm_close,
        },
    },
};

/* ---------------------------- Enet Peripherals ---------------------------- */
Enet_Obj gEnetSoc_perObj[] =
{
    /* CPSW_3G Enet driver/peripheral */
    {
        .enetPer = &gEnetSoc_cpsw3g.enetPer,
    },

    /* ICSSG0 Dual-MAC Port 1 Enet driver/peripheral */
    {
        .enetPer = &gEnetSoc_icssg0DMacp1.enetPer,
    },

    /* ICSSG0 Dual-MAC Port 2 Enet driver/peripheral */
    {
        .enetPer = &gEnetSoc_icssg0DMacp2.enetPer,
    },

    /* ICSSG1 Dual-MAC Port 1 Enet driver/peripheral */
    {
        .enetPer = &gEnetSoc_icssg1DMacp1.enetPer,
    },

    /* ICSSG1 Dual-MAC Port 2 Enet driver/peripheral */
    {
        .enetPer = &gEnetSoc_icssg1DMacp2.enetPer,
    },

    /* ICSSG0 Switch Enet driver/peripheral */
    {
        .enetPer = &gEnetSoc_icssg0Swt.enetPer,
    },

    /* ICSSG1 Switch Enet driver/peripheral */
    {
        .enetPer = &gEnetSoc_icssg1Swt.enetPer,
    },

};

/* ------------------------------- DMA objects ------------------------------ */

EnetUdma_DrvObj gEnetSoc_dmaObj[ENET_ARRAYSIZE(gEnetSoc_perObj)];

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

int32_t EnetSoc_init(void)
{
    memset(gEnetSoc_dmaObj, 0, sizeof(gEnetSoc_dmaObj));

    gEnetSoc_PruIcssg0.lock = EnetOsal_createMutex();
    EnetSoc_assert((gEnetSoc_PruIcssg0.lock != NULL),
                   "Failed to create PRUICSSG0's mutex\n");

    gEnetSoc_PruIcssg1.lock = EnetOsal_createMutex();
    EnetSoc_assert((gEnetSoc_PruIcssg1.lock != NULL),
                   "Failed to create PRUICSSG1's mutex\n");

    return ENET_SOK;
}

void EnetSoc_deinit(void)
{
    gEnetSoc_PruIcssg1.initialized = false;
    EnetOsal_deleteMutex(gEnetSoc_PruIcssg1.lock);
    gEnetSoc_PruIcssg1.lock = NULL;

    gEnetSoc_PruIcssg0.initialized = false;
    EnetOsal_deleteMutex(gEnetSoc_PruIcssg0.lock);
    gEnetSoc_PruIcssg0.lock = NULL;
}

EnetDma_Handle EnetSoc_getDmaHandle(Enet_Type enetType,
                                    uint32_t instId)
{
    EnetDma_Handle hDma = NULL;

    switch (enetType)
    {
        case ENET_CPSW_3G:
            if (instId == 0U)
            {
                hDma = &gEnetSoc_dmaObj[0U];
            }
            break;

        case ENET_ICSSG_DUALMAC:
            switch (instId)
            {
                case 0:
                    hDma = &gEnetSoc_dmaObj[1U];
                    break;
                case 1:
                    hDma = &gEnetSoc_dmaObj[2U];
                    break;
                case 2:
                    hDma = &gEnetSoc_dmaObj[3U];
                    break;
                case 3:
                    hDma = &gEnetSoc_dmaObj[4U];
                    break;
                default:
                    EnetSoc_assert(false, "Invalid peripheral (eneType=%u instId=%u)\n", enetType, instId);
                    break;
            }
            break;

        case ENET_ICSSG_SWITCH:
            switch (instId)
            {
                case 0:
                    hDma = &gEnetSoc_dmaObj[5U];
                    break;
                case 1:
                    hDma = &gEnetSoc_dmaObj[6U];
                    break;
                default:
                    EnetSoc_assert(false, "Invalid peripheral (eneType=%u instId=%u)\n", enetType, instId);
                    break;
            }
            break;

        default:
            break;
    }

    return hDma;
}

Enet_Handle EnetSoc_getEnetHandleByIdx(uint32_t idx)
{
    Enet_Handle hEnet = NULL;

    if (idx < ENET_ARRAYSIZE(gEnetSoc_perObj))
    {
        hEnet = &gEnetSoc_perObj[idx];
    }

    return hEnet;
}

Enet_Handle EnetSoc_getEnetHandle(Enet_Type enetType,
                                  uint32_t instId)
{
    Enet_Handle hEnet = NULL;

    switch (enetType)
    {
        case ENET_CPSW_3G:
            if (instId == 0U)
            {
                hEnet = &gEnetSoc_perObj[0U];
            }
            break;

        case ENET_ICSSG_DUALMAC:
            switch (instId)
            {
                case 0:
                    hEnet = &gEnetSoc_perObj[1U];
                    break;
                case 1:
                    hEnet = &gEnetSoc_perObj[2U];
                    break;
                case 2:
                    hEnet = &gEnetSoc_perObj[3U];
                    break;
                case 3:
                    hEnet = &gEnetSoc_perObj[4U];
                    break;
                default:
                    EnetSoc_assert(false, "Invalid peripheral (eneType=%u instId=%u)\n", enetType, instId);
                    break;
            }
            break;

        case ENET_ICSSG_SWITCH:
            switch (instId)
            {
                case 0:
                    hEnet = &gEnetSoc_perObj[5U];
                    break;
                case 1:
                    hEnet = &gEnetSoc_perObj[6U];
                    break;
                case 2:
                    hEnet = &gEnetSoc_perObj[7U];
                    break;
                default:
                    EnetSoc_assert(false, "Invalid peripheral (eneType=%u instId=%u)\n", enetType, instId);
                    break;
            }
            break;

        default:
            {
                EnetSoc_assert(false, "Invalid peripheral (eneType=%u instId=%u)\n", enetType, instId);
            }
            break;
    }
    return hEnet;
}

uint32_t EnetSoc_getCoreId(void)
{
    uint32_t coreId;

#if defined(BUILD_MCU1_0)
    coreId = CSL_CORE_ID_R5FSS0_0;
#else
#error "Enet AM64x SOC: Core not supported!!"
#endif

    return coreId;
}

uint32_t EnetSoc_getCoreKey(uint32_t coreId)
{
    return coreId;
}

bool EnetSoc_isCoreAllowed(Enet_Type enetType,
                           uint32_t instId,
                           uint32_t coreId)
{
    return true;
}

uint32_t EnetSoc_getEnetNum(void)
{
    return ENET_ARRAYSIZE(gEnetSoc_perObj);
}

uint32_t EnetSoc_getMacPortMax(Enet_Type enetType,
                               uint32_t instId)
{
    uint32_t numPorts = 0U;

    if ((enetType == ENET_CPSW_3G) && (instId == 0U))
    {
        numPorts = gEnetSoc_cpsw3g.macPortNum;
    }
    else if (enetType == ENET_ICSSG_DUALMAC)
    {
        numPorts = 1U;
    }
    else if (enetType == ENET_ICSSG_SWITCH)
    {
        numPorts = 2U;
    }
    else
    {
        EnetSoc_assert(false, "Invalid peripheral (eneType=%u instId=%u)\n", enetType, instId);
    }

    return numPorts;
}

uint32_t EnetSoc_isIpSupported(Enet_Type enetType,
                               uint32_t instId)
{
    bool supported = false;

    if ((enetType == ENET_CPSW_3G) && (0U == instId))
    {
        supported = true;
    }
#if defined(ENET_ENABLE_ICSSG)
    else if (((enetType == ENET_ICSSG_DUALMAC) && (instId < 3U)) ||
             ((enetType == ENET_ICSSG_SWITCH) && (instId < 3U)))
    {
        supported = true;
    }
#endif
    else
    {
        EnetSoc_assert(false, "Invalid Enet & instId type %d, %d\n", enetType, instId);
    }

    return supported;
}

uint32_t EnetSoc_getRxFlowCount(Enet_Type enetType,
                                uint32_t instId)
{
    uint32_t rxFlowCount = 0U;

    /* Get SoC array index */
    if (enetType == ENET_CPSW_3G)
    {
        rxFlowCount = gEnetSoc_cpsw3gSocCfg.rxFlowCount;
    }
    else if (enetType == ENET_ICSSG_DUALMAC)
    {
        switch (instId)
        {
            case 0:
                rxFlowCount = gEnetSoc_icssg0DMacSocCfg[0].rxFlowCount;
                break;
            case 1:
                rxFlowCount = gEnetSoc_icssg0DMacSocCfg[1].rxFlowCount;
                break;
            case 2:
                rxFlowCount = gEnetSoc_icssg1DMacSocCfg[0].rxFlowCount;
                break;
            case 3:
                rxFlowCount = gEnetSoc_icssg1DMacSocCfg[1].rxFlowCount;
                break;
            default:
                EnetSoc_assert(false, "Invalid peripheral (eneType=%u instId=%u)\n", enetType, instId);
                break;
        }
    }
    else if (enetType == ENET_ICSSG_SWITCH)
    {
        switch (instId)
        {
            case 0:
                rxFlowCount = gEnetSoc_icssg0SwtSocCfg.rxFlowCount;
                break;
            case 1:
                rxFlowCount = gEnetSoc_icssg1SwtSocCfg.rxFlowCount;
                break;
            default:
                EnetSoc_assert(false, "Invalid peripheral (eneType=%u instId=%u)\n", enetType, instId);
                break;
        }
    }
    else
    {
        EnetSoc_assert(false, "Invalid Enet type %d\n", enetType);
    }

    return rxFlowCount;
}

uint32_t EnetSoc_getRxChPeerId(Enet_Type enetType,
                               uint32_t instId,
                               uint32_t chIdx)
{
    uint32_t peerChNum = 0U;

    /* Get SoC array index */
    if (enetType == ENET_CPSW_3G)
    {
        peerChNum = gEnetSoc_cpsw3gSocCfg.rxChPeerThreadId;
    }
    else if (enetType == ENET_ICSSG_DUALMAC)
    {
        switch (instId)
        {
            case 0:
                peerChNum = gEnetSoc_icssg0DMacSocCfg[0].rxChPeerThreadId;
                break;
            case 1:
                peerChNum = gEnetSoc_icssg0DMacSocCfg[1].rxChPeerThreadId;
                break;
            case 2:
                peerChNum = gEnetSoc_icssg1DMacSocCfg[0].rxChPeerThreadId;
                break;
            case 3:
                peerChNum = gEnetSoc_icssg1DMacSocCfg[1].rxChPeerThreadId;
                break;
            default:
                EnetSoc_assert(false, "Invalid peripheral (eneType=%u instId=%u)\n", enetType, instId);
                break;
        }
    }
    else if (enetType == ENET_ICSSG_SWITCH)
    {
        EnetSoc_assert(((chIdx == 0U) || (chIdx == 1U)), "Invalid channel index %u\n", chIdx);
        switch (instId)
        {
            case 0:
                peerChNum = gEnetSoc_icssg0SwtSocCfg.rxChPeerThreadId + chIdx;
                break;
            case 1:
                peerChNum = gEnetSoc_icssg1SwtSocCfg.rxChPeerThreadId + chIdx;
                break;
            default:
                EnetSoc_assert(false, "Invalid peripheral (eneType=%u instId=%u)\n", enetType, instId);
                break;
        }
    }
    else
    {
        EnetSoc_assert(false, "Invalid Enet type %d\n", enetType);
    }

    return peerChNum;
}

uint32_t EnetSoc_getTxChCount(Enet_Type enetType,
                              uint32_t instId)
{
    uint32_t txChCount = 0U;

    /* Get SoC array index */
    if (enetType == ENET_CPSW_3G)
    {
        txChCount = gEnetSoc_cpsw3gSocCfg.txChCount;
    }
    else if (enetType == ENET_ICSSG_DUALMAC)
    {
        switch (instId)
        {
            case 0:
                txChCount = gEnetSoc_icssg0DMacSocCfg[0].txChCount;
                break;
            case 1:
                txChCount = gEnetSoc_icssg0DMacSocCfg[1].txChCount;
                break;
            case 2:
                txChCount = gEnetSoc_icssg1DMacSocCfg[0].txChCount;
                break;
            case 3:
                txChCount = gEnetSoc_icssg1DMacSocCfg[1].txChCount;
                break;
            default:
                EnetSoc_assert(false, "Invalid peripheral (eneType=%u instId=%u)\n", enetType, instId);
                break;
        }
    }
    else if (enetType == ENET_ICSSG_SWITCH)
    {
        switch (instId)
        {
            case 0:
                txChCount = gEnetSoc_icssg0SwtSocCfg.txChCount;
                break;
            case 1:
                txChCount = gEnetSoc_icssg1SwtSocCfg.txChCount;
                break;
            default:
                EnetSoc_assert(false, "Invalid peripheral (eneType=%u instId=%u)\n", enetType, instId);
                break;
        }
    }
    else
    {
        EnetSoc_assert(false, "Invalid Enet type %d\n", enetType);
    }

    return txChCount;
}

uint32_t EnetSoc_getTxChPeerId(Enet_Type enetType,
                               uint32_t instId,
                               uint32_t chNum)
{
    uint32_t peerChNum = 0U;

    /* Get SoC array index */
    if (enetType == ENET_CPSW_3G)
    {
        peerChNum = gEnetSoc_cpsw3gSocCfg.txChPeerThreadId;
    }
    else if (enetType == ENET_ICSSG_DUALMAC)
    {
        switch (instId)
        {
            case 0:
                peerChNum = gEnetSoc_icssg0DMacSocCfg[0].txChPeerThreadId;
                break;
            case 1:
                peerChNum = gEnetSoc_icssg0DMacSocCfg[1].txChPeerThreadId;
                break;
            case 2:
                peerChNum = gEnetSoc_icssg1DMacSocCfg[0].txChPeerThreadId;
                break;
            case 3:
                peerChNum = gEnetSoc_icssg1DMacSocCfg[1].txChPeerThreadId;
                break;
            default:
                EnetSoc_assert(false, "Invalid peripheral (eneType=%u instId=%u)\n", enetType, instId);
                break;
        }
    }
    else if (enetType == ENET_ICSSG_SWITCH)
    {
        switch (instId)
        {
            case 0:
                peerChNum = gEnetSoc_icssg0SwtSocCfg.txChPeerThreadId;
                break;
            case 1:
                peerChNum = gEnetSoc_icssg1SwtSocCfg.txChPeerThreadId;
                break;
            default:
                EnetSoc_assert(false, "Invalid peripheral (eneType=%u instId=%u)\n", enetType, instId);
                break;
        }
    }
    else
    {
        EnetSoc_assert(false, "Invalid Enet type %d\n", enetType);
    }

    /* Get PSI-L destination thread offset for Tx channel */
    peerChNum = (peerChNum + chNum);

    return peerChNum;
}

uint32_t EnetSoc_getClkFreq(Enet_Type enetType,
                            uint32_t instId,
                            uint32_t clkId)
{
    uint32_t freq = 0U;

    if (clkId == CPSW_CPPI_CLK)
    {
        if ((enetType == ENET_CPSW_3G) && (instId == 0U))
        {
            freq = gEnetSoc_cpsw3gSocCfg.cppiClkFreqHz;
        }
        else if ((enetType == ENET_ICSSG_DUALMAC)|| (enetType == ENET_ICSSG_SWITCH))
        {
            //FIXME
            freq = gEnetSoc_cpsw3gSocCfg.cppiClkFreqHz;
        }
        else
        {
            EnetSoc_assert(false, "Invalid peripheral (eneType=%u instId=%u)\n", enetType, instId);
        }
    }
    else
    {
        EnetSoc_assert(false, "Invalid clk id %u\n", clkId);
    }

    return freq;
}

int32_t EnetSoc_setupIntrCfg(Enet_Type enetType,
                             uint32_t instId,
                             uint32_t intrId)
{
    const EnetSoc_IntrConnCfg *socIntrs = NULL;
    uint32_t numSocIntrs = 0U;
    uint16_t coreDevId = EnetSoc_getCoreDevId();
    uint16_t perDevId = 0U;
    int32_t status = ENET_SOK;

    if ((enetType == ENET_CPSW_3G) && (instId == 0U))
    {
        perDevId    = gEnetSoc_cpsw3gSocCfg.dmscDevId;
        socIntrs    = gEnetSoc_cpsw3gSocCfg.intrs;
        numSocIntrs = ENET_ARRAYSIZE(gEnetSoc_cpsw3gSocCfg.intrs);
    }
    else
    {
        EnetSoc_assert(false, "Invalid peripheral (eneType=%u instId=%u)\n", enetType, instId);
        status = ENET_EINVALIDPARAMS;
    }

    if (status == ENET_SOK)
    {
        status = EnetSocJ7x_setupIntrCfg(intrId, coreDevId, perDevId, socIntrs, numSocIntrs);
    }

    return status;
}

int32_t EnetSoc_releaseIntrCfg(Enet_Type enetType,
                               uint32_t instId,
                               uint32_t intrId)
{
    const EnetSoc_IntrConnCfg *socIntrs = NULL;
    uint32_t numSocIntrs = 0U;
    uint16_t coreDevId = EnetSoc_getCoreDevId();
    uint16_t perDevId = 0U;
    int32_t status = ENET_SOK;

    if ((enetType == ENET_CPSW_3G) && (instId == 0U))
    {
        perDevId    = gEnetSoc_cpsw3gSocCfg.dmscDevId;
        socIntrs    = gEnetSoc_cpsw3gSocCfg.intrs;
        numSocIntrs = ENET_ARRAYSIZE(gEnetSoc_cpsw3gSocCfg.intrs);
    }
    else
    {
        EnetSoc_assert(false, "Invalid peripheral (eneType=%u instId=%u)\n", enetType, instId);
        status = ENET_EINVALIDPARAMS;
    }

    if (status == ENET_SOK)
    {
        status = EnetSocJ7x_releaseIntrCfg(intrId, coreDevId, perDevId, socIntrs, numSocIntrs);
    }

    return status;
}

uint32_t EnetSoc_getIntrNum(Enet_Type enetType,
                            uint32_t instId,
                            uint32_t intrId)
{
    const EnetSoc_IntrConnCfg *socIntrs = NULL;
    uint32_t numSocIntrs = 0U;
    uint32_t intrNum = 0U;
    int32_t status = ENET_SOK;

    if ((enetType == ENET_CPSW_3G) && (instId == 0U))
    {
        socIntrs    = gEnetSoc_cpsw3gSocCfg.intrs;
        numSocIntrs = ENET_ARRAYSIZE(gEnetSoc_cpsw3gSocCfg.intrs);
    }
    else
    {
        EnetSoc_assert(false, "Invalid peripheral (eneType=%u instId=%u)\n", enetType, instId);
        status = ENET_EINVALIDPARAMS;
    }

    if (status == ENET_SOK)
    {
        intrNum = EnetSocJ7x_getIntrNum(intrId, socIntrs, numSocIntrs);
    }

    return intrNum;
}

uint32_t EnetSoc_getIntrTriggerType(Enet_Type enetType,
                                    uint32_t instId,
                                    uint32_t intrId)
{
    return ENETOSAL_ARM_GIC_TRIG_TYPE_LEVEL;
}
/* Stupid hack to workaround FAE board issues */
//#define ENET_MAC_ADDR_HACK (TRUE)
int32_t EnetSoc_getEFusedMacAddrs(uint8_t macAddr[][ENET_MAC_ADDR_LEN],
                                  uint32_t *num)
{
#ifndef ENET_MAC_ADDR_HACK
    CSL_main_ctrl_mmr_cfg0Regs *mmrRegs;
    uint32_t val;

    if (*num >= 1U)
    {
        mmrRegs = (CSL_main_ctrl_mmr_cfg0Regs *)(uintptr_t)CSL_CTRL_MMR0_CFG0_BASE;

        val = CSL_REG32_RD(&mmrRegs->MAC_ID0);
        macAddr[0][5] = (uint8_t)((val & 0x000000FFU) >> 0U);
        macAddr[0][4] = (uint8_t)((val & 0x0000FF00U) >> 8U);
        macAddr[0][3] = (uint8_t)((val & 0x00FF0000U) >> 16U);
        macAddr[0][2] = (uint8_t)((val & 0xFF000000U) >> 24U);

        val = CSL_REG32_RD(&mmrRegs->MAC_ID1);
        macAddr[0][1] = (uint8_t)((val & 0x000000FFU) >> 0U);
        macAddr[0][0] = (uint8_t)((val & 0x0000FF00U) >> 8U);

        *num = 1U;
    }
#else
    macAddr[0][0] = 0xF4;
    macAddr[0][1] = 0x84;
    macAddr[0][2] = 0x4c;
    macAddr[0][3] = 0xf9;
    macAddr[0][4] = 0x4d;
    macAddr[0][5] = 0x29;
    *num = 1U;
#endif
    return ENET_SOK;
}

uint32_t EnetSoc_getMacPortCaps(Enet_Type enetType,
                                uint32_t instId,
                                Enet_MacPort macPort)
{
    uint32_t linkCaps = 0U;

    switch (enetType)
    {
        case ENET_CPSW_3G:
            if (macPort <= ENET_MAC_PORT_2)
            {
                linkCaps = (ENETPHY_LINK_CAP_HD10 | ENETPHY_LINK_CAP_FD10 |
                            ENETPHY_LINK_CAP_HD100 | ENETPHY_LINK_CAP_FD100 |
                            ENETPHY_LINK_CAP_FD1000);
            }
            break;

        case ENET_ICSSG_DUALMAC:
        case ENET_ICSSG_SWITCH:
            if (macPort <= ENET_MAC_PORT_8)
            {
                linkCaps = (ENETPHY_LINK_CAP_HD10 | ENETPHY_LINK_CAP_FD10 |
                            ENETPHY_LINK_CAP_HD100 | ENETPHY_LINK_CAP_FD100 |
                            ENETPHY_LINK_CAP_FD1000);
            }
            break;

        default:
            EnetSoc_assert(false, "Invalid peripheral type: %u\n", enetType);
            break;

    }

    return linkCaps;
}

int32_t EnetSoc_getMacPortMii(Enet_Type enetType,
                              uint32_t instId,
                              Enet_MacPort macPort,
                              EnetMacPort_Interface *mii)
{
    EnetMac_LayerType *enetLayer = &mii->layerType;
    EnetMac_SublayerType *enetSublayer = &mii->sublayerType;
    uint32_t modeSel = CPSW_ENET_CTRL_MODE_RGMII;
    int32_t status = ENET_EFAIL;

    switch (enetType)
    {
        case ENET_CPSW_3G:
            status = EnetSoc_getMcuEnetControl(macPort, &modeSel);
            break;

        case ENET_ICSSG_DUALMAC:
        case ENET_ICSSG_SWITCH:
            status = EnetSoc_getMainEnetControl(macPort, &modeSel);
            break;

        default:
            EnetSoc_assert(false, "Invalid peripheral type: %u\n", enetType);
            break;
    }

    if (status == ENET_SOK)
    {
        switch (modeSel)
        {
            /* RMII */
            case CPSW_ENET_CTRL_MODE_RMII:
                *enetLayer    = ENET_MAC_LAYER_MII;
                *enetSublayer = ENET_MAC_SUBLAYER_REDUCED;
                break;

            /* RGMII */
            case CPSW_ENET_CTRL_MODE_RGMII:
                *enetLayer    = ENET_MAC_LAYER_GMII;
                *enetSublayer = ENET_MAC_SUBLAYER_REDUCED;
                break;

            default:
                status = ENET_EINVALIDPARAMS;
                break;
        }
    }

    return status;
}

static uint32_t EnetSoc_getMcuEnetControl(Enet_MacPort macPort,
                                          uint32_t *modeSel)
{
    CSL_main_ctrl_mmr_cfg0Regs *regs =
        (CSL_main_ctrl_mmr_cfg0Regs *)(uintptr_t)CSL_CTRL_MMR0_CFG0_BASE;
    int32_t status = ENET_SOK;

    switch (macPort)
    {
        case ENET_MAC_PORT_1:
            *modeSel = CSL_FEXT(regs->ENET1_CTRL, MAIN_CTRL_MMR_CFG0_ENET1_CTRL_PORT_MODE_SEL);
            break;

        case ENET_MAC_PORT_2:
            *modeSel = CSL_FEXT(regs->ENET2_CTRL, MAIN_CTRL_MMR_CFG0_ENET2_CTRL_PORT_MODE_SEL);
            break;

        default:
            status = ENET_EINVALIDPARAMS;
            break;
    }

    if (status == ENET_SOK)
    {
        switch (*modeSel)
        {
            case CPSW_ENET_CTRL_MODE_RMII:
            case CPSW_ENET_CTRL_MODE_RGMII:
                break;

            default:
                status = ENET_EINVALIDPARAMS;
                break;
        }
    }

    return status;
}

static uint32_t EnetSoc_getMainEnetControl(Enet_MacPort macPort,
                                           uint32_t *modeSel)
{
    int32_t status = ENET_SOK;

    return status;
}

static uint32_t EnetSoc_getCoreDevId(void)
{
    uint32_t coreDevId;

#if defined(BUILD_MCU1_0)
    coreDevId = TISCI_DEV_R5FSS0_CORE0;
#else
#error "Enet AM65xx SOC: Core not supported!!"
#endif

    return coreDevId;
}

int32_t EnetSoc_validateQsgmiiCfg(Enet_Type enetType,
                                  uint32_t instId)
{
    return ENET_ENOTSUPPORTED;
}

int32_t EnetSoc_mapPort2QsgmiiId(Enet_Type enetType,
                                 uint32_t instId,
                                 Enet_MacPort portNum,
                                 uint32_t *qsgmiiId)
{
    return ENET_ENOTSUPPORTED;
}
