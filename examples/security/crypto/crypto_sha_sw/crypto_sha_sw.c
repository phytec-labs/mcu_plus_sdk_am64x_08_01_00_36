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

/* This example demonstrates the SW implementation of SHA512 */

#include <string.h>
#include <kernel/dpl/DebugP.h>
#include "ti_drivers_config.h"
#include "ti_drivers_open_close.h"
#include "ti_board_open_close.h"

/* SHA512 length */
#define APP_SHA512_LENGTH               (64U)

/* Test buffer for sha computation */
static uint8_t gCryptoSha512TestBuf[10] = {"abcdefpra"};

/* SHA-512 test vectors,this is expected hash for the test buffer */
static uint8_t gCryptoSha512TestSum[APP_SHA512_LENGTH] =
{
    0x1d, 0xa6, 0x8d, 0x60, 0x62, 0x9b, 0x61, 0xf4,
    0xab, 0x16, 0x67, 0x77, 0x2a, 0x21, 0xae, 0x85,
    0xbf, 0x5c, 0x20, 0xdf, 0x5c, 0x38, 0xcc, 0xa5,
    0x29, 0xc6, 0xce, 0x09, 0x22, 0xbe, 0x15, 0x7f,
    0x04, 0x9c, 0x22, 0x0a, 0xab, 0x85, 0xb4, 0x3c,
    0x49, 0x66, 0x12, 0xfa, 0x12, 0xd5, 0x41, 0xac,
    0x78, 0x50, 0x9a, 0x5f, 0x03, 0x4c, 0xc9, 0xcb,
    0x64, 0x39, 0x0a, 0x74, 0x2a, 0xb6, 0xab, 0x43
};

/* Context memory */
static Crypto_ShaContext gCryptoSha512Context;

void crypto_sha_sw(void *args)
{
    int32_t             status;
    uint8_t             sha512sum[APP_SHA512_LENGTH];
    Crypto_ShaHandle    shaHandle;
    Crypto_ShaParams    params;

    Drivers_open();
    Board_driversOpen();

    DebugP_log("[CRYPTO] SHA512 SW example started ...\r\n");

    /* Open SHA instance */
    Crypto_ShaParams_init(&params);
    params.authMode = CRYPTO_SHA_AUTHMODE_SHA2_512;
    params.type     = CRYPTO_TYPE_SW;
    shaHandle = Crypto_shaOpen(&gCryptoSha512Context, &params);
    DebugP_assert(shaHandle != NULL);

    /* Perform SHA operation */
    status = Crypto_shaSingleShot(shaHandle, &gCryptoSha512TestBuf[0], sizeof(gCryptoSha512TestBuf) - 1, sha512sum);
    DebugP_assert(SystemP_SUCCESS == status);

    /* Close SHA instance */
    status = Crypto_shaClose(shaHandle);
    DebugP_assert(SystemP_SUCCESS == status);

    /*comparing result with expected test results*/
    if(memcmp(sha512sum, gCryptoSha512TestSum, APP_SHA512_LENGTH) != 0)
    {
        DebugP_log("[CRYPTO] SHA512 SW example failed!!\r\n");
    }
    else
    {
        DebugP_log("[CRYPTO] SHA512 SW example completed!!\r\n");
        DebugP_log("All tests have passed!!\r\n");
    }

    Board_driversClose();
    Drivers_close();

    return;
}