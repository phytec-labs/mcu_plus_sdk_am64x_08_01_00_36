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
 *  \defgroup DRV_MPU_FIREWALL_MODULE APIs for MPU Firewall
 *  \ingroup DRV_MODULE
 *
 *  This module contains APIs to program the MPU Firewall module.
 *
 *  @{
 */

/**
 *  \file v0/mpu_firewall.h
 *
 *  \brief MPU Firewall Driver API/interface file.
 */

#ifndef MPU_FIREWALL_H_
#define MPU_FIREWALL_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <drivers/hw_include/cslr_soc.h>
#include <drivers/mpu_firewall/v0/cslr_mpu.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/**
 *  \brief  MPU Region Parameters
 *
 *  This structure contains MPPA configuration prarameters
 *  of a MPU Firewall region.
 */
typedef struct
{
    /**< Firewall ID */
    uint32_t  id ;
    /**< Region number in a particular firewall */
    uint32_t  regionNumber ;
    /**< Start address of a firewall region */
    uint32_t startAddress ;
    /**< End address of a firewall region */
    uint32_t endAddress ;
    /**< AID configuration - It is a bit mask with each bit denoting an AID
    An AID is allowed if the value in bit position allocated for it is 1  */
    uint32_t aidConfig ;
    /**< External AID configuration - 0->not allowed, 1->allowed */
    uint8_t aidxConfig ;
    /**< Supervisor read permission - 0->not allowed, 1->allowed */
    uint8_t supervisorReadConfig ;
    /**< Supervisor write permission - 0->not allowed, 1->allowed */
    uint8_t supervisorWriteConfig ;
    /**< Supervisor Execute permission - 0->not allowed, 1->allowed */
    uint8_t supervisorExecConfig ;
    /**< User read permission - 0->not allowed, 1->allowed */
    uint8_t userReadConfig ;
    /**< User write permission - 0->not allowed, 1->allowed */
    uint8_t userWriteConfig ;
    /**< User Execute permission - 0->not allowed, 1->allowed */
    uint8_t userExecConfig ;
    /**< Non secure access permission - 0->not allowed, 1->allowed */
    uint8_t nonSecureConfig ;
    /**< Debug permission - 0->not allowed, 1->allowed */
    uint8_t debugConfig ;

}MPU_FIREWALL_RegionParams;

/**
 *  \brief  MPU Firewall Parameters
 *
 *  This structure contains parameters associated with an MPU Firewall.
 */
typedef struct
{
    /**< Base address of the firewall */
    uint32_t  baseAddr ;
    /**< Number of regions in the firewall */
    uint32_t  numRegions ;

}MPU_FIREWALL_Config;

/* ========================================================================== */
/*                       Function Declarations                                */
/* ========================================================================== */

/**
 *  \brief  This function initializes the MPU Firewall module
 */
void MPU_FIREWALL_init();

/**
 *  \brief  Function to configure a Firewall region.
 *
 *  \param  mpuParams      Structure containing all region configuration parameters.
 *
 *  \return #SystemP_SUCCESS on successful region configuration; else error on failure
 */
int32_t MPU_FIREWALL_setRegion(MPU_FIREWALL_RegionParams* mpuParams);

/**
 *  \brief  Function to get parameters of a firewall.
 *
 *  \param  firewallId      Firewall ID.
 *  \param  firewallConfig  Structure to save firewall config.
 *
 *  \return #SystemP_SUCCESS on successful config read; else error on failure
 */
int32_t MPU_FIREWALL_getFirewallConfig(uint32_t firewallId, MPU_FIREWALL_Config* firewallConfig);

/**
 *  \brief  Function to initialize the parameters of a region.
 *
 *  \param  mpuParams      Region parameter structure to be initialized.
 */
void MPU_FIREWALL_RegionParams_init(MPU_FIREWALL_RegionParams* mpuParams);

/**
 *  \brief  Function to read the fault address that created the firewall
 *          violation.
 *
 *  \param  baseAddr      MPU Firewall base address
 *
 *  \return Fault address
 */
uint32_t MPU_FIREWALL_readFaultAddress (uint32_t baseAddr);

/**
 *  \brief  Function to read the fault status register. It contains information
 *          on the kind of firewall violation that had occurred.
 *
 *  \param  baseAddr      MPU Firewall base address
 *
 *  \return Value of fault status register
 */
uint32_t MPU_FIREWALL_readFaultStatus (uint32_t baseAddr);

/**
 *  \brief  Function to clear MPU Firewall fault address and
 *          fault status register .
 *
 *  \param  baseAddr      MPU Firewall base address
 */
void MPU_FIREWALL_clearFault (uint32_t baseAddr);

/**
 *  \brief  Function to enable MPU Firewall interrupt.
 *
 *  \param  baseAddr      MPU Firewall base address
 *  \param  flag          Flag to denote the different firewall interrupts
 *                        to be enabled.
 */
void MPU_FIREWALL_interruptEnable (uint32_t baseAddr, uint32_t flag);

/**
 *  \brief  Function to disable MPU Firewall interrupt.
 *
 *  \param  baseAddr      MPU Firewall base address
 *  \param  flag          Flag to denote the different firewall interrupts
 *                        to be disabled.
 */
void MPU_FIREWALL_clearInterruptEnable (uint32_t baseAddr, uint32_t flag);

/**
 *  \brief  Function to set interrupt. This can be used for testing interrupts.
 *
 *  \param  baseAddr      MPU Firewall base address
 *  \param  flag          Flag to denote the different firewall interrupts
 *                        to be set.
 */
void MPU_FIREWALL_setInterruptStatus (uint32_t baseAddr, uint32_t flag);

/**
 *  \brief  Function to read interrupt status.
 *
 *  \param  baseAddr      MPU Firewall base address
 *
 *  \return Value of interrupt status register.
 */
uint32_t MPU_FIREWALL_getInterruptStatus (uint32_t baseAddr);

/**
 *  \brief  Function to clear interrupt status.
 *
 *  \param  baseAddr      MPU Firewall base address
 *  \param  flag          Flag to denote the different firewall interrupts
 *                        to be cleared.
 */
void MPU_FIREWALL_clearInterruptStatus (uint32_t baseAddr, uint32_t flag);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef MPU_FIREWALL_H_ */

/** @} */