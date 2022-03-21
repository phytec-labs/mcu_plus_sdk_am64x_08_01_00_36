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
 *  \file   crypto_aes.c
 *
 *  \brief  This file contains the wrapper for Advance Encryption Standard (AES) driver
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdlib.h>
#include <string.h>
#include <kernel/dpl/SystemP.h>
#include <security/crypto/include/crypto_aes.h>

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

static Crypto_AesFxns *Crypto_getAesFxns(uint32_t type);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

void Crypto_AesParams_init(Crypto_AesParams *params)
{
    if(params)
    {
        params->aesMode = CRYPTO_AES_CBC_256;
        params->algoType = CRYPTO_AES_ENCRYPT;
        memset( &params->iv, 0, CRYPTO_AES_IV_LENGTH);
        memset( &params->key, 0, CRYPTO_AES_KEY_LENGTH);
        params->keySizeInBits = 256;
        params->type     = CRYPTO_TYPE_SW;
    }
}

Crypto_AesHandle Crypto_aesOpen(Crypto_AesContext *ctx, const Crypto_AesParams *params)
{
    int32_t             status;
    Crypto_AesFxns      *fxns;
    Crypto_AesHandle    handle = NULL;

    if(ctx && params)
    {
        fxns = Crypto_getAesFxns(params->type);
        if(fxns && fxns->commonFxns->openFxn)
        {
            ctx->fxns = fxns;
            status = ctx->fxns->commonFxns->openFxn(ctx, params);
            if(status == SystemP_SUCCESS)
            {
                handle = (Crypto_AesHandle) ctx;
            }
        }
    }

    return (handle);
}

int32_t Crypto_aesClose(Crypto_AesHandle handle)
{
    int32_t             status = SystemP_FAILURE;
    Crypto_AesContext  *ctx = (Crypto_AesContext *) handle;

    if(ctx && ctx->fxns && ctx->fxns->commonFxns->closeFxn)
    {
        status = ctx->fxns->commonFxns->closeFxn(ctx);
    }

    return (status);
}

int32_t Crypto_aesSetKeyEnc(Crypto_AesHandle handle)
{
    int32_t             status = SystemP_FAILURE;
    Crypto_AesContext  *ctx = (Crypto_AesContext *) handle;

    if(ctx && ctx->fxns && ctx->fxns->cbcFxns->setKeyEncFxn)
    {
        status = ctx->fxns->cbcFxns->setKeyEncFxn(ctx);
    }

    return (status);
}

int32_t Crypto_aesSetKeyDec(Crypto_AesHandle handle)
{
    int32_t             status = SystemP_FAILURE;
    Crypto_AesContext  *ctx = (Crypto_AesContext *) handle;

    if(ctx && ctx->fxns && ctx->fxns->cbcFxns->setKeyDecFxn)
    {
        status = ctx->fxns->cbcFxns->setKeyDecFxn(ctx);
    }

    return (status);
}

int32_t Crypto_aesCbc(Crypto_AesHandle handle, const uint8_t *input, uint32_t ilen, uint8_t *output )
{
    int32_t             status = SystemP_FAILURE;
    Crypto_AesContext  *ctx = (Crypto_AesContext *) handle;

    if(ctx && ctx->fxns && ctx->fxns->cbcFxns->cbcFxn)
    {
        status = ctx->fxns->cbcFxns->cbcFxn(ctx, input, ilen, output);
    }

    return (status);
}

int32_t Crypto_cmacSetup(Crypto_AesHandle handle)
{
    int32_t             status = SystemP_FAILURE;
    Crypto_AesContext  *ctx = (Crypto_AesContext *) handle;

    if(ctx && ctx->fxns && ctx->fxns->cmacFxns->setUpFxn)
    {
        status = ctx->fxns->cmacFxns->setUpFxn(ctx);
    }

    return (status);
}

int32_t Crypto_cmacStarts(Crypto_AesHandle handle)
{
    int32_t             status = SystemP_FAILURE;
    Crypto_AesContext  *ctx = (Crypto_AesContext *) handle;

    if(ctx && ctx->fxns && ctx->fxns->cmacFxns->startFxn)
    {
        status = ctx->fxns->cmacFxns->startFxn(ctx);
    }

    return (status);
}

int32_t Crypto_cmacUpdate(Crypto_AesHandle handle, const uint8_t *input, uint32_t ilen)
{
    int32_t             status = SystemP_FAILURE;
    Crypto_AesContext  *ctx = (Crypto_AesContext *) handle;

    if(ctx && ctx->fxns && ctx->fxns->cmacFxns->updateFxn)
    {
        status = ctx->fxns->cmacFxns->updateFxn(ctx, input, ilen);
    }

    return (status);
}

int32_t Crypto_cmacFinish(Crypto_AesHandle handle, uint8_t *output)
{
    int32_t             status = SystemP_FAILURE;
    Crypto_AesContext  *ctx = (Crypto_AesContext *) handle;

    if(ctx && ctx->fxns && ctx->fxns->cmacFxns->finishFxn)
    {
        status = ctx->fxns->cmacFxns->finishFxn(ctx, output);
    }

    return (status);
}

int32_t Crypto_cmacSingleShot(Crypto_AesHandle handle, const uint8_t *input, uint32_t ilen, uint8_t *output)
{
    int32_t             status = SystemP_FAILURE;
    Crypto_AesContext  *ctx = (Crypto_AesContext *) handle;

    if(ctx && ctx->fxns && ctx->fxns->cmacFxns->singleShotFxn)
    {
        status = ctx->fxns->cmacFxns->singleShotFxn(ctx, input, ilen, output);
    }

    return (status);
}

static Crypto_AesFxns *Crypto_getAesFxns(uint32_t type)
{
    Crypto_AesFxns *fxns = NULL;

    if(type < CRYPTO_NUM_TYPE)
    {
        fxns = gCryptoAesConfig[type].fxns;
    }

    return (fxns);
}