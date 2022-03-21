/*
 * Copyright (C) 2021 Texas Instruments Incorporated - http://www.ti.com/
 *
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*!
 * \file  lwip2enet.h
 *
 * \brief Header file for the LwIP to Enet helper functions.
 */

#ifndef LWIP2ENET_H_
#define LWIP2ENET_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#ifdef __cplusplus
extern "C" {
#endif

/* Standard language headers */
#include <stdint.h>
#include <assert.h>

/* OS/Posix headers */
#include <kernel/dpl/TaskP.h>
#include <kernel/dpl/SemaphoreP.h>
#include <kernel/dpl/ClockP.h>
#include <kernel/dpl/DebugP.h>

/* Project dependency headers */
#include "lwipif2enet_AppIf.h"
#include <networking/enet/enet.h>
#include <networking/enet/enet_cfg.h>
#include "pbufQ.h"

/* ========================================================================== */
/*                                 Macros                                     */
/* ========================================================================== */

/*
 * Pre-Pad Packet Data Offset
 *
 *   The TCP/IP stack library requires that every packet device
 *   include enough L2 header room for all supported headers. In
 *   order to support PPPoE, this requires a 22 byte L2 header.
 *   Thus, since standard Ethernet is only 14 bytes, we must add
 *   on an additional 8 byte offset, or PPPoE can not function
 *   with our driver.
 */
// #define     PKT_PREPAD                      ((uint32_t)8U)
#define     PKT_PREPAD                      ((uint32_t)0)

/* Indicates whether RAM based multicast lists are suported for this
 * peripheral.
 */
#define     RAM_MCAST                       0U

/* Indicates whether HASH based multicasting is suported for this
 * peripheral.
 */
#define     HASH_MCAST                      0U

/* Multicast Address List Size */
#define     PKT_MAX_MCAST                   ((uint32_t)31U)

/* TODO
 * Change the Rx Tx packets to match lwip
 * Lwip is configured to send 1 pbuf at a time so this may cause issues
 */
/*
 * Packet count of packets given to the hardware. We should use twice this number
 * for the number of packets maintained
 */

#define LWIP2ENET_TX_PACKETS             (PBUF_POOL_SIZE/3)
#define LWIP2ENET_RX_PACKETS             (LWIP2ENET_TX_PACKETS*2)

/* ========================================================================== */
/*                         Structures and Enums                               */
/* ========================================================================== */

enum LWIP2ENET_IOCTL_
{
    LWIP2ENET_IOCTL_BASE            = 0x00000000,
    /** lwip2enet get rxPacketTaskObj load function IOCTL value. IOCTL param is empty */
    /* IMP: Taking this value as it should not conflict with Enet IOCTL commands */
    LWIP2ENET_IOCTL_GET_RXTASK_LOAD = 0x0000ABAA,
    /** lwip2enet get txPacketTaskObj load function IOCTL value. IOCTL param is empty */
    /* IMP: Taking this value as it should not conflict with Enet IOCTL commands */
    LWIP2ENET_IOCTL_GET_TXTASK_LOAD = 0x0000ABAB
};

#define HISTORY_CNT ((uint32_t)256U)

typedef LwipifEnetAppIf_GetHandleOutArgs Lwip2Enet_AppInfo;

typedef struct LWIP2ENET_PKT_TASK_STATS_
{
    uint32_t rawNotifyCnt;
    uint32_t dataNotifyCnt;
    uint32_t zeroNotifyCnt;
    uint32_t totalPktCnt;
    uint32_t totalCycleCnt;

    uint32_t pktsPerNotifyMax;
    uint32_t pktsPerNotify[HISTORY_CNT];
    uint32_t cycleCntPerNotifyMax;
    uint32_t cycleCntPerNotify[HISTORY_CNT];
    uint32_t cycleCntPerPktMax;
    uint32_t cycleCntPerPkt[HISTORY_CNT];
    uint32_t taskLoad[HISTORY_CNT];
}
Lwip2Enet_PktTaskStats;

typedef struct Lwip2Enet_STATS_
{
    Lwip2Enet_PktTaskStats rxStats;
    uint32_t rxFreePbufPktEnq;
    uint32_t rxFreePbufPktDq;
    uint32_t rxFreeAppPktEnq;
    uint32_t rxFreeAppPktDq;
    uint32_t rxUnderFlowCnt;
    uint32_t rxChkSumErr;
    uint32_t rxStackNotifyCnt;
    uint32_t rxPbufAllocFailCnt;

    Lwip2Enet_PktTaskStats txStats;
    uint32_t txReadyPbufPktEnq;
    uint32_t txReadyPbufPktDq;
    uint32_t txFreeAppPktEnq;
    uint32_t txFreeAppPktDq;
    uint32_t txDroppedPktCnt;

    uint32_t gCpuLoad[HISTORY_CNT];
    uint32_t gHwiLoad[HISTORY_CNT];
}
Lwip2Enet_Stats;

/**
 * \brief
 *  Packet device information
 *
 * \details
 *  This structure caches the device info.
 */
typedef struct Lwip2Enet_OBJECT_
{
    /*! lwIP network interface */
    struct netif *netif;

    Lwip2Enet_AppInfo appInfo;
    /** Initialization flag.*/
    uint32_t initDone;
    /** Index of currently connect physical port.*/
    uint32_t currLinkedIf;

    /** Current RX filter */
    uint32_t rxFilter;
    /** Previous MCast Address Counter */
    uint32_t oldMCastCnt;
    /** Previous Multicast list configured by the Application.*/
    uint8_t bOldMCast[(uint32_t)ENET_MAC_ADDR_LEN * PKT_MAX_MCAST];
    /** Current MCast Address Counter */
    uint32_t MCastCnt;
    /** Multicast list configured by the Application.*/
    uint8_t bMCast[(uint32_t)ENET_MAC_ADDR_LEN * PKT_MAX_MCAST];
    /** Link is up flag. */
    uint32_t linkIsUp;
    /** Device is operating in test digital loopback mode.*/
    uint32_t inDLBMode;
    /** Total number of PBM packets allocated by application - used for debug purpose.*/
    uint32_t numAllocPbufPkts;

    /*! Enet LLD packet info structure used to exchange data between adaptation
     *  layer and the driver.
     *
     *  Note: This is equal to no. of DMA descriptors and half of total PBUF packets
     *        kept in free reserve. This is done to ensure that every submitRxPkt call
     *        after EnetDma_retrieveRxPkts has empty PBUF packets available. */
    EnetDma_Pkt pktInfoMem[LWIP2ENET_RX_PACKETS + LWIP2ENET_TX_PACKETS];

    /*! Queue that holds packets ready to be given to the hardware */
    pbufQ rxFreePbufPktQ;

    /*!
     * \brief       DMA Rx free packet info queue (holds packets returned from the
     *              hardware.
     */
    EnetDma_PktQ rxFreePktInfoQ;

    /*
     * Handle to Rx task, whose job it is to receive packets used by the hardware
     * and give them to the stack, and return freed packets back to the hardware.
     */
    TaskP_Object rxPacketTaskObj;

    /*
     * Handle to Rx semaphore, on which the rxPacketTaskObj awaits for notification
     * of used packets available.
     */
    SemaphoreP_Object rxPacketSemObj;

    /*
     * Clock handle for triggering the packet Rx notify
     */
    ClockP_Object pacingClkObj;

    /*
     * Clock handle for triggering the packet Rx notify
     */
    ClockP_Object pollLinkClkObj;

    /*!
     * \brief       DMA free queue (holds free hardware packets awaiting)
     */
    EnetDma_PktQ txFreePktInfoQ;

    /*! Queue that holds packets ready to be sent to the hardware */
    pbufQ txReadyPbufPktQ;

    /*! Queue that holds packets that were not sent to the hardware in previous submit */
    pbufQ txUnUsedPbufPktQ;

    /*! Handle to Tx task whose job is to retrieve packets consumed by the hardware and
     *  give them to the stack */
    TaskP_Object txPacketTaskObj;

    /*
     * Handle to Tx semaphore, on which the txPacketTaskObj awaits for notification
     * of used packets available.
     */
    SemaphoreP_Object txPacketSemObj;

    /*
     * Handle to counting shutdown semaphore, which all subtasks created in the
     * open function must post before the close operation can complete.
     */
    SemaphoreP_Object shutDownSemObj;


    /*
     * Handle to input task that sends polls the link status
     */
    TaskP_Object lWIPIF2LWIPpollObj;

    /*
     * Handle to Binary Semaphore LWIP_LWIPIF_input when Rx packet queue is ready
     */
    SemaphoreP_Object pollLinkSemObj;

    /** Boolean to indicate shutDownFlag status of translation layer.*/
    volatile bool shutDownFlag;

    /**< Print buffer */
    char printBuf[ENET_CFG_PRINT_BUF_LEN];

    /**< Print Function */
    Enet_Print print;
}
Lwip2Enet_Object, *Lwip2Enet_Handle;

/* ========================================================================== */
/*                         Global Variables Declarations                      */
/* ========================================================================== */

extern Lwip2Enet_Stats gLwip2EnetStats;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */
/*
 * Functions Provided by our translation layer code
 */
extern Lwip2Enet_Handle Lwip2Enet_open(struct netif *netif);

extern void Lwip2Enet_close(Lwip2Enet_Handle hlwip2enet);

extern void Lwip2Enet_setRx(Lwip2Enet_Handle hlwip2enet);

extern void Lwip2Enet_sendTxPackets(Lwip2Enet_Handle hlwip2enet);

extern int32_t Lwip2Enet_ioctl(Lwip2Enet_Handle hlwip2enet,
                              uint32_t cmd,
                              void *param,
                              uint32_t size);

extern void Lwip2Enet_poll(Lwip2Enet_Handle hlwip2enet,
                          uint32_t fTimerTick);

extern void Lwip2Enet_periodicFxn(Lwip2Enet_Handle hLwip2Enet);

/* ========================================================================== */
/*                        Deprecated Function Declarations                    */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                       Static Function Definitions                          */
/* ========================================================================== */

static inline void Lwip2EnetStats_addOne(uint32_t *statCnt)
{
#if defined(LWIPIF_INSTRUMENTATION_ENABLED)
    *statCnt += 1U;
#endif
}

static inline void Lwip2EnetStats_addNum(uint32_t *statCnt,
                                         uint32_t addCnt)
{
#if defined(LWIPIF_INSTRUMENTATION_ENABLED)
    *statCnt += addCnt;
#endif
}

static inline void Lwip2Enet_assert(bool cond)
{
    volatile static bool gCpswAssertWaitInLoop = TRUE;

    if (!(cond))
    {
        void EnetAppUtils_print(const char *pcString,...);
        EnetAppUtils_print("Assertion @ Line: %d in %s : failed !!!\r\n",
                            (int32_t) __LINE__, (const char *) __FILE__);
        while (gCpswAssertWaitInLoop)
        {
        }
    }
}

#ifdef __cplusplus
}
#endif

#endif /* LWIP2ENET_H_ */
