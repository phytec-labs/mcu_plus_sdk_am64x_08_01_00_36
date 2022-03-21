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
 *  \file   crypto_sha.c
 *
 *  \brief  This file contains the wrapper for Secure Hash Algorithem (SHA) driver
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdlib.h>
#include <kernel/dpl/SystemP.h>
#include <security/crypto/include/crypto_sha.h>

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

static Crypto_ShaFxns *Crypto_getShaFxns(uint32_t type);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

void Crypto_ShaParams_init(Crypto_ShaParams *params)
{
    if(params)
    {
        params->authMode = CRYPTO_SHA_AUTHMODE_SHA2_512;
        params->type     = CRYPTO_TYPE_HW;
    }
}

Crypto_ShaHandle Crypto_shaOpen(Crypto_ShaContext *ctx, const Crypto_ShaParams *params)
{
    int32_t             status;
    Crypto_ShaFxns     *fxns;
    Crypto_ShaHandle    handle = NULL;

    if(ctx && params)
    {
        fxns = Crypto_getShaFxns(params->type);
        if(fxns && fxns->openFxn)
        {
            ctx->fxns = fxns;
            status = ctx->fxns->openFxn(ctx, params);
            if(status == SystemP_SUCCESS)
            {
                handle = (Crypto_ShaHandle) ctx;
            }
        }
    }
    return (handle);
}

int32_t Crypto_shaClose(Crypto_ShaHandle handle)
{
    int32_t             status = SystemP_FAILURE;
    Crypto_ShaContext  *ctx = (Crypto_ShaContext *) handle;

    if(ctx && ctx->fxns && ctx->fxns->closeFxn)
    {
        status = ctx->fxns->closeFxn(ctx);
    }

    return (status);
}

int32_t Crypto_shaStarts(Crypto_ShaHandle handle)
{
    int32_t             status = SystemP_FAILURE;
    Crypto_ShaContext  *ctx = (Crypto_ShaContext *) handle;

    if(ctx && ctx->fxns && ctx->fxns->startFxn)
    {
        status = ctx->fxns->startFxn(ctx);
    }

    return (status);
}

int32_t Crypto_shaUpdate(Crypto_ShaHandle handle, const uint8_t *input, uint32_t ilen)
{
    int32_t             status = SystemP_FAILURE;
    Crypto_ShaContext  *ctx = (Crypto_ShaContext *) handle;

    if(ctx && ctx->fxns && ctx->fxns->updateFxn)
    {
        status = ctx->fxns->updateFxn(ctx, input, ilen);
    }

    return (status);
}

int32_t Crypto_shaFinish(Crypto_ShaHandle handle, uint8_t *output)
{
    int32_t             status = SystemP_FAILURE;
    Crypto_ShaContext  *ctx = (Crypto_ShaContext *) handle;

    if(ctx && ctx->fxns && ctx->fxns->finishFxn)
    {
        status = ctx->fxns->finishFxn(ctx, output);
    }

    return (status);
}

int32_t Crypto_shaSingleShot(Crypto_ShaHandle handle, const uint8_t *input, uint32_t ilen, uint8_t *output)
{
    int32_t             status = SystemP_FAILURE;
    Crypto_ShaContext  *ctx = (Crypto_ShaContext *) handle;

    if(ctx && ctx->fxns && ctx->fxns->singleShotFxn)
    {
        status = ctx->fxns->singleShotFxn(ctx, input, ilen, output);
    }

    return (status);
}

int32_t Crypto_hmacSha(Crypto_ShaHandle handle, const uint8_t *input, uint32_t ilen, uint8_t *output)
{
    int32_t             status = SystemP_FAILURE;
    Crypto_ShaContext  *ctx = (Crypto_ShaContext *) handle;

    if(ctx && ctx->fxns && ctx->fxns->hmacShaFxn)
    {
        status = ctx->fxns->hmacShaFxn(ctx, input, ilen, output);
    }

    return (status);
}

static Crypto_ShaFxns *Crypto_getShaFxns(uint32_t type)
{
    Crypto_ShaFxns *fxns = NULL;

    if(type < CRYPTO_NUM_TYPE)
    {
        fxns = gCryptoShaConfig[type].fxns;
    }

    return (fxns);
}
