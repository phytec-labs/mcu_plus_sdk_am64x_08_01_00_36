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

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */
#include <kernel/dpl/HwiP.h>
#include <kernel/dpl/SemaphoreP.h>
#include "icss_timeSync_osal.h"
#include "icss_timeSync_memory_map.h"
#include <drivers/mdio.h>
#include "icss_timeSync_utils.h"
#include <kernel/dpl/ClockP.h>
#include <drivers/hw_include/hw_types.h>

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

/*TODO: Review this function*/
int8_t TimeSync_isrAndTaskInit(TimeSync_ParamsHandle_t timeSyncHandle)
{
    int32_t status = SystemP_FAILURE;
    HwiP_Params hwiParams;
    TaskP_Params taskParams;
    uint8_t portNum = 0;

    /*-----------Create Interrupts------------*/

    /*Setup Tx callback interrupt*/
    HwiP_Params_init(&hwiParams);

    hwiParams.intNum = timeSyncHandle->timeSyncConfig.txIntNum;
    hwiParams.callback = (HwiP_FxnCallback)TimeSync_txTSIsr;
    hwiParams.args = (void *)timeSyncHandle;
    hwiParams.priority = PTP_TX_CALLBACK_HWI_PRIORITY;

    status = HwiP_construct(&(timeSyncHandle->timeSync_txTSIsrObject), &hwiParams);

    if(status == SystemP_FAILURE)
    {
        return TIME_SYNC_UNABLE_TO_CREATE_INTERRUPT;
    }

    /*Enable interrupt*/
    HwiP_enableInt(timeSyncHandle->timeSyncConfig.txIntNum);

    if(timeSyncHandle->timeSyncConfig.process_TOD_and_sync_2_1PPS)
    {
        /*Setup latch0 interrupt*/
        HwiP_Params_init(&hwiParams);

        hwiParams.intNum = timeSyncHandle->timeSyncConfig.latchIntNum;
        hwiParams.callback = (HwiP_FxnCallback)TimeSync_latchIsr;
        hwiParams.args = (void *)timeSyncHandle;
        hwiParams.priority = PTP_LATCH0_HWI_PRIORITY;

        status = HwiP_construct(&(timeSyncHandle->timeSync_latchIsrObject), &hwiParams);

        if(status == SystemP_FAILURE)
        {
            return TIME_SYNC_UNABLE_TO_CREATE_INTERRUPT;
        }

        /*Enable interrupt*/
        HwiP_enableInt(timeSyncHandle->timeSyncConfig.latchIntNum);
    }

    // /*---------------DM Timer setup---------------------*/
    // TimerP_Params_init(&timerParams);

    // timerParams.period = timeSyncHandle->timeSyncConfig.syncSendInterval;
    // timerParams.periodType = TimerP_PeriodType_MICROSECS;
    // timerParams.arg = (void *)timeSyncHandle;
    // /*24 Mhz, care must be taken to ensure that this values
    //  * is modified if the source clock frequency is modified */
    // //timerParams.extfreqLo = DMTIMER_SRC_CLK_FREQ;
    // //timerParams.extfreqHi = 0;
    // timerParams.startMode = TimerP_StartMode_USER;
    // timerParams.runMode = TimerP_RunMode_CONTINUOUS;

    // timeSyncHandle->timeSync_syncTxTimer = TimerP_create(DMTIMER_4_ID,
    //                                        (TimerP_Fxn)TimeSync_syncTxIsr,
    //                                        &timerParams);

    // if(timeSyncHandle->timeSync_syncTxTimer == NULL)
    // {
    //     return TIME_SYNC_UNABLE_TO_CREATE_CLOCK;
    // }

    /*Create semaphore for sending Sync frames*/

    status = SemaphoreP_constructBinary(&(timeSyncHandle->syncTxSemObject), 0);

    if(status == SystemP_FAILURE)
    {
        return TIME_SYNC_UNABLE_TO_CREATE_SEMAPHORE;
    }

    if(timeSyncHandle->timeSyncConfig.process_TOD_and_sync_2_1PPS)
    {
        /*Create semaphore which is posted by latch ISR which in turn triggers the task used to synchronize the clock*/
        status = SemaphoreP_constructBinary(&(timeSyncHandle->latch_sync_SemObject), 0);

        if(status == SystemP_FAILURE)
        {
            return TIME_SYNC_UNABLE_TO_CREATE_SEMAPHORE;
        }
    }


    if(timeSyncHandle->timeSyncConfig.type == E2E)
    {
        /*Create semaphore for sending delay request frames*/
        status = SemaphoreP_constructBinary(&(timeSyncHandle->delayReqTxSemObject), 0);

        if(status == SystemP_FAILURE)
        {
            return TIME_SYNC_UNABLE_TO_CREATE_SEMAPHORE;
        }
    }

    /*Assign event ID's*/
    timeSyncHandle->eventIdSync = ICSS_TIMESYNC_EVENT_BIT2;
    timeSyncHandle->eventIdPdelayReq = ICSS_TIMESYNC_EVENT_BIT3;
    timeSyncHandle->eventIdPdelayResp = ICSS_TIMESYNC_EVENT_BIT4;
    timeSyncHandle->eventIdPdelayRespFlwUp = ICSS_TIMESYNC_EVENT_BIT5;
    timeSyncHandle->eventIdDelayReq = ICSS_TIMESYNC_EVENT_BIT6;
    timeSyncHandle->eventIdFlwUpGenerated = ICSS_TIMESYNC_EVENT_BIT7;

    for(portNum = 0; portNum < ICSS_EMAC_MAX_PORTS_PER_INSTANCE; portNum++)
    {
        status = EventP_construct(&(timeSyncHandle->ptpPdelayResEvtObject[portNum]));

        if(status == SystemP_FAILURE)
        {
            return TIME_SYNC_UNABLE_TO_CREATE_EVENT;
        }
    }

    for(portNum = 0; portNum < ICSS_EMAC_MAX_PORTS_PER_INSTANCE; portNum++)
    {
        status = EventP_construct(&(timeSyncHandle->txTSAvailableEvtObject[portNum]));

        if(status == SystemP_FAILURE)
        {
            return TIME_SYNC_UNABLE_TO_CREATE_EVENT;
        }
    }

    for(portNum = 0; portNum < ICSS_EMAC_MAX_PORTS_PER_INSTANCE; portNum++)
    {
        status = EventP_construct(&(timeSyncHandle->ptpPdelayReqEvtObject[portNum]));

        if(status == SystemP_FAILURE)
        {
            return TIME_SYNC_UNABLE_TO_CREATE_EVENT;
        }
    }

    for(portNum = 0; portNum < ICSS_EMAC_MAX_PORTS_PER_INSTANCE; portNum++)
    {
        status = EventP_construct(&(timeSyncHandle->ptpSendFollowUpEvtObject[portNum]));

        if(status == SystemP_FAILURE)
        {
            return TIME_SYNC_UNABLE_TO_CREATE_EVENT;
        }
    }

    /*-----------Create Tasks------------*/

    if(timeSyncHandle->timeSyncConfig.type == P2P)
    {
        TaskP_Params_init(&taskParams);

        taskParams.stackSize = TASK_STACK_SIZE;
        taskParams.stack = (uint8_t *)(&(timeSyncHandle->pDelayReqSendTaskStack));
        taskParams.priority = PTP_DELAY_REQ_SEND_TASK_PRIORITY;
        taskParams.args = (void *)timeSyncHandle;
        taskParams.taskMain = (TaskP_FxnMain)TimeSync_PdelayReqSendTask;

        status = TaskP_construct(&(timeSyncHandle->timeSync_pDelayReqSendTask), &taskParams);

        if(status == SystemP_FAILURE)
        {
            return TIME_SYNC_UNABLE_TO_CREATE_TASK;
        }
    }

    if(timeSyncHandle->timeSyncConfig.type == E2E)
    {
        TaskP_Params_init(&taskParams);

        taskParams.stackSize = TASK_STACK_SIZE;
        taskParams.stack = (uint8_t *)(&(timeSyncHandle->delayReqSendTaskStack));
        taskParams.priority = PTP_DELAY_REQ_SEND_TASK_PRIORITY;
        taskParams.args = (void *)timeSyncHandle;
        taskParams.taskMain = (TaskP_FxnMain)TimeSync_delayReqSendTask;

        status = TaskP_construct(&(timeSyncHandle->timeSync_delayReqSendTask), &taskParams);

        if(status == SystemP_FAILURE)
        {
            return TIME_SYNC_UNABLE_TO_CREATE_TASK;
        }
    }

    TaskP_Params_init(&taskParams);

    taskParams.stackSize = TASK_STACK_SIZE;
    taskParams.stack = (uint8_t *)(&(timeSyncHandle->txTSTaskP1Stack));
    taskParams.priority = PTP_TX_TS_TASK_PRIORITY;
    taskParams.args = (void *)timeSyncHandle;
    taskParams.taskMain = (TaskP_FxnMain)TimeSync_TxTSTask_P1;

    status = TaskP_construct(&(timeSyncHandle->timeSync_TxTSTaskP1), &taskParams);

    if(status == SystemP_FAILURE)
    {
        return TIME_SYNC_UNABLE_TO_CREATE_TASK;
    }

    TaskP_Params_init(&taskParams);

    taskParams.stackSize = TASK_STACK_SIZE;
    taskParams.stack = (uint8_t *)(&(timeSyncHandle->txTSTaskP2Stack));
    taskParams.priority = PTP_TX_TS_TASK_PRIORITY;
    taskParams.args = (void *)timeSyncHandle;
    taskParams.taskMain = (TaskP_FxnMain)TimeSync_TxTSTask_P2;

    status = TaskP_construct(&(timeSyncHandle->timeSync_TxTSTaskP2), &taskParams);

    if(status == SystemP_FAILURE)
    {
        return TIME_SYNC_UNABLE_TO_CREATE_TASK;
    }

    /* Sync Tx Task*/
    TaskP_Params_init(&taskParams);

    taskParams.stackSize = TASK_STACK_SIZE;
    taskParams.stack = (uint8_t *)(&(timeSyncHandle->syncTxTaskStack));
    taskParams.priority = PTP_SYNC_TASK_PRIORITY;
    taskParams.args = (void *)timeSyncHandle;
    taskParams.taskMain = (TaskP_FxnMain)TimeSync_SyncTxTask;

    status = TaskP_construct(&(timeSyncHandle->timeSync_syncTxTask), &taskParams);

    if(status == SystemP_FAILURE)
    {
        return TIME_SYNC_UNABLE_TO_CREATE_TASK;
    }

    /* Announce Tx Task*/
    TaskP_Params_init(&taskParams);

    taskParams.stackSize = TASK_STACK_SIZE;
    taskParams.stack = (uint8_t *)(&(timeSyncHandle->announceTxTaskStack));
    taskParams.priority = PTP_ANNOUNCE_TASK_PRIORITY;
    taskParams.args = (void *)timeSyncHandle;
    taskParams.taskMain = (TaskP_FxnMain)TimeSync_AnnounceTxTask;

    status = TaskP_construct(&(timeSyncHandle->timeSync_announceTxTask), &taskParams);

    if(status == SystemP_FAILURE)
    {
        return TIME_SYNC_UNABLE_TO_CREATE_TASK;
    }

    /* NRT Task to process peer delay frames*/
    TaskP_Params_init(&taskParams);

    taskParams.stackSize = TASK_STACK_SIZE;
    taskParams.stack = (uint8_t *)(&(timeSyncHandle->NRT_TaskStack));
    taskParams.priority = 8;
    taskParams.args = (void *)timeSyncHandle;
    taskParams.taskMain = (TaskP_FxnMain)TimeSync_NRT_Task;

    status = TaskP_construct(&(timeSyncHandle->timeSync_NRT_Task), &taskParams);

    if(status == SystemP_FAILURE)
    {
        return TIME_SYNC_UNABLE_TO_CREATE_TASK;
    }

    /* Background Task to do computation*/
    TaskP_Params_init(&taskParams);

    taskParams.stackSize = TASK_STACK_SIZE;
    taskParams.stack = (uint8_t *)(&(timeSyncHandle->backgroundTaskStack));
    taskParams.priority = 7;
    taskParams.args = (void *)timeSyncHandle;
    taskParams.taskMain = (TaskP_FxnMain)TimeSync_BackgroundTask;

    status = TaskP_construct(&(timeSyncHandle->timeSync_backgroundTask), &taskParams);

    if(status == SystemP_FAILURE)
    {
        return TIME_SYNC_UNABLE_TO_CREATE_TASK;
    }

    if(timeSyncHandle->timeSyncConfig.process_TOD_and_sync_2_1PPS)
    {
        /* Task where local clock is synchronized to latch0 timestamp*/
        TaskP_Params_init(&taskParams);

        taskParams.stackSize = TASK_STACK_SIZE;
        taskParams.stack = (uint8_t *)(&(timeSyncHandle->latchSyncTaskStack));
        taskParams.priority = 6;
        taskParams.args = (void *)timeSyncHandle;
        taskParams.taskMain = (TaskP_FxnMain)TimeSync_latchSyncTask;

        status = TaskP_construct(&(timeSyncHandle->timeSync_latchSyncTask), &taskParams);

        if(status == SystemP_FAILURE)
        {
            return TIME_SYNC_UNABLE_TO_CREATE_TASK;
        }
    }
    return TIME_SYNC_OK;
}

/* ========================================================================== */
/*                           Task Definitions                                 */
/* ========================================================================== */

void TimeSync_PdelayReqSendTask(void *args)
{
    uint8_t linkStatus = 0;
    uint32_t retVal = SystemP_FAILURE;
    uint8_t frameCount = 0;
    ICSS_EMAC_TxArgument txArg;
    TimeSync_ParamsHandle_t timeSyncHandle = (TimeSync_ParamsHandle_t)args;

    uint8_t offset = timeSyncHandle->timeSyncConfig.frame_offset;

    while(1)
    {
        if(P2P == timeSyncHandle->timeSyncConfig.type)
        {

            if(timeSyncHandle->enabled == TRUE)
            {
                /*TODO: Review this*/
                /*Construct a Delay Request packet and send it on both ports*/
                // linkStatus = ((ICSS_EmacObject *)
                //               (timeSyncHandle->emacHandle)->object)->linkStatus[ICSS_EMAC_PORT_1 - 1];
                linkStatus = 0;
                retVal = MDIO_phyLinkStatus(((PRUICSS_HwAttrs const *)(timeSyncHandle->pruicssHandle->hwAttrs))->miiMdioRegBase,
                                                ((ICSS_EMAC_Attrs *)(timeSyncHandle->emacHandle->attrs))->phyAddr[ICSS_EMAC_PORT_1 - 1]);

                if(retVal == SystemP_SUCCESS)
                    linkStatus = 1;

                /*Send delay request frames in a burst*/
                for(frameCount = 0;
                        frameCount < timeSyncHandle->timeSyncConfig.pdelayBurstNumPkts; frameCount++)
                {
                    if(linkStatus)
                    {
                        /*write sequence id into memory*/
                        TimeSync_addHalfWord(timeSyncHandle->timeSyncBuff.pdelayReq_TxBuf[0] + PTP_SEQ_ID_OFFSET
                                    - offset,
                                    timeSyncHandle->tsRunTimeVar->pDelReqSequenceID[ICSS_EMAC_PORT_1 - 1]);

                        /**Add Source Port ID*/
                        TimeSync_addHalfWord(timeSyncHandle->timeSyncBuff.pdelayReq_TxBuf[0] +
                                    PTP_SRC_PORT_ID_OFFSET - offset,
                                    ICSS_EMAC_PORT_1);

                        /*Use registered callback to send packet on Port 1
                         * In case of LL frame with HSR tag*/
                        if(timeSyncHandle->timeSyncConfig.ll_has_hsrTag ||
                                timeSyncHandle->timeSyncConfig.custom_tx_api)
                        {
/*TODO: Review this*/
// #ifdef NEW_TX_CALLBACK
//                             txArg.customFlag = 0;
// #endif //NEW_TX_CALLBACK
                            txArg.icssEmacHandle = timeSyncHandle->emacHandle;
                            txArg.lengthOfPacket = TIMESYNC_PDELAY_BUF_SIZE;
                            txArg.portNumber = ICSS_EMAC_PORT_1;
                            txArg.queuePriority = ICSS_EMAC_QUEUE1;
                            txArg.srcAddress = timeSyncHandle->timeSyncBuff.pdelayReq_TxBuf[0];

                            /*TODO: Review this*/
                            // if(((((ICSS_EmacObject *)
                            //         timeSyncHandle->emacHandle->object)->callBackHandle)->txCallBack)->callBack(
                            //             &txArg,
                            //             ((((ICSS_EmacObject *)
                            //                timeSyncHandle->emacHandle->object)->callBackHandle)->txCallBack)->userArg) ==
                            //         0)
                            if(ICSS_EMAC_txPacket(&txArg, NULL) == SystemP_SUCCESS)
                            {
                                timeSyncHandle->tsRunTimeVar->pDelReqSequenceID[ICSS_EMAC_PORT_1 - 1]++;
                            }
                        }

                        else
                        {
                            /*TODO: Review this*/
                            txArg.icssEmacHandle = timeSyncHandle->emacHandle;
                            txArg.lengthOfPacket = TIMESYNC_PDELAY_BUF_SIZE;
                            txArg.portNumber = ICSS_EMAC_PORT_1;
                            txArg.queuePriority = ICSS_EMAC_QUEUE1;
                            txArg.srcAddress = timeSyncHandle->timeSyncBuff.pdelayReq_TxBuf[0];

                            /*send packet on port 1 without tag*/
                            // if(ICSS_EmacTxPacketEnqueue(timeSyncHandle->emacHandle,
                            //                             timeSyncHandle->timeSyncBuff.pdelayReq_TxBuf[0],
                            //                             ICSS_EMAC_PORT_1,
                            //                             ICSS_EMAC_QUEUE1, TIMESYNC_PDELAY_BUF_SIZE) ==  0)
                            if(ICSS_EMAC_txPacket(&txArg, NULL) == SystemP_SUCCESS)
                            {
                                timeSyncHandle->tsRunTimeVar->pDelReqSequenceID[ICSS_EMAC_PORT_1 - 1]++;
                            }

                        }

                    }

                    /*Add small delay between two requests on two ports
                     * to avoid collision scenarios
                     */
                    /*TODO: Review this*/
                    ClockP_usleep(ClockP_ticksToUsec(10));

                    /***************************************************/

                    /*TODO: Review this*/
                    // linkStatus = ((ICSS_EmacObject *)
                    //               (timeSyncHandle->emacHandle)->object)->linkStatus[ICSS_EMAC_PORT_2 - 1];
                    linkStatus = 0;
                    retVal = MDIO_phyLinkStatus(((PRUICSS_HwAttrs const *)(timeSyncHandle->pruicssHandle->hwAttrs))->miiMdioRegBase,
                                                    ((ICSS_EMAC_Attrs *)(timeSyncHandle->emacHandle->attrs))->phyAddr[ICSS_EMAC_PORT_2 - 1]);
                    if(retVal == SystemP_SUCCESS)
                        linkStatus = 1;

                    if(linkStatus)
                    {
                        /*write sequence id into memory*/
                        TimeSync_addHalfWord(timeSyncHandle->timeSyncBuff.pdelayReq_TxBuf[1] + PTP_SEQ_ID_OFFSET
                                    - offset,
                                    timeSyncHandle->tsRunTimeVar->pDelReqSequenceID[ICSS_EMAC_PORT_2 - 1]);

                        /**Add Source Port ID*/
                        TimeSync_addHalfWord(timeSyncHandle->timeSyncBuff.pdelayReq_TxBuf[1] +
                                    PTP_SRC_PORT_ID_OFFSET - offset,
                                    ICSS_EMAC_PORT_2);

                        /*Use registered callback to send packet on Port 2*/
                        if(timeSyncHandle->timeSyncConfig.ll_has_hsrTag ||
                                timeSyncHandle->timeSyncConfig.custom_tx_api)
                        {
/*TODO: Review this*/
// #ifdef NEW_TX_CALLBACK
//                             txArg.customFlag = 0;
// #endif //NEW_TX_CALLBACK
                            txArg.icssEmacHandle = timeSyncHandle->emacHandle;
                            txArg.lengthOfPacket = TIMESYNC_PDELAY_BUF_SIZE;
                            txArg.portNumber = ICSS_EMAC_PORT_2;
                            txArg.queuePriority = ICSS_EMAC_QUEUE1;
                            txArg.srcAddress = timeSyncHandle->timeSyncBuff.pdelayReq_TxBuf[1];

                            // if(((((ICSS_EmacObject *)
                            //         timeSyncHandle->emacHandle->object)->callBackHandle)->txCallBack)->callBack(
                            //             &txArg,
                            //             ((((ICSS_EmacObject *)
                            //                timeSyncHandle->emacHandle->object)->callBackHandle)->txCallBack)->userArg) ==
                            //         0)
                            if(ICSS_EMAC_txPacket(&txArg, NULL) == SystemP_SUCCESS)
                            {
                                timeSyncHandle->tsRunTimeVar->pDelReqSequenceID[ICSS_EMAC_PORT_2 - 1]++;
                            }
                        }

                        else
                        {
                            /*send packet on port 2 without tag*/
                            txArg.icssEmacHandle = timeSyncHandle->emacHandle;
                            txArg.lengthOfPacket = TIMESYNC_PDELAY_BUF_SIZE;
                            txArg.portNumber = ICSS_EMAC_PORT_2;
                            txArg.queuePriority = ICSS_EMAC_QUEUE1;
                            txArg.srcAddress = timeSyncHandle->timeSyncBuff.pdelayReq_TxBuf[1];
                            // if(ICSS_EmacTxPacketEnqueue(timeSyncHandle->emacHandle,
                            //                             timeSyncHandle->timeSyncBuff.pdelayReq_TxBuf[1],
                            //                             ICSS_EMAC_PORT_2,
                            //                             ICSS_EMAC_QUEUE1, TIMESYNC_PDELAY_BUF_SIZE) ==  0)
                            if(ICSS_EMAC_txPacket(&txArg, NULL) == SystemP_SUCCESS)
                            {
                                timeSyncHandle->tsRunTimeVar->pDelReqSequenceID[ICSS_EMAC_PORT_2 - 1]++;
                            }
                        }


                    }

                    ClockP_usleep(ClockP_ticksToUsec(timeSyncHandle->timeSyncConfig.pdelayBurstInterval));
                }

            }

            ClockP_usleep(ClockP_ticksToUsec(timeSyncHandle->timeSyncConfig.pDelReqPktInterval));

        }

    }

}

void TimeSync_delayReqSendTask(void *args)
{
    ICSS_EMAC_TxArgument txArg;
    TimeSync_ParamsHandle_t timeSyncHandle = (TimeSync_ParamsHandle_t)args;
    uint8_t offset = timeSyncHandle->timeSyncConfig.frame_offset;

    while(1)
    {
        /*Wait for sync frame to post the semaphore
         * and then send a delay request frame on the port on which
         * master is connected
         */
        SemaphoreP_pend(&(timeSyncHandle->delayReqTxSemObject), SystemP_WAIT_FOREVER);
        /*Use registered callback to send packet on Port connected to master*/

        /*Add port number*/
        TimeSync_addHalfWord(timeSyncHandle->timeSyncBuff.delayReq_TxBuf +
                    PTP_SRC_PORT_ID_OFFSET - offset,
                    timeSyncHandle->tsRunTimeVar->syncPortNum);

        /*Add sequence ID*/
        TimeSync_addHalfWord(timeSyncHandle->timeSyncBuff.delayReq_TxBuf + PTP_SEQ_ID_OFFSET
                    - offset,
                    timeSyncHandle->tsRunTimeVar->delReqSequenceID);

        if(timeSyncHandle->timeSyncConfig.custom_tx_api)
        {
// #ifdef NEW_TX_CALLBACK
//             txArg.customFlag = 0;
// #endif //NEW_TX_CALLBACK
            txArg.icssEmacHandle = timeSyncHandle->emacHandle;
            txArg.lengthOfPacket = TIMESYNC_DELAY_REQ_BUF_SIZE;
            txArg.portNumber = timeSyncHandle->tsRunTimeVar->syncPortNum;
            txArg.queuePriority = ICSS_EMAC_QUEUE1;
            txArg.srcAddress = timeSyncHandle->timeSyncBuff.delayReq_TxBuf;

            // if(((((ICSS_EmacObject *)
            //         timeSyncHandle->emacHandle->object)->callBackHandle)->txCallBack)->callBack(
            //             &txArg, ((((ICSS_EmacObject *)
            //                        timeSyncHandle->emacHandle->object)->callBackHandle)->txCallBack)->userArg) ==
            //         0)
            if(ICSS_EMAC_txPacket(&txArg, NULL) == SystemP_SUCCESS)
            {
                timeSyncHandle->tsRunTimeVar->delReqSequenceID++;
            }
        }

        else
        {
            txArg.icssEmacHandle = timeSyncHandle->emacHandle;
            txArg.lengthOfPacket = TIMESYNC_DELAY_REQ_BUF_SIZE;
            txArg.portNumber = timeSyncHandle->tsRunTimeVar->syncPortNum;
            txArg.queuePriority = ICSS_EMAC_QUEUE1;
            txArg.srcAddress = timeSyncHandle->timeSyncBuff.delayReq_TxBuf;
            /*send packet on same port as master*/
            // if(ICSS_EmacTxPacketEnqueue(timeSyncHandle->emacHandle,
            //                             timeSyncHandle->timeSyncBuff.delayReq_TxBuf,
            //                             timeSyncHandle->tsRunTimeVar->syncPortNum,
            //                             ICSS_EMAC_QUEUE1, TIMESYNC_DELAY_REQ_BUF_SIZE) ==  0)
            if(ICSS_EMAC_txPacket(&txArg, NULL) == SystemP_SUCCESS)
            {
                timeSyncHandle->tsRunTimeVar->delReqSequenceID++;
            }
        }
    }
}

void TimeSync_TxTSTask_P1(void *args)
{
    TimeSync_ParamsHandle_t timeSyncHandle = (TimeSync_ParamsHandle_t)args;
    uint32_t events = 0;

    while(1)
    {
        /*Pend on event to process Tx timestamp interrupt*/
        EventP_waitBits(&(timeSyncHandle->txTSAvailableEvtObject[ICSS_EMAC_PORT_1 - 1]),
                        (timeSyncHandle->eventIdPdelayReq + timeSyncHandle->eventIdSync + timeSyncHandle->eventIdPdelayResp),
                        1, /*Clear bits on exit */
                        0, /*Wait for one of the event bits to be set*/
                        SystemP_WAIT_FOREVER,
                        &events);

        if(events & timeSyncHandle->eventIdSync)
        {
            TimeSync_getTxTS(timeSyncHandle, ICSS_EMAC_PORT_1, SYNC_FRAME);
        }

        if(events & timeSyncHandle->eventIdPdelayReq)
        {
            TimeSync_getTxTS(timeSyncHandle, ICSS_EMAC_PORT_1, DELAY_REQ_FRAME);
        }

        if(events & timeSyncHandle->eventIdPdelayResp)
        {
            TimeSync_getTxTS(timeSyncHandle, ICSS_EMAC_PORT_1, DELAY_RESP_FRAME);
        }
    }
}

void TimeSync_TxTSTask_P2(void *args)
{
    TimeSync_ParamsHandle_t timeSyncHandle = (TimeSync_ParamsHandle_t)args;
    uint32_t events = 0;

    while(1)
    {
        /*Pend on event to process Tx timestamp interrupt*/
        EventP_waitBits(&(timeSyncHandle->txTSAvailableEvtObject[ICSS_EMAC_PORT_2 - 1]),
                        (timeSyncHandle->eventIdPdelayReq + timeSyncHandle->eventIdSync + timeSyncHandle->eventIdPdelayResp),
                        1, /*Clear bits on exit */
                        0, /*Wait for one of the event bits to be set*/
                        SystemP_WAIT_FOREVER,
                        &events);

        if(events & timeSyncHandle->eventIdSync)
        {
            TimeSync_getTxTS(timeSyncHandle, ICSS_EMAC_PORT_2, SYNC_FRAME);
        }

        if(events & timeSyncHandle->eventIdPdelayReq)
        {
            TimeSync_getTxTS(timeSyncHandle, ICSS_EMAC_PORT_2, DELAY_REQ_FRAME);
        }

        if(events & timeSyncHandle->eventIdPdelayResp)
        {
            TimeSync_getTxTS(timeSyncHandle, ICSS_EMAC_PORT_2, DELAY_RESP_FRAME);
        }
    }
}

void TimeSync_NRT_Task(void *args)
{
    uint32_t events = 0;
    TimeSync_ParamsHandle_t timeSyncHandle = (TimeSync_ParamsHandle_t)args;

    while(1)
    {
        /*Wait 1ms then move on to other tasks*/
        EventP_waitBits(&(timeSyncHandle->ptpPdelayResEvtObject[ICSS_EMAC_PORT_1 - 1]),
                        (timeSyncHandle->eventIdPdelayResp + timeSyncHandle->eventIdPdelayRespFlwUp),
                        1, /*Clear bits on exit */
                        1, /*Wait for all bits to be set*/
                        1,
                        &events);

        /*Calculate Peer Delay on Port 1*/
        if(events)
        {
            TimeSync_processPdelayRespFrame(timeSyncHandle,
                                            timeSyncHandle->timeSyncBuff.pdelayRes_RxBuf[ICSS_EMAC_PORT_1 - 1],
                                            FALSE, ICSS_EMAC_PORT_1);

            if(timeSyncHandle->pDelayParams[ICSS_EMAC_PORT_1 - 1].ifTwoStep)
            {

                TimeSync_processPdelayRespFrame(timeSyncHandle,
                                                timeSyncHandle->timeSyncBuff.pdelayResFlwUp_RxBuf[ICSS_EMAC_PORT_1 - 1],
                                                TRUE, ICSS_EMAC_PORT_1);
            }
        }

        /*Wait 1ms then move on to other tasks*/
        EventP_waitBits(&(timeSyncHandle->ptpPdelayResEvtObject[ICSS_EMAC_PORT_2 - 1]),
                        (timeSyncHandle->eventIdPdelayResp + timeSyncHandle->eventIdPdelayRespFlwUp),
                        1, /*Clear bits on exit */
                        1, /*Wait for all bits to be set*/
                        1,
                        &events);

        /*Calculate Peer Delay on Port 2*/
        if(events)
        {
            TimeSync_processPdelayRespFrame(timeSyncHandle,
                                            timeSyncHandle->timeSyncBuff.pdelayRes_RxBuf[ICSS_EMAC_PORT_2 - 1],
                                            FALSE, ICSS_EMAC_PORT_2);

            if(timeSyncHandle->pDelayParams[ICSS_EMAC_PORT_2 - 1].ifTwoStep)
            {
                TimeSync_processPdelayRespFrame(timeSyncHandle,
                                                timeSyncHandle->timeSyncBuff.pdelayResFlwUp_RxBuf[ICSS_EMAC_PORT_2 - 1],
                                                TRUE, ICSS_EMAC_PORT_2);
            }
        }

        /*Send Pdelay response on Port 1*/
        EventP_waitBits(&(timeSyncHandle->ptpPdelayReqEvtObject[ICSS_EMAC_PORT_1 - 1]),
                        (timeSyncHandle->eventIdPdelayReq),
                        1, /*Clear bits on exit */
                        1, /*Wait for all bits to be set*/
                        1,
                        &events);

        if(events)
        {
            TimeSync_processPdelayReqFrame(timeSyncHandle,
                                           timeSyncHandle->timeSyncBuff.pdelayReq_RxBuf[ICSS_EMAC_PORT_1 - 1], \
                                           ICSS_EMAC_PORT_1);
        }

        /*Send Pdelay response on Port 2*/
        EventP_waitBits(&(timeSyncHandle->ptpPdelayReqEvtObject[ICSS_EMAC_PORT_2 - 1]),
                        (timeSyncHandle->eventIdPdelayReq),
                        1, /*Clear bits on exit */
                        1, /*Wait for all bits to be set*/
                        1,
                        &events);
        if(events)
        {
            TimeSync_processPdelayReqFrame(timeSyncHandle,
                                           timeSyncHandle->timeSyncBuff.pdelayReq_RxBuf[ICSS_EMAC_PORT_2 - 1], \
                                           ICSS_EMAC_PORT_2);
        }

        /*Wait 1ms then move on to other tasks*/
        EventP_waitBits(&(timeSyncHandle->ptpSendFollowUpEvtObject[ICSS_EMAC_PORT_1 - 1]),
                        (timeSyncHandle->eventIdSync + timeSyncHandle->eventIdFlwUpGenerated),
                        1, /*Clear bits on exit */
                        1, /*Wait for all bits to be set*/
                        1,
                        &events);

        if(events)
        {
            TimeSync_forced2StepBDCalc(timeSyncHandle, ICSS_EMAC_PORT_1);
        }

        /*Wait 1ms then move on to other tasks*/
        EventP_waitBits(&(timeSyncHandle->ptpSendFollowUpEvtObject[ICSS_EMAC_PORT_2 - 1]),
                        (timeSyncHandle->eventIdSync + timeSyncHandle->eventIdFlwUpGenerated),
                        1, /*Clear bits on exit */
                        1, /*Wait for all bits to be set*/
                        1,
                        &events);

        if(events)
        {
            TimeSync_forced2StepBDCalc(timeSyncHandle, ICSS_EMAC_PORT_2);
        }
    }

}

void TimeSync_BackgroundTask(void *args)
{
    TimeSync_ParamsHandle_t timeSyncHandle = (TimeSync_ParamsHandle_t)args;
    uint32_t sharedRAMbaseAddress = 0;
    uint8_t count = 0;
    int32_t avgCorrection = 0;

    sharedRAMbaseAddress = (uint32_t)((PRUICSS_HwAttrs const *)(timeSyncHandle->pruicssHandle->hwAttrs))->sharedDramBase;

    while(1)
    {
        /*Increment the tick counter*/
        timeSyncHandle->tsRunTimeVar->tickCounter++;

        /*check for sync timeout*/
        if(timeSyncHandle->tsRunTimeVar->syncLastSeenCounter++ >
                timeSyncHandle->tsRunTimeVar->syncTimeoutInterval)
        {

            /*reset the master port used in redundant clock*/
            HW_WR_REG8(sharedRAMbaseAddress + MASTER_PORT_NUM_OFFSET, 0);
            timeSyncHandle->numSyncMissed++;

            /*If we continuously miss sync frames then reset*/
            if((timeSyncHandle->numSyncMissed % NUM_SYNC_MISSED_THRESHOLD) == 0)
            {
                TimeSync_reset(timeSyncHandle);
            }

            /*reset so that we don't keep getting this error*/
            timeSyncHandle->tsRunTimeVar->syncLastSeenCounter = 0;

        }

        /*Check if we have to process the data*/
        if(timeSyncHandle->offsetAlgo->binFull == 1)
        {
            /*get average of all offsets*/
            avgCorrection = 0;

            for(count = 0; count < 5; count++)
            {
                avgCorrection += timeSyncHandle->offsetAlgo->correction[count];
            }

            avgCorrection /= 5;
            /*add the new correction value to initial offset or the PPM value*/
            timeSyncHandle->tsRunTimeVar->initialOffset += avgCorrection;
            /*Reset the variables so another round of measurements can begin*/
            timeSyncHandle->offsetAlgo->binFull = 0;
            timeSyncHandle->offsetAlgo->num_entries_index = 0;
        }

        ClockP_usleep(ClockP_ticksToUsec(PTP_BG_TASK_TICK_PERIOD));
    }

}

void TimeSync_SyncTxTask(uintptr_t arg)
{
    TimeSync_ParamsHandle_t timeSyncHandle = (TimeSync_ParamsHandle_t)arg;
    ICSS_EMAC_TxArgument txArg;
    uint8_t frameSentP1 = 0;
    uint8_t frameSentP2 = 0;
    uint8_t offset = 0;
    uint64_t sec = 0;
    uint32_t n_sec = 0;

    offset = timeSyncHandle->timeSyncConfig.frame_offset;

    while(1)
    {
        /*Pend on semaphore*/
        SemaphoreP_pend(&(timeSyncHandle->syncTxSemObject), SystemP_WAIT_FOREVER);

        if(timeSyncHandle->timeSyncConfig.masterParams.ptp_flags[TS_PTP_TWO_STEP_INDEX])
        {
            /*Use API to get current time*/
            TimeSync_getCurrentTime(timeSyncHandle, &n_sec, &sec);

            /*Write seconds into packet*/
            TimeSync_convEndianess(&sec, timeSyncHandle->timeSyncBuff.sync_TxBuf + \
                          PTP_REQ_RCPT_TS_SEC_OFFSET - offset, 6);
            /*Write nanoseconds into packet*/
            TimeSync_convEndianess(&n_sec, timeSyncHandle->timeSyncBuff.sync_TxBuf + \
                          PTP_REQ_RCPT_TS_NSEC_OFFSET - offset, 4);
        }

        /*write sequence id into frame*/
        TimeSync_addHalfWord(timeSyncHandle->timeSyncBuff.sync_TxBuf + PTP_SEQ_ID_OFFSET -
                    offset,
                    timeSyncHandle->tsRunTimeVar->syncSequenceID);

        /*if 2-step then write same sequence id into follow up frame*/
        if(timeSyncHandle->timeSyncConfig.masterParams.ptp_flags[TS_PTP_TWO_STEP_INDEX])
        {
            TimeSync_addHalfWord(timeSyncHandle->timeSyncBuff.followUp_TxBuf[0] + PTP_SEQ_ID_OFFSET -
                        offset,
                        timeSyncHandle->tsRunTimeVar->syncSequenceID);

            TimeSync_addHalfWord(timeSyncHandle->timeSyncBuff.followUp_TxBuf[1] + PTP_SEQ_ID_OFFSET -
                        offset,
                        timeSyncHandle->tsRunTimeVar->syncSequenceID);
        }

        /*-----------Send Sync on Port 1-------------*/
        /**Add Source Port ID*/
        TimeSync_addHalfWord(timeSyncHandle->timeSyncBuff.sync_TxBuf + PTP_SRC_PORT_ID_OFFSET -
                    offset,
                    ICSS_EMAC_PORT_1);

        if(!timeSyncHandle->timeSyncConfig.masterParams.ptp_flags[TS_PTP_TWO_STEP_INDEX]
                &&
                (V2 == timeSyncHandle->timeSyncConfig.icssVersion))
        {
            TimeSync_writeTS_SingleStep_Sync(timeSyncHandle, ICSS_EMAC_PORT_1);
        }

        if(timeSyncHandle->timeSyncConfig.hsrEnabled ||
                timeSyncHandle->timeSyncConfig.custom_tx_api)
        {

// #ifdef NEW_TX_CALLBACK
//             txArg.customFlag = 0;
// #endif //NEW_TX_CALLBACK
            txArg.icssEmacHandle = timeSyncHandle->emacHandle;
            txArg.lengthOfPacket = timeSyncHandle->timeSyncBuff.syncBuf_size;
            txArg.portNumber = ICSS_EMAC_PORT_1;
            txArg.queuePriority = ICSS_EMAC_QUEUE1;
            txArg.srcAddress = timeSyncHandle->timeSyncBuff.sync_TxBuf;

            // if(((((ICSS_EmacObject *)
            //         timeSyncHandle->emacHandle->object)->callBackHandle)->txCallBack)->callBack(
            //             &txArg,
            //             ((((ICSS_EmacObject *)
            //                timeSyncHandle->emacHandle->object)->callBackHandle)->txCallBack)->userArg) ==
            //         0)
            if(ICSS_EMAC_txPacket(&txArg, NULL) == SystemP_SUCCESS)

            {
                frameSentP1 = 1;
            }
        }

        else
        {
            /*send packet on port 1 without tag*/
            txArg.icssEmacHandle = timeSyncHandle->emacHandle;
            txArg.lengthOfPacket = timeSyncHandle->timeSyncBuff.syncBuf_size;
            txArg.portNumber = ICSS_EMAC_PORT_1;
            txArg.queuePriority = ICSS_EMAC_QUEUE1;
            txArg.srcAddress = timeSyncHandle->timeSyncBuff.sync_TxBuf;
            // if(ICSS_EmacTxPacketEnqueue(timeSyncHandle->emacHandle,
            //                             timeSyncHandle->timeSyncBuff.sync_TxBuf,
            //                             ICSS_EMAC_PORT_1,
            //                             ICSS_EMAC_QUEUE1, timeSyncHandle->timeSyncBuff.syncBuf_size) ==  0)
            if(ICSS_EMAC_txPacket(&txArg, NULL) == SystemP_SUCCESS)
            {
                frameSentP1 = 1;
            }
        }

        /*-----------Send Sync on Port 2-------------*/
        /**Add Source Port ID*/
        TimeSync_addHalfWord(timeSyncHandle->timeSyncBuff.sync_TxBuf + PTP_SRC_PORT_ID_OFFSET -
                    offset,
                    ICSS_EMAC_PORT_2);

        if(!timeSyncHandle->timeSyncConfig.masterParams.ptp_flags[TS_PTP_TWO_STEP_INDEX])
        {
            TimeSync_writeTS_SingleStep_Sync(timeSyncHandle, ICSS_EMAC_PORT_2);
        }

        if(timeSyncHandle->timeSyncConfig.hsrEnabled ||
                timeSyncHandle->timeSyncConfig.custom_tx_api)
        {

            /*Only port number changes*/
            txArg.portNumber = ICSS_EMAC_PORT_2;

            // if(((((ICSS_EmacObject *)
            //         timeSyncHandle->emacHandle->object)->callBackHandle)->txCallBack)->callBack(
            //             &txArg,
            //             ((((ICSS_EmacObject *)
            //                timeSyncHandle->emacHandle->object)->callBackHandle)->txCallBack)->userArg) ==
            //         0)
            if(ICSS_EMAC_txPacket(&txArg, NULL) == SystemP_SUCCESS)
            {
                frameSentP2 = 1;
            }

        }

        else
        {
            /*send packet on port 2 without tag*/
            txArg.portNumber = ICSS_EMAC_PORT_2;

            // if(ICSS_EmacTxPacketEnqueue(timeSyncHandle->emacHandle,
            //                             timeSyncHandle->timeSyncBuff.sync_TxBuf,
            //                             ICSS_EMAC_PORT_2,
            //                             ICSS_EMAC_QUEUE1, timeSyncHandle->timeSyncBuff.syncBuf_size) ==  0)
            if(ICSS_EMAC_txPacket(&txArg, NULL) == SystemP_SUCCESS)
            {
                frameSentP2 = 1;
            }
        }

        /*If any one frame was sent out, increment counter*/
        if(frameSentP2 | frameSentP1)
        {
            timeSyncHandle->tsRunTimeVar->syncSequenceID++;
        }
    }
}

void TimeSync_AnnounceTxTask(void *args)
{
    TimeSync_ParamsHandle_t timeSyncHandle = (TimeSync_ParamsHandle_t)args;
    ICSS_EMAC_TxArgument txArg;
    uint8_t frameSentP1 = 0;
    uint8_t frameSentP2 = 0;
    uint64_t sec = 0;
    uint32_t n_sec = 0;
    uint8_t offset = timeSyncHandle->timeSyncConfig.frame_offset;

    while(1)
    {
        /*Send if master is enabled*/
        if(timeSyncHandle->timeSyncConfig.isMaster)
        {

            /*Use API to get current time*/
            TimeSync_getCurrentTime(timeSyncHandle, &n_sec, &sec);

            /*Write seconds into packet*/
            TimeSync_convEndianess(&sec, timeSyncHandle->timeSyncBuff.announce_TxBuf + \
                          PTP_REQ_RCPT_TS_SEC_OFFSET - offset, 6);
            /*Write nanoseconds into packet*/
            TimeSync_convEndianess(&n_sec, timeSyncHandle->timeSyncBuff.announce_TxBuf + \
                          PTP_REQ_RCPT_TS_NSEC_OFFSET - offset, 4);

            /*write sequence id into memory*/
            TimeSync_addHalfWord(timeSyncHandle->timeSyncBuff.announce_TxBuf + PTP_SEQ_ID_OFFSET -
                        offset,
                        timeSyncHandle->tsRunTimeVar->announceSequenceID);

            /*-----------Send Announce on Port 1-------------*/
            /**Add Source Port ID*/
            TimeSync_addHalfWord(timeSyncHandle->timeSyncBuff.announce_TxBuf + PTP_SRC_PORT_ID_OFFSET
                        - offset,
                        ICSS_EMAC_PORT_1);

            if(timeSyncHandle->timeSyncConfig.hsrEnabled ||
                    timeSyncHandle->timeSyncConfig.custom_tx_api)
            {

// #ifdef NEW_TX_CALLBACK
//                 txArg.customFlag = 0;
// #endif //NEW_TX_CALLBACK
                txArg.icssEmacHandle = timeSyncHandle->emacHandle;
                txArg.lengthOfPacket = timeSyncHandle->timeSyncBuff.announceBuf_size;
                txArg.portNumber = ICSS_EMAC_PORT_1;
                txArg.queuePriority = ICSS_EMAC_QUEUE1;
                txArg.srcAddress = timeSyncHandle->timeSyncBuff.announce_TxBuf;

                // if(((((ICSS_EmacObject *)
                //         timeSyncHandle->emacHandle->object)->callBackHandle)->txCallBack)->callBack(
                //             &txArg,
                //             ((((ICSS_EmacObject *)
                //                timeSyncHandle->emacHandle->object)->callBackHandle)->txCallBack)->userArg) ==
                //         0)
                if(ICSS_EMAC_txPacket(&txArg, NULL) == SystemP_SUCCESS)
                {
                    frameSentP1 = 1;
                }
            }

            else
            {
                /*send packet on port 1 without tag*/
                txArg.icssEmacHandle = timeSyncHandle->emacHandle;
                txArg.lengthOfPacket = timeSyncHandle->timeSyncBuff.announceBuf_size;
                txArg.portNumber = ICSS_EMAC_PORT_1;
                txArg.queuePriority = ICSS_EMAC_QUEUE1;
                txArg.srcAddress = timeSyncHandle->timeSyncBuff.announce_TxBuf;

                // if(ICSS_EmacTxPacketEnqueue(timeSyncHandle->emacHandle,
                //                             timeSyncHandle->timeSyncBuff.announce_TxBuf,
                //                             ICSS_EMAC_PORT_1,
                //                             ICSS_EMAC_QUEUE1, timeSyncHandle->timeSyncBuff.announceBuf_size) ==  0)
                if(ICSS_EMAC_txPacket(&txArg, NULL) == SystemP_SUCCESS)
                {
                    frameSentP1 = 1;
                }
            }

            /**Add Source Port ID*/
            TimeSync_addHalfWord(timeSyncHandle->timeSyncBuff.announce_TxBuf + PTP_SRC_PORT_ID_OFFSET
                        - offset,
                        ICSS_EMAC_PORT_2);

            /*Only port number changes*/
            txArg.portNumber = ICSS_EMAC_PORT_2;

            if(timeSyncHandle->timeSyncConfig.hsrEnabled ||
                    timeSyncHandle->timeSyncConfig.custom_tx_api)
            {

                // if(((((ICSS_EmacObject *)
                //         timeSyncHandle->emacHandle->object)->callBackHandle)->txCallBack)->callBack(
                //             &txArg,
                //             ((((ICSS_EmacObject *)
                //                timeSyncHandle->emacHandle->object)->callBackHandle)->txCallBack)->userArg) ==
                //         0)
                if(ICSS_EMAC_txPacket(&txArg, NULL) == SystemP_SUCCESS)
                {
                    frameSentP2 = 1;
                }
            }

            else
            {
                /*send packet on port 1 without tag*/
                // if(ICSS_EmacTxPacketEnqueue(timeSyncHandle->emacHandle,
                //                             timeSyncHandle->timeSyncBuff.announce_TxBuf,
                //                             ICSS_EMAC_PORT_2,
                //                             ICSS_EMAC_QUEUE1, timeSyncHandle->timeSyncBuff.announceBuf_size) ==  0)
                if(ICSS_EMAC_txPacket(&txArg, NULL) == SystemP_SUCCESS)
                {
                    frameSentP2 = 1;
                }
            }

            /*If any one frame was sent out, increment counter*/
            if(frameSentP2 | frameSentP1)
            {
                timeSyncHandle->tsRunTimeVar->announceSequenceID++;
            }

        }
        frameSentP1 = 0;
        frameSentP2 = 0;
        ClockP_usleep(ClockP_ticksToUsec(timeSyncHandle->timeSyncConfig.announceSendInterval));
    }
}

void TimeSync_latchSyncTask(uintptr_t arg0, uintptr_t arg1)
{
    TimeSync_ParamsHandle_t timeSyncHandle = (TimeSync_ParamsHandle_t)arg0;

    while(1)
    {
        /*Pend on semaphore*/
        SemaphoreP_pend(&(timeSyncHandle->latch_sync_SemObject), SystemP_WAIT_FOREVER);

        if(timeSyncHandle->timeSyncConfig.process_TOD_and_sync_2_1PPS)
        {
            /*Get the latch time stamp*/
            TimeSync_getLatchTime(timeSyncHandle,
                                  &(timeSyncHandle->tsToDParams->latch_Ts_NS),
                                  &(timeSyncHandle->tsToDParams->latch_Ts_Sec));
            /*Synchronize the clock*/
            TimeSync_synchronizeToD(timeSyncHandle);
            /*Calculate sync interval*/
            TimeSync_calcSyncInterval_ToD(timeSyncHandle);

        }
    }
}

/* ========================================================================== */
/*                           ISR Definitions                                  */
/* ========================================================================== */

void TimeSync_txTSIsr(uintptr_t arg)
{
    volatile uint32_t *intStatusPtr = NULL;
    volatile uint8_t *bytePtr = NULL;

    uint8_t portNum = 0;
    uint8_t oppPort = 0;
    uint32_t nanoseconds = 0;
    uint32_t sharedRAMbaseAddress = 0;
    uint64_t seconds = 0;

    TimeSync_ParamsHandle_t timeSyncHandle = (TimeSync_ParamsHandle_t)arg;

    sharedRAMbaseAddress = (uint32_t)((PRUICSS_HwAttrs const *)(timeSyncHandle->pruicssHandle->hwAttrs))->sharedDramBase;

    /*TODO: Review this*/
    intStatusPtr = (uint32_t *)(uint32_t)((uint32_t)((PRUICSS_HwAttrs const *)(timeSyncHandle->pruicssHandle->hwAttrs))->intcRegBase
                                          + CSL_ICSS_G_PR1_ICSS_INTC_INTC_SLV_ENA_STATUS_REG0);

    /*Check which port posted the interrupt*/
    if(*intStatusPtr & TIMESYNC_TX_TS_ISR_MASK_P1)
    {
        bytePtr = (uint8_t *)((uint32_t)(sharedRAMbaseAddress +
                                         TX_TS_NOTIFICATION_OFFSET_SYNC_P1));
        *intStatusPtr = TIMESYNC_TX_TS_ISR_MASK_P1;
        portNum = ICSS_EMAC_PORT_1;
        oppPort = ICSS_EMAC_PORT_2;
    }

    else
    {
        bytePtr = (uint8_t *)((uint32_t)(sharedRAMbaseAddress +
                                         TX_TS_NOTIFICATION_OFFSET_SYNC_P2));
        *intStatusPtr = TIMESYNC_TX_TS_ISR_MASK_P2;
        portNum = ICSS_EMAC_PORT_2;
        oppPort = ICSS_EMAC_PORT_1;
    }

    if(*(bytePtr))      /*sync frame*/
    {
        TimeSync_getTxTimestamp(timeSyncHandle, SYNC_FRAME, portNum,
                                &nanoseconds, &seconds);


        if(timeSyncHandle->timeSyncConfig.isMaster)
        {
            timeSyncHandle->syncParam[portNum - 1]->txTsSec = seconds;
            timeSyncHandle->syncParam[portNum - 1]->txTs = nanoseconds;
        }

        else
        {
            timeSyncHandle->syncParam[oppPort - 1]->txTsSec = seconds;
            timeSyncHandle->syncParam[oppPort - 1]->txTs = nanoseconds;
        }

        /*If master then post event to
         * send out follow up frame*/
        if(timeSyncHandle->timeSyncConfig.isMaster &&
                timeSyncHandle->timeSyncConfig.masterParams.ptp_flags[TS_PTP_TWO_STEP_INDEX])
        {
            EventP_setBits(&(timeSyncHandle->txTSAvailableEvtObject[portNum - 1]), timeSyncHandle->eventIdSync);
        }

        /*If slave and forced 2-step then post event to send out follow up frame*/
        if(timeSyncHandle->tsRunTimeVar->forced2step[oppPort - 1])
        {
            EventP_setBits(&(timeSyncHandle->txTSAvailableEvtObject[portNum - 1]), timeSyncHandle->eventIdSync);
        }

        *(bytePtr) = 0;
    }

    if(*(bytePtr + 1))      /*delay request frame*/
    {
        TimeSync_getTxTimestamp(timeSyncHandle, DELAY_REQ_FRAME, portNum,
                                &nanoseconds, &seconds);

        if(timeSyncHandle->timeSyncConfig.type == P2P)
        {
            timeSyncHandle->pDelayParams[portNum - 1].T1Sec = seconds;
            /*Copy nanoseconds*/
            timeSyncHandle->pDelayParams[portNum - 1].T1Nsec = nanoseconds;
        }

        else
        {
            timeSyncHandle->delayParams->delReqTxTsNS = nanoseconds;
            timeSyncHandle->delayParams->delReqTxTsSec = seconds;
        }

        *(bytePtr + 1) = 0;
    }

    if(*(bytePtr + 2))      /*Pdelay response frame*/
    {
        EventP_setBits(&(timeSyncHandle->txTSAvailableEvtObject[portNum - 1]), timeSyncHandle->eventIdPdelayResp);
        *(bytePtr + 2) = 0;
    }

}

void TimeSync_latchIsr(uintptr_t arg)
{
    volatile uint32_t *intStatusPtr = NULL;


    TimeSync_ParamsHandle_t timeSyncHandle = (TimeSync_ParamsHandle_t)arg;

    /*TODO: Review this*/
    intStatusPtr = (uint32_t *)(uint32_t)((uint32_t)((PRUICSS_HwAttrs const *)(timeSyncHandle->pruicssHandle->hwAttrs))->intcRegBase
                                          + CSL_ICSS_G_PR1_ICSS_INTC_INTC_SLV_ENA_STATUS_REG0);
    /*clear the interrupt*/
    *intStatusPtr = TIMESYNC_LATCH0_ISR_MASK;

    /*Post semaphore for latch sync task*/
    SemaphoreP_post(&(timeSyncHandle->latch_sync_SemObject));

}

void TimeSync_syncTxIsr(uintptr_t arg)
{
    TimeSync_ParamsHandle_t timeSyncHandle = (TimeSync_ParamsHandle_t)arg;
    SemaphoreP_post(&(timeSyncHandle->syncTxSemObject));
}

void TimeSync_enableMaster(TimeSync_ParamsHandle_t timeSyncHandle)
{
    uint8_t *bytePtr = NULL;
    /*Enable mode*/
    timeSyncHandle->timeSyncConfig.isMaster = 1;
    /*Write into memory*/
    bytePtr = (uint8_t *)(((PRUICSS_HwAttrs const *)(timeSyncHandle->pruicssHandle->hwAttrs))->sharedDramBase +
              DUT_IS_MASTER_OFFSET);
    *bytePtr = 1;

    /*Populate parameters of DUT into the follow up frame*/
    TimeSync_initializeFollowUp(timeSyncHandle);

    /*TODO: Review this*/
    /*Start the clock*/
    TimerP_start(timeSyncHandle->timeSync_syncTxTimerHandle);

}

void TimeSync_disableMaster(TimeSync_ParamsHandle_t timeSyncHandle)
{
    uint8_t *bytePtr = NULL;
    /*Disable mode*/
    timeSyncHandle->timeSyncConfig.isMaster = 0;
    /*Write into memory*/
    bytePtr = (uint8_t *)(((PRUICSS_HwAttrs const *)(timeSyncHandle->pruicssHandle->hwAttrs))->sharedDramBase +
              DUT_IS_MASTER_OFFSET);
    *bytePtr = 0;

    /*TODO: Review this*/
    /*Stop the timer*/
    TimerP_stop(timeSyncHandle->timeSync_syncTxTimerHandle);
}
