/*!
 *  \file ESL_version.c
 *
 *  \brief
 *  Brief description of purpose and functionality.
 *
 *  \author
 *  KUNBUS GmbH
 *
 *  \date
 *  2021-10-04
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
 *
 */

/* for snprintf */
#include <stdio.h>
#include <osal.h>
#include <ESL_version.h>

EC_API_EError_t ESL_setSWVersion(EC_API_SLV_SHandle_t* pEcSlave_p)
{
    EC_API_EError_t retVal      = EC_API_eERR_INVALID;
    char            aszVersion[sizeof("x.xx.xx")]   = {0};
    uint32_t        version;

    if (!pEcSlave_p)
    {
        goto Exit;
    }

#if (defined KUNBUS_EC_STACK_REVISION)
    version = KUNBUS_EC_STACK_REVISION;
#else
    version = EC_API_SLV_getVersion();
#endif

    snprintf(aszVersion, sizeof(aszVersion), "%01u.%02u.%02u",
             ((version >> 16) & 0xff),
             ((version >>  8) & 0xff),
             ((version >>  0) & 0xff)
             );

    retVal = (EC_API_EError_t)EC_API_SLV_setSwVersion(pEcSlave_p, aszVersion);
    if (retVal != EC_API_eERR_NONE)
    {
        OSAL_printf("%s:%d (%s) Error code: 0x%08x\r\n", __FUNCTION__, __LINE__, __FUNCTION__, retVal);
        goto Exit;
    }

Exit:
    return retVal;
}

//*************************************************************************************************
