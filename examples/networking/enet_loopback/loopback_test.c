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
 * \file  loopback_test.c
 *
 * \brief This file contains the loopback test implementation.
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include "loopback_common.h"
#include "loopback_cfg.h"
#include "FreeRTOS.h"

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

static void EnetLpbk_createRxTxTasks(void);

static uint32_t EnetLpbk_retrieveFreeTxPkts(void);

static void EnetLpbk_txTask(void *args);

static uint32_t EnetLpbk_receivePkts(void);

static void EnetLpbk_rxTask(void *args);

static void EnetLpbk_initCpswCfg(Cpsw_Cfg *cpswCfg);

static int32_t EnetLpbk_setupCpswAle(void);

static int32_t EnetLpbk_openEnet(void);

static void EnetLpbk_closeEnet(void);

static int32_t EnetLpbk_showAlivePhys(void);

static int32_t EnetLpbk_waitForLinkUp(void);

static void EnetLpbk_showCpswStats(void);

static int32_t EnetLpbk_macMode2PhyMii(emac_mode macMode,
                                       EnetPhy_Mii *mii);

static void EnetLpbk_macMode2MacMii(emac_mode macMode,
                                    EnetMacPort_Interface *mii);

static void EnetLpbk_rxIsrFxn(void *appData);

static void EnetLpbk_txIsrFxn(void *appData);

static void EnetLpbk_initTxFreePktQ(void);

static void EnetLpbk_initRxReadyPktQ(void);

static int32_t EnetLpbk_openDma(void);

static void EnetLpbk_closeDma(void);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/* Test application stack */
static uint8_t gEnetLpbkTaskStackTx[ENETLPBK_TASK_STACK_SZ] __attribute__ ((aligned(32)));
static uint8_t gEnetLpbkTaskStackRx[ENETLPBK_TASK_STACK_SZ] __attribute__ ((aligned(32)));

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

int32_t EnetLpbk_loopbackTest(void)
{
    EnetOsal_Cfg osalCfg;
    EnetUtils_Cfg utilsCfg;
    Enet_IoctlPrms prms;
    int32_t status;

    EnetAppUtils_enableClocks(gEnetLpbk.enetType, gEnetLpbk.instId);

    /* Create TX/RX semaphores */
    status = SemaphoreP_constructBinary(&gEnetLpbk.rxSemObj, 0);
    DebugP_assert(SystemP_SUCCESS == status);

    status = SemaphoreP_constructBinary(&gEnetLpbk.rxDoneSemObj, 0);
    DebugP_assert(SystemP_SUCCESS == status);

    status = SemaphoreP_constructBinary(&gEnetLpbk.txSemObj, 0);
    DebugP_assert(SystemP_SUCCESS == status);

    status = SemaphoreP_constructBinary(&gEnetLpbk.txDoneSemObj, 0);
    DebugP_assert(SystemP_SUCCESS == status);

    /* Local core id */
    gEnetLpbk.coreId = EnetSoc_getCoreId();

    /* Initialize Enet driver (use default OSAL and utils) */
    Enet_initOsalCfg(&osalCfg);
    Enet_initUtilsCfg(&utilsCfg);
    Enet_init(&osalCfg, &utilsCfg);

    /* Open Enet driver */
    status = EnetLpbk_openEnet();
    if (status != ENET_SOK)
    {
        EnetAppUtils_print("Failed to open Enet driver: %d\r\n", status);
    }

    if (status == ENET_SOK)
    {
        /* memutils open should happen after Cpsw is opened as it uses CpswUtils_Q
         * functions */
        status = EnetMem_init();
        EnetAppUtils_assert(ENET_SOK == status);
    }

    /* Open DMA driver */
    if (status == ENET_SOK)
    {
        status = EnetLpbk_openDma();
        if (status != ENET_SOK)
        {
            EnetAppUtils_print("Failed to open DMA: %d\r\n", status);
        }
    }

    /* Enable host port */
    if (status == ENET_SOK)
    {
        if (Enet_isCpswFamily(gEnetLpbk.enetType))
        {
            status = EnetLpbk_setupCpswAle();
            if (status != ENET_SOK)
            {
                EnetAppUtils_print("Failed to setup CPSW ALE: %d\r\n", status);
            }
        }

        if (status == ENET_SOK)
        {
            ENET_IOCTL_SET_NO_ARGS(&prms);
            status = Enet_ioctl(gEnetLpbk.hEnet, gEnetLpbk.coreId, ENET_HOSTPORT_IOCTL_ENABLE, &prms);
            if (status != ENET_SOK)
            {
                EnetAppUtils_print("Failed to enable host port: %d\r\n", status);
            }
        }
    }

    /* Show alive PHYs */
    if (status == ENET_SOK)
    {
        status = EnetLpbk_showAlivePhys();
    }

    /* Start timer */
    if (status == ENET_SOK)
    {
        ClockP_start(&gEnetLpbk.tickTimerObj);
    }


    /* Wait for link up */
    if ((status == ENET_SOK) && (gEnetLpbk.testLoopBackType == LOOPBACK_TYPE_PHY))
    {
        status = EnetLpbk_waitForLinkUp();
    }

    /* Do packet transmission and reception */
    if (status == ENET_SOK)
    {
        EnetLpbk_createRxTxTasks();

        SemaphoreP_pend(&gEnetLpbk.txDoneSemObj, SystemP_WAIT_FOREVER);
        SemaphoreP_pend(&gEnetLpbk.rxDoneSemObj, SystemP_WAIT_FOREVER);
    }

    /* Print network statistics */
    if (status == ENET_SOK)
    {
        if (Enet_isCpswFamily(gEnetLpbk.enetType))
        {
            EnetLpbk_showCpswStats();
        }
    }

    /* Disable host port */
    if (status == ENET_SOK)
    {
        ENET_IOCTL_SET_NO_ARGS(&prms);
        status = Enet_ioctl(gEnetLpbk.hEnet, gEnetLpbk.coreId, ENET_HOSTPORT_IOCTL_DISABLE, &prms);
        if (status != ENET_SOK)
        {
            EnetAppUtils_print("Failed to disable host port: %d\r\n", status);
        }
    }

    /* Stop periodic tick timer */
    ClockP_stop(&gEnetLpbk.tickTimerObj);

    /* Print DMA statistics */
    if (status == ENET_SOK)
    {
        EnetAppUtils_showRxChStats(gEnetLpbk.hRxCh);
        EnetAppUtils_showTxChStats(gEnetLpbk.hTxCh);
    }

    /* Close Enet DMA driver */
    EnetLpbk_closeDma();

    /* Close Enet driver */
    EnetLpbk_closeEnet();

    /* Disable peripheral clocks */
    EnetAppUtils_disableClocks(gEnetLpbk.enetType, gEnetLpbk.instId);

    /* Deinit Enet driver */
    Enet_deinit();

    /* Delete all TX/RX semaphores */
    SemaphoreP_destruct(&gEnetLpbk.rxSemObj);
    SemaphoreP_destruct(&gEnetLpbk.rxDoneSemObj);
    SemaphoreP_destruct(&gEnetLpbk.txSemObj);
    SemaphoreP_destruct(&gEnetLpbk.txDoneSemObj);

    EnetAppUtils_print("Test complete: %s\r\n", (status == ENET_SOK) ? "PASS" : "FAIL");

    return status;
}

/* ========================================================================== */
/*                   Static Function Definitions                              */
/* ========================================================================== */

static void EnetLpbk_createRxTxTasks(void)
{
    TaskP_Params taskParams;
    int32_t status;

    TaskP_Params_init(&taskParams);
    taskParams.priority       = 3U;
    taskParams.stack          = gEnetLpbkTaskStackRx;
    taskParams.stackSize      = sizeof(gEnetLpbkTaskStackRx);
    taskParams.args           = (void*)&gEnetLpbk.rxSemObj;
    taskParams.name           = "Rx Task";
    taskParams.taskMain       = &EnetLpbk_rxTask;

    status = TaskP_construct(&gEnetLpbk.rxTaskObj, &taskParams);
    DebugP_assert(SystemP_SUCCESS == status);

    TaskP_Params_init(&taskParams);
    taskParams.priority       = 2U;
    taskParams.stack          = gEnetLpbkTaskStackTx;
    taskParams.stackSize      = sizeof(gEnetLpbkTaskStackTx);
    taskParams.args           = (void*)&gEnetLpbk.txSemObj;
    taskParams.name           = "Tx Task";
    taskParams.taskMain       = &EnetLpbk_txTask;

    status = TaskP_construct(&gEnetLpbk.txTaskObj, &taskParams);
    DebugP_assert(SystemP_SUCCESS == status);
}

static uint32_t EnetLpbk_retrieveFreeTxPkts(void)
{
    EnetDma_PktQ txFreeQ;
    EnetDma_Pkt *pktInfo;
    int32_t status;
    uint32_t txFreeQCnt = 0U;

    EnetQueue_initQ(&txFreeQ);

    /* Retrieve any CPSW packets that may be free now */
    status = EnetDma_retrieveTxPktQ(gEnetLpbk.hTxCh, &txFreeQ);
    if (status == ENET_SOK)
    {
        txFreeQCnt = EnetQueue_getQCount(&txFreeQ);

        pktInfo = (EnetDma_Pkt *)EnetQueue_deq(&txFreeQ);
        while (NULL != pktInfo)
        {
            EnetDma_checkPktState(&pktInfo->pktState,
                                  ENET_PKTSTATE_MODULE_APP,
                                  ENET_PKTSTATE_APP_WITH_DRIVER,
                                  ENET_PKTSTATE_APP_WITH_FREEQ);

            EnetQueue_enq(&gEnetLpbk.txFreePktInfoQ, &pktInfo->node);
            pktInfo = (EnetDma_Pkt *)EnetQueue_deq(&txFreeQ);
        }
    }
    else
    {
        EnetAppUtils_print("retrieveFreeTxPkts() failed to retrieve pkts: %d\r\n",
                           status);
    }

    return txFreeQCnt;
}

static void EnetLpbk_txTask(void *args)
{
    EnetDma_PktQ txSubmitQ;
    EnetDma_Pkt *pktInfo;
    EthFrame *frame;
    uint32_t txRetrievePktCnt;
    uint32_t loopCnt, pktCnt;
    int32_t status = ENET_SOK;
    uint8_t bcastAddr[ENET_MAC_ADDR_LEN] = {0xffU, 0xffU, 0xffU, 0xffU, 0xffU, 0xffU};

    gEnetLpbk.totalTxCnt = 0U;
    for (loopCnt = 0U; loopCnt < ENETLPBK_NUM_ITERATION; loopCnt++)
    {
        pktCnt = 0U;
        while (pktCnt < ENETLPBK_TEST_PKT_NUM)
        {
            /* Transmit a single packet */
            EnetQueue_initQ(&txSubmitQ);

            /* Dequeue one free TX Eth packet */
            pktInfo = (EnetDma_Pkt *)EnetQueue_deq(&gEnetLpbk.txFreePktInfoQ);

            while (NULL != pktInfo)
            {
                pktCnt++;
                /* Fill the TX Eth frame with test content */
                frame = (EthFrame *)pktInfo->bufPtr;
                memcpy(frame->hdr.dstMac, bcastAddr, ENET_MAC_ADDR_LEN);
                memcpy(frame->hdr.srcMac, &gEnetLpbk.hostMacAddr[0U], ENET_MAC_ADDR_LEN);
                frame->hdr.etherType = Enet_htons(ETHERTYPE_EXPERIMENTAL1);
                memset(&frame->payload[0U], (uint8_t)(0xA5 + pktCnt), ENETLPBK_TEST_PKT_LEN);
                pktInfo->userBufLen = ENETLPBK_TEST_PKT_LEN + sizeof(EthFrameHeader);
                pktInfo->appPriv    = &gEnetLpbk;
                EnetDma_checkPktState(&pktInfo->pktState,
                                      ENET_PKTSTATE_MODULE_APP,
                                      ENET_PKTSTATE_APP_WITH_FREEQ,
                                      ENET_PKTSTATE_APP_WITH_DRIVER);

                /* Enqueue the packet for later transmission */
                EnetQueue_enq(&txSubmitQ, &pktInfo->node);

                if (pktCnt >= ENETLPBK_TEST_PKT_NUM)
                {
                    break;
                }

                /* Dequeue one free TX Eth packet */
                pktInfo = (EnetDma_Pkt *)EnetQueue_deq(&gEnetLpbk.txFreePktInfoQ);
            }

            while (0U != EnetQueue_getQCount(&txSubmitQ))
            {
                uint32_t txCnt = EnetQueue_getQCount(&txSubmitQ);
                status = EnetDma_submitTxPktQ(gEnetLpbk.hTxCh,
                                              &txSubmitQ);
                SemaphoreP_pend(&gEnetLpbk.txSemObj, SystemP_WAIT_FOREVER);

                /* Retrieve TX free packets */
                if (status == ENET_SOK)
                {
                    txCnt            = txCnt - EnetQueue_getQCount(&txSubmitQ);
                    txRetrievePktCnt = 0U;
                    while (txRetrievePktCnt != txCnt)
                    {
                        /* This is not failure as HW is busy sending packets, we
                         * need to wait and again call retrieve packets */
                        ClockP_usleep(1000);
                        txRetrievePktCnt += EnetLpbk_retrieveFreeTxPkts();
                    }
                }
                else
                {
                    break;
                }
            }
        }

        gEnetLpbk.totalTxCnt += pktCnt;
    }

    EnetAppUtils_print("Transmitted %d packets \r\n", gEnetLpbk.totalTxCnt);
    SemaphoreP_post(&gEnetLpbk.txDoneSemObj);

    EnetAppUtils_print("Delete EnetLpbk_txTask() and exit..\r\n");
    TaskP_destruct(&gEnetLpbk.txTaskObj);
    TaskP_exit();
}

static uint32_t EnetLpbk_receivePkts(void)
{
    EnetDma_PktQ rxReadyQ;
    EnetDma_Pkt *pktInfo;
    int32_t status;
    uint32_t rxReadyCnt = 0U;

    EnetQueue_initQ(&rxReadyQ);

    /* Retrieve any CPSW packets which are ready */
    status = EnetDma_retrieveRxPktQ(gEnetLpbk.hRxCh, &rxReadyQ);
    if (status == ENET_SOK)
    {
        rxReadyCnt = EnetQueue_getQCount(&rxReadyQ);

        /* Queue the received packet to rxReadyQ and pass new ones from rxFreeQ
        **/
        pktInfo = (EnetDma_Pkt *)EnetQueue_deq(&rxReadyQ);
        while (pktInfo != NULL)
        {
            EnetDma_checkPktState(&pktInfo->pktState,
                                  ENET_PKTSTATE_MODULE_APP,
                                  ENET_PKTSTATE_APP_WITH_DRIVER,
                                  ENET_PKTSTATE_APP_WITH_READYQ);

            EnetQueue_enq(&gEnetLpbk.rxReadyQ, &pktInfo->node);
            pktInfo = (EnetDma_Pkt *)EnetQueue_deq(&rxReadyQ);
        }
    }
    else
    {
        EnetAppUtils_print("receivePkts() failed to retrieve pkts: %d\r\n",
                           status);
    }

    return rxReadyCnt;
}

static void EnetLpbk_rxTask(void *args)
{
    EnetDma_Pkt *pktInfo;
    EthFrame *frame;
    uint32_t rxReadyCnt;
    uint32_t loopCnt, loopRxPktCnt;
    int32_t status = ENET_SOK;

    gEnetLpbk.totalRxCnt = 0U;

    for (loopCnt = 0U; loopCnt < ENETLPBK_NUM_ITERATION; loopCnt++)
    {
        loopRxPktCnt = 0U;
        /* Wait for packet reception */
        do
        {
            SemaphoreP_pend(&gEnetLpbk.rxSemObj, SystemP_WAIT_FOREVER);
            /* Get the packets received so far */
            rxReadyCnt = EnetLpbk_receivePkts();
            if (rxReadyCnt > 0U)
            {
                /* Consume the received packets and release them */
                pktInfo = (EnetDma_Pkt *)EnetQueue_deq(&gEnetLpbk.rxReadyQ);
                while (NULL != pktInfo)
                {
                    EnetDma_checkPktState(&pktInfo->pktState,
                                          ENET_PKTSTATE_MODULE_APP,
                                          ENET_PKTSTATE_APP_WITH_READYQ,
                                          ENET_PKTSTATE_APP_WITH_FREEQ);

                    /* Consume the packet by just printing its content */
                    if (gEnetLpbk.printFrame)
                    {
                        frame = (EthFrame *)pktInfo->bufPtr;

                        EnetAppUtils_printFrame(frame,
                                                pktInfo->userBufLen - sizeof(EthFrameHeader));
                    }

                    /* Release the received packet */
                    EnetQueue_enq(&gEnetLpbk.rxFreeQ, &pktInfo->node);
                    pktInfo = (EnetDma_Pkt *)EnetQueue_deq(&gEnetLpbk.rxReadyQ);
                }

                /*Submit now processed buffers */
                if (status == ENET_SOK)
                {
                    EnetAppUtils_validatePacketState(&gEnetLpbk.rxFreeQ,
                                                     ENET_PKTSTATE_APP_WITH_FREEQ,
                                                     ENET_PKTSTATE_APP_WITH_DRIVER);

                    EnetDma_submitRxPktQ(gEnetLpbk.hRxCh,
                                         &gEnetLpbk.rxFreeQ);
                }
            }

            loopRxPktCnt += rxReadyCnt;
        }
        while (loopRxPktCnt < ENETLPBK_TEST_PKT_NUM);

        gEnetLpbk.totalRxCnt += loopRxPktCnt;
    }

    if (status != ENET_SOK)
    {
        EnetAppUtils_print("Failed to transmit/receive packets: %d, transmitted: %d \r\n", ENETLPBK_TEST_PKT_NUM, gEnetLpbk.totalRxCnt);
    }
    else
    {
        EnetAppUtils_print("Received %d packets\r\n", gEnetLpbk.totalRxCnt);
    }

    SemaphoreP_post(&gEnetLpbk.rxDoneSemObj);

    EnetAppUtils_print("Delete EnetLpbk_rxTask() and exit..\r\n");
    TaskP_destruct(&gEnetLpbk.rxTaskObj);
    TaskP_exit();
}

static void EnetLpbk_initCpswCfg(Cpsw_Cfg *cpswCfg)
{
    CpswHostPort_Cfg *hostPortCfg = &cpswCfg->hostPortCfg;
    CpswAle_Cfg *aleCfg = &cpswCfg->aleCfg;
    CpswCpts_Cfg *cptsCfg = &cpswCfg->cptsCfg;

    /* Set initial config */
    Enet_initCfg(gEnetLpbk.enetType, gEnetLpbk.instId, cpswCfg, sizeof(*cpswCfg));

    /* Peripheral config */
    cpswCfg->vlanCfg.vlanAware = false;

    /* Host port config */
    hostPortCfg->removeCrc      = true;
    hostPortCfg->padShortPacket = true;
    hostPortCfg->passCrcErrors  = true;

    /* ALE config */
    aleCfg->modeFlags                          = CPSW_ALE_CFG_MODULE_EN;
    aleCfg->agingCfg.autoAgingEn               = true;
    aleCfg->agingCfg.agingPeriodInMs           = 1000;
    aleCfg->nwSecCfg.vid0ModeEn                = true;
    aleCfg->vlanCfg.aleVlanAwareMode           = false;
    aleCfg->vlanCfg.cpswVlanAwareMode          = false;
    aleCfg->vlanCfg.unknownUnregMcastFloodMask = CPSW_ALE_ALL_PORTS_MASK;
    aleCfg->vlanCfg.unknownRegMcastFloodMask   = CPSW_ALE_ALL_PORTS_MASK;
    aleCfg->vlanCfg.unknownVlanMemberListMask  = CPSW_ALE_ALL_PORTS_MASK;

    /* CPTS config */
    /* Note: Timestamping and MAC loopback are not supported together because of
     * IP limitation, so disabling timestamping for this application */
    cptsCfg->hostRxTsEn = false;

    EnetAppUtils_initResourceConfig(gEnetLpbk.enetType, gEnetLpbk.coreId, &cpswCfg->resCfg);
}

static int32_t EnetLpbk_setupCpswAle(void)
{
    Enet_IoctlPrms prms;
    CpswAle_SetPortStateInArgs setPortStateInArgs;
    CpswAle_SetUcastEntryInArgs setUcastInArgs;
    uint32_t entryIdx;
    int32_t status;

    /* ALE entry with "secure" bit cleared is required for loopback */
    setUcastInArgs.addr.vlanId  = 0U;
    setUcastInArgs.info.portNum = CPSW_ALE_HOST_PORT_NUM;
    setUcastInArgs.info.blocked = false;
    setUcastInArgs.info.secure  = false;
    setUcastInArgs.info.super   = false;
    setUcastInArgs.info.ageable = false;
    setUcastInArgs.info.trunk   = false;
    EnetUtils_copyMacAddr(&setUcastInArgs.addr.addr[0U], gEnetLpbk.hostMacAddr);
    ENET_IOCTL_SET_INOUT_ARGS(&prms, &setUcastInArgs, &entryIdx);

    status = Enet_ioctl(gEnetLpbk.hEnet, gEnetLpbk.coreId, CPSW_ALE_IOCTL_ADD_UCAST, &prms);
    if (status != ENET_SOK)
    {
        EnetAppUtils_print("Failed to add ucast entry: %d\r\n", status);
    }

    /* Set host port to 'forwarding' state */
    if (status == ENET_SOK)
    {
        setPortStateInArgs.portNum   = CPSW_ALE_HOST_PORT_NUM;
        setPortStateInArgs.portState = CPSW_ALE_PORTSTATE_FORWARD;
        ENET_IOCTL_SET_IN_ARGS(&prms, &setPortStateInArgs);

        status = Enet_ioctl(gEnetLpbk.hEnet, gEnetLpbk.coreId, CPSW_ALE_IOCTL_SET_PORT_STATE, &prms);
        if (status != ENET_SOK)
        {
            EnetAppUtils_print("Failed to set ALE port state: %d\r\n", status);
        }
    }

    return status;
}

static int32_t EnetLpbk_openEnet(void)
{
    Cpsw_Cfg cpswCfg;
    EnetCpdma_Cfg dmaCfg;
    Enet_IoctlPrms prms;
    EnetPer_PortLinkCfg portLinkCfg;
    CpswMacPort_Cfg macCfg;
    int32_t status = ENET_SOK;

    cpswCfg.dmaCfg = &dmaCfg;

    /* Initialize peripheral config */
    EnetLpbk_initCpswCfg(&cpswCfg);

    if (gEnetLpbk.enetType == ENET_CPSW_2G)
    {
        EnetAppUtils_print("CPSW_2G Test\r\n");
    } 
	if (gEnetLpbk.enetType == ENET_CPSW_3G)
    {
        EnetAppUtils_print("CPSW_3G Test\r\n");
    }

    /* Set Enet global runtime log level */
    Enet_setTraceLevel(ENET_TRACE_DEBUG);

    /* Open the Enet driver */
    gEnetLpbk.hEnet = Enet_open(gEnetLpbk.enetType, gEnetLpbk.instId, &cpswCfg, sizeof(cpswCfg));
    if (gEnetLpbk.hEnet == NULL)
    {
        EnetAppUtils_print("Failed to open Enet driver\r\n");
        status = ENET_EFAIL;
    }

    /* Setup port link open parameters */
    if (status == ENET_SOK)
    {
        EnetBoard_EthPort ethPort;
        EnetMacPort_LinkCfg *linkCfg = &portLinkCfg.linkCfg;
        EnetMacPort_Interface *mii = &portLinkCfg.mii;
        EnetPhy_Cfg *phyCfg = &portLinkCfg.phyCfg;
        EnetPhy_Mii phyMii;

        /* Setup board for requested Ethernet port */
        ethPort.enetType = gEnetLpbk.enetType;
        ethPort.instId   = gEnetLpbk.instId;
        ethPort.macPort  = gEnetLpbk.macPort;
        ethPort.boardId  = gEnetLpbk.boardId;
        EnetLpbk_macMode2MacMii(gEnetLpbk.macMode, &ethPort.mii);

        status = EnetBoard_setupPorts(&ethPort, 1U);
        EnetAppUtils_assert(status == ENET_SOK);

        /* Set port link params */
        portLinkCfg.macPort = gEnetLpbk.macPort;
        portLinkCfg.macCfg = &macCfg;

        CpswMacPort_initCfg(&macCfg);
        EnetLpbk_macMode2MacMii(gEnetLpbk.macMode, mii);

        if (gEnetLpbk.testLoopBackType == LOOPBACK_TYPE_PHY)
        {
            const EnetBoard_PhyCfg *boardPhyCfg = NULL;

            /* Set PHY configuration params */
            EnetPhy_initCfg(phyCfg);
            status = EnetLpbk_macMode2PhyMii(gEnetLpbk.macMode, &phyMii);

            if (status == ENET_SOK)
            {
                boardPhyCfg = EnetBoard_getPhyCfg(&ethPort);
                if (boardPhyCfg != NULL)
                {
                    phyCfg->phyAddr     = boardPhyCfg->phyAddr;
                    phyCfg->isStrapped  = boardPhyCfg->isStrapped;
                    phyCfg->skipExtendedCfg = boardPhyCfg->skipExtendedCfg;
                    phyCfg->extendedCfgSize = boardPhyCfg->extendedCfgSize;
                    memcpy(phyCfg->extendedCfg, boardPhyCfg->extendedCfg, phyCfg->extendedCfgSize);
                }
                else
                {
                    EnetAppUtils_print("PHY info not found\r\n");
                    EnetAppUtils_assert(false);
                }

                if ((phyMii == ENETPHY_MAC_MII_MII) ||
                    (phyMii == ENETPHY_MAC_MII_RMII))
                {
                    linkCfg->speed = ENET_SPEED_100MBIT;
                }
                else
                {
                    /* Speed is always 100 Mbits on AM273x */
                    linkCfg->speed = ENET_SPEED_100MBIT;
                }

                linkCfg->duplexity = ENET_DUPLEX_FULL;
            }
        }
        else
        {
            phyCfg->phyAddr = ENETPHY_INVALID_PHYADDR;

            if (mii->layerType == ENET_MAC_LAYER_MII)
            {
                linkCfg->speed = ENET_SPEED_100MBIT;
            }
            else
            {
                linkCfg->speed = ENET_SPEED_1GBIT;
            }

            linkCfg->duplexity = ENET_DUPLEX_FULL;
        }

        /* MAC and PHY loopbacks are mutually exclusive */
        phyCfg->loopbackEn = (gEnetLpbk.testLoopBackType == LOOPBACK_TYPE_PHY);
        macCfg.loopbackEn = (gEnetLpbk.testLoopBackType == LOOPBACK_TYPE_MAC);
    }

    /* Open port link */
    if (status == ENET_SOK)
    {
        ENET_IOCTL_SET_IN_ARGS(&prms, &portLinkCfg);

        status = Enet_ioctl(gEnetLpbk.hEnet, gEnetLpbk.coreId, ENET_PER_IOCTL_OPEN_PORT_LINK, &prms);
        if (status != ENET_SOK)
        {
            EnetAppUtils_print("Failed to open port link: %d\r\n", status);
        }
    }

    return status;
}

static void EnetLpbk_closeEnet(void)
{
    Enet_IoctlPrms prms;
    int32_t status;

    /* Close port link */
    ENET_IOCTL_SET_IN_ARGS(&prms, &gEnetLpbk.macPort);

    status = Enet_ioctl(gEnetLpbk.hEnet, gEnetLpbk.coreId, ENET_PER_IOCTL_CLOSE_PORT_LINK, &prms);
    if (status != ENET_SOK)
    {
        EnetAppUtils_print("Failed to close port link: %d\r\n", status);
    }

    /* Close Enet driver */
    Enet_close(gEnetLpbk.hEnet);

    gEnetLpbk.hEnet = NULL;
}

static int32_t EnetLpbk_showAlivePhys(void)
{
    Enet_IoctlPrms prms;
    bool alive = false;
    int8_t i;
    int32_t status;

    for (i = 0U; i < ENET_MDIO_PHY_CNT_MAX; i++)
    {
        ENET_IOCTL_SET_INOUT_ARGS(&prms, &i, &alive);

        status = Enet_ioctl(gEnetLpbk.hEnet, gEnetLpbk.coreId, ENET_MDIO_IOCTL_IS_ALIVE, &prms);
        if (status == ENET_SOK)
        {
            if (alive == true)
            {
                EnetAppUtils_print("PHY %u is alive\r\n", i);
            }
        }
        else
        {
            EnetAppUtils_print("Failed to get PHY %u alive status: %d\r\n", i, status);
        }
    }

    return status;
}

static int32_t EnetLpbk_waitForLinkUp(void)
{
    Enet_IoctlPrms prms;
    bool linked = false;
    int32_t status = ENET_SOK;

    ENET_IOCTL_SET_INOUT_ARGS(&prms, &gEnetLpbk.macPort, &linked);

    while (!linked)
    {
        status = Enet_ioctl(gEnetLpbk.hEnet, gEnetLpbk.coreId, ENET_PER_IOCTL_IS_PORT_LINK_UP, &prms);
        if (status != ENET_SOK)
        {
            EnetAppUtils_print("Failed to get port %u's link status: %d\r\n",
                            ENET_MACPORT_ID(gEnetLpbk.macPort), status);
            linked = false;
            break;
        }

        if (!linked)
        {
            EnetUtils_delay(10U);
        }
    }

    return status;
}

static void EnetLpbk_showCpswStats(void)
{
    Enet_IoctlPrms prms;
    CpswStats_PortStats portStats;
    int32_t status;

    /* Show host port statistics */
    ENET_IOCTL_SET_OUT_ARGS(&prms, &portStats);
    status = Enet_ioctl(gEnetLpbk.hEnet, gEnetLpbk.coreId, ENET_STATS_IOCTL_GET_HOSTPORT_STATS, &prms);
    if (status == ENET_SOK)
    {
        EnetAppUtils_print("\r\n Port 0 Statistics\r\n");
        EnetAppUtils_print("-----------------------------------------\r\n");
        EnetAppUtils_printHostPortStats2G((CpswStats_HostPort_2g *)&portStats);
        EnetAppUtils_print("\r\n");
    }
    else
    {
        EnetAppUtils_print("Failed to get host stats: %d\r\n", status);
    }

    /* Show MAC port statistics */
    if (status == ENET_SOK)
    {
        ENET_IOCTL_SET_INOUT_ARGS(&prms, &gEnetLpbk.macPort, &portStats);
        status = Enet_ioctl(gEnetLpbk.hEnet, gEnetLpbk.coreId, ENET_STATS_IOCTL_GET_MACPORT_STATS, &prms);
        if (status == ENET_SOK)
        {
            EnetAppUtils_print("\r\n Port 1 Statistics\r\n");
            EnetAppUtils_print("-----------------------------------------\r\n");
            EnetAppUtils_printMacPortStats2G((CpswStats_MacPort_2g *)&portStats);
            EnetAppUtils_print("\r\n");
        }
        else
        {
            EnetAppUtils_print("Failed to get MAC stats: %d\r\n", status);
        }
    }
}

static int32_t EnetLpbk_macMode2PhyMii(emac_mode macMode,
                                    EnetPhy_Mii *mii)
{
    int32_t status = ENET_SOK;

    switch (macMode)
    {
        case RMII:
            *mii = ENETPHY_MAC_MII_RMII;
            break;

        case RGMII:
            *mii = ENETPHY_MAC_MII_RGMII;
            break;
        default:
            status = ENET_EFAIL;
            EnetAppUtils_print("Invalid MAC mode: %u\r\n", macMode);
            EnetAppUtils_assert(false);
            break;
    }

    return status;
}

static void EnetLpbk_macMode2MacMii(emac_mode macMode,
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

static void EnetLpbk_rxIsrFxn(void *appData)
{
    SemaphoreP_post(&gEnetLpbk.rxSemObj);
}

static void EnetLpbk_txIsrFxn(void *appData)
{
    SemaphoreP_post(&gEnetLpbk.txSemObj);
}

static void EnetLpbk_initTxFreePktQ(void)
{
    EnetDma_Pkt *pPktInfo;
    uint32_t i;

    /* Initialize all queues */
    EnetQueue_initQ(&gEnetLpbk.txFreePktInfoQ);

    /* Initialize TX EthPkts and queue them to txFreePktInfoQ */
    for (i = 0U; i < ENET_MEM_NUM_TX_PKTS; i++)
    {
        pPktInfo = EnetMem_allocEthPkt(&gEnetLpbk,
                                       ENET_MEM_LARGE_POOL_PKT_SIZE,
                                       ENETDMA_CACHELINE_ALIGNMENT);
        EnetAppUtils_assert(pPktInfo != NULL);
        ENET_UTILS_SET_PKT_APP_STATE(&pPktInfo->pktState, ENET_PKTSTATE_APP_WITH_FREEQ);

        EnetQueue_enq(&gEnetLpbk.txFreePktInfoQ, &pPktInfo->node);
    }

    EnetAppUtils_print("initQs() txFreePktInfoQ initialized with %d pkts\r\n",
                       EnetQueue_getQCount(&gEnetLpbk.txFreePktInfoQ));
}

static void EnetLpbk_initRxReadyPktQ(void)
{
    EnetDma_PktQ rxReadyQ;
    EnetDma_Pkt *pPktInfo;
    int32_t status;
    uint32_t i;

    EnetQueue_initQ(&gEnetLpbk.rxFreeQ);
    EnetQueue_initQ(&gEnetLpbk.rxReadyQ);
    EnetQueue_initQ(&rxReadyQ);

    for (i = 0U; i < ENET_MEM_NUM_RX_PKTS; i++)
    {
        pPktInfo = EnetMem_allocEthPkt(&gEnetLpbk,
                                       ENET_MEM_LARGE_POOL_PKT_SIZE,
                                       ENETDMA_CACHELINE_ALIGNMENT);
        EnetAppUtils_assert(pPktInfo != NULL);
        ENET_UTILS_SET_PKT_APP_STATE(&pPktInfo->pktState, ENET_PKTSTATE_APP_WITH_FREEQ);
        EnetQueue_enq(&gEnetLpbk.rxFreeQ, &pPktInfo->node);
    }

    /* Retrieve any CPSW packets which are ready */
    status = EnetDma_retrieveRxPktQ(gEnetLpbk.hRxCh, &rxReadyQ);
    EnetAppUtils_assert(status == ENET_SOK);
    /* There should not be any packet with DMA during init */
    EnetAppUtils_assert(EnetQueue_getQCount(&rxReadyQ) == 0U);

    EnetAppUtils_validatePacketState(&gEnetLpbk.rxFreeQ,
                                     ENET_PKTSTATE_APP_WITH_FREEQ,
                                     ENET_PKTSTATE_APP_WITH_DRIVER);

    EnetDma_submitRxPktQ(gEnetLpbk.hRxCh,
                         &gEnetLpbk.rxFreeQ);

    /* Assert here as during init no. of DMA descriptors should be equal to
     * no. of free Ethernet buffers available with app */

    EnetAppUtils_assert(0U == EnetQueue_getQCount(&gEnetLpbk.rxFreeQ));
}

static int32_t EnetLpbk_openDma(void)
{
    int32_t status = ENET_SOK;
    EnetCpdma_OpenRxChPrms rxChCfg;
    EnetCpdma_OpenTxChPrms txChCfg;

    /* Open the CPSW TX channel  */
    if (status == ENET_SOK)
    {
        EnetDma_initTxChParams(&txChCfg);

        txChCfg.cbArg   = &gEnetLpbk;
        txChCfg.notifyCb = EnetLpbk_txIsrFxn;

        EnetAppUtils_setCommonTxChPrms(&txChCfg);

        EnetAppUtils_openTxCh(gEnetLpbk.hEnet,
                              gEnetLpbk.coreKey,
                              gEnetLpbk.coreId,
                              &gEnetLpbk.txChNum,
                              &gEnetLpbk.hTxCh,
                              &txChCfg);

        EnetLpbk_initTxFreePktQ();

        if (NULL != gEnetLpbk.hTxCh)
        {
            status = ENET_SOK;
            if (ENET_SOK != status)
            {
                EnetAppUtils_print("EnetUdma_startTxCh() failed: %d\r\n", status);
                status = ENET_EFAIL;
            }
        }
        else
        {
            EnetAppUtils_print("EnetDma_openTxCh() failed to open: %d\r\n",
                               status);
            status = ENET_EFAIL;
        }
    }

    /* Open the CPSW RX flow  */
    if (status == ENET_SOK)
    {
        EnetDma_initRxChParams(&rxChCfg);

        rxChCfg.notifyCb = EnetLpbk_rxIsrFxn;
        rxChCfg.cbArg   = &gEnetLpbk;

        EnetAppUtils_setCommonRxChPrms(&rxChCfg);
        EnetAppUtils_openRxCh(gEnetLpbk.hEnet,
                              gEnetLpbk.coreKey,
                              gEnetLpbk.coreId,
                              &gEnetLpbk.rxChNum,
                              &gEnetLpbk.hRxCh,
                              &rxChCfg);

        if (NULL == gEnetLpbk.hRxCh)
        {
            EnetAppUtils_print("EnetDma_openRxCh() failed to open: %d\r\n",
                               status);
            EnetAppUtils_assert(NULL != gEnetLpbk.hRxCh);
        }
        else
        {
            //AM273x openRxCh doesn't allocate MAC addresses which seems correct behaviour
            status = EnetAppUtils_allocMac(gEnetLpbk.hEnet,
                                           gEnetLpbk.coreKey,
                                           gEnetLpbk.coreId,
                                           gEnetLpbk.hostMacAddr);
            EnetAppUtils_print("Host MAC address: ");
            EnetAppUtils_printMacAddr(gEnetLpbk.hostMacAddr);
            /* Submit all ready RX buffers to DMA.*/
            EnetLpbk_initRxReadyPktQ();
        }
    }

    return status;
}

static void EnetLpbk_closeDma(void)
{
    EnetDma_PktQ fqPktInfoQ;
    EnetDma_PktQ cqPktInfoQ;

    EnetQueue_initQ(&fqPktInfoQ);
    EnetQueue_initQ(&cqPktInfoQ);

    /* There should not be any ready packet */
    EnetAppUtils_assert(0U == EnetQueue_getQCount(&gEnetLpbk.rxReadyQ));

    /* Close RX channel */
    EnetAppUtils_closeRxCh(gEnetLpbk.hEnet,
                           gEnetLpbk.coreKey,
                           gEnetLpbk.coreId,
                           &fqPktInfoQ,
                           &cqPktInfoQ,
                           gEnetLpbk.hRxCh,
                           gEnetLpbk.rxChNum);

    EnetAppUtils_freePktInfoQ(&fqPktInfoQ);
    EnetAppUtils_freePktInfoQ(&cqPktInfoQ);

    /* Close TX channel */
    EnetQueue_initQ(&fqPktInfoQ);
    EnetQueue_initQ(&cqPktInfoQ);

    EnetAppUtils_closeTxCh(gEnetLpbk.hEnet,
                           gEnetLpbk.coreKey,
                           gEnetLpbk.coreId,
                           &fqPktInfoQ,
                           &cqPktInfoQ,
                           gEnetLpbk.hTxCh,
                           gEnetLpbk.txChNum);
    EnetAppUtils_freePktInfoQ(&fqPktInfoQ);
    EnetAppUtils_freePktInfoQ(&cqPktInfoQ);

    EnetAppUtils_freePktInfoQ(&gEnetLpbk.rxFreeQ);
    EnetAppUtils_freePktInfoQ(&gEnetLpbk.txFreePktInfoQ);

    EnetMem_deInit();
}

