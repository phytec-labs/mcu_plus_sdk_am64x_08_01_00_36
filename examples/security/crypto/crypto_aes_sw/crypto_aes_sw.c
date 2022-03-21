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

/* This example demonstrates the AES 256 cbc Encryption and Decryptions. */

#include <string.h>
#include <kernel/dpl/DebugP.h>
#include "ti_drivers_config.h"
#include "ti_drivers_open_close.h"
#include "ti_board_open_close.h"

/* input or output length*/
#define APP_AES_INOUT_LENGTH           (16U)
/* Aes max key length*/
#define APP_AES_MAXKEY_LENGTH          (32U)
/* Aes key length in bites*/
#define APP_AES_KEY_LENGTH_IN_BITS     (256U)

/* input buffer for encryption or decryption */
static uint8_t gCryptoAesInput[APP_AES_INOUT_LENGTH] =
{
    0x81, 0xEA, 0x5B, 0xA4, 0x69, 0x45, 0xC1, 0x70,
    0x5F, 0x6F, 0x89, 0x77, 0x88, 0x68, 0xCC, 0x67
};

/* The AES encryption algorithm encrypts and decrypts data in blocks of 128 bits. It can do this using 128-bit, 192-bit, or 256-bit keys */
static uint8_t gCryptoAesKey[APP_AES_MAXKEY_LENGTH] =
{
    0x33, 0xA3, 0x66, 0x46, 0xFE, 0x56, 0xF7, 0x0D,
    0xC0, 0xC5, 0x1A, 0x31, 0x17, 0xE6, 0x39, 0xF1,
    0x82, 0xDE, 0xF8, 0xCA, 0xB5, 0xC0, 0x66, 0x71,
    0xEE, 0xA0, 0x40, 0x7C, 0x48, 0xA9, 0xC7, 0x57
};

/* initialization vector (IV) is an arbitrary number that can be used along with a secret key for data encryption/decryption. */
static uint8_t gCryptoAesIv[CRYPTO_AES_IV_LENGTH] =
{
    0x7C, 0xE2, 0xAB, 0xAF, 0x8B, 0xEF, 0x23, 0xC4,
    0x81, 0x6D, 0xC8, 0xCE, 0x84, 0x20, 0x48, 0xA7
};
/* Encryption output buf */
uint8_t     aesEncResultBuf[APP_AES_INOUT_LENGTH];
/* Decryption output buf */
uint8_t     aesDecResultBuf[APP_AES_INOUT_LENGTH];

/* Context memory */
static Crypto_AesContext gCryptoAesEncContext, gCryptoAesDecContext;

void crypto_aes_sw(void *args)
{
    int32_t             status;
    Crypto_AesHandle    aesEncHandle,aesDecHandle;
    Crypto_AesParams    params;

    Drivers_open();
    Board_driversOpen();

    DebugP_log("[CRYPTO] AES SW example started ...\r\n");

    /* Encryption */
    Crypto_AesParams_init(&params);
    params.aesMode       = CRYPTO_AES_CBC_256;
    memcpy(&params.iv, gCryptoAesIv, CRYPTO_AES_IV_LENGTH);
    memcpy(&params.key, gCryptoAesKey, CRYPTO_AES_KEY_LENGTH);
    params.keySizeInBits = APP_AES_KEY_LENGTH_IN_BITS;
    params.type          = CRYPTO_TYPE_SW;
    params.algoType      = CRYPTO_AES_ENCRYPT;
    aesEncHandle = Crypto_aesOpen(&gCryptoAesEncContext, &params);
    DebugP_assert(aesEncHandle != NULL);

    /* This function sets the AES encryption key */
    status = Crypto_aesSetKeyEnc(aesEncHandle);
    DebugP_assert(SystemP_SUCCESS == status);

    /* This function finishes the AES operation, and writes the result to the output buffer */
    status = Crypto_aesCbc(aesEncHandle, gCryptoAesInput, sizeof(gCryptoAesInput), aesEncResultBuf);
    DebugP_assert(SystemP_SUCCESS == status);

    /* Decryption */
    params.aesMode       = CRYPTO_AES_CBC_256;
    memcpy(&params.iv, gCryptoAesIv, CRYPTO_AES_IV_LENGTH);
    memcpy(&params.key, gCryptoAesKey, CRYPTO_AES_KEY_LENGTH);
    params.keySizeInBits = APP_AES_KEY_LENGTH_IN_BITS;
    params.type          = CRYPTO_TYPE_SW;
    params.algoType      = CRYPTO_AES_DECRYPT;
    aesDecHandle = Crypto_aesOpen(&gCryptoAesDecContext, &params);
    DebugP_assert(aesDecHandle != NULL);

    /* This function sets the AES decryption key */
    status = Crypto_aesSetKeyDec(aesDecHandle);
    DebugP_assert(SystemP_SUCCESS == status);

    /*This function finishes the AES operation, and writes the result to the output buffer */
    status = Crypto_aesCbc(aesDecHandle, aesEncResultBuf, sizeof(aesEncResultBuf), aesDecResultBuf);
    DebugP_assert(SystemP_SUCCESS == status);

    /* Close AES instance */
    status = Crypto_aesClose(aesDecHandle);
    DebugP_assert(SystemP_SUCCESS == status);
    /* Close AES instance */
    status = Crypto_aesClose(aesEncHandle);
    DebugP_assert(SystemP_SUCCESS == status);

    /* comparing result with expected test results */
    if(memcmp(aesDecResultBuf, gCryptoAesInput, APP_AES_INOUT_LENGTH) != 0)
    {
        DebugP_log("[CRYPTO] AES SW Encryption example failed!!\r\n");
    }

    else
    {
        DebugP_log("[CRYPTO] AES SW example completed!!\r\n");
        DebugP_log("All tests have passed!!\r\n");
    }

    Board_driversClose();
    Drivers_close();

    return;
}