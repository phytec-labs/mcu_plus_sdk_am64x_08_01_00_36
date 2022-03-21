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
 * \file  cpsw_hostport.c
 *
 * \brief This file contains the implementation of the CPSW host port module.
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
#include <include/mod/cpsw_hostport.h>
#include <priv/core/enet_trace_priv.h>
#include <priv/mod/cpsw_hostport_priv.h>
#include <priv/mod/cpsw_clks.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* Supported AM64x version */
#define CPSW_HOSTPORT_VER_REVMAJ_AM64X       (0x00000001U)
#define CPSW_HOSTPORT_VER_REVMIN_AM64X       (0x00000003U)
#define CPSW_HOSTPORT_VER_REVRTL_AM64X       (0x00000001U)
#define CPSW_HOSTPORT_VER_ID_AM64X           (0x00006BA8U)

/* Supported AM273X version */
#define CPSW_HOSTPORT_VER_REVMAJ_AM273X        (0x00000001U)
#define CPSW_HOSTPORT_VER_REVMIN_AM273X        (0x00000002U)
#define CPSW_HOSTPORT_VER_REVRTL_AM273X        (0x00000000U)
#define CPSW_HOSTPORT_VER_ID_AM273X            (0x00006B90U)

/* Supported AM263X version */
#define CPSW_HOSTPORT_VER_REVMAJ_AM263X        (0x00000001U)
#define CPSW_HOSTPORT_VER_REVMIN_AM263X        (0x00000002U)
#define CPSW_HOSTPORT_VER_REVRTL_AM263X        (0x00000000U)
#define CPSW_HOSTPORT_VER_ID_AM263X            (0x00006B90U)

/* Supported AWR294X version */
#define CPSW_HOSTPORT_VER_REVMAJ_AWR294X        (0x00000001U)
#define CPSW_HOSTPORT_VER_REVMIN_AWR294X        (0x00000002U)
#define CPSW_HOSTPORT_VER_REVRTL_AWR294X        (0x00000000U)
#define CPSW_HOSTPORT_VER_ID_AWR294X            (0x00006B90U)

/*! \brief Host port start offset in CSL register overlay. */
#define CPSW_HOSTPORT_START_REG_OFFSET        (CSL_XGE_CPSW_P0_CONTROL_REG)

/*! \brief Host port end offset in CSL register overlay. */
#define CPSW_HOSTPORT_END_REG_OFFSET          (CSL_XGE_CPSW_P0_HOST_BLKS_PRI_REG)

/*! \brief Default value used for host port RX MTU. */
#define CPSW_HOSTPORT_RX_MTU_DEFAULT          (1518U)

/*! \brief Rate limiting count to transfer rate divider. */
#define CPSW_HOSTPORT_RATELIM_DIV             (32768U)

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

#if ENET_CFG_IS_ON(DEV_ERROR)
static int32_t CpswHostPort_isSupported(CSL_Xge_cpswRegs *regs);
#endif

static void CpswHostPort_printRegs(CSL_Xge_cpswRegs *regs);

static int32_t CpswHostPort_setTxPriority(CSL_Xge_cpswRegs *regs,
                                          EnetPort_PriorityMap *txPriority);

static void CpswHostPort_getTxPriority(CSL_Xge_cpswRegs *regs,
                                       EnetPort_PriorityMap *txPriority);

static int32_t CpswHostPort_setRxPriority(CSL_Xge_cpswRegs *regs,
                                          EnetPort_PriorityMap *rxPriority);

static void CpswHostPort_getRxPriority(CSL_Xge_cpswRegs *regs,
                                       EnetPort_PriorityMap *rxPriority);

static int32_t CpswHostPort_setDscpPriority(CSL_Xge_cpswRegs *regs,
                                            EnetPort_DscpPriorityMap *dscpPriority);

static void CpswHostPort_getDscpPriority(CSL_Xge_cpswRegs *regs,
                                         EnetPort_DscpPriorityMap *dscpPriority);

#if ENET_CFG_IS_ON(CPSW_HOSTPORT_TRAFFIC_SHAPING)
static uint64_t CpswHostPort_mapBwToCnt(uint64_t rateInBps,
                                        uint32_t cppiClkFreqHz);

static uint64_t CpswHostPort_mapCntToBw(uint32_t cntVal,
                                        uint32_t cppiClkFreqHz);

static int32_t CpswHostPort_setTrafficShaping(CSL_Xge_cpswRegs *regs,
                                              const EnetPort_TrafficShapingCfg *cfg,
                                              uint32_t cppiClkFreqHz);

static void CpswHostPort_disableTrafficShaping(CSL_Xge_cpswRegs *regs);

static int32_t CpswHostPort_getTrafficShaping(CSL_Xge_cpswRegs *regs,
                                              EnetPort_TrafficShapingCfg *cfg,
                                              uint32_t cppiClkFreqHz);
#endif

static void CpswHostPort_getMaxLen(CSL_Xge_cpswRegs *regs,
                                   EnetPort_MaxLen *maxLen);

static void CpswHostPort_getFifoStats(CSL_Xge_cpswRegs *regs,
                                      CpswHostPort_FifoStats *fifoStats);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

#if ENET_CFG_IS_ON(DEV_ERROR)
/*! \brief CPSW hostport versions supported by this driver. */
static CSL_CPSW_VERSION CpswHostPort_gSupportedVer[] =
{
    {   /* AM64x */
        .majorVer = CPSW_HOSTPORT_VER_REVMAJ_AM64X,
        .minorVer = CPSW_HOSTPORT_VER_REVMIN_AM64X,
        .rtlVer   = CPSW_HOSTPORT_VER_REVRTL_AM64X,
        .id       = CPSW_HOSTPORT_VER_ID_AM64X,
    },
    {   /* AM273X */
        .majorVer = CPSW_HOSTPORT_VER_REVMAJ_AM273X,
        .minorVer = CPSW_HOSTPORT_VER_REVMIN_AM273X,
        .rtlVer   = CPSW_HOSTPORT_VER_REVRTL_AM273X,
        .id       = CPSW_HOSTPORT_VER_ID_AM273X,
    },
	{   /* AM263X */
        .majorVer = CPSW_HOSTPORT_VER_REVMAJ_AM263X,
        .minorVer = CPSW_HOSTPORT_VER_REVMIN_AM263X,
        .rtlVer   = CPSW_HOSTPORT_VER_REVRTL_AM263X,
        .id       = CPSW_HOSTPORT_VER_ID_AM263X,
    },
    {   /* AWR294X */
        .majorVer = CPSW_HOSTPORT_VER_REVMAJ_AWR294X,
        .minorVer = CPSW_HOSTPORT_VER_REVMIN_AWR294X,
        .rtlVer   = CPSW_HOSTPORT_VER_REVRTL_AWR294X,
        .id       = CPSW_HOSTPORT_VER_ID_AWR294X,
    },
};

/* Public host port IOCTL validation data. */
static Enet_IoctlValidate gCpswHostPort_ioctlValidate[] =
{
    ENET_IOCTL_VALID_PRMS(CPSW_HOSTPORT_IOCTL_GET_FIFO_STATS,
                          0U,
                          sizeof(CpswHostPort_FifoStats)),

    ENET_IOCTL_VALID_PRMS(CPSW_HOSTPORT_GET_FLOW_ID_OFFSET,
                          0U,
                          sizeof(uint32_t)),
};

/* Private host port IOCTL validation data. */
static Enet_IoctlValidate gCpswHostPort_privIoctlValidate[] =
{
    ENET_IOCTL_VALID_PRMS(CPSW_HOSTPORT_SET_FLOW_ID_OFFSET,
                          sizeof(uint32_t),
                          0U),
};
#endif

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

void CpswHostPort_initCfg(CpswHostPort_Cfg *hostPortCfg)
{
    hostPortCfg->crcType           = ENET_CRC_ETHERNET;
    hostPortCfg->removeCrc         = false;
    hostPortCfg->padShortPacket    = true; /* Short packets were dropped by default */
    hostPortCfg->passCrcErrors     = false;
    hostPortCfg->passPriorityTaggedUnchanged = false;
    hostPortCfg->rxVlanRemapEn     = false;
    hostPortCfg->rxDscpIPv4RemapEn = false;
    hostPortCfg->rxDscpIPv6RemapEn = false;
    hostPortCfg->rxMtu             = CPSW_HOSTPORT_RX_MTU_DEFAULT;
    hostPortCfg->vlanCfg.portPri   = 0U;
    hostPortCfg->vlanCfg.portCfi   = 0U;
    hostPortCfg->vlanCfg.portVID   = 0U;
    hostPortCfg->rxPriorityType    = ENET_INGRESS_PRI_TYPE_FIXED;
    hostPortCfg->txPriorityType    = ENET_EGRESS_PRI_TYPE_FIXED;

    /* Keep checksum offload enabled by default as no side-effect if app doesn't use it */
    hostPortCfg->csumOffloadEn     = false;
}

int32_t CpswHostPort_open(EnetMod_Handle hMod,
                          Enet_Type enetType,
                          uint32_t instId,
                          const void *cfg,
                          uint32_t cfgSize)
{
    CpswHostPort_Handle hPort = (CpswHostPort_Handle)hMod;
    const CpswHostPort_Cfg *hostPortCfg = (const CpswHostPort_Cfg *)cfg;
    CSL_Xge_cpswRegs *regs = (CSL_Xge_cpswRegs *)hMod->virtAddr;
    CSL_CPSW_CONTROL control;
    CSL_CPSW_PTYPE pType;
    CSL_CPSW_CPPI_P0_CONTROL cppiP0ControlCfg;
    uint32_t status = ENET_SOK;

    Enet_devAssert(cfgSize == sizeof(CpswHostPort_Cfg),
                   "Invalid host port config params size %u (expected %u)\n",
                   cfgSize, sizeof(CpswHostPort_Cfg));

    Enet_devAssert(regs != NULL, "CPSW hostport regs address is not valid\n");

    /* Check supported host port module versions */
#if ENET_CFG_IS_ON(DEV_ERROR)
    status = CpswHostPort_isSupported(regs);
    Enet_devAssert(status == ENET_SOK, "Host port version is not supported\n");
#endif

    /* Save peripheral info to use it later to query SoC parameters */
    hPort->enetType = enetType;
    hPort->instId = instId;

    if (hostPortCfg->crcType == ENET_CRC_ETHERNET)
    {
        CSL_CPSW_disableP0TxCastagnoliCRC(regs);
    }
    else
    {
        CSL_CPSW_enableP0TxCastagnoliCRC(regs);
    }

    CSL_CPSW_getCpswControlReg(regs, &control);
    control.p0Enable       = false;
    control.p0PassPriTag   = hostPortCfg->passPriorityTaggedUnchanged;
#if ENET_CFG_IS_ON(DISABLE_CRC_STRIP)
    ENETTRACE_WARN_IF(hostPortCfg->removeCrc,
        "ETHFW-1705 - CRC removal is not allowed, CRC will be passed in packet buffer\n");
    control.p0TxCrcRemove  = false;
#else
    control.p0TxCrcRemove  = hostPortCfg->removeCrc;
#endif
    control.p0RxPad        = hostPortCfg->padShortPacket;
    control.p0RxPassCrcErr = hostPortCfg->passCrcErrors;
    CSL_CPSW_setCpswControlReg(regs, &control);

    CSL_CPSW_setPort0VlanReg(regs,
                             hostPortCfg->vlanCfg.portVID,
                             hostPortCfg->vlanCfg.portCfi,
                             hostPortCfg->vlanCfg.portPri);

    CSL_CPSW_setPort0RxMaxLen(regs, hostPortCfg->rxMtu);

    CSL_CPSW_getPTypeReg(regs, &pType);
    if (hostPortCfg->txPriorityType == ENET_EGRESS_PRI_TYPE_FIXED)
    {
        pType.port0PriorityTypeEscalateEnable = FALSE;
    }
    else
    {
        pType.port0PriorityTypeEscalateEnable = TRUE;
    }

    CSL_CPSW_setPTypeReg(regs, &pType);

    CSL_CPSW_getCppiP0Control(regs, &cppiP0ControlCfg);

    if (hostPortCfg->csumOffloadEn)
    {
        cppiP0ControlCfg.p0RxChksumEn = TRUE;
    }
    else
    {
        cppiP0ControlCfg.p0RxChksumEn = FALSE;
    }

    cppiP0ControlCfg.p0RxRemapVlan     = hostPortCfg->rxVlanRemapEn ? TRUE : FALSE;
    cppiP0ControlCfg.p0RxRemapDscpIpv4 = hostPortCfg->rxDscpIPv4RemapEn ? TRUE : FALSE;
    cppiP0ControlCfg.p0RxRemapDscpIpv6 = hostPortCfg->rxDscpIPv6RemapEn ? TRUE : FALSE;

    CSL_CPSW_setCppiP0Control(regs, &cppiP0ControlCfg);

    return status;
}

int32_t CpswHostPort_rejoin(EnetMod_Handle hMod,
                            Enet_Type enetType,
                            uint32_t instId)
{
    CpswHostPort_Handle hPort = (CpswHostPort_Handle)hMod;

    /* Save peripheral info to use it later to query SoC parameters */
    hPort->enetType = enetType;
    hPort->instId = instId;

    return ENET_SOK;
}

void CpswHostPort_close(EnetMod_Handle hMod)
{
    CSL_Xge_cpswRegs *regs = (CSL_Xge_cpswRegs *)hMod->virtAddr;

    Enet_devAssert(regs != NULL, "CPSW hostport regs address is not valid\n");

    /* Disable host port */
    CSL_CPSW_disablePort0(regs);
}

int32_t CpswHostPort_ioctl(EnetMod_Handle hMod,
                           uint32_t cmd,
                           Enet_IoctlPrms *prms)
{
#if ENET_CFG_IS_ON(CPSW_HOSTPORT_TRAFFIC_SHAPING)
    CpswHostPort_Handle hPort = (CpswHostPort_Handle)hMod;
#endif
    CSL_Xge_cpswRegs *regs = (CSL_Xge_cpswRegs *)hMod->virtAddr;
    int32_t status = ENET_SOK;

#if ENET_CFG_IS_ON(DEV_ERROR)
    /* Validate CPSW host port IOCTL parameters */
    if (ENET_IOCTL_GET_PER(cmd) == ENET_IOCTL_PER_CPSW)
    {
        if (ENET_IOCTL_GET_TYPE(cmd) == ENET_IOCTL_TYPE_PUBLIC)
        {
            status = Enet_validateIoctl(cmd, prms,
                                        gCpswHostPort_ioctlValidate,
                                        ENET_ARRAYSIZE(gCpswHostPort_ioctlValidate));
        }
        else
        {
            status = Enet_validateIoctl(cmd, prms,
                                        gCpswHostPort_privIoctlValidate,
                                        ENET_ARRAYSIZE(gCpswHostPort_privIoctlValidate));
        }

        ENETTRACE_ERR_IF(status != ENET_SOK, "IOCTL 0x%08x params are not valid\n", cmd);
    }
#endif

    if (status == ENET_SOK)
    {
        Enet_devAssert(regs != NULL, "CPSW hostport regs address is not valid\n");

        switch (cmd)
        {
            case ENET_HOSTPORT_IOCTL_GET_VERSION:
            {
                Enet_Version *version = (Enet_Version *)prms->outArgs;
                CSL_CPSW_VERSION ver;

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

            case ENET_HOSTPORT_IOCTL_PRINT_REGS:
            {
                CpswHostPort_printRegs(regs);
            }
            break;

            case ENET_HOSTPORT_IOCTL_ENABLE:
            {
                CSL_CPSW_enablePort0(regs);
            }
            break;

            case ENET_HOSTPORT_IOCTL_DISABLE:
            {
                CSL_CPSW_disablePort0(regs);
            }
            break;

            case ENET_HOSTPORT_IOCTL_SET_INGRESS_DSCP_PRI_MAP:
            {
                EnetPort_DscpPriorityMap *dscpPriority = (EnetPort_DscpPriorityMap *)prms->inArgs;

                status = CpswHostPort_setDscpPriority(regs, dscpPriority);
                ENETTRACE_ERR_IF(status != ENET_SOK, "Failed to set DSCP priority: %d\n", status);
            }
            break;

            case ENET_HOSTPORT_IOCTL_GET_INGRESS_DSCP_PRI_MAP:
            {
                EnetPort_DscpPriorityMap *dscpPriority = (EnetPort_DscpPriorityMap *)prms->outArgs;

                CpswHostPort_getDscpPriority(regs, dscpPriority);
            }
            break;

            case ENET_HOSTPORT_IOCTL_SET_PRI_REGEN_MAP:
            {
                EnetPort_PriorityMap *rxPriority = (EnetPort_PriorityMap *)prms->inArgs;

                status = CpswHostPort_setRxPriority(regs, rxPriority);
                ENETTRACE_ERR_IF(status != ENET_SOK, "Failed to set RX priority: %d\n", status);
            }
            break;

            case ENET_HOSTPORT_IOCTL_GET_PRI_REGEN_MAP:
            {
                EnetPort_PriorityMap *rxPriority = (EnetPort_PriorityMap *)prms->outArgs;

                CpswHostPort_getRxPriority(regs, rxPriority);
            }
            break;

            case ENET_HOSTPORT_IOCTL_SET_EGRESS_QOS_PRI_MAP:
            {
                EnetPort_PriorityMap *txPriority = (EnetPort_PriorityMap *)prms->inArgs;

                status = CpswHostPort_setTxPriority(regs, txPriority);
                ENETTRACE_ERR_IF(status != ENET_SOK, "failed to set TX priority: %d\n", status);
            }
            break;

            case ENET_HOSTPORT_IOCTL_GET_EGRESS_QOS_PRI_MAP:
            {
                EnetPort_PriorityMap *txPriority = (EnetPort_PriorityMap *)prms->outArgs;

                CpswHostPort_getTxPriority(regs, txPriority);
            }
            break;

            case ENET_HOSTPORT_IOCTL_ENABLE_INGRESS_TRAFFIC_SHAPING:
            {
#if ENET_CFG_IS_ON(CPSW_HOSTPORT_TRAFFIC_SHAPING)
                EnetPort_TrafficShapingCfg *shapingCfg = (EnetPort_TrafficShapingCfg *)prms->inArgs;
                uint32_t cppiClkFreqHz;

                cppiClkFreqHz = EnetSoc_getClkFreq(hPort->enetType, hPort->instId, CPSW_CPPI_CLK);

                status = CpswHostPort_setTrafficShaping(regs, shapingCfg, cppiClkFreqHz);
                ENETTRACE_ERR_IF(status != ENET_SOK, "Failed to set traffic shaping: %d\n", status);
#else
                status = ENET_ENOTSUPPORTED;
#endif
            }
            break;

            case ENET_HOSTPORT_IOCTL_DISABLE_INGRESS_TRAFFIC_SHAPING:
            {
#if ENET_CFG_IS_ON(CPSW_HOSTPORT_TRAFFIC_SHAPING)
                CpswHostPort_disableTrafficShaping(regs);
#else
                status = ENET_ENOTSUPPORTED;
#endif
            }
            break;

            case ENET_HOSTPORT_IOCTL_GET_INGRESS_TRAFFIC_SHAPING:
            {
#if ENET_CFG_IS_ON(CPSW_HOSTPORT_TRAFFIC_SHAPING)
                EnetPort_TrafficShapingCfg *shapingCfg = (EnetPort_TrafficShapingCfg *)prms->outArgs;
                uint32_t cppiClkFreqHz;

                cppiClkFreqHz = EnetSoc_getClkFreq(hPort->enetType, hPort->instId, CPSW_CPPI_CLK);

                status = CpswHostPort_getTrafficShaping(regs, shapingCfg, cppiClkFreqHz);
                ENETTRACE_ERR_IF(status != ENET_SOK, "Failed to get traffic shaping: %d\n", status);
#else
                status = ENET_ENOTSUPPORTED;
#endif
            }
            break;

            case ENET_HOSTPORT_IOCTL_GET_MAXLEN:
            {
                EnetPort_MaxLen *maxLen = (EnetPort_MaxLen *)prms->inArgs;

                CpswHostPort_getMaxLen(regs, maxLen);
            }
            break;

            case ENET_HOSTPORT_IS_CSUM_OFFLOAD_ENABLED:
            {
                bool *csumEn = (bool *)prms->outArgs;
                CSL_CPSW_CPPI_P0_CONTROL cppiP0ControlCfg;

                CSL_CPSW_getCppiP0Control(regs, &cppiP0ControlCfg);
                *csumEn = (cppiP0ControlCfg.p0RxChksumEn != 0U);
            }
            break;

            case CPSW_HOSTPORT_IOCTL_GET_FIFO_STATS:
            {
                CpswHostPort_FifoStats *fifoStats = (CpswHostPort_FifoStats *)prms->outArgs;

                CpswHostPort_getFifoStats(regs, fifoStats);
            }
            break;

            case CPSW_HOSTPORT_GET_FLOW_ID_OFFSET:
            {
                uint32_t *flowIdOffset = (uint32_t *)prms->outArgs;

                *flowIdOffset = CSL_CPSW_getPort0FlowIdOffset(regs);
            }
            break;

            case CPSW_HOSTPORT_SET_FLOW_ID_OFFSET:
            {
                uint32_t *flowIdOffset = (uint32_t *)prms->inArgs;

                CSL_CPSW_setPort0FlowIdOffset(regs, *flowIdOffset);
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
static int32_t CpswHostPort_isSupported(CSL_Xge_cpswRegs *regs)
{
    CSL_CPSW_VERSION version;
    uint32_t i;
    int32_t status = ENET_ENOTSUPPORTED;

    CSL_CPSW_getCpswVersionInfo(regs, &version);

    for (i = 0U; i < ENET_ARRAYSIZE(CpswHostPort_gSupportedVer); i++)
    {
        if ((version.majorVer == CpswHostPort_gSupportedVer[i].majorVer) &&
            (version.minorVer == CpswHostPort_gSupportedVer[i].minorVer) &&
            (version.rtlVer == CpswHostPort_gSupportedVer[i].rtlVer) &&
            (version.id == CpswHostPort_gSupportedVer[i].id))
        {
            status = ENET_SOK;
            break;
        }
    }

    return status;
}
#endif

static void CpswHostPort_printRegs(CSL_Xge_cpswRegs *regs)
{
    uint32_t *regAddr = (uint32_t *)((uintptr_t)regs + CPSW_HOSTPORT_START_REG_OFFSET);
    uint32_t regIdx = 0;

    while ((uintptr_t)regAddr <= ((uintptr_t)regs + CPSW_HOSTPORT_END_REG_OFFSET))
    {
        if (*regAddr != 0U)
        {
            ENETTRACE_INFO("HOSTPORT: %u: 0x%08x\n", regIdx, *regAddr);
        }

        regAddr++;
        regIdx++;
    }
}

static int32_t CpswHostPort_setTxPriority(CSL_Xge_cpswRegs *regs,
                                          EnetPort_PriorityMap *txPriority)
{
    int32_t status = ENET_SOK;
    uint32_t i;

    for (i = 0U; i < ENET_PRI_NUM; i++)
    {
        if (txPriority->priorityMap[i] > ENET_PRI_MAX)
        {
            ENETTRACE_ERR("Invalid priority map %u -> %u\n", i, txPriority->priorityMap[i]);
            status = ENET_EINVALIDPARAMS;
            break;
        }
    }

    if (status == ENET_SOK)
    {
        CSL_CPSW_setPortTxPriMapReg(regs, 0U, txPriority->priorityMap);
    }

    return status;
}

static void CpswHostPort_getTxPriority(CSL_Xge_cpswRegs *regs,
                                       EnetPort_PriorityMap *txPriority)
{
    CSL_CPSW_getPortTxPriMapReg(regs, 0U, txPriority->priorityMap);
}

static int32_t CpswHostPort_setRxPriority(CSL_Xge_cpswRegs *regs,
                                          EnetPort_PriorityMap *rxPriority)
{
    int32_t status = ENET_SOK;
    uint32_t i;

    for (i = 0U; i < ENET_PRI_NUM; i++)
    {
        if (rxPriority->priorityMap[i] > ENET_PRI_MAX)
        {
            ENETTRACE_ERR("Invalid priority map %u -> %u\n", i, rxPriority->priorityMap[i]);
            status = ENET_EINVALIDPARAMS;
            break;
        }
    }

    if (status == ENET_SOK)
    {
        CSL_CPSW_setPortRxPriMapReg(regs, 0U, rxPriority->priorityMap);
    }

    return status;
}

static void CpswHostPort_getRxPriority(CSL_Xge_cpswRegs *regs,
                                       EnetPort_PriorityMap *rxPriority)
{
    CSL_CPSW_getPortRxPriMapReg(regs, 0U, rxPriority->priorityMap);
}

static int32_t CpswHostPort_setDscpPriority(CSL_Xge_cpswRegs *regs,
                                            EnetPort_DscpPriorityMap *dscpPriority)
{
    CSL_CPSW_PORT_CONTROL control;
    int32_t status = ENET_SOK;
    uint32_t i;

    for (i = 0U; i < ENET_TOS_PRI_NUM; i++)
    {
        if (dscpPriority->tosMap[i] > ENET_PRI_MAX)
        {
            ENETTRACE_ERR("Invalid priority map %u -> %u\n", i, dscpPriority->tosMap[i]);
            status = ENET_EINVALIDPARAMS;
            break;
        }
    }

    if (status == ENET_SOK)
    {
        CSL_CPSW_setPortRxDscpMap(regs, 0U, dscpPriority->tosMap);

        CSL_CPSW_getPortControlReg(regs, 0U, &control);

        control.dscpIpv4Enable = dscpPriority->dscpIPv4En;
        control.dscpIpv6Enable = dscpPriority->dscpIPv6En;

        CSL_CPSW_setPortControlReg(regs, 0U, &control);
    }

    return status;
}

static void CpswHostPort_getDscpPriority(CSL_Xge_cpswRegs *regs,
                                         EnetPort_DscpPriorityMap *dscpPriority)
{
    CSL_CPSW_PORT_CONTROL control;

    CSL_CPSW_getPortRxDscpMap(regs, 0U, dscpPriority->tosMap);

    CSL_CPSW_getPortControlReg(regs, 0U, &control);

    dscpPriority->dscpIPv4En = control.dscpIpv4Enable;
    dscpPriority->dscpIPv6En = control.dscpIpv6Enable;
}

#if ENET_CFG_IS_ON(CPSW_HOSTPORT_TRAFFIC_SHAPING)
static uint64_t CpswHostPort_mapBwToCnt(uint64_t rateInBps,
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
    return ENET_DIV_ROUNDUP(rateInBps * CPSW_HOSTPORT_RATELIM_DIV, cppiClkFreqHz);
}

static uint64_t CpswHostPort_mapCntToBw(uint32_t cntVal,
                                        uint32_t cppiClkFreqHz)
{
    /*
     * CIR or EIR count value to transfer rate equation:
     *
     *                         count value * CPPI_ICLK freq (MHz)
     * Transfer rate (Mbps) = ------------------------------------
     *                                      32768
     */
    return ((uint64_t)cntVal * cppiClkFreqHz) / CPSW_HOSTPORT_RATELIM_DIV;
}

static int32_t CpswHostPort_setTrafficShaping(CSL_Xge_cpswRegs *regs,
                                              const EnetPort_TrafficShapingCfg *cfg,
                                              uint32_t cppiClkFreqHz)
{
    uint64_t cirBps, eirBps;
    uint64_t cir, eir;
    uint32_t i;
    int32_t status = ENET_SOK;
    bool rateLimEnabled = false;

    for (i = 0U; i < ENET_PRI_NUM; i++)
    {
        cirBps = cfg->rates[i].committedRateBitsPerSec;
        eirBps = cfg->rates[i].excessRateBitsPerSec;
        cir = CpswHostPort_mapBwToCnt(cirBps, cppiClkFreqHz);
        eir = CpswHostPort_mapBwToCnt(eirBps, cppiClkFreqHz);

        /* CIR must be non-zero if EIR is non-zero */
        if ((cir == 0ULL) && (eir != 0ULL))
        {
            ENETTRACE_ERR("EIR is enabled (%llubps = %llu) but CIR is not (%llubps = %llu) for priority %u\n",
                          eirBps, eir, cirBps, cir, i);
            status = ENET_EINVALIDPARAMS;
            break;
        }

        /* Rate limit must be the highest priority channels */
        if ((cir == 0ULL) && rateLimEnabled)
        {
            ENETTRACE_ERR("Rate limiting disabled for priority %u\n", i);
            status = ENET_EINVALIDPARAMS;
            break;
        }

        /* Out of range */
        if ((cir > 0x0FFFFFFFULL) || (eir > 0x0FFFFFFFULL))
        {
            ENETTRACE_ERR("Invalid CIR=%llu (%llubps) EIR=%llu (%llubps) for priority %u\n",
                          cir, cirBps, eir, eirBps, i);
            status = ENET_EINVALIDPARAMS;
            break;
        }

        ENETTRACE_DBG("Rate limiting %s for priority %u, CIR=%ubps (%llu) EIR=%ubps (%llu)\n",
                      (cir != 0ULL) ? "enabled" : "disabled",
                      i, cirBps, cir, eirBps, eir);

        CSL_CPSW_setCppiPriCirEir(regs, i, (uint32_t)cir, (uint32_t)eir);

        if (cir != 0ULL)
        {
            rateLimEnabled = true;
        }
    }

    return status;
}

static void CpswHostPort_disableTrafficShaping(CSL_Xge_cpswRegs *regs)
{
    uint32_t i;

    for (i = 0U; i < ENET_PRI_NUM; i++)
    {
        CSL_CPSW_setCppiPriCirEir(regs, i, 0U, 0U);
    }
}

static int32_t CpswHostPort_getTrafficShaping(CSL_Xge_cpswRegs *regs,
                                              EnetPort_TrafficShapingCfg *cfg,
                                              uint32_t cppiClkFreqHz)
{
    uint32_t cir, eir;
    uint64_t cirBps, eirBps;
    uint32_t i;
    int32_t status = ENET_SOK;

    for (i = 0U; i < ENET_PRI_NUM; i++)
    {
        CSL_CPSW_getCppiPriCirEir(regs, i, &cir, &eir);

        cirBps = CpswHostPort_mapCntToBw(cir, cppiClkFreqHz);
        eirBps = CpswHostPort_mapCntToBw(eir, cppiClkFreqHz);

        /* CIR must be non-zero if EIR is non-zero */
        if ((cirBps == 0ULL) && (eirBps != 0ULL))
        {
            ENETTRACE_ERR("EIR is enabled (%llubps = %u) but CIR is not (%llubps = %u) for priority %u\n",
                          eirBps, eir, cirBps, cir, i);
            status = ENET_EUNEXPECTED;
            break;
        }

        ENETTRACE_DBG("Rate limiting %s for priority %u, CIR=%llubps (%u) EIR=%llubps (%u)\n",
                      (cir != 0U) ? "enabled" : "disabled",
                      i, cirBps, cir, eirBps, eir);

        cfg->rates[i].committedRateBitsPerSec = cirBps;
        cfg->rates[i].excessRateBitsPerSec = eirBps;
    }

    return status;
}
#endif

static void CpswHostPort_getMaxLen(CSL_Xge_cpswRegs *regs,
                                   EnetPort_MaxLen *maxLen)
{
    uint32_t i;

    maxLen->mru = CSL_CPSW_getPort0RxMaxLen(regs);

    for (i = 0U; i < ENET_PRI_NUM; i++)
    {
        maxLen->mtu[i] = CSL_CPSW_getTxMaxLenPerPriority(regs, i);
    }
}

static void CpswHostPort_getFifoStats(CSL_Xge_cpswRegs *regs,
                                      CpswHostPort_FifoStats *fifoStats)
{
    CSL_CPSW_THRURATE thruRate;
    CSL_CPSW_CPPI_P0_FIFOSTATUS p0FifoStatus;

    CSL_CPSW_getPortBlockCountReg(regs, 0U,
                                  &fifoStats->rxBlockCountExpress,
                                  &fifoStats->rxBlockCountPreempt,
                                  &fifoStats->txBlockCount);

    CSL_CPSW_getThruRateReg(regs, &thruRate);

    fifoStats->rxThroughputRate = thruRate.cppiRxThruRate;

    CSL_CPSW_getP0FifoStatus(regs, &p0FifoStatus);

    fifoStats->txActiveFifo[0U] = p0FifoStatus.p0TxPriActivePri0;
    fifoStats->txActiveFifo[1U] = p0FifoStatus.p0TxPriActivePri1;
    fifoStats->txActiveFifo[2U] = p0FifoStatus.p0TxPriActivePri2;
    fifoStats->txActiveFifo[3U] = p0FifoStatus.p0TxPriActivePri3;
    fifoStats->txActiveFifo[4U] = p0FifoStatus.p0TxPriActivePri4;
    fifoStats->txActiveFifo[5U] = p0FifoStatus.p0TxPriActivePri5;
    fifoStats->txActiveFifo[6U] = p0FifoStatus.p0TxPriActivePri6;
    fifoStats->txActiveFifo[7U] = p0FifoStatus.p0TxPriActivePri7;
}
