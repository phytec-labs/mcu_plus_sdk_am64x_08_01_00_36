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
 * \file  l2_cpsw_cfg.c
 *
 * \brief This file contains the implementation of the APIs for peripheral configuration for l2 cpsw example.
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */
#include "l2_cpsw_common.h"
#include "l2_cpsw_cfg.h"
#include "l2_cpsw_dataflow.h"

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

extern uint32_t Board_getEthBoardId(void);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

int32_t EnetApp_init(void)
{
    EnetOsal_Cfg osalCfg;
    EnetUtils_Cfg utilsCfg;
    int32_t status = ENET_SOK;

    /* Initialize Enet driver (use default OSAL and utils) */
    EnetAppUtils_print("Init Enet's OSAL and utils to use defaults\r\n");
    Enet_initOsalCfg(&osalCfg);
    Enet_initUtilsCfg(&utilsCfg);
    Enet_init(&osalCfg, &utilsCfg);

    gEnetApp.coreId = EnetSoc_getCoreId();

    /* Initialize memory */
    EnetAppUtils_print("Init memory utils\r\n");
    status = EnetMem_init();
    if (status != ENET_SOK)
    {
        EnetAppUtils_print("Failed to initialize memory utils: %d\r\n", status);
        EnetAppUtils_assert(false);
    }

    /* Initialize all queues */
    EnetQueue_initQ(&gEnetApp.txFreePktInfoQ);

    /* Create task, clock and semaphore used for periodic tick */
    if (status == ENET_SOK)
    {
        EnetAppUtils_print("Create clock and task for periodic tick\r\n");
        EnetApp_createClock();
    }

    /* Open UDMA driver which is the same handle to be used for all peripherals */
    if (status == ENET_SOK)
    {
        EnetAppUtils_print("Open Main UDMA driver\r\n");
        gEnetApp.hMainUdmaDrv = EnetAppUtils_udmaOpen(ENET_CPSW_3G, NULL);
        if (gEnetApp.hMainUdmaDrv == NULL)
        {
            EnetAppUtils_print("Failed to open Main UDMA driver: %d\r\n", status);
            status = ENET_EALLOC;
            EnetAppUtils_assert(false);
        }
    }

    return status;
}

void EnetApp_deinit(void)
{
    /* Open UDMA driver which is the same handle to be used for all peripherals */
    EnetAppUtils_print("Close UDMA driver\r\n");
    EnetAppUtils_udmaclose(gEnetApp.hMainUdmaDrv);

    /* Initialize Enet driver (use default OSAL and utils) */
    EnetAppUtils_print("Deinit Enet driver\r\n");
    Enet_deinit();

    /* Delete task, clock and semaphore used for periodic tick */
    EnetAppUtils_print("Delete clock and task for periodic tick\r\n");
    EnetApp_deleteClock();

    /* Deinitialize memory */
    EnetAppUtils_print("Deinit memory utils\r\n");
    EnetMem_deInit();

    EnetAppUtils_print("Deinit complete\r\n");

}

void EnetApp_showMenu(void)
{
    EnetAppUtils_print("\nEnet L2 cpsw Menu:\r\n");
    EnetAppUtils_print(" 's'  -  Print statistics\r\n");
    EnetAppUtils_print(" 'r'  -  Reset statistics\r\n");
    EnetAppUtils_print(" 'm'  -  Show allocated MAC addresses\r\n");
    EnetAppUtils_print(" 'x'  -  Stop the test\r\n\n");
}

void EnetApp_portLinkStatusChangeCb(Enet_MacPort macPort,
                                          bool isLinkUp,
                                          void *appArg)
{
    EnetAppUtils_print("MAC Port %u: link %s\r\n",
                       ENET_MACPORT_ID(macPort), isLinkUp ? "up" : "down");
}

void EnetApp_mdioLinkStatusChange(Cpsw_MdioLinkStateChangeInfo *info,
                                             void *appArg)
{
    static uint32_t linkUpCount = 0;
    if ((info->linkChanged) && (info->isLinked))
    {
        linkUpCount++;
    }
}

void EnetApp_initEnetLinkCbPrms(Cpsw_Cfg *cpswCfg)
{
    cpswCfg->mdioLinkStateChangeCb     = EnetApp_mdioLinkStatusChange;
    cpswCfg->mdioLinkStateChangeCbArg  = &gEnetApp;
    cpswCfg->portLinkStatusChangeCb    = &EnetApp_portLinkStatusChangeCb;
    cpswCfg->portLinkStatusChangeCbArg = &gEnetApp;
}

void EnetApp_initAleConfig(CpswAle_Cfg *aleCfg)
{
    aleCfg->modeFlags = CPSW_ALE_CFG_MODULE_EN;
    aleCfg->agingCfg.autoAgingEn = true;
    aleCfg->agingCfg.agingPeriodInMs = 1000;

    aleCfg->nwSecCfg.vid0ModeEn                = true;
    aleCfg->vlanCfg.aleVlanAwareMode           = FALSE;
    aleCfg->vlanCfg.cpswVlanAwareMode          = FALSE;
    aleCfg->vlanCfg.unknownUnregMcastFloodMask = CPSW_ALE_ALL_PORTS_MASK;
    aleCfg->vlanCfg.unknownRegMcastFloodMask   = CPSW_ALE_ALL_PORTS_MASK;
    aleCfg->vlanCfg.unknownVlanMemberListMask  = CPSW_ALE_ALL_PORTS_MASK;
    aleCfg->policerGlobalCfg.policingEn        = true;
    aleCfg->policerGlobalCfg.yellowDropEn      = false;
    /* Enables the ALE to drop the red colored packets. */
    aleCfg->policerGlobalCfg.redDropEn         = false;
    /* Policing match mode */
    aleCfg->policerGlobalCfg.policerNoMatchMode = CPSW_ALE_POLICER_NOMATCH_MODE_GREEN;
}

int32_t EnetApp_open(EnetApp_PerCtxt *perCtxts,
                           uint32_t numPerCtxts)
{
    EnetUdma_Cfg *dmaCfg;
    EnetRm_ResCfg *resCfg;
    Enet_IoctlPrms prms;
    EnetPer_AttachCoreOutArgs attachCoreOutArgs;

    uint32_t i;
    int32_t status = ENET_SOK;

    /* Do peripheral dependent initalization */
    EnetAppUtils_print("\nInit all peripheral clocks\r\n");
    EnetAppUtils_print("----------------------------------------------\r\n");
    for (i = 0U; i < numPerCtxts; i++)
    {
        EnetApp_PerCtxt *perCtxt = &perCtxts[i];
        EnetAppUtils_enableClocks(perCtxt->enetType, perCtxt->instId);
    }

    /* Prepare init configuration for all peripherals */
    EnetAppUtils_print("\nInit all configs\r\n");
    EnetAppUtils_print("----------------------------------------------\r\n");
    for (i = 0U; i < numPerCtxts; i++)
    {
        EnetApp_PerCtxt *perCtxt = &perCtxts[i];

        dmaCfg = &perCtxt->dmaCfg;
        dmaCfg->rxChInitPrms.dmaPriority = UDMA_DEFAULT_RX_CH_DMA_PRIORITY;
        EnetAppUtils_print("%s: init config\r\n", perCtxt->name);

        Cpsw_Cfg *cpswCfg = &perCtxt->cpswCfg;
        Enet_initCfg(perCtxt->enetType, perCtxt->instId, cpswCfg, sizeof(*cpswCfg));
        cpswCfg->vlanCfg.vlanAware          = false;
        cpswCfg->hostPortCfg.removeCrc      = true;
        cpswCfg->hostPortCfg.padShortPacket = true;
        cpswCfg->hostPortCfg.passCrcErrors  = true;
        EnetApp_initEnetLinkCbPrms(cpswCfg);
        resCfg = &cpswCfg->resCfg;
        EnetApp_initAleConfig(&cpswCfg->aleCfg);
        /* Set DMA configuration */
        dmaCfg->hUdmaDrv = gEnetApp.hMainUdmaDrv;
        cpswCfg->dmaCfg = (void *)dmaCfg;

        /* Initialize RM */
        EnetAppUtils_initResourceConfig(perCtxt->enetType, EnetSoc_getCoreId(), resCfg);

    }
        /* Create RX tasks for each peripheral */
    if (status == ENET_SOK)
    {
        EnetAppUtils_print("\nCreate RX tasks\r\n");
        EnetAppUtils_print("----------------------------------------------\r\n");
        for (i = 0U; i < numPerCtxts; i++)
        {
            EnetApp_PerCtxt *perCtxt = &perCtxts[i];

            EnetAppUtils_print("%s: Create RX task\r\n", perCtxt->name);

            EnetApp_createRxTask(perCtxt);
        }
    }

    /* Open Enet driver for all peripherals */
    EnetAppUtils_print("\nOpen all peripherals\r\n");
    EnetAppUtils_print("----------------------------------------------\r\n");
    for (i = 0U; i < numPerCtxts; i++)
    {
        EnetApp_PerCtxt *perCtxt = &perCtxts[i];
        EnetAppUtils_print("%s: Open enet\r\n", perCtxt->name);
        Cpsw_Cfg *cpswCfg = &perCtxt->cpswCfg;
        perCtxt->hEnet = Enet_open(perCtxt->enetType, perCtxt->instId, cpswCfg, sizeof(*cpswCfg));

        if (perCtxt->hEnet == NULL)
        {
            EnetAppUtils_print("%s: failed to open enet\r\n", perCtxt->name);
            status = ENET_EFAIL;
            break;
        }
    }

    /* Start PHY tick timer */
    if (status == ENET_SOK)
    {
        ClockP_start(&gEnetApp.tickTimerObj);
    }

    /* Attach the core with RM */
    if (status == ENET_SOK)
    {
        EnetAppUtils_print("\nAttach core id %u on all peripherals\r\n", gEnetApp.coreId);
        EnetAppUtils_print("----------------------------------------------\r\n");
        for (i = 0U; i < numPerCtxts; i++)
        {
            EnetApp_PerCtxt *perCtxt = &perCtxts[i];

            EnetAppUtils_print("%s: Attach core\r\n", perCtxt->name);

            ENET_IOCTL_SET_INOUT_ARGS(&prms, &gEnetApp.coreId, &attachCoreOutArgs);

            status = Enet_ioctl(perCtxt->hEnet, gEnetApp.coreId, ENET_PER_IOCTL_ATTACH_CORE, &prms);
            if (status != ENET_SOK)
            {
                EnetAppUtils_print("%s: failed to attach: %d\r\n", perCtxt->name, status);
            }
            else
            {
                gEnetApp.coreKey = attachCoreOutArgs.coreKey;
            }
        }
    }

    if (status == ENET_SOK)
    {
        status = EnetApp_openPort(perCtxts);
        if (status != ENET_SOK)
        {
            EnetAppUtils_print("%s: Failed to open port link: %d\r\n", perCtxts->name, status);
        }
    }

    /* Open DMA for peripheral/port */
    if (status == ENET_SOK)
    {
        EnetAppUtils_print("%s: Open DMA\r\n", perCtxts->name);

        status = EnetApp_openDma(perCtxts);
        if (status != ENET_SOK)
        {
            EnetAppUtils_print("%s: failed to open DMA: %d\r\n", perCtxts->name, status);
        }
    }

    if (status == ENET_SOK)
    {
        status = EnetApp_waitForLinkUp(perCtxts);
        if (status != ENET_SOK)
        {
            EnetAppUtils_print("%s: Failed to wait for link up: %d\r\n", perCtxts->name, status);
        }
    }

    EnetAppUtils_print("%s: MAC port addr: ", perCtxts->name);

    EnetAppUtils_printMacAddr(&perCtxts->macAddr[0U]);

    return status;
}

void EnetApp_close(EnetApp_PerCtxt *perCtxts,
                         uint32_t numPerCtxts)
{
    Enet_IoctlPrms prms;
    uint32_t i;
    int32_t status;

    EnetAppUtils_print("\nClose Ports for all peripherals\r\n");
    EnetAppUtils_print("----------------------------------------------\r\n");
    for (i = 0U; i < numPerCtxts; i++)
    {
        EnetApp_PerCtxt *perCtxt = &perCtxts[i];

        EnetAppUtils_print("%s: Close Port\r\n", perCtxt->name);

        EnetApp_closePort(perCtxt);
    }

    EnetAppUtils_print("\nClose DMA for all peripherals\r\n");
    EnetAppUtils_print("----------------------------------------------\r\n");
    for (i = 0U; i < numPerCtxts; i++)
    {
        EnetApp_PerCtxt *perCtxt = &perCtxts[i];

        EnetAppUtils_print("%s: Close DMA\r\n", perCtxt->name);

        EnetApp_closeDma(perCtxt);
    }

    /* Delete RX tasks created for all peripherals */
    EnetAppUtils_print("\nDelete RX tasks\r\n");
    EnetAppUtils_print("----------------------------------------------\r\n");
    for (i = 0U; i < numPerCtxts; i++)
    {
        EnetApp_destroyRxTask(&perCtxts[i]);
    }

    /* Detach core */
    EnetAppUtils_print("\nDetach core from all peripherals\r\n");
    EnetAppUtils_print("----------------------------------------------\r\n");
    for (i = 0U; i < numPerCtxts; i++)
    {
        EnetApp_PerCtxt *perCtxt = &perCtxts[i];

        EnetAppUtils_print("%s: Detach core\r\n", perCtxt->name);

        ENET_IOCTL_SET_IN_ARGS(&prms, &gEnetApp.coreKey);
        status = Enet_ioctl(perCtxt->hEnet, gEnetApp.coreId, ENET_PER_IOCTL_DETACH_CORE, &prms);
        if (status != ENET_SOK)
        {
            EnetAppUtils_print("%s: Failed to detach: %d\r\n", perCtxt->name);
        }
    }

    /* Close opened Enet drivers if any peripheral failed */
    EnetAppUtils_print("\nClose all peripherals\r\n");
    EnetAppUtils_print("----------------------------------------------\r\n");
    for (i = 0U; i < numPerCtxts; i++)
    {
        EnetApp_PerCtxt *perCtxt = &perCtxts[i];
        EnetAppUtils_print("%s: Close enet\r\n", perCtxt->name);
        Enet_close(perCtxt->hEnet);
        perCtxt->hEnet = NULL;
    }

    /* Do peripheral dependent initalization */
    EnetAppUtils_print("\nDeinit all peripheral clocks\r\n");
    EnetAppUtils_print("----------------------------------------------\r\n");
    for (i = 0U; i < numPerCtxts; i++)
    {
        EnetApp_PerCtxt *perCtxt = &perCtxts[i];
        EnetAppUtils_disableClocks(perCtxt->enetType, perCtxt->instId);
    }
}

void EnetApp_printStats(EnetApp_PerCtxt *perCtxts,
                              uint32_t numPerCtxts)
{
    Enet_IoctlPrms prms;
    Enet_MacPort macPort;
    uint32_t i;
    int32_t status;

    EnetAppUtils_print("\nPrint statistics\r\n");
    EnetAppUtils_print("----------------------------------------------\r\n");
    for (i = 0U; i < numPerCtxts; i++)
    {
        EnetApp_PerCtxt *perCtxt = &gEnetApp.perCtxt[i];

        ENET_IOCTL_SET_OUT_ARGS(&prms, &gEnetApp_cpswStats);

        status = Enet_ioctl(perCtxt->hEnet, gEnetApp.coreId, ENET_STATS_IOCTL_GET_HOSTPORT_STATS, &prms);
        if (status != ENET_SOK)
        {
            EnetAppUtils_print("%s: Failed to get port stats\r\n", perCtxt->name);
            continue;
        }
        EnetAppUtils_printHostPortStats9G((CpswStats_HostPort_Ng *)&gEnetApp_cpswStats);


        macPort = perCtxt->macPort;

        EnetAppUtils_print("\n %s - Port %u statistics\r\n", perCtxt->name, ENET_MACPORT_ID(macPort));
        EnetAppUtils_print("--------------------------------\r\n");

        ENET_IOCTL_SET_INOUT_ARGS(&prms, &macPort, &gEnetApp_cpswStats);

        status = Enet_ioctl(perCtxt->hEnet, gEnetApp.coreId, ENET_STATS_IOCTL_GET_MACPORT_STATS, &prms);
        if (status != ENET_SOK)
        {
            EnetAppUtils_print("%s: Failed to get port %u stats\r\n", perCtxt->name, ENET_MACPORT_ID(macPort));
            continue;
        }

        EnetAppUtils_printMacPortStats9G((CpswStats_MacPort_Ng *)&gEnetApp_cpswStats);

        EnetAppUtils_print("\n");

    }
}

void EnetApp_resetStats(EnetApp_PerCtxt *perCtxts,
                              uint32_t numPerCtxts)
{
    Enet_IoctlPrms prms;
    Enet_MacPort macPort;
    uint32_t i;
    int32_t status;

    EnetAppUtils_print("\nReset statistics\r\n");
    EnetAppUtils_print("----------------------------------------------\r\n");
    for (i = 0U; i < numPerCtxts; i++)
    {
        EnetApp_PerCtxt *perCtxt = &gEnetApp.perCtxt[i];

        EnetAppUtils_print("%s: Reset statistics\r\n", perCtxt->name);

        ENET_IOCTL_SET_NO_ARGS(&prms);
        status = Enet_ioctl(perCtxt->hEnet, gEnetApp.coreId, ENET_STATS_IOCTL_RESET_HOSTPORT_STATS, &prms);
        if (status != ENET_SOK)
        {
            EnetAppUtils_print("%s: Failed to reset  host port stats\r\n", perCtxt->name);
            continue;
        }

        macPort = perCtxt->macPort;

        ENET_IOCTL_SET_IN_ARGS(&prms, &macPort);
        status = Enet_ioctl(perCtxt->hEnet, gEnetApp.coreId, ENET_STATS_IOCTL_RESET_MACPORT_STATS, &prms);
        if (status != ENET_SOK)
        {
            EnetAppUtils_print("%s: Failed to reset port %u stats\r\n", perCtxt->name, ENET_MACPORT_ID(macPort));
            continue;
        }

    }
}

void EnetApp_showMacAddrs(EnetApp_PerCtxt *perCtxts,
                                uint32_t numPerCtxts)
{
    uint32_t i;

    EnetAppUtils_print("\nAllocated MAC addresses\r\n");
    EnetAppUtils_print("----------------------------------------------\r\n");
    for (i = 0U; i < numPerCtxts; i++)
    {
        EnetApp_PerCtxt *perCtxt = &gEnetApp.perCtxt[i];

        EnetAppUtils_print("%s: \t", perCtxt->name);
        EnetAppUtils_printMacAddr(&perCtxt->macAddr[0U]);
    }
}

int32_t EnetApp_openPort(EnetApp_PerCtxt *perCtxt)
{
    Enet_IoctlPrms prms;
    EnetBoard_EthPort ethPort;
    const EnetBoard_PhyCfg *boardPhyCfg;
    EnetPer_PortLinkCfg portLinkCfg;
    CpswMacPort_Cfg cpswMacCfg;
    EnetMacPort_LinkCfg *linkCfg = &portLinkCfg.linkCfg;
    EnetMacPort_Interface *mii = &portLinkCfg.mii;
    EnetPhy_Cfg *phyCfg = &portLinkCfg.phyCfg;
    Enet_MacPort macPort;
    int32_t status = ENET_SOK;

    macPort = perCtxt->macPort;

    EnetAppUtils_print("%s: Open port %u\r\n", perCtxt->name, ENET_MACPORT_ID(macPort));

    /* Setup board for requested Ethernet port */
    ethPort.enetType = perCtxt->enetType;
    ethPort.instId   = perCtxt->instId;
    ethPort.macPort  = macPort;
    ethPort.boardId  = Board_getEthBoardId();
    EnetApp_macMode2MacMii(RGMII, &ethPort.mii);

    status = EnetBoard_setupPorts(&ethPort, 1U);
    if (status != ENET_SOK)
    {
        EnetAppUtils_print("%s: Failed to setup MAC port %u\r\n", perCtxt->name, ENET_MACPORT_ID(macPort));
        EnetAppUtils_assert(false);
    }

    CpswMacPort_initCfg(&cpswMacCfg);
    portLinkCfg.macCfg = &cpswMacCfg;

    /* Set port link params */
    portLinkCfg.macPort = macPort;

    mii->layerType     = ethPort.mii.layerType;
    mii->sublayerType  = ethPort.mii.sublayerType;
    mii->variantType   = ENET_MAC_VARIANT_FORCED;

    linkCfg->speed     = ENET_SPEED_AUTO;
    linkCfg->duplexity = ENET_DUPLEX_AUTO;

    boardPhyCfg = EnetBoard_getPhyCfg(&ethPort);
    if (boardPhyCfg != NULL)
    {
        EnetPhy_initCfg(phyCfg);
        phyCfg->phyAddr     = CONFIG_ENET_CPSW0_PHY0_ADDR;
        phyCfg->isStrapped  = boardPhyCfg->isStrapped;
        phyCfg->loopbackEn  = false;
        phyCfg->skipExtendedCfg = boardPhyCfg->skipExtendedCfg;
        phyCfg->extendedCfgSize = boardPhyCfg->extendedCfgSize;
        /* Setting Tx and Rx skew delay values */
        Board_TxRxDelaySet(boardPhyCfg);
        memcpy(phyCfg->extendedCfg, boardPhyCfg->extendedCfg, phyCfg->extendedCfgSize);

    }
    else
    {
        EnetAppUtils_print("%s: No PHY configuration found\r\n", perCtxt->name);
        EnetAppUtils_assert(false);
    }

    /* Open port link */
    if (status == ENET_SOK)
    {
        ENET_IOCTL_SET_IN_ARGS(&prms, &portLinkCfg);

        EnetAppUtils_print("%s: Open port %u link\r\n", perCtxt->name, ENET_MACPORT_ID(macPort));
        status = Enet_ioctl(perCtxt->hEnet, gEnetApp.coreId, ENET_PER_IOCTL_OPEN_PORT_LINK, &prms);
        if (status != ENET_SOK)
        {
            EnetAppUtils_print("%s: Failed to open port link: %d\r\n", perCtxt->name, status);
        }
    }

    return status;
}

void EnetApp_closePort(EnetApp_PerCtxt *perCtxt)
{
    Enet_IoctlPrms prms;
    Enet_MacPort macPort;
    int32_t status;

    macPort = perCtxt->macPort;

    EnetAppUtils_print("%s: Close port %u\r\n", perCtxt->name, ENET_MACPORT_ID(macPort));

    /* Close port link */
    ENET_IOCTL_SET_IN_ARGS(&prms, &macPort);

    EnetAppUtils_print("%s: Close port %u link\r\n", perCtxt->name, ENET_MACPORT_ID(macPort));
    status = Enet_ioctl(perCtxt->hEnet, gEnetApp.coreId, ENET_PER_IOCTL_CLOSE_PORT_LINK, &prms);
    if (status != ENET_SOK)
    {
        EnetAppUtils_print("%s: Failed to close port link: %d\r\n", perCtxt->name, status);
    }
}

int32_t EnetApp_waitForLinkUp(EnetApp_PerCtxt *perCtxt)
{
    Enet_IoctlPrms prms;
    Enet_MacPort macPort;
    bool linked;
    int32_t status = ENET_SOK;

    EnetAppUtils_print("%s: Waiting for link up...\r\n", perCtxt->name);


    macPort = perCtxt->macPort;
    linked = false;

    while (gEnetApp.run && !linked)
    {
        ENET_IOCTL_SET_INOUT_ARGS(&prms, &macPort, &linked);

        status = Enet_ioctl(perCtxt->hEnet, gEnetApp.coreId, ENET_PER_IOCTL_IS_PORT_LINK_UP, &prms);
        if (status != ENET_SOK)
        {
            EnetAppUtils_print("%s: Failed to get port %u link status: %d\r\n",
                                perCtxt->name, ENET_MACPORT_ID(macPort), status);
            linked = false;
            break;
        }

        if (!linked)
        {
            ClockP_sleep(1);
        }
    }

    if (gEnetApp.run)
    {
        EnetAppUtils_print("%s: Port %u link is %s\r\n",
                            perCtxt->name, ENET_MACPORT_ID(macPort), linked ? "up" : "down");

        /* Set port to 'Forward' state */
        if (status == ENET_SOK)
        {

            CpswAle_SetPortStateInArgs setPortStateInArgs;
            setPortStateInArgs.portNum   = CPSW_ALE_HOST_PORT_NUM;
            setPortStateInArgs.portState = CPSW_ALE_PORTSTATE_FORWARD;
            ENET_IOCTL_SET_IN_ARGS(&prms, &setPortStateInArgs);
            prms.outArgs = NULL;
            status = Enet_ioctl(perCtxt->hEnet,
                                gEnetApp.coreId,
                                CPSW_ALE_IOCTL_SET_PORT_STATE,
                                &prms);
            if (status != ENET_SOK)
            {
                EnetAppUtils_print("%s: CPSW_ALE_IOCTL_SET_PORT_STATE IOCTL failed : %d\r\n",
                                    perCtxt->name, status);
            }
            if (status == ENET_SOK)
            {
                ENET_IOCTL_SET_NO_ARGS(&prms);
                status = Enet_ioctl(perCtxt->hEnet,
                                    gEnetApp.coreId,
                                    ENET_HOSTPORT_IOCTL_ENABLE,
                                    &prms);
                if (status != ENET_SOK)
                {
                    EnetAppUtils_print("%s: Failed to enable host port: %d\r\n", perCtxt->name, status);
                }
            }
        }
    }

    return status;
}

void EnetApp_macMode2MacMii(emac_mode macMode,
                                  EnetMacPort_Interface *mii)
{
    switch (macMode)
    {
        case RMII:
            mii->layerType    = ENET_MAC_LAYER_MII;
            mii->sublayerType = ENET_MAC_SUBLAYER_REDUCED;
            mii->variantType  = ENET_MAC_VARIANT_NONE;
            break;

        case RGMII:
            mii->layerType    = ENET_MAC_LAYER_GMII;
            mii->sublayerType = ENET_MAC_SUBLAYER_REDUCED;
            mii->variantType  = ENET_MAC_VARIANT_FORCED;
            break;
        default:
            EnetAppUtils_print("Invalid MAC mode: %u\r\n", macMode);
            EnetAppUtils_assert(false);
            break;
    }
}