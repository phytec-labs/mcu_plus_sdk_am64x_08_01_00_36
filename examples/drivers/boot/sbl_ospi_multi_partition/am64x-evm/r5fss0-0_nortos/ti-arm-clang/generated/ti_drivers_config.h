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

/*
 * Auto generated file - DO NOT MODIFY
 */

#ifndef TI_DRIVERS_CONFIG_H_
#define TI_DRIVERS_CONFIG_H_

#include <stdint.h>
#include <drivers/hw_include/cslr_soc.h>
#include "ti_dpl_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Common Functions
 */
void System_init(void);
void System_deinit(void);

/*
 * BOOTLOADER
 */
#include <drivers/bootloader.h>

/* BOOTLOADER Instance Macros */
#define CONFIG_BOOTLOADER_FLASH_R5FSS0_0 (0U)
#define CONFIG_BOOTLOADER_FLASH_R5FSS0_1 (1U)
#define CONFIG_BOOTLOADER_FLASH_R5FSS1_0 (2U)
#define CONFIG_BOOTLOADER_FLASH_R5FSS1_1 (3U)
#define CONFIG_BOOTLOADER_FLASH_M4FSS0_0 (4U)
#define CONFIG_BOOTLOADER_FLASH_A53SS0_0 (5U)
#define CONFIG_BOOTLOADER_NUM_INSTANCES (6U)

/*
 * DDR
 */
#include <drivers/ddr.h>

/*
 * UART
 */
#include <drivers/uart.h>

/* UART Instance Macros */
#define CONFIG_UART0 (0U)
#define CONFIG_UART_NUM_INSTANCES (1U)


#ifdef __cplusplus
}
#endif

#endif /* TI_DRIVERS_CONFIG_H_ */
