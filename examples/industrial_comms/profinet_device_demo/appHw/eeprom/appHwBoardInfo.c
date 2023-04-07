/*!
 *  \example appHwBoardInfo.c
 *
 *  \brief
 *  Common implementation to get the board info from eeprom
 *
 *  \author
 *  KUNBUS GmbH
 *
 *  \date
 *  2021-10-20
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
 */

#include <stdio.h>
#include <string.h>
#include "board/eeprom.h"
#include "ti_board_open_close.h"
#include "appHwBoardInfo.h"
#include "appHwBoardInfoSpecific.h"
#include "appHwError.h"
#include "ti_drivers_open_close.h" // True

#include <drivers/i2c.h>  // True

#define I2C_READ_SLAVE_ADDR             (0x50U) //True

/*!
 *  \brief We store the board info data here.
 */
static APP_HW_BOARD_INFO_SData_t APP_HW_BOARD_INFO_data_g = { 0 };

/*!
 *  Function: APP_HW_BOARD_INFO_read
 *
 *  \brief
 *  Reads the board info data from the AT24C EEPROM.
 *
 *  \details
 *  Reads the board info data from the AT24C EEPROM (I2C address 0x50) and updates the 
 *  global variable #APP_HW_BOARD_INFO_data_g
 *
 *  \remarks
 *  This is a blocking function, since i2c transfers are blocking. Ideally this function
 *  should only be called once during startup.
 *
 *  \param[in]     none
 *
 *  \return        Error code as #APP_HW_EError_t
 *  \retval        #APP_HW_eNO_ERROR          No error.
 *  \retval        #APP_HW_eFATAL_ERROR       A fatal error occurred.
 */
APP_HW_EError_t APP_HW_BOARD_INFO_read(void)
{
    int32_t status = SystemP_SUCCESS;
    APP_HW_EError_t result = APP_HW_eNO_ERROR;

    I2C_Handle      i2cHandle; //True
    I2C_Transaction i2cTransaction; //True

    //status = EEPROM_read(gEepromHandle[CONFIG_EEPROM0], 0, (uint8_t *)&APP_HW_BOARD_INFO_data_g, sizeof(APP_HW_BOARD_INFO_data_g)); // True commented
    i2cHandle = gI2cHandle[CONFIG_I2C_EEPROM]; //True

    I2C_Transaction_init(&i2cTransaction); // True

    /* Override with required transaction parameters */ // True
    i2cTransaction.readBuf      = (uint8_t *)&APP_HW_BOARD_INFO_data_g; // True
    i2cTransaction.readCount    = sizeof(APP_HW_BOARD_INFO_data_g); // True
    i2cTransaction.slaveAddress = I2C_READ_SLAVE_ADDR; // True

    status = I2C_transfer(i2cHandle, &i2cTransaction); // True



    if (status != SystemP_SUCCESS)
    {
        result = APP_HW_eFATAL_ERROR;
    }
    return result;
}

/*!
 *  Function: APP_HW_BOARD_INFO_getMacInfo
 *
 *  \brief
 *  Gets a copy of the read mac address section from the board info EEPROM.
 *
 *  \remarks
 *  Call #APP_HW_BOARD_INFO_read first to read the data from EEPROM.
 * 
 *  \param[in]     none
 *
 *  \return        copy of the member "mac" of #APP_HW_BOARD_INFO_data_g
 */
APP_HW_BOARD_INFO_SMAC_Addr_t APP_HW_BOARD_INFO_getMacInfo(void)
{
    return APP_HW_BOARD_INFO_data_g.mac;
}
