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

/**
 *  \file mpu_firewall.c
 *
 *  \brief File containing MPU Firewall Driver APIs implementation for version V0.
 *
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdint.h>
#include <string.h>
#include <kernel/dpl/SystemP.h>
#include <drivers/hw_include/hw_types.h>
#include <drivers/mpu_firewall/v0/mpu_firewall.h>

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

static void MPU_FIREWALL_setProgrammableStartAddress (uint32_t baseAddr, uint8_t regionNum, uint32_t address);
static void MPU_FIREWALL_setProgrammableEndAddress (uint32_t baseAddr, uint8_t regionNum, uint32_t address);
static uint32_t MPU_FIREWALL_getMPPAValue(MPU_FIREWALL_RegionParams* mpuParams);
static void MPU_FIREWALL_setMPPAValue (uint32_t baseAddr, uint8_t regionNum, uint32_t mppaRegValue);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

extern uint32_t gMpuFirewallNumRegions;
extern MPU_FIREWALL_RegionParams gMpuFirewallRegionConfig[];
extern MPU_FIREWALL_Config gMpuFirewallConfig[];

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

void MPU_FIREWALL_init()
{
    uint32_t i;
    /*
    * Initialize MPU Firewall regions
    */
    for (i = 0; i < gMpuFirewallNumRegions; i++)
    {
        MPU_FIREWALL_setRegion(&gMpuFirewallRegionConfig[i]);
    }
}

int32_t MPU_FIREWALL_setRegion(MPU_FIREWALL_RegionParams* mpuParams)
{
    uint32_t baseAddr;
    uint32_t regionNumber;
    uint32_t mppaRegValue;
    int32_t status = SystemP_FAILURE;
    MPU_FIREWALL_Config* firewallConfig = NULL;

    /* Get the firewall parameters */
    MPU_FIREWALL_getFirewallConfig(mpuParams->id, firewallConfig);

    /* Get the base address of the firewall to be configured */
    baseAddr = firewallConfig->baseAddr;

    /* Get the region number to be configured */
    regionNumber = mpuParams->regionNumber;

    /* Check if the region number is less than the total regions in firewall */
    if(regionNumber < firewallConfig->numRegions)
    {
        status = SystemP_SUCCESS;

        /* Start address configuration */
        MPU_FIREWALL_setProgrammableStartAddress (baseAddr,  regionNumber,
                                                mpuParams->startAddress);
        /* End Address configuration */
        MPU_FIREWALL_setProgrammableEndAddress (baseAddr,  regionNumber,
                                                mpuParams->endAddress);

        mppaRegValue = MPU_FIREWALL_getMPPAValue( mpuParams );
        /* MPPA configuration */
        MPU_FIREWALL_setMPPAValue(baseAddr, regionNumber, mppaRegValue);
    }
    return status;
}

int32_t MPU_FIREWALL_getFirewallConfig(uint32_t firewallId, MPU_FIREWALL_Config* firewallConfig)
{
    int32_t status = SystemP_FAILURE;

    /* Check if the firewal ID is less then the total number of firewalls in SOC */
    if(firewallId < CSL_FW_CNT)
    {
        status = SystemP_SUCCESS;
        firewallConfig = &gMpuFirewallConfig[firewallId];
    }
    return status;
}

void MPU_FIREWALL_RegionParams_init(MPU_FIREWALL_RegionParams* mpuParams)
{
    memset(mpuParams , 0, sizeof(*mpuParams));
}

static uint32_t MPU_FIREWALL_getMPPAValue(MPU_FIREWALL_RegionParams* mpuParams)
{
    /* Formulate the MPPA value from region parameters */
    uint32_t mppaValue =
          ((uint32_t)(mpuParams->aidConfig             & 0xFFFF) << 10)
        | ((uint32_t)(mpuParams->aidxConfig               & 0x1) <<  9)
        | ((uint32_t)(mpuParams->nonSecureConfig          & 0x1) <<  7)
        | ((uint32_t)(mpuParams->debugConfig              & 0x1) <<  6)
        | ((uint32_t)(mpuParams->supervisorReadConfig     & 0x1) <<  5)
        | ((uint32_t)(mpuParams->supervisorWriteConfig    & 0x1) <<  4)
        | ((uint32_t)(mpuParams->supervisorExecConfig     & 0x1) <<  3)
        | ((uint32_t)(mpuParams->userReadConfig           & 0x1) <<  2)
        | ((uint32_t)(mpuParams->userWriteConfig          & 0x1) <<  1)
        | ((uint32_t)(mpuParams->userExecConfig           & 0x1) <<  0);

    return mppaValue;
}

static void MPU_FIREWALL_setProgrammableStartAddress (uint32_t baseAddr, uint8_t regionNum, uint32_t address)
{
    /* Set Start address for the region */
    ((CSL_MpuRegs*) baseAddr)->PROG_REGION[regionNum].PROG_START_ADDRESS = address;
}

static void MPU_FIREWALL_setProgrammableEndAddress (uint32_t baseAddr, uint8_t regionNum, uint32_t address)
{
    /* Set End address for the region */
     ((CSL_MpuRegs*) baseAddr)->PROG_REGION[regionNum].PROG_END_ADDRESS = address;
}

static void MPU_FIREWALL_setMPPAValue (uint32_t baseAddr, uint8_t regionNum, uint32_t mppaRegValue)
{
    /* Write region configurations to the MPPA register */
    ((CSL_MpuRegs*) baseAddr)->PROG_REGION[regionNum].PROG_MPPA = mppaRegValue;
}

uint32_t MPU_FIREWALL_readFaultAddress (uint32_t baseAddr)
{
    uint32_t faultAddress = 0;

    /* Read the fault address register */
    faultAddress = HW_RD_REG32(&((CSL_MpuRegs*) baseAddr)->FAULT_ADDRESS);

    return faultAddress;
}

uint32_t MPU_FIREWALL_readFaultStatus (uint32_t baseAddr)
{
    uint32_t faultStatus = 0;

    /* Read the fault status register */
    faultStatus = HW_RD_REG32(&((CSL_MpuRegs*) baseAddr)->FAULT_STATUS);

    return faultStatus;
}

void MPU_FIREWALL_clearFault (uint32_t baseAddr)
{
    /* Clear the fault address and fault status register */
    HW_WR_REG32(&((CSL_MpuRegs*) baseAddr)->FAULT_CLEAR, (uint32_t)1U );
}

void MPU_FIREWALL_interruptEnable (uint32_t baseAddr, uint32_t flag)
{
    /* Enable interrupt */
    HW_WR_REG32(&((CSL_MpuRegs*) baseAddr)->INT_ENABLE, flag );
}

void MPU_FIREWALL_clearInterruptEnable (uint32_t baseAddr, uint32_t flag)
{
    /* Disable interrupt */
    HW_WR_REG32(&((CSL_MpuRegs*) baseAddr)->INT_ENABLE_CLEAR, flag );
}

void MPU_FIREWALL_setInterruptStatus (uint32_t baseAddr, uint32_t flag)
{
    /* Set interrupt status bit */
    HW_WR_REG32(&((CSL_MpuRegs*) baseAddr)->INT_RAW_STATUS_SET, flag );
}

uint32_t MPU_FIREWALL_getInterruptStatus (uint32_t baseAddr)
{
    uint32_t interruptStatus = 0;

    /* Read the interrupt status register */
    interruptStatus = HW_RD_REG32(&((CSL_MpuRegs*) baseAddr)->INT_RAW_STATUS_SET);

    return interruptStatus;
}

void MPU_FIREWALL_clearInterruptStatus (uint32_t baseAddr, uint32_t flag)
{
    /* Clear interrupt status bit */
    HW_WR_REG32(&((CSL_MpuRegs*) baseAddr)->INT_ENABLED_STATUS_CLEAR, flag );
}