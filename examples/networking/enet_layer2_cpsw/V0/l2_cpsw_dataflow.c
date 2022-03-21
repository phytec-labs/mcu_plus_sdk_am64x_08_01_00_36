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
 * \file  l2_cpsw_dataflow.c
 *
 * \brief This file contains the implementation of the APIs for data flow for L2 cpsw example.
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */
#include "l2_cpsw_common.h"
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

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

void EnetApp_createClock(void)
{
    TaskP_Params taskParams;
    ClockP_Params clkParams;
    int32_t status;
    /* Create timer semaphore */
    status = SemaphoreP_constructCounting(&gEnetApp.timerSemObj, 0, COUNTING_SEM_COUNT);
    DebugP_assert(SystemP_SUCCESS == status);

    /* Initialize the periodic tick task params */
    TaskP_Params_init(&taskParams);
    taskParams.priority       = 7U;
    taskParams.stack          = gEnetAppTaskStackTick;
    taskParams.stackSize      = sizeof(gEnetAppTaskStackTick);
    taskParams.args           = (void*)&gEnetApp.timerSemObj;
    taskParams.name           = "Periodic tick task";
    taskParams.taskMain       = &EnetApp_tickTask;
    /* Create periodic tick task */
    EnetAppUtils_print("Create periodic tick task\r\n");

    status = TaskP_construct(&gEnetApp.tickTaskObj, &taskParams);
    DebugP_assert(SystemP_SUCCESS == status);

    ClockP_Params_init(&clkParams);
    clkParams.timeout    = ENETAPP_PERIODIC_TICK_MS;
    clkParams.period    = ENETAPP_PERIODIC_TICK_MS;
    clkParams.args       = (void*)&gEnetApp.timerSemObj;
    clkParams.callback  = &EnetApp_timerCallback;

    clkParams.start = FALSE;
    /* Creating timer and setting timer callback function */
    EnetAppUtils_print("Create periodic tick clock\r\n");
    status = ClockP_construct(&gEnetApp.tickTimerObj, &clkParams);
    DebugP_assert(SystemP_SUCCESS == status);

}

void EnetApp_deleteClock(void)
{
    /* Delete periodic tick timer */
    EnetAppUtils_print("Delete periodic tick clock\r\n");
    ClockP_destruct(&gEnetApp.tickTimerObj);
    /* Delete periodic tick task */
     TaskP_destruct(&gEnetApp.tickTaskObj);
    /* Delete periodic tick timer */
    SemaphoreP_destruct(&gEnetApp.timerSemObj);

}

void EnetApp_timerCallback(ClockP_Object *clkInst, void * arg)
{
    SemaphoreP_Object* hSem = (SemaphoreP_Object*)arg;

    /* Tick! */
    SemaphoreP_post(hSem);
}

void EnetApp_tickTask(void *args)
{
    SemaphoreP_Object* hSem = (SemaphoreP_Object*)args;
    uint32_t i;

    while (gEnetApp.run)
    {
        SemaphoreP_pend(hSem, SystemP_WAIT_FOREVER);

        /* Periodic tick should be called from non-ISR context */
        for (i = 0U; i < gEnetApp.numPerCtxts; i++)
        {
            Enet_periodicTick(gEnetApp.perCtxt[i].hEnet);
        }
    }
    TaskP_exit();
}

void EnetApp_rxIsrFxn(void *appData)
{
    EnetApp_PerCtxt *perCtxt = (EnetApp_PerCtxt *)appData;

    SemaphoreP_post(&perCtxt->rxSemObj);
}

int32_t EnetApp_openDma(EnetApp_PerCtxt *perCtxt)
{
    EnetUdma_OpenRxFlowPrms rxChCfg;
    EnetUdma_OpenTxChPrms txChCfg;
    int32_t status = ENET_SOK;

    /* Open the TX channel */
    EnetDma_initTxChParams(&txChCfg);

    txChCfg.hUdmaDrv = gEnetApp.hMainUdmaDrv;
    txChCfg.cbArg    = NULL;
    txChCfg.notifyCb = NULL;
    txChCfg.useGlobalEvt = true;

    EnetAppUtils_setCommonTxChPrms(&txChCfg);

    EnetAppUtils_openTxCh(perCtxt->hEnet,
                          gEnetApp.coreKey,
                          gEnetApp.coreId,
                          &perCtxt->txChNum,
                          &perCtxt->hTxCh,
                          &txChCfg);
    if (perCtxt->hTxCh == NULL)
    {
#if FIX_RM
        /* Free the channel number if open Tx channel failed */
        EnetAppUtils_freeTxCh(gEnetApp.hEnet,
                              gEnetApp.coreKey,
                              gEnetApp.coreId,
                              gEnetApp.txChNum);
#endif
        EnetAppUtils_print("EnetApp_openDma() failed to open TX channel\r\n");
        status = ENET_EFAIL;
        EnetAppUtils_assert(perCtxt->hTxCh != NULL);
    }

    /* Allocate TX packets and keep them locally enqueued */
    if (status == ENET_SOK)
    {
        EnetApp_initTxFreePktQ();
    }

    /* Open the RX flow for Regular frames */
    if (status == ENET_SOK)
    {
        EnetDma_initRxChParams(&rxChCfg);

        rxChCfg.hUdmaDrv = gEnetApp.hMainUdmaDrv;
        rxChCfg.notifyCb = EnetApp_rxIsrFxn;
        rxChCfg.cbArg    = perCtxt;
        rxChCfg.useGlobalEvt = true;
        rxChCfg.flowPrms.sizeThreshEn = 0U;

        EnetAppUtils_setCommonRxFlowPrms(&rxChCfg);
        EnetAppUtils_openRxFlowForChIdx(perCtxt->enetType,
                                        perCtxt->hEnet,
                                        gEnetApp.coreKey,
                                        gEnetApp.coreId,
                                        true,
                                        0,
                                        &perCtxt->rxStartFlowIdx,
                                        &perCtxt->rxFlowIdx,
                                        &perCtxt->macAddr[0U],
                                        &perCtxt->hRxCh,
                                        &rxChCfg);
        if (perCtxt->hRxCh == NULL)
        {
            EnetAppUtils_print("EnetApp_openRxCh() failed to open RX flow\r\n");
            status = ENET_EFAIL;
            EnetAppUtils_assert(perCtxt->hRxCh != NULL);
        }
    }

    /* Submit all ready RX buffers to DMA */
    if (status == ENET_SOK)
    {

        EnetApp_initRxReadyPktQ(perCtxt->hRxCh);
    }

     return status;
}

void EnetApp_closeDma(EnetApp_PerCtxt *perCtxt)
{
    EnetDma_PktQ fqPktInfoQ;
    EnetDma_PktQ cqPktInfoQ;

    EnetQueue_initQ(&fqPktInfoQ);
    EnetQueue_initQ(&cqPktInfoQ);

    /* Close Regular RX channel */
    EnetAppUtils_closeRxFlowForChIdx(perCtxt->enetType,
                                        perCtxt->hEnet,
                                        gEnetApp.coreKey,
                                        gEnetApp.coreId,
                                        true,
                                        &fqPktInfoQ,
                                        &cqPktInfoQ,
                                        0,
                                        perCtxt->rxStartFlowIdx,
                                        perCtxt->rxFlowIdx,
                                        perCtxt->macAddr,
                                        perCtxt->hRxCh);

    EnetAppUtils_freePktInfoQ(&fqPktInfoQ);
    EnetAppUtils_freePktInfoQ(&cqPktInfoQ);

    /* Close TX channel */
    EnetQueue_initQ(&fqPktInfoQ);
    EnetQueue_initQ(&cqPktInfoQ);

    /* Retrieve any pending TX packets from driver */
    EnetApp_retrieveFreeTxPkts(perCtxt);

    EnetAppUtils_closeTxCh(perCtxt->hEnet,
                           gEnetApp.coreKey,
                           gEnetApp.coreId,
                           &fqPktInfoQ,
                           &cqPktInfoQ,
                           perCtxt->hTxCh,
                           perCtxt->txChNum);

    EnetAppUtils_freePktInfoQ(&fqPktInfoQ);
    EnetAppUtils_freePktInfoQ(&cqPktInfoQ);

    EnetAppUtils_freePktInfoQ(&gEnetApp.txFreePktInfoQ);
}

void EnetApp_initTxFreePktQ(void)
{
    EnetDma_Pkt *pPktInfo;
    uint32_t i;

    /* Initialize TX EthPkts and queue them to txFreePktInfoQ */
    for (i = 0U; i < ENET_MEM_NUM_TX_PKTS; i++)
    {
        pPktInfo = EnetMem_allocEthPkt(&gEnetApp,
                                       ENET_MEM_LARGE_POOL_PKT_SIZE,
                                       ENETDMA_CACHELINE_ALIGNMENT);
        EnetAppUtils_assert(pPktInfo != NULL);
        ENET_UTILS_SET_PKT_APP_STATE(&pPktInfo->pktState, ENET_PKTSTATE_APP_WITH_FREEQ);

        EnetQueue_enq(&gEnetApp.txFreePktInfoQ, &pPktInfo->node);
    }

    EnetAppUtils_print("initQs() txFreePktInfoQ initialized with %d pkts\r\n",
                       EnetQueue_getQCount(&gEnetApp.txFreePktInfoQ));
}

void EnetApp_initRxReadyPktQ(EnetDma_RxChHandle hRxCh)
{
    EnetDma_PktQ rxReadyQ;
    EnetDma_PktQ rxFreeQ;
    EnetDma_Pkt *pPktInfo;
    uint32_t i;
    int32_t status;

    EnetQueue_initQ(&rxFreeQ);

    for (i = 0U; i < ENET_MEM_NUM_RX_PKTS; i++)
    {
        pPktInfo = EnetMem_allocEthPkt(&gEnetApp,
                                       ENET_MEM_LARGE_POOL_PKT_SIZE,
                                       ENETDMA_CACHELINE_ALIGNMENT);
        EnetAppUtils_assert(pPktInfo != NULL);

        ENET_UTILS_SET_PKT_APP_STATE(&pPktInfo->pktState, ENET_PKTSTATE_APP_WITH_FREEQ);

        EnetQueue_enq(&rxFreeQ, &pPktInfo->node);
    }

    /* Retrieve any packets which are ready */
    EnetQueue_initQ(&rxReadyQ);
    status = EnetDma_retrieveRxPktQ(hRxCh, &rxReadyQ);
    EnetAppUtils_assert(status == ENET_SOK);

    /* There should not be any packet with DMA during init */
    EnetAppUtils_assert(EnetQueue_getQCount(&rxReadyQ) == 0U);

    EnetAppUtils_validatePacketState(&rxFreeQ,
                                     ENET_PKTSTATE_APP_WITH_FREEQ,
                                     ENET_PKTSTATE_APP_WITH_DRIVER);

    EnetDma_submitRxPktQ(hRxCh, &rxFreeQ);

    /* Assert here, as during init, the number of DMA descriptors should be equal to
     * the number of free Ethernet buffers available with app */
    EnetAppUtils_assert(EnetQueue_getQCount(&rxFreeQ) == 0U);
}

uint32_t EnetApp_retrieveFreeTxPkts(EnetApp_PerCtxt *perCtxt)
{
    EnetDma_PktQ txFreeQ;
    EnetDma_Pkt *pktInfo;
    uint32_t txFreeQCnt = 0U;
    int32_t status;

    EnetQueue_initQ(&txFreeQ);

    /* Retrieve any packets that may be free now */
    status = EnetDma_retrieveTxPktQ(perCtxt->hTxCh, &txFreeQ);
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

            EnetQueue_enq(&gEnetApp.txFreePktInfoQ, &pktInfo->node);
            pktInfo = (EnetDma_Pkt *)EnetQueue_deq(&txFreeQ);
        }
    }
    else
    {
        EnetAppUtils_print("retrieveFreeTxPkts() failed to retrieve pkts: %d\r\n", status);
    }

    return txFreeQCnt;
}

void EnetApp_createRxTask(EnetApp_PerCtxt *perCtxt)
{
    TaskP_Params taskParams;
    int32_t status;
    status = SemaphoreP_constructBinary(&perCtxt->rxSemObj, 0);
    DebugP_assert(SystemP_SUCCESS == status);

    status = SemaphoreP_constructCounting(&perCtxt->rxDoneSemObj, 0, COUNTING_SEM_COUNT);
    DebugP_assert(SystemP_SUCCESS == status);

    TaskP_Params_init(&taskParams);
    taskParams.priority       = 5U;
    taskParams.stack          = gEnetAppTaskStackRx;
    taskParams.stackSize      = sizeof(gEnetAppTaskStackRx);
    taskParams.args           = (void*)perCtxt;
    taskParams.name           = "Rx Task";
    taskParams.taskMain           = &EnetApp_rxTask;

    status = TaskP_construct(&perCtxt->rxTaskObj, &taskParams);
    DebugP_assert(SystemP_SUCCESS == status);

}

void EnetApp_destroyRxTask(EnetApp_PerCtxt *perCtxt)
{
    SemaphoreP_destruct(&perCtxt->rxSemObj);
    SemaphoreP_destruct(&perCtxt->rxDoneSemObj);
    TaskP_destruct(&perCtxt->rxTaskObj);
}

void EnetApp_rxTask(void *args)
{
    EnetApp_PerCtxt *perCtxt = (EnetApp_PerCtxt *)args;
    EnetDma_PktQ rxReadyQ;
    EnetDma_PktQ rxFreeQ;
    EnetDma_PktQ txSubmitQ;
    EnetDma_Pkt *rxPktInfo;
    EnetDma_Pkt *txPktInfo;
    EthFrame *rxFrame;
    EthFrame *txFrame;
    uint32_t reqTs;
    uint32_t totalRxCnt = 0U;
    int32_t status = ENET_SOK;

    while ((ENET_SOK == status) && (gEnetApp.run))
    {
        /* Wait for packet reception */
        SemaphoreP_pend(&perCtxt->rxSemObj, SystemP_WAIT_FOREVER);

        /* All peripherals have single hardware RX channel, so we only need to retrieve
         * packets from a single flow.*/
        EnetQueue_initQ(&rxReadyQ);
        EnetQueue_initQ(&rxFreeQ);
        EnetQueue_initQ(&txSubmitQ);

        /* Get the packets received so far */
        status = EnetDma_retrieveRxPktQ(perCtxt->hRxCh, &rxReadyQ);
        if (status != ENET_SOK)
        {
            /* Should we bail out here? */
            EnetAppUtils_print("Failed to retrieve RX pkt queue: %d\r\n", status);
            continue;
        }
#if DEBUG
        EnetAppUtils_print("%s: Received %u packets\r\n", perCtxt->name, EnetQueue_getQCount(&rxReadyQ));
#endif
        totalRxCnt += EnetQueue_getQCount(&rxReadyQ);
        reqTs = 0U;

        /* Consume the received packets and send them back */
        rxPktInfo = (EnetDma_Pkt *)EnetQueue_deq(&rxReadyQ);
        while (rxPktInfo != NULL)
        {
            rxFrame = (EthFrame *)rxPktInfo->bufPtr;
            EnetDma_checkPktState(&rxPktInfo->pktState,
                                    ENET_PKTSTATE_MODULE_APP,
                                    ENET_PKTSTATE_APP_WITH_DRIVER,
                                    ENET_PKTSTATE_APP_WITH_READYQ);

            /* Retrieve TX packets from driver and recycle them */
            EnetApp_retrieveFreeTxPkts(perCtxt);

            /* Dequeue one free TX Eth packet */
            txPktInfo = (EnetDma_Pkt *)EnetQueue_deq(&gEnetApp.txFreePktInfoQ);
            if (txPktInfo != NULL)
            {
                /* Fill the TX Eth frame with test content */
                txFrame = (EthFrame *)txPktInfo->bufPtr;
                memcpy(txFrame->hdr.dstMac, rxFrame->hdr.srcMac, ENET_MAC_ADDR_LEN);
                memcpy(txFrame->hdr.srcMac, &perCtxt->macAddr[0U], ENET_MAC_ADDR_LEN);
                txFrame->hdr.etherType = rxFrame->hdr.etherType;

                memcpy(&txFrame->payload[0U],
                        &rxFrame->payload[0U],
                        rxPktInfo->userBufLen - sizeof(EthFrameHeader));

                txPktInfo->userBufLen = rxPktInfo->userBufLen;
                txPktInfo->appPriv = &gEnetApp;
                txPktInfo->tsInfo.enableHostTxTs = false;

                EnetDma_checkPktState(&txPktInfo->pktState,
                                        ENET_PKTSTATE_MODULE_APP,
                                        ENET_PKTSTATE_APP_WITH_FREEQ,
                                        ENET_PKTSTATE_APP_WITH_DRIVER);

                /* Enqueue the packet for later transmission */
                EnetQueue_enq(&txSubmitQ, &txPktInfo->node);
            }
            else
            {
                EnetAppUtils_print("%s: Drop due to TX pkt not available\r\n", perCtxt->name);
            }

            EnetDma_checkPktState(&rxPktInfo->pktState,
                                    ENET_PKTSTATE_MODULE_APP,
                                    ENET_PKTSTATE_APP_WITH_READYQ,
                                    ENET_PKTSTATE_APP_WITH_FREEQ);

            /* Release the received packet */
            EnetQueue_enq(&rxFreeQ, &rxPktInfo->node);
            rxPktInfo = (EnetDma_Pkt *)EnetQueue_deq(&rxReadyQ);
        }

        /* Transmit all enqueued packets */
        status = EnetDma_submitTxPktQ(perCtxt->hTxCh, &txSubmitQ);
        if (status != ENET_SOK)
        {
            EnetAppUtils_print("%s: Failed to submit TX pkt queue: %d\r\n", perCtxt->name, status);
        }

        EnetAppUtils_validatePacketState(&rxFreeQ,
                                            ENET_PKTSTATE_APP_WITH_FREEQ,
                                            ENET_PKTSTATE_APP_WITH_DRIVER);

        /* Submit now processed buffers */
        EnetDma_submitRxPktQ(perCtxt->hRxCh, &rxFreeQ);
        if (status != ENET_SOK)
        {
            EnetAppUtils_print("%s: Failed to submit RX pkt queue: %d\r\n", perCtxt->name, status);
        }
    }

#if DEBUG
    EnetAppUtils_print("%s: Received %u packets\r\n", perCtxt->name, totalRxCnt);
#endif

    SemaphoreP_post(&perCtxt->rxDoneSemObj);
    TaskP_exit();
}