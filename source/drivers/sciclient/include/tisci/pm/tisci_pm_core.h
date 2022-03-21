/*
 *  Copyright (C) 2017-2021 Texas Instruments Incorporated
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
/**
 * \ingroup TISCI
 * \defgroup tisci_pm_core tisci_pm_core
 *
 * DMSC controls the power management, security and resource management
 * of the device.
 *
 *
 * @{
 */
/**
 *
 *  \brief  This file contains:
 *
 *          WARNING!!: Autogenerated file from SYSFW. DO NOT MODIFY!!
 * System Firmware
 *
 * Cortex-M3 (CM3) firmware for power management
 *
 */
#ifndef TISCI_PM_TISCI_CORE_H
#define TISCI_PM_TISCI_CORE_H


/**
 * \brief Empty request for TISCI_MSG_WAKE_REASON.
 *
 * Although this message is essentially empty and contains only a header
 * a full data structure is created for consistency in implementation.
 *
 * \param hdr TISCI header.
 */
struct tisci_msg_wake_reason_req {
    struct tisci_header hdr;
} __attribute__((__packed__));

/**
 * \brief Response for TISCI_MSG_WAKE_REASON.
 *
 * \param hdr TISCI header to provide ACK/NAK flags to the host.
 * \param mode Mode that was active before wake event.
 * \param reason Reason the wakeup happened.
 * \param time_ms Time spent in low power mode.
 */
struct tisci_msg_wake_reason_resp {
    struct tisci_header    hdr;
    char            mode[32];
    char            reason[32];
    uint32_t            time_ms;
} __attribute__((__packed__));

/**
 * \brief Empty request for TISCI_MSG_WAKE_RESET.
 *
 * Although this message is essentially empty and contains only a header
 * a full data structure is created for consistency in implementation.
 *
 * \param hdr TISCI header.
 */
struct tisci_msg_wake_reset_req {
    struct tisci_header hdr;
} __attribute__((__packed__));

/**
 * \brief Empty response for TISCI_MSG_WAKE_RESET.
 *
 * Although this message is essentially empty and contains only a header
 * a full data structure is created for consistency in implementation.
 *
 * \param hdr TISCI header.
 */
struct tisci_msg_wake_reset_resp {
    struct tisci_header hdr;
} __attribute__((__packed__));

/**
 * \brief Empty request for TISCI_MSG_ENABLE_WDT.
 *
 * Although this message is essentially empty and contains only a header
 * a full data structure is created for consistency in implementation.
 *
 * \param hdr TISCI header.
 */
struct tisci_msg_enable_wdt_req {
    struct tisci_header hdr;
} __attribute__((__packed__));

/**
 * \brief Empty response for TISCI_MSG_ENABLE_WDT.
 *
 * Although this message is essentially empty and contains only a header
 * a full data structure is created for consistency in implementation.
 *
 * \param hdr TISCI header.
 */
struct tisci_msg_enable_wdt_resp {
    struct tisci_header hdr;
} __attribute__((__packed__));

/**
 * \brief Empty request for TISCI_MSG_GOODBYE.
 *
 * Although this message is essentially empty and contains only a header
 * a full data structure is created for consistency in implementation.
 *
 * \param hdr TISCI header.
 */
struct tisci_msg_goodbye_req {
    struct tisci_header hdr;
} __attribute__((__packed__));

/**
 * \brief Empty response for TISCI_MSG_GOODBYE.
 *
 * Although this message is essentially empty and contains only a header
 * a full data structure is created for consistency in implementation.
 *
 * \param hdr TISCI header.
 */
struct tisci_msg_goodbye_resp {
    struct tisci_header hdr;
} __attribute__((__packed__));

/**
 * \brief Request for TISCI_MSG_SYS_RESET.
 *
 * \param hdr TISCI header to provide ACK/NAK flags to the host.
 * \param domain Domain to be reset.
 */
struct tisci_msg_sys_reset_req {
    struct tisci_header    hdr;
    domgrp_t        domain;
} __attribute__((__packed__));

/**
 * \brief Empty response for TISCI_MSG_SYS_RESET.
 *
 * Although this message is essentially empty and contains only a header
 * a full data structure is created for consistency in implementation.
 *
 * \param hdr TISCI header.
 */
struct tisci_msg_sys_reset_resp {
    struct tisci_header hdr;
} __attribute__((__packed__));

#endif /* TISCI_PM_TISCI_CORE_H */

/** @} */
