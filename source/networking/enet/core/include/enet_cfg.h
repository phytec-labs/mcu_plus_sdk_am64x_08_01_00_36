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
 * \file  enet_cfg.h
 *
 * \brief This file contains the Enet configuration parameters.
 */

#ifndef ENET_CFG_H_
#define ENET_CFG_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                                 Macros                                     */
/* ========================================================================== */

/*! \brief Build-time config option is enabled. */
#define ENET_ON                                     (1U)

/*! \brief Build-time config option is disabled. */
#define ENET_OFF                                    (0U)

/*! \brief Preprocessor check if config option is enabled. */
#define ENET_CFG_IS_ON(name)                        ((ENET_CFG_ ## name) == ENET_ON)

/*! \brief Preprocessor check if config option is disabled. */
#define ENET_CFG_IS_OFF(name)                       ((ENET_CFG_ ## name) == ENET_OFF)

/* --------------------------------------------------------------------------*/
/*                         Enet generic config options                       */
/* --------------------------------------------------------------------------*/

/*! \brief EnetUtils print buffer length. */
#define ENET_CFG_PRINT_BUF_LEN                      (200U)

/*! \brief Whether Enet driver has a default OSAL implementation. */
#define ENET_CFG_HAS_DEFAULT_OSAL                   (ENET_ON)

/*! \brief Whether Enet driver has a default utils implementation. */
#define ENET_CFG_HAS_DEFAULT_UTILS                  (ENET_ON)

/*! \brief Enable top-layer sanity checks and misc debug info. */
#define ENET_CFG_SANITY_CHECKS                      (ENET_ON)

/*! \brief Maximum number of supported PHYs (allocated PHY objects). */
#if defined(SOC_AM64X) || defined(SOC_AM243X) || defined(SOC_AM263X)
#define ENET_CFG_ENETPHY_PHY_MAX                    (2U)
#elif defined(SOC_AM273X) || defined(SOC_AWR294X)
#define ENET_CFG_ENETPHY_PHY_MAX                    (1U)
#else
#define ENET_CFG_ENETPHY_PHY_MAX                    (13U)
#endif

/* --------------------------------------------------------------------------*/
/*        CPSW Peripheral and CPSW Module related config options             */
/* --------------------------------------------------------------------------*/

/*! \brief CPSW Q/SGMII support (requires #ENET_CFG_CPSW_MACPORT_SGMII). */
#if defined(SOC_AM273X) || defined(SOC_AWR294X) || defined(SOC_AM64X) || defined(SOC_AM243X) || defined(SOC_AM263X)
#define ENET_CFG_CPSW_SGMII                         (ENET_OFF)
#else
#define ENET_CFG_CPSW_SGMII                         (ENET_ON)
#endif

/*! \brief CPSW interVLAN support support (requires #ENET_CFG_CPSW_MACPORT_INTERVLAN). */
#if defined(SOC_AM273X) || defined(SOC_AWR294X) || defined(SOC_AM64X) || defined(SOC_AM243X) || defined(SOC_AM263X)
#define ENET_CFG_CPSW_INTERVLAN                     (ENET_OFF)
#else
#define ENET_CFG_CPSW_INTERVLAN                     (ENET_ON)
#endif

/*! \brief MDIO Clause-45 frame support. */
#if defined(SOC_AM273X) || defined(SOC_AWR294X) || defined(SOC_AM64X) || defined(SOC_AM243X) || defined(SOC_AM263X)
#define ENET_CFG_MDIO_CLAUSE45                      (ENET_OFF)
#else
#define ENET_CFG_MDIO_CLAUSE45                      (ENET_ON)
#endif

#if defined(SOC_AM273X) || defined(SOC_AWR294X) || defined(SOC_AM64X) || defined(SOC_AM243X) || defined(SOC_AM263X)
#define ENET_CFG_CPSW_HOSTPORT_TRAFFIC_SHAPING      (ENET_OFF)
#define ENET_CFG_CPSW_MACPORT_SGMII                 (ENET_OFF)
#define ENET_CFG_CPSW_MACPORT_INTERVLAN             (ENET_OFF)
#define ENET_CFG_CPSW_CPTS_STATS                    (ENET_OFF)
#define ENET_CFG_CPSW_CPTS_EVENTS_POOL_SIZE         (8U)
#define ENET_CFG_REMOTE_CLIENT_CORES_MAX            (1U)
#define ENET_CFG_RM_MAC_ADDR_MAX                    (3U)
#define ENET_CFG_RM_TX_CH_MAX                       (2U)
#define ENET_CFG_RM_RX_CH_MAX                       (4U)
#else
#define ENET_CFG_CPSW_HOSTPORT_TRAFFIC_SHAPING      (ENET_ON)
#define ENET_CFG_CPSW_MACPORT_SGMII                 (ENET_ON)
#define ENET_CFG_CPSW_MACPORT_INTERVLAN             (ENET_ON)
#define ENET_CFG_CPSW_CPTS_STATS                    (ENET_ON)
#define ENET_CFG_CPSW_CPTS_EVENTS_POOL_SIZE         (128U)
/*! \brief Maximum number of client core that the Enet driver can serve. */
#define ENET_CFG_REMOTE_CLIENT_CORES_MAX            (6U)
/*! \brief Maximum number of MAC addresses that Enet RM can manage. */
#define ENET_CFG_RM_MAC_ADDR_MAX                    (10U)
/*! \brief Maximum number of TX channels that Enet RM can manage. */
#define ENET_CFG_RM_TX_CH_MAX                       (8U)
/*! \brief Maximum number of RX channels that Enet RM can manage. */
#define ENET_CFG_RM_RX_CH_MAX                       (64U)
#endif

/** \brief SOC specific configuration defines */
#if defined(SOC_AM273X) || defined(SOC_AWR294X) || defined(SOC_AM263X)
#define ENET_CFG_RM_PRESENT                         (ENET_OFF)
#elif defined(SOC_AM64X) || defined(SOC_AM243X)
#define ENET_CFG_RM_PRESENT                         (ENET_ON)
#else
#error "SOC not supported"
#endif

#if defined(SOC_AM64X) || defined(SOC_AM243X) || defined(SOC_AM273X) || defined(SOC_AWR294X) || defined(SOC_AM263X)

/*! \brief Overwrite UDMA config for ICSSG as we use more flows/channels for
 *         multiport testNumber of TX channels. */
#define ENET_CFG_NUM_INSTANCES                      (1U)
#define ENET_CFG_TX_CHANNELS_NUM                    (1U * ENET_CFG_NUM_INSTANCES)
#define ENET_CFG_RX_FLOWS_NUM                       (3U * ENET_CFG_NUM_INSTANCES)
#define ENET_CFG_RING_MON_NUM                       (3U * ENET_CFG_NUM_INSTANCES)
#else
#define ENET_CFG_TX_CHANNELS_NUM                    (8U)
#define ENET_CFG_RX_FLOWS_NUM                       (8U)
#define ENET_CFG_RING_MON_NUM                       (4U)
#endif

/* --------------------------------------------------------------------------*/
/*       ICSS-G Peripheral and CPSW Module related config options            */
/* --------------------------------------------------------------------------*/

/* --------------------------------------------------------------------------*/
/*        GMAC Peripheral and CPSW Module related config options             */
/* --------------------------------------------------------------------------*/

/*! \brief Maximum number of MAC port stats blocks. */
#if defined(SOC_AM273X) || defined(SOC_AWR294X)
#define CPSW_STATS_MACPORT_MAX              (1U)
#elif defined(SOC_AM64X) || defined(SOC_AM243X) || defined(SOC_AM263X)
#define CPSW_STATS_MACPORT_MAX              (2U)
#else
#error "SOC not supported"
#endif

/* ========================================================================== */
/*                         Structures and Enums                               */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                         Global Variables Declarations                      */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/* None */

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

#endif /* ENET_CFG_H_ */
