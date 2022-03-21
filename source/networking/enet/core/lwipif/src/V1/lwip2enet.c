/**
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

/**
 * Copyright (c) 2018 Texas Instruments Incorporated
 *
 * This file is dervied from the ``ethernetif.c'' skeleton Ethernet network
 * interface driver for lwIP.
 *
 */

/* Standard language headers */
#include <stdio.h>
#include <assert.h>
#include <string.h>

/* xdc header - should be first file included as due to order dependency */

/* OS/Posix headers */

/**
 * lwIP specific header files
 */
#include "lwip/opt.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip2enet.h"

/**
 * Enet device specific header files
 */
#include <networking/enet/enet.h>
#include <networking/enet/core/include/core/enet_utils.h>
#include <networking/enet/enet_cfg.h>

#include <networking/enet/core/lwipif/src/lwip2lwipif_priv.h>


/*---------------------------------------------------------------------------*\
 |                             Extern Declarations                             |
 \*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*\
 |                            Local Macros/Defines                             |
 \*---------------------------------------------------------------------------*/

#define OS_TASKPRIHIGH              8

#define LWIPIF_RX_PACKET_TASK_PRI      (OS_TASKPRIHIGH)
#define LWIPIF_RX_PACKET_TASK_STACK    (4096)

#define LWIPIF_TX_PACKET_TASK_PRI      (OS_TASKPRIHIGH)
#define LWIPIF_TX_PACKET_TASK_STACK    (4096)

#define LWIP2ENET_DIVIDER_US_TO_MS  (1000U)

/*---------------------------------------------------------------------------*\
 |                         Local Function Declarations                         |
 \*---------------------------------------------------------------------------*/

static void Lwip2Enet_notifyRxPackets(void *cbArg);

static void Lwip2Enet_notifyTxPackets(void *cbArg);

static void Lwip2Enet_rxPacketTask(void *arg0);

static void Lwip2Enet_prepFreePktInfoQ(Lwip2Enet_Handle hLwip2Enet);

static void Lwip2Enet_pbufQ2PktInfoQ(Lwip2Enet_Handle hLwip2Enet,
                                   pbufQ *pbufPktQ,
                                   EnetDma_PktQ *pDmaPktInfoQ);

static void Lwip2Enet_pktInfoQ2PbufQ(Lwip2Enet_Handle hLwip2Enet,
                                   EnetDma_PktQ *pDmaPktInfoQ,
                                   pbufQ *pbufPktQ);

static void Lwip2Enet_allocRxPackets(Lwip2Enet_Handle hLwip2Enet);

static uint32_t Lwip2Enet_prepRxPktQ(Lwip2Enet_Handle hLwip2Enet,
                                    EnetDma_PktQ *pPktQ);

static void Lwip2Enet_submitRxPktQ(Lwip2Enet_Handle hLwip2Enet);

static void Lwip2Enet_txPacketTask(void *arg0);

static uint32_t Lwip2Enet_prepTxPktQ(Lwip2Enet_Handle hLwip2Enet,
                                    EnetDma_PktQ *pPktQ);

static void Lwip2Enet_freePbufPackets(EnetDma_PktQ *tempQueue);

static void Lwip2Enet_updateRxNotifyStats(uint32_t packetCount,
                                         uint32_t timeDiff);

static void Lwip2Enet_updateTxNotifyStats(uint32_t packetCount,
                                         uint32_t timeDiff);

static void Lwip2Enet_print(const char *prnStr,
                           ...);

static int32_t Lwip2Enet_startRxTx(Lwip2Enet_Handle hLwip2Enet);

static void Lwip2Enet_stopRxTx(Lwip2Enet_Handle hLwip2Enet);

static void Lwip2Enet_submitTxPackets(Lwip2Enet_Handle hLwip2Enet,
                                     EnetDma_PktQ *pSubmitQ);

static void Lwip2Enet_submitRxPackets(Lwip2Enet_Handle hLwip2Enet,
                                     EnetDma_PktQ *pSubmitQ);

static void Lwip2Enet_retrieveTxPkts(Lwip2Enet_Handle hLwip2Enet);

static void Lwip2Enet_timerCb(ClockP_Object *hClk, void * arg);

static void Lwip2Enet_createTimer(Lwip2Enet_Handle hLwip2Enet);

static void Lwip2Enet_initGetHandleInArgs(Lwip2Enet_Handle hLwip2Enet,
                                         LwipifEnetAppIf_GetHandleInArgs *inArgs);

static void Lwip2Enet_initReleaseHandleInArgs(Lwip2Enet_Handle hLwip2Enet,
                                             LwipifEnetAppIf_ReleaseHandleInfo *inArgs);

/*---------------------------------------------------------------------------*\
 |                         Local Variable Declarations                         |
 \*---------------------------------------------------------------------------*/

static Lwip2Enet_Object gLwip2EnetObj;

/*---------------------------------------------------------------------------*\
 |                         Global Variable Declarations                        |
 \*---------------------------------------------------------------------------*/

Lwip2Enet_Stats gLwip2EnetStats;
static uint8_t gLwip2EnetRxPacketTaskStack[LWIPIF_RX_PACKET_TASK_STACK]
__attribute__ ((aligned(32)));
static uint8_t gLwip2EnetTxPacketTaskStack[LWIPIF_TX_PACKET_TASK_STACK]
__attribute__ ((aligned(32)));

/**
 * Initializes Ethernet peripheral hardware
 */
Lwip2Enet_Handle Lwip2Enet_open(struct netif *netif)
{
    int32_t status;
    LwipifEnetAppIf_GetHandleInArgs getHandleInArgs;
    Lwip2Enet_Handle hLwip2Enet;
    TaskP_Params params;
    uint32_t semInitCnt;

    hLwip2Enet = &gLwip2EnetObj;

    /* Clear instrumentation statistics structure */
    memset(&gLwip2EnetStats, 0, sizeof(Lwip2Enet_Stats));

    /* Initialize the allocated memory block. */
    memset(hLwip2Enet, 0, sizeof(Lwip2Enet_Object));

    /* lwIP interface relevant for this adaptation layer */
    hLwip2Enet->netif = netif;

    /* Store total number of free PBUF packets allocated. Used during close to
     * make sure all PBUF packets are returned to stack */

    /* Initialize the Rx Queue */
    pbufQ_init(&hLwip2Enet->rxFreePbufPktQ);

    /* Initialize the Tx Queues & Data */
    pbufQ_init(&hLwip2Enet->txReadyPbufPktQ);
    pbufQ_init(&hLwip2Enet->txUnUsedPbufPktQ);

    /* MCast List is EMPTY */
    hLwip2Enet->MCastCnt = 0;

    /* Init internal bookkeeping fields */
    hLwip2Enet->oldMCastCnt = 0;

    /* First init tasks, semaphores and clocks. This is required because
     * EnetDma event cb ISR can happen immediately after opening rx flow
     * in LwipifEnetAppCb_getHandle and all semaphores, clocks and tasks should
     * be valid at that point
     */
    /* Create semaphore objects, init shutDownFlag status */
    hLwip2Enet->shutDownFlag = false;

    semInitCnt              = 0U;
    status = SemaphoreP_constructBinary(&hLwip2Enet->shutDownSemObj, semInitCnt);
    Lwip2Enet_assert(status == SystemP_SUCCESS);

    semInitCnt              = 0U;
    status = SemaphoreP_constructBinary(&hLwip2Enet->rxPacketSemObj, semInitCnt);
    Lwip2Enet_assert(status == SystemP_SUCCESS);

    semInitCnt              = 0U;
    status = SemaphoreP_constructBinary(&hLwip2Enet->txPacketSemObj, semInitCnt);
    Lwip2Enet_assert(status == SystemP_SUCCESS);

    /* Start the packet processing tasks now that the channels are open */
    TaskP_Params_init(&params);
    params.name = "Lwip2Enet_RxPacketTask";
    params.priority       = LWIPIF_RX_PACKET_TASK_PRI;
    params.stack          = gLwip2EnetRxPacketTaskStack;
    params.stackSize      = sizeof(gLwip2EnetRxPacketTaskStack);
    params.args           = hLwip2Enet;
    params.taskMain       = &Lwip2Enet_rxPacketTask;

    status = TaskP_construct(&hLwip2Enet->rxPacketTaskObj , &params);
    Lwip2Enet_assert(status == SystemP_SUCCESS);

    TaskP_Params_init(&params);
    params.name = "Lwip2Enet_txPacketTask";
    params.priority       = LWIPIF_TX_PACKET_TASK_PRI;
    params.stack          = gLwip2EnetTxPacketTaskStack;
    params.stackSize      = sizeof(gLwip2EnetTxPacketTaskStack);
    params.args           = hLwip2Enet;
    params.taskMain       = &Lwip2Enet_txPacketTask;

    status = TaskP_construct(&hLwip2Enet->txPacketTaskObj , &params);
    Lwip2Enet_assert(status == SystemP_SUCCESS);

    /* Get Enet & DMA Drv Handle */
    Lwip2Enet_initGetHandleInArgs(hLwip2Enet, &getHandleInArgs);
    LwipifEnetAppCb_getHandle(&getHandleInArgs,
                            &hLwip2Enet->appInfo);
    Lwip2Enet_assert(hLwip2Enet->appInfo.hEnet != NULL);
    Lwip2Enet_assert(hLwip2Enet->appInfo.hUdmaDrv != NULL);
    Lwip2Enet_assert(hLwip2Enet->appInfo.isPortLinkedFxn != NULL);
    Lwip2Enet_assert(hLwip2Enet->appInfo.rxInfo.hRxFlow != NULL);
    Lwip2Enet_assert(hLwip2Enet->appInfo.txInfo.hTxChannel != NULL);

    if (NULL != hLwip2Enet->appInfo.print)
    {
        /* set the print function callback if not null */
        hLwip2Enet->print = hLwip2Enet->appInfo.print;
    }
    else
    {
        hLwip2Enet->print = (Enet_Print) &EnetUtils_printf;
    }

    /* Open DMA driver */
    status = Lwip2Enet_startRxTx(hLwip2Enet);
    if (status != ENET_SOK)
    {
        Lwip2Enet_print("Failed to open DMA: %d\n", status);
    }

    /* Get initial link/interface status from the driver */
    hLwip2Enet->linkIsUp = hLwip2Enet->appInfo.isPortLinkedFxn(hLwip2Enet->appInfo.hEnet);

    if (hLwip2Enet->appInfo.disableTxEvent)
    {
        EnetDma_disableTxEvent(hLwip2Enet->appInfo.txInfo.hTxChannel);
    }

    if (false == hLwip2Enet->appInfo.isRingMonUsed)
    {
        EnetDma_disableRxEvent(hLwip2Enet->appInfo.rxInfo.hRxFlow);
    }

    /* assert if clk period is not valid  */
    Lwip2Enet_assert(0U != hLwip2Enet->appInfo.timerPeriodUs);
    Lwip2Enet_createTimer(hLwip2Enet);
    ClockP_start(&hLwip2Enet->pacingClkObj);

    hLwip2Enet->initDone = TRUE;

    return hLwip2Enet;
}

/*!
 *  @b Lwip2Enet_close
 *  @n
 *      Closes Ethernet peripheral and disables interrupts.
 *
 *  \param[in]  hLwip2Enet
 *      Lwip2Enet_object structure pointer.
 *
 *  \retval
 *      void
 */
void Lwip2Enet_close(Lwip2Enet_Handle hLwip2Enet)
{
    uint32_t pktInfoQCnt;
    LwipifEnetAppIf_ReleaseHandleInfo releaseHandleInfo;

    Lwip2Enet_assert(NULL != hLwip2Enet);

    /* Set the translation layer shutdown flag */
    hLwip2Enet->shutDownFlag = true;

    /* Stop and delete the tick timer */
    ClockP_stop (&hLwip2Enet->pacingClkObj);
    ClockP_destruct (&hLwip2Enet->pacingClkObj);

    Lwip2Enet_stopRxTx(hLwip2Enet);

    Lwip2Enet_initReleaseHandleInArgs(hLwip2Enet, &releaseHandleInfo);
    LwipifEnetAppCb_releaseHandle(&releaseHandleInfo);

    /* free txReadyPbufPktQ */
    while (pbufQ_count(&hLwip2Enet->txReadyPbufPktQ) != 0U)
    {
        struct pbuf* hPbufPacket = pbufQ_deQ(&hLwip2Enet->txReadyPbufPktQ);
        Lwip2Enet_assert(NULL != hPbufPacket);
        pbuf_free(hPbufPacket);
    }

    pktInfoQCnt = EnetQueue_getQCount(&hLwip2Enet->txFreePktInfoQ) +
                  EnetQueue_getQCount(&hLwip2Enet->rxFreePktInfoQ);

    if (ENET_ARRAYSIZE(hLwip2Enet->pktInfoMem) != pktInfoQCnt)
    {
        Lwip2Enet_print("Lwip2Enet_stopRxTx: Failed to retrieve all PktInfo\n");
    }

    memset((void *)&hLwip2Enet->appInfo, 0, sizeof(hLwip2Enet->appInfo));

    /* Pend on shutDownSemObj (twice for two sub-tasks) */
    SemaphoreP_pend(&hLwip2Enet->shutDownSemObj, SystemP_WAIT_FOREVER);
    SemaphoreP_pend(&hLwip2Enet->shutDownSemObj, SystemP_WAIT_FOREVER);

    /* Delete the semaphore objects */
    SemaphoreP_destruct(&hLwip2Enet->rxPacketSemObj);
    SemaphoreP_destruct(&hLwip2Enet->txPacketSemObj);
    SemaphoreP_destruct(&hLwip2Enet->shutDownSemObj);

    /* TODO: Terminate and delete tasks */

    /* Clear the allocated translation */
    memset(hLwip2Enet, 0, sizeof(Lwip2Enet_Object));
}

/*!
 *  @b Lwip2Enet_setRx
 *  @n
 *      Sets the filter for Ethernet peripheral. Sets up the multicast addresses in
 *      the ALE.
 *
 *  \param[in]  hLwip2Enet
 *      Lwip2Enet_object structure pointer.
 *
 *  \retval
 *      void
 */
void Lwip2Enet_setRx(Lwip2Enet_Handle hLwip2Enet)
{
}

/*!
 *  @b Lwip2Enet_sendTxPackets
 *  @n
 *      Routine to send out queued Tx packets to the hardware driver
 *
 *  \param[in]  hLwip2Enet
 *      Lwip2Enet_object structure pointer.
 *
 *  \retval
 *      void
 */
void Lwip2Enet_sendTxPackets(Lwip2Enet_Handle hLwip2Enet)
{
    EnetDma_Pkt *pCurrDmaPacket;
    struct pbuf *hPbufPkt;
    // bool flag = 0;

    /* If link is not up, simply return */
    if (hLwip2Enet->linkIsUp)
    {
        EnetDma_PktQ txSubmitQ;

        EnetQueue_initQ(&txSubmitQ);

        if (pbufQ_count(&hLwip2Enet->txUnUsedPbufPktQ))
        {
            /* send any pending TX Q's */
            Lwip2Enet_pbufQ2PktInfoQ(hLwip2Enet, &hLwip2Enet->txUnUsedPbufPktQ, &txSubmitQ);
        }

        /* Check if there is anything to transmit, else simply return */
        while (pbufQ_count(&hLwip2Enet->txReadyPbufPktQ) != 0)
        {
            /* Dequeue one free TX Eth packet */
            pCurrDmaPacket = (EnetDma_Pkt *)EnetQueue_deq(&hLwip2Enet->txFreePktInfoQ);

            if (pCurrDmaPacket == NULL)
            {
                /* If we run out of packet info Q, retrieve packets from HW
                * and try to dequeue free packet again */
                Lwip2Enet_retrieveTxPkts(hLwip2Enet);
                pCurrDmaPacket = (EnetDma_Pkt *)EnetQueue_deq(&hLwip2Enet->txFreePktInfoQ);
            }

            if (NULL != pCurrDmaPacket)
            {
                hPbufPkt = pbufQ_deQ(&hLwip2Enet->txReadyPbufPktQ);
                EnetDma_initPktInfo(pCurrDmaPacket);

                pCurrDmaPacket->bufPtr = (uint8_t *) hPbufPkt->payload;
                pCurrDmaPacket->appPriv    = hPbufPkt;
                pCurrDmaPacket->orgBufLen  = PBUF_POOL_BUFSIZE;
                pCurrDmaPacket->userBufLen = hPbufPkt->len;
                pCurrDmaPacket->node.next = NULL;


#ifdef LWIPIF_CHECKSUM_SUPPORT
                /* TODO: Add checksum support */
#endif
                ENET_UTILS_COMPILETIME_ASSERT(offsetof(EnetDma_Pkt, node) == 0);
                EnetQueue_enq(&txSubmitQ, &(pCurrDmaPacket->node));

                Lwip2EnetStats_addOne(&gLwip2EnetStats.txFreeAppPktDq);
                Lwip2EnetStats_addOne(&gLwip2EnetStats.txReadyPbufPktDq);
                //hPbufPkt = hPbufPkt->next;
            }
            else
            {
                break;
            }
        }

        /* Give the accumulated packets to the hardware */
        /* Submit the packet for transmission */
        Lwip2Enet_submitTxPackets(hLwip2Enet, &txSubmitQ);
    }
}


/*!
 *  @b Lwip2Enet_ioctl
 *  @n
 *  Low level driver Ioctl interface. This interface can be used for
 *  ALE configuration, control, statistics
 *
 *  \param[in]  hLwip2Enet
 *      Lwip2Enet_object structure pointer.
 *  \param[in]  cmd
 *      Ioctl command.
 *  \param[in]  pBuf
 *      Ioctl buffer with commands and params to set/get
 *      configuration from hardware.
 *  \param[in]  size
 *      Size of Ioctl buffer.
 *
 *  \retval
 *      void
 */
int32_t Lwip2Enet_ioctl(Lwip2Enet_Handle hLwip2Enet,
                       uint32_t cmd,
                       void *param,
                       uint32_t size)
{
    int32_t retVal = 0U;

    // TODO - make IOCTL design consistent with Enet/Enet modules

    /* Decode the command and act on it */
    switch (cmd)
    {
        /* Rx Task Load IOCTL */
        case LWIP2ENET_IOCTL_GET_RXTASK_LOAD:
        {
#if 0
            Load_Stat stat;
            if (Load_getTaskLoad(hLwip2Enet->rxPacketTaskObj, &stat))
            {
                retVal = Load_calculateLoad(&stat);
            }
#endif
            break;
        }

        /* Adding Tx Task Load  */
        case LWIP2ENET_IOCTL_GET_TXTASK_LOAD:
        {
#if 0
            Load_Stat stat;
            if (Load_getTaskLoad(hLwip2Enet->txPacketTaskObj, &stat))
            {
                retVal = Load_calculateLoad(&stat);
            }
#endif
            break;
        }

        default:
        {
            Lwip2Enet_assert(FALSE);
        }
    }

    return retVal;
}

/*---------------------------------------------------------------------------*\
 |                           Local Function Definitions                        |
 \*---------------------------------------------------------------------------*/

 void Lwip2Enet_periodicFxn(Lwip2Enet_Handle hLwip2Enet)
{
    uint32_t prevLinkState     = hLwip2Enet->linkIsUp;
    uint32_t prevLinkInterface = hLwip2Enet->currLinkedIf;

#if (1U == ENET_CFG_DEV_ERROR)

    EnetQueue_verifyQCount(&hLwip2Enet->txFreePktInfoQ);
    EnetQueue_verifyQCount(&hLwip2Enet->rxFreePktInfoQ);

    {
        int32_t status;

        status = EnetUdma_checkRxFlowSanity(hLwip2Enet->appInfo.rxInfo.hRxFlow, 5U);
        if (status != ENET_SOK)
        {
            Lwip2Enet_print("EnetUdma_checkRxFlowSanity Failed\n");
        }

        status = EnetUdma_checkTxChSanity(hLwip2Enet->appInfo.txInfo.hTxChannel, 5U);
        if (status != ENET_SOK)
        {
            Lwip2Enet_print("EnetUdma_checkTxChSanity Failed\n");
        }
    }
#endif

    /*
     * Return the same DMA packets back to the DMA channel (but now
     * associated with a new PBUF Packet and buffer)
     */
    if (pbufQ_count(&hLwip2Enet->rxFreePbufPktQ) != 0U)
    {
        Lwip2Enet_submitRxPktQ(hLwip2Enet);
    }

#if defined(LWIPIF_INSTRUMENTATION_ENABLED)
    static uint32_t loadCount = 0;
    Load_Stat stat;
    gLwip2EnetStats.gCpuLoad[loadCount] = Load_getCPULoad();

    if (Load_getGlobalHwiLoad(&stat))
    {
        gLwip2EnetStats.gHwiLoad[loadCount] = Load_calculateLoad(&stat);
    }

    if (Load_getTaskLoad(hLwip2Enet->rxPacketTaskObj, &stat))
    {
        gLwip2EnetStats.rxStats.taskLoad[loadCount] = Load_calculateLoad(&stat);
    }

    if (Load_getTaskLoad(hLwip2Enet->txPacketTaskObj, &stat))
    {
        gLwip2EnetStats.txStats.taskLoad[loadCount] = Load_calculateLoad(&stat);
    }

    loadCount = (loadCount + 1U) & (HISTORY_CNT - 1U);
#endif

    /* Get current link status as reported by the hardware driver */
    hLwip2Enet->linkIsUp = hLwip2Enet->appInfo.isPortLinkedFxn(hLwip2Enet->appInfo.hEnet);

    /* If the interface changed, discard any queue packets (since the MAC would now be wrong) */
    if (prevLinkInterface != hLwip2Enet->currLinkedIf)
    {
        /* ToDo: Discard all queued packets */
    }

    /* If link status changed from down->up, then send any queued packets */
    if ((prevLinkState == 0) && (hLwip2Enet->linkIsUp))
    {
        Lwip2Enet_sendTxPackets(hLwip2Enet);
    }
}

static void Lwip2Enet_processRxUnusedQ(Lwip2Enet_Handle hLwip2Enet,
                                      EnetDma_PktQ *unUsedQ)
{
    EnetDma_Pkt *pDmaPacket;

    pDmaPacket = (EnetDma_Pkt *)EnetQueue_deq(unUsedQ);
    while (pDmaPacket != NULL)
    {
        /* Get the full PBUF packet that needs to be returned to the rxFreePbufPktQ */
        struct pbuf* hPbufPacket = (struct pbuf *)pDmaPacket->appPriv;
        if (hPbufPacket)
        {
            /* Enqueue the received packet to rxFreePbufPktQ*/
            pbufQ_enQ(&hLwip2Enet->rxFreePbufPktQ, hPbufPacket);

            /* Put packet info into free Q as we have removed the Pbuf buffers
             * from the it */
            EnetQueue_enq(&hLwip2Enet->rxFreePktInfoQ, &pDmaPacket->node);
            Lwip2EnetStats_addOne(&gLwip2EnetStats.rxFreeAppPktEnq);

            pDmaPacket = (EnetDma_Pkt *)EnetQueue_deq(unUsedQ);
        }
        else
        {
            /* should never happen as this is received from HW */
            Lwip2Enet_assert(FALSE);
        }
    }
}

static void Lwip2Enet_submitRxPackets(Lwip2Enet_Handle hLwip2Enet,
                                     EnetDma_PktQ *pSubmitQ)
{
    int32_t retVal;

    retVal = EnetDma_submitRxPktQ(hLwip2Enet->appInfo.rxInfo.hRxFlow, pSubmitQ);
    if (ENET_SOK != retVal)
    {
        Lwip2Enet_print("EnetDma_submitRxPktQ: failed to submit pkts: %d\n", retVal);
    }

    if (EnetQueue_getQCount(pSubmitQ))
    {
        /* Copy unused packets to back to readyQ */
        Lwip2Enet_processRxUnusedQ(hLwip2Enet, pSubmitQ);
    }
}

/* May lead to infinite loop if no free memory
 * available*/
static void Lwip2Enet_pbufQ2PktInfoQ(Lwip2Enet_Handle hLwip2Enet,
                                   pbufQ *pbufPktQ,
                                   EnetDma_PktQ *pDmaPktInfoQ)
{
    EnetDma_Pkt *pCurrDmaPacket;
    struct pbuf *hPbufPkt = NULL;

    while(pbufQ_count(pbufPktQ) != 0U)
    {

        /* Dequeue one free TX Eth packet */
        pCurrDmaPacket = (EnetDma_Pkt *)EnetQueue_deq(&hLwip2Enet->txFreePktInfoQ);

        if (pCurrDmaPacket == NULL)
        {
        /* If we run out of packet info Q, retrieve packets from HW
            * and try to dequeue free packet again */
        Lwip2Enet_retrieveTxPkts(hLwip2Enet);
        pCurrDmaPacket = (EnetDma_Pkt *)EnetQueue_deq(&hLwip2Enet->txFreePktInfoQ);
        }

        if (NULL != pCurrDmaPacket)
        {
            hPbufPkt = pbufQ_deQ(pbufPktQ);
            EnetDma_initPktInfo(pCurrDmaPacket);

            pCurrDmaPacket->bufPtr = (uint8_t *) hPbufPkt->payload;
            pCurrDmaPacket->appPriv    = hPbufPkt;
            pCurrDmaPacket->orgBufLen  = PBUF_POOL_BUFSIZE;
            pCurrDmaPacket->userBufLen = hPbufPkt->len;
            pCurrDmaPacket->node.next = NULL;

            ENET_UTILS_COMPILETIME_ASSERT(offsetof(EnetDma_Pkt, node) == 0);
            EnetQueue_enq(pDmaPktInfoQ, &(pCurrDmaPacket->node));

            Lwip2EnetStats_addOne(&gLwip2EnetStats.txFreeAppPktDq);
        }
        else
        {
            break;
        }
    }
}

static void Lwip2Enet_pktInfoQ2PbufQ(Lwip2Enet_Handle hLwip2Enet,
                                   EnetDma_PktQ *pDmaPktInfoQ,
                                   pbufQ *pbufPktQ)
{
    EnetDma_Pkt *pDmaPacket;
    struct pbuf *hPbufPacket;

    while (EnetQueue_getQCount(pDmaPktInfoQ) != 0U)
    {
        pDmaPacket = (EnetDma_Pkt *)EnetQueue_deq(pDmaPktInfoQ);
        hPbufPacket = (struct pbuf *)(pDmaPacket->appPriv);

        Lwip2Enet_assert(hPbufPacket != NULL);
        /*Don't want to make a copy, since it would cause waste memory*/
        pbufQ_enQ(pbufPktQ, hPbufPacket);
    }
}

static void Lwip2Enet_submitTxPackets(Lwip2Enet_Handle hLwip2Enet,
                                     EnetDma_PktQ *pSubmitQ)
{
    int32_t retVal;

    retVal = EnetDma_submitTxPktQ(hLwip2Enet->appInfo.txInfo.hTxChannel, pSubmitQ);
    if (ENET_SOK != retVal)
    {
        Lwip2Enet_print("EnetDma_submitTxPktQ: failed to submit pkts: %d\n", retVal);
    }

    if (EnetQueue_getQCount(pSubmitQ))
    {
        /* TODO: txUnUsedPBMPktQ is needed for packets that were not able to be
         *       submitted to driver.  It can be removed if stack supported any
         *       mechanism to enqueue them to the head of the queue. */
        Lwip2Enet_pktInfoQ2PbufQ(hLwip2Enet, pSubmitQ, &hLwip2Enet->txUnUsedPbufPktQ);
        EnetQueue_append(&hLwip2Enet->txFreePktInfoQ, pSubmitQ);
        Lwip2EnetStats_addNum(&gLwip2EnetStats.txFreeAppPktEnq, EnetQueue_getQCount(pSubmitQ));
    }
}

static void Lwip2Enet_freePbufPackets(EnetDma_PktQ *tempQueue)
{
    EnetDma_Pkt *pCurrDmaPacket;
    struct pbuf* hPbufPacket;

    while (EnetQueue_getQCount(tempQueue))
    {
        pCurrDmaPacket = (EnetDma_Pkt *)EnetQueue_deq(tempQueue);
        hPbufPacket     = (struct pbuf *)pCurrDmaPacket->appPriv;
        pbuf_free(hPbufPacket);
    }
}

static void Lwip2Enet_notifyRxPackets(void *cbArg)
{
    /* Post semaphore to rx handling task */
    Lwip2Enet_Handle hLwip2Enet = (Lwip2Enet_Handle)cbArg;

    /* do not post events if init not done or shutdown in progress */
    if ((hLwip2Enet->initDone) && (hLwip2Enet->shutDownFlag == false))
    {
        EnetDma_disableRxEvent(hLwip2Enet->appInfo.rxInfo.hRxFlow);
        SemaphoreP_post(&hLwip2Enet->rxPacketSemObj);
    }
}

static void Lwip2Enet_notifyTxPackets(void *cbArg)
{
    /* Post semaphore to tx handling task */
    Lwip2Enet_Handle hLwip2Enet = (Lwip2Enet_Handle)cbArg;

    /* do not post events if init not done or shutdown in progress */
    if ((hLwip2Enet->initDone) && (hLwip2Enet->shutDownFlag == false))
    {
        SemaphoreP_post(&hLwip2Enet->txPacketSemObj);
    }
}

static void Lwip2Enet_rxPacketTask(void *arg0)
{
    Lwip2Enet_Handle hLwip2Enet = (Lwip2Enet_Handle)arg0;
    EnetDma_PktQ tempQueue;
    int32_t retVal;
    uint32_t pktCnt;

    while (!hLwip2Enet->shutDownFlag)
    {
        /* Wait for the Rx ISR to notify us that packets are available with data */
        SemaphoreP_pend(&hLwip2Enet->rxPacketSemObj, SystemP_WAIT_FOREVER);

        if (hLwip2Enet->shutDownFlag)
        {
            /* This translation layer is shutting down, don't give anything else to the stack */
            break;
        }

        Lwip2EnetStats_addOne(&gLwip2EnetStats.rxStats.rawNotifyCnt);
        pktCnt = 0;

        /* Retrieve the used (filled) packets from the channel */
        {
            EnetQueue_initQ(&tempQueue);
            retVal = EnetDma_retrieveRxPktQ(hLwip2Enet->appInfo.rxInfo.hRxFlow,
                                               &tempQueue);
            if (ENET_SOK != retVal)
            {
                Lwip2Enet_print("Lwip2Enet_rxPacketTask: failed to retrieve RX pkts: %d\n", retVal);
            }
        }
        if (tempQueue.count == 0)
        {
            Lwip2EnetStats_addOne(&gLwip2EnetStats.rxStats.zeroNotifyCnt);
        }

        /*
         * Call Lwip2Enet_prepRxPktQ() even if no packets were received.
         * This allows new packets to be submitted if PBUF buffers became
         * newly available and there were outstanding free packets.
         */
        {
            /*
             * Get all used Rx DMA packets from the hardware, then send the buffers
             * of those packets on to the LwIP stack to be parsed/processed.
             */
            pktCnt = Lwip2Enet_prepRxPktQ(hLwip2Enet, &tempQueue);
        }

        /*
         * We don't want to time the semaphore post used to notify the LwIP stack as that may cause a
         * task transition. We don't want to time the semaphore pend, since that would time us doing
         * nothing but waiting.
         */
        if (pktCnt != 0)
        {
            Lwip2Enet_updateRxNotifyStats(pktCnt, 0U);
        }

        /* As we are operating in one shot mode, the Timer_start should be
         * called for the next timeout period
         */
        ClockP_start(&hLwip2Enet->pacingClkObj);

        if (hLwip2Enet->appInfo.isRingMonUsed)
        {
            EnetDma_enableRxEvent(hLwip2Enet->appInfo.rxInfo.hRxFlow);
        }
    }

    /* We are shutting down, notify that we are done */
    SemaphoreP_post(&hLwip2Enet->shutDownSemObj);
}

static void Lwip2Enet_txPacketTask(void *arg0)
{
    Lwip2Enet_Handle hLwip2Enet = (Lwip2Enet_Handle)arg0;

    while (!hLwip2Enet->shutDownFlag)
    {
        /*
         * Wait for the Tx ISR to notify us that empty packets are available
         * that were used to send data
         */
        SemaphoreP_pend(&hLwip2Enet->txPacketSemObj, SystemP_WAIT_FOREVER);
        Lwip2Enet_retrieveTxPkts(hLwip2Enet);
    }

    /* We are shutting down, notify that we are done */
    SemaphoreP_post(&hLwip2Enet->shutDownSemObj);
}

static void Lwip2Enet_allocRxPackets(Lwip2Enet_Handle hLwip2Enet)
{
    struct pbuf* hPbufPacket;
    uint32_t bufSize;
    uint32_t i;

    Lwip2Enet_assert(hLwip2Enet->appInfo.hostPortRxMtu != 0);

    /*
     * Pre-allocate twice as many lwIP stack packets as we plan to give to/get from the hardware.
     * The idea here is that even if we fill up all the DMA descriptors submit queue,
     * we will have another complete set to swap in right away.
     */
    // Make sure this is defined as twice what is required from the hardware
    /* Creates just as many Rx pbufs as the driver free packets*/
    for (i = 0U; i < ((uint32_t)LWIP2ENET_RX_PACKETS); i++)
    {
        bufSize = ENET_UTILS_ALIGN(PBUF_POOL_BUFSIZE, ENETDMA_CACHELINE_ALIGNMENT);

        hPbufPacket = pbuf_alloc(PBUF_RAW, bufSize, PBUF_POOL);
        if (hPbufPacket != NULL)
        {
            Lwip2Enet_assert(hPbufPacket->payload != NULL);

            /*
             * Adjusts the pbuf payload size to accomodate the space lost
             * due to cacheline adjustment
             */
            hPbufPacket->len -= (!ENET_UTILS_IS_ALIGNED(hPbufPacket->payload, ENETDMA_CACHELINE_ALIGNMENT)) * ENETDMA_CACHELINE_ALIGNMENT;
            hPbufPacket->tot_len = hPbufPacket->len;

            /*
             * Ensures that the ether packet always starts on a fresh cacheline
             */
            hPbufPacket->payload = (void *) ENET_UTILS_ALIGN((uint32_t)hPbufPacket->payload, ENETDMA_CACHELINE_ALIGNMENT);

            Lwip2Enet_assert(ENET_UTILS_IS_ALIGNED(hPbufPacket->payload, ENETDMA_CACHELINE_ALIGNMENT));

            /* Enqueue to the free queue */
            pbufQ_enQ(&hLwip2Enet->rxFreePbufPktQ, hPbufPacket);

            Lwip2EnetStats_addOne(&gLwip2EnetStats.rxFreePbufPktEnq);
        }
        else
        {
            Lwip2Enet_print("ERROR: Pbuf_alloc() failure...exiting!\n");
            Lwip2Enet_assert(FALSE);
        }
    }
}

static void Lwip2Enet_prepFreePktInfoQ(Lwip2Enet_Handle hLwip2Enet)
{
    uint32_t i;
    EnetDma_Pkt *pPktInfo;

    /* Initialize the DMA Free Queue*/
    EnetQueue_initQ(&hLwip2Enet->txFreePktInfoQ);
    EnetQueue_initQ(&hLwip2Enet->rxFreePktInfoQ);

    /*
     * Allocate the free pkt info Q. This is used to exchange buffer info from
     * DMA basically for submitting free packets and retrieving ready packets.
     */
    for (i = 0U; i < (uint32_t)(LWIP2ENET_TX_PACKETS); i++)
    {
        /* Initialize Pkt info Q from allocated memory */
        pPktInfo = &hLwip2Enet->pktInfoMem[i];
        Lwip2Enet_assert(pPktInfo != NULL);
        EnetDma_initPktInfo(pPktInfo);
        EnetQueue_enq(&hLwip2Enet->txFreePktInfoQ, &pPktInfo->node);
        Lwip2EnetStats_addOne(&gLwip2EnetStats.txFreeAppPktEnq);
    }

    for (i = LWIP2ENET_TX_PACKETS; i < (uint32_t)(LWIP2ENET_TX_PACKETS + LWIP2ENET_RX_PACKETS); i++)
    {
        /* Initialize Pkt info Q from allocated memory */
        pPktInfo = &hLwip2Enet->pktInfoMem[i];
        Lwip2Enet_assert(pPktInfo != NULL);
        EnetDma_initPktInfo(pPktInfo);
        EnetQueue_enq(&hLwip2Enet->rxFreePktInfoQ, &pPktInfo->node);
        Lwip2EnetStats_addOne(&gLwip2EnetStats.rxFreeAppPktEnq);
    }
}

/*
 * Enqueue a new packet and make sure that buffer descriptors are properly linked.
 * NOTE: Not thread safe
 */
static void Lwip2Enet_submitRxPktQ(Lwip2Enet_Handle hLwip2Enet)
{
    EnetDma_PktQ resubmitPktQ;
    struct pbuf* hPbufPacket;
    EnetDma_Pkt *pCurrDmaPacket;

    EnetQueue_initQ(&resubmitPktQ);

    /*
     * Fill in as many packets as we can with new PBUF buffers so they can be
     * returned to the stack to be filled in.
     */
    pCurrDmaPacket = (EnetDma_Pkt *)EnetQueue_deq(&hLwip2Enet->rxFreePktInfoQ);

    while (NULL != pCurrDmaPacket)
    {
        hPbufPacket = pbufQ_deQ(&hLwip2Enet->rxFreePbufPktQ);
        if (hPbufPacket)
        {
            Lwip2EnetStats_addOne(&gLwip2EnetStats.rxFreePbufPktDq);
            Lwip2EnetStats_addOne(&gLwip2EnetStats.rxFreeAppPktDq);

            EnetDma_initPktInfo(pCurrDmaPacket);
            pCurrDmaPacket->bufPtr = (uint8_t *) hPbufPacket->payload;
            pCurrDmaPacket->orgBufLen = hPbufPacket->len;
            pCurrDmaPacket->userBufLen = hPbufPacket->len;

            /* Save off the PBM packet handle so it can be handled by this layer later */
            pCurrDmaPacket->appPriv = (void *)hPbufPacket;
            EnetQueue_enq(&resubmitPktQ, &pCurrDmaPacket->node);

            pCurrDmaPacket = (EnetDma_Pkt *)EnetQueue_deq(&hLwip2Enet->rxFreePktInfoQ);
        }
        else
        {
            EnetQueue_enq(&hLwip2Enet->rxFreePktInfoQ, &pCurrDmaPacket->node);
            break;
        }
    }

    /*
     * Return the same DMA packets back to the DMA channel (but now
     * associated with a new PBM Packet and buffer)
     */
    if (EnetQueue_getQCount(&resubmitPktQ))
    {
        Lwip2Enet_submitRxPackets(hLwip2Enet, &resubmitPktQ);
    }
}

static uint32_t Lwip2Enet_prepRxPktQ(Lwip2Enet_Handle hLwip2Enet,
                                    EnetDma_PktQ *pPktQ)
{
    uint32_t packetCount = 0;
    EnetDma_Pkt *pCurrDmaPacket;
    uint32_t csumInfo;
    bool chkSumErr = false;

    pCurrDmaPacket = (EnetDma_Pkt *)EnetQueue_deq(pPktQ);
    while (pCurrDmaPacket)
    {
        /* Get the full PBUF packet that needs to be returned to the LwIP stack */
        struct pbuf* hPbufPacket = (struct pbuf *)pCurrDmaPacket->appPriv;
        if (hPbufPacket)
        {
            uint32_t validLen = pCurrDmaPacket->userBufLen;

            /* Fill in PBUF packet length field */
            hPbufPacket->len = validLen;
            hPbufPacket->tot_len = validLen;
            Lwip2Enet_assert(hPbufPacket->payload != NULL);
            /* We don't check if HW checksum offload is enabled while checking for checksum error
             * as default value of this field when offload not enabled is false */
            csumInfo = pCurrDmaPacket->chkSumInfo;
#ifdef LWIPIF_CHECKSUM_SUPPORT
            if ( ENETUDMA_CPPIPSI_GET_IPV4_FLAG(csumInfo)||
                 ENETUDMA_CPPIPSI_GET_IPV6_FLAG(csumInfo) )
            {
                chkSumErr = ENETUDMA_CPPIPSI_GET_CHKSUM_ERR_FLAG(csumInfo);
            }
#else
            chkSumErr = false;
#endif
            if (!chkSumErr)
            {
                /* Pass the received packet to the LwIP stack */
                LWIPIF_LWIP_input(hLwip2Enet->netif, hPbufPacket);
                packetCount++;
            }
            else
            {
                /* Put PBUF buffer in free Q as we are not passing to stack */
                pbufQ_enQ(&hLwip2Enet->rxFreePbufPktQ, hPbufPacket);
                Lwip2EnetStats_addOne(&gLwip2EnetStats.rxChkSumErr);
            }

            /* Put packet info into free Q as we have removed the PBUF buffers
             * from the it */
            EnetQueue_enq(&hLwip2Enet->rxFreePktInfoQ, &pCurrDmaPacket->node);
            Lwip2EnetStats_addOne(&gLwip2EnetStats.rxFreeAppPktEnq);

            pCurrDmaPacket = (EnetDma_Pkt *)EnetQueue_deq(pPktQ);
        }
        else
        {
            /* Should never happen as this is received from HW */
            Lwip2Enet_assert(FALSE);
        }
    }

    /* return as many packets to driver as we can */
    Lwip2Enet_submitRxPktQ(hLwip2Enet);

    return packetCount;
}

static uint32_t Lwip2Enet_prepTxPktQ(Lwip2Enet_Handle hLwip2Enet,
                                    EnetDma_PktQ *pPktQ)
{
    uint32_t packetCount;
    EnetDma_Pkt *pCurrDmaPacket;
    struct pbuf* hPbufPacket;

    packetCount = EnetQueue_getQCount(pPktQ);

    pCurrDmaPacket = (EnetDma_Pkt *)EnetQueue_deq(pPktQ);
    while (pCurrDmaPacket)
    {
        hPbufPacket = (struct pbuf *)pCurrDmaPacket->appPriv;

        Lwip2Enet_assert(hPbufPacket != NULL);
        /* Free PBUF buffer as it is transmitted by DMA now */
        pbuf_free(hPbufPacket);
        /* Return packet info to free pool */
        EnetQueue_enq(&hLwip2Enet->txFreePktInfoQ, &pCurrDmaPacket->node);
        pCurrDmaPacket = (EnetDma_Pkt *)EnetQueue_deq(pPktQ);
    }

    Lwip2EnetStats_addNum(&gLwip2EnetStats.txFreeAppPktEnq, packetCount);

    return packetCount;
}

static void Lwip2Enet_updateTxNotifyStats(uint32_t packetCount,
                                         uint32_t timeDiff)
{
#if defined(LWIPIF_INSTRUMENTATION_ENABLED)
    uint32_t notificationCount;
    uint32_t timePerPacket = timeDiff / packetCount;

    notificationCount = gLwip2EnetStats.txStats.dataNotifyCnt & (HISTORY_CNT - 1U);
    gLwip2EnetStats.txStats.dataNotifyCnt++;

    gLwip2EnetStats.txStats.totalPktCnt   += packetCount;
    gLwip2EnetStats.txStats.totalCycleCnt += timeDiff;

    gLwip2EnetStats.txStats.cycleCntPerNotify[notificationCount] = timeDiff;
    if (timeDiff > gLwip2EnetStats.txStats.cycleCntPerNotifyMax)
    {
        gLwip2EnetStats.txStats.cycleCntPerNotifyMax = timeDiff;
    }

    gLwip2EnetStats.txStats.pktsPerNotify[notificationCount] = packetCount;
    if (packetCount > gLwip2EnetStats.txStats.pktsPerNotifyMax)
    {
        gLwip2EnetStats.txStats.pktsPerNotifyMax = packetCount;
    }

    gLwip2EnetStats.txStats.cycleCntPerPkt[notificationCount] = timePerPacket;
    if (timePerPacket > gLwip2EnetStats.txStats.cycleCntPerPktMax)
    {
        gLwip2EnetStats.txStats.cycleCntPerPktMax = timePerPacket;
    }
#endif
}

static void Lwip2Enet_updateRxNotifyStats(uint32_t packetCount,
                                         uint32_t timeDiff)
{
#if defined(LWIPIF_INSTRUMENTATION_ENABLED)
    uint32_t notificationCount;
    uint32_t timePerPacket = timeDiff / packetCount;

    notificationCount = gLwip2EnetStats.rxStats.dataNotifyCnt & (HISTORY_CNT - 1U);
    gLwip2EnetStats.rxStats.dataNotifyCnt++;

    gLwip2EnetStats.rxStats.totalPktCnt   += packetCount;
    gLwip2EnetStats.rxStats.totalCycleCnt += timeDiff;

    gLwip2EnetStats.rxStats.cycleCntPerNotify[notificationCount] = timeDiff;
    if (timeDiff > gLwip2EnetStats.rxStats.cycleCntPerNotifyMax)
    {
        gLwip2EnetStats.rxStats.cycleCntPerNotifyMax = timeDiff;
    }

    gLwip2EnetStats.rxStats.pktsPerNotify[notificationCount] = packetCount;
    if (packetCount > gLwip2EnetStats.rxStats.pktsPerNotifyMax)
    {
        gLwip2EnetStats.rxStats.pktsPerNotifyMax = packetCount;
    }

    gLwip2EnetStats.rxStats.cycleCntPerPkt[notificationCount] = timePerPacket;
    if (timePerPacket > gLwip2EnetStats.rxStats.cycleCntPerPktMax)
    {
        gLwip2EnetStats.rxStats.cycleCntPerPktMax = timePerPacket;
    }
#endif
}

static void Lwip2Enet_print(const char *prnStr,
                           ...)
{
    // TODO fix me
    if (NULL != gLwip2EnetObj.print)
    {
        /* Function is non- reentrant */
        va_list vaArgPtr;
        char *buf;

        buf = &gLwip2EnetObj.printBuf[0];
        va_start(vaArgPtr, prnStr);
        vsnprintf(buf, ENET_CFG_PRINT_BUF_LEN, (const char *)prnStr, vaArgPtr);
        va_end(vaArgPtr);

        (*gLwip2EnetObj.print)("[LWIP2ENET] ");
        (*gLwip2EnetObj.print)(gLwip2EnetObj.printBuf);
    }
}

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

static int32_t Lwip2Enet_startRxTx(Lwip2Enet_Handle hLwip2Enet)
{
    int32_t status = ENET_SOK;

    Lwip2Enet_assert(NULL != hLwip2Enet->appInfo.txInfo.hTxChannel);
    Lwip2Enet_assert(NULL != hLwip2Enet->appInfo.rxInfo.hRxFlow);

    status = EnetDma_enableTxEvent(hLwip2Enet->appInfo.txInfo.hTxChannel);

    /* Prepare a Queue of DmaPktInfo pointers pointing to DmaPktInfoMem */
    Lwip2Enet_prepFreePktInfoQ(hLwip2Enet);

    /* Allocate/fill the PBUF/DMA packets for the Rx channel */
    Lwip2Enet_allocRxPackets(hLwip2Enet);

    /* Submit all allocated packets to DMA so it can use for packet RX */
    Lwip2Enet_submitRxPktQ(hLwip2Enet);

    return status;
}

static void Lwip2Enet_stopRxTx(Lwip2Enet_Handle hLwip2Enet)
{
    /* Close RX channel */
    /* Post to rx packet task so that it will terminate (shutDownFlag flag is already set) */
    {
        SemaphoreP_post(&hLwip2Enet->rxPacketSemObj);
    }

    /* Close TX channel */
    /* Post to tx packet task so that it will terminate (shutDownFlag flag is already set) */
    {
        SemaphoreP_post(&hLwip2Enet->txPacketSemObj);
    }
}
static void Lwip2Enet_freeTxPktCb(void *cbArg,
                                 EnetDma_PktQ *fqPktInfoQ,
                                 EnetDma_PktQ *cqPktInfoQ)
{
    Lwip2Enet_Handle hLwip2Enet = (Lwip2Enet_Handle)cbArg;

    EnetQueue_append(&hLwip2Enet->txFreePktInfoQ, fqPktInfoQ);
    Lwip2EnetStats_addNum(&gLwip2EnetStats.txFreeAppPktEnq, EnetQueue_getQCount(fqPktInfoQ));
    Lwip2Enet_freePbufPackets(fqPktInfoQ);

    EnetQueue_append(&hLwip2Enet->txFreePktInfoQ, cqPktInfoQ);
    Lwip2EnetStats_addNum(&gLwip2EnetStats.txFreeAppPktEnq, EnetQueue_getQCount(cqPktInfoQ));
    Lwip2Enet_freePbufPackets(cqPktInfoQ);
}

static void Lwip2Enet_freeRxPktCb(void *cbArg,
                                 EnetDma_PktQ *fqPktInfoQ,
                                 EnetDma_PktQ *cqPktInfoQ)
{
    Lwip2Enet_Handle hLwip2Enet = (Lwip2Enet_Handle)cbArg;

    /* Now that we free PBUF buffers, push all freed pktInfo's into Rx
     * free Q */
    EnetQueue_append(&hLwip2Enet->rxFreePktInfoQ, fqPktInfoQ);
    Lwip2EnetStats_addNum(&gLwip2EnetStats.rxFreeAppPktEnq, EnetQueue_getQCount(fqPktInfoQ));
    Lwip2Enet_freePbufPackets(fqPktInfoQ);

    EnetQueue_append(&hLwip2Enet->rxFreePktInfoQ, cqPktInfoQ);
    Lwip2EnetStats_addNum(&gLwip2EnetStats.rxFreeAppPktEnq, EnetQueue_getQCount(cqPktInfoQ));
    Lwip2Enet_freePbufPackets(cqPktInfoQ);

    /* Flush out our pending receive queues */
    while (pbufQ_count(&hLwip2Enet->rxFreePbufPktQ) != 0)
    {
        pbuf_free(pbufQ_deQ(&hLwip2Enet->rxFreePbufPktQ));
        Lwip2EnetStats_addOne(&gLwip2EnetStats.rxFreePbufPktDq);
    }
}

static void Lwip2Enet_retrieveTxPkts(Lwip2Enet_Handle hLwip2Enet)
{
    EnetDma_PktQ tempQueue;
    uint32_t packetCount = 0;
    int32_t retVal;

    Lwip2EnetStats_addOne(&gLwip2EnetStats.txStats.rawNotifyCnt);
    packetCount = 0;

    /* Retrieve the used (sent/empty) packets from the channel */
    {
        EnetQueue_initQ(&tempQueue);
        /* Retrieve all TX packets and keep them locally */
        retVal = EnetDma_retrieveTxPktQ(hLwip2Enet->appInfo.txInfo.hTxChannel,
                                               &tempQueue);
        if (ENET_SOK != retVal)
        {
            Lwip2Enet_print("Lwip2Enet_retrieveTxPkts: Failed to retrieve TX pkts: %d\n", retVal);
        }
    }

    if (tempQueue.count != 0U)
    {
        /*
         * Get all used Tx DMA packets from the hardware, then return those
         * buffers to the txFreePktQ so they can be used later to send with.
         */
        packetCount = Lwip2Enet_prepTxPktQ(hLwip2Enet, &tempQueue);
    }
    else
    {
        Lwip2EnetStats_addOne(&gLwip2EnetStats.txStats.zeroNotifyCnt);
    }

    if (packetCount != 0)
    {
        Lwip2Enet_updateTxNotifyStats(packetCount, 0U);
    }
}

static void Lwip2Enet_timerCb(ClockP_Object *hClk, void * arg)
{
    /* Post semaphore to rx handling task */
    Lwip2Enet_Handle hLwip2Enet = (Lwip2Enet_Handle)arg;

    if ((hLwip2Enet->initDone) && (hLwip2Enet->shutDownFlag == false))
    {
        SemaphoreP_post(&hLwip2Enet->rxPacketSemObj);
    }
}

static void Lwip2Enet_createTimer(Lwip2Enet_Handle hLwip2Enet)
{
    ClockP_Params clkPrms;
    int32_t status;

    ClockP_Params_init(&clkPrms);
    clkPrms.start  = 0;
    clkPrms.timeout = ClockP_usecToTicks(hLwip2Enet->appInfo.timerPeriodUs);
    clkPrms.period = ClockP_usecToTicks(hLwip2Enet->appInfo.timerPeriodUs);
    clkPrms.callback = &Lwip2Enet_timerCb;
    clkPrms.args = hLwip2Enet;

    status =  ClockP_construct(&hLwip2Enet->pacingClkObj, &clkPrms);
    Lwip2Enet_assert(status == SystemP_SUCCESS);
}

static void  Lwip2Enet_initGetHandleInArgs(Lwip2Enet_Handle hLwip2Enet,
                                          LwipifEnetAppIf_GetHandleInArgs *inArgs)
{
    inArgs->txCfg.cbArg      = hLwip2Enet;
    inArgs->txCfg.notifyCb   = &Lwip2Enet_notifyTxPackets;
    inArgs->txCfg.numPackets = LWIP2ENET_TX_PACKETS;

    inArgs->rxCfg.cbArg      = hLwip2Enet;
    inArgs->rxCfg.notifyCb   = &Lwip2Enet_notifyRxPackets;
    inArgs->rxCfg.numPackets = LWIP2ENET_RX_PACKETS;
}

static void Lwip2Enet_initReleaseHandleInArgs(Lwip2Enet_Handle hLwip2Enet,
                                             LwipifEnetAppIf_ReleaseHandleInfo *inArgs)
{
    inArgs->coreId       = hLwip2Enet->appInfo.coreId;
    inArgs->coreKey      = hLwip2Enet->appInfo.coreKey;
    inArgs->hEnet        = hLwip2Enet->appInfo.hEnet;
    inArgs->hUdmaDrv     = hLwip2Enet->appInfo.hUdmaDrv;
    inArgs->rxFreePktCb  = &Lwip2Enet_freeRxPktCb;
    inArgs->rxInfo       = hLwip2Enet->appInfo.rxInfo;
    inArgs->txFreePktCb  = &Lwip2Enet_freeTxPktCb;
    inArgs->txInfo       = hLwip2Enet->appInfo.txInfo;
    inArgs->freePktCbArg = hLwip2Enet;
}
