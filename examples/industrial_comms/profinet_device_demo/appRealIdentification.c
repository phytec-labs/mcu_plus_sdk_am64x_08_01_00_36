/*!
 *  \example appRealIdentification.c
 *
 *  \brief
 *  PROFINET Device Real Identification;
 *  such as Submodules, I&M data...
 *
 *  \author
 *  KUNBUS GmbH
 *
 *  \date
 *  2020-06-19
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

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <osal.h>

#include <PN_API_ETH.h>
#include <PN_API_SNMP.h>
#include <PN_API_DEV.h>
#include <PN_API_IM.h>

#include "appRealIdentification.h"
#include "appPermanentData.h"
#include "appUserInfo.h"

/*
 *  Example of how different flavors of I&M carriers can be added to the array.
 *  A real submodule must be plugged prior adding an I&M carrier.
 *  Submodule must be able to be an I&M carrier (consistency with GSD).
 *  By adding a new I&M set, don't forget to adjust APP_IM_CARRIER_NUM.
 *
 *   {.api = 0x00000000, .slotNumber = 0x0001, .subslotNumber = 0x0001,
 *   .supportedIm = PN_API_IM_eSUPPORT_IM1 | PN_API_IM_eSUPPORT_IM2 | PN_API_IM_eSUPPORT_IM3,
 *   .deviceRepresentative = false, .moduleRepresentative = true, .memoryIndex = 1},
 *
 *   {.api = 0x12344567, .slotNumber = 0x00aa, .subslotNumber = 0x00fe,
 *   .supportedIm = PN_API_IM_eSUPPORT_IM2,
 *   .deviceRepresentative = false, .moduleRepresentative = false, .memoryIndex = 2},
 */
static const APP_SImCarrierDescriptor_t imCarrierDescriptors_s[APP_IM_CARRIER_NUM] =
{
    {.api = 0x00000000, .slotNumber = 0x0000, .subslotNumber = 0x0001,
    .supportedIm = PN_API_IM_eSUPPORT_IM1 | PN_API_IM_eSUPPORT_IM2 | PN_API_IM_eSUPPORT_IM3 | PN_API_IM_eSUPPORT_IM4,
    .deviceRepresentative = true, .moduleRepresentative = true, .memoryIndex = 0}
};

static void APP_createImDataCarrier(
    PN_API_IM_SCarrier_t *pImCarrier_p,
    uint8_t carrierIndex_p
);

static const APP_SImCarrierDescriptor_t *APP_searchImCarrierByAddress(
    uint32_t api_p,
    uint16_t slotNum_p,
    uint16_t subSlotNum_p
);

static void APP_readIMField (
    uint8_t memoryIndex_p,
    PN_API_IM_EFieldType_t imFieldType_p,
    uint8_t *pData_p
);

static void APP_writeIMField (
    uint8_t memoryIndex_p,
    PN_API_IM_EFieldType_t imFieldType_p,
    uint8_t *pData_p
);

 /*!
 *  <!-- Description: -->
 * \brief
 * Configure modules and submodules of the PROFINET device.
 *
 * \details
 * Function configures and plugs equipment: modules and submodules.
 * First a module must be plugged into a slot. If a slot is equipped with a module, 
 * submodule can be plugged to a subslot and attached to the aforementioned slot.
 *
 * In this example 19 slots are being equipped with modules: from 0 to 18.
 * Module in slot 0 has 4 submodules, the rest of the modules - 1 submodule each.
 * 
 * \remarks
 * It is not possible to change attributes of a module / submodule, after it was plugged.
 * 
 * <!-- Parameters and return values: -->
 *
 *  \return     Error code of uint32_t type. 0 on success, else failure.
 *
 */
uint32_t APP_configureSubmodules(
        void)
{
    uint32_t result = PN_API_DEV_eOK;

    PN_API_DEV_SModuleDescriptor_t module = { 0 };
    PN_API_DEV_SSubmoduleDescriptor_t submodule = { 0 };

    PN_API_DEV_SModuleSubstitute_t aDefaultModuleSubstitutes[] = { {0x00000002}, {0x00000003}, {0x00000004}, {0x00000005}, {0x00000006},
                                                                   {0x00000007}, {0x00000008}, {0x00000020}, {0x00000030}, {0x00000040},
                                                                   {0x00000050}, {0x00000060}, {0x00000070}, {0x00000080}, {0x00000200},
                                                                   {0x00000300}, {0x00000400}, {0x00000500}, {0x00000600}, {0x00000700},
                                                                   {0x00000800} };

    PN_API_DEV_SModuleSubstitute_t aDAPModuleSubstitutes[] = { {0xd0000000} };

    PN_API_DEV_SSubmoduleSubstitute_t aDefaultSubmoduleSubstitutes[] = {{0x00000001}, {0x00000002}, {0x00000003}, {0x00000004}, {0x00000005},
                                                                        {0x00000006}, {0x00000007}, {0x00000008}, {0x00000009}, {0x0000000a},
                                                                        {0x0000000b}, {0x0000000c}, {0x0000000d}, {0x0000000e}, {0x0000000f}};

    PN_API_DEV_SModuleSubstituteEntry_t *pDAPModuleSubstitutesAddress = NULL;
    PN_API_DEV_SModuleSubstituteEntry_t *pDefaultModuleSubstitutesAddress = NULL;

    PN_API_DEV_SSubmoduleSubstituteEntry_t *pDefaultSubmoduleSubstituteAddress = NULL;

    uint16_t inputDataLength = 0;
    uint16_t inputDataOffset = 0;
    uint16_t outputDataLength = 0;
    uint16_t outputDataOffset = 0;

    //------------------------------------------
    //              Module Substitutes
    //                    DAP
    //------------------------------------------
    result = PN_API_DEV_registerModuleSubstitutes(aDAPModuleSubstitutes, sizeof(aDAPModuleSubstitutes) / sizeof((aDAPModuleSubstitutes)[0]), &pDAPModuleSubstitutesAddress);
    if (result != PN_API_DEV_eOK)
    {
        goto laError;
    }

    //------------------------------------------
    //              Module Substitutes
    //                 Default
    //------------------------------------------
    result = PN_API_DEV_registerModuleSubstitutes(aDefaultModuleSubstitutes, sizeof(aDefaultModuleSubstitutes) / sizeof((aDefaultModuleSubstitutes)[0]), &pDefaultModuleSubstitutesAddress);
    if (result != PN_API_DEV_eOK)
    {
        goto laError;
    }

    //------------------------------------------
    //              Submodule Substitutes
    //                     Default
    //------------------------------------------
    result = PN_API_DEV_registerSubmoduleSubstitutes(aDefaultSubmoduleSubstitutes, sizeof(aDefaultSubmoduleSubstitutes) / sizeof((aDefaultSubmoduleSubstitutes)[0]), &pDefaultSubmoduleSubstituteAddress);
    if (result != PN_API_DEV_eOK)
    {
        goto laError;
    }

    //------------------------------------------
    //                   Slot 0
    //                     DAP
    //               ID 0x80050000
    //------------------------------------------
    memset(&module, 0, sizeof(PN_API_DEV_SModuleDescriptor_t));

    module.api = 0;
    module.id = 0x80050000;
    module.type = PN_API_DEV_eEQU_MT_DAP;
    module.pSubstitutes = pDAPModuleSubstitutesAddress;

    result = PN_API_DEV_plugModule(0, &module);
    if (result != PN_API_DEV_eOK)
    {
        goto laError;
    }

    //------------------------------------------
    //                   Slot 0
    //               Subslot 0x0001
    //------------------------------------------

    //calculate offset for the next submodule depending on the data length of the previous one
    //Note: in this example there are no gaps, but it is allowed to have gaps
    inputDataOffset += inputDataLength;
    outputDataOffset += outputDataLength;
    inputDataLength = 0;
    outputDataLength = 0;

    memset(&submodule, 0, sizeof(PN_API_DEV_SSubmoduleDescriptor_t));

    submodule.api = 0;
    submodule.id = 0x000000001;
    submodule.type = PN_API_DEV_eEQU_ST_INPUT;
    submodule.inputLength = inputDataLength;
    submodule.inputOffsetPii = inputDataOffset;
    submodule.outputLength = outputDataLength;
    submodule.outputOffsetPio = outputDataOffset;
    submodule.pSubstitutes = pDefaultSubmoduleSubstituteAddress;

    result = PN_API_DEV_plugSubmodule(0, 0x0001, &submodule);
    if (result != PN_API_DEV_eOK)
    {
        goto laError;
    }

    //------------------------------------------
    //                   Slot 0
    //               Subslot 0x8000
    //------------------------------------------
    inputDataOffset += inputDataLength;
    outputDataOffset += outputDataLength;
    inputDataLength = 0;
    outputDataLength = 0;

    memset(&submodule, 0, sizeof(PN_API_DEV_SSubmoduleDescriptor_t));

    submodule.api = 0;
    submodule.id = 0x00000000a;
    submodule.type = PN_API_DEV_eEQU_ST_INPUT;
    submodule.inputLength = inputDataLength;
    submodule.inputOffsetPii = inputDataOffset;
    submodule.outputLength = outputDataLength;
    submodule.outputOffsetPio = outputDataOffset;
    submodule.pSubstitutes = pDefaultSubmoduleSubstituteAddress;

    result = PN_API_DEV_plugSubmodule(0, 0x8000, &submodule);
    if (result != PN_API_DEV_eOK)
    {
        goto laError;
    }

    //------------------------------------------
    //                   Slot 0
    //               Subslot 0x8001
    //------------------------------------------
    inputDataOffset += inputDataLength;
    outputDataOffset += outputDataLength;
    inputDataLength = 0;
    outputDataLength = 0;

    memset(&submodule, 0, sizeof(PN_API_DEV_SSubmoduleDescriptor_t));

    submodule.api = 0;
    submodule.id = 0x00000000b;
    submodule.type = PN_API_DEV_eEQU_ST_INPUT;
    submodule.inputLength = inputDataLength;
    submodule.inputOffsetPii = inputDataOffset;
    submodule.outputLength = outputDataLength;
    submodule.outputOffsetPio = outputDataOffset;
    submodule.pSubstitutes = pDefaultSubmoduleSubstituteAddress;

    result = PN_API_DEV_plugSubmodule(0, 0x8001, &submodule);
    if (result != PN_API_DEV_eOK)
    {
        goto laError;
    }

    //------------------------------------------
    //                   Slot 0
    //               Subslot 0x8002
    //------------------------------------------
    inputDataOffset += inputDataLength;
    outputDataOffset += outputDataLength;
    inputDataLength = 0;
    outputDataLength = 0;

    memset(&submodule, 0, sizeof(PN_API_DEV_SSubmoduleDescriptor_t));

    submodule.api = 0;
    submodule.id = 0x00000000c;
    submodule.type = PN_API_DEV_eEQU_ST_INPUT;
    submodule.inputLength = inputDataLength;
    submodule.inputOffsetPii = inputDataOffset;
    submodule.outputLength = outputDataLength;
    submodule.outputOffsetPio = outputDataOffset;
    submodule.pSubstitutes = pDefaultSubmoduleSubstituteAddress;

    result = PN_API_DEV_plugSubmodule(0, 0x8002, &submodule);
    if (result != PN_API_DEV_eOK)
    {
        goto laError;
    }

    //------------------------------------------
    //                   Slot 1
    //               ID 0x00000005
    //----------------------------------------
    memset(&module, 0, sizeof(PN_API_DEV_SModuleDescriptor_t));
    module.api = 0;
    module.id = 0x00000005;
    module.type = PN_API_DEV_eEQU_MT_IO;
    module.pSubstitutes = pDefaultModuleSubstitutesAddress;

    result = PN_API_DEV_plugModule(1, &module);
    if (result != PN_API_DEV_eOK)
    {
        goto laError;
    }

    //------------------------------------------
    //                   Slot 1
    //               Subslot 0x0001
    //------------------------------------------
    inputDataOffset += inputDataLength;
    outputDataOffset += outputDataLength;
    inputDataLength = 16;
    outputDataLength = 0;

    memset(&submodule, 0, sizeof(PN_API_DEV_SSubmoduleDescriptor_t));

    submodule.api = 0;
    submodule.id = 0x000000001;
    submodule.type = PN_API_DEV_eEQU_ST_INPUT;
    submodule.inputLength = inputDataLength;
    submodule.inputOffsetPii = inputDataOffset;
    submodule.outputLength = 0;
    submodule.outputOffsetPio = 0;
    submodule.pSubstitutes = pDefaultSubmoduleSubstituteAddress;

    result = PN_API_DEV_plugSubmodule(1, 0x0001, &submodule);
    if (result != PN_API_DEV_eOK)
    {
        goto laError;
    }

    //------------------------------------------
    //                   Slot 2
    //               ID 0x00000005
    //----------------------------------------
    memset(&module, 0, sizeof(PN_API_DEV_SModuleDescriptor_t));
    module.api = 0;
    module.id = 0x00000005;
    module.type = PN_API_DEV_eEQU_MT_IO;
    module.pSubstitutes = pDefaultModuleSubstitutesAddress;

    result = PN_API_DEV_plugModule(2, &module);
    if (result != PN_API_DEV_eOK)
    {
        goto laError;
    }

    //------------------------------------------
    //                   Slot 2
    //               Subslot 0x0001
    //------------------------------------------
    inputDataOffset += inputDataLength;
    outputDataOffset += outputDataLength;
    inputDataLength = 16;
    outputDataLength = 0;

    memset(&submodule, 0, sizeof(PN_API_DEV_SSubmoduleDescriptor_t));

    submodule.api = 0;
    submodule.id = 0x000000001;
    submodule.type = PN_API_DEV_eEQU_ST_INPUT;
    submodule.inputLength = inputDataLength;
    submodule.inputOffsetPii = inputDataOffset;
    submodule.outputLength = 0;
    submodule.outputOffsetPio = 0;
    submodule.pSubstitutes = pDefaultSubmoduleSubstituteAddress;

    result = PN_API_DEV_plugSubmodule(2, 0x0001, &submodule);
    if (result != PN_API_DEV_eOK)
    {
        goto laError;
    }

    //------------------------------------------
    //                   Slot 3
    //               ID 0x00000006
    //----------------------------------------
    memset(&module, 0, sizeof(PN_API_DEV_SModuleDescriptor_t));
    module.api = 0;
    module.id = 0x00000006;
    module.type = PN_API_DEV_eEQU_MT_IO;
    module.pSubstitutes = pDefaultModuleSubstitutesAddress;

    result = PN_API_DEV_plugModule(3, &module);
    if (result != PN_API_DEV_eOK)
    {
        goto laError;
    }

    //------------------------------------------
    //                   Slot 3
    //               Subslot 0x0001
    //------------------------------------------
    inputDataOffset += inputDataLength;
    outputDataOffset += outputDataLength;
    inputDataLength = 32;
    outputDataLength = 0;

    memset(&submodule, 0, sizeof(PN_API_DEV_SSubmoduleDescriptor_t));

    submodule.api = 0;
    submodule.id = 0x000000001;
    submodule.type = PN_API_DEV_eEQU_ST_INPUT;
    submodule.inputLength = inputDataLength;
    submodule.inputOffsetPii = inputDataOffset;
    submodule.outputLength = 0;
    submodule.outputOffsetPio = 0;
    submodule.pSubstitutes = pDefaultSubmoduleSubstituteAddress;

    result = PN_API_DEV_plugSubmodule(3, 0x0001, &submodule);
    if (result != PN_API_DEV_eOK)
    {
        goto laError;
    }

    //------------------------------------------
    //                   Slot 4
    //               ID 0x00000006
    //----------------------------------------
    memset(&module, 0, sizeof(PN_API_DEV_SModuleDescriptor_t));
    module.api = 0;
    module.id = 0x00000006;
    module.type = PN_API_DEV_eEQU_MT_IO;
    module.pSubstitutes = pDefaultModuleSubstitutesAddress;

    result = PN_API_DEV_plugModule(4, &module);
    if (result != PN_API_DEV_eOK)
    {
        goto laError;
    }

    //------------------------------------------
    //                   Slot 4
    //               Subslot 0x0001
    //------------------------------------------
    inputDataOffset += inputDataLength;
    outputDataOffset += outputDataLength;
    inputDataLength = 32;
    outputDataLength = 0;

    memset(&submodule, 0, sizeof(PN_API_DEV_SSubmoduleDescriptor_t));

    submodule.api = 0;
    submodule.id = 0x000000001;
    submodule.type = PN_API_DEV_eEQU_ST_INPUT;
    submodule.inputLength = inputDataLength;
    submodule.inputOffsetPii = inputDataOffset;
    submodule.outputLength = 0;
    submodule.outputOffsetPio = 0;
    submodule.pSubstitutes = pDefaultSubmoduleSubstituteAddress;

    result = PN_API_DEV_plugSubmodule(4, 0x0001, &submodule);
    if (result != PN_API_DEV_eOK)
    {
        goto laError;
    }

    //------------------------------------------
    //                   Slot 5
    //               ID 0x00000006
    //----------------------------------------
    memset(&module, 0, sizeof(PN_API_DEV_SModuleDescriptor_t));
    module.api = 0;
    module.id = 0x00000006;
    module.type = PN_API_DEV_eEQU_MT_IO;
    module.pSubstitutes = pDefaultModuleSubstitutesAddress;

    result = PN_API_DEV_plugModule(5, &module);
    if (result != PN_API_DEV_eOK)
    {
        goto laError;
    }

    //------------------------------------------
    //                   Slot 5
    //               Subslot 0x0001
    //------------------------------------------
    inputDataOffset += inputDataLength;
    outputDataOffset += outputDataLength;
    inputDataLength = 32;
    outputDataLength = 0;

    memset(&submodule, 0, sizeof(PN_API_DEV_SSubmoduleDescriptor_t));

    submodule.api = 0;
    submodule.id = 0x000000001;
    submodule.type = PN_API_DEV_eEQU_ST_INPUT;
    submodule.inputLength = inputDataLength;
    submodule.inputOffsetPii = inputDataOffset;
    submodule.outputLength = 0;
    submodule.outputOffsetPio = 0;
    submodule.pSubstitutes = pDefaultSubmoduleSubstituteAddress;

    result = PN_API_DEV_plugSubmodule(5, 0x0001, &submodule);
    if (result != PN_API_DEV_eOK)
    {
        goto laError;
    }

    //------------------------------------------
    //                   Slot 6
    //               ID 0x00000007
    //----------------------------------------
    memset(&module, 0, sizeof(PN_API_DEV_SModuleDescriptor_t));
    module.api = 0;
    module.id = 0x00000007;
    module.type = PN_API_DEV_eEQU_MT_IO;
    module.pSubstitutes = pDefaultModuleSubstitutesAddress;

    result = PN_API_DEV_plugModule(6, &module);
    if (result != PN_API_DEV_eOK)
    {
        goto laError;
    }

    //------------------------------------------
    //                   Slot 6
    //               Subslot 0x0001
    //------------------------------------------
    inputDataOffset += inputDataLength;
    outputDataOffset += outputDataLength;
    inputDataLength = 64;
    outputDataLength = 0;

    memset(&submodule, 0, sizeof(PN_API_DEV_SSubmoduleDescriptor_t));

    submodule.api = 0;
    submodule.id = 0x000000001;
    submodule.type = PN_API_DEV_eEQU_ST_INPUT;
    submodule.inputLength = inputDataLength;
    submodule.inputOffsetPii = inputDataOffset;
    submodule.outputLength = 0;
    submodule.outputOffsetPio = 0;
    submodule.pSubstitutes = pDefaultSubmoduleSubstituteAddress;

    result = PN_API_DEV_plugSubmodule(6, 0x0001, &submodule);
    if (result != PN_API_DEV_eOK)
    {
        goto laError;
    }

    //------------------------------------------
    //                   Slot 7
    //               ID 0x00000007
    //----------------------------------------
    memset(&module, 0, sizeof(PN_API_DEV_SModuleDescriptor_t));
    module.api = 0;
    module.id = 0x00000007;
    module.type = PN_API_DEV_eEQU_MT_IO;
    module.pSubstitutes = pDefaultModuleSubstitutesAddress;

    result = PN_API_DEV_plugModule(7, &module);
    if (result != PN_API_DEV_eOK)
    {
        goto laError;
    }

    //------------------------------------------
    //                   Slot 7
    //               Subslot 0x0001
    //------------------------------------------
    inputDataOffset += inputDataLength;
    outputDataOffset += outputDataLength;
    inputDataLength = 64;
    outputDataLength = 0;

    memset(&submodule, 0, sizeof(PN_API_DEV_SSubmoduleDescriptor_t));

    submodule.api = 0;
    submodule.id = 0x000000001;
    submodule.type = PN_API_DEV_eEQU_ST_INPUT;
    submodule.inputLength = inputDataLength;
    submodule.inputOffsetPii = inputDataOffset;
    submodule.outputLength = 0;
    submodule.outputOffsetPio = 0;
    submodule.pSubstitutes = pDefaultSubmoduleSubstituteAddress;

    result = PN_API_DEV_plugSubmodule(7, 0x0001, &submodule);
    if (result != PN_API_DEV_eOK)
    {
        goto laError;
    }

    //------------------------------------------
    //                   Slot 8
    //               ID 0x00000007
    //------------------------------------------
    memset(&module, 0, sizeof(PN_API_DEV_SModuleDescriptor_t));
    module.api = 0;
    module.id = 0x00000007;
    module.type = PN_API_DEV_eEQU_MT_IO;
    module.pSubstitutes = pDefaultModuleSubstitutesAddress;

    result = PN_API_DEV_plugModule(8, &module);
    if (result != PN_API_DEV_eOK)
    {
        goto laError;
    }

    //------------------------------------------
    //                   Slot 8
    //               Subslot 0x0001
    //------------------------------------------
    inputDataOffset += inputDataLength;
    outputDataOffset += outputDataLength;
    inputDataLength = 64;
    outputDataLength = 0;

    memset(&submodule, 0, sizeof(PN_API_DEV_SSubmoduleDescriptor_t));

    submodule.api = 0;
    submodule.id = 0x000000001;
    submodule.type = PN_API_DEV_eEQU_ST_INPUT;
    submodule.inputLength = inputDataLength;
    submodule.inputOffsetPii = inputDataOffset;
    submodule.outputLength = 0;
    submodule.outputOffsetPio = 0;
    submodule.pSubstitutes = pDefaultSubmoduleSubstituteAddress;

    result = PN_API_DEV_plugSubmodule(8, 0x0001, &submodule);
    if (result != PN_API_DEV_eOK)
    {
        goto laError;
    }

    //------------------------------------------
    //                   Slot 9
    //               ID 0x00000007
    //------------------------------------------
    memset(&module, 0, sizeof(PN_API_DEV_SModuleDescriptor_t));
    module.api = 0;
    module.id = 0x00000007;
    module.type = PN_API_DEV_eEQU_MT_IO;
    module.pSubstitutes = pDefaultModuleSubstitutesAddress;

    result = PN_API_DEV_plugModule(9, &module);
    if (result != PN_API_DEV_eOK)
    {
        goto laError;
    }

    //------------------------------------------
    //                   Slot 9
    //               Subslot 0x0001
    //------------------------------------------
    inputDataOffset += inputDataLength;
    outputDataOffset += outputDataLength;
    inputDataLength = 64;
    outputDataLength = 0;

    memset(&submodule, 0, sizeof(PN_API_DEV_SSubmoduleDescriptor_t));

    submodule.api = 0;
    submodule.id = 0x000000001;
    submodule.type = PN_API_DEV_eEQU_ST_INPUT;
    submodule.inputLength = inputDataLength;
    submodule.inputOffsetPii = inputDataOffset;
    submodule.outputLength = 0;
    submodule.outputOffsetPio = 0;
    submodule.pSubstitutes = pDefaultSubmoduleSubstituteAddress;

    result = PN_API_DEV_plugSubmodule(9, 0x0001, &submodule);
    if (result != PN_API_DEV_eOK)
    {
        goto laError;
    }

    //------------------------------------------
    //                   Slot 10
    //               ID 0x00000050
    //------------------------------------------
    memset(&module, 0, sizeof(PN_API_DEV_SModuleDescriptor_t));
    module.api = 0;
    module.id = 0x00000050;
    module.type = PN_API_DEV_eEQU_MT_IO;
    module.pSubstitutes = pDefaultModuleSubstitutesAddress;

    result = PN_API_DEV_plugModule(10, &module);
    if (result != PN_API_DEV_eOK)
    {
        goto laError;
    }

    //------------------------------------------
    //                   Slot 10
    //               Subslot 0x0001
    //------------------------------------------
    inputDataOffset += inputDataLength;
    outputDataOffset += outputDataLength;
    inputDataLength = 0;
    outputDataLength = 16;

    memset(&submodule, 0, sizeof(PN_API_DEV_SSubmoduleDescriptor_t));

    submodule.api = 0;
    submodule.id = 0x000000001;
    submodule.type = PN_API_DEV_eEQU_ST_OUTPUT;
    submodule.inputLength = 0;
    submodule.inputOffsetPii = 0;
    submodule.outputLength = outputDataLength;
    submodule.outputOffsetPio = outputDataOffset;
    submodule.pSubstitutes = pDefaultSubmoduleSubstituteAddress;

    result = PN_API_DEV_plugSubmodule(10, 0x0001, &submodule);
    if (result != PN_API_DEV_eOK)
    {
        goto laError;
    }

    //------------------------------------------
    //                   Slot 11
    //               ID 0x00000050
    //------------------------------------------
    memset(&module, 0, sizeof(PN_API_DEV_SModuleDescriptor_t));
    module.api = 0;
    module.id = 0x00000050;
    module.type = PN_API_DEV_eEQU_MT_IO;
    module.pSubstitutes = pDefaultModuleSubstitutesAddress;

    result = PN_API_DEV_plugModule(11, &module);
    if (result != PN_API_DEV_eOK)
    {
        goto laError;
    }

    //------------------------------------------
    //                   Slot 11
    //               Subslot 0x0001
    //------------------------------------------
    inputDataOffset += inputDataLength;
    outputDataOffset += outputDataLength;
    inputDataLength = 0;
    outputDataLength = 16;

    memset(&submodule, 0, sizeof(PN_API_DEV_SSubmoduleDescriptor_t));

    submodule.api = 0;
    submodule.id = 0x000000001;
    submodule.type = PN_API_DEV_eEQU_ST_OUTPUT;
    submodule.inputLength = 0;
    submodule.inputOffsetPii = 0;
    submodule.outputLength = outputDataLength;
    submodule.outputOffsetPio = outputDataOffset;
    submodule.pSubstitutes = pDefaultSubmoduleSubstituteAddress;

    result = PN_API_DEV_plugSubmodule(11, 0x0001, &submodule);
    if (result != PN_API_DEV_eOK)
    {
        goto laError;
    }

    //------------------------------------------
    //                   Slot 12
    //               ID 0x00000060
    //------------------------------------------
    memset(&module, 0, sizeof(PN_API_DEV_SModuleDescriptor_t));
    module.api = 0;
    module.id = 0x00000060;
    module.type = PN_API_DEV_eEQU_MT_IO;
    module.pSubstitutes = pDefaultModuleSubstitutesAddress;

    result = PN_API_DEV_plugModule(12, &module);
    if (result != PN_API_DEV_eOK)
    {
        goto laError;
    }

    //------------------------------------------
    //                   Slot 12
    //               Subslot 0x0001
    //------------------------------------------
    inputDataOffset += inputDataLength;
    outputDataOffset += outputDataLength;
    inputDataLength = 0;
    outputDataLength = 32;

    memset(&submodule, 0, sizeof(PN_API_DEV_SSubmoduleDescriptor_t));

    submodule.api = 0;
    submodule.id = 0x000000001;
    submodule.type = PN_API_DEV_eEQU_ST_OUTPUT;
    submodule.inputLength = 0;
    submodule.inputOffsetPii = 0;
    submodule.outputLength = outputDataLength;
    submodule.outputOffsetPio = outputDataOffset;
    submodule.pSubstitutes = pDefaultSubmoduleSubstituteAddress;

    result = PN_API_DEV_plugSubmodule(12, 0x0001, &submodule);
    if (result != PN_API_DEV_eOK)
    {
        goto laError;
    }

    //------------------------------------------
    //                   Slot 13
    //               ID 0x00000060
    //------------------------------------------
    memset(&module, 0, sizeof(PN_API_DEV_SModuleDescriptor_t));
    module.api = 0;
    module.id = 0x00000060;
    module.type = PN_API_DEV_eEQU_MT_IO;
    module.pSubstitutes = pDefaultModuleSubstitutesAddress;

    result = PN_API_DEV_plugModule(13, &module);
    if (result != PN_API_DEV_eOK)
    {
        goto laError;
    }

    //------------------------------------------
    //                   Slot 13
    //               Subslot 0x0001
    //------------------------------------------
    inputDataOffset += inputDataLength;
    outputDataOffset += outputDataLength;
    inputDataLength = 0;
    outputDataLength = 32;

    memset(&submodule, 0, sizeof(PN_API_DEV_SSubmoduleDescriptor_t));

    submodule.api = 0;
    submodule.id = 0x000000001;
    submodule.type = PN_API_DEV_eEQU_ST_OUTPUT;
    submodule.inputLength = 0;
    submodule.inputOffsetPii = 0;
    submodule.outputLength = outputDataLength;
    submodule.outputOffsetPio = outputDataOffset;
    submodule.pSubstitutes = pDefaultSubmoduleSubstituteAddress;

    result = PN_API_DEV_plugSubmodule(13, 0x0001, &submodule);
    if (result != PN_API_DEV_eOK)
    {
        goto laError;
    }

    //------------------------------------------
    //                   Slot 14
    //               ID 0x00000060
    //------------------------------------------
    memset(&module, 0, sizeof(PN_API_DEV_SModuleDescriptor_t));
    module.api = 0;
    module.id = 0x00000060;
    module.type = PN_API_DEV_eEQU_MT_IO;
    module.pSubstitutes = pDefaultModuleSubstitutesAddress;

    result = PN_API_DEV_plugModule(14, &module);
    if (result != PN_API_DEV_eOK)
    {
        goto laError;
    }

    //------------------------------------------
    //                   Slot 14
    //               Subslot 0x0001
    //------------------------------------------
    inputDataOffset += inputDataLength;
    outputDataOffset += outputDataLength;
    inputDataLength = 0;
    outputDataLength = 32;

    memset(&submodule, 0, sizeof(PN_API_DEV_SSubmoduleDescriptor_t));

    submodule.api = 0;
    submodule.id = 0x000000001;
    submodule.type = PN_API_DEV_eEQU_ST_OUTPUT;
    submodule.inputLength = 0;
    submodule.inputOffsetPii = 0;
    submodule.outputLength = outputDataLength;
    submodule.outputOffsetPio = outputDataOffset;
    submodule.pSubstitutes = pDefaultSubmoduleSubstituteAddress;

    result = PN_API_DEV_plugSubmodule(14, 0x0001, &submodule);
    if (result != PN_API_DEV_eOK)
    {
        goto laError;
    }

    //------------------------------------------
    //                   Slot 15
    //               ID 0x00000070
    //------------------------------------------
    memset(&module, 0, sizeof(PN_API_DEV_SModuleDescriptor_t));
    module.api = 0;
    module.id = 0x00000070;
    module.type = PN_API_DEV_eEQU_MT_IO;
    module.pSubstitutes = pDefaultModuleSubstitutesAddress;

    result = PN_API_DEV_plugModule(15, &module);
    if (result != PN_API_DEV_eOK)
    {
        goto laError;
    }

    //------------------------------------------
    //                   Slot 15
    //               Subslot 0x0001
    //------------------------------------------
    inputDataOffset += inputDataLength;
    outputDataOffset += outputDataLength;
    inputDataLength = 0;
    outputDataLength = 64;

    memset(&submodule, 0, sizeof(PN_API_DEV_SSubmoduleDescriptor_t));

    submodule.api = 0;
    submodule.id = 0x000000001;
    submodule.type = PN_API_DEV_eEQU_ST_OUTPUT;
    submodule.inputLength = 0;
    submodule.inputOffsetPii = 0;
    submodule.outputLength = outputDataLength;
    submodule.outputOffsetPio = outputDataOffset;
    submodule.pSubstitutes = pDefaultSubmoduleSubstituteAddress;

    result = PN_API_DEV_plugSubmodule(15, 0x0001, &submodule);
    if (result != PN_API_DEV_eOK)
    {
        goto laError;
    }

    //------------------------------------------
    //                   Slot 16
    //               ID 0x00000070
    //------------------------------------------
    memset(&module, 0, sizeof(PN_API_DEV_SModuleDescriptor_t));
    module.api = 0;
    module.id = 0x00000070;
    module.type = PN_API_DEV_eEQU_MT_IO;
    module.pSubstitutes = pDefaultModuleSubstitutesAddress;

    result = PN_API_DEV_plugModule(16, &module);
    if (result != PN_API_DEV_eOK)
    {
        goto laError;
    }

    //------------------------------------------
    //                   Slot 16
    //               Subslot 0x0001
    //------------------------------------------
    inputDataOffset += inputDataLength;
    outputDataOffset += outputDataLength;
    inputDataLength = 0;
    outputDataLength = 64;

    memset(&submodule, 0, sizeof(PN_API_DEV_SSubmoduleDescriptor_t));

    submodule.api = 0;
    submodule.id = 0x000000001;
    submodule.type = PN_API_DEV_eEQU_ST_OUTPUT;
    submodule.inputLength = 0;
    submodule.inputOffsetPii = 0;
    submodule.outputLength = outputDataLength;
    submodule.outputOffsetPio = outputDataOffset;
    submodule.pSubstitutes = pDefaultSubmoduleSubstituteAddress;

    result = PN_API_DEV_plugSubmodule(16, 0x0001, &submodule);
    if (result != PN_API_DEV_eOK)
    {
        goto laError;
    }

    //------------------------------------------
    //                   Slot 17
    //               ID 0x00000070
    //------------------------------------------
    memset(&module, 0, sizeof(PN_API_DEV_SModuleDescriptor_t));
    module.api = 0;
    module.id = 0x00000070;
    module.type = PN_API_DEV_eEQU_MT_IO;
    module.pSubstitutes = pDefaultModuleSubstitutesAddress;

    result = PN_API_DEV_plugModule(17, &module);
    if (result != PN_API_DEV_eOK)
    {
        goto laError;
    }

    //------------------------------------------
    //                   Slot 17
    //               Subslot 0x0001
    //------------------------------------------
    inputDataOffset += inputDataLength;
    outputDataOffset += outputDataLength;
    inputDataLength = 0;
    outputDataLength = 64;

    memset(&submodule, 0, sizeof(PN_API_DEV_SSubmoduleDescriptor_t));

    submodule.api = 0;
    submodule.id = 0x000000001;
    submodule.type = PN_API_DEV_eEQU_ST_OUTPUT;
    submodule.inputLength = 0;
    submodule.inputOffsetPii = 0;
    submodule.outputLength = outputDataLength;
    submodule.outputOffsetPio = outputDataOffset;
    submodule.pSubstitutes = pDefaultSubmoduleSubstituteAddress;

    result = PN_API_DEV_plugSubmodule(17, 0x0001, &submodule);
    if (result != PN_API_DEV_eOK)
    {
        goto laError;
    }

    //------------------------------------------
    //                   Slot 18
    //               ID 0x00000070
    //------------------------------------------
    memset(&module, 0, sizeof(PN_API_DEV_SModuleDescriptor_t));
    module.api = 0;
    module.id = 0x00000070;
    module.type = PN_API_DEV_eEQU_MT_IO;
    module.pSubstitutes = pDefaultModuleSubstitutesAddress;

    result = PN_API_DEV_plugModule(18, &module);
    if (result != PN_API_DEV_eOK)
    {
        goto laError;
    }

    //------------------------------------------
    //                   Slot 18
    //               Subslot 0x0001
    //------------------------------------------
    inputDataOffset += inputDataLength;
    outputDataOffset += outputDataLength;
    inputDataLength = 0;
    outputDataLength = 64;

    memset(&submodule, 0, sizeof(PN_API_DEV_SSubmoduleDescriptor_t));

    submodule.api = 0;
    submodule.id = 0x000000001;
    submodule.type = PN_API_DEV_eEQU_ST_OUTPUT;
    submodule.inputLength = 0;
    submodule.inputOffsetPii = 0;
    submodule.outputLength = outputDataLength;
    submodule.outputOffsetPio = outputDataOffset;
    submodule.pSubstitutes = pDefaultSubmoduleSubstituteAddress;

    result = PN_API_DEV_plugSubmodule(18, 0x0001, &submodule);
    if (result != PN_API_DEV_eOK)
    {
        goto laError;
    }

    return result;

laError:
    OSAL_printf("\r[APP] ERROR: Failed to configure submodules; Err: 0x%8.8x", result);
    return result;
}

 /*!
 *  <!-- Description: -->
 * \brief
 * Configure I&M data.
 *
 * \details
 * Function configures and attaches I&M data to submodules -> adds I&M carriers to stack.
 *
 * \remarks
 * According to the PROFINET specification, every PROFINET device must have at least one I&M carrier.
 * This I&M carrier must be I&M device representative.
 *
 * <!-- Parameters and return values: -->
 *
 *  \return     Error code of uint32_t type. 0 on success, else failure.
 *
 */
uint32_t APP_configureImData(
        void)
{
    uint32_t result = PN_API_IM_eOK;
    uint8_t imCarrierCounter = 0;

    for (imCarrierCounter = 0; imCarrierCounter < APP_IM_CARRIER_NUM; imCarrierCounter++)
    {
        PN_API_IM_SCarrier_t imCarrier = { 0 };
        APP_createImDataCarrier(&imCarrier, imCarrierCounter);
        result = PN_API_IM_addImCarrier(&imCarrier);

        if(result != PN_API_IM_eOK)
        {
            break;
        }
    }

    return result;
}

 /*!
 *  <!-- Description: -->
 * \brief
 * Write Record Indication callback.
 *
 * \details
 * As stack has no access to persistent memory, application is responsible for storing writable data.
 * User must register this callback function on stack using PN_API_DEV_registerWriteRecordIndicatedCallback() to be able to store persistent data.
 * Record index specifies kind of record data, which needs to be stored.
 *
 * In this example, the focus is to store I&M writable data fields.
 *
 *
 * <!-- Parameters and return values: -->
 *
 * \return     Error code of uint32_t type. 0 on success, else failure.
 *
 */
uint32_t APP_cbWriteRecordInd (
    uint32_t api_p,
    uint16_t slot_p,
    uint16_t subslot_p,
    uint16_t index_p,
    uint8_t *pData_p,
    uint16_t dataLength_p)
{
    uint32_t result = APP_eRWE_NO_ERROR;

    APP_ERecordIndexType_t indexType = (APP_ERecordIndexType_t)index_p;
    switch (indexType)
    {
        case APP_eREC_INDEX_IM1_DATA:
        {
            const APP_SImCarrierDescriptor_t *imCarrier = APP_searchImCarrierByAddress(api_p, slot_p, subslot_p);
            if (imCarrier != NULL)
            {
                uint8_t im1Supported = (imCarrier->supportedIm & PN_API_IM_eSUPPORT_IM1);
                if (im1Supported != 0)
                {
                    // I&M1 data record is a special case, as it has two I&M fields "Function" and "Location"
                    // Both of them are passed in one buffer.
                    uint8_t *pTagFunctionData = pData_p;
                    APP_writeIMField(imCarrier->memoryIndex, PN_API_IM_eFT_TAG_FUNCTION, pTagFunctionData);

                    uint8_t *pTagLocationData = pData_p + PN_API_IM1_TAG_FUNCTION_LENGTH;
                    APP_writeIMField(imCarrier->memoryIndex, PN_API_IM_eFT_TAG_LOCATION, pTagLocationData);
                }
                else
                {
                    result = APP_eRWE_ACCESS_DENIED;
                }
            }
            else
            {
                result = APP_eRWE_ACCESS_INVALID_SLOT;
            }
        }break;

        case APP_eREC_INDEX_IM2_DATA:
        {
            const APP_SImCarrierDescriptor_t *pImCarrier = APP_searchImCarrierByAddress(api_p, slot_p, subslot_p);
            if (pImCarrier != NULL)
            {
                uint8_t im2Supported = (pImCarrier->supportedIm & PN_API_IM_eSUPPORT_IM2);
                if (im2Supported != 0)
                {
                    uint8_t *pInstallationDate = pData_p;
                    APP_writeIMField(pImCarrier->memoryIndex, PN_API_IM_eFT_INSTALLATION_DATE, pInstallationDate);
                }
                else
                {
                    result = APP_eRWE_ACCESS_DENIED;
                }
            }
            else
            {
                result = APP_eRWE_ACCESS_INVALID_SLOT;
            }
        }break;

        case APP_eREC_INDEX_IM3_DATA:
        {
            const APP_SImCarrierDescriptor_t *pImCarrier = APP_searchImCarrierByAddress(api_p, slot_p, subslot_p);
            if (pImCarrier != NULL)
            {
                uint8_t im3Supported = (pImCarrier->supportedIm & PN_API_IM_eSUPPORT_IM3);
                if (im3Supported != 0)
                {
                    uint8_t *pDescriptor = pData_p;
                    APP_writeIMField(pImCarrier->memoryIndex, PN_API_IM_eFT_DESCRIPTOR, pDescriptor);
                }
                else
                {
                    result = APP_eRWE_ACCESS_DENIED;
                }
            }
            else
            {
                result = APP_eRWE_ACCESS_INVALID_SLOT;
            }
        }break;

        case APP_eREC_INDEX_IM4_DATA:
        {
            const APP_SImCarrierDescriptor_t *pImCarrier = APP_searchImCarrierByAddress(api_p, slot_p, subslot_p);
            if (pImCarrier != NULL)
            {
                uint8_t im4Supported = (pImCarrier->supportedIm & PN_API_IM_eSUPPORT_IM4);
                if (im4Supported != 0)
                {
                    uint8_t *pSignature = pData_p;
                    APP_writeIMField(pImCarrier->memoryIndex, PN_API_IM_eFT_SIGNATURE, pSignature);
                }
                else
                {
                    result = APP_eRWE_ACCESS_DENIED;
                }
            }
            else
            {
                result = APP_eRWE_ACCESS_INVALID_SLOT;
            }
        }break;

        default:
        {
            // Unknown index value
            result = APP_eRWE_ACCESS_INVALID_INDEX;
        }break;
    }

    return result;
}

 /*!
 *  <!-- Description: -->
 * \brief
 * Helper function to prepare an instance of I&M carrier before adding it to PROFINET stack.
 *
 * \details
 * Function fills an instance of I&M carrier structure with required I&M data.
 * I&M0 data is read-only and can be hard-coded.
 * I&M1 - I&M4 data is writable, so it must be stored in non-volatile memory, that can be erased an reprogrammed.
 *
 * In current example a separate structure PN_API_IM_APP_SFields_t is used, to store writable I&M data in flash.
 * In contrast to APP_SImCarrierDescriptor_t, which contains all the I&M information, for persistent storage purpose.
 * PN_API_IM_APP_SFields_t contains address, read-only I&M0 data and memory index.
 * Memory index is used here, to map members of the imCarrierDescriptors_s array to corresponding members of APP_permanentDataWorkingCopy_s array.
 * APP_permanentDataWorkingCopy_s stores only writeable I&M data of the carriers in persistent memory.
 *
 * For demonstration three different I&M carriers will be created. Depending on the type passed to the function:
 *
 * - I&M device representative
 * - I&M module representative
 * - I&M carrier, representing only it's own submodule
 *
 * \warnings
 *
 * I&M0 data of the device representative must be consistent with Chassis ID.
 * 
 * <!-- Parameters and return values: -->
 * 
 * \param[in]  pImCarrier_p           I&M carrier to fill.
 * \param[in]  carrierIndex_p         Carrier position in imCarrierDescriptors_s array.
 * 
 * \return        None
 */
static void APP_createImDataCarrier(
    PN_API_IM_SCarrier_t *pImCarrier_p,
    uint8_t carrierIndex_p
)
{
    uint8_t memoryIndex = 0;
    uint8_t im1Supported = 0;
    uint8_t im2Supported = 0;
    uint8_t im3Supported = 0;
    uint8_t im4Supported = 0;

    memset(pImCarrier_p, 0, sizeof(PN_API_IM_SCarrier_t));

    //
    // Fill the I&M carrier structure according to I&M descriptors
    //
    pImCarrier_p->address.api = imCarrierDescriptors_s[carrierIndex_p].api;
    pImCarrier_p->address.slotNum = imCarrierDescriptors_s[carrierIndex_p].slotNumber;
    pImCarrier_p->address.subslotNum = imCarrierDescriptors_s[carrierIndex_p].subslotNumber;
    pImCarrier_p->representative.device = imCarrierDescriptors_s[carrierIndex_p].deviceRepresentative;
    pImCarrier_p->representative.module = imCarrierDescriptors_s[carrierIndex_p].moduleRepresentative;

    //
    // I&M0 data record must be supported for every I&M carrier
    // Note: For order id and serial number - all unused characters must be set to "blank" (0x20)
    //       I&M device representative must support I&M1, I&M2 and I&M3 data records
    //
    pImCarrier_p->im0.vendorID = PNC_VENDOR_ID;

    memcpy(pImCarrier_p->im0.aOrderID, "100074              ", PN_API_IM0_ORDER_ID_LENGTH);
    APP_UI_getSerialNumberString(pImCarrier_p->im0.aSerialNum, PN_API_IM0_SERIAL_ID_LENGTH);
    pImCarrier_p->im0.hwRevision = APP_UI_getHardwareRevision();
    pImCarrier_p->im0.swRevision.prefix = PNC_DEVICE_SW_REV_PREFIX;
    pImCarrier_p->im0.swRevision.funcEnhancement = APP_UI_getSwRevFuncEnhancement();
    pImCarrier_p->im0.swRevision.bugFix = APP_UI_getSwRevBugFix();
    pImCarrier_p->im0.swRevision.internalChange = APP_UI_getSwRevInternalChange();
    pImCarrier_p->im0.revisionCounter = APP_UI_getDeviceRevCounter();
    pImCarrier_p->im0.profileID = 0xf600;
    pImCarrier_p->im0.profileSpecificType = 0x0000;
    pImCarrier_p->im0.version.major = 0x01;
    pImCarrier_p->im0.version.minor = 0x01;

    pImCarrier_p->im0.imSupported = imCarrierDescriptors_s[carrierIndex_p].supportedIm;

    //
    // Writable data is stored in persistent memory, where memory slots represent I&M carriers.
    // For accessing proper memory slots a memory index will be used here.
    //
    memoryIndex = imCarrierDescriptors_s[carrierIndex_p].memoryIndex;

    //! Memory index shall be equal or less than defined number of I&M carriers
    if (memoryIndex >= APP_IM_CARRIER_NUM)
    {
        return;
    }

    im1Supported = pImCarrier_p->im0.imSupported & PN_API_IM_eSUPPORT_IM1;
    if (im1Supported != 0)
    {
        uint8_t *pTagFunction = NULL;
        uint8_t *pTagLocation = NULL;

        pTagFunction = &pImCarrier_p->im1.aTagFunction[0];
        APP_readIMField(memoryIndex, PN_API_IM_eFT_TAG_FUNCTION, pTagFunction);

        pTagLocation = &pImCarrier_p->im1.aTagLocation[0];
        APP_readIMField(memoryIndex, PN_API_IM_eFT_TAG_LOCATION, pTagLocation);
    }

    im2Supported = pImCarrier_p->im0.imSupported & PN_API_IM_eSUPPORT_IM2;
    if (im2Supported != 0)
    {
        uint8_t *pInstallationDate = &pImCarrier_p->im2.aInstallationDate[0];
        APP_readIMField(memoryIndex, PN_API_IM_eFT_INSTALLATION_DATE, pInstallationDate);
    }

    im3Supported = pImCarrier_p->im0.imSupported & PN_API_IM_eSUPPORT_IM3;
    if (im3Supported != 0)
    {
        uint8_t *pDescriptor = &pImCarrier_p->im3.aDescriptor[0];
        APP_readIMField(memoryIndex, PN_API_IM_eFT_DESCRIPTOR, pDescriptor);
    }

    im4Supported = pImCarrier_p->im0.imSupported & PN_API_IM_eSUPPORT_IM4;
    if (im4Supported != 0)
    {
        uint8_t *pSignature = &pImCarrier_p->im4.aSignature[0];
        APP_readIMField(memoryIndex, PN_API_IM_eFT_SIGNATURE, pSignature);
    }
}

 /*!
 *  <!-- Description: -->
 * \brief
 * Read an I&M field.
 *
 * \details
 * Helper function, to read writable I&M data from a memory slot in persistent data.
 * A memory slot represents an I&M carrier.
 *
 * <!-- Parameters and return values: -->
 *
 *  \param[in]  memoryIndex_p   Index of memory slot, representing an I&M carrier
 *  \param[in]  imFieldType_p   Type of I&M data field
 *  \param[in]  pBuffer_p       Data
 *
 *  \return        None
 *
 */
static void APP_readIMField (
    uint8_t memoryIndex_p,
    PN_API_IM_EFieldType_t imFieldType_p,
    uint8_t *pData_p
)
{
    APP_SPermanentData_t *pPermData = APP_getPermStorage ();

    PN_API_IM_SFields_t *pImFields = &pPermData->aImFields[memoryIndex_p];

    switch (imFieldType_p)
    {
        case PN_API_IM_eFT_TAG_FUNCTION:
        {
            memcpy (pData_p, pImFields->aTagFunction, PN_API_IM1_TAG_FUNCTION_LENGTH);
        }   break;
        case PN_API_IM_eFT_TAG_LOCATION:
        {
            memcpy (pData_p, pImFields->aTagLocation, PN_API_IM1_TAG_LOCATION_LENGTH);
        }   break;
        case PN_API_IM_eFT_INSTALLATION_DATE:
        {
            memcpy (pData_p, pImFields->aInstallationDate, PN_API_IM2_INSTALLATION_DATE_LENGTH);
        }   break;
        case PN_API_IM_eFT_DESCRIPTOR:
        {
            memcpy (pData_p, pImFields->aDescriptor, PN_API_IM3_DESCRIPTOR_LENGTH);
        }   break;
        case PN_API_IM_eFT_SIGNATURE:
        {
            memcpy (pData_p, pImFields->aSignature, PN_API_IM4_SIGNATURE_LENGTH);
        }   break;
        default:
        {
        }   break;
    }
}

 /*!
 *  <!-- Description: -->
 * \brief
 * Write an I&M field.
 *
 * \details
 * Helper function, to store writable I&M data in a memory slot in persistent data.
 * A memory slot represents an I&M carrier.
 *
 * <!-- Parameters and return values: -->
 *
 * \param[in]  memoryIndex_p   Index of memory slot, representing an I&M carrier
 * \param[in]  imFieldType_p   Type of I&M data field
 * \param[in]  pBuffer_p       Data
 *
 * \return        None
 *
 */
static void APP_writeIMField (
    uint8_t memoryIndex_p,
    PN_API_IM_EFieldType_t imFieldType_p,
    uint8_t *pData_p
)
{
    APP_SPermanentData_t *pPermData = APP_getPermStorage ();

    PN_API_IM_SFields_t *pImFields = &pPermData->aImFields[memoryIndex_p];

    switch (imFieldType_p)
    {
        case PN_API_IM_eFT_TAG_FUNCTION:
        {
            memcpy (pImFields->aTagFunction, pData_p, PN_API_IM1_TAG_FUNCTION_LENGTH);
        }   break;
        case PN_API_IM_eFT_TAG_LOCATION:
        {
            memcpy (pImFields->aTagLocation, pData_p, PN_API_IM1_TAG_LOCATION_LENGTH);
        }   break;
        case PN_API_IM_eFT_INSTALLATION_DATE:
        {
            memcpy (pImFields->aInstallationDate, pData_p, PN_API_IM2_INSTALLATION_DATE_LENGTH);
        }   break;
        case PN_API_IM_eFT_DESCRIPTOR:
        {
            memcpy (pImFields->aDescriptor, pData_p, PN_API_IM3_DESCRIPTOR_LENGTH);
        }   break;
        case PN_API_IM_eFT_SIGNATURE:
        {
            memcpy (pImFields->aSignature, pData_p, PN_API_IM4_SIGNATURE_LENGTH);
        }   break;
        default:
        {
        }   break;
    }
}

 /*!
 *  <!-- Description: -->
 * \brief
 * Search for I&M carrier.
 *
 * \details
 * Helper function, to search for a carrier with specified address in the imCarrierDescriptors_s array.
 *
 * <!-- Parameters and return values: -->
 *
 * \param[in]  api_p           API
 * \param[in]  slotNum_p       Slot
 * \param[in]  subSlotNum_p    Subslot
 *
 * \return     APP_SImCarrierDescriptor_t* pointer to I&M carrier with specified address
 *
 */
static const APP_SImCarrierDescriptor_t *APP_searchImCarrierByAddress(
    uint32_t api_p,
    uint16_t slotNum_p,
    uint16_t subSlotNum_p
)
{
    const APP_SImCarrierDescriptor_t *pImCarrier = NULL;
    uint8_t carrierIndex = 0;

    for (carrierIndex = 0; carrierIndex < APP_IM_CARRIER_NUM; carrierIndex++)
    {
        const APP_SImCarrierDescriptor_t *pCurrentDescriptor = &imCarrierDescriptors_s[carrierIndex];
        if ((pCurrentDescriptor->api == api_p) &&
            (pCurrentDescriptor->slotNumber == slotNum_p) &&
            (pCurrentDescriptor->subslotNumber == subSlotNum_p))
        {
            pImCarrier = pCurrentDescriptor;

            break;
        }
    }
    return pImCarrier;
}
