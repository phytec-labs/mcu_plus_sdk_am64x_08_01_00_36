/*
 * K3 System Firmware Resource Management Configuration Data
 * Auto generated from K3 Resource Partitioning tool
 *
 * Copyright (c) 2018-2020, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/**
 *  \file am64x/sciclient_defaultBoardcfg.c
 *
 *  \brief File containing the tisci_boardcfg default data structure to
 *      send TISCI_MSG_BOARD_CONFIG message (for A53 running linux).
 *
 */
/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <drivers/sciclient.h>
#include <drivers/sciclient/include/tisci/am64x_am243x/tisci_hosts.h>
#include <drivers/sciclient/include/tisci/am64x_am243x/tisci_boardcfg_constraints.h>
#include <drivers/sciclient/include/tisci/am64x_am243x/tisci_devices.h>

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/* \brief Structure to hold the RM board configuration */
struct tisci_local_rm_boardcfg {
    struct tisci_boardcfg_rm      rm_boardcfg;
    /**< Board configuration parameter */
    struct tisci_boardcfg_rm_resasg_entry resasg_entries[TISCI_RESASG_ENTRIES_MAX];
    /**< Resource assignment entries */
};

const struct tisci_local_rm_boardcfg gBoardConfigLow_rm_linux
__attribute__(( aligned(128), section(".boardcfg_data") )) =
{
	.rm_boardcfg = {
		/* boardcfg_abi_rev */
		.rev = {
            .tisci_boardcfg_abi_maj = TISCI_BOARDCFG_RM_ABI_MAJ_VALUE,
            .tisci_boardcfg_abi_min = TISCI_BOARDCFG_RM_ABI_MIN_VALUE,
        },

		/* boardcfg_rm_host_cfg */
		.host_cfg = {
			.subhdr = {
				.magic = TISCI_BOARDCFG_RM_HOST_CFG_MAGIC_NUM,
                .size = (uint16_t) sizeof(struct tisci_boardcfg_rm_host_cfg),
			},
			.host_cfg_entries = {
				{
					.host_id = TISCI_HOST_ID_A53_2,
					.allowed_atype = 0b101010,
					.allowed_qos = 0xAAAA,
					.allowed_orderid = 0xAAAAAAAA,
					.allowed_priority = 0xAAAA,
					.allowed_sched_priority = 0xAA,
				},
				{
					.host_id = TISCI_HOST_ID_M4_0,
					.allowed_atype = 0b101010,
					.allowed_qos = 0xAAAA,
					.allowed_orderid = 0xAAAAAAAA,
					.allowed_priority = 0xAAAA,
					.allowed_sched_priority = 0xAA,
				},
				{
					.host_id = TISCI_HOST_ID_MAIN_0_R5_1,
					.allowed_atype = 0b101010,
					.allowed_qos = 0xAAAA,
					.allowed_orderid = 0xAAAAAAAA,
					.allowed_priority = 0xAAAA,
					.allowed_sched_priority = 0xAA,
				},
				{
					.host_id = TISCI_HOST_ID_MAIN_0_R5_3,
					.allowed_atype = 0b101010,
					.allowed_qos = 0xAAAA,
					.allowed_orderid = 0xAAAAAAAA,
					.allowed_priority = 0xAAAA,
					.allowed_sched_priority = 0xAA,
				},
				{
					.host_id = TISCI_HOST_ID_MAIN_1_R5_1,
					.allowed_atype = 0b101010,
					.allowed_qos = 0xAAAA,
					.allowed_orderid = 0xAAAAAAAA,
					.allowed_priority = 0xAAAA,
					.allowed_sched_priority = 0xAA,
				},
				{
					.host_id = TISCI_HOST_ID_MAIN_1_R5_3,
					.allowed_atype = 0b101010,
					.allowed_qos = 0xAAAA,
					.allowed_orderid = 0xAAAAAAAA,
					.allowed_priority = 0xAAAA,
					.allowed_sched_priority = 0xAA,
				},
			}
		},

		/* boardcfg_rm_resasg */
		.resasg = {
			.subhdr = {
				.magic = TISCI_BOARDCFG_RM_RESASG_MAGIC_NUM,
                .size = (uint16_t) sizeof(struct tisci_boardcfg_rm_resasg),
			},
			.resasg_entries_size =
				158 *
				sizeof (struct tisci_boardcfg_rm_resasg_entry),
			.reserved = 0,
			/* .resasg_entries is set via boardcfg_rm_local */
		},
	},

	/* This is actually part of .resasg */
	.resasg_entries = {
		/* Compare event Interrupt Router */
		{
			.start_resource = 0,
			.num_resource = 16,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_CMP_EVENT_INTROUTER0,
					TISCI_RESASG_SUBTYPE_IR_OUTPUT),
			.host_id = TISCI_HOST_ID_A53_2,
		},
		{
			.start_resource = 16,
			.num_resource = 4,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_CMP_EVENT_INTROUTER0,
					TISCI_RESASG_SUBTYPE_IR_OUTPUT),
			.host_id = TISCI_HOST_ID_MAIN_0_R5_0,
		},
		{
			.start_resource = 16,
			.num_resource = 4,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_CMP_EVENT_INTROUTER0,
					TISCI_RESASG_SUBTYPE_IR_OUTPUT),
			.host_id = TISCI_HOST_ID_MAIN_0_R5_1,
		},
		{
			.start_resource = 20,
			.num_resource = 4,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_CMP_EVENT_INTROUTER0,
					TISCI_RESASG_SUBTYPE_IR_OUTPUT),
			.host_id = TISCI_HOST_ID_MAIN_0_R5_3,
		},
		{
			.start_resource = 24,
			.num_resource = 4,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_CMP_EVENT_INTROUTER0,
					TISCI_RESASG_SUBTYPE_IR_OUTPUT),
			.host_id = TISCI_HOST_ID_MAIN_1_R5_1,
		},
		{
			.start_resource = 28,
			.num_resource = 4,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_CMP_EVENT_INTROUTER0,
					TISCI_RESASG_SUBTYPE_IR_OUTPUT),
			.host_id = TISCI_HOST_ID_MAIN_1_R5_3,
		},
		{
			.start_resource = 32,
			.num_resource = 8,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_CMP_EVENT_INTROUTER0,
					TISCI_RESASG_SUBTYPE_IR_OUTPUT),
			.host_id = TISCI_HOST_ID_ALL,
		},
		/* Main GPIO Interrupt Router */
		{
			.start_resource = 0,
			.num_resource = 12,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_MAIN_GPIOMUX_INTROUTER0,
					TISCI_RESASG_SUBTYPE_IR_OUTPUT),
			.host_id = TISCI_HOST_ID_A53_2,
		},
		{
			.start_resource = 12,
			.num_resource = 2,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_MAIN_GPIOMUX_INTROUTER0,
					TISCI_RESASG_SUBTYPE_IR_OUTPUT),
			.host_id = TISCI_HOST_ID_MAIN_1_R5_1,
		},
		{
			.start_resource = 14,
			.num_resource = 2,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_MAIN_GPIOMUX_INTROUTER0,
					TISCI_RESASG_SUBTYPE_IR_OUTPUT),
			.host_id = TISCI_HOST_ID_MAIN_1_R5_3,
		},
		/* MCU GPIO Interrupt Router */
		{
			.start_resource = 0,
			.num_resource = 4,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_MCU_MCU_GPIOMUX_INTROUTER0,
					TISCI_RESASG_SUBTYPE_IR_OUTPUT),
			.host_id = TISCI_HOST_ID_A53_2,
		},
		{
			.start_resource = 4,
			.num_resource = 4,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_MCU_MCU_GPIOMUX_INTROUTER0,
					TISCI_RESASG_SUBTYPE_IR_OUTPUT),
			.host_id = TISCI_HOST_ID_M4_0,
		},
		/* Timesync Interrupt Router */
		{
			.start_resource = 0,
			.num_resource = 41,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_TIMESYNC_EVENT_INTROUTER0,
					TISCI_RESASG_SUBTYPE_IR_OUTPUT),
			.host_id = TISCI_HOST_ID_ALL,
		},
		/* Block Copy DMA Global event trigger */
		{
			.start_resource = 50176,
			.num_resource = 136,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_BCDMA_0,
					TISCI_RESASG_SUBTYPE_GLOBAL_EVENT_TRIGGER),
			.host_id = TISCI_HOST_ID_ALL,
		},
		/* Block Copy DMA Global config */
		{
			.start_resource = 0,
			.num_resource = 1,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_BCDMA_0,
					TISCI_RESASG_SUBTYPE_UDMAP_GLOBAL_CONFIG),
			.host_id = TISCI_HOST_ID_ALL,
		},
		/* Block Copy DMA Rings for Block copy channels */
		{
			.start_resource = 0,
			.num_resource = 12,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_BCDMA_0,
					TISCI_RESASG_SUBTYPE_BCDMA_RING_BLOCK_COPY_CHAN),
			.host_id = TISCI_HOST_ID_A53_2,
		},
		{
			.start_resource = 12,
			.num_resource = 6,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_BCDMA_0,
					TISCI_RESASG_SUBTYPE_BCDMA_RING_BLOCK_COPY_CHAN),
			.host_id = TISCI_HOST_ID_MAIN_0_R5_0,
		},
		{
			.start_resource = 12,
			.num_resource = 6,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_BCDMA_0,
					TISCI_RESASG_SUBTYPE_BCDMA_RING_BLOCK_COPY_CHAN),
			.host_id = TISCI_HOST_ID_MAIN_0_R5_1,
		},
		{
			.start_resource = 18,
			.num_resource = 2,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_BCDMA_0,
					TISCI_RESASG_SUBTYPE_BCDMA_RING_BLOCK_COPY_CHAN),
			.host_id = TISCI_HOST_ID_MAIN_0_R5_3,
		},
		{
			.start_resource = 20,
			.num_resource = 4,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_BCDMA_0,
					TISCI_RESASG_SUBTYPE_BCDMA_RING_BLOCK_COPY_CHAN),
			.host_id = TISCI_HOST_ID_MAIN_1_R5_1,
		},
		{
			.start_resource = 24,
			.num_resource = 2,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_BCDMA_0,
					TISCI_RESASG_SUBTYPE_BCDMA_RING_BLOCK_COPY_CHAN),
			.host_id = TISCI_HOST_ID_MAIN_1_R5_3,
		},
		{
			.start_resource = 26,
			.num_resource = 1,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_BCDMA_0,
					TISCI_RESASG_SUBTYPE_BCDMA_RING_BLOCK_COPY_CHAN),
			.host_id = TISCI_HOST_ID_M4_0,
		},
		{
			.start_resource = 27,
			.num_resource = 1,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_BCDMA_0,
					TISCI_RESASG_SUBTYPE_BCDMA_RING_BLOCK_COPY_CHAN),
			.host_id = TISCI_HOST_ID_ALL,
		},
		/* Block Copy DMA Rings for Split TR Rx channel */
		{
			.start_resource = 48,
			.num_resource = 6,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_BCDMA_0,
					TISCI_RESASG_SUBTYPE_BCDMA_RING_SPLIT_TR_RX_CHAN),
			.host_id = TISCI_HOST_ID_A53_2,
		},
		{
			.start_resource = 54,
			.num_resource = 6,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_BCDMA_0,
					TISCI_RESASG_SUBTYPE_BCDMA_RING_SPLIT_TR_RX_CHAN),
			.host_id = TISCI_HOST_ID_MAIN_0_R5_0,
		},
		{
			.start_resource = 54,
			.num_resource = 6,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_BCDMA_0,
					TISCI_RESASG_SUBTYPE_BCDMA_RING_SPLIT_TR_RX_CHAN),
			.host_id = TISCI_HOST_ID_MAIN_0_R5_1,
		},
		{
			.start_resource = 60,
			.num_resource = 2,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_BCDMA_0,
					TISCI_RESASG_SUBTYPE_BCDMA_RING_SPLIT_TR_RX_CHAN),
			.host_id = TISCI_HOST_ID_MAIN_0_R5_3,
		},
		{
			.start_resource = 62,
			.num_resource = 4,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_BCDMA_0,
					TISCI_RESASG_SUBTYPE_BCDMA_RING_SPLIT_TR_RX_CHAN),
			.host_id = TISCI_HOST_ID_MAIN_1_R5_1,
		},
		{
			.start_resource = 66,
			.num_resource = 2,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_BCDMA_0,
					TISCI_RESASG_SUBTYPE_BCDMA_RING_SPLIT_TR_RX_CHAN),
			.host_id = TISCI_HOST_ID_MAIN_1_R5_3,
		},
		/* Block Copy DMA Rings for Split TR Tx channel */
		{
			.start_resource = 28,
			.num_resource = 6,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_BCDMA_0,
					TISCI_RESASG_SUBTYPE_BCDMA_RING_SPLIT_TR_TX_CHAN),
			.host_id = TISCI_HOST_ID_A53_2,
		},
		{
			.start_resource = 34,
			.num_resource = 6,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_BCDMA_0,
					TISCI_RESASG_SUBTYPE_BCDMA_RING_SPLIT_TR_TX_CHAN),
			.host_id = TISCI_HOST_ID_MAIN_0_R5_0,
		},
		{
			.start_resource = 34,
			.num_resource = 6,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_BCDMA_0,
					TISCI_RESASG_SUBTYPE_BCDMA_RING_SPLIT_TR_TX_CHAN),
			.host_id = TISCI_HOST_ID_MAIN_0_R5_1,
		},
		{
			.start_resource = 40,
			.num_resource = 2,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_BCDMA_0,
					TISCI_RESASG_SUBTYPE_BCDMA_RING_SPLIT_TR_TX_CHAN),
			.host_id = TISCI_HOST_ID_MAIN_0_R5_3,
		},
		{
			.start_resource = 42,
			.num_resource = 4,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_BCDMA_0,
					TISCI_RESASG_SUBTYPE_BCDMA_RING_SPLIT_TR_TX_CHAN),
			.host_id = TISCI_HOST_ID_MAIN_1_R5_1,
		},
		{
			.start_resource = 46,
			.num_resource = 2,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_BCDMA_0,
					TISCI_RESASG_SUBTYPE_BCDMA_RING_SPLIT_TR_TX_CHAN),
			.host_id = TISCI_HOST_ID_MAIN_1_R5_3,
		},
		/* Block Copy DMA Block copy channels */
		{
			.start_resource = 0,
			.num_resource = 12,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_BCDMA_0,
					TISCI_RESASG_SUBTYPE_BCDMA_BLOCK_COPY_CHAN),
			.host_id = TISCI_HOST_ID_A53_2,
		},
		{
			.start_resource = 12,
			.num_resource = 6,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_BCDMA_0,
					TISCI_RESASG_SUBTYPE_BCDMA_BLOCK_COPY_CHAN),
			.host_id = TISCI_HOST_ID_MAIN_0_R5_0,
		},
		{
			.start_resource = 12,
			.num_resource = 6,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_BCDMA_0,
					TISCI_RESASG_SUBTYPE_BCDMA_BLOCK_COPY_CHAN),
			.host_id = TISCI_HOST_ID_MAIN_0_R5_1,
		},
		{
			.start_resource = 18,
			.num_resource = 2,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_BCDMA_0,
					TISCI_RESASG_SUBTYPE_BCDMA_BLOCK_COPY_CHAN),
			.host_id = TISCI_HOST_ID_MAIN_0_R5_3,
		},
		{
			.start_resource = 20,
			.num_resource = 4,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_BCDMA_0,
					TISCI_RESASG_SUBTYPE_BCDMA_BLOCK_COPY_CHAN),
			.host_id = TISCI_HOST_ID_MAIN_1_R5_1,
		},
		{
			.start_resource = 24,
			.num_resource = 2,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_BCDMA_0,
					TISCI_RESASG_SUBTYPE_BCDMA_BLOCK_COPY_CHAN),
			.host_id = TISCI_HOST_ID_MAIN_1_R5_3,
		},
		{
			.start_resource = 26,
			.num_resource = 1,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_BCDMA_0,
					TISCI_RESASG_SUBTYPE_BCDMA_BLOCK_COPY_CHAN),
			.host_id = TISCI_HOST_ID_M4_0,
		},
		{
			.start_resource = 27,
			.num_resource = 1,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_BCDMA_0,
					TISCI_RESASG_SUBTYPE_BCDMA_BLOCK_COPY_CHAN),
			.host_id = TISCI_HOST_ID_ALL,
		},
		/* Block Copy DMA Split TR Rx channels */
		{
			.start_resource = 0,
			.num_resource = 6,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_BCDMA_0,
					TISCI_RESASG_SUBTYPE_BCDMA_SPLIT_TR_RX_CHAN),
			.host_id = TISCI_HOST_ID_A53_2,
		},
		{
			.start_resource = 6,
			.num_resource = 6,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_BCDMA_0,
					TISCI_RESASG_SUBTYPE_BCDMA_SPLIT_TR_RX_CHAN),
			.host_id = TISCI_HOST_ID_MAIN_0_R5_0,
		},
		{
			.start_resource = 6,
			.num_resource = 6,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_BCDMA_0,
					TISCI_RESASG_SUBTYPE_BCDMA_SPLIT_TR_RX_CHAN),
			.host_id = TISCI_HOST_ID_MAIN_0_R5_1,
		},
		{
			.start_resource = 12,
			.num_resource = 2,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_BCDMA_0,
					TISCI_RESASG_SUBTYPE_BCDMA_SPLIT_TR_RX_CHAN),
			.host_id = TISCI_HOST_ID_MAIN_0_R5_3,
		},
		{
			.start_resource = 14,
			.num_resource = 4,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_BCDMA_0,
					TISCI_RESASG_SUBTYPE_BCDMA_SPLIT_TR_RX_CHAN),
			.host_id = TISCI_HOST_ID_MAIN_1_R5_1,
		},
		{
			.start_resource = 18,
			.num_resource = 2,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_BCDMA_0,
					TISCI_RESASG_SUBTYPE_BCDMA_SPLIT_TR_RX_CHAN),
			.host_id = TISCI_HOST_ID_MAIN_1_R5_3,
		},
		/* Block Copy DMA Split TR Tx channels */
		{
			.start_resource = 0,
			.num_resource = 6,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_BCDMA_0,
					TISCI_RESASG_SUBTYPE_BCDMA_SPLIT_TR_TX_CHAN),
			.host_id = TISCI_HOST_ID_A53_2,
		},
		{
			.start_resource = 6,
			.num_resource = 6,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_BCDMA_0,
					TISCI_RESASG_SUBTYPE_BCDMA_SPLIT_TR_TX_CHAN),
			.host_id = TISCI_HOST_ID_MAIN_0_R5_0,
		},
		{
			.start_resource = 6,
			.num_resource = 6,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_BCDMA_0,
					TISCI_RESASG_SUBTYPE_BCDMA_SPLIT_TR_TX_CHAN),
			.host_id = TISCI_HOST_ID_MAIN_0_R5_1,
		},
		{
			.start_resource = 12,
			.num_resource = 2,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_BCDMA_0,
					TISCI_RESASG_SUBTYPE_BCDMA_SPLIT_TR_TX_CHAN),
			.host_id = TISCI_HOST_ID_MAIN_0_R5_3,
		},
		{
			.start_resource = 14,
			.num_resource = 4,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_BCDMA_0,
					TISCI_RESASG_SUBTYPE_BCDMA_SPLIT_TR_TX_CHAN),
			.host_id = TISCI_HOST_ID_MAIN_1_R5_1,
		},
		{
			.start_resource = 18,
			.num_resource = 2,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_BCDMA_0,
					TISCI_RESASG_SUBTYPE_BCDMA_SPLIT_TR_TX_CHAN),
			.host_id = TISCI_HOST_ID_MAIN_1_R5_3,
		},
		/* DMASS Interrupt aggregator Virtual interrupts */
		{
			.start_resource = 4,
			.num_resource = 36,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_INTAGGR_0,
					TISCI_RESASG_SUBTYPE_IA_VINT),
			.host_id = TISCI_HOST_ID_A53_2,
		},
		{
			.start_resource = 44,
			.num_resource = 14,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_INTAGGR_0,
					TISCI_RESASG_SUBTYPE_IA_VINT),
			.host_id = TISCI_HOST_ID_MAIN_0_R5_0,
		},
		{
			.start_resource = 44,
			.num_resource = 14,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_INTAGGR_0,
					TISCI_RESASG_SUBTYPE_IA_VINT),
			.host_id = TISCI_HOST_ID_MAIN_0_R5_1,
		},
		{
			.start_resource = 58,
			.num_resource = 14,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_INTAGGR_0,
					TISCI_RESASG_SUBTYPE_IA_VINT),
			.host_id = TISCI_HOST_ID_MAIN_0_R5_3,
		},
		{
			.start_resource = 92,
			.num_resource = 14,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_INTAGGR_0,
					TISCI_RESASG_SUBTYPE_IA_VINT),
			.host_id = TISCI_HOST_ID_MAIN_1_R5_1,
		},
		{
			.start_resource = 106,
			.num_resource = 14,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_INTAGGR_0,
					TISCI_RESASG_SUBTYPE_IA_VINT),
			.host_id = TISCI_HOST_ID_MAIN_1_R5_3,
		},
		{
			.start_resource = 168,
			.num_resource = 16,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_INTAGGR_0,
					TISCI_RESASG_SUBTYPE_IA_VINT),
			.host_id = TISCI_HOST_ID_M4_0,
		},
		/* DMASS Interrupt aggregator Global events */
		{
			.start_resource = 15,
			.num_resource = 512,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_INTAGGR_0,
					TISCI_RESASG_SUBTYPE_GLOBAL_EVENT_SEVT),
			.host_id = TISCI_HOST_ID_A53_2,
		},
		{
			.start_resource = 527,
			.num_resource = 256,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_INTAGGR_0,
					TISCI_RESASG_SUBTYPE_GLOBAL_EVENT_SEVT),
			.host_id = TISCI_HOST_ID_MAIN_0_R5_0,
		},
		{
			.start_resource = 527,
			.num_resource = 256,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_INTAGGR_0,
					TISCI_RESASG_SUBTYPE_GLOBAL_EVENT_SEVT),
			.host_id = TISCI_HOST_ID_MAIN_0_R5_1,
		},
		{
			.start_resource = 783,
			.num_resource = 192,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_INTAGGR_0,
					TISCI_RESASG_SUBTYPE_GLOBAL_EVENT_SEVT),
			.host_id = TISCI_HOST_ID_MAIN_0_R5_3,
		},
		{
			.start_resource = 975,
			.num_resource = 256,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_INTAGGR_0,
					TISCI_RESASG_SUBTYPE_GLOBAL_EVENT_SEVT),
			.host_id = TISCI_HOST_ID_MAIN_1_R5_1,
		},
		{
			.start_resource = 1231,
			.num_resource = 192,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_INTAGGR_0,
					TISCI_RESASG_SUBTYPE_GLOBAL_EVENT_SEVT),
			.host_id = TISCI_HOST_ID_MAIN_1_R5_3,
		},
		{
			.start_resource = 1423,
			.num_resource = 96,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_INTAGGR_0,
					TISCI_RESASG_SUBTYPE_GLOBAL_EVENT_SEVT),
			.host_id = TISCI_HOST_ID_M4_0,
		},
		{
			.start_resource = 1519,
			.num_resource = 17,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_INTAGGR_0,
					TISCI_RESASG_SUBTYPE_GLOBAL_EVENT_SEVT),
			.host_id = TISCI_HOST_ID_ALL,
		},
		/* DMASS timer manager event */
		{
			.start_resource = 0,
			.num_resource = 1024,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_INTAGGR_0,
					TISCI_RESASG_SUBTYPE_IA_TIMERMGR_EVT_OES),
			.host_id = TISCI_HOST_ID_ALL,
		},
		/* Packet DMA Tx channel error event */
		{
			.start_resource = 4096,
			.num_resource = 42,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_INTAGGR_0,
					TISCI_RESASG_SUBTYPE_IA_PKTDMA_TX_CHAN_ERROR_OES),
			.host_id = TISCI_HOST_ID_ALL,
		},
		/* Packet DMA Tx flow completion event */
		{
			.start_resource = 4608,
			.num_resource = 112,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_INTAGGR_0,
					TISCI_RESASG_SUBTYPE_IA_PKTDMA_TX_FLOW_COMPLETION_OES),
			.host_id = TISCI_HOST_ID_ALL,
		},
		/* Packet DMA Rx channel error event */
		{
			.start_resource = 5120,
			.num_resource = 29,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_INTAGGR_0,
					TISCI_RESASG_SUBTYPE_IA_PKTDMA_RX_CHAN_ERROR_OES),
			.host_id = TISCI_HOST_ID_ALL,
		},
		/* Packet DMA Rx flow completion event */
		{
			.start_resource = 5632,
			.num_resource = 176,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_INTAGGR_0,
					TISCI_RESASG_SUBTYPE_IA_PKTDMA_RX_FLOW_COMPLETION_OES),
			.host_id = TISCI_HOST_ID_ALL,
		},
		/* Packet DMA Rx flow starvation event */
		{
			.start_resource = 6144,
			.num_resource = 176,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_INTAGGR_0,
					TISCI_RESASG_SUBTYPE_IA_PKTDMA_RX_FLOW_STARVATION_OES),
			.host_id = TISCI_HOST_ID_ALL,
		},
		/* Packet DMA Rx flow firewall event */
		{
			.start_resource = 6656,
			.num_resource = 176,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_INTAGGR_0,
					TISCI_RESASG_SUBTYPE_IA_PKTDMA_RX_FLOW_FIREWALL_OES),
			.host_id = TISCI_HOST_ID_ALL,
		},
		/* Block copy DMA BC channel error event */
		{
			.start_resource = 8192,
			.num_resource = 28,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_INTAGGR_0,
					TISCI_RESASG_SUBTYPE_IA_BCDMA_CHAN_ERROR_OES),
			.host_id = TISCI_HOST_ID_ALL,
		},
		/* Block copy DMA BC channel data completion event */
		{
			.start_resource = 8704,
			.num_resource = 28,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_INTAGGR_0,
					TISCI_RESASG_SUBTYPE_IA_BCDMA_CHAN_DATA_COMPLETION_OES),
			.host_id = TISCI_HOST_ID_ALL,
		},
		/* Block copy DMA BC channel ring completion event */
		{
			.start_resource = 9216,
			.num_resource = 28,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_INTAGGR_0,
					TISCI_RESASG_SUBTYPE_IA_BCDMA_CHAN_RING_COMPLETION_OES),
			.host_id = TISCI_HOST_ID_ALL,
		},
		/* Block copy DMA Tx channel error event */
		{
			.start_resource = 9728,
			.num_resource = 20,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_INTAGGR_0,
					TISCI_RESASG_SUBTYPE_IA_BCDMA_TX_CHAN_ERROR_OES),
			.host_id = TISCI_HOST_ID_ALL,
		},
		/* Block copy DMA Tx channel data completion event */
		{
			.start_resource = 10240,
			.num_resource = 20,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_INTAGGR_0,
					TISCI_RESASG_SUBTYPE_IA_BCDMA_TX_CHAN_DATA_COMPLETION_OES),
			.host_id = TISCI_HOST_ID_ALL,
		},
		/* Block copy DMA Tx channel ring completion event */
		{
			.start_resource = 10752,
			.num_resource = 20,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_INTAGGR_0,
					TISCI_RESASG_SUBTYPE_IA_BCDMA_TX_CHAN_RING_COMPLETION_OES),
			.host_id = TISCI_HOST_ID_ALL,
		},
		/* Block copy DMA Rx channel error event */
		{
			.start_resource = 11264,
			.num_resource = 20,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_INTAGGR_0,
					TISCI_RESASG_SUBTYPE_IA_BCDMA_RX_CHAN_ERROR_OES),
			.host_id = TISCI_HOST_ID_ALL,
		},
		/* Block copy DMA Rx channel data completion event */
		{
			.start_resource = 11776,
			.num_resource = 20,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_INTAGGR_0,
					TISCI_RESASG_SUBTYPE_IA_BCDMA_RX_CHAN_DATA_COMPLETION_OES),
			.host_id = TISCI_HOST_ID_ALL,
		},
		/* Block copy DMA Rx channel ring completion event */
		{
			.start_resource = 12288,
			.num_resource = 20,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_INTAGGR_0,
					TISCI_RESASG_SUBTYPE_IA_BCDMA_RX_CHAN_RING_COMPLETION_OES),
			.host_id = TISCI_HOST_ID_ALL,
		},
		/* DMASS UDMA global config */
		{
			.start_resource = 0,
			.num_resource = 1,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_PKTDMA_0,
					TISCI_RESASG_SUBTYPE_UDMAP_GLOBAL_CONFIG),
			.host_id = TISCI_HOST_ID_ALL,
		},
		/* Packet DMA Free rings for Tx channel */
		{
			.start_resource = 0,
			.num_resource = 4,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_PKTDMA_0,
					TISCI_RESASG_SUBTYPE_PKTDMA_RING_UNMAPPED_TX_CHAN),
			.host_id = TISCI_HOST_ID_A53_2,
		},
		{
			.start_resource = 4,
			.num_resource = 3,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_PKTDMA_0,
					TISCI_RESASG_SUBTYPE_PKTDMA_RING_UNMAPPED_TX_CHAN),
			.host_id = TISCI_HOST_ID_MAIN_0_R5_0,
		},
		{
			.start_resource = 4,
			.num_resource = 3,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_PKTDMA_0,
					TISCI_RESASG_SUBTYPE_PKTDMA_RING_UNMAPPED_TX_CHAN),
			.host_id = TISCI_HOST_ID_MAIN_0_R5_1,
		},
		{
			.start_resource = 7,
			.num_resource = 2,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_PKTDMA_0,
					TISCI_RESASG_SUBTYPE_PKTDMA_RING_UNMAPPED_TX_CHAN),
			.host_id = TISCI_HOST_ID_MAIN_0_R5_3,
		},
		{
			.start_resource = 9,
			.num_resource = 4,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_PKTDMA_0,
					TISCI_RESASG_SUBTYPE_PKTDMA_RING_UNMAPPED_TX_CHAN),
			.host_id = TISCI_HOST_ID_MAIN_1_R5_1,
		},
		{
			.start_resource = 13,
			.num_resource = 2,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_PKTDMA_0,
					TISCI_RESASG_SUBTYPE_PKTDMA_RING_UNMAPPED_TX_CHAN),
			.host_id = TISCI_HOST_ID_MAIN_1_R5_3,
		},
		{
			.start_resource = 15,
			.num_resource = 1,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_PKTDMA_0,
					TISCI_RESASG_SUBTYPE_PKTDMA_RING_UNMAPPED_TX_CHAN),
			.host_id = TISCI_HOST_ID_M4_0,
		},
		/* Packet DMA Rings for CPSW Tx channel */
		{
			.start_resource = 16,
			.num_resource = 64,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_PKTDMA_0,
					TISCI_RESASG_SUBTYPE_PKTDMA_RING_CPSW_TX_CHAN),
			.host_id = TISCI_HOST_ID_A53_2,
		},
        /* Packet DMA Rings for SA2UL Tx channel0 */
		{
			.start_resource = 81,
			.num_resource = 7,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_PKTDMA_0,
					TISCI_RESASG_SUBTYPE_PKTDMA_RING_SAUL_TX_0_CHAN),
			.host_id = TISCI_HOST_ID_ALL,
		},
		/* Packet DMA Rings for SA2UL Tx channel1 */
		{
			.start_resource = 88,
			.num_resource = 8,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_PKTDMA_0,
					TISCI_RESASG_SUBTYPE_PKTDMA_RING_SAUL_TX_1_CHAN),
			.host_id = TISCI_HOST_ID_A53_2,
		},
		/* Packet DMA Rings for ICSSG0 Tx channel */
		{
			.start_resource = 96,
			.num_resource = 8,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_PKTDMA_0,
					TISCI_RESASG_SUBTYPE_PKTDMA_RING_ICSSG_0_TX_CHAN),
			.host_id = TISCI_HOST_ID_A53_2,
		},
		/* Packet DMA Rings for ICSSG1 Tx channel */
		{
			.start_resource = 104,
			.num_resource = 8,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_PKTDMA_0,
					TISCI_RESASG_SUBTYPE_PKTDMA_RING_ICSSG_1_TX_CHAN),
			.host_id = TISCI_HOST_ID_A53_2,
		},
		/* Packet DMA Free rings for Rx channel */
		{
			.start_resource = 112,
			.num_resource = 4,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_PKTDMA_0,
					TISCI_RESASG_SUBTYPE_PKTDMA_RING_UNMAPPED_RX_CHAN),
			.host_id = TISCI_HOST_ID_A53_2,
		},
		{
			.start_resource = 116,
			.num_resource = 3,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_PKTDMA_0,
					TISCI_RESASG_SUBTYPE_PKTDMA_RING_UNMAPPED_RX_CHAN),
			.host_id = TISCI_HOST_ID_MAIN_0_R5_0,
		},
		{
			.start_resource = 116,
			.num_resource = 3,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_PKTDMA_0,
					TISCI_RESASG_SUBTYPE_PKTDMA_RING_UNMAPPED_RX_CHAN),
			.host_id = TISCI_HOST_ID_MAIN_0_R5_1,
		},
		{
			.start_resource = 119,
			.num_resource = 2,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_PKTDMA_0,
					TISCI_RESASG_SUBTYPE_PKTDMA_RING_UNMAPPED_RX_CHAN),
			.host_id = TISCI_HOST_ID_MAIN_0_R5_3,
		},
		{
			.start_resource = 121,
			.num_resource = 4,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_PKTDMA_0,
					TISCI_RESASG_SUBTYPE_PKTDMA_RING_UNMAPPED_RX_CHAN),
			.host_id = TISCI_HOST_ID_MAIN_1_R5_1,
		},
		{
			.start_resource = 125,
			.num_resource = 2,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_PKTDMA_0,
					TISCI_RESASG_SUBTYPE_PKTDMA_RING_UNMAPPED_RX_CHAN),
			.host_id = TISCI_HOST_ID_MAIN_1_R5_3,
		},
		{
			.start_resource = 127,
			.num_resource = 1,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_PKTDMA_0,
					TISCI_RESASG_SUBTYPE_PKTDMA_RING_UNMAPPED_RX_CHAN),
			.host_id = TISCI_HOST_ID_M4_0,
		},
		/* Packet DMA Rings for CPSW Rx channel */
		{
			.start_resource = 128,
			.num_resource = 16,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_PKTDMA_0,
					TISCI_RESASG_SUBTYPE_PKTDMA_RING_CPSW_RX_CHAN),
			.host_id = TISCI_HOST_ID_A53_2,
		},
        /* Packet DMA Rings for SA2UL Rx channel0 */
		{
			.start_resource = 145,
			.num_resource = 7,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_PKTDMA_0,
					TISCI_RESASG_SUBTYPE_PKTDMA_RING_SAUL_RX_0_CHAN),
			.host_id = TISCI_HOST_ID_ALL,
		},
		/* Packet DMA Rings for SA2UL Rx channel1 */
		{
			.start_resource = 144,
			.num_resource = 8,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_PKTDMA_0,
					TISCI_RESASG_SUBTYPE_PKTDMA_RING_SAUL_RX_1_CHAN),
			.host_id = TISCI_HOST_ID_ALL,
		},
		/* Packet DMA Rings for SA2UL Rx channel2 */
		{
			.start_resource = 152,
			.num_resource = 8,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_PKTDMA_0,
					TISCI_RESASG_SUBTYPE_PKTDMA_RING_SAUL_RX_2_CHAN),
			.host_id = TISCI_HOST_ID_A53_2,
		},
		/* Packet DMA Rings for SA2UL Rx channel3 */
		{
			.start_resource = 152,
			.num_resource = 8,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_PKTDMA_0,
					TISCI_RESASG_SUBTYPE_PKTDMA_RING_SAUL_RX_3_CHAN),
			.host_id = TISCI_HOST_ID_A53_2,
		},
		/* Packet DMA Rings for ICSSG0 Rx channel */
		{
			.start_resource = 160,
			.num_resource = 64,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_PKTDMA_0,
					TISCI_RESASG_SUBTYPE_PKTDMA_RING_ICSSG_0_RX_CHAN),
			.host_id = TISCI_HOST_ID_A53_2,
		},
		/* Packet DMA Rings for ICSSG1 Rx channel */
		{
			.start_resource = 224,
			.num_resource = 64,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_PKTDMA_0,
					TISCI_RESASG_SUBTYPE_PKTDMA_RING_ICSSG_1_RX_CHAN),
			.host_id = TISCI_HOST_ID_A53_2,
		},
		/* Packet DMA Free Tx channels */
		{
			.start_resource = 0,
			.num_resource = 4,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_PKTDMA_0,
					TISCI_RESASG_SUBTYPE_PKTDMA_UNMAPPED_TX_CHAN),
			.host_id = TISCI_HOST_ID_A53_2,
		},
		{
			.start_resource = 4,
			.num_resource = 3,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_PKTDMA_0,
					TISCI_RESASG_SUBTYPE_PKTDMA_UNMAPPED_TX_CHAN),
			.host_id = TISCI_HOST_ID_MAIN_0_R5_0,
		},
		{
			.start_resource = 4,
			.num_resource = 3,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_PKTDMA_0,
					TISCI_RESASG_SUBTYPE_PKTDMA_UNMAPPED_TX_CHAN),
			.host_id = TISCI_HOST_ID_MAIN_0_R5_1,
		},
		{
			.start_resource = 7,
			.num_resource = 2,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_PKTDMA_0,
					TISCI_RESASG_SUBTYPE_PKTDMA_UNMAPPED_TX_CHAN),
			.host_id = TISCI_HOST_ID_MAIN_0_R5_3,
		},
		{
			.start_resource = 9,
			.num_resource = 4,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_PKTDMA_0,
					TISCI_RESASG_SUBTYPE_PKTDMA_UNMAPPED_TX_CHAN),
			.host_id = TISCI_HOST_ID_MAIN_1_R5_1,
		},
		{
			.start_resource = 13,
			.num_resource = 2,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_PKTDMA_0,
					TISCI_RESASG_SUBTYPE_PKTDMA_UNMAPPED_TX_CHAN),
			.host_id = TISCI_HOST_ID_MAIN_1_R5_3,
		},
		{
			.start_resource = 15,
			.num_resource = 1,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_PKTDMA_0,
					TISCI_RESASG_SUBTYPE_PKTDMA_UNMAPPED_TX_CHAN),
			.host_id = TISCI_HOST_ID_M4_0,
		},
		/* Packet DMA CPSW Tx channels */
		{
			.start_resource = 16,
			.num_resource = 8,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_PKTDMA_0,
					TISCI_RESASG_SUBTYPE_PKTDMA_CPSW_TX_CHAN),
			.host_id = TISCI_HOST_ID_A53_2,
		},
		/* Packet DMA SA2UL Tx channel1 */
		{
			.start_resource = 25,
			.num_resource = 1,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_PKTDMA_0,
					TISCI_RESASG_SUBTYPE_PKTDMA_SAUL_TX_1_CHAN),
			.host_id = TISCI_HOST_ID_A53_2,
		},
		/* Packet DMA ICSSG0 Tx channels */
		{
			.start_resource = 26,
			.num_resource = 8,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_PKTDMA_0,
					TISCI_RESASG_SUBTYPE_PKTDMA_ICSSG_0_TX_CHAN),
			.host_id = TISCI_HOST_ID_A53_2,
		},
		/* Packet DMA ICSSG1 Tx channels */
		{
			.start_resource = 34,
			.num_resource = 8,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_PKTDMA_0,
					TISCI_RESASG_SUBTYPE_PKTDMA_ICSSG_1_TX_CHAN),
			.host_id = TISCI_HOST_ID_A53_2,
		},
		/* Packet DMA Free Rx channels */
		{
			.start_resource = 0,
			.num_resource = 4,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_PKTDMA_0,
					TISCI_RESASG_SUBTYPE_PKTDMA_UNMAPPED_RX_CHAN),
			.host_id = TISCI_HOST_ID_A53_2,
		},
		{
			.start_resource = 4,
			.num_resource = 3,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_PKTDMA_0,
					TISCI_RESASG_SUBTYPE_PKTDMA_UNMAPPED_RX_CHAN),
			.host_id = TISCI_HOST_ID_MAIN_0_R5_0,
		},
		{
			.start_resource = 4,
			.num_resource = 3,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_PKTDMA_0,
					TISCI_RESASG_SUBTYPE_PKTDMA_UNMAPPED_RX_CHAN),
			.host_id = TISCI_HOST_ID_MAIN_0_R5_1,
		},
		{
			.start_resource = 7,
			.num_resource = 2,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_PKTDMA_0,
					TISCI_RESASG_SUBTYPE_PKTDMA_UNMAPPED_RX_CHAN),
			.host_id = TISCI_HOST_ID_MAIN_0_R5_3,
		},
		{
			.start_resource = 9,
			.num_resource = 4,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_PKTDMA_0,
					TISCI_RESASG_SUBTYPE_PKTDMA_UNMAPPED_RX_CHAN),
			.host_id = TISCI_HOST_ID_MAIN_1_R5_1,
		},
		{
			.start_resource = 13,
			.num_resource = 2,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_PKTDMA_0,
					TISCI_RESASG_SUBTYPE_PKTDMA_UNMAPPED_RX_CHAN),
			.host_id = TISCI_HOST_ID_MAIN_1_R5_3,
		},
		{
			.start_resource = 15,
			.num_resource = 1,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_PKTDMA_0,
					TISCI_RESASG_SUBTYPE_PKTDMA_UNMAPPED_RX_CHAN),
			.host_id = TISCI_HOST_ID_M4_0,
		},
		/* Packet DMA Free flows for Rx channels */
		{
			.start_resource = 0,
			.num_resource = 4,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_PKTDMA_0,
					TISCI_RESASG_SUBTYPE_PKTDMA_FLOW_UNMAPPED_RX_CHAN),
			.host_id = TISCI_HOST_ID_A53_2,
		},
		{
			.start_resource = 4,
			.num_resource = 3,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_PKTDMA_0,
					TISCI_RESASG_SUBTYPE_PKTDMA_FLOW_UNMAPPED_RX_CHAN),
			.host_id = TISCI_HOST_ID_MAIN_0_R5_0,
		},
		{
			.start_resource = 4,
			.num_resource = 3,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_PKTDMA_0,
					TISCI_RESASG_SUBTYPE_PKTDMA_FLOW_UNMAPPED_RX_CHAN),
			.host_id = TISCI_HOST_ID_MAIN_0_R5_1,
		},
		{
			.start_resource = 7,
			.num_resource = 2,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_PKTDMA_0,
					TISCI_RESASG_SUBTYPE_PKTDMA_FLOW_UNMAPPED_RX_CHAN),
			.host_id = TISCI_HOST_ID_MAIN_0_R5_3,
		},
		{
			.start_resource = 9,
			.num_resource = 4,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_PKTDMA_0,
					TISCI_RESASG_SUBTYPE_PKTDMA_FLOW_UNMAPPED_RX_CHAN),
			.host_id = TISCI_HOST_ID_MAIN_1_R5_1,
		},
		{
			.start_resource = 13,
			.num_resource = 2,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_PKTDMA_0,
					TISCI_RESASG_SUBTYPE_PKTDMA_FLOW_UNMAPPED_RX_CHAN),
			.host_id = TISCI_HOST_ID_MAIN_1_R5_3,
		},
		{
			.start_resource = 15,
			.num_resource = 1,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_PKTDMA_0,
					TISCI_RESASG_SUBTYPE_PKTDMA_FLOW_UNMAPPED_RX_CHAN),
			.host_id = TISCI_HOST_ID_M4_0,
		},
		/* Packet DMA CPSW Rx channel */
		{
			.start_resource = 16,
			.num_resource = 1,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_PKTDMA_0,
					TISCI_RESASG_SUBTYPE_PKTDMA_CPSW_RX_CHAN),
			.host_id = TISCI_HOST_ID_A53_2,
		},
		/* Packet DMA CPSW Rx flows */
		{
			.start_resource = 16,
			.num_resource = 16,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_PKTDMA_0,
					TISCI_RESASG_SUBTYPE_PKTDMA_FLOW_CPSW_RX_CHAN),
			.host_id = TISCI_HOST_ID_A53_2,
		},
		/* Packet DMA SA2UL Rx channel0 flows */
		{
			.start_resource = 32,
			.num_resource = 8,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_PKTDMA_0,
					TISCI_RESASG_SUBTYPE_PKTDMA_FLOW_SAUL_RX_0_CHAN),
			.host_id = TISCI_HOST_ID_ALL,
		},
		/* Packet DMA SA2UL Rx channel1 flows */
		{
			.start_resource = 32,
			.num_resource = 8,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_PKTDMA_0,
					TISCI_RESASG_SUBTYPE_PKTDMA_FLOW_SAUL_RX_1_CHAN),
			.host_id = TISCI_HOST_ID_ALL,
		},
		/* Packet DMA SA2UL Rx channel2 */
		{
			.start_resource = 19,
			.num_resource = 1,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_PKTDMA_0,
					TISCI_RESASG_SUBTYPE_PKTDMA_SAUL_RX_2_CHAN),
			.host_id = TISCI_HOST_ID_A53_2,
		},
		/* Packet DMA SA2UL Rx channel2 flows */
		{
			.start_resource = 40,
			.num_resource = 8,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_PKTDMA_0,
					TISCI_RESASG_SUBTYPE_PKTDMA_FLOW_SAUL_RX_2_CHAN),
			.host_id = TISCI_HOST_ID_A53_2,
		},
		/* Packet DMA SA2UL Rx channel3 */
		{
			.start_resource = 20,
			.num_resource = 1,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_PKTDMA_0,
					TISCI_RESASG_SUBTYPE_PKTDMA_SAUL_RX_3_CHAN),
			.host_id = TISCI_HOST_ID_A53_2,
		},
		/* Packet DMA SA2UL Rx channel3 flows */
		{
			.start_resource = 40,
			.num_resource = 8,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_PKTDMA_0,
					TISCI_RESASG_SUBTYPE_PKTDMA_FLOW_SAUL_RX_3_CHAN),
			.host_id = TISCI_HOST_ID_A53_2,
		},
		/* Packet DMA ICSSG0 Rx channel */
		{
			.start_resource = 21,
			.num_resource = 4,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_PKTDMA_0,
					TISCI_RESASG_SUBTYPE_PKTDMA_ICSSG_0_RX_CHAN),
			.host_id = TISCI_HOST_ID_A53_2,
		},
		/* Packet DMA ICSSG0 Rx flows */
		{
			.start_resource = 48,
			.num_resource = 64,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_PKTDMA_0,
					TISCI_RESASG_SUBTYPE_PKTDMA_FLOW_ICSSG_0_RX_CHAN),
			.host_id = TISCI_HOST_ID_A53_2,
		},
		/* Packet DMA ICSSG1 Rx channel */
		{
			.start_resource = 25,
			.num_resource = 4,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_PKTDMA_0,
					TISCI_RESASG_SUBTYPE_PKTDMA_ICSSG_1_RX_CHAN),
			.host_id = TISCI_HOST_ID_A53_2,
		},
		/* Packet DMA ICSSG1 Rx flows */
		{
			.start_resource = 112,
			.num_resource = 64,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_PKTDMA_0,
					TISCI_RESASG_SUBTYPE_PKTDMA_FLOW_ICSSG_1_RX_CHAN),
			.host_id = TISCI_HOST_ID_A53_2,
		},
		/* Packet DMA Ring accelerator error event */
		{
			.start_resource = 0,
			.num_resource = 1,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_RINGACC_0,
					TISCI_RESASG_SUBTYPE_RA_ERROR_OES),
			.host_id = TISCI_HOST_ID_ALL,
		},
		/* Packet DMA virt_id range */
		{
			.start_resource = 2,
			.num_resource = 2,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_RINGACC_0,
					TISCI_RESASG_SUBTYPE_RA_VIRTID),
			.host_id = TISCI_HOST_ID_A53_2,
		},
		/* Packet DMA Rings for IPC */
		{
			.start_resource = 20,
			.num_resource = 2,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_RINGACC_0,
					TISCI_RESASG_SUBTYPE_RA_GENERIC_IPC),
			.host_id = TISCI_HOST_ID_MAIN_0_R5_0,
		},
		{
			.start_resource = 20,
			.num_resource = 2,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_RINGACC_0,
					TISCI_RESASG_SUBTYPE_RA_GENERIC_IPC),
			.host_id = TISCI_HOST_ID_MAIN_0_R5_1,
		},
		{
			.start_resource = 22,
			.num_resource = 2,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_RINGACC_0,
					TISCI_RESASG_SUBTYPE_RA_GENERIC_IPC),
			.host_id = TISCI_HOST_ID_MAIN_0_R5_3,
		},
		{
			.start_resource = 24,
			.num_resource = 2,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_RINGACC_0,
					TISCI_RESASG_SUBTYPE_RA_GENERIC_IPC),
			.host_id = TISCI_HOST_ID_MAIN_1_R5_1,
		},
		{
			.start_resource = 26,
			.num_resource = 2,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_RINGACC_0,
					TISCI_RESASG_SUBTYPE_RA_GENERIC_IPC),
			.host_id = TISCI_HOST_ID_MAIN_1_R5_3,
		},
		{
			.start_resource = 28,
			.num_resource = 4,
			.type = TISCI_RESASG_UTYPE (TISCI_DEV_DMASS0_RINGACC_0,
					TISCI_RESASG_SUBTYPE_RA_GENERIC_IPC),
			.host_id = TISCI_HOST_ID_ALL,
		},
	},
};
