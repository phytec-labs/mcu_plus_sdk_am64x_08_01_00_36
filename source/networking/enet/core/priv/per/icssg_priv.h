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
 * \file  icssg_priv.h
 *
 * \brief This file contains the private type definitions and helper macros for
 *        the ICSSG peripheral.
 */

#ifndef ICSSG_PRIV_H_
#define ICSSG_PRIV_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdint.h>
#include <drivers/pruicss.h>
#include <priv/core/enet_base_priv.h>
#include <priv/mod/mdio_priv.h>
#include <priv/mod/icssg_timesync_priv.h>
#include <priv/mod/icssg_stats_priv.h>
#include <enet.h>
#include <include/core/enet_per.h>
#include <include/core/enet_types.h>
#include <include/mod/mdio.h>
#include <include/phy/enetphy.h>
#include <include/core/enet_mod_phy.h>
#include <priv/core/enet_rm_priv.h>


#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                                 Macros                                     */
/* ========================================================================== */

/*! \brief PRU ICSS subsystem instance 0 */
#define ICSSG_PRUSS_ID_0                          (0U)

/*! \brief PRU ICSS subsystem instance 1 */
#define ICSSG_PRUSS_ID_1                          (1U)

/*! \brief PRU ICSS subsystem instance 2 */
#define ICSSG_PRUSS_ID_2                          (2U)

/*! \brief Cache alignment used for IOCTL command structure */
#define ICSSG_CACHELINE_ALIGNMENT                 (64U)

/* ========================================================================== */
/*                         Structures and Enums                               */
/* ========================================================================== */
/*!
 * \brief Icssg event callback info structure
 */
typedef struct Icssg_evtCbInfo_s
{
    /*! Event callback function for command complete response event */
    Enet_EventCallback evtCb;

    /*! Event callback function arguments */
    void *evtCbArgs;
} Icssg_evtCbInfo;

/*!
 * \brief ICSS_PRU object.
 *
 * This structure represents the ICSS_PRU subsystem irrespective of the Ethernet
 * specific abstractions of the driver.  For instance, the two MAC ports in ICSSG
 * Dual-MAC mode are abstracted logically as separate peripherals in spite of
 * both running on the same ICSSPRU subsystem.
 *
 * In ICSSG Dual-MAC mode, each of the two #Icssg_Obj structures will have a
 * pointer to the same #Icssg_Pruss object.
 *
 * In Switch mode, there is a single #Icssg_Obj per PRUSS, also indicated via
 * pointer to the corresponding #Icssg_Pruss object.
 *
 * The #Icssg_Obj to #Icssg_Pruss binding is done by EnetSoc layer.
 */
typedef struct Icssg_Pruss_s
{
    /*! Handle to PRUICSS driver */
    PRUICSS_Handle hPruss;

    /*! PRUICSS instance number */
    uint32_t instance;

    /*! Whether the PRUICSSG has been initialized or not */
    bool initialized;

    /*! Whether IEP0 is in use or not */
    bool iep0InUse;

    /*! PRUSS instance lock to be used to protect any operations that involve
     *  any fields of this structure, i.e. using PRUSS driver.
     *  This lock is to be initialized by SoC layer. */
    void *lock;
} Icssg_Pruss;

/*!
 * \brief ICSSG firmware.
 *
 * Container structure for the ICSSG firmware blobs for PRU, RTU and TX PRU.
 */
typedef struct Icssg_Fw_s
{
    /*! Pointer to PRU firmware header */
    const uint32_t *pru;

    /*! Size of PRU firmware header */
    uint32_t pruSize;

    /*! Pointer to RTU firmware header */
    const uint32_t *rtu;

    /*! Size of RTU firmware header */
    uint32_t rtuSize;

    /*! Pointer to TXPRU firmware header */
    const uint32_t *txpru;

    /*! Size of TX PRU firmware header */
    uint32_t txpruSize;
} Icssg_Fw;

/*!
 * \brief IOCTL command structure used to communicate with ICSSG.
 */
typedef struct Icssg_IoctlCmd_s
{
    /*! Command parameter */
    uint8_t param;

    /*! Sequence number */
    uint8_t seqNum;

    /*! Command type */
    uint8_t type;

    /*! Command header */
    uint8_t header;

    /*! Spare data. Used for commands that take additional arguments */
    uint32_t spare[3];
} __attribute__((packed)) Icssg_IoctlCmd;

/*!
 * \brief IOCTL command response structure used to communicate with ICSSG.
 */
typedef struct Icssg_IoctlCmdResp_s
{
    /*! Status for IOCTL command */
    uint8_t status;

    /*! Sequence number of the command request. Returned to application in response */
    uint8_t seqNum;

    /*! Number of bytes in the respParams field, this is optional */
    uint32_t paramsLen;

    /*! Optional response parameters */
    uint32_t params[3U];
} Icssg_IoctlCmdResp;

/*!
 * \brief Icssg per object.
 */
typedef struct Icssg_Obj_s
{
    /*! EnetMod must be the first member */
    EnetPer_Obj enetPer;

    /*! PRUSS instance. SoC layer binds this ICSSG object to the corresponding PRUSS. */
    Icssg_Pruss *pruss;

    /*! ICSSG firmware configuration: image addresses and sizes.
     *  - Switch peripheral (#ENET_ICSSG_SWITCH), application must populate all
     *    firmwares entries of this array.
     *  - Dual-MAC peripheral (#ENET_ICSSG_DUALMAC), application must populate
     *    only the first firmware entry. */
    Icssg_Fw fw[ICSSG_MAC_PORT_MAX];

    /*! Asycnronuous IOCTL sequence number */
    uint8_t asyncIoctlSeqNum;

    /*! Asycnronuous IOCTL type */
    uint32_t asyncIoctlType;

    /*! Event callback information object for async command resp. callback */
    Icssg_evtCbInfo asyncCmdRespCbEvtInfo;

    /*! Event callback information object for TX timestamp event callback */
    Icssg_evtCbInfo txTsCbEvtInfo;

    /*! Resource Manager object */
    EnetRm_Obj rmObj;

    /*! Resource Manager handle */
    EnetMod_Handle hRm;

    /*! Core on which Icssg_Open() is executed */
    uint32_t selfCoreId;

    /*! DMA handle */
    EnetDma_Handle hDma;

    /*! Number of required UDMA RX channels */
    uint32_t numRxCh;

    /*! DMA Rx Reserved flow handle */
    EnetDma_RxChHandle hRxRsvdFlow[ICSSG_MAC_PORT_MAX];

    /*! DMA resource information */
    Enet_dmaResInfo dmaResInfo[ICSSG_MAC_PORT_MAX];

    /*! DMA Rx Reserved flow Id */
    uint32_t rsvdFlowId[ICSSG_MAC_PORT_MAX];

    /*! MDIO object */
    Mdio_Obj mdioObj;

    /*! MDIO handle */
    EnetMod_Handle hMdio;

    /*! PHY handles */
    EnetPhy_Handle hPhy[ICSSG_MAC_PORT_MAX];

    /*! TimesSync object */
    IcssgTimeSync_Obj timeSyncObj;

    /*! TimesSync handle */
    EnetMod_Handle hTimeSync;

    /*! Stats object */
    IcssgStats_Obj statsObj;

    /*! Stats handle */
    EnetMod_Handle hStats;

    /*! IOCTL command */
    Icssg_IoctlCmd cmd __attribute__ ((aligned(ICSSG_CACHELINE_ALIGNMENT)));
} Icssg_Obj;

/*!
 * \brief MAC port module handle.
 */
typedef Icssg_Obj *Icssg_Handle;

/* ========================================================================== */
/*                         Global Variables Declarations                      */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/*!
 * \brief Initialize ICSSG peripheral's configuration parameters.
 *
 * Initializes the configuration parameter of the ICSSG peripheral.
 *
 * \param hPer      Enet Peripheral handle
 * \param enetType  Enet Peripheral type
 * \param cfg       Configuration parameters to be initialized.  The config
 *                  is of type #Icssg_Cfg.
 * \param cfgSize   Size of the configuration parameters.  It must be the size
 *                  of #Icssg_Cfg config structure.
 */
void Icssg_initCfg(EnetPer_Handle hPer,
                   Enet_Type enetType,
                   void *cfg,
                   uint32_t cfgSize);

/*!
 * \brief Open and initialize the ICSSG Peripheral.
 *
 * Opens and initializes the ICSSG peripheral with the configuration parameters
 * provided by the caller.
 *
 * \param hPer      Enet Peripheral handle
 * \param enetType  Enet Peripheral type
 * \param instId    Enet Peripheral instance id
 * \param cfg       Configuration parameters to be initialized.  The config
 *                  is of type #Icssg_Cfg.
 * \param cfgSize   Size of the configuration parameters.  It must be the size
 *                  of #Icssg_Cfg config structure.
 *
 * \return \ref Enet_ErrorCodes
 */
int32_t Icssg_open(EnetPer_Handle hPer,
                   Enet_Type enetType,
                   uint32_t instId,
                   const void *cfg,
                   uint32_t cfgSize);

/*!
 * \brief Rejoin a running ICSSG peripheral.
 *
 * This operation is not supported by the ICSSG peripheral.  Calling this
 * function will return #ENET_ENOTSUPPORTED.
 *
 * \param hPer      Enet Peripheral handle
 * \param enetType  Enet Peripheral type
 * \param instId    Enet Peripheral instance id
 *
 * \retval ENET_ENOTSUPPORTED
 */
int32_t Icssg_rejoin(EnetPer_Handle hPer,
                     Enet_Type enetType,
                     uint32_t instId);

/*!
 * \brief Issue an operation on the ICSSG peripheral.
 *
 * Issues a control operation on the ICSSG peripheral.
 *
 * \param hPer         Enet Peripheral handle
 * \param cmd          IOCTL command Id
 * \param prms         IOCTL parameters
 *
 * \return \ref Enet_ErrorCodes
 */
int32_t Icssg_ioctl(EnetPer_Handle hPer,
                    uint32_t cmd,
                    Enet_IoctlPrms *prms);

/*!
 * \brief Poll for Ethernet events.
 *
 * Unblocking poll for the events specified in \p evt. ICSSG uses this
 * function to poll for completion of asynchronous IOCTLs.
 *
 * \param hPer         Enet Peripheral handle
 * \param evt          Event type
 * \param arg          Pointer to the poll argument. This is specific to the
 *                     poll event type
 * \param argSize      Size of \p arg
 */
void Icssg_poll(EnetPer_Handle hPer,
                Enet_Event evt,
                const void *arg,
                uint32_t argSize);

/*!
 * \brief Converts ICSSG timestamp to nanoseconds.
 *
 * ICSSG timestamp encodes IEP count low/high and rollover count as bit fields
 * in the 64-bit value returned by ICSSG.  This value needs to be converted
 * to nanoseconds before application can consume it.
 *
 * \param hPer         Enet Peripheral handle
 * \param ts           Timestamp value, definition is peripheral specific
 *
 * \return Nanoseconds value.
 */
uint64_t Icssg_convertTs(EnetPer_Handle hPer,
                         uint64_t ts);

/*!
 * \brief Run periodic tick on the ICSSG peripheral.
 *
 * Run PHY periodic tick on the ICSSG peripheral.  The peripheral driver in
 * turn runs the periodic tick operation on all opened PHYs.
 *
 * \param hPer        Enet Peripheral handle
 */
void Icssg_periodicTick(EnetPer_Handle hPer);

void Icssg_registerEventCb(EnetPer_Handle hPer,
                           Enet_Event evt,
                           uint32_t evtNum,
                           Enet_EventCallback evtCb,
                           void *evtCbArgs);

void Icssg_unregisterEventCb(EnetPer_Handle hPer,
                             Enet_Event evt,
                             uint32_t evtNum);

/*!
 * \brief Close the ICSSG peripheral.
 *
 * Closes the ICSSG peripheral.
 *
 * \param hPer        Enet Peripheral handle
 */
void Icssg_close(EnetPer_Handle hPer);

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

#endif /* ICSSG_PRIV_H_ */
