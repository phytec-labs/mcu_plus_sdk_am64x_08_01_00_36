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

/* This example demonstrates the AES CMAC 256. */

#include <string.h>
#include <kernel/dpl/DebugP.h>
#include "ti_drivers_config.h"
#include "ti_drivers_open_close.h"
#include "ti_board_open_close.h"

/* input length*/
#define APP_AES_INPUT_LENGTH                (64U)
/* output length*/
#define APP_AES_OUTPUT_LENGTH               (16U)
/* Aes cmac max key length*/
#define APP_AES_CMAC_MAXKEY_LENGTH          (32U)
/* Aes cmac key length in bites*/
#define APP_AES_CMAC_KEY_LENGTH_IN_BITS     (256U)

/* input buffer for Aes cmac operation */
static uint8_t gCryptoAesCmacInput[APP_AES_INPUT_LENGTH] =
{
    0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96,
    0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a,
    0xae, 0x2d, 0x8a, 0x57, 0x1e, 0x03, 0xac, 0x9c,
    0x9e, 0xb7, 0x6f, 0xac, 0x45, 0xaf, 0x8e, 0x51,
    0x30, 0xc8, 0x1c, 0x46, 0xa3, 0x5c, 0xe4, 0x11,
    0xe5, 0xfb, 0xc1, 0x19, 0x1a, 0x0a, 0x52, 0xef,
    0xf6, 0x9f, 0x24, 0x45, 0xdf, 0x4f, 0x9b, 0x17,
    0xad, 0x2b, 0x41, 0x7b, 0xe6, 0x6c, 0x37, 0x10
};

/* The AES CMAC 256-bit keys */
static uint8_t gCryptoAesCmacKey[APP_AES_CMAC_MAXKEY_LENGTH] =
{
    0x60, 0x3d, 0xeb, 0x10, 0x15, 0xca, 0x71, 0xbe,
    0x2b, 0x73, 0xae, 0xf0, 0x85, 0x7d, 0x77, 0x81,
    0x1f, 0x35, 0x2c, 0x07, 0x3b, 0x61, 0x08, 0xd7,
    0x2d, 0x98, 0x10, 0xa3, 0x09, 0x14, 0xdf, 0xf4
};

/* The AES CMAC 256-bit expected output */
static uint8_t gCryptoAesCmacExpectedResult[APP_AES_OUTPUT_LENGTH] =
{
    0xe1, 0x99, 0x21, 0x90, 0x54, 0x9f, 0x6e, 0xd5,
    0x69, 0x6a, 0x2c, 0x05, 0x6c, 0x31, 0x54, 0x10
};

/* CMAC output buffer */
uint8_t     gCryptoAesCmacResultBuf[APP_AES_OUTPUT_LENGTH];

/* Context memory */
static Crypto_AesContext gCryptoAesCmacContext;

void crypto_aes_cmac_sw(void *args)
{
    int32_t             status;
    Crypto_AesHandle    aesCmacHandle;
    Crypto_AesParams    params;

    Drivers_open();
    Board_driversOpen();

    DebugP_log("[CRYPTO] AES CMAC SW example started ...\r\n");

    Crypto_AesParams_init(&params);
    params.aesMode       = CRYPTO_AES_CMAC_256;
    memcpy(&params.key, gCryptoAesCmacKey, APP_AES_CMAC_MAXKEY_LENGTH);
    params.keySizeInBits = APP_AES_CMAC_KEY_LENGTH_IN_BITS;
    params.type          = CRYPTO_TYPE_SW;
    params.algoType      = CRYPTO_AES_CMAC;
    aesCmacHandle        = Crypto_aesOpen(&gCryptoAesCmacContext, &params);
    DebugP_assert(aesCmacHandle != NULL);

    /* This function finishes the AES CMAC operation, and writes the result to the output buffer */
    status = Crypto_cmacSingleShot(aesCmacHandle, gCryptoAesCmacInput, sizeof(gCryptoAesCmacInput), gCryptoAesCmacResultBuf);

    /* Close AES instance */
    status = Crypto_aesClose(aesCmacHandle);
    DebugP_assert(SystemP_SUCCESS == status);

    /* comparing result with expected test results */
    if(memcmp(gCryptoAesCmacResultBuf, gCryptoAesCmacExpectedResult, APP_AES_OUTPUT_LENGTH) != 0)
    {
        DebugP_log("[CRYPTO] AES CMAC SW Encryption example failed!!\r\n");
    }

    else
    {
        DebugP_log("[CRYPTO] AES CMAC SW example completed!!\r\n");
        DebugP_log("All tests have passed!!\r\n");
    }

    Board_driversClose();
    Drivers_close();

    return;
}