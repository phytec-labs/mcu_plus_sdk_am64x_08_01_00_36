/*!
 *  \example appStackConfigure.c
 *  \ingroup Example1 
 *
 *  \brief
 *  PROFINET Device Stack Configuration
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
 
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include <osal.h>

#include <PN_API_PDEV.h>
#include <PN_API_ETH.h>
#include <PN_API_SNMP.h>
#include <PN_API_DEV.h>
#include <PN_API_IM.h>

#include "appStackConfigure.h"
#include "appUserInfo.h"
#include "appPermanentData.h"
#include "appStackInit.h"
#include "appRealIdentification.h"

static uint32_t APP_initProfinetStack(uint8_t pruLogicalInstance_p);
static uint32_t APP_configureProfinetStack(void);

static uint32_t APP_configPdev (void);
static uint32_t APP_configEth (void);
static uint32_t APP_configSnmp (void);
static uint32_t APP_configDev (void);

/*!
*  <!-- Description: -->
* \brief
* Start PROFINET stack.
*
* \details
* Function executes init, configure and start sequences for PROFINET stack.
*
* <!-- Parameters and return values: -->
*
* \param[in]  pruLogicalInstance_p            Logical instance of the PRU defined for the PROFINET communication .
*
* \return     Error code of uint32_t type. 0 on success, else failure.
*
*/
uint32_t APP_startProfinetStack (
    uint8_t pruLogicalInstance_p)
{
    uint32_t result = 0;

    /*
    * Init sequence.
    */
    result = APP_initProfinetStack(pruLogicalInstance_p);
    if (result != PN_API_PDEV_eOK)
    {
        goto laError;
    }

    /*
    * Configuration sequence.
    */
    result = APP_configureProfinetStack();
    if (result != PN_API_PDEV_eOK)
    {
        goto laError;
    }

    /*
    * Start sequence.
    */

    /* Start PROFINET stack. */
    OSAL_printf("\r[APP] INFO: Starting PROFINET stack...\n");
    result = PN_API_PDEV_start ();
    if (result != PN_API_PDEV_eOK)
    {
        OSAL_printf ("\r\n[APP] ERROR: Failed to start PROFINET stack. Error code:  0x%8.8x", result);
        goto laError;
    }
    OSAL_printf("\r[APP] INFO: Done!\n");

    /* Start PRU. Shall be called after starting PROFINET stack. */
    OSAL_printf("\r[APP] INFO: Starting PRU...\n");
    APP_pruStart ();
    OSAL_printf("\r[APP] INFO: Done!\n");

    return result;

laError:
    return result;
}

/*!
*  <!-- Description: -->
* \brief
* Initialization of PROFINET stack and necessary peripherals.
*
* \details
* Initialize:
* - PRU instance assigned for the PROFINET communication;
* - permanent data (e.g. I&M);
* - PROFINET physical device with all its interfaces;
* 
* <!-- Parameters and return values: -->
*
*  \param[in]  pruLogicalInstance_p            Logical instance of the PRU defined for the PROFINET communication .
*
* \return     Error code of uint32_t type. 0 on success, else failure.
*
*/
static uint32_t APP_initProfinetStack(
    uint8_t pruLogicalInstance_p)
{
    uint32_t result = 0;

    OSAL_printf("\r[APP] INFO: Initializing PRU instance ...\n");
    result = APP_pruInit(pruLogicalInstance_p);
    if (OSAL_eERR_NOERROR != result)
    {
        goto laError;
    }
    OSAL_printf("\r[APP] INFO: Done!\n");

    OSAL_printf("\r\n[APP] INFO: Initializing permanent data...");
    result = APP_initPermStorage();
    if (OSAL_eERR_NOERROR != result)
    {
        goto laError;
    }
    OSAL_printf("\r[APP] INFO: Done!\n");

    OSAL_printf("\r[APP] INFO: Initializing physical device...\n");
    PN_API_PDEV_init();
    OSAL_printf("\r[APP] INFO: Done!\n");

    return result;

laError:
    return result;
}

/*!
*  <!-- Description: -->
* \brief
* Configuration of PROFINET stack.
*
* \details
* Configures following components of a PROFINET device:
* - Physical device;
* - Ethernet interface;
* - SNMP interface;
* - Logical device (equipment);
*
* <!-- Parameters and return values: -->
*
* \return     Error code of uint32_t type. 0 on success, else failure.
*
*/
static uint32_t APP_configureProfinetStack(
    void)
{
    uint32_t result = PN_API_PDEV_eOK;

    OSAL_printf("\r[APP] INFO: Configuring physical device...\n");
    result = APP_configPdev();
    if (result != PN_API_PDEV_eOK)
    {
        goto laError;
    }
    OSAL_printf("\r[APP] INFO: Done!\n");

    OSAL_printf("\r[APP] INFO: Configuring Ethernet interface...\n");
    result = APP_configEth();
    if (result != PN_API_ETH_eOK)
    {
        goto laError;
    }
    OSAL_printf("\r[APP] INFO: Done!\n");

    OSAL_printf("\r[APP] INFO: Configuring SNMP interface...\n");
    result = APP_configSnmp();
    if (result != PN_API_SNMP_eOK)
    {
        goto laError;
    }
    OSAL_printf("\r[APP] INFO: Done!\n");

    OSAL_printf("\r[APP] INFO: Configuring logical device and equipment...\n");
    result = APP_configDev();
    if (result != PN_API_DEV_eOK)
    {
        goto laError;
    }
    OSAL_printf("\r[APP] INFO: Done!\n");

    return result;

laError:
    return result;
}

/*!
*  <!-- Description: -->
* \brief
* Configuration of the PROFINET physical device.
*
* \details
* An instance of PN_API_PDEV_SConfiguration_t must be filled with valid data and passed to the stack.
*
* \warning
* The configuration cannot be altered after starting the stack.
* 
* <!-- Parameters and return values: -->
*
* \return     Error code of uint32_t type. 0 on success, else failure.
*
*/
static uint32_t APP_configPdev (
    void)
{
    uint32_t result                                                                         = PN_API_PDEV_eOK;
    PN_API_PDEV_SConfiguration_t configuration                                              = {0};
    uint8_t aSerialNumber[PN_API_PDEV_MAX_SERIAL_NUMBER_LENGTH]                             = {0};
    uint8_t aHwRevision[PN_API_PDEV_MAX_HW_REVISION_LENGTH]                                 = {0};
    uint8_t aSwRevisionFunctionalEnhancement[PN_API_PDEV_MAX_SW_REVISION_SUBSTRING_LENGTH]  = {0};
    uint8_t aSwRevisionBugFix[PN_API_PDEV_MAX_SW_REVISION_SUBSTRING_LENGTH]                 = {0};
    uint8_t aSwRevisionInternalChange[PN_API_PDEV_MAX_SW_REVISION_SUBSTRING_LENGTH]         = {0};

    APP_UI_getSerialNumberString (aSerialNumber, PN_API_PDEV_MAX_SERIAL_NUMBER_LENGTH);
    APP_UI_getHardwareRevisionString (aHwRevision, PN_API_PDEV_MAX_HW_REVISION_LENGTH);
    APP_UI_getSwRevFuncEnhancementString (aSwRevisionFunctionalEnhancement, PN_API_PDEV_MAX_SW_REVISION_SUBSTRING_LENGTH);
    APP_UI_getSwRevBugFixString (aSwRevisionBugFix, PN_API_PDEV_MAX_SW_REVISION_SUBSTRING_LENGTH);
    APP_UI_getSwRevInternalChangeString (aSwRevisionInternalChange, PN_API_PDEV_MAX_SW_REVISION_SUBSTRING_LENGTH);

    configuration.vendorId = PNC_VENDOR_ID;
    configuration.deviceId = PNC_DEVICE_ID;

    configuration.chassisId.deviceDescriptionLength = sizeof (PNC_DEVICE_DESCRIPTION) - 1;
    configuration.chassisId.pDeviceDescription = (uint8_t *)PNC_DEVICE_DESCRIPTION;
    configuration.chassisId.orderIdLength = sizeof (PNC_DEVICE_ORDER_ID) - 1;
    configuration.chassisId.pOrderId = (uint8_t *)PNC_DEVICE_ORDER_ID;
    configuration.chassisId.serialNumberLength = PN_API_PDEV_MAX_SERIAL_NUMBER_LENGTH;
    configuration.chassisId.pSerialNumber = aSerialNumber;
    configuration.chassisId.hwRevisionLength = PN_API_PDEV_MAX_HW_REVISION_LENGTH;
    configuration.chassisId.pHwRevision = aHwRevision;
    configuration.chassisId.swRevisionPrefix = PNC_DEVICE_SW_REV_PREFIX;
    configuration.chassisId.swRevisionFunctionalEnhancementLength = PN_API_PDEV_MAX_SW_REVISION_SUBSTRING_LENGTH;
    configuration.chassisId.pSwRevisionFunctionalEnhancement = aSwRevisionFunctionalEnhancement;
    configuration.chassisId.swRevisionBugFixLength = PN_API_PDEV_MAX_SW_REVISION_SUBSTRING_LENGTH;
    configuration.chassisId.pSwRevisionBugFix = aSwRevisionBugFix;
    configuration.chassisId.swRevisionInternalChangeLength = PN_API_PDEV_MAX_SW_REVISION_SUBSTRING_LENGTH;
    configuration.chassisId.pSwRevisionInternalChange = aSwRevisionInternalChange;

    result = PN_API_PDEV_applyConfiguration (&configuration);

    if (result != PN_API_PDEV_eOK)
    {
        OSAL_printf("\r[APP] ERROR: Error during configuration of the physical device. Error code:  0x%8.8x\n", result);
    }

    return result;
}

/*!
*  <!-- Description: -->
* \brief
* Configuration of the Ethernet interface.
*
* \details
* An instance of PN_API_ETH_SConfiguration_t must be filled with valid data and passed to the stack.
*
* <!-- Parameters and return values: -->
*
* \return     Error code of uint32_t type. 0 on success, else failure.
*
*/
static uint32_t APP_configEth (
    void)
{
    uint32_t result                                 = 0;
    PN_API_ETH_SConfiguration_t configuration       = {0};
    PN_API_ETH_SPermanentDcpData_t permanentDcpData = {0};
    APP_SPermanentData_t *pPermanentData            = NULL;
    
    pPermanentData = APP_getPermStorage();

    memmove (&configuration.macAddress, APP_UI_getMacAddr (), sizeof (PN_API_ETH_SMacAddress_t));

    permanentDcpData.ipAddress = pPermanentData->ipAddress;
    permanentDcpData.subnetMask = pPermanentData->subNetMask;
    permanentDcpData.gateway = pPermanentData->gateWayAddress;

    permanentDcpData.stationNameLength = pPermanentData->stationNameLength;
    memcpy (permanentDcpData.aStationName, pPermanentData->aStationName, pPermanentData->stationNameLength);

    configuration.pPermanentDcpData = &permanentDcpData;
    configuration.pPermanentPortData = &pPermanentData->permanentPortData;

    configuration.callBacks.cbHandleInternalError = &APP_UI_cbErrorHandler;
    configuration.callBacks.cbGetLocalTime = &APP_UI_cbGetLocalTime;
    configuration.callBacks.cbGetServerBootTime = &APP_UI_cbGetServerBootTime;
    configuration.callBacks.cbSavePermanentData = &APP_UI_cbSavePermanentData;
    configuration.callBacks.cbSignalLinkLed = &APP_UI_cbSignalLinkLed;
    configuration.callBacks.cbSetStationName = &APP_UI_cbSetStationName;
    configuration.callBacks.cbSetIpAddress = &APP_UI_cbSetIpAddress;
    configuration.callBacks.cbFactoryReset = &APP_UI_cbFactoryReset;
    configuration.callBacks.cbResetToFactory = &APP_UI_cbResetToFactory;
    configuration.callBacks.cbWritePortDataCheck = &APP_UI_cbWritePortDataCheck;
    configuration.callBacks.cbWritePortDataAdjust = &APP_UI_cbWritePortDataAdjust;

    result = PN_API_ETH_applyConfiguration (&configuration);

    if (result != PN_API_ETH_eOK)
    {
        OSAL_printf("\r[APP] ERROR: Error during configuration of the Ethernet interface. Error code:  0x%8.8x\n", result);
    }

    return result;
}

/*!
*  <!-- Description: -->
* \brief
* Configuration of the SNMP interface.
*
* \details
* An instance of PN_API_SNMP_SConfiguration_t must be filled with valid data and passed to the stack.
*
* \remarks
* SNMP is mandatory for PROFINET Device with conformance class CC-B or CC-C.
* 
* <!-- Parameters and return values: -->
*
* \return     Error code of uint32_t type. 0 on success, else failure.
*
*/
static uint32_t APP_configSnmp (
    void)
{
    uint32_t result                            = PN_API_SNMP_eOK;
    PN_API_SNMP_SConfiguration_t configuration = {0};
    APP_SPermanentData_t *pPermanentData       = NULL;

    pPermanentData = APP_getPermStorage();

    configuration.interfaceDescriptionLength = sizeof (PNC_IF_DESCRIPTION) - 1;
    configuration.pInterfaceDescription = (uint8_t *)PNC_IF_DESCRIPTION;
    configuration.port1DescriptionLength = sizeof (PNC_PORT_1_DESCRIPTION) - 1;
    configuration.pPort1Description = (uint8_t *)PNC_PORT_1_DESCRIPTION;
    configuration.port2DescriptionLength = sizeof (PNC_PORT_2_DESCRIPTION) - 1;
    configuration.pPort2Description = (uint8_t *)PNC_PORT_2_DESCRIPTION;

    configuration.pPermanentData = &pPermanentData->permanentSnmpData;

    configuration.callBacks.cbSetPermanentData = &APP_UI_cbSetPermanentSnmpData;

    result = PN_API_SNMP_applyConfiguration (&configuration);
    if (result != PN_API_SNMP_eOK)
    {
        OSAL_printf("\r[APP] ERROR: Error during configuration of the SNMP interface. Error code:  0x%8.8x\n", result);
    }

    return result;
}

/*!
*  <!-- Description: -->
* \brief
* Configuration of the Logical device.
*
* \details
* In this function logical device is being configured: 
* - process data buffers are assigned,
* - equipment is being plugged, 
* - I&M data is being assigned...
*
* <!-- Parameters and return values: -->
*
* \return     Error code of uint32_t type. 0 on success, else failure.
*
*/
static uint32_t APP_configDev (
    void)
{
    uint32_t result = PN_API_DEV_eOK;

    result = PN_API_DEV_setOutputDataBuffer (aCycDataOutput_g, aCycDataOutput_g, aCycDataOutput_g, CYC_DATA_OUTPUT_LEN);
    if (result != PN_API_DEV_eOK)
    {
        goto laError;
    }

    result = PN_API_DEV_setInputDataBuffer (aCycDataInput_g, aCycDataInput_g, aCycDataInput_g, CYC_DATA_INPUT_LEN);
    if (result != PN_API_DEV_eOK)
    {
        goto laError;
    }

    result = APP_configureSubmodules();
    if (result != PN_API_DEV_eOK)
    {
        goto laError;
    }

    result = APP_configureImData();
    if (result != PN_API_DEV_eOK)
    {
        goto laError;
    }

    // Stack must be able to store writable I&M data to persistent storage.
    // It will use the general WriteRecordIndicatedCallback to indicate application, that record data shall be stored.
    // Application is responsible to store writable I&M data fields.
    result = PN_API_DEV_registerWriteRecordIndicatedCallback(APP_cbWriteRecordInd);
    if (result != PN_API_DEV_eOK)
    {
        goto laError;
    }

    //----------------------------------------------------
    //          Equipment configuration finished
    // (must be always invoked to apply new configuration)
    //----------------------------------------------------
    result = PN_API_DEV_applyEquipmentConfiguration();
    if (result != PN_API_DEV_eOK)
    {
        goto laError;
    }

    return result;

laError:
    OSAL_printf("\r[APP] ERROR: Error during configuration of the logical device. Error code:  0x%8.8x\n", result);
    return result;
}

/*!
*  <!-- Description: -->
* \brief
* Run PROFINET stack and update process data.
*
* \details
* Here application consumes data delivered by stack (output data from PROFINET controller) 
* and provides data to stack (input data to PROFINET controller).
* 
* In this example application manipulates first two bytes prior providing them to controller:
* Byte 0 has exactly same value as it was received from controller
* Byte 1 is being constantly incremented. It is used for flashing of the cyclic LEDs.
*
* PN_API_PDEV_run function, which must be called regularly from the main application, is called here.
* 
* <!-- Parameters and return values: -->
*
* \return     Error code of uint32_t type. 0 on success, else failure.
*
*/
uint32_t APP_runProfinetStack (
    void)
{
    uint32_t result                 = 0;
    uint8_t *pBufOut                = NULL;
    uint8_t *pBufIn                 = NULL;
    static uint8_t incrementValue   = 0;

    /*This function must be called regularly.*/
    result = PN_API_PDEV_run ();
    if (result != PN_API_PDEV_eOK)
    {
        goto laError;
    }

    result = PN_API_DEV_getBufferOutputData (&pBufOut);
    if (result != PN_API_DEV_eOK)
    {
        goto laError;
    }

    result = PN_API_DEV_getBufferInputData (&pBufIn);
    if (result != PN_API_DEV_eOK)
    {
        goto laError;
    }

    /*Input data is updated before being sent to controller.*/
    pBufIn[0] = pBufOut[0];
    pBufIn[1] = ++incrementValue;

    result = PN_API_DEV_releaseBufferInputData ();
    if (result != PN_API_DEV_eOK)
    {
        goto laError;
    }

    return result;

laError:
    OSAL_printf("\r[APP] ERROR: Error during PROFINET stack run. Error code:  0x%8.8x\n", result);
    return result;
}