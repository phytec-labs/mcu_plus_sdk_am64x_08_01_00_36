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
 * \file  cpsw_macport.c
 *
 * \brief This file contains the implementation of the CPSW MAC port module.
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdint.h>
#include <stdarg.h>
#include <csl_cpswitch.h>
#include <enet_cfg.h>
#include <include/core/enet_utils.h>
#include <include/core/enet_soc.h>
#include <include/mod/cpsw_macport.h>
#include <priv/core/enet_trace_priv.h>
#include <priv/mod/cpsw_macport_priv.h>
#include <priv/mod/cpsw_clks.h>
#include "cpsw_macport_intervlan.h"

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* Supported AM64x version */
#define CPSW_MACPORT_VER_REVMAJ_AM64X        (0x00000001U)
#define CPSW_MACPORT_VER_REVMIN_AM64X        (0x00000003U)
#define CPSW_MACPORT_VER_REVRTL_AM64X        (0x00000001U)
#define CPSW_MACPORT_VER_ID_AM64X            (0x00006BA8U)

/* Supported AM273x version */
#define CPSW_MACPORT_VER_REVMAJ_AM273X         (0x00000001U)
#define CPSW_MACPORT_VER_REVMIN_AM273X         (0x00000002U)
#define CPSW_MACPORT_VER_REVRTL_AM273X         (0x00000000U)
#define CPSW_MACPORT_VER_ID_AM273X             (0x00006B90U)

/* Supported AM263x version */
#define CPSW_MACPORT_VER_REVMAJ_AM263X         (0x00000001U)
#define CPSW_MACPORT_VER_REVMIN_AM263X         (0x00000002U)
#define CPSW_MACPORT_VER_REVRTL_AM263X         (0x00000000U)
#define CPSW_MACPORT_VER_ID_AM263X             (0x00006B90U)

/* Supported AWR294x version */
#define CPSW_MACPORT_VER_REVMAJ_AWR294X         (0x00000001U)
#define CPSW_MACPORT_VER_REVMIN_AWR294X         (0x00000002U)
#define CPSW_MACPORT_VER_REVRTL_AWR294X         (0x00000000U)
#define CPSW_MACPORT_VER_ID_AWR294X             (0x00006B90U)

/*! \brief MAC port register start offset. */
#define CPSW_MACPORT_START_REG_OFFSET         (offsetof(CSL_Xge_cpswEnetportRegs, PN_CONTROL_REG))

/*! \brief MAC port register end offset. */
#define CPSW_MACPORT_END_REG_OFFSET           (offsetof(CSL_Xge_cpswEnetportRegs, PN_INTERVLAN_OPX_D_REG))

/*! \brief Default value used for MAC port RX MTU (MRU). */
#define CPSW_MACPORT_RX_MTU_DEFAULT           (1518U)

/*! \brief Rate limiting count to transfer rate divider. */
#define CPSW_MACPORT_RATELIM_DIV              (32768U)

/*!
 * \brief Priority Escalation value for switch scheduler
 *
 * When a port is in escalate priority, this is the number of higher priority
 * packets sent before the next lower priority is allowed to send a packet.
 * Escalate priority allows lower priority packets to be sent at a fixed rate
 * relative to the next higher priority.  The min value of esc_pri_ld_val = 2.
 */
#define CPSW_MACPORT_ESC_PRI_LD_VAL           (2U)

/*! \brief PTP sequence ID offset minimum value. */
#define CPSW_MACPORT_PTP_SEQID_OFFSET_MIN_VAL (0x6U)

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

#if ENET_CFG_IS_ON(DEV_ERROR)
static int32_t CpswMacPort_isSupported(CSL_Xge_cpswRegs *regs);

static int32_t CpswMacPort_isMiiSupported(EnetMod_Handle hMod,
                                          const EnetMacPort_Interface *mii);
#endif

static int32_t CpswMacPort_checkSocCfg(Enet_Type enetType,
                                       uint32_t instId,
                                       Enet_MacPort macPort,
                                       const EnetMacPort_Interface *mii);

static void CpswMacPort_reset(CSL_Xge_cpswRegs *regs,
                              Enet_MacPort macPort);

static void CpswMacPort_setSwitchTxSched(CSL_Xge_cpswRegs *regs,
                                         Enet_MacPort macPort,
                                         EnetPort_EgressPriorityType priority);

static int32_t CpswMacPort_enableLoopback(CSL_Xge_cpswRegs *regs,
                                          Enet_MacPort macPort,
                                          const EnetMacPort_Interface *mii);

static int32_t CpswMacPort_setInterface(CSL_Xge_cpswRegs *regs,
                                        Enet_MacPort macPort,
                                        const EnetMacPort_Interface *mii);

static bool CpswMacPort_isPortEnabled(CSL_Xge_cpswRegs *regs,
                                      Enet_MacPort macPort);

static int32_t CpswMacPort_enablePort(CSL_Xge_cpswRegs *regs,
                                      Enet_MacPort macPort,
                                      const EnetMacPort_Interface *mii,
                                      const EnetMacPort_LinkCfg *linkCfg);

static void CpswMacPort_disablePort(CSL_Xge_cpswRegs *regs,
                                    Enet_MacPort macPort);

static void CpswMacPort_printRegs(CSL_Xge_cpswRegs *regs,
                                  Enet_MacPort macPort);

static int32_t CpswMacPort_setDscpPriority(CSL_Xge_cpswRegs *regs,
                                           Enet_MacPort macPort,
                                           EnetPort_DscpPriorityMap *dscpPriority);

static void CpswMacPort_getDscpPriority(CSL_Xge_cpswRegs *regs,
                                        Enet_MacPort macPort,
                                        EnetPort_DscpPriorityMap *dscpPriority);

static int32_t CpswMacPort_setRxPriority(CSL_Xge_cpswRegs *regs,
                                         Enet_MacPort macPort,
                                         EnetPort_PriorityMap *rxPriority);

static void CpswMacPort_getRxPriority(CSL_Xge_cpswRegs *regs,
                                      Enet_MacPort macPort,
                                      EnetPort_PriorityMap *rxPriority);

static int32_t CpswMacPort_setTxPriority(CSL_Xge_cpswRegs *regs,
                                         Enet_MacPort macPort,
                                         EnetPort_PriorityMap *txPriority);

static void CpswMacPort_getTxPriority(CSL_Xge_cpswRegs *regs,
                                      Enet_MacPort macPort,
                                      EnetPort_PriorityMap *txPriority);

#if ENET_CFG_IS_ON(CPSW_MACPORT_TRAFFIC_SHAPING)
static uint64_t CpswMacPort_mapBwToCnt(uint64_t rateInBps,
                                       uint32_t cppiClkFreqHz);

static uint64_t CpswMacPort_mapCntToBw(uint32_t cntVal,
                                       uint32_t cppiClkFreqHz);

static int32_t CpswMacPort_setTrafficShaping(CSL_Xge_cpswRegs *regs,
                                             Enet_MacPort macPort,
                                             const EnetPort_TrafficShapingCfg *cfg,
                                             uint32_t cppiClkFreqHz);

static void CpswMacPort_disableTrafficShaping(CSL_Xge_cpswRegs *regs,
                                              Enet_MacPort macPort);

static int32_t CpswMacPort_getTrafficShaping(CSL_Xge_cpswRegs *regs,
                                             Enet_MacPort macPort,
                                             EnetPort_TrafficShapingCfg *cfg,
                                             uint32_t cppiClkFreqHz);
#endif

static void CpswMacPort_getMaxLen(CSL_Xge_cpswRegs *regs,
                                  Enet_MacPort macPort,
                                  EnetPort_MaxLen *maxLen);

static void CpswMacPort_getFifoStats(CSL_Xge_cpswRegs *regs,
                                     Enet_MacPort macPort,
                                     CpswMacPort_FifoStats *stats);

static void CpswMacPort_getLinkCfg(CSL_Xge_cpswRegs *regs,
                                   Enet_MacPort macPort,
                                   EnetMacPort_LinkCfg *linkCfg);

static int32_t CpswMacPort_enableCptsPortEvents(CSL_Xge_cpswRegs *regs,
                                                Enet_MacPort macPort,
                                                CpswMacPort_TsEventCfg *tsEventCfg);

static void CpswMacPort_disableCptsPortEvents(CSL_Xge_cpswRegs *regs,
                                              Enet_MacPort macPort);

static void CpswMacPort_setShortIpgCfg(CSL_Xge_cpswRegs *regs,
                                       Enet_MacPort macPort,
                                       const CpswMacPort_TxShortIpgCfg *shortIpgCfg);

static void CpswMacPort_getShortIpgCfg(CSL_Xge_cpswRegs *regs,
                                       Enet_MacPort macPort,
                                       CpswMacPort_TxShortIpgCfg *shortIpgCfg);

#if ENET_CFG_IS_ON(CPSW_MACPORT_SGMII)
#if ENET_CFG_IS_ON(DEV_ERROR)
static int32_t CpswMacPort_isSgmiiSupported(CSL_CpsgmiiRegs *sgmiiRegs,
                                            Enet_MacPort macPort);
#endif

static void CpswMacPort_resetSgmiiPort(CSL_CpsgmiiRegs *sgmiiRegs,
                                       Enet_MacPort macPort);

static int32_t CpswMacPort_enableSgmiiLoopback(CSL_Xge_cpswRegs *regs,
                                               CSL_CpsgmiiRegs *sgmiiRegs,
                                               Enet_MacPort macPort);

static int32_t CpswMacPort_setSgmiiInterface(CSL_Xge_cpswRegs *regs,
                                             CSL_CpsgmiiRegs *sgmiiRegs,
                                             Enet_MacPort macPort,
                                             EnetMac_SgmiiMode sgmiiMode,
                                             const EnetMacPort_LinkCfg *linkCfg);

static int32_t CpswMacPort_configSgmii(CSL_CpsgmiiRegs *sgmiiRegs,
                                       EnetMac_SgmiiMode sgmiiMode,
                                       Enet_MacPort macPort,
                                       const EnetMacPort_LinkCfg *linkCfg);

static void CpswMacPort_mapSgmiiLinkCfg(CSL_SGMII_ADVABILITY *sgmiiAdvAbility,
                                        const EnetMacPort_LinkCfg *linkCfg);

static bool CpswMacPort_getSgmiiStatus(CSL_CpsgmiiRegs *sgmiiRegs,
                                       Enet_MacPort macPort);

static int32_t CpswMacPort_checkSgmiiAutoNegStatus(CSL_CpsgmiiRegs *sgmiiRegs,
                                                   Enet_MacPort macPort);

static int32_t CpswMacPort_enableSgmiiPort(CSL_Xge_cpswRegs *regs,
                                           CSL_CpsgmiiRegs *sgmiiRegs,
                                           Enet_MacPort macPort,
                                           const EnetMacPort_Interface *mii,
                                           const EnetMacPort_LinkCfg *linkCfg);
#endif

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

#if ENET_CFG_IS_ON(DEV_ERROR)
/*! \brief CPSW MAC port versions supported by this driver. */
static CSL_CPSW_VERSION CpswMacPort_gSupportedVer[] =
{
    {   /* AM64x */
        .majorVer = CPSW_MACPORT_VER_REVMAJ_AM64X,
        .minorVer = CPSW_MACPORT_VER_REVMIN_AM64X,
        .rtlVer   = CPSW_MACPORT_VER_REVRTL_AM64X,
        .id       = CPSW_MACPORT_VER_ID_AM64X,
    },
    {   /* AM273X */
        .majorVer = CPSW_MACPORT_VER_REVMAJ_AM273X,
        .minorVer = CPSW_MACPORT_VER_REVMIN_AM273X,
        .rtlVer   = CPSW_MACPORT_VER_REVRTL_AM273X,
        .id       = CPSW_MACPORT_VER_ID_AM273X,
    },
	{   /* AM263X */
        .majorVer = CPSW_MACPORT_VER_REVMAJ_AM263X,
        .minorVer = CPSW_MACPORT_VER_REVMIN_AM263X,
        .rtlVer   = CPSW_MACPORT_VER_REVRTL_AM263X,
        .id       = CPSW_MACPORT_VER_ID_AM263X,
    },
    {   /* AWR294X */
        .majorVer = CPSW_MACPORT_VER_REVMAJ_AWR294X,
        .minorVer = CPSW_MACPORT_VER_REVMIN_AWR294X,
        .rtlVer   = CPSW_MACPORT_VER_REVRTL_AWR294X,
        .id       = CPSW_MACPORT_VER_ID_AWR294X,
    },
};

/* Public MAC port IOCTL validation data. */
static Enet_IoctlValidate gCpswMacPort_ioctlValidate[] =
{
    ENET_IOCTL_VALID_PRMS(CPSW_MACPORT_IOCTL_GET_FIFO_STATS,
                          sizeof(EnetMacPort_GenericInArgs),
                          sizeof(CpswMacPort_FifoStats)),

    ENET_IOCTL_VALID_PRMS(CPSW_MACPORT_IOCTL_ENABLE_CPTS_EVENT,
                          sizeof(CpswMacPort_EnableTsEventInArgs),
                          0U),

    ENET_IOCTL_VALID_PRMS(CPSW_MACPORT_IOCTL_DISABLE_CPTS_EVENT,
                          sizeof(CpswMacPort_EnableTsEventInArgs),
                          0U),
};

/* Private MAC port IOCTL validation data. */
static Enet_IoctlValidate gCpswMacPort_privIoctlValidate[] =
{
    ENET_IOCTL_VALID_PRMS(CPSW_MACPORT_IOCTL_ENABLE,
                          sizeof(EnetMacPort_LinkCfg),
                          0U),

    ENET_IOCTL_VALID_PRMS(CPSW_MACPORT_IOCTL_DISABLE,
                          0U,
                          0U),

    ENET_IOCTL_VALID_PRMS(CPSW_MACPORT_IOCTL_SET_INTERVLAN_ROUTE,
                          sizeof(CpswMacPort_InterVlanRoutingCfg),
                          sizeof(CpswMacPort_InterVlanRouteId)),

    ENET_IOCTL_VALID_PRMS(CPSW_MACPORT_IOCTL_SET_SPECIFIC_INTERVLAN_ROUTE,
                          sizeof(CpswMacPort_SetSpecificInterVlanRouteInArgs),
                          0U),

    ENET_IOCTL_VALID_PRMS(CPSW_MACPORT_IOCTL_GET_INTERVLAN_ROUTE,
                          sizeof(CpswMacPort_InterVlanRouteId),
                          sizeof(CpswMacPort_InterVlanRoutingCfg)),

    ENET_IOCTL_VALID_PRMS(CPSW_MACPORT_IOCTL_DELETE_INTERVLAN_ROUTE,
                          sizeof(CpswMacPort_InterVlanRoutingCfg),
                          sizeof(CpswMacPort_InterVlanRouteId)),

    ENET_IOCTL_VALID_PRMS(CPSW_MACPORT_IOCTL_GET_INTERVLAN_FREEROUTES,
                          0U,
                          sizeof(CpswMacPort_InterVlanFreeRouteInfo)),

    ENET_IOCTL_VALID_PRMS(CPSW_MACPORT_IOCTL_FIND_INTERVLAN_ROUTE,
                          sizeof(CpswMacPort_InterVlanRoutingCfg),
                          sizeof(CpswMacPort_InterVlanRouteId)),

    ENET_IOCTL_VALID_PRMS(CPSW_MACPORT_IOCTL_IS_INTERVLAN_ROUTE_FREE,
                          sizeof(CpswMacPort_InterVlanRouteId),
                          sizeof(bool)),

    ENET_IOCTL_VALID_PRMS(CPSW_MACPORT_IOCTL_SET_SHORT_IPG,
                          sizeof(CpswMacPort_PortTxShortIpgCfg),
                          0U),

    ENET_IOCTL_VALID_PRMS(CPSW_MACPORT_IOCTL_GET_SHORT_IPG,
                          sizeof(EnetMacPort_GenericInArgs),
                          sizeof(CpswMacPort_TxShortIpgCfg)),

    ENET_IOCTL_VALID_PRMS(CPSW_MACPORT_IOCTL_GET_SGMII_AUTONEG_LINK_STATUS,
                          sizeof(EnetMacPort_GenericInArgs),
                          sizeof(bool)),

    ENET_IOCTL_VALID_PRMS(CPSW_MACPORT_IOCTL_GET_SGMII_LINK_STATUS,
                          sizeof(EnetMacPort_GenericInArgs),
                          sizeof(bool)),
};
#endif

#if ENET_CFG_IS_ON(CPSW_MACPORT_SGMII)
#if (ENET_CFG_TRACE_LEVEL >= ENET_CFG_TRACE_LEVEL_INFO)
static const char *CpswMacPort_gSgmiiSpeedNames[] =
{
    [CSL_SGMII_10_MBPS]   = "10-Mbps",
    [CSL_SGMII_100_MBPS]  = "100-Mbps",
    [CSL_SGMII_1000_MBPS] = "1-Gbps",
};

static const char *CpswMacPort_gSgmiiDuplexNames[] =
{
    [CSL_SGMII_HALF_DUPLEX] = "Half-Duplex",
    [CSL_SGMII_FULL_DUPLEX] = "Full-Duplex",
};
#endif
#endif

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

void CpswMacPort_initCfg(CpswMacPort_Cfg *macPortCfg)
{
    macPortCfg->loopbackEn        = false;
    macPortCfg->crcType           = ENET_CRC_ETHERNET;
    macPortCfg->rxMtu             = CPSW_MACPORT_RX_MTU_DEFAULT;
    macPortCfg->passPriorityTaggedUnchanged = false;
    macPortCfg->vlanCfg.portPri   = 0U;
    macPortCfg->vlanCfg.portCfi   = 0U;
    macPortCfg->vlanCfg.portVID   = 0U;
    macPortCfg->txPriorityType    = ENET_EGRESS_PRI_TYPE_FIXED;
    macPortCfg->sgmiiMode         = ENET_MAC_SGMIIMODE_INVALID;
}

int32_t CpswMacPort_open(EnetMod_Handle hMod,
                         Enet_Type enetType,
                         uint32_t instId,
                         const void *cfg,
                         uint32_t cfgSize)
{
    CpswMacPort_Handle hPort = (CpswMacPort_Handle)hMod;
    const CpswMacPort_ModCfg *macModCfg = (const CpswMacPort_ModCfg *)cfg;
    const CpswMacPort_Cfg *macCfg = &macModCfg->macCfg;
    const EnetMacPort_Interface *mii = &macModCfg->mii;
    CSL_Xge_cpswRegs *regs = (CSL_Xge_cpswRegs *)hMod->virtAddr;
#if ENET_CFG_IS_ON(CPSW_MACPORT_SGMII)
    CSL_CpsgmiiRegs *sgmiiRegs = (CSL_CpsgmiiRegs *)hMod->virtAddr2;
#endif
    Enet_MacPort macPort = hPort->macPort;
    uint32_t portNum = ENET_MACPORT_NORM(macPort);
    uint32_t portId = ENET_MACPORT_ID(macPort);
    int32_t status = ENET_SOK;

    ENETTRACE_VAR(portId);
    Enet_devAssert(cfgSize == sizeof(CpswMacPort_ModCfg),
                   "Invalid MAC port config params size %u (expected %u)\n",
                   cfgSize, sizeof(CpswMacPort_ModCfg));

    Enet_devAssert(regs != NULL, "MAC %u: regs address is not valid\n", portId);

#if ENET_CFG_IS_ON(CPSW_MACPORT_SGMII)
    Enet_devAssert(sgmiiRegs != NULL, "MAC %u: SGMII regs address is not valid\n", portId);
#endif

    /* Check supported MAC port module versions */
#if ENET_CFG_IS_ON(DEV_ERROR)
    status = CpswMacPort_isSupported(regs);
    Enet_devAssert(status == ENET_SOK, "MAC %u: version is not supported\n", portId);

#if ENET_CFG_IS_ON(CPSW_MACPORT_SGMII)
    if (ENET_FEAT_IS_EN(hMod->features, CPSW_MACPORT_FEATURE_SGMII))
    {
        status = CpswMacPort_isSgmiiSupported(sgmiiRegs, macPort);
        Enet_devAssert(status == ENET_SOK, "MAC %u: SGMII version is not supported\n", portId);
    }
#endif

    /* Check if MII is supported */
    status = CpswMacPort_isMiiSupported(hMod, mii);
    Enet_devAssert(status == ENET_SOK, "MAC %u: MII not supported\n", portId);
#endif

    /* Save peripheral info to use it later to query SoC parameters */
    hPort->enetType = enetType;
    hPort->instId = instId;

    /* Check if SoC settings (if any) matches the requested MII config */
    status = CpswMacPort_checkSocCfg(enetType, instId, macPort, mii);
    ENETTRACE_ERR_IF(status != ENET_SOK, "MAC %u: MII mismatch with SoC settings\n", portId);

    /* Soft-reset the Ethernet MAC logic and SGMII port */
    if (status == ENET_SOK)
    {
        CpswMacPort_reset(regs, macPort);

#if ENET_CFG_IS_ON(CPSW_MACPORT_SGMII)
        if (ENET_FEAT_IS_EN(hMod->features, CPSW_MACPORT_FEATURE_SGMII))
        {
            if (EnetMacPort_isSgmii(mii) ||
                EnetMacPort_isQsgmii(mii))
            {
                CpswMacPort_resetSgmiiPort(sgmiiRegs, macPort);
            }
        }
#endif
    }

    /* Set CRC, MRU and port VLAN config */
    if (status == ENET_SOK)
    {
        if (macCfg->crcType == ENET_CRC_ETHERNET)
        {
            CSL_CPGMAC_SL_disableCastagnoliCRC(regs, portNum);
        }
        else
        {
            CSL_CPGMAC_SL_enableCastagnoliCRC(regs, portNum);
        }

        CSL_CPGMAC_SL_setRxMaxLen(regs, portNum, macCfg->rxMtu);

        if (macCfg->passPriorityTaggedUnchanged)
        {
            CSL_CPSW_enablePortPassPriTag(regs, portNum + 1U);
        }
        else
        {
            CSL_CPSW_disablePortPassPriTag(regs, portNum + 1U);
        }

        CSL_CPSW_setPortVlanReg(regs, portNum + 1U,
                                macCfg->vlanCfg.portVID,
                                macCfg->vlanCfg.portCfi,
                                macCfg->vlanCfg.portPri);

        CpswMacPort_setSwitchTxSched(regs, macPort, macCfg->txPriorityType);
    }

    /* Set normal mode or loopback mode */
    if (status == ENET_SOK)
    {
        if (macCfg->loopbackEn)
        {
            if (EnetMacPort_isSgmii(mii))
            {
#if ENET_CFG_IS_ON(CPSW_MACPORT_SGMII)
                status = CpswMacPort_enableSgmiiLoopback(regs, sgmiiRegs, macPort);
#else
                status = ENET_ENOTSUPPORTED;
#endif
                ENETTRACE_ERR_IF(status != ENET_SOK,
                                 "MAC %u: failed to set SGMII loopback mode: %d\n", portId, status);
            }
            else
            {
                status = CpswMacPort_enableLoopback(regs, macPort, mii);
                ENETTRACE_ERR_IF(status != ENET_SOK,
                                 "MAC %u: failed to set loopback mode: %d\n", portId, status);
            }
        }
        else
        {
            CSL_CPGMAC_SL_disableLoopback(regs, portNum);
        }
    }

    /* Configure MII interface (except for SGMII loopback mode) */
    if (status == ENET_SOK)
    {
        if (EnetMacPort_isSgmii(mii) ||
            EnetMacPort_isQsgmii(mii))
        {
#if ENET_CFG_IS_ON(CPSW_MACPORT_SGMII)
            /* SGMII loopback is digital loopback (before the SERDES) from the CPSGMII transmit
             * to the CPSGMII receive. The SGMII modes are complementary to loopback so
             * configuring MAC interface in loopback mode will cause conflicting configuration */
            if (!macCfg->loopbackEn)
            {
                status = CpswMacPort_setSgmiiInterface(regs, sgmiiRegs,
                                                       macPort,
                                                       macCfg->sgmiiMode,
                                                       &macModCfg->linkCfg);
            }
#else
            status = ENET_ENOTSUPPORTED;
#endif
            ENETTRACE_ERR_IF(status != ENET_SOK,
                             "MAC %u: failed to set Q/SGMII interface: %d\n", portId, status);
        }
        else
        {
            status = CpswMacPort_setInterface(regs, macPort, mii);
            ENETTRACE_ERR_IF(status != ENET_SOK,
                             "MAC %u: failed to set interface: %d\n", portId, status);
        }
    }

#if ENET_CFG_IS_ON(CPSW_MACPORT_INTERVLAN)
    /* Open InterVLAN (clear VLAN routes) */
    if (status == ENET_SOK)
    {
        if (ENET_FEAT_IS_EN(hMod->features, CPSW_MACPORT_FEATURE_INTERVLAN))
        {
            CpswMacPort_openInterVlan(hMod);
        }
    }
#endif

    return status;
}

int32_t CpswMacPort_rejoin(EnetMod_Handle hMod,
                           Enet_Type enetType,
                           uint32_t instId)
{
    CpswMacPort_Handle hPort = (CpswMacPort_Handle)hMod;

    /* Save peripheral info to use it later to query SoC parameters */
    hPort->enetType = enetType;
    hPort->instId = instId;

    return ENET_SOK;
}

void CpswMacPort_close(EnetMod_Handle hMod)
{
    CpswMacPort_Handle hPort = (CpswMacPort_Handle)hMod;
    CSL_Xge_cpswRegs *regs = (CSL_Xge_cpswRegs *)hMod->virtAddr;
    bool enabled;

    enabled = CpswMacPort_isPortEnabled(regs, hPort->macPort);
    if (enabled)
    {
        CpswMacPort_disablePort(regs, hPort->macPort);
    }
}

int32_t CpswMacPort_ioctl(EnetMod_Handle hMod,
                          uint32_t cmd,
                          Enet_IoctlPrms *prms)
{
    CpswMacPort_Handle hPort = (CpswMacPort_Handle)hMod;
    CSL_Xge_cpswRegs *regs = (CSL_Xge_cpswRegs *)hMod->virtAddr;
#if ENET_CFG_IS_ON(CPSW_MACPORT_SGMII)
    CSL_CpsgmiiRegs *sgmiiRegs = (CSL_CpsgmiiRegs *)hMod->virtAddr2;
#endif
    Enet_MacPort macPort = hPort->macPort;
    uint32_t portId = ENET_MACPORT_ID(macPort);
    int32_t status = ENET_SOK;

    ENETTRACE_VAR(portId);
#if ENET_CFG_IS_ON(DEV_ERROR)
    /* Validate CPSW MAC port IOCTL parameters */
    if (ENET_IOCTL_GET_PER(cmd) == ENET_IOCTL_PER_CPSW)
    {
        if (ENET_IOCTL_GET_TYPE(cmd) == ENET_IOCTL_TYPE_PUBLIC)
        {
            status = Enet_validateIoctl(cmd, prms,
                                        gCpswMacPort_ioctlValidate,
                                        ENET_ARRAYSIZE(gCpswMacPort_ioctlValidate));
        }
        else
        {
            status = Enet_validateIoctl(cmd, prms,
                                        gCpswMacPort_privIoctlValidate,
                                        ENET_ARRAYSIZE(gCpswMacPort_privIoctlValidate));
        }

        ENETTRACE_ERR_IF(status != ENET_SOK, "MAC %u: IOCTL 0x%08x params are not valid\n", portId, cmd);
    }
#endif

    if (status == ENET_SOK)
    {
        switch (cmd)
        {
            case ENET_MACPORT_IOCTL_GET_VERSION:
            {
                EnetMacPort_GenericInArgs *inArgs = (EnetMacPort_GenericInArgs *)prms->inArgs;
                Enet_Version *version = (Enet_Version *)prms->outArgs;
                CSL_CPSW_VERSION ver;

                Enet_devAssert(macPort == inArgs->macPort,
                               "MAC %u: Port mismatch %u\n", portId, inArgs->macPort);

                /* Report CPSW Control version as ours */
                CSL_CPSW_getCpswVersionInfo(regs, &ver);
                version->maj = ver.majorVer;
                version->min = ver.minorVer;
                version->rtl = ver.rtlVer;
                version->id  = ver.id;
                version->other1 = ENET_VERSION_NONE;
                version->other2 = ENET_VERSION_NONE;
            }
            break;

            case ENET_MACPORT_IOCTL_PRINT_REGS:
            {
                EnetMacPort_GenericInArgs *inArgs = (EnetMacPort_GenericInArgs *)prms->inArgs;

                Enet_devAssert(macPort == inArgs->macPort,
                               "MAC %u: Port mismatch %u\n", portId, inArgs->macPort);

                CpswMacPort_printRegs(regs, macPort);
            }
            break;

            case ENET_MACPORT_IOCTL_SET_INGRESS_DSCP_PRI_MAP:
            {
                EnetMacPort_SetIngressDscpPriorityMapInArgs *inArgs =
                    (EnetMacPort_SetIngressDscpPriorityMapInArgs *)prms->inArgs;

                Enet_devAssert(macPort == inArgs->macPort,
                               "MAC %u: Port mismatch %u\n", portId, inArgs->macPort);

                status = CpswMacPort_setDscpPriority(regs, macPort, &inArgs->dscpPriorityMap);
                ENETTRACE_ERR_IF(status != ENET_SOK,
                                 "MAC %u: Failed to set DSCP priority: %d\n", portId, status);
            }
            break;

            case ENET_MACPORT_IOCTL_GET_INGRESS_DSCP_PRI_MAP:
            {
                EnetMacPort_GenericInArgs *inArgs = (EnetMacPort_GenericInArgs *)prms->inArgs;
                EnetPort_DscpPriorityMap *dscpPriority = (EnetPort_DscpPriorityMap *)prms->outArgs;

                Enet_devAssert(macPort == inArgs->macPort,
                               "MAC %u: Port mismatch %u\n", portId, inArgs->macPort);

                CpswMacPort_getDscpPriority(regs, macPort, dscpPriority);
            }
            break;

            case ENET_MACPORT_IOCTL_SET_PRI_REGEN_MAP:
            {
                EnetMacPort_SetPriorityRegenMapInArgs *inArgs =
                    (EnetMacPort_SetPriorityRegenMapInArgs *)prms->inArgs;

                Enet_devAssert(macPort == inArgs->macPort,
                               "MAC %u: Port mismatch %u\n", portId, inArgs->macPort);

                status = CpswMacPort_setRxPriority(regs, macPort, &inArgs->priorityRegenMap);
                ENETTRACE_ERR_IF(status != ENET_SOK,
                                 "MAC %u: Failed to set RX priority: %d\n", portId, status);
            }
            break;

            case ENET_MACPORT_IOCTL_GET_PRI_REGEN_MAP:
            {
                EnetMacPort_GenericInArgs *inArgs = (EnetMacPort_GenericInArgs *)prms->inArgs;
                EnetPort_PriorityMap *rxPriority = (EnetPort_PriorityMap *)prms->outArgs;

                Enet_devAssert(macPort == inArgs->macPort,
                               "MAC %u: Port mismatch %u\n", portId, inArgs->macPort);

                CpswMacPort_getRxPriority(regs, macPort, rxPriority);
            }
            break;

            case ENET_MACPORT_IOCTL_SET_EGRESS_QOS_PRI_MAP:
            {
                EnetMacPort_SetEgressPriorityMapInArgs *inArgs =
                    (EnetMacPort_SetEgressPriorityMapInArgs *)prms->inArgs;

                Enet_devAssert(macPort == inArgs->macPort,
                               "MAC %u: Port mismatch %u\n", portId, inArgs->macPort);

                status = CpswMacPort_setTxPriority(regs, macPort, &inArgs->priorityMap);
                ENETTRACE_ERR_IF(status != ENET_SOK,
                                 "MAC %u: Failed to set RX priority: %d\n", portId, status);
            }
            break;

            case ENET_MACPORT_IOCTL_GET_EGRESS_QOS_PRI_MAP:
            {
                EnetMacPort_GenericInArgs *inArgs = (EnetMacPort_GenericInArgs *)prms->inArgs;
                EnetPort_PriorityMap *txPriority = (EnetPort_PriorityMap *)prms->outArgs;

                Enet_devAssert(macPort == inArgs->macPort,
                               "MAC %u: Port mismatch %u\n", portId, inArgs->macPort);

                CpswMacPort_getTxPriority(regs, macPort, txPriority);
            }
            break;

            case ENET_MACPORT_IOCTL_ENABLE_EGRESS_TRAFFIC_SHAPING:
            {
#if ENET_CFG_IS_ON(CPSW_MACPORT_TRAFFIC_SHAPING)
                const EnetMacPort_EnableEgressTrafficShapingInArgs *inArgs =
                    (const EnetMacPort_EnableEgressTrafficShapingInArgs *)prms->inArgs;
                uint32_t cppiClkFreqHz;

                Enet_devAssert(macPort == inArgs->macPort,
                               "MAC %u: Port mismatch %u\n", portId, inArgs->macPort);

                cppiClkFreqHz = EnetSoc_getClkFreq(hPort->enetType, hPort->instId, CPSW_CPPI_CLK);

                status = CpswMacPort_setTrafficShaping(regs, macPort, &inArgs->trafficShapingCfg, cppiClkFreqHz);
                ENETTRACE_ERR_IF(status != ENET_SOK, "Failed to set traffic shaping: %d\n", status);
#else
                status = ENET_ENOTSUPPORTED;
#endif
            }
            break;

            case ENET_MACPORT_IOCTL_DISABLE_EGRESS_TRAFFIC_SHAPING:
            {
#if ENET_CFG_IS_ON(CPSW_MACPORT_TRAFFIC_SHAPING)
                EnetMacPort_GenericInArgs *inArgs = (EnetMacPort_GenericInArgs *)prms->inArgs;

                Enet_devAssert(macPort == inArgs->macPort,
                               "MAC %u: Port mismatch %u\n", portId, inArgs->macPort);

                CpswMacPort_disableTrafficShaping(regs, macPort);
#else
                status = ENET_ENOTSUPPORTED;
#endif
            }
            break;

            case ENET_MACPORT_IOCTL_GET_EGRESS_TRAFFIC_SHAPING:
            {
#if ENET_CFG_IS_ON(CPSW_MACPORT_TRAFFIC_SHAPING)
                EnetMacPort_GenericInArgs *inArgs = (EnetMacPort_GenericInArgs *)prms->inArgs;
                EnetPort_TrafficShapingCfg *shapingCfg = (EnetPort_TrafficShapingCfg *)prms->outArgs;
                uint32_t cppiClkFreqHz;

                Enet_devAssert(macPort == inArgs->macPort,
                               "MAC %u: Port mismatch %u\n", portId, inArgs->macPort);

                cppiClkFreqHz = EnetSoc_getClkFreq(hPort->enetType, hPort->instId, CPSW_CPPI_CLK);

                status = CpswMacPort_getTrafficShaping(regs, macPort, shapingCfg, cppiClkFreqHz);
                ENETTRACE_ERR_IF(status != ENET_SOK, "Failed to get traffic shaping: %d\n", status);
#else
                status = ENET_ENOTSUPPORTED;
#endif
            }
            break;

            case ENET_MACPORT_IOCTL_GET_MAXLEN:
            {
                EnetMacPort_GenericInArgs *inArgs = (EnetMacPort_GenericInArgs *)prms->inArgs;
                EnetPort_MaxLen *maxLen = (EnetPort_MaxLen *)prms->outArgs;

                Enet_devAssert(macPort == inArgs->macPort,
                               "MAC %u: Port mismatch %u\n", portId, inArgs->macPort);

                CpswMacPort_getMaxLen(regs, macPort, maxLen);
            }
            break;

            case ENET_MACPORT_IOCTL_GET_LINK_CFG:
            {
                EnetMacPort_GenericInArgs *inArgs = (EnetMacPort_GenericInArgs *)prms->inArgs;
                EnetMacPort_LinkCfg *linkCfg = (EnetMacPort_LinkCfg *)prms->outArgs;

                Enet_devAssert(macPort == inArgs->macPort,
                               "MAC %u: Port mismatch %u\n", portId, inArgs->macPort);

                CpswMacPort_getLinkCfg(regs, macPort, linkCfg);
            }
            break;

            case CPSW_MACPORT_IOCTL_GET_FIFO_STATS:
            {
                EnetMacPort_GenericInArgs *inArgs = (EnetMacPort_GenericInArgs *)prms->inArgs;
                CpswMacPort_FifoStats *stats = (CpswMacPort_FifoStats *)prms->outArgs;

                Enet_devAssert(macPort == inArgs->macPort,
                               "MAC %u: Port mismatch %u\n", portId, inArgs->macPort);

                CpswMacPort_getFifoStats(regs, macPort, stats);
            }
            break;

            case CPSW_MACPORT_IOCTL_ENABLE_CPTS_EVENT:
            {
                CpswMacPort_EnableTsEventInArgs *inArgs = (CpswMacPort_EnableTsEventInArgs *)prms->inArgs;

                Enet_devAssert(macPort == inArgs->macPort,
                               "MAC %u: Port mismatch %u\n", portId, inArgs->macPort);

                status = CpswMacPort_enableCptsPortEvents(regs, macPort, &inArgs->tsEventCfg);
                ENETTRACE_ERR_IF(status != ENET_SOK,
                                 "MAC %u: Failed to enable CPTS event: %d\n", portId, status);
            }
            break;

            case CPSW_MACPORT_IOCTL_DISABLE_CPTS_EVENT:
            {
                EnetMacPort_GenericInArgs *inArgs = (EnetMacPort_GenericInArgs *)prms->inArgs;

                Enet_devAssert(macPort == inArgs->macPort,
                               "MAC %u: Port mismatch %u\n", portId, inArgs->macPort);

                CpswMacPort_disableCptsPortEvents(regs, macPort);
            }
            break;

            case CPSW_MACPORT_IOCTL_ENABLE:
            {
                const EnetMacPort_LinkCfg *linkCfg = (const EnetMacPort_LinkCfg *)prms->inArgs;
                EnetMacPort_Interface mii;

                status = EnetSoc_getMacPortMii(hPort->enetType, hPort->instId, macPort, &mii);
                if (status == ENET_SOK)
                {
                    if (EnetMacPort_isSgmii(&mii) || EnetMacPort_isQsgmii(&mii))
                    {
#if ENET_CFG_IS_ON(CPSW_MACPORT_SGMII)
                        if (ENET_FEAT_IS_EN(hMod->features, CPSW_MACPORT_FEATURE_SGMII))
                        {
                            status = CpswMacPort_enableSgmiiPort(regs, sgmiiRegs, macPort, &mii, linkCfg);
                        }
                        else
                        {
                            status = ENET_ENOTSUPPORTED;
                        }
#else
                        status = ENET_ENOTSUPPORTED;
#endif
                    }
                    else
                    {
                        status = CpswMacPort_enablePort(regs, macPort, &mii, linkCfg);
                    }
                }

                ENETTRACE_ERR_IF(status != ENET_SOK,
                                 "MAC %u: Failed to enable port: %d\n", portId, status);
            }
            break;

            case CPSW_MACPORT_IOCTL_DISABLE:
            {
                CpswMacPort_disablePort(regs, macPort);
            }
            break;

            case CPSW_MACPORT_IOCTL_SET_INTERVLAN_ROUTE:
            case CPSW_MACPORT_IOCTL_SET_SPECIFIC_INTERVLAN_ROUTE:
            case CPSW_MACPORT_IOCTL_GET_INTERVLAN_ROUTE:
            case CPSW_MACPORT_IOCTL_DELETE_INTERVLAN_ROUTE:
            case CPSW_MACPORT_IOCTL_GET_INTERVLAN_FREEROUTES:
            case CPSW_MACPORT_IOCTL_FIND_INTERVLAN_ROUTE:
            case CPSW_MACPORT_IOCTL_IS_INTERVLAN_ROUTE_FREE:
            {
#if ENET_CFG_IS_ON(CPSW_MACPORT_INTERVLAN)
                if (ENET_FEAT_IS_EN(hMod->features, CPSW_MACPORT_FEATURE_INTERVLAN))
                {
                    status = CpswMacPort_ioctlInterVlan(hMod, cmd, prms);
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

            case CPSW_MACPORT_IOCTL_SET_SHORT_IPG:
            {
                const CpswMacPort_PortTxShortIpgCfg *inArgs =
                    (const CpswMacPort_PortTxShortIpgCfg *)prms->inArgs;

                Enet_devAssert(macPort == inArgs->macPort,
                               "MAC %u: Port mismatch %u\n", portId, inArgs->macPort);

                CpswMacPort_setShortIpgCfg(regs, macPort, &inArgs->shortIpgCfg);
            }
            break;

            case CPSW_MACPORT_IOCTL_GET_SHORT_IPG:
            {
                EnetMacPort_GenericInArgs *inArgs = (EnetMacPort_GenericInArgs *)prms->inArgs;
                CpswMacPort_TxShortIpgCfg *shortIpgCfg = (CpswMacPort_TxShortIpgCfg *)prms->outArgs;

                Enet_devAssert(macPort == inArgs->macPort,
                               "MAC %u: Port mismatch %u\n", portId, inArgs->macPort);

                CpswMacPort_getShortIpgCfg(regs, macPort, shortIpgCfg);
            }
            break;

            case CPSW_MACPORT_IOCTL_GET_SGMII_AUTONEG_LINK_STATUS:
            {
#if ENET_CFG_IS_ON(CPSW_MACPORT_SGMII)
                if (ENET_FEAT_IS_EN(hMod->features, CPSW_MACPORT_FEATURE_SGMII))
                {
                    EnetMacPort_GenericInArgs *inArgs = (EnetMacPort_GenericInArgs *)prms->inArgs;
                    bool *autoNegDone = (bool *)prms->outArgs;

                    Enet_devAssert(macPort == inArgs->macPort,
                                   "MAC %u: Port mismatch %u\n", portId, inArgs->macPort);

                    status = CpswMacPort_checkSgmiiAutoNegStatus(sgmiiRegs, macPort);
                    ENETTRACE_ERR_IF(status != ENET_SOK,
                                     "MAC %u: Failed to get SGMII link status: %d\n", portId, status);

                    *autoNegDone = (ENET_SOK == status);
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

            case CPSW_MACPORT_IOCTL_GET_SGMII_LINK_STATUS:
            {
#if ENET_CFG_IS_ON(CPSW_MACPORT_SGMII)
                if (ENET_FEAT_IS_EN(hMod->features, CPSW_MACPORT_FEATURE_SGMII))
                {
                    EnetMacPort_GenericInArgs *inArgs = (EnetMacPort_GenericInArgs *)prms->inArgs;
                    bool *linkUp = (bool *)prms->outArgs;

                    Enet_devAssert(macPort == inArgs->macPort,
                                   "MAC %u: Port mismatch %u\n", portId, inArgs->macPort);

                    *linkUp = CpswMacPort_getSgmiiStatus(sgmiiRegs, macPort);
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

            default:
            {
                status = ENET_EINVALIDPARAMS;
            }
            break;
        }
    }

    return status;
}

#if ENET_CFG_IS_ON(DEV_ERROR)
static int32_t CpswMacPort_isSupported(CSL_Xge_cpswRegs *regs)
{
    CSL_CPSW_VERSION version;
    uint32_t i;
    int32_t status = ENET_ENOTSUPPORTED;

    CSL_CPSW_getCpswVersionInfo(regs, &version);

    for (i = 0U; i < ENET_ARRAYSIZE(CpswMacPort_gSupportedVer); i++)
    {
        if ((version.majorVer == CpswMacPort_gSupportedVer[i].majorVer) &&
            (version.minorVer == CpswMacPort_gSupportedVer[i].minorVer) &&
            (version.rtlVer == CpswMacPort_gSupportedVer[i].rtlVer) &&
            (version.id == CpswMacPort_gSupportedVer[i].id))
        {
            status = ENET_SOK;
            break;
        }
    }

    return status;
}

static int32_t CpswMacPort_isMiiSupported(EnetMod_Handle hMod,
                                          const EnetMacPort_Interface *mii)
{
    int32_t status = ENET_ENOTSUPPORTED;

    if (EnetMacPort_isRmii(mii))
    {
        status = ENET_SOK;
    }
    else if (EnetMacPort_isRgmii(mii))
    {
        status = ENET_SOK;
    }
    else if (EnetMacPort_isSgmii(mii) ||
             EnetMacPort_isQsgmii(mii))
    {
#if ENET_CFG_IS_ON(CPSW_MACPORT_SGMII)
        if (ENET_FEAT_IS_EN(hMod->features, CPSW_MACPORT_FEATURE_SGMII))
        {
            status = ENET_SOK;
        }
#endif
    }
    else
    {
        status = ENET_ENOTSUPPORTED;
    }

    return status;
}
#endif

static int32_t CpswMacPort_checkSocCfg(Enet_Type enetType,
                                       uint32_t instId,
                                       Enet_MacPort macPort,
                                       const EnetMacPort_Interface *mii)
{
    EnetMacPort_Interface miiSoc;
    int32_t status;

    status = EnetSoc_getMacPortMii(enetType, instId, macPort, &miiSoc);
    if (status == ENET_SOK)
    {
        if ((miiSoc.layerType != mii->layerType) ||
            (miiSoc.sublayerType != mii->sublayerType))
        {
            status = ENET_EINVALIDPARAMS;
        }
    }
    else
    {
        /* SoC layer reports ENET_ENOTSUPPORTED when there isn't any SoC layer
         * settings related to the MAC port configuration.  So we don't treat
         * that as an error */
        if (status == ENET_ENOTSUPPORTED)
        {
            status = ENET_SOK;
        }
    }

    return status;
}

static void CpswMacPort_reset(CSL_Xge_cpswRegs *regs,
                              Enet_MacPort macPort)
{
    CSL_CPGMAC_SL_MACSTATUS macStatus;
    uint32_t portNum = ENET_MACPORT_NORM(macPort);
    uint32_t gmiiEn;
    uint32_t done;

    gmiiEn = CSL_CPGMAC_SL_isGMIIEnabled(regs, portNum);

    /* Idle MAC port */
    CSL_CPGMAC_SL_enableIdleMode(regs, portNum);
    do
    {
        CSL_CPGMAC_SL_getMacStatusReg(regs, portNum, &macStatus);
        done = (macStatus.idle == 1U);

        /* TX idle is set only if GMII clock is enabled */
        if (gmiiEn == 1U)
        {
            done = done && (macStatus.macTxIdle == 1U);
        }
    }
    while (done == FALSE);

    /* Soft-reset the Ethernet MAC logic */
    CSL_CPGMAC_SL_resetMac(regs, portNum);
    do
    {
        done = CSL_CPGMAC_SL_isMACResetDone(regs, portNum);
    }
    while (done == FALSE);
}

static void CpswMacPort_setSwitchTxSched(CSL_Xge_cpswRegs *regs,
                                         Enet_MacPort macPort,
                                         EnetPort_EgressPriorityType priority)
{
    CSL_CPSW_PTYPE pType;
    uint32_t escEn;
    uint32_t escPriLoadVal;

    if (priority == ENET_EGRESS_PRI_TYPE_FIXED)
    {
        escEn = FALSE;
        escPriLoadVal = CPSW_MACPORT_ESC_PRI_LD_VAL;
    }
    else
    {
        escEn = TRUE;
        escPriLoadVal = 0U;
    }

    CSL_CPSW_getPTypeReg(regs, &pType);

    switch (macPort)
    {
        case ENET_MAC_PORT_1:
            pType.port1PriorityTypeEscalateEnable = escEn;
            pType.escPriLoadVal = escPriLoadVal;
            break;

        case ENET_MAC_PORT_2:
            pType.port2PriorityTypeEscalateEnable = escEn;
            pType.escPriLoadVal = escPriLoadVal;
            break;

        case ENET_MAC_PORT_3:
            pType.port3PriorityTypeEscalateEnable = escEn;
            pType.escPriLoadVal = escPriLoadVal;
            break;

        case ENET_MAC_PORT_4:
            pType.port4PriorityTypeEscalateEnable = escEn;
            pType.escPriLoadVal = escPriLoadVal;
            break;

        case ENET_MAC_PORT_5:
            pType.port5PriorityTypeEscalateEnable = escEn;
            pType.escPriLoadVal = escPriLoadVal;
            break;

        case ENET_MAC_PORT_6:
            pType.port6PriorityTypeEscalateEnable = escEn;
            pType.escPriLoadVal = escPriLoadVal;
            break;

        case ENET_MAC_PORT_7:
            pType.port7PriorityTypeEscalateEnable = escEn;
            pType.escPriLoadVal = escPriLoadVal;
            break;

        case ENET_MAC_PORT_8:
            pType.port8PriorityTypeEscalateEnable = escEn;
            pType.escPriLoadVal = escPriLoadVal;
            break;

        default:
            Enet_devAssert(false, "MAC %u: Invalid MAC port\n", ENET_MACPORT_ID(macPort));
            break;
    }

    CSL_CPSW_setPTypeReg(regs, &pType);
}

static int32_t CpswMacPort_enableLoopback(CSL_Xge_cpswRegs *regs,
                                          Enet_MacPort macPort,
                                          const EnetMacPort_Interface *mii)
{
    uint32_t portNum = ENET_MACPORT_NORM(macPort);
    uint32_t portId = ENET_MACPORT_ID(macPort);
    int32_t status = ENET_SOK;

    ENETTRACE_VAR(portId);
    if (EnetMacPort_isRmii(mii) ||
        EnetMacPort_isRgmii(mii))
    {
        CSL_CPGMAC_SL_enableLoopback(regs, portNum);
    }
    else
    {
        ENETTRACE_ERR("MAC %u: Loopback is not supported in MII mode %u-%u\n",
                      portId, mii->layerType, mii->sublayerType);
        status = ENET_ENOTSUPPORTED;
    }

    return status;
}

static int32_t CpswMacPort_setInterface(CSL_Xge_cpswRegs *regs,
                                        Enet_MacPort macPort,
                                        const EnetMacPort_Interface *mii)
{
    uint32_t macControl;
    uint32_t portNum = ENET_MACPORT_NORM(macPort);
    uint32_t portId = ENET_MACPORT_ID(macPort);
    int32_t status = ENET_EINVALIDPARAMS;

    ENETTRACE_VAR(portId);
    macControl = CSL_CPGMAC_SL_getMacControlReg(regs, portNum);

    /* Clear fields not supported by hardware */
    CSL_FINS(macControl, XGE_CPSW_PN_MAC_CONTROL_REG_EXT_RX_FLOW_EN, 0U);
    CSL_FINS(macControl, XGE_CPSW_PN_MAC_CONTROL_REG_EXT_TX_FLOW_EN, 0U);

    /* Set MAC_CONTROL register settings according to the MII type */
    if (EnetMacPort_isRmii(mii))
    {
        status = ENET_SOK;
    }
    else if (EnetMacPort_isRgmii(mii))
    {
        CSL_FINS(macControl, XGE_CPSW_PN_MAC_CONTROL_REG_EXT_EN, 0U);
        CSL_FINS(macControl, XGE_CPSW_PN_MAC_CONTROL_REG_GIG_FORCE, 1U);
        status = ENET_SOK;
    }
    else if (EnetMacPort_isXfi(mii))
    {
        CSL_FINS(macControl, XGE_CPSW_PN_MAC_CONTROL_REG_EXT_EN_XGIG, 0U);
        status = ENET_SOK;
    }
    else
    {
        status = ENET_EINVALIDPARAMS;
    }

    /* Set interface layer and sublayer */
    if (status == ENET_SOK)
    {
        CSL_CPGMAC_SL_setMacControlReg(regs, portNum, macControl);
    }

    ENETTRACE_ERR_IF(status != ENET_SOK,
                     "MAC %u: failed to set interface layer %u sublayer %u: %d\n",
                     portId, mii->layerType, mii->sublayerType, status);

    return status;
}

static bool CpswMacPort_isPortEnabled(CSL_Xge_cpswRegs *regs,
                                      Enet_MacPort macPort)
{
    uint32_t portNum = ENET_MACPORT_NORM(macPort);
    uint32_t macControl;
    uint32_t gmiiEn;
    uint32_t xgmiiEn;

    macControl = CSL_CPGMAC_SL_getMacControlReg(regs, portNum);
    gmiiEn = CSL_FEXT(macControl, XGE_CPSW_PN_MAC_CONTROL_REG_GMII_EN);
    xgmiiEn = CSL_FEXT(macControl, XGE_CPSW_PN_MAC_CONTROL_REG_XGMII_EN);

    return((gmiiEn == 1U) || (xgmiiEn == 1U));
}

static int32_t CpswMacPort_enablePort(CSL_Xge_cpswRegs *regs,
                                      Enet_MacPort macPort,
                                      const EnetMacPort_Interface *mii,
                                      const EnetMacPort_LinkCfg *linkCfg)
{
    uint32_t portNum = ENET_MACPORT_NORM(macPort);
    uint32_t portId = ENET_MACPORT_ID(macPort);
    uint32_t extCtrl;
    uint32_t macControl;
    int32_t status;
    bool forced = true;

    ENETTRACE_VAR(portId);

    macControl = CSL_CPGMAC_SL_getMacControlReg(regs, portNum);

    /* Disable all fields, enable as needed based on link config */
    CSL_FINS(macControl, XGE_CPSW_PN_MAC_CONTROL_REG_FULLDUPLEX, 0U);
    CSL_FINS(macControl, XGE_CPSW_PN_MAC_CONTROL_REG_GMII_EN, 0U);
    CSL_FINS(macControl, XGE_CPSW_PN_MAC_CONTROL_REG_GIG, 0U);
    CSL_FINS(macControl, XGE_CPSW_PN_MAC_CONTROL_REG_XGMII_EN, 0U);
    CSL_FINS(macControl, XGE_CPSW_PN_MAC_CONTROL_REG_IFCTL_A, 0U);
    CSL_FINS(macControl, XGE_CPSW_PN_MAC_CONTROL_REG_IFCTL_B, 0U);

    if (EnetMacPort_isRmii(mii))
    {
        /* In RMII mode, ifctl_a bit determines the RMII link speed (0=10mbps, 1=100mbps) */
        if ((linkCfg->speed == ENET_SPEED_100MBIT) ||
            (linkCfg->speed == ENET_SPEED_10MBIT))
        {
            if (linkCfg->speed == ENET_SPEED_100MBIT)
            {
                CSL_FINS(macControl, XGE_CPSW_PN_MAC_CONTROL_REG_IFCTL_A, 1U);
            }

            CSL_FINS(macControl, XGE_CPSW_PN_MAC_CONTROL_REG_GMII_EN, 1U);
            status = ENET_SOK;
        }
        else
        {
            ENETTRACE_ERR("MAC %u: Invalid speed for RMII mode\n", portId);
            status = ENET_EINVALIDPARAMS;
        }

        forced = true;
    }
    else if (EnetMacPort_isRgmii(mii))
    {
        /* RGMII */
        extCtrl = CSL_FEXT(macControl, XGE_CPSW_PN_MAC_CONTROL_REG_EXT_EN);
        forced = (extCtrl == TRUE) ? false : true;

        if (forced)
        {
            if (linkCfg->speed == ENET_SPEED_1GBIT)
            {
                CSL_FINS(macControl, XGE_CPSW_PN_MAC_CONTROL_REG_GIG, 1U);
            }
        }

        CSL_FINS(macControl, XGE_CPSW_PN_MAC_CONTROL_REG_GMII_EN, 1U);
        status = ENET_SOK;
    }
    else if (EnetMacPort_isXfi(mii))
    {
        /* XGMII */
        extCtrl = CSL_FEXT(macControl, XGE_CPSW_PN_MAC_CONTROL_REG_EXT_EN_XGIG);
        forced = (extCtrl == TRUE) ? false : true;

        if (forced)
        {
            CSL_FINS(macControl, XGE_CPSW_PN_MAC_CONTROL_REG_XGMII_EN, 1U);
            status = ENET_SOK;
        }
        else
        {
            /* XGMII in-band mode is not supported in hardware */
            ENETTRACE_ERR("MAC %u: XGMII in-band mode is not supported\n", portId);
            status = ENET_ENOTSUPPORTED;
        }
    }
    else
    {
        /* Q/SGMII has a dedicated function, called if features is enabled */
        status = ENET_ENOTSUPPORTED;
    }

    /* Half-duplex mode is supported only at 10/100 Mbps */
    if (status == ENET_SOK)
    {
        if ((linkCfg->duplexity == ENET_DUPLEX_HALF) &&
            (linkCfg->speed != ENET_SPEED_10MBIT) &&
            (linkCfg->speed != ENET_SPEED_100MBIT))
        {
            ENETTRACE_ERR("MAC %u: 1Gbps half-duplex is not supported\n", portId);
            status = ENET_EINVALIDPARAMS;
        }
    }

    /* Set duplexity, speed related fields in MAC control */
    if (status == ENET_SOK)
    {
        if (forced && (linkCfg->duplexity == ENET_DUPLEX_FULL))
        {
            CSL_FINS(macControl, XGE_CPSW_PN_MAC_CONTROL_REG_FULLDUPLEX, 1U);
        }

        /* Set interface layer, sublayer, speed */
        CSL_CPGMAC_SL_setMacControlReg(regs, portNum, macControl);
    }

    ENETTRACE_ERR_IF(status != ENET_SOK,
                     "MAC %u: Failed to enable MAC port: %d\n", portId, status);

    return status;
}

static void CpswMacPort_disablePort(CSL_Xge_cpswRegs *regs,
                                    Enet_MacPort macPort)
{
    uint32_t portNum = ENET_MACPORT_NORM(macPort);
    uint32_t macControl;

    /* Save MAC_CONTROL register context before soft-reset */
    macControl = CSL_CPGMAC_SL_getMacControlReg(regs, portNum);

    CSL_FINS(macControl, XGE_CPSW_PN_MAC_CONTROL_REG_FULLDUPLEX, 0U);
    CSL_FINS(macControl, XGE_CPSW_PN_MAC_CONTROL_REG_GMII_EN, 0U);
    CSL_FINS(macControl, XGE_CPSW_PN_MAC_CONTROL_REG_GIG, 0U);
    CSL_FINS(macControl, XGE_CPSW_PN_MAC_CONTROL_REG_XGMII_EN, 0U);
    CSL_FINS(macControl, XGE_CPSW_PN_MAC_CONTROL_REG_IFCTL_A, 0U);
    CSL_FINS(macControl, XGE_CPSW_PN_MAC_CONTROL_REG_IFCTL_B, 0U);

    /* Soft-reset the Ethernet MAC logic */
    CpswMacPort_reset(regs, macPort);

    /* Restore context of fields in MAC_CONTROL register that are configured at
     * open time via CpswMacPort_Cfg and that are not related to link configuration */
    CSL_CPGMAC_SL_setMacControlReg(regs, portNum, macControl);
}

static void CpswMacPort_printRegs(CSL_Xge_cpswRegs *regs,
                                  Enet_MacPort macPort)
{
    uint32_t portNum = ENET_MACPORT_NORM(macPort);
    uint32_t portId = ENET_MACPORT_ID(macPort);
    CSL_Xge_cpswEnetportRegs *macPortRegs = &regs->ENETPORT[portNum];
    uint32_t *regAddr = (uint32_t *)((uintptr_t)macPortRegs + CPSW_MACPORT_START_REG_OFFSET);
    uint32_t regIdx = 0U;

    ENETTRACE_VAR(portId);
    while (((uintptr_t)regAddr) <= ((uintptr_t)macPortRegs + CPSW_MACPORT_END_REG_OFFSET))
    {
        if (*regAddr != 0U)
        {
            ENETTRACE_INFO("MACPORT %u: %u: 0x%08x\n", portId, regIdx, *regAddr);
        }

        regAddr++;
        regIdx++;
    }
}

static int32_t CpswMacPort_setDscpPriority(CSL_Xge_cpswRegs *regs,
                                           Enet_MacPort macPort,
                                           EnetPort_DscpPriorityMap *dscpPriority)
{
    CSL_CPSW_PORT_CONTROL control;
    uint32_t portNum = ENET_MACPORT_NORM(macPort);
    uint32_t portId = ENET_MACPORT_ID(macPort);
    uint32_t i;
    int32_t status = ENET_SOK;

    ENETTRACE_VAR(portId);
    for (i = 0U; i < ENET_TOS_PRI_NUM; i++)
    {
        if (dscpPriority->tosMap[i] > ENET_PRI_MAX)
        {
            ENETTRACE_ERR("MAC %u: Invalid priority map %u -> %u\n", portId, i, dscpPriority->tosMap[i]);
            status = ENET_EINVALIDPARAMS;
            break;
        }
    }

    if (status == ENET_SOK)
    {
        CSL_CPSW_setPortRxDscpMap(regs, portNum + 1U, dscpPriority->tosMap);
        CSL_CPSW_getPortControlReg(regs, portNum + 1U, &control);

        control.dscpIpv4Enable = dscpPriority->dscpIPv4En;
        control.dscpIpv6Enable = dscpPriority->dscpIPv6En;

        CSL_CPSW_setPortControlReg(regs, portNum + 1U, &control);
    }

    return status;
}

static void CpswMacPort_getDscpPriority(CSL_Xge_cpswRegs *regs,
                                        Enet_MacPort macPort,
                                        EnetPort_DscpPriorityMap *dscpPriority)
{
    CSL_CPSW_PORT_CONTROL control;
    uint32_t portNum = ENET_MACPORT_NORM(macPort);

    CSL_CPSW_getPortRxDscpMap(regs, portNum + 1U, dscpPriority->tosMap);
    CSL_CPSW_getPortControlReg(regs, portNum + 1U, &control);

    dscpPriority->dscpIPv4En = control.dscpIpv4Enable;
    dscpPriority->dscpIPv6En = control.dscpIpv6Enable;
}

static int32_t CpswMacPort_setRxPriority(CSL_Xge_cpswRegs *regs,
                                         Enet_MacPort macPort,
                                         EnetPort_PriorityMap *rxPriority)
{
    uint32_t portNum = ENET_MACPORT_NORM(macPort);
    uint32_t portId = ENET_MACPORT_ID(macPort);
    uint32_t i;
    int32_t status = ENET_SOK;

    ENETTRACE_VAR(portId);
    for (i = 0U; i < ENET_PRI_NUM; i++)
    {
        if (rxPriority->priorityMap[i] > ENET_PRI_MAX)
        {
            ENETTRACE_ERR("MAC %u: Invalid priority map %u -> %u\n", portId, i, rxPriority->priorityMap[i]);
            status = ENET_EINVALIDPARAMS;
            break;
        }
    }

    if (status == ENET_SOK)
    {
        CSL_CPSW_setPortRxPriMapReg(regs, portNum + 1U, rxPriority->priorityMap);
    }

    return status;
}

static void CpswMacPort_getRxPriority(CSL_Xge_cpswRegs *regs,
                                      Enet_MacPort macPort,
                                      EnetPort_PriorityMap *rxPriority)
{
    uint32_t portNum = ENET_MACPORT_NORM(macPort);

    CSL_CPSW_getPortRxPriMapReg(regs, portNum + 1U, rxPriority->priorityMap);
}

static int32_t CpswMacPort_setTxPriority(CSL_Xge_cpswRegs *regs,
                                         Enet_MacPort macPort,
                                         EnetPort_PriorityMap *txPriority)
{
    uint32_t portNum = ENET_MACPORT_NORM(macPort);
    uint32_t portId = ENET_MACPORT_ID(macPort);
    uint32_t i;
    int32_t status = ENET_SOK;

    ENETTRACE_VAR(portId);
    for (i = 0U; i < ENET_PRI_NUM; i++)
    {
        if (txPriority->priorityMap[i] > ENET_PRI_MAX)
        {
            ENETTRACE_ERR("MAC %u: Invalid priority map %u -> %u\n", portId, i, txPriority->priorityMap[i]);
            status = ENET_EINVALIDPARAMS;
            break;
        }
    }

    if (status == ENET_SOK)
    {
        CSL_CPSW_setPortTxPriMapReg(regs, portNum + 1U, txPriority->priorityMap);
    }

    return status;
}

static void CpswMacPort_getTxPriority(CSL_Xge_cpswRegs *regs,
                                      Enet_MacPort macPort,
                                      EnetPort_PriorityMap *txPriority)
{
    uint32_t portNum = ENET_MACPORT_NORM(macPort);

    CSL_CPSW_getPortTxPriMapReg(regs, portNum + 1U, txPriority->priorityMap);
}

#if ENET_CFG_IS_ON(CPSW_MACPORT_TRAFFIC_SHAPING)
static uint64_t CpswMacPort_mapBwToCnt(uint64_t rateInBps,
                                       uint32_t cppiClkFreqHz)
{
    /*
     * Transfer rate to CIR or EIR count value equation:
     *
     *                Transfer rate (Mbps) * 32768
     * Count value = ------------------------------
     *                   CPPI_ICLK freq (MHz)
     *
     * Round division to prevent transfer rates below traffic shaping resolution
     * end up with no shaping at all.
     */
    return ENET_DIV_ROUNDUP(rateInBps * CPSW_MACPORT_RATELIM_DIV, cppiClkFreqHz);
}

static uint64_t CpswMacPort_mapCntToBw(uint32_t cntVal,
                                       uint32_t cppiClkFreqHz)
{
    /*
     * CIR or EIR count value to transfer rate equation:
     *
     *                         count value * CPPI_ICLK freq (MHz)
     * Transfer rate (Mbps) = ------------------------------------
     *                                      32768
     */
    return ((uint64_t)cntVal * cppiClkFreqHz) / CPSW_MACPORT_RATELIM_DIV;
}

static int32_t CpswMacPort_setTrafficShaping(CSL_Xge_cpswRegs *regs,
                                             Enet_MacPort macPort,
                                             const EnetPort_TrafficShapingCfg *cfg,
                                             uint32_t cppiClkFreqHz)
{
    uint64_t cirBps, eirBps;
    uint64_t cir, eir;
    uint32_t portNum = ENET_MACPORT_NORM(macPort);
    uint32_t portId = ENET_MACPORT_ID(macPort);
    uint32_t i;
    int32_t status= ENET_SOK;
    bool rateLimEnabled = false;

    ENETTRACE_VAR(portId);
    for (i = 0U; i < ENET_PRI_NUM; i++)
    {
        cirBps = cfg->rates[i].committedRateBitsPerSec;
        eirBps = cfg->rates[i].excessRateBitsPerSec;
        cir = CpswMacPort_mapBwToCnt(cirBps, cppiClkFreqHz);
        eir = CpswMacPort_mapBwToCnt(eirBps, cppiClkFreqHz);

        /* CIR must be non-zero if EIR is non-zero */
        if ((cir == 0ULL) && (eir != 0ULL))
        {
            ENETTRACE_ERR("MAC %u: EIR is enabled (%ubps = %llu) but CIR is not (%ubps = %llu) "
                          "for priority %u\n",
                          portId, eirBps, eir, cirBps, cir, i);
            status = ENET_EINVALIDPARAMS;
            break;
        }

        /* Rate limit must be the highest priority channels */
        if ((cir == 0ULL) && rateLimEnabled)
        {
            ENETTRACE_ERR("MAC %u: rate limiting disabled for priority %u\n", portId, i);
            status = ENET_EINVALIDPARAMS;
            break;
        }

        /* Out of range */
        if ((cir > 0x0FFFFFFFULL) || (eir > 0x0FFFFFFFULL))
        {
            ENETTRACE_ERR("MAC %u: invalid CIR=%llu (%llubps) EIR=%llu (%llubps) for priority %u\n",
                          portId, cir, cirBps, eir, eirBps, i);
            status = ENET_EINVALIDPARAMS;
            break;
        }

        ENETTRACE_DBG("MAC %u: rate limiting %s for priority %u, CIR=%llubps (%llu) EIR=%llubps (%llu)\n",
                      portId, (cir != 0ULL) ? "enabled" : "disabled",
                      i, cirBps, cir, eirBps, eir);

        CSL_CPSW_setPriCirEir(regs, portNum, i, (uint32_t)cir, (uint32_t)eir);

        if (cir != 0ULL)
        {
            rateLimEnabled = true;
        }
    }

    return status;
}

static void CpswMacPort_disableTrafficShaping(CSL_Xge_cpswRegs *regs,
                                              Enet_MacPort macPort)
{
    uint32_t portNum = ENET_MACPORT_NORM(macPort);
    uint32_t i;

    for (i = 0U; i < ENET_PRI_NUM; i++)
    {
        CSL_CPSW_setPriCirEir(regs, portNum, i, 0U, 0U);
    }
}

static int32_t CpswMacPort_getTrafficShaping(CSL_Xge_cpswRegs *regs,
                                             Enet_MacPort macPort,
                                             EnetPort_TrafficShapingCfg *cfg,
                                             uint32_t cppiClkFreqHz)
{
    uint32_t cir, eir;
    uint64_t cirBps, eirBps;
    uint32_t portNum = ENET_MACPORT_NORM(macPort);
    uint32_t portId = ENET_MACPORT_ID(macPort);
    uint32_t i;
    int32_t status = ENET_SOK;

    ENETTRACE_VAR(portId);
    for (i = 0U; i < ENET_PRI_NUM; i++)
    {
        CSL_CPSW_getPriCirEir(regs, portNum, i, &cir, &eir);

        cirBps = CpswMacPort_mapCntToBw(cir, cppiClkFreqHz);
        eirBps = CpswMacPort_mapCntToBw(eir, cppiClkFreqHz);

        /* CIR must be non-zero if EIR is non-zero */
        if ((cirBps == 0ULL) && (eirBps != 0ULL))
        {
            ENETTRACE_ERR("MAC %u: EIR is enabled (%llubps = %u) but CIR is not (%llubps = %u) "
                          "for priority %u\n",
                          portId, eirBps, eir, cirBps, cir, i);
            status = ENET_EUNEXPECTED;
            break;
        }

        ENETTRACE_DBG("MAC %u: rate limiting %s for priority %u, CIR=%llubps (%u) EIR=%llubps (%u)\n",
                      portId, (cir != 0U) ? "enabled" : "disabled",
                      i, cirBps, cir, eirBps, eir);

        cfg->rates[i].committedRateBitsPerSec = cirBps;
        cfg->rates[i].excessRateBitsPerSec = eirBps;
    }

    return status;
}
#endif

static void CpswMacPort_getMaxLen(CSL_Xge_cpswRegs *regs,
                                  Enet_MacPort macPort,
                                  EnetPort_MaxLen *maxLen)
{
    uint32_t portNum = ENET_MACPORT_NORM(macPort);
    uint32_t i;

    maxLen->mru = CSL_CPGMAC_SL_getRxMaxLen(regs, portNum);

    for (i = 0U; i < ENET_PRI_NUM; i++)
    {
        maxLen->mtu[i] = CSL_CPSW_getTxMaxLenPerPriority(regs, i);
    }
}

static void CpswMacPort_getFifoStats(CSL_Xge_cpswRegs *regs,
                                     Enet_MacPort macPort,
                                     CpswMacPort_FifoStats *stats)
{
    CSL_CPSW_THRURATE thruRate;
    CSL_CPGMAC_SL_FIFOSTATUS fifoStatus;
    uint32_t portNum = ENET_MACPORT_NORM(macPort);
    uint32_t i;

    CSL_CPSW_getPortBlockCountReg(regs, portNum + 1U,
                                  &stats->rxExpressBlockCount,
                                  &stats->rxPreemptBlockCount,
                                  &stats->txBlockCount);

    CSL_CPSW_getPortMaxBlksReg(regs, portNum + 1U,
                               &stats->rxMaxBlocks,
                               &stats->txMaxBlocks);

    CSL_CPSW_getThruRateReg(regs, &thruRate);
    stats->rxThroughputRate = thruRate.enetRxThruRate;
    stats->txStartWords = CSL_CPSW_getTxStartWords(regs);

    CSL_CPGMAC_SL_getFifoStatus(regs, portNum, &fifoStatus);
    for (i = 0U; i < ENET_ARRAYSIZE(stats->txActiveFifo); i++)
    {
        stats->txActiveFifo[i] = (fifoStatus.txPriActive >> i) & 0x1U;
    }
}

static void CpswMacPort_getLinkCfg(CSL_Xge_cpswRegs *regs,
                                   Enet_MacPort macPort,
                                   EnetMacPort_LinkCfg *linkCfg)
{
    uint32_t portNum = ENET_MACPORT_NORM(macPort);
    CSL_CPGMAC_SL_MACSTATUS macStatus;

    CSL_CPGMAC_SL_getMacStatusReg(regs, portNum, &macStatus);

    /* TODO - How to identify 10Mbps */
    if (macStatus.extGigabitEnabled == TRUE)
    {
        linkCfg->speed = ENET_SPEED_1GBIT;
    }
    else
    {
        linkCfg->speed = ENET_SPEED_100MBIT;
    }

    if (macStatus.extFullDuplexEnabled == TRUE)
    {
        linkCfg->duplexity = ENET_DUPLEX_FULL;
    }
    else
    {
        linkCfg->duplexity = ENET_DUPLEX_HALF;
    }
}

static int32_t CpswMacPort_enableCptsPortEvents(CSL_Xge_cpswRegs *regs,
                                                Enet_MacPort macPort,
                                                CpswMacPort_TsEventCfg *tsEventCfg)
{
    CSL_CPSW_TSCONFIG timeSyncCfg;
    CpswMacPort_IpTsCfg *commonPortIpCfg;
    uint32_t portNum = ENET_MACPORT_NORM(macPort);
    int32_t status = ENET_SOK;

    if (tsEventCfg->seqIdOffset >= CPSW_MACPORT_PTP_SEQID_OFFSET_MIN_VAL)
    {
        commonPortIpCfg = &tsEventCfg->commonPortIpCfg;

        memset(&timeSyncCfg, 0, sizeof(CSL_CPSW_TSCONFIG));

        timeSyncCfg.tsTxHostEnable   = tsEventCfg->txHostTsEn;
        timeSyncCfg.tsRxAnnexDEnable = tsEventCfg->rxAnnexDEn;
        timeSyncCfg.tsRxAnnexEEnable = tsEventCfg->rxAnnexEEn;
        timeSyncCfg.tsRxAnnexFEnable = tsEventCfg->rxAnnexFEn;
        timeSyncCfg.tsTxAnnexDEnable = tsEventCfg->txAnnexDEn;
        timeSyncCfg.tsTxAnnexEEnable = tsEventCfg->txAnnexEEn;
        timeSyncCfg.tsTxAnnexFEnable = tsEventCfg->txAnnexFEn;
        timeSyncCfg.tsMsgTypeEnable  = tsEventCfg->messageType;
        timeSyncCfg.tsSeqIdOffset    = tsEventCfg->seqIdOffset;
        timeSyncCfg.tsDomainOffset   = tsEventCfg->domainOffset;

        if (tsEventCfg->rxAnnexDEn || tsEventCfg->rxAnnexEEn ||
            tsEventCfg->txAnnexDEn || tsEventCfg->txAnnexEEn)
        {
            if (tsEventCfg->rxAnnexEEn || tsEventCfg->txAnnexEEn)
            {
                timeSyncCfg.tsMcastTypeEnable = tsEventCfg->mcastType;
            }

            timeSyncCfg.tsTTLNonzeroEnable = commonPortIpCfg->ttlNonzeroEn;
            timeSyncCfg.tsUniEnable        = commonPortIpCfg->unicastEn;

            if (timeSyncCfg.tsUniEnable == FALSE)
            {
                timeSyncCfg.ts107Enable = commonPortIpCfg->tsIp107En;
                timeSyncCfg.ts129Enable = commonPortIpCfg->tsIp129En;
                timeSyncCfg.ts130Enable = commonPortIpCfg->tsIp130En;
                timeSyncCfg.ts131Enable = commonPortIpCfg->tsIp131En;
                timeSyncCfg.ts132Enable = commonPortIpCfg->tsIp132En;
                timeSyncCfg.ts319Enable = commonPortIpCfg->tsPort319En;
                timeSyncCfg.ts320Enable = commonPortIpCfg->tsPort320En;
            }
        }

        if (tsEventCfg->rxAnnexFEn || tsEventCfg->txAnnexFEn)
        {
            timeSyncCfg.tsLType2Enable  = tsEventCfg->ltype2En;
            timeSyncCfg.tsLType1        = ENET_ETHERTYPE_PTP;

            if (timeSyncCfg.tsLType2Enable == TRUE)
            {
                timeSyncCfg.tsLType2    = ENET_ETHERTYPE_PTP;
            }
        }

        if (tsEventCfg->rxVlanType != ENET_MACPORT_VLAN_TYPE_NONE)
        {
            timeSyncCfg.tsRxVlanLType1Enable = TRUE;
            if (tsEventCfg->rxVlanType == ENET_MACPORT_VLAN_TYPE_STACKED_TAGS)
            {
                timeSyncCfg.tsRxVlanLType2Enable = TRUE;
            }
        }

        if (tsEventCfg->txVlanType != ENET_MACPORT_VLAN_TYPE_NONE)
        {
            timeSyncCfg.tsTxVlanLType1Enable = TRUE;
            if (tsEventCfg->txVlanType == ENET_MACPORT_VLAN_TYPE_STACKED_TAGS)
            {
                timeSyncCfg.tsTxVlanLType2Enable = TRUE;
            }
        }

        if ((tsEventCfg->rxVlanType != ENET_MACPORT_VLAN_TYPE_NONE) ||
            (tsEventCfg->txVlanType != ENET_MACPORT_VLAN_TYPE_NONE))
        {
            timeSyncCfg.tsVlanLType1 = tsEventCfg->vlanLType1;

            if ((tsEventCfg->rxVlanType != ENET_MACPORT_VLAN_TYPE_STACKED_TAGS)||
                (tsEventCfg->txVlanType != ENET_MACPORT_VLAN_TYPE_STACKED_TAGS))
            {
                timeSyncCfg.tsVlanLType2 = tsEventCfg->vlanLType2;
            }
        }

        CSL_CPSW_setPortTimeSyncConfig(regs, portNum + 1U, &timeSyncCfg);
    }
    else
    {
        status = ENET_EINVALIDPARAMS;
    }

    return status;
}

static void CpswMacPort_disableCptsPortEvents(CSL_Xge_cpswRegs *regs,
                                              Enet_MacPort macPort)
{
    CSL_CPSW_TSCONFIG timeSyncCfg;
    uint32_t portNum = ENET_MACPORT_NORM(macPort);

    CSL_CPSW_getPortTimeSyncConfig(regs, portNum + 1U, &timeSyncCfg);

    timeSyncCfg.tsTxHostEnable   = FALSE;
    timeSyncCfg.tsRxAnnexDEnable = FALSE;
    timeSyncCfg.tsRxAnnexEEnable = FALSE;
    timeSyncCfg.tsRxAnnexFEnable = FALSE;
    timeSyncCfg.tsTxAnnexDEnable = FALSE;
    timeSyncCfg.tsTxAnnexEEnable = FALSE;
    timeSyncCfg.tsTxAnnexFEnable = FALSE;
    timeSyncCfg.tsMsgTypeEnable  = FALSE;

    CSL_CPSW_setPortTimeSyncConfig(regs, portNum + 1U, &timeSyncCfg);
}

static void CpswMacPort_setShortIpgCfg(CSL_Xge_cpswRegs *regs,
                                       Enet_MacPort macPort,
                                       const CpswMacPort_TxShortIpgCfg *shortIpgCfg)
{
    uint32_t portNum = ENET_MACPORT_NORM(macPort);

    if (shortIpgCfg->txShortGapLimitEn)
    {
        CSL_CPGMAC_SL_enableTxShortGapLimit(regs, portNum);
    }
    else
    {
        CSL_CPGMAC_SL_disableTxShortGapLimit(regs, portNum);
    }

    if (shortIpgCfg->txShortGapEn)
    {
        CSL_CPGMAC_SL_enableTxShortGap(regs, portNum);
    }
    else
    {
        CSL_CPGMAC_SL_disableTxShortGap(regs, portNum);
    }
}

static void CpswMacPort_getShortIpgCfg(CSL_Xge_cpswRegs *regs,
                                       Enet_MacPort macPort,
                                       CpswMacPort_TxShortIpgCfg *shortIpgCfg)
{
    uint32_t portNum = ENET_MACPORT_NORM(macPort);

    shortIpgCfg->txShortGapEn = (CSL_CPGMAC_SL_isTxShortGapEnabled(regs, portNum) == TRUE);
    shortIpgCfg->txShortGapLimitEn = (CSL_CPGMAC_SL_isTxShortGapLimitEnabled(regs, portNum) == TRUE);
}

#if ENET_CFG_IS_ON(CPSW_MACPORT_SGMII)
#if ENET_CFG_IS_ON(DEV_ERROR)
static int32_t CpswMacPort_isSgmiiSupported(CSL_CpsgmiiRegs *sgmiiRegs,
                                            Enet_MacPort macPort)
{
    CSL_SGMII_VERSION version;
    uint32_t portNum = ENET_MACPORT_NORM(macPort);
    uint32_t portId = ENET_MACPORT_ID(macPort);
    int32_t status = ENET_ENOTSUPPORTED;

    ENETTRACE_VAR(portId);
    CSL_SGMII_getVersionInfo(sgmiiRegs, portNum, &version);

    if ((version.ident_val == CPSW_MACPORT_SGMII_VER_TX_ID_J7X) &&
        (version.major_version == CPSW_MACPORT_SGMII_VER_REVMAJ_J7X) &&
        (version.minor_version == CPSW_MACPORT_SGMII_VER_REVMIN_J7X) &&
        (version.rtl_version == CPSW_MACPORT_SGMII_VER_REVRTL))
    {
        status = ENET_SOK;
    }

    /* The SGMII registers will be reset to zero if the SERDES clock is not initialized
     * i.e. the SERDES must be configured for the SGMII module to be configured */
    if (version.major_version == 0U)
    {
        ENETTRACE_ERR("MAC %u: SGMII port not ready, SERDES PLL not locked\n", portId);
        Enet_devAssert(false);
        status = ENET_EUNEXPECTED;
    }

    return status;
}
#endif

static void CpswMacPort_resetSgmiiPort(CSL_CpsgmiiRegs *sgmiiRegs,
                                       Enet_MacPort macPort)
{
    uint32_t portNum = ENET_MACPORT_NORM(macPort);

    CSL_SGMII_startRxTxSoftReset(sgmiiRegs, portNum);
    CSL_SGMII_endRxTxSoftReset(sgmiiRegs, portNum);

    /* Wait till software reset is complete. SGMII reset is expected
     * to happen immediately */
    while (CSL_SGMII_getRxTxSoftResetStatus(sgmiiRegs, portNum) != 0U)
    {
         EnetUtils_delay(0U);
    }
}

static int32_t CpswMacPort_enableSgmiiLoopback(CSL_Xge_cpswRegs *regs,
                                               CSL_CpsgmiiRegs *sgmiiRegs,
                                               Enet_MacPort macPort)
{
    uint32_t portNum = ENET_MACPORT_NORM(macPort);
    int32_t status = ENET_SOK;

    CSL_SGMII_disableAutoNegotiation(sgmiiRegs, portNum);
    CSL_SGMII_startRxTxSoftReset(sgmiiRegs, portNum);
    CSL_SGMII_enableLoopback(sgmiiRegs, portNum);
    CSL_SGMII_endRxTxSoftReset(sgmiiRegs, portNum);

    /* Software reset should happen immediately, hence don't wait for
     * completion, just check if it's done */
    if (CSL_SGMII_getRxTxSoftResetStatus(sgmiiRegs, portNum) != 0U)
    {
        status = ENET_EFAIL;
    }

    if (status == ENET_SOK)
    {
        CSL_CPGMAC_SL_enableLoopback(regs, portNum);
    }

    return status;
}

static int32_t CpswMacPort_setSgmiiInterface(CSL_Xge_cpswRegs *regs,
                                             CSL_CpsgmiiRegs *sgmiiRegs,
                                             Enet_MacPort macPort,
                                             EnetMac_SgmiiMode sgmiiMode,
                                             const EnetMacPort_LinkCfg *linkCfg)
{
    uint32_t portNum = ENET_MACPORT_NORM(macPort);
    uint32_t portId = ENET_MACPORT_ID(macPort);
    uint32_t macControl;
    int32_t status = ENET_SOK;

    ENETTRACE_VAR(portId);
    macControl = CSL_CPGMAC_SL_getMacControlReg(regs, portNum);

    /* Clear fields not supported by hardware */
    CSL_FINS(macControl, XGE_CPSW_PN_MAC_CONTROL_REG_EXT_RX_FLOW_EN, 0U);
    CSL_FINS(macControl, XGE_CPSW_PN_MAC_CONTROL_REG_EXT_TX_FLOW_EN, 0U);

    /* In all SGMII modes EXT_EN bit in the CONTROL register must be set to
     * allow the speed and duplexity to be set by the signals from the CPSGMII */
    CSL_FINS(macControl, XGE_CPSW_PN_MAC_CONTROL_REG_EXT_EN, 1U);

    CSL_SGMII_DisableTestPattern(sgmiiRegs, portNum);
    CSL_SGMII_disableMasterMode(sgmiiRegs, portNum);
    CSL_SGMII_disableLoopback(sgmiiRegs, portNum);
    CSL_SGMII_disableAutoNegotiation(sgmiiRegs, portNum);

    /* In case of SGMII, the MDIO controls the auto-negotiation of the PHY to the
     * other side of the fiber or wire (ie. to the remote system). The CPSGMII is
     * the link to the PHY.  The PHY auto-negotiates with the other side and
     * then the CPSGMII auto-negotiates with the PHY. */

    /* Confirm SERDES PLL is locked before configuring the port */
    if (CSL_SGMII_getSerdesPLLLockStatus(sgmiiRegs, portNum) == 0U)
    {
        ENETTRACE_ERR("MAC %u: SERDES PLL is not locked\n", portId);
        status = ENET_EUNEXPECTED;
    }

    if (status == ENET_SOK)
    {
        status = CpswMacPort_configSgmii(sgmiiRegs, sgmiiMode, macPort, linkCfg);
        ENETTRACE_ERR_IF(status != ENET_SOK,
                         "MAC %u: Failed to config SGMII interface: %d\n", portId, status);
    }

    if (status == ENET_SOK)
    {
        CSL_CPGMAC_SL_setMacControlReg(regs, portNum, macControl);
    }

    ENETTRACE_ERR_IF(status != ENET_SOK,
                     "MAC %u: Failed to set SGMII interface: %d\n", portId, status);

    return status;
}

static int32_t CpswMacPort_configSgmii(CSL_CpsgmiiRegs *sgmiiRegs,
                                       EnetMac_SgmiiMode sgmiiMode,
                                       Enet_MacPort macPort,
                                       const EnetMacPort_LinkCfg *linkCfg)
{
    CSL_SGMII_ADVABILITY sgmiiAdvAbility;
    CSL_SGMII_STATUS sgmiiStatus;
    uint32_t portNum = ENET_MACPORT_NORM(macPort);
    uint32_t portId = ENET_MACPORT_ID(macPort);
    int32_t status = ENET_SOK;

    ENETTRACE_VAR(portId);
    memset(&sgmiiAdvAbility, 0, sizeof(sgmiiAdvAbility));

    if (sgmiiMode == ENET_MAC_SGMIIMODE_FIBER_WITH_PHY)
    {
        ENETTRACE_DBG("MAC %u: Configuring SGMII in FIBER_WITH_PHY mode\n", portId);

        /* In fiber mode Advertise full-duplex only */
        sgmiiAdvAbility.duplexMode = CSL_SGMII_FULL_DUPLEX;
        sgmiiAdvAbility.sgmiiMode  = CSL_SGMII_MODE_FIBER;

        CSL_SGMII_setAdvAbility(sgmiiRegs, portNum, &sgmiiAdvAbility);
        CSL_SGMII_enableAutoNegotiation(sgmiiRegs, portNum);
    }
    else if (sgmiiMode == ENET_MAC_SGMIIMODE_SGMII_WITH_PHY)
    {
        ENETTRACE_DBG("MAC %u: Configuring SGMII in SGMII_WITH_PHY mode\n", portId);

        /* Set highest speed when auto-negotiating with PHY. We don't need to configure
         * user provided speed/duplexity here as it is set in PHY and we auto-negotiate
         * with PHY.  For example, app wants to set 100Mbps full-duplex, it will be set
         * in PHY and PHY would auto-negotiate with with remote PHY with 100Mbps or lower
         * which will be fine with SGMII as we are at highest config. */
        sgmiiAdvAbility.duplexMode = CSL_SGMII_FULL_DUPLEX;
        sgmiiAdvAbility.linkSpeed  = CSL_SGMII_1000_MBPS;
        sgmiiAdvAbility.bLinkUp    = 1U;
        sgmiiAdvAbility.sgmiiMode  = CSL_SGMII_MODE_SGMII;

        CSL_SGMII_setAdvAbility(sgmiiRegs, portNum, &sgmiiAdvAbility);

        /* Note - For SGMII connection with PHY, auto negotiate should always be a '1'
         * (master/slave should be selected) */
        CSL_SGMII_enableAutoNegotiation(sgmiiRegs, portNum);
    }
    else if (sgmiiMode == ENET_MAC_SGMIIMODE_SGMII_AUTONEG_MASTER)
    {
        ENETTRACE_DBG("MAC %u: Configure SGMII in SGMII_AUTONEG_MASTER mode\n", portId);

        /* For SGMII master, advertise full-duplex gigabit */
        sgmiiAdvAbility.linkSpeed  = CSL_SGMII_1000_MBPS;
        sgmiiAdvAbility.duplexMode = CSL_SGMII_FULL_DUPLEX;
        sgmiiAdvAbility.bLinkUp    = 1U;
        sgmiiAdvAbility.sgmiiMode  = CSL_SGMII_MODE_SGMII;

        CSL_SGMII_setAdvAbility(sgmiiRegs, portNum, &sgmiiAdvAbility);
        CSL_SGMII_enableMasterMode(sgmiiRegs, portNum);
        CSL_SGMII_enableAutoNegotiation(sgmiiRegs, portNum);
    }
    else if (sgmiiMode == ENET_MAC_SGMIIMODE_SGMII_AUTONEG_SLAVE)
    {
        ENETTRACE_DBG("MAC %u: Configure SGMII in SGMII_AUTONEG_SLAVE mode\n", portId);

        /* To write 1 to tx_config_reg[0] bit, we pass empty ability structure */
        sgmiiAdvAbility.sgmiiMode  = CSL_SGMII_MODE_SGMII;

        CSL_SGMII_setAdvAbility(sgmiiRegs, portNum, &sgmiiAdvAbility);
        CSL_SGMII_disableMasterMode(sgmiiRegs, portNum);
        CSL_SGMII_enableAutoNegotiation(sgmiiRegs, portNum);
    }
    else if (sgmiiMode == ENET_MAC_SGMIIMODE_SGMII_FORCEDLINK)
    {
        ENETTRACE_DBG("MAC %u: Configure SGMII in SGMII_FORCEDLINK mode\n", portId);

        CpswMacPort_mapSgmiiLinkCfg(&sgmiiAdvAbility, linkCfg);
        sgmiiAdvAbility.bLinkUp   = 1U;
        sgmiiAdvAbility.sgmiiMode = CSL_SGMII_MODE_SGMII;

        CSL_SGMII_setAdvAbility(sgmiiRegs, portNum, &sgmiiAdvAbility);
        CSL_SGMII_enableMasterMode(sgmiiRegs, portNum);
        CSL_SGMII_disableAutoNegotiation(sgmiiRegs, portNum);

        /* Wait for SGMII link */
        do
        {
            CSL_SGMII_getStatus(sgmiiRegs, portNum, &sgmiiStatus);
        }
        while (sgmiiStatus.bIsLinkUp != 1U);

        status = ENET_SOK;
    }
    else
    {
        ENETTRACE_ERR("MAC %u: Invalid SGMII mode config\n", portId);
        status = ENET_EINVALIDPARAMS;
    }

    return status;
}

static void CpswMacPort_mapSgmiiLinkCfg(CSL_SGMII_ADVABILITY *sgmiiAdvAbility,
                                        const EnetMacPort_LinkCfg *linkCfg)
{
    /* Set SGMII duplexity config */
    if (linkCfg->duplexity == ENET_DUPLEX_HALF)
    {
        sgmiiAdvAbility->duplexMode = CSL_SGMII_HALF_DUPLEX;
    }
    else
    {
        sgmiiAdvAbility->duplexMode = CSL_SGMII_FULL_DUPLEX;
    }

    /* Set SGMII speed config */
    if (linkCfg->speed == ENET_SPEED_10MBIT)
    {
        sgmiiAdvAbility->linkSpeed  = CSL_SGMII_10_MBPS;
    }
    else if (linkCfg->speed == ENET_SPEED_100MBIT)
    {
        sgmiiAdvAbility->linkSpeed  = CSL_SGMII_100_MBPS;
    }
    else
    {
        sgmiiAdvAbility->linkSpeed  = CSL_SGMII_1000_MBPS;
    }
}

static bool CpswMacPort_getSgmiiStatus(CSL_CpsgmiiRegs *sgmiiRegs,
                                       Enet_MacPort macPort)
{
    CSL_SGMII_STATUS sgmiiStatus;
    uint32_t portNum = ENET_MACPORT_NORM(macPort);

    CSL_SGMII_getStatus(sgmiiRegs, portNum, &sgmiiStatus);

    return (sgmiiStatus.bIsLinkUp != 0U);
}

static int32_t CpswMacPort_checkSgmiiAutoNegStatus(CSL_CpsgmiiRegs *sgmiiRegs,
                                                   Enet_MacPort macPort)
{
    CSL_SGMII_STATUS sgmiiStatus;
    uint32_t portNum = ENET_MACPORT_NORM(macPort);
    uint32_t portId = ENET_MACPORT_ID(macPort);
    int32_t status = ENET_SOK;

    ENETTRACE_VAR(portId);
    /* Wait for SGMII Autonegotiation to complete without error */
    do
    {
        CSL_SGMII_getStatus(sgmiiRegs, portNum, &sgmiiStatus);
        if (sgmiiStatus.bIsAutoNegError != 0U)
        {
            /* Auto-negotiation error */
            ENETTRACE_ERR("MAC %u: SGMII auto-negotiation failed: %d\n", portId, status);
            status = ENET_EFAIL;
            break;
        }
    }
    while (sgmiiStatus.bIsAutoNegComplete != 1U);

    /* Check SGMII link status */
    if (status == ENET_SOK)
    {
        /* Link indicator is not valid until the lock status bit is asserted, so
         * check for lock first */
        if (CSL_SGMII_getSerdesPLLLockStatus(sgmiiRegs, portNum) != 1U)
        {
            ENETTRACE_ERR("MAC %u: SGMII SERDES PLL not locked: %d\n", portId, status);
            status = ENET_EUNEXPECTED;
        }
        else
        {
            /* Wait for SGMII link */
            do
            {
                CSL_SGMII_getStatus(sgmiiRegs, portNum, &sgmiiStatus);
            }
            while (sgmiiStatus.bIsLinkUp != 1U);
        }
    }

    return status;
}

static int32_t CpswMacPort_checkSgmiiStatus(CSL_CpsgmiiRegs *sgmiiRegs,
                                            Enet_MacPort macPort)
{
    CSL_SGMII_ADVABILITY sgmiiAdvAbility;
    uint32_t portNum = ENET_MACPORT_NORM(macPort);
    uint32_t portId = ENET_MACPORT_ID(macPort);
    int32_t status;

    ENETTRACE_VAR(portId);
    if (CSL_SGMII_isAutoNegotiationEnabled(sgmiiRegs, portNum) == 1U)
    {
        status = CpswMacPort_checkSgmiiAutoNegStatus(sgmiiRegs, macPort);
        if (status == ENET_SOK)
        {
            if (CSL_SGMII_isMasterModeEnabled(sgmiiRegs, portNum) == 1U)
            {
                /* In SGMII AUTONEG with MASTER SLAVE config, the master tells the slave
                 * the rate so the slave would auto negotiate to master config. Hence we
                 * report master configured speed/duplexity */
                CSL_SGMII_getAdvAbility(sgmiiRegs, portNum, &sgmiiAdvAbility);
            }
            else
            {
                /* Get link partner speed/duplexity */
                CSL_SGMII_getLinkPartnerAdvAbility(sgmiiRegs, portNum, &sgmiiAdvAbility);
            }

            ENETTRACE_INFO("MAC %u: SGMII link parter config port: link %s: %s %s\n",
                           portId, sgmiiAdvAbility.bLinkUp ? "up" : "down",
                           CpswMacPort_gSgmiiSpeedNames[sgmiiAdvAbility.linkSpeed],
                           CpswMacPort_gSgmiiDuplexNames[sgmiiAdvAbility.duplexMode]);
        }
        else
        {
            ENETTRACE_ERR("MAC %u: SGMII auto-neggotiation failed: %d\n", portId, status);
        }
    }
    else
    {
        /* Forced or loopback mode */
        status = ENET_SOK;
    }

    return status;
}

static int32_t CpswMacPort_enableSgmiiPort(CSL_Xge_cpswRegs *regs,
                                           CSL_CpsgmiiRegs *sgmiiRegs,
                                           Enet_MacPort macPort,
                                           const EnetMacPort_Interface *mii,
                                           const EnetMacPort_LinkCfg *linkCfg)
{
    uint32_t portId = ENET_MACPORT_ID(macPort);
    uint32_t portNum = ENET_MACPORT_NORM(macPort);
    uint32_t extCtrl;
    uint32_t macControl;
    int32_t status = ENET_SOK;
    bool forced = true;

    ENETTRACE_VAR(portId);
    macControl = CSL_CPGMAC_SL_getMacControlReg(regs, portNum);

    /* Disable all fields, enable as needed based on link config */
    CSL_FINS(macControl, XGE_CPSW_PN_MAC_CONTROL_REG_FULLDUPLEX, 0U);
    CSL_FINS(macControl, XGE_CPSW_PN_MAC_CONTROL_REG_GMII_EN, 0U);
    CSL_FINS(macControl, XGE_CPSW_PN_MAC_CONTROL_REG_GIG, 0U);
    CSL_FINS(macControl, XGE_CPSW_PN_MAC_CONTROL_REG_XGMII_EN, 0U);
    CSL_FINS(macControl, XGE_CPSW_PN_MAC_CONTROL_REG_IFCTL_A, 0U);
    CSL_FINS(macControl, XGE_CPSW_PN_MAC_CONTROL_REG_IFCTL_B, 0U);

    if (!EnetMacPort_isSgmii(mii) &&
        !EnetMacPort_isQsgmii(mii))
    {
        status = ENET_EINVALIDPARAMS;
    }

    /* Set speed mode */
    if (status == ENET_SOK)
    {
        extCtrl = CSL_FEXT(macControl, XGE_CPSW_PN_MAC_CONTROL_REG_EXT_EN);
        forced = (extCtrl == TRUE) ? false : true;

        if (linkCfg->speed == ENET_SPEED_1GBIT)
        {
            CSL_FINS(macControl, XGE_CPSW_PN_MAC_CONTROL_REG_GIG, 1U);
        }

        CSL_FINS(macControl, XGE_CPSW_PN_MAC_CONTROL_REG_GMII_EN, 1U);

        status = CpswMacPort_checkSgmiiStatus(sgmiiRegs, macPort);
    }

    /* Half-duplex mode is supported only at 10/100 Mbps */
    if (status == ENET_SOK)
    {
        if ((linkCfg->duplexity == ENET_DUPLEX_HALF) &&
            (linkCfg->speed != ENET_SPEED_10MBIT) &&
            (linkCfg->speed != ENET_SPEED_100MBIT))
        {
            ENETTRACE_ERR("MAC %u: 1Gbps half-duplex is not supported\n", portId);
            status = ENET_EINVALIDPARAMS;
        }
    }

    /* Set duplexity, speed related fields in MAC control */
    if (status == ENET_SOK)
    {
        if (forced && (linkCfg->duplexity == ENET_DUPLEX_FULL))
        {
            CSL_FINS(macControl, XGE_CPSW_PN_MAC_CONTROL_REG_FULLDUPLEX, 1U);
        }

        /* Set interface layer, sublayer, speed */
        CSL_CPGMAC_SL_setMacControlReg(regs, portNum, macControl);
    }

    ENETTRACE_ERR_IF(status != ENET_SOK,
                     "MAC %u: Failed to enable MAC port: %d\n", portId, status);

    return status;
}
#endif
