/*
 * Copyright (c) 2001,2002 Florian Schulze.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the authors nor the names of the contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * test.c - This file is part of lwIP test
 *
 */

/* C runtime includes */
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <kernel/dpl/TaskP.h>
#include <kernel/dpl/SystemP.h>

#include <drivers/udma/udma_priv.h>
#include <drivers/udma.h>

#include <networking/enet/enet.h>
#include <networking/enet/core/include/per/cpsw.h>
#include <networking/enet/utils/include/enet_appmemutils_cfg.h>
#include <networking/enet/utils/include/enet_apputils.h>
#include <networking/enet/utils/include/enet_appmemutils.h>
#include <networking/enet/utils/include/enet_appboardutils.h>
#include <networking/enet/utils/include/enet_mcm.h>
#include <networking/enet/utils/include/enet_appsoc.h>
#include <networking/enet/utils/include/enet_apprm.h>


#if defined(ENET_ENABLE_ICSSG)
#include <networking/enet/core/include/per/icssg.h>
#endif

/*
 * Using CPSW defined macros for this specific file
 */
#undef htons
#undef ntohs
#undef htonl
#undef ntohl

#include <networking/enet/core/lwipif/inc/lwip2lwipif.h>
#include <networking/enet/core/lwipif/inc/lwipif2enet_AppIf.h>


typedef struct
{
    EnetMcm_CmdIf hMcmCmdIf[ENET_TYPE_NUM];
    Udma_DrvHandle hUdmaDrv;
    Enet_Type enetType;
    uint32_t instId;
    Enet_MacPort macPortList[ENET_MAC_PORT_NUM];
    uint8_t numMacPorts;
} LwipIfCb_AppObj;

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */
#define ENETLWIP_PACKET_POLL_PERIOD_US                                  (1000U)
#define ENETLWIP_USE_DEFAULT_FLOW                                       (true)


/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

static LwipIfCb_AppObj gLwipIfCbObj =
{
    .hMcmCmdIf =
    {
        [ENET_CPSW_3G] = {.hMboxCmd = NULL, .hMboxResponse = NULL},
        [ENET_ICSSG_DUALMAC] = {.hMboxCmd = NULL, .hMboxResponse = NULL},
        [ENET_ICSSG_SWITCH] = {.hMboxCmd = NULL, .hMboxResponse = NULL},
        [ENET_GMAC_3G] = {.hMboxCmd = NULL, .hMboxResponse = NULL},
        [ENET_CPSW_2G] = {.hMboxCmd = NULL, .hMboxResponse = NULL},
        [ENET_CPSW_3G] = {.hMboxCmd = NULL, .hMboxResponse = NULL},
        [ENET_CPSW_5G] = {.hMboxCmd = NULL, .hMboxResponse = NULL},
        [ENET_CPSW_9G] = {.hMboxCmd = NULL, .hMboxResponse = NULL},
    },
};


/* ========================================================================== */
/*                            Function Declaration                            */
/* ========================================================================== */


static int32_t LwipApp_init(Enet_Type enetType)
{
    int32_t status = ENET_SOK;
    EnetMcm_InitConfig enetMcmCfg;
    Cpsw_Cfg cpswCfg;
#if defined(ENET_ENABLE_ICSSG)
    Icssg_Cfg icssgCfg;
#endif
    EnetRm_ResCfg *resCfg = &cpswCfg.resCfg;
    EnetUdma_Cfg dmaCfg;

    EnetApp_getEnetInstInfo(&gLwipIfCbObj.enetType,
                            &gLwipIfCbObj.instId,
                            gLwipIfCbObj.macPortList,
                            &gLwipIfCbObj.numMacPorts);

    EnetAppUtils_assert(gLwipIfCbObj.enetType == enetType);

    /* Open UDMA */
    gLwipIfCbObj.hUdmaDrv = EnetAppUtils_udmaOpen(gLwipIfCbObj.enetType, NULL);
    EnetAppUtils_assert(NULL != gLwipIfCbObj.hUdmaDrv);
    dmaCfg.rxChInitPrms.dmaPriority = UDMA_DEFAULT_RX_CH_DMA_PRIORITY;
    dmaCfg.hUdmaDrv = gLwipIfCbObj.hUdmaDrv;

    /* Set configuration parameters */
    if (Enet_isCpswFamily(gLwipIfCbObj.enetType))
    {
        cpswCfg.dmaCfg = (void *)&dmaCfg;
        Enet_initCfg(gLwipIfCbObj.enetType, gLwipIfCbObj.instId, &cpswCfg, sizeof(cpswCfg));
        EnetApp_getCpswInitCfg(gLwipIfCbObj.enetType, gLwipIfCbObj.instId, &cpswCfg);

        enetMcmCfg.perCfg = &cpswCfg;
        resCfg = &cpswCfg.resCfg;
    }
#if defined(ENET_ENABLE_ICSSG)
    else
    {
        Enet_initCfg(gLwipIfCbObj.enetType, gLwipIfCbObj.instId, &icssgCfg, sizeof(icssgCfg));

        /* Currently we only support one ICSSG port in NIMU */
        EnetAppUtils_assert(gLwipIfCbObj.numMacPorts == 1U);

        resCfg = &icssgCfg.resCfg;
        icssgCfg.dmaCfg = (void *)&dmaCfg;

        enetMcmCfg.perCfg = &icssgCfg;
    }
#endif

    EnetAppUtils_assert(NULL != enetMcmCfg.perCfg);
    EnetAppUtils_initResourceConfig(gLwipIfCbObj.enetType, EnetSoc_getCoreId(), resCfg);

    enetMcmCfg.enetType           = gLwipIfCbObj.enetType;
    enetMcmCfg.instId             = gLwipIfCbObj.instId;
    enetMcmCfg.setPortLinkCfg     = EnetApp_initLinkArgs;
    enetMcmCfg.numMacPorts        = gLwipIfCbObj.numMacPorts;
    enetMcmCfg.periodicTaskPeriod = ENETPHY_FSM_TICK_PERIOD_MS; /* msecs */
    enetMcmCfg.print              = EnetAppUtils_print;

    memcpy(&enetMcmCfg.macPortList[0U], &gLwipIfCbObj.macPortList[0U], sizeof(enetMcmCfg.macPortList));
    status = EnetMcm_init(&enetMcmCfg);

    return status;
}


void LwipifEnetAppCb_getHandle(LwipifEnetAppIf_GetHandleInArgs *inArgs,
                               LwipifEnetAppIf_GetHandleOutArgs *outArgs)
{
    int32_t status;
    EnetMcm_HandleInfo handleInfo;
    EnetPer_AttachCoreOutArgs attachInfo;
    uint32_t coreId          = EnetSoc_getCoreId();
    EnetMcm_CmdIf *pMcmCmdIf;

    EnetUdma_OpenRxFlowPrms enetRxFlowCfg;
    EnetUdma_OpenTxChPrms enetTxChCfg;
    bool useRingMon = false;
    bool useDfltFlow = ENETLWIP_USE_DEFAULT_FLOW;

    EnetApp_getEnetInstInfo(&gLwipIfCbObj.enetType,
                            &gLwipIfCbObj.instId,
                            gLwipIfCbObj.macPortList,
                            &gLwipIfCbObj.numMacPorts);
    pMcmCmdIf = &gLwipIfCbObj.hMcmCmdIf[gLwipIfCbObj.enetType];
    if (pMcmCmdIf->hMboxCmd == NULL)
    {
        status = LwipApp_init(gLwipIfCbObj.enetType);

        if (status != ENET_SOK)
        {
            EnetAppUtils_print("Failed to open ENET: %d\r\n", status);
        }
        EnetAppUtils_assert(status == ENET_SOK);
        EnetMcm_getCmdIf(gLwipIfCbObj.enetType, pMcmCmdIf);
    }

    EnetAppUtils_assert(pMcmCmdIf->hMboxCmd != NULL);
    EnetAppUtils_assert(pMcmCmdIf->hMboxResponse != NULL);
    EnetMcm_acquireHandleInfo(pMcmCmdIf, &handleInfo);
    EnetMcm_coreAttach(pMcmCmdIf, coreId, &attachInfo);

    /* Confirm HW checksum offload is enabled as NIMU enables it by default */
    if (Enet_isCpswFamily(gLwipIfCbObj.enetType))
    {
        Enet_IoctlPrms prms;
        bool csumOffloadFlg;
        ENET_IOCTL_SET_OUT_ARGS(&prms, &csumOffloadFlg);
        status = Enet_ioctl(handleInfo.hEnet,
                            coreId,
                            ENET_HOSTPORT_IS_CSUM_OFFLOAD_ENABLED,
                            &prms);
        if (status != ENET_SOK)
        {
            EnetAppUtils_print("() Failed to get checksum offload info: %d\r\n", status);
        }

        EnetAppUtils_assert(false == csumOffloadFlg);
    }

    /* Open TX channel */
    EnetDma_initTxChParams(&enetTxChCfg);

    enetTxChCfg.hUdmaDrv  = handleInfo.hUdmaDrv;
    enetTxChCfg.useProxy  = true;
    enetTxChCfg.numTxPkts = inArgs->txCfg.numPackets;
    enetTxChCfg.cbArg     = inArgs->txCfg.cbArg;
    enetTxChCfg.notifyCb  = inArgs->txCfg.notifyCb;
    EnetAppUtils_setCommonTxChPrms(&enetTxChCfg);

    EnetAppUtils_openTxCh(handleInfo.hEnet,
                          attachInfo.coreKey,
                          coreId,
                          &outArgs->txInfo.txChNum,
                          &outArgs->txInfo.hTxChannel,
                          &enetTxChCfg);
    /* Open RX Flow */
    EnetDma_initRxChParams(&enetRxFlowCfg);
    enetRxFlowCfg.notifyCb  = inArgs->rxCfg.notifyCb;
    enetRxFlowCfg.numRxPkts = inArgs->rxCfg.numPackets;
    enetRxFlowCfg.cbArg     = inArgs->rxCfg.cbArg;
    enetRxFlowCfg.hUdmaDrv  = handleInfo.hUdmaDrv;
    enetRxFlowCfg.useProxy  = true;

#if (UDMA_SOC_CFG_UDMAP_PRESENT == 1)
    /* Use ring monitor for the CQ ring of RX flow */
    EnetUdma_UdmaRingPrms *pFqRingPrms = &enetRxFlowCfg.udmaChPrms.fqRingPrms;
    pFqRingPrms->useRingMon = true;
    pFqRingPrms->ringMonCfg.mode = TISCI_MSG_VALUE_RM_MON_MODE_THRESHOLD;
    /* Ring mon low threshold */

#if defined _DEBUG_
    /* In debug mode as CPU is processing lesser packets per event, keep threshold more */
    pFqRingPrms->ringMonCfg.data0 = (inArgs->rxCfg.numPackets - 10U);
#else
    pFqRingPrms->ringMonCfg.data0 = (inArgs->rxCfg.numPackets - 20U);
#endif
    /* Ring mon high threshold - to get only low  threshold event, setting high threshold as more than ring depth*/
    pFqRingPrms->ringMonCfg.data1 = inArgs->rxCfg.numPackets;
#endif

    EnetAppUtils_setCommonRxFlowPrms(&enetRxFlowCfg);

#if defined(ENET_ENABLE_ICSSG)
    if(Enet_isIcssFamily(gLwipIfCbObj.enetType))
    {
        enetRxFlowCfg.flowPrms.sizeThreshEn = 0U;
    }
#endif

    EnetAppUtils_openRxFlow(gLwipIfCbObj.enetType,
                            gLwipIfCbObj.instId,
                            handleInfo.hEnet,
                            attachInfo.coreKey,
                            coreId,
                            useDfltFlow,
                            &outArgs->rxInfo.rxFlowStartIdx,
                            &outArgs->rxInfo.rxFlowIdx,
                            &outArgs->rxInfo.macAddr[0U],
                            &outArgs->rxInfo.hRxFlow,
                            &enetRxFlowCfg);


    outArgs->coreId        = coreId;
    outArgs->coreKey       = attachInfo.coreKey;
    outArgs->hEnet         = handleInfo.hEnet;
    outArgs->hostPortRxMtu = attachInfo.rxMtu;
    ENET_UTILS_ARRAY_COPY(outArgs->txMtu, attachInfo.txMtu);
    outArgs->print           = &EnetAppUtils_print;
    outArgs->isPortLinkedFxn = &EnetApp_isPortLinked;
    outArgs->hUdmaDrv      = handleInfo.hUdmaDrv;
    outArgs->isRingMonUsed = useRingMon;


    outArgs->timerPeriodUs   = ENETLWIP_PACKET_POLL_PERIOD_US;

    /* Use optimized processing where TX packets are relinquished in next TX submit call */
    outArgs->disableTxEvent = true;

    EnetAppUtils_print("Host MAC address: ");
    EnetAppUtils_printMacAddr(&outArgs->rxInfo.macAddr[0U]);

#if defined(ENET_ENABLE_ICSSG)
    /* Add port MAC entry in case of ICSSG dual MAC */
    if (ENET_ICSSG_DUALMAC == gLwipIfCbObj.enetType)
    {
        Enet_IoctlPrms prms;
        IcssgMacPort_SetMacAddressInArgs inArgs;

        memset(&inArgs, 0, sizeof(inArgs));
        memcpy(&inArgs.macAddr[0U], &outArgs->rxInfo.macAddr[0U], sizeof(inArgs.macAddr));
        inArgs.macPort = gLwipIfCbObj.macPortList[0U];

        ENET_IOCTL_SET_IN_ARGS(&prms, &inArgs);
        status = Enet_ioctl(handleInfo.hEnet, coreId, ICSSG_MACPORT_IOCTL_SET_MACADDR, &prms);
        if (status != ENET_SOK)
        {
            EnetAppUtils_print("EnetAppUtils_addHostPortEntry() failed ICSSG_MACPORT_IOCTL_ADD_INTERFACE_MACADDR: %d\r\n",
                               status);
        }
        EnetAppUtils_assert(status == ENET_SOK);
    }
#endif
}

void LwipifEnetAppCb_releaseHandle(LwipifEnetAppIf_ReleaseHandleInfo *releaseInfo)
{
    EnetDma_PktQ fqPktInfoQ;
    EnetDma_PktQ cqPktInfoQ;
    bool useDfltFlow = ENETLWIP_USE_DEFAULT_FLOW;
    EnetMcm_CmdIf *pMcmCmdIf = &gLwipIfCbObj.hMcmCmdIf[gLwipIfCbObj.enetType];

    EnetAppUtils_assert(pMcmCmdIf->hMboxCmd != NULL);
    EnetAppUtils_assert(pMcmCmdIf->hMboxResponse != NULL);

    /* Close TX channel */
    {
        EnetQueue_initQ(&fqPktInfoQ);
        EnetQueue_initQ(&cqPktInfoQ);
        EnetAppUtils_closeTxCh(releaseInfo->hEnet,
                               releaseInfo->coreKey,
                               releaseInfo->coreId,
                               &fqPktInfoQ,
                               &cqPktInfoQ,
                               releaseInfo->txInfo.hTxChannel,
                               releaseInfo->txInfo.txChNum);
        releaseInfo->txFreePktCb(releaseInfo->freePktCbArg, &fqPktInfoQ, &cqPktInfoQ);
    }

    {
        /* Close RX Flow */
        EnetQueue_initQ(&fqPktInfoQ);
        EnetQueue_initQ(&cqPktInfoQ);

        EnetAppUtils_closeRxFlow(gLwipIfCbObj.enetType,
                                 releaseInfo->hEnet,
                                 releaseInfo->coreKey,
                                 releaseInfo->coreId,
                                 useDfltFlow,
                                 &fqPktInfoQ,
                                 &cqPktInfoQ,
                                 releaseInfo->rxInfo.rxFlowStartIdx,
                                 releaseInfo->rxInfo.rxFlowIdx,
                                 releaseInfo->rxInfo.macAddr,
                                 releaseInfo->rxInfo.hRxFlow);
        releaseInfo->rxFreePktCb(releaseInfo->freePktCbArg, &fqPktInfoQ, &cqPktInfoQ);
    }

    EnetMcm_coreDetach(pMcmCmdIf, releaseInfo->coreId, releaseInfo->coreKey);
    EnetMcm_releaseHandleInfo(pMcmCmdIf);
}


