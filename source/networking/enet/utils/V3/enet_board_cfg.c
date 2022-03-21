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
 * \file  enet_board_am65xevm.c
 *
 * \brief This file contains the implementation of AM64x EVM support.
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdint.h>
#include <enet.h>
#include <include/phy/enetphy.h>
#include <include/phy/dp83867.h>
#include <include/phy/dp83869.h>
#include <utils/V3/enet_board_cfg.h>

#include <utils/include/enet_apputils.h>
#include <kernel/dpl/SystemP.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */


/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

static const EnetBoard_PortCfg *EnetBoard_getPortCfg(const EnetBoard_EthPort *ethPort);

static const EnetBoard_PortCfg *EnetBoard_findPortCfg(const EnetBoard_EthPort *ethPort,
                                                      const EnetBoard_PortCfg *ethPortCfgs,
                                                      uint32_t numEthPorts);

static void EnetBoard_setEnetControl(Enet_Type enetType,
                                     Enet_MacPort macPort,
                                     EnetMacPort_Interface *mii);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/*!
 * \brief Common Processor Board (CPB) board's DP83867 PHY configuration.
 */
static const Dp83867_Cfg gEnetCpbBoard_dp83867PhyCfg =
{
    .txClkShiftEn         = true,
    .rxClkShiftEn         = true,
    .txDelayInPs          = 1500U,  /* 2.00 ns */
    .rxDelayInPs          = 2000U,  /* 2.00 ns */
    .txFifoDepth          = 4U,
    .idleCntThresh        = 4U,
    .impedanceInMilliOhms = 35000,  /* 35 ohms */
    .idleCntThresh        = 4U,     /* Improves short cable performance */
    .gpio0Mode            = DP83867_GPIO0_LED3,
    .gpio1Mode            = DP83867_GPIO1_COL, /* Unused */
    .ledMode              =
    {
        DP83867_LED_LINKED,         /* Unused */
        DP83867_LED_LINKED_100BTX,
        DP83867_LED_RXTXACT,
        DP83867_LED_LINKED_1000BT,
    },
};

/*!
 * \brief Common Processor Board (CPB) board's DP83869 PHY configuration.
 */
static const Dp83869_Cfg gEnetCpbBoard_dp83869PhyCfg =
{
    .txClkShiftEn         = false,
    .rxClkShiftEn         = true,
    .rxDelayInPs          = 1500U,
    .txFifoDepth          = 4U,
    .idleCntThresh        = 4U,
    .impedanceInMilliOhms = 35000,  /* 35 ohms */
    .idleCntThresh        = 4U,     /* Improves short cable performance */
    .gpio0Mode            = DP83869_GPIO0_LED3,
    .gpio1Mode            = DP83869_GPIO1_COL, /* Unused */
    .ledMode              =
    {
        DP83869_LED_LINKED,         /* Unused */
        DP83869_LED_LINKED_100BTX,
        DP83869_LED_RXTXACT,
        DP83869_LED_LINKED_1000BT,
    },
};
/*
 * AM64x board configuration.
 *
 * 1 x RGMII PHY connected to AM64x CPSW_2G MAC port.
 */
static const EnetBoard_PortCfg gEnetCpbBoard_am64xEthPort[] =
{
    {    /* "CPSW3G" */
        .enetType = ENET_CPSW_3G,
        .instId   = 0U,
        .macPort  = ENET_MAC_PORT_1,
        .mii      = { ENET_MAC_LAYER_GMII, ENET_MAC_SUBLAYER_REDUCED },
        .phyCfg   =
        {
            .phyAddr         = 0U,
            .isStrapped      = false,
            .skipExtendedCfg = false,
            .extendedCfg     = &gEnetCpbBoard_dp83867PhyCfg,
            .extendedCfgSize = sizeof(gEnetCpbBoard_dp83867PhyCfg),
        },
        .flags    = 0U,
    },
    {    /* "CPSW3G" */
        .enetType = ENET_CPSW_3G,
        .instId   = 0U,
        .macPort  = ENET_MAC_PORT_2,
        .mii      = { ENET_MAC_LAYER_GMII, ENET_MAC_SUBLAYER_REDUCED },
        .phyCfg   =
        {
            .phyAddr         = 3U,
            .isStrapped      = false,
            .skipExtendedCfg = false,
            .extendedCfg     = &gEnetCpbBoard_dp83867PhyCfg,
            .extendedCfgSize = sizeof(gEnetCpbBoard_dp83867PhyCfg),
        },
        .flags    = 0U,
    },
    {    /* "ETH0" (ICSSG0 Dual-MAC port 1) */
        .enetType = ENET_ICSSG_DUALMAC,
        .instId   = 0U,
        .macPort  = ENET_MAC_PORT_1,
        .mii      = { ENET_MAC_LAYER_GMII, ENET_MAC_SUBLAYER_REDUCED },
        .phyCfg   =
        {
            .phyAddr         = 0xFU,
            .isStrapped      = true,
            .skipExtendedCfg = false,
            .extendedCfg     = &gEnetCpbBoard_dp83869PhyCfg,
            .extendedCfgSize = sizeof(gEnetCpbBoard_dp83869PhyCfg),
        },
        .flags    = 0U,
    },
    {    /* "ETH1" (ICSSG0 Dual-MAC port 2) */
        .enetType = ENET_ICSSG_DUALMAC,
        .instId   = 1U,
        .macPort  = ENET_MAC_PORT_1,
        .mii      = { ENET_MAC_LAYER_GMII, ENET_MAC_SUBLAYER_REDUCED },
        .phyCfg   =
        {
            .phyAddr         = 3U,
            .isStrapped      = true,
            .skipExtendedCfg = false,
            .extendedCfg     = &gEnetCpbBoard_dp83869PhyCfg,
            .extendedCfgSize = sizeof(gEnetCpbBoard_dp83869PhyCfg),
        },
        .flags    = 0U,
    },
    {    /* "ETH2" (ICSSG1 Dual-MAC port 1) */
        .enetType = ENET_ICSSG_DUALMAC,
        .instId   = 2U,
        .macPort  = ENET_MAC_PORT_1,
        .mii      = { ENET_MAC_LAYER_GMII, ENET_MAC_SUBLAYER_REDUCED },
        .phyCfg   =
        {
            .phyAddr         = 0xFU,
            .isStrapped      = false,
            .skipExtendedCfg = false,
            .extendedCfg     = &gEnetCpbBoard_dp83869PhyCfg,
            .extendedCfgSize = sizeof(gEnetCpbBoard_dp83869PhyCfg),
        },
        .flags    = 0U,
    },
    {    /* "ETH3" (ICSSG1 Dual-MAC port 2) */
        .enetType = ENET_ICSSG_DUALMAC,
        .instId   = 3U,
        .macPort  = ENET_MAC_PORT_1,
        .mii      = { ENET_MAC_LAYER_GMII, ENET_MAC_SUBLAYER_REDUCED },
        .phyCfg   =
        {
            .phyAddr         = 3U,
            .isStrapped      = false,
            .skipExtendedCfg = false,
            .extendedCfg     = &gEnetCpbBoard_dp83869PhyCfg,
            .extendedCfgSize = sizeof(gEnetCpbBoard_dp83869PhyCfg),
        },
        .flags    = 0U,
    },
    {    /* "ETH0" (ICSSG0 Switch port 1) */
        .enetType = ENET_ICSSG_SWITCH,
        .instId   = 0U,
        .macPort  = ENET_MAC_PORT_1,
        .mii      = { ENET_MAC_LAYER_GMII, ENET_MAC_SUBLAYER_REDUCED },
        .phyCfg   =
        {
            .phyAddr         = 0xFU,
            .isStrapped      = true,
            .skipExtendedCfg = false,
            .extendedCfg     = &gEnetCpbBoard_dp83869PhyCfg,
            .extendedCfgSize = sizeof(gEnetCpbBoard_dp83869PhyCfg),
        },
        .flags    = 0U,
    },
    {    /* "ETH1" (ICSSG0 Switch port 2) */
        .enetType = ENET_ICSSG_SWITCH,
        .instId   = 0U,
        .macPort  = ENET_MAC_PORT_2,
        .mii      = { ENET_MAC_LAYER_GMII, ENET_MAC_SUBLAYER_REDUCED },
        .phyCfg   =
        {
            .phyAddr         = 3U,
            .isStrapped      = true,
            .skipExtendedCfg = false,
            .extendedCfg     = &gEnetCpbBoard_dp83869PhyCfg,
            .extendedCfgSize = sizeof(gEnetCpbBoard_dp83869PhyCfg),
        },
        .flags    = 0U,
    },
    {    /* "ETH2" (ICSSG1 Switch port 1) */
        .enetType = ENET_ICSSG_SWITCH,
        .instId   = 1U,
        .macPort  = ENET_MAC_PORT_1,
        .mii      = { ENET_MAC_LAYER_GMII, ENET_MAC_SUBLAYER_REDUCED },
        .phyCfg   =
        {
            .phyAddr         = 0xFU,
            .isStrapped      = false,
            .skipExtendedCfg = false,
            .extendedCfg     = &gEnetCpbBoard_dp83869PhyCfg,
            .extendedCfgSize = sizeof(gEnetCpbBoard_dp83869PhyCfg),
        },
        .flags    = 0U,
    },
    {    /* "ETH3" (ICSSG1 Switch port 2) */
        .enetType = ENET_ICSSG_SWITCH,
        .instId   = 1U,
        .macPort  = ENET_MAC_PORT_2,
        .mii      = { ENET_MAC_LAYER_GMII, ENET_MAC_SUBLAYER_REDUCED },
        .phyCfg   =
        {
            .phyAddr         = 3U,
            .isStrapped      = false,
            .skipExtendedCfg = false,
            .extendedCfg     = &gEnetCpbBoard_dp83869PhyCfg,
            .extendedCfgSize = sizeof(gEnetCpbBoard_dp83869PhyCfg),
        },
        .flags    = 0U,
    },
};

/*
 * AM64xx MII board configuration.
 *
 * 2 x MII PHY connected to AM64xx ICSSG1 MAC ports.
 */
static const EnetBoard_PortCfg gEnetMiiBoard_am64xxEthPort[] =
{
    {    /* "ETH2" (ICSSG1 Dual-MAC port 1) */
        .enetType = ENET_ICSSG_DUALMAC,
        .instId   = 2U,
        .macPort  = ENET_MAC_PORT_1,
        .mii      = { ENET_MAC_LAYER_MII, ENET_MAC_SUBLAYER_STANDARD },
        .phyCfg   =
        {
            .phyAddr         = 0xFU,
            .isStrapped      = false,
            .skipExtendedCfg = false,
            .extendedCfg     = &gEnetCpbBoard_dp83869PhyCfg,
            .extendedCfgSize = sizeof(gEnetCpbBoard_dp83869PhyCfg),
        },
        .flags    = 0U,
    },
    {    /* "ETH3" (ICSSG1 Dual-MAC port 2) */
        .enetType = ENET_ICSSG_DUALMAC,
        .instId   = 3U,
        .macPort  = ENET_MAC_PORT_1,
        .mii      = { ENET_MAC_LAYER_MII, ENET_MAC_SUBLAYER_STANDARD },
        .phyCfg   =
        {
            .phyAddr         = 3U,
            .isStrapped      = false,
            .skipExtendedCfg = false,
            .extendedCfg     = &gEnetCpbBoard_dp83869PhyCfg,
            .extendedCfgSize = sizeof(gEnetCpbBoard_dp83869PhyCfg),
        },
        .flags    = 0U,
    },
    {    /* "ETH2" (ICSSG1 Switch port 1) */
        .enetType = ENET_ICSSG_SWITCH,
        .instId   = 1U,
        .macPort  = ENET_MAC_PORT_1,
        .mii      = { ENET_MAC_LAYER_MII, ENET_MAC_SUBLAYER_STANDARD },
        .phyCfg   =
        {
            .phyAddr         = 0xFU,
            .isStrapped      = false,
            .skipExtendedCfg = false,
            .extendedCfg     = &gEnetCpbBoard_dp83869PhyCfg,
            .extendedCfgSize = sizeof(gEnetCpbBoard_dp83869PhyCfg),
        },
        .flags    = 0U,
    },
    {    /* "ETH3" (ICSSG1 Switch port 2) */
        .enetType = ENET_ICSSG_SWITCH,
        .instId   = 1U,
        .macPort  = ENET_MAC_PORT_2,
        .mii      = { ENET_MAC_LAYER_MII, ENET_MAC_SUBLAYER_STANDARD },
        .phyCfg   =
        {
            .phyAddr         = 3U,
            .isStrapped      = false,
            .skipExtendedCfg = false,
            .extendedCfg     = &gEnetCpbBoard_dp83869PhyCfg,
            .extendedCfgSize = sizeof(gEnetCpbBoard_dp83869PhyCfg),
        },
        .flags    = 0U,
    },
};

/*
 * J721E virtual board used for MAC loopback setup.
 */
static const EnetBoard_PortCfg gEnetVirtBoard_am64xEthPort[] =
{
    {    /* RGMII MAC loopback */
        .enetType = ENET_CPSW_3G,
        .instId   = 0U,
        .macPort  = ENET_MAC_PORT_1,
        .mii      = { ENET_MAC_LAYER_GMII, ENET_MAC_SUBLAYER_REDUCED },
        .phyCfg   =
        {
            .phyAddr = ENETPHY_INVALID_PHYADDR,
        },
        .flags    = 0U,
    },
    {    /* RMII MAC loopback */
        .enetType = ENET_CPSW_3G,
        .instId   = 0U,
        .macPort  = ENET_MAC_PORT_1,
        .mii      = { ENET_MAC_LAYER_MII, ENET_MAC_SUBLAYER_REDUCED },
        .phyCfg   =
        {
            .phyAddr = ENETPHY_INVALID_PHYADDR,
        },
        .flags    = 0U,
    },
};

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

const EnetBoard_PhyCfg *EnetBoard_getPhyCfg(const EnetBoard_EthPort *ethPort)
{
    const EnetBoard_PortCfg *portCfg;

    portCfg = EnetBoard_getPortCfg(ethPort);

    return (portCfg != NULL) ? &portCfg->phyCfg : NULL;
}

static const EnetBoard_PortCfg *EnetBoard_getPortCfg(const EnetBoard_EthPort *ethPort)
{
    const EnetBoard_PortCfg *portCfg = NULL;

    if (ENET_NOT_ZERO(ethPort->boardId & ENETBOARD_CPB_ID))
    {
        portCfg = EnetBoard_findPortCfg(ethPort,
                                        gEnetCpbBoard_am64xEthPort,
                                        ENETPHY_ARRAYSIZE(gEnetCpbBoard_am64xEthPort));
    }
    if (ENET_NOT_ZERO(ethPort->boardId & ENETBOARD_MII_ID))
    {
        portCfg = EnetBoard_findPortCfg(ethPort,
                                        gEnetMiiBoard_am64xxEthPort,
                                        ENETPHY_ARRAYSIZE(gEnetMiiBoard_am64xxEthPort));
    }
    if ((portCfg == NULL) &&
        ENET_NOT_ZERO(ethPort->boardId & ENETBOARD_LOOPBACK_ID))
    {
        portCfg = EnetBoard_findPortCfg(ethPort,
                                        gEnetVirtBoard_am64xEthPort,
                                        ENETPHY_ARRAYSIZE(gEnetVirtBoard_am64xEthPort));
    }

    return portCfg;
}

static const EnetBoard_PortCfg *EnetBoard_findPortCfg(const EnetBoard_EthPort *ethPort,
                                                      const EnetBoard_PortCfg *ethPortCfgs,
                                                      uint32_t numEthPorts)
{
    const EnetBoard_PortCfg *ethPortCfg = NULL;
    bool found = false;
    uint32_t i;

    for (i = 0U; i < numEthPorts; i++)
    {
        ethPortCfg = &ethPortCfgs[i];

        if ((ethPortCfg->enetType == ethPort->enetType) &&
            (ethPortCfg->instId == ethPort->instId) &&
            (ethPortCfg->macPort == ethPort->macPort) &&
            (ethPortCfg->mii.layerType == ethPort->mii.layerType) &&
            (ethPortCfg->mii.sublayerType == ethPort->mii.sublayerType))
        {
            found = true;
            break;
        }
    }

    return found ? ethPortCfg : NULL;
}

int32_t EnetBoard_setupPorts(EnetBoard_EthPort *ethPorts,
                             uint32_t numEthPorts)
{
    uint32_t i;

    /* Nothing else to do */
    for (i = 0U; i < numEthPorts; i++)
    {
        EnetBoard_EthPort *ethPort = &ethPorts[i];
        /* Override the ENET control set by board lib */
        EnetBoard_setEnetControl(ethPort->enetType, ethPort->macPort, &ethPort->mii);

    }
    return ENET_SOK;
}

//Board_STATUS Board_unlockMMR(void);
static void EnetBoard_setEnetControl(Enet_Type enetType,
                                     Enet_MacPort macPort,
                                     EnetMacPort_Interface *mii)
{
    uint32_t modeSel = 0U;
    int32_t status = ENET_SOK;

    if (EnetMacPort_isRmii(mii))
    {
        modeSel = RMII;
    }
    else if (EnetMacPort_isRgmii(mii))
    {
        modeSel = RGMII;
    }
    else if (EnetMacPort_isMii(mii))
    {
        //modeSel = MII;
    }
    else
    {
        EnetAppUtils_print("Invalid MII type: layer %u suyblayer %u\n", mii->layerType, mii->sublayerType);
        EnetAppUtils_assert(false);
    }

    switch (enetType)
    {
        case ENET_CPSW_3G:
            break;

        case ENET_ICSSG_DUALMAC:
        case ENET_ICSSG_SWITCH:
            break;

        default:
            break;
    }

    EnetAppUtils_assert(status == ENET_SOK);
}

void EnetBoard_getMacAddrList(uint8_t macAddr[][ENET_MAC_ADDR_LEN],
                              uint32_t maxMacEntries,
                              uint32_t *pAvailMacEntries)
{
#if STATIC_ADDRESSES
    uint8_t macAddrBuf[ENET_RM_NUM_MACADDR_MAX * BOARD_MAC_ADDR_BYTES];
    Board_STATUS boardStatus;
    uint32_t macAddrCnt, tempCnt;
    uint32_t allocMacEntries = 0;
    uint32_t i, j;

    EnetAppUtils_assert(pAvailMacEntries != NULL);

    if (Board_detectBoard(BOARD_ID_GESI))
    {
        /* Read number of MAC addresses in GESI board */
        boardStatus = Board_readMacAddrCount(BOARD_ID_GESI, &macAddrCnt);
        EnetAppUtils_assert(boardStatus == BOARD_SOK);
        EnetAppUtils_assert(macAddrCnt <= ENET_RM_NUM_MACADDR_MAX);

        /* Read MAC addresses */
        boardStatus = Board_readMacAddr(BOARD_ID_GESI,
                                        macAddrBuf,
                                        sizeof(macAddrBuf),
                                        &tempCnt);
        EnetAppUtils_assert(boardStatus == BOARD_SOK);
        EnetAppUtils_assert(tempCnt == macAddrCnt);

        /* Save only those required to meet the max number of MAC entries */
        macAddrCnt = EnetUtils_min(macAddrCnt, maxMacEntries);
        for (i = 0U; i < macAddrCnt; i++)
        {
            ENET_UTILS_COMPILETIME_ASSERT(ENET_MAC_ADDR_LEN == BOARD_MAC_ADDR_BYTES);
            memcpy(macAddr[i], &macAddrBuf[i * BOARD_MAC_ADDR_BYTES], ENET_MAC_ADDR_LEN);
        }

        allocMacEntries = macAddrCnt;
    }

    *pAvailMacEntries = allocMacEntries;

    if (allocMacEntries == 0U)
    {
        EnetAppUtils_print("EnetBoard_getMacAddrList Failed - IDK not present\n");
        EnetAppUtils_assert(false);
    }
#else
    /*
     * Workaround for EthFw/u-boot I2C conflicts:
     * EthFw reads MAC addresses from GESI and QUAD_ETH boards during EthFw
     * initialization which are stored in EEPROM memories and are read via
     * I2C.  These I2C accesses tend to occur around the same u-boot is also
     * performing I2C accesses, causing transactions to timeout or other
     * similar symptoms.
     *
     * I2C sharing is a known limitation but no current solution exists at
     * this time.  As a temporary workaround, EthFw will use fixed MAC
     * addresses in Linux builds. For RTOS build, MAC addresses will still
     * be read from EEPROM as such I2C contention isn't an problem.
     */
    uint8_t macAddrBuf[][ENET_MAC_ADDR_LEN] =
    {
        { 0x70U, 0xFFU, 0x76U, 0x1DU, 0x92U, 0xC1U },
        { 0x70U, 0xFFU, 0x76U, 0x1DU, 0x92U, 0xC2U },
        { 0x70U, 0xFFU, 0x76U, 0x1DU, 0x92U, 0xC3U },
        { 0x70U, 0xFFU, 0x76U, 0x1DU, 0x92U, 0xC4U },
    };
    uint32_t macAddrCnt = ENET_ARRAYSIZE(macAddrBuf);

    /* Save only those required to meet the max number of MAC entries */
    *pAvailMacEntries = EnetUtils_min(macAddrCnt, maxMacEntries);
    memcpy(&macAddr[0U][0U], &macAddrBuf[0U][0U], *pAvailMacEntries * ENET_MAC_ADDR_LEN);
#endif
}

