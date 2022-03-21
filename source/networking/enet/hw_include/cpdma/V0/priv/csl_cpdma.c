/**
 * @file  csl_cpdma.c
 *
 * @brief
 *  C implementation file for Ethernet CPDMA module CSL.
 *
 *  Contains the different control command and status query functions definations
 *
 *  \par
 *  ============================================================================
 *  @n   (C) Copyright 2020, Texas Instruments, Inc.
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
*/

#include <csl_cpdma.h>

void CSL_CPSW_getCpdmaTxVersionInfo (CSL_CpdmaRegs           *hCpdmaRegs,
                                     CSL_CPSW_CPDMA_VERSION  *pVersionInfo)
{
    Uint32 value = CSL_REG32_RD(hCpdmaRegs->TX_IDVER);

    pVersionInfo->minorVer  =   CSL_FEXT (value, CPDMA_TX_IDVER_TX_MINOR_VER);
    pVersionInfo->majorVer  =   CSL_FEXT (value, CPDMA_TX_IDVER_TX_MAJOR_VER);
    pVersionInfo->rtlVer    =   0U;
    pVersionInfo->id        =   CSL_FEXT (value, CPDMA_TX_IDVER_TX_IDENT);

    return;
}


void CSL_CPSW_getCpdmaRxVersionInfo (CSL_CpdmaRegs           *hCpdmaRegs,
                                     CSL_CPSW_CPDMA_VERSION  *pVersionInfo)
{
    Uint32 value = CSL_REG32_RD(hCpdmaRegs->RX_IDVER);

    pVersionInfo->minorVer  =   CSL_FEXT (value, CPDMA_RX_IDVER_RX_MINOR_VER);
    pVersionInfo->majorVer  =   CSL_FEXT (value, CPDMA_RX_IDVER_RX_MAJOR_VER);
    pVersionInfo->rtlVer    =   0U;
    pVersionInfo->id        =   CSL_FEXT (value, CPDMA_RX_IDVER_RX_IDENT);

    return;
}

void CSL_CPSW_configCpdma
(
    CSL_CpdmaRegs           *hCpdmaRegs,
    CSL_CPSW_CPDMA_CONFIG   *pConfig
)
{
    uint32_t  value = 0U;

    CSL_FINS (value,  CPDMA_DMACONTROL_TX_PTYPE, pConfig->txPtype);
    CSL_FINS (value,  CPDMA_DMACONTROL_RX_OWNERSHIP, pConfig->rxOnwBit);
    CSL_FINS (value,  CPDMA_DMACONTROL_RX_OFFLEN_BLOCK, pConfig->rxOffLenBlockEn);
    CSL_FINS (value,  CPDMA_DMACONTROL_CMD_IDLE, pConfig->idleCmd);
    CSL_FINS (value,  CPDMA_DMACONTROL_RX_CEF, pConfig->rxCEFEn);
    CSL_FINS (value,  CPDMA_DMACONTROL_RX_VLAN_ENCAP, pConfig->rxVLANEn);
    CSL_FINS (value,  CPDMA_DMACONTROL_RX_TS_ENCAP, pConfig->rxTSEn);
    CSL_FINS (value,  CPDMA_DMACONTROL_TX_RLIM, pConfig->txRlimType);
    hCpdmaRegs->DMACONTROL = value;
}

void CSL_CPSW_getCpdmaStatus
(
    CSL_CpdmaRegs           *hCpdmaRegs,
    CSL_CPSW_CPDMA_STATUS   *pStatusInfo
)
{
    Uint32 value = CSL_REG32_RD(hCpdmaRegs->DMASTATUS);

    pStatusInfo->rxErrCh    = CSL_FEXT (value, CPDMA_DMASTATUS_RX_ERR_CH);
    pStatusInfo->rxErrCode  = CSL_FEXT (value, CPDMA_DMASTATUS_RX_HOST_ERR_CODE);
    pStatusInfo->txErrCh    = CSL_FEXT (value, CPDMA_DMASTATUS_TX_ERR_CH);
    pStatusInfo->txErrCode  = CSL_FEXT (value, CPDMA_DMASTATUS_TX_HOST_ERR_CODE);
    pStatusInfo->idle       = CSL_FEXT (value, CPDMA_DMASTATUS_IDLE);
}
