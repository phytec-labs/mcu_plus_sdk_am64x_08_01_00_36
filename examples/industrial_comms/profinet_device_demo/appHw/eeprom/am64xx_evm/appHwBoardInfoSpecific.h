/*!
 *  \example appHwBoardInfoSpecific.h
 *
 *  \brief
 *  am64x-evm specific board info eeprom definitions
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

#ifndef APP_HW_BOARD_INFO_SPECIFIC
#define APP_HW_BOARD_INFO_SPECIFIC

#include "appHwBoardInfo.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
    *  \brief Structure to store the board data
    */
typedef struct APP_HW_BOARD_INFO_SData
{
    APP_HW_BOARD_INFO_SHeader_t header;
    APP_HW_BOARD_INFO_SIdent_t boardIdent;
    APP_HW_BOARD_INFO_SDDRRam_t ddr;
    APP_HW_BOARD_INFO_SMAC_Addr_t mac;
    uint8_t endList;
}__attribute__((packed)) APP_HW_BOARD_INFO_SData_t;

#ifdef  __cplusplus 
}
#endif 

#endif // APP_HW_BOARD_INFO_SPECIFIC
