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
 *  \file   crypto_aes_cmac_sw.c
 *
 *  \brief  This file contains the implementation of Cipher-based Message Authentication Code (CMAC) driver
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <string.h>
#include <kernel/dpl/SystemP.h>
#include <kernel/dpl/DebugP.h>
#include <security/crypto/sw/crypto_aes_cmac_sw.h>
#include <security/mbedtls/mbedtls-stack/include/mbedtls/cmac.h>

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

/** Sw Cmac function table */
Crypto_AesCmacFxns gCryptoAesCmacSwFxns =
{
    .setUpFxn       = Crypto_cmacSwSetup,
    .startFxn       = Crypto_cmacSwStarts,
    .updateFxn      = Crypto_cmacSwUpdate,
    .finishFxn      = Crypto_cmacSwFinish,
    .singleShotFxn  = Crypto_cmacSwSingleShot,
};

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

int32_t Crypto_cmacSwSetup(Crypto_AesContext *ctx)
{
    int32_t status = SystemP_SUCCESS;
    const mbedtls_cipher_info_t *cipher_info;
    cipher_info = mbedtls_cipher_info_from_type( (ctx->params.aesMode)-1 );
   
    if(NULL == ctx || cipher_info == NULL)
    {
        status = SystemP_FAILURE;
    }

    else
    {
        switch(ctx->params.aesMode)
        {
            case CRYPTO_AES_CMAC_128:
            case CRYPTO_AES_CMAC_192:
            case CRYPTO_AES_CMAC_256:
                status = mbedtls_cipher_setup((mbedtls_cipher_context_t *)&ctx->rsv, cipher_info);
                break;

            default:
                status = SystemP_FAILURE;
                break;
        }
    }

    return (status);
}

int32_t Crypto_cmacSwStarts(Crypto_AesContext *ctx)
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
            case CRYPTO_AES_CMAC_128:
            case CRYPTO_AES_CMAC_192:
            case CRYPTO_AES_CMAC_256:
                status = mbedtls_cipher_cmac_starts((mbedtls_cipher_context_t *)&ctx->rsv, (uint8_t *)&ctx->params.key, (uint32_t)ctx->params.keySizeInBits);
                break;

            default:
                status = SystemP_FAILURE;
                break;
        }
    }

    return (status);
}

int32_t Crypto_cmacSwUpdate(Crypto_AesContext *ctx, const uint8_t *input, uint32_t ilen)
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
            case CRYPTO_AES_CMAC_128:
            case CRYPTO_AES_CMAC_192:
            case CRYPTO_AES_CMAC_256:
                status = mbedtls_cipher_cmac_update((mbedtls_cipher_context_t *)&ctx->rsv, input, ilen);
                break;

            default:
                status = SystemP_FAILURE;
                break;
        }
    }

    return (status);
}

int32_t Crypto_cmacSwFinish(Crypto_AesContext *ctx, uint8_t *output)
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
            case CRYPTO_AES_CMAC_128:
            case CRYPTO_AES_CMAC_192:
            case CRYPTO_AES_CMAC_256:
                status = mbedtls_cipher_cmac_finish((mbedtls_cipher_context_t *)&ctx->rsv, output);
                break;

            default:
                status = SystemP_FAILURE;
                break;
        }
    }

    return (status);
}

int32_t Crypto_cmacSwSingleShot(Crypto_AesContext *ctx, const uint8_t *input, uint32_t ilen, uint8_t *output)
{
    int32_t status = SystemP_SUCCESS;
    const mbedtls_cipher_info_t *cipher_info;
    cipher_info = mbedtls_cipher_info_from_type( (ctx->params.aesMode) - 1 );
   
    if(NULL == ctx || cipher_info == NULL)
    {
        status = SystemP_FAILURE;
    }
    else
    {
        switch(ctx->params.aesMode)
        {
            case CRYPTO_AES_CMAC_128:
            case CRYPTO_AES_CMAC_192:
            case CRYPTO_AES_CMAC_256:
                status = mbedtls_cipher_cmac(cipher_info, (uint8_t *)&ctx->params.key, (uint32_t)ctx->params.keySizeInBits, input, ilen, output);
                break;

            default:
                status = SystemP_FAILURE;
                break;
        }
    }

    return (status);
}
