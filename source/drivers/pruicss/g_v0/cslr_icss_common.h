/*
 *  Copyright (C) 2021 Texas Instruments Incorporated
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef CSLR_ICSS_COMMON_H_
#define CSLR_ICSS_COMMON_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include "cslr_icss_g.h"

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/*
 * Industrial Communications FWHAL and ICSS-EMAC sources are common for multiple
 * PRUICSS IP versions. In order to not use CSL for a specific version in these
 * sources, this common layer is added which acts as redirection to IP version
 * specific CSL.
 */

#define CSL_ICSS_PR1_IEP0_SLV_COUNT_REG0                                    CSL_ICSS_G_PR1_IEP0_SLV_COUNT_REG0
#define CSL_ICSS_PR1_IEP0_SLV_CAP_CFG_REG                                   CSL_ICSS_G_PR1_IEP0_SLV_CAP_CFG_REG
#define CSL_ICSS_PR1_IEP0_SLV_DIGIO_CTRL_REG                                CSL_ICSS_G_PR1_IEP0_SLV_DIGIO_CTRL_REG
#define CSL_ICSS_PR1_IEP0_SLV_DIGIO_EXP_REG                                 CSL_ICSS_G_PR1_IEP0_SLV_DIGIO_EXP_REG
#define CSL_ICSS_PR1_IEP0_SLV_DIGIO_DATA_OUT_REG                            CSL_ICSS_G_PR1_IEP0_SLV_DIGIO_DATA_OUT_REG

#define CSL_ICSS_PR1_MDIO_V1P7_MDIO_LINK_REG                                CSL_ICSS_G_PR1_MDIO_V1P7_MDIO_LINK_REG

#define CSL_ICSS_PR1_MII_RT_PR1_MII_RT_CFG_TXCFG0                           CSL_ICSS_G_PR1_MII_RT_PR1_MII_RT_CFG_TXCFG0
#define CSL_ICSS_PR1_MII_RT_PR1_MII_RT_CFG_TXCFG0_TX_IPG_WIRE_CLK_EN0_SHIFT CSL_ICSS_G_PR1_MII_RT_PR1_MII_RT_CFG_TXCFG0_TX_IPG_WIRE_CLK_EN0_SHIFT
#define CSL_ICSS_PR1_MII_RT_PR1_MII_RT_CFG_TXCFG0_TX_IPG_WIRE_CLK_EN0_MASK  CSL_ICSS_G_PR1_MII_RT_PR1_MII_RT_CFG_TXCFG0_TX_IPG_WIRE_CLK_EN0_MASK
#define CSL_ICSS_PR1_MII_RT_PR1_MII_RT_CFG_TXCFG1                           CSL_ICSS_G_PR1_MII_RT_PR1_MII_RT_CFG_TXCFG1
#define CSL_ICSS_PR1_MII_RT_PR1_MII_RT_CFG_TXCFG1_TX_IPG_WIRE_CLK_EN1_SHIFT CSL_ICSS_G_PR1_MII_RT_PR1_MII_RT_CFG_TXCFG1_TX_IPG_WIRE_CLK_EN1_SHIFT
#define CSL_ICSS_PR1_MII_RT_PR1_MII_RT_CFG_TXCFG1_TX_IPG_WIRE_CLK_EN1_MASK  CSL_ICSS_G_PR1_MII_RT_PR1_MII_RT_CFG_TXCFG1_TX_IPG_WIRE_CLK_EN1_MASK
#define CSL_ICSS_PR1_MII_RT_PR1_MII_RT_CFG_TX_IPG0                          CSL_ICSS_G_PR1_MII_RT_PR1_MII_RT_CFG_TX_IPG0
#define CSL_ICSS_PR1_MII_RT_PR1_MII_RT_CFG_TX_IPG0_TX_IPG0_SHIFT            CSL_ICSS_G_PR1_MII_RT_PR1_MII_RT_CFG_TX_IPG0_TX_IPG0_SHIFT
#define CSL_ICSS_PR1_MII_RT_PR1_MII_RT_CFG_TX_IPG0_TX_IPG0_MASK             CSL_ICSS_G_PR1_MII_RT_PR1_MII_RT_CFG_TX_IPG0_TX_IPG0_MASK
#define CSL_ICSS_PR1_MII_RT_PR1_MII_RT_CFG_TX_IPG1                          CSL_ICSS_G_PR1_MII_RT_PR1_MII_RT_CFG_TX_IPG1
#define CSL_ICSS_PR1_MII_RT_PR1_MII_RT_CFG_TX_IPG1_TX_IPG1_SHIFT            CSL_ICSS_G_PR1_MII_RT_PR1_MII_RT_CFG_TX_IPG1_TX_IPG1_SHIFT
#define CSL_ICSS_PR1_MII_RT_PR1_MII_RT_CFG_TX_IPG1_TX_IPG1_MASK             CSL_ICSS_G_PR1_MII_RT_PR1_MII_RT_CFG_TX_IPG1_TX_IPG1_MASK

#define CSL_ICSS_RAT_REGS_0_BASE                                            CSL_ICSS_G_RAT_REGS_0_BASE
#define CSL_ICSS_RAT_REGS_1_BASE                                            CSL_ICSS_G_RAT_REGS_1_BASE

#ifdef __cplusplus
}
#endif
#endif /* #ifndef CSLR_ICSS_COMMON_H_ */
