/*!
 *  \example <PN_API_IM>.h
 *
 *  \brief
 *  Profinet API for I&M data.
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

#ifndef _PN_API_IM_H_
#define _PN_API_IM_H_

#ifdef __cplusplus
extern "C" {
#endif

//+=============================================================================================
//|     Defines
//+=============================================================================================

#define PN_API_IM_MAX_CARRIERS                      5       // Maximum I&M Carriers capacity on stack

#define PN_API_IM_BLANK                             0x20    // 'Blank' value for unused octets of I&M data

#define PN_API_IM0_ORDER_ID_LENGTH                  20      // Length of the Order Id string in I&M 0
#define PN_API_IM0_SERIAL_ID_LENGTH                 16      // Length of the Serial Id string in I&M 0

#define PN_API_IM1_TAG_FUNCTION_LENGTH              32      // Length of the Tag Function string in I&M 1
#define PN_API_IM1_TAG_LOCATION_LENGTH              22      // Length of the Tag Function string in I&M 1
#define PN_API_IM2_INSTALLATION_DATE_LENGTH         16      // Length of the Date string in I&M 2
#define PN_API_IM3_DESCRIPTOR_LENGTH                54      // Length of the Descriptor string in I&M 3
#define PN_API_IM4_SIGNATURE_LENGTH                 54      // Length of the Signature string in I&M 4

//+=============================================================================================
//|     Structures
//+=============================================================================================

/*!
*  \brief
*  I&M data Error Codes.
*  \ingroup PN_API_IM_ERROR_CODES
*/
typedef enum PN_API_IM_EError
{
    PN_API_IM_eOK                                   = 0x00000000, /*!< No Error, I&M services are working as expected.*/

    // Error codes for adding I&M carrier
    PN_API_IM_eERR_ADD_INVALID_POINTER_PARAMETER    = 0x38030601, /*!< Invalid parameters(NULL pointers).*/
    PN_API_IM_eERR_ADD_INVALID_ADDRESS              = 0x38030602, /*!< Invalid slot number(> 0x7FFF) or subslot number(> 0x9FFF).*/
    PN_API_IM_eERR_ADD_NO_FREE_SPACE                = 0x38030603, /*!< No free space for a new I&M carrier.*/
    PN_API_IM_eERR_ADD_INVALID_IM_VERSION           = 0x38030604, /*!< Invalid I&M version(major != 0x01 || minor != 0x01).*/
    PN_API_IM_eERR_ADD_INVALID_SW_PREFIX            = 0x38030605, /*!< Invalid SW Revision prefix(for reference see PN_API_IM_EImSwRevPrefix_t).*/
    PN_API_IM_eERR_ADD_INVALID_STRING               = 0x38030606, /*!< I&M0, I&M1, I&M2 or I&M3 data contains a non - visible character.*/
    PN_API_IM_eERR_ADD_SUBMODULE_ALREADY_ASSIGNED   = 0x38030607, /*!< An I&M carrier is already assigned to submodule with specified address.*/
    PN_API_IM_eERR_ADD_MODULE_ALREADY_ASSIGNED      = 0x38030608, /*!< Specified module has already an I & M module representative.*/
    PN_API_IM_eERR_ADD_DEVICE_ALREADY_ASSIGNED      = 0x38030609, /*!< I&M device representative is already assigned.*/
    PN_API_IM_eERR_ADD_DEVICE_NO_IM123_SUPPORT      = 0x3803060a, /*!< I&M device representative must support I&M1, I&M2 and I&M3 data.*/
    PN_API_IM_eERR_ADD_SUBMODULE_NOT_PLUGGED        = 0x3803060b, /*!< Corresponding real submodule isn't plugged.*/

    // Error codes for removing I&M carrier
    PN_API_IM_eERR_REMOVE_IM_INVALID_POINTER        = 0x38030610, /*!< Invalid input pointers.*/
    PN_API_IM_eERR_REMOVE_IM_CARRIER_NOT_EXIST      = 0x38030611  /*!< Specified I&M carrier does not exist.*/

} PN_API_IM_EError_t;

typedef enum PN_API_IM_EFieldType
{
    PN_API_IM_eFT_TAG_FUNCTION,
    PN_API_IM_eFT_TAG_LOCATION,
    PN_API_IM_eFT_INSTALLATION_DATE,
    PN_API_IM_eFT_DESCRIPTOR,
    PN_API_IM_eFT_SIGNATURE
} PN_API_IM_EFieldType_t;

typedef enum PN_API_IM_ESupported
{
    PN_API_IM_eSUPPORT_SPECIFIC = (1 << 0),
    PN_API_IM_eSUPPORT_IM1      = (1 << 1),
    PN_API_IM_eSUPPORT_IM2      = (1 << 2),
    PN_API_IM_eSUPPORT_IM3      = (1 << 3),
    PN_API_IM_eSUPPORT_IM4      = (1 << 4)
} PN_API_IM_ESupported_t;

typedef enum PN_API_IM_ESwRevPrefix
{
    PN_API_IM_eSW_PREFIX_V = 'V',
    PN_API_IM_eSW_PREFIX_R = 'R',
    PN_API_IM_eSW_PREFIX_P = 'P',
    PN_API_IM_eSW_PREFIX_U = 'U',
    PN_API_IM_eSW_PREFIX_T = 'T'
} PN_API_IM_ESwRevPrefix_t;


typedef struct PN_API_IM_SFields
{
    uint8_t aTagFunction[PN_API_IM1_TAG_FUNCTION_LENGTH];
    uint8_t aTagLocation[PN_API_IM1_TAG_LOCATION_LENGTH];
    uint8_t aInstallationDate[PN_API_IM2_INSTALLATION_DATE_LENGTH];
    uint8_t aDescriptor[PN_API_IM3_DESCRIPTOR_LENGTH];
    uint8_t aSignature[PN_API_IM4_SIGNATURE_LENGTH];
} PN_API_IM_SFields_t;

typedef struct PN_API_IM_SIm4
{
    uint8_t aSignature[PN_API_IM4_SIGNATURE_LENGTH];
    // This optional attribute should only be used together with Functional Safety.  
    // This attribute is an Octet String.

} PN_API_IM_SIm4_t;

typedef struct PN_API_IM_SIm3
{
    uint8_t aDescriptor[PN_API_IM3_DESCRIPTOR_LENGTH];
    // This attribute indicates the date of installation or commissioning of a device or module
    // Unused characters shall be set to 0x20 (blank).

} PN_API_IM_SIm3_t;

typedef struct PN_API_IM_SIm2
{
    uint8_t aInstallationDate[PN_API_IM2_INSTALLATION_DATE_LENGTH];
    // This attribute indicates the date of installation or commissioning of a device or module
    // Shall be coded according to IEC CD 61158-6-10 / 5.2.8.16 Coding of the field IM_Date

} PN_API_IM_SIm2_t;

typedef struct PN_API_IM_SIm1
{
    uint8_t aTagFunction[PN_API_IM1_TAG_FUNCTION_LENGTH];
    // This attribute indicates the submodule’s function or task.
    // Unused characters shall be set to 0x20 (blank).

    uint8_t aTagLocation[PN_API_IM1_TAG_LOCATION_LENGTH];
    // This attribute indicates the submodule’s function or task.
    // Unused characters shall be set to 0x20 (blank).
} PN_API_IM_SIm1_t;

typedef struct PN_API_IM_SRepresentative
{
    bool module;             // This I&M carrier is a representative for the module

    bool device;             // This I&M carrier is a representative for the device

} PN_API_IM_SRepresentative_t;

typedef struct PN_API_IM_SVersion
{
    uint8_t major;             // Major version number of the implemented version of the I&M functions (MSB)
                             // Allowed values: 0x01 shall be set in this version

    uint8_t minor;             // Minor version number of the implemented version of the I&M functions (LSB)
                             // Allowed values: 0x01 shall be set in this version
} PN_API_IM_SVersion_t;

typedef struct PN_API_IM_SSwRevision
{
    uint8_t prefix;             // The prefix attribute forms the "V" part of “Vx.y.z” software revision.
                              // Allowed values:
                              // “V” (= officially released version)
                              // “R”(= Revision)
                              // “P” = Prototype
                              // “U” = Under Test(field test)
                              // “T” = Test Device

    uint8_t funcEnhancement;    // The functional enhancement attribute forms the "x" part of “Vx.y.z” software revision.
                              // Allowed values: 0 to 0xFF

    uint8_t bugFix;             // The functional enhancement attribute forms the "y" part of “Vx.y.z” software revision.
                              // Allowed values: 0 to 0xFF

    uint8_t internalChange;     // The functional enhancement attribute forms the "z" part of “Vx.y.z” software revision.
                              // Allowed values: 0 to 0xFF

} PN_API_IM_SSwRevision_t;

typedef struct PN_API_IM_SIm0
{
    uint16_t vendorID;                         // ID of the submodule’s manufacturer. 0x0000 is used in case a manufacturer does not want to handle an ID.
                                             // Allowed values: 0 to 0xFFFF.

    uint8_t aOrderID[PN_API_IM0_ORDER_ID_LENGTH];                       // Complete order number or at least a relevant part that allows unambiguous identification of the submodule 
                                             // within the manufacturer’s order spectrum. The attribute is filled as visible string. 
                                             // Unused octets shall be set to 0x20 (blank).

    uint8_t aSerialNum[PN_API_IM0_SERIAL_ID_LENGTH];
    // Serial number of the submodule. The attribute is filled as visible string. 
    // Unused octets shall be set to 0x20 (blank).

    uint16_t hwRevision;                       // Hardware revision of the submodule.
                                             // Allowed values: 0 to 0xFFFF.

    PN_API_IM_SSwRevision_t swRevision;   // This attribute defines the software or firmware revision of the submodule. 

    uint16_t revisionCounter;                  // This attribute is not used in the context of this document.
                                             // Allowed values: 0

    uint16_t profileID;                        // Defines the ID of the application profile the submodule is following.
                                             // Allowed values: 
                                             //     0x0000            Non profile device 
                                             //     0x0001 – 0xF6FF   Administrative number 
                                             //     0xF700 – 0xFFFE   Reserved for profiles
                                             //     0xFFFF            Reserved

    uint16_t profileSpecificType;              // In case a submodule follows a special application profile, this attribute offers information 
                                             // about the usage of its channels and /or sub devices.
                                             // The submodule / channel information shall be according to the respective definitions of the application profile.
                                             // Allowed values
                                             //
                                             // In conjunction with profileID == 0x0000
                                             //     0x0000            Unspecified 
                                             //     0x0001            Standard Controller
                                             //     0x0002            PC - based Station
                                             //     0x0003            IO - Module
                                             //     0x0004            Communication Module
                                             //     0x0005            Interface Module
                                             //     0x0006            Active Network Infrastructure Component
                                             //     0x0007            Media attachment unit
                                             //     0x0100 – 0x7FFF   Manufacturer Specific Type
                                             //     0x8000 - 0xFFFF   Reserved
                                             // 
                                             // In conjunction with profileID range 0x0001 - 0xF6FF
                                             //     0x0000            Unspecified, if not defined otherwise by the profile
                                             //     0x0001 – 0x00FF   Reserved
                                             //     0x0100 – 0x7FFF   Shall be defined by the manufacturer
                                             //     0x8000 – 0xFFFF   Using shall be defined by the profile identified by the profileID 

    PN_API_IM_SVersion_t version;     // Implemented version of the I&M functions.


    PN_API_IM_ESupported_t imSupported; // Indicates availability of further I&M records.
                                             // Bit 0 indicates the availability of Profile specific I&M, 
                                             // Bit 1 the availability of I&M1...

} PN_API_IM_SIm0_t;

typedef struct PN_API_IM_SCarrierAddress
{
    uint32_t api;              // number of the application process the submodule is assigned to.
    uint16_t slotNum;          // number of the slot the submodule is assigned to.
    uint16_t subslotNum;       // number of the subslot the submodule is assigned to.
} PN_API_IM_SCarrierAddress_t;

typedef struct PN_API_IM_SCarrier
{
    PN_API_IM_SCarrierAddress_t address;            // address of the I&M carrier (API -> slot number -> subslot number)
    PN_API_IM_SIm0_t im0;                           // I&M0 data
    PN_API_IM_SIm1_t im1;                           // I&M1 data
    PN_API_IM_SIm2_t im2;                           // I&M1 data
    PN_API_IM_SIm3_t im3;                           // I&M3 data
    PN_API_IM_SIm4_t im4;                           // I&M4 data    
    PN_API_IM_SRepresentative_t representative;     // Representation scope of the I&M Carrier
} PN_API_IM_SCarrier_t;

//+=============================================================================================
//|     Function prototypes
//+=============================================================================================

extern uint32_t PN_API_IM_addImCarrier(PN_API_IM_SCarrier_t *pImCarrier_p);
extern uint32_t PN_API_IM_removeImCarrier(PN_API_IM_SCarrierAddress_t *pAddress_p);
extern uint32_t PN_API_IM_resetToFactory(bool resetIm4_p);

#ifdef  __cplusplus 
}
#endif 

#endif //_PN_API_IM_H_
