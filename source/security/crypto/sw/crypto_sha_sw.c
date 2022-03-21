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
 *  \file   crypto_sha_sw.c
 *
 *  \brief  This file contains the implementation of Secure Hash Algorithem (SHA) driver
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <string.h>
#include <kernel/dpl/SystemP.h>
#include <kernel/dpl/DebugP.h>
#include <security/crypto/sw/crypto_sha_sw.h>
#include <security/crypto/sw/crypto_hmac_sha_sw.h>
#include <security/mbedtls/mbedtls-stack/include/mbedtls/sha512.h>
#include <security/mbedtls/mbedtls-stack/include/mbedtls/sha256.h>
#include <security/mbedtls/mbedtls-stack/include/mbedtls/sha1.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

#define CRYPTO_SHA_SW_512_TYPE                  (0U)
#define CRYPTO_SHA_SW_384_TYPE                  (1U)
#define CRYPTO_SHA_SW_256_TYPE                  (0U)
#define CRYPTO_SHA_SW_224_TYPE                  (1U)

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

/** Sw sha function table */
Crypto_ShaFxns gCryptoShaSwFxns =
{
    .openFxn        = Crypto_shaSwOpen,
    .closeFxn       = Crypto_shaSwClose,
    .startFxn       = Crypto_shaSwStarts,
    .updateFxn      = Crypto_shaSwUpdate,
    .finishFxn      = Crypto_shaSwFinish,
    .singleShotFxn  = Crypto_shaSwSingleShot,
    .hmacShaFxn     = Crypto_hmacSwSha,
};

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

int32_t Crypto_shaSwOpen(Crypto_ShaContext *ctx, const Crypto_ShaParams *params)
{
    int32_t status = SystemP_SUCCESS;

    if((NULL == ctx) || (NULL == params))
    {
        status = SystemP_FAILURE;
    }
    else
    {
        memcpy(&ctx->params, params, sizeof(ctx->params));
        switch(ctx->params.authMode)
        {
            case CRYPTO_SHA_AUTHMODE_SHA2_512:
            case CRYPTO_SHA_AUTHMODE_SHA2_384:
            case CRYPTO_SHA_AUTHMODE_HMAC_SHA2_512:
                DebugP_assert(sizeof(mbedtls_sha512_context) <= sizeof(ctx->rsv));
                mbedtls_sha512_init((mbedtls_sha512_context *)&ctx->rsv);
                break;

            case CRYPTO_SHA_AUTHMODE_SHA2_256:
            case CRYPTO_SHA_AUTHMODE_SHA2_224:
            case CRYPTO_SHA_AUTHMODE_HMAC_SHA2_256:
                DebugP_assert(sizeof(mbedtls_sha256_context) <= sizeof(ctx->rsv));
                mbedtls_sha256_init((mbedtls_sha256_context *)&ctx->rsv);
                break;

            case CRYPTO_SHA_AUTHMODE_SHA1:
            case CRYPTO_SHA_AUTHMODE_HMAC_SHA1:
                DebugP_assert(sizeof(mbedtls_sha1_context) <= sizeof(ctx->rsv));
                mbedtls_sha1_init((mbedtls_sha1_context *)&ctx->rsv);
                break;

            default:
                status = SystemP_FAILURE;
                break;
        }
    }

    return (status);
}

int32_t Crypto_shaSwClose(Crypto_ShaContext *ctx)
{
    int32_t status = SystemP_SUCCESS;

    if(NULL != ctx)
    {
        switch(ctx->params.authMode)
        {
            case CRYPTO_SHA_AUTHMODE_SHA2_512:
            case CRYPTO_SHA_AUTHMODE_SHA2_384:
            case CRYPTO_SHA_AUTHMODE_HMAC_SHA2_512:
                mbedtls_sha512_free((mbedtls_sha512_context *)&ctx->rsv);
                break;

            case CRYPTO_SHA_AUTHMODE_SHA2_256:
            case CRYPTO_SHA_AUTHMODE_SHA2_224:
            case CRYPTO_SHA_AUTHMODE_HMAC_SHA2_256:
                mbedtls_sha256_free((mbedtls_sha256_context *)&ctx->rsv);
                break;

            case CRYPTO_SHA_AUTHMODE_SHA1:
            case CRYPTO_SHA_AUTHMODE_HMAC_SHA1:
                mbedtls_sha1_free((mbedtls_sha1_context *)&ctx->rsv);
                break;

            default:
                status = SystemP_FAILURE;
                break;
        }

        memset(ctx, 0, sizeof(Crypto_ShaContext));
    }

    return (status);
}

int32_t Crypto_shaSwStarts(Crypto_ShaContext *ctx)
{
    int32_t status = SystemP_SUCCESS;

    if(NULL == ctx)
    {
        status = SystemP_FAILURE;
    }
    else
    {
        switch(ctx->params.authMode)
        {
            case CRYPTO_SHA_AUTHMODE_SHA2_512:
                status = mbedtls_sha512_starts((mbedtls_sha512_context *)&ctx->rsv, CRYPTO_SHA_SW_512_TYPE);
                break;

            case CRYPTO_SHA_AUTHMODE_SHA2_384:
                status = mbedtls_sha512_starts((mbedtls_sha512_context *)&ctx->rsv, CRYPTO_SHA_SW_384_TYPE);
                break;

            case CRYPTO_SHA_AUTHMODE_SHA2_256:
                status = mbedtls_sha256_starts((mbedtls_sha256_context *)&ctx->rsv, CRYPTO_SHA_SW_256_TYPE);
                break;

            case CRYPTO_SHA_AUTHMODE_SHA2_224:
                status = mbedtls_sha256_starts((mbedtls_sha256_context *)&ctx->rsv, CRYPTO_SHA_SW_224_TYPE);
                break;

            case CRYPTO_SHA_AUTHMODE_SHA1:
                status = mbedtls_sha1_starts((mbedtls_sha1_context *)&ctx->rsv);
                break;

            default:
                status = SystemP_FAILURE;
                break;
        }
    }

    return (status);
}

int32_t Crypto_shaSwUpdate(Crypto_ShaContext *ctx, const uint8_t *input, uint32_t ilen)
{
    int32_t status = SystemP_SUCCESS;

    if(NULL == ctx)
    {
        status = SystemP_FAILURE;
    }
    else
    {
        switch(ctx->params.authMode)
        {
            case CRYPTO_SHA_AUTHMODE_SHA2_512:
            case CRYPTO_SHA_AUTHMODE_SHA2_384:
                status = mbedtls_sha512_update((mbedtls_sha512_context *)&ctx->rsv, input, ilen);
                break;

            case CRYPTO_SHA_AUTHMODE_SHA2_256:
            case CRYPTO_SHA_AUTHMODE_SHA2_224:
                status = mbedtls_sha256_update((mbedtls_sha256_context *)&ctx->rsv, input, ilen);
                break;

            case CRYPTO_SHA_AUTHMODE_SHA1:
                status = mbedtls_sha1_update((mbedtls_sha1_context *)&ctx->rsv, input, ilen);
                break;

            default:
                status = SystemP_FAILURE;
                break;
        }
    }

    return (status);
}

int32_t Crypto_shaSwFinish(Crypto_ShaContext *ctx, uint8_t *output)
{
    int32_t status = SystemP_SUCCESS;

    if(NULL == ctx)
    {
        status = SystemP_FAILURE;
    }
    else
    {
        switch(ctx->params.authMode)
        {
            case CRYPTO_SHA_AUTHMODE_SHA2_512:
            case CRYPTO_SHA_AUTHMODE_SHA2_384:
                status = mbedtls_sha512_finish((mbedtls_sha512_context *)&ctx->rsv, output);
                break;

            case CRYPTO_SHA_AUTHMODE_SHA2_256:
            case CRYPTO_SHA_AUTHMODE_SHA2_224:
                status = mbedtls_sha256_finish((mbedtls_sha256_context *)&ctx->rsv, output);
                break;

            case CRYPTO_SHA_AUTHMODE_SHA1:
                status = mbedtls_sha1_finish((mbedtls_sha1_context *)&ctx->rsv, output);
                break;

            default:
                status = SystemP_FAILURE;
                break;
        }
    }

    return (status);
}

int32_t Crypto_shaSwSingleShot(Crypto_ShaContext *ctx, const uint8_t *input, uint32_t ilen, uint8_t *output)
{
    int32_t status = SystemP_SUCCESS;

    if(NULL == ctx)
    {
        status = SystemP_FAILURE;
    }
    else
    {
        switch(ctx->params.authMode)
        {
            case CRYPTO_SHA_AUTHMODE_SHA2_512:
                status = mbedtls_sha512(input, ilen, output, CRYPTO_SHA_SW_512_TYPE);
                break;

            case CRYPTO_SHA_AUTHMODE_SHA2_384:
                status = mbedtls_sha512(input, ilen, output, CRYPTO_SHA_SW_384_TYPE);
                break;

            case CRYPTO_SHA_AUTHMODE_SHA2_256:
                status = mbedtls_sha256(input, ilen, output, CRYPTO_SHA_SW_256_TYPE);
                break;

            case CRYPTO_SHA_AUTHMODE_SHA2_224:
                status = mbedtls_sha256(input, ilen, output, CRYPTO_SHA_SW_224_TYPE);
                break;

            case CRYPTO_SHA_AUTHMODE_SHA1:
                status = mbedtls_sha1(input, ilen, output);
                break;

            default:
                status = SystemP_FAILURE;
                break;
        }
    }

    return (status);
}
