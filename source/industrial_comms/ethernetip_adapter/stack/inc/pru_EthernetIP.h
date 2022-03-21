/*!
* \file pru_EthernetIP.h
*
* \brief
* PRU Integration: EthernetIP specific interface.
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

#if !(defined __PRU_ETHERNETIP_H__)
#define __PRU_ETHERNETIP_H__		1

#include <stdint.h>
#include <pru.h>

#define PRU_EIP_MAC_ADDR_LEN             6

#define PRU_EIP_RX_INT_NUM               20
#define PRU_EIP_LINK_INT_NUM             26

#define PRU_EIP_TX_QUEUE_0               0         //! used for PTP/DLR or IP Frames
#define PRU_EIP_TX_QUEUE_1               1         //! used for PTP/DLR or IP Frames
#define PRU_EIP_TX_QUEUE_2               2         //! used for PTP/DLR or IP Frames
#define PRU_EIP_TX_QUEUE_3               3         //! used for IP related frames Low Priority

#define PRU_EIP_TX_PORT_1                1         //! Ethernet Port to send telegramm
#define PRU_EIP_TX_PORT_2                2         //! Ethernet Port to send telegramm

#if   defined (SOC_K2G)
    #define ARM_INTERRUPT_OFFSET_ICSS0 236
    #define ARM_INTERRUPT_OFFSET_ICSS1 244
#else
    #if defined (__aarch64__)
        /* A53 */
        #define ARM_INTERRUPT_OFFSET_ICSS0 (286)
        #define ARM_INTERRUPT_OFFSET_ICSS1 (294)
        #define ARM_INTERRUPT_OFFSET_ICSS2 (302)
    #else
        /* R5F */
        #define ARM_INTERRUPT_OFFSET_ICSS0 (120)
        #define ARM_INTERRUPT_OFFSET_ICSS1 (248)
    #endif
#endif

#define MCU_getARMInterruptOffset(pruInst) \
    ( (PRUICSS_INSTANCE_ONE == (pruInst))?((int16_t)ARM_INTERRUPT_OFFSET_ICSS0):((int16_t)ARM_INTERRUPT_OFFSET_ICSS1) )

typedef enum
{
    PRU_EIP_enPORT_INVALID,
    PRU_EIP_enPORT_DEFAULT,
    PRU_EIP_enPORT_1,
    PRU_EIP_enPORT_2,
    PRU_EIP_enPORT_ALL,
    PRU_EIP_enPORT_FORCE32BIT = 0xffffffff,
} PRU_EIP_EEthernetPort;

typedef enum
{
    PRU_EIP_enLST_INVALID,
    PRU_EIP_enLST_UP,
    PRU_EIP_enLST_DOWN,
    PRU_EIP_enLST_FORCE32BIT = 0xffffffff
} PRU_EIP_ELinkState;

typedef enum
{
    PRU_EIP_enPHS_INVALID,
    PRU_EIP_enPHS_10MB,
    PRU_EIP_enPHS_100MB,
    PRU_EIP_enPHS_1GB,
    PRU_EIP_enPHS_FORCE32BIT = 0xffffffff
} PRU_EIP_EPhySpeed;

typedef enum
{
    PRU_EIP_enPHM_INVALID,
    PRU_EIP_enPHM_HALF,
    PRU_EIP_enPHM_FULL,
    PRU_EIP_enPHM_FORCE32BIT = 0xffffffff
} PRU_EIP_EPhyDuplexMode;

typedef enum
{
    PRU_EIP_ePHY_MDI_MANUAL_CONFIG = 0x00,
    PRU_EIP_ePHY_MDIX_MANUAL_CONFIG = 0x01,
    PRU_EIP_ePHY_MDIX_AUTO_CROSSOVER = 0x02,
    PRU_EIP_ePHY_MDIX_FORCE32BIT = 0xffffffff
} PRU_EIP_ePhyMdiMode_t;

typedef struct
{
    bool                  active;
    bool                  autoNeg;
    uint8_t               speed;
    bool                  fullDuplex;
    PRU_EIP_ePhyMdiMode_t mdix;
} PRU_EIP_TPhyConfiguration_t;

typedef void (*PRU_EIP_TInterruptFunc) (void *pvParam_p);
typedef void (*PRU_EIP_TLinkInterruptFunc) (void *pvParam_p, PRU_EIP_ELinkState enLinkP1_p, PRU_EIP_ELinkState enLinkP2_p);
typedef void (*PRU_EIP_TRxFrameInterruptFunc) (void *pvParam_p, int32_t i32sPacketLength_p, uint32_t i32uQueue_p);

typedef struct PRU_EIP_SPruLoadParameter
{
    bool                        quickConnectEnabled;
    uint8_t                     aMacAddr[PRU_EIP_MAC_ADDR_LEN];
    PRU_EIP_TPhyConfiguration_t tPhyConfig[2];
} PRU_EIP_TPruLoadParameter;

typedef struct PRU_EIP_SPortState
{
    PRU_EIP_ELinkState enLink;
    PRU_EIP_EPhySpeed enSpeed;
    PRU_EIP_EPhyDuplexMode enMode;
} PRU_EIP_TPortState;

typedef struct PRU_EIP_SInterfaceCounters
{
    uint32_t i32uInOctets;
    uint32_t i32uInUnicastPackets;
    uint32_t i32uInNonUnicastPackets;
    uint32_t i32uInDiscards;
    uint32_t i32uInErrors;
    uint32_t i32uInUnknownProtos;

    uint32_t i32uOutOctets;
    uint32_t i32uOutUnicastPackets;
    uint32_t i32uOutNonUnicastPackets;
    uint32_t i32uOutDiscards;
    uint32_t i32uOutErrors;
} PRU_EIP_TInterfaceCounters;

typedef struct PRU_EIP_SMediaCounters
{
    uint32_t i32uAlignmentErrors;
    uint32_t i32uFCSErrors;
    uint32_t i32uSingleCollisions;
    uint32_t i32uMultipleCollisions;
    uint32_t i32uSQETestErrors;
    uint32_t i32uDeferredTransmissions;
    uint32_t i32uLateCollisions;
    uint32_t i32uExcessiveCollisions;
    uint32_t i32uMACTransmitErrors;
    uint32_t i32uCarrierSenseErrors;
    uint32_t i32uFrameTooLong;
    uint32_t i32uMACReceiveErrors;
} PRU_EIP_TMediaCounters;

#if (defined __cplusplus)
extern "C" {
#endif

    extern uint32_t PRU_EIP_loadPru (PRU_EIP_TPruLoadParameter* ptPara_p, uint8_t logicPru);
    extern void PRU_EIP_stop(void);

    extern void PRU_EIP_registerRtRxFrameCb (PRU_EIP_TRxFrameInterruptFunc ptCbFunc_p, void *pvParam_p);
    extern void PRU_EIP_registerNrtRxFrameCb (PRU_EIP_TRxFrameInterruptFunc ptCbFunc_p, void *pvParam_p);

    extern void PRU_EIP_registerLinkStateChangeCb (PRU_EIP_TLinkInterruptFunc ptCbFunc_p, void *pvParam_p);
    extern uint32_t PRU_EIP_getIoFrame (uint32_t i32uQueue_p, uint8_t *pi8uBuffer_p, uint32_t i32uMaxBufLen_p, uint32_t *pi32uRecData_p, uint8_t *pi8Port_p);
    extern uint32_t PRU_EIP_sendTxPacket (const uint8_t *srcAddress, PRU_EIP_EEthernetPort enPort_p, uint8_t queuePriority, uint16_t lengthOfPacket);
    extern uint32_t PRU_EIP_getPortState (PRU_EIP_EEthernetPort enPort_p, PRU_EIP_TPortState *ptState_p);
    extern uint32_t PRU_EIP_getInterfaceCounters(PRU_EIP_EEthernetPort enPort_p, PRU_EIP_TInterfaceCounters *ptCounters_p);
    extern uint32_t PRU_EIP_getMediaCounters(PRU_EIP_EEthernetPort enPort_p, PRU_EIP_TMediaCounters* ptCounters_p);
    extern uint32_t PRU_EIP_clearInterfaceCounters(PRU_EIP_EEthernetPort enPort_p);
    extern uint32_t PRU_EIP_clearMediaCounters(PRU_EIP_EEthernetPort enPort_p);
    extern uint8_t PRU_EIP_getDlrTopology(void);
    extern uint8_t PRU_EIP_getDlrStatus(void);
    extern uint32_t PRU_EIP_getDlrSupIpAddr(void);
    extern uint8_t* PRU_EIP_getDlrSupMacAddr(void);


#if (defined __cplusplus)
}
#endif


#endif  // __PRU_ETHERNETIP_H__
