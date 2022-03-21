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

#ifndef CRYPTO_HMAC_SHA_SW_H_
#define CRYPTO_HMAC_SHA_SW_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

/* Inner Padding value for HMAC */
#define CRYPTO_HMAC_SHA_SW_IPAD               (0x36U)
/* Outer Padding value for HMAC */
#define CRYPTO_HMAC_SHA_SW_OPAD               (0x5CU)

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <security/crypto/include/crypto_sha.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* Key length for HMAC-SHA512 Process */
#define CRYPTO_HMAC_SHA512_SW_KEYLEN_BYTES       (128U)
/* Key length for HMAC-SHA1 Process */
#define CRYPTO_HMAC_SHA1_SW_KEYLEN_BYTES         (64U)
/* Key length for HMAC-SHA256 Process */
#define CRYPTO_HMAC_SHA256_SW_KEYLEN_BYTES       (64U)

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

int32_t Crypto_hmacSwSha(Crypto_ShaContext *ctx, const uint8_t *input, uint32_t ilen, uint8_t *output);
int32_t Crypto_hmacSwSha1(Crypto_ShaContext *ctx, const uint8_t *input, uint32_t ilen, uint8_t *output);
int32_t Crypto_hmacSwSha256(Crypto_ShaContext *ctx, const uint8_t *input, uint32_t ilen, uint8_t *output);
int32_t Crypto_hmacSwSha512(Crypto_ShaContext *ctx, const uint8_t *input, uint32_t ilen, uint8_t *output);

/* ========================================================================== */
/*                       Static Function Definitions                          */
/* ========================================================================== */

/* None */

#ifdef __cplusplus
}
#endif

#endif /* CRYPTO_HMAC_SHA_SW_H_ */