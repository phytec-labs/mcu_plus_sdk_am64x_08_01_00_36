/*
 *  Copyright (C) 2021 Texas Instruments Incorporated
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

/**
 *  \file mcspi_v0.c
 *
 *  \brief File containing MCSPI Driver APIs implementation for version V0.
 *
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

/* This is needed for memset/memcpy */
#include <string.h>
#include <drivers/mcspi.h>
#include <kernel/dpl/SemaphoreP.h>
#include <kernel/dpl/HwiP.h>
#include <kernel/dpl/AddrTranslateP.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

typedef struct
{
    void                   *lock;
    /**< Driver lock - to protect across open/close */
    SemaphoreP_Object       lockObj;
    /**< Driver lock object */
} MCSPI_DrvObj;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/* Driver internal functions */
static void MCSPI_masterIsr(void *args);
static void MCSPI_initiateLastChunkTransfer(MCSPI_Object *obj,
                                            MCSPI_ChObject *chObj,
                                            MCSPI_Transaction *transaction);
static uint32_t MCSPI_continueTxRx(MCSPI_Object *obj,
                                   MCSPI_ChObject *chObj,
                                   MCSPI_Transaction *transaction);
static int32_t MCSPI_transferMasterPoll(MCSPI_Object *obj,
                                        MCSPI_ChObject *chObj,
                                        const MCSPI_Attrs *attrs,
                                        MCSPI_Transaction *transaction);
static int32_t MCSPI_transferMasterIntr(MCSPI_Object *obj,
                                        MCSPI_ChObject *chObj,
                                        const MCSPI_Attrs *attrs,
                                        MCSPI_Transaction *transaction);
static void MCSPI_slaveIsr(void *args);
static uint32_t MCSPI_continueSlaveTxRx(MCSPI_Object *obj,
                                        MCSPI_ChObject *chObj,
                                        MCSPI_Transaction *transaction);
static int32_t MCSPI_transferSlavePoll(MCSPI_Object *obj,
                                       MCSPI_ChObject *chObj,
                                       const MCSPI_Attrs *attrs,
                                       MCSPI_Transaction *transaction);
static int32_t MCSPI_transferSlaveIntr(MCSPI_Object *obj,
                                        MCSPI_ChObject *chObj,
                                        const MCSPI_Attrs *attrs,
                                        MCSPI_Transaction *transaction);
static void MCSPI_fifoWrite(uint32_t baseAddr, MCSPI_ChObject *chObj, uint32_t transferLength);
static void MCSPI_fifoRead(uint32_t baseAddr, MCSPI_ChObject *chObj, uint32_t transferLength);
static void MCSPI_configInstance(MCSPI_Config *config);
static void MCSPI_setChConfig(MCSPI_Config *config,
                              MCSPI_ChObject *chObj);
static int32_t MCSPI_checkOpenParams(const MCSPI_OpenParams *openPrms);
static int32_t MCSPI_checkChConfig(MCSPI_Object   *obj, const MCSPI_ChConfig *chCfg);
static int32_t MCSPI_checkTransaction(const MCSPI_Object *obj,
                                      MCSPI_Transaction *transaction);
static uint32_t MCSPI_getDataWidthBitMask(uint32_t dataWidth);

/* Low level HW functions */
static void MCSPI_reset(uint32_t baseAddr);
static void MCSPI_setClkConfig(uint32_t baseAddr,
                               uint32_t chNum,
                               uint32_t inputClkFreq,
                               uint32_t bitRate);
static void MCSPI_setFifoConfig(const MCSPI_ChObject *chObj,
                                uint32_t baseAddr,
                                uint32_t numWordsTxRx);
static void MCSPI_setSlaveFifoConfig(MCSPI_ChObject *chObj,
                                     uint32_t baseAddr,
                                     uint32_t numWordsTxRx);
static const uint8_t *MCSPI_fifoWrite8(uint32_t        baseAddr,
                                       uint32_t        chNum,
                                       const uint8_t  *bufPtr,
                                       uint32_t        transferLength);
static const uint16_t *MCSPI_fifoWrite16(uint32_t        baseAddr,
                                         uint32_t        chNum,
                                         const uint16_t *bufPtr,
                                         uint32_t        transferLength);
static const uint32_t *MCSPI_fifoWrite32(uint32_t        baseAddr,
                                         uint32_t        chNum,
                                         const uint32_t *bufPtr,
                                         uint32_t        transferLength);
static uint8_t *MCSPI_fifoRead8(uint32_t  baseAddr,
                                uint32_t  chNum,
                                uint8_t  *bufPtr,
                                uint32_t  transferLength,
                                uint32_t  dataWidthBitMask);
static uint16_t *MCSPI_fifoRead16(uint32_t  baseAddr,
                                  uint32_t  chNum,
                                  uint16_t  *bufPtr,
                                  uint32_t  transferLength,
                                  uint32_t  dataWidthBitMask);
static uint32_t *MCSPI_fifoRead32(uint32_t  baseAddr,
                                  uint32_t  chNum,
                                  uint32_t  *bufPtr,
                                  uint32_t  transferLength,
                                  uint32_t  dataWidthBitMask);
static void MCSPI_fifoWriteDefault(uint32_t baseAddr,
                                   uint32_t chNum,
                                   uint32_t defaultTxData,
                                   uint32_t transferLength);
static void MCSPI_fifoReadDiscard(uint32_t baseAddr,
                                  uint32_t chNum,
                                  uint32_t transferLength);
static void MCSPI_clearAllIrqStatus(uint32_t baseAddr);
static inline void MCSPI_intrStatusClear(uint32_t baseAddr, uint32_t intFlags);
static uint32_t Spi_mcspiGetRxMask(uint32_t csNum);
static uint32_t Spi_mcspiGetTxMask(uint32_t csNum);
static void MCSPI_stop(MCSPI_Object *obj, const MCSPI_Attrs *attrs,
                       MCSPI_ChObject *chObj, uint32_t chNum);
/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/** \brief Driver object */
static MCSPI_DrvObj     gMcspiDrvObj =
{
    .lock           = NULL,
};

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

void MCSPI_init(void)
{
    int32_t         status;
    uint32_t        cnt;
    MCSPI_Object   *obj;

    /* Init each driver instance object */
    for (cnt = 0U; cnt < gMcspiConfigNum; cnt++)
    {
        /* initialize object varibles */
        obj = gMcspiConfig[cnt].object;
        DebugP_assert(NULL != obj);
        memset(obj, 0, sizeof(MCSPI_Object));
    }

    /* Create driver lock */
    status = SemaphoreP_constructMutex(&gMcspiDrvObj.lockObj);
    if(SystemP_SUCCESS == status)
    {
        gMcspiDrvObj.lock = &gMcspiDrvObj.lockObj;
    }

    return;
}

void MCSPI_deinit(void)
{
    /* Delete driver lock */
    if(NULL != gMcspiDrvObj.lock)
    {
        SemaphoreP_destruct(&gMcspiDrvObj.lockObj);
        gMcspiDrvObj.lock = NULL;
    }

    return;
}

MCSPI_Handle MCSPI_open(uint32_t index, const MCSPI_OpenParams *openPrms)
{
    int32_t              status = SystemP_SUCCESS;
    MCSPI_Handle         handle = NULL;
    MCSPI_Config        *config = NULL;
    MCSPI_Object        *obj    = NULL;
    HwiP_Params          hwiPrms;
    const MCSPI_Attrs   *attrs;

    /* Check index */
    if(index >= gMcspiConfigNum)
    {
        status = SystemP_FAILURE;
    }
    else
    {
        config = &gMcspiConfig[index];
    }

    DebugP_assert(NULL != gMcspiDrvObj.lock);
    SemaphoreP_pend(&gMcspiDrvObj.lockObj, SystemP_WAIT_FOREVER);

    if(SystemP_SUCCESS == status)
    {
        obj = config->object;
        DebugP_assert(NULL != obj);
        DebugP_assert(NULL != config->attrs);
        attrs = config->attrs;
        if(TRUE == obj->isOpen)
        {
            /* Handle already opended */
            status = SystemP_FAILURE;
        }
    }

    if(SystemP_SUCCESS == status)
    {
        /* Init state */
        obj->handle = (MCSPI_Handle) config;
        memset(&obj->chObj, 0, sizeof(obj->chObj));
        obj->baseAddr = (uint32_t) AddrTranslateP_getLocalAddr(attrs->baseAddr);
        if(NULL != openPrms)
        {
            memcpy(&obj->openPrms, openPrms, sizeof(MCSPI_OpenParams));
        }
        else
        {
            /* Init with default if NULL is passed */
            MCSPI_OpenParams_init(&obj->openPrms);
        }

        /* Check open parameters */
        status = MCSPI_checkOpenParams(&obj->openPrms);
    }

    if(SystemP_SUCCESS == status)
    {
        /* Configure the MCSPI instance parameters */
        MCSPI_configInstance(config);

        /* Create transfer sync semaphore */
        status += SemaphoreP_constructBinary(&obj->transferSemObj, 0U);
        if(SystemP_SUCCESS == status)
        {
            obj->transferSem = &obj->transferSemObj;
        }

        /* Register interrupt */
        if(TRUE == attrs->intrEnable)
        {
            HwiP_Params_init(&hwiPrms);
            hwiPrms.intNum      = attrs->intrNum;
            hwiPrms.priority    = attrs->intrPriority;
            if(MCSPI_MS_MODE_MASTER == obj->openPrms.msMode)
            {
                hwiPrms.callback    = &MCSPI_masterIsr;
            }
            else
            {
                hwiPrms.callback    = &MCSPI_slaveIsr;
            }

            hwiPrms.priority    = attrs->intrPriority;
            hwiPrms.args        = (void *) config;
            status += HwiP_construct(&obj->hwiObj, &hwiPrms);
            if(SystemP_SUCCESS == status)
            {
                obj->hwiHandle = &obj->hwiObj;
            }
        }
    }

    if(SystemP_SUCCESS == status)
    {
        obj->isOpen = TRUE;
        handle = (MCSPI_Handle) config;
    }

    SemaphoreP_post(&gMcspiDrvObj.lockObj);

    /* Free-up resources in case of error */
    if(SystemP_SUCCESS != status)
    {
        if(NULL != config)
        {
            MCSPI_close((MCSPI_Handle) config);
        }
    }

    return (handle);
}

void MCSPI_close(MCSPI_Handle handle)
{
    MCSPI_Config   *config;
    MCSPI_Object   *obj;
    uint32_t        baseAddr;

    if(NULL != handle)
    {
        config = (MCSPI_Config *) handle;
        obj = config->object;
        DebugP_assert(NULL != obj);

        DebugP_assert(NULL != gMcspiDrvObj.lock);
        SemaphoreP_pend(&gMcspiDrvObj.lockObj, SystemP_WAIT_FOREVER);

        baseAddr = config->object->baseAddr;
        /* Reset MCSPI */
        MCSPI_reset(baseAddr);

        if(NULL != obj->transferSem)
        {
            SemaphoreP_destruct(&obj->transferSemObj);
            obj->transferSem = NULL;
        }
        if(NULL != obj->hwiHandle)
        {
            HwiP_destruct(&obj->hwiObj);
            obj->hwiHandle = NULL;
        }

        obj->isOpen = FALSE;
        SemaphoreP_post(&gMcspiDrvObj.lockObj);
    }

    return;
}

int32_t MCSPI_chConfig(MCSPI_Handle handle,
                       const MCSPI_ChConfig *chCfg)
{
    int32_t         status = SystemP_SUCCESS;
    MCSPI_Config   *config;
    MCSPI_Object   *obj;
    MCSPI_ChObject  *chObj;

    /* Check parameters */
    if((NULL == handle) ||
       (NULL == chCfg) ||
       (chCfg->chNum >= MCSPI_MAX_NUM_CHANNELS))
    {
        status = SystemP_FAILURE;
    }

    if(SystemP_SUCCESS == status)
    {
        config = (MCSPI_Config *) handle;
        obj = config->object;
        DebugP_assert(NULL != obj);
        /* Check channel parameters */
        status = MCSPI_checkChConfig(obj, chCfg);
    }

    if(SystemP_SUCCESS == status)
    {
        chObj = &obj->chObj[chCfg->chNum];
        memcpy(&chObj->chCfg, chCfg, sizeof(MCSPI_ChConfig));
    }

    if(SystemP_SUCCESS == status)
    {
        /* Configure the MCSPI channel */
        MCSPI_setChConfig(config, chObj);
        chObj->isOpen = TRUE;
    }

    return (status);
}

int32_t MCSPI_transfer(MCSPI_Handle handle, MCSPI_Transaction *transaction)
{
    int32_t             status = SystemP_SUCCESS, semStatus;
    uint32_t            baseAddr, chNum;
    MCSPI_Config       *config;
    MCSPI_Object       *obj;
    const MCSPI_Attrs  *attrs;
    MCSPI_ChObject     *chObj;

    /* Check parameters */
    if((NULL == handle) ||
       (NULL == transaction))
    {
        status = SystemP_FAILURE;
    }

    if(SystemP_SUCCESS == status)
    {
        config = (MCSPI_Config *) handle;
        obj = config->object;
        DebugP_assert(NULL != obj);
        DebugP_assert(NULL != config->attrs);
        attrs = config->attrs;
        baseAddr = obj->baseAddr;

        status = MCSPI_checkTransaction(obj, transaction);
    }

    if(SystemP_SUCCESS == status)
    {
        uintptr_t key;
        key = HwiP_disable();

        /* Check if any transaction is in progress */
        if(NULL != obj->currTransaction)
        {
            transaction->status = MCSPI_TRANSFER_CANCELLED;
            status = SystemP_FAILURE;
        }
        else
        {
            /* Start transfer */
            obj->currTransaction = transaction;
        }
        HwiP_restore(key);

        if (SystemP_SUCCESS == status)
        {
            /* Reset counter and other params */
            chNum = transaction->channel;
            chObj = &obj->chObj[chNum];
            chObj->curTxBufPtr = transaction->txBuf;
            chObj->curRxBufPtr = transaction->rxBuf;
            chObj->curTxWords  = 0U;
            chObj->curRxWords  = 0U;
            transaction->status = MCSPI_TRANSFER_STARTED;

            if(MCSPI_MS_MODE_MASTER == obj->openPrms.msMode)
            {
                /*
                * Common configuration
                */
                MCSPI_setFifoConfig(chObj, baseAddr, transaction->count);
                MCSPI_intrStatusClear(baseAddr, chObj->intrMask);

                if(TRUE == attrs->intrEnable)
                {
                    status = MCSPI_transferMasterIntr(obj, chObj, attrs, transaction);
                    if (obj->openPrms.transferMode == MCSPI_TRANSFER_MODE_BLOCKING)
                    {
                        /* Block on transferSem till the transfer completion. */
                        DebugP_assert(NULL != obj->transferSem);
                        semStatus = SemaphoreP_pend(&obj->transferSemObj, obj->openPrms.transferTimeout);
                        if (semStatus == SystemP_SUCCESS)
                        {
                            status = SystemP_SUCCESS;
                        }
                        else
                        {
                            /* Stop MCSPI Channel */
                            MCSPI_stop(obj, attrs, chObj, chNum);
                            status = SystemP_FAILURE;
                            transaction->status = MCSPI_TRANSFER_TIMEOUT;
                            obj->currTransaction = NULL;
                        }
                    }
                }
                else
                {
                    status = MCSPI_transferMasterPoll(obj, chObj, attrs, transaction);
                    /* transfer completed */
                    transaction->status  = MCSPI_TRANSFER_COMPLETED;
                    key = HwiP_disable();
                    obj->currTransaction = NULL;
                    HwiP_restore(key);
                }
            }
            else
            {
                /*
                * Common configuration
                */
                MCSPI_setSlaveFifoConfig(chObj, baseAddr, transaction->count);
                MCSPI_intrStatusClear(baseAddr, chObj->intrMask);

                if (TRUE == attrs->intrEnable)
                {
                    status = MCSPI_transferSlaveIntr(obj, chObj, attrs, transaction);
                    if (obj->openPrms.transferMode == MCSPI_TRANSFER_MODE_BLOCKING)
                    {
                        /* Block on transferSem till the transfer completion. */
                        DebugP_assert(NULL != obj->transferSem);
                        semStatus = SemaphoreP_pend(&obj->transferSemObj, obj->openPrms.transferTimeout);
                        if (semStatus == SystemP_SUCCESS)
                        {
                            status = SystemP_SUCCESS;
                        }
                        else
                        {
                            /* Stop MCSPI Channel */
                            MCSPI_stop(obj, attrs, chObj, chNum);
                            status = SystemP_FAILURE;
                            transaction->status = MCSPI_TRANSFER_TIMEOUT;
                            obj->currTransaction = NULL;
                        }
                    }
                }
                else
                {
                    status = MCSPI_transferSlavePoll(obj, chObj, attrs, transaction);
                    /* transfer completed */
                    transaction->status  = MCSPI_TRANSFER_COMPLETED;
                    key = HwiP_disable();
                    obj->currTransaction = NULL;
                    HwiP_restore(key);
                }
            }
        }
    }

    return (status);
}

int32_t MCSPI_transferCancel(MCSPI_Handle handle)
{
    int32_t             status = SystemP_SUCCESS;
    uint32_t            chNum;
    MCSPI_Config       *config;
    MCSPI_Object       *obj;
    const MCSPI_Attrs  *attrs;
    MCSPI_ChObject     *chObj;

    /* Check parameters */
    if (NULL == handle)
    {
        status = SystemP_FAILURE;
    }

    if(SystemP_SUCCESS == status)
    {
        config = (MCSPI_Config *) handle;
        obj = config->object;
        DebugP_assert(NULL != obj);
        DebugP_assert(NULL != config->attrs);
        attrs = config->attrs;
        if (obj->currTransaction == NULL)
        {
            status = SystemP_FAILURE;
        }
    }

    if(SystemP_SUCCESS == status)
    {
        chNum = obj->currTransaction->channel;
        chObj = &obj->chObj[chNum];
        /* Stop MCSPI Channel */
        MCSPI_stop(obj, attrs, chObj, chNum);
        obj->currTransaction->status  = MCSPI_TRANSFER_CANCELLED;
        /* Return the actual number of words transferred */
        obj->currTransaction->count = chObj->curRxWords;
        if (MCSPI_TR_MODE_TX_ONLY == chObj->chCfg.trMode)
        {
            obj->currTransaction->count = chObj->curTxWords;
        }
        /*
        * Post transfer Sem in case of bloacking transfer.
        * Call the callback function in case of Callback mode.
        */
        if (obj->openPrms.transferMode == MCSPI_TRANSFER_MODE_BLOCKING)
        {
            SemaphoreP_post(&obj->transferSemObj);
        }
        else
        {
            obj->openPrms.transferCallbackFxn((MCSPI_Handle) config, obj->currTransaction);
        }

        obj->currTransaction = NULL;
    }

    return (status);
}

static void MCSPI_masterIsr(void *args)
{
    int32_t             status = SystemP_SUCCESS;
    uint32_t            transferStatus;
    MCSPI_Config       *config;
    MCSPI_Object       *obj;
    const MCSPI_Attrs  *attrs;
    MCSPI_ChObject     *chObj;
    MCSPI_Transaction  *transaction;
    uint32_t            baseAddr, chNum;

    /* Check parameters */
    if(NULL == args)
    {
        status = SystemP_FAILURE;
    }

    if(SystemP_SUCCESS == status)
    {
        config = (MCSPI_Config *) args;
        obj = config->object;
        attrs = config->attrs;
        DebugP_assert(NULL != obj);
        DebugP_assert(NULL != config->attrs);

        transaction = obj->currTransaction;
        baseAddr = obj->baseAddr;
        if (transaction != NULL)
        {
            chNum = transaction->channel;
            chObj = &obj->chObj[chNum];
            transferStatus = MCSPI_continueTxRx(obj, chObj, transaction);
            if (MCSPI_TRANSFER_COMPLETED == transferStatus)
            {
                /* Process the transfer completion. */
                /* Stop MCSPI Channel */
                MCSPI_stop(obj, attrs, chObj, chNum);
                /* Update the driver internal status. */
                /* transfer completed */
                transaction->status  = MCSPI_TRANSFER_COMPLETED;
                /* Return the actual number of words transferred */
                obj->currTransaction->count = chObj->curRxWords;
                if (MCSPI_TR_MODE_TX_ONLY == chObj->chCfg.trMode)
                {
                    obj->currTransaction->count = chObj->curTxWords;
                }
                obj->currTransaction = NULL;

                /*
                * Post transfer Sem in case of blocking transfer.
                * Call the callback function in case of Callback mode.
                */
                if (obj->openPrms.transferMode == MCSPI_TRANSFER_MODE_BLOCKING)
                {
                    SemaphoreP_post(&obj->transferSemObj);
                }
                else
                {
                    obj->openPrms.transferCallbackFxn((MCSPI_Handle) config, transaction);
                }
            }
            /*
            * Else the transfer is still pending.
            * Do nothing, wait for next interrupt.
            */
        }
        else
        {
            /* There is no ongoing transfer. Disable and clear all interrupts. */
            CSL_REG32_WR(baseAddr + CSL_MCSPI_IRQENABLE, 0U);
            MCSPI_clearAllIrqStatus(baseAddr);
        }
    }
    return;
}

static void MCSPI_initiateLastChunkTransfer(MCSPI_Object *obj,
                                            MCSPI_ChObject *chObj,
                                            MCSPI_Transaction *transaction)
{
    uint32_t        baseAddr, chNum;
    uint32_t        reminder;
    uint32_t        regVal;

    baseAddr = obj->baseAddr;
    chNum = chObj->chCfg.chNum;

    /* Disable channel so that new settings takes effect */
    CSL_REG32_FINS(
        baseAddr + MCSPI_CHCTRL(chNum),
        MCSPI_CH0CTRL_EN,
        CSL_MCSPI_CH0CTRL_EN_NACT);

    /* Start transferring only multiple of FIFO trigger level */
    if(MCSPI_TR_MODE_RX_ONLY != chObj->chCfg.trMode)
    {
        reminder = (transaction->count & (chObj->effTxFifoDepth - 1U));
    }
    else
    {
        reminder = (transaction->count & (chObj->effRxFifoDepth - 1U));
    }

    /* Set FIFO trigger level and word count */
    regVal  = CSL_REG32_RD(baseAddr + CSL_MCSPI_XFERLEVEL);

    regVal &= ~(CSL_MCSPI_XFERLEVEL_AFL_MASK | CSL_MCSPI_XFERLEVEL_AEL_MASK);
    if (chObj->rxFifoTrigLvl != 1)
    {
        regVal |= ((((reminder << chObj->bufWidthShift) - 1U) << CSL_MCSPI_XFERLEVEL_AFL_SHIFT) &
                CSL_MCSPI_XFERLEVEL_AFL_MASK);
    }
    if (chObj->txFifoTrigLvl != 1)
    {
        regVal |= ((((reminder << chObj->bufWidthShift) - 1U) << CSL_MCSPI_XFERLEVEL_AEL_SHIFT) &
                CSL_MCSPI_XFERLEVEL_AEL_MASK);
    }

    regVal &= ~CSL_MCSPI_XFERLEVEL_WCNT_MASK;
    regVal |= ((reminder << CSL_MCSPI_XFERLEVEL_WCNT_SHIFT) &
               CSL_MCSPI_XFERLEVEL_WCNT_MASK);

    CSL_REG32_WR(baseAddr + CSL_MCSPI_XFERLEVEL, regVal);

    /* Enable channel */
    CSL_REG32_FINS(
        baseAddr + MCSPI_CHCTRL(chNum),
        MCSPI_CH0CTRL_EN,
        CSL_MCSPI_CH0CTRL_EN_ACT);
}

static uint32_t MCSPI_continueTxRx(MCSPI_Object *obj,
                                   MCSPI_ChObject *chObj,
                                   MCSPI_Transaction *transaction)
{
    uint32_t        baseAddr, chNum, txEmptyMask, rxFullMask;
    uint32_t        retVal = MCSPI_TRANSFER_STARTED;
    volatile uint32_t        irqStatus, chStat;

    baseAddr = obj->baseAddr;
    chNum = chObj->chCfg.chNum;
    txEmptyMask = Spi_mcspiGetTxMask(chNum);
    rxFullMask  = Spi_mcspiGetRxMask(chNum);

    irqStatus = CSL_REG32_RD(baseAddr + CSL_MCSPI_IRQSTATUS);

    if ((irqStatus & chObj->intrMask) != 0)
    {
        /* Clear the interrupts being serviced. */
        CSL_REG32_WR(baseAddr + CSL_MCSPI_IRQSTATUS, (irqStatus & chObj->intrMask));

        /* First read the data from the Rx FIFO. */
        if ((irqStatus & rxFullMask) == rxFullMask)
        {
            /* Perform RX only when enabled */
            if(MCSPI_TR_MODE_TX_ONLY != chObj->chCfg.trMode)
            {
                uint32_t numWordsToRead = transaction->count - chObj->curRxWords;
                if (numWordsToRead > chObj->effRxFifoDepth)
                {
                    numWordsToRead = chObj->effRxFifoDepth;
                }
                /* Read data from RX FIFO. */
                MCSPI_fifoRead(baseAddr, chObj, numWordsToRead);
            }
        }
        if ((irqStatus & txEmptyMask) == txEmptyMask)
        {
            uint32_t numWordsToWrite = transaction->count - chObj->curTxWords;
            if (numWordsToWrite > chObj->effTxFifoDepth)
            {
                numWordsToWrite = chObj->effTxFifoDepth;
            }

            /* Write the data in TX FIFO.Even in RX only mode, dummy data has to
               be written to receive data from Slave */
            MCSPI_fifoWrite(baseAddr, chObj, numWordsToWrite);
        }
        if ((irqStatus & CSL_MCSPI_IRQSTATUS_EOW_MASK) == CSL_MCSPI_IRQSTATUS_EOW_MASK)
        {
            if (MCSPI_TR_MODE_RX_ONLY != chObj->chCfg.trMode)
            {
                if (transaction->count == chObj->curTxWords)
                {
                    do{
                        /* Wait for end of transfer. */
                        chStat = CSL_REG32_RD(baseAddr + MCSPI_CHSTAT(chNum));
                    }while ((chStat & CSL_MCSPI_CH0STAT_EOT_MASK) == 0);

                    /* read the last data if any from Rx FIFO. */
                    if ((MCSPI_TR_MODE_TX_ONLY != chObj->chCfg.trMode) &&
                        (transaction->count != chObj->curRxWords))
                    {
                        /* This is a corner case. EOW is set at the end of transmission.
                            * the reception is not complete by the time we are processing EOW.
                            * Read the remaining bytes.
                            */
                        MCSPI_fifoRead(baseAddr, chObj, (transaction->count - chObj->curRxWords));
                    }
                    /* Clear all interrupts. */
                    MCSPI_intrStatusClear(baseAddr, chObj->intrMask);
                    retVal = MCSPI_TRANSFER_COMPLETED;
                }
                else
                {
                    MCSPI_initiateLastChunkTransfer(obj, chObj, transaction);
                }
            }
            else
            {
                if (transaction->count == chObj->curRxWords)
                {
                    do{
                        /* Wait for end of transfer. */
                        chStat = CSL_REG32_RD(baseAddr + MCSPI_CHSTAT(chNum));
                    }while ((chStat & CSL_MCSPI_CH0STAT_EOT_MASK) == 0);
                    /* Clear all interrupts. */
                    MCSPI_intrStatusClear(baseAddr, chObj->intrMask);
                    retVal = MCSPI_TRANSFER_COMPLETED;
                }
                else
                {
                    MCSPI_initiateLastChunkTransfer(obj, chObj, transaction);
                }
            }
        }
    }
    return retVal;
}

static int32_t MCSPI_transferMasterPoll(MCSPI_Object *obj,
                                        MCSPI_ChObject *chObj,
                                        const MCSPI_Attrs *attrs,
                                        MCSPI_Transaction *transaction)
{
    int32_t         status = SystemP_SUCCESS;
    uint32_t        baseAddr, chNum;
    uint32_t        transferStatus = MCSPI_TRANSFER_STARTED;

    baseAddr = obj->baseAddr;
    chNum = chObj->chCfg.chNum;

    /* Manual CS assert */
    if(MCSPI_CH_MODE_SINGLE == attrs->chMode)
    {
        CSL_REG32_FINS(
            baseAddr + MCSPI_CHCONF(chNum),
            MCSPI_CH0CONF_FORCE,
            CSL_MCSPI_CH0CONF_FORCE_ASSERT);
    }

    /* Enable channel */
    CSL_REG32_FINS(
        baseAddr + MCSPI_CHCTRL(chNum),
        MCSPI_CH0CTRL_EN,
        CSL_MCSPI_CH0CTRL_EN_ACT);

    /* Busy loop till channel transfer is completed */
    do
    {
        transferStatus = MCSPI_continueTxRx(obj, chObj, transaction);
    } while (transferStatus != MCSPI_TRANSFER_COMPLETED);

    /* Stop MCSPI Channel */
    MCSPI_stop(obj, attrs, chObj, chNum);

    return (status);
}

static int32_t MCSPI_transferMasterIntr(MCSPI_Object *obj,
                                        MCSPI_ChObject *chObj,
                                        const MCSPI_Attrs *attrs,
                                        MCSPI_Transaction *transaction)
{
    int32_t         status = SystemP_SUCCESS;
    uint32_t        baseAddr, chNum;

    baseAddr = obj->baseAddr;
    chNum = chObj->chCfg.chNum;

    CSL_REG32_WR(baseAddr + CSL_MCSPI_IRQENABLE, chObj->intrMask);

    /* Manual CS assert */
    if(MCSPI_CH_MODE_SINGLE == attrs->chMode)
    {
        CSL_REG32_FINS(
            baseAddr + MCSPI_CHCONF(chNum),
            MCSPI_CH0CONF_FORCE,
            CSL_MCSPI_CH0CONF_FORCE_ASSERT);
    }

    /* Enable channel */
    CSL_REG32_FINS(
        baseAddr + MCSPI_CHCTRL(chNum),
        MCSPI_CH0CTRL_EN,
        CSL_MCSPI_CH0CTRL_EN_ACT);

    /*
     * Note: Once the channel is enabled, we will get the TX almost empty
     *       interrupt. No data transfer is required here!!
     */

    return (status);
}

static void MCSPI_slaveIsr(void *args)
{
    int32_t             status = SystemP_SUCCESS;
    uint32_t            transferStatus;
    MCSPI_Config       *config;
    MCSPI_Object       *obj;
    const MCSPI_Attrs  *attrs;
    MCSPI_ChObject     *chObj;
    MCSPI_Transaction  *transaction;
    uint32_t            baseAddr, chNum;
    uint32_t            regVal;

    /* Check parameters */
    if(NULL == args)
    {
        status = SystemP_FAILURE;
    }

    if(SystemP_SUCCESS == status)
    {
        config = (MCSPI_Config *) args;
        obj = config->object;
        attrs = config->attrs;
        DebugP_assert(NULL != obj);
        DebugP_assert(NULL != config->attrs);

        transaction = obj->currTransaction;
        baseAddr = obj->baseAddr;
        if (transaction != NULL)
        {
            chNum = transaction->channel;
            chObj = &obj->chObj[chNum];
            transferStatus = MCSPI_continueSlaveTxRx(obj, chObj, transaction);
            if ((MCSPI_TRANSFER_COMPLETED == transferStatus) ||
                    (MCSPI_TRANSFER_CANCELLED == transferStatus))
            {
                /* Process the transfer completion. */
                /* Stop MCSPI Channel */
                MCSPI_stop(obj, attrs, chObj, chNum);

                /* Disable TX and RX FIFO */
                regVal  = CSL_REG32_RD(baseAddr + MCSPI_CHCONF(chObj->chCfg.chNum));
                regVal &= ~(CSL_MCSPI_CH0CONF_FFEW_MASK | CSL_MCSPI_CH0CONF_FFER_MASK);
                CSL_REG32_WR(baseAddr + MCSPI_CHCONF(chObj->chCfg.chNum), regVal);

                /* Update the driver internal status. */
                /* transfer completed */
                transaction->status  = transferStatus;
                /* Return the actual number of words transferred */
                obj->currTransaction->count = chObj->curRxWords;
                if (MCSPI_TR_MODE_TX_ONLY == chObj->chCfg.trMode)
                {
                    obj->currTransaction->count = chObj->curTxWords;
                }
                obj->currTransaction = NULL;

                /*
                * Post transfer Sem in case of bloacking transfer.
                * Call the callback function in case of Callback mode.
                */
                if (obj->openPrms.transferMode == MCSPI_TRANSFER_MODE_BLOCKING)
                {
                    SemaphoreP_post(&obj->transferSemObj);
                }
                else
                {
                    obj->openPrms.transferCallbackFxn((MCSPI_Handle) config, transaction);
                }
            }
            /*
            * Else the transfer is still pending.
            * Do nothing, wait for next interrupt.
            */
        }
        else
        {
            /* There is no ongoing transfer. Disable and clear all interrupts. */
            CSL_REG32_WR(baseAddr + CSL_MCSPI_IRQENABLE, 0U);
            MCSPI_clearAllIrqStatus(baseAddr);
        }
    }
    return;
}

static uint32_t MCSPI_continueSlaveTxRx(MCSPI_Object *obj,
                                        MCSPI_ChObject *chObj,
                                        MCSPI_Transaction *transaction)
{
    uint32_t            baseAddr, chNum, chStat;
    uint32_t            retVal = MCSPI_TRANSFER_STARTED;
    volatile uint32_t   irqStatus;

    baseAddr = obj->baseAddr;
    chNum = chObj->chCfg.chNum;

    irqStatus = CSL_REG32_RD(baseAddr + CSL_MCSPI_IRQSTATUS);

    if ((irqStatus & chObj->intrMask) != 0)
    {
        /* Clear the interrupts being serviced. */
        CSL_REG32_WR(baseAddr + CSL_MCSPI_IRQSTATUS, (irqStatus & chObj->intrMask));

        /* First read the data from the Rx FIFO. */
        if(irqStatus & (CSL_MCSPI_IRQSTATUS_RX0_FULL_MASK << (4 * chNum)))
        {
            /* Perform RX only when enabled */
            if(MCSPI_TR_MODE_TX_ONLY != chObj->chCfg.trMode)
            {
                uint32_t numWordsToRead = transaction->count - chObj->curRxWords;
                if (numWordsToRead > chObj->effRxFifoDepth)
                {
                    numWordsToRead = chObj->effRxFifoDepth;
                }
                /* Read data from RX FIFO. */
                MCSPI_fifoRead(baseAddr, chObj, numWordsToRead);
                /* Check if transfer is completed for current transaction. */
                if (transaction->count == chObj->curRxWords)
                {
                    retVal = MCSPI_TRANSFER_COMPLETED;
                }
            }
        }
        if (irqStatus & (CSL_MCSPI_IRQSTATUS_TX0_EMPTY_MASK << (4 * chNum)))
        {
            /* Perform TX only when enabled */
            if(MCSPI_TR_MODE_RX_ONLY != chObj->chCfg.trMode)
            {
                uint32_t numWordsToWrite = transaction->count - chObj->curTxWords;
                if (numWordsToWrite > chObj->effTxFifoDepth)
                {
                    numWordsToWrite = chObj->effTxFifoDepth;
                }
                /* Write the data in TX FIFO. */
                MCSPI_fifoWrite(baseAddr, chObj, numWordsToWrite);
                if(MCSPI_TR_MODE_TX_ONLY == chObj->chCfg.trMode)
                {
                    /* Check if transfer is completed for current transaction. */
                    if (transaction->count == chObj->curTxWords)
                    {
                        do{
                            /* Wait for TX FIFO Empty. */
                            chStat = CSL_REG32_RD(baseAddr + MCSPI_CHSTAT(chNum));
                        }while ((chStat & CSL_MCSPI_CH0STAT_TXFFE_MASK) == 0);
                        do{
                            /* Wait for end of transfer. */
                            chStat = CSL_REG32_RD(baseAddr + MCSPI_CHSTAT(chNum));
                        }while ((chStat & CSL_MCSPI_CH0STAT_EOT_MASK) == 0);
                        retVal = MCSPI_TRANSFER_COMPLETED;
                    }
                }
            }
        }
        /* Check for Rx overflow or Tx underflow.
         * Cancel the current transfer and return error. */
        if ((irqStatus & (CSL_MCSPI_IRQSTATUS_RX0_OVERFLOW_MASK)) ||
            (irqStatus & (CSL_MCSPI_IRQSTATUS_TX0_UNDERFLOW_MASK << (4 * chNum))))
        {
            retVal = MCSPI_TRANSFER_CANCELLED;
        }
    }

    return retVal;
}

static int32_t MCSPI_transferSlavePoll(MCSPI_Object *obj,
                                        MCSPI_ChObject *chObj,
                                        const MCSPI_Attrs *attrs,
                                        MCSPI_Transaction *transaction)
{
    int32_t         status = SystemP_SUCCESS;
    uint32_t        baseAddr, chNum;
    uint32_t        transferStatus = MCSPI_TRANSFER_STARTED;

    baseAddr = obj->baseAddr;
    chNum = chObj->chCfg.chNum;

    /* Enable channel */
    CSL_REG32_FINS(
        baseAddr + MCSPI_CHCTRL(chNum),
        MCSPI_CH0CTRL_EN,
        CSL_MCSPI_CH0CTRL_EN_ACT);

    /* Busy loop till channel transfer is completed */
    do
    {
        transferStatus = MCSPI_continueSlaveTxRx(obj, chObj, transaction);
    } while (transferStatus != MCSPI_TRANSFER_COMPLETED);

    /* Stop MCSPI Channel */
    MCSPI_stop(obj, attrs, chObj, chNum);

    return (status);
}

static int32_t MCSPI_transferSlaveIntr(MCSPI_Object *obj,
                                        MCSPI_ChObject *chObj,
                                        const MCSPI_Attrs *attrs,
                                        MCSPI_Transaction *transaction)
{
    int32_t         status = SystemP_SUCCESS;
    uint32_t        baseAddr, chNum;

    baseAddr = obj->baseAddr;
    chNum = chObj->chCfg.chNum;

    CSL_REG32_WR(baseAddr + CSL_MCSPI_IRQENABLE, chObj->intrMask);

    /* Enable channel */
    CSL_REG32_FINS(
        baseAddr + MCSPI_CHCTRL(chNum),
        MCSPI_CH0CTRL_EN,
        CSL_MCSPI_CH0CTRL_EN_ACT);

    /*
     * Note: Once the channel is enabled, we will get the TX almost empty
     *       interrupt. No data transfer is required here!!
     */

    return (status);
}

static void MCSPI_fifoWrite(uint32_t baseAddr, MCSPI_ChObject *chObj, uint32_t transferLength)
{
    uint32_t        chNum;

    chNum = chObj->chCfg.chNum;
    if(NULL != chObj->curTxBufPtr)
    {
        if(0U == chObj->bufWidthShift)
        {
            chObj->curTxBufPtr = MCSPI_fifoWrite8(
                                     baseAddr,
                                     chNum,
                                     chObj->curTxBufPtr,
                                     transferLength);
        }
        else if(1U == chObj->bufWidthShift)
        {
            chObj->curTxBufPtr = (const uint8_t *) MCSPI_fifoWrite16(
                                     baseAddr,
                                     chNum,
                                     (const uint16_t *) chObj->curTxBufPtr,
                                     transferLength);
        }
        else
        {
            chObj->curTxBufPtr = (const uint8_t *) MCSPI_fifoWrite32(
                                     baseAddr,
                                     chNum,
                                     (const uint32_t *) chObj->curTxBufPtr,
                                     transferLength);
        }
    }
    else
    {
        /* NULL TX pointer provided. Use default data */
        MCSPI_fifoWriteDefault(
            baseAddr, chNum, chObj->chCfg.defaultTxData, transferLength);
    }
    chObj->curTxWords += transferLength;

    return;
}

static void MCSPI_fifoRead(uint32_t baseAddr, MCSPI_ChObject *chObj, uint32_t transferLength)
{
    uint32_t        chNum;

    chNum = chObj->chCfg.chNum;
    if(NULL != chObj->curRxBufPtr)
    {
        if(0U == chObj->bufWidthShift)
        {
            chObj->curRxBufPtr = MCSPI_fifoRead8(
                                     baseAddr,
                                     chNum,
                                     chObj->curRxBufPtr,
                                     transferLength,
                                     chObj->dataWidthBitMask);
        }
        else if(1U == chObj->bufWidthShift)
        {
            chObj->curRxBufPtr = (uint8_t *) MCSPI_fifoRead16(
                                     baseAddr,
                                     chNum,
                                     (uint16_t *) chObj->curRxBufPtr,
                                     transferLength,
                                     chObj->dataWidthBitMask);
        }
        else
        {
            chObj->curRxBufPtr = (uint8_t *) MCSPI_fifoRead32(
                                     baseAddr,
                                     chNum,
                                     (uint32_t *) chObj->curRxBufPtr,
                                     transferLength,
                                     chObj->dataWidthBitMask);
        }
    }
    else
    {
        /* NULL RX pointer provided. Read and discard data */
        MCSPI_fifoReadDiscard(baseAddr, chNum, transferLength);
    }
    chObj->curRxWords += transferLength;

    return;
}

static void MCSPI_configInstance(MCSPI_Config *config)
{
    uint32_t                regVal;
    uint32_t                baseAddr;
    const MCSPI_Attrs      *attrs;
    MCSPI_OpenParams       *openPrms;

    DebugP_assert(NULL != config->attrs);
    DebugP_assert(NULL != config->object);
    attrs = config->attrs;
    baseAddr = config->object->baseAddr;
    openPrms = &config->object->openPrms;

    /* Reset MCSPI */
    MCSPI_reset(baseAddr);

    /* Set sysconfig */
    regVal = ((CSL_MCSPI_SYSCONFIG_CLOCKACTIVITY_BOTH <<
               CSL_MCSPI_SYSCONFIG_CLOCKACTIVITY_SHIFT) |
              (CSL_MCSPI_SYSCONFIG_SIDLEMODE_NO <<
               CSL_MCSPI_SYSCONFIG_SIDLEMODE_SHIFT) |
              (CSL_MCSPI_SYSCONFIG_ENAWAKEUP_NOWAKEUP <<
               CSL_MCSPI_SYSCONFIG_ENAWAKEUP_SHIFT) |
              (CSL_MCSPI_SYSCONFIG_AUTOIDLE_OFF <<
               CSL_MCSPI_SYSCONFIG_AUTOIDLE_SHIFT));
    CSL_REG32_WR(baseAddr + CSL_MCSPI_SYSCONFIG, regVal);

    /* Set module control */
    regVal = (openPrms->msMode << CSL_MCSPI_MODULCTRL_MS_SHIFT);
    /* Configure Single/Multi Channel in master mode only */
    if(MCSPI_MS_MODE_MASTER == openPrms->msMode)
    {
        regVal |= (attrs->chMode << CSL_MCSPI_MODULCTRL_SINGLE_SHIFT);
    }
    if(MCSPI_CH_MODE_SINGLE == attrs->chMode)
    {
        /* 3/4 pin mode applicable only in single channel mode.
         * For  multi-ch mode, CS is always controlled by HW during transfer */
        regVal |= (attrs->pinMode << CSL_MCSPI_MODULCTRL_PIN34_SHIFT);
        if(MCSPI_MS_MODE_MASTER == openPrms->msMode)
        {
            /* Init delay applicable only for single master mode */
            regVal |= (attrs->initDelay << CSL_MCSPI_MODULCTRL_INITDLY_SHIFT);
        }
    }
    CSL_REG32_WR(baseAddr + CSL_MCSPI_MODULCTRL, regVal);

    return;
}

static void MCSPI_setChConfig(MCSPI_Config *config,
                              MCSPI_ChObject *chObj)
{
    uint32_t                regVal;
    uint32_t                baseAddr, chNum;
    MCSPI_ChConfig         *chCfg;
    const MCSPI_Attrs      *attrs;
    uint32_t                txFifoTrigLvl, rxFifoTrigLvl;

    DebugP_assert(NULL != config->attrs);
    DebugP_assert(NULL != config->object);
    attrs = config->attrs;
    baseAddr = config->object->baseAddr;
    chNum = chObj->chCfg.chNum;
    chCfg = &chObj->chCfg;

    regVal = CSL_REG32_RD(baseAddr + MCSPI_CHCONF(chNum));

    /* Clear and set PHA, POL fields */
    regVal &= ~((uint32_t) CSL_MCSPI_CH0CONF_PHA_MASK |
                (uint32_t) CSL_MCSPI_CH3CONF_POL_MASK);
    regVal |= (chCfg->frameFormat & ((uint32_t) CSL_MCSPI_CH0CONF_PHA_MASK |
                                     (uint32_t) CSL_MCSPI_CH3CONF_POL_MASK));
    CSL_FINS(regVal, MCSPI_CH0CONF_EPOL, chCfg->csPolarity);
    CSL_FINS(regVal, MCSPI_CH0CONF_WL, (chCfg->dataSize - 1U));
    CSL_FINS(regVal, MCSPI_CH0CONF_TRM, chCfg->trMode);
    CSL_FINS(regVal, MCSPI_CH0CONF_IS, chCfg->inputSelect);
    CSL_FINS(regVal, MCSPI_CH0CONF_DPE0, chCfg->dpe0);
    CSL_FINS(regVal, MCSPI_CH0CONF_DPE1, chCfg->dpe1);
    CSL_FINS(regVal, MCSPI_CH0CONF_SPIENSLV, chCfg->slvCsSelect);
    CSL_FINS(regVal, MCSPI_CH0CONF_SBE, chCfg->startBitEnable);
    CSL_FINS(regVal, MCSPI_CH0CONF_SBPOL, chCfg->startBitPolarity);
    CSL_FINS(regVal, MCSPI_CH0CONF_TCS0, chCfg->csIdleTime);

    CSL_REG32_WR(baseAddr + MCSPI_CHCONF(chNum), regVal);

    /* Set clock dividers */
    MCSPI_setClkConfig(baseAddr, chNum, attrs->inputClkFreq, chCfg->bitRate);

    /* Calculate buffer access width */
    chObj->bufWidthShift = MCSPI_getBufWidthShift(chObj->chCfg.dataSize);

    /* Calculate data width mask depending on SPI word size */
    chObj->dataWidthBitMask = MCSPI_getDataWidthBitMask(chObj->chCfg.dataSize);

    /*
     * Note: TX/RX FIFO trigger levels are set to half the TX/RX FIFO levels,
     *       so that it is efficient as well as we don't get TX underflow or
     *       RX overflow.
     *       Setting a low value is in-efficient where as setting a high value
     *       leads to overflow/underflow!!
     */
    /* Calculate effective FIFO Trigger level */
    if(TRUE == attrs->intrEnable)
    {
        if(MCSPI_TR_MODE_TX_RX == chObj->chCfg.trMode)
        {
            /* Both TX/RX is used, divide the FIFO equally */
            txFifoTrigLvl = (MCSPI_FIFO_LENGTH >> 2U);
            rxFifoTrigLvl = (MCSPI_FIFO_LENGTH >> 2U);
        }
        else if(MCSPI_TR_MODE_TX_ONLY == chObj->chCfg.trMode)
        {
            /* TX only mode, full FIFO is used for TX */
            txFifoTrigLvl = (MCSPI_FIFO_LENGTH >> 1U);
            rxFifoTrigLvl = 1U;
        }
        else
        {
            /* RX only mode, full FIFO is used for RX */
            txFifoTrigLvl = 1U;
            rxFifoTrigLvl = (MCSPI_FIFO_LENGTH >> 1U);
        }
    }
    else
    {
        /* we can use double the FIFO depth for polled mode as half empty/full
         * trigger is not used */
        if(MCSPI_TR_MODE_TX_RX == chObj->chCfg.trMode)
        {
            /* Both TX/RX is used, divide the FIFO equally */
            txFifoTrigLvl = (MCSPI_FIFO_LENGTH >> 1U);
            rxFifoTrigLvl = (MCSPI_FIFO_LENGTH >> 1U);
        }
        else if(MCSPI_TR_MODE_TX_ONLY == chObj->chCfg.trMode)
        {
            /* TX only mode, full FIFO is used for TX */
            txFifoTrigLvl = (MCSPI_FIFO_LENGTH);
            rxFifoTrigLvl = 1U;
        }
        else
        {
            /* RX only mode, full FIFO is used for RX */
            txFifoTrigLvl = 1U;
            rxFifoTrigLvl = (MCSPI_FIFO_LENGTH);
        }
    }

    chObj->txFifoTrigLvl = txFifoTrigLvl;
    chObj->rxFifoTrigLvl = rxFifoTrigLvl;
    chObj->effTxFifoDepth = chObj->txFifoTrigLvl >> chObj->bufWidthShift;
    chObj->effRxFifoDepth = chObj->rxFifoTrigLvl >> chObj->bufWidthShift;

    if (config->object->openPrms.msMode == MCSPI_MS_MODE_SLAVE)
    {
        if(MCSPI_TR_MODE_TX_RX == chObj->chCfg.trMode)
        {
            chObj->intrMask = ((CSL_MCSPI_IRQSTATUS_RX0_FULL_MASK      << (4 * chNum)) |
                               (CSL_MCSPI_IRQSTATUS_TX0_EMPTY_MASK     << (4 * chNum)) |
                               (CSL_MCSPI_IRQSTATUS_TX0_UNDERFLOW_MASK << (4 * chNum)) |
                               (CSL_MCSPI_IRQSTATUS_RX0_OVERFLOW_MASK));
        }
        else if(MCSPI_TR_MODE_TX_ONLY == chObj->chCfg.trMode)
        {
            chObj->intrMask = ((CSL_MCSPI_IRQSTATUS_TX0_EMPTY_MASK     << (4 * chNum)) |
                               (CSL_MCSPI_IRQSTATUS_TX0_UNDERFLOW_MASK << (4 * chNum)));
        }
        else
        {
            chObj->intrMask = ((CSL_MCSPI_IRQSTATUS_RX0_FULL_MASK     << (4 * chNum)) |
                               (CSL_MCSPI_IRQSTATUS_RX0_OVERFLOW_MASK));
        }
    }
    else
    {
        if(MCSPI_TR_MODE_TX_RX == chObj->chCfg.trMode)
        {
            chObj->intrMask =  ((CSL_MCSPI_IRQSTATUS_RX0_FULL_MASK  << (4 * chNum)) |
                                (CSL_MCSPI_IRQSTATUS_TX0_EMPTY_MASK << (4 * chNum)) |
                                 CSL_MCSPI_IRQSTATUS_EOW_MASK);
        }
        else if(MCSPI_TR_MODE_TX_ONLY == chObj->chCfg.trMode)
        {
            chObj->intrMask = ((CSL_MCSPI_IRQSTATUS_TX0_EMPTY_MASK << (4 * chNum)) |
                               (CSL_MCSPI_IRQSTATUS_EOW_MASK));
        }
        else
        {
            chObj->intrMask = ((CSL_MCSPI_IRQSTATUS_RX0_FULL_MASK << (4 * chNum)) |
                               (CSL_MCSPI_IRQSTATUS_TX0_EMPTY_MASK << (4 * chNum)) |
                               (CSL_MCSPI_IRQSTATUS_EOW_MASK));
        }
    }

    return;
}

static int32_t MCSPI_checkOpenParams(const MCSPI_OpenParams *openPrms)
{
    int32_t     status = SystemP_SUCCESS;

    if((MCSPI_TRANSFER_MODE_CALLBACK == openPrms->transferMode) &&
       (NULL == openPrms->transferCallbackFxn))
    {
        DebugP_logError("[MCSPI] Callback should be provided when using callback mode !!!\r\n");
        status = SystemP_FAILURE;
    }

    return (status);
}

static int32_t MCSPI_checkChConfig(MCSPI_Object   *obj, const MCSPI_ChConfig *chCfg)
{
    int32_t     status = SystemP_SUCCESS;

    if((chCfg->dataSize < 4U) || (chCfg->dataSize > 32U))
    {
        DebugP_logError("[MCSPI] Unsupported word length or data size !!!\r\n");
        status = SystemP_FAILURE;
    }
    if((obj->openPrms.msMode == MCSPI_MS_MODE_SLAVE) && (chCfg->chNum != 0))
    {
        DebugP_logError("[MCSPI] Only channel 0 supported in slave mode !!!\r\n");
        status = SystemP_FAILURE;
    }

    return (status);
}

static int32_t MCSPI_checkTransaction(const MCSPI_Object *obj,
                                      MCSPI_Transaction *transaction)
{
    int32_t     status = SystemP_SUCCESS;

    if(0U == transaction->count)
    {
        /* Transfer count should be positive */
        transaction->status = MCSPI_TRANSFER_FAILED;
        status = SystemP_FAILURE;
    }
    if(transaction->channel >= MCSPI_MAX_NUM_CHANNELS)
    {
        /* Invalid channel */
        transaction->status = MCSPI_TRANSFER_FAILED;
        status = SystemP_FAILURE;
    }
    else
    {
        /* Check if the channel is configured */
        if(TRUE != obj->chObj[transaction->channel].isOpen)
        {
            /* Channel not configured */
            transaction->status = MCSPI_TRANSFER_FAILED;
            status = SystemP_FAILURE;
        }
    }

    return (status);
}

static uint32_t MCSPI_getDataWidthBitMask(uint32_t dataWidth)
{
    uint32_t    i, fifoBitMask = 0x0U;
    uint32_t    tmpVar = 0U;

    for (i = 0U; i < dataWidth; i++)
    {
        tmpVar = ((uint32_t)1U << i);
        fifoBitMask |= tmpVar;
    }

    return (fifoBitMask);
}

static void MCSPI_reset(uint32_t baseAddr)
{
    uint32_t    regVal;

    /* Set the SOFTRESET field of MCSPI_SYSCONFIG register. */
    CSL_REG32_FINS(
        baseAddr + CSL_MCSPI_SYSCONFIG,
        MCSPI_SYSCONFIG_SOFTRESET,
        CSL_MCSPI_SYSCONFIG_SOFTRESET_ON);

    /* Stay in the loop until reset is done. */
    while(1U)
    {
        regVal = CSL_REG32_RD(baseAddr + CSL_MCSPI_SYSSTATUS);
        if((regVal & CSL_MCSPI_SYSSTATUS_RESETDONE_MASK) ==
            CSL_MCSPI_SYSSTATUS_RESETDONE_MASK)
        {
            break;
        }
        /* Busy wait */
    }
}

static void MCSPI_setClkConfig(uint32_t baseAddr,
                               uint32_t chNum,
                               uint32_t inputClkFreq,
                               uint32_t bitRate)
{
    uint32_t fRatio;
    uint32_t clkD;
    uint32_t extClk;

    /* Calculate the value of fRatio. */
    fRatio = inputClkFreq / bitRate;
    if((fRatio*bitRate) != inputClkFreq)
    {
        /* use a higher divider value in case the ratio
         * is fractional so that we get a lower SPI clock
         * than requested. This ensures we don't go beyond
         * recommended clock speed for the SPI slave */
        fRatio++;
    }

    /* If fRatio is not a power of 2, set granularity of 1 clock cycle */
    if((uint32_t) 0U != (fRatio & (fRatio - 1U)))
    {
        /* Set the clock granularity to 1 clock cycle */
        CSL_REG32_FINS(
            baseAddr + MCSPI_CHCONF(chNum),
            MCSPI_CH0CONF_CLKG,
            CSL_MCSPI_CH0CONF_CLKG_ONECYCLE);

        /* Calculate the ratios clkD and extClk based on fClk */
        extClk = (fRatio - 1U) >> 4U;
        clkD   = (fRatio - 1U) & (uint32_t) MCSPI_CLKD_MASK;

        /* Set the extClk field */
        CSL_REG32_FINS(
            baseAddr + MCSPI_CHCTRL(chNum),
            MCSPI_CH0CTRL_EXTCLK,
            extClk);
    }
    else
    {
        /* Clock granularity of power of 2 */
        CSL_REG32_FINS(
            baseAddr + MCSPI_CHCONF(chNum),
            MCSPI_CH0CONF_CLKG,
            CSL_MCSPI_CH0CONF_CLKG_POWERTWO);

        clkD = 0U;
        while (1U != fRatio)
        {
            fRatio >>= 1U;
            clkD++;
        }
    }

    /* Configure the clkD field */
    CSL_REG32_FINS(baseAddr + MCSPI_CHCONF(chNum), MCSPI_CH0CONF_CLKD, clkD);

    return;
}

static void MCSPI_setFifoConfig(const MCSPI_ChObject *chObj,
                                uint32_t baseAddr,
                                uint32_t numWordsTxRx)
{
    uint32_t txFifoTrigLvl, rxFifoTrigLvl;
    uint32_t regVal;
    uint32_t reminder, effNumWordsTxRx;

    /* Start transferring only multiple of FIFO trigger level */
    if(MCSPI_TR_MODE_RX_ONLY != chObj->chCfg.trMode)
    {
        reminder = (numWordsTxRx & (chObj->effTxFifoDepth - 1U));
    }
    else
    {
        reminder = (numWordsTxRx & (chObj->effRxFifoDepth - 1U));
    }

    effNumWordsTxRx = numWordsTxRx - reminder;

    rxFifoTrigLvl = chObj->rxFifoTrigLvl;
    txFifoTrigLvl = chObj->txFifoTrigLvl;

    /* Handle transfers with less than FIFO level.
     * Set FIFO trigger level and word count to be equal to the
     * reminder word. Otherwise the HW doesn't generating TX
     * empty interrupt if WCNT is less than FIFO trigger level */
    if(effNumWordsTxRx == 0U)
    {
        effNumWordsTxRx = reminder;
        if(txFifoTrigLvl != 1U)
        {
            /* Set TX level only when TX FIFO is enabled */
            txFifoTrigLvl = reminder << chObj->bufWidthShift;
        }
        if(rxFifoTrigLvl != 1U)
        {
            /* Set RX level only when RX FIFO is enabled */
            rxFifoTrigLvl = reminder << chObj->bufWidthShift;
        }
    }

    /* Set FIFO trigger level and word count */
    regVal  = CSL_REG32_RD(baseAddr + CSL_MCSPI_XFERLEVEL);
    regVal &= ~(CSL_MCSPI_XFERLEVEL_AFL_MASK | CSL_MCSPI_XFERLEVEL_AEL_MASK);
    regVal |= (((rxFifoTrigLvl - 1U) << CSL_MCSPI_XFERLEVEL_AFL_SHIFT) &
               CSL_MCSPI_XFERLEVEL_AFL_MASK);
    regVal |= (((txFifoTrigLvl - 1U) << CSL_MCSPI_XFERLEVEL_AEL_SHIFT) &
               CSL_MCSPI_XFERLEVEL_AEL_MASK);
    regVal &= ~CSL_MCSPI_XFERLEVEL_WCNT_MASK;
    regVal |= ((effNumWordsTxRx << CSL_MCSPI_XFERLEVEL_WCNT_SHIFT) &
               CSL_MCSPI_XFERLEVEL_WCNT_MASK);
    CSL_REG32_WR(baseAddr + CSL_MCSPI_XFERLEVEL, regVal);

    /* Enable TX and RX FIFO */
    regVal  = CSL_REG32_RD(baseAddr + MCSPI_CHCONF(chObj->chCfg.chNum));
    regVal &= ~(CSL_MCSPI_CH0CONF_FFEW_MASK | CSL_MCSPI_CH0CONF_FFER_MASK);
    if(MCSPI_TR_MODE_TX_RX == chObj->chCfg.trMode)
    {
        regVal |= (CSL_MCSPI_CH0CONF_FFEW_FFENABLED << CSL_MCSPI_CH0CONF_FFEW_SHIFT);
        regVal |= (CSL_MCSPI_CH0CONF_FFER_FFENABLED << CSL_MCSPI_CH0CONF_FFER_SHIFT);
    }
    else if(MCSPI_TR_MODE_TX_ONLY == chObj->chCfg.trMode)
    {
        regVal |= (CSL_MCSPI_CH0CONF_FFEW_FFENABLED << CSL_MCSPI_CH0CONF_FFEW_SHIFT);
    }
    else
    {
        regVal |= (CSL_MCSPI_CH0CONF_FFER_FFENABLED << CSL_MCSPI_CH0CONF_FFER_SHIFT);
    }
    CSL_REG32_WR(baseAddr + MCSPI_CHCONF(chObj->chCfg.chNum), regVal);

    return;
}

static uint32_t MCSPI_getFifoTrigLvl(uint32_t numWords, uint32_t fifoDepth)
{
    uint32_t fifoTrigLvl = 1U;
    if (numWords > fifoDepth)
    {
        uint32_t i;
        for (i=fifoDepth; i>0; i--)
        {
            if ((numWords%i) == 0 )
            {
                fifoTrigLvl = i;
                break;
            }
        }
    }
    else
    {
        fifoTrigLvl = numWords;
    }
    return fifoTrigLvl;
}

static void MCSPI_setSlaveFifoConfig(MCSPI_ChObject *chObj,
                                     uint32_t baseAddr,
                                     uint32_t numWordsTxRx)
{
    uint32_t txFifoTrigLvl, rxFifoTrigLvl;
    uint32_t regVal;

    /* Find Fifo depth to configure to be multiple of number of words to transfer. */
    chObj->effTxFifoDepth = MCSPI_getFifoTrigLvl(numWordsTxRx, chObj->txFifoTrigLvl >> chObj->bufWidthShift);
    chObj->effRxFifoDepth = MCSPI_getFifoTrigLvl(numWordsTxRx, chObj->rxFifoTrigLvl >> chObj->bufWidthShift);

    txFifoTrigLvl = chObj->effTxFifoDepth << chObj->bufWidthShift;
    rxFifoTrigLvl = chObj->effRxFifoDepth << chObj->bufWidthShift;

    /* Set FIFO trigger level. word count set to 0 for slve mode. */
    regVal  = CSL_REG32_RD(baseAddr + CSL_MCSPI_XFERLEVEL);
    regVal &= ~(CSL_MCSPI_XFERLEVEL_AFL_MASK | CSL_MCSPI_XFERLEVEL_AEL_MASK);
    regVal |= (((rxFifoTrigLvl - 1U) << CSL_MCSPI_XFERLEVEL_AFL_SHIFT) &
               CSL_MCSPI_XFERLEVEL_AFL_MASK);
    regVal |= (((txFifoTrigLvl - 1U) << CSL_MCSPI_XFERLEVEL_AEL_SHIFT) &
               CSL_MCSPI_XFERLEVEL_AEL_MASK);
    regVal &= ~CSL_MCSPI_XFERLEVEL_WCNT_MASK;
    CSL_REG32_WR(baseAddr + CSL_MCSPI_XFERLEVEL, regVal);

    /* Enable TX and RX FIFO */
    regVal  = CSL_REG32_RD(baseAddr + MCSPI_CHCONF(chObj->chCfg.chNum));
    regVal &= ~(CSL_MCSPI_CH0CONF_FFEW_MASK | CSL_MCSPI_CH0CONF_FFER_MASK);
    if(MCSPI_TR_MODE_TX_RX == chObj->chCfg.trMode)
    {
        regVal |= (CSL_MCSPI_CH0CONF_FFEW_FFENABLED << CSL_MCSPI_CH0CONF_FFEW_SHIFT);
        regVal |= (CSL_MCSPI_CH0CONF_FFER_FFENABLED << CSL_MCSPI_CH0CONF_FFER_SHIFT);
    }
    else if(MCSPI_TR_MODE_TX_ONLY == chObj->chCfg.trMode)
    {
        regVal |= (CSL_MCSPI_CH0CONF_FFEW_FFENABLED << CSL_MCSPI_CH0CONF_FFEW_SHIFT);
    }
    else
    {
        regVal |= (CSL_MCSPI_CH0CONF_FFER_FFENABLED << CSL_MCSPI_CH0CONF_FFER_SHIFT);
    }
    CSL_REG32_WR(baseAddr + MCSPI_CHCONF(chObj->chCfg.chNum), regVal);

    return;
}

static const uint8_t *MCSPI_fifoWrite8(uint32_t        baseAddr,
                                       uint32_t        chNum,
                                       const uint8_t  *bufPtr,
                                       uint32_t        transferLength)
{
    uint32_t        i, txData;

    /* Write the data in TX FIFO for 8-bit transfer */
    for(i = 0; i < transferLength; i++)
    {
        txData = *bufPtr++;
        CSL_REG32_WR(baseAddr + MCSPI_CHTX(chNum), txData);
    }

    return (bufPtr);
}

static const uint16_t *MCSPI_fifoWrite16(uint32_t        baseAddr,
                                         uint32_t        chNum,
                                         const uint16_t *bufPtr,
                                         uint32_t        transferLength)
{
    uint32_t        i, txData;

    /* Write the data in TX FIFO for 16-bit transfer */
    for(i = 0; i < transferLength; i++)
    {
        txData = *bufPtr++;
        CSL_REG32_WR(baseAddr + MCSPI_CHTX(chNum), txData);
    }

    return (bufPtr);
}

static const uint32_t *MCSPI_fifoWrite32(uint32_t        baseAddr,
                                         uint32_t        chNum,
                                         const uint32_t *bufPtr,
                                         uint32_t        transferLength)
{
    uint32_t        i, txData;

    /* Write the data in TX FIFO for 32-bit transfer */
    for(i = 0; i < transferLength; i++)
    {
        txData = *bufPtr++;
        CSL_REG32_WR(baseAddr + MCSPI_CHTX(chNum), txData);
    }

    return (bufPtr);
}

static uint8_t *MCSPI_fifoRead8(uint32_t  baseAddr,
                                uint32_t  chNum,
                                uint8_t  *bufPtr,
                                uint32_t  transferLength,
                                uint32_t  dataWidthBitMask)
{
    uint32_t        i, rxData;

    /* Read the data from RX FIFO for 8-bit transfer */
    for(i = 0; i < transferLength; i++)
    {
        rxData = MCSPI_readRxDataReg(baseAddr, chNum);
        rxData &= dataWidthBitMask;         /* Clear unused bits */
        *bufPtr++ = (uint8_t) rxData;
    }

    return (bufPtr);
}

static uint16_t *MCSPI_fifoRead16(uint32_t  baseAddr,
                                  uint32_t  chNum,
                                  uint16_t  *bufPtr,
                                  uint32_t  transferLength,
                                  uint32_t  dataWidthBitMask)
{
    uint32_t        i, rxData;

    /* Read the data from RX FIFO for 16-bit transfer */
    for(i = 0; i < transferLength; i++)
    {
        rxData = MCSPI_readRxDataReg(baseAddr, chNum);
        rxData &= dataWidthBitMask;         /* Clear unused bits */
        *bufPtr++ = (uint16_t) rxData;
    }

    return (bufPtr);
}

static uint32_t *MCSPI_fifoRead32(uint32_t  baseAddr,
                                  uint32_t  chNum,
                                  uint32_t  *bufPtr,
                                  uint32_t  transferLength,
                                  uint32_t  dataWidthBitMask)
{
    uint32_t        i, rxData;

    /* Read the data from RX FIFO for 32-bit transfer */
    for(i = 0; i < transferLength; i++)
    {
        rxData = MCSPI_readRxDataReg(baseAddr, chNum);
        rxData &= dataWidthBitMask;         /* Clear unused bits */
        *bufPtr++ = (uint32_t) rxData;
    }

    return (bufPtr);
}

static void MCSPI_fifoWriteDefault(uint32_t baseAddr,
                                   uint32_t chNum,
                                   uint32_t defaultTxData,
                                   uint32_t transferLength)
{
    uint32_t        i;

    /* Write default data to TX FIFO */
    for(i = 0; i < transferLength; i++)
    {
        CSL_REG32_WR(baseAddr + MCSPI_CHTX(chNum), defaultTxData);
    }

    return;
}

static void MCSPI_fifoReadDiscard(uint32_t baseAddr,
                                  uint32_t chNum,
                                  uint32_t transferLength)
{
    uint32_t            i;
    volatile uint32_t   rxData;

    /* Read the data from RX FIFO and discard it */
    for(i = 0; i < transferLength; i++)
    {
        rxData = MCSPI_readRxDataReg(baseAddr, chNum);
    }

    return;
}

static void MCSPI_clearAllIrqStatus(uint32_t baseAddr)
{
    /* Clear all previous interrupt status */
    CSL_REG32_FINS(baseAddr + CSL_MCSPI_SYST, MCSPI_SYST_SSB, CSL_MCSPI_SYST_SSB_OFF);
    CSL_REG32_WR(baseAddr + CSL_MCSPI_IRQSTATUS, MCSPI_IRQSTATUS_CLEAR_ALL);

    return;
}

static uint32_t Spi_mcspiGetTxMask(uint32_t csNum)
{
    uint32_t txEmptyMask;

    if ((uint32_t)MCSPI_CHANNEL_0 == csNum)
    {
        txEmptyMask = (uint32_t) CSL_MCSPI_IRQSTATUS_TX0_EMPTY_MASK;
    }
    else if ((uint32_t)MCSPI_CHANNEL_1 == csNum)
    {
        txEmptyMask = (uint32_t) CSL_MCSPI_IRQSTATUS_TX1_EMPTY_MASK;
    }
    else if ((uint32_t)MCSPI_CHANNEL_2 == csNum)
    {
        txEmptyMask = (uint32_t) CSL_MCSPI_IRQSTATUS_TX2_EMPTY_MASK;
    }
    else
    {
        txEmptyMask = (uint32_t) CSL_MCSPI_IRQSTATUS_TX3_EMPTY_MASK;
    }

    return (txEmptyMask);
}

static uint32_t Spi_mcspiGetRxMask(uint32_t csNum)
{
    uint32_t rxFullMask;

    if ((uint32_t)MCSPI_CHANNEL_0 == csNum)
    {
        rxFullMask = (uint32_t) CSL_MCSPI_IRQSTATUS_RX0_FULL_MASK;
    }
    else if ((uint32_t)MCSPI_CHANNEL_1 == csNum)
    {
        rxFullMask = (uint32_t) CSL_MCSPI_IRQSTATUS_RX1_FULL_MASK;
    }
    else if ((uint32_t)MCSPI_CHANNEL_2 == csNum)
    {
        rxFullMask = (uint32_t) CSL_MCSPI_IRQSTATUS_RX2_FULL_MASK;
    }
    else
    {
        rxFullMask = (uint32_t) CSL_MCSPI_IRQSTATUS_RX3_FULL_MASK;
    }

    return (rxFullMask);
}

static inline void MCSPI_intrStatusClear(uint32_t baseAddr, uint32_t intFlags)
{
    /* Clear the SSB bit in the MCSPI_SYST register. */
    CSL_REG32_FINS(baseAddr + CSL_MCSPI_SYST, MCSPI_SYST_SSB, CSL_MCSPI_SYST_SSB_OFF);

    /* Clear the interrupt status. */
    CSL_REG32_WR(baseAddr + CSL_MCSPI_IRQSTATUS, intFlags);
}

static void MCSPI_stop(MCSPI_Object *obj, const MCSPI_Attrs *attrs,
                       MCSPI_ChObject *chObj, uint32_t chNum)
{
    uint32_t regVal, baseAddr;

    baseAddr = obj->baseAddr;
    /* Disable channel interrupts. */
    regVal = CSL_REG32_RD(baseAddr + CSL_MCSPI_IRQENABLE);
    regVal &= ~(chObj->intrMask);
    CSL_REG32_WR(baseAddr + CSL_MCSPI_IRQENABLE, regVal);
    MCSPI_intrStatusClear(baseAddr, chObj->intrMask);

    if(MCSPI_MS_MODE_MASTER == obj->openPrms.msMode)
    {
        /* Manual CS de-assert */
        if(MCSPI_CH_MODE_SINGLE == attrs->chMode)
        {
            CSL_REG32_FINS(
                baseAddr + MCSPI_CHCONF(chNum),
                MCSPI_CH0CONF_FORCE,
                CSL_MCSPI_CH0CONF_FORCE_DEASSERT);
        }
    }

    /* Disable channel */
    CSL_REG32_FINS(
        baseAddr + MCSPI_CHCTRL(chNum),
        MCSPI_CH0CTRL_EN,
        CSL_MCSPI_CH0CTRL_EN_NACT);

    /* Disable TX and RX FIFO This is required so that next CS can
     * use the FIFO */
    regVal  = CSL_REG32_RD(baseAddr + MCSPI_CHCONF(chNum));
    regVal &= ~(CSL_MCSPI_CH0CONF_FFEW_MASK | CSL_MCSPI_CH0CONF_FFER_MASK);
    CSL_REG32_WR(baseAddr + MCSPI_CHCONF(chNum), regVal);

}

/* ========================================================================== */
/*                       Advanced Function Definitions                        */
/* ========================================================================== */
uint32_t MCSPI_getBaseAddr(MCSPI_Handle handle)
{
    MCSPI_Config       *config;
    MCSPI_Object       *obj;
    uint32_t            baseAddr;

    /* Check parameters */
    if (NULL == handle)
    {
        baseAddr = 0U;
    }
    else
    {
        config   = (MCSPI_Config *) handle;
        obj      = config->object;
        baseAddr = obj->baseAddr;
    }

    return baseAddr;
}

int32_t MCSPI_reConfigFifo(MCSPI_Handle handle,
                           uint32_t chNum,
                           uint32_t numWordsRxTx)
{
    uint32_t        baseAddr;
    MCSPI_Config   *config;
    MCSPI_Object   *obj;
    MCSPI_ChObject *chObj;
    int32_t         status = SystemP_SUCCESS;

    /* Check parameters */
    if((NULL == handle) ||
       (chNum >= MCSPI_MAX_NUM_CHANNELS))
    {
        status = SystemP_FAILURE;
    }

    if(SystemP_SUCCESS == status)
    {
        config = (MCSPI_Config *) handle;
        obj = config->object;
        baseAddr = config->object->baseAddr;
        chObj = &obj->chObj[chNum];

        /* Re-Configure word count */
        MCSPI_setFifoConfig(chObj, baseAddr, numWordsRxTx);
    }

    return status;
}
