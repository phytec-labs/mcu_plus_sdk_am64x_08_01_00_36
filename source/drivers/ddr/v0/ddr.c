/******************************************************************************
 * Copyright (c) 2021 Texas Instruments Incorporated - http://www.ti.com
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
 *
 *****************************************************************************/

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdint.h>
#include <string.h>
#include <kernel/dpl/DebugP.h>
#include <drivers/hw_include/hw_types.h>
#include <drivers/soc.h>
#include <drivers/ddr.h>
#include <drivers/ddr/v0/cdn_drv.h>

/* ========================================================================== */
/*                             Macros & Typedefs                              */
/* ========================================================================== */

#define DDR_CTL_REG_OFFSET              (0)
#define DDR_SRAM_MAX                    (512U)

/* ========================================================================== */
/*                 Internal Function Declarations                             */
/* ========================================================================== */


/* ========================================================================== */
/*                         Global Variables Declarations                      */
/* ========================================================================== */

static LPDDR4_Config gLpddrCfg;
static LPDDR4_PrivateData gLpddrPd;

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

static int32_t DDR_setClock(uint64_t freq)
{
    int32_t status = SystemP_SUCCESS;
    uint32_t module = TISCI_DEV_DDR16SS0;
    uint32_t clock = TISCI_DEV_DDR16SS0_DDRSS_DDR_PLL_CLK;

    status = SOC_moduleSetClockFrequency(module, clock, freq);
    if(status != SystemP_SUCCESS)
    {
        DebugP_logError("SOC_moduleSetClockFrequency failed !!!\n");
    }

    return status;
}

static int32_t DDR_probe(void)
{
    uint32_t status = 0U;
    uint16_t configsize = 0U;
    int32_t ret = SystemP_SUCCESS;

    status = LPDDR4_Probe(&gLpddrCfg, &configsize);
    if ((status != 0) || (configsize != sizeof(LPDDR4_PrivateData)) ||
        (configsize > DDR_SRAM_MAX))
    {
        DebugP_logError("LPDDR4_Probe failed !!!\n");
        ret = SystemP_FAILURE;
    }

    return ret;
}

static int32_t DDR_initDrv(void)
{
    uint32_t status = 0U;
    int32_t ret = SystemP_SUCCESS;

    if ((sizeof(gLpddrPd) != sizeof(LPDDR4_PrivateData)) ||
        (sizeof(gLpddrPd) > DDR_SRAM_MAX))
    {
        DebugP_logError("Invalid parameters !!!\n");
        ret = SystemP_FAILURE;
    }

    if(ret == SystemP_SUCCESS)
    {
        gLpddrCfg.ctlBase = (struct LPDDR4_CtlRegs_s *)CSL_DDR16SS0_CTL_CFG_BASE;
        gLpddrCfg.infoHandler = NULL;

        status = LPDDR4_Init(&gLpddrPd, &gLpddrCfg);
        if ((status > 0U) ||
            (gLpddrPd.ctlBase != (struct LPDDR4_CtlRegs_s *)gLpddrCfg.ctlBase) ||
            (gLpddrPd.ctlInterruptHandler != gLpddrCfg.ctlInterruptHandler) ||
            (gLpddrPd.phyIndepInterruptHandler != gLpddrCfg.phyIndepInterruptHandler))
        {
            DebugP_logError("LPDDR4_Init failed !!!\n");
            ret = SystemP_FAILURE;
        }
    }

    return ret;
}

static int32_t DDR_initHwRegs(DDR_Params *prms)
{
    uint32_t status = 0U;
    int32_t ret = SystemP_SUCCESS;

    status = LPDDR4_WriteCtlConfig(&gLpddrPd,
                                    prms->ddrssCtlReg,
                                    prms->ddrssCtlRegNum,
                                    prms->ddrssCtlRegCount);
    if (status == SystemP_SUCCESS)
    {
        status = LPDDR4_WritePhyIndepConfig(&gLpddrPd,
                                            prms->ddrssPhyIndepReg,
                                            prms->ddrssPhyIndepRegNum,
                                            prms->ddrssPhyIndepRegCount);
    }
    if (status == SystemP_SUCCESS)
    {
        status = LPDDR4_WritePhyConfig(&gLpddrPd,
                                        prms->ddrssPhyReg,
                                        prms->ddrssPhyRegNum,
                                        prms->ddrssPhyRegCount);
    }

    if (status != SystemP_SUCCESS)
    {
        DebugP_logError("DDR config write failed !!!\r\n");
        ret = SystemP_FAILURE;
    }
    return ret;
}

static int32_t DDR_start(void)
{
    uint32_t status = 0U;
    uint32_t regval = 0U;
    uint32_t offset = 0U;
    int32_t ret = SystemP_SUCCESS;

    offset = DDR_CTL_REG_OFFSET;

    status = LPDDR4_ReadReg(&gLpddrPd, LPDDR4_CTL_REGS, offset, &regval);
    if ((status > 0U) || ((regval & 0x1U) != 0U))
    {
        DebugP_logError("LPDDR4_ReadReg failed !!!\r\n");
        ret = SystemP_FAILURE;
    }

    if(ret == SystemP_SUCCESS)
    {
        status = LPDDR4_Start(&gLpddrPd);
        if (status > 0U)
        {
            DebugP_logError("LPDDR4_Start failed !!!\r\n");
            ret = SystemP_FAILURE;
        }

        if(ret == SystemP_SUCCESS)
        {
            status = LPDDR4_ReadReg(&gLpddrPd, LPDDR4_CTL_REGS, offset, &regval);
            if ((status > 0U) || ((regval & 0x1U) != 1U))
            {
                DebugP_logError("LPDDR4_ReadReg failed !!!\r\n");
                ret = SystemP_FAILURE;
            }
        }
    }

    return ret;
}

int32_t DDR_init(DDR_Params *prm)
{
    int32_t status = SystemP_SUCCESS;

    /* power and clock to DDR and EMIF is done form outside using SysConfig */

    /* Configure MSMC2DDR Bridge Control register. Configure REGION_IDX, SDRAM_IDX and SDRAM_3QT.*/
    HW_WR_REG32((CSL_DDR16SS0_SS_CFG_BASE + 0x20), 0x1EF);

    /* Configure DDRSS_ECC_CTRL_REG register. Disable ECC. */
    HW_WR_REG32((CSL_DDR16SS0_SS_CFG_BASE + 0x120), 0x00);

    status = DDR_probe();
    if(status == SystemP_SUCCESS)
    {
        status = DDR_initDrv();
    }
    if(status == SystemP_SUCCESS)
    {
        status = DDR_initHwRegs(prm);
    }
    if(status == SystemP_SUCCESS)
    {
        status = DDR_setClock(prm->clkFreq);
    }
    if(status == SystemP_SUCCESS)
    {
        status = DDR_start();
    }

    return status;
}


void DDR_Params_init(DDR_Params *prms)
{
    memset((void*)prms, 0, sizeof(DDR_Params));
}
