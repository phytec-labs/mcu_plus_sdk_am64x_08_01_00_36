/*!
 *  \example <PN_API_DEV>.h
 *
 *  \brief
 *  Profinet API for IO device.
 *
 *  \author
 *  KUNBUS GmbH
 *
 *  \date
 *  2020-11-11
 *
 *  \copyright
 *  Copyright (c) 2021, KUNBUS GmbH<br /><br />
 *  All rights reserved.<br />
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:<br />
 *  <ol>
 *  <li>Redistributions of source code must retain the above copyright notice, this
 *     list of conditions and the following disclaimer.</li>
 *  <li>Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.</li>
 *  <li>Neither the name of the copyright holder nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.</li>
 *  </ol>
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

#ifndef _PN_API_DEV_H_
#define _PN_API_DEV_H_

#ifdef __cplusplus
extern "C" {
#endif

/*!
 *  \brief
 *  IO Device Error Codes.
 *  \ingroup PN_API_DEV_ERROR_CODES
 */
typedef enum PN_API_DEV_EError
{
    PN_API_DEV_eOK                                                                              = 0x00000000, /*!< No Error, stack is working as expected*/
    PN_API_DEV_eERROR_MISSING_PARAMETER_POINTER                                                 = 0x38030401, /*!< error code to be documented*/
    PN_API_DEV_eERROR_PARAMETER_OUT_OF_RANGE                                                    = 0x38030402, /*!< parameter value out of specified range.*/
    PN_API_DEV_eERROR_NOT_INITIALIZED                                                           = 0x38030403, /*!< error code to be documented*/
    PN_API_DEV_eERROR_WRONG_STATE                                                               = 0x38030404, /*!< error code to be documented*/
    PN_API_DEV_eERROR_DIAGNOSIS_DATA_QUEUE_FULL                                                 = 0x38030405, /*!< Diagnosis Data Queue is full. Maximum Number of Diagnosis Data Events is set to #PNC_MAX_DIAG_ITEM.*/
    PN_API_DEV_eERROR_ALARM_QUEUE_FULL                                                          = 0x38030406, /*!< Alarm Queue is full. Maximum Number of Alarm Events is set to #PNC_MAX_ALARM_ITEM.*/
    PN_API_DEV_eERROR_PLUG_MODULE_INVALID_SLOT                                                  = 0x38030407, /*!< Invalid slot number exceeding the maximum limit set. The maximum limit is defined as #PNC_SLOT_MAX.*/
    PN_API_DEV_eERROR_PLUG_MODULE_SLOT_ALREADY_PLUGGED                                          = 0x38030408, /*!< At the slot number specified a module is already plugged.*/
    PN_API_DEV_eERROR_PLUG_MODULE_INVALID_MODULE_TYPE                                           = 0x38030409, /*!< Invalid type of the module.*/
    PN_API_DEV_eERROR_PLUG_SUBMODULE_INVALID_SLOT                                               = 0x3803040a, /*!< Either slot is empty or invalid slot number.*/
    PN_API_DEV_eERROR_PLUG_SUBMODULE_INVALID_SUBSLOT                                            = 0x3803040b, /*!< reserved for future use.*/ //9FF is per spec max subslot number.
    PN_API_DEV_eERROR_PLUG_SUBMODULE_NO_FREE_SUBSLOT                                            = 0x3803040c, /*!< No free subslot. Subslot Number is exceeding it's set limit of #PNC_SUB_SLOT_MAX.*/
    PN_API_DEV_eERROR_PLUG_SUBMODULE_INVALID_SUBMODULE_TYPE                                     = 0x3803040d, /*!< Invalid submodule type.*/
    PN_API_DEV_eERROR_PLUG_SUBMODULE_SUBSSLOT_ALREADY_PLUGGED                                   = 0x3803040e, /*!< Subslot with this number is already plugged.*/
    PN_API_DEV_eERROR_PLUG_MODULE_INVALID_SLOT_ADDRESS                                          = 0x3803040f, /*!< obsolete error code - to be removed in next version!*/
    PN_API_DEV_eERROR_PULL_MODULE_INVALID_SLOT                                                  = 0x38030410, /*!< Invalid slot number (either > #PNC_SLOT_MAX, or the slot is empty).*/
    PN_API_DEV_eERROR_PULL_SUBMODULE_INVALID_SLOT                                               = 0x38030411, /*!< Either slot is empty or invalid slot number.*/
    PN_API_DEV_eERROR_PULL_SUBMODULE_INVALID_SUBSLOT                                            = 0x38030412, /*!< error code to be documented*/
    PN_API_DEV_eERROR_PULL_SUBMODULE_NOT_SUCCEDED                                               = 0x38030413, /*!< error code to be documented*/
    PN_API_DEV_eERROR_CONFIG_DAP_MISSING                                                        = 0x38030414, /*!< DAP Module is missing.*/
    PN_API_DEV_eERROR_CONFIG_IM_DEVICE_REPRESENTATIVE_MISSING                                   = 0x38030415, /*!< No submodule was nominated as I&M device representative.*/
    PN_API_DEV_eERROR_CONFIG_IM_DEVICE_REPRESENTATIVE_INVALID                                   = 0x38030416, /*!< obsolete error code - to be removed in next version!*/
    PN_API_DEV_eERROR_CONFIG_NO_SUBMODULES_IN_MODULE                                            = 0x38030417, /*!< No Submodule plugged in Module. Every module must contain at least one submodule.*/
    PN_API_DEV_eERROR_IM_INVALID_REPRESENTATIVE_SCOPE                                           = 0x38030418, /*!< obsolete error code - to be removed in next version!*/
    PN_API_DEV_eERROR_IM_DEVICE_REPRESENTATIVE_ALREADY_ASSIGNED                                 = 0x38030419, /*!< obsolete error code - to be removed in next version!*/
    PN_API_DEV_eERROR_IM_MODULE_REPRESENTATIVE_ALREADY_ASSIGNED                                 = 0x3803041a, /*!< obsolete error code - to be removed in next version!*/
    PN_API_DEV_eERROR_IM_CARRIER_NO_IM0_DATA                                                    = 0x3803041b, /*!< obsolete error code - to be removed in next version!*/
    PN_API_DEV_eERROR_IM_DEVICE_REPRESENTATIVE_NO_IM1_DATA                                      = 0x3803041c, /*!< obsolete error code - to be removed in next version!*/
    PN_API_DEV_eERROR_IM_DEVICE_REPRESENTATIVE_NO_IM2_DATA                                      = 0x3803041d, /*!< obsolete error code - to be removed in next version!*/
    PN_API_DEV_eERROR_IM_DEVICE_REPRESENTATIVE_NO_IM3_DATA                                      = 0x3803041e, /*!< obsolete error code - to be removed in next version!*/
    PN_API_DEV_eERROR_IM_DATA_SET_INVALID_POINTER                                               = 0x3803041f, /*!< obsolete error code - to be removed in next version!*/
    PN_API_DEV_eERROR_IM_DATA_SET_NO_FREE_SPACE                                                 = 0x38030420, /*!< obsolete error code - to be removed in next version!*/
    PN_API_DEV_eERROR_MAP_LENGTH_OF_PROCESS_IMAGE_INPUT_EXCEEDED                                = 0x38030421, /*!< error code to be documented*/
    PN_API_DEV_eERROR_MAP_LENGTH_OF_PROCESS_IMAGE_OUTPUT_EXCEEDED                               = 0x38030422, /*!< error code to be documented*/
    PN_API_DEV_eERROR_MAP_PROCESS_IMAGE_INPUT_OVERLAP                                           = 0x38030423, /*!< error code to be documented*/
    PN_API_DEV_eERROR_MAP_PROCESS_IMAGE_OUTPUT_OVERLAP                                          = 0x38030424, /*!< error code to be documented*/
    PN_API_DEV_eERROR_MAP_INVALID_SLOT                                                          = 0x38030425, /*!< error code to be documented*/
    PN_API_DEV_eERROR_MAP_INVALID_SUBSLOT                                                       = 0x38030426, /*!< error code to be documented*/
    PN_API_DEV_eERROR_PROCESS_IDENTIFIER_ALREADY_PRESENT                                        = 0x38030427, /*!< obsolete error code - to be removed in next version*/
    PN_API_DEV_eERROR_PROCESS_IDENTIFIER_LIST_FULL                                              = 0x38030428, /*!< obsolete error code - to be removed in next version*/
    PN_API_DEV_eERROR_SET_MODULE_PROCESS_IDENTIFIER_NOT_SUPPORTED                               = 0x38030429, /*!< obsolete error code - to be removed in next version*/
    PN_API_DEV_eERROR_SET_SUBMODULE_PROCESS_IDENTIFIER_NOT_SUPPORTED                            = 0x3803042a, /*!< obsolete error code - to be removed in next version*/
    PN_API_DEV_eERROR_REG_MOD_SUBST_INVALID_POINTER                                             = 0x3803042b, /*!< Invalid Pointer Address NULL out of module substitute context.*/
    PN_API_DEV_eERROR_REG_MOD_SUBST_INVALID_NUM_OF_ELEMS                                        = 0x3803042c, /*!< Module Substitutes array size exceeds set maximum limit or is equal zero. The maximum limit is defined as #PNC_MAX_MOD_SUBST_ENTRIES (numberElements_p == 0) || (numberElements_p > PNC_MAX_MOD_SUBST_ENTRIES)*/
    PN_API_DEV_eERROR_REG_MOD_SUBST_NO_FREE_SPACE                                               = 0x3803042d, /*!< Number of systemwide Module Substitutes exceeds set maximum limit. The maximum limit is defined as #PNC_MAX_MOD_SUBST_ENTRIES */
    PN_API_DEV_eERROR_REG_SUBMOD_SUBST_INVALID_POINTER                                          = 0x3803042e, /*!< Invalid Pointer Address NULL out of submodule substitute context.*/
    PN_API_DEV_eERROR_REG_SUBMOD_SUBST_INVALID_NUM_OF_ELEMS                                     = 0x3803042f, /*!< Submodule Substitutes array size exceeds set maximum limit or is equal zero. The maximum limit is defined as #PNC_MAX_SUBMOD_SUBST_ENTRIES (numberElements_p == 0) || (numberElements_p > PNC_MAX_MOD_SUBST_ENTRIES)*/
    PN_API_DEV_eERROR_REG_SUBMOD_SUBST_NO_FREE_SPACE                                            = 0x38030430  /*!< Number of systemwide Submodule Substitutes exceeds set maximum limit. The maximum limit is defined as #PNC_MAX_SUBMOD_SUBST_ENTRIES */
} PN_API_DEV_EError_t;

typedef enum PN_API_DEV_ERunState
{
    PN_API_DEV_eRS_NO_CONNECTION,
    PN_API_DEV_eRS_CONNECTION,
    PN_API_DEV_eRS_RUN,
    PN_API_DEV_eRS_FORCE32BIT = 0xffffffff  /*!< Force compiler to set enum size to 32 bit */
} PN_API_DEV_ERunState_t;

typedef enum PN_API_DEV_EDeviceState
{
    PN_API_DEV_eDS_DEVICE_OK,
    PN_API_DEV_eDS_DEVICE_FAILURE,
    PN_API_DEV_eDS_MAINTENANCE,
    PN_API_DEV_eDS_MANUFACTURER_SPECIFIC_STATUS,
    PN_API_DEV_eDS_FORCE32BIT = 0xffffffff  /*!< Force compiler to set enum size to 32 bit */
} PN_API_DEV_EDeviceState_t;

typedef enum PN_API_DEV_EApplicationDataState
{
    PN_API_DEV_eADS_INVALID,
    PN_API_DEV_eADS_VALID,
    PN_API_DEV_eADS_FORCE32BIT = 0xffffffff  /*!< Force compiler to set enum size to 32 bit */
} PN_API_DEV_EApplicationDataState_t;

typedef enum PN_API_DEV_EAr
{
    PN_API_DEV_eAR_UNKNOWN,
    PN_API_DEV_eAR_IOD_1,
    PN_API_DEV_eAR_IOD_2,
    PN_API_DEV_eAR_DA,
    PN_API_DEV_eAR_FORCE32BIT = 0xffffffff  /*!< Force compiler to set enum size to 32 bit */
} PN_API_DEV_EAr_t;

typedef enum PN_API_DEV_EEquipmentModuleType
{
    PN_API_DEV_eEQU_MT_IO,
    PN_API_DEV_eEQU_MT_DAP,
    PN_API_DEV_eEQU_MT_FORCE32BIT = 0xffffffff  /*!< Force compiler to set enum size to 32 bit */
} PN_API_DEV_EEquipmentModuleType_t;

typedef enum PN_API_DEV_EEquipmentSubmoduleType
{
    PN_API_DEV_eEQU_ST_NOIO,
    PN_API_DEV_eEQU_ST_INPUT,
    PN_API_DEV_eEQU_ST_OUTPUT,
    PN_API_DEV_eEQU_ST_INOUT,
    PN_API_DEV_eEQU_ST_FORCE32BIT = 0xffffffff  /*!< Force compiler to set enum size to 32 bit */
} PN_API_DEV_EEquipmentSubmoduleType_t;

typedef enum PN_API_DEV_EDiagnosisChannelPropertiesType
{
    PN_API_DEV_eDIAG_CPT_UNSPECIFIC,
    PN_API_DEV_eDIAG_CPT_1BIT,
    PN_API_DEV_eDIAG_CPT_2BIT,
    PN_API_DEV_eDIAG_CPT_4BIT,
    PN_API_DEV_eDIAG_CPT_BYTE,
    PN_API_DEV_eDIAG_CPT_WORD,
    PN_API_DEV_eDIAG_CPT_2WORD,
    PN_API_DEV_eDIAG_CPT_4WORD,
    PN_API_DEV_eDIAG_CPT_FORCE32BIT = 0xffffffff  /*!< Force compiler to set enum size to 32 bit */
} PN_API_DEV_EDiagnosisChannelPropertiesType_t;

typedef enum PN_API_DEV_EDiagnosisChannelPropertiesAccumulative
{
    PN_API_DEV_eDIAG_CPA_INDIVIDUAL,
    PN_API_DEV_eDIAG_CPA_CHANNELGROUP,
    PN_API_DEV_eDIAG_CPA_FORCE32BIT = 0xffffffff  /*!< Force compiler to set enum size to 32 bit */
} PN_API_DEV_EDiagnosisChannelPropertiesAccumulative_t;

typedef enum PN_API_DEV_EDiagnosisChannelPropertiesMaintenance
{
    PN_API_DEV_eDIAG_CPM_DIAGNOSIS,
    PN_API_DEV_eDIAG_CPM_MAINTENANCE_REQUIRED,
    PN_API_DEV_eDIAG_CPM_MAINTENANCE_DEMANDED,
    PN_API_DEV_eDIAG_CPM_QUALIFIED_DIAGNOSIS,
    PN_API_DEV_eDIAG_CPM_FORCE32BIT = 0xffffffff  /*!< Force compiler to set enum size to 32 bit */
} PN_API_DEV_EDiagnosisChannelPropertiesMaintenance_t;

typedef enum PN_API_DEV_EDiagnosisChannelPropertiesDirection
{
    PN_API_DEV_eDIAG_CPD_INPUT_CHANNEL,
    PN_API_DEV_eDIAG_CPD_OUTPUT_CHANNEL,
    PN_API_DEV_eDIAG_CPD_BIDIRECTIONAL_CHANNEL,
    PN_API_DEV_eDIAG_CPD_MANUFACTURER_SPECIFIC,
    PN_API_DEV_eDIAG_CPD_FORCE32BIT = 0xffffffff  /*!< Force compiler to set enum size to 32 bit */
} PN_API_DEV_EDiagnosisChannelPropertiesDirection_t;

typedef enum PN_API_DEV_EDiagnosisChannelPropertiesSpecifier
{
    PN_API_DEV_eDIAG_CPS_APPEARS,
    PN_API_DEV_eDIAG_CPS_DISAPPEARS,
    PN_API_DEV_eDIAG_CPS_FORCE32BIT = 0xffffffff  /*!< Force compiler to set enum size to 32 bit */
} PN_API_DEV_EDiagnosisChannelPropertiesSpecifier_t;

typedef enum PN_API_DEV_EDiagnosisUserStructureIdentifier
{
    PN_API_DEV_eDIAG_USI_CHANNEL_DIAGNOSIS,
    PN_API_DEV_eDIAG_USI_EXT_CHANNEL_DIAGNOSIS,
    PN_API_DEV_eDIAG_USI_QUALIFIED_CHANNEL_DIAGNOSIS,
    PN_API_DEV_eDIAG_USI_MANUFACTURER_SPECIFIC,
    PN_API_DEV_eDIAG_USI_FORCE32BIT = 0xffffffff  /*!< Force compiler to set enum size to 32 bit */
} PN_API_DEV_EDiagnosisUserStructureIdentifier_t;

typedef void (*PN_API_DEV_getSerialNumberCallback) (uint8_t *pBuffer_p, uint16_t length_p);
typedef uint16_t (*PN_API_DEV_getHwRevisionCallback) (void);
typedef uint8_t (*PN_API_DEV_getSwRevisionFunctionalEnhancementCallback) (void);
typedef uint8_t (*PN_API_DEV_getSwRevisionBugfixCallback) (void);
typedef uint8_t (*PN_API_DEV_getSwRevisionInternalChangeCallback) (void);
typedef uint16_t (*PN_API_DEV_getRevisionCounterCallback) (uint16_t index_p);
typedef void (*PN_API_DEV_setStationNameIndicatedCallback) (const uint8_t *pName_p, uint16_t length_p);
typedef void (*PN_API_DEV_setIpAddressIndicatedCallback) (uint32_t ipAddress_p, uint32_t subNetMask_p, uint32_t gateWayAddress_p);
typedef void (*PN_API_DEV_factoryResetIndicatedCallback) (void);
typedef void (*PN_API_DEV_resetToFactoryIndicatedCallback) (void);
typedef void (*PN_API_DEV_connectResponseCallback) (PN_API_DEV_EAr_t ar_p, uint32_t err_p);
typedef void (*PN_API_DEV_applicationReadyRequestCallback) (PN_API_DEV_EAr_t ar_p, uint32_t err_p);
typedef void (*PN_API_DEV_releaseResponseCallback) (PN_API_DEV_EAr_t ar_p, uint32_t err_p);
typedef void (*PN_API_DEV_readRecordResponseCallback) (PN_API_DEV_EAr_t ar_p, uint32_t err_p);
typedef void (*PN_API_DEV_writeRecordResponseCallback) (PN_API_DEV_EAr_t ar_p, uint32_t err_p);
typedef void (*PN_API_DEV_connectIndicatedCallback) (PN_API_DEV_EAr_t ar_p);
typedef void (*PN_API_DEV_parametrizationEndIndicatedCallback) (PN_API_DEV_EAr_t ar_p);
typedef void (*PN_API_DEV_abortIndicatedCallback) (PN_API_DEV_EAr_t ar_p);
typedef void (*PN_API_DEV_alarmUpdateIndicatedCallback) (uint32_t api_p, uint16_t slot_p, uint16_t subslot_p, uint16_t alarmSpecifier_p);
typedef uint32_t (*PN_API_DEV_readRecordIndicatedCallback) (uint32_t api_p, uint16_t slot_p, uint16_t subslot_p, uint16_t index_p, uint16_t maxDataLength_p, uint8_t *pData_p, uint16_t *pDataLength_p);
typedef uint32_t (*PN_API_DEV_writeRecordIndicatedCallback) (uint32_t api_p, uint16_t slot_p, uint16_t subslot_p, uint16_t index_p, uint8_t *pData_p, uint16_t dataLength_p);

typedef struct PN_API_DEV_SDiagnosisData
{
    uint16_t channel;
    PN_API_DEV_EDiagnosisChannelPropertiesType_t type;
    PN_API_DEV_EDiagnosisChannelPropertiesAccumulative_t accumulative;
    PN_API_DEV_EDiagnosisChannelPropertiesDirection_t direction;
    PN_API_DEV_EDiagnosisChannelPropertiesSpecifier_t specifier;
    PN_API_DEV_EDiagnosisChannelPropertiesMaintenance_t maintenance;
    PN_API_DEV_EDiagnosisUserStructureIdentifier_t usi;
    uint16_t channelErrorType;
    uint16_t extChannelErrorType;
    uint32_t extChannelAddValue;
    uint32_t qualifiedChannelQualifier;
} PN_API_DEV_SDiagnosisData_t;

typedef struct PN_API_DEV_SModuleSubstitute
{
    uint32_t id;
} PN_API_DEV_SModuleSubstitute_t;

typedef struct PN_API_DEV_SModuleSubstituteEntry_t PN_API_DEV_SModuleSubstituteEntry_t;

typedef struct PN_API_DEV_SSubmoduleSubstitute
{
    uint32_t id;
} PN_API_DEV_SSubmoduleSubstitute_t;

typedef struct PN_API_DEV_SSubmoduleSubstituteEntry_t PN_API_DEV_SSubmoduleSubstituteEntry_t;

typedef struct PN_API_DEV_SSubmoduleDescriptor
{
    uint32_t api;
    uint32_t id;
    PN_API_DEV_EEquipmentSubmoduleType_t type;
    uint16_t inputLength;
    uint16_t inputOffsetPii;
    uint16_t outputLength;
    uint16_t outputOffsetPio;
    PN_API_DEV_SSubmoduleSubstituteEntry_t *pSubstitutes;
} PN_API_DEV_SSubmoduleDescriptor_t;

typedef struct PN_API_DEV_SModuleDescriptor
{
    uint32_t api;
    uint32_t id;
    PN_API_DEV_EEquipmentModuleType_t type;
    PN_API_DEV_SModuleSubstituteEntry_t *pSubstitutes;
} PN_API_DEV_SModuleDescriptor_t;

typedef struct PN_API_DEV_SModule
{
    uint16_t slotNumber;
    uint32_t moduleIdNumber;
    uint16_t moduleState;
    uint16_t numberOfSubModules;
} PN_API_DEV_SModule_t;

typedef struct PN_API_DEV_SSubmodule
{
    uint16_t subSlot;
    uint32_t id;
    union
    {
        uint16_t properties;
        uint16_t state;
    };
} PN_API_DEV_SSubmodule_t;

extern uint32_t PN_API_DEV_setInputDataBuffer(uint8_t *pAddr1_p, uint8_t *pAddr2_p, uint8_t *pAddr3_p, uint16_t bufferSize_p);
extern uint32_t PN_API_DEV_setOutputDataBuffer(uint8_t *pAddr1_p, uint8_t *pAddr2_p, uint8_t *pAddr3_p, uint16_t bufferSize_p);
extern uint32_t PN_API_DEV_getBufferOutputData(uint8_t **ppBuffer_p);
extern uint32_t PN_API_DEV_getBufferInputData(uint8_t **ppBuffer_p);
extern uint32_t PN_API_DEV_releaseBufferInputData(void);
extern uint32_t PN_API_DEV_applyEquipmentConfiguration(void);
extern uint32_t PN_API_DEV_plugModule(uint16_t slotNumber_p, PN_API_DEV_SModuleDescriptor_t *pModuleDescriptor_p);
extern uint32_t PN_API_DEV_pullModule(uint16_t slotNumber_p);
extern uint32_t PN_API_DEV_plugSubmodule(uint16_t slotNumber_p, uint16_t subslotNumber_p, PN_API_DEV_SSubmoduleDescriptor_t *pSubmoduleDescriptor_p);
extern uint32_t PN_API_DEV_pullSubmodule(uint16_t slotNumber_p, uint16_t subslotNumber_p);
extern uint32_t PN_API_DEV_registerModuleSubstitutes(PN_API_DEV_SModuleSubstitute_t *pModuleSubstitutes_p, uint8_t numberElements_p, PN_API_DEV_SModuleSubstituteEntry_t **ppModuleSubstituteEntryAddress_p);
extern uint32_t PN_API_DEV_registerSubmoduleSubstitutes(PN_API_DEV_SSubmoduleSubstitute_t *pSubmoduleSubstitutes_p, uint8_t numberElements_p, PN_API_DEV_SSubmoduleSubstituteEntry_t **ppSubmoduleSubstituteEntryAddress_p);
extern uint32_t PN_API_DEV_getRunState(PN_API_DEV_ERunState_t *pRunState_p);
extern uint32_t PN_API_DEV_getDeviceState(PN_API_DEV_EDeviceState_t *pDeviceState_p);
extern uint32_t PN_API_DEV_setApplicationReady(PN_API_DEV_EAr_t ar_p);
extern uint32_t PN_API_DEV_setApplicationDataState(PN_API_DEV_EApplicationDataState_t state_p);
extern uint32_t PN_API_DEV_setDiagnosisData(PN_API_DEV_SDiagnosisData_t *pData_p);
extern uint32_t PN_API_DEV_setAlarm(uint16_t alarmType_p);
extern uint32_t PN_API_DEV_registerSetStationNameIndicatedCallback(PN_API_DEV_setStationNameIndicatedCallback cbFunction_p);
extern uint32_t PN_API_DEV_registerSetIpAddressIndicatedCallback(PN_API_DEV_setIpAddressIndicatedCallback cbFunction_p);
extern uint32_t PN_API_DEV_registerFactoryResetIndicatedCallback(PN_API_DEV_factoryResetIndicatedCallback cbFunction_p);
extern uint32_t PN_API_DEV_registerResetToFactoryIndicatedCallback(PN_API_DEV_resetToFactoryIndicatedCallback cbFunction_p);
extern uint32_t PN_API_DEV_registerConnectResponseCallback(PN_API_DEV_connectResponseCallback cbFunction_p);
extern uint32_t PN_API_DEV_registerApplicationReadyRequestCallback(PN_API_DEV_applicationReadyRequestCallback cbFunction_p);
extern uint32_t PN_API_DEV_registerReleaseResponseCallback(PN_API_DEV_releaseResponseCallback cbFunction_p);
extern uint32_t PN_API_DEV_registerReadRecordResponseCallback(PN_API_DEV_readRecordResponseCallback cbFunction_p);
extern uint32_t PN_API_DEV_registerWriteRecordResponseCallback(PN_API_DEV_writeRecordResponseCallback cbFunction_p);
extern uint32_t PN_API_DEV_registerConnectIndicatedCallback(PN_API_DEV_connectIndicatedCallback cbFunction_p);
extern uint32_t PN_API_DEV_registerParametrizationEndIndicatedCallback(PN_API_DEV_parametrizationEndIndicatedCallback cbFunction_p);
extern uint32_t PN_API_DEV_registerAbortIndicatedCallback(PN_API_DEV_abortIndicatedCallback cbFunction_p);
extern uint32_t PN_API_DEV_registerAlarmUpdateIndicatedCallback(PN_API_DEV_alarmUpdateIndicatedCallback cbFunction_p);
extern uint32_t PN_API_DEV_registerReadRecordIndicatedCallback(PN_API_DEV_readRecordIndicatedCallback cbFunction_p);
extern uint32_t PN_API_DEV_registerWriteRecordIndicatedCallback(PN_API_DEV_writeRecordIndicatedCallback cbFunction_p);
extern uint32_t PN_API_DEV_getModuleDiffBlockNumberOfModules(PN_API_DEV_EAr_t ar_p, uint16_t *pNumberOfModules_p);
extern uint32_t PN_API_DEV_getModuleDiffBlockModule(PN_API_DEV_EAr_t ar_p, uint16_t index_p, PN_API_DEV_SModule_t *pModule_p);
extern uint32_t PN_API_DEV_getModuleDiffBlockSubmodule(PN_API_DEV_EAr_t ar_p, PN_API_DEV_SModule_t *pModule_p, uint16_t index_p, PN_API_DEV_SSubmodule_t *pSubmodule_p);
extern uint32_t PN_API_DEV_getNumberOfExpectedModules(PN_API_DEV_EAr_t ar_p, uint16_t *pNumberOfModules_p);
extern uint32_t PN_API_DEV_getExpectedModule(PN_API_DEV_EAr_t ar_p, uint16_t index_p, PN_API_DEV_SModule_t *pModule_p);
extern uint32_t PN_API_DEV_getExpectedSubmodule(PN_API_DEV_EAr_t ar_p, PN_API_DEV_SModule_t *pModule_p, uint16_t index_p, PN_API_DEV_SSubmodule_t *pSubmodule_p);
#ifdef  __cplusplus 
} 
#endif 

#endif //_PN_API_DEV_H_
