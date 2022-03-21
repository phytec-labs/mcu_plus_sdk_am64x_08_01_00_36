/*!
 *  \example appHwError.h
 *
 *  \brief
 *  Error definitions for the hardware specific functions.
 *
 *  \author
 *  KUNBUS GmbH
 *
 *  \date
 *  2021-07-14
 *
 *  \copyright
 *  Copyright (c) 2021, KUNBUS GmbH<br /><br />
 *  All rights reserved.<br />
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:<br />
 *  <ol>
 *  <li>Redistributions of source code must retain the above copyright notice, this
 *     list of conditions and the following disclaimer.</li>
 *  <li>Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.</li>
 *  <li>Neither the name of the copyright holder nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.</li>
 *  </ol>
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 *  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 *  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 *  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef APP_HW_ERROR
#define APP_HW_ERROR

#ifdef __cplusplus
extern "C" {
#endif

/*!
 *  \brief Errors returned by hardware specific functions.
 */
typedef enum APP_HW_EError
{
    APP_HW_eNO_ERROR = 0,
    APP_HW_eINVALID_PARAM = -1,
    APP_HW_eFATAL_ERROR = -2
} APP_HW_EError_t;

#ifdef  __cplusplus
}
#endif

#endif // APP_HW_ERROR
