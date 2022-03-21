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
 * \file  loopback_cfg.c
 *
 * \brief This file contains the configuration related APIs of enet loopback app.
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include "loopback_common.h"
#include "loopback_cfg.h"

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* 100-ms periodic tick */
#define ENETLPBK_PERIODIC_TICK_MS                (100U)

/* Counting Semaphore count */
#define COUNTING_SEM_COUNT                       (10U)

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

typedef struct EnetLpbk_EnetTypeMenu_s
{
    const char *text;
    Enet_Type enetType;
    uint32_t instId;
} EnetLpbk_EnetTypeMenu;

typedef struct EnetLpbk_PortMenu_s
{
    const char *text;
    Enet_MacPort macPort;
    emac_mode macMode;
    uint32_t boardId;
} EnetLpbk_PortMenu;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

static void EnetLpbk_showEnetTypeMenu(Enet_Type *enetType,
                                      uint32_t *instId);

static void EnetLpbk_showLpbkMenu(EnetLpbk_type *loopbackType);

static int32_t EnetLpbk_showPortMenu(Enet_Type enetType,
                                     uint32_t instId,
                                     EnetLpbk_type loopbackType,
                                     Enet_MacPort *macPort,
                                     emac_mode *macMode,
                                     uint32_t *boardId);

static void EnetLpbk_timerCallback(ClockP_Object *clkInst, void* arg);

static void EnetLpbk_tickTask(void *args);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

static EnetLpbk_EnetTypeMenu gEnetLpbk_EnetTypeMenu[] =
{
    { "CPSW 2G", ENET_CPSW_2G, 0U },
    { "CPSW 3G", ENET_CPSW_3G, 0U },
};

static EnetLpbk_PortMenu gEnetLpbk_MacLpbkMenu[] =
{
    ENETLPBK_PORT_OPT(ENET_MAC_PORT_1, RGMII, ENETBOARD_LOOPBACK_ID),
};

static EnetLpbk_PortMenu gEnetLpbk_cpsw2gPhyLpbkMenu[] =
{
    ENETLPBK_PORT_OPT(ENET_MAC_PORT_1, RGMII, ENETBOARD_CPB_ID),
};

static EnetLpbk_PortMenu gEnetLpbk_cpsw3gPhyLpbkMenu[] =
{
    ENETLPBK_PORT_OPT(ENET_MAC_PORT_1, RGMII, ENETBOARD_CPB_ID),
};

/* Enet loopback test object */
EnetLpbk_Obj gEnetLpbk;

/* Test application stack */
static uint8_t gEnetLpbkTaskStackTick[ENETLPBK_TASK_STACK_SZ] __attribute__ ((aligned(32)));

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

void EnetLpbk_showMenu(void)
{
    int32_t status;

    do
    {
        EnetLpbk_showEnetTypeMenu(&gEnetLpbk.enetType,
                                  &gEnetLpbk.instId);

        EnetLpbk_showLpbkMenu(&gEnetLpbk.testLoopBackType);

        status = EnetLpbk_showPortMenu(gEnetLpbk.enetType,
                                       gEnetLpbk.instId,
                                       gEnetLpbk.testLoopBackType,
                                       &gEnetLpbk.macPort,
                                       &gEnetLpbk.macMode,
                                       &gEnetLpbk.boardId);

        if (status != ENET_SOK)
        {
            EnetAppUtils_print("Try again...\r\n\n");
        }
    }
    while (status != ENET_SOK);
}

void EnetLpbk_createClock(void)
{
    TaskP_Params taskParams;
    ClockP_Params clkParams;
    int32_t status;

    /* Create timer semaphore */
    status = SemaphoreP_constructCounting(&gEnetLpbk.timerSemObj, 0, COUNTING_SEM_COUNT);
    DebugP_assert(SystemP_SUCCESS == status);

    /* Reset the exitFlag */
    gEnetLpbk.exitFlag     = false;
    gEnetLpbk.exitFlagDone = false;

    /* Initialize the periodic tick task params */
    TaskP_Params_init(&taskParams);
    taskParams.priority       = 7U;
    taskParams.stack          = gEnetLpbkTaskStackTick;
    taskParams.stackSize      = sizeof(gEnetLpbkTaskStackTick);
    taskParams.args           = (void*)&gEnetLpbk.timerSemObj;
    taskParams.name           = "Periodic tick task";
    taskParams.taskMain       = &EnetLpbk_tickTask;

    /* Create periodic tick task */
    EnetAppUtils_print("Create periodic tick task\r\n");
    status = TaskP_construct(&gEnetLpbk.tickTaskObj, &taskParams);
    DebugP_assert(SystemP_SUCCESS == status);

    ClockP_Params_init(&clkParams);
    clkParams.timeout   = ENETLPBK_PERIODIC_TICK_MS;
    clkParams.period    = ENETLPBK_PERIODIC_TICK_MS;
    clkParams.args      = (void*)&gEnetLpbk.timerSemObj;
    clkParams.callback  = &EnetLpbk_timerCallback;
    clkParams.start     = FALSE;

    /* Creating timer and setting timer callback function */
    status = ClockP_construct(&gEnetLpbk.tickTimerObj, &clkParams);
    DebugP_assert(SystemP_SUCCESS == status);
}

void EnetLpbk_deleteClock(void)
{
    gEnetLpbk.exitFlag = true;

    SemaphoreP_post(&gEnetLpbk.timerSemObj);

    do
    {
        ClockP_usleep(ClockP_ticksToUsec(1));
    } while (gEnetLpbk.exitFlagDone != true);

    /* Delete periodic tick timer */
    ClockP_destruct(&gEnetLpbk.tickTimerObj);

    /* Delete periodic tick semaphore */
    SemaphoreP_destruct(&gEnetLpbk.timerSemObj);
}

/* ========================================================================== */
/*                   Static Function Definitions                              */
/* ========================================================================== */

static void EnetLpbk_showEnetTypeMenu(Enet_Type *enetType,
                                      uint32_t *instId)
{
    bool retry;
    int32_t choice = -1;
    uint32_t i;

    do
    {
        EnetAppUtils_print("Select Enet peripheral type:\r\n");
        for (i = 0U; i < ENET_ARRAYSIZE(gEnetLpbk_EnetTypeMenu); i++)
        {
            EnetAppUtils_print("%u: %s\r\n", i, gEnetLpbk_EnetTypeMenu[i].text);
        }

        DebugP_scanf("%i", &choice);

        if ((choice >= 0) &&
            (choice < ENET_ARRAYSIZE(gEnetLpbk_EnetTypeMenu)))
        {
            *enetType = gEnetLpbk_EnetTypeMenu[choice].enetType;
            *instId = gEnetLpbk_EnetTypeMenu[choice].instId;
            retry = false;
        }
        else
        {
            EnetAppUtils_print("Wrong option, try again...\r\n\n");
            retry = true;
        }
    }
    while (retry);

}

static void EnetLpbk_showLpbkMenu(EnetLpbk_type *loopbackType)
{
    bool retry;
    int32_t choice = -1;

    do
    {
        EnetAppUtils_print("Select loopback type:\r\n");
        EnetAppUtils_print("0: Internal (MAC loopback)\r\n");
        EnetAppUtils_print("1: External (PHY loopback)\r\n");

        DebugP_scanf("%i", &choice);

        switch (choice)
        {
            case 0:
                *loopbackType = LOOPBACK_TYPE_MAC;
                retry = false;
                break;
            case 1:
                *loopbackType = LOOPBACK_TYPE_PHY;
                retry = false;
                break;
            default:
                EnetAppUtils_print("Wrong option, try again...\r\n\n");
                retry = true;
                break;
        }

    }
    while (retry);
}

static int32_t EnetLpbk_showPortMenu(Enet_Type enetType,
                                     uint32_t instId,
                                     EnetLpbk_type loopbackType,
                                     Enet_MacPort *macPort,
                                     emac_mode *macMode,
                                     uint32_t *boardId)
{
    EnetLpbk_PortMenu *portMenu = NULL;
    uint32_t portMenuLen = 0U;
    uint32_t i;
    int32_t choice = -1;
    bool retry;
    int32_t status = ENET_SOK;

    if (LOOPBACK_TYPE_PHY == loopbackType)
    {
        if (enetType == ENET_CPSW_2G)
        {
            portMenu = gEnetLpbk_cpsw2gPhyLpbkMenu;
            portMenuLen = ENET_ARRAYSIZE(gEnetLpbk_cpsw2gPhyLpbkMenu);
        }
		if (enetType == ENET_CPSW_3G)
        {
            portMenu = gEnetLpbk_cpsw3gPhyLpbkMenu;
            portMenuLen = ENET_ARRAYSIZE(gEnetLpbk_cpsw3gPhyLpbkMenu);
        }
    }
    else
    {
        portMenu = gEnetLpbk_MacLpbkMenu;
        portMenuLen = ENET_ARRAYSIZE(gEnetLpbk_MacLpbkMenu);
    }

    if ((portMenuLen == 0U) && (portMenu == NULL))
    {
        EnetAppUtils_print("Ethernet periperhal not supported\r\n");
        status = ENET_EINVALIDPARAMS;
    }
    else if ((portMenuLen == 0U) && (portMenu != NULL))
    {
        EnetAppUtils_print("No ports supported on current core\r\n");
        status = ENET_EINVALIDPARAMS;
    }
    else
    {
        do
        {
            EnetAppUtils_print("Select MAC port:\r\n");
            for (i = 0U; i < portMenuLen; i++)
            {
                EnetAppUtils_print("%u: %s\r\n", i, portMenu[i].text);
            }

            DebugP_scanf("%i", &choice);

            if ((choice >= 0) &&
                (choice < portMenuLen))
            {
                *macPort = portMenu[choice].macPort;
                *macMode = portMenu[choice].macMode;
                *boardId = portMenu[choice].boardId;
                retry = false;
            }
            else
            {
                EnetAppUtils_print("Wrong option, try again...\r\n\n");
                retry = true;
            }
        }
        while (retry);
    }

    return status;
}

static void EnetLpbk_timerCallback(ClockP_Object *clkInst, void* arg)
{
    SemaphoreP_Object* hSem = (SemaphoreP_Object*)arg;

    /* Tick! */
    SemaphoreP_post(hSem);
}

static void EnetLpbk_tickTask(void *args)
{
    SemaphoreP_Object* hSem = (SemaphoreP_Object*)args;

    while (!gEnetLpbk.exitFlag)
    {
        SemaphoreP_pend(hSem, SystemP_WAIT_FOREVER);

        /* Periodic tick should be called from non-ISR context */
        Enet_periodicTick(gEnetLpbk.hEnet);
    }

    EnetAppUtils_print("Delete EnetLpbk_tickTask() and exit..\r\n");

    gEnetLpbk.exitFlagDone = true;

    /* Delete periodic tick task */
    TaskP_destruct(&gEnetLpbk.tickTaskObj);
    TaskP_exit();
}