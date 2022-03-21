/*
 *  Copyright (c) Texas Instruments Incorporated 2020
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

/*!
 * \file  enet_cpdma.h
 *
 * \brief This file contains the type definitions and helper macros for the
 *        Enet CPDMA data path (DMA) interface.
 */

/*!
 * \ingroup  ENET_DMA_API
 * \defgroup ENET_CPDMA_API Enet CPDMA APIs
 *
 * @{
 */

#ifndef ENET_CPDMA_H_
#define ENET_CPDMA_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdint.h>
#include <include/core/enet_types.h>
#include <include/core/enet_base.h>
#include <include/core/enet_queue.h>
#include <include/dma/cpdma/enet_cpdma_types.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                                 Macros                                     */
/* ========================================================================== */

/*!
 * \name Enet CPDMA instance configuration
 *
 * Configuration macros for Enet CPDMA module.
 *
 * @{
 */

/*! \brief Maximum number of CPSW TX DMA channels. */
#define ENET_CPDMA_CPSW_MAX_TX_CH                                (8U)

/*! \brief Maximum number of CPSW RX DMA channels. */
#define ENET_CPDMA_CPSW_MAX_RX_CH                                (8U)

/*! @} */

/*!
 * \brief Enet DMA configuration structure.
 *
 *  \name Enet DMA driver opaque handles
 *
 *  Opaque handle typedefs for Enet DMA driver objects.
 *  @{
 */

/*!
 * \brief Opaque handle that holds config Info for Enet DMA channel.
 */
typedef struct EnetCpdma_Cfg_s EnetDma_Cfg;

/* TODO: may be deleted */
/*!
 * \brief Function pointer type for packet notify call back.
 *
 * This is called by driver when packet is received on the RX channel or
 * transmission completed from TX channel.
 */
typedef void (*EnetDma_PktNotifyCb)(void *cbArg);

/*! @} */

/* ========================================================================== */
/*                         Structures and Enums                               */
/* ========================================================================== */

/*!
 * \defgroup ENET_CPDMA_PKT CPDMA Packet Definitions
 *
 * This group contains details about Enet CPDMA packet queue structures. These
 * packet queue's are used to exchange data between application and driver
 * module.
 *
 * @{
 */

/*
 * TODO: CPDMA does not support timestamp within packet
 *       Stub to share the application code?
 */

/*!
 * \brief CPPI buffer timestamp info.
 *
 * Buffer timestamp info structure.
 */
typedef struct EnetCpdma_PktTsInfo_s
{
    /*! Flag to enable/disable host TX packet timestamping */
    bool enableHostTxTs;

    /*! Host TX packet's sequence Id value */
    uint32_t txPktSeqId;

    /*! Host TX packet's message type value */
    uint8_t txPktMsgType;

    /*! Host TX packet's domain value */
    uint8_t txPktDomain;

    /*! Received packet's 64-bit ingress timestamp value */
    uint64_t rxPktTs;
} EnetCpdma_PktTsInfo;

/*!
 * \brief Packet data structure.
 *
 * This structure contains packet info used by application for getting/passing
 * the packet data with DMA module.
 */
typedef struct EnetCpdma_PktInfo_s
{
    /*! Pointer to next buffer.
     *  Note: Keep EnetQ_Node as first member always as driver uses generic
     *  queue functions and dereferences to this member */
    EnetQ_Node node;

    /*! Pointer to data buffer */
    uint8_t *bufPtr;

    /*! Original length of the data buffer passed */
    uint32_t orgBufLen;

    /*! Actual filled buffer length while receiving/transmitting data with DMA */
    uint32_t userBufLen;

    /*! Pointer to application layer specific data blob */
    void *appPriv;

    /*! Packet state info. Refer to #EnetDma_PktStateModuleType,
     *  #EnetDma_PktStateMemMgr, #EnetDma_PktStateDma and
     *  #EnetDma_PktStateApp.
     *
     *  This is only for runtime check and debug, not to be used by application */
    uint32_t pktState;

    /*! Protocol information word. Currently used for checksum offload related
     *  information.
     *
     *  For RX it contains HW computed checksum. App should call appropriate
     *  CPPI helper macros to extract required fields.
     *
     *  For TX it contains information passed to HW for checksum computation.
     *  App should call appropriate CPPI helper macros to insert required
     *  fields */
    uint32_t chkSumInfo;

    /*! Packet time stamp information.
     *
     * For TX, if tsInfo.enableHostTxTs flag is set to true, packet will be timestampped
     * on egress and will trigger host transmit event in CPTS. The timestamp value is then
     * stored in CPTS FIFO with given sequence Id, message type and domain value. This
     * can be used to timestamp any packet (even non-PTP) sent from host.
     *
     * For RX, the received packet's ingress timestamp is captured and is stored in
     * tsInfo.rxPktTs. All ingress packets are timestampped. */
    EnetCpdma_PktTsInfo tsInfo;

    /*! Directed port number.
     *
     *  The port number to which the packet is to be directed bypassing ALE lookup.
     *  This value is written to Dst Tag – Low bits of packet descriptor.
     *  If ALE lookup based switching is to be performed, this value should
     *  be set to ENET_MAC_PORT_INV.
     *
     *  Note: Directed packets go to the directed port, but an ALE lookup is performed to
     *  determine untagged egress in VLAN aware mode. */
    Enet_MacPort txPortNum;

    /*! Packet's received port number
     *  This variable contains the port number from which the packet was received.
     *  This value is obtained from the Source Tag – Low bits of packet descriptor. */
    Enet_MacPort rxPortNum;
} EnetCpdma_PktInfo;

/*! @} */


/*!
 * \defgroup ENET_CPDMA_CONFIG_DEFS CPDMA Configuration
 *
 * This group contains structure and type definitions needed to properly
 * construct the Enet CPDMA configuration structure which is part of the top-level
 * Enet CPDMA configuration structure.
 *
 * @{
 */

/*!
 * \brief Param struct for the TX channel open function.
 *
 * The configuration structure for the TX channel open function # EnetDma_openTxCh().
 */
typedef struct EnetCpdma_OpenTxChPrms_s
{
    /*! Enet handle*/
    Enet_Handle hEnet;

    /*! CPDMA channel number allocated for transmit. */
    uint32_t chNum;

    /*! Enet CPDMA event callback function - this function will be called when
     *  the registered packets are transmitted on TX channel */
    EnetDma_PktNotifyCb notifyCb;

    /*! Maximum number of transmit packets, used for allocating number of DMA descriptors
     */
    uint32_t numTxPkts;

    /*! Argument to be used for the callback routines (it should mean something
     *  to layer into which the callback calls) */
    void *cbArg;

} EnetCpdma_OpenTxChPrms;

/*!
 * \brief Param struct for the RX channel open function.
 *
 * The configuration structure for the RX channel open function #EnetDma_openTxCh().
 */
typedef struct EnetCpdma_OpenRxChPrms_s
{
    /*! Enet handle*/
    Enet_Handle hEnet;

    /*! CPDMA channel number allocated for receive. */
    uint32_t chNum;

    /*! Enet CPDMA event callback function - this function will be called when
     *  the registered packets are transmitted on TX channel */
    EnetDma_PktNotifyCb notifyCb;

    /*! Maximum number of transmit packets, used for allocating number of DMA descriptors
     */
    uint32_t numRxPkts;

    /*! Argument to be used for the callback routines (it should mean something
     *  to layer into which the callback calls) */
    void *cbArg;

} EnetCpdma_OpenRxChPrms;

/*!
 * \brief Param struct for the RX channel open function. We include this typedef as top level
 *        DMA APIs use EnetDma_OpenRxChPrms struct.
 */
typedef EnetCpdma_OpenRxChPrms EnetDma_OpenRxChPrms;

/*!
 * \brief Global Param struct for the Rx channel open
 *
 * The parameter structure for the Rx channel open, containing a global channel config
 * structure for all rx channels.
 */
typedef struct EnetCpdma_RxChInitPrms_s
{
    /*! This field selects which scheduling bin the channel will be placed in
     *  for bandwidth allocation of the Rx DMA units. */
    /* TODO: It may not be required anymore */
    uint8_t dmaPriority;

    /*! Receive buffer offset */
    uint32_t rxBufferOffset;
} EnetCpdma_RxChInitPrms;

/*!
 * \brief Config structure for Enet CPDMA
 *
 * The parameter structure for Enet CPDMA configuration, containing a Rx channel
 * config and NAVSS instance id.
 */
typedef struct EnetCpdma_Cfg_s
{
    /*! flag to indicate whether the descriptor area is cacheable */
    bool        isCacheable;

    /*! Number of rx Interrupts per millisecond */
    uint32_t rxInterruptPerMSec;

    /*! RX channel configuration parameters */
    EnetCpdma_RxChInitPrms rxChInitPrms;
} EnetCpdma_Cfg;

/*!
 * \brief Config structure for Enet CPDMA Data Path initialization
 *
 * The parameter strcture for Enet CPDMA data path init configuration.
 */
typedef struct EnetDma_initCfg_s
{
}
EnetDma_initCfg;
/*! @} */

/* ========================================================================== */
/*                         Global Variables Declarations                      */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */
/*!
 * \brief ENET CPDMA Rx Threshold interrupt service routine
 *
 * Processes Rx Threshold interrupt. This function retrieves the received packets
 * from the linked CPPI descriptors and passes them to the receive queue when the
 * number of received descriptors exceed the threshold
 *
 * Requirement:
 *
 * \param hEnetDma     [IN] Enet DMA Handle
 *
 * \return \ref Enet_ErrorCodes
 */
int32_t EnetCpdma_rxThreshIsr(EnetDma_Handle hEnetDma);

/*!
 * \brief ENET CPDMA Rx interrupt service routine
 *
 * Processes Rx interrupt. This function retrieves the received packets from
 * the linked CPPI descriptors and passes them to the receive queue when one
 * or more packets has been received
 *
 * Requirement:
 *
 * \param hEnetDma     [IN] Enet DMA Handle
 *
 * \return \ref Enet_ErrorCodes
 */
int32_t EnetCpdma_rxIsr(EnetDma_Handle hEnetDma);

/*!
 * \brief ENET CPDMA Tx interrupt service routine
 *
 * Processes Tx interrupt. This function retrieves the completed Tx packets
 * from the linked CPPI descriptors and passes them to the free queue when
 * one or more packets have been transmitted.
 *
 * Requirement:
 *
 * \param hEnetDma     [IN] Enet DMA Handle
 *
 * \return \ref Enet_ErrorCodes
 */
int32_t EnetCpdma_txIsr(EnetDma_Handle hEnetDma);

/*!
 * \brief ENET CPDMA Miscellaneous interrupt service routine
 *
 * Processes Miscellaneous interrupt. This function extracts the intrrupt
 * status bit masks from MMR and pass it to the caller to process all
 * masked interrupts.
 *
 * Requirement:
 *
 * \param hEnetDma     [IN] Enet DMA Handle
 * \param pStatusMask  [IN] pointer to the interrupt status bit mask
 *
 * \return \ref Enet_ErrorCodes
 */
int32_t EnetCpdma_miscIsr(EnetDma_Handle hEnetDma, uint32_t *pStatusMask);

/*!
 * \brief Initialize CPDMA config params
 *
 * Initialize CPDMA config params to default values
 *
 * Requirement:
 *
 * \param enetType     [IN] Enet Type
 * \param pDmaConfig  [IN] pointer to the config structure
 *
 */
void EnetCpdma_initParams(Enet_Type enetType, EnetDma_Cfg *pDmaConfig);

/* ========================================================================== */
/*                        Deprecated Function Declarations                    */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                       Static Function Definitions                          */
/* ========================================================================== */

/* None */

#ifdef __cplusplus
}
#endif

#endif /* ENET_CPDMA_H_ */

/*! @} */
