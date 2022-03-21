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
 * \file  mdio.c
 *
 * \brief This file contains the implementation of the MDIO module.
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
#include <include/mod/mdio.h>
#include <priv/core/enet_trace_priv.h>
#include <priv/mod/mdio_priv.h>
#include <priv/mod/cpsw_clks.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/*! \brief Supported MDIO versions. */
#define MDIO_VER_MODID_J7X                    (0x00000007U)
#define MDIO_VER_REVMAJ_J7X                   (0x00000001U)
#define MDIO_VER_REVMIN_J7X                   (0x00000007U)
#define MDIO_VER_REVRTL_J7X                   (0x00000001U)

/*! \brief Supported ICSSG MDIO versions. */
#define ICSSG_MDIO_VER_MODID_J7X              (0x00000007U)
#define ICSSG_MDIO_VER_REVMAJ_J7X             (0x00000001U)
#define ICSSG_MDIO_VER_REVMIN_J7X             (0x00000007U)
#define ICSSG_MDIO_VER_REVRTL_J7X             (0x00000000U)

/*! \brief Max number of PHYs that can be monitored by MDIO in normal mode. */
#define MDIO_PHY_MONITOR_MAX                  (2U)

/*! \brief Default MDIO bus frequency. */
#define MDIO_MDIOBUS_DFLT_FREQ_HZ             (2200000U)

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

#if ENET_CFG_IS_ON(DEV_ERROR)
static int32_t Mdio_isSupported(CSL_mdioHandle mdioRegs);
#endif

static int32_t Mdio_setupNormalMode(CSL_mdioHandle mdioRegs,
                                    const Mdio_Cfg *cfg);

static void Mdio_setupStatusChangeMode(CSL_mdioHandle mdioRegs,
                                       const Mdio_Cfg *cfg);

static void Mdio_printRegs(CSL_mdioHandle mdioRegs);

#if ENET_CFG_IS_ON(MDIO_CLAUSE45)
static int32_t Mdio_readRegC45(CSL_mdioHandle mdioRegs,
                               uint32_t userCh,
                               uint32_t mmd,
                               uint8_t phyAddr,
                               uint16_t reg,
                               uint16_t *val);

static int32_t Mdio_writeRegC45(CSL_mdioHandle mdioRegs,
                                uint32_t userCh,
                                uint32_t mmd,
                                uint8_t phyAddr,
                                uint16_t reg,
                                uint16_t val);
#endif

static void Mdio_handleIntr(CSL_mdioHandle mdioRegs,
                            Mdio_Callbacks *callbacks);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

#if ENET_CFG_IS_ON(DEV_ERROR)
/*! \brief MDIO versions supported by this driver. */
static CSL_MDIO_VERSION gMdio_supportedVer[] =
{
    {   /* AM65xx and J7x devices */
        .modId  = MDIO_VER_MODID_J7X,
        .revMaj = MDIO_VER_REVMAJ_J7X,
        .revMin = MDIO_VER_REVMIN_J7X,
        .revRtl = MDIO_VER_REVRTL_J7X,
    },
    {   /* ICSSG MDIO on AM65xx and J7x devices */
        .modId  = ICSSG_MDIO_VER_MODID_J7X,
        .revMaj = ICSSG_MDIO_VER_REVMAJ_J7X,
        .revMin = ICSSG_MDIO_VER_REVMIN_J7X,
        .revRtl = ICSSG_MDIO_VER_REVRTL_J7X,
    },
};

/* Private MDIO IOCTL validation data. */
static Enet_IoctlValidate gMdio_privIoctlValidate[] =
{
    ENET_IOCTL_VALID_PRMS(MDIO_IOCTL_HANDLE_INTR,
                          sizeof(Mdio_Callbacks),
                          0U),
};
#endif

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

void Mdio_initCfg(Mdio_Cfg *mdioCfg)
{
    mdioCfg->mode               = MDIO_MODE_STATE_CHANGE_MON;
    mdioCfg->mdioBusFreqHz      = MDIO_MDIOBUS_DFLT_FREQ_HZ;
    mdioCfg->phyStatePollFreqHz = mdioCfg->mdioBusFreqHz;
    mdioCfg->pollEnMask         = ENET_MDIO_PHY_ADDR_MASK_ALL;
    mdioCfg->c45EnMask          = ENET_MDIO_PHY_ADDR_MASK_NONE;
    mdioCfg->isMaster           = true;
}

int32_t Mdio_open(EnetMod_Handle hMod,
                  Enet_Type enetType,
                  uint32_t instId,
                  const void *cfg,
                  uint32_t cfgSize)
{
    Mdio_Handle hMdio = (Mdio_Handle)hMod;
    const Mdio_Cfg *mdioCfg = (const Mdio_Cfg *)cfg;
    CSL_mdioHandle mdioRegs = (CSL_mdioHandle)hMod->virtAddr;
    uint32_t cppiClkFreqHz;
    uint32_t clkdiv;
    uint32_t ipgRatio;
    int32_t status = ENET_SOK;

    Enet_devAssert(cfgSize == sizeof(Mdio_Cfg),
                   "Invalid MDIO config params size %u (expected %u)\n",
                   cfgSize, sizeof(Mdio_Cfg));

    Enet_devAssert(mdioRegs != NULL, "MDIO reg address is not valid\n");

    /* Check supported MDIO module versions */
#if ENET_CFG_IS_ON(DEV_ERROR)
    status = Mdio_isSupported(mdioRegs);
    Enet_devAssert(status == ENET_SOK, "MDIO version is not supported\n");
#endif

    hMdio->isMaster = mdioCfg->isMaster;

    /* Perform global MDIO configuration only for module in master role */
    if (hMdio->isMaster)
    {
        /* Compute MDIO clock divider */
        if (status == ENET_SOK)
        {
            if (mdioCfg->mdioBusFreqHz != 0U)
            {
                cppiClkFreqHz = EnetSoc_getClkFreq(enetType, instId, CPSW_CPPI_CLK);

                clkdiv = (cppiClkFreqHz / mdioCfg->mdioBusFreqHz) - 1U;
                if (clkdiv > 0xFFFFU)
                {
                    ENETTRACE_ERR("unsupported clk div %u (CPPI %u Hz, MDIO bus %u Hz)\n",
                                  clkdiv, cppiClkFreqHz, mdioCfg->mdioBusFreqHz);
                    status = ENET_EFAIL;
                }

                ipgRatio = mdioCfg->mdioBusFreqHz / mdioCfg->phyStatePollFreqHz;
                if (ipgRatio > 0xFFU)
                {
                    ENETTRACE_ERR("unsupported IPG ratio %u (MDIO bus %u Hz, PHY poll %u Hz)\n",
                                  ipgRatio, mdioCfg->mdioBusFreqHz, mdioCfg->phyStatePollFreqHz);
                    status = ENET_EFAIL;
                }
            }
            else
            {
                ENETTRACE_ERR("invalid MDIO bus freq %u Hz\n", mdioCfg->mdioBusFreqHz);
                status = ENET_EINVALIDPARAMS;
            }
        }

        /* Setup MDIO mode: normal, monitor or manual mode */
        if (status == ENET_SOK)
        {
            CSL_MDIO_clearUnmaskedLinkStatusChangeInt(mdioRegs, 0U);
            CSL_MDIO_clearUnmaskedLinkStatusChangeInt(mdioRegs, 1U);

            switch (mdioCfg->mode)
            {
                case MDIO_MODE_NORMAL:
                    status = Mdio_setupNormalMode(mdioRegs, mdioCfg);
                    break;

                case MDIO_MODE_STATE_CHANGE_MON:
                    Mdio_setupStatusChangeMode(mdioRegs, mdioCfg);
                    break;

                case MDIO_MODE_MANUAL:
                    ENETTRACE_ERR("manual mode is not supported\n");
                    status = ENET_ENOTSUPPORTED;
                    break;
            }
        }

        /* Write MDIO configuration */
        if (status == ENET_SOK)
        {
            CSL_MDIO_setPollIPG(mdioRegs, (uint8_t)ipgRatio);
            CSL_MDIO_setClkDivVal(mdioRegs, (uint16_t)clkdiv);
            CSL_MDIO_enableStateMachine(mdioRegs);
#if ENET_CFG_IS_ON(MDIO_CLAUSE45)
            if (ENET_FEAT_IS_EN(hMod->features, MDIO_FEATURE_CLAUSE45))
            {
                CSL_MDIO_setClause45EnableMask(mdioRegs, mdioCfg->c45EnMask);
            }
#endif
        }
    }

    return status;
}

int32_t Mdio_rejoin(EnetMod_Handle hMod,
                    Enet_Type enetType,
                    uint32_t instId)
{
    return ENET_SOK;
}

void Mdio_close(EnetMod_Handle hMod)
{
    Mdio_Handle hMdio = (Mdio_Handle)hMod;
    CSL_mdioHandle mdioRegs = (CSL_mdioHandle)hMod->virtAddr;
    uint32_t i;

    Enet_devAssert(mdioRegs != NULL, "MDIO reg address is not valid\n");

    if (hMdio->isMaster)
    {
        for (i = 0U; i < MDIO_PHY_MONITOR_MAX; i++)
        {
            CSL_MDIO_disableLinkStatusChangeInterrupt(mdioRegs, i, 0U);
        }

        CSL_MDIO_disableStatusChangeModeInterrupt(mdioRegs);
        CSL_MDIO_disableStateMachine(mdioRegs);
        CSL_MDIO_clearUnmaskedLinkStatusChangeInt(mdioRegs, 0U);
        CSL_MDIO_clearUnmaskedLinkStatusChangeInt(mdioRegs, 1U);
    }
}

int32_t Mdio_ioctl(EnetMod_Handle hMod,
                   uint32_t cmd,
                   Enet_IoctlPrms *prms)
{
    CSL_mdioHandle mdioRegs = (CSL_mdioHandle)hMod->virtAddr;
    int32_t status = ENET_SOK;

#if ENET_CFG_IS_ON(DEV_ERROR)
    /* Validate MDIO IOCTL parameters */
    if (ENET_IOCTL_GET_PER(cmd) == ENET_IOCTL_PER_CPSW)
    {
        if (ENET_IOCTL_GET_TYPE(cmd) == ENET_IOCTL_TYPE_PRIVATE)
        {
            status = Enet_validateIoctl(cmd, prms,
                                        gMdio_privIoctlValidate,
                                        ENET_ARRAYSIZE(gMdio_privIoctlValidate));

            ENETTRACE_ERR_IF(status != ENET_SOK, "IOCTL 0x%08x params are not valid\n", cmd);
        }
    }
#endif

    if (status == ENET_SOK)
    {
        Enet_devAssert(mdioRegs != NULL, "MDIO reg address is not valid\n");

        switch (cmd)
        {
            case ENET_MDIO_IOCTL_GET_VERSION:
            {
                Enet_Version *version = (Enet_Version *)prms->outArgs;
                CSL_MDIO_VERSION ver;

                CSL_MDIO_getVersionInfo(mdioRegs, &ver);
                version->maj    = ver.revMaj;
                version->min    = ver.revMin;
                version->rtl    = ver.revRtl;
                version->id     = ver.modId;
                version->other1 = ver.scheme;
                version->other2 = ver.bu;
            }
            break;

            case ENET_MDIO_IOCTL_PRINT_REGS:
            {
                Mdio_printRegs(mdioRegs);
            }
            break;

            case ENET_MDIO_IOCTL_IS_ALIVE:
            {
                uint8_t *phyAddr = (uint8_t *)prms->inArgs;
                bool *alive = (bool *)prms->outArgs;

                *alive = (CSL_MDIO_isPhyAlive(mdioRegs, *phyAddr) == 0U) ? false : true;
            }
            break;

            case ENET_MDIO_IOCTL_IS_LINKED:
            {
                uint8_t *phyAddr = (uint8_t *)prms->inArgs;
                bool *linked = (bool *)prms->outArgs;

                *linked = (CSL_MDIO_isPhyLinked(mdioRegs, *phyAddr) == 0) ? false : true;
            }
            break;

            case ENET_MDIO_IOCTL_IS_POLL_ENABLED:
            {
                uint8_t *phyAddr = (uint8_t *)prms->inArgs;
                bool *enabled = (bool *)prms->outArgs;
                uint8_t monPhyAddr;
                uint32_t isStatusChangeMode;
                uint32_t pollMask = 0U;
                uint32_t i;

                isStatusChangeMode = CSL_MDIO_isStateChangeModeEnabled(mdioRegs);
                if (isStatusChangeMode == 1U)
                {
                    pollMask = CSL_MDIO_getPollEnableMask(mdioRegs);
                }
                else
                {
                    for (i = 0U; i < MDIO_PHY_MONITOR_MAX; i++)
                    {
                        monPhyAddr = CSL_MDIO_getLinkStatusChangePhyAddr(mdioRegs, i);
                        pollMask |= ENET_MDIO_PHY_ADDR_MASK(monPhyAddr);
                    }
                }

                *enabled = ENET_IS_BIT_SET(pollMask, *phyAddr);
            }
            break;

            case ENET_MDIO_IOCTL_C22_READ:
            {
                EnetMdio_C22ReadInArgs *inArgs = (EnetMdio_C22ReadInArgs *)prms->inArgs;
                uint16_t *val = (uint16_t *)prms->outArgs;
                uint32_t ack;
#if ENET_CFG_IS_ON(MDIO_CLAUSE45)
                uint32_t c45EnMask;

                if (ENET_FEAT_IS_EN(hMod->features, MDIO_FEATURE_CLAUSE45))
                {
                    c45EnMask = CSL_MDIO_getClause45EnableMask(mdioRegs);
                    if (ENET_IS_BIT_SET(c45EnMask, inArgs->phyAddr))
                    {
                        ENETTRACE_ERR("PHY %u is not configured for C22 access\n", inArgs->phyAddr);
                        status = ENET_EPERM;
                    }
                }
#endif

                if (status == ENET_SOK)
                {
                    ack = CSL_MDIO_phyRegRead2(mdioRegs,
                                               inArgs->group,
                                               inArgs->phyAddr,
                                               inArgs->reg,
                                               val);
                    status = (ack == TRUE) ? ENET_SOK : ENET_EFAIL;
                    ENETTRACE_ERR_IF(status != ENET_SOK,
                                     "failed to read PHY %u C22 reg %u: %d\n",
                                     inArgs->phyAddr, inArgs->reg, status);
                }
            }
            break;

            case ENET_MDIO_IOCTL_C22_WRITE:
            {
                EnetMdio_C22WriteInArgs *inArgs = (EnetMdio_C22WriteInArgs *)prms->inArgs;
#if ENET_CFG_IS_ON(MDIO_CLAUSE45)
                uint32_t c45EnMask;

                if (ENET_FEAT_IS_EN(hMod->features, MDIO_FEATURE_CLAUSE45))
                {
                    c45EnMask = CSL_MDIO_getClause45EnableMask(mdioRegs);
                    if (ENET_IS_BIT_SET(c45EnMask, inArgs->phyAddr))
                    {
                        ENETTRACE_ERR("PHY %u is not configured for C22 access\n", inArgs->phyAddr);
                        status = ENET_EPERM;
                    }
                }
#endif

                if (status == ENET_SOK)
                {
                    CSL_MDIO_phyRegWrite2(mdioRegs,
                                          inArgs->group,
                                          inArgs->phyAddr,
                                          inArgs->reg,
                                          inArgs->val);
                }
            }
            break;

            case ENET_MDIO_IOCTL_C45_READ:
            {
#if ENET_CFG_IS_ON(MDIO_CLAUSE45)
                EnetMdio_C45ReadInArgs *inArgs = (EnetMdio_C45ReadInArgs *)prms->inArgs;
                uint16_t *val = (uint16_t *)prms->outArgs;

                if (ENET_FEAT_IS_EN(hMod->features, MDIO_FEATURE_CLAUSE45))
                {
                    status = Mdio_readRegC45(mdioRegs,
                                             (uint32_t)inArgs->group,
                                             (uint32_t)inArgs->mmd,
                                             inArgs->phyAddr,
                                             inArgs->reg,
                                             val);
                    ENETTRACE_ERR_IF(status != ENET_SOK,
                                     "failed to read PHY %u C45 MMD %u reg %u: %d\n",
                                     inArgs->phyAddr, inArgs->mmd, inArgs->reg, status);
                }
                else
                {
                    ENETTRACE_ERR("C45 support is not supported\n");
                    status = ENET_ENOTSUPPORTED;
                }
#else
                ENETTRACE_ERR("C45 support is not enabled\n");
                status = ENET_ENOTSUPPORTED;
#endif
            }
            break;

            case ENET_MDIO_IOCTL_C45_WRITE:
            {
#if ENET_CFG_IS_ON(MDIO_CLAUSE45)
                EnetMdio_C45WriteInArgs *inArgs = (EnetMdio_C45WriteInArgs *)prms->inArgs;

                if (ENET_FEAT_IS_EN(hMod->features, MDIO_FEATURE_CLAUSE45))
                {
                    status = Mdio_writeRegC45(mdioRegs,
                                              (uint32_t)inArgs->group,
                                              (uint32_t)inArgs->mmd,
                                              inArgs->phyAddr,
                                              inArgs->reg,
                                              inArgs->val);
                    ENETTRACE_ERR_IF(status != ENET_SOK,
                                     "failed to write PHY %u C45 MMD %u reg %u: %d\n",
                                     inArgs->phyAddr, inArgs->mmd, inArgs->reg, status);
                }
                else
                {
                    ENETTRACE_ERR("C45 support is not supported\n");
                    status = ENET_ENOTSUPPORTED;
                }
#else
                ENETTRACE_ERR("C45 support is not enabled\n");
                status = ENET_ENOTSUPPORTED;
#endif
            }
            break;

            case MDIO_IOCTL_HANDLE_INTR:
            {
                Mdio_Callbacks *callbacks = (Mdio_Callbacks *)prms->inArgs;

                Mdio_handleIntr(mdioRegs, callbacks);
            }
            break;

            default:
                status = ENET_EINVALIDPARAMS;
                break;
        }
    }

    return status;
}

#if ENET_CFG_IS_ON(DEV_ERROR)
static int32_t Mdio_isSupported(CSL_mdioHandle mdioRegs)
{
    CSL_MDIO_VERSION version;
    uint32_t i;
    int32_t status = ENET_ENOTSUPPORTED;

    CSL_MDIO_getVersionInfo(mdioRegs, &version);

    for (i = 0U; i < ENET_ARRAYSIZE(gMdio_supportedVer); i++)
    {
        if ((version.revMaj == gMdio_supportedVer[i].revMaj) &&
            (version.revMin == gMdio_supportedVer[i].revMin) &&
            (version.revRtl == gMdio_supportedVer[i].revRtl) &&
            (version.modId  == gMdio_supportedVer[i].modId))
        {
            status = ENET_SOK;
            break;
        }
    }

    return status;
}
#endif

static int32_t Mdio_setupNormalMode(CSL_mdioHandle mdioRegs,
                                    const Mdio_Cfg *cfg)
{
    uint32_t phyMonIndex = 0U;
    uint32_t i;
    int32_t status = ENET_SOK;

    for (i = 0U; i <= MDIO_MAX_PHY_CNT; i++)
    {
        if (ENET_IS_BIT_SET(cfg->pollEnMask, i))
        {
            if (phyMonIndex < MDIO_PHY_MONITOR_MAX)
            {
                CSL_MDIO_enableLinkStatusChangeInterrupt(mdioRegs, phyMonIndex, i);
            }

            phyMonIndex++;
        }
    }

    if (phyMonIndex <= MDIO_PHY_MONITOR_MAX)
    {
        /* Normal Mode implies State Change Mode is disabled */
        CSL_MDIO_disableStateChangeMode(mdioRegs);
    }
    else
    {
        ENETTRACE_ERR("invalid PHY monitor count %d\n", phyMonIndex);
        for (i = 0U; i < MDIO_PHY_MONITOR_MAX; i++)
        {
            CSL_MDIO_disableLinkStatusChangeInterrupt(mdioRegs, i, 0U);
        }

        CSL_MDIO_clearUnmaskedLinkStatusChangeInt(mdioRegs, 0U);
        CSL_MDIO_clearUnmaskedLinkStatusChangeInt(mdioRegs, 1U);
        status = ENET_EINVALIDPARAMS;
    }

    return status;
}

static void Mdio_setupStatusChangeMode(CSL_mdioHandle mdioRegs,
                                       const Mdio_Cfg *cfg)
{
    CSL_MDIO_setPollEnableMask(mdioRegs, cfg->pollEnMask);
    CSL_MDIO_enableStatusChangeModeInterrupt(mdioRegs);
    CSL_MDIO_enableStateChangeMode(mdioRegs);
}

static void Mdio_printRegs(CSL_mdioHandle mdioRegs)
{
    uint32_t *regAddr = (uint32_t *)mdioRegs;
    uint32_t regIdx = 0U;

    while ((uintptr_t)regAddr < ((uintptr_t)mdioRegs + sizeof(*mdioRegs)))
    {
        if (*regAddr != 0U)
        {
            ENETTRACE_INFO("MDIO: %u: 0x%08x\n", regIdx, *regAddr);
        }

        regAddr++;
        regIdx++;
    }
}

#if ENET_CFG_IS_ON(MDIO_CLAUSE45)
static int32_t Mdio_readRegC45(CSL_mdioHandle mdioRegs,
                               uint32_t userCh,
                               uint32_t mmd,
                               uint8_t phyAddr,
                               uint16_t reg,
                               uint16_t *val)
{
    bool isComplete;
    uint32_t c45EnMask;
    int32_t status;

    c45EnMask = CSL_MDIO_getClause45EnableMask(mdioRegs);
    if (ENET_IS_BIT_SET(c45EnMask, phyAddr))
    {
        /* Wait for any ongoing transaction to complete */
        do
        {
            isComplete = CSL_MDIO_isPhyRegAccessComplete(mdioRegs, userCh);
        }
        while (isComplete == FALSE);

        /* Initiate register read */
        status = CSL_MDIO_phyInitiateRegReadC45(mdioRegs, userCh, phyAddr, mmd, reg);
        ENETTRACE_ERR_IF(status != CSL_PASS,
                         "failed to initiate PHY %u C45 MMD %u register %u read: %d\n",
                         phyAddr, mmd, reg, status);

        /* Wait for register read transaction to complete */
        if (status == CSL_PASS)
        {
            do
            {
                isComplete = CSL_MDIO_isPhyRegAccessComplete(mdioRegs, userCh);
            }
            while (isComplete == FALSE);
        }

        /* Get the value read from PHY register once transaction is complete */
        if (status == CSL_PASS)
        {
            status = CSL_MDIO_phyGetRegReadVal(mdioRegs, userCh, val);
            ENETTRACE_ERR_IF(status == CSL_ETIMEOUT,
                             "C45 register read %u was not acknowledged by PHY %u: %d\n",
                             reg, phyAddr, status);
            ENETTRACE_ERR_IF(status == CSL_EFAIL,
                             "failed to read PHY %u C45 MMD %u register %u: %d\n",
                             phyAddr, mmd, reg, status);
        }
    }
    else
    {
        ENETTRACE_ERR("PHY %u is not configured for C45 access\n", phyAddr);
        status = ENET_EPERM;
    }

    return status;
}

static int32_t Mdio_writeRegC45(CSL_mdioHandle mdioRegs,
                                uint32_t userCh,
                                uint32_t mmd,
                                uint8_t phyAddr,
                                uint16_t reg,
                                uint16_t val)
{
    bool isComplete;
    uint32_t c45EnMask;
    int32_t status;

    c45EnMask = CSL_MDIO_getClause45EnableMask(mdioRegs);
    if (ENET_IS_BIT_SET(c45EnMask, phyAddr))
    {
        /* Wait for any ongoing transaction to complete */
        do
        {
            isComplete = CSL_MDIO_isPhyRegAccessComplete(mdioRegs, userCh);
        }
        while (isComplete == FALSE);

        /* Initiate register write */
        status = CSL_MDIO_phyInitiateRegWriteC45(mdioRegs, userCh, phyAddr, mmd, reg, val);
        ENETTRACE_ERR_IF(status != CSL_PASS,
                         "failed to initiate PHY %u C45 MMD %u register %u write: %d\n",
                         phyAddr, mmd, reg, status);

        /* Wait for register write transaction to complete */
        if (status == CSL_PASS)
        {
            do
            {
                isComplete = CSL_MDIO_isPhyRegAccessComplete(mdioRegs, userCh);
            }
            while (isComplete == FALSE);
        }
    }
    else
    {
        ENETTRACE_ERR("PHY %u is not configured for C45 access\n", phyAddr);
        status = ENET_EPERM;
    }

    return status;
}
#endif /* MDIO_CLAUSE45 */

static void Mdio_handleIntr(CSL_mdioHandle mdioRegs,
                            Mdio_Callbacks *callbacks)
{
    Mdio_PhyStatus phyStatus;
    uint32_t phyAddr;
    uint32_t isStatusChangeMode;
    uint32_t pollEnMask = ENET_MDIO_PHY_ADDR_MASK_NONE;
    uint32_t linkChanged;
    uint32_t i;
    bool linkInt0Changed = false;
    bool linkInt1Changed = false;

    linkChanged = CSL_MDIO_isUnmaskedLinkStatusChangeIntSet(mdioRegs, 0U);
    if (linkChanged == 1U)
    {
        CSL_MDIO_clearUnmaskedLinkStatusChangeInt(mdioRegs, 0U);
        linkInt0Changed = true;
    }

    linkChanged = CSL_MDIO_isUnmaskedLinkStatusChangeIntSet(mdioRegs, 1U);
    if (linkChanged == 1U)
    {
        CSL_MDIO_clearUnmaskedLinkStatusChangeInt(mdioRegs, 1U);
        linkInt1Changed = true;
    }

    /* Only report state change of PHYs being polled */
    isStatusChangeMode = CSL_MDIO_isStateChangeModeEnabled(mdioRegs);
    if (isStatusChangeMode == 0)
    {
        for (i = 0U; i < MDIO_PHY_MONITOR_MAX; i++)
        {
            phyAddr = CSL_MDIO_getLinkStatusChangePhyAddr(mdioRegs, i);

            pollEnMask |= ENET_MDIO_PHY_ADDR_MASK(phyAddr);
        }

        phyStatus.aliveMask  = ENET_MDIO_PHY_ADDR_MASK_NONE;
        phyStatus.linkedMask = mdioRegs->LINK_REG & pollEnMask;
    }
    else
    {
        pollEnMask = CSL_MDIO_getPollEnableMask(mdioRegs);

        phyStatus.aliveMask  = mdioRegs->ALIVE_REG & pollEnMask;
        phyStatus.linkedMask = mdioRegs->LINK_REG & pollEnMask;
    }

    /* MDIO_LINKINT[0] event handling */
    if (linkInt0Changed)
    {
        if (callbacks->linkStateCb != NULL)
        {
            callbacks->linkStateCb(ENET_MDIO_GROUP_0, &phyStatus, callbacks->cbArgs);
        }
    }

    /* MDIO_LINKINT[1] event handling */
    if (linkInt1Changed)
    {
        if (isStatusChangeMode == 0U)
        {
            if (callbacks->linkStateCb != NULL)
            {
                callbacks->linkStateCb(ENET_MDIO_GROUP_1, &phyStatus, callbacks->cbArgs);
            }
        }
        else
        {
            /* MDIO_LINKINT[1] is unexpected in State Change Mode */
            /* TODO: Report an error message */
        }
    }
}
