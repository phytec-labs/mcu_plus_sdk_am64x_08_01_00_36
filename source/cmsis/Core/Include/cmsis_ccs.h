/**
 *  \file   cmsis_ccs.h
 *
 *  \brief  This file contains the TI specific intrisics for CMSIS.
 *
 */

/*
 * Copyright (C) 2020 Texas Instruments Incorporated - http://www.ti.com/
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the
 * distribution.
 *
 * Neither the name of Texas Instruments Incorporated nor the names of
 * its contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */


#ifndef __CMSIS_CCS_H
#define __CMSIS_CCS_H

#ifdef __ARM_FEATURE_DSP /* Defined by compiler for R5F target */
#define __CLZ                       __clz
#define __PKHBT(ARG1,ARG2,ARG3)     _pkhbt(ARG1,ARG2,ARG3)
#define __PKHTB(ARG1,ARG2,ARG3)     _pkhtb(ARG1,ARG2,ARG3)
#define __QADD                      _sadd
#define __QADD8                     _qadd8
#define __QADD16                    _qadd16
#define __QASX                      _qaddsubx
#define __QSAX                      _qsubaddx
#define __QSUB                      _ssub
#define __QSUB8                     _qsub8
#define __QSUB16                    _qsub16
#define __ROR                       __ror
#define __SHADD16                   _shadd16
#define __SHASX                     _shaddsubx
#define __SHSAX                     _shsubaddx
#define __SHSUB16                   _shsub16
#define __SMLAD                     _smlad
#define __SMLADX                    _smladx
#define __SMLALD(ARG1,ARG2,ARG3)    _smlald(ARG3,ARG1,ARG2)
#define __SMLALDX(ARG1,ARG2,ARG3)   _smlaldx(ARG3,ARG1,ARG2)
#define __SMLSDX                    _smlsdx
#define __SMMLA                     _smmla
#define __SMUAD                     _smuad
#define __SMUADX                    _smuadx
#define __SMUSD                     _smusd
#define __SMUSDX                    _smusdx
#define __SSAT(ARG1,ARG2)           _ssata(ARG1,0,ARG2)
#define __SXTB16(ARG1)              _sxtb16(ARG1,0)
#endif

#endif /* __CMSIS_ARMCC_H */
