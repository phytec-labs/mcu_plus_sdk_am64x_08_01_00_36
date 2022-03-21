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

#ifndef TI_DRVIERS_OPEN_CLOSE_H_
#define TI_DRVIERS_OPEN_CLOSE_H_

#include <stdint.h>
#include "ti_drivers_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Common Functions
 */
void Drivers_open(void);
void Drivers_close(void);

/*
 * FreeRTOS+FAT
 */
#include <fs/freertos_fat/FreeRTOS-FAT/include/ff_headers.h>
#include <fs/freertos_fat/portable/ff_mmcsd.h>
#include <fs/freertos_fat/FreeRTOS-FAT/include/ff_stdio.h>

/* FF Disks */
extern FF_Disk_t gFFDisks[CONFIG_FF_NUM_INSTANCES];
/* Readable Macros for partition indexes */
#define FF_PARTITION_SD0 (0U)

void Drivers_ffOpen(void);
void Drivers_ffClose(void);
/*
 * MMCSD
 */
#include <drivers/mmcsd.h>

/* MMCSD Driver handles */
extern MMCSD_Handle gMmcsdHandle[CONFIG_MMCSD_NUM_INSTANCES];

/*
 * MMCSD Driver Advance Parameters - to be used only when Driver_open() and
 * Driver_close() is not used by the application
 */
/* MMCSD Driver Parameters */
extern MMCSD_Params gMmcsdParams[CONFIG_MMCSD_NUM_INSTANCES];
/* MMCSD Driver open/close - can be used by application when Driver_open() and
 * Driver_close() is not used directly and app wants to control the various driver
 * open/close sequences */
void Drivers_mmcsdOpen(void);
void Drivers_mmcsdClose(void);

/*
 * UART
 */
#include <drivers/uart.h>

/* UART Driver handles */
extern UART_Handle gUartHandle[CONFIG_UART_NUM_INSTANCES];

/*
 * UART Driver Advance Parameters - to be used only when Driver_open() and
 * Driver_close() is not used by the application
 */
/* UART Driver Parameters */
extern UART_Params gUartParams[CONFIG_UART_NUM_INSTANCES];
/* UART Driver open/close - can be used by application when Driver_open() and
 * Driver_close() is not used directly and app wants to control the various driver
 * open/close sequences */
void Drivers_uartOpen(void);
void Drivers_uartClose(void);


#ifdef __cplusplus
}
#endif

#endif /* TI_DRVIERS_OPEN_CLOSE_H_ */
