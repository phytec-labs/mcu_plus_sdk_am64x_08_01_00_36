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

/* This example demonstrates the SW implementation of HMAC SHA1 */

#include <string.h>
#include <kernel/dpl/DebugP.h>
#include "ti_drivers_config.h"
#include "ti_drivers_open_close.h"
#include "ti_board_open_close.h"

/* SHA1 length */
#define APP_CRYPTO_HMAC_SHA1_SW_LENGTH               (20U)
/* SHA1 key length */
#define APP_CRYPTO_HMAC_SHA1_SW_KEYLEN_BYTES         (64U)
/* SHA1 input key length */
#define APP_CRYPTO_HMAC_SHA1_SW_INPUT_KEYLEN_BYTES   (20U)

/* Input buffer for hmac sha computation */
static uint8_t gCryptoHmacSha1SwInput[10] = {"abcdefpra"};

/* Key buffer for hmac sha computation */
static uint8_t gCryptoHmacSha1SwKey[APP_CRYPTO_HMAC_SHA1_SW_KEYLEN_BYTES] =
{
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 
    0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, 0x40, 0x41, 0x42, 0x43
};

/* Expected output buffer for hmac sha computation */
uint8_t gCryptoHmacSha1SwExpectedOutput[APP_CRYPTO_HMAC_SHA1_SW_LENGTH] =
{
    0x7c, 0xb5, 0x18, 0x18, 0x17, 0xab, 0x4e, 0x6d, 0x46, 0x89,
    0x0f, 0x69, 0xab, 0xbd, 0x99, 0xb8, 0x53, 0xfc, 0x3d, 0x97
};

/* Context memory */
static Crypto_ShaContext gCryptoHmacSha1SwContext;

void crypto_hmac_sha1_sw(void *args)
{
    int32_t             status = SystemP_SUCCESS;
    uint8_t             sha1sum[APP_CRYPTO_HMAC_SHA1_SW_LENGTH];
    Crypto_ShaHandle    hmacShaHandle;
    Crypto_ShaParams    params;

    Drivers_open();
    Board_driversOpen();

    /* Open SHA instance */
    Crypto_ShaParams_init(&params);
    params.authMode       = CRYPTO_SHA_AUTHMODE_HMAC_SHA1;
    params.type           = CRYPTO_TYPE_SW;
    memcpy(&params.key, gCryptoHmacSha1SwKey, APP_CRYPTO_HMAC_SHA1_SW_KEYLEN_BYTES);
    params.keySizeInBits  = APP_CRYPTO_HMAC_SHA1_SW_INPUT_KEYLEN_BYTES;
    hmacShaHandle         = Crypto_shaOpen(&gCryptoHmacSha1SwContext, &params);
    DebugP_assert(hmacShaHandle != NULL);

    DebugP_log("[CRYPTO] HMAC SHA-1 example started ...\r\n");

    /* Perform HMAC SHA operation */
    status = Crypto_hmacSha(hmacShaHandle, &gCryptoHmacSha1SwInput[0], sizeof(gCryptoHmacSha1SwInput)-1, sha1sum);
    DebugP_assert(SystemP_SUCCESS == status);

    /* Close SHA instance */
    status = Crypto_shaClose(hmacShaHandle);
    DebugP_assert(SystemP_SUCCESS == status);

    /*comparing result with expected test results*/
    if(memcmp(sha1sum, gCryptoHmacSha1SwExpectedOutput, APP_CRYPTO_HMAC_SHA1_SW_LENGTH) != 0)
    {
        DebugP_log("[CRYPTO] HMAC SHA-1 SW example failed!!\r\n");
    }
    else
    {
        DebugP_log("[CRYPTO] HMAC SHA-1 example completed!!\r\n");
        DebugP_log("All tests have passed!!\r\n");
    }

    return;
}