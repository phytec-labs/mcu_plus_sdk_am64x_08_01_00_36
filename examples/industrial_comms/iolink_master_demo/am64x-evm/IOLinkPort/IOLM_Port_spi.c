/*!
* \file IOLM_Port_spi.c
*
* \brief
* Interface for SPI Communication on IOLink Board
*
* \author
* KUNBUS GmbH
*
* \date
* 2021-05-19
*
* \copyright
* Copyright (c) 2021, KUNBUS GmbH<br /><br />
* All rights reserved.<br />
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:<br />
* <ol>
* <li>Redistributions of source code must retain the above copyright notice, this
* list of conditions and the following disclaimer.</li>
* <li>Redistributions in binary form must reproduce the above copyright notice,
* this list of conditions and the following disclaimer in the documentation
* and/or other materials provided with the distribution.</li>
* <li>Neither the name of the copyright holder nor the names of its
* contributors may be used to endorse or promote products derived from
* this software without specific prior written permission.</li>
* </ol>
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*/

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <osal.h>
#include "IOLM_Port_spi.h"
#include <ti_drivers_config.h>
#include <drivers/mcspi.h>

/* ========================================================================== */
/*                          Local Variables                                   */
/* ========================================================================== */
static MCSPI_Handle     spiHandle_s = NULL;     /* SPI handle for all channels */

extern MCSPI_Handle gMcspiHandle[CONFIG_MCSPI_NUM_INSTANCES];

OSAL_SCHED_SMutexHandle_t*  IOLM_SPI_mutex_s;
uint32_t                    IOLM_SPI_activeChannel_s;


/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

/* documentation in header */
void IOLM_SPI_init(void)
{
    IOLM_SPI_mutex_s = OSAL_MTXCTRLBLK_alloc();
    OSAL_MTX_init(IOLM_SPI_mutex_s);
    
    if (NULL == spiHandle_s) /* check if handle has already been initialized */
    {
        spiHandle_s = gMcspiHandle[CONFIG_MCSPI_IOL];
    }
}

void IOLM_SPI_close(void)
{
    OSAL_MTXCTRLBLK_free(IOLM_SPI_mutex_s);
}

/* documentation in header */
uint32_t IOLM_SPI_transfer(uint32_t channel_p, uint32_t tx_p)
{
    MCSPI_Transaction transaction;  /* SPI transaction structure */
    uint32_t        txBuf = 0;
    uint32_t        rxBuf = 0;
    uint32_t        terminateXfer = 1;

    txBuf = (tx_p & 0xffff);  /* reduce tx to 16 bit width */

    if (channel_p < MCSPI_MAX_NUM_CHANNELS)
    {
        transaction.channel = channel_p;    /* 1 = led, 0 = iq */
    }
    else
    {
        goto laExit;
    }

    transaction.txBuf = (uint8_t*)&txBuf;
    transaction.rxBuf = (uint8_t*)&rxBuf;
    transaction.count = 1;  /* single frame */
    transaction.args = (void*)&terminateXfer;

    OSAL_MTX_get(IOLM_SPI_mutex_s, OSAL_WAIT_INFINITE, NULL);
    MCSPI_transfer(spiHandle_s, &transaction);
    OSAL_MTX_release(IOLM_SPI_mutex_s);

laExit:
    return rxBuf;
}

