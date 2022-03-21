/*!
 *  \example appPermanentData.c
 *
 *  \brief
 *  PROFINET Device Permanent Data;
 *  manages Initialization, Saving, Reset of Permanent Data (Storage)
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

#include <PN_API_PDEV.h>
#include <PN_API_ETH.h>
#include <PN_API_SNMP.h>
#include <PN_API_DEV.h>
#include <PN_API_IM.h>

#include "appPermanentData.h"

#include <ti_board_open_close.h>

typedef struct APP_SPermHeader
{
    uint16_t magicNumber;
    uint32_t version;
    uint32_t checksum;
} APP_SPermHeader_t;

/*
 * Shall be changed with every modification of APP_SPermanentData_t data structure.
 * Otherwise old data structure will be fetched from flash causing a crash.
 */
#define APP_PERM_DATA_VERSION      1

/*
 * It must be possible to boot application from OSPI Flash.
 * Bootloader image is flashed with offset 0x0.
 * Application image is flsahed with offset 0x80000 by default.
 * Therefore it is important to design offset for permanent data in such a way,
 * that permanent data do not corrupt neither bootloader nor application images.
 *
*/
#define APP_PERM_DATA_OFFSET        0x200000
#define APP_PERM_DATA_OFFSET_START  APP_PERM_DATA_OFFSET
#define APP_PERM_DATA_OFFSET_END    (APP_PERM_DATA_OFFSET_START + sizeof (APP_SPermanentData_t))

Flash_Handle flashHandle_g;

static int32_t APP_erasePermStorage (void);

//*************************************************************************************************
static APP_SPermanentData_t permanentDataWorkingCopy_s;
static APP_SPermanentData_t permanentDataFactoryDefault_s =
{
    .magicNumber = (('J' << 8) | 'H'),
    .version = APP_PERM_DATA_VERSION,
    .checksum = 0,
    .ipAddress = 0,
    .subNetMask = 0,
    .gateWayAddress = 0,
    .stationNameLength = 0,
    .aStationName = { 0 },
    .helloMode = 0,
    .helloInterval = 0,
    .helloDelay = 0,
    .helloRetry = 0,
    .aImFields =
    {
        {   // Memory slot for I&M Carrier No. 1 (index 0)
            {
                0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                0x20, 0x20
            },     // tagFunction[PN_API_DEV_APP_IM_1_TAG_FUNCTION_LENGTH]
            {
                0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                0x20, 0x20
            },     // tagLocation[PN_API_DEV_APP_IM_1_TAG_LOCATION_LENGTH]
            {
                0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                0x20, 0x20, 0x20, 0x20, 0x20, 0x20
            },     // installationDate[PN_API_DEV_APP_IM_2_INSTALLATION_DATE_LENGTH]
            {
                0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                0x20, 0x20, 0x20, 0x20
            },     // descriptor[PN_API_DEV_APP_IM_3_DESCRIPTOR_LENGTH]
            {
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00
            }      // signature[PN_API_DEV_APP_IM_4_SIGNATURE_LENGTH]
        },
        {   // Memory slot for I&M Carrier No. 2 (index 1)

            {
                0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                0x20, 0x20
            },     // tagFunction[PN_API_DEV_APP_IM_1_TAG_FUNCTION_LENGTH]
            {
                0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                0x20, 0x20
            },     // tagLocation[PN_API_DEV_APP_IM_1_TAG_LOCATION_LENGTH]
            {
                0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                0x20, 0x20, 0x20, 0x20, 0x20, 0x20
            },     // installationDate[PN_API_DEV_APP_IM_2_INSTALLATION_DATE_LENGTH]
            {
                0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                0x20, 0x20, 0x20, 0x20
            },     // descriptor[PN_API_DEV_APP_IM_3_DESCRIPTOR_LENGTH]
            {
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00
            }      // signature[PN_API_DEV_APP_IM_4_SIGNATURE_LENGTH]
        },
        {   // Memory slot for I&M Carrier No. 3 (index 2)

            {
                0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                0x20, 0x20
            },     // tagFunction[PN_API_DEV_APP_IM_1_TAG_FUNCTION_LENGTH]
            {
                0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                0x20, 0x20
            },     // tagLocation[PN_API_DEV_APP_IM_1_TAG_LOCATION_LENGTH]
            {
                0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                0x20, 0x20, 0x20, 0x20, 0x20, 0x20
            },     // installationDate[PN_API_DEV_APP_IM_2_INSTALLATION_DATE_LENGTH]
            {
                0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                0x20, 0x20, 0x20, 0x20
            },     // descriptor[PN_API_DEV_APP_IM_3_DESCRIPTOR_LENGTH]
            {
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00
            }      // signature[PN_API_DEV_APP_IM_4_SIGNATURE_LENGTH]
        },
        {   // Memory slot for I&M Carrier No. 4 (index 3)

            {
                0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                0x20, 0x20
            },     // tagFunction[PN_API_DEV_APP_IM_1_TAG_FUNCTION_LENGTH]
            {
                0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                0x20, 0x20
            },     // tagLocation[PN_API_DEV_APP_IM_1_TAG_LOCATION_LENGTH]
            {
                0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                0x20, 0x20, 0x20, 0x20, 0x20, 0x20
            },     // installationDate[PN_API_DEV_APP_IM_2_INSTALLATION_DATE_LENGTH]
            {
                0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                0x20, 0x20, 0x20, 0x20
            },     // descriptor[PN_API_DEV_APP_IM_3_DESCRIPTOR_LENGTH]
            {
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00
            }      // signature[PN_API_DEV_APP_IM_4_SIGNATURE_LENGTH]
        },
        {   // Memory slot for I&M Carrier No. 5 (index 4)

            {
                0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                0x20, 0x20
            },     // tagFunction[PN_API_DEV_APP_IM_1_TAG_FUNCTION_LENGTH]
            {
                0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                0x20, 0x20
            },     // tagLocation[PN_API_DEV_APP_IM_1_TAG_LOCATION_LENGTH]
            {
                0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                0x20, 0x20, 0x20, 0x20, 0x20, 0x20
            },     // installationDate[PN_API_DEV_APP_IM_2_INSTALLATION_DATE_LENGTH]
            {
                0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                0x20, 0x20, 0x20, 0x20
            },     // descriptor[PN_API_DEV_APP_IM_3_DESCRIPTOR_LENGTH]
            {
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00
            }      // signature[PN_API_DEV_APP_IM_4_SIGNATURE_LENGTH]
        }
    },
    .permanentPortData =
    {
        .portDataAdjustPort1 =
        {
            .peer2PeerBoundaries =
            {
                .valid = 0,
                .boundaryLLDP = 0,
                .boundaryPTCP = 0,
                .boundaryTime = 0
            }
        },
        .portDataAdjustPort2 =
        {
            .peer2PeerBoundaries =
            {
                .valid = 0,
                .boundaryLLDP = 0,
                .boundaryPTCP = 0,
                .boundaryTime = 0
            }
        },
        .portDataCheckPort1 =
        {
            .numberOfPeers = 0,
            .aPeers =
            {
                {
                    .portIdLength = 0,
                    .aPortId =
                    {
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00
                    },
                    .chassisIdLength = 0,
                    .aChassisId =
                    {
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
                    }
                }
            },
            .checkLineDelay = 0,
            .lineDelayFormat = 0,
            .lineDelay = 0,
            .checkMauType = 0,
            .mauType = 0,
            .checkLinkState = 0,
            .linkStateLink = 0,
            .linkStatePort = 0,
            .checkSyncDiff = 0,
            .syncMaster = 0,
            .cableDelay = 0,
            .checkMauTypeMode = 0,
            .mauTypeMode = 0
        },
        .portDataCheckPort2 =
        {
            .numberOfPeers = 0,
            .aPeers =
            {
                {
                    .portIdLength = 0,
                    .aPortId =
                    {
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00
                    },
                    .chassisIdLength = 0,
                    .aChassisId =
                    {
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
                    }
                }
            },
            .checkLineDelay = 0,
            .lineDelayFormat = 0,
            .lineDelay = 0,
            .checkMauType = 0,
            .mauType = 0,
            .checkLinkState = 0,
            .linkStateLink = 0,
            .linkStatePort = 0,
            .checkSyncDiff = 0,
            .syncMaster = 0,
            .cableDelay = 0,
            .checkMauTypeMode = 0,
            .mauTypeMode = 0
        },
    },
    .permanentSnmpData =
    {
        .sysContactLength = 0,
        .aSysContact = { 0 },
        .sysNameLength = 0,
        .aSysName = { 0 },
        .sysLocationLength = 0,
        .aSysLocation = { 0 },
        .lldpMessageTxInterval = 5,
        .lldpMessageTxHoldMultiplier = 4,
        .lldpReinitDelay = 2,
        .lldpTxDelay = 2,
        .lldpNotificationInterval = 5,
        .port1 =
        {
            .lldpPortConfigAdminStatus = PN_API_SNMP_eAS_TX_AND_RX,
            .lldpPortConfigNotificationEnable = PN_API_SNMP_eBOOL_TRUE,
            .lldpPortConfigTLVsTxEnable = PN_API_SNMP_eLLDP_TX_ENABLE_PORT_DESC |
                                          PN_API_SNMP_eLLDP_TX_ENABLE_SYS_DESC |
                                          PN_API_SNMP_eLLDP_TX_ENABLE_SYS_CAP,
            .lldpXPnoConfigSPDTxEnable = PN_API_SNMP_eBOOL_TRUE,
            .lldpXPnoConfigPortStatusTxEnable = PN_API_SNMP_eBOOL_TRUE,
            .lldpXPnoConfigAliasTxEnable = PN_API_SNMP_eBOOL_TRUE,
            .lldpXPnoConfigMrpTxEnable = PN_API_SNMP_eBOOL_FALSE,
            .lldpXPnoConfigPtcpTxEnable = PN_API_SNMP_eBOOL_TRUE,
            .lldpXdot3PortConfigTLVsTxEnable = PN_API_SNMP_eLLDP_TX_MAC_PHY_CONFIG_STATUS |
                                               PN_API_SNMP_eLLDP_TX_POWER_VIA_MDI |
                                               PN_API_SNMP_eLLDP_TX_LINK_AGGREGATION |
                                               PN_API_SNMP_eLLDP_TX_MAX_FRAME_SIZE,
            .lldpXdot1ConfigPortVlanTxEnable = PN_API_SNMP_eBOOL_TRUE,
            .aLldpXdot1ConfigVlanNameTxEnable = { PN_API_SNMP_eBOOL_TRUE },
            .aLldpXdot1ConfigProtoVlanTxEnable = { PN_API_SNMP_eBOOL_TRUE },
            .aLldpXdot1ConfigProtocolTxEnable = { PN_API_SNMP_eBOOL_TRUE }
        },
        .port2 =
        {
            .lldpPortConfigAdminStatus = PN_API_SNMP_eAS_TX_AND_RX,
            .lldpPortConfigNotificationEnable = PN_API_SNMP_eBOOL_TRUE,
            .lldpPortConfigTLVsTxEnable = PN_API_SNMP_eLLDP_TX_ENABLE_PORT_DESC |
                                          PN_API_SNMP_eLLDP_TX_ENABLE_SYS_DESC |
                                          PN_API_SNMP_eLLDP_TX_ENABLE_SYS_CAP,
            .lldpXPnoConfigSPDTxEnable = PN_API_SNMP_eBOOL_TRUE,
            .lldpXPnoConfigPortStatusTxEnable = PN_API_SNMP_eBOOL_TRUE,
            .lldpXPnoConfigAliasTxEnable = PN_API_SNMP_eBOOL_TRUE,
            .lldpXPnoConfigMrpTxEnable = PN_API_SNMP_eBOOL_FALSE,
            .lldpXPnoConfigPtcpTxEnable = PN_API_SNMP_eBOOL_TRUE,
            .lldpXdot3PortConfigTLVsTxEnable = PN_API_SNMP_eLLDP_TX_MAC_PHY_CONFIG_STATUS |
                                               PN_API_SNMP_eLLDP_TX_POWER_VIA_MDI |
                                               PN_API_SNMP_eLLDP_TX_LINK_AGGREGATION |
                                               PN_API_SNMP_eLLDP_TX_MAX_FRAME_SIZE,
            .lldpXdot1ConfigPortVlanTxEnable = PN_API_SNMP_eBOOL_TRUE,
            .aLldpXdot1ConfigVlanNameTxEnable = { PN_API_SNMP_eBOOL_TRUE },
            .aLldpXdot1ConfigProtoVlanTxEnable = { PN_API_SNMP_eBOOL_TRUE },
            .aLldpXdot1ConfigProtocolTxEnable = { PN_API_SNMP_eBOOL_TRUE }
        },
        .lldpConfigManAddrPortsTxEnable = PN_API_SNMP_eLLDP_PORT_1_ENABLE |
                                          PN_API_SNMP_eLLDP_PORT_2_ENABLE
    }
};

/*!
*  <!-- Description: -->
* \brief
* Initialize permanent data.
*
* \details
* Read permanent data stored on OSPI Flash and copy the content to the working copy of permanent data structure.
* If the content is corrupted or inconsistent, it will be replaced with default factory settings.
* In this case factory settings will be written to flash as well.
*
* Following content check mechanisms are used here:
*
* - permanent data version (APP_PERM_DATA_VERSION).
* If the permanent data structure has been changed during development, this constant should be modified.
* Otherwise previously stored structure cannot be properly copied to the new structur.
*
* - magic number
* The known field of the structure must have this value.
*
* - checksum
* TBD
*
* \remarks
* Relevant blocks in OSPI Flash must be erased prior every writing.
*
* <!-- Parameters and return values: -->
*
* \return     Error code of uint32_t type.  SystemP_SUCCESS on success, else failure.
*
*/
int32_t APP_initPermStorage (
    void)
{
    int32_t     result      = SystemP_SUCCESS;
    uint32_t    checkSum    = 0;

    flashHandle_g = Flash_getHandle (CONFIG_FLASH0);

    result = Flash_read (flashHandle_g, APP_PERM_DATA_OFFSET, (uint8_t *)&permanentDataWorkingCopy_s, sizeof (APP_SPermanentData_t));
    if (SystemP_SUCCESS != result)
    {
        goto laError;
    }

    checkSum = 0;  /* TBD: calculate checksum */

     if (   (permanentDataWorkingCopy_s.magicNumber != (('J' << 8) | 'H'))
        || (permanentDataWorkingCopy_s.version != APP_PERM_DATA_VERSION)
        || (permanentDataWorkingCopy_s.checksum != checkSum)
       )
    {   /* Data inconsistent -> write default values */

        OSAL_printf ("\r[APP] WARN: Permanent data is corrupt. Restoring factory settings.\n");
        memcpy (&permanentDataWorkingCopy_s, &permanentDataFactoryDefault_s, sizeof (APP_SPermanentData_t));

        permanentDataWorkingCopy_s.magicNumber = (('J' << 8) | 'H');
        permanentDataWorkingCopy_s.version = APP_PERM_DATA_VERSION;
        permanentDataWorkingCopy_s.checksum = checkSum;

        result = APP_erasePermStorage ();
        if (SystemP_SUCCESS != result)
        {
            goto laError;
        }

        result = Flash_write (flashHandle_g, APP_PERM_DATA_OFFSET, (uint8_t *)&permanentDataWorkingCopy_s, sizeof (APP_SPermanentData_t));
        if (SystemP_SUCCESS != result)
        {
            goto laError;
        }
        OSAL_printf ("\r[APP] INFO: Wrote factory settings to OSPI flash.\n");
    }

    return result;

laError:
    OSAL_printf ("\r[APP] ERROR: Failed to initialize permanent data from OSPI flash.");
    return result;
}
    
/*!
*  <!-- Description: -->
* \brief
* Save the working copy of permanent data.
*
* \details
* This function must be called to store the working copy of the permanent data, which resides in RAM, to permanent storage.
* 
* \remarks
* Relevant blocks in OSPI Flash must be erased prior every writing.
*
* <!-- Parameters and return values: -->
*
* \param[in]  pData_p                       Permanent data to be saved.
*
* \return     Error code of int32_t type. SystemP_SUCCESS on success, else failure.
*
*/
int32_t APP_savePermStorage (
    APP_SPermanentData_t *pData_p)
{
    int32_t     result      = SystemP_SUCCESS;
    uint32_t    checkSum    = 0;

    checkSum = 0;  /* TBD: calculate checksum */
    
    pData_p->magicNumber = (('J' << 8) | 'H');
    pData_p->version = APP_PERM_DATA_VERSION;
    pData_p->checksum = checkSum;

    APP_erasePermStorage ();

    result = Flash_write (flashHandle_g, APP_PERM_DATA_OFFSET, (uint8_t *)pData_p, sizeof (APP_SPermanentData_t));
    if (SystemP_SUCCESS != result)
    {
        goto laError;
    }

    OSAL_printf ("\r[APP] INFO: Wrote permanent data to OSPI flash.\n");
    return result;

laError:
    OSAL_printf("\r[APP] ERROR: Failed to write OSPI flash.\n");
    return result;
}
    
/*!
*  <!-- Description: -->
* \brief
* Get the working copy of permanent data.
*
* \details
* Prerequisite: permanent data must be initialized using APP_initPermStorage
*
* <!-- Parameters and return values: -->
*
** \param[in]  pData_p                       Permanent data to be saved.
*
*  \return     Handle to the working copy of the permanent data.
*
*/
APP_SPermanentData_t *APP_getPermStorage (
    void)
{
    return (&permanentDataWorkingCopy_s);
}

/*!
*  <!-- Description: -->
* \brief
* Factory Reset of permanent data.
*
* \details
* Reset all values that desired to be reset on Factory Reset.
*
* \remarks
* This function doesn't write reseted permanent data to OSPI Flash.
* This shall be done separately, using APP_getPermStorage and APP_savePermStorage.
*
* <!-- Parameters and return values: -->
*
* \param[in]  pData_p                       Permanent data to be saved.
*
* \return     Handle to the working copy of the permanent data.
*
*/
void APP_factoryResetPermStorage (
    void)
{
    memcpy (&permanentDataWorkingCopy_s, &permanentDataFactoryDefault_s, sizeof (APP_SPermanentData_t));
}

/*!
*  <!-- Description: -->
* \brief
* Reset Communication parameter of permanent data.
*
* \details
* Reset all values that must be reset on Reset to Factory Mode 2 (Reset Communication parameter).
* These are: name of station, IP, all PDev parameters...
*
* \remarks
* This function doesn't write permanent data to the permanent storage.
* This shall be done separately, using APP_getPermStorage and APP_savePermStorage.
*
*
* <!-- Parameters and return values: -->
*
*/
void APP_resetCommParamPermStorage (
    void)
{
    permanentDataWorkingCopy_s.ipAddress = permanentDataFactoryDefault_s.ipAddress;
    permanentDataWorkingCopy_s.subNetMask = permanentDataFactoryDefault_s.subNetMask;
    permanentDataWorkingCopy_s.gateWayAddress = permanentDataFactoryDefault_s.gateWayAddress;
    permanentDataWorkingCopy_s.stationNameLength = permanentDataFactoryDefault_s.stationNameLength;
    memcpy (&permanentDataWorkingCopy_s.aStationName, &permanentDataFactoryDefault_s.aStationName, PN_API_ETH_MAX_STATION_NAME_LENGTH);
    permanentDataWorkingCopy_s.helloMode = permanentDataFactoryDefault_s.helloMode;
    permanentDataWorkingCopy_s.helloInterval = permanentDataFactoryDefault_s.helloInterval;
    permanentDataWorkingCopy_s.helloDelay = permanentDataFactoryDefault_s.helloDelay;
    permanentDataWorkingCopy_s.helloRetry = permanentDataFactoryDefault_s.helloRetry;
    memcpy (&permanentDataWorkingCopy_s.permanentPortData, &permanentDataFactoryDefault_s.permanentPortData, sizeof (PN_API_ETH_SPermanentPortData_t));
    memcpy (&permanentDataWorkingCopy_s.permanentSnmpData, &permanentDataFactoryDefault_s.permanentSnmpData, sizeof (PN_API_SNMP_SPermanentData_t));
}

/*!
*  <!-- Description: -->
* \brief
* Erase permanent data.
*
* \details
* Permanent data structure will be erased on the permanent storage.
*
* \remarks
* It must be done prior to every write to the OSPI flash memory.
*
* <!-- Parameters and return values: -->
*
* \return     Error code of int32_t type. SystemP_SUCCESS on success, else failure.
*
*/
int32_t APP_erasePermStorage (
    void)
{
    Flash_Attrs *pAttr      = NULL;
    int32_t     result      = SystemP_SUCCESS;
    uint32_t    offset      = 0;
    uint32_t    block       = 0;
    uint32_t    page        = 0;

    pAttr = Flash_getAttrs (CONFIG_FLASH0);

    for (offset = APP_PERM_DATA_OFFSET_START; offset <= APP_PERM_DATA_OFFSET_END; offset += pAttr->blockSize)
    {
        result = Flash_offsetToBlkPage (flashHandle_g, offset, &block, &page);
        if (SystemP_SUCCESS != result)
        {
            goto laError;
        }

        result = Flash_eraseBlk (flashHandle_g, block);
        if (SystemP_SUCCESS != result)
        {
            goto laError;
        }
    }
    OSAL_printf ("\r[APP] INFO: Erased permanent data from OSPI flash.\n");
    return result;

laError:
    OSAL_printf ("\r[APP] ERROR: Failed to erase permanent data from OSPI flash.\n");
    return result;
}