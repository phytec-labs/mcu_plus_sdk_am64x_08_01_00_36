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
 * \file  enet_layer2_icssg.c
 *
 * \brief This file contains the implementation of the Enet layer2 Icssg example.
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <include/core/enet_osal.h>
#include <kernel/dpl/TaskP.h>
#include <kernel/dpl/ClockP.h>
#include <kernel/dpl/SemaphoreP.h>
#include <drivers/sciclient.h>
#include <drivers/udma/udma_priv.h>
#include <enet.h>
#include <enet_cfg.h>
#include <include/core/enet_dma.h>
#include <include/per/icssg.h>
#include <include/per/cpsw.h>
#include <enet_apputils.h>
#include <enet_appmemutils.h>
#include <enet_appmemutils_cfg.h>
#include <enet_appboardutils.h>
#include <enet_board_cfg.h>
/* SDK includes */
#include "ti_drivers_open_close.h"
#include "ti_board_open_close.h"

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* Max number of ports supported per context */
#define ENETMP_PORT_MAX                          (2U)

/* Max number of hardware RX channels. Note that this is different than Enet LLD's
 * RX channel concept which maps to UDMA hardware RX flows */
#define ENETMP_HW_RXCH_MAX                       (2U)

/* Local flag to disable a peripheral from test list */
#define ENETMP_DISABLED                          (0U)

/* Max number of ports that can be tested with this app */
#define ENETMP_CPSW_INSTANCE_MAX                 (1U)
#define ENETMP_ICSSG_INSTANCE_MAX                (2U)
#define ENETMP_PER_MAX                           (ENETMP_ICSSG_INSTANCE_MAX)

/* Task stack size */
#define ENETMP_TASK_STACK_SZ                     (10U * 1024U)

/* 100-ms periodic tick */
#define ENETMP_PERIODIC_TICK_MS                  (100U)

/*Counting Semaphore count*/
#define COUNTING_SEM_COUNT                       (10U)

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */
/* Test parameters for each port in the multiport test */
typedef struct EnetMp_TestParams_s
{
    /* Peripheral type */
    Enet_Type enetType;

    /* Peripheral instance */
    uint32_t instId;

    /* Peripheral's MAC ports to use */
    Enet_MacPort macPort[ENETMP_PORT_MAX];

    /* Number of MAC ports in macPorts array */
    uint32_t macPortNum;

    /* Name of this port to be used for logging */
    char *name;
} EnetMp_TestParams;

/* Context of a peripheral/port */
typedef struct EnetMp_PerCtxt_s
{
    /* Peripheral type */
    Enet_Type enetType;

    /* Peripheral instance */
    uint32_t instId;

    /* Peripheral's MAC ports to use */
    Enet_MacPort macPort[ENETMP_PORT_MAX];

    /* Number of MAC ports in macPorts array */
    uint32_t macPortNum;

    /* Name of this port to be used for logging */
    char *name;

    /* Enet driver handle for this peripheral type/instance */
    Enet_Handle hEnet;

    /* ICSSG configuration */
    Icssg_Cfg icssgCfg;

    /* CPSW configuration */
    Cpsw_Cfg cpswCfg;

    /* MAC address. It's port's MAC address in Dual-MAC or
     * host port's MAC addres in Switch */
    uint8_t macAddr[ENET_MAC_ADDR_LEN];

    /* UDMA driver configuration */
    EnetUdma_Cfg dmaCfg;

    /* TX channel number */
    uint32_t txChNum;

    /* TX channel handle */
    EnetDma_TxChHandle hTxCh;

    /* Start flow index */
    uint32_t rxStartFlowIdx[ENETMP_HW_RXCH_MAX];

    /* Flow index */
    uint32_t rxFlowIdx[ENETMP_HW_RXCH_MAX];

    /* RX channel handle */
    EnetDma_RxChHandle hRxCh[ENETMP_HW_RXCH_MAX];

    /* Number of RX channels in hardware. This value is 1 for all peripherals,
     * except for ICSSG Switch where there are two UDMA RX channels */
    uint32_t numHwRxCh;

    /* RX task handle - receives packets, changes source/dest MAC addresses
     * and transmits the packets back */
    TaskP_Object rxTaskObj;

    /* Semaphore posted from RX callback when packets have arrived */
    SemaphoreP_Object rxSemObj;

    /* Semaphore used to synchronize all RX tasks exits */
    SemaphoreP_Object rxDoneSemObj;

    /* Semaphore posted from event callback upon asynchronous IOCTL completion */
    SemaphoreP_Object ayncIoctlSemObj;

    /* Timestamp of the last received packets */
    uint64_t rxTs[ENET_MEM_NUM_RX_PKTS];

    /* Timestamp of the last transmitted packets */
    uint64_t txTs[ENET_MEM_NUM_RX_PKTS];

    /* Sequence number used as cookie for timestamp events. This value is passed
     * to the DMA packet when submitting a packet for transmission. Driver will
     * pass this same value when timestamp for that packet is ready. */
    uint32_t txTsSeqId;

    /* Semaphore posted from event callback upon asynchronous IOCTL completion */
    SemaphoreP_Object txTsSemObj;
} EnetMp_PerCtxt;

typedef struct EnetMp_Obj_s
{
    /* Flag which indicates if test shall run */
    volatile bool run;

    /* This core's id */
    uint32_t coreId;

    /* Core key returned by Enet RM after attaching this core */
    uint32_t coreKey;

    /* Main UDMA driver handle */
    Udma_DrvHandle hMainUdmaDrv;

    /* Queue of free TX packets */
    EnetDma_PktQ txFreePktInfoQ;

    /* Periodic tick timer - used only to post a semaphore */
    ClockP_Object tickTimerObj;

    /* Periodic tick task - Enet period tick needs to be called from task context
     * hence it's called from this task (i.e. as opposed to in timer callback) */
    TaskP_Object tickTaskObj;

    /* Semaphore posted by tick timer to run tick task */
    SemaphoreP_Object timerSemObj;

    /* Array of all peripheral/port contexts used in the test */
    EnetMp_PerCtxt perCtxt[ENETMP_PER_MAX];

    /* Number of active contexts being used */
    uint32_t numPerCtxts;

    /* Whether promiscuous mode is enabled or not */
    bool promisc;

    /* Whether timestamp are enabled or not */
    bool enableTs;
} EnetMp_Obj;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

void EnetMp_mainTask(void *args);

static int32_t EnetMp_init(void);

static void EnetMp_deinit(void);

static int32_t EnetMp_open(EnetMp_PerCtxt *perCtxts,
                           uint32_t numPerCtxts);

static void EnetMp_close(EnetMp_PerCtxt *perCtxts,
                         uint32_t numPerCtxts);

static int32_t EnetMp_openPort(EnetMp_PerCtxt *perCtxt);

static void EnetMp_closePort(EnetMp_PerCtxt *perCtxt);

static void EnetMp_togglePromisc(EnetMp_PerCtxt *perCtxts,
                                 uint32_t numPerCtxts);

static void EnetMp_printStats(EnetMp_PerCtxt *perCtxts,
                              uint32_t numPerCtxts);

static void EnetMp_resetStats(EnetMp_PerCtxt *perCtxts,
                              uint32_t numPerCtxts);

static void EnetMp_showMacAddrs(EnetMp_PerCtxt *perCtxts,
                                uint32_t numPerCtxts);

static int32_t EnetMp_waitForLinkUp(EnetMp_PerCtxt *perCtxt);

static void EnetMp_macMode2MacMii(emac_mode macMode,
                                  EnetMacPort_Interface *mii);

static void EnetMp_createClock(void);

static void EnetMp_deleteClock(void);

static void EnetMp_timerCallback(ClockP_Object *clkInst, void * arg);

static void EnetMp_tickTask(void *args);

static int32_t EnetMp_openDma(EnetMp_PerCtxt *perCtxt);

static void EnetMp_closeDma(EnetMp_PerCtxt *perCtxt);

static void EnetMp_initTxFreePktQ(void);

static void EnetMp_initRxReadyPktQ(EnetDma_RxChHandle hRxCh);

static uint32_t EnetMp_retrieveFreeTxPkts(EnetMp_PerCtxt *perCtxt);

static void EnetMp_createRxTask(EnetMp_PerCtxt *perCtxt,
                                uint8_t *taskStack,
                                uint32_t taskStackSize);

static void EnetMp_destroyRxTask(EnetMp_PerCtxt *perCtxt);

static void EnetMp_rxTask(void *args);

extern uint32_t Board_getPhyAddr(void);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/* Enet multiport test object */
EnetMp_Obj gEnetMp;

/* Statistics */
IcssgStats_MacPort gEnetMp_icssgStats;
IcssgStats_Pa gEnetMp_icssgPaStats;
CpswStats_PortStats gEnetMp_cpswStats;

/* Test application stack */
static uint8_t gEnetMpTaskStackTick[ENETMP_TASK_STACK_SZ] __attribute__ ((aligned(32)));
static uint8_t gEnetMpTaskStackRx[ENETMP_PER_MAX][ENETMP_TASK_STACK_SZ] __attribute__ ((aligned(32)));

extern Icssg_FwPoolMem gEnetSoc_Icssg1_1_FwPoolMem[];
extern Icssg_FwPoolMem gEnetSoc_Icssg1_Swt_FwPoolMem[];

// #define ENET_TEST_MII_MODE
// #define DUAL_MAC_MODE  /* TODO: Need to allocate TX channels as 2 in enet_cfg.h file to get both MAC ports work simultaneously*/
/* Use this array to select the ports that will be used in the test */
static EnetMp_TestParams testParams[] =
{
#if defined(DUAL_MAC_MODE)
    { ENET_ICSSG_DUALMAC, 2U, { ENET_MAC_PORT_1 }, 1U, "icssg1-p1", },
    { ENET_ICSSG_DUALMAC, 3U, { ENET_MAC_PORT_1 }, 1U, "icssg1-p2", },
#else
    { ENET_ICSSG_SWITCH, 1U, { ENET_MAC_PORT_1, ENET_MAC_PORT_2 }, 2U, "icssg1", },
#endif
};

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

Icssg_FwPoolMem* EnetCb_getFwPoolMem(Enet_Type enetType, uint32_t instId)
{
#if defined(DUAL_MAC_MODE)
    EnetAppUtils_assert(ENET_ICSSG_DUALMAC == enetType);
    return ((Icssg_FwPoolMem*)&gEnetSoc_Icssg1_1_FwPoolMem);
#else
    EnetAppUtils_assert((ENET_ICSSG_SWITCH == enetType) && (1U == instId));
    return ((Icssg_FwPoolMem*)&gEnetSoc_Icssg1_Swt_FwPoolMem);
#endif

}

static void EnetMp_showMenu(void)
{
    EnetAppUtils_print("\nEnet Multiport Menu:\r\n");
    EnetAppUtils_print(" 's'  -  Print statistics\r\n");
    EnetAppUtils_print(" 'r'  -  Reset statistics\r\n");
    EnetAppUtils_print(" 'm'  -  Show allocated MAC addresses\r\n");
    EnetAppUtils_print(" 'x'  -  Stop the test\r\n\n");
}

void EnetMp_mainTask(void *args)
{
    char option;
    uint32_t i, j;
    int32_t status;

    Drivers_open();
    Board_driversOpen();

    DebugP_log("==========================\r\n");
    DebugP_log("      MULTIPORT TEST      \r\n");
    DebugP_log("==========================\r\n");

    /* Initialize test config */
    memset(&gEnetMp, 0, sizeof(gEnetMp));
    gEnetMp.run = true;
    gEnetMp.promisc = false;
    gEnetMp.enableTs = false;

    gEnetMp.numPerCtxts = ENET_ARRAYSIZE(testParams);

    for (i = 0U; i < gEnetMp.numPerCtxts; i++)
    {
        gEnetMp.perCtxt[i].enetType = testParams[i].enetType;
        gEnetMp.perCtxt[i].instId   = testParams[i].instId;
        gEnetMp.perCtxt[i].name     = testParams[i].name; /* shallow copy */

        gEnetMp.perCtxt[i].macPortNum = testParams[i].macPortNum;
        for (j = 0; j < gEnetMp.perCtxt[i].macPortNum; j++)
        {
            gEnetMp.perCtxt[i].macPort[j]  = testParams[i].macPort[j];
        }
    }

    /* Init driver */
    status = EnetMp_init();
    if (status != ENET_SOK)
    {
        EnetAppUtils_print("Failed to initialize multiport test: %d\r\n", status);
    }

    /* Open all peripherals */
    if (status == ENET_SOK)
    {
        status = EnetMp_open(gEnetMp.perCtxt, gEnetMp.numPerCtxts);
        if (status != ENET_SOK)
        {
            EnetAppUtils_print("Failed to open peripherals: %d\r\n", status);
        }
    }

    if (status == ENET_SOK)
    {
        /* Start periodic tick timer */
        ClockP_start(&gEnetMp.tickTimerObj);

        /* Wait for user input to exit the test */
        EnetMp_showMenu();
        while (true)
        {
            option = ' ';
            status = DebugP_scanf("%c", &option);
            if (option == 'x')
            {
                EnetAppUtils_print("Stopping...\r\n");
                gEnetMp.run = false;
                break;
            }
            else if (option == 't')
            {
                EnetAppUtils_print("Disable timestamp prints\r\n");
                gEnetMp.enableTs = false;
            }
            else if (option == 'T')
            {
                EnetAppUtils_print("Enable timestamp prints\r\n");
                gEnetMp.enableTs = true;
            }
            else if (option == 'p')
            {
                EnetMp_togglePromisc(gEnetMp.perCtxt, gEnetMp.numPerCtxts);
            }
            else if (option == 's')
            {
                EnetMp_printStats(gEnetMp.perCtxt, gEnetMp.numPerCtxts);
            }
            else if (option == 'r')
            {
                EnetMp_resetStats(gEnetMp.perCtxt, gEnetMp.numPerCtxts);
            }
            else if (option == 'm')
            {
                EnetMp_showMacAddrs(gEnetMp.perCtxt, gEnetMp.numPerCtxts);
            }
            else
            {
                EnetAppUtils_print("Invalid option, try again...\r\n");
                EnetMp_showMenu();
            }
            TaskP_yield();
        }

        /* Print statistics */
        EnetMp_printStats(gEnetMp.perCtxt, gEnetMp.numPerCtxts);

        /* Wait until RX tasks have exited */
        for (i = 0U; i < gEnetMp.numPerCtxts; i++)
        {
            EnetAppUtils_print("Waiting for RX task %u to exit\r\n", i+1);
            SemaphoreP_post(&gEnetMp.perCtxt[i].rxSemObj);
            SemaphoreP_pend(&gEnetMp.perCtxt[i].rxDoneSemObj, SystemP_WAIT_FOREVER);
        }

        EnetAppUtils_print("All RX tasks have exited\r\n");
        /* Stop periodic tick timer */
        ClockP_stop(&gEnetMp.tickTimerObj);
    }

    /* Close all peripherals */
    EnetMp_close(gEnetMp.perCtxt, gEnetMp.numPerCtxts);

    /* Deinit driver */
    EnetMp_deinit();
}

static int32_t EnetMp_init(void)
{
    EnetOsal_Cfg osalCfg;
    EnetUtils_Cfg utilsCfg;
    int32_t status = ENET_SOK;

    /* Initialize Enet driver (use default OSAL and utils) */
    EnetAppUtils_print("Init Enet's OSAL and utils to use defaults\r\n");
    Enet_initOsalCfg(&osalCfg);
    Enet_initUtilsCfg(&utilsCfg);
    Enet_init(&osalCfg, &utilsCfg);

    gEnetMp.coreId = EnetSoc_getCoreId();

    /* Initialize memory */
    EnetAppUtils_print("Init memory utils\r\n");
    status = EnetMem_init();
    if (status != ENET_SOK)
    {
        EnetAppUtils_print("Failed to initialize memory utils: %d\r\n", status);
        EnetAppUtils_assert(false);
    }

    /* Initialize all queues */
    EnetQueue_initQ(&gEnetMp.txFreePktInfoQ);

    /* Create task, clock and semaphore used for periodic tick */
    if (status == ENET_SOK)
    {
        EnetAppUtils_print("Create clock and task for periodic tick\r\n");
        EnetMp_createClock();
    }

    /* Open UDMA driver which is the same handle to be used for all peripherals */
    if (status == ENET_SOK)
    {
        EnetAppUtils_print("Open Main UDMA driver\r\n");
        gEnetMp.hMainUdmaDrv = EnetAppUtils_udmaOpen(ENET_ICSSG_DUALMAC, NULL);
        if (gEnetMp.hMainUdmaDrv == NULL)
        {
            EnetAppUtils_print("Failed to open Main UDMA driver: %d\r\n", status);
            status = ENET_EALLOC;
            EnetAppUtils_assert(false);
        }
    }

    return status;
}

static void EnetMp_asyncIoctlCb(Enet_Event evt,
                                uint32_t evtNum,
                                void *evtCbArgs,
                                void *arg1,
                                void *arg2)
{
    EnetMp_PerCtxt *perCtxt = (EnetMp_PerCtxt *)evtCbArgs;

    EnetAppUtils_print("%s: Async IOCTL completed\r\n", perCtxt->name);
    SemaphoreP_post(&perCtxt->ayncIoctlSemObj);
}

static void EnetMp_txTsCb(Enet_Event evt,
                          uint32_t evtNum,
                          void *evtCbArgs,
                          void *arg1,
                          void *arg2)
{
    EnetMp_PerCtxt *perCtxt = (EnetMp_PerCtxt *)evtCbArgs;
    Icssg_TxTsEvtCbInfo *txTsInfo = (Icssg_TxTsEvtCbInfo *)arg1;
    Enet_MacPort macPort = *(Enet_MacPort *)arg2;
    uint32_t tsId = txTsInfo->txTsId;
    uint64_t txTs = txTsInfo->ts;
    uint64_t rxTs = perCtxt->rxTs[tsId % ENET_MEM_NUM_RX_PKTS];
    uint64_t prevTs;
    int64_t dt;
    bool status = true;

    dt = txTs - rxTs;

    EnetAppUtils_print("%s: Port %u: RX-to-TX timestamp delta = %10lld (RX=%llu, TX=%llu)\r\n",
                       perCtxt->name, ENET_MACPORT_ID(macPort), dt, rxTs, txTs);

    /* Check correct timestamp delta */
    if (dt < 0)
    {
        EnetAppUtils_print("%s: Port %u: ERROR: RX timestamp > TX timestamp: %llu > %llu\r\n",
                           perCtxt->name, ENET_MACPORT_ID(macPort), rxTs, txTs);
            status = false;
    }

    /* Check monotonicity of the TX and RX timestamps */
    if (txTsInfo->txTsId > 0U)
    {
        prevTs = perCtxt->rxTs[(tsId - 1) % ENET_MEM_NUM_RX_PKTS];
        if (prevTs > rxTs)
        {
            EnetAppUtils_print("%s: Port %u: ERROR: Non monotonic RX timestamp: %llu -> %llu\r\n",
                               perCtxt->name, ENET_MACPORT_ID(macPort), prevTs, rxTs);
            status = false;
        }

        prevTs = perCtxt->txTs[(tsId - 1) % ENET_MEM_NUM_RX_PKTS];
        if (prevTs > txTs)
        {
            EnetAppUtils_print("%s: Port %u: ERROR: Non monotonic TX timestamp: %llu -> %llu\r\n",
                               perCtxt->name, ENET_MACPORT_ID(macPort), prevTs, txTs);
            status = false;
        }
    }

    if (!status)
    {
        EnetAppUtils_print("\r\n");
    }

    /* Save current timestamp for future monotonicity checks */
     perCtxt->txTs[txTsInfo->txTsId % ENET_MEM_NUM_RX_PKTS] = txTs;

    SemaphoreP_post(&perCtxt->txTsSemObj);
}

static void EnetMp_deinit(void)
{
    /* Open UDMA driver which is the same handle to be used for all peripherals */
    EnetAppUtils_print("Close UDMA driver\r\n");
    EnetAppUtils_udmaclose(gEnetMp.hMainUdmaDrv);

    /* Initialize Enet driver (use default OSAL and utils) */
    EnetAppUtils_print("Deinit Enet driver\r\n");
    Enet_deinit();

    /* Delete task, clock and semaphore used for periodic tick */
    EnetAppUtils_print("Delete clock and task for periodic tick\r\n");
    EnetMp_deleteClock();

    /* Deinitialize memory */
    EnetAppUtils_print("Deinit memory utils\r\n");
    EnetMem_deInit();

    EnetAppUtils_print("Deinit complete\r\n");

}

static void EnetMp_portLinkStatusChangeCb(Enet_MacPort macPort,
                                          bool isLinkUp,
                                          void *appArg)
{
    EnetAppUtils_print("MAC Port %u: link %s\r\n",
                       ENET_MACPORT_ID(macPort), isLinkUp ? "up" : "down");
}

static void EnetMp_mdioLinkStatusChange(Cpsw_MdioLinkStateChangeInfo *info,
                                             void *appArg)
{
    static uint32_t linkUpCount = 0;
    if ((info->linkChanged) && (info->isLinked))
    {
        linkUpCount++;
    }
}

static void EnetMp_initEnetLinkCbPrms(Cpsw_Cfg *cpswCfg)
{
    cpswCfg->mdioLinkStateChangeCb     = EnetMp_mdioLinkStatusChange;
    cpswCfg->mdioLinkStateChangeCbArg  = &gEnetMp;
    cpswCfg->portLinkStatusChangeCb    = &EnetMp_portLinkStatusChangeCb;
    cpswCfg->portLinkStatusChangeCbArg = &gEnetMp;
}

void EnetMp_initAleConfig(CpswAle_Cfg *aleCfg)
{
    int32_t status = ENET_SOK;
    aleCfg->modeFlags = CPSW_ALE_CFG_MODULE_EN;
    aleCfg->agingCfg.autoAgingEn = true;
    aleCfg->agingCfg.agingPeriodInMs = 1000;
    EnetAppUtils_assert(status == ENET_SOK);

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

static int32_t EnetMp_open(EnetMp_PerCtxt *perCtxts,
                           uint32_t numPerCtxts)
{
    EnetUdma_Cfg *dmaCfg;
    EnetRm_ResCfg *resCfg;
    Enet_IoctlPrms prms;
    EnetPer_AttachCoreOutArgs attachCoreOutArgs;
    uint32_t i, j;
    int32_t status = ENET_SOK;

    /* Initialize async IOCTL and TX timestamp semaphores */
    for (i = 0U; i < numPerCtxts; i++)
    {
        EnetMp_PerCtxt *perCtxt = &perCtxts[i];

        status = SemaphoreP_constructBinary(&perCtxt->ayncIoctlSemObj, 0);
        DebugP_assert(SystemP_SUCCESS == status);

        status = SemaphoreP_constructBinary(&perCtxt->txTsSemObj, 0);
        DebugP_assert(SystemP_SUCCESS == status);
    }

    /* Do peripheral dependent initalization */
    EnetAppUtils_print("\nInit all peripheral clocks\r\n");
    EnetAppUtils_print("----------------------------------------------\r\n");
    for (i = 0U; i < numPerCtxts; i++)
    {
        EnetMp_PerCtxt *perCtxt = &perCtxts[i];
        EnetAppUtils_enableClocks(perCtxt->enetType, perCtxt->instId);
    }

    /* Prepare init configuration for all peripherals */
    EnetAppUtils_print("\nInit all configs\r\n");
    EnetAppUtils_print("----------------------------------------------\r\n");
    for (i = 0U; i < numPerCtxts; i++)
    {
        EnetMp_PerCtxt *perCtxt = &perCtxts[i];

        dmaCfg = &perCtxt->dmaCfg;
        dmaCfg->rxChInitPrms.dmaPriority = UDMA_DEFAULT_RX_CH_DMA_PRIORITY;
        EnetAppUtils_print("%s: init config\r\n", perCtxt->name);
        if (Enet_isCpswFamily(perCtxt->enetType))
        {
            Cpsw_Cfg *cpswCfg = &perCtxt->cpswCfg;
            Enet_initCfg(perCtxt->enetType, perCtxt->instId, cpswCfg, sizeof(*cpswCfg));
            cpswCfg->vlanCfg.vlanAware          = false;
            cpswCfg->hostPortCfg.removeCrc      = true;
            cpswCfg->hostPortCfg.padShortPacket = true;
            cpswCfg->hostPortCfg.passCrcErrors  = true;
            EnetMp_initEnetLinkCbPrms(cpswCfg);
            resCfg = &cpswCfg->resCfg;
            EnetMp_initAleConfig(&cpswCfg->aleCfg);
            /* Set DMA configuration */
            dmaCfg->hUdmaDrv = gEnetMp.hMainUdmaDrv;
            cpswCfg->dmaCfg = (void *)dmaCfg;
        }
        else
        {
            Icssg_Cfg *icssgCfg = &perCtxt->icssgCfg;

            Enet_initCfg(perCtxt->enetType, perCtxt->instId, icssgCfg, sizeof(*icssgCfg));
#if defined(ENET_TEST_MII_MODE)
            icssgCfg->mii.layerType    = ENET_MAC_LAYER_MII;
            icssgCfg->mii.sublayerType = ENET_MAC_SUBLAYER_STANDARD;
            icssgCfg->mii.variantType  = ENET_MAC_VARIANT_NONE;
#endif
            resCfg = &icssgCfg->resCfg;

            /* Set DMA configuration */
            dmaCfg->hUdmaDrv = gEnetMp.hMainUdmaDrv;
            icssgCfg->dmaCfg = (void *)dmaCfg;
        }

        /* Initialize RM */
        EnetAppUtils_initResourceConfig(perCtxt->enetType, EnetSoc_getCoreId(), resCfg);

        /* We use software MAC address pool from apputils, but it will give same MAC address.
         * Add port index to make them unique */
        for (j = 0U; j < ENETMP_PORT_MAX; j++)
        {
            resCfg->macList.macAddress[j][ENET_MAC_ADDR_LEN - 1] += (i * ENETMP_PORT_MAX) + j;
        }
        resCfg->macList.numMacAddress = ENETMP_PORT_MAX;

    }

    /* Open Enet driver for all peripherals */
    EnetAppUtils_print("\nOpen all peripherals\r\n");
    EnetAppUtils_print("----------------------------------------------\r\n");
    for (i = 0U; i < numPerCtxts; i++)
    {
        EnetMp_PerCtxt *perCtxt = &perCtxts[i];
        EnetAppUtils_print("%s: Open enet\r\n", perCtxt->name);
        if (Enet_isCpswFamily(perCtxt->enetType))
        {
            Cpsw_Cfg *cpswCfg = &perCtxt->cpswCfg;
            perCtxt->hEnet = Enet_open(perCtxt->enetType, perCtxt->instId, cpswCfg, sizeof(*cpswCfg));
        }
        else
        {
            Icssg_Cfg *icssgCfg = &perCtxt->icssgCfg;
            perCtxt->hEnet = Enet_open(perCtxt->enetType, perCtxt->instId, icssgCfg, sizeof(*icssgCfg));
        }

        if (perCtxt->hEnet == NULL)
        {
            EnetAppUtils_print("%s: failed to open enet\r\n", perCtxt->name);
            status = ENET_EFAIL;
            break;
        }

        if (Enet_isIcssFamily(perCtxt->enetType))
        {
            EnetAppUtils_print("%s: Register async IOCTL callback\r\n", perCtxt->name);
            Enet_registerEventCb(perCtxt->hEnet,
                                 ENET_EVT_ASYNC_CMD_RESP,
                                 0U,
                                 EnetMp_asyncIoctlCb,
                                 (void *)perCtxt);

            EnetAppUtils_print("%s: Register TX timestamp callback\r\n", perCtxt->name);
            perCtxt->txTsSeqId = 0U;
            Enet_registerEventCb(perCtxt->hEnet,
                                 ENET_EVT_TIMESTAMP_TX,
                                 0U,
                                 EnetMp_txTsCb,
                                 (void *)perCtxt);
        }
    }

    /* Attach the core with RM */
    if (status == ENET_SOK)
    {
        EnetAppUtils_print("\nAttach core id %u on all peripherals\r\n", gEnetMp.coreId);
        EnetAppUtils_print("----------------------------------------------\r\n");
        for (i = 0U; i < numPerCtxts; i++)
        {
            EnetMp_PerCtxt *perCtxt = &perCtxts[i];

            EnetAppUtils_print("%s: Attach core\r\n", perCtxt->name);

            ENET_IOCTL_SET_INOUT_ARGS(&prms, &gEnetMp.coreId, &attachCoreOutArgs);

            status = Enet_ioctl(perCtxt->hEnet, gEnetMp.coreId, ENET_PER_IOCTL_ATTACH_CORE, &prms);
            if (status != ENET_SOK)
            {
                EnetAppUtils_print("%s: failed to attach: %d\r\n", perCtxt->name, status);
            }
            else
            {
                gEnetMp.coreKey = attachCoreOutArgs.coreKey;
            }
        }
    }

    /* Create RX tasks for each peripheral */
    if (status == ENET_SOK)
    {
        EnetAppUtils_print("\nCreate RX tasks\r\n");
        EnetAppUtils_print("----------------------------------------------\r\n");
        for (i = 0U; i < numPerCtxts; i++)
        {
            EnetMp_PerCtxt *perCtxt = &perCtxts[i];

            EnetAppUtils_print("%s: Create RX task\r\n", perCtxt->name);

            EnetMp_createRxTask(perCtxt, &gEnetMpTaskStackRx[i][0U], sizeof(gEnetMpTaskStackRx[i]));
        }
    }

    return status;
}

static void EnetMp_close(EnetMp_PerCtxt *perCtxts,
                         uint32_t numPerCtxts)
{
    Enet_IoctlPrms prms;
    uint32_t i;
    int32_t status;

    EnetAppUtils_print("\nClose DMA for all peripherals\r\n");
    EnetAppUtils_print("----------------------------------------------\r\n");
    for (i = 0U; i < numPerCtxts; i++)
    {
        EnetMp_PerCtxt *perCtxt = &perCtxts[i];

        EnetAppUtils_print("%s: Close DMA\r\n", perCtxt->name);

        EnetMp_closeDma(perCtxt);
    }

    /* Delete RX tasks created for all peripherals */
    EnetAppUtils_print("\nDelete RX tasks\r\n");
    EnetAppUtils_print("----------------------------------------------\r\n");
    for (i = 0U; i < numPerCtxts; i++)
    {
        EnetMp_destroyRxTask(&perCtxts[i]);
    }

    /* Detach core */
    EnetAppUtils_print("\nDetach core from all peripherals\r\n");
    EnetAppUtils_print("----------------------------------------------\r\n");
    for (i = 0U; i < numPerCtxts; i++)
    {
        EnetMp_PerCtxt *perCtxt = &perCtxts[i];

        EnetAppUtils_print("%s: Detach core\r\n", perCtxt->name);

        ENET_IOCTL_SET_IN_ARGS(&prms, &gEnetMp.coreKey);
        status = Enet_ioctl(perCtxt->hEnet, gEnetMp.coreId, ENET_PER_IOCTL_DETACH_CORE, &prms);
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
        EnetMp_PerCtxt *perCtxt = &perCtxts[i];
        EnetAppUtils_print("%s: Close enet\r\n", perCtxt->name);

        if (Enet_isIcssFamily(perCtxt->enetType))
        {
            EnetAppUtils_print("%s: Unregister async IOCTL callback\r\n", perCtxt->name);
            Enet_unregisterEventCb(perCtxt->hEnet,
                                   ENET_EVT_ASYNC_CMD_RESP,
                                   0U);

            EnetAppUtils_print("%s: Unregister TX timestamp callback\r\n", perCtxt->name);
            Enet_unregisterEventCb(perCtxt->hEnet,
                                   ENET_EVT_TIMESTAMP_TX,
                                   0U);
        }

        Enet_close(perCtxt->hEnet);
        perCtxt->hEnet = NULL;
    }

    /* Do peripheral dependent initalization */
    EnetAppUtils_print("\nDeinit all peripheral clocks\r\n");
    EnetAppUtils_print("----------------------------------------------\r\n");
    for (i = 0U; i < numPerCtxts; i++)
    {
        EnetMp_PerCtxt *perCtxt = &perCtxts[i];
        EnetAppUtils_disableClocks(perCtxt->enetType, perCtxt->instId);
    }
    /* Destroy async IOCTL semaphore */
    for (i = 0U; i < numPerCtxts; i++)
    {
        EnetMp_PerCtxt *perCtxt = &perCtxts[i];

        SemaphoreP_destruct(&perCtxt->ayncIoctlSemObj);
    }
}

static void EnetMp_togglePromisc(EnetMp_PerCtxt *perCtxts,
                                 uint32_t numPerCtxts)
{
    Enet_IoctlPrms prms;
    Enet_MacPort macPort;
    uint32_t cmd;
    uint32_t i;
    uint32_t j;
    int32_t status;

    gEnetMp.promisc = !gEnetMp.promisc;
    EnetAppUtils_print("\n%s promiscuous mode\r\n", gEnetMp.promisc ? "Enable" : "Disable");

    for (i = 0U; i < numPerCtxts; i++)
    {
        EnetMp_PerCtxt *perCtxt = &gEnetMp.perCtxt[i];

        /* Promiscuous test in this app not implemented for CPSW, only for ICSSG */
        if (Enet_isIcssFamily(perCtxt->enetType))
        {
            cmd = gEnetMp.promisc ? ICSSG_MACPORT_IOCTL_ENABLE_PROMISC_MODE :
                                    ICSSG_MACPORT_IOCTL_DISABLE_PROMISC_MODE;

            for (j = 0U; j < perCtxt->macPortNum; j++)
            {
                macPort = perCtxt->macPort[j];
                ENET_IOCTL_SET_IN_ARGS(&prms, &macPort);

                status = Enet_ioctl(perCtxt->hEnet, gEnetMp.coreId, cmd, &prms);
                if (status != ENET_SOK)
                {
                    EnetAppUtils_print("%s: Failed to set promisc mode: %d\r\n",
                                       perCtxt->name, status);
                    continue;
                }
            }
        }
    }
}

static void EnetMp_printStats(EnetMp_PerCtxt *perCtxts,
                              uint32_t numPerCtxts)
{
    Enet_IoctlPrms prms;
    Enet_MacPort macPort;
    uint32_t i;
    uint32_t j;
    int32_t status;

    EnetAppUtils_print("\nPrint statistics\r\n");
    EnetAppUtils_print("----------------------------------------------\r\n");
    for (i = 0U; i < numPerCtxts; i++)
    {
        EnetMp_PerCtxt *perCtxt = &gEnetMp.perCtxt[i];

        if (Enet_isIcssFamily(perCtxt->enetType))
        {
            EnetAppUtils_print("\n %s - PA statistics\r\n", perCtxt->name);
            EnetAppUtils_print("--------------------------------\r\n");
            ENET_IOCTL_SET_OUT_ARGS(&prms, &gEnetMp_icssgPaStats);
            status = Enet_ioctl(perCtxt->hEnet, gEnetMp.coreId, ENET_STATS_IOCTL_GET_HOSTPORT_STATS, &prms);
            if (status != ENET_SOK)
            {
                EnetAppUtils_print("%s: Failed to get PA stats\r\n", perCtxt->name);
            }

            EnetAppUtils_printIcssgPaStats(&gEnetMp_icssgPaStats);
            EnetAppUtils_print("\r\n");
        }

        for (j = 0U; j < perCtxt->macPortNum; j++)
        {
            macPort = perCtxt->macPort[j];

            EnetAppUtils_print("\n %s - Port %u statistics\r\n", perCtxt->name, ENET_MACPORT_ID(macPort));
            EnetAppUtils_print("--------------------------------\r\n");

            if (Enet_isCpswFamily(perCtxt->enetType))
            {
                ENET_IOCTL_SET_INOUT_ARGS(&prms, &macPort, &gEnetMp_cpswStats);
            }
            else
            {
                ENET_IOCTL_SET_INOUT_ARGS(&prms, &macPort, &gEnetMp_icssgStats);
            }

            status = Enet_ioctl(perCtxt->hEnet, gEnetMp.coreId, ENET_STATS_IOCTL_GET_MACPORT_STATS, &prms);
            if (status != ENET_SOK)
            {
                EnetAppUtils_print("%s: Failed to get port %u stats\r\n", perCtxt->name, ENET_MACPORT_ID(macPort));
                continue;
            }

            if (Enet_isCpswFamily(perCtxt->enetType))
            {
                EnetAppUtils_printMacPortStats9G((CpswStats_MacPort_Ng *)&gEnetMp_cpswStats);
            }
            else
            {
                EnetAppUtils_printIcssgMacPortStats(&gEnetMp_icssgStats, false);
            }

            EnetAppUtils_print("\r\n");
        }
    }
}

static void EnetMp_resetStats(EnetMp_PerCtxt *perCtxts,
                              uint32_t numPerCtxts)
{
    Enet_IoctlPrms prms;
    Enet_MacPort macPort;
    uint32_t i;
    uint32_t j;
    int32_t status;

    EnetAppUtils_print("\nReset statistics\r\n");
    EnetAppUtils_print("----------------------------------------------\r\n");
    for (i = 0U; i < numPerCtxts; i++)
    {
        EnetMp_PerCtxt *perCtxt = &gEnetMp.perCtxt[i];

        EnetAppUtils_print("%s: Reset statistics\r\n", perCtxt->name);

        for (j = 0U; j < perCtxt->macPortNum; j++)
        {
            macPort = perCtxt->macPort[j];

            ENET_IOCTL_SET_IN_ARGS(&prms, &macPort);
            status = Enet_ioctl(perCtxt->hEnet, gEnetMp.coreId, ENET_STATS_IOCTL_RESET_MACPORT_STATS, &prms);
            if (status != ENET_SOK)
            {
                EnetAppUtils_print("%s: Failed to reset port %u stats\r\n", perCtxt->name, ENET_MACPORT_ID(macPort));
                continue;
            }
        }
    }
}

static void EnetMp_showMacAddrs(EnetMp_PerCtxt *perCtxts,
                                uint32_t numPerCtxts)
{
    uint32_t i;

    EnetAppUtils_print("\nAllocated MAC addresses\r\n");
    EnetAppUtils_print("----------------------------------------------\r\n");
    for (i = 0U; i < numPerCtxts; i++)
    {
        EnetMp_PerCtxt *perCtxt = &gEnetMp.perCtxt[i];

        EnetAppUtils_print("%s: \t", perCtxt->name);
        EnetAppUtils_printMacAddr(&perCtxt->macAddr[0U]);
    }
}

static int32_t EnetMp_openPort(EnetMp_PerCtxt *perCtxt)
{
    Enet_IoctlPrms prms;
    EnetBoard_EthPort ethPort;
    const EnetBoard_PhyCfg *boardPhyCfg;
    EnetPer_PortLinkCfg portLinkCfg;
    CpswMacPort_Cfg cpswMacCfg;
    IcssgMacPort_Cfg icssgMacCfg;
    EnetMacPort_LinkCfg *linkCfg = &portLinkCfg.linkCfg;
    EnetMacPort_Interface *mii = &portLinkCfg.mii;
    EnetPhy_Cfg *phyCfg = &portLinkCfg.phyCfg;
    Enet_MacPort macPort;
    uint32_t i;
    int32_t status = ENET_SOK;

    for (i = 0U; i < perCtxt->macPortNum; i++)
    {
        macPort = perCtxt->macPort[i];

        EnetAppUtils_print("%s: Open port %u\r\n", perCtxt->name, ENET_MACPORT_ID(macPort));

        /* Setup board for requested Ethernet port */
        ethPort.enetType = perCtxt->enetType;
        ethPort.instId   = perCtxt->instId;
        ethPort.macPort  = macPort;
#if defined(ENET_TEST_MII_MODE)
        ethPort.boardId  = ENETBOARD_MII_ID;
        ethPort.mii.layerType    = ENET_MAC_LAYER_MII;
        ethPort.mii.sublayerType = ENET_MAC_SUBLAYER_STANDARD;
        ethPort.mii.variantType  = ENET_MAC_VARIANT_NONE;
#else
        ethPort.boardId  = ENETBOARD_AM64X_AM243X_EVM;
        EnetMp_macMode2MacMii(RGMII, &ethPort.mii);
#endif
        status = EnetBoard_setupPorts(&ethPort, 1U);
        if (status != ENET_SOK)
        {
            EnetAppUtils_print("%s: Failed to setup MAC port %u\r\n", perCtxt->name, ENET_MACPORT_ID(macPort));
            EnetAppUtils_assert(false);
        }

        if (Enet_isCpswFamily(perCtxt->enetType))
        {
            CpswMacPort_initCfg(&cpswMacCfg);
            portLinkCfg.macCfg = &cpswMacCfg;
        }
        else
        {
            IcssgMacPort_initCfg(&icssgMacCfg);
            icssgMacCfg.specialFramePrio = 1U;
            portLinkCfg.macCfg = &icssgMacCfg;
        }

        /* Set port link params */
        portLinkCfg.macPort = macPort;

        mii->layerType     = ethPort.mii.layerType;
        mii->sublayerType  = ethPort.mii.sublayerType;
#if defined(ENET_TEST_MII_MODE)
        mii->variantType   = ethPort.mii.variantType;
#else
        mii->variantType   = ENET_MAC_VARIANT_FORCED;
#endif
        linkCfg->speed     = ENET_SPEED_AUTO;
        linkCfg->duplexity = ENET_DUPLEX_AUTO;

        boardPhyCfg = EnetBoard_getPhyCfg(&ethPort);
        if (boardPhyCfg != NULL)
        {
            EnetPhy_initCfg(phyCfg);
            if ((ENET_ICSSG_DUALMAC == perCtxt->enetType) && (2U == perCtxt->instId))
            {
                phyCfg->phyAddr     = CONFIG_ENET_ICSS0_PHY1_ADDR;
            }
            else if ((ENET_ICSSG_DUALMAC == perCtxt->enetType) && (3U == perCtxt->instId))
            {
                phyCfg->phyAddr     = CONFIG_ENET_ICSS0_PHY2_ADDR;
            }
            else if ((ENET_ICSSG_SWITCH == perCtxt->enetType) && (1U == perCtxt->instId))
            {
                if (macPort == ENET_MAC_PORT_1)
                {
                    phyCfg->phyAddr     = CONFIG_ENET_ICSS0_PHY1_ADDR;
                }
                else
                {
                    phyCfg->phyAddr     = CONFIG_ENET_ICSS0_PHY2_ADDR;
                }
            }
            else
            {
                EnetAppUtils_assert(false);
            }

            phyCfg->isStrapped  = boardPhyCfg->isStrapped;
            phyCfg->loopbackEn  = false;
            phyCfg->skipExtendedCfg = boardPhyCfg->skipExtendedCfg;
            phyCfg->extendedCfgSize = boardPhyCfg->extendedCfgSize;
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
            status = Enet_ioctl(perCtxt->hEnet, gEnetMp.coreId, ENET_PER_IOCTL_OPEN_PORT_LINK, &prms);
            if (status != ENET_SOK)
            {
                EnetAppUtils_print("%s: Failed to open port link: %d\r\n", perCtxt->name, status);
            }
        }
    }

    return status;
}

static void EnetMp_closePort(EnetMp_PerCtxt *perCtxt)
{
    Enet_IoctlPrms prms;
    Enet_MacPort macPort;
    uint32_t i;
    int32_t status;

    for (i = 0U; i < perCtxt->macPortNum; i++)
    {
        macPort = perCtxt->macPort[i];

        EnetAppUtils_print("%s: Close port %u\r\n", perCtxt->name, ENET_MACPORT_ID(macPort));

        /* Close port link */
        ENET_IOCTL_SET_IN_ARGS(&prms, &macPort);

        EnetAppUtils_print("%s: Close port %u link\r\n", perCtxt->name, ENET_MACPORT_ID(macPort));
        status = Enet_ioctl(perCtxt->hEnet, gEnetMp.coreId, ENET_PER_IOCTL_CLOSE_PORT_LINK, &prms);
        if (status != ENET_SOK)
        {
            EnetAppUtils_print("%s: Failed to close port link: %d\r\n", perCtxt->name, status);
        }
    }
}

static int32_t EnetMp_waitForLinkUp(EnetMp_PerCtxt *perCtxt)
{
    Enet_IoctlPrms prms;
    Enet_MacPort macPort;
    IcssgMacPort_SetPortStateInArgs setPortStateInArgs;
    bool linked;
    uint32_t i;
    int32_t status = ENET_SOK;

    EnetAppUtils_print("%s: Waiting for link up...\r\n", perCtxt->name);

    for (i = 0U; i < perCtxt->macPortNum; i++)
    {
        macPort = perCtxt->macPort[i];
        linked = false;

        while (gEnetMp.run && !linked)
        {
            ENET_IOCTL_SET_INOUT_ARGS(&prms, &macPort, &linked);

            status = Enet_ioctl(perCtxt->hEnet, gEnetMp.coreId, ENET_PER_IOCTL_IS_PORT_LINK_UP, &prms);
            if (status != ENET_SOK)
            {
                EnetAppUtils_print("%s: Failed to get port %u link status: %d\r\n",
                                   perCtxt->name, ENET_MACPORT_ID(macPort), status);
                linked = false;
                break;
            }

            if (!linked)
            {
                ClockP_usleep(1000);
            }
        }

        if (gEnetMp.run)
        {
            EnetAppUtils_print("%s: Port %u link is %s\r\n",
                               perCtxt->name, ENET_MACPORT_ID(macPort), linked ? "up" : "down");

            /* Set port to 'Forward' state */
            if (status == ENET_SOK)
            {
                if (Enet_isCpswFamily(perCtxt->enetType))
                {
                CpswAle_SetPortStateInArgs setPortStateInArgs;
                setPortStateInArgs.portNum   = CPSW_ALE_HOST_PORT_NUM;
                setPortStateInArgs.portState = CPSW_ALE_PORTSTATE_FORWARD;
                ENET_IOCTL_SET_IN_ARGS(&prms, &setPortStateInArgs);
                prms.outArgs = NULL;
                status = Enet_ioctl(perCtxt->hEnet,
                                    gEnetMp.coreId,
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
                                        gEnetMp.coreId,
                                        ENET_HOSTPORT_IOCTL_ENABLE,
                                        &prms);
                    if (status != ENET_SOK)
                    {
                        EnetAppUtils_print("%s: Failed to enable host port: %d\r\n", perCtxt->name, status);
                        }
                    }
                }
                else
                {
                    EnetAppUtils_print("%s: Set port state to 'Forward'\r\n", perCtxt->name);

                    setPortStateInArgs.macPort   = macPort;
                    setPortStateInArgs.portState = ICSSG_PORT_STATE_FORWARD;
                    ENET_IOCTL_SET_IN_ARGS(&prms, &setPortStateInArgs);

                    status = Enet_ioctl(perCtxt->hEnet, gEnetMp.coreId, ICSSG_PER_IOCTL_SET_PORT_STATE, &prms);
                    if (status == ENET_SINPROGRESS)
                    {
                        /* Wait for asyc ioctl to complete */
                        do
                        {
                            Enet_poll(perCtxt->hEnet, ENET_EVT_ASYNC_CMD_RESP, NULL, 0U);
                            status = SemaphoreP_pend(&perCtxt->ayncIoctlSemObj, SystemP_WAIT_FOREVER);
                            if (SystemP_SUCCESS == status)
                            {
                                break;
                            }
                        } while (1);

                        status = ENET_SOK;
                    }
                    else
                    {
                        EnetAppUtils_print("%s: Failed to set port state: %d\n", perCtxt->name, status);
                    }
                }
            }
        }
    }

    return status;
}

static void EnetMp_macMode2MacMii(emac_mode macMode,
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

static void EnetMp_createClock(void)
{
    TaskP_Params taskParams;
    ClockP_Params clkParams;
    int32_t status;
    /* Create timer semaphore */
    status = SemaphoreP_constructCounting(&gEnetMp.timerSemObj, 0, COUNTING_SEM_COUNT);
    DebugP_assert(SystemP_SUCCESS == status);

    /* Initialize the periodic tick task params */
    TaskP_Params_init(&taskParams);
    taskParams.priority       = 7U;
    taskParams.stack          = gEnetMpTaskStackTick;
    taskParams.stackSize      = sizeof(gEnetMpTaskStackTick);
    taskParams.args           = (void*)&gEnetMp.timerSemObj;
    taskParams.name           = "Periodic tick task";
    taskParams.taskMain       = &EnetMp_tickTask;
    /* Create periodic tick task */
    EnetAppUtils_print("Create periodic tick task\r\n");

    status = TaskP_construct(&gEnetMp.tickTaskObj, &taskParams);
    DebugP_assert(SystemP_SUCCESS == status);

    ClockP_Params_init(&clkParams);
    clkParams.timeout    = ENETMP_PERIODIC_TICK_MS;
    clkParams.period    = ENETMP_PERIODIC_TICK_MS;
    clkParams.args       = (void*)&gEnetMp.timerSemObj;
    clkParams.callback  = &EnetMp_timerCallback;

    clkParams.start = FALSE;
    /* Creating timer and setting timer callback function */
    EnetAppUtils_print("Create periodic tick clock\r\n");
    status = ClockP_construct(&gEnetMp.tickTimerObj, &clkParams);
    DebugP_assert(SystemP_SUCCESS == status);

}

static void EnetMp_deleteClock(void)
{
    /* Delete periodic tick timer */
    EnetAppUtils_print("Delete periodic tick clock\r\n");
    ClockP_destruct(&gEnetMp.tickTimerObj);
    /* Delete periodic tick task */
     TaskP_destruct(&gEnetMp.tickTaskObj);
    /* Delete periodic tick timer */
    SemaphoreP_destruct(&gEnetMp.timerSemObj);

}

static void EnetMp_timerCallback(ClockP_Object *clkInst, void * arg)
{
    SemaphoreP_Object* hSem = (SemaphoreP_Object*)arg;

    /* Tick! */
    SemaphoreP_post(hSem);
}

static void EnetMp_tickTask(void *args)
{
    SemaphoreP_Object* hSem = (SemaphoreP_Object*)args;
    uint32_t i;

    while (gEnetMp.run)
    {
        SemaphoreP_pend(hSem, SystemP_WAIT_FOREVER);

        /* Periodic tick should be called from non-ISR context */
        for (i = 0U; i < gEnetMp.numPerCtxts; i++)
        {
            Enet_periodicTick(gEnetMp.perCtxt[i].hEnet);
        }
    }
    TaskP_exit();
}

static void EnetMp_rxIsrFxn(void *appData)
{
    EnetMp_PerCtxt *perCtxt = (EnetMp_PerCtxt *)appData;

    SemaphoreP_post(&perCtxt->rxSemObj);
}

static int32_t EnetMp_openDma(EnetMp_PerCtxt *perCtxt)
{
    EnetUdma_OpenRxFlowPrms rxChCfg;
    EnetUdma_OpenTxChPrms txChCfg;
    uint32_t i;
    int32_t status = ENET_SOK;

    /* Open the TX channel */
    EnetDma_initTxChParams(&txChCfg);

    txChCfg.hUdmaDrv = gEnetMp.hMainUdmaDrv;
    txChCfg.cbArg    = NULL;
    txChCfg.notifyCb = NULL;
    txChCfg.useGlobalEvt = true;

    EnetAppUtils_setCommonTxChPrms(&txChCfg);
    txChCfg.numTxPkts = ENET_MEM_NUM_TX_PKTS/2U;

    EnetAppUtils_openTxCh(perCtxt->hEnet,
                          gEnetMp.coreKey,
                          gEnetMp.coreId,
                          &perCtxt->txChNum,
                          &perCtxt->hTxCh,
                          &txChCfg);
    if (perCtxt->hTxCh == NULL)
    {
#if FIX_RM
        /* Free the channel number if open Tx channel failed */
        EnetAppUtils_freeTxCh(gEnetMp.hEnet,
                              gEnetMp.coreKey,
                              gEnetMp.coreId,
                              gEnetMp.txChNum);
#endif
        EnetAppUtils_print("EnetMp_openDma() failed to open TX channel\r\n");
        status = ENET_EFAIL;
        EnetAppUtils_assert(perCtxt->hTxCh != NULL);
    }

    /* Allocate TX packets and keep them locally enqueued */
    if (status == ENET_SOK)
    {
        EnetMp_initTxFreePktQ();
    }

    /* Open the RX flow */
    if (status == ENET_SOK)
    {
        perCtxt->numHwRxCh = (perCtxt->enetType == ENET_ICSSG_SWITCH) ? 2U : 1U;

        EnetDma_initRxChParams(&rxChCfg);

        rxChCfg.hUdmaDrv = gEnetMp.hMainUdmaDrv;
        rxChCfg.notifyCb = EnetMp_rxIsrFxn;
        rxChCfg.cbArg    = perCtxt;
        rxChCfg.useGlobalEvt = true;
        rxChCfg.flowPrms.sizeThreshEn = 0U;

        for (i = 0U; i < perCtxt->numHwRxCh; i++)
        {
            EnetAppUtils_setCommonRxFlowPrms(&rxChCfg);
            rxChCfg.numRxPkts = ENET_MEM_NUM_RX_PKTS/2U;
            EnetAppUtils_openRxFlowForChIdx(perCtxt->enetType,
                                            perCtxt->hEnet,
                                            gEnetMp.coreKey,
                                            gEnetMp.coreId,
                                            true,
                                            i,
                                            &perCtxt->rxStartFlowIdx[i],
                                            &perCtxt->rxFlowIdx[i],
                                            &perCtxt->macAddr[0U],
                                            &perCtxt->hRxCh[i],
                                            &rxChCfg);
            if (perCtxt->hRxCh[i] == NULL)
            {
                EnetAppUtils_print("EnetMp_openRxCh() failed to open RX flow\r\n");
                status = ENET_EFAIL;
                EnetAppUtils_assert(perCtxt->hRxCh[i] != NULL);
            }
        }
    }

    /* Submit all ready RX buffers to DMA */
    if (status == ENET_SOK)
    {
        for (i = 0U; i < perCtxt->numHwRxCh; i++)
        {
            EnetMp_initRxReadyPktQ(perCtxt->hRxCh[i]);
        }
    }

     return status;
}

static void EnetMp_closeDma(EnetMp_PerCtxt *perCtxt)
{
    EnetDma_PktQ fqPktInfoQ;
    EnetDma_PktQ cqPktInfoQ;
    uint32_t i;

    EnetQueue_initQ(&fqPktInfoQ);
    EnetQueue_initQ(&cqPktInfoQ);

    /* Close RX channel */
    for (i = 0U; i < perCtxt->numHwRxCh; i++)
    {
        EnetAppUtils_closeRxFlowForChIdx(perCtxt->enetType,
                                         perCtxt->hEnet,
                                         gEnetMp.coreKey,
                                         gEnetMp.coreId,
                                         true,
                                         &fqPktInfoQ,
                                         &cqPktInfoQ,
                                         i,
                                         perCtxt->rxStartFlowIdx[i],
                                         perCtxt->rxFlowIdx[i],
                                         perCtxt->macAddr,
                                         perCtxt->hRxCh[i]);

        EnetAppUtils_freePktInfoQ(&fqPktInfoQ);
        EnetAppUtils_freePktInfoQ(&cqPktInfoQ);
    }

    /* Close TX channel */
    EnetQueue_initQ(&fqPktInfoQ);
    EnetQueue_initQ(&cqPktInfoQ);

    /* Retrieve any pending TX packets from driver */
    EnetMp_retrieveFreeTxPkts(perCtxt);

    EnetAppUtils_closeTxCh(perCtxt->hEnet,
                           gEnetMp.coreKey,
                           gEnetMp.coreId,
                           &fqPktInfoQ,
                           &cqPktInfoQ,
                           perCtxt->hTxCh,
                           perCtxt->txChNum);

    EnetAppUtils_freePktInfoQ(&fqPktInfoQ);
    EnetAppUtils_freePktInfoQ(&cqPktInfoQ);

    EnetAppUtils_freePktInfoQ(&gEnetMp.txFreePktInfoQ);
}

static void EnetMp_initTxFreePktQ(void)
{
    EnetDma_Pkt *pPktInfo;
    uint32_t i;

    /* Initialize TX EthPkts and queue them to txFreePktInfoQ */
    for (i = 0U; i < (ENET_MEM_NUM_TX_PKTS/2); i++)
    {
        pPktInfo = EnetMem_allocEthPkt(&gEnetMp,
                                       ENET_MEM_LARGE_POOL_PKT_SIZE,
                                       ENETDMA_CACHELINE_ALIGNMENT);
        EnetAppUtils_assert(pPktInfo != NULL);
        ENET_UTILS_SET_PKT_APP_STATE(&pPktInfo->pktState, ENET_PKTSTATE_APP_WITH_FREEQ);

        EnetQueue_enq(&gEnetMp.txFreePktInfoQ, &pPktInfo->node);
    }

    EnetAppUtils_print("initQs() txFreePktInfoQ initialized with %d pkts\r\n",
                       EnetQueue_getQCount(&gEnetMp.txFreePktInfoQ));
}

static void EnetMp_initRxReadyPktQ(EnetDma_RxChHandle hRxCh)
{
    EnetDma_PktQ rxReadyQ;
    EnetDma_PktQ rxFreeQ;
    EnetDma_Pkt *pPktInfo;
    uint32_t i;
    int32_t status;

    EnetQueue_initQ(&rxFreeQ);

    for (i = 0U; i < (ENET_MEM_NUM_RX_PKTS/2); i++)
    {
        pPktInfo = EnetMem_allocEthPkt(&gEnetMp,
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

static uint32_t EnetMp_retrieveFreeTxPkts(EnetMp_PerCtxt *perCtxt)
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

            EnetQueue_enq(&gEnetMp.txFreePktInfoQ, &pktInfo->node);
            pktInfo = (EnetDma_Pkt *)EnetQueue_deq(&txFreeQ);
        }
    }
    else
    {
        EnetAppUtils_print("retrieveFreeTxPkts() failed to retrieve pkts: %d\r\n", status);
    }

    return txFreeQCnt;
}

static void EnetMp_createRxTask(EnetMp_PerCtxt *perCtxt,
                                uint8_t *taskStack,
                                uint32_t taskStackSize)
{
    TaskP_Params taskParams;
    int32_t status;
    status = SemaphoreP_constructBinary(&perCtxt->rxSemObj, 0);
    DebugP_assert(SystemP_SUCCESS == status);

    status = SemaphoreP_constructCounting(&perCtxt->rxDoneSemObj, 0, COUNTING_SEM_COUNT);
    DebugP_assert(SystemP_SUCCESS == status);

    TaskP_Params_init(&taskParams);
    taskParams.priority       = 2U;
    taskParams.stack          = taskStack;
    taskParams.stackSize      = taskStackSize;
    taskParams.args           = (void*)perCtxt;
    taskParams.name           = "Rx Task";
    taskParams.taskMain           = &EnetMp_rxTask;

    status = TaskP_construct(&perCtxt->rxTaskObj, &taskParams);
    DebugP_assert(SystemP_SUCCESS == status);
}

static void EnetMp_destroyRxTask(EnetMp_PerCtxt *perCtxt)
{
    SemaphoreP_destruct(&perCtxt->rxSemObj);
    SemaphoreP_destruct(&perCtxt->rxDoneSemObj);
    TaskP_destruct(&perCtxt->rxTaskObj);
}

static void EnetMp_rxTask(void *args)
{
    EnetMp_PerCtxt *perCtxt = (EnetMp_PerCtxt *)args;
    EnetDma_PktQ rxReadyQ;
    EnetDma_PktQ rxFreeQ;
    EnetDma_PktQ txSubmitQ;
    EnetDma_Pkt *rxPktInfo;
    EnetDma_Pkt *txPktInfo;
    EthFrame *rxFrame;
    EthFrame *txFrame;
    Enet_IoctlPrms prms;
    bool semStatus;
    uint32_t reqTs;
    uint32_t totalRxCnt = 0U;
    uint32_t i;
    int32_t status = ENET_SOK;

    status = EnetMp_openPort(perCtxt);
    if (status != ENET_SOK)
    {
        EnetAppUtils_print("%s: Failed to open port link: %d\r\n", perCtxt->name, status);
    }

    status = EnetMp_waitForLinkUp(perCtxt);
    if (status != ENET_SOK)
    {
        EnetAppUtils_print("%s: Failed to wait for link up: %d\n", perCtxt->name, status);
    }

    /* Open DMA for peripheral/port */
    if (status == ENET_SOK)
    {
        EnetAppUtils_print("%s: Open DMA\r\n", perCtxt->name);

        status = EnetMp_openDma(perCtxt);
        if (status != ENET_SOK)
        {
            EnetAppUtils_print("%s: failed to open DMA: %d\r\n", perCtxt->name, status);
        }
    }

    /* Add port MAC entry */
    if ((status == ENET_SOK) && (Enet_isIcssFamily(perCtxt->enetType)))
    {
        EnetAppUtils_print("%s: Set MAC addr: ", perCtxt->name);
        EnetAppUtils_printMacAddr(&perCtxt->macAddr[0U]);

        if (perCtxt->enetType == ENET_ICSSG_DUALMAC)
        {
            IcssgMacPort_SetMacAddressInArgs inArgs;

            memset(&inArgs, 0, sizeof(inArgs));
            inArgs.macPort = perCtxt->macPort[0U];
            EnetUtils_copyMacAddr(&inArgs.macAddr[0U], &perCtxt->macAddr[0U]);
            ENET_IOCTL_SET_IN_ARGS(&prms, &inArgs);

            status = Enet_ioctl(perCtxt->hEnet, gEnetMp.coreId, ICSSG_MACPORT_IOCTL_SET_MACADDR, &prms);
        }
        else
        {
            Icssg_MacAddr addr; // FIXME Icssg_MacAddr type

            /* Set host port's MAC address */
            EnetUtils_copyMacAddr(&addr.macAddr[0U], &perCtxt->macAddr[0U]);
            ENET_IOCTL_SET_IN_ARGS(&prms, &addr);

            status = Enet_ioctl(perCtxt->hEnet, gEnetMp.coreId, ICSSG_HOSTPORT_IOCTL_SET_MACADDR, &prms);
        }

        if (status != ENET_SOK)
        {
                EnetAppUtils_print("%s: Failed to set MAC address entry: %d\n", perCtxt->name, status);
        }
    }

    EnetAppUtils_print("%s: MAC port addr: ", perCtxt->name);
    EnetAppUtils_printMacAddr(&perCtxt->macAddr[0U]);

    while ((ENET_SOK == status) && (gEnetMp.run))
    {
        /* Wait for packet reception */
        SemaphoreP_pend(&perCtxt->rxSemObj, SystemP_WAIT_FOREVER);

        /* All peripherals have single hardware RX channel, so we only need to retrieve
         * packets from a single flow.  But ICSSG Switch has two hardware channels, so
         * we need to retrieve packets from two flows, one flow per channel */
        for (i = 0U; i < perCtxt->numHwRxCh; i++)
        {
            EnetQueue_initQ(&rxReadyQ);
            EnetQueue_initQ(&rxFreeQ);
            EnetQueue_initQ(&txSubmitQ);

            /* Get the packets received so far */
            status = EnetDma_retrieveRxPktQ(perCtxt->hRxCh[i], &rxReadyQ);
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
                EnetMp_retrieveFreeTxPkts(perCtxt);

                /* Dequeue one free TX Eth packet */
                txPktInfo = (EnetDma_Pkt *)EnetQueue_deq(&gEnetMp.txFreePktInfoQ);
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
                    txPktInfo->appPriv = &gEnetMp;

                    /* Set timestamp info in DMA packet.
                     * Packet timestamp currently enabled only for ICSSG. */
                    if (gEnetMp.enableTs &&
                        Enet_isIcssFamily(perCtxt->enetType))
                    {
                        /* Save the timestamp of received packet that we are about to send back,
                         * so we can calculate the RX-to-TX time diffence in TX timestamp callback */
                        perCtxt->rxTs[perCtxt->txTsSeqId % ENET_MEM_NUM_RX_PKTS] = rxPktInfo->tsInfo.rxPktTs;

                        txPktInfo->tsInfo.enableHostTxTs = true;
                        txPktInfo->tsInfo.txPktSeqId     = perCtxt->txTsSeqId++;
                        txPktInfo->tsInfo.txPktMsgType   = 0U; /* Don't care for ICSSG */
                        txPktInfo->tsInfo.txPktDomain    = 0U; /* Don't care for ICSSG */
                        reqTs++;
                    }
                    else
                    {
                        txPktInfo->tsInfo.enableHostTxTs = false;
                    }

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

            /* Wait for TX timestamp */
            while (gEnetMp.run && (reqTs != 0U))
            {
                Enet_MacPort macPort = ENET_MACPORT_DENORM(i);

                Enet_poll(perCtxt->hEnet, ENET_EVT_TIMESTAMP_TX, &macPort, sizeof(macPort));
                semStatus = SemaphoreP_pend(&perCtxt->txTsSemObj, SystemP_WAIT_FOREVER);
                if (semStatus == SystemP_SUCCESS)
                {
                    reqTs--;
                }
            }

            /* Submit now processed buffers */
            EnetDma_submitRxPktQ(perCtxt->hRxCh[i], &rxFreeQ);
            if (status != ENET_SOK)
            {
                EnetAppUtils_print("%s: Failed to submit RX pkt queue: %d\r\n", perCtxt->name, status);
            }
        }
    }

#if DEBUG
    EnetAppUtils_print("%s: Received %u packets\r\n", perCtxt->name, totalRxCnt);
#endif

    EnetMp_closePort(perCtxt);

    SemaphoreP_post(&perCtxt->rxDoneSemObj);
    TaskP_exit();
}
