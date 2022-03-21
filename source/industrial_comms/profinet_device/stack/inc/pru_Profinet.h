/*!
* \file pru_Profinet.h
*
* \brief
* PRU Integration: Profinet specific interface.
*
* \author
* KUNBUS GmbH
*
* \date
* 2021-05-20
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

#if !(defined __PRU_PROFINET_H__)
#define __PRU_PROFINET_H__		1

#include <stdint.h>

  #include <industrial_comms/profinet_device/icss_fwhal/PN_Handle.h>
  #include <industrial_comms/profinet_device/icss_fwhal/iPtcpDrv.h>

#define PRU_PN_MAC_ADDR_LEN             6

#if   defined (SOC_K2G)
    #define ARM_INTERRUPT_OFFSET_ICSS0 236
    #define ARM_INTERRUPT_OFFSET_ICSS1 244
#else
    #if defined (__aarch64__)
        /* A53 */
        #define ARM_INTERRUPT_OFFSET_ICSS0 (286-20)
        #define ARM_INTERRUPT_OFFSET_ICSS1 (294-20)
        #define ARM_INTERRUPT_OFFSET_ICSS2 (302-20)
    #else
        /* R5F */
        #define ARM_INTERRUPT_OFFSET_ICSS0 (120-20)
        #define ARM_INTERRUPT_OFFSET_ICSS1 (248-20)
    #endif
#endif

#define MCU_getARMInterruptOffset(pruInst) \
    ( (PRUICSS_INSTANCE_ONE == (pruInst))?((int16_t)ARM_INTERRUPT_OFFSET_ICSS0):((int16_t)ARM_INTERRUPT_OFFSET_ICSS1) )





#define PRU_PN_TX_QUEUE_0               0         //! used for profinet frames High Priority
#define PRU_PN_TX_QUEUE_1               1         //! used for profinet frames Low Priority
#define PRU_PN_TX_QUEUE_2               2         //! used for IP related frames High Priority
#define PRU_PN_TX_QUEUE_3               3         //! used for IP related frames Low Priority

#define PRU_PN_TX_PORT_1                1         //! Ethernet Port to send telegramm
#define PRU_PN_TX_PORT_2                2         //! Ethernet Port to send telegramm

typedef enum
{
    PRU_PN_enPORT_0,
    PRU_PN_enPORT_1,
    PRU_PN_enPORT_2,
    PRU_PN_enPORT_FORCE32BIT = 0xffffffff,
} PRU_PN_EEthernetPort;

typedef enum
{
    PRU_PN_enLST_INVALID,
    PRU_PN_enLST_UP,
    PRU_PN_enLST_DOWN,
    PRU_PN_enLST_FORCE32BIT = 0xffffffff
} PRU_PN_ELinkState;

typedef enum
{
    PRU_PN_enPHS_INVALID,
    PRU_PN_enPHS_10MB,
    PRU_PN_enPHS_100MB,
    PRU_PN_enPHS_1GB,
    PRU_PN_enPHS_FORCE32BIT = 0xffffffff
} PRU_PN_EPhySpeed;

typedef enum
{
    PRU_PN_enPHM_INVALID,
    PRU_PN_enPHM_HALF,
    PRU_PN_enPHM_FULL,
    PRU_PN_enPHM_FORCE32BIT = 0xffffffff
} PRU_PN_EPhyDuplexMode;

typedef void (*PRU_PN_TInterruptFunc) (void* pvParam_p);
typedef void (*PRU_PN_TLinkInterruptFunc) (void* pvParam_p, PRU_PN_ELinkState enLinkP1_p, PRU_PN_ELinkState enLinkP2_p);
typedef void (*PRU_PN_TRxFrameInterruptFunc) (void* pvParam_p, int32_t i32sPacketLength_p, uint32_t i32uQueue_p);
typedef void (*PRU_PN_ptcpDelayUpdate_t) (void *pvThis_p, PRU_PN_EEthernetPort enPort_p, uint32_t rxDelayLocal_p,
  uint32_t rxDelayRemote_p, uint32_t txDelayLocal_p, uint32_t txDelayRemote_p, uint32_t lineDelay_p);
typedef void (*PRU_PN_ptcpSyncStatus_t) (void *pvThis_p, syncState_t enSyncState_p, ptcpSyncInfo_t *ptSyncInfo_p);

typedef struct PRU_PN_SPruLoadParameter
{
    uint8_t ai8uMacAddr[PRU_PN_MAC_ADDR_LEN];
    int32_t i32LogicPruInstance;
} PRU_PN_TPruLoadParameter;

typedef struct PRU_PN_SPortState
{
    PRU_PN_ELinkState enLink;
    PRU_PN_EPhySpeed enSpeed;
    PRU_PN_EPhyDuplexMode enMode;
} PRU_PN_TPortState;

typedef struct PRU_PN_SPortStatistic
{
    uint32_t rxOctets;
    uint32_t txOctets;
    uint32_t rxDiscards;
    uint32_t txDiscards;
    uint32_t rxErrors;
    uint32_t txErrors;
} PRU_PN_SPortStatistic_t;


#if (defined __cplusplus)
extern "C" {
#endif

    extern uint32_t PRU_PN_loadPru (PRU_PN_TPruLoadParameter *ptPara_p);
    extern void PRU_PN_registerRxFrameCb (PRU_PN_TRxFrameInterruptFunc ptCbFunc_p, void* pvParam_p);
    extern void PRU_PN_registerPtcpDelayUpdate (PRU_PN_ptcpDelayUpdate_t cbDelay_p, void *pThis_p);
    extern void PRU_PN_registerPtcpSyncStatus (PRU_PN_ptcpSyncStatus_t cbDelay_p, void *pThis_p);
    extern void PRU_PN_registerTxPpmFrameCb (PRU_PN_TInterruptFunc ptCbFunc_p, void* pvParam_p);
    extern void PRU_PN_registerRxCpmFrameCb (PRU_PN_TInterruptFunc ptCbFunc_p, void* pvParam_p);
    extern void PRU_PN_registerDhtEventCb (PRU_PN_TInterruptFunc ptCbFunc_p, void* pvParam_p);
    extern void PRU_PN_registerPtcpEventCb (PRU_PN_TInterruptFunc ptCbFunc_p, void* pvParam_p);
    extern void PRU_PN_registerLinkStateChangeCb (PRU_PN_TLinkInterruptFunc ptCbFunc_p, void* pvParam_p);
    extern void PRU_PN_registerIsomEventCb (PRU_PN_TInterruptFunc ptCbFunc_p, void* pvParam_p);
    extern uint32_t PRU_PN_getIoFrame (uint32_t i32uQueue_p, uint8_t* pi8uBuffer_p, uint32_t i32uMaxBufLen_p, uint32_t* pi32uRecData_p, uint8_t *pi8Port_p);
    extern uint32_t PRU_PN_sendTxPacket (const uint8_t* srcAddress, PRU_PN_EEthernetPort enPort_p, uint8_t queuePriority, uint16_t lengthOfPacket);
    extern uint32_t PRU_PN_getPortState (PRU_PN_EEthernetPort enPort_p, PRU_PN_TPortState *ptState_p);
    extern PN_Handle PRU_PN_getPnHandle (void);
    extern PRU_PN_EEthernetPort PRU_PN_getPortFromMacAddr (const uint8_t *pi8uMacAddr_p);
    extern uint32_t PRU_PN_getPortStatistics (PRU_PN_EEthernetPort enPort_p, PRU_PN_SPortStatistic_t *ptState_p);

#if (defined __cplusplus)
}
#endif


#endif  // __PRU_PROFINET_H__
