/*!
* \file IOLM_Port_spi.h
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

#ifndef IO_LINK_IOLINK_SPI_H_
#define IO_LINK_IOLINK_SPI_H_

 /* ========================================================================== */
 /*                                Defines                                     */
 /* ========================================================================== */

#define IOLM_SPI_IQ_CHANNEL     (0U)
#define IOLM_SPI_LED_CHANNEL    (1U)

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */
#include <stdint.h>
#include "IOLM_Port_LEDTask.h"

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/**
 * \brief  Initialize the first SPI instance
 *
 *
 * \return void
 *
 */
void IOLM_SPI_init(void);

/**
 * \brief  Close the first SPI instance
 *
 *
 * \return void
 *
 */
void IOLM_SPI_close(void);

/**
 * \brief  Transfer one 16Bit frame over the first SPI interface
 *
 * \param  channel_p	multi channel SPI channel (0-IQ, 1-LED on AM64x)
 * \param  portNum_p    data to send
 *
 * \return received data
 *
 */
uint32_t IOLM_SPI_transfer(uint32_t channel_p, uint32_t tx_p);

#endif /* IO_LINK_IOLINK_SPI_H_ */
