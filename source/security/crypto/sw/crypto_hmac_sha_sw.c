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
 *  \file   crypto_hmac_sha_sw.c
 *
 *  \brief  This file contains the implementation of Keyed-Hash Message Authentication Code Secure Hash Algorithm (HMAC-SHA) driver
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <string.h>
#include <kernel/dpl/SystemP.h>
#include <kernel/dpl/DebugP.h>
#include <security/crypto/sw/crypto_hmac_sha_sw.h>
#include <security/mbedtls/mbedtls-stack/include/mbedtls/sha1.h>
#include <security/mbedtls/mbedtls-stack/include/mbedtls/sha256.h>
#include <security/mbedtls/mbedtls-stack/include/mbedtls/sha512.h>

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

/* None */

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

int32_t Crypto_hmacSwSha(Crypto_ShaContext *ctx, const uint8_t *input, uint32_t ilen, uint8_t *output)
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
            case CRYPTO_SHA_AUTHMODE_HMAC_SHA1:
                status = Crypto_hmacSwSha1(ctx, input, ilen, output);
                break;

            case CRYPTO_SHA_AUTHMODE_HMAC_SHA2_256:
                status = Crypto_hmacSwSha256(ctx, input, ilen, output);
                break;

            case CRYPTO_SHA_AUTHMODE_HMAC_SHA2_512:
                status = Crypto_hmacSwSha512(ctx, input, ilen, output);
                break;

            default:
                status = SystemP_FAILURE;
                break;
        }
    }

    return (status);
}

int32_t Crypto_hmacSwSha1(Crypto_ShaContext *ctx, const uint8_t *input, uint32_t ilen, uint8_t *output)
{
    int32_t status = SystemP_SUCCESS;
    int32_t i = 0;
    uint8_t hmacKey[CRYPTO_HMAC_SHA1_SW_KEYLEN_BYTES];
    uint8_t tempOut[CRYPTO_HMAC_SHA1_SW_KEYLEN_BYTES];
    uint8_t *pIpad, *pOpad;
    uint8_t  keybyte;

    /* Check for KeyLength */
    if ( (uint32_t)ctx->params.keySizeInBits > CRYPTO_HMAC_SHA1_SW_KEYLEN_BYTES)
    {
        /* Not Implemented */
        status = SystemP_FAILURE;
    }

    else
    {
        /* Copy input Key */
        memcpy(hmacKey, (uint8_t *)&ctx->params.key, (uint32_t)ctx->params.keySizeInBits);
    }

    /* Initialize the remaining bytes to zero */
    memset(hmacKey + (uint32_t)ctx->params.keySizeInBits, 0, CRYPTO_HMAC_SHA1_SW_KEYLEN_BYTES - (uint32_t)ctx->params.keySizeInBits);

    /* Stack optimization resue hmackey and tempOut buffers */
    pIpad = hmacKey;
    pOpad = tempOut;

    /* Compute Inner/Outer Padding  */
    for (i = 0; i < CRYPTO_HMAC_SHA1_SW_KEYLEN_BYTES; i++)
    {
        keybyte = hmacKey[i];
        pIpad[i] = keybyte ^ CRYPTO_HMAC_SHA_SW_IPAD;
        pOpad[i] = keybyte ^ CRYPTO_HMAC_SHA_SW_OPAD;
    }

    mbedtls_sha1_init((mbedtls_sha1_context *)&ctx->rsv);
    status =  mbedtls_sha1_starts((mbedtls_sha1_context *)&ctx->rsv);
    if (SystemP_SUCCESS == status)
    {
        status =  mbedtls_sha1_update((mbedtls_sha1_context *)&ctx->rsv,pIpad, CRYPTO_HMAC_SHA1_SW_KEYLEN_BYTES);
    }

    status =  mbedtls_sha1_update((mbedtls_sha1_context *)&ctx->rsv,input, ilen);
    if (SystemP_SUCCESS == status)
    {
        mbedtls_sha1_finish((mbedtls_sha1_context *)&ctx->rsv,output);
        mbedtls_sha1_free((mbedtls_sha1_context *)&ctx->rsv);
        mbedtls_sha1_init((mbedtls_sha1_context *)&ctx->rsv);
    }

    if (SystemP_SUCCESS == status)
    {
        status =  mbedtls_sha1_starts((mbedtls_sha1_context *)&ctx->rsv);
    }

    if (SystemP_SUCCESS == status)
    {
        status =  mbedtls_sha1_update((mbedtls_sha1_context *)&ctx->rsv,(void *)(uint32_t)pOpad, CRYPTO_HMAC_SHA1_SW_KEYLEN_BYTES);
    }

    if (SystemP_SUCCESS == status)
    {
        status =  mbedtls_sha1_update((mbedtls_sha1_context *)&ctx->rsv,(void *)(uint32_t)output,20);
    }

    if (SystemP_SUCCESS == status)
    {
        mbedtls_sha1_finish((mbedtls_sha1_context *)&ctx->rsv,output);
        mbedtls_sha1_free((mbedtls_sha1_context *)&ctx->rsv);
    }

    return (status);
}

int32_t Crypto_hmacSwSha256(Crypto_ShaContext *ctx, const uint8_t *input, uint32_t ilen, uint8_t *output)
{
    int32_t status = SystemP_SUCCESS;
    uint8_t hmacKey[CRYPTO_HMAC_SHA256_SW_KEYLEN_BYTES];
    uint8_t tempOut[CRYPTO_HMAC_SHA256_SW_KEYLEN_BYTES];
    uint8_t *pIpad, *pOpad;
    uint32_t i;
    uint8_t  keybyte;

    /* Check for KeyLength */
    if ( (uint32_t)ctx->params.keySizeInBits > CRYPTO_HMAC_SHA256_SW_KEYLEN_BYTES)
    {
        /* Not Implemented */
        status = SystemP_FAILURE;
    }

    else
    {
        /* Copy input Key */
        memcpy(hmacKey, (uint8_t *)&ctx->params.key, (uint32_t)ctx->params.keySizeInBits);
    }

    /* Initialize the remaining bytes to zero */
    memset(hmacKey + (uint32_t)ctx->params.keySizeInBits, 0, CRYPTO_HMAC_SHA256_SW_KEYLEN_BYTES - (uint32_t)ctx->params.keySizeInBits);

    /* Stack optimization resue hmackey and tempOut buffers */
    pIpad = hmacKey;
    pOpad = tempOut;

    /* Compute Inner/Outer Padding  */
    for (i = 0; i < CRYPTO_HMAC_SHA256_SW_KEYLEN_BYTES; i++)
    {
        keybyte = hmacKey[i];
        pIpad[i] = keybyte ^ CRYPTO_HMAC_SHA_SW_IPAD;
        pOpad[i] = keybyte ^ CRYPTO_HMAC_SHA_SW_OPAD;
    }

    mbedtls_sha256_init((mbedtls_sha256_context *)&ctx->rsv);
    status =  mbedtls_sha256_starts((mbedtls_sha256_context *)&ctx->rsv,0);
    if (SystemP_SUCCESS == status)
    {
        status =  mbedtls_sha256_update((mbedtls_sha256_context *)&ctx->rsv,pIpad, CRYPTO_HMAC_SHA256_SW_KEYLEN_BYTES);
    }

    status =  mbedtls_sha256_update((mbedtls_sha256_context *)&ctx->rsv,input, ilen);
    if (SystemP_SUCCESS == status)
    {
        mbedtls_sha256_finish((mbedtls_sha256_context *)&ctx->rsv,output);
        mbedtls_sha256_free((mbedtls_sha256_context *)&ctx->rsv);
        mbedtls_sha256_init((mbedtls_sha256_context *)&ctx->rsv);
    }

    if (SystemP_SUCCESS == status)
    {
        status =  mbedtls_sha256_starts((mbedtls_sha256_context *)&ctx->rsv,0);
    }

    if (SystemP_SUCCESS == status)
    {
        status =  mbedtls_sha256_update((mbedtls_sha256_context *)&ctx->rsv,(void *)(uint32_t)pOpad, CRYPTO_HMAC_SHA256_SW_KEYLEN_BYTES);
    }

    if (SystemP_SUCCESS == status)
    {
        status =  mbedtls_sha256_update((mbedtls_sha256_context *)&ctx->rsv,(void *)(uint32_t)output, 32);
    }

    if (SystemP_SUCCESS == status)
    {
        mbedtls_sha256_finish((mbedtls_sha256_context *)&ctx->rsv,output);
        mbedtls_sha256_free((mbedtls_sha256_context *)&ctx->rsv);
    }

    return (status);
}

int32_t Crypto_hmacSwSha512(Crypto_ShaContext *ctx, const uint8_t *input, uint32_t ilen, uint8_t *output)
{
    int32_t status = SystemP_SUCCESS;
    uint8_t hmacKey[CRYPTO_HMAC_SHA512_SW_KEYLEN_BYTES];
    uint8_t tempOut[CRYPTO_HMAC_SHA512_SW_KEYLEN_BYTES];
    uint8_t *pIpad, *pOpad;
    uint32_t i;
    uint8_t  keybyte;

    /* Check for KeyLength */
    if ( (uint32_t)ctx->params.keySizeInBits > CRYPTO_HMAC_SHA512_SW_KEYLEN_BYTES)
    {
        /* Not Implemented */
        status = SystemP_FAILURE;
    }

    else
    {
        /* Copy input Key */
        memcpy(hmacKey, (uint8_t *)&ctx->params.key, (uint32_t)ctx->params.keySizeInBits);
    }

    /* Initialize the remaining bytes to zero */
    memset(hmacKey + (uint32_t)ctx->params.keySizeInBits, 0, CRYPTO_HMAC_SHA512_SW_KEYLEN_BYTES - (uint32_t)ctx->params.keySizeInBits);

    /* Stack optimization resue hmackey and tempOut buffers */
    pIpad = hmacKey;
    pOpad = tempOut;

    /* Compute Inner/Outer Padding  */
    for (i = 0; i < CRYPTO_HMAC_SHA512_SW_KEYLEN_BYTES; i++)
    {
        keybyte = hmacKey[i];
        pIpad[i] = keybyte ^ CRYPTO_HMAC_SHA_SW_IPAD;
        pOpad[i] = keybyte ^ CRYPTO_HMAC_SHA_SW_OPAD;
    }

    mbedtls_sha512_init((mbedtls_sha512_context *)&ctx->rsv);
    status =  mbedtls_sha512_starts((mbedtls_sha512_context *)&ctx->rsv,0);
    if (SystemP_SUCCESS == status)
    {
        status =  mbedtls_sha512_update((mbedtls_sha512_context *)&ctx->rsv,pIpad, CRYPTO_HMAC_SHA512_SW_KEYLEN_BYTES);
    }

    status =  mbedtls_sha512_update((mbedtls_sha512_context *)&ctx->rsv,input, ilen);
    if (SystemP_SUCCESS == status)
    {
        mbedtls_sha512_finish((mbedtls_sha512_context *)&ctx->rsv,output);
        mbedtls_sha512_free((mbedtls_sha512_context *)&ctx->rsv);
        mbedtls_sha512_init((mbedtls_sha512_context *)&ctx->rsv);
    }

    if (SystemP_SUCCESS == status)
    {
        status =  mbedtls_sha512_starts((mbedtls_sha512_context *)&ctx->rsv,0);
    }

    if (SystemP_SUCCESS == status)
    {
        status =  mbedtls_sha512_update((mbedtls_sha512_context *)&ctx->rsv,(void *)(uint32_t)pOpad, CRYPTO_HMAC_SHA512_SW_KEYLEN_BYTES);
    }

    if (SystemP_SUCCESS == status)
    {
        status =  mbedtls_sha512_update((mbedtls_sha512_context *)&ctx->rsv,(void *)(uint32_t)output,64);
    }

    if (SystemP_SUCCESS == status)
    {
        mbedtls_sha512_finish((mbedtls_sha512_context *)&ctx->rsv,output);
        mbedtls_sha512_free((mbedtls_sha512_context *)&ctx->rsv);
    }

    return (status);
}