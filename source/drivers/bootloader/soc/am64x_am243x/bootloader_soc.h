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

#ifndef BOOTLOADER_SOC_AM64X_H_
#define BOOTLOADER_SOC_AM64X_H_

#include <drivers/hw_include/cslr_soc.h>

/**
 * \brief Data structure containing information about a core specific to the AM64x SOC
 *
 * This structure is used to store the data about cores in the SoC in the form of a lookup table which will be
 * used by various APIs.
 *
 */
typedef struct
{
	uint32_t tisciProcId;
	uint32_t tisciDevId;
	uint32_t tisciClockId;
	uint32_t defaultClockHz;
	char coreName[8];

} Bootloader_CoreBootInfo;

/**
 * \brief Request for a particular CPU in the AM64x SOC
 *
 * This API internally makes Sciclient calls to request control of the CPU
 *
 * \param cpuId [in] The CSL ID of the core
 *
 * \return SystemP_SUCCESS on success, else failure
 */
int32_t  Bootloader_socCpuRequest(uint32_t cpuId);

/**
 * \brief Release a particular CPU in the AM64x SOC
 *
 * This API internally makes Sciclient calls to release control of the CPU
 *
 * \param cpuId [in] The CSL ID of the core
 *
 * \return SystemP_SUCCESS on success, else failure
 */
int32_t  Bootloader_socCpuRelease(uint32_t cpuId);

/**
 * \brief Set the clock of a particular CPU in the AM64x SOC
 *
 * This API internally makes Sciclient calls to set CPU clock
 *
 * \param cpuId [in] The CSL ID of the core
 * \param cpuHz [in] Desired clock frequency of the CPU in Hertz
 *
 * \return SystemP_SUCCESS on success, else failure
 */
int32_t  Bootloader_socCpuSetClock(uint32_t cpuId, uint32_t cpuHz);

/**
 * \brief Get the clock of a particular CPU in the AM64x SOC
 *
 * This API internally makes Sciclient calls to get the current clock frequency of CPU
 *
 * \param cpuId [in] The CSL ID of the core
 *
 * \return Current clock speed of the CPU
 */
uint64_t Bootloader_socCpuGetClock(uint32_t cpuId);

/**
 * \brief Get the default clock of a particular CPU in the AM64x SOC
 *
 * This API queries and internal lookup table to fetch the default clock speed at which a
 * particular CPU should run.
 *
 * \param cpuId [in] The CSL ID of the core
 *
 * \return Default clock speed of the CPU
 */
uint32_t Bootloader_socCpuGetClkDefault(uint32_t cpuId);

/**
 * \brief Do power-on-reset of a particular CPU in the AM64x SOC
 *
 * This API is called only when booting a non-self CPU.
 *
 * \param cpuId [in] The CSL ID of the core
 *
 * \return SystemP_SUCCESS on success, else failure
 */
int32_t  Bootloader_socCpuPowerOnReset(uint32_t cpuId);

/**
 * \brief Release a particular CPU in the AM64x SOC from reset
 *
 * This API is called only when booting a non-self CPU. There is a different
 * API \ref Bootloader_socCpuResetReleaseSelf in the case of a self CPU
 *
 * \param cpuId       [in] The CSL ID of the core
 * \param entryPoint [in] The entryPoint of the CPU, from where it should start execution
 *
 * \return SystemP_SUCCESS on success, else failure
 */
int32_t  Bootloader_socCpuResetRelease(uint32_t cpuId, uintptr_t entryPoint);

/**
 * \brief Release self CPU in the AM64x SOC from reset
 *
 * \return SystemP_SUCCESS on success, else failure
 */
int32_t  Bootloader_socCpuResetReleaseSelf();

/**
 * \brief Set entry point for self CPU in the AM64x SOC from reset
 *
 * This API need not be called when booting a non-self CPU. The entry point can be specified
 * in the \ref Bootloader_socCpuResetRelease function itself
 *
 * \param cpuId      [in] The CSL ID of the core
 * \param entryPoint [in] The entryPoint of the CPU, from where it should start execution
 *
 * \return SystemP_SUCCESS on success, else failure
 */
int32_t  Bootloader_socCpuSetEntryPoint(uint32_t cpuId, uintptr_t entryPoint);

/**
 * \brief Loads the System Controller Firmware (SYSFW) onto the Cortex M3 in AM64x SOC
 *
 * The SYSFW is a special controller firmware loaded onto the Device Management and Security Controller
 * (DMSC) core, i.e the Cortex M3. The SYSFW controls a multitude of things in the SOC, including Power,
 * Clock, Resource Management and Security. So it is important to load the SYSFW onto the DMSC core before
 * booting any application. Typically this API is called before anything else in the main loop of a
 * Bootloader application
 *
 */
void Bootloader_socLoadSysFw(void);

/**
 * \brief Translate a CPU address to the SOC address wherever applicable
 *
 * This API need not be called when booting a non-self CPU. The entry point can be specified
 * in the \ref Bootloader_socCpuResetRelease function itself
 *
 * \param cslCoreId [in] The CSL ID of the core
 * \param addr      [in] The CPU addr
 *
 * \return SystemP_SUCCESS on success, else failure
 */
uint32_t Bootloader_socTranslateSectionAddr(uint32_t cslCoreId, uint32_t addr);

/**
 * \brief Obtain the CSL core ID of a CPU from its RPRC core ID
 *
 * \param rprcCoreId [in] The RPRC ID of the core
 *
 * \return CSL core ID of a CPU
 */
uint32_t Bootloader_socRprcToCslCoreId(uint32_t rprcCoreId);

/**
 * \brief Get the list of self cpus in the SOC.
 *
 * \return List of self cpus ending with an invalid core id
 */
uint32_t* Bootloader_socGetSelfCpuList(void);

/**
 * \brief Get the name of a core
 *
 * \param cpuId [in] The CSL ID of the core
 *
 * \return Name of the CPU
 */
char* Bootloader_socGetCoreName(uint32_t cpuId);

/**
 * \brief Initialize the core memories of a specific core
 *
 * \param cpuId [in] The CSL ID of the core
 *
 * \return SystemP_SUCCESS on success, else failure
 */
int32_t Bootloader_socMemInitCpu(uint32_t cpuId);

/**
 * \brief API to trigger the security handover from SYSFW
 *
 * \return SystemP_SUCCESS on success, else failure
 */
int32_t Bootloader_socSecHandover(void);

/**
 * \brief Loads the System Controller Firmware (SYSFW) onto the Cortex M3 in AM64x SOC (When SBL boots Linux)
 *
 * The SYSFW is a special controller firmware loaded onto the Device Management and Security Controller
 * (DMSC) core, i.e the Cortex M3. The SYSFW controls a multitude of things in the SOC, including Power,
 * Clock, Resource Management and Security. So it is important to load the SYSFW onto the DMSC core before
 * booting any application. Typically this API is called before anything else in the main loop of a
 * Bootloader application
 *
 */
void Bootloader_socLoadSysFwLinux(void);

/**
 * \brief API to get the scratch memory limits used by SBL. If the application tries to load in this region, it might over write SBL
 */
void Bootloader_socGetSBLMem(uint32_t *start, uint32_t *end);

#endif /* BOOTLOADER_SOC_AM64X_H_ */