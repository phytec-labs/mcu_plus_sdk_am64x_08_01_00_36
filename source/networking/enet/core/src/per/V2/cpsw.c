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
 * \file  cpsw.c
 *
 * \brief This file contains the implementation of the CPSW_2G/CPSW_3G peripheral
 *        variant which is present in the AM273X and AM263x device.
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdint.h>
#include <stdarg.h>
#include <csl_cpswitch.h>
#include <enet_cfg.h>
#include <priv/mod/cpsw_ale_priv.h>
#include <priv/mod/cpsw_cpts_priv.h>
#include <priv/mod/cpsw_hostport_priv.h>
#include <priv/mod/cpsw_macport_priv.h>
#include <priv/mod/mdio_priv.h>
#include <priv/mod/cpsw_stats_priv.h>
#include <include/core/enet_utils.h>
#include <include/core/enet_osal.h>
#include <include/core/enet_soc.h>
#include <include/core/enet_per.h>
#include <include/per/cpsw.h>
#include <include/dma/cpdma/enet_cpdma.h>
#include <priv/per/cpsw_cpdma_priv.h>
#include <priv/per/enet_hostport_cpdma.h>
#include <include/core/enet_utils.h>
#include <include/common/enet_phymdio_dflt.h>
#include <include/phy/enetphy.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/*! \brief Default common TX MTU. */
#define CPSW_COMMON_TX_MTU_DEFAULT            (2024U)

/*!
 * \brief Priority escalation value for switch scheduler.
 *
 * When a port is in escalate priority, this is the number of higher priority
 * packets sent before the next lower priority is allowed to send a packet.
 * Escalate priority allows lower priority packets to be sent at a fixed rate
 * relative to the next higher priority.  The min value of esc_pri_ld_val = 2
 */
#define CPSW_ESC_PRI_LD_VAL                   (2U)

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

static int32_t Cpsw_openInternal(Cpsw_Handle hCpsw,
                                 Enet_Type enetType,
                                 uint32_t instId,
                                 const Cpsw_Cfg *cfg);

static void Cpsw_closeInternal(Cpsw_Handle hCpsw);

static int32_t Cpsw_ioctlInternal(EnetPer_Handle hPer,
                                  uint32_t cmd,
                                  Enet_IoctlPrms *prms);

static int32_t Cpsw_registerIntrs(Cpsw_Handle hCpsw,
                                  const Cpsw_Cfg *cfg);

static void Cpsw_unregisterIntrs(Cpsw_Handle hCpsw);

static void Cpsw_statsIsr(uintptr_t arg);

static void Cpsw_mdioIsr(uintptr_t arg);

static void Cpsw_cptsIsr(uintptr_t arg);

static void Cpsw_dmaRxThreshIsr(uintptr_t arg);

static void Cpsw_dmaRxIsr(uintptr_t arg);

static void Cpsw_dmaTxIsr(uintptr_t arg);

static void Cpsw_dmaMiscIsr(uintptr_t arg);

static void Cpsw_handleMdioLinkStateChange(EnetMdio_Group group,
                                           Mdio_PhyStatus *phyStatus,
                                           void *cbArgs);

static Cpsw_PortLinkState *Cpsw_getPortLinkState(Cpsw_Handle hCpsw,
                                                 uint32_t phyAddr);

static int32_t Cpsw_openPortLinkWithPhy(Cpsw_Handle hCpsw,
                                        Enet_MacPort macPort,
                                        const CpswMacPort_Cfg *macCfg,
                                        const EnetPhy_Cfg *phyCfg,
                                        const EnetMacPort_Interface *mii,
                                        const EnetMacPort_LinkCfg *linkCfg);

static int32_t Cpsw_openPortLinkNoPhy(Cpsw_Handle hCpsw,
                                      Enet_MacPort macPort,
                                      const CpswMacPort_Cfg *macCfg,
                                      const EnetMacPort_Interface *mii,
                                      const EnetMacPort_LinkCfg *linkCfg);

static int32_t Cpsw_openPortLink(Cpsw_Handle hCpsw,
                                 Enet_MacPort macPort,
                                 const CpswMacPort_Cfg *macCfg,
                                 const EnetPhy_Cfg *phyCfg,
                                 const EnetMacPort_Interface *mii,
                                 const EnetMacPort_LinkCfg *linkCfg);

static void Cpsw_closePortLink(Cpsw_Handle hCpsw,
                               Enet_MacPort macPort);

static int32_t Cpsw_handleLinkUp(Cpsw_Handle hCpsw,
                                 Enet_MacPort macPort);

static int32_t Cpsw_handleLinkDown(Cpsw_Handle hCpsw,
                                   Enet_MacPort macPort);

static int32_t Cpsw_getPortLinkCfg(Cpsw_Handle hCpsw,
                                   Enet_MacPort macPort,
                                   EnetMacPort_LinkCfg *linkCfg);

static int32_t Cpsw_validateTxShortIpgCfg(const Cpsw_Handle hCpsw,
                                          const Cpsw_SetTxShortIpgCfgInArgs *inArgs);

static int32_t Cpsw_setTxShortIpgCfg(const Cpsw_Handle hCpsw,
                                     const Cpsw_SetTxShortIpgCfgInArgs *inArgs);

static int32_t Cpsw_getTxShortIpgCfg(const Cpsw_Handle hCpsw,
                                     Cpsw_TxShortIpgCfg *shortIpgCfg);

static uint32_t Cpsw_getRxMtuPort0(Cpsw_Handle hCpsw);

static uint32_t Cpsw_getTxMtuPerPriority(Cpsw_Handle hCpsw,
                                         uint32_t priority);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

#if (ENET_CFG_TRACE_LEVEL >= ENET_CFG_TRACE_LEVEL_ERROR)
static const char *Cpsw_gSpeedNames[] =
{
    [ENET_SPEED_10MBIT]  = "10-Mbps",
    [ENET_SPEED_100MBIT] = "100-Mbps",
    [ENET_SPEED_1GBIT]   = "1-Gbps",
    [ENET_SPEED_AUTO]    = "auto",
};

static const char *Cpsw_gDuplexNames[] =
{
    [ENET_DUPLEX_HALF] = "Half-Duplex",
    [ENET_DUPLEX_FULL] = "Full-Duplex",
    [ENET_DUPLEX_AUTO] = "auto",
};
#endif

/*!\ brief Default host and MAC port TX priority MTUs */
static const uint32_t Cpsw_txPriMtuDefault[] =
{
    CPSW_COMMON_TX_MTU_DEFAULT, /* TX Priority 0 MTU */
    CPSW_COMMON_TX_MTU_DEFAULT, /* TX Priority 1 MTU */
    CPSW_COMMON_TX_MTU_DEFAULT, /* TX Priority 2 MTU */
    CPSW_COMMON_TX_MTU_DEFAULT, /* TX Priority 3 MTU */
    CPSW_COMMON_TX_MTU_DEFAULT, /* TX Priority 4 MTU */
    CPSW_COMMON_TX_MTU_DEFAULT, /* TX Priority 5 MTU */
    CPSW_COMMON_TX_MTU_DEFAULT, /* TX Priority 6 MTU */
    CPSW_COMMON_TX_MTU_DEFAULT  /* TX Priority 7 MTU */
};

#if ENET_CFG_IS_ON(DEV_ERROR)
/* Public CPSW peripheral IOCTL validation data. */
static Enet_IoctlValidate gCpsw_ioctlValidate[] =
{
    ENET_IOCTL_VALID_PRMS(CPSW_PER_IOCTL_SET_INTERVLAN_ROUTE_UNI_EGRESS,
                          sizeof(Cpsw_SetInterVlanRouteUniEgressInArgs),
                          sizeof(Cpsw_SetInterVlanRouteUniEgressOutArgs)),

    ENET_IOCTL_VALID_PRMS(CPSW_PER_IOCTL_CLEAR_INTERVLAN_ROUTE_UNI_EGRESS,
                          sizeof(Cpsw_ClearInterVlanRouteUniEgressInArgs),
                          sizeof(CpswMacPort_InterVlanRouteId)),

    ENET_IOCTL_VALID_PRMS(CPSW_PER_IOCTL_SET_INTERVLAN_ROUTE_MULTI_EGRESS,
                          sizeof(Cpsw_SetInterVlanRouteMultiEgressInArgs),
                          sizeof(Cpsw_SetInterVlanRouteMultiEgressOutArgs)),

    ENET_IOCTL_VALID_PRMS(CPSW_PER_IOCTL_CLEAR_INTERVLAN_ROUTE_MULTI_EGRESS,
                          sizeof(Cpsw_ClearInterVlanRouteMultiEgressInArgs),
                          sizeof(CpswMacPort_InterVlanRouteId)),

    ENET_IOCTL_VALID_PRMS(CPSW_PER_IOCTL_SET_SHORT_IPG_CFG,
                          sizeof(Cpsw_SetTxShortIpgCfgInArgs),
                          0U),

    ENET_IOCTL_VALID_PRMS(CPSW_PER_IOCTL_GET_SHORT_IPG_CFG,
                          0U,
                          sizeof(Cpsw_TxShortIpgCfg)),

    ENET_IOCTL_VALID_PRMS(ENET_IOCTL_REGISTER_RX_DEFAULT_FLOW,
                          sizeof(Enet_DfltFlowInfo),
                          0U),

    ENET_IOCTL_VALID_PRMS(ENET_IOCTL_UNREGISTER_RX_DEFAULT_FLOW,
                          sizeof(Enet_DfltFlowInfo),
                          0U),

    ENET_IOCTL_VALID_PRMS(ENET_IOCTL_REGISTER_DSTMAC_RX_FLOW,
                          sizeof(Enet_MacDstFlowInfo),
                          0U),

    ENET_IOCTL_VALID_PRMS(ENET_IOCTL_UNREGISTER_DSTMAC_RX_FLOW,
                          sizeof(Enet_MacDstFlowInfo),
                          0U),
};
#endif

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

void Cpsw_initCfg(EnetPer_Handle hPer,
                  Enet_Type enetType,
                  void *cfg,
                  uint32_t cfgSize)
{
    Cpsw_Cfg *cpswCfg = (Cpsw_Cfg *)cfg;
    uint32_t i;

    Enet_devAssert(cfgSize == sizeof(Cpsw_Cfg),
                   "Invalid CPSW peripheral config params size %u (expected %u)\r\n",
                   cfgSize, sizeof(Cpsw_Cfg));

    cpswCfg->escalatePriorityLoadVal = CPSW_ESC_PRI_LD_VAL;

    /* VLAN configuration parameters */
    cpswCfg->vlanCfg.vlanAware  = false;
    cpswCfg->vlanCfg.vlanSwitch = ENET_VLAN_TAG_TYPE_INNER;
    cpswCfg->vlanCfg.outerVlan  = 0x88A8U;
    cpswCfg->vlanCfg.innerVlan  = 0x8100U;

    ENET_UTILS_COMPILETIME_ASSERT(ENET_ARRAYSIZE(cpswCfg->txMtu) == ENET_PRI_NUM);

    for (i = 0U; i < ENET_PRI_NUM; i++)
    {
        cpswCfg->txMtu[i] = Cpsw_txPriMtuDefault[i];
    }

    /* Initialize host port config params */
    CpswHostPort_initCfg(&cpswCfg->hostPortCfg);

    /* Initialize CPTS config params */
    CpswCpts_initCfg(&cpswCfg->cptsCfg);

    /* Initialize MDIO config params */
    Mdio_initCfg(&cpswCfg->mdioCfg);

    /* Initialize ALE params */
    CpswAle_initCfg(&cpswCfg->aleCfg);

    /* Initialize CPDMA params */
    EnetHostPortDma_initCfg(enetType, cpswCfg->dmaCfg);

    cpswCfg->intrPriority              = 1U;
    cpswCfg->mdioLinkStateChangeCb     = NULL;
    cpswCfg->mdioLinkStateChangeCbArg  = NULL;
    cpswCfg->portLinkStatusChangeCb    = NULL;
    cpswCfg->portLinkStatusChangeCbArg = NULL;

}

int32_t Cpsw_open(EnetPer_Handle hPer,
                  Enet_Type enetType,
                  uint32_t instId,
                  const void *cfg,
                  uint32_t cfgSize)
{
    Cpsw_Handle hCpsw = (Cpsw_Handle)hPer;
    Cpsw_Cfg *cpswCfg = (Cpsw_Cfg *)cfg;
    Enet_IoctlPrms prms;
    CSL_Xge_cpswRegs *regs = (CSL_Xge_cpswRegs *)hPer->virtAddr;
    CSL_CPSW_PTYPE pType;
    uintptr_t key;
    uint32_t i;
    int32_t status = ENET_SOK;

    Enet_devAssert(cfgSize == sizeof(Cpsw_Cfg),
                   "Invalid CPSW peripheral config params size %u (expected %u)\r\n",
                   cfgSize, sizeof(Cpsw_Cfg));

    /* Save EnetMod handles for easy access */
    for (i = 0U; i < hCpsw->macPortNum; i++)
    {
        hCpsw->hMacPort[i] = ENET_MOD(&hCpsw->macPortObj[i]);
    }

    hCpsw->hHostPort = ENET_MOD(&hCpsw->hostPortObj);
    hCpsw->hStats    = ENET_MOD(&hCpsw->statsObj);
    hCpsw->hAle      = ENET_MOD(&hCpsw->aleObj);
    hCpsw->hCpts     = ENET_MOD(&hCpsw->cptsObj);
    hCpsw->hMdio     = ENET_MOD(&hCpsw->mdioObj);

    /* Open DMA */
    hCpsw->hDma = EnetHostPortDma_open(hPer, cpswCfg->dmaCfg, NULL);
    if (NULL == hCpsw->hDma)
    {
        ENETTRACE_ERR("Failed to open CPSW DMA\r\n");
        status = ENET_EFAIL;
    }

    if (status == ENET_SOK)
    {
        /* Set escalate priority value (number of higher priority packets to be sent
        * before next lower priority is allowed to send a packet */
        CSL_CPSW_getPTypeReg(regs, &pType);
        pType.escPriLoadVal = cpswCfg->escalatePriorityLoadVal;
        CSL_CPSW_setPTypeReg(regs, &pType);

        /* Set VLAN config: aware/non-aware, inner/outer tag */
        if (cpswCfg->vlanCfg.vlanAware)
        {
            CSL_CPSW_enableVlanAware(regs);
        }
        else
        {
            CSL_CPSW_disableVlanAware(regs);
        }

        CSL_CPSW_setVlanType(regs, (uint32_t)cpswCfg->vlanCfg.vlanSwitch);
        CSL_CPSW_setVlanLTypeReg(regs, cpswCfg->vlanCfg.innerVlan, cpswCfg->vlanCfg.outerVlan);

        /* Set port global config */
        for (i = 0U; i < ENET_PRI_NUM; i++)
        {
            CSL_CPSW_setTxMaxLenPerPriority(regs, i, cpswCfg->txMtu[i]);

            /* Save largest of per priority port egress MTUs to check against
             * host port and MAC port RX MTUs */
            if (cpswCfg->txMtu[i] > hCpsw->maxPerPrioMtu)
            {
                hCpsw->maxPerPrioMtu = cpswCfg->txMtu[i];
            }
        }

        /* Initialize all CPSW modules */
        status = Cpsw_openInternal(hCpsw, enetType, instId, cpswCfg);
        ENETTRACE_ERR_IF(status != ENET_SOK, "Failed to open CPSW modules: %d\r\n", status);
    }

    key = EnetOsal_disableAllIntr();

    /* Register interrupts */
    if (status == ENET_SOK)
    {
        status = Cpsw_registerIntrs(hCpsw, cpswCfg);
        ENETTRACE_ERR_IF(status != ENET_SOK, "Failed to register interrupts: %d\r\n", status);
    }

    /* Enable CPTS interrupt */
    if (status == ENET_SOK)
    {
        ENET_IOCTL_SET_NO_ARGS(&prms);
        status = EnetMod_ioctl(hCpsw->hCpts, CPSW_CPTS_IOCTL_ENABLE_INTR, &prms);
        ENETTRACE_ERR_IF(status != ENET_SOK, "Failed to enable CPTS Interrupt: %d\r\n", status);
    }

    /* All initialization is complete */
    if (status == ENET_SOK)
    {
        /* Nothing else do to */
    }
    else if (status != ENET_EALREADYOPEN)
    {
        /* Rollback if any error other than trying to open while already open */
        Cpsw_unregisterIntrs(hCpsw);
        Cpsw_closeInternal(hCpsw);
    }
    else
    {
        ENETTRACE_ERR("Unexpected status: %d\r\n", status);
    }

    EnetOsal_restoreAllIntr(key);

    return status;
}

int32_t Cpsw_rejoin(EnetPer_Handle hPer,
                    Enet_Type enetType,
                    uint32_t instId)
{
    Cpsw_Handle hCpsw = (Cpsw_Handle)hPer;
    uint32_t i;

    /* Save EnetMod handles for easy access */
    for (i = 0U; i < hCpsw->macPortNum; i++)
    {
        hCpsw->hMacPort[i] = ENET_MOD(&hCpsw->macPortObj[i]);
    }

    hCpsw->hHostPort = ENET_MOD(&hCpsw->hostPortObj);
    hCpsw->hStats    = ENET_MOD(&hCpsw->statsObj);
    hCpsw->hAle      = ENET_MOD(&hCpsw->aleObj);
    hCpsw->hCpts     = ENET_MOD(&hCpsw->cptsObj);
    hCpsw->hMdio     = ENET_MOD(&hCpsw->mdioObj);

    return ENET_SOK;
}

void Cpsw_close(EnetPer_Handle hPer)
{
    Cpsw_Handle hCpsw = (Cpsw_Handle)hPer;
    Enet_IoctlPrms prms;
    uintptr_t key;
    int32_t status;

    key = EnetOsal_disableAllIntr();

    /* Disable CPTS interrupt */
    ENET_IOCTL_SET_NO_ARGS(&prms);
    status = EnetMod_ioctl(hCpsw->hCpts, CPSW_CPTS_IOCTL_DISABLE_INTR, &prms);
    ENETTRACE_ERR_IF(status != ENET_SOK, "Failed to disable CPTS Interrupt: %d\r\n", status);

    /* Unregister interrupts */
    if (status == ENET_SOK)
    {
        Cpsw_unregisterIntrs(hCpsw);
        Cpsw_closeInternal(hCpsw);
    }

   EnetOsal_restoreAllIntr(key);
}

int32_t Cpsw_ioctl(EnetPer_Handle hPer,
                   uint32_t cmd,
                   Enet_IoctlPrms *prms)
{
    Cpsw_Handle hCpsw = (Cpsw_Handle)hPer;
    uint32_t major;
    int32_t status = ENET_SOK;

    if (ENET_IOCTL_GET_TYPE(cmd) != ENET_IOCTL_TYPE_PUBLIC)
    {
        ENETTRACE_ERR("%s: IOCTL cmd %u is not public\r\n", hPer->name, cmd);
        status = ENET_EINVALIDPARAMS;
    }

    /* Route IOCTL command to Per or Mod */
    if (status == ENET_SOK)
    {
        major = ENET_IOCTL_GET_MAJ(cmd);
        switch (major)
        {
            case ENET_IOCTL_PER_BASE:
            {
                status = Cpsw_ioctlInternal(hPer, cmd, prms);
            }
            break;

            case ENET_IOCTL_FDB_BASE:
            {
                status = EnetMod_ioctl(hCpsw->hAle, cmd, prms);
            }
            break;

            case ENET_IOCTL_TIMESYNC_BASE:
            {
                status = EnetMod_ioctl(hCpsw->hCpts, cmd, prms);
            }
            break;

            case ENET_IOCTL_HOSTPORT_BASE:
            {
                status = EnetMod_ioctl(hCpsw->hHostPort, cmd, prms);
            }
            break;

            case ENET_IOCTL_MACPORT_BASE:
            {
                /* Note: Typecast to GenericInArgs is possible because all public
                 * MAC port IOCTL input args have macPort as their first member */
                EnetMacPort_GenericInArgs *inArgs = (EnetMacPort_GenericInArgs *)prms->inArgs;
                uint32_t portNum = ENET_MACPORT_NORM(inArgs->macPort);

                if (portNum < EnetSoc_getMacPortMax(hPer->enetType, hPer->instId))
                {
                    status = EnetMod_ioctl(hCpsw->hMacPort[portNum], cmd, prms);
                }
                else
                {
                    status = ENET_EINVALIDPARAMS;
                }
            }
            break;

            case ENET_IOCTL_MDIO_BASE:
            {
                status = EnetMod_ioctl(hCpsw->hMdio, cmd, prms);
            }
            break;

            case ENET_IOCTL_STATS_BASE:
            {
                status = EnetMod_ioctl(hCpsw->hStats, cmd, prms);
            }
            break;

            case ENET_IOCTL_PHY_BASE:
            {
                /* Note: Typecast to GenericInArgs is possible because all public
                 * MAC port IOCTL input args have macPort as their first member */
                EnetPhy_GenericInArgs *inArgs = (EnetPhy_GenericInArgs *)prms->inArgs;
                uint32_t portNum = ENET_MACPORT_NORM(inArgs->macPort);

                /* Assert if port number is not correct */
                Enet_assert(portNum < EnetSoc_getMacPortMax(hPer->enetType, hPer->instId),
                            "Invalid Port Id: %u\r\n", portNum);

                EnetPhy_Handle hPhy = hCpsw->hPhy[portNum];

                if (hPhy != NULL)
                {
                    status = EnetPhyMdioDflt_ioctl(hCpsw->hPhy[portNum], cmd, prms);
                }
                else
                {
                    status = ENET_EFAIL;
                }
            }
            break;

            case ENET_IOCTL_RM_BASE:
            {
                status = ENET_ENOTSUPPORTED;
            }
            break;

            default:
                break;
        }
    }

    return status;
}

void Cpsw_poll(EnetPer_Handle hPer,
               Enet_Event evt,
               const void *arg,
               uint32_t argSize)
{
    /* Nothing to do, not supported */
    ENETTRACE_WARN("Poll feature is not supported by CPSW Per\r\n");
}

void Cpsw_periodicTick(EnetPer_Handle hPer)
{
    Cpsw_Handle hCpsw = (Cpsw_Handle)hPer;
    EnetPhy_Handle hPhy;
    Enet_MacPort macPort;
    Cpsw_PortLinkState *portLinkState;
    EnetPhy_LinkStatus linkStatus;
    bool linked;
    uint32_t maxPorts = 0U;
    uint32_t portId;
    uint32_t i;
    int32_t status;
    uint32_t numPortCb;
    struct Cpsw_PortLinkCbInfoList_s
    {
        bool linked;
        Enet_MacPort macPort;
    } portCbInfoList[CPSW_MAC_PORT_NUM];

    numPortCb = 0;

    /* Get the max number of ports */
    maxPorts = EnetSoc_getMacPortMax(hPer->enetType, hPer->instId);

    /* Run PHY tick and handle link up/down events */
    for (i = 0U; i < maxPorts; i++)
    {
        macPort = ENET_MACPORT_DENORM(i);
        portId = ENET_MACPORT_ID(macPort);

        ENETTRACE_VAR(portId);
        hPhy = hCpsw->hPhy[i];
        portLinkState = &hCpsw->portLinkState[i];

        /* Check if the corresponding PHY is enabled */
        if (portLinkState->isTickEnabled)
        {
            /* TODO: Need to make lock more granular */
            //EnetOsal_lockMutex(hCpsw->lock);

            if (hPhy != NULL)
            {
                /* Run PHY tick */
                linkStatus = EnetPhy_tick(hPhy);

                /* Handle link up/down events */
                if ((linkStatus == ENETPHY_GOT_LINK) ||
                    (linkStatus == ENETPHY_LOST_LINK))
                {
                    linked = (linkStatus == ENETPHY_GOT_LINK);
                    status = linked ? Cpsw_handleLinkUp(hCpsw, macPort) :
                                      Cpsw_handleLinkDown(hCpsw, macPort);
                    ENETTRACE_ERR_IF(status != ENET_SOK,
                                     "Port %u: Failed to handle link change: %d\r\n", portId, status);

                    /* Call application callback when port link is up - at this point app can
                     * start data flow */
                    if ((status == ENET_SOK) && (hCpsw->portLinkStatusChangeCb != NULL))
                    {
                        /* Add port's to callback info list.
                         * All portLinkStatus Cb functions are invoked at the
                         * end of function after relinquishing locks */
                        Enet_devAssert(numPortCb < ENET_ARRAYSIZE(portCbInfoList),
                                       "Invalid port number %u, expected < %u\r\n",
                                       numPortCb, ENET_ARRAYSIZE(portCbInfoList));
                        portCbInfoList[numPortCb].macPort = macPort;
                        portCbInfoList[numPortCb].linked  = linked;
                        numPortCb++;
                    }

                    /* All checks cleared, link state can be updated now */
                    if (status == ENET_SOK)
                    {
                        portLinkState->isLinkUp = linked;

                        /* Disable periodic tick while link is up. It will be re-enabled
                         * by MDIO_LINKINT upon link down detection */
                        if (portLinkState->isPollEnabled && linked)
                        {
                            portLinkState->isTickEnabled = false;
                        }
                    }
                }
            }
            else if (EnetMod_isOpen(hCpsw->hMacPort[i]))
            {
                /* If port is in NOPHY mode, invoke the portLinkUp Cb */
                portLinkState->isLinkUp      = true;
                portLinkState->isTickEnabled = false;
                if (hCpsw->portLinkStatusChangeCb != NULL)
                {
                    /* Add port's to callback info list.
                     * All portLinkStatus Cb functions are invoked at the
                     * end of function after relinquishing locks */
                    Enet_devAssert(numPortCb < ENET_ARRAYSIZE(portCbInfoList),
                                   "Invalid port number %u, expected < %u\r\n",
                                   numPortCb, ENET_ARRAYSIZE(portCbInfoList));
                    portCbInfoList[numPortCb].macPort = macPort;
                    portCbInfoList[numPortCb].linked  = true;
                    numPortCb++;
                }
            }

            //EnetOsal_unlockMutex(hCpsw->lock);
        }
    }

    for (i = 0U; i < numPortCb; i++)
    {
        if (hCpsw->portLinkStatusChangeCb != NULL)
        {
            /* Call application's port link status change callback */
            hCpsw->portLinkStatusChangeCb(portCbInfoList[i].macPort,
                                          portCbInfoList[i].linked,
                                          hCpsw->portLinkStatusChangeCbArg);
        }
    }

    return;
}

EnetDma_Handle Cpsw_getDmaHandle(const Enet_Handle hEnet)
{
    Cpsw_Handle hCpsw = (Cpsw_Handle)Enet_getPerHandle(hEnet);

    return(hCpsw->hDma);
}

static int32_t Cpsw_openInternal(Cpsw_Handle hCpsw,
                                 Enet_Type enetType,
                                 uint32_t instId,
                                 const Cpsw_Cfg *cfg)
{
    Cpsw_MdioLinkIntCtx *linkIntCtx = &hCpsw->mdioLinkIntCtx;
    Cpsw_PortLinkState *portLinkState;
    uint32_t i;
    int32_t status = ENET_SOK;

    /* Clear PHY link state */
    for (i = 0U; i < ENET_ARRAYSIZE(hCpsw->portLinkState); i++)
    {
        portLinkState = &hCpsw->portLinkState[i];

        portLinkState->phyAddr       = ENETPHY_INVALID_PHYADDR;
        portLinkState->isOpen        = false;
        portLinkState->isLinkUp      = false;
        portLinkState->isPollEnabled = false;
        portLinkState->isTickEnabled = true;
    }

    linkIntCtx->aliveMask            = ENET_MDIO_PHY_ADDR_MASK_NONE;
    linkIntCtx->linkedMask           = ENET_MDIO_PHY_ADDR_MASK_NONE;
    linkIntCtx->pollEnableMask       = cfg->mdioCfg.pollEnMask;
    linkIntCtx->linkStateChangeCb    = cfg->mdioLinkStateChangeCb;
    linkIntCtx->linkStateChangeCbArg = cfg->mdioLinkStateChangeCbArg;
    hCpsw->portLinkStatusChangeCb    = cfg->portLinkStatusChangeCb;
    hCpsw->portLinkStatusChangeCbArg = cfg->portLinkStatusChangeCbArg;

    /* Host port and MAC port MTUs should not be greater than largest of the port
     * egress per priority MTU as packet would get dropped by switch.
     * Though it is valid from hardware configuration, we return an error as it
     * serves no purpose */
    if (cfg->hostPortCfg.rxMtu > hCpsw->maxPerPrioMtu)
    {
        ENETTRACE_ERR("Host Port RX MTU (%d) exceeds max of TX Priority 0-7 MTU (%d)\r\n ",
                      cfg->hostPortCfg.rxMtu, hCpsw->maxPerPrioMtu);
        status = ENET_EINVALIDPARAMS;
    }

    /* Open host port */
    if (status == ENET_SOK)
    {
        status = EnetMod_open(hCpsw->hHostPort, enetType, instId, &cfg->hostPortCfg, sizeof(cfg->hostPortCfg));
        ENETTRACE_ERR_IF(status != ENET_SOK, "Failed to open host port: %d\r\n", status);
    }

    /* Open ALE */
    if (status == ENET_SOK)
    {
        status = EnetMod_open(hCpsw->hAle, enetType, instId, &cfg->aleCfg, sizeof(cfg->aleCfg));
        ENETTRACE_ERR_IF(status != ENET_SOK, "Failed to open ALE: %d\r\n", status);
    }

    /* Open CPTS */
    if (status == ENET_SOK)
    {
        status = EnetMod_open(hCpsw->hCpts, enetType, instId, &cfg->cptsCfg, sizeof(cfg->cptsCfg));
        ENETTRACE_ERR_IF(status != ENET_SOK, "Failed to open CPTS: %d\r\n", status);
    }

    /* Open MDIO */
    if (status == ENET_SOK)
    {
        status = EnetMod_open(hCpsw->hMdio, enetType, instId, &cfg->mdioCfg, sizeof(cfg->mdioCfg));
        ENETTRACE_ERR_IF(status != ENET_SOK, "Failed to open MDIO: %d\r\n", status);
    }

    /* Open statistics */
    if (status == ENET_SOK)
    {
        status = EnetMod_open(hCpsw->hStats, enetType, instId, NULL, 0U);
        ENETTRACE_ERR_IF(status != ENET_SOK, "Failed to open stats: %d\r\n", status);
    }

    if (status != ENET_SOK)
    {
        Cpsw_closeInternal(hCpsw);
    }

    return status;
}

static uint32_t Cpsw_getTxMtuPerPriority(Cpsw_Handle hCpsw,
                                         uint32_t priority)
{
    uint32_t txMtu = 0U;
    EnetPer_Handle hPer = (EnetPer_Handle)hCpsw;
    CSL_Xge_cpswRegs *regs = (CSL_Xge_cpswRegs *)hPer->virtAddr;

    if (hCpsw != NULL)
    {
        txMtu = CSL_CPSW_getTxMaxLenPerPriority(regs, priority);
    }

    return txMtu;
}

static uint32_t Cpsw_getRxMtuPort0(Cpsw_Handle hCpsw)
{
    int32_t status = ENET_SOK;
    Enet_IoctlPrms prms;
    EnetPort_MaxLen maxLen;

    maxLen.mru = 0U;
    ENET_IOCTL_SET_IN_ARGS(&prms, &maxLen);
    status = CpswHostPort_ioctl(hCpsw->hHostPort,
                                ENET_HOSTPORT_IOCTL_GET_MAXLEN,
                                &prms);
    if (status != ENET_SOK)
    {
        ENETTRACE_ERR("failed to get host port's RX MTU: %d\r\n", status);
    }

    return maxLen.mru;
}

static void Cpsw_closeInternal(Cpsw_Handle hCpsw)
{
    EnetMod_close(hCpsw->hHostPort);
    EnetMod_close(hCpsw->hAle);
    EnetMod_close(hCpsw->hCpts);
    EnetMod_close(hCpsw->hMdio);
    EnetMod_close(hCpsw->hStats);

    Enet_assert(hCpsw->hDma != NULL);
    EnetHostPortDma_close(hCpsw->hDma);
    hCpsw->hDma = NULL;
}

static int32_t Cpsw_ioctlInternal(EnetPer_Handle hPer,
                                  uint32_t cmd,
                                  Enet_IoctlPrms *prms)
{
    Cpsw_Handle hCpsw = (Cpsw_Handle)hPer;
    CSL_Xge_cpswRegs *regs = (CSL_Xge_cpswRegs *)hPer->virtAddr;
    int32_t status = ENET_SOK;

#if ENET_CFG_IS_ON(DEV_ERROR)
    /* Validate CPSW peripheral IOCTL parameters */
    if ((ENET_IOCTL_GET_PER(cmd) == ENET_IOCTL_PER_CPSW) &&
        (ENET_IOCTL_GET_TYPE(cmd) == ENET_IOCTL_TYPE_PUBLIC))
    {
        status = Enet_validateIoctl(cmd, prms,
                                    gCpsw_ioctlValidate,
                                    ENET_ARRAYSIZE(gCpsw_ioctlValidate));
        ENETTRACE_ERR_IF(status != ENET_SOK, "IOCTL 0x%08x params are not valid\r\n", cmd);
    }
#endif

    switch (cmd)
    {
        case ENET_PER_IOCTL_GET_VERSION:
        {
            Enet_Version *version = (Enet_Version *)prms->outArgs;
            CSL_CPSW_VERSION ver;

            CSL_CPSW_getCpswVersionInfo(regs, &ver);
            version->maj = ver.majorVer;
            version->min = ver.minorVer;
            version->rtl = ver.rtlVer;
            version->id  = ver.id;
            version->other1 = ENET_VERSION_NONE;
            version->other2 = ENET_VERSION_NONE;
        }
        break;

        case ENET_PER_IOCTL_PRINT_REGS:
        {
            /* TODO: Print CPSW SS + CPSW NU registers */
            status = ENET_ENOTSUPPORTED;
        }
        break;

        case ENET_PER_IOCTL_OPEN_PORT_LINK:
        {
            const EnetPer_PortLinkCfg *portLinkCfg = (const EnetPer_PortLinkCfg *)prms->inArgs;
            Enet_MacPort macPort = portLinkCfg->macPort;
            const CpswMacPort_Cfg *macCfg = (CpswMacPort_Cfg *)portLinkCfg->macCfg;
            const EnetPhy_Cfg *phyCfg = &portLinkCfg->phyCfg;
            const EnetMacPort_Interface *mii = &portLinkCfg->mii;
            const EnetMacPort_LinkCfg *linkCfg = &portLinkCfg->linkCfg;

            if (macCfg->rxMtu > hCpsw->maxPerPrioMtu)
            {
                /* Host port and MAC port MTU should not be greater than largest of the port
                 * egress per priority MTU as packet would get dropped by Switch.
                 * Though it is valid from HW configuration, we return an error as it serves
                 * no purpose */
                ENETTRACE_ERR("Port %u: RX MTU (%d) exceeds max of TX Priority 0-7 MTU (%u)\r\n ",
                              ENET_MACPORT_ID(macPort), macCfg->rxMtu, hCpsw->maxPerPrioMtu);
                status = ENET_EINVALIDPARAMS;
            }
            else
            {
                status = Cpsw_openPortLink(hCpsw, macPort, macCfg, phyCfg, mii, linkCfg);
                ENETTRACE_ERR_IF(status != ENET_SOK,
                                 "Port %u: Failed to open port link: %d\r\n",
                                 ENET_MACPORT_ID(macPort), status);
            }
        }
        break;

        case ENET_PER_IOCTL_CLOSE_PORT_LINK:
        {
            Enet_MacPort macPort = *(Enet_MacPort *)prms->inArgs;

            Cpsw_closePortLink(hCpsw, macPort);
        }
        break;

        case ENET_PER_IOCTL_IS_PORT_LINK_UP:
        {
            Enet_MacPort macPort = *(Enet_MacPort *)prms->inArgs;
            bool *linkUp = (bool *)prms->outArgs;
            uint32_t portNum = ENET_MACPORT_NORM(macPort);

            /* Assert if port number is not correct */
            Enet_assert(portNum < EnetSoc_getMacPortMax(hPer->enetType, hPer->instId),
                        "Invalid Port Id: %u\r\n", portNum);

            *linkUp = hCpsw->portLinkState[portNum].isLinkUp;
        }
        break;

        case ENET_PER_IOCTL_GET_PORT_LINK_CFG:
        {
            Enet_MacPort macPort = *(Enet_MacPort *)prms->inArgs;
            EnetMacPort_LinkCfg *linkCfg = (EnetMacPort_LinkCfg *)prms->outArgs;

            status = Cpsw_getPortLinkCfg(hCpsw, macPort, linkCfg);
            ENETTRACE_ERR_IF(status != ENET_SOK,
                             "Port %u: Failed to get link config: %d\r\n",
                             ENET_MACPORT_ID(macPort), status);
        }
        break;

        case ENET_PER_IOCTL_ATTACH_CORE:
        {
        	EnetPer_AttachCoreOutArgs *outArgs = (EnetPer_AttachCoreOutArgs *)prms->outArgs;
        	uint32_t i;

        	/* Get MTU values */
        	for (i = 0U; i < ENET_PRI_NUM; i++)
        	{
        		outArgs->txMtu[i] = Cpsw_getTxMtuPerPriority(hCpsw, i);
        	}

        	outArgs->rxMtu = Cpsw_getRxMtuPort0(hCpsw);
            status = ENET_SOK;
        }
        break;

        case ENET_PER_IOCTL_DETACH_CORE:
        {
            /* Nothing to be done for AM273X CORE detach as no RM is present */
            status = ENET_SOK;
        }
        break;

        case CPSW_PER_IOCTL_SET_INTERVLAN_ROUTE_UNI_EGRESS:
        case CPSW_PER_IOCTL_CLEAR_INTERVLAN_ROUTE_UNI_EGRESS:
        case CPSW_PER_IOCTL_SET_INTERVLAN_ROUTE_MULTI_EGRESS:
        case CPSW_PER_IOCTL_CLEAR_INTERVLAN_ROUTE_MULTI_EGRESS:
        {
#if ENET_CFG_IS_ON(CPSW_INTERVLAN)
#if ENET_CFG_IS_OFF(CPSW_MACPORT_INTERVLAN)
#error "CPSW interVLAN feature requires ENET_CFG_CPSW_MACPORT_INTERVLAN"
#endif
            if (ENET_FEAT_IS_EN(hPer->features, CPSW_FEATURE_INTERVLAN))
            {
                status = Cpsw_ioctlInterVlan(hPer, cmd, prms);
            }
            else
            {
                status = ENET_ENOTSUPPORTED;
            }
#else
            status = ENET_ENOTSUPPORTED;
#endif
        }
        break;

        case CPSW_PER_IOCTL_SET_SHORT_IPG_CFG:
        {
            const Cpsw_SetTxShortIpgCfgInArgs *inArgs = (const Cpsw_SetTxShortIpgCfgInArgs *)prms->inArgs;

            status = Cpsw_setTxShortIpgCfg(hCpsw, inArgs);
            ENETTRACE_ERR_IF(status != ENET_SOK, "Failed to set TX short IPG config: %d\r\n", status);
        }
        break;

        case CPSW_PER_IOCTL_GET_SHORT_IPG_CFG:
        {
            Cpsw_TxShortIpgCfg *shortIpgCfg = (Cpsw_TxShortIpgCfg *)prms->outArgs;

            status = Cpsw_getTxShortIpgCfg(hCpsw, shortIpgCfg);
            ENETTRACE_ERR_IF(status != ENET_SOK, "Failed to get TX short IPG config: %d\r\n", status);
        }
        break;

        case ENET_IOCTL_REGISTER_RX_DEFAULT_FLOW:
        case ENET_IOCTL_UNREGISTER_RX_DEFAULT_FLOW:
        case ENET_IOCTL_REGISTER_DSTMAC_RX_FLOW:
        case ENET_IOCTL_UNREGISTER_DSTMAC_RX_FLOW:
        {
            status = ENET_ENOTSUPPORTED;
        }
        break;

        default:
        {
            status = ENET_EINVALIDPARAMS;
        }
        break;
    }

    return status;
}

static int32_t Cpsw_registerIntrs(Cpsw_Handle hCpsw,
                                  const Cpsw_Cfg *cfg)
{
    EnetPer_Handle hPer = (EnetPer_Handle)hCpsw;
    Enet_Type enetType = hPer->enetType;
    uint32_t instId = hPer->instId;
    uint32_t intrNum;
    uint32_t trigType;
    int32_t status = ENET_SOK;

    /* Register Rx Threshold interrupt */
    if (status == ENET_SOK)
    {
        intrNum = EnetSoc_getIntrNum(enetType, instId, CPSW_INTR_RX_THRESH);
        trigType = EnetSoc_getIntrTriggerType(enetType, instId, CPSW_INTR_RX_THRESH);
        hCpsw->hDmaRxThreshIntr = EnetOsal_registerIntr(Cpsw_dmaRxThreshIsr,
                                                        intrNum,
                                                        cfg->intrPriority,
                                                        trigType,
                                                        hCpsw);
        if (hCpsw->hDmaRxThreshIntr == NULL)
        {
            ENETTRACE_ERR("Failed to register rx thresh intr\r\n");
            status = ENET_EFAIL;
        }
    }

    /* Register Rx interrupt */
    if (status == ENET_SOK)
    {
        intrNum = EnetSoc_getIntrNum(enetType, instId, CPSW_INTR_RX_PEND);
        trigType = EnetSoc_getIntrTriggerType(enetType, instId, CPSW_INTR_RX_PEND);
        hCpsw->hDmaRxIntr = EnetOsal_registerIntr(Cpsw_dmaRxIsr,
                                                  intrNum,
                                                  cfg->intrPriority,
                                                  trigType,
                                                  hCpsw);
        if (hCpsw->hDmaRxIntr == NULL)
        {
            ENETTRACE_ERR("Failed to register rx intr\r\n");
            status = ENET_EFAIL;
        }
    }

     /* Register Tx interrupt */
    if (status == ENET_SOK)
    {
        intrNum = EnetSoc_getIntrNum(enetType, instId, CPSW_INTR_TX_PEND);
        trigType = EnetSoc_getIntrTriggerType(enetType, instId, CPSW_INTR_TX_PEND);
        hCpsw->hDmaTxIntr = EnetOsal_registerIntr(Cpsw_dmaTxIsr,
                                                  intrNum,
                                                  cfg->intrPriority,
                                                  trigType,
                                                  hCpsw);
        if (hCpsw->hDmaTxIntr == NULL)
        {
            ENETTRACE_ERR("Failed to register tx intr\r\n");
            status = ENET_EFAIL;
        }
    }

     /* Register Miscellaneous interrupt */
    if (status == ENET_SOK)
    {
        intrNum = EnetSoc_getIntrNum(enetType, instId, CPSW_INTR_MISC_PEND);
        trigType = EnetSoc_getIntrTriggerType(enetType, instId, CPSW_INTR_MISC_PEND);
        hCpsw->hDmaMiscIntr = EnetOsal_registerIntr(Cpsw_dmaMiscIsr,
                                                    intrNum,
                                                    cfg->intrPriority,
                                                    trigType,
                                                    hCpsw);
        if (hCpsw->hDmaMiscIntr == NULL)
        {
            ENETTRACE_ERR("Failed to register misc intr\r\n");
            status = ENET_EFAIL;
        }
    }

    /* Unwind in case of any error */
    if (status != ENET_SOK)
    {
        if (hCpsw->hDmaRxThreshIntr != NULL)
        {
            EnetOsal_unregisterIntr(hCpsw->hDmaRxThreshIntr);
        }

        if (hCpsw->hDmaRxIntr != NULL)
        {
            EnetOsal_unregisterIntr(hCpsw->hDmaRxIntr);
        }

        if (hCpsw->hDmaTxIntr != NULL)
        {
            EnetOsal_unregisterIntr(hCpsw->hDmaTxIntr);
        }

        if (hCpsw->hDmaMiscIntr != NULL)
        {
            EnetOsal_unregisterIntr(hCpsw->hDmaMiscIntr);
        }
    }

    return status;
}

static void Cpsw_unregisterIntrs(Cpsw_Handle hCpsw)
{
    /* Unregister Rx Threshold interrupt */
    if (hCpsw->hDmaRxThreshIntr != NULL)
    {
        EnetOsal_unregisterIntr(hCpsw->hDmaRxThreshIntr);
    }

    /* Unregister Rx interrupt */
    if (hCpsw->hDmaRxIntr != NULL)
    {
        EnetOsal_unregisterIntr(hCpsw->hDmaRxIntr);
    }

    /* Unregister Tx interrupt */
    if (hCpsw->hDmaTxIntr != NULL)
    {
        EnetOsal_unregisterIntr(hCpsw->hDmaTxIntr);
    }

    /* Unregister Miscellaneous interrupt */
    if (hCpsw->hDmaMiscIntr != NULL)
    {
        EnetOsal_unregisterIntr(hCpsw->hDmaMiscIntr);
    }
}

static void Cpsw_statsIsr(uintptr_t arg)
{
    EnetMod_Handle hStats = (EnetMod_Handle)arg;
    Enet_IoctlPrms prms;
    int32_t status;

    ENET_IOCTL_SET_NO_ARGS(&prms);
    status = EnetMod_ioctl(hStats, CPSW_STATS_IOCTL_SYNC, &prms);

    /* TODO: Add ISR safe error:
     * ("Failed to sync statistics counters: %d\r\n", status); */
    ENET_UNUSED(status);
}

static void Cpsw_mdioIsr(uintptr_t arg)
{
    Cpsw_Handle hCpsw = (Cpsw_Handle)arg;
    EnetMod_Handle hMdio = hCpsw->hMdio;
    Enet_IoctlPrms prms;
    Mdio_Callbacks callbacks =
    {
        .linkStateCb  = Cpsw_handleMdioLinkStateChange,
        .userAccessCb = NULL,
        .cbArgs       = hCpsw,
    };
    int32_t status;

    ENET_IOCTL_SET_IN_ARGS(&prms, &callbacks);
    status = EnetMod_ioctl(hMdio, MDIO_IOCTL_HANDLE_INTR, &prms);

    /* TODO: Add ISR safe error:
     * ("Failed to handle MDIO intr: %d\r\n", status); */
    ENET_UNUSED(status);
}

static void Cpsw_cptsIsr(uintptr_t arg)
{
    Cpsw_Handle hCpsw = (Cpsw_Handle)arg;
    EnetMod_Handle hCpts = hCpsw->hCpts;
    Enet_IoctlPrms prms;
    int32_t status;

    ENET_IOCTL_SET_NO_ARGS(&prms);
    status = EnetMod_ioctl(hCpts, CPSW_CPTS_IOCTL_HANDLE_INTR, &prms);

    /* TODO: Add ISR safe error:
     * ("Failed to handle CPTS intr: %d\r\n", status); */
    ENET_UNUSED(status);
}

static void Cpsw_dmaRxThreshIsr(uintptr_t arg)
{
    Cpsw_Handle hCpsw = (Cpsw_Handle)arg;
    EnetDma_Handle hEnetDma = hCpsw->hDma;
    int32_t status;

    Enet_assert(hEnetDma != NULL);

    status = EnetCpdma_rxThreshIsr(hEnetDma);

    /* TODO: Add ISR safe error:
     * failed to handle Rx Thresh intr: %d\r\n", status); */
    ENET_UNUSED(status);
}

static void Cpsw_dmaRxIsr(uintptr_t arg)
{
    Cpsw_Handle hCpsw = (Cpsw_Handle)arg;
    EnetDma_Handle hEnetDma = hCpsw->hDma;
    int32_t status;
    uintptr_t key;

    key = EnetOsal_disableAllIntr();
    Enet_assert(hEnetDma != NULL);

    status = EnetCpdma_rxIsr(hEnetDma);

    /* TODO: Add ISR safe error:
     * failed to handle Rx intr: %d\r\n", status); */
    ENET_UNUSED(status);
    EnetOsal_restoreAllIntr(key);
}

static void Cpsw_dmaTxIsr(uintptr_t arg)
{
    Cpsw_Handle hCpsw = (Cpsw_Handle)arg;
    EnetDma_Handle hEnetDma = hCpsw->hDma;
    int32_t status;
    uintptr_t key;

    Enet_assert(hEnetDma != NULL);
    key = EnetOsal_disableAllIntr();

    status = EnetCpdma_txIsr(hEnetDma);

    /* TODO: Add ISR safe error:
     * failed to handle Tx intr: %d\r\n", status); */
    ENET_UNUSED(status);
    EnetOsal_restoreAllIntr(key);
}

static void Cpsw_dmaMiscIsr(uintptr_t arg)
{
    Cpsw_Handle hCpsw = (Cpsw_Handle)arg;
    EnetDma_Handle hEnetDma = hCpsw->hDma;
    int32_t status;
    uint32_t statusMask;

    Enet_assert(hEnetDma != NULL);

    status = EnetCpdma_miscIsr(hEnetDma, &statusMask);

    if ((statusMask & (CPSW_MISC_INT_MDIO_USERINT_MASK | CPSW_MISC_INT_MDIO_LINKINT_MASK)) != 0U)
    {
        Cpsw_mdioIsr(arg);
    }

    if ((statusMask & CPSW_MISC_INT_CPTS_EVENT_MASK) != 0U)
    {
        Cpsw_cptsIsr(arg);
    }

    if ((statusMask & CPSW_MISC_INT_STAT_OVERFLOW_MASK) != 0U)
    {
        Cpsw_statsIsr((uintptr_t)hCpsw->hStats);
    }

    /* TODO: Add ISR safe error:
     * failed to handle Miscellaneous intr: %d\r\n", status); */
    ENET_UNUSED(status);
}

static void Cpsw_handleMdioLinkStateChange(EnetMdio_Group group,
                                           Mdio_PhyStatus *phyStatus,
                                           void *cbArgs)
{
    Cpsw_Handle hCpsw = (Cpsw_Handle)cbArgs;
    Cpsw_MdioLinkIntCtx *linkIntCtx = &hCpsw->mdioLinkIntCtx;
    Cpsw_PortLinkState *portLinkState;
    Cpsw_MdioLinkStateChangeInfo info;
    uint32_t aliveMaskChange;
    uint32_t linkedMaskChange;
    uint32_t i;

    aliveMaskChange  = phyStatus->aliveMask ^ linkIntCtx->aliveMask;
    linkedMaskChange = phyStatus->linkedMask ^ linkIntCtx->linkedMask;

    for (i = 0U; i <= MDIO_MAX_PHY_CNT; i++)
    {
        info.aliveChanged = ENET_IS_BIT_SET(aliveMaskChange, i);
        info.linkChanged  = ENET_IS_BIT_SET(linkedMaskChange, i);

        if (info.aliveChanged || info.linkChanged)
        {
            info.phyAddr  = i;
            info.isAlive  = ENET_IS_BIT_SET(phyStatus->aliveMask, i);
            info.isLinked = ENET_IS_BIT_SET(phyStatus->linkedMask, i);

            /* Re-enable periodic tick when link down is detected */
            if (info.linkChanged && !info.isLinked)
            {
                portLinkState = Cpsw_getPortLinkState(hCpsw, i);
                if (portLinkState != NULL)
                {
                    portLinkState->isTickEnabled = true;
                }
            }

            linkIntCtx->linkStateChangeCb(&info, linkIntCtx->linkStateChangeCbArg);
        }
    }

    linkIntCtx->aliveMask  = phyStatus->aliveMask;
    linkIntCtx->linkedMask = phyStatus->linkedMask;
}

static Cpsw_PortLinkState *Cpsw_getPortLinkState(Cpsw_Handle hCpsw,
                                                 uint32_t phyAddr)
{
    Cpsw_PortLinkState *portLinkState = NULL;
    uint32_t i;

    for (i = 0U; i < CPSW_MAC_PORT_NUM; i++)
    {
        if (hCpsw->portLinkState[i].isOpen &&
            (phyAddr == hCpsw->portLinkState[i].phyAddr))
        {
            portLinkState = &hCpsw->portLinkState[i];
            break;
        }
    }

    return portLinkState;
}

static int32_t Cpsw_openPortLinkWithPhy(Cpsw_Handle hCpsw,
                                        Enet_MacPort macPort,
                                        const CpswMacPort_Cfg *macCfg,
                                        const EnetPhy_Cfg *phyCfg,
                                        const EnetMacPort_Interface *mii,
                                        const EnetMacPort_LinkCfg *linkCfg)
{
    EnetPer_Handle hPer = (EnetPer_Handle)hCpsw;
    uint32_t portNum = ENET_MACPORT_NORM(macPort);
    uint32_t portId = ENET_MACPORT_ID(macPort);
    EnetMod_Handle hMacPort = hCpsw->hMacPort[portNum];
    EnetPhy_MdioHandle hPhyMdio = EnetPhyMdioDflt_getPhyMdio();
    EnetPhy_Mii phyMii;
    EnetPhy_LinkCfg phyLinkCfg;
    CpswMacPort_ModCfg macModCfg;
    uint32_t macPortCaps;
    int32_t status;

    /* Assert if port number is not correct */
    Enet_assert(portNum < EnetSoc_getMacPortMax(hPer->enetType, hPer->instId),
                "Invalid Port Id: %u\r\n", portNum);

    ENETTRACE_VAR(portId);
    /* Enet module takes a single config structure, which in CPSW MAC port case is
     * EnetMacPort_ModCfg. This structure contains the initial MAC specific configuration
     * and also the link configuration (speed/duplexity) and MII interface type. */
    macModCfg.macCfg = *macCfg;
    macModCfg.mii = *mii;
    macModCfg.linkCfg = *linkCfg;

    /* Open MAC port */
    status = EnetMod_open(hMacPort, hPer->enetType, hPer->instId, &macModCfg, sizeof(macModCfg));
    ENETTRACE_ERR_IF(status != ENET_SOK, "Port %u: Failed to open MAC: %d\r\n", portId, status);

    /* Open PHY */
    if (status == ENET_SOK)
    {
        /* Convert MII and link configuration from Enet to ENETPHY types */
        phyMii = EnetUtils_macToPhyMii(mii);
        phyLinkCfg.speed = (EnetPhy_Speed)linkCfg->speed;
        phyLinkCfg.duplexity = (EnetPhy_Duplexity)linkCfg->duplexity;

        /* Get MAC port capabilities from SoC standpoint */
        macPortCaps = EnetSoc_getMacPortCaps(hPer->enetType, hPer->instId, macPort);

        /* Open ENETPHY driver */
        hCpsw->hPhy[portNum] = EnetPhy_open(phyCfg, phyMii, &phyLinkCfg, macPortCaps, hPhyMdio, hCpsw->hMdio);
        if (hCpsw->hPhy[portNum] == NULL)
        {
            ENETTRACE_ERR("Port %u: Failed to open PHY\r\n", portId);
            CpswMacPort_close(hMacPort);
        }
    }

    return status;
}

static int32_t Cpsw_openPortLinkNoPhy(Cpsw_Handle hCpsw,
                                      Enet_MacPort macPort,
                                      const CpswMacPort_Cfg *macCfg,
                                      const EnetMacPort_Interface *mii,
                                      const EnetMacPort_LinkCfg *linkCfg)
{
    EnetPer_Handle hPer = (EnetPer_Handle)hCpsw;
    uint32_t portNum = ENET_MACPORT_NORM(macPort);
    uint32_t portId = ENET_MACPORT_ID(macPort);
    EnetMod_Handle hMacPort = hCpsw->hMacPort[portNum];
    CpswMacPort_ModCfg macModCfg;
    EnetMacPort_LinkCfg *macLinkCfg = &macModCfg.linkCfg;
    Enet_IoctlPrms prms;
    CpswAle_SetPortStateInArgs setPortStateInArgs;
    int32_t status = ENET_SOK;

    /* Assert if port number is not correct */
    Enet_assert(portNum < EnetSoc_getMacPortMax(hPer->enetType, hPer->instId),
                "Invalid Port Id: %u\r\n", portNum);

    ENETTRACE_VAR(portId);
    hCpsw->hPhy[portNum] = NULL;

    /* Enet module takes a single config structure, which in CPSW MAC port case is
     * EnetMacPort_ModCfg. This structure contains the initial MAC specific configuration
     * and also the link configuration (speed/duplexity) and MII interface type. */
    macModCfg.macCfg = *macCfg;
    macModCfg.mii = *mii;
    macModCfg.linkCfg = *linkCfg;

    /* Explicit speed and duplexity must be provided, can't be discovered */
    if ((macLinkCfg->speed == ENET_SPEED_AUTO) ||
        (macLinkCfg->duplexity == ENET_DUPLEX_AUTO))
    {
        ENETTRACE_ERR("Port %u: Auto-speed/duplexity is not valid for PHY-less links\r\n", portId);
        status = ENET_EINVALIDPARAMS;
    }

    /* Open MAC port in force mode */
    if (status == ENET_SOK)
    {
        if (EnetMacPort_isRgmii(mii))
        {
            if ((macCfg->loopbackEn == true) &&
                (macLinkCfg->speed != ENET_SPEED_1GBIT))
            {
                /* PDK-4633 - RGMII loopback fails in 100Mbps mode  */
                ENETTRACE_WARN("Port %u: RGMII loopback in 1G mode not supported, "
                               "overriding speed to 1Gbps\r\n", portId);
                macLinkCfg->speed = ENET_SPEED_1GBIT;
            }
        }

        status = EnetMod_open(hMacPort, hPer->enetType, hPer->instId, &macModCfg, sizeof(macModCfg));
        ENETTRACE_ERR_IF(status != ENET_SOK, "Port %u: Failed to open MAC: %d\r\n", portId, status);
    }

    /* Enable MAC port with requested speed/duplexity */
    if (status == ENET_SOK)
    {
        ENET_IOCTL_SET_IN_ARGS(&prms, macLinkCfg);
        status = EnetMod_ioctl(hMacPort, CPSW_MACPORT_IOCTL_ENABLE, &prms);
        ENETTRACE_ERR_IF(status != ENET_SOK, "Port %u: Failed to enable MAC port: %d\r\n", portId, status);
    }

    /* Set ALE port in forward state */
    if (status == ENET_SOK)
    {
        setPortStateInArgs.portNum   = CPSW_ALE_MACPORT_TO_ALEPORT(portNum);
        setPortStateInArgs.portState = CPSW_ALE_PORTSTATE_FORWARD;
        ENET_IOCTL_SET_IN_ARGS(&prms, &setPortStateInArgs);

        status = EnetMod_ioctl(hCpsw->hAle, CPSW_ALE_IOCTL_SET_PORT_STATE, &prms);
        ENETTRACE_ERR_IF(status != ENET_SOK,
                         "Port %u: Failed to set ALE port %u to forward state: %d\r\n",
                         portId, setPortStateInArgs.portNum, status);
    }

    if (status != ENET_SOK)
    {
        Cpsw_closePortLink(hCpsw, macPort);
    }

    return status;
}

static int32_t Cpsw_openPortLink(Cpsw_Handle hCpsw,
                                 Enet_MacPort macPort,
                                 const CpswMacPort_Cfg *macCfg,
                                 const EnetPhy_Cfg *phyCfg,
                                 const EnetMacPort_Interface *mii,
                                 const EnetMacPort_LinkCfg *linkCfg)
{
    EnetPer_Handle hPer = (EnetPer_Handle)hCpsw;
    uint32_t portNum = ENET_MACPORT_NORM(macPort);
    uint32_t portId = ENET_MACPORT_ID(macPort);
    EnetMod_Handle hMacPort = hCpsw->hMacPort[portNum];
    EnetPhy_Handle hPhy = hCpsw->hPhy[portNum];
    Cpsw_PortLinkState *portLinkState = &hCpsw->portLinkState[portNum];
    uint32_t pollEnMask = hCpsw->mdioLinkIntCtx.pollEnableMask;
    uint32_t phyAddr = phyCfg->phyAddr;
    int32_t status = ENET_SOK;

    ENETTRACE_VAR(portId);
    if ((portNum >= EnetSoc_getMacPortMax(hPer->enetType, hPer->instId)) ||
        EnetMod_isOpen(hMacPort) ||
        (hPhy != NULL))
    {
        ENETTRACE_ERR("Port %u: Failed to open port (MAC port %s, PHY %s)\r\n",
                      portId,
                      EnetMod_isOpen(hMacPort) ? "open" : "closed",
                      (hPhy != NULL) ? "open" : "closed");
        status = ENET_EINVALIDPARAMS;
    }

    if (ENET_SOK == status)
    {
        if (phyAddr == ENETPHY_INVALID_PHYADDR)
        {
            status = Cpsw_openPortLinkNoPhy(hCpsw, macPort, macCfg, mii, linkCfg);
        }
        else
        {
            status = Cpsw_openPortLinkWithPhy(hCpsw, macPort, macCfg, phyCfg, mii, linkCfg);
        }
    }

    if (status == ENET_SOK)
    {
        portLinkState->isOpen  = true;
        portLinkState->phyAddr = phyAddr;

        if ((hCpsw->hMdio != NULL) &&
            ENETPHY_IS_ADDR_VALID(phyAddr))
        {
            portLinkState->isPollEnabled = ENET_MDIO_IS_PHY_ADDR_SET(pollEnMask, phyAddr);
        }
    }

    return status;
}

static void Cpsw_closePortLink(Cpsw_Handle hCpsw,
                               Enet_MacPort macPort)
{
    uint32_t portNum = ENET_MACPORT_NORM(macPort);
    EnetMod_Handle hMacPort = hCpsw->hMacPort[portNum];
    EnetPhy_Handle hPhy = hCpsw->hPhy[portNum];
    EnetPer_Handle hPer = (EnetPer_Handle)hCpsw;
    bool linked;

    if (portNum < EnetSoc_getMacPortMax(hPer->enetType, hPer->instId))
    {
        if (hPhy == NULL)
        {
            /* Link is always true for no-PHY mode */
            linked = true;
        }
        else
        {
            linked = hCpsw->portLinkState[portNum].isLinkUp;
        }

        /* Force link-down handling to undo settings done for this port link
         * i.e. ALE port state, MAC port disable, etc */
        if (linked)
        {
            Cpsw_handleLinkDown(hCpsw, macPort);
        }

        if (EnetMod_isOpen(hMacPort))
        {
            EnetMod_close(hMacPort);
        }

        if (hPhy != NULL)
        {
            EnetPhy_close(hPhy);
            hCpsw->hPhy[portNum] = NULL;
        }

        hCpsw->portLinkState[portNum].isOpen        = false;
        hCpsw->portLinkState[portNum].isPollEnabled = false;
        hCpsw->portLinkState[portNum].isTickEnabled = true;
    }
    else
    {
        /* Assert if port number is not correct */
        Enet_assert(false);
    }
}

static int32_t Cpsw_handleLinkUp(Cpsw_Handle hCpsw,
                                 Enet_MacPort macPort)
{
    EnetPer_Handle hPer = (EnetPer_Handle)hCpsw;
    uint32_t portNum = ENET_MACPORT_NORM(macPort);
    uint32_t portId = ENET_MACPORT_ID(macPort);
    EnetMod_Handle hMacPort = hCpsw->hMacPort[portNum];
    EnetPhy_Handle hPhy = hCpsw->hPhy[portNum];
    Enet_IoctlPrms prms;
    CpswAle_SetPortStateInArgs setPortStateInArgs;
    EnetPhy_LinkCfg phyLinkCfg;
    EnetMacPort_LinkCfg macLinkCfg;
    int32_t status;

    /* Assert if port number is not correct */
    Enet_assert(portNum < EnetSoc_getMacPortMax(hPer->enetType, hPer->instId),
                "Invalid Port Id: %u\r\n", portNum);

    ENETTRACE_VAR(portId);

    /* Get link parameters (speed/duplexity) from PHY state machine */
    status = EnetPhy_getLinkCfg(hPhy, &phyLinkCfg);
    ENETTRACE_ERR_IF(status != ENET_SOK, "Port %u: Failed to get PHY link config: %d\r\n", portId, status);

    /* Enable MAC port */
    if (status == ENET_SOK)
    {
        macLinkCfg.speed = (Enet_Speed)phyLinkCfg.speed;
        macLinkCfg.duplexity = (Enet_Duplexity)phyLinkCfg.duplexity;
        ENET_IOCTL_SET_IN_ARGS(&prms, &macLinkCfg);

        status = EnetMod_ioctl(hMacPort, CPSW_MACPORT_IOCTL_ENABLE, &prms);
        ENETTRACE_ERR_IF(status != ENET_SOK, "Port %u: Failed to enable MAC: %d\r\n", portId, status);
    }

    /* Set ALE port state to 'Forward' */
    if (status == ENET_SOK)
    {
        setPortStateInArgs.portNum   = CPSW_ALE_MACPORT_TO_ALEPORT(portNum);
        setPortStateInArgs.portState = CPSW_ALE_PORTSTATE_FORWARD;
        ENET_IOCTL_SET_IN_ARGS(&prms, &setPortStateInArgs);

        status = EnetMod_ioctl(hCpsw->hAle, CPSW_ALE_IOCTL_SET_PORT_STATE, &prms);
        ENETTRACE_ERR_IF(status != ENET_SOK,
                         "Port %u: Failed to set ALE port %u to forward state: %d\r\n",
                         portId, setPortStateInArgs.portNum, status);
    }

    ENETTRACE_INFO_IF(status == ENET_SOK,
                      "Port %d: Link up: %s %s\r\n",
                      portId,
                      Cpsw_gSpeedNames[phyLinkCfg.speed],
                      Cpsw_gDuplexNames[phyLinkCfg.duplexity]);

    return status;
}

static int32_t Cpsw_handleLinkDown(Cpsw_Handle hCpsw,
                                   Enet_MacPort macPort)
{
    EnetPer_Handle hPer = (EnetPer_Handle)hCpsw;
    uint32_t portNum = ENET_MACPORT_NORM(macPort);
    uint32_t portId = ENET_MACPORT_ID(macPort);
    EnetMod_Handle hMacPort = hCpsw->hMacPort[portNum];
    Enet_IoctlPrms prms;
    CpswAle_SetPortStateInArgs setPortStateInArgs;
    uint32_t alePortNum;
    uint32_t numEntries = 0U;
    int32_t status;

    /* Assert if port number is not correct */
    Enet_assert(portNum < EnetSoc_getMacPortMax(hPer->enetType, hPer->instId),
                "Invalid Port Id: %u\r\n", portNum);

    ENETTRACE_VAR(portId);
    ENETTRACE_INFO("Port %d: Link down\r\n", portId);

    ENET_IOCTL_SET_NO_ARGS(&prms);
    status = EnetMod_ioctl(hMacPort, CPSW_MACPORT_IOCTL_DISABLE, &prms);
    ENETTRACE_ERR_IF(status != ENET_SOK,
                     "Port %u: Failed to disable MAC port: %d\r\n", portId, status);

    if (status == ENET_SOK)
    {
        setPortStateInArgs.portNum   = CPSW_ALE_MACPORT_TO_ALEPORT(portNum);
        setPortStateInArgs.portState = CPSW_ALE_PORTSTATE_DISABLED;
        ENET_IOCTL_SET_IN_ARGS(&prms, &setPortStateInArgs);

        status = EnetMod_ioctl(hCpsw->hAle, CPSW_ALE_IOCTL_SET_PORT_STATE, &prms);
        ENETTRACE_ERR_IF(status != ENET_SOK,
                         "Port %u: Failed to set ALE port %u to disabled state: %d\r\n",
                         portId, setPortStateInArgs.portNum, status);
    }

    if (status == ENET_SOK)
    {
        alePortNum = CPSW_ALE_MACPORT_TO_ALEPORT(portNum);
        ENET_IOCTL_SET_INOUT_ARGS(&prms, &alePortNum, &numEntries);

        status = EnetMod_ioctl(hCpsw->hAle, CPSW_ALE_IOCTL_REMOVE_LEARNED_ENTRIES, &prms);
        ENETTRACE_ERR_IF(status != ENET_SOK,
                         "Port %u: Failed to delete learned ALE entries: %d\r\n", portId, status);
    }

    return status;
}

static int32_t Cpsw_getPortLinkCfg(Cpsw_Handle hCpsw,
                                   Enet_MacPort macPort,
                                   EnetMacPort_LinkCfg *linkCfg)
{
    EnetPer_Handle hPer = (EnetPer_Handle)hCpsw;
    EnetMac_LayerType enetLayer;
    EnetMac_SublayerType enetSublayer;
    EnetMacPort_Interface mii;
    CSL_Xge_cpsw_ss_sRegs *ssRegs = (CSL_Xge_cpsw_ss_sRegs *)hPer->virtAddr2;
    CSL_CPSW_SS_RGMIISTATUS rgmiiStatus;
    uint32_t portNum = ENET_MACPORT_NORM(macPort);
    uint32_t portId = ENET_MACPORT_ID(macPort);
    int32_t status;

    ENETTRACE_VAR(portId);
    status = EnetSoc_getMacPortMii(hPer->enetType, hPer->instId, macPort, &mii);
    ENETTRACE_ERR_IF(status != ENET_SOK, "Port %u: Failed to get ENET_CTRL: %d\r\n", portId, status);

    if (status == ENET_SOK)
    {
        enetLayer = mii.layerType;
        enetSublayer = mii.sublayerType;

        /* Link configuration is only available for RGMII */
        if ((enetLayer == ENET_MAC_LAYER_GMII) &&
            (enetSublayer == ENET_MAC_SUBLAYER_REDUCED))
        {
            CSL_CPSW_SS_getRGMIIStatus(ssRegs, portNum, &rgmiiStatus);

            /* Get link speed */
            if (rgmiiStatus.speed == 0U)
            {
                linkCfg->speed = ENET_SPEED_10MBIT;
            }
            else if (rgmiiStatus.speed == 1U)
            {
                linkCfg->speed = ENET_SPEED_100MBIT;
            }
            else if (rgmiiStatus.speed == 2U)
            {
                linkCfg->speed = ENET_SPEED_1GBIT;
            }
            else
            {
                ENETTRACE_ERR("Port %u: invalid RGMII speed: %d\r\n", portId, rgmiiStatus.speed);
                Enet_assert(false, "Invalid RGMII speed value: %d\r\n", rgmiiStatus.speed);
            }

            /* Get link duplexity */
            if (rgmiiStatus.fullDuplex == 0U)
            {
                linkCfg->duplexity = ENET_DUPLEX_HALF;
            }
            else
            {
                linkCfg->duplexity = ENET_DUPLEX_FULL;
            }
        }
        else
        {
            ENETTRACE_ERR("Port %u: Link config not available for layer %u sublayer %u\r\n",
                          portId, enetLayer, enetSublayer);
            status = ENET_ENOTSUPPORTED;
        }
    }
    else
    {
        ENETTRACE_ERR("Port %u: Failed to get ENET_CTRL %u: %d\r\n", portId, status);
    }

    return status;
}

static int32_t Cpsw_validateTxShortIpgCfg(const Cpsw_Handle hCpsw,
                                          const Cpsw_SetTxShortIpgCfgInArgs *inArgs)
{
    EnetPer_Handle hPer = (EnetPer_Handle)hCpsw;
    const CpswMacPort_PortTxShortIpgCfg *portShortIpgCfg;
    uint32_t macPortList = 0U;
    uint32_t portNum;
    uint32_t portId;
    uint32_t i;
    int32_t status = ENET_SOK;

    if (inArgs->configureGapThresh &&
        (inArgs->ipgTriggerThreshBlkCnt > CSL_XGE_CPSW_GAP_THRESH_REG_GAP_THRESH_MAX))
    {
        ENETTRACE_ERR("IPG trigger threshold block count %u exceeded max\r\n",
                      inArgs->ipgTriggerThreshBlkCnt);
        status = ENET_EINVALIDPARAMS;
    }

    if (ENET_SOK == status)
    {
        if (inArgs->numMacPorts > ENET_ARRAYSIZE(inArgs->portShortIpgCfg))
        {
            ENETTRACE_ERR("Invalid number of MAC ports %u exceeded %u\r\n",
                          inArgs->numMacPorts, ENET_ARRAYSIZE(inArgs->portShortIpgCfg));
            status = ENET_EINVALIDPARAMS;
        }
    }

    if (ENET_SOK == status)
    {
        for (i = 0U; i < inArgs->numMacPorts; i++)
        {
            portShortIpgCfg = &inArgs->portShortIpgCfg[i];
            portNum = ENET_MACPORT_NORM(portShortIpgCfg->macPort);
            ENETTRACE_VAR(portId);
            portId = ENET_MACPORT_ID(portShortIpgCfg->macPort);

            /* Assert if port number is not correct */
            Enet_assert(portNum < EnetSoc_getMacPortMax(hPer->enetType, hPer->instId),
                        "Invalid Port Id: %u\r\n", portNum);

            if ((portNum >= hCpsw->macPortNum) ||
                !EnetMod_isOpen(hCpsw->hMacPort[portNum]))
            {
                ENETTRACE_ERR("Invalid MAC port %u\r\n", portId);
                status = ENET_EINVALIDPARAMS;
                break;
            }

            if (ENET_IS_BIT_SET(macPortList, portNum))
            {
                /* inArgs->portIpgCfg entries should have list of unique MAC ports.
                 * Flag error as previously seen macPort in inArgs->portIpgCfg
                 * is encountered again */
                ENETTRACE_ERR("Multiple occurence of MAC port %u\r\n", portId);
                status = ENET_EINVALIDPARAMS;
                break;
            }

            if ((portShortIpgCfg->shortIpgCfg.txShortGapEn == false) &&
                (portShortIpgCfg->shortIpgCfg.txShortGapLimitEn == true))
            {
                ENETTRACE_ERR("MAC port %u's short gap limit can be enabled only if short gap is enabled\r\n", portId);
                status = ENET_EINVALIDPARAMS;
                break;
            }

            macPortList |= ENET_BIT(portNum);
        }
    }

    return status;
}

static int32_t Cpsw_setTxShortIpgCfg(const Cpsw_Handle hCpsw,
                                     const Cpsw_SetTxShortIpgCfgInArgs *inArgs)
{
    EnetPer_Handle hPer = (EnetPer_Handle)hCpsw;
    CSL_Xge_cpswRegs *regs = (CSL_Xge_cpswRegs *)hPer->virtAddr;
    const CpswMacPort_PortTxShortIpgCfg *portShortIpgCfg;
    Enet_IoctlPrms prms;
    uint32_t portNum;
    uint32_t portId;
    uint32_t i;
    int32_t status;

    status = Cpsw_validateTxShortIpgCfg(hCpsw, inArgs);
    if (status == ENET_SOK)
    {
        if (inArgs->configureGapThresh)
        {
            CSL_CPSW_setGapThreshold(regs, inArgs->ipgTriggerThreshBlkCnt);
        }

        for (i = 0U; i < inArgs->numMacPorts; i++)
        {
            portShortIpgCfg = &inArgs->portShortIpgCfg[i];
            portNum = ENET_MACPORT_NORM(portShortIpgCfg->macPort);
            portId = ENET_MACPORT_ID(portShortIpgCfg->macPort);
            ENETTRACE_VAR(portId);

            /* Assert if port number is not correct */
            Enet_assert(portNum < EnetSoc_getMacPortMax(hPer->enetType, hPer->instId),
                        "Invalid Port Id: %u\r\n", portNum);
            ENET_IOCTL_SET_IN_ARGS(&prms, portShortIpgCfg);

            status = EnetMod_ioctl(hCpsw->hMacPort[portNum], CPSW_MACPORT_IOCTL_SET_SHORT_IPG, &prms);
            if (status != ENET_SOK)
            {
                ENETTRACE_ERR("Failed to set MAC port %u's short IPG: %d\r\n", portId, status);
                break;
            }
        }
    }
    else
    {
        ENETTRACE_ERR("Failed to validated short IPG config: %d\r\n", status);
    }

    return status;
}

static int32_t Cpsw_getTxShortIpgCfg(const Cpsw_Handle hCpsw,
                                     Cpsw_TxShortIpgCfg *shortIpgCfg)
{
    EnetPer_Handle hPer = (EnetPer_Handle)hCpsw;
    CSL_Xge_cpswRegs *regs = (CSL_Xge_cpswRegs *)hPer->virtAddr;
    EnetMod_Handle hMacPort;
    Enet_IoctlPrms prms;
    uint32_t i;
    int32_t status = ENET_SOK;

    shortIpgCfg->ipgTriggerThreshBlkCnt = CSL_CPSW_getGapThreshold(regs);
    shortIpgCfg->numMacPorts = 0U;

    for (i = 0U; i < hCpsw->macPortNum; i++)
    {
        hMacPort = hCpsw->hMacPort[i];

        if (EnetMod_isOpen(hMacPort))
        {
            EnetMacPort_GenericInArgs inArgs;
            CpswMacPort_PortTxShortIpgCfg *portIpgCfg;

            inArgs.macPort = ENET_MACPORT_DENORM(i);
            portIpgCfg = &shortIpgCfg->portShortIpgCfg[shortIpgCfg->numMacPorts];
            portIpgCfg->macPort = inArgs.macPort;
            ENET_IOCTL_SET_INOUT_ARGS(&prms, &inArgs, &portIpgCfg->shortIpgCfg);

            status = EnetMod_ioctl(hMacPort, CPSW_MACPORT_IOCTL_GET_SHORT_IPG, &prms);
            if (status != ENET_SOK)
            {
                ENETTRACE_ERR("Port %u: Failed to get short IPG: %d\r\n",
                              ENET_MACPORT_ID(inArgs.macPort), status);
                break;
            }

            shortIpgCfg->numMacPorts++;
        }
    }

    return status;
}



