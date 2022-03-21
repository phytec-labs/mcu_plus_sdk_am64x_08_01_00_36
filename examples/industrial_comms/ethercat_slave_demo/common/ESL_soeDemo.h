//+=============================================================================================
//|
//!        \file ESL_soeDemo.h
//!
//!        SoE Protocol Application Example.
//|
//+---------------------------------------------------------------------------------------------
//|
//|        File-ID:        $Id: $
//|        Location:        $URL: $
//|        Company:        $Cpn: KUNBUS GmbH $
//|
//+---------------------------------------------------------------------------------------------
//|
//|        Files required:    --
//|
//+=============================================================================================



#if !(defined __ESL_SOEDEMO_H__)
#define __ESL_SOEDEMO_H__		1

#include <ecSlvApi.h>
#include <ecApiError.h>
#include <ecApiDef.h>
#include <stdint.h>
#include <string.h>

#if (defined __cplusplus)
extern "C" {
#endif

extern void     EC_SLV_APP_SoE_recv       (uint16_t   soEService_p
                                            ,uint16_t   soEFlags_p
                                            ,void*      pData_p
                                            ,uint16_t*  pLen_p);
extern uint8_t  EC_SLV_APP_SoE_nofReq     (void);

#if (defined __cplusplus)
}
#endif



#endif /* __ESL_SOEDEMO_H__ */
