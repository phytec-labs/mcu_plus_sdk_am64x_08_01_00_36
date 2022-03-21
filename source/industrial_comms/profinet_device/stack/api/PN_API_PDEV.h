/*!
 *  \example <PN_API_PDEV>.h
 *
 *  \brief
 *  Profinet API for physical device.
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

#ifndef _PN_API_PDEV_H_
#define _PN_API_PDEV_H_

#ifdef __cplusplus
extern "C" {
#endif

#define PN_API_PDEV_MAX_DEVICE_DESCRIPTION_LENGTH       25
#define PN_API_PDEV_MAX_ORDER_ID_LENGTH                 20
#define PN_API_PDEV_MAX_SERIAL_NUMBER_LENGTH            16
#define PN_API_PDEV_MAX_HW_REVISION_LENGTH               5
#define PN_API_PDEV_MAX_SW_REVISION_SUBSTRING_LENGTH     3

/*!
 *  \brief
 *  Physical Device error codes.
 *  \ingroup PN_API_PDEV_ERROR_CODES
 */
typedef enum PN_API_PDEV_EError
{
    PN_API_PDEV_eOK                                               = 0x00000000,  /*!< error code to be documented*/
    PN_API_PDEV_eERROR_WRONG_STATE                                = 0x38030101,  /*!< error code to be documented*/
    PN_API_PDEV_eERROR_MISSING_PARAMETER_POINTER                  = 0x38030102,  /*!< error code to be documented*/
    PN_API_PDEV_eERROR_EXCEEDING_MAX_DEVICE_DESCRIPTION_LENGTH    = 0x38030103,  /*!< error code to be documented*/
    PN_API_PDEV_eERROR_EXCEEDING_MAX_ORDER_ID_LENGTH              = 0x38030104,  /*!< error code to be documented*/
    PN_API_PDEV_eERROR_EXCEEDING_MAX_SERIAL_NUMBER_LENGTH         = 0x38030105,  /*!< error code to be documented*/
    PN_API_PDEV_eERROR_EXCEEDING_MAX_HW_REVISION_LENGTH           = 0x38030106,  /*!< error code to be documented*/
    PN_API_PDEV_eERROR_EXCEEDING_MAX_SW_REVISION_SUBSTRING_LENGTH = 0x38030107   /*!< error code to be documented*/
} PN_API_PDEV_EError_t;                                                          
                                                                                 
typedef struct PN_API_PDEV_SChassisId
{
    uint16_t deviceDescriptionLength;
    uint8_t *pDeviceDescription;
    uint16_t orderIdLength;
    uint8_t *pOrderId;
    uint16_t serialNumberLength;
    uint8_t *pSerialNumber;
    uint16_t hwRevisionLength;
    uint8_t *pHwRevision;
    uint8_t swRevisionPrefix;
    uint16_t swRevisionFunctionalEnhancementLength;
    uint8_t *pSwRevisionFunctionalEnhancement;
    uint16_t swRevisionBugFixLength;
    uint8_t *pSwRevisionBugFix;
    uint16_t swRevisionInternalChangeLength;
    uint8_t *pSwRevisionInternalChange;
} PN_API_PDEV_SChassisId_t;

typedef struct PN_API_PDEV_SConfiguration
{
    uint16_t vendorId;
    uint16_t deviceId;
    PN_API_PDEV_SChassisId_t chassisId;
} PN_API_PDEV_SConfiguration_t;

extern void PN_API_PDEV_init(void);
extern uint32_t PN_API_PDEV_start(void);
extern uint32_t PN_API_PDEV_run(void);
extern uint32_t PN_API_PDEV_applyConfiguration(const PN_API_PDEV_SConfiguration_t *pConfiguration_p);

#ifdef  __cplusplus 
} 
#endif 

#endif //_PN_API_PDEV_H_
