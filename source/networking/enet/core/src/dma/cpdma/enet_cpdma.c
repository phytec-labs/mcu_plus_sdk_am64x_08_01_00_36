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
 * \file  enet_cpdma.c
 *
 * \brief This file contains the implementation of the Enet data path with CPDMA.
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <string.h>
#include <csl_cpswitch.h>

#include <enet_cfg.h>
#include <include/core/enet_base.h>
#include <include/core/enet_utils.h>
#include <include/core/enet_soc.h>
#include <include/core/enet_per.h>
#include <include/core/enet_queue.h>
#include <priv/core/enet_base_priv.h>
#include <priv/core/enet_trace_priv.h>
#include <include/common/enet_osal_dflt.h>
#include <include/common/enet_utils_dflt.h>

#include <include/core/enet_dma.h>
#include <include/core/enet_dma_pktutils.h>
#include <src/dma/cpdma/enet_cpdma_priv.h>
#if defined (SOC_AM273X) || defined(SOC_AWR294X) || defined(SOC_AM263X)
#include <priv/per/cpsw_cpdma_priv.h>
#endif

#if defined (SOC_AM273X)
#include <hw_include/am273x/csl_soc_util.h>
#endif

#if defined (SOC_AWR294X)
#include <hw_include/awr294x/csl_soc_util.h>
#endif

#if defined (SOC_AM263X)
#include <hw_include/am263x/csl_soc_util.h>
#endif

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

static int32_t EnetCpdma_checkTxChParams(EnetCpdma_OpenTxChPrms *pTxChPrms);

static int32_t EnetCpdma_checkRxChParams(EnetCpdma_OpenRxChPrms *pRxChPrms);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

/** ============================================================================
 *  @n@b EnetCpdma_enableChannel()
 *
 *  @b Description
 *  @n This function configures the appropriate registers to initialize a
 *  DMA channel.
 *
 *  @b Arguments
 *  @verbatim
 *      channel             Channel number
 *      direction           Channel Direction, i.e., CPSW_DMA_DIR_TX/CPSW_DMA_DIR_RX
    @endverbatim
 *
 *  <b> Return Value </b>
 *  @n  None
 *
 *  <b> Pre Condition </b>
 *  @n  None
 *
 *  <b> Post Condition </b>
 *  @n  Initializes the Tx/Rx HDP and enables interrupts on the specific channel.
 *
 *  @b Example
    @endverbatim
 * ============================================================================
 */
static void EnetCpdma_enableChannel(EnetDma_Handle hEnetDma, uint32_t channel, uint32_t direction); /* for misra warning */
static void EnetCpdma_enableChannel(EnetDma_Handle hEnetDma, uint32_t channel, uint32_t direction)
{
    if (direction == ENET_CPDMA_DIR_TX)
    {
        /* enable a TX Dma channel for transfers here */

        CSL_CPSW_setCpdmaTxHdrDescPtr(hEnetDma->cpdmaRegs, 0, channel);
        CSL_CPSW_enableCpdmaTxInt(hEnetDma->cpdmaRegs, channel);
        CSL_CPSW_enableWrTxInt(hEnetDma->cpswSsRegs, 0, channel);

        /* mark the channel as open */
        hEnetDma->chIsOpen[ENET_CPDMA_DIR_TX][channel] = true;
    }
    else
    {
        /* enable RX Dma channel for transfer here */
        EnetCpdma_DescCh  *rxChan;
        rxChan = &hEnetDma->rxCppi[channel];
        CSL_CPSW_enableCpdmaRxInt(hEnetDma->cpdmaRegs, channel);
        CSL_CPSW_enableWrRxInt(hEnetDma->cpswSsRegs, 0, channel);
        /* mark the channel as open */
        hEnetDma->chIsOpen[ENET_CPDMA_DIR_RX][channel] = true;

        /* Mark the Rx channel should be restarted when the descrtptor and buffers are ready */
        rxChan->restart = 1U;
    }
}

/** ============================================================================
 *  @n@b EnetCpdma_disableChannel()
 *
 *  @b Description
 *  @n This function configures the appropriate registers to de-initialize a
 *  DMA channel.
 *
 *  @b Arguments
 *  @verbatim
 *      channel             Channel number
 *      direction           Channel Direction, i.e., ENET_CPDMA_DIR_TX/ENET_CPDMA_DIR_RX
    @endverbatim
 *
 *  <b> Return Value </b>
 *  @n  Always returns ENET_SOK
 *
 *  <b> Pre Condition </b>
 *  @n  None
 *
 *  <b> Post Condition </b>
 *  @n  De-Initializes the channel by resetting Tx/Rx HDP and disabling interrupts on
 *  the specific channel.
 *
 *  @b Example
    @endverbatim
 * ============================================================================
 */
static void EnetCpdma_disableChannel(EnetDma_Handle hEnetDma, uint32_t channel, uint32_t direction); /* for misra warning */
static void EnetCpdma_disableChannel(EnetDma_Handle hEnetDma, uint32_t channel, uint32_t direction)
{
    uint32_t desc;
    hEnetDma->tdPending[direction][channel] = true;

    if (direction == ENET_CPDMA_DIR_TX)
    {
        /* disable the requested Tx DMA channel here */

        /* command teardown  */
        CSL_CPSW_cpdmaTxTeardown(hEnetDma->cpdmaRegs, channel);

        /* wait for ack ,examine CP for EMAC_TEARDOWN_DESC (0xfffffffc) */
        do
        {
            CSL_CPSW_getCpdmaTxCp(hEnetDma->cpdmaRegs, channel, &desc);
        } while(desc != ENET_CPDMA_TEARDOWN_DESC);


        /* disable Tx Interrupt on this channel */
        CSL_CPSW_disableCpdmaTxInt(hEnetDma->cpdmaRegs, channel);

        /* also disable in the wrapper */
        CSL_CPSW_disableWrTxInt(hEnetDma->cpswSsRegs, 0, channel);
    }
    else
    {
        /* disable the requested Rx Dma channel here */

        /* command teardown  */
        CSL_CPSW_cpdmaRxTeardown(hEnetDma->cpdmaRegs, channel);

        /* wait for ack ,examine CP for 0xfffffffc */
        do
        {
            CSL_CPSW_getCpdmaRxCp(hEnetDma->cpdmaRegs, channel, &desc);
        } while(desc != ENET_CPDMA_TEARDOWN_DESC);

        /*disable Rx Interrupt on this channel */
        CSL_CPSW_disableCpdmaRxInt(hEnetDma->cpdmaRegs, channel);

        /* also disable in the wrapper */
        CSL_CPSW_disableWrRxInt(hEnetDma->cpswSsRegs, 0, channel);
    }

    hEnetDma->tdPending[direction][channel] = false;
    hEnetDma->chIsOpen[direction][channel] = false;
}

/** ============================================================================
 *  @n@b EnetCpdma_initTxChannel()
 *
 *  @b Description
 *  @n This function sets up the Transmit Buffer descriptors.
 *
 *  @b Arguments
 *  @verbatim
 *      chInfo              Channel object
    @endverbatim
 *
 *  <b> Return Value </b>
 *  ENET_EBADARGS  -   Returned on BD allocation error
 *  ENET_SOK       -   On Success
 *
 *  <b> Pre Condition </b>
 *  @n  None
 *
 *  <b> Post Condition </b>
 *  @n  Sets up the Tx Buffer descriptors. Tx channel ready for send.
 *
 *  @b Example
    @endverbatim
 * ============================================================================
 */
static int32_t EnetCpdma_initTxChannel(EnetDma_Handle hEnetDma, EnetCpdma_ChInfo *chInfo); /* for misra warning */
static int32_t EnetCpdma_initTxChannel(EnetDma_Handle hEnetDma, EnetCpdma_ChInfo *chInfo)
{
    int32_t retVal = ENET_SOK ;
    EnetCpdma_DescCh* txChan = &hEnetDma->txCppi[chInfo->chNum];
    EnetCpdma_cppiDesc    *pDesc;

    /* zero init the book keeping structure */
    memset(txChan, 0, sizeof(EnetCpdma_DescCh));

    /* store pointer to channel info structure */
    txChan->chInfo = chInfo;

    /*
     * Setup Transmit Buffer Descriptors
     */
    /* Pointer to first descriptor to use on this channl */
    pDesc = (EnetCpdma_cppiDesc *)hEnetDma->cppiRamBase;
    pDesc += hEnetDma->numBdsAllocated; /* advance to next free BD */

    if ((hEnetDma->numBdsAllocated + chInfo->numBD) > ENET_CPDMA_MAX_BDS)
    {
        /* not enough room for the requested number of BDs, fail request */
        ENETTRACE_ERR("InitTx Channel : Unable to allocate %d BDs for channel %d. %d BDs already in use\n",
                      chInfo->numBD,chInfo->chNum,hEnetDma->numBdsAllocated);
        retVal = ENET_EBADARGS;
    }
    else
    {
        /* Setup Transmit Buffers */
        txChan->hEnetDma   = hEnetDma;
        txChan->descMax    = chInfo->numBD;
        /*Pointer for first TX desc = pointer to RX + num of RX desc.*/
        txChan->pDescFirst = pDesc;
        txChan->pDescLast  = pDesc + (chInfo->numBD - 1);
        txChan->pDescRead  = pDesc;
        txChan->pDescWrite = (EnetCpdma_cppiDesc*)0xffffffffU;
        txChan->dmaInProgress = 0;

        /* clear the teardown pending flag */
        hEnetDma->tdPending[ENET_CPDMA_DIR_TX][chInfo->chNum] = false;
        /* update the Bd allocation count */
        hEnetDma->numBdsAllocated += chInfo->numBD;
        hEnetDma->chIsInit[ENET_CPDMA_DIR_TX][chInfo->chNum] = true;
    }
    return (retVal);
}

/** ============================================================================
 *  @n@b EnetCpdma_initRxChannel()
 *
 *  @b Description
 *  @n This function sets up the Receive Buffer descriptors.
 *
 *  @b Arguments
 *  @verbatim
 *      chInfo              Channel object
    @endverbatim
 *
 *  <b> Return Value </b>
 *  ENET_EBADARGS  -   Returned on BD allocation error
 *  ENET_SOK      -   On Success
 *
 *  <b> Pre Condition </b>
 *  @n  None
 *
 *  <b> Post Condition </b>
 *  @n  Sets up the Rx Buffer descriptors. Rx channel ready for receive.
 *
 *  @b Example
    @endverbatim
 * ============================================================================
 */
static int32_t EnetCpdma_initRxChannel(EnetDma_Handle hEnetDma, EnetCpdma_ChInfo *chInfo); /* for misra warning */
static int32_t EnetCpdma_initRxChannel(EnetDma_Handle hEnetDma, EnetCpdma_ChInfo *chInfo)
{
    int32_t retVal = ENET_SOK;
    EnetCpdma_DescCh* rxChan = &hEnetDma->rxCppi[chInfo->chNum];
    EnetCpdma_cppiDesc *pDesc;

    /* zero init the book keeping structure */
    memset(rxChan, 0, sizeof(EnetCpdma_DescCh));

    /* store pointer to channel info structure */
    rxChan->chInfo = chInfo;

    /*
     * Setup Receive Buffers
     */
    /* Pointer to first descriptor to use on RX */
    pDesc = (EnetCpdma_cppiDesc *)hEnetDma->cppiRamBase;
    if (retVal == ENET_SOK)
    {
        pDesc += hEnetDma->numBdsAllocated; /* advance to next free BD */

        if ((hEnetDma->numBdsAllocated + chInfo->numBD) >  ENET_CPDMA_MAX_BDS)
        {
            /* not enough room for the requested number of BDs, fail request */
            ENETTRACE_ERR("InitRx Channel : Unable to allocate %d BDs for channel %d.%d BDs already in use\n",
                          chInfo->numBD,chInfo->chNum,hEnetDma->numBdsAllocated);
            retVal =  ENET_EBADARGS;
        }
        else
        {
            /* Init the Rx channel */
            rxChan->hEnetDma   = hEnetDma;
            rxChan->descMax    = chInfo->numBD;
            rxChan->pDescFirst = pDesc;
            rxChan->pDescLast  = pDesc + (chInfo->numBD - 1);
            rxChan->pDescRead  = pDesc;
            rxChan->pDescWrite = pDesc;
            rxChan->dmaInProgress = 0;

            /* clear the teardown pending flag */
            hEnetDma->tdPending[ENET_CPDMA_DIR_RX][chInfo->chNum] = false;
            hEnetDma->numBdsAllocated += chInfo->numBD;
            hEnetDma->chIsInit[ENET_CPDMA_DIR_RX][chInfo->chNum] = true;
        }
    }
    return (retVal);
}


/** ============================================================================
 *  @n@b EnetCpdma_unInitTxChannel()
 *
 *  @b Description
 *  @n This function frees up the enqueued Transmit Buffer descriptors and the
 *  packets held in any of its queues.
 *
 *  @b Arguments
 *  @verbatim
 *      chInfo              Channel object
    @endverbatim
 *
 *  <b> Return Value </b>
 *  None
 *
 *  <b> Pre Condition </b>
 *  @n  None
 *
 *  <b> Post Condition </b> None
 *
 *  @b Example
    @endverbatim
 * ============================================================================
 */
static void EnetCpdma_unInitTxChannel(EnetDma_Handle hEnetDma, const EnetCpdma_ChInfo *chInfo); /* for misra warning */
static void EnetCpdma_unInitTxChannel(EnetDma_Handle hEnetDma, const EnetCpdma_ChInfo *chInfo)
{
    EnetCpdma_DescCh* txChan = &hEnetDma->txCppi[chInfo->chNum];

    EnetQueue_append(&txChan->waitQueue, &txChan->descQueue);
    EnetQueue_initQ(&txChan->descQueue);
    hEnetDma->numBdsAllocated -= txChan->chInfo->numBD;
    hEnetDma->chIsInit[ENET_CPDMA_DIR_TX][chInfo->chNum] = false;
}

/** ============================================================================
 *  @n@b EnetCpdma_unInitRxChannel()
 *
 *  @b Description
 *  @n This function frees up the enqueued Receive Buffer descriptors and any
 *  packets held in the Rx queue.
 *
 *  @b Arguments
 *  @verbatim
 *      chInfo              Channel object
    @endverbatim
 *
 *  <b> Return Value </b>
 *  None
 *
 *  <b> Pre Condition </b>
 *  @n  None
 *
 *  <b> Post Condition </b> None
 *
 *  @b Example
    @endverbatim
 * ============================================================================
 */
static void EnetCpdma_unInitRxChannel(EnetDma_Handle hEnetDma, const EnetCpdma_ChInfo *chInfo); /* for misra warning */
static void EnetCpdma_unInitRxChannel(EnetDma_Handle hEnetDma, const EnetCpdma_ChInfo *chInfo)
{
    EnetCpdma_DescCh* rxChan = &hEnetDma->rxCppi[chInfo->chNum];

    EnetQueue_append(&rxChan->freeQueue, &rxChan->descQueue);
    EnetQueue_initQ(&rxChan->descQueue);
    hEnetDma->numBdsAllocated -= rxChan->chInfo->numBD;
    hEnetDma->chIsInit[ENET_CPDMA_DIR_RX][chInfo->chNum] = false;
}

/** ============================================================================
 *  @n@b EnetCpdma_netChOpen()
 *
 *  @b Description
 *  @n This function opens a data channel on the CPPI DMA engine.
 *
 *  @b Arguments
 *  @verbatim
 *      chInfo              Channel object to setup.
    @endverbatim
 *
 *  <b> Return Value </b>
 *  ENET_SOK          - Channel setup successful
 *  CPSW3G_ERR_CH_ALREADY_INIT  - Channel already initialized
 *  Other error values if Channel init failed.
 *
 *  <b> Pre Condition </b>
 *  @n  None
 *
 *  <b> Post Condition </b> None
 *
 *  @b Example
    @endverbatim
 * ============================================================================
 */
static int32_t  EnetCpdma_netChOpen(EnetDma_Handle hEnetDma, EnetCpdma_ChInfo *chInfo); /* for misra warning */
static int32_t  EnetCpdma_netChOpen(EnetDma_Handle hEnetDma, EnetCpdma_ChInfo *chInfo)
{
    int32_t retVal = ENET_SOK;
    uint32_t numChans = (chInfo->chDir == ENET_CPDMA_DIR_TX)?hEnetDma->numTxChans:
                                                             hEnetDma->numRxChans;
    /* Perform sanity checks on input params */
    if( chInfo->chNum >= numChans)
    {
        ENETTRACE_ERR("NetChOpen: Channel number (%d) invalid \n", chInfo->chNum);
        retVal = ENET_EINVALIDPARAMS;
    }
    else
    {
        if(hEnetDma->chIsInit[chInfo->chDir][chInfo->chNum] == true)
        {
            ENETTRACE_ERR("NetChOpen: %s Channel %d already initialized\n",
                          ((chInfo->chDir == ENET_CPDMA_DIR_TX) ? "TX" : "RX"), chInfo->chNum);
            retVal = ENET_EALREADYOPEN;
        }
        else
        {
            /* Perform book keeping for indv channel */
            if (chInfo->chDir == ENET_CPDMA_DIR_TX)
            {
                retVal = EnetCpdma_initTxChannel(hEnetDma, chInfo);
            }
            else
            {
                retVal = EnetCpdma_initRxChannel(hEnetDma, chInfo);
            }

            if (retVal != ENET_SOK)
            {
                ENETTRACE_ERR("NetChOpen: Error in initializing %s channel %d",
                              ((chInfo->chDir == ENET_CPDMA_DIR_TX) ? "TX" : "RX"), chInfo->chNum);
            }
            else
            {
                /* Enable this channel for use */
                EnetCpdma_enableChannel(hEnetDma, chInfo->chNum, chInfo->chDir);
            }
        }
    }
    return (retVal);
}

/** ============================================================================
 *  @n@b EnetCpdma_netChClose()
 *
 *  @b Description
 *  @n This function closes a previously open data channel on the CPPI DMA engine
 *  and frees up any memory held associated with it.
 *
 *  @b Arguments
 *  @verbatim
 *      chInfo              Channel object to clean up.
    @endverbatim
 *
 *  <b> Return Value </b>
 *          None
 *
 *  <b> Pre Condition </b>
 *  @n  None
 *
 *  <b> Post Condition </b> None
 *
 *  @b Example
    @endverbatim
 * ============================================================================
 */
static int32_t EnetCpdma_netChClose(EnetDma_Handle hEnetDma, const EnetCpdma_ChInfo *chInfo); /* for misra warning */
static int32_t EnetCpdma_netChClose(EnetDma_Handle hEnetDma, const EnetCpdma_ChInfo *chInfo)
{

    int32_t retVal = ENET_SOK;

    if(hEnetDma->chIsOpen[chInfo->chDir][chInfo->chNum]  == true)
    {
        EnetCpdma_disableChannel(hEnetDma, chInfo->chNum, chInfo->chDir);
    }

    /* TODO: should we perform dequeue here? */

    if(hEnetDma->chIsInit[chInfo->chDir][chInfo->chNum] == true)
    {
        if(chInfo->chDir == ENET_CPDMA_DIR_TX)
        {
            EnetCpdma_unInitTxChannel(hEnetDma, chInfo);
        }
        else
        {
            EnetCpdma_unInitRxChannel(hEnetDma, chInfo);
        }
    }
    return (retVal);
}

/** ============================================================================
 *  @n@b EnetCpdma_enqueueTx()
 *
 *  @b Description
 *  @n Enqueue a TX packet and restart transmitter as needed
 *
 *  @b Arguments
 *  @verbatim
        pq  pointer to Channel descriptor
    @endverbatim
 *
 *  <b> Return Value </b>  None
 *
 *  <b> Pre Condition </b>
 *  @n  None
 *
 *  <b> Post Condition </b>
 *  @n  Enqueue a TX packet and restart transmitter as needed
 *
 *  @b Example
 *  @verbatim
        EnetCpdma_DescCh *pDescCh;

        EnetCpdma_enqueueTx ( pDescCh );
    @endverbatim
 * ============================================================================
 */
static void EnetCpdma_enqueueTx( EnetCpdma_DescCh *pDescCh)
{
    EnetCpdma_cppiDesc    *pDescThis = NULL;
    EnetCpdma_PktInfo     *pPkt = NULL;
    volatile EnetCpdma_cppiDesc *pStartPtr = NULL;
    volatile EnetCpdma_cppiDesc *pDescPrev = NULL;
    uint32_t count;
    uintptr_t key;

    key = EnetOsal_disableAllIntr();
    /* TODO: enhance to remove the dmaInProgess restriction: add link to the previous pointer */
    if (pDescCh->dmaInProgress == 0U)
    {
        /* pDescWrite is in initialized to 0xffffffff to indicate 1st time tx enqueue, starting point */
        if (pDescCh->pDescWrite == (EnetCpdma_cppiDesc*)0xffffffffU)
        {
            pDescCh->pDescWrite = pDescCh->pDescFirst;
        }
        pStartPtr = pDescCh->pDescWrite;
        /* Try to post any waiting packets if there is enough room */
        while (((count = EnetQueue_getQCount(&pDescCh->waitQueue)) > 0U) &&
               (pDescCh->descCount < pDescCh->descMax))
        {
            pPkt = (EnetCpdma_PktInfo *)EnetQueue_deq(&pDescCh->waitQueue);
            if (pPkt != NULL)
            {
                /* Assign the pointer to "this" desc */
                pDescThis = pDescCh->pDescWrite;
                /* Move the write pointer and bump count */
                if (pDescCh->pDescWrite == pDescCh->pDescLast)
                {
                    pDescCh->pDescWrite = pDescCh->pDescFirst;
                }
                else
                {
                    pDescCh->pDescWrite++;
                }

                /*
                 * If this is the last descriptor, the forward pointer is (void *)0
                 * Otherwise; this desc points to the next desc in the wait queue
                 */
                if (count == 1U)
                {
                    pDescThis->pNext = NULL;
                }
                else
                {
                    pDescThis->pNext = (EnetCpdma_cppiDesc *)(uintptr_t)CSL_locToGlobAddr((uintptr_t)pDescCh->pDescWrite);
                }

                if (pDescPrev)
                {
                    pDescPrev->pNext = (EnetCpdma_cppiDesc *)(uintptr_t)CSL_locToGlobAddr((uintptr_t)pDescThis);
                    //EnetOsal_cacheWbInv((void*)pDescPrev, sizeof(void *));
                }

                pDescThis->pBuffer   = (uint8_t *)(uintptr_t)CSL_locToGlobAddr((uintptr_t)pPkt->bufPtr);
                pDescThis->bufOffLen = pPkt->userBufLen;
                /* Only single-buffer packet is supported */
                pDescThis->pktFlgLen = pPkt->userBufLen | ENET_CPDMA_DESC_PKT_FLAG_SOP |
                                       ENET_CPDMA_DESC_PKT_FLAG_EOP | ENET_CPDMA_DESC_PKT_FLAG_OWNER;

                /* For directed packet case, port is specified, need to update TO_PORT_ENABLE and TO_PORT field in the desciptor */
                /* TODO: add range check */
                if (pPkt->txPortNum != ENET_MAC_PORT_INV)
                {
                    uint8_t portNum = (uint8_t)pPkt->txPortNum + 1U;
                    pDescThis->pktFlgLen |= (((uint32_t)1U) << ENET_CPDMA_DESC_PSINFO_TO_PORT_ENABLE_SHIFT);
                    pDescThis->pktFlgLen |= (portNum << ENET_CPDMA_DESC_PSINFO_TO_PORT_SHIFT);
                }

                //EnetOsal_cacheWbInv((void*)pDescThis, sizeof(EnetCpdma_cppiDesc));

                pDescPrev = pDescThis;

                /* TODO: consider global address transaltion */
                EnetOsal_cacheWbInv((void*)pPkt->bufPtr, pPkt->orgBufLen);
                EnetQueue_enq(&pDescCh->descQueue, &pPkt->node);
                pDescCh->descCount++;
            }
            else
            {
                /* TODO: error handling: queue corruption */
            }
        }

        if (pPkt != NULL)
        {
            /* last descriptor in the chain should have pointer to next descriptor as NULL */
            if (pDescThis)
            {
                pDescThis->pNext = NULL;
            }

            EnetOsal_cacheWbInv((void*)pDescCh->pDescFirst, pDescCh->descMax * sizeof(EnetCpdma_cppiDesc));

            pDescCh->dmaInProgress = 1;
            //HW_SYNC_BARRIER();
            CSL_CPSW_setCpdmaTxHdrDescPtr(pDescCh->hEnetDma->cpdmaRegs,
                                          (uint32_t)(uintptr_t)CSL_locToGlobAddr((uintptr_t)pStartPtr),
                                          pDescCh->chInfo->chNum);
        }
    }
    EnetOsal_restoreAllIntr(key);
}

/** ============================================================================
 *  @n@b EnetCpdma_dequeueTx()
 *
 *  @b Description
 *  @n Dequeue all completed TX packets and return buffers to freeQueue
 *
 *  @b Arguments
 *  @verbatim
        pDescCh     pointer to channel descriptor
        pDescAck    pointer to Descriptor object (from ISR)
    @endverbatim
 *
 *  <b> Return Value </b>  None
 *
 *  <b> Pre Condition </b>
 *  @n  None
 *
 *  <b> Post Condition </b>
 *  @n  Dequeue all completed TX packets and return buffers to application
 *
 *  @b Example
 *  @verbatim
        EnetCpdma_DescCh *pDescCh;
        EnetCpdma_cppiDesc *pDescAck;

        EnetCpdma_dequeueTx ( pDescCh, pDescAck );
    @endverbatim
 * ============================================================================
 */
static void EnetCpdma_dequeueTx(EnetCpdma_DescCh *pDescCh, EnetCpdma_cppiDesc *pDescAck)
{
    EnetCpdma_PktInfo *pPkt = NULL;
    int32_t tmp = 1;
    EnetCpdma_cppiDesc *pDescLast = NULL;
    EnetCpdma_cppiDesc *pDesc;
    uintptr_t key;

    key = EnetOsal_disableAllIntr();
    while ((tmp == 1) && (pDescCh->descCount > 0U))
    {
        pDesc = pDescCh->pDescRead;

        EnetOsal_cacheInv((void*)pDesc,  sizeof(EnetCpdma_cppiDesc));

        if ((pDesc->pktFlgLen) & ENET_CPDMA_DESC_PKT_FLAG_OWNER)
        {
            tmp = 0;
            if(pDescLast != NULL)
            {
                /* Ack Tx completion interrupt */
                CSL_CPSW_setCpdmaTxCp(pDescCh->hEnetDma->cpdmaRegs, pDescCh->chInfo->chNum,
                                    (uint32_t)(uintptr_t)CSL_locToGlobAddr((uintptr_t)pDescLast));
            }
        }
        else
        {
            /* we now own the packet meaning its been transferred to the port */
            pPkt = (EnetCpdma_PktInfo *)EnetQueue_deq(&pDescCh->descQueue);
            if (pPkt)
            {
                EnetQueue_enq(&pDescCh->freeQueue, &pPkt->node);
                pDescCh->descCount--;
            }
            else
            {
                /* TODO: error case: descriptor out of sync */
            }

            pDescLast = pDesc;

            /* Move the read pointer */
            if (pDescCh->pDescRead == pDescCh->pDescLast)
            {
                pDescCh->pDescRead = pDescCh->pDescFirst;
            }
            else
            {
                pDescCh->pDescRead++;
            }

            /* we have EOQ and we need to see if any packets chained to this one which will require re-start of transmitter */
            if( pDesc->pktFlgLen & ENET_CPDMA_DESC_PKT_FLAG_EOQ)
            {
                /* Ack Tx completion interrupt */
                CSL_CPSW_setCpdmaTxCp(pDescCh->hEnetDma->cpdmaRegs, pDescCh->chInfo->chNum,
                                      (uint32_t)(uintptr_t)CSL_locToGlobAddr((uintptr_t)pDesc));

                /* whether packets are chained or not, we are finished processing for this interrupt cycle */
                tmp = 0;
                if(pDesc->pNext)
                {
                    CSL_CPSW_setCpdmaTxHdrDescPtr(pDescCh->hEnetDma->cpdmaRegs, (uint32_t)pDesc->pNext,
                                                  pDescCh->chInfo->chNum);
                }
                else
                {
                    pDescCh->dmaInProgress = 0U;
                }
            }
            /* EOQ is not set, check to see if we chained any descriptors, if we did, continue with loop */
            else
            {
                if(pDesc->pNext)
                {
            #if 0
                    /* pNext should point to the next Read pointer */
                    if (pDescCh->pDescRead != (EnetCpdma_cppiDesc *)CSL_globToLocAddr((uint64_t)(uintptr_t)pDesc->pNext))
                    {
                        /* TODO: error handling */
                    }
            #endif
                }
                else
                {
                    /* TODO: error handling required (Non-EOQ packet should point to the next one */
                    tmp = 0;
                }
            }
        }
     }

    /* Try to post any waiting TX packets */
    if ((EnetQueue_getQCount(&pDescCh->waitQueue) > 0U) && (pDescCh->dmaInProgress == 0U))
    {
        EnetCpdma_enqueueTx(pDescCh);
    }
    EnetOsal_restoreAllIntr(key);
}

/** =========================================================================
 *  @n@b EnetCpdma_enqueueRx()
 *
 *  @b Description
 *  @n Fill any empty RX descriptors with new buffers from the free queue
 *
 *  @b Arguments
 *  @verbatim
        pDescCh     pointer to Descriptor object
        fRestart    re-fill packet
    @endverbatim
 *
 *  <b> Return Value </b>  None
 *
 *  <b> Pre Condition </b>
 *  @n  None
 *
 *  <b> Post Condition </b>
 *  @n  Fill any empty RX descriptors with new buffers from the freeQueue.
 *
 *  @b Example
 *  @verbatim
        EnetCpdma_DescCh  *pDescCh;
        uint32_t         fRestart;

        EnetCpdma_enqueueRx( pDescCh, fRestart );
    @endverbatim
 * ============================================================================
 */
void EnetCpdma_enqueueRx(EnetCpdma_DescCh *pDescCh, uint32_t fRestart)
{
    EnetCpdma_PktInfo     *pPkt;
    EnetCpdma_cppiDesc    *pDesc, *pDescPrev;
    uintptr_t             key;

    key = EnetOsal_disableAllIntr();
    /* Fill RX Packets Until Full */
    while (pDescCh->descCount < pDescCh->descMax)
    {
        /* Get a new buffer from the free Queue */
        pPkt = (EnetCpdma_PktInfo *)EnetQueue_deq(&pDescCh->freeQueue);

        /* If no more buffers are available, break out of loop */
        if (pPkt == NULL)
        {
            break;
        }
        /* Fill in the descriptor for this buffer */
        pDesc = pDescCh->pDescWrite;

        /* Move the write pointer and bump count */
        if (pDescCh->pDescWrite == pDescCh->pDescLast)
        {
            pDescCh->pDescWrite = pDescCh->pDescFirst;
        }
        else
        {
            pDescCh->pDescWrite++;
        }
        pDescCh->descCount++;

        /* Supply buffer pointer with application supplied offset */
        pDesc->pNext     = NULL;
        pDesc->pBuffer   = (uint8_t *)(uintptr_t)CSL_locToGlobAddr((uintptr_t)pPkt->bufPtr);
        pDesc->bufOffLen = pPkt->orgBufLen;
        pDesc->pktFlgLen = ENET_CPDMA_DESC_PKT_FLAG_OWNER;

        /* Make the previous buffer point to us */
        if (pDesc == pDescCh->pDescFirst)
        {
            pDescPrev = pDescCh->pDescLast;
        }
        else
        {
            pDescPrev = pDesc - 1U;
        }
        pDescPrev->pNext = (EnetCpdma_cppiDesc *)(uintptr_t)CSL_locToGlobAddr((uintptr_t)pDesc);
        /* Push the packet buffer on the local descriptor queue */
        /* TODO: global address translation? */
        EnetOsal_cacheInv((void*)pPkt->bufPtr, pPkt->orgBufLen);
        EnetQueue_enq(&pDescCh->descQueue, &pPkt->node);
    }

    EnetOsal_cacheWbInv((void*)pDescCh->pDescFirst, pDescCh->descMax * sizeof(EnetCpdma_cppiDesc));

    /* Restart RX if we had ran out of descriptors and got some here */
    if ((pDescCh->restart == 1U) && (pDescCh->descCount != 0U))
    {
        CSL_CPSW_setCpdmaRxHdrDescPtr(pDescCh->hEnetDma->cpdmaRegs,
                                      (uint32_t)(uintptr_t)CSL_locToGlobAddr((uintptr_t)pDescCh->pDescRead),
                                      pDescCh->chInfo->chNum);
        pDescCh->restart = 0U;
    }
    EnetOsal_restoreAllIntr(key);
}

/** ============================================================================
 *  @n@b EnetCpdma_dequeueRx()
 *
 *  @b Description
 *  @n Dequeue all completed RX packets and start buffers to wait Queue
 *
 *  @b Arguments
 *  @verbatim
        pDescCh     pointer to descriptor channel object
        pDescAck    pointer to the acknowledge
    @endverbatim
 *
 *  <b> Return Value </b>  None
 *
 *  <b> Pre Condition </b>
 *  @n  None
 *
 *  <b> Post Condition </b>
 *  @n  Dequeue all completed RX packets and give buffers to application
 *
 *  @b Example
 *  @verbatim
        EnetCpdma_DescCh *pDescCh;
        EnetCpdma_cppiDesc   *pDescAck;

        EnetCpdma_dequeueRx( pDescCh, pDescAck );
    @endverbatim
 * ============================================================================
 */
void EnetCpdma_dequeueRx(EnetCpdma_DescCh *pDescCh, const EnetCpdma_cppiDesc *pDescAck)
{
    EnetCpdma_PktInfo  *pPkt = NULL;
    EnetCpdma_cppiDesc *pDesc;
    int32_t  tmp;
    uint32_t pktFlgLen;
    uint32_t tempVar;
    uintptr_t key;

    key = EnetOsal_disableAllIntr();
    /*
     * Pop & Free Buffers 'till the last Descriptor
     * One thing we know for sure is that all the decriptors from
     * the read pointer to pDescAsk are linked to each other via
     * their pNext field.
    */
    /* TODO: error check only */

    tmp = 1;
    while ((tmp == 1) && (pDescCh->descCount > 0U))
    {
        pDesc = pDescCh->pDescRead;
        EnetOsal_cacheInv((void*)pDesc,  sizeof(EnetCpdma_cppiDesc));

        /* Get the status of this descriptor */
        pktFlgLen = pDesc->pktFlgLen;

        /* Bit 16,17 and 18 indicate the port number(ingress)
         * Passcrc bit is always set in the received packets.Clear it before putting the \
         * packet in receive queue */
        pktFlgLen = pktFlgLen & (uint32_t)~ENET_CPDMA_DESC_PSINFO_PASSCRC_FLAG;

        /* Check the ownership of the packet */
        tempVar = pktFlgLen & ENET_CPDMA_DESC_PKT_FLAG_OWNER;
        if (tempVar == 0U)
        {
            /* Recover the buffer and free it */
            pPkt = (EnetCpdma_PktInfo *)EnetQueue_deq(&pDescCh->descQueue);
            if (pPkt != NULL)
            {
                EnetOsal_cacheInv((void*)pPkt->bufPtr, pPkt->orgBufLen);
                /* Fill in the necessary packet header fields */
                pPkt->userBufLen = pktFlgLen & 0xFFFFU;
                pPkt->rxPortNum = (Enet_MacPort)((pktFlgLen & ENET_CPDMA_DESC_PSINFO_FROM_PORT_MASK)
                                                 >> ENET_CPDMA_DESC_PSINFO_FROM_PORT_SHIFT);

                /* Store the packet to the wait Queue */
                EnetQueue_enq(&pDescCh->waitQueue, &pPkt->node);
            }
            else
            {
                DebugP_assert(false);
            }

            /* Acking Rx completion interrupt is done in Rx Isr. Dont do it here  */
            if (pktFlgLen & ENET_CPDMA_DESC_PKT_FLAG_EOQ)
            {
                DebugP_assert((pktFlgLen & ENET_CPDMA_DESC_PKT_FLAG_EOP) != 0);
                tmp = 2;
            }

            /* Move the read pointer and decrement count */
            if (pDescCh->pDescRead == pDescCh->pDescLast)
            {
                pDescCh->pDescRead = pDescCh->pDescFirst;
            }
            else
            {
                pDescCh->pDescRead++;
            }
            pDescCh->descCount--;
        }
        else
        {
            tmp = 0;
        }
    }

    /* If the receiver stopped and we have more descriptors for EOQ case, then restart */
    if (tmp == 2)
    {
        if (pDescCh->descCount != 0U)
        {
            CSL_CPSW_setCpdmaRxHdrDescPtr(pDescCh->hEnetDma->cpdmaRegs,
                                          (uint32_t)(uintptr_t)CSL_locToGlobAddr((uintptr_t)pDescCh->pDescRead),
                                          pDescCh->chInfo->chNum);
        }
        else
        {
            pDescCh->restart = 1U;
        }
    }
    EnetOsal_restoreAllIntr(key);
}

/**
 *  @b EnetCpdma_rxIsrProc
 *  @n
 *     Common Processing function for both Rx and Rx Thresh interrupt
 *
 *  @param[in]  hCPswDma
 *
 *  @retval
 *      ENET_SOK
 */
int32_t EnetCpdma_rxIsrProc(EnetDma_Handle hEnetDma)
{
    int32_t retVal = ENET_SOK;
    uint32_t         desc;
    uint32_t         channel = 0U;
    EnetCpdma_DescCh  *rxChan = &hEnetDma->rxCppi[channel];

    /* TODO: assume channel 0 for now */
    CSL_CPSW_getCpdmaRxCp(hEnetDma->cpdmaRegs, channel, &desc);

    if (desc == 0U)
    {
        /* False alarm, do nothing */
    }
    else if (desc == ENET_CPDMA_TEARDOWN_DESC)
    {
        /* Terdown is in progress */
        /* ToDO: should we invoke EnetCpdma_dequeueRx */
        CSL_CPSW_setCpdmaRxCp(hEnetDma->cpdmaRegs, channel, ENET_CPDMA_TEARDOWN_DESC);
    }
    else
    {
        EnetCpdma_dequeueRx(rxChan, (EnetCpdma_cppiDesc *)desc);
        CSL_CPSW_setCpdmaRxCp(hEnetDma->cpdmaRegs, channel, desc);
        /* Re-fill Rx buffer queue if needed */
        if (rxChan->descCount < rxChan->descMax)
        {
            EnetCpdma_enqueueRx(rxChan, true);
        }
    }

    /* Invoke callback function */
    if(NULL != rxChan->chInfo->notifyCb)
    {
        rxChan->chInfo->notifyCb(rxChan->chInfo->hCbArg);
    }

    return (retVal);
}
/**
 *  @b EnetCpdma_rxThreshIsr
 *  @n
 *      CPSW CPDMA Receive Threshold ISR.
 *
 *  @param[in]  hCPswDma
 *
 *  @retval
 *      ENET_SOK
 */
int32_t EnetCpdma_rxThreshIsr(EnetDma_Handle hEnetDma)
{
    int32_t retVal = ENET_SOK;

    /* Validate our handle */
    if (hEnetDma == NULL)
    {
        retVal = ENET_EBADARGS;
    }
    else
    {
        retVal = EnetCpdma_rxIsrProc(hEnetDma);
        CSL_CPSW_setCpdmaRxThresholdEndOfIntVector(hEnetDma->cpdmaRegs);
    }

    return (retVal);
}

/**
 *  @b EnetCpdma_rxIsr
 *  @n
 *      CPSW CPDMA Receive ISR.
 *
 *  @param[in]  hCPswDma
 *
 *  @retval
 *      ENET_SOK
 */
int32_t EnetCpdma_rxIsr(EnetDma_Handle hEnetDma)
{
    int32_t retVal = ENET_SOK;

    /* Validate our handle */
    if (hEnetDma == NULL)
    {
        retVal = ENET_EBADARGS;
    }
    else
    {
        retVal = EnetCpdma_rxIsrProc(hEnetDma);
        CSL_CPSW_setCpdmaRxEndOfIntVector(hEnetDma->cpdmaRegs);
    }
    return (retVal);
}

/**
 *  @b EnetCpdma_txIsr
 *  @n
 *      CPSW CPDMA Transmit ISR.
 *
 *  @param[in]  hCPswDma
 *
 *  @retval
 *      ENET_SOK
 */
int32_t EnetCpdma_txIsr(EnetDma_Handle hEnetDma)
{
    int32_t retVal = ENET_SOK;
    uint32_t         desc;
    uint32_t         channel = 0U;

    /* Validate our handle */
    if (hEnetDma == NULL)
    {
        retVal = ENET_EBADARGS;
    }
    else
    {
        /* TODO: assume channel 0 for now */
        EnetCpdma_DescCh  *txChan = &hEnetDma->txCppi[channel];

        /* find out if interrupt happend */
        CSL_CPSW_getCpdmaTxCp(hEnetDma->cpdmaRegs, channel, &desc);
        /* this should only happen while teardown is in process */
        if(desc == ENET_CPDMA_TEARDOWN_DESC)
        {
            /* need to ack with acknowledge value */
            CSL_CPSW_setCpdmaTxCp(hEnetDma->cpdmaRegs, channel, ENET_CPDMA_TEARDOWN_DESC);
        }
        else
        {
            EnetCpdma_dequeueTx(txChan, (EnetCpdma_cppiDesc *)desc);
        }

        /* write the EOI register */
        CSL_CPSW_setCpdmaTxEndOfIntVector(hEnetDma->cpdmaRegs);
        /* Invoke callback function */
        if(NULL != txChan->chInfo->notifyCb)
        {
            txChan->chInfo->notifyCb(txChan->chInfo->hCbArg);
        }
    }

    return (retVal);
}

/**
 *  @b EnetCpdma_miscIsr
 *  @n
 *      CPSW CPDMA Miscellaneous ISR.
 *
 *  @param[in]  hCPswDma
 *  @param[in]  pStatusMask
 *
 *  @retval
 *      ENET_SOK
 */
int32_t EnetCpdma_miscIsr(EnetDma_Handle hEnetDma, uint32_t *pStatusMask)
{
    int32_t retVal = ENET_SOK;

    /* Validate our handle */
    if (hEnetDma == NULL)
    {
        retVal = ENET_EBADARGS;
    }
    else
    {
        CSL_CPSW_setCpdmaMiscEndOfIntVector(hEnetDma->cpdmaRegs);
        *pStatusMask = CSL_CPSW_getWrMiscIntStatus(hEnetDma->cpswSsRegs, 0U);
    }

    return (retVal);
}

int32_t EnetCpdma_checkRxChSanity(EnetDma_RxChHandle hRxCh,
                                  uint32_t margin)
{
    int32_t retVal       = ENET_SOK;

    return retVal;
}

int32_t EnetCpdma_checkTxChSanity(EnetDma_TxChHandle hTxCh,
                                uint32_t margin)
{
    int32_t retVal       = ENET_SOK;

    return retVal;
}

static int32_t EnetCpdma_checkRxChParams(EnetCpdma_OpenRxChPrms *pRxChPrms)
{
    int32_t retVal = ENET_SOK;

    if (NULL == pRxChPrms)
    {
        ENETTRACE_ERR("[Enet CPDMA] Flow params is NULL!!\n");
        retVal = ENET_EBADARGS;
    }
    else
    {
        if (NULL == pRxChPrms->hEnet)
        {
            ENETTRACE_ERR("[Enet CPDMA] ENET not opened !!\n");
            retVal = ENET_EBADARGS;
        }
    }

    if (ENET_SOK == retVal)
    {
        if (pRxChPrms->chNum >= ENET_CPDMA_MAX_CHANNELS)
        {
            ENETTRACE_ERR("[Enet CPDMA] channel number for CPSW RX channel open !!\n", pRxChPrms->chNum);
            retVal = ENET_EBADARGS;
        }
    }
    return retVal;
}

static int32_t EnetCpdma_checkTxChParams(EnetCpdma_OpenTxChPrms *pTxChPrms)
{
    int32_t retVal = ENET_SOK;

    if (NULL == pTxChPrms)
    {
        ENETTRACE_ERR("[Enet CPDMA] TX channel params can't be NULL !!\n");
        retVal = ENET_EBADARGS;
    }
    else
    {
        if (NULL == pTxChPrms->hEnet)
        {
            /* DMA should be opened before opening Ch/Flow */
            ENETTRACE_ERR("[Enet CPDMA] ENET not opened !!\n");
            retVal = ENET_EBADARGS;
        }
    }

    if (ENET_SOK == retVal)
    {
        if (pTxChPrms->chNum >= ENET_CPDMA_MAX_CHANNELS)
        {
            ENETTRACE_ERR("[Enet CPDMA] Invalid channel number for CPDMA TX channel open !!: %d\n", pTxChPrms->chNum);
            retVal = ENET_EBADARGS;
        }
    }

    return retVal;
}

/* TODO: new functions */
void EnetCpdma_initParams(Enet_Type enetType, EnetDma_Cfg *pDmaConfig)
{
    pDmaConfig->isCacheable                 = true;
    pDmaConfig->rxInterruptPerMSec          = 0U;
    pDmaConfig->rxChInitPrms.dmaPriority    = 0U;
    pDmaConfig->rxChInitPrms.rxBufferOffset = 0U;
}

EnetDma_Handle EnetDma_open(Enet_Type enetType,
                            uint32_t instId,
                            const void *dmaCfg)
{
    EnetCpdma_DrvObj *pEnetDmaObj = NULL;
    EnetCpdma_Cfg *pDmaCfg = (EnetCpdma_Cfg *)dmaCfg;
    uint32_t status;
    int32_t retVal = ENET_SOK;

    /* Error check */
    if (NULL == pDmaCfg)
    {
        ENETTRACE_ERR("[ENET DMA Error] pDmaCfg NULL !!\n");
        retVal = ENET_EBADARGS;
    }

    if (ENET_SOK == retVal)
    {
        /* TODO: more error check */
        pEnetDmaObj = EnetSoc_getDmaHandle(enetType, 0U /* instId */);

        Enet_assert(pEnetDmaObj != NULL);

        pEnetDmaObj->isCacheable = pDmaCfg->isCacheable;

        /* TODO: how to configure Rx and Tx channel count (from sOC) */
        pEnetDmaObj->numTxChans = 1U;
        pEnetDmaObj->numRxChans = 1U;

        /* Initialize memory manager module managing driver object memories */
        EnetCpdma_memMgrInit();

        CSL_CPSW_resetCpdma(pEnetDmaObj->cpdmaRegs);
        status = CSL_CPSW_isCpdmaResetDone(pEnetDmaObj->cpdmaRegs);
        while (status == ((uint32_t)false))
        {
            status = CSL_CPSW_isCpdmaResetDone(pEnetDmaObj->cpdmaRegs);
        }

        CSL_CPSW_setCpdmaRxBufOffset(pEnetDmaObj->cpdmaRegs,
                                     pDmaCfg->rxChInitPrms.rxBufferOffset);

        /* TODO: why channel 0 only? Assume one channel for now */
        CSL_CPSW_disableCpdmaTxInt(pEnetDmaObj->cpdmaRegs, 0);
        CSL_CPSW_disableCpdmaRxInt(pEnetDmaObj->cpdmaRegs, 0);

        /* Acknowledge receive and transmit interrupts for proper interrupt pulsing */
        CSL_CPSW_setCpdmaTxEndOfIntVector(pEnetDmaObj->cpdmaRegs);
        CSL_CPSW_setCpdmaRxEndOfIntVector(pEnetDmaObj->cpdmaRegs);

        CSL_CPSW_enableCpdmaTx(pEnetDmaObj->cpdmaRegs);
        CSL_CPSW_enableCpdmaRx(pEnetDmaObj->cpdmaRegs);

        /* Enable Miscellaneous interrupts - stats and host error interupt */
        CSL_CPSW_enableCpdmaDmaInt(pEnetDmaObj->cpdmaRegs, CSL_CPDMA_DMA_INTMASK_SET_STAT_INT_MASK_MASK |
                                                           CSL_CPDMA_DMA_INTMASK_SET_HOST_ERR_INT_MASK_MASK);
        /* enable host,stats interrupt in cpsw_ss_s wrapper */
        /* TODO: should we enable CPTS/MDIO interrupt here */
        CSL_CPSW_enableWrMiscInt(pEnetDmaObj->cpswSsRegs, 0,
            CPSW_MISC_INT_HOSTERR_MASK | CPSW_MISC_INT_STAT_OVERFLOW_MASK);

        if (pDmaCfg->rxInterruptPerMSec != 0U)
        {
            /* enable Interrupt Pacing Logic in the Wrapper */
            CSL_CPSW_setWrRxIntPerMSec(pEnetDmaObj->cpswSsRegs, 0U, pDmaCfg->rxInterruptPerMSec);
            CSL_CPSW_setWrIntPacingControl(pEnetDmaObj->cpswSsRegs,  CPSW_INT_CONTROL_INT_PACE_EN_C0_RX);
            CSL_CPSW_setWrIntPrescaler(pEnetDmaObj->cpswSsRegs, 0x370U);
        }
    }
    return pEnetDmaObj;
}

int32_t EnetDma_close(EnetDma_Handle hEnetDma)
{
    int32_t retVal = ENET_SOK;

    // TODO check whether all Rx & Tx channels are closed
    /* Error check */
    if (hEnetDma == NULL)
    {
        ENETTRACE_ERR("[Enet CPDMA] Enet CPDMA handle is NULL!! \n");
        retVal = ENET_EBADARGS;
    }

    if (ENET_SOK == retVal)
    {
        if (hEnetDma->initFlag == false)
        {
            ENETTRACE_ERR("[Enet CPDMA] DMA is not initialized before close !! \n");
            retVal = ENET_EFAIL;
        }
    }

    if (ENET_SOK == retVal)
    {
        //uint32_t channel;
        uint32_t status;

        #if 0 /* TODO: uninit/disable active channel */
              /* Error condition, the applicatio needs to close both rx/tx channel prior to close */
        /* Close TX Channels */
        for (channel = 0; channel < EnetCpdma_CPDMA_MAX_CHANNELS; channel++)
        {
            EnetCpdma_NetChClose(&hEnetDma->chInfo[ENET_CPDMA_DIR_TX][channel]);
        }

        /* Close RX Channels */
        for (channel = 0; channel < EnetCpdma_CPDMA_MAX_CHANNELS; channel++)
        {
            EnetCpdma_NetChClose(&hEnetDma->chInfo[ENET_CPDMA_DIR_RX][channel]);
        }
        #endif

        /* Disable Adapter check interrupts - Disable stats interupt */
        CSL_CPSW_disableCpdmaDmaInt(hEnetDma->cpdmaRegs, CSL_CPDMA_DMA_INTMASK_CLEAR_STAT_INT_MASK_MASK |
                                                         CSL_CPDMA_DMA_INTMASK_CLEAR_HOST_ERR_INT_MASK_MASK);

        /* Disable host,stats interrupt in cpsw_3gss_s wrapper */
        CSL_CPSW_disableWrMiscInt(hEnetDma->cpswSsRegs, 0,
                                  CPSW_MISC_INT_HOSTERR_MASK | CPSW_MISC_INT_STAT_OVERFLOW_MASK);

        /* soft reset */
        CSL_CPSW_resetCpdma(hEnetDma->cpdmaRegs);
        status = CSL_CPSW_isCpdmaResetDone(hEnetDma->cpdmaRegs);
        while (status == ((uint32_t)false))
        {
            status = CSL_CPSW_isCpdmaResetDone(hEnetDma->cpdmaRegs);
        }
        /* De-initialize driver memory manager */
        EnetCpdma_memMgrDeInit();
        hEnetDma->initFlag = false;
    }

    return retVal;
}


void EnetDma_initRxChParams(void *pRxChCfg)
{
    EnetCpdma_OpenRxChPrms *pRxChPrms = (EnetCpdma_OpenRxChPrms *)pRxChCfg;

    pRxChPrms->hEnet = NULL;
    pRxChPrms->chNum = 0U;
    pRxChPrms->numRxPkts = 0U;
    pRxChPrms->notifyCb = NULL;
    pRxChPrms->cbArg   = NULL;

    return;
}

EnetDma_RxChHandle EnetDma_openRxCh(EnetDma_Handle hDma,
                                    const void *pRxChCfg)
{
    int32_t retVal;
    EnetCpdma_RxChObj *pRxCh;
    bool allocChObj = false;
    uint32_t intrKey;
    EnetCpdma_OpenRxChPrms *pRxChPrms = (EnetCpdma_OpenRxChPrms *)pRxChCfg;

    intrKey = EnetOsal_disableAllIntr();

    /* Set to NULL so if error condition we return NULL */
    pRxCh = NULL;

    /* Error check */
    retVal = EnetCpdma_checkRxChParams(pRxChPrms);

    if (ENET_SOK == retVal)
    {
        pRxCh = EnetCpdma_memMgrAllocRxChObj();

        if (pRxCh == NULL)
        {
            ENETTRACE_ERR("[Enet CPDMA] Memory allocation for Rx flow object failed !!\n");
            retVal = ENET_EALLOC;
        }
        else
        {
            allocChObj = true;
            memset(pRxCh, 0U, sizeof(*pRxCh));
            /* Save Flow config */
            pRxCh->rxChPrms = *pRxChPrms;
            pRxCh->hEnetDma = Cpsw_getDmaHandle(pRxChPrms->hEnet);

            /* TODO range check and derive channel if any */
            pRxCh->chInfo.chNum = pRxChPrms->chNum;
            pRxCh->chInfo.numBD = pRxChPrms->numRxPkts;
            pRxCh->chInfo.chDir = ENET_CPDMA_DIR_RX;
            //pRxCh->chInfo.bufSize = pRxChPrms->rxChMtu;
            pRxCh->chInfo.notifyCb = pRxChPrms->notifyCb;
            pRxCh->chInfo.hCbArg = pRxChPrms->cbArg;

            retVal = EnetCpdma_netChOpen(pRxCh->hEnetDma, &pRxCh->chInfo);

            if( ENET_SOK != retVal)
            {
                EnetCpdma_memMgrFreeRxChObj (pRxCh);
                pRxCh = NULL;
                ENETTRACE_ERR("[Enet CPDMA] NetChOpen(Tx) failed !!: 0x%x\n", retVal);
            }
        }
    }

    if (ENET_SOK == retVal)
    {
        EnetCpdma_DescCh *pRxDescCh = &pRxCh->hEnetDma->rxCppi[pRxCh->chInfo.chNum];
        /* Initialize the Rx queues.
         * Must be done before enabling CQ events */
        EnetQueue_initQ(&pRxDescCh->waitQueue);
        EnetQueue_initQ(&pRxDescCh->descQueue);
        EnetQueue_initQ(&pRxDescCh->freeQueue);
    }

    if (ENET_SOK == retVal)
    {
        pRxCh->initFlag    = true;
    }
    else
    {
        /* Free the rxChObj last */
        if (allocChObj)
        {
            EnetCpdma_memMgrFreeRxChObj(pRxCh);
        }

        /* As flow open is failed return NULL */
        pRxCh = NULL;
    }

    EnetOsal_restoreAllIntr(intrKey);

    return pRxCh;
}

int32_t EnetDma_closeRxCh(EnetDma_RxChHandle hRxCh,
                          EnetDma_PktQ *fq,
                          EnetDma_PktQ *cq)
{
    int32_t retVal = ENET_SOK;
    uint32_t intrKey;

#if ENET_CFG_IS_ON(DEV_ERROR)
    if ((NULL == hRxCh) ||
        (NULL == fq) ||
        (NULL == cq))
    {
        ENETTRACE_ERR_IF((NULL == hRxCh), "[Enet CPDMA] hRxCh is NULL!!\n");
        ENETTRACE_ERR_IF((NULL == fq), "[Enet CPDMA] fq is NULL!!\n");
        ENETTRACE_ERR_IF((NULL == cq), "[Enet CPDMA] cq is NULL!!\n");
        Enet_assert(false);
        retVal = ENET_EBADARGS;
    }
    else
#endif
    {
        intrKey = EnetOsal_disableAllIntr();

        /* Error check */
        if (hRxCh == NULL)
        {
            retVal = ENET_EBADARGS;
        }

        if (ENET_SOK == retVal)
        {
            EnetCpdma_DescCh *pRxDescCh = &hRxCh->hEnetDma->rxCppi[hRxCh->chInfo.chNum];

            /* TODO: trigger the final rx Operation */

            retVal = EnetCpdma_netChClose(hRxCh->hEnetDma, &hRxCh->chInfo);

            if( ENET_SOK != retVal)
            {
                ENETTRACE_ERR("[Enet CPDMA] NetChClose(Rx) failed !!: 0x%x\n", retVal);
            }
            else
            {
                hRxCh->initFlag = false;
                EnetQueue_append(fq, &pRxDescCh->freeQueue);
                EnetQueue_append(cq, &pRxDescCh->waitQueue);
                /* Free Tx channel driver object memory */
                EnetCpdma_memMgrFreeRxChObj(hRxCh);
            }
        }

        EnetOsal_restoreAllIntr(intrKey);
    }
    return retVal;
}

int32_t EnetDma_enableRxEvent(EnetDma_RxChHandle hRxCh)
{
    return ENET_SOK;
}

int32_t EnetDma_disableRxEvent(EnetDma_RxChHandle hRxCh)
{
    return ENET_SOK;
}

void EnetDma_initTxChParams(void *pTxChCfg)
{
    EnetCpdma_OpenTxChPrms *pTxChPrms = (EnetCpdma_OpenTxChPrms *)pTxChCfg;

    pTxChPrms->hEnet = NULL;
    pTxChPrms->chNum = 0U;
    pTxChPrms->numTxPkts = 0U;
    pTxChPrms->notifyCb = NULL;
    pTxChPrms->cbArg   = NULL;
    return;
}

EnetDma_TxChHandle EnetDma_openTxCh(EnetDma_Handle hDma,
                                    const void *pTxChCfg)
{
    int32_t retVal         = ENET_SOK;
    EnetCpdma_TxChObj *pTxCh = NULL;
    uint32_t intrKey;
    EnetCpdma_OpenTxChPrms *pTxChPrms = (EnetCpdma_OpenTxChPrms *)pTxChCfg;

    intrKey = EnetOsal_disableAllIntr();

    /* Set to NULL so if error condition we return NULL */
    pTxCh = NULL;

    /* Error check */
    retVal = EnetCpdma_checkTxChParams(pTxChPrms);

    if (ENET_SOK == retVal)
    {
        /* allocate Tx channel object */
        pTxCh = EnetCpdma_memMgrAllocTxChObj();
        if (pTxCh == NULL)
        {
            ENETTRACE_ERR("[Enet CPDMA] Memory allocation for TX channel object failed !!\n");
            retVal = ENET_EALLOC;
        }
        else
        {
            memset(pTxCh, 0U, sizeof(*pTxCh));
            /* Save channel config */
            pTxCh->txChPrms = *pTxChPrms;
            pTxCh->hEnetDma = Cpsw_getDmaHandle(pTxChPrms->hEnet);
            /* TODO range check and derive channel if any */
            pTxCh->chInfo.chNum = pTxChPrms->chNum;
            pTxCh->chInfo.numBD = pTxChPrms->numTxPkts;
            pTxCh->chInfo.chDir = ENET_CPDMA_DIR_TX;
            //pTxCh->chInfo.bufSize = 0U;
            pTxCh->chInfo.notifyCb = pTxChPrms->notifyCb;
            pTxCh->chInfo.hCbArg = pTxChPrms->cbArg;
            retVal = EnetCpdma_netChOpen(pTxCh->hEnetDma, &pTxCh->chInfo);

            if( ENET_SOK != retVal)
            {
                EnetCpdma_memMgrFreeTxChObj (pTxCh);
                pTxCh = NULL;
                ENETTRACE_ERR("[Enet CPDMA] NetChOpen(Tx) failed !!: 0x%x\n", retVal);
            }
            else
            {
                EnetCpdma_DescCh *pTxDescCh = &pTxCh->hEnetDma->txCppi[pTxCh->chInfo.chNum];
                /* Initialize the Tx queues.
                * Must be done before enabling CQ events */
                EnetQueue_initQ(&pTxDescCh->waitQueue);
                EnetQueue_initQ(&pTxDescCh->descQueue);
                EnetQueue_initQ(&pTxDescCh->freeQueue);
                pTxCh->initFlag = true;
            }
        }
    }

    EnetOsal_restoreAllIntr(intrKey);

    return pTxCh;

}

int32_t EnetDma_closeTxCh(EnetDma_TxChHandle hTxCh,
                          EnetDma_PktQ *fq,
                          EnetDma_PktQ *cq)
{
    int32_t retVal = ENET_SOK;
    uint32_t intrKey;

#if ENET_CFG_IS_ON(DEV_ERROR)
    if ((NULL == hTxCh) ||
        (NULL == fq) ||
        (NULL == cq))
    {
        ENETTRACE_ERR_IF((NULL == hTxCh), "[Enet CPDMA] hTxCh is NULL!!\n");
        ENETTRACE_ERR_IF((NULL == fq), "[Enet CPDMA] fq is NULL!!\n");
        ENETTRACE_ERR_IF((NULL == cq), "[Enet CPDMA] cq is NULL!!\n");
        Enet_assert(false);
        retVal = ENET_EBADARGS;
    }
    else
#endif
    {
        /* TODO: intrrupt should be enabled for teradown  */
        intrKey = EnetOsal_disableAllIntr();

        /* Error check */
        if (hTxCh == NULL)
        {
            retVal = ENET_EBADARGS;
        }

        if (ENET_SOK == retVal)
        {
            if (!hTxCh->initFlag)
            {
                retVal = ENET_EFAIL;
            }
        }

        if (ENET_SOK == retVal)
        {
            retVal = EnetCpdma_netChClose(hTxCh->hEnetDma, &hTxCh->chInfo);

            if( ENET_SOK != retVal)
            {
                ENETTRACE_ERR("[Enet CPDMA] NetChClose(Tx) failed !!: 0x%x\n", retVal);
            }
            else
            {
                /* TODO: range check: Assert */
                EnetCpdma_DescCh *pTxDescCh = &hTxCh->hEnetDma->txCppi[hTxCh->chInfo.chNum];
                hTxCh->initFlag = false;
                EnetQueue_append(fq, &pTxDescCh->waitQueue);
                EnetQueue_append(cq, &pTxDescCh->freeQueue);
                EnetCpdma_memMgrFreeTxChObj(hTxCh);
            }
        }

        EnetOsal_restoreAllIntr(intrKey);
    }

    return retVal;
}

int32_t EnetDma_enableTxEvent(EnetDma_TxChHandle hTxCh)
{
    return ENET_SOK;
}

int32_t EnetDma_disableTxEvent(EnetDma_TxChHandle hTxCh)
{
    return ENET_SOK;
}

int32_t EnetDma_retrieveRxPktQ(EnetDma_RxChHandle hRxCh,
                               EnetDma_PktQ *pRetrieveQ)
{
    int32_t retVal = ENET_SOK;
    EnetCpdma_DescCh *pDescCh;
    uint32_t key;

#if ENET_CFG_IS_ON(DEV_ERROR)
    if ((NULL == hRxCh) ||
        (NULL == pRetrieveQ))
    {
        ENETTRACE_ERR_IF((NULL == hRxCh), "[Enet CPDMA] hRxCh is NULL!!\n");
        ENETTRACE_ERR_IF((NULL == pRetrieveQ), "[Enet CPDMA] pRetrieveQ is NULL!!\n");
        Enet_assert(false);
        retVal = ENET_EBADARGS;
    }
    else
#endif
    {
#if defined(ENETDMA_INSTRUMENTATION_ENABLED)
        uint32_t startTime, diffTime;
        uint32_t pktCnt;
        startTime = EnetOsal_timerRead();
#endif

        pDescCh = &hRxCh->hEnetDma->rxCppi[hRxCh->chInfo.chNum];
        EnetQueue_initQ(pRetrieveQ);

        if (ENET_SOK == retVal)
        {
            key = EnetOsal_disableAllIntr();
            //EnetCpdma_dequeueRx(pDescCh, NULL);
            EnetQueue_append(pRetrieveQ, &pDescCh->waitQueue);
            EnetQueue_initQ(&pDescCh->waitQueue);
            EnetOsal_restoreAllIntr(key);
        }

#if defined(ENETDMA_INSTRUMENTATION_ENABLED)
        pktCnt = EnetQueue_getQCount(pRetrieveQ);
        EnetCpdmaStats_addCnt(&hRxCh->stats.rxRetrievePktDeq, pktCnt);
        diffTime = EnetOsal_timerGetDiff(startTime);
        EnetCpdmaStats_updateNotifyStats(&hRxCh->stats.retrievePktStats, pktCnt, diffTime);
#endif
    }

    return retVal;
}

int32_t EnetDma_retrieveRxPkt(EnetDma_RxChHandle hRxCh,
                              EnetDma_Pkt **ppPkt)
{
    int32_t retVal = ENET_SOK;
    EnetCpdma_DescCh *pDescCh;
    uint32_t key;

#if ENET_CFG_IS_ON(DEV_ERROR)
    if ((NULL == hRxCh) ||
        (NULL == ppPkt))
    {
        ENETTRACE_ERR_IF((NULL == hRxCh), "[Enet CPDMA] hRxCh is NULL!!\n");
        ENETTRACE_ERR_IF((NULL == ppPkt), "[Enet CPDMA] ppPkt is NULL!!\n");
        Enet_assert(false);
        retVal = ENET_EBADARGS;
    }
    else
#endif
    {
#if defined(ENETDMA_INSTRUMENTATION_ENABLED)
        uint32_t startTime, diffTime;
        startTime = EnetOsal_timerRead();
#endif

        pDescCh = &hRxCh->hEnetDma->rxCppi[hRxCh->chInfo.chNum];

        if (ENET_SOK == retVal)
        {
            key = EnetOsal_disableAllIntr();
            //EnetCpdma_dequeueRx(pDescCh, NULL);
            *ppPkt = (EnetCpdma_PktInfo *)EnetQueue_deq(&pDescCh->waitQueue);
            EnetOsal_restoreAllIntr(key);
        }

#if defined(ENETDMA_INSTRUMENTATION_ENABLED)
        if(ppPkt != NULL)
        {
            EnetCpdmaStats_addCnt(&hRxCh->stats.rxRetrievePktDeq, 1U);
            diffTime = EnetOsal_timerGetDiff(startTime);
            EnetCpdmaStats_updateNotifyStats(&hRxCh->stats.retrievePktStats, 1U, diffTime);
        }
#endif
    }

    return retVal;
}

int32_t EnetDma_submitRxPktQ(EnetDma_RxChHandle hRxCh,
                             EnetDma_PktQ *pSubmitQ)
{
    int32_t retVal = ENET_SOK;
    EnetCpdma_DescCh *pDescCh;

#if ENET_CFG_IS_ON(DEV_ERROR)
    if ((NULL == hRxCh) ||
        (NULL == pSubmitQ))
    {
        ENETTRACE_ERR_IF((NULL == hRxCh), "[Enet CPDMA] hRxCh is NULL!!\n");
        ENETTRACE_ERR_IF((NULL == pSubmitQ), "[Enet CPDMA] pSubmitQ is NULL!!\n");
        Enet_assert(false);
        retVal = ENET_EBADARGS;
    }
    else
#endif
    {
#if defined(ENETDMA_INSTRUMENTATION_ENABLED)
        uint32_t startTime, diffTime;
        uint32_t pktCnt;
        startTime = EnetOsal_timerRead();
        pktCnt    = EnetQueue_getQCount(pSubmitQ);
#endif

        pDescCh = &hRxCh->hEnetDma->rxCppi[hRxCh->chInfo.chNum];

        /* Enqueue descs to fqRing regardless of caller's queue state */
        if (EnetQueue_getQCount(pSubmitQ) > 0U)
        {
            EnetQueue_append(&pDescCh->freeQueue, pSubmitQ);
            EnetQueue_initQ(pSubmitQ);
            EnetCpdma_enqueueRx(pDescCh, false);
        }

#if defined(ENETDMA_INSTRUMENTATION_ENABLED)
        EnetCpdmaStats_addCnt(&hRxCh->stats.rxSubmitPktEnq, pktCnt);
        diffTime = EnetOsal_timerGetDiff(startTime);
        EnetCpdmaStats_updateNotifyStats(&hRxCh->stats.submitPktStats, pktCnt, diffTime);
#endif
    }

    return retVal;
}

int32_t EnetDma_submitRxPkt(EnetDma_RxChHandle hRxCh,
                            EnetDma_Pkt *pPkt)
{
    int32_t retVal = ENET_SOK;
    EnetCpdma_DescCh *pDescCh;

#if ENET_CFG_IS_ON(DEV_ERROR)
    if ((NULL == hRxCh) ||
        (NULL == pPkt))
    {
        ENETTRACE_ERR_IF((NULL == hRxCh), "[Enet CPDMA] hRxCh is NULL!!\n");
        ENETTRACE_ERR_IF((NULL == pPkt), "[Enet CPDMA] pPkt is NULL!!\n");
        Enet_assert(false);
        retVal = ENET_EBADARGS;
    }
    else
#endif
    {
#if defined(ENETDMA_INSTRUMENTATION_ENABLED)
        uint32_t startTime, diffTime;
        startTime = EnetOsal_timerRead();
#endif

        pDescCh = &hRxCh->hEnetDma->rxCppi[hRxCh->chInfo.chNum];

        /* Enqueue descs to fqRing regardless of caller's queue state */
        EnetQueue_enq(&pDescCh->freeQueue, &pPkt->node);
        EnetCpdma_enqueueRx(pDescCh, false);

#if defined(ENETDMA_INSTRUMENTATION_ENABLED)
        EnetCpdmaStats_addCnt(&hRxCh->stats.rxSubmitPktEnq, 1U);
        diffTime = EnetOsal_timerGetDiff(startTime);
        EnetCpdmaStats_updateNotifyStats(&hRxCh->stats.submitPktStats, 1U, diffTime);
#endif
    }

    return retVal;
}

int32_t EnetDma_retrieveTxPktQ(EnetDma_TxChHandle hTxCh,
                                   EnetDma_PktQ *pRetrieveQ)
{
    int32_t retVal = ENET_SOK;
    EnetCpdma_DescCh *pDescCh;
    uint32_t key;

#if ENET_CFG_IS_ON(DEV_ERROR)
    if ((NULL == hTxCh) ||
        (NULL == pRetrieveQ))
    {
        ENETTRACE_ERR_IF((NULL == hTxCh), "[Enet CPDMA] hTxCh is NULL!!\n");
        ENETTRACE_ERR_IF((NULL == pRetrieveQ), "[Enet CPDMA] pRetrieveQ is NULL!!\n");
        Enet_assert(false);
        retVal = ENET_EBADARGS;
    }
    else
#endif
    {
#if defined(ENETDMA_INSTRUMENTATION_ENABLED)
        uint32_t startTime, diffTime;
        uint32_t pktCnt;
        startTime = EnetOsal_timerRead();
#endif

        pDescCh = &hTxCh->hEnetDma->txCppi[hTxCh->chInfo.chNum];

        EnetQueue_initQ(pRetrieveQ);
        key = EnetOsal_disableAllIntr();
        EnetCpdma_dequeueTx(pDescCh, NULL);
        EnetQueue_append(pRetrieveQ, &pDescCh->freeQueue);
        EnetQueue_initQ(&pDescCh->freeQueue);
        EnetOsal_restoreAllIntr(key);

#if defined(ENETDMA_INSTRUMENTATION_ENABLED)
        pktCnt = EnetQueue_getQCount(pRetrieveQ);
        EnetCpdmaStats_addCnt(&hTxCh->stats.txRetrievePktDeq, pktCnt);
        diffTime = EnetOsal_timerGetDiff(startTime);
        EnetCpdmaStats_updateNotifyStats(&hTxCh->stats.retrievePktStats, pktCnt, diffTime);
#endif
    }

    return retVal;
}

int32_t EnetDma_retrieveTxPkt(EnetDma_TxChHandle hTxCh,
                              EnetDma_Pkt **ppPkt)
{
    int32_t retVal = ENET_SOK;
    EnetCpdma_DescCh *pDescCh;
    uint32_t key;

#if ENET_CFG_IS_ON(DEV_ERROR)
    if ((NULL == hTxCh) ||
        (NULL == ppPkt))
    {
        ENETTRACE_ERR_IF((NULL == hTxCh), "[Enet CPDMA] hTxCh is NULL!!\n");
        ENETTRACE_ERR_IF((NULL == ppPkt), "[Enet CPDMA] ppPkt is NULL!!\n");
        Enet_assert(false);
        retVal = ENET_EBADARGS;
    }
    else
#endif
    {
#if defined(ENETDMA_INSTRUMENTATION_ENABLED)
        uint32_t startTime, diffTime;
        startTime = EnetOsal_timerRead();
#endif

        pDescCh = &hTxCh->hEnetDma->txCppi[hTxCh->chInfo.chNum];

        key = EnetOsal_disableAllIntr();
        EnetCpdma_dequeueTx(pDescCh, NULL);
        *ppPkt = (EnetDma_Pkt *)EnetQueue_deq(&pDescCh->freeQueue);
        EnetOsal_restoreAllIntr(key);

#if defined(ENETDMA_INSTRUMENTATION_ENABLED)
        if(NULL != *ppPkt)
        {
            EnetCpdmaStats_addCnt(&hTxCh->stats.txRetrievePktDeq, 1U);
            diffTime = EnetOsal_timerGetDiff(startTime);
            EnetCpdmaStats_updateNotifyStats(&hTxCh->stats.retrievePktStats, 1U, diffTime);
        }
#endif
    }

    return retVal;
}

int32_t EnetDma_submitTxPktQ(EnetDma_TxChHandle hTxCh,
                             EnetDma_PktQ *pSubmitQ)
{
    int32_t retVal = ENET_SOK;
    EnetCpdma_DescCh *pDescCh;
    uint32_t key;

#if ENET_CFG_IS_ON(DEV_ERROR)
    if ((NULL == hTxCh) ||
        (NULL == pSubmitQ))
    {
        ENETTRACE_ERR_IF((NULL == hTxCh), "[Enet CPDMA] hTxCh is NULL!!\n");
        ENETTRACE_ERR_IF((NULL == pSubmitQ), "[Enet CPDMA] pSubmitQ is NULL!!\n");
        Enet_assert(false);
        retVal = ENET_EBADARGS;
    }
    else
#endif
    {
#if defined(ENETDMA_INSTRUMENTATION_ENABLED)
        uint32_t startTime, diffTime;
        uint32_t pktCnt;
        startTime = EnetOsal_timerRead();
        pktCnt    = EnetQueue_getQCount(pSubmitQ);
#endif

        /* Enqueue descs to fqRing regardless of caller's queue state */
        if (EnetQueue_getQCount(pSubmitQ) > 0U)
        {
            volatile uint32_t dmaInProgress;

            pDescCh = &hTxCh->hEnetDma->txCppi[hTxCh->chInfo.chNum];
            dmaInProgress = pDescCh->dmaInProgress;
            /* Horrible hack due to way the Trasnmit is implemented which does not support queing */
            while(dmaInProgress)
            {
                dmaInProgress = pDescCh->dmaInProgress;
            }
            key = EnetOsal_disableAllIntr();
            EnetQueue_append(&pDescCh->waitQueue, pSubmitQ);
            EnetQueue_initQ(pSubmitQ);
            EnetCpdma_enqueueTx(pDescCh);
            EnetOsal_restoreAllIntr(key);
        }

#if defined(ENETDMA_INSTRUMENTATION_ENABLED)
        EnetCpdmaStats_addCnt(&hTxCh->stats.txSubmitPktEnq, pktCnt);
        diffTime = EnetOsal_timerGetDiff(startTime);
        EnetCpdmaStats_updateNotifyStats(&hTxCh->stats.submitPktStats, pktCnt, diffTime);
#endif
    }

    return retVal;
}

int32_t EnetDma_submitTxPkt(EnetDma_TxChHandle hTxCh,
                                 EnetDma_Pkt *pPkt)
{
    int32_t retVal = ENET_SOK;
    EnetCpdma_DescCh *pDescCh;
    uint32_t key;

#if ENET_CFG_IS_ON(DEV_ERROR)
    if ((NULL == hTxCh) ||
        (NULL == pPkt))
    {
        ENETTRACE_ERR_IF((NULL == hTxCh), "[Enet CPDMA] hTxCh is NULL!!\n");
        ENETTRACE_ERR_IF((NULL == pPkt), "[Enet CPDMA] pPkt is NULL!!\n");
        Enet_assert(false);
        retVal = ENET_EBADARGS;
    }
    else
#endif
    {
#if defined(ENETDMA_INSTRUMENTATION_ENABLED)
        uint32_t startTime, diffTime;
        startTime = EnetOsal_timerRead();
#endif

        /* Enqueue descs to fqRing regardless of caller's queue state */
        pDescCh = &hTxCh->hEnetDma->txCppi[hTxCh->chInfo.chNum];
        key = EnetOsal_disableAllIntr();
        EnetQueue_enq(&pDescCh->waitQueue, &pPkt->node);
        EnetCpdma_enqueueTx(pDescCh);
        EnetOsal_restoreAllIntr(key);

#if defined(ENETDMA_INSTRUMENTATION_ENABLED)
        EnetCpdmaStats_addCnt(&hTxCh->stats.txSubmitPktEnq, 1U);
        diffTime = EnetOsal_timerGetDiff(startTime);
        EnetCpdmaStats_updateNotifyStats(&hTxCh->stats.submitPktStats, 1U, diffTime);
#endif
    }

    return retVal;
}

void EnetDma_initPktInfo(EnetDma_Pkt *pktInfo)
{
    memset(&pktInfo->node, 0U, sizeof(pktInfo->node));
    pktInfo->bufPtr                = NULL;
    pktInfo->orgBufLen             = 0U;
    pktInfo->userBufLen            = 0U;
    pktInfo->appPriv               = NULL;
    pktInfo->tsInfo.enableHostTxTs = false;
    pktInfo->txPortNum             = ENET_MAC_PORT_INV;
    ENET_UTILS_SET_PKT_DRIVER_STATE(&pktInfo->pktState,
                                    (uint32_t)ENET_PKTSTATE_DMA_NOT_WITH_HW);
    return;
}

int32_t EnetDma_getRxChStats(EnetDma_RxChHandle hRxCh,
                             EnetDma_RxChStats *pStats)
{
    int32_t retVal = ENET_ENOTSUPPORTED;

#if defined(ENETDMA_INSTRUMENTATION_ENABLED)
    *pStats = hRxCh->stats;
    retVal = ENET_SOK;
#endif

    return retVal;
}

int32_t EnetDma_getTxChStats(EnetDma_TxChHandle hTxCh,
                             EnetDma_TxChStats *pStats)
{
    int32_t retVal = ENET_ENOTSUPPORTED;

#if defined(ENETDMA_INSTRUMENTATION_ENABLED)
    *pStats = hTxCh->stats;
    retVal = ENET_SOK;
#endif

    return retVal;
}

int32_t EnetDma_resetRxChStats(EnetDma_RxChHandle hRxCh)
{
    int32_t retVal = ENET_ENOTSUPPORTED;

#if defined(ENETDMA_INSTRUMENTATION_ENABLED)
    memset(&hRxCh->stats, 0, sizeof(hRxCh->stats));
    retVal = ENET_SOK;
#endif

    return retVal;
}

int32_t EnetDma_resetTxChStats(EnetDma_TxChHandle hTxCh)
{
    int32_t retVal = ENET_ENOTSUPPORTED;

#if defined(ENETDMA_INSTRUMENTATION_ENABLED)
    memset(&hTxCh->stats, 0, sizeof(hTxCh->stats));
    retVal = ENET_SOK;
#endif

    return retVal;
}

#if 0 /* TODO */

void EnetCpdma_initDataPathParams(EnetDma_initCfg *pDmaConfig)
{
    return;
}

EnetDma_Handle EnetCpdma_initDataPath(Enet_Type enetType,
                                      uint32_t instId,
                                      const EnetDma_initCfg *pDmaConfig)
{
    EnetCpdma_Cfg cfg;
    EnetDma_Handle hDmaHandle;

    EnetDma_initCfg(enetType, &cfg);
    hDmaHandle   = EnetDma_open(enetType, instId, &cfg);
    return hDmaHandle;
}

int32_t EnetCpdma_deInitDataPath(EnetDma_Handle hEnetDma)
{
    int32_t status;

    status = EnetDma_close(hEnetDma);
    return status;
}

#endif

/* End of file */
