/*!
 *  \file ESL_BOARD_config.h
 *
 *  \brief
 *  Board configuration for AM243X GP EVM.
 *
 *  \author
 *  KUNBUS GmbH
 *
 *  \date
 *  2021-06-21
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

#if !(defined __ESL_BOARD_CONFIG_H__)
#define __ESL_BOARD_CONFIG_H__		1

#include <ESL_OS_os.h>
#include <ESL_BOARD_OS_config.h>

#define TIESC_PHYADDR_0                     15
#define TIESC_PHYADDR_1                     3
#define TIESC_PHYADDR_2                     15
#define TIESC_PHYADDR_3                     3
#define TIESC_LINK0_POLINVERT               true
#define TIESC_LINK1_POLINVERT               true
#define TIESC_LINK2_POLINVERT               true
#define TIESC_LINK3_POLINVERT               true
#define TIESC_LINK4_POLINVERT               true
#define TIESC_LINK5_POLINVERT               true
#define TIESC_LINK0_USERXLINK               true /* could use ..... maybe iomux missing */
#define TIESC_LINK1_USERXLINK               true
#define TIESC_LINK2_USERXLINK               true
#define TIESC_LINK3_USERXLINK               true

#define TIESC_I2CDEVICE                     "/dev/i2c-0"

 #define ECAT_PRODUCTCODE_CTT               0x54490023      // set product code
 #define ECAT_PRODUCTCODE_CIA402            0x54490024      // set product code
 #define ECAT_PRODUCTCODE_SIMPLE            0x54490025      // set product code
 #define ECAT_PRODUCTNAME_CTT               "TI EtherCAT Toolkit Conformance for AM243X.R5F"
 #define ECAT_PRODUCTNAME_CIA402            "TI EtherCAT Toolkit CiA402 for AM243X.R5F"
 #define ECAT_PRODUCTNAME_SIMPLE            "TI EtherCAT Toolkit for AM243X.R5F"
 #define ECAT_REVISION_NO                   0x00010000

#define ESL_DEFAULT_PRUICSS                 EC_API_SLV_ePRUICSS_INSTANCE_TWO

#if (defined __cplusplus)
extern "C" {
#endif

extern void ESL_BOARD_OS_initPruss(uint32_t pruSelect_p, int32_t* pBaseIrqOffset_p);
extern void ESL_Board_StatusLED(void* pGpioHandle_p, uint32_t selectedPru_p, bool runLed_p, bool errLed_p);

#if (defined __cplusplus)
}
#endif

#endif /* __ESL_BOARD_CONFIG_H__ */
