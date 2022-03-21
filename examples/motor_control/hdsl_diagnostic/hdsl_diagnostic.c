/*
 *  Copyright (C) 2021 Texas Instruments Incorporated
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
 *  "AS IS" AND ANY EXPgResS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
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


#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

#include <kernel/dpl/DebugP.h>
#include <drivers/pruicss.h>
#include <motor_control/position_sense/hdsl/include/hdsl_interface.h>
#include <motor_control/position_sense/hdsl/firmware/hdsl_master_icssg_bin.h>
#include <motor_control/position_sense/hdsl/firmware/hdsl_master_icssg_sync_bin.h>
#include "pruss_intc_mapping.h"
#include "hdsl_diagnostic.h"
#include "ti_drivers_config.h"
#include "ti_drivers_open_close.h"
#include "ti_board_open_close.h"

#ifdef HDSL_AM64xE1_TRANSCEIVER
#include <board/ioexp/ioexp_tca6424.h>
#endif

#define PRUICSS_PRUx  PRUICSS_PRU1

#define ENDAT_EN (0x1 << 26)
/* OCP as clock, div 24 */
#define ENDAT_TX_CFG (0x10 | (23 << 16))
/* OCP as clock, div 3, 8x OSR */
#define ENDAT_RX_CFG (0x10 | (2 << 16) | 7 | 0x08)
#define CTR_EN (1 << 3)
#define MAX_WAIT 20000

/*TSR configuration:*/

/*inEvent value:*/
/* ICSSG_0_EDC1_SYNC0 ICSSG0 IEP1 sync event 0 Pulse */
#define SYNCEVENT_INTRTR_IN_27 27

/*outEvent values:*/
/*SYNC0_OUT Pin Selectable timesync event 24 Edge (4+(24*4)) */
#define SYNCEVT_RTR_SYNC28_EVT 0x64
/* SYNC1_OUT Pin Selectable timesync event 25 Edge (4+(25*4)) */
#define SYNCEVT_RTR_SYNC29_EVT 0x68
/* SYNC2_OUT Pin Selectable timesync event 26 Edge (4+(26*4)) */
#define SYNCEVT_RTR_SYNC30_EVT 0x6C
/* SYNC3_OUT Pin Selectable timesync event 27 Edge (4+(27*4)) */
#define SYNCEVT_RTR_SYNC31_EVT 0x70
/* ICSSG0_PR1_EDC1_LATCH0_IN PRU_ICSSG0 (4+(10*4)) */
#define SYNCEVT_RTR_SYNC10_EVT 0x2C

extern PRUICSS_Config gPruicssConfig[2];

static char gUart_buffer[256];

struct hdslInterface *hdslInterface;
struct hdslInterface *hdslInterface2;
static uint32_t gMulti_turn, gRes;
static uint64_t gMask;
static uint32_t gPc_addr, gPc_data;

/** \brief Global Structure pointer holding PRUSS1 memory Map. */
PRUICSS_Handle gPruIcss0Handle;
PRUICSS_IntcInitData gPruss0_intc_initdata = PRU_ICSS0_INTC_INITDATA;
PRUICSS_Handle gPruIcss1Handle;
PRUICSS_IntcInitData gPruss1_intc_initdata = PRU_ICSS1_INTC_INITDATA;

static void *gPru_cfg, *gPru_ctrl;
void *gPru_dramx;

#ifdef HDSL_AM64xE1_TRANSCEIVER
static TCA6424_Config  gTCA6424_Config;
#endif

void hdsl_pruss_init(void)
{
    gPruIcss0Handle = PRUICSS_open(CONFIG_PRU_ICSS0);

    gPruIcss1Handle = PRUICSS_open(CONFIG_PRU_ICSS1);

    PRUICSS_disableCore(gPruIcss0Handle, PRUICSS_PRUx);

    /* clear ICSS0 PRU data RAM */
    gPru_dramx = (void *)((((PRUICSS_HwAttrs *)(gPruIcss0Handle->hwAttrs))->baseAddr) + PRUICSS_DATARAM(PRUICSS_PRUx));
    memset(gPru_dramx, 0, (4 * 1024));

    gPru_cfg = (void *)(((PRUICSS_HwAttrs *)(gPruIcss0Handle->hwAttrs))->cfgRegBase);

    HW_WR_REG32(gPru_cfg + CSL_ICSSCFG_GPCFG1, ENDAT_EN);

    HW_WR_REG32(gPru_cfg + CSL_ICSSCFG_EDPRU1TXCFGREGISTER, ENDAT_TX_CFG);

    HW_WR_REG32(gPru_cfg + CSL_ICSSCFG_EDPRU1RXCFGREGISTER, ENDAT_RX_CFG);

    PRUICSS_intcInit(gPruIcss0Handle, &gPruss0_intc_initdata);

    PRUICSS_intcInit(gPruIcss1Handle, &gPruss1_intc_initdata);

    /* configure C28 to PRU_ICSS_CTRL and C29 to EDMA + 0x1000 */
    /*6.4.14.1.1 ICSSG_PRU_CONTROL RegisterPRU_ICSSG0_PR1_PDSP0_IRAM 00B0 2400h*/
    PRUICSS_setConstantTblEntry(gPruIcss0Handle, PRUICSS_PRUx, PRUICSS_CONST_TBL_ENTRY_C28, 0x0240);
    /*IEP1 base */
    PRUICSS_setConstantTblEntry(gPruIcss0Handle, PRUICSS_PRUx, PRUICSS_CONST_TBL_ENTRY_C29, 0x0002F000);

    /* enable cycle counter */
    gPru_ctrl =  (void *)((((PRUICSS_HwAttrs *)(gPruIcss0Handle->hwAttrs))->baseAddr) + CSL_ICSS_G_PR1_PDSP1_IRAM_REGS_BASE);

    HW_WR_REG32(gPru_ctrl, CTR_EN);

    hdslInterface = (struct hdslInterface *)gPru_dramx;
    hdslInterface2 = (struct hdslInterface *)(gPru_dramx + 0x90);
}


 void hdsl_iep_init(void)
{
    void *pru_iep;

    pru_iep = (void *)(((PRUICSS_HwAttrs *)(gPruIcss0Handle->hwAttrs))->iep0RegBase);

    HW_WR_REG32(pru_iep + CSL_ICSS_G_PR1_IEP1_SLV_GLOBAL_CFG_REG,
                (CSL_ICSS_G_PR1_IEP1_SLV_GLOBAL_CFG_REG_CNT_ENABLE_MASK | (1 << CSL_ICSS_G_PR1_IEP1_SLV_GLOBAL_CFG_REG_DEFAULT_INC_SHIFT)));
    /* Use OCP as IEP CLK src */
    HW_WR_REG32(gPru_cfg + CSL_ICSSCFG_IEPCLK, CSL_ICSSCFG_IEPCLK_OCP_EN_MASK);
}


void hdsl_enable_sync_signal(void)
{
    /*programm here*/
    uint32_t start_time = 10000;
    uint32_t period;
    uint32_t ES;
    uint32_t inEvent;
    uint32_t outEvent_latch;
    uint32_t outEvent_gpio;
    uint32_t iep_base;

    DebugP_log("\r\nEnter ES(number of frames per sync period), note ES=0 for FREE RUN mode: ");
    DebugP_scanf("%d", &ES);
    hdslInterface->SYNC_CTRL = ES;
    if(ES == 0)
    {
        return;
    }
    else
    {
        DebugP_log("\r\nEnter period for SYNC PULSE in unit of cycles(1 cycle = 4.44ns):");
        DebugP_scanf("%d",&period);
    }

    iep_base = CSL_PRU_ICSSG0_DRAM0_SLV_RAM_BASE + CSL_ICSS_G_PR1_IEP1_SLV_REGS_BASE;
    /*Enable IEP. Enable the Counter and set the DEFAULT_INC and CMP_INC to 1.*/
    HWREG(CSL_PRU_ICSSG0_DRAM0_SLV_RAM_BASE + CSL_ICSS_G_PR1_IEP1_SLV_REGS_BASE + CSL_ICSS_G_PR1_IEP1_SLV_GLOBAL_CFG_REG) = 0x111;

    /*Enable SYNC0 and program pulse width*/
    /*Enable SYNC and SYNC0*/
    HWREG(iep_base + CSL_ICSS_G_PR1_IEP1_SLV_SYNC_CTRL_REG) |= 0x03;
    /*Enable cyclic mod*/
    HWREG(iep_base + CSL_ICSS_G_PR1_IEP1_SLV_SYNC_CTRL_REG) |= 0x20;
    /*32504 4500 = num_of_cycles, 50Khz signals, 20us time period//(pulse_width / 4) - 1; */
    HWREG(iep_base + CSL_ICSS_G_PR1_IEP1_SLV_SYNC_PWIDTH_REG) = (period)/2;
    /* (period / 4) - 1; */
    HWREG(iep_base + CSL_ICSS_G_PR1_IEP1_SLV_SYNC0_PERIOD_REG) =(period);

    /*Program CMP1*/
    HWREG(iep_base + CSL_ICSS_G_PR1_IEP1_SLV_CMP_CFG_REG) |= 0x00000004;
    /*<start time>; Ensure this start time is in future*/
    HWREG(iep_base + CSL_ICSS_G_PR1_IEP1_SLV_CMP1_REG0) = start_time;

    /*TSR configuration:*/
    inEvent = SYNCEVENT_INTRTR_IN_27;
    outEvent_latch = SYNCEVT_RTR_SYNC10_EVT;
    outEvent_gpio = SYNCEVT_RTR_SYNC30_EVT;

    HWREG(CSL_TIMESYNC_EVENT_INTROUTER0_CFG_BASE + outEvent_latch) = inEvent | 0x10000;
    HWREG(CSL_TIMESYNC_EVENT_INTROUTER0_CFG_BASE + outEvent_gpio) = inEvent | 0x10000;
    HWREG(CSL_TIMESYNC_EVENT_INTROUTER0_CFG_BASE + SYNCEVT_RTR_SYNC28_EVT) = inEvent | 0x10000;
}

void hdsl_sync_calculation(void)
{
    if(hdslInterface->SYNC_CTRL == 0)
    {
        DebugP_log("\r\nFREE RUN MODE\n");
        return;
    }
    uint8_t ES;
    uint16_t wait_before_start;
    uint32_t counter, period, index;
    volatile uint32_t cap6_rise0, cap6_rise1, cap6_fall0, cap6_fall1;
    uint8_t EXTRA_EDGE_ARR[8] = {0x00 ,0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE};
    uint32_t minm_bits = 112, cycle_per_bit = 24, max_stuffing = 26, stuffing_size = 6, cycle_per_overclock_bit =3, minm_extra_size = 4, sync_param_mem_start = 0xDC;
    uint32_t cycles_left, additional_bits, minm_cycles, time_gRest, extra_edge, extra_size, num_of_stuffing, extra_size_remainder, stuffing_remainder, bottom_up_cycles;

    /*measure of SYNC period starts*/
    ES = hdslInterface->SYNC_CTRL;
    volatile uint32_t* carp6_rise_addr =   (uint32_t*)(CSL_PRU_ICSSG0_DRAM0_SLV_RAM_BASE + CSL_ICSS_G_PR1_IEP1_SLV_REGS_BASE + CSL_ICSS_G_PR1_IEP0_SLV_CAPR6_REG0);
    volatile uint32_t* carp6_fall_addr =   (uint32_t*)(CSL_PRU_ICSSG0_DRAM0_SLV_RAM_BASE + CSL_ICSS_G_PR1_IEP1_SLV_REGS_BASE + CSL_ICSS_G_PR1_IEP0_SLV_CAPF6_REG0);
    cap6_rise0 = *(carp6_rise_addr);
    cap6_fall0 = *(carp6_fall_addr);
    cap6_rise1 = cap6_rise0;
    cap6_fall1 = cap6_fall0;
    counter = 0;

    for(index = 0 ; index <2 ; index++)
    {
        cap6_rise0 = cap6_rise1;
        cap6_fall0 = cap6_fall1;
        while((cap6_fall0 == cap6_fall1) || (cap6_rise0 == cap6_rise1))
        {
            cap6_rise1 = *(carp6_rise_addr);
            cap6_fall1 = *(carp6_fall_addr);
            counter++;
            if(counter > MAX_WAIT)
            {
                DebugP_log("\rSYNC PULSE NOT FOUND, WAITING FOR SYNC PULSE\n");
                counter = 0;
            }
        }
    }

    period = cap6_rise1 - cap6_rise0;
    /*measure of SYNC period ends*/

    minm_cycles = minm_bits * ES * cycle_per_bit;
    cycles_left = period - minm_cycles;
    time_gRest = (cycles_left % cycle_per_bit) / cycle_per_overclock_bit;
    additional_bits = cycles_left / cycle_per_bit;
    extra_edge = EXTRA_EDGE_ARR[time_gRest];
    num_of_stuffing = additional_bits / stuffing_size;
    extra_size = additional_bits % stuffing_size;
    extra_size = extra_size + minm_extra_size * ES;
    if(num_of_stuffing > ES * max_stuffing)
    {
        extra_size = extra_size + (((num_of_stuffing) - (max_stuffing * ES)) * stuffing_size);
        num_of_stuffing = ES * max_stuffing;
    }
    extra_size_remainder = extra_size % ES;
    extra_size = extra_size / ES;
    stuffing_remainder = num_of_stuffing % ES;
    num_of_stuffing = num_of_stuffing / ES;
    bottom_up_cycles = (minm_cycles - minm_extra_size * ES * cycle_per_bit);
    bottom_up_cycles = bottom_up_cycles + (stuffing_size * (ES * num_of_stuffing + stuffing_remainder))*cycle_per_bit;
    bottom_up_cycles = bottom_up_cycles + ((ES * extra_size  + extra_size_remainder) * cycle_per_bit ) + time_gRest * cycle_per_overclock_bit;
    wait_before_start = (84 * cycle_per_bit)+((8 - time_gRest)*cycle_per_overclock_bit)+(num_of_stuffing * stuffing_size * cycle_per_bit);
    if(stuffing_remainder != 0)
    {
        wait_before_start = wait_before_start+(stuffing_size * cycle_per_bit);
    }
    wait_before_start = wait_before_start - 51;
    if(extra_size < 4 || extra_size > 9)
    {
        DebugP_log("\rERROR: ES or period selected is Invalid \n");
        return;
    }
    DebugP_log("\r********************************************************************\n");
    DebugP_log("\rSYNC MODE: period = %d\n", period);
    DebugP_log("\rSYNC MODE: ES = %d\n", ES);
    DebugP_log("\rSYNC MODE: counter = %d\n", counter);
    DebugP_log("\rSYNC MODE: wait_before_start = %d\n", wait_before_start);
    DebugP_log("\rSYNC MODE: bottom_up_cycles = %d\n", bottom_up_cycles);
    DebugP_log("\rSYNC MODE: extra_size = %d\n", extra_size);
    DebugP_log("\rSYNC MODE: temp_gRest = %d\n", time_gRest);
    DebugP_log("\rSYNC MODE: extra_edge = %d\n", extra_edge);
    DebugP_log("\rSYNC MODE: num_of_stuffing = %d\n", num_of_stuffing);
    DebugP_log("\rSYNC MODE: extra_size_remainder = %d\n", extra_size_remainder);
    DebugP_log("\rSYNC MODE: stuffing_remainder = %d\n", stuffing_remainder);
    DebugP_log("\r********************************************************************\n");

    sync_param_mem_start =sync_param_mem_start + (uint32_t)gPru_dramx;
    HWREGB(sync_param_mem_start) = extra_size;
    sync_param_mem_start = sync_param_mem_start + 1;
    HWREGB(sync_param_mem_start) = num_of_stuffing;
    sync_param_mem_start = sync_param_mem_start + 1;
    HWREGB(sync_param_mem_start) = extra_edge;
    sync_param_mem_start = sync_param_mem_start + 1;
    HWREGB(sync_param_mem_start) = time_gRest;
    sync_param_mem_start = sync_param_mem_start + 1;
    HWREGB(sync_param_mem_start) = extra_size_remainder;
    sync_param_mem_start = sync_param_mem_start + 1;
    HWREGB(sync_param_mem_start) = stuffing_remainder;
    sync_param_mem_start = sync_param_mem_start + 1;
    HWREGH(sync_param_mem_start) = wait_before_start;
}


void hdsl_pruss_load_run_fw(void)
{
    PRUICSS_disableCore(gPruIcss0Handle, PRUICSS_PRUx);

    if(hdslInterface->SYNC_CTRL == 0)
    {
        /*free run*/
        PRUICSS_writeMemory(gPruIcss0Handle, PRUICSS_IRAM_PRU(PRUICSS_PRUx),
                    0, (uint32_t *) Hiperface_DSL2_0,
                    sizeof(Hiperface_DSL2_0));
    }
    else
    {
        /*sync_mode*/
        PRUICSS_writeMemory(gPruIcss0Handle, PRUICSS_IRAM_PRU(PRUICSS_PRUx),
                        0, (uint32_t *) Hiperface_DSL_SYNC2_0,
                        sizeof(Hiperface_DSL_SYNC2_0));
    }

    PRUICSS_resetCore(gPruIcss0Handle, PRUICSS_PRUx);

    /*Run firmware*/
    PRUICSS_enableCore(gPruIcss0Handle, PRUICSS_PRUx);
}

static void hdsl_init()
{
    hdsl_pruss_init();

    hdsl_iep_init();

    hdsl_enable_sync_signal();

    hdsl_generate_memory_image();

    hdsl_sync_calculation();

    hdsl_pruss_load_run_fw();
}


static void hdsl_get_pos(bool is_vpos)
{
    uint64_t val, val2;
    uint8_t ureg, ureg1;
    float pos, pos2;

    if (is_vpos == true)
    {
        /* Safe Position 1 */
        val = hdslInterface->VPOS0 | (hdslInterface->VPOS1 << 8) |
               (hdslInterface->VPOS2 << 16) | (hdslInterface->VPOS3 << 24);
        val |= (uint64_t)hdslInterface->VPOS4 << 32;

        /* Safe Position 2 */
        val2 = hdslInterface2->VPOS20 | (hdslInterface2->VPOS21 << 8) |
               (hdslInterface2->VPOS22 << 16) | (hdslInterface2->VPOS23 << 24);
        val2 |= (uint64_t)hdslInterface2->VPOS24 << 32;

        pos2 = (float)(val2 & gMask) / (float)(gMask + 1) * (float)360;
    }
    else
    {
        /* Fast Position */
        val = hdslInterface->POS0 | (hdslInterface->POS1 << 8) |
               (hdslInterface->POS2 << 16) | (hdslInterface->POS3 << 24);
        val |= (uint64_t)hdslInterface->POS4 << 32;
    }
    pos = (float)(val & gMask) / (float)(gMask + 1) * (float)360;

    ureg = (hdslInterface->DELAY & 0xF0) >> 4;
    ureg1 = hdslInterface->MASTER_QM & 0xF;

    if (gMulti_turn)
    {
        uint64_t turn, turn2;

        turn = val & ~gMask;
        turn >>= gRes;

        turn2 = val2 & ~gMask;
        turn2 >>= gRes;

        sprintf(gUart_buffer, "| Angle: %10.6f\tTurn: %llu\t RSSI: %u\n\r| SafePos2: %10.6f\tTurn: %llu\t QM:  %u\n ", pos, turn, ureg, pos2, turn2, ureg1 );
    }
    else
    {
        sprintf(gUart_buffer, "| Angle: %10.6f\n", pos);
    }
}

static void hdsl_get_vpos(void)
{
    hdsl_get_pos(true);
}

static void hdsl_get_qm(void)
{
    uint8_t ureg = hdslInterface->MASTER_QM & 0xF;

    sprintf(gUart_buffer, "| Quality monitoring value: %u\n", ureg);
}

static void hdsl_get_events(void)
{
    uint16_t ureg = hdslInterface->EVENT_L | (hdslInterface->EVENT_H << 8);

    sprintf(gUart_buffer, "| Events: 0x%x\n", ureg);
}

static void hdsl_get_sum(void)
{
    uint8_t ureg = hdslInterface->SUMMARY;

    sprintf(gUart_buffer, "| Summarized slave status: 0x%x\n", ureg);
}

static void hdsl_get_acc_err_cnt(void)
{
    uint8_t ureg = hdslInterface->ACC_ERR_CNT & 0x1F;

    sprintf(gUart_buffer, "| Acceleration error counter: %u\n", ureg);
}

static void hdsl_get_rssi(void)
{
    uint8_t ureg = (hdslInterface->DELAY & 0xF0) >> 4;

    sprintf(gUart_buffer, "| RSSI: %u\n", ureg);
}

static void hdsl_write_pc_short_msg(void)
{
    hdslInterface->PC_DATA = gPc_data;
    hdslInterface->SLAVE_REG_CTRL = gPc_addr;

    sprintf(gUart_buffer, "| Parameter channel short message written\n");
}

enum {
    MENU_SAFE_POSITION,
    MENU_QUALITY_MONITORING,
    MENU_EVENTS,
    MENU_SUMMARY,
    MENU_ACC_ERR_CNT,
    MENU_RSSI,
    MENU_PC_SHORT_MSG_WRITE,
    MENU_LIMIT,
    MENU_INVALID,
};

static void hdsl_display_menu(void)
{
    DebugP_log("\r|------------------------------------------------------------------------------|\n");
    DebugP_log("\r|                                    MENU                                      |\n");
    DebugP_log("\r|------------------------------------------------------------------------------|\n");
    DebugP_log("\r| %2d : Safe position                                                           |\n", MENU_SAFE_POSITION);
    DebugP_log("\r| %2d : Quality monitoring                                                      |\n", MENU_QUALITY_MONITORING);
    DebugP_log("\r| %2d : Events                                                                  |\n", MENU_EVENTS);
    DebugP_log("\r| %2d : Summarized slave status                                                 |\n", MENU_SUMMARY);
    DebugP_log("\r| %2d : Acceleration error counter                                              |\n", MENU_ACC_ERR_CNT);
    DebugP_log("\r| %2d : RSSI                                                                    |\n", MENU_RSSI);
    DebugP_log("\r| %2d : Parameter channel short message write                                   |\n", MENU_PC_SHORT_MSG_WRITE);
    DebugP_log("\r|------------------------------------------------------------------------------|\n");
    DebugP_log("\r| enter value: ");
}

static int hdsl_get_menu(void)
{
    unsigned int cmd;

    if(DebugP_scanf("%d\n", &cmd) < 0 || cmd >= MENU_LIMIT)
    {
        DebugP_log("| WARNING: invalid option, Safe position selected\r\n");
        cmd = MENU_SAFE_POSITION;
    }

    if (cmd == MENU_PC_SHORT_MSG_WRITE)
    {
        DebugP_log("\r| enter addgRess (hex value): ");
        if(DebugP_scanf("%x\n", &gPc_addr) < 0 || gPc_addr > 0x3f)
        {
            DebugP_log("\r| WARNING: invalid addgRess\n|\n|\n");
            return MENU_INVALID;
        }
        DebugP_log("\r| enter data (hex value): ");
        if (DebugP_scanf("%x\n", &gPc_data) < 0 || gPc_data > 0xff)
        {
            DebugP_log("\r| WARNING: invalid data\n|\n|\n");
            return MENU_INVALID;
        }
    }

    return cmd;
}

static void hdsl_process_request(int menu)
{
    switch(menu)
    {
        case MENU_SAFE_POSITION:
            hdsl_get_vpos();
            break;
        case MENU_QUALITY_MONITORING:
            hdsl_get_qm();
            break;
        case MENU_EVENTS:
            hdsl_get_events();
            break;
        case MENU_SUMMARY:
            hdsl_get_sum();
            break;
        case MENU_ACC_ERR_CNT:
            hdsl_get_acc_err_cnt();
            break;
        case MENU_RSSI:
            hdsl_get_rssi();
            break;
        case MENU_PC_SHORT_MSG_WRITE:
            hdsl_write_pc_short_msg();
            break;
        default:
            sprintf(gUart_buffer, "\r| ERROR: invalid request\n");
            break;
    }
}

#ifdef HDSL_AM64xE1_TRANSCEIVER
static void hdsl_i2c_io_expander(void *args)
{
    int32_t             status = SystemP_SUCCESS;
    /* P20 = LED 3 bits, pin, 2 bits port.*/
    uint32_t            ioIndex = 0x10;
    TCA6424_Params      tca6424Params;

    TCA6424_Params_init(&tca6424Params);

    status = TCA6424_open(&gTCA6424_Config, &tca6424Params);

    if(status == SystemP_SUCCESS)
    {
        /* Set output to HIGH before config so that LED start with On state */
        status = TCA6424_setOutput(
                     &gTCA6424_Config,
                     ioIndex,
                     TCA6424_OUT_STATE_HIGH);

        /* Configure as output  */
        status += TCA6424_config(
                      &gTCA6424_Config,
                      ioIndex,
                      TCA6424_MODE_OUTPUT);
        /* set P12 high which controls CPSW_FET_SEL -> enable PRU1 and PRU0 GPIOs */
        ioIndex = 0x0a;
        status = TCA6424_setOutput(
                     &gTCA6424_Config,
                     ioIndex,
                     TCA6424_OUT_STATE_HIGH);

        /* Configure as output  */
        status += TCA6424_config(
                      &gTCA6424_Config,
                      ioIndex,
                      TCA6424_MODE_OUTPUT);


    }
    TCA6424_close(&gTCA6424_Config);
}
#endif

void hdsl_diagnostic_main(void *arg)
{
    uint32_t val, acc_bits, pos_bits;
    uint8_t ureg;

    /* Open drivers to open the UART driver for console */
    Drivers_open();
    Board_driversOpen();

    #ifndef HDSL_AM64xE1_TRANSCEIVER
    /* Configure g_mux_en to 1 in ICSSG_SA_MX_REG Register. This is required to remap EnDAT signals correctly via Interface card.*/
    HW_WR_REG32((CSL_PRU_ICSSG0_PR1_CFG_SLV_BASE+0x40), (0x80));

    /*Configure GPIO42 for HDSL mode.*/
    GPIO_setDirMode(CONFIG_GPIO0_BASE_ADDR, CONFIG_GPIO0_PIN, CONFIG_GPIO0_DIR);
    GPIO_pinWriteHigh(CONFIG_GPIO0_BASE_ADDR, CONFIG_GPIO0_PIN);
    #else
    /* Configure g_mux_en to 0 in ICSSG_SA_MX_REG Register. */
    HW_WR_REG32((CSL_PRU_ICSSG0_PR1_CFG_SLV_BASE+0x40), (0x00));
    /*Configure GPIO42 for HDSL mode. New transceiver card needs the pin to be configured as input*/
    HW_WR_REG32(0x000F41D4, 0x00050001);   /* PRG0_PRU1_GPI9 as input */
    hdsl_i2c_io_expander(NULL);
    #endif

    DebugP_logInfo("\n\nHiperface DSL diagnostic\n");
    hdsl_init();
    DebugP_logInfo("\r\nHDSL setup finished\n");
    /*need some extra time for SYNC mode since frames are longer*/
    ClockP_sleep(1);

    for (ureg = hdslInterface->MASTER_QM, val = 0; !(ureg & 0x80); ureg = hdslInterface->MASTER_QM, val++, ClockP_usleep(10))
    {
        if (val > 100)
        { /* wait 1ms to detect, increase if reqd. */
            while(1)
            {
                DebugP_log("\r\nHiperface DSL encoder not detected\n\n");
                ClockP_usleep(5000);
            }
        }
    }

    DebugP_log("\r\n");
    DebugP_log("\r|------------------------------------------------------------------------------|\n");
    DebugP_log("\r|                            Hiperface DSL diagnostic                          |\n");
    DebugP_log("\r|------------------------------------------------------------------------------|\n");
    DebugP_log("\r|\n");
    DebugP_log("\r| Quality monitoring value: %u\n", ureg & 0xF);

    ureg = hdslInterface->EDGES;
    DebugP_log("\r| Edges: 0x%x\n", ureg);

    ureg = hdslInterface->DELAY;
    DebugP_log("\r| Cable delay: %u\tRSSI: %u\n", ureg & 0xF, (ureg & 0xF0) >> 4);

    val = hdslInterface->ENC_ID0 | (hdslInterface->ENC_ID1 << 8) |
              (hdslInterface->ENC_ID2 << 16);
    acc_bits = val & 0xF;
    acc_bits += 8;
    pos_bits = (val & 0x3F0) >> 4;
    pos_bits += acc_bits;
    DebugP_log("\r| Encoder ID: 0x%x", val);
    DebugP_log("(");
    DebugP_log("Acceleration bits: %u ,", acc_bits);
    DebugP_log("Position bits: %u,", pos_bits);
    DebugP_log("%s", val & 0x400 ? " Bipolar position" : " Unipolar position");
    DebugP_log(")\r|\n");

    DebugP_log("\r| Enter single turn bits: ");
    if((DebugP_scanf("%d\n", &gRes) < 0) || gRes > pos_bits)
    {
            DebugP_log("\r| WARNING: invalid single turn bits, assuming single turn encoder\n");
            gRes = pos_bits;
    }
    gMulti_turn = pos_bits - gRes;
    gMask = pow(2, gRes) - 1;
    if (gMulti_turn)
        DebugP_log("\r| Multi turn bits: %u\n", gMulti_turn);

    while(1)
    {
        int menu;

        hdsl_display_menu();
        menu = hdsl_get_menu();
        hdsl_process_request(menu);
        DebugP_log("|\r\n");
        DebugP_log("\r%s", gUart_buffer);
        DebugP_log("\r|\n\r|\n\r|\n");
    }

    Board_driversClose();
    Drivers_close();
}

