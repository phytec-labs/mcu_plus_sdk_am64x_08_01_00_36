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
 *  \file   crypto_aes_sw.c
 *
 *  \brief  This file contains the implementation of Advance Encryption Standard (AES) operation driver
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <string.h>
#include <kernel/dpl/SystemP.h>
#include <kernel/dpl/DebugP.h>
#include <security/crypto/sw/crypto_aes_sw.h>
#include <security/crypto/sw/crypto_aes_cmac_sw.h>
#include <security/mbedtls/mbedtls-stack/include/mbedtls/cmac.h>
#include <security/mbedtls/mbedtls-stack/include/mbedtls/aes.h>

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

/** Sw aes common function table */
Crypto_AesCommonFxns gCryptoAesCommonSwFxns =
{
    .openFxn        = Crypto_aesSwOpen,
    .closeFxn       = Crypto_aesSwClose,
};

/** Sw aes cbc function table */
Crypto_AesCbcFxns gCryptoAesCbcSwFxns =
{
    .setKeyEncFxn   = Crypto_aesSwSetKeyEnc,
    .setKeyDecFxn   = Crypto_aesSwSetKeyDec,
    .cbcFxn         = Crypto_aesSwCbc,
};

/** Sw aes all function table */
Crypto_AesFxns gCryptoAesSwFxns =
{
    .commonFxns     = &gCryptoAesCommonSwFxns,
    .cbcFxns        = &gCryptoAesCbcSwFxns,
    .cmacFxns       = &gCryptoAesCmacSwFxns,
};

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

int32_t Crypto_aesSwOpen(Crypto_AesContext *ctx, const Crypto_AesParams *params)
{
    int32_t status = SystemP_SUCCESS;

    if((NULL == ctx) || (NULL == params))
    {
        status = SystemP_FAILURE;
    }
    else
    {
        memcpy(&ctx->params, params, sizeof(ctx->params));
        switch(ctx->params.aesMode)
        {
            case CRYPTO_AES_CBC_128:
            case CRYPTO_AES_CBC_192:
            case CRYPTO_AES_CBC_256:
                DebugP_assert(sizeof(mbedtls_aes_context) <= sizeof(ctx->rsv));
                mbedtls_aes_init((mbedtls_aes_context *)&ctx->rsv);
                break;
            case CRYPTO_AES_CMAC_128:
            case CRYPTO_AES_CMAC_192:
            case CRYPTO_AES_CMAC_256:
                DebugP_assert(sizeof(mbedtls_cipher_context_t) <= sizeof(ctx->rsv));
                mbedtls_cipher_init((mbedtls_cipher_context_t *)&ctx->rsv);
                break;

            default:
                status = SystemP_FAILURE;
                break;
        }
    }

    return (status);
}

int32_t Crypto_aesSwClose(Crypto_AesContext *ctx)
{
    int32_t status = SystemP_SUCCESS;

    if(NULL != ctx)
    {
        switch(ctx->params.aesMode)
        {
            case CRYPTO_AES_CBC_128:
            case CRYPTO_AES_CBC_192:
            case CRYPTO_AES_CBC_256:
                mbedtls_aes_free((mbedtls_aes_context *)&ctx->rsv);
                break;
            case CRYPTO_AES_CMAC_128:
            case CRYPTO_AES_CMAC_192:
            case CRYPTO_AES_CMAC_256:
                mbedtls_cipher_free((mbedtls_cipher_context_t *)&ctx->rsv);
                break;

            default:
                status = SystemP_FAILURE;
                break;
        }

        memset(ctx, 0, sizeof(Crypto_AesContext));
    }

    return (status);
}

int32_t Crypto_aesSwSetKeyEnc(Crypto_AesContext *ctx)
{
    int32_t status = SystemP_SUCCESS;
    
    if(NULL == ctx)
    {
        status = SystemP_FAILURE;
    }
    else
    {
        switch(ctx->params.aesMode)
        {
            case CRYPTO_AES_CBC_128:
            case CRYPTO_AES_CBC_192:
            case CRYPTO_AES_CBC_256:
                status = mbedtls_aes_setkey_enc((mbedtls_aes_context *)&ctx->rsv, (uint8_t *)&ctx->params.key, (uint32_t)ctx->params.keySizeInBits);
                break;

            default:
                status = SystemP_FAILURE;
                break;
        }
    }

    return (status);
}

int32_t Crypto_aesSwSetKeyDec(Crypto_AesContext *ctx)
{
    int32_t status = SystemP_SUCCESS;

    if(NULL == ctx)
    {
        status = SystemP_FAILURE;
    }
    else
    {
        switch(ctx->params.aesMode)
        {
            case CRYPTO_AES_CBC_128:
            case CRYPTO_AES_CBC_192:
            case CRYPTO_AES_CBC_256:
                status = mbedtls_aes_setkey_dec((mbedtls_aes_context *)&ctx->rsv, (uint8_t *)&ctx->params.key, (uint32_t)ctx->params.keySizeInBits);
                break;

            default:
                status = SystemP_FAILURE;
                break;
        }
    }

    return (status);
}

int32_t Crypto_aesSwCbc(Crypto_AesContext *ctx, const uint8_t *input, uint32_t ilen, uint8_t *output)
{
    int32_t status = SystemP_SUCCESS;

    if(NULL == ctx)
    {
        status = SystemP_FAILURE;
    }
    else
    {
        switch(ctx->params.aesMode)
        {
            case CRYPTO_AES_CBC_128:
            case CRYPTO_AES_CBC_192:
            case CRYPTO_AES_CBC_256:
                status = mbedtls_aes_crypt_cbc((mbedtls_aes_context *)&ctx->rsv, (uint32_t)ctx->params.algoType, ilen,(uint8_t *)&ctx->params.iv, input, output);
                break;

            default:
                status = SystemP_FAILURE;
                break;
        }
    }

    return (status);
}
