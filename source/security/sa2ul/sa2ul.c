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
 *  \file   sa2ul.c
 *
 *  \brief  This file contains the implementation of Secure Hash Algorithem (SHA) driver
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <string.h>
#include <stddef.h>
#include <drivers/hw_include/cslr.h>
#include <kernel/dpl/DebugP.h>
#include <kernel/dpl/CacheP.h>
#include <security/sa2ul.h>
#include <drivers/hw_include/am64x_am243x/cslr_soc_baseaddress.h>
#include <drivers/sciclient.h>
#include <drivers/hw_include/am64x_am243x/cslr_main_ctrl_mmr.h>
#include <drivers/hw_include/am64x_am243x/cslr_soc_baseaddress.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/** \brief Check address aligned */
#define SA2UL_IS_ALIGNED_PTR(ptr, align)      ((((uint32_t)ptr) & ((align)-1)) == 0)

#define SA2UL_IS_HMAC(alg)              ((alg & 0x10u) == 0)

/** \brief CSL macros */
#define CSL_SA2UL_SCCTL1_OWNER_MASK                     (0x80000000u)
#define CSL_SA2UL_SCCTL1_OWNER_SHIFT                    (31u)

#define CSL_SA2UL_SCCTL1_EVICT_DONE_MASK                (0x40000000u)
#define CSL_SA2UL_SCCTL1_EVICT_DONE_SHIFT               (30u)

#define CSL_SA2UL_SCCTL1_FETCH_EVICT_CONTROL_MASK       (0xff0000u)
#define CSL_SA2UL_SCCTL1_FETCH_EVICT_CONTROL_SHIFT      (16u)

#define CSL_SA2UL_SCCTL1_FETCH_EVICT_SIZE_MASK          (0xff0000u)
#define CSL_SA2UL_SCCTL1_FETCH_EVICT_SIZE_SHIFT         (16u)

#define CSL_SA2UL_SCCTL1_SCID_MASK                      (0xffffu)
#define CSL_SA2UL_SCCTL1_SCID_SHIFT                     (0u)

#define CSL_SA2UL_SCCTL2_OVERWRITE_FLOWID_MASK          (0x80000000u)
#define CSL_SA2UL_SCCTL2_OVERWRITE_FLOWID_SHIFT         (31u)

#define CSL_SA2UL_SCCTL2_PRIVID_MASK                    (0xff0000u)
#define CSL_SA2UL_SCCTL2_PRIVID_SHIFT                   (16u)

#define CSL_SA2UL_SCCTL2_PRIV_MASK                      (0x300u)
#define CSL_SA2UL_SCCTL2_PRIV_SHIFT                     (8u)

#define CSL_SA2UL_SCCTL2_ALLOW_PROMOTE_MASK             (0x80u)
#define CSL_SA2UL_SCCTL2_ALLOW_PROMOTE_SHIFT            (7u)

#define CSL_SA2UL_SCCTL2_ALLOW_DEMOTE_MASK              (0x40u)
#define CSL_SA2UL_SCCTL2_ALLOW_DEMOTE_SHIFT             (6u)

#define CSL_SA2UL_SCCTL2_ALLOW_NON_SECURE_MASK          (0x20u)
#define CSL_SA2UL_SCCTL2_ALLOW_NON_SECURE_SHIFT         (5u)

#define CSL_SA2UL_SCCTL2_SECURE_MASK                    (0x1u)
#define CSL_SA2UL_SCCTL2_SECURE_SHIFT                   (0u)

#define CSL_SA2UL_SCPTRH_FLOWID_MASK                    (0x3fff0000u)
#define CSL_SA2UL_SCPTRH_FLOWID_SHIFT                   (16u)

#define CSL_SA2UL_AUTHCTX1_MODESEL_MASK                 (0x80000000u)
#define CSL_SA2UL_AUTHCTX1_MODESEL_SHIFT                (31u)

#define CSL_SA2UL_AUTHCTX1_DEFAULT_NEXT_ENGINE_ID_MASK  (0x1f000000u)
#define CSL_SA2UL_AUTHCTX1_DEFAULT_NEXT_ENGINE_ID_SHIFT (24u)

#define CSL_SA2UL_AUTHCTX1_SW_CONTROL_MASK              (0xff0000u)
#define CSL_SA2UL_AUTHCTX1_SW_CONTROL_SHIFT             (16u)

#define CSL_SA2UL_ENCRCTL_MODESEL_MASK                  (0x80000000u)
#define CSL_SA2UL_ENCRCTL_MODESEL_SHIFT                 (31u)

#define CSL_SA2UL_ENCRCTL_USE_DKEK_MASK                 (0x40000000u)
#define CSL_SA2UL_ENCRCTL_USE_DKEK_SHIFT                (30u)

#define CSL_SA2UL_ENCRCTL_DEFAULT_NEXT_ENGINE_ID_MASK   (0x1f000000u)
#define CSL_SA2UL_ENCRCTL_DEFAULT_NEXT_ENGINE_ID_SHIFT  (24u)

#define CSL_SA2UL_ENCRCTL_TRAILER_EVERY_CHUNK_MASK      (0x800000u)
#define CSL_SA2UL_ENCRCTL_TRAILER_EVERY_CHUNK_SHIFT     (23u)

#define CSL_SA2UL_ENCRCTL_TRAILER_AT_END_MASK           (0x400000u)
#define CSL_SA2UL_ENCRCTL_TRAILER_AT_END_SHIFT          (22u)

#define CSL_SA2UL_ENCRCTL_PKT_DATA_SECTION_UPDATE_MASK  (0x200000u)
#define CSL_SA2UL_ENCRCTL_PKT_DATA_SECTION_UPDATE_SHIFT (21u)

#define CSL_SA2UL_ENCRCTL_ENCRYPT_DECRYPT_MASK          (0x100000u)
#define CSL_SA2UL_ENCRCTL_ENCRYPT_DECRYPT_SHIFT         (20u)

#define CSL_SA2UL_ENCRCTL_BLK_SIZE_MASK                 (0x70000u)
#define CSL_SA2UL_ENCRCTL_BLK_SIZE_SHIFT                (16u)

#define CSL_SA2UL_ENCRCTL_SOP_OFFSET_MASK               (0xF00u)
#define CSL_SA2UL_ENCRCTL_SOP_OFFSET_SHIFT              (8u)

#define CSL_SA2UL_ENCRCTL_MIDDLE_OFFSET_MASK            (0xF0u)
#define CSL_SA2UL_ENCRCTL_MIDDLE_OFFSET_SHIFT           (4u)

#define CSL_SA2UL_ENCRCTL_EOP_OFFSET_MASK               (0xFu)
#define CSL_SA2UL_ENCRCTL_EOP_OFFSET_SHIFT              (0u)

#define CSL_SA2UL_SWWORD0_BYP_CMD_LBL_LEN_MASK          (0x80000000u)
#define CSL_SA2UL_SWWORD0_BYP_CMD_LBL_LEN_SHIFT         (31u)

#define CSL_SA2UL_SWWORD0_CPPI_DST_INFO_PRESENT_MASK    (0x40000000u)
#define CSL_SA2UL_SWWORD0_CPPI_DST_INFO_PRESENT_SHIFT   (30u)

#define CSL_SA2UL_SWWORD0_ENGINE_ID_MASK                (0x3e000000u)
#define CSL_SA2UL_SWWORD0_ENGINE_ID_SHIFT               (25u)

#define CSL_SA2UL_SWWORD0_CMD_LBL_PRESENT_MASK          (0x1000000u)
#define CSL_SA2UL_SWWORD0_CMD_LBL_PRESENT_SHIFT         (24u)

#define CSL_SA2UL_SWWORD0_CMD_LBL_OFFSET_MASK           (0xf00000u)
#define CSL_SA2UL_SWWORD0_CMD_LBL_OFFSET_SHIFT          (20u)

#define CSL_SA2UL_SWWORD0_FRAGMENT_MASK                 (0x80000u)
#define CSL_SA2UL_SWWORD0_FRAGMENT_SHIFT                (19u)

#define CSL_SA2UL_SWWORD0_NO_PAYLOAD_MASK               (0x40000u)
#define CSL_SA2UL_SWWORD0_NO_PAYLOAD_SHIFT              (18u)

#define CSL_SA2UL_SWWORD0_TEARDOWN_MASK                 (0x20000u)
#define CSL_SA2UL_SWWORD0_TEARDOWN_SHIFT                (17u)

#define CSL_SA2UL_SWWORD0_EVICT_MASK                    (0x10000u)
#define CSL_SA2UL_SWWORD0_EVICT_SHIFT                   (16u)

#define CSL_SA2UL_SWWORD0_SCID_MASK                     (0xffffu)
#define CSL_SA2UL_SWWORD0_SCID_SHIFT                    (0u)

#define CSL_SA2UL_SCPTRH_EGRESS_CPPI_STATUS_LEN_MASK    (0xff000000u)
#define CSL_SA2UL_SCPTRH_EGRESS_CPPI_STATUS_LEN_SHIFT   (24u)

#define CSL_SA2UL_INPSIINFO_EGRESS_CPPI_DEST_QUEUE_NUM_MASK  (0xffff0000u)
#define CSL_SA2UL_INPSIINFO_EGRESS_CPPI_DEST_QUEUE_NUM_SHIFT (16u)

#define CSL_SA2UL_INPSIINFO_NONSEC_CRYPTO_MASK          (0x8u)
#define CSL_SA2UL_INPSIINFO_NONSEC_CRYPTO_SHIFT         (3u)

#define CSL_SA2UL_INPSIINFO_DEMOTE_MASK                 (0x4u)
#define CSL_SA2UL_INPSIINFO_DEMOTE_SHIFT                (2u)

#define CSL_SA2UL_INPSIINFO_PROMOTE_MASK                (0x2u)
#define CSL_SA2UL_INPSIINFO_PROMOTE_SHIFT               (1u)

#define CSL_SA2UL_CMDLBLHDR2_OPTION1_CTX_OFFSET_MASK    (0xf80000u)
#define CSL_SA2UL_CMDLBLHDR2_OPTION1_CTX_OFFSET_SHIFT   (19u)

#define CSL_SA2UL_CMDLBLHDR2_OPTION1_LEN_MASK           (0x70000u)
#define CSL_SA2UL_CMDLBLHDR2_OPTION1_LEN_SHIFT          (16u)

#define CSL_SA2UL_CMDLBLHDR2_OPTION2_CTX_OFFSET_MASK    (0xf800u)
#define CSL_SA2UL_CMDLBLHDR2_OPTION2_CTX_OFFSET_SHIFT   (11u)

#define CSL_SA2UL_CMDLBLHDR2_OPTION2_LEN_MASK           (0x700u)
#define CSL_SA2UL_CMDLBLHDR2_OPTION2_LEN_SHIFT          (8u)

#define CSL_SA2UL_CMDLBLHDR2_OPTION3_CTX_OFFSET_MASK    (0xf8u)
#define CSL_SA2UL_CMDLBLHDR2_OPTION3_CTX_OFFSET_SHIFT   (3u)

#define CSL_SA2UL_CMDLBLHDR2_OPTION3_LEN_MASK           (0x7u)
#define CSL_SA2UL_CMDLBLHDR2_OPTION3_LEN_SHIFT          (0u)

#define CSL_SA2UL_CMDLBLHDR2_SOP_BYPASS_LEN_MASK        (0xff000000u)
#define CSL_SA2UL_CMDLBLHDR2_SOP_BYPASS_LEN_SHIFT       (24u)

#define CSL_SA2UL_CMDLBLHDR1_LEN_TO_BE_PROCESSESED_MASK (0xffffu)
#define CSL_SA2UL_CMDLBLHDR1_LEN_TO_BE_PROCESSESED_SHIFT (0u)

#define CSL_SA2UL_CMDLBLHDR1_CMD_LABEL_LEN_MASK         (0xff0000u)
#define CSL_SA2UL_CMDLBLHDR1_CMD_LABEL_LEN_SHIFT        (16u)

#define CSL_SA2UL_CMDLBLHDR1_NEXT_ENGINE_SELECT_CODE_MASK  (0xff000000u)
#define CSL_SA2UL_CMDLBLHDR1_NEXT_ENGINE_SELECT_CODE_SHIFT (24u)

#define SA2UL_PSIL_DST_THREAD_OFFSET                    (0x8000U)

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/**
 * \brief Extended packet info for SA2UL
 */
struct SA2UL_ExtendedPktInfo
{
    uint32_t timestamp;
    uint32_t swWord0;
    uint32_t scptrL;
    uint32_t scptrH;
} __attribute__((__packed__));

/**
 * \brief Protocol-specific word in pkts sent to SA2UL
 */
struct SA2UL_PSDataTx
{
    uint32_t inPsiInfo;
    uint32_t cmdLblHdr1;
    uint32_t cmdLblHdr2;
    uint32_t optionWords[52/4];
} __attribute__((__packed__));

/**
 * \brief Protocol specific data for packets recieved from SA2UL
 */
struct SA2UL_PSDataRx
{
    uint32_t trailerData[16];
} __attribute__((__packed__));

/**
 * \brief Host descriptor structure for SA2UL TX with extended pkt info and
 *        PS words, 128 bytes in size.
 */
struct SA2UL_HostDescrTx
{
    /* Host packet descriptor */
    CSL_PktdmaCppi5HMPD pd;

    /* Extended packet info (16 bytes) for SA2UL */
    struct SA2UL_ExtendedPktInfo exPktInfo;

    /* Protocol-specific word for SA2UL */
    struct SA2UL_PSDataTx psData;
} __attribute__((__packed__));

/**
 * \brief Host descriptor structure for SA2UL Rx with extended pkt info and
 *        PS words, 128 bytes in size.
 */
struct SA2UL_HostDescrRx
{
    /* Host packet descriptor */
    CSL_PktdmaCppi5HMPD pd;

    /* Extended packet info (16 bytes) for SA2UL */
    struct SA2UL_ExtendedPktInfo exPktInfo;

    /* Protocol-specific word for SA2UL */
    struct SA2UL_PSDataRx psData;
} __attribute__((__packed__));

enum
{
    MCE_DATA_ARRAY_INDEX_AES_256_ECB = 0,
    MCE_DATA_ARRAY_INDEX_AES_256_CBC_ENCRYPT,
    MCE_DATA_ARRAY_INDEX_AES_256_CBC_DECRYPT
};

static const uint32_t sa2ulHashSizeBytes[] =
{
    0u,  /* NULL */
    16u, /* MD5 */
    20u, /* SHA1 */
    28u, /* SHA2_224 */
    32u, /* SHA2_256 */
    48u, /* SHA2_384 */
    64u, /* SHA2_512 */
    0u   /* NULL */
};

static const uint32_t sa2ulHashBlkSizeBits[] =
{
    0u, /* NULL */
    512u, /* MD5 */
    512u, /* SHA1 */
    512u, /* SHA2_224 */
    512u, /* SHA2_256 */
    1024u, /* SHA2_384 */
    1024u, /* SHA2_512 */
    0u     /* NULL */
};

/* ========================================================================== */
/*                 Internal Function Declarations                             */
/* ========================================================================== */
static void SA2UL_ringAccelWriteDescr(SA2UL_Config *config, uint32_t ringNum, uint64_t descr);
static uint64_t SA2UL_ringAccelReadDescr(SA2UL_Config *config,uint32_t ringNum);
static uint32_t SA2UL_storageQinsert (SA2UL_Object *object,uint64_t val);
static int32_t SA2UL_getMceIndex(SA2UL_ContextObject *ctxObj, uint8_t *aesKeyInvFlag);
static void SA2UL_u32LeToU8(uint8_t *dest, const uint32_t *src, uint32_t len);
static void SA2UL_64bEndianSwap(uint32_t *dest, const uint32_t *src, uint32_t len);
static int32_t SA2UL_setupTxChannel(SA2UL_Config  *config);
static int32_t SA2UL_setupRxChannel(SA2UL_Config  *config);
static int32_t SA2UL_dmaInit(SA2UL_Config *config);
static int32_t SA2UL_configInstance(SA2UL_Config *config);
static int32_t SA2UL_checkOpenParams(const SA2UL_Params *prms);
static int32_t SA2UL_pushBuffer(SA2UL_ContextObject *pCtxObj,const uint8_t  *input, uint32_t ilen, uint8_t  *output);
static int32_t SA2UL_popBuffer(SA2UL_ContextObject *pCtxObj, uint64_t *doneBuf, uint32_t *doneBufSize, uint8_t *dataTransferDone);
static int32_t SA2UL_hwInit(SA2UL_Attrs  *attrs);
static uint32_t SA2UL_hwDeInit(SA2UL_Attrs  *attrs);
/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

void SA2UL_init(void)
{
    return ;
}
void SA2UL_deinit(void)
{
    return ;
}

SA2UL_Handle SA2UL_open(uint32_t index, const SA2UL_Params *params)
{
    uint32_t retVal           = SystemP_SUCCESS;
    SA2UL_Handle     handle   = NULL;
    SA2UL_Config     *config  = NULL;
    SA2UL_Object     *object  = NULL;

    /* Check instance */
    if(index >= gSa2ulConfigNum)
    {
        retVal = SystemP_FAILURE;
    }
    else
    {
        config = &gSa2ulConfig[index];
    }

    if(SystemP_SUCCESS == retVal)
    {
        DebugP_assert(NULL != config->object);
        DebugP_assert(NULL != config->attrs);
        object = config->object;
        if(TRUE == object->isOpen)
        {
            /* Handle is already opened */
            retVal = SystemP_FAILURE;
        }
    }

    if(SystemP_SUCCESS == retVal)
    {
        /* Init state */
        object->handle = (SA2UL_Handle) config;
        if(NULL != params)
        {
            memcpy(&object->prms, params, sizeof(SA2UL_Params));
        }
        else
        {
            /* Init with default if NULL is passed */
            SA2UL_Params_init(&object->prms);
        }

        /* Check open parameters */
        retVal = SA2UL_checkOpenParams(&object->prms);
    }

    if(SystemP_SUCCESS == retVal)
    {
        /* Configure the SA2UL instance parameters HwInit*/
        retVal = SA2UL_hwInit(config->attrs);
        if(SystemP_SUCCESS == retVal)
        {
            /* Initialize DMA */
            retVal = SA2UL_dmaInit(config);
        }
    }

    if(SystemP_SUCCESS == retVal)
    {
        object->isOpen = TRUE;
        handle = (SA2UL_Handle) config;
    }

    /* Free-up resources in case of error */
    if(SystemP_SUCCESS != retVal)
    {
        if(NULL != config)
        {
            SA2UL_close((SA2UL_Handle) config);
        }
    }

    return (handle);
}

void SA2UL_close(SA2UL_Handle handle)
{
    SA2UL_Object *object;
    SA2UL_Config *config;
    const SA2UL_Attrs *attrs;
    config = (SA2UL_Config *) handle;
    if((NULL != config) && (config->object != NULL) && (config->object->isOpen != (uint32_t)FALSE))
    {
        object = config->object;
        attrs = config->attrs;
        Udma_chClose(object->txChHandle);
        Udma_chClose(object->rxChHandle[0]);
        Udma_chClose(object->rxChHandle[1]);
        SA2UL_hwDeInit(config->attrs);
        DebugP_assert(NULL != object);
        DebugP_assert(NULL != attrs);
        object->isOpen = FALSE;
        /* TO module disable */
        handle = NULL;
        SA2UL_hwDeInit(config->attrs);
    }
    return;
}

int32_t SA2UL_contextAlloc(SA2UL_Handle handle, SA2UL_ContextObject *ctxObj, const SA2UL_ContextParams *ctxPrms)
{
    uint32_t retVal = SystemP_SUCCESS;
    SA2UL_SecCtx sc;
    uint64_t authLen;
    SA2UL_Object *saObj;
    SA2UL_Config  *saCfg;
    SA2UL_Attrs   *saAttrs;
    saCfg = (SA2UL_Config *) handle;
    saObj = saCfg->object;
    saAttrs = saCfg->attrs;

    if(!SA2UL_IS_ALIGNED_PTR(ctxObj, 128))
    {
        retVal =  SystemP_FAILURE;
    }

    if (SystemP_SUCCESS == retVal)
    {
        if(ctxPrms != NULL)
        {
            memcpy(&ctxObj->ctxPrms, ctxPrms, sizeof(SA2UL_ContextParams));
        }
    
        else
        {
            retVal = SystemP_FAILURE;
        }
    }
    if (SystemP_SUCCESS == retVal)
    {
        /* Increment global context-ID */
        ctxObj->secCtxId = saObj->contextId;
        saObj->contextId++;
        ctxObj->txBytesCnt = 0u;
        ctxObj->rxBytesCnt = 0u;
        ctxObj->computationStatus = 0u;
        ctxObj->sa2ulErrCnt = 0u;

        memset(&sc, 0, sizeof(SA2UL_SecCtx));

        authLen = (ctxObj->ctxPrms.inputLen << 3) ;

        if(ctxObj->ctxPrms.opType == SA2UL_OP_AUTH)
        {
            if(SA2UL_IS_HMAC(ctxObj->ctxPrms.hashAlg))
            {
                /* HMAC context not supported by ROM driver
                HMAC for ROM is done with SHA512 context */
                retVal = SystemP_FAILURE;
            }
            if (SystemP_SUCCESS == retVal)
            {
                sc.u.auth.authCtx1 =
                    CSL_FMK(SA2UL_AUTHCTX1_MODESEL, 0u) |
                    CSL_FMK(SA2UL_AUTHCTX1_DEFAULT_NEXT_ENGINE_ID, SA2UL_ENGINE_CODE_DEFAULT_EGRESS_PORT) |
                    CSL_FMK(SA2UL_AUTHCTX1_SW_CONTROL, 0x40u | ctxObj->ctxPrms.hashAlg);

                /* Authentication length in bits for basic hash, length in bits */
                sc.u.auth.authenticationLengthLo = authLen;
                sc.u.auth.authenticationLengthLo = 0;
                sc.u.auth.authenticationLengthHi = (uint32_t)(authLen >> 32);

                sc.scctl.scctl1 =
                    CSL_FMK(SA2UL_SCCTL1_OWNER, 1u) |
                    CSL_FMK(SA2UL_SCCTL1_EVICT_DONE, 1u) |
                    CSL_FMK(SA2UL_SCCTL1_FETCH_EVICT_CONTROL, 0x91u);
            }
        }
        else if(ctxObj->ctxPrms.opType == SA2UL_OP_ENC)
        {
            if((ctxObj->ctxPrms.encAlg > SA2UL_ENC_ALG_MAX) ||
                (ctxObj->ctxPrms.encMode > SA2UL_ENC_MODE_MAX) ||
                (ctxObj->ctxPrms.encKeySize > SA2UL_ENC_KEYSIZE_MAX))
            {
                retVal = SystemP_FAILURE;
            }
            if (SystemP_SUCCESS == retVal)
            {
                sc.scctl.scctl1 =
                    CSL_FMK(SA2UL_SCCTL1_OWNER, 1u) |
                    CSL_FMK(SA2UL_SCCTL1_EVICT_DONE, 1u) |
                    CSL_FMK(SA2UL_SCCTL1_FETCH_EVICT_CONTROL, 0x8Du);
            }
        }
        else
        {
            /* Not implemented at present */
            retVal = SystemP_FAILURE;
        }
        if (SystemP_SUCCESS == retVal)
        {
            sc.scctl.scctl2 =
                CSL_FMK(SA2UL_SCCTL2_PRIVID, saAttrs->privId) |
                CSL_FMK(SA2UL_SCCTL2_PRIV, saAttrs->priv) |
                CSL_FMK(SA2UL_SCCTL2_SECURE, saAttrs->secure);

            SA2UL_64bEndianSwap((uint32_t*)&ctxObj->secCtx, (uint32_t*)&sc, sizeof(sc));

            /* Perform cache writeback */
            CacheP_wb(&ctxObj->secCtx, sizeof(sc), CacheP_TYPE_ALLD);

            /* Perform cache writeback */
            CacheP_inv(&ctxObj->secCtx, sizeof(sc), CacheP_TYPE_ALLD);

            ctxObj->handle = (SA2UL_Config *)handle;
        }
    }
    return (retVal);
}

int32_t SA2UL_contextFree(SA2UL_ContextObject *pCtxObj)
{
    uint32_t retVal = SystemP_SUCCESS;

    if(pCtxObj->totalLengthInByes != pCtxObj->txBytesCnt)
    {
        retVal = SystemP_FAILURE;
    }
    else
    {
        memset(pCtxObj, 0, sizeof(SA2UL_ContextObject));
    }
    return (retVal);
}

int32_t SA2UL_contextProcess(SA2UL_ContextObject *pCtxObj,const uint8_t  *input, uint32_t ilen, uint8_t  *output)
{
    uint32_t retVal = SystemP_SUCCESS;
    uint64_t doneBufAddr=0;
    uint32_t donedataLen=0;
    uint8_t doneFlag=0;

    retVal = SA2UL_pushBuffer(pCtxObj,input,ilen,output);

    /* Consider timeout */
    while((doneBufAddr != (uint64_t)input) && ( donedataLen != ilen ) && (doneFlag != (uint8_t)TRUE))
    {
        retVal = SA2UL_popBuffer(pCtxObj,&doneBufAddr, &donedataLen, &doneFlag);
    }
    return (retVal);
}

static uint32_t SA2UL_storageQremove(SA2UL_Object *object,uint64_t *val)
{
    uint32_t retVal = SystemP_FAILURE;

    if(object->storageQueueFree != SA2UL_RING_N_ELEMS){
        *val = object->storageQueue[object->storageQueueTail];
        retVal = object->storageQueueTail;
        object->storageQueueTail = (object->storageQueueTail + 1) % SA2UL_RING_N_ELEMS;
        object->storageQueueFree++;
    }

    return (retVal);
}

static uint64_t SA2UL_ringAccelReadDescr(SA2UL_Config *config,uint32_t ringNum)
{
    uint64_t                descr;
    SA2UL_Attrs             *attrs;
    SA2UL_Object            *object;
    attrs  = (SA2UL_Attrs *)config->attrs;
    object = (SA2UL_Object *)config->object;

    if((ringNum == object->ringaccChnls[attrs->txRingNumInt]) ||
       (ringNum == object->ringaccChnls[attrs->txRingNumInt]))
    {
        Udma_ringDequeueRaw(object->txRingHandle,&descr);
    }
    else if((ringNum == object->ringaccChnls[attrs->rxRingNumInt]) ||
            (ringNum == object->ringaccChnls[attrs->rxRingNumInt]))
    {
        Udma_ringDequeueRaw(object->rxRingHandle,&descr);
    }
    else
    {
        SA2UL_storageQremove(object,&descr);
    }

    return (descr);
}

static uint32_t SA2UL_getStorageRingOcc(SA2UL_Object *object, uint32_t ringNum)
{
    return (SA2UL_RING_N_ELEMS - object->storageQueueFree);
}

static uint32_t SA2UL_storageQinsert(SA2UL_Object *object,uint64_t val)
{
    uint32_t retVal = SystemP_FAILURE;

    if(object->storageQueueFree)
    {
        object->storageQueue[object->storageQueueHead] = val;
        retVal = object->storageQueueHead;
        object->storageQueueHead = (object->storageQueueHead + 1) % SA2UL_RING_N_ELEMS;
        object->storageQueueFree--;
    }

    return (retVal);
}

static void SA2UL_ringAccelWriteDescr(SA2UL_Config *config, uint32_t ringNum, uint64_t descr)
{
    SA2UL_Attrs             *attrs;
    SA2UL_Object            *object;
    attrs  = (SA2UL_Attrs *)config->attrs;
    object = (SA2UL_Object *)config->object;

    if((ringNum == object->ringaccChnls[attrs->txRingNumInt]) ||
       (ringNum == object->ringaccChnls[attrs->txRingNumInt]))
    {
        Udma_ringQueueRaw(object->txRingHandle,descr);
    }
    else if((ringNum == object->ringaccChnls[attrs->rxRingNumInt]) ||
            (ringNum == object->ringaccChnls[attrs->rxRingNumInt]))
    {
        Udma_ringQueueRaw(object->rxRingHandle,descr);
    }
    else
    {
        SA2UL_storageQinsert(object, descr);
    }
}

static int32_t SA2UL_setupRxChannel(SA2UL_Config *config)
{
    Udma_ChPrms         chPrms;
    Udma_ChRxPrms       rxPrms;
    Udma_RingHandle     ringHandle;
    int32_t             retVal = UDMA_SOK;
    const SA2UL_Attrs  *attrs;
    SA2UL_Params       *prms;
    SA2UL_Object       *object;

    DebugP_assert(NULL != config->attrs);
    DebugP_assert(NULL != config->object);
    attrs       = config->attrs;
    object      = config->object;
    prms        = &config->object->prms;

    if(!SA2UL_IS_ALIGNED_PTR(attrs->rxRingMemAddr, 128))
    {
        retVal = UDMA_EFAIL;
    }
    if (UDMA_SOK == retVal)
    {
        /* RX channel parameters */
        UdmaChPrms_init(&chPrms, UDMA_CH_TYPE_RX_MAPPED);
        chPrms.peerChNum            = attrs->rxPsil0ThreadId;
        chPrms.mappedChGrp          = attrs->udmaSaRxGroupNum;
        chPrms.fqRingPrms.ringMem   = (uint64_t*)attrs->rxRingMemAddr;
        chPrms.fqRingPrms.elemCnt   = attrs->ringCnt;
        chPrms.fqRingPrms.mode      = TISCI_MSG_VALUE_RM_RING_MODE_RING;
        chPrms.fqRingPrms.asel      = UDMA_RINGACC_ASEL_ENDPOINT_PHYSADDR;

        /* Open RX channel for receive from SA */
        object->rxChHandle[1]       = &object->udmaRxChObj[1];
        retVal = Udma_chOpen(object->drvHandle, object->rxChHandle[1], UDMA_CH_TYPE_RX_MAPPED, &chPrms);
        if(UDMA_SOK == retVal)
        {
            UdmaChRxPrms_init(&rxPrms, UDMA_CH_TYPE_RX_MAPPED);
            rxPrms.dmaPriority       = TISCI_MSG_VALUE_RM_UDMAP_CH_SCHED_PRIOR_MEDHIGH;
            rxPrms.fetchWordSize     = attrs->descSize >> 2;
            rxPrms.flowEInfoPresent  = 1;
            rxPrms.flowPsInfoPresent = 1;
            retVal = Udma_chConfigRx(object->rxChHandle[1], &rxPrms);
        }
        else
        {
            retVal = UDMA_EFAIL;
            DebugP_logError("error in Rx-1 Udma_chOpen()  \n");
        }
    }

    if (UDMA_SOK == retVal)
    {
        /* Update the Rx Ring numbers */
        ringHandle                   = Udma_chGetFqRingHandle(object->rxChHandle[1]);
        object->rxFreeRingHandle     = ringHandle;
        object->ringaccChnls[attrs->rxRingNumInt] = Udma_ringGetNum(ringHandle);
        object->rxFlowHandle         = Udma_chGetDefaultFlowHandle(object->rxChHandle[1]);
        object->udmapRxFlownum       = Udma_flowGetNum(object->rxFlowHandle);

        /* Update the Rx Ring numbers */
        ringHandle                   = Udma_chGetCqRingHandle(object->rxChHandle[1]);
        object->rxRingHandle         = ringHandle;
        object->ringaccChnls[attrs->rxRingNumInt] = Udma_ringGetNum(ringHandle);

        /* Create the channel for the second thread with same flow as other thread */
        UdmaChPrms_init(&chPrms, UDMA_CH_TYPE_RX_MAPPED);
        chPrms.peerChNum             = attrs->rxPsil1ThreadId;
        chPrms.mappedChGrp           = attrs->udmaSaRxGroupNum;
        chPrms.fqRingPrms.elemCnt    = attrs->ringCnt;
        chPrms.fqRingPrms.mode       = TISCI_MSG_VALUE_RM_RING_MODE_RING;
        chPrms.fqRingPrms.asel       = UDMA_RINGACC_ASEL_ENDPOINT_PHYSADDR;
        chPrms.fqRingPrms.ringMem    = (uint64_t*)attrs->rxRingMemAddr;
        object->rxChHandle[0]        = &object->udmaRxChObj[0];
    }
    retVal = Udma_chOpen(object->drvHandle, object->rxChHandle[0], UDMA_CH_TYPE_RX_MAPPED, &chPrms);
    if(UDMA_SOK == retVal)
    {
        UdmaChRxPrms_init(&rxPrms, UDMA_CH_TYPE_RX_MAPPED);
        rxPrms.dmaPriority       = TISCI_MSG_VALUE_RM_UDMAP_CH_SCHED_PRIOR_MEDHIGH;
        rxPrms.fetchWordSize     = attrs->descSize >> 2;
        rxPrms.configDefaultFlow = TRUE;
        retVal = Udma_chConfigRx(object->rxChHandle[0], &rxPrms);
    }
    else
    {
        retVal = UDMA_EFAIL;
        DebugP_logError("error in Rx-0 Udma_chOpen()  \n");
    }

    /* Enable the channel after everything is setup */
    if(UDMA_SOK == retVal)
    {
        retVal = Udma_chEnable(object->rxChHandle[1]);
        if(UDMA_SOK == retVal)
        {
            retVal = Udma_chEnable(object->rxChHandle[0]);
        }

        if(retVal != UDMA_SOK)
        {
            retVal = UDMA_EFAIL;
            DebugP_logError("error in Udma_chEnable()  \n");
        }

    }
    if(UDMA_SOK == retVal)
    {
        /* Update Rx channels */
        object->udmapRxChnum[0]      = Udma_chGetNum(object->rxChHandle[0]);
        object->udmapRxChnum[1]      = Udma_chGetNum(object->rxChHandle[1]);

        /* (SW) storage ring */
        object->ringaccChnls[attrs->swRingNumInt] = SA2UL_SW_RING_NUM;
    }

    return (retVal);
}

static int32_t SA2UL_setupTxChannel(SA2UL_Config *config)
{
    Udma_ChPrms         chPrms;
    Udma_ChTxPrms       txPrms;
    Udma_RingHandle     ringHandle;
    int32_t             retVal = UDMA_SOK;
    const SA2UL_Attrs   *attrs;
    SA2UL_Params        *prms;
    SA2UL_Object        *object;

    DebugP_assert(NULL != config->attrs);
    DebugP_assert(NULL != config->object);
    attrs = config->attrs;
    object = config->object;
    prms = &config->object->prms;

    if(!SA2UL_IS_ALIGNED_PTR(attrs->txRingMemAddr, 128))
    {
        retVal = UDMA_EFAIL;
    }
    if(UDMA_SOK == retVal)
    {
        UdmaChPrms_init(&chPrms, UDMA_CH_TYPE_TX_MAPPED);
        chPrms.mappedChGrp           = attrs->udmaSaTxGroupNum;
        chPrms.peerChNum             = attrs->txPsilThreadId | SA2UL_PSIL_DST_THREAD_OFFSET;
        chPrms.fqRingPrms.ringMem    = (uint64_t*)attrs->txRingMemAddr;
        chPrms.fqRingPrms.elemCnt    = attrs->ringCnt;

        /* this is the dual ring mode */
        chPrms.fqRingPrms.mode       = TISCI_MSG_VALUE_RM_RING_MODE_RING;

        /* Open TX channel for transmit */
        object->txChHandle           = &object->udmaTxChObj;
        object->drvHandle            = (void *)attrs->udmaHandle;
    }

    retVal = Udma_chOpen(object->drvHandle, object->txChHandle, UDMA_CH_TYPE_TX_MAPPED, &chPrms);
    if(UDMA_SOK == retVal)
    {
        UdmaChTxPrms_init(&txPrms, UDMA_CH_TYPE_TX_MAPPED);
        txPrms.dmaPriority       = TISCI_MSG_VALUE_RM_UDMAP_CH_SCHED_PRIOR_MEDHIGH;
        txPrms.fetchWordSize     = attrs->descSize >> 2;
        retVal = Udma_chConfigTx(object->txChHandle, &txPrms);
        if(UDMA_SOK == retVal)
        {
            retVal = Udma_chEnable(object->txChHandle);
            object->udmapTxChnum = Udma_chGetNum(object->txChHandle);
        }
    }
    else
    {
        retVal = UDMA_EFAIL;
        DebugP_logError("error in Tx Udma_chOpen()  \n");
    }

    if(retVal == UDMA_SOK)
    {
        /* Update the Tx Ring numbers */
        ringHandle               = Udma_chGetFqRingHandle(object->txChHandle);
        object->txRingHandle     = ringHandle;
        object->ringaccChnls[attrs->txRingNumInt] = Udma_ringGetNum(ringHandle);
        ringHandle               = Udma_chGetCqRingHandle(object->txChHandle);
        object->txComplRingHandle = ringHandle;
        object->ringaccChnls[attrs->txRingNumInt] = Udma_ringGetNum(ringHandle);
    }

    return (retVal);
}

static int32_t SA2UL_dmaInit(SA2UL_Config *config)
{
    uint32_t retVal  = SystemP_SUCCESS;
    SA2UL_Object       *object;
    SA2UL_Attrs        *attrs;
    uint8_t            cnt;
    uint64_t           phys;
    DebugP_assert(NULL != config->object);
    object = config->object;
    attrs  = config->attrs;

    object->contextId            = attrs->contextIdStart;
    object->storageQueueFree     = SA2UL_RING_N_ELEMS;

    /* setup Tx Channel */
    retVal = SA2UL_setupTxChannel(config);
    if(retVal != SystemP_SUCCESS)
    {
        DebugP_logError("error in creating the dma tx channel \n");
        retVal = SystemP_FAILURE;
    }

    /* setup Tx Channel */
    retVal = SA2UL_setupRxChannel(config);
    if(retVal != SystemP_SUCCESS)
    {
        DebugP_logError("error in creating the dma Rx channel \n");
        retVal = SystemP_FAILURE;
    }

    if(retVal == SystemP_SUCCESS)
    {
        phys = attrs->descMemAddr;
        for (cnt=0; cnt<(attrs->descMemAddr / attrs->descMemSize); cnt++)
        {
            if(cnt == (SA2UL_RING_N_ELEMS * 2))
            {
                break;
            }
            SA2UL_ringAccelWriteDescr(config,object->ringaccChnls[attrs->swRingNumInt], phys);
            phys += attrs->descSize;
        }
    }

    return (retVal);
}

static void SA2UL_u32LeToU8(uint8_t *dest, const uint32_t *src, uint32_t len)
{
    uint32_t i, t;

    for (i=0; i<len; i+=4) {
        t = *src++;
        *dest++ = t >> 24;
        *dest++ = t >> 16;
        *dest++ = t >> 8;
        *dest++ = t;
    }
}

static void SA2UL_64bEndianSwap(uint32_t *dest, const uint32_t *src, uint32_t len)
{
    uint32_t tmp[4], *t;
    uint32_t i, j;

    t = &tmp[0];
    for (i=0; i<len; i+=16) {
        for (j=0; j<4; j++) *t++ = *src++;
        for (j=0; j<4; j++) *dest++ = *--t;
    }
}

static int32_t SA2UL_getMceIndex(SA2UL_ContextObject *ctxObj, uint8_t *aesKeyInvFlag)
{
    uint32_t retVal  = SystemP_SUCCESS;
    *aesKeyInvFlag = (uint8_t) FALSE;
    if(ctxObj->ctxPrms.encAlg == SA2UL_ENC_ALG_AES &&
        ctxObj->ctxPrms.encKeySize == SA2UL_ENC_KEYSIZE_256) {
        if(ctxObj->ctxPrms.encMode == SA2UL_ENC_MODE_ECB) {
            if(ctxObj->ctxPrms.encDirection == SA2UL_ENC_DIR_DECRYPT)
                *aesKeyInvFlag = (uint8_t)TRUE;
            retVal = MCE_DATA_ARRAY_INDEX_AES_256_ECB;
        }
        else if(ctxObj->ctxPrms.encMode == SA2UL_ENC_MODE_CBC) {
            if(ctxObj->ctxPrms.encDirection == SA2UL_ENC_DIR_ENCRYPT) {
                retVal = MCE_DATA_ARRAY_INDEX_AES_256_CBC_ENCRYPT;
            } else {
                *aesKeyInvFlag = (uint8_t)TRUE;
                retVal = MCE_DATA_ARRAY_INDEX_AES_256_CBC_DECRYPT;
            }
        }
    }

    return (SystemP_FAILURE);
}

static int32_t SA2UL_pushBuffer(SA2UL_ContextObject *pCtxObj,const uint8_t  *input, uint32_t ilen, uint8_t  *output)
{
    uint32_t retVal = SystemP_SUCCESS;
    struct SA2UL_HostDescrTx *txDescr;
    struct SA2UL_HostDescrRx *rxDescr;
    uint64_t phys, authLen;
    uint32_t lenTBP;
    SA2UL_Object *object;
    SA2UL_Attrs  *attrs;
    SA2UL_Config *config;
    config = (SA2UL_Config *)pCtxObj->handle;
    object = (SA2UL_Object *)config->object;
    attrs  = (SA2UL_Attrs *)config->attrs;

    if((pCtxObj->txBytesCnt + ilen) > pCtxObj->totalLengthInByes)
        retVal = SystemP_FAILURE;
    if(SystemP_SUCCESS == retVal)
    {
        if(ilen > CSL_FEXT(0xffffffffu, UDMAP_CPPI5_PD_DESCINFO_PKTLEN))
        {
            /* Packet length must not be greater than the the field
            * size in Host pkt descriptor */
            retVal = SystemP_FAILURE;
        }
    
        lenTBP = CSL_FEXT(0xffffffffu, SA2UL_CMDLBLHDR1_LEN_TO_BE_PROCESSESED);
        if(ilen > lenTBP)
        {
            if(pCtxObj->ctxPrms.opType == SA2UL_OP_ENC)
            {
                /* Packet length must not be greater than the the field
                * size in command label for encryption */
                retVal = SystemP_FAILURE;
            }
        }
        else
        {
            lenTBP = ilen;
        }
    }
    /* Check the occupancy of storage queue */
    if(SA2UL_getStorageRingOcc(object, attrs->swRingNumInt) < 2u)
    {
        retVal = SystemP_FAILURE;
    }
    if(SystemP_SUCCESS == retVal)
    {
        phys = SA2UL_ringAccelReadDescr(config, object->ringaccChnls[attrs->swRingNumInt]);
        txDescr = (struct SA2UL_HostDescrTx *)(phys);

        phys = SA2UL_ringAccelReadDescr(config, object->ringaccChnls[attrs->swRingNumInt]);
        rxDescr = (struct SA2UL_HostDescrRx *)(phys);

        memset(txDescr, 0, sizeof(struct SA2UL_HostDescrTx));
        txDescr->pd.descInfo =
            /* Host descriptor type */
            CSL_FMK(UDMAP_CPPI5_PD_DESCINFO_DTYPE, CSL_UDMAP_CPPI5_PD_DESCINFO_DTYPE_VAL_HOST) |
            /* extended info is present */
            CSL_FMK(UDMAP_CPPI5_PD_DESCINFO_EINFO, CSL_UDMAP_CPPI5_PD_DESCINFO_EINFO_VAL_IS_PRESENT) |
            /* 12 bytes of protocol specific info in desc */
            CSL_FMK(UDMAP_CPPI5_PD_DESCINFO_PSWCNT, 12u >> 2) |
            /* packet length */
            CSL_FMK(UDMAP_CPPI5_PD_DESCINFO_PKTLEN, ilen);

        txDescr->pd.pktInfo1 =
            CSL_FMK(UDMAP_CPPI5_PD_PKTINFO1_FLOWID, object->udmapRxFlownum);

        txDescr->pd.pktInfo2 =
            CSL_FMK(UDMAP_CPPI5_PD_PKTINFO2_RETQ, object->ringaccChnls[attrs->txRingNumInt]);

        txDescr->exPktInfo.swWord0 =
            CSL_FMK(SA2UL_SWWORD0_CPPI_DST_INFO_PRESENT, 1u) |
            CSL_FMK(SA2UL_SWWORD0_CMD_LBL_PRESENT, 1u) |
            CSL_FMK(SA2UL_SWWORD0_CMD_LBL_OFFSET, 0u) |
            CSL_FMK(SA2UL_SWWORD0_SCID, pCtxObj->secCtxId);

        txDescr->psData.cmdLblHdr1 =
            CSL_FMK(SA2UL_CMDLBLHDR1_LEN_TO_BE_PROCESSESED, lenTBP) |
            CSL_FMK(SA2UL_CMDLBLHDR1_CMD_LABEL_LEN, 8u) |
            CSL_FMK(SA2UL_CMDLBLHDR1_NEXT_ENGINE_SELECT_CODE,
                    SA2UL_ENGINE_CODE_DEFAULT_EGRESS_PORT);

        if(pCtxObj->ctxPrms.opType == SA2UL_OP_AUTH)
        {
            txDescr->exPktInfo.swWord0 |= (CSL_FMK(SA2UL_SWWORD0_ENGINE_ID,
            SA2UL_ENGINE_CODE_AUTHENTICATION_MODULE_P1));

            if((pCtxObj->txBytesCnt + ilen) == pCtxObj->totalLengthInByes)
            {
                /* Evict and teardown security context after last packet */
                txDescr->exPktInfo.swWord0 |= (CSL_FMK(SA2UL_SWWORD0_TEARDOWN, 1u) | CSL_FMK(SA2UL_SWWORD0_EVICT, 1u));
            }
            else
            {
                /* Set the fragment bit */
                txDescr->exPktInfo.swWord0 |= (CSL_FMK(SA2UL_SWWORD0_BYP_CMD_LBL_LEN, 1u) | CSL_FMK(SA2UL_SWWORD0_FRAGMENT, 1u));
            }

            if(pCtxObj->ctxPrms.inputLen > CSL_FEXT(0xffffffffu, SA2UL_CMDLBLHDR1_LEN_TO_BE_PROCESSESED))
            {
                /* Update protocol specific data length, add 8 */
                CSL_FINS(txDescr->pd.descInfo,
                    UDMAP_CPPI5_PD_DESCINFO_PSWCNT, 20u >> 2);

                /* Update command header data length, add 8 */
                CSL_FINS(txDescr->psData.cmdLblHdr1,
                    SA2UL_CMDLBLHDR1_CMD_LABEL_LEN, 16u);

                /* Add option 1 to replace the Authnentication length
                * parameter in context */
                txDescr->psData.cmdLblHdr2 =
                    CSL_FMK(SA2UL_CMDLBLHDR2_OPTION1_CTX_OFFSET, 1u) |
                    CSL_FMK(SA2UL_CMDLBLHDR2_OPTION1_LEN, 1u);

                /* Authentication length in bits */
                authLen = ((uint64_t)pCtxObj->ctxPrms.inputLen) << 3;

                /* Add one block length of additional processing size
                * for HMAC */
                if(SA2UL_IS_HMAC(pCtxObj->ctxPrms.hashAlg)) {
                    authLen += sa2ulHashBlkSizeBits[pCtxObj->ctxPrms.hashAlg & 7u];
                }
            
            txDescr->psData.optionWords[0] = (uint32_t)(authLen >> 32);
            txDescr->psData.optionWords[1] = (uint32_t)authLen;
            }
        }
    
        else
        {
            txDescr->exPktInfo.swWord0 |= (CSL_FMK(SA2UL_SWWORD0_ENGINE_ID, SA2UL_ENGINE_CODE_ENCRYPTION_MODULE_P1));
            if((pCtxObj->txBytesCnt + ilen) == pCtxObj->ctxPrms.inputLen)
            {
                /* Evict and teardown security context after last packet*/
                txDescr->exPktInfo.swWord0 |= (CSL_FMK(SA2UL_SWWORD0_TEARDOWN, 1u) | CSL_FMK(SA2UL_SWWORD0_EVICT, 1u));
            }
        }
        /* Load SecContec pointer in extended PktInfo */
        phys = (uint64_t)(&pCtxObj->secCtx);
        txDescr->exPktInfo.scptrL = (uint32_t)phys;
        txDescr->exPktInfo.scptrH = ((uint32_t)(phys >> 32));
        if(pCtxObj->ctxPrms.opType == SA2UL_OP_AUTH)
        {
            txDescr->exPktInfo.scptrH |= (CSL_FMK(SA2UL_SCPTRH_EGRESS_CPPI_STATUS_LEN, sa2ulHashSizeBytes[pCtxObj->ctxPrms.hashAlg & 7u]));
        }

        /* Egress CPPI Destination Queue */
        txDescr->psData.inPsiInfo = CSL_FMK(SA2UL_INPSIINFO_EGRESS_CPPI_DEST_QUEUE_NUM, object->ringaccChnls[attrs->rxRingNumInt]);

        txDescr->pd.bufPtr = (uint64_t)input;
        txDescr->pd.bufInfo1 = ilen;
        txDescr->pd.orgBufPtr = (uint64_t)input;
        txDescr->pd.orgBufLen = ilen;

        /* Create a Rx descriptor */
        memset(rxDescr, 0, sizeof(struct SA2UL_HostDescrRx));
        rxDescr->pd.descInfo =
        /* Host descriptor type */
        CSL_FMK(UDMAP_CPPI5_PD_DESCINFO_DTYPE, CSL_PKTDMA_CPPI5_PD_DESCINFO_DTYPE_VAL_HOST);
        rxDescr->pd.bufPtr = (uint64_t)output;
        rxDescr->pd.bufInfo1 = ilen;
        rxDescr->pd.orgBufPtr = (uint64_t)output;
        rxDescr->pd.orgBufLen = ilen;
    }
    /* Check ring accelerator accupancy */
    if((Udma_ringGetForwardRingOcc(object->rxRingHandle) == SA2UL_RING_N_ELEMS) ||
        (Udma_ringGetForwardRingOcc(object->txRingHandle) == SA2UL_RING_N_ELEMS))
    {
        retVal = SystemP_FAILURE;
    }
    if(SystemP_SUCCESS == retVal)
    {
        /* Perform cache writeback */
        CacheP_wb(rxDescr, attrs->descSize, CacheP_TYPE_ALLD);

        /* Perform cache writeback */
        CacheP_inv(rxDescr, attrs->descSize, CacheP_TYPE_ALLD);

        /* Perform cache writeback */
        CacheP_wb(txDescr, attrs->descSize, CacheP_TYPE_ALLD);

        /* Perform cache writeback */
        CacheP_inv(txDescr, attrs->descSize, CacheP_TYPE_ALLD);

        /* Push the RX descriptor to RX free ring */
        phys = (uint64_t)(rxDescr);
        SA2UL_ringAccelWriteDescr(config,object->ringaccChnls[attrs->rxRingNumInt], phys);

        /* Push the TX descriptor to TX send ring*/
        phys = (uint64_t)(txDescr);
        SA2UL_ringAccelWriteDescr(config,object->ringaccChnls[attrs->txRingNumInt], phys);

        pCtxObj->txBytesCnt += ilen;

        /* Perform cache writeback */
        CacheP_wb(output, ilen, CacheP_TYPE_ALLD);

        /* Perform cache writeback */
        CacheP_inv(output, ilen, CacheP_TYPE_ALLD);
    }
    return (retVal);
}

static int32_t SA2UL_popBuffer(SA2UL_ContextObject *pCtxObj, uint64_t *doneBuf, uint32_t *doneBufSize, uint8_t *dataTransferDone)
{
    uint32_t retVal = SystemP_SUCCESS;
    struct SA2UL_HostDescrTx *txDescr;
    struct SA2UL_HostDescrRx *rxDescr;
    uint64_t phys;
    uint32_t reg;
    SA2UL_Object *object;
    SA2UL_Attrs  *attrs;
    SA2UL_Config *config;
    config = (SA2UL_Config *)pCtxObj->handle;
    attrs  = (SA2UL_Attrs  *)config->attrs;
    object = (SA2UL_Object *)config->object;
    *dataTransferDone = (uint8_t)FALSE;

    /* Check the occupancy of Tx complete and RX done queues  */
    if((Udma_ringGetReverseRingOcc(object->rxRingHandle) == 0) || \
       (Udma_ringGetReverseRingOcc(object->txRingHandle) == 0))
    {
        retVal = SystemP_FAILURE;
    }
    if(SystemP_SUCCESS == retVal)
    {
        /* Read the TX done descriptor from TX free ring for cleanup */
        phys = SA2UL_ringAccelReadDescr(config,(uint32_t)object->ringaccChnls[attrs->txRingNumInt]);
        SA2UL_ringAccelWriteDescr(config,(uint32_t)object->ringaccChnls[attrs->swRingNumInt], phys);
        txDescr = (struct SA2UL_HostDescrTx *)(phys);

        *doneBuf = txDescr->pd.bufPtr;
        *doneBufSize = txDescr->pd.bufInfo1;

        /* Read the RX done descriptor from RX done ring */
        phys = SA2UL_ringAccelReadDescr(config,object->ringaccChnls[attrs->rxRingNumInt]);
        SA2UL_ringAccelWriteDescr(config,object->ringaccChnls[attrs->swRingNumInt], phys);
        rxDescr = (struct SA2UL_HostDescrRx *)(phys);

        /* Check for protocol-specific flags for SA2UL errors */
        if(0u != CSL_FEXT(rxDescr->pd.pktInfo1,
                    UDMAP_CPPI5_PD_PKTINFO1_PSFLGS))
                pCtxObj->sa2ulErrCnt ++;

        reg = CSL_FEXT(rxDescr->pd.descInfo, PKTDMA_CPPI5_PD_DESCINFO_PKTLEN);
        pCtxObj->rxBytesCnt += reg;

        if(pCtxObj->rxBytesCnt == pCtxObj->ctxPrms.inputLen)
        {
            if(pCtxObj->ctxPrms.opType == SA2UL_OP_AUTH)
            {
                /* Check for protocol-specific data for SA2UL hash ouput */
                reg = CSL_FEXT(rxDescr->pd.descInfo,
                        PKTDMA_CPPI5_PD_DESCINFO_PSWCNT);

                if((reg << 2) == sa2ulHashSizeBytes[pCtxObj->ctxPrms.hashAlg & 7u])
                {

                    if(pCtxObj->sa2ulErrCnt == 0u)
                    {
                        /* Copy final hash value from last descriptor */
                        SA2UL_u32LeToU8(pCtxObj->computedHash, &rxDescr->psData.trailerData[0], sa2ulHashSizeBytes[pCtxObj->ctxPrms.hashAlg & 7u]);

                        pCtxObj->computationStatus = 0;
                    }
                }
            }
            else
            {
                pCtxObj->computationStatus = 0;
            }

            *dataTransferDone =  (uint8_t)TRUE;
        }
    }

    return (retVal);
}

static int32_t SA2UL_hwInit(SA2UL_Attrs  *attrs)
{
    uint32_t retVal           = SystemP_SUCCESS;
    uint32_t                    reg;
    CSL_Cp_aceRegs *pSaRegs = (CSL_Cp_aceRegs *)attrs->saBaseAddr;
    /* Is sha enabled in efuses */
    reg = CSL_FEXT(CSL_REG_RD(&pSaRegs->MMR.EFUSE_EN), CP_ACE_EFUSE_EN_ENABLE);
    if((reg & 1u) == 0u)
    {
        retVal = SystemP_FAILURE;
    }
    else
    {
        /* Enable specific SA2UL engine modules */
        reg = CSL_REG_RD(&pSaRegs->UPDATES.ENGINE_ENABLE);
        CSL_FINS(reg, CP_ACE_UPDATES_ENGINE_ENABLE_CTX_EN, 1u);
        CSL_FINS(reg, CP_ACE_UPDATES_ENGINE_ENABLE_CDMA_IN_EN, 1u);
        CSL_FINS(reg, CP_ACE_UPDATES_ENGINE_ENABLE_CDMA_OUT_EN, 1u);
        CSL_FINS(reg, CP_ACE_UPDATES_ENGINE_ENABLE_PKA_EN, 1u);
        CSL_FINS(reg, CP_ACE_UPDATES_ENGINE_ENABLE_TRNG_EN, 1u);
        CSL_FINS(reg, CP_ACE_UPDATES_ENGINE_ENABLE_AUTHSS_EN, 1u);
        CSL_FINS(reg, CP_ACE_UPDATES_ENGINE_ENABLE_ENCSS_EN, 1u);
        CSL_REG_WR(&pSaRegs->UPDATES.ENGINE_ENABLE, reg);

        reg = CSL_CP_ACE_CMD_STATUS_CTXCACH_EN_MASK       |
              CSL_CP_ACE_CMD_STATUS_CDMA_IN_PORT_EN_MASK  |
              CSL_CP_ACE_CMD_STATUS_CDMA_OUT_PORT_EN_MASK |
              CSL_CP_ACE_CMD_STATUS_PKA_EN_MASK           |
              CSL_CP_ACE_CMD_STATUS_TRNG_EN_MASK          |
              CSL_CP_ACE_CMD_STATUS_ENCSS_EN_MASK         |
              CSL_CP_ACE_CMD_STATUS_AUTHSS_EN_MASK;        

        /* Consider timeout */
        while ((reg & CSL_REG_RD(&pSaRegs->MMR.CMD_STATUS)) != reg)
        {
        }
        /* incase timeout */
        if((reg & CSL_REG_RD(&pSaRegs->MMR.CMD_STATUS)) != reg)
        {
            retVal = SystemP_FAILURE;
        }
    }
    return (retVal);

}

static uint32_t SA2UL_hwDeInit(SA2UL_Attrs  *attrs)
{
    uint32_t retVal           = SystemP_SUCCESS;
    uint32_t                    reg;
    CSL_Cp_aceRegs *pSaRegs = (CSL_Cp_aceRegs *)attrs->saBaseAddr;
    /* Is sha enabled in efuses */
    reg = CSL_FEXT(CSL_REG_RD(&pSaRegs->MMR.EFUSE_EN), CP_ACE_EFUSE_EN_ENABLE);
    if((reg & 1u) == 0u)
    {
        retVal = SystemP_FAILURE;
    }
    else
    {
        /* Enable specific SA2UL engine modules */
        reg = CSL_REG_RD(&pSaRegs->UPDATES.ENGINE_ENABLE);
        CSL_FINS(reg, CP_ACE_UPDATES_ENGINE_ENABLE_CTX_EN, 0u);
        CSL_FINS(reg, CP_ACE_UPDATES_ENGINE_ENABLE_CDMA_IN_EN, 0u);
        CSL_FINS(reg, CP_ACE_UPDATES_ENGINE_ENABLE_CDMA_OUT_EN, 0u);
        CSL_FINS(reg, CP_ACE_UPDATES_ENGINE_ENABLE_PKA_EN, 0u);
        CSL_FINS(reg, CP_ACE_UPDATES_ENGINE_ENABLE_TRNG_EN, 0u);
        CSL_FINS(reg, CP_ACE_UPDATES_ENGINE_ENABLE_AUTHSS_EN, 0u);
        CSL_FINS(reg, CP_ACE_UPDATES_ENGINE_ENABLE_ENCSS_EN, 0u);
        CSL_REG_WR(&pSaRegs->UPDATES.ENGINE_ENABLE, reg);

        reg = CSL_CP_ACE_CMD_STATUS_CTXCACH_EN_MASK       |
              CSL_CP_ACE_CMD_STATUS_CDMA_IN_PORT_EN_MASK  |
              CSL_CP_ACE_CMD_STATUS_CDMA_OUT_PORT_EN_MASK |
              CSL_CP_ACE_CMD_STATUS_PKA_EN_MASK           |
              CSL_CP_ACE_CMD_STATUS_TRNG_EN_MASK          |
              CSL_CP_ACE_CMD_STATUS_ENCSS_EN_MASK         |
              CSL_CP_ACE_CMD_STATUS_AUTHSS_EN_MASK;        
    }

    return (retVal);
}

static int32_t SA2UL_checkOpenParams(const SA2UL_Params *prms)
{
    int32_t     retVal = SystemP_SUCCESS;

    return (retVal);
}

int32_t SA2UL_rngSetup(SA2UL_Handle handle)
{
    uint32_t val = 0, updated_bits = 0, retVal = SystemP_SUCCESS;
    SA2UL_Config     *config  = (SA2UL_Config *)handle;
    CSL_Cp_aceRegs   *pSaRegs = (CSL_Cp_aceRegs *)config->attrs->saBaseAddr;
    CSL_Cp_aceTrngRegs *pTrngRegs = &pSaRegs->TRNG;
    CSL_main_ctrl_mmr_cfg0Regs *pMainMmrCtrl =(CSL_main_ctrl_mmr_cfg0Regs *) CSL_CTRL_MMR0_CFG0_BASE;

    CSL_REG_WR(&pTrngRegs->TRNG_CONTROL, 0U);
    CSL_REG_WR(&pTrngRegs->TRNG_CONTROL, 0U);

    /* Initialize TRNG_CONFIG to 0 */
	val = ((uint32_t) 0U);
	val |= ((((uint32_t) 5U) << CSL_CP_ACE_TRNG_CONFIG_SAMPLE_CYCLES_SHIFT) & (CSL_CP_ACE_TRNG_CONFIG_SAMPLE_CYCLES_MASK));
    CSL_REG_WR(&pTrngRegs->TRNG_CONFIG, val);

    /* Leave the ALARMCNT register at its reset value */
	val = ((uint32_t) 0xFFU);
    CSL_REG_WR(&pTrngRegs->TRNG_ALARMCNT, val);

    /* write zeros to ALARMMASK and ALARMSTOP registers */
	val = ((uint32_t) 0U);
	CSL_REG_WR(&pTrngRegs->TRNG_ALARMMASK, val);
	CSL_REG_WR(&pTrngRegs->TRNG_ALARMSTOP, val);

    /* We have 8 FRO's in the RNG */
	val = ((uint32_t) 0xFFU);
    CSL_REG_WR(&pTrngRegs->TRNG_FROENABLE, val);

    /* Enable TRNG Section 5.2.5 */
	val = ((((uint32_t) 1U) << CSL_CP_ACE_TRNG_CONTROL_ENABLE_TRNG_SHIFT));
    CSL_REG_WR(&pTrngRegs->TRNG_CONTROL, val);

    /* Initial Seed values:
    *JTAGID, JTAG_USER_ID and DIE_ID0 to 3, In future additional device specific ids will add for rest of fields */
    pTrngRegs->TRNG_PS_AI_0  = (uint32_t)pMainMmrCtrl->JTAGID;
    pTrngRegs->TRNG_PS_AI_1  = (uint32_t)pMainMmrCtrl->JTAG_USER_ID;
    pTrngRegs->TRNG_PS_AI_2  = (uint32_t)pMainMmrCtrl->DIE_ID0;
    pTrngRegs->TRNG_PS_AI_3  = (uint32_t)pMainMmrCtrl->DIE_ID1;
    pTrngRegs->TRNG_PS_AI_4  = (uint32_t)pMainMmrCtrl->DIE_ID2;
    pTrngRegs->TRNG_PS_AI_5  = (uint32_t)pMainMmrCtrl->DIE_ID3;
    pTrngRegs->TRNG_PS_AI_6  = 0x2111ecc9;
    pTrngRegs->TRNG_PS_AI_7  = 0x122111ec;
    pTrngRegs->TRNG_PS_AI_8  = 0x6574b6d7;
    pTrngRegs->TRNG_PS_AI_9  = 0x02cd76ac;
    pTrngRegs->TRNG_PS_AI_10 = 0x76acadd3;
    pTrngRegs->TRNG_PS_AI_11 = 0x74b6d702;
    val = CSL_REG_RD(&pTrngRegs->TRNG_CONTROL);

    /* We always request the maximum number of blocks possible */
	updated_bits |= CSL_CP_ACE_TRNG_CONTROL_DATA_BLOCKS_MASK;
	/* Set the request data bit */
	updated_bits |= CSL_CP_ACE_TRNG_CONTROL_REQUEST_DATA_MASK;

	val |= updated_bits;

	CSL_REG_WR(&pTrngRegs->TRNG_CONTROL, val);

    return (retVal);

}

int32_t SA2UL_rngRead(SA2UL_Handle handle, uint32_t *out)
{
    int32_t retVal = SystemP_SUCCESS;
    uint32_t val = 0U, mask = 0U, i = 0U, j = 0U;
    uint32_t ready = 0U;
    SA2UL_Config        *config         = (SA2UL_Config *)handle;
    CSL_Cp_aceRegs      *pSaRegs        = (CSL_Cp_aceRegs *)config->attrs->saBaseAddr;
    CSL_Cp_aceTrngRegs  *pTrngRegs      = &pSaRegs->TRNG;

    val = CSL_REG_RD(&pTrngRegs->TRNG_CONTROL);

    mask = ((((uint32_t) 1U) << CSL_CP_ACE_TRNG_CONTROL_ENABLE_TRNG_SHIFT));

	if ((val & mask) == mask)
    {
		retVal = SystemP_SUCCESS;
	}

    else
    {
		retVal = SystemP_FAILURE;
        DebugP_assert(SystemP_SUCCESS == retVal);
	}

    if(SystemP_SUCCESS == retVal)
    {
        /* Check if random data is available */
        j = i*4;
		val = CSL_REG_RD(&pTrngRegs->TRNG_STATUS);
		ready =  (val & CSL_CP_ACE_TRNG_STATUS_READY_MASK) >> CSL_CP_ACE_TRNG_STATUS_READY_SHIFT;
		while (ready != 1U)
        {
            val = CSL_REG_RD(&pTrngRegs->TRNG_STATUS);
            ready =  (val & CSL_CP_ACE_TRNG_STATUS_READY_MASK) >> CSL_CP_ACE_TRNG_STATUS_READY_SHIFT;
        }

		if ( SystemP_SUCCESS == retVal )
        {
            /* If data is available, read it into the output buffer */
			out[0]  = CSL_REG_RD(&pTrngRegs->TRNG_INPUT_0);
            out[1]  = CSL_REG_RD(&pTrngRegs->TRNG_INPUT_1);
            out[2]  = CSL_REG_RD(&pTrngRegs->TRNG_INPUT_2);
            out[3]  = CSL_REG_RD(&pTrngRegs->TRNG_INPUT_3);

			/*Set the INTACK and go back*/
			CSL_REG_WR(&pTrngRegs->TRNG_STATUS, (CSL_CP_ACE_TRNG_INTACK_READY_ACK_MASK << CSL_CP_ACE_TRNG_INTACK_READY_ACK_SHIFT));
		}
	}
    return (retVal);
}