/*
 *  Copyright (c) 2021, Kunbus GmbH
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice, this
 *     list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *
 *  3. Neither the name of the copyright holder nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 *  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 *  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 *  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef _APP_REAL_IDENT_H_
#define _APP_REAL_IDENT_H_

#ifdef __cplusplus
extern "C" {
#endif

//Shall be less or equal to PN_API_IM_MAX_CARRIERS
#define APP_IM_CARRIER_NUM          1

//Helper structure for storing I&M carrier data in application
typedef struct APP_SImCarrierDescriptor
{
    uint32_t api;
    uint16_t slotNumber;
    uint16_t subslotNumber;
    uint16_t supportedIm;
    bool     deviceRepresentative;
    bool     moduleRepresentative;
    uint8_t  memoryIndex;                       // Number of element in the I&M fields array in persistent memory.
} APP_SImCarrierDescriptor_t;

// Examples of values for record index.
// For reference see
// IEC CD 61158-6-10  IEC:2020
// Chapter 5.2.4.4.4 Record index
typedef enum APP_ERecordIndexType
{
    APP_eREC_INDEX_USER_SPECIFIC_MIN = 0x0000,
    APP_eREC_INDEX_USER_SPECIFIC_MAX = 0x7FFF,
    APP_eREC_INDEX_IM0_DATA          = 0xAFF0,
    APP_eREC_INDEX_IM1_DATA          = 0xAFF1,
    APP_eREC_INDEX_IM2_DATA          = 0xAFF2,
    APP_eREC_INDEX_IM3_DATA          = 0xAFF3,
    APP_eREC_INDEX_IM4_DATA          = 0xAFF4,
    APP_eREC_INDEX_IM0_FILTERDATA    = 0xF840
} APP_ERecordIndexType_t;

// Examples of values for ErrorCode1 and ErrorCode2 (Low 16 bits are relevant).
// For reference see
// IEC CD 61158-6-10 © IEC:2020
// Chapter 4.10.3.4.6 Coding of the field PNIOStatus
// Table 645 - Coding of ErrorCode1 with ErrorDecode PNIORW
// Table 646 - Coding of ErrorCode2 with ErrorDecode PNIORW
typedef enum APP_ERwErrorCode
{
    APP_eRWE_NO_ERROR               = 0x0000,
    APP_eRWE_APPL_USER1             = 0xaa00,
    APP_eRWE_ACCESS_INVALID_INDEX   = 0xb000,
    APP_eRWE_ACCESS_INVALID_SLOT    = 0xb200,
    APP_eRWE_ACCESS_DENIED          = 0xb600
} APP_ERwErrorCode_t;

//+=============================================================================================
//|     Function prototypes
//+=============================================================================================

extern uint32_t APP_configureSubmodules(void);
extern uint32_t APP_configureImData(void);

extern uint32_t APP_cbWriteRecordInd (
    uint32_t api_p,
    uint16_t slot_p,
    uint16_t subslot_p,
    uint16_t index_p,
    uint8_t *pData_p,
    uint16_t dataLength_p);

#ifdef __cplusplus
}
#endif

#endif //_APP_REAL_IDENT_H_
