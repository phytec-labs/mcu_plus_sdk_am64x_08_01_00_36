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
 *  \file   crypto_sha_hw.c
 *
 *  \brief  This file contains the implementation of Secure Hash Algorithem (SHA) driver
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <string.h>
#include <kernel/dpl/SystemP.h>
#include <kernel/dpl/DebugP.h>
#include <security/crypto/hw/crypto_sha_hw.h>
#include <security/sa2ul.h>

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

/* None */

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/** HW sha function table */
Crypto_ShaFxns gCryptoShaHwFxns =
{
    .openFxn        = Crypto_shaHwOpen,
    .closeFxn       = Crypto_shaHwClose,
    .startFxn       = Crypto_shaHwStarts,
    .updateFxn      = Crypto_shaHwUpdate,
    .finishFxn      = Crypto_shaHwFinish,
    .singleShotFxn  = Crypto_shaHwSingleShot,
};

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

int32_t Crypto_shaHwOpen(Crypto_ShaContext *ctx, const Crypto_ShaParams *params)
{
    int32_t status = SystemP_SUCCESS;
    SA2UL_Params prms;

    if((NULL == ctx) || (NULL == params))
    {
        status = SystemP_FAILURE;
    }
    else
    {
        memcpy(&ctx->params, params, sizeof(ctx->params));

        if(status == SystemP_SUCCESS)
        {
            ctx->drvHandle = SA2UL_open(0,&prms);
        }
        if(ctx->drvHandle == NULL)
        {
            status = SystemP_FAILURE;
        }
    }
    return (status);
}

int32_t Crypto_shaHwClose(Crypto_ShaContext *ctx)
{
    int32_t status = SystemP_SUCCESS;

    if(NULL != ctx)
    {
        SA2UL_close(ctx->drvHandle);
        memset(ctx, 0, sizeof(Crypto_ShaContext));
    }
    return (status);
}

int32_t Crypto_shaHwStarts(Crypto_ShaContext *ctx)
{
    int32_t status = SystemP_FAILURE;

    return (status);
}

int32_t Crypto_shaHwUpdate(Crypto_ShaContext *ctx, const uint8_t *input, uint32_t ilen)
{
    int32_t status = SystemP_FAILURE;

    return (status);
}

int32_t Crypto_shaHwFinish(Crypto_ShaContext *ctx, uint8_t *output)
{
    int32_t status = SystemP_FAILURE;

    return (status);
}

int32_t Crypto_shaHwSingleShot(Crypto_ShaContext *ctx, const uint8_t *input, uint32_t ilen, uint8_t *output)
{
    int32_t status = SystemP_FAILURE;

    return (status);
}