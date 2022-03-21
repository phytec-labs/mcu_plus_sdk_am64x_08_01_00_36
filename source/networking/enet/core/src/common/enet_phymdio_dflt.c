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
 * \file  enet_phymdio_dflt.c
 *
 * \brief This file contains the default implementation of the MDIO interface
 *        of the Ethernet PHY (ENETPHY) driver with Enet LLD APIs.
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <include/phy/enetphy.h>
#include <include/core/enet_mod_mdio.h>
#include <include/core/enet_mod_phy.h>
#include <include/core/enet_mod_macport.h>
#include <include/common/enet_phymdio_dflt.h>
#include <priv/core/enet_trace_priv.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

int32_t EnetPhyMdioDflt_isAlive(uint32_t phyAddr,
                                bool *isAlive,
                                void *args);

int32_t EnetPhyMdioDflt_isLinked(uint32_t phyAddr,
                                 bool *isLinked,
                                 void *args);

int32_t EnetPhyMdioDflt_readC22(uint32_t group,
                                uint8_t phyAddr,
                                uint32_t reg,
                                uint16_t *val,
                                void *args);

int32_t EnetPhyMdioDflt_writeC22(uint32_t group,
                                 uint8_t phyAddr,
                                 uint32_t reg,
                                 uint16_t val,
                                 void *args);

int32_t EnetPhyMdioDflt_readC45(uint32_t group,
                                uint8_t phyAddr,
                                uint8_t mmd,
                                uint16_t reg,
                                uint16_t *val,
                                void *args);

int32_t EnetPhyMdioDflt_writeC45(uint32_t group,
                                 uint8_t phyAddr,
                                 uint8_t mmd,
                                 uint16_t reg,
                                 uint16_t val,
                                 void *args);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

static EnetPhy_Mdio gEnet_PhyMdioDflt =
{
    .isAlive  = EnetPhyMdioDflt_isAlive,
    .isLinked = EnetPhyMdioDflt_isLinked,
    .readC22  = EnetPhyMdioDflt_readC22,
    .writeC22 = EnetPhyMdioDflt_writeC22,
    .readC45  = EnetPhyMdioDflt_readC45,
    .writeC45 = EnetPhyMdioDflt_writeC45,
};


/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

EnetPhy_MdioHandle EnetPhyMdioDflt_getPhyMdio(void)
{
    return &gEnet_PhyMdioDflt;
}

int32_t EnetPhyMdioDflt_isAlive(uint32_t phyAddr,
                                bool *isAlive,
                                void *args)
{
    EnetMod_Handle hMdio = ENET_MOD(args);
    Enet_IoctlPrms prms;
    bool isOpen;
    int32_t status = ENETPHY_SOK;

    if (hMdio == NULL)
    {
        ENETTRACE_ERR("PHY %u: Invalid MDIO handle\n", phyAddr);
        status = ENETPHY_EBADARGS;
    }

    if (status == ENET_SOK)
    {
        isOpen = EnetMod_isOpen(hMdio);
        if (!isOpen)
        {
            ENETTRACE_ERR("PHY %u: MDIO module is not open\n", phyAddr);
            status = ENETPHY_EFAIL;
        }
    }
    
    if (status == ENET_SOK)
    {
        ENET_IOCTL_SET_INOUT_ARGS(&prms, &phyAddr, isAlive);

        status = EnetMod_ioctl(hMdio, ENET_MDIO_IOCTL_IS_ALIVE, &prms);
        ENETTRACE_ERR_IF(status != ENET_SOK,
                         "PHY %u: Failed to get MDIO alive status: %d\n", phyAddr, status);
    }

    return status;
}

int32_t EnetPhyMdioDflt_isLinked(uint32_t phyAddr,
                                 bool *isLinked,
                                 void *args)
{
    EnetMod_Handle hMdio = ENET_MOD(args);
    Enet_IoctlPrms prms;
    bool isOpen;
    int32_t status = ENETPHY_SOK;

    if (hMdio == NULL)
    {
        ENETTRACE_ERR("PHY %u: Invalid MDIO handle\n", phyAddr);
        status = ENETPHY_EBADARGS;
    }

    if (status == ENET_SOK)
    {
        isOpen = EnetMod_isOpen(hMdio);
        if (!isOpen)
        {
            ENETTRACE_ERR("PHY %u: MDIO module is not open\n", phyAddr);
            status = ENETPHY_EFAIL;
        }
    }
    
    if (status == ENET_SOK)
    {
        ENET_IOCTL_SET_INOUT_ARGS(&prms, &phyAddr, isLinked);

        status = EnetMod_ioctl(hMdio, ENET_MDIO_IOCTL_IS_LINKED, &prms);
        ENETTRACE_ERR_IF(status != ENET_SOK,
                         "PHY %u: Failed to get link status: %d\n", phyAddr, status);
    }

    return status;
}

int32_t EnetPhyMdioDflt_readC22(uint32_t group,
                                uint8_t phyAddr,
                                uint32_t reg,
                                uint16_t *val,
                                void *args)
{
    EnetMod_Handle hMdio = ENET_MOD(args);
    Enet_IoctlPrms prms;
    EnetMdio_C22ReadInArgs inArgs;
    bool isOpen;
    int32_t status = ENETPHY_SOK;

    if (hMdio == NULL)
    {
        ENETTRACE_ERR("PHY %u: Invalid MDIO handle\n", phyAddr);
        status = ENETPHY_EBADARGS;
    }

    if (status == ENET_SOK)
    {
        isOpen = EnetMod_isOpen(hMdio);
        if (!isOpen)
        {
            ENETTRACE_ERR("PHY %u: MDIO module is not open\n", phyAddr);
            status = ENETPHY_EFAIL;
        }
    }
    
    if (status == ENET_SOK)
    {
        inArgs.group = (EnetMdio_Group)group;
        inArgs.phyAddr = phyAddr;
        inArgs.reg = reg;
        ENET_IOCTL_SET_INOUT_ARGS(&prms, &inArgs, val);

        status = EnetMod_ioctl(hMdio, ENET_MDIO_IOCTL_C22_READ, &prms);
        ENETTRACE_ERR_IF(status != ENET_SOK,
                         "PHY %u: Failed to read C22 reg: %d\n", phyAddr, status);
    }

    return status;
}

int32_t EnetPhyMdioDflt_writeC22(uint32_t group,
                                 uint8_t phyAddr,
                                 uint32_t reg,
                                 uint16_t val,
                                 void *args)
{
    EnetMod_Handle hMdio = ENET_MOD(args);
    Enet_IoctlPrms prms;
    EnetMdio_C22WriteInArgs inArgs;
    bool isOpen;
    int32_t status = ENETPHY_SOK;

    if (hMdio == NULL)
    {
        ENETTRACE_ERR("PHY %u: Invalid MDIO handle\n", phyAddr);
        status = ENETPHY_EBADARGS;
    }

    if (status == ENET_SOK)
    {
        isOpen = EnetMod_isOpen(hMdio);
        if (!isOpen)
        {
            ENETTRACE_ERR("PHY %u: MDIO module is not open\n", phyAddr);
            status = ENETPHY_EFAIL;
        }
    }
    
    if (status == ENET_SOK)
    {
        inArgs.group = (EnetMdio_Group)group;
        inArgs.phyAddr = phyAddr;
        inArgs.reg = reg;
        inArgs.val = val;
        ENET_IOCTL_SET_IN_ARGS(&prms, &inArgs);

        status = EnetMod_ioctl(hMdio, ENET_MDIO_IOCTL_C22_WRITE, &prms);
        ENETTRACE_ERR_IF(status != ENET_SOK,
                         "PHY %u: Failed to write C22 reg: %d\n", phyAddr, status);
    }

    return status;
}

int32_t EnetPhyMdioDflt_readC45(uint32_t group,
                                uint8_t phyAddr,
                                uint8_t mmd,
                                uint16_t reg,
                                uint16_t *val,
                                void *args)
{
    EnetMod_Handle hMdio = ENET_MOD(args);
    Enet_IoctlPrms prms;
    EnetMdio_C45ReadInArgs inArgs;
    bool isOpen;
    int32_t status = ENETPHY_SOK;

    if (hMdio == NULL)
    {
        ENETTRACE_ERR("PHY %u: Invalid MDIO handle\n", phyAddr);
        status = ENETPHY_EBADARGS;
    }

    if (status == ENET_SOK)
    {
        isOpen = EnetMod_isOpen(hMdio);
        if (!isOpen)
        {
            ENETTRACE_ERR("PHY %u: MDIO module is not open\n", phyAddr);
            status = ENETPHY_EFAIL;
        }
    }
    
    if (status == ENET_SOK)
    {
        inArgs.group = (EnetMdio_Group)group;
        inArgs.phyAddr = phyAddr;
        inArgs.mmd = (EnetMdio_C45Mmd)mmd;
        inArgs.reg = reg;
        ENET_IOCTL_SET_INOUT_ARGS(&prms, &inArgs, val);

        status = EnetMod_ioctl(hMdio, ENET_MDIO_IOCTL_C45_READ, &prms);
        ENETTRACE_ERR_IF(status != ENET_SOK,
                         "PHY %u: Failed to read C45 reg: %d\n", phyAddr, status);
    }

    return status;
}

int32_t EnetPhyMdioDflt_writeC45(uint32_t group,
                                 uint8_t phyAddr,
                                 uint8_t mmd,
                                 uint16_t reg,
                                 uint16_t val,
                                 void *args)
{
    EnetMod_Handle hMdio = ENET_MOD(args);
    Enet_IoctlPrms prms;
    EnetMdio_C45WriteInArgs inArgs;
    bool isOpen;
    int32_t status = ENETPHY_SOK;

    if (hMdio == NULL)
    {
        ENETTRACE_ERR("PHY %u: Invalid MDIO handle\n", phyAddr);
        status = ENETPHY_EBADARGS;
    }

    if (status == ENET_SOK)
    {
        isOpen = EnetMod_isOpen(hMdio);
        if (!isOpen)
        {
            ENETTRACE_ERR("PHY %u: MDIO module is not open\n", phyAddr);
            status = ENETPHY_EFAIL;
        }
    }
    
    if (status == ENET_SOK)
    {
        inArgs.group = (EnetMdio_Group)group;
        inArgs.phyAddr = phyAddr;
        inArgs.mmd = (EnetMdio_C45Mmd)mmd;
        inArgs.reg = reg;
        inArgs.val = val;
        ENET_IOCTL_SET_IN_ARGS(&prms, &inArgs);

        status = EnetMod_ioctl(hMdio, ENET_MDIO_IOCTL_C45_WRITE, &prms);
        ENETTRACE_ERR_IF(status != ENET_SOK, "Failed to write C45 reg: %d\n", status);
    }

    return status;
}

int32_t EnetPhyMdioDflt_ioctl(EnetPhy_Handle hPhy,
                              uint32_t cmd,
                              Enet_IoctlPrms *prms)
{
    int32_t status = ENET_SOK;

    switch (cmd)
    {
        case ENET_PHY_IOCTL_GET_ID:
        {
#if (ENET_CFG_TRACE_LEVEL >= ENET_CFG_TRACE_LEVEL_ERROR)
            const EnetPhy_GenericInArgs *inArgs = (const EnetPhy_GenericInArgs *)prms->inArgs;
#endif
            EnetPhy_Version *version = (EnetPhy_Version *)prms->outArgs;

            status = EnetPhy_getId(hPhy, version);
            ENETTRACE_ERR_IF(status != ENETPHY_SOK,
                             "Port %u: Failed to get PHY id: %d\n",
                             ENET_MACPORT_ID(inArgs->macPort), status);
        }
        break;

        case ENET_PHY_IOCTL_GET_SUPPORTED_MODES:
        {
            /* Currently not supported */
            status = ENET_ENOTSUPPORTED;
        }
        break;

        case ENET_PHY_IOCTL_GET_LOOPBACK_STATE:
        {
            /* Currently not supported */
            status = ENET_ENOTSUPPORTED;
        }
        break;

        case ENET_PHY_IOCTL_IS_ALIVE:
        {
            bool *isAlive = (bool *)prms->outArgs;

            *isAlive = EnetPhy_isAlive(hPhy);
        }
        break;

        case ENET_PHY_IOCTL_IS_LINKED:
        {
            bool *isLinked = (bool *)prms->outArgs;

            *isLinked = EnetPhy_isLinked(hPhy);
        }
        break;

        case ENET_PHY_IOCTL_GET_LINK_MODE:
        {
#if (ENET_CFG_TRACE_LEVEL >= ENET_CFG_TRACE_LEVEL_ERROR)
            const EnetPhy_GenericInArgs *inArgs = (const EnetPhy_GenericInArgs *)prms->inArgs;
#endif
            EnetMacPort_LinkCfg *linkCfg = (EnetMacPort_LinkCfg *)prms->outArgs;
            EnetPhy_LinkCfg phyLinkCfg;

            status = EnetPhy_getLinkCfg(hPhy, &phyLinkCfg);
            if (status == ENETPHY_SOK)
            {
                linkCfg->speed = (Enet_Speed)phyLinkCfg.speed;
                linkCfg->duplexity = (Enet_Duplexity)phyLinkCfg.duplexity;
            }
            else
            {
                ENETTRACE_ERR("Port %u: Failed to get link config: %d\n",
                              ENET_MACPORT_ID(inArgs->macPort), status);
            }
        }
        break;

        case ENET_PHY_IOCTL_RESET:
        {
            /* Currently not supported */
            status = ENET_ENOTSUPPORTED;
        }
        break;

        case ENET_PHY_IOCTL_READ_REG:
        {
            const EnetPhy_ReadRegInArgs *inArgs = (const EnetPhy_ReadRegInArgs *)prms->inArgs;
            uint16_t *val = (uint16_t *)prms->outArgs;

            status = EnetPhy_readReg(hPhy, inArgs->reg, val);
            ENETTRACE_ERR_IF(status != ENETPHY_SOK,
                             "Port %u: Failed to read reg %u: %d\n",
                             ENET_MACPORT_ID(inArgs->macPort), inArgs->reg, status);
        }
        break;

        case ENET_PHY_IOCTL_WRITE_REG:
        {
            const EnetPhy_WriteRegInArgs *inArgs = (const EnetPhy_WriteRegInArgs *)prms->inArgs;

            status = EnetPhy_writeReg(hPhy, inArgs->reg, inArgs->val);
            ENETTRACE_ERR_IF(status != ENETPHY_SOK,
                             "Port %u: Failed to write reg %u: %d\n",
                             ENET_MACPORT_ID(inArgs->macPort), inArgs->reg, status);
        }
        break;

        case ENET_PHY_IOCTL_READ_EXT_REG:
        {
            const EnetPhy_ReadRegInArgs *inArgs = (const EnetPhy_ReadRegInArgs *)prms->inArgs;
            uint16_t *val = (uint16_t *)prms->outArgs;

            status = EnetPhy_readExtReg(hPhy, inArgs->reg, val);
            ENETTRACE_ERR_IF(status != ENETPHY_SOK,
                             "Port %u: Failed to read ext reg %u: %d\n",
                             ENET_MACPORT_ID(inArgs->macPort), inArgs->reg, status);
        }
        break;

        case ENET_PHY_IOCTL_WRITE_EXT_REG:
        {
            const EnetPhy_WriteRegInArgs *inArgs = (const EnetPhy_WriteRegInArgs *)prms->inArgs;

            status = EnetPhy_writeExtReg(hPhy, inArgs->reg, inArgs->val);
            ENETTRACE_ERR_IF(status != ENETPHY_SOK,
                             "Port %u: Failed to write ext reg %u: %d\n",
                             ENET_MACPORT_ID(inArgs->macPort), inArgs->reg, status);
        }
        break;

        case ENET_PHY_IOCTL_C45_READ_REG:
        {
            const EnetPhy_C45ReadRegInArgs *inArgs = (const EnetPhy_C45ReadRegInArgs *)prms->inArgs;
            uint16_t *val = (uint16_t *)prms->outArgs;

            status = EnetPhy_readC45Reg(hPhy, inArgs->mmd, inArgs->reg, val);
            ENETTRACE_ERR_IF(status != ENETPHY_SOK,
                             "Port %u: Failed to read C45 mmd %u reg %u: %d\n",
                             ENET_MACPORT_ID(inArgs->macPort), inArgs->mmd, inArgs->reg, status);
        }
        break;

        case ENET_PHY_IOCTL_C45_WRITE_REG:
        {
            const EnetPhy_C45WriteRegInArgs *inArgs = (const EnetPhy_C45WriteRegInArgs *)prms->inArgs;

            status = EnetPhy_writeC45Reg(hPhy, inArgs->mmd, inArgs->reg, inArgs->val);
            ENETTRACE_ERR_IF(status != ENETPHY_SOK,
                             "Port %u: Failed to write C45 mmd %u reg %u: %d\n",
                             ENET_MACPORT_ID(inArgs->macPort), inArgs->mmd, inArgs->reg, status);
        }
        break;

        case ENET_PHY_IOCTL_PRINT_REGS:
        {
            EnetPhy_printRegs(hPhy);
        }
        break;

        default:
        {
            status = ENET_EINVALIDPARAMS;
            break;
        }
    }

    return status;
}
