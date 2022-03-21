/*
 *  Copyright (C) 2018-2021 Texas Instruments Incorporated
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
 *  \defgroup SECURITY_CRYPTO_SHA_MODULE APIs for CRYPTO SHA
 *  \ingroup SECURITY_MODULE
 *
 *  @{
 *
 */

#ifndef CRYPTO_SHA_H_
#define CRYPTO_SHA_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdint.h>
#include <security/crypto.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/**
 *  \anchor Crypto_ShaAuthMode
 *  \name SHA authentication mode
 *  @{
 */
/** \brief No individual Authentication */
#define CRYPTO_SHA_AUTHMODE_NULL            (0U)
/** \brief SHA1 mode */
#define CRYPTO_SHA_AUTHMODE_SHA1            (1U)
/** \brief 224-bit SHA2 mode */
#define CRYPTO_SHA_AUTHMODE_SHA2_224        (2U)
/** \brief 256-bit SHA2 mode */
#define CRYPTO_SHA_AUTHMODE_SHA2_256        (3U)
/** \brief 384-bit SHA2 mode. Note: This mode is used at Data Mode only for SA2_UL */
#define CRYPTO_SHA_AUTHMODE_SHA2_384        (4U)
/** \brief 512-bit SHA2 mode. Note: This mode is used at Data Mode only for SA2_UL */
#define CRYPTO_SHA_AUTHMODE_SHA2_512        (5U)
/** \brief HMAC with SHA1 mode */
#define CRYPTO_SHA_AUTHMODE_HMAC_SHA1       (6U)
/** \brief HMAC with 224-bit SHA2 mode */
#define CRYPTO_SHA_AUTHMODE_HMAC_SHA2_224   (7U)
/** \brief HMAC with 256-bit SHA2 mode */
#define CRYPTO_SHA_AUTHMODE_HMAC_SHA2_256   (8U)
/** \brief HMAC with 224-bit SHA mode. Note: This mode is used at Data Mode only for SA2_UL */
#define CRYPTO_SHA_AUTHMODE_HMAC_SHA2_384   (9U)
/** \brief HMAC with 256-bit SHA mode. Note: This mode is used at Data Mode only for SA2_UL */
#define CRYPTO_SHA_AUTHMODE_HMAC_SHA2_512   (10U)
/** @} */

/**
 *  \anchor Crypto_Key
 *  \name HMAC SHA Max key lenght 
 *  @{
 */
/* Key length for HMAC-SHA Process */
#define CRYPTO_HMAC_SHA_SW_MAX_KEYLEN_BYTES       (128U)
/** @} */

/** \brief Handle to the Crypto SHA driver returned by #Crypto_shaOpen() */
typedef void *Crypto_ShaHandle;

/** \brief Forward declaration of \ref Crypto_ShaContext_s */
typedef struct Crypto_ShaContext_s Crypto_ShaContext;

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/**
 *  \brief Parameters passed during Crypto_shaOpen()
 */
typedef struct Crypto_ShaParams_s
{
    uint32_t                authMode;
    /**< Authentication mode. Refer \ref Crypto_ShaAuthMode */
    uint32_t                type;
    /**< Type to which to assign the context. Refer Crypto_Type */
    uint8_t                 key[CRYPTO_HMAC_SHA_SW_MAX_KEYLEN_BYTES];
    /** Key for Hmac sha alogorithm. refer \ref Crypto_Key */
    uint32_t                keySizeInBits;
    /** Key size in bytes for Aes alogorithm */
} Crypto_ShaParams;

/** \brief callback functions declarations*/
typedef int32_t (*Crypto_ShaOpenFxn)(Crypto_ShaContext *ctx, const Crypto_ShaParams *params);
typedef int32_t (*Crypto_ShaCloseFxn)(Crypto_ShaContext *ctx);
typedef int32_t (*Crypto_ShaStartFxn)(Crypto_ShaContext *ctx);
typedef int32_t (*Crypto_ShaUpdateFxn)(Crypto_ShaContext *ctx, const uint8_t *input, uint32_t ilen);
typedef int32_t (*Crypto_ShaFinishFxn)(Crypto_ShaContext *ctx, uint8_t *output);
typedef int32_t (*Crypto_ShaSingleShotFxn)(Crypto_ShaContext *ctx, const uint8_t *input, uint32_t ilen, uint8_t *output);
typedef int32_t (*Crypto_hmacShaFxn)(Crypto_ShaContext *ctx, const uint8_t *input, uint32_t ilen, uint8_t *output);

/** \brief Driver implementation callbacks */
typedef struct Crypto_ShaFxns_s
{
    Crypto_ShaOpenFxn       openFxn;
    /**< CRYPTO driver implementation specific callback */
    Crypto_ShaCloseFxn      closeFxn;
    /**< CRYPTO driver implementation specific callback */
    Crypto_ShaStartFxn      startFxn;
    /**< CRYPTO driver implementation specific callback */
    Crypto_ShaUpdateFxn     updateFxn;
    /**< CRYPTO driver implementation specific callback */
    Crypto_ShaFinishFxn     finishFxn;
    /**< CRYPTO driver implementation specific callback */
    Crypto_ShaSingleShotFxn singleShotFxn;
    /**< CRYPTO driver implementation specific callback */
    Crypto_hmacShaFxn       hmacShaFxn;
    /**< CRYPTO driver implementation specific callback */
} Crypto_ShaFxns;

/** \brief CRYPTO SHA driver context */
struct Crypto_ShaContext_s
{
    Crypto_ShaParams    params;
    /**< Driver params passed during open */
    Crypto_ShaFxns     *fxns;
    /**< Pointer to driver functions */
    void               *drvHandle;
    /**< Handle for SA2UL driver */
    uintptr_t          rsv[60U];
    /**< Driver object. Should NOT be modified by end users */
};

/**
 *  \brief CRYPTO SHA global configuration array
 *
 *  This structure needs to be defined before calling #Crypto_shaOpen() and it must
 *  not be changed by user thereafter.
 */
typedef struct
{
    Crypto_ShaFxns     *fxns;
    /**< Pointer to driver functions */
} Crypto_ShaConfig;

/** \brief Externally defined SHA driver configuration array */
extern Crypto_ShaConfig gCryptoShaConfig[CRYPTO_NUM_TYPE];

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/**
 *  \brief Set default parameters in the \ref Crypto_ShaParams structure
 *
 *  Call this API to set defaults and then override the fields as needed
 *  before calling  \ref Crypto_shaOpen.
 *
 *  \param params   [OUT] Initialized parameters
 */
void Crypto_ShaParams_init(Crypto_ShaParams *params);

/**
 * \brief               This function gives the configuration based on type
 *
 * \param ctx           The SHA context to initialize. This must not be \c NULL.
 * \param params        Type to which to assign the context
 */
Crypto_ShaHandle Crypto_shaOpen(Crypto_ShaContext *ctx, const Crypto_ShaParams *params);

/**
 * \brief          This function clears a Secure Hash Algorithm (SHA ) context.
 *
 * \param handle   The SHA  context to clear. This may be \c NULL,
 *                 in which case this function does nothing. If it
 *                 is not \c NULL, it must point to an initialized
 *                 SHA  context.
*/
int32_t Crypto_shaClose(Crypto_ShaHandle handle);

/**
 * \brief          This function starts a Secure Hash Algorithm (SHA ) checksum
 *                 calculation.
 *
 * \param handle   SHA driver handle from \ref Crypto_shaOpen
 *
 * \return status  \c 0 on success.
 *                 A negative error code on failure.
 */
int32_t Crypto_shaStarts(Crypto_ShaHandle handle);

/**
 * \brief          This function feeds an input buffer into an ongoing Secure Hash Algorithm
 *                 (SHA ) checksum calculation.
 *
 *\param handle    SHA driver handle from \ref Crypto_shaOpen
 *
 * \param input    The buffer holding the input data. This must
 *                 be a readable buffer of length \p ilen Bytes.
 * \param ilen     The length of the input data in Bytes.
 *
 * \return status  \c 0 on success.
 *                 A negative error code on failure.
 */
int32_t Crypto_shaUpdate(Crypto_ShaHandle handle, const uint8_t *input, uint32_t ilen);

/**
 * \brief          This function finishes the Secure Hash Algorithm (SHA ) operation, and writes
 *                 the result to the output buffer.
 *
 * \param handle   SHA driver handle from \ref Crypto_shaOpen
 *
 * \param output   The SHA  checksum result.
 *
 * \return status  \c 0 on success.
 *                 A negative error code on failure.
 */
int32_t Crypto_shaFinish(Crypto_ShaHandle handle, uint8_t *output);

/**
 * \brief          This function calculates the Secure Hash Algorithm (SHA)
 *                 checksum of a buffer.
 *
 *                 The function allocates the context, performs the
 *                 calculation, and frees the context.
 *
 *                 The SHA result is calculated as
 *                 output = SHA(input buffer).
 *
 * \param handle   SHA driver handle from \ref Crypto_shaOpen
 *
 * \param input    The buffer holding the input data. This must be
 *                 a readable buffer of length \p ilen Bytes.
 * \param ilen     The length of the input data in Bytes.
 * \param output   The SHA checksum result.
 *
 * \return status  \c 0 on success.
 *                 A negative error code on failure.
 */
int32_t Crypto_shaSingleShot(Crypto_ShaHandle handle, const uint8_t *input, uint32_t ilen, uint8_t *output);

/**
 * \brief          This function calculates the Keyed-Hash Message Authentication Code Secure Hash Algorithm (HMAC-SHA)
 *                 checksum of a buffer.
 *
 *                 The function allocates the context, performs the
 *                 calculation, and frees the context.
 *
 * \param handle   SHA driver handle from \ref Crypto_shaOpen
 *
 * \param input    The buffer holding the input data. This must be
 *                 a readable buffer of length \p ilen Bytes.
 * \param ilen     The length of the input data in Bytes.
 * \param output   The SHA checksum result.
 *
 * \return status  \c 0 on success.
 *                 A negative error code on failure.
 */
int32_t Crypto_hmacSha(Crypto_ShaHandle handle, const uint8_t *input, uint32_t ilen, uint8_t *output);

/* ========================================================================== */
/*                       Static Function Definitions                          */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                  Internal/Private Structure Declarations                   */
/* ========================================================================== */

/* None */

#ifdef __cplusplus
}
#endif

#endif /* CRYPTO_SHA_H_ */

/** @} */
