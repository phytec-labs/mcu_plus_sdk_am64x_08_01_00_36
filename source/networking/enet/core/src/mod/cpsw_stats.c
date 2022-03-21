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
 * \file  cpsw_stats.c
 *
 * \brief This file contains the implementation of the CPSW statistics module.
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
#include <include/mod/cpsw_stats.h>
#include <priv/core/enet_trace_priv.h>
#include <priv/mod/cpsw_stats_priv.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* Supported AM64x version */
#define CPSW_STATS_VER_REVMAJ_AM64X          (0x00000001U)
#define CPSW_STATS_VER_REVMIN_AM64X          (0x00000003U)
#define CPSW_STATS_VER_REVRTL_AM64X          (0x00000001U)
#define CPSW_STATS_VER_ID_AM64X              (0x00006BA8U)

/* Supported AM273X version */
#define CPSW_STATS_VER_REVMAJ_AM273X           (0x00000001U)
#define CPSW_STATS_VER_REVMIN_AM273X           (0x00000002U)
#define CPSW_STATS_VER_REVRTL_AM273X           (0x00000000U)
#define CPSW_STATS_VER_ID_AM273X               (0x00006B90U)

/* Supported AM263X version */
#define CPSW_STATS_VER_REVMAJ_AM263X           (0x00000001U)
#define CPSW_STATS_VER_REVMIN_AM263X           (0x00000002U)
#define CPSW_STATS_VER_REVRTL_AM263X           (0x00000000U)
#define CPSW_STATS_VER_ID_AM263X               (0x00006B90U)

/* Supported AWR294X version */
#define CPSW_STATS_VER_REVMAJ_AWR294X           (0x00000001U)
#define CPSW_STATS_VER_REVMIN_AWR294X           (0x00000002U)
#define CPSW_STATS_VER_REVRTL_AWR294X           (0x00000000U)
#define CPSW_STATS_VER_ID_AWR294X               (0x00006B90U)

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

#if ENET_CFG_IS_ON(DEV_ERROR)
static int32_t CpswStats_isSupported(CSL_Xge_cpswRegs *regs);
#endif

static void CpswStats_resetHostStats(CpswStats_Handle hStats);

static void CpswStats_resetMacStats(CpswStats_Handle hStats,
                                    Enet_MacPort macPort);

static void CpswStats_readHostStats(CpswStats_Handle hStats);

static void CpswStats_readMacStats(CpswStats_Handle hStats,
                                   Enet_MacPort macPort);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

#if ENET_CFG_IS_ON(DEV_ERROR)
/*! \brief CPSW statistics versions supported by this driver. */
static CSL_CPSW_VERSION CpswStats_gSupportedVer[] =
{
    {   /* AM64x */
        .majorVer = CPSW_STATS_VER_REVMAJ_AM64X,
        .minorVer = CPSW_STATS_VER_REVMIN_AM64X,
        .rtlVer   = CPSW_STATS_VER_REVRTL_AM64X,
        .id       = CPSW_STATS_VER_ID_AM64X,
    },
    {   /* AM273X */
        .majorVer = CPSW_STATS_VER_REVMAJ_AM273X,
        .minorVer = CPSW_STATS_VER_REVMIN_AM273X,
        .rtlVer   = CPSW_STATS_VER_REVRTL_AM273X,
        .id       = CPSW_STATS_VER_ID_AM273X,
    },
	{   /* AM263X */
        .majorVer = CPSW_STATS_VER_REVMAJ_AM263X,
        .minorVer = CPSW_STATS_VER_REVMIN_AM263X,
        .rtlVer   = CPSW_STATS_VER_REVRTL_AM263X,
        .id       = CPSW_STATS_VER_ID_AM263X,
    },
    {   /* AWR294X */
        .majorVer = CPSW_STATS_VER_REVMAJ_AWR294X,
        .minorVer = CPSW_STATS_VER_REVMIN_AWR294X,
        .rtlVer   = CPSW_STATS_VER_REVRTL_AWR294X,
        .id       = CPSW_STATS_VER_ID_AWR294X,
    },
};

/* Public statistics IOCTL validation data. */
static Enet_IoctlValidate gCpswStats_ioctlValidate[] =
{
    ENET_IOCTL_VALID_PRMS(ENET_STATS_IOCTL_GET_HOSTPORT_STATS,
                          0U,
                          sizeof(CpswStats_PortStats)),

    ENET_IOCTL_VALID_PRMS(ENET_STATS_IOCTL_GET_MACPORT_STATS,
                          sizeof(Enet_MacPort),
                          sizeof(CpswStats_PortStats)),

    ENET_IOCTL_VALID_PRMS(ENET_STATS_IOCTL_RESET_HOSTPORT_STATS,
                          0U,
                          0U),

    ENET_IOCTL_VALID_PRMS(ENET_STATS_IOCTL_RESET_MACPORT_STATS,
                          sizeof(Enet_MacPort),
                          0U),
};

/* Private statistics IOCTL validation data. */
static Enet_IoctlValidate gCpswStats_privIoctlValidate[] =
{
    ENET_IOCTL_VALID_PRMS(CPSW_STATS_IOCTL_SYNC,
                          0U,
                          0U),
};
#endif

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

int32_t CpswStats_open(EnetMod_Handle hMod,
                       Enet_Type enetType,
                       uint32_t instId,
                       const void *cfg,
                       uint32_t cfgSize)
{
    CpswStats_Handle hStats = (CpswStats_Handle)hMod;
    CSL_Xge_cpswRegs *regs = (CSL_Xge_cpswRegs *)hMod->virtAddr;
    CSL_CPSW_PORTSTAT portStat;
    uint32_t i;
#if ENET_CFG_IS_ON(DEV_ERROR)
    int32_t status = ENET_SOK;
#endif

    Enet_devAssert(cfgSize == 0U,
                   "Invalid stats config params size %u (expected %u)\n",
                   cfgSize, 0U);

    Enet_devAssert(regs != NULL, "CPSW stats regs address is not valid\n");

    /* Check supported stats module versions */
#if ENET_CFG_IS_ON(DEV_ERROR)
    status = CpswStats_isSupported(regs);
    Enet_devAssert(status == ENET_SOK, "Stats version is not supported\n");
#endif

    memset(&portStat, 0, sizeof(CSL_CPSW_PORTSTAT));

    /* For now, use statistics block memories from stats module object */
    hStats->hostPortStats = &hStats->hostPortStatsMem;
    hStats->macPortStats = &hStats->macPortStatsMem[0U];
    hStats->macPortNum = ENET_ARRAYSIZE(hStats->macPortStatsMem);

    hStats->enetType = enetType;

    /* Enable statistics on all applicable ports */
    portStat.p0StatEnable = true;
    if (enetType == ENET_CPSW_9G)
    {
        portStat.p1StatEnable = true;
        portStat.p2StatEnable = true;
        portStat.p3StatEnable = true;
        portStat.p4StatEnable = true;
        portStat.p5StatEnable = true;
        portStat.p6StatEnable = true;
        portStat.p7StatEnable = true;
        portStat.p8StatEnable = true;
    }
    else if (enetType == ENET_CPSW_5G)
    {
        portStat.p1StatEnable = true;
        portStat.p2StatEnable = true;
        portStat.p3StatEnable = true;
        portStat.p4StatEnable = true;
    }
    else if (enetType == ENET_CPSW_3G)
    {
        portStat.p1StatEnable = true;
        portStat.p2StatEnable = true;
    }
    else
    {
        portStat.p1StatEnable = true;
    }

    CSL_CPSW_setPortStatsEnableReg(regs, &portStat);

    /* Clear all statistics counters */
    CpswStats_resetHostStats(hStats);
    for (i = 0U; i < hStats->macPortNum; i++)
    {
        CpswStats_resetMacStats(hStats, ENET_MACPORT_DENORM(i));
    }

    return ENET_SOK;
}

int32_t CpswStats_rejoin(EnetMod_Handle hMod,
                         Enet_Type enetType,
                         uint32_t instId)
{
    CpswStats_Handle hStats = (CpswStats_Handle)hMod;

    /* For now, use statistics block memories from stats module object */
    hStats->hostPortStats = &hStats->hostPortStatsMem;
    hStats->macPortStats = &hStats->macPortStatsMem[0U];
    hStats->macPortNum = ENET_ARRAYSIZE(hStats->macPortStatsMem);

    hStats->enetType = enetType;

    /* Clear only host statistics counters */
    CpswStats_resetHostStats(hStats);

    return ENET_SOK;
}

void CpswStats_close(EnetMod_Handle hMod)
{
    /* Nothing to do */
}

int32_t CpswStats_ioctl(EnetMod_Handle hMod,
                        uint32_t cmd,
                        Enet_IoctlPrms *prms)
{
    CpswStats_Handle hStats = (CpswStats_Handle)hMod;
    CSL_Xge_cpswRegs *regs = (CSL_Xge_cpswRegs *)hMod->virtAddr;
    int32_t status = ENET_SOK;

#if ENET_CFG_IS_ON(DEV_ERROR)
    /* Validate CPSW statistics IOCTL parameters */
    if (ENET_IOCTL_GET_PER(cmd) == ENET_IOCTL_PER_CPSW)
    {
        if (ENET_IOCTL_GET_TYPE(cmd) == ENET_IOCTL_TYPE_PUBLIC)
        {
            status = Enet_validateIoctl(cmd, prms,
                                        gCpswStats_ioctlValidate,
                                        ENET_ARRAYSIZE(gCpswStats_ioctlValidate));
        }
        else
        {
            status = Enet_validateIoctl(cmd, prms,
                                        gCpswStats_privIoctlValidate,
                                        ENET_ARRAYSIZE(gCpswStats_privIoctlValidate));
        }

        ENETTRACE_ERR_IF(status != ENET_SOK, "IOCTL 0x%08x params are not valid\n", cmd);
    }
#endif

    if (status == ENET_SOK)
    {
        switch (cmd)
        {
            case ENET_STATS_IOCTL_GET_VERSION:
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

            case ENET_STATS_IOCTL_PRINT_REGS:
            {
                CSL_Xge_cpswRegs *regs = (CSL_Xge_cpswRegs *)hMod->virtAddr;
                ENETTRACE_VAR(regs);
                ENETTRACE_INFO("STATS: 0x%08x\n", regs->STAT_PORT_EN_REG);
            }
            break;

            case ENET_STATS_IOCTL_GET_HOSTPORT_STATS:
            {
                status = Enet_checkOutArgs(prms, sizeof(CpswStats_PortStats));
                if (status == ENET_SOK)
                {
                    CpswStats_PortStats *portStats = (CpswStats_PortStats *)prms->outArgs;

                    CpswStats_readHostStats(hStats);
                    memcpy(portStats, hStats->hostPortStats, sizeof(CpswStats_PortStats));
                }
                else
                {
                    ENETTRACE_ERR("Invalid GET_HOSTPORT_STATS params\n");
                }
            }
            break;

            case ENET_STATS_IOCTL_GET_MACPORT_STATS:
            {
                status = Enet_checkInOutArgs(prms, sizeof(Enet_MacPort), sizeof(CpswStats_PortStats));
                if (status == ENET_SOK)
                {
                    Enet_MacPort *macPort = (Enet_MacPort *)prms->inArgs;
                    CpswStats_PortStats *portStats = (CpswStats_PortStats *)prms->outArgs;
                    uint32_t portNum = ENET_MACPORT_NORM(*macPort);

                    CpswStats_readMacStats(hStats, *macPort);
                    memcpy(portStats, &hStats->macPortStats[portNum], sizeof(CpswStats_PortStats));
                }
                else
                {
                    ENETTRACE_ERR("Invalid GET_MACPORT_STATS params\n");
                }
            }
            break;

            case ENET_STATS_IOCTL_RESET_HOSTPORT_STATS:
            {
                CpswStats_resetHostStats(hStats);
            }
            break;

            case ENET_STATS_IOCTL_RESET_MACPORT_STATS:
            {
                status = Enet_checkInArgs(prms, sizeof(Enet_MacPort));
                if (status == ENET_SOK)
                {
                    Enet_MacPort *macPort = (Enet_MacPort *)prms->inArgs;

                    CpswStats_resetMacStats(hStats, *macPort);
                }
                else
                {
                    ENETTRACE_ERR("Invalid RESET_MACPORT_STATS params\n");
                }
            }
            break;

            case CPSW_STATS_IOCTL_SYNC:
            {
                uint32_t i;

                CpswStats_readHostStats(hStats);
                for (i = 0U; i < hStats->macPortNum; i++)
                {
                    CpswStats_readMacStats(hStats, ENET_MACPORT_DENORM(i));
                }
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
static int32_t CpswStats_isSupported(CSL_Xge_cpswRegs *regs)
{
    CSL_CPSW_VERSION version;
    uint32_t i;
    int32_t status = ENET_ENOTSUPPORTED;

    CSL_CPSW_getCpswVersionInfo(regs, &version);

    for (i = 0U; i < ENET_ARRAYSIZE(CpswStats_gSupportedVer); i++)
    {
        if ((version.majorVer == CpswStats_gSupportedVer[i].majorVer) &&
            (version.minorVer == CpswStats_gSupportedVer[i].minorVer) &&
            (version.rtlVer == CpswStats_gSupportedVer[i].rtlVer) &&
            (version.id == CpswStats_gSupportedVer[i].id))
        {
            status = ENET_SOK;
            break;
        }
    }

    return status;
}
#endif

static void CpswStats_resetHostStats(CpswStats_Handle hStats)
{
    CSL_Xge_cpswRegs *regs = (CSL_Xge_cpswRegs *)hStats->enetMod.virtAddr;
    CSL_CPSW_STATS portStats;

    Enet_devAssert(hStats->hostPortStats != NULL, "Invalid host port stats memory address\n");

    /* Clear local stats */
    memset(hStats->hostPortStats, 0, sizeof(CpswStats_PortStats));

    /* Clear hardware stats through a dummy read */
    memset(&portStats, 0, sizeof(CSL_CPSW_STATS));
    CSL_CPSW_getPortStats(regs, 0U, &portStats);
}

static void CpswStats_resetMacStats(CpswStats_Handle hStats,
                                    Enet_MacPort macPort)
{
    CSL_Xge_cpswRegs *regs = (CSL_Xge_cpswRegs *)hStats->enetMod.virtAddr;
    CSL_CPSW_STATS portStats;
    uint32_t portNum = ENET_MACPORT_NORM(macPort);

    Enet_devAssert(portNum < hStats->macPortNum, "Invalid port number %u\n", portNum);
    Enet_devAssert(hStats->macPortStats != NULL, "Invalid MAC port stats memory address\n");

    /* Clear local stats */
    memset(&hStats->macPortStats[portNum], 0, sizeof(CpswStats_PortStats));

    /* Clear hardware stats through a dummy read */
    memset(&portStats, 0, sizeof(CSL_CPSW_STATS));
    CSL_CPSW_getPortStats(regs, portNum + 1U, &portStats);
}

static void CpswStats_readHostStats(CpswStats_Handle hStats)
{
    CSL_Xge_cpswRegs *regs = (CSL_Xge_cpswRegs *)hStats->enetMod.virtAddr;
    CSL_CPSW_STATS portStats;
    uint64_t *stats64;
    uint32_t *stats32 = (uint32_t *)&portStats;
    uint32_t i;

    Enet_devAssert(hStats->hostPortStats != NULL, "Invalid host port stats memory address\n");

    /* CSL blindly reads all registers in the statistics block regardless
     * of whether they are applicable or not to a CPSW instance type */
    memset(&portStats, 0, sizeof(CSL_CPSW_STATS));
    CSL_CPSW_getPortStats(regs, 0U, &portStats);

    stats64 = &hStats->hostPortStats->val[0U];
    for (i = 0U; i < CPSW_STATS_BLOCK_ELEM_NUM; i++)
    {
        stats64[i] += stats32[i];
    }

    /* Clear reserved fields */
    switch (hStats->enetType)
    {
        case ENET_CPSW_2G:
        {
            CpswStats_HostPort_2g *hostStats2g = NULL;
            hostStats2g = (CpswStats_HostPort_2g *)stats64;

            hostStats2g->reserved4 = 0U;
            hostStats2g->reserved6 = 0U;
            hostStats2g->reserved8 = 0U;
            memset(hostStats2g->reserved17to25, 0, sizeof(hostStats2g->reserved17to25));
            memset(hostStats2g->reserved52to95, 0, sizeof(hostStats2g->reserved52to95));
            memset(hostStats2g->reserved97to128, 0, sizeof(hostStats2g->reserved97to128));
        }
        break;

        case ENET_CPSW_3G:
        case ENET_CPSW_5G:
        case ENET_CPSW_9G:
        {
            CpswStats_HostPort_Ng *hostStatsNg = NULL;
            hostStatsNg = (CpswStats_HostPort_Ng *)stats64;

            hostStatsNg->reserved4 = 0U;
            hostStatsNg->reserved6 = 0U;
            hostStatsNg->reserved8 = 0U;
            memset(hostStatsNg->reserved17to25, 0, sizeof(hostStatsNg->reserved17to25));
            memset(hostStatsNg->reserved57to80, 0, sizeof(hostStatsNg->reserved57to80));
            memset(hostStatsNg->reserved87to95, 0, sizeof(hostStatsNg->reserved87to95));
        }
        break;

        default:
        {
            Enet_devAssert(false, "Unsupported peripheral type %u\n", hStats->enetType);
        }
        break;
    }
}

static void CpswStats_readMacStats(CpswStats_Handle hStats,
                                   Enet_MacPort macPort)
{
    CSL_Xge_cpswRegs *regs = (CSL_Xge_cpswRegs *)hStats->enetMod.virtAddr;
    CSL_CPSW_STATS portStats;
    uint32_t portNum = ENET_MACPORT_NORM(macPort);
    uint32_t portId = ENET_MACPORT_ID(macPort);
    uint64_t *stats64;
    uint32_t *stats32 = (uint32_t *)&portStats;
    uint32_t i;

    ENETTRACE_VAR(portId);
    Enet_devAssert(portNum < hStats->macPortNum, "Invalid MAC port %u\n", portId);
    Enet_devAssert(hStats->macPortStats != NULL, "Invalid MAC port stats memory address\n");

    /* CSL blindly reads all registers in the statistics block regardless
     * of whether they are applicable or not to a CPSW instance type */
    memset(&portStats, 0, sizeof(CSL_CPSW_STATS));
    CSL_CPSW_getPortStats(regs, portNum + 1U, &portStats);

    stats64 = &hStats->macPortStats[portNum].val[0U];
    for (i = 0U; i < CPSW_STATS_BLOCK_ELEM_NUM; i++)
    {
        stats64[i] += stats32[i];
    }

    /* Clear reserved fields */
    switch (hStats->enetType)
    {
        case ENET_CPSW_2G:
        {
            CpswStats_MacPort_2g *macStats2g = (CpswStats_MacPort_2g *)stats64;

            memset(macStats2g->reserved52to95, 0, sizeof(macStats2g->reserved52to95));
        }
        break;

        case ENET_CPSW_3G:
        case ENET_CPSW_5G:
        case ENET_CPSW_9G:
        {
            CpswStats_MacPort_Ng *macStatsNg = (CpswStats_MacPort_Ng *)stats64;

            memset(macStatsNg->reserved57to80, 0, sizeof(macStatsNg->reserved57to80));
            memset(macStatsNg->reserved87to95, 0, sizeof(macStatsNg->reserved87to95));
        }
        break;

        default:
        {
            Enet_devAssert(false, "Unsupported peripheral type %u\n", hStats->enetType);
        }
        break;
    }
}
