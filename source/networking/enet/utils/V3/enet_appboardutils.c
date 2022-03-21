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
 * \file     enet_appboardutils.c
 *
 * \brief    This file contains the board specific utilities for AM64x and AM243x EVM.
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdbool.h>

#include <include/phy/dp83867.h>
#include <enet.h>

#include <utils/include/enet_apputils.h>
#include <utils/include/enet_board.h>
#include <utils/include/enet_appboardutils.h>

#include <drivers/hw_include/cslr_soc.h>
#include <drivers/hw_include/am64x_am243x/cslr_main_ctrl_mmr.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                         Structures and Enums                               */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                 Internal Function Declarations                             */
/* ========================================================================== */

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

uint32_t EnetBoard_getPhyAddr(Enet_Type enetType,
                                      Enet_MacPort portNum)
{
    uint32_t phyAddr = ENETPHY_INVALID_PHYADDR;

    switch (enetType)
    {
        case ENET_CPSW_3G:
            phyAddr = 0x00U;
            break;

        default:
            EnetAppUtils_assert(false);
            break;
    }

    return phyAddr;
}

void EnetBoard_setPhyConfigSgmii(Enet_MacPort portNum,
                                         CpswMacPort_Cfg *macCfg,
                                         EnetMacPort_Interface *interface,
                                         EnetPhy_Cfg *phyCfg)
{
    EnetAppUtils_assert(false);
}

void EnetBoard_setPhyConfigQsgmii(Enet_Type enetType,
                                          Enet_MacPort portNum,
                                          CpswMacPort_Cfg *macCfg,
                                          EnetMacPort_Interface *interface,
                                          EnetPhy_Cfg *phyCfg)
{
    EnetAppUtils_assert(false);
}

void EnetBoard_setPhyConfigRmii(Enet_MacPort portNum,
                                        CpswMacPort_Cfg *macCfg,
                                        EnetMacPort_Interface *interface,
                                        EnetPhy_Cfg *phyCfg)
{
    EnetAppUtils_assert(false);
}

void EnetBoard_setPhyConfigRgmii(Enet_Type enetType,
                                 Enet_MacPort portNum,
                                 CpswMacPort_Cfg *macCfg,
                                 EnetMacPort_Interface *interface,
                                 EnetPhy_Cfg *phyCfg)
{
    Dp83867_Cfg extendedCfg;

    Dp83867_initCfg(&extendedCfg);
    EnetPhy_initCfg(phyCfg);
    phyCfg->phyAddr = EnetBoard_getPhyAddr(enetType, portNum);

    switch (enetType)
    {
        case ENET_CPSW_3G:
            EnetAppUtils_assert(portNum == ENET_MAC_PORT_1);

            /* DP83867 specific configuration */
            extendedCfg.txFifoDepth = 4U;

            extendedCfg.txClkShiftEn     = true;
            extendedCfg.rxClkShiftEn     = true;
            extendedCfg.impedanceInMilliOhms = 35000; /* 35 ohms */
            extendedCfg.gpio0Mode            = DP83867_GPIO0_LED3;
            extendedCfg.ledMode[1]           = DP83867_LED_LINKED_100BTX;
            extendedCfg.ledMode[2]           = DP83867_LED_RXTXACT;
            extendedCfg.ledMode[3]           = DP83867_LED_LINKED_1000BT;

            /* RGMII interface type */
            interface->layerType    = ENET_MAC_LAYER_GMII;
            interface->sublayerType = ENET_MAC_SUBLAYER_REDUCED;
            interface->variantType  = ENET_MAC_VARIANT_NONE;
            break;

        default:
            EnetAppUtils_print("Invalid ENET type\r\n");
            EnetAppUtils_assert(false);
    }

    EnetPhy_setExtendedCfg(phyCfg,
                              &extendedCfg,
                              sizeof(extendedCfg));
}

void EnetBoard_setPhyConfig(Enet_Type enetType,
                                    Enet_MacPort portNum,
                                    CpswMacPort_Cfg *macCfg,
                                    EnetMacPort_Interface *interface,
                                    EnetPhy_Cfg *phyCfg)
{
    return EnetBoard_setPhyConfigRgmii(enetType,
                                               portNum,
                                               macCfg,
                                               interface,
                                               phyCfg);
}

void EnetBoard_setNoPhyConfig(EnetMacPort_Interface *interface,
                                      EnetPhy_Cfg *phyCfg)
{
    phyCfg->phyAddr      = ENETPHY_INVALID_PHYADDR;
    interface->layerType    = ENET_MAC_LAYER_GMII;
    interface->sublayerType = ENET_MAC_SUBLAYER_REDUCED;
    interface->variantType  = ENET_MAC_VARIANT_FORCED;
}

void EnetBoard_setLpbkCfg(bool phyLpbk,
                                  Enet_MacPort portNum,
                                  const EnetMacPort_Interface *interface,
                                  CpswMacPort_Cfg *macCfg,
                                  EnetPhy_Cfg *phyCfg,
                                  EnetMacPort_LinkCfg *linkCfg)
{
    /* Set speed according to the interface type */
    if (interface->layerType == ENET_MAC_LAYER_MII)
    {
        linkCfg->speed = ENET_SPEED_100MBIT;
    }
    else
    {
        linkCfg->speed = ENET_SPEED_1GBIT;
    }

    linkCfg->duplexity = ENET_DUPLEX_FULL;

    /* MAC and PHY loopbacks are mutually exclusive */
    phyCfg->loopbackEn = phyLpbk;
    macCfg->loopbackEn = !phyLpbk;
}

static void EnetBoard_setMcuEnetControl(uint32_t modeSel)
{
    CSL_main_ctrl_mmr_cfg0Regs *mcuRegs;
    EnetAppUtils_MmrLockState prevLockState;

    mcuRegs = (CSL_main_ctrl_mmr_cfg0Regs *)(uintptr_t)CSL_CTRL_MMR0_CFG0_BASE;
    prevLockState = EnetAppUtils_mcuMmrCtrl(ENETAPPUTILS_MMR_LOCK1, ENETAPPUTILS_UNLOCK_MMR);
    CSL_REG32_FINS(&mcuRegs->ENET1_CTRL,
                   MAIN_CTRL_MMR_CFG0_ENET1_CTRL_PORT_MODE_SEL,
                   modeSel);
    if (prevLockState == ENETAPPUTILS_LOCK_MMR)
    {
        EnetAppUtils_mcuMmrCtrl(ENETAPPUTILS_MMR_LOCK1, ENETAPPUTILS_LOCK_MMR);
    }
}

void EnetBoard_setEnetControl(Enet_Type enetType,
                                      uint32_t instId,
                                      Enet_MacPort portNum,
                                      uint32_t modeSel)
{
    EnetAppUtils_assert(portNum == ENET_MAC_PORT_1);
    EnetAppUtils_assert(enetType == ENET_CPSW_3G);
    EnetAppUtils_assert(((modeSel == RMII) || (modeSel == RGMII)));
    EnetBoard_setMcuEnetControl(modeSel);
}
