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
 *  \defgroup SECURITY_CRYPTO_AES_MODULE APIs for CRYPTO AES
 *  \ingroup SECURITY_MODULE
 *
 *  @{
 *
 */

#ifndef CRYPTO_AES_H_
#define CRYPTO_AES_H_

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
 *  \anchor Crypto_AesAlgoType
 *  \name AES algorithm type
 *  @{
 */
/** \brief AES decryption algorithm */
#define CRYPTO_AES_DECRYPT          (0U)
/** \brief AES encryption algorithm */
#define CRYPTO_AES_ENCRYPT          (1U)
/** \brief AES CMAC algorithm */
#define CRYPTO_AES_CMAC             (2U)
/** @} */

/**
 *  \anchor Crypto_AesMode
 *  \name AES modes
 *  @{
 */
/** \brief AES CBC 128 bit MODE and Key size should be 128 bits */
#define CRYPTO_AES_CBC_128                  (0U)
/** \brief AES CBC 192 bit MODE and Key size should be 192 bits */
#define CRYPTO_AES_CBC_192                  (1U)
/** \brief AES CBC 256 bit MODE and Key size should be 256 bits */
#define CRYPTO_AES_CBC_256                  (2U)
/** \brief AES CMAC with 128-bit ECB mode. */
#define CRYPTO_AES_CMAC_128                 (3U)
/** \brief AES CMAC with 192-bit ECB mode. */
#define CRYPTO_AES_CMAC_192                 (4U)
/** \brief AES CMAC with 256-bit ECB mode. */
#define CRYPTO_AES_CMAC_256                 (5U)
/** @} */

/** \brief IV buffer length should be 16 bytes */
#define CRYPTO_AES_IV_LENGTH          (16U)

/** \brief key buffer length, min 16 bytes and max 32 bytes */
#define CRYPTO_AES_KEY_LENGTH         (32U)

/** \brief Handle to the Crypto AES driver returned by #Crypto_aesOpen() */
typedef void *Crypto_AesHandle;

/** \brief Forward declaration of \ref Crypto_AesContext_s */
typedef struct Crypto_AesContext_s Crypto_AesContext;

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/**
 *  \brief Parameters passed during Crypto_aesOpen()
 */
typedef struct Crypto_AesParams_s
{
    uint32_t                aesMode;
    /**< Aes mode. Refer \ref Crypto_AesMode */
    uint32_t                algoType;
    /**< Aes mode. Refer \ref Crypto_AesAlgoType */
    uint8_t                 iv[CRYPTO_AES_IV_LENGTH];
    /** Initialization vector (IV) for Aes alogorithm if Cmac case IV not required */
    uint8_t                 key[CRYPTO_AES_KEY_LENGTH];
    /** Key for Aes alogorithm */
    uint32_t                keySizeInBits;
    /** Key size in bytes for Aes alogorithm */
    uint32_t                type;
    /**< Type to which to assign the context. Refer Crypto_Type */
} Crypto_AesParams;

/** \brief common callback functions declarations*/
typedef int32_t (*Crypto_AesOpenFxn)(Crypto_AesContext *ctx, const Crypto_AesParams *params);
typedef int32_t (*Crypto_AesCloseFxn)(Crypto_AesContext *ctx);

/** \brief aes cbc callback functions declarations*/
typedef int32_t (*Crypto_AesSetKeyEncFxn)(Crypto_AesContext *ctx);
typedef int32_t (*Crypto_AesSetKeyDecFxn)(Crypto_AesContext *ctx);
typedef int32_t (*Crypto_AesCbcFxn)(Crypto_AesContext *ctx, const uint8_t *input, uint32_t inputLength, uint8_t *output );

/** \brief aes cmac callback functions declarations*/
typedef int32_t (*Crypto_CmacSetUpFxn)(Crypto_AesContext *ctx);
typedef int32_t (*Crypto_CmacStartFxn)(Crypto_AesContext *ctx);
typedef int32_t (*Crypto_CmacUpdateFxn)(Crypto_AesContext *ctx, const uint8_t *input, uint32_t ilen);
typedef int32_t (*Crypto_CmacFinishFxn)(Crypto_AesContext *ctx, uint8_t *output);
typedef int32_t (*Crypto_CmacSingleShotFxn)(Crypto_AesContext *ctx, const uint8_t *input, uint32_t ilen, uint8_t *output);

/** \brief Driver implementation callbacks */
typedef struct Crypto_AesCommonFxns_s
{
    Crypto_AesOpenFxn        openFxn;
    /**< CRYPTO driver implementation specific callback */
    Crypto_AesCloseFxn       closeFxn;
    /**< CRYPTO driver implementation specific callback */
} Crypto_AesCommonFxns;

/** \brief Driver implementation callbacks */
typedef struct Crypto_AesCbcFxns_s
{
    Crypto_AesSetKeyEncFxn   setKeyEncFxn;
    /**< CRYPTO driver implementation specific callback */
    Crypto_AesSetKeyDecFxn   setKeyDecFxn;
    /**< CRYPTO driver implementation specific callback */
    Crypto_AesCbcFxn         cbcFxn;
    /**< CRYPTO driver implementation specific callback */
} Crypto_AesCbcFxns;

/** \brief Driver implementation callbacks */
typedef struct Crypto_AesCmacFxns_s
{
    Crypto_CmacSetUpFxn      setUpFxn;
    /**< CRYPTO driver implementation specific callback */
    Crypto_CmacStartFxn      startFxn;
    /**< CRYPTO driver implementation specific callback */
    Crypto_CmacUpdateFxn     updateFxn;
    /**< CRYPTO driver implementation specific callback */
    Crypto_CmacFinishFxn     finishFxn;
    /**< CRYPTO driver implementation specific callback */
    Crypto_CmacSingleShotFxn singleShotFxn;
    /**< CRYPTO driver implementation specific callback */
} Crypto_AesCmacFxns;

/** \brief Driver implementation callbacks */
typedef struct Crypto_AesFxns_s
{
    Crypto_AesCommonFxns    *commonFxns;
    /**< CRYPTO driver implementation specific callback */
    Crypto_AesCbcFxns       *cbcFxns;
    /**< CRYPTO driver implementation specific callback */
    Crypto_AesCmacFxns      *cmacFxns;
    /**< CRYPTO driver implementation specific callback */
} Crypto_AesFxns;

/** \brief CRYPTO AES driver context */
struct Crypto_AesContext_s
{
    Crypto_AesParams    params;
    /**< Driver params passed during open */
    Crypto_AesFxns      *fxns;
    /**< Pointer to driver functions */
    uintptr_t           rsv[70U];
    /**< Driver object. Should NOT be modified by end users */
};

/**
 *  \brief CRYPTO AES global configuration array
 *
 *  This structure needs to be defined before calling #Crypto_aesOpen() and it must
 *  not be changed by user thereafter.
 */
typedef struct
{
    Crypto_AesFxns      *fxns;
    /**< Pointer to driver functions */
} Crypto_AesConfig;

/** \brief Externally defined AES driver configuration array */
extern Crypto_AesConfig gCryptoAesConfig[CRYPTO_NUM_TYPE];

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/**
 *  \brief Set default parameters in the \ref Crypto_AesParams structure
 *
 *  Call this API to set defaults and then override the fields as needed
 *  before calling  \ref Crypto_aesOpen.
 *
 *  \param params   [OUT] Initialized parameters
 */
void Crypto_AesParams_init(Crypto_AesParams *params);

/**
 * \brief               This function gives the configuration based on type
 * 
 * \param ctx           The AES context to initialize. This must not be \c NULL.
 * \param params        Type to which to assign the context
 */
Crypto_AesHandle Crypto_aesOpen(Crypto_AesContext *ctx, const Crypto_AesParams *params);

/**
 * \brief          This function clears a Advance Encryption Standard (AES) context.
 *
 * \param handle   The AES  context to clear. This may be \c NULL,
 *                 in which case this function does nothing. If it
 *                 is not \c NULL, it must point to an initialized
 *                 AES  context.
*/
int32_t Crypto_aesClose(Crypto_AesHandle handle);

/**
 * \brief          This function sets the Advance Encryption Standard (AES) encryption key.
 *
 * \param handle   AES driver handle from \ref Crypto_aesOpen.
 * 
 * \return status  \c 0 on success.
 *                 A negative error code on failure.
 */
int32_t Crypto_aesSetKeyEnc(Crypto_AesHandle handle);

/**
 * \brief          This function sets the Advance Encryption Standard (AES) decryption key.
 *
 * \param handle    AES driver handle from \ref Crypto_aesOpen.
 *
 * \return status  \c 0 on success.
 *                 A negative error code on failure.
 */
int32_t Crypto_aesSetKeyDec(Crypto_AesHandle handle);

/**
 * \brief          This function finishes the Advance Encryption Standard Cipher Block Chaining (AES-CBC) operation, and writes
 *                 the result to the output buffer.
 *
 * \param handle   AES driver handle from \ref Crypto_aesOpen
 * 
 * \param input    The buffer holding the input data. This must be
 *                 a readable buffer of length \p ilen Bytes.
 * 
 * \param ilen     The length of the input data in Bytes.
 * 
 * \param output   The AES  algorithm result.
 *
 * \return status  \c 0 on success.
 *                 A negative error code on failure.
 */
int32_t Crypto_aesCbc(Crypto_AesHandle handle, const uint8_t *input, uint32_t ilen, uint8_t *output );

/**
 * \brief          This function prepares a cipher context for use with the given cipher primitive.
 *
 * \param handle    AES driver handle from \ref Crypto_aesOpen.
 *
 * \return status  \c 0 on success.
 *                 A negative error code on failure.
 */
int32_t Crypto_cmacSetup(Crypto_AesHandle handle);

/**
 * \brief          This function sets the CMAC key, and prepares to authenticate the input data.
 *
 * \param handle    AES driver handle from \ref Crypto_aesOpen.
 *
 * \return status  \c 0 on success.
 *                 A negative error code on failure.
 */
int32_t Crypto_cmacStarts(Crypto_AesHandle handle);

/**
 * \brief          This function feeds an input buffer into an ongoing CMAC computation.
 *
 * \param handle    AES driver handle from \ref Crypto_aesOpen.
 *
 * \param input    The buffer holding the input data. This must
 *                 be a readable buffer of length \p ilen Bytes.
 * \param ilen     The length of the input data in Bytes.
 *
 * \return status  \c 0 on success.
 *                 A negative error code on failure.
 */
int32_t Crypto_cmacUpdate(Crypto_AesHandle handle, const uint8_t *input, uint32_t ilen);

/**
 * \brief           This function finishes the CMAC operation, and writes the result to the output buffer.
 *
 * \param handle    AES driver handle from \ref Crypto_aesOpen.
 *
 * \param output    The AES cmac algorithm result.
 *
 * \return status  \c 0 on success.
 *                 A negative error code on failure.
 */
int32_t Crypto_cmacFinish(Crypto_AesHandle handle, uint8_t *output);

/**
 * \brief           This function calculates the Advance Encryption Standard Cipher-based Message Authentication Code (AES-CMAC) of a buffer.
 *
 * \param handle    AES driver handle from \ref Crypto_aesOpen.
 *
 * \param input     The buffer holding the input data. This must be
 *                  a readable buffer of length \p ilen Bytes.
 *
 * \param ilen      The length of the input data in Bytes.
 *
 * \param output    The AES cmac algorithm result.
 *
 * \return status   \c 0 on success.
 *                  A negative error code on failure.
 */
int32_t Crypto_cmacSingleShot(Crypto_AesHandle handle, const uint8_t *input, uint32_t ilen, uint8_t *output);

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

#endif /* CRYPTO_AES_H_ */

/** @} */
