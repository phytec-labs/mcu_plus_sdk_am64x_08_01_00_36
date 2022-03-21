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

/**
 *  \file ospi_v0.c
 *
 *  \brief File containing OSPI Driver APIs implementation for version V0.
 *
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

/* This is needed for memset/memcpy */
#include <string.h>
#include <drivers/ospi.h>
#include <drivers/ospi/ospi_sfdp.h>
#include <drivers/hw_include/cslr.h>

/* ========================================================================== */
/*                             Macro Definitions                              */
/* ========================================================================== */

#define OSPI_SFDP_GET_BITFIELD(val, start, end) ( (val >> start) & ((1U << (end-start+1))-1) )

/* ========================================================================== */
/*                       Internal Function Declarations                       */
/* ========================================================================== */

static void OSPI_Sfdp_getEraseSizes(OSPI_SfdpBasicFlashParamTable *bfpt, OSPI_GenericXspiDevDefines *xspiDefines);
static uint8_t OSPI_Sfdp_getDummyBitPattern(OSPI_SfdpSCCRParamTable *sccr, uint32_t dummyClk);


/*===========================================================================*/
/*                            Structure Definitions                          */
/*===========================================================================*/

/* None */

/*===========================================================================*/
/*                            Global Variables                               */
/*===========================================================================*/

uint32_t gOSPI_Sfdp_ParamTableIds[OSPI_SFDP_NPH_MAX] = {
    OSPI_SFDP_BASIC_PARAM_TABLE_ID,
    OSPI_SFDP_SECTOR_MAP_TABLE_ID,
    OSPI_SFDP_RPMC_TABLE_ID,
    OSPI_SFDP_4BYTE_ADDR_INSTR_TABLE_ID,
    OSPI_SFDP_XSPI_PROFILE_TABLE_ID,
    OSPI_SFDP_XSPI_PROFILE_2_TABLE_ID,
    OSPI_SFDP_SCCR_TABLE_ID,
    OSPI_SFDP_SCCR_MULTISPI_OFFSETS_TABLE_ID,
    OSPI_SFDP_SCCR_XSPI_PROFILE_2_TABLE_ID,
    OSPI_SFDP_OCTAL_CMD_SEQ_TABLE_ID,
    OSPI_SFDP_LONG_LATENCY_NVM_MSP_TABLE_ID,
    OSPI_SFDP_QUAD_IO_WITH_DS_TABLE_ID,
    OSPI_SFDP_QUAD_CMD_SEQ_TABLE_ID,
};

char* gOSPI_Sfdp_ParamTableNames[OSPI_SFDP_NPH_MAX] = {
    "BASIC PARAMETER TABLE",
    "SECTOR MAP TABLE",
    "RPMC TABLE",
    "4 BYTE ADDRESSING MODE INSTRUCTIONS TABLE",
    "XSPI PROFILE TABLE ",
    "XSPI PROFILE 2.0 TABLE",
    "STATUS CONTROL AND CONFIGURATION REGISTER MAP TABLE",
    "STATUS CONTROL AND CONFIGURATION REGISTER MAP MULTISPI OFFSETS TABLE",
    "STATUS CONTROL AND CONFIGURATION REGISTER MAP XSPI PROFILE 2.0  TABLE",
    "OCTAL DDR MODE COMMAND SEQUENCE TABLE",
    "LONG LATENCY NVM MEDIA SPECIFIC PARAMETER TABLE",
    "QUAD IO WITH DS TABLE",
    "QUAD DDR MODE COMMAND SEQUENCE TABLE",
};


/*===========================================================================*/
/*                            Function Definitions                           */
/*===========================================================================*/
char* OSPI_Sfdp_getParameterTableName(uint32_t paramTableId)
{
    uint32_t i, idx = OSPI_SFDP_NPH_MAX;
    char *p = NULL;

    for(i = 0; i < OSPI_SFDP_NPH_MAX; i++)
    {
        if(paramTableId == gOSPI_Sfdp_ParamTableIds[i])
        {
            idx = i;
            break;
        }
    }

    if(idx == OSPI_SFDP_NPH_MAX)
    {
        /* Unrecognized param table */
        p = NULL;
    }
    else
    {
        p = gOSPI_Sfdp_ParamTableNames[idx];
    }

    return p;
}

uint32_t OSPI_Sfdp_getPtp(OSPI_SfdpParamHeader *paramHeader)
{
    uint32_t ptp = 0xFFFFFFFFU;

    if(paramHeader != NULL)
    {
        ptp = (uint32_t)(paramHeader->paramTablePtr[2] << 16) |
              (uint32_t)(paramHeader->paramTablePtr[1] << 8)  |
              (uint32_t)(paramHeader->paramTablePtr[0]);
    }

    return ptp;
}

int32_t OSPI_Sfdp_parseBfpt(OSPI_SfdpBasicFlashParamTable *bfpt, OSPI_GenericXspiDevDefines *xspiDefines, uint32_t numDwords)
{
    int32_t status = SystemP_SUCCESS;

    /* Some XSPI standards */
    xspiDefines->XSPI_NOR_CMD_WREN = 0x06;
    xspiDefines->XSPI_NOR_CMD_RDSR = 0x05;
    xspiDefines->XSPI_NOR_CMD_RDID = 0x9F;
    xspiDefines->XSPI_NOR_CMD_READ = 0x03;
    xspiDefines->XSPI_NOR_CMD_PAGE_PROG_3B = 0x02;

    /* Always set this to 5. RDID will keep reading 5+1 ID bytes in 1s mode, this way no need to change with 8D/4D */
    xspiDefines->XSPI_NOR_RDID_NUM_BYTES = 5;

    /* Number of address bytes */
    xspiDefines->addrByteSupport = OSPI_SFDP_GET_BITFIELD(bfpt->dtrQFRNumAddr, 17, 18);

    /* DTR support */
    xspiDefines->dtrSupport = OSPI_SFDP_GET_BITFIELD(bfpt->dtrQFRNumAddr, 19, 19);

    /* Flash Size */
    xspiDefines->XSPI_NOR_FLASH_SIZE = 0U;

    if(OSPI_SFDP_GET_BITFIELD(bfpt->memoryDensity, 31, 31) == 0)
    {
        xspiDefines->XSPI_NOR_FLASH_SIZE = OSPI_SFDP_GET_BITFIELD(bfpt->memoryDensity, 0, 30) + 1;
    }
    else
    {
        uint32_t n = OSPI_SFDP_GET_BITFIELD(bfpt->memoryDensity, 0, 30);

        if(n > 63)
        {
            DebugP_logError("Bad flash size read from SFDP !! \r\n");
            xspiDefines->XSPI_NOR_FLASH_SIZE = 0U;
        }
        else
        {
            xspiDefines->XSPI_NOR_FLASH_SIZE = 1U << n;
        }
    }
    /* Convert to bytes */
    xspiDefines->XSPI_NOR_FLASH_SIZE >>= 3;

    /* Check for 1-1-4 mode */
    xspiDefines->XSPI_NOR_CMD_114_READ = OSPI_SFDP_GET_BITFIELD(bfpt->fastRead_114_144_WMI, 24, 31);

    if(xspiDefines->XSPI_NOR_CMD_114_READ != 0)
    {
        /* 1-1-4 mode is supported */
        xspiDefines->XSPI_NOR_114_READ_MODE_CLKS = OSPI_SFDP_GET_BITFIELD(bfpt->fastRead_114_144_WMI, 21, 23);
        xspiDefines->XSPI_NOR_114_READ_DUMMY_CYCLES = OSPI_SFDP_GET_BITFIELD(bfpt->fastRead_114_144_WMI, 16, 20);
    }
    else
    {
        xspiDefines->XSPI_NOR_114_READ_MODE_CLKS = 0;
        xspiDefines->XSPI_NOR_114_READ_DUMMY_CYCLES = 0;
    }

    /* Check for 4-4-4 mode */
    if(OSPI_SFDP_GET_BITFIELD(bfpt->fastReadSupport_222_444, 4, 4) != 0)
    {
        xspiDefines->XSPI_NOR_CMD_444_SDR_READ = OSPI_SFDP_GET_BITFIELD(bfpt->fastRead_444_WMI, 24, 31);
        xspiDefines->XSPI_NOR_444_READ_MODE_CLKS = OSPI_SFDP_GET_BITFIELD(bfpt->fastRead_444_WMI, 21, 23);
        xspiDefines->XSPI_NOR_444_READ_DUMMY_CYCLES = OSPI_SFDP_GET_BITFIELD(bfpt->fastRead_444_WMI, 16, 20);
    }
    else
    {
        xspiDefines->XSPI_NOR_CMD_444_SDR_READ = 0;
        xspiDefines->XSPI_NOR_444_READ_MODE_CLKS = 0;
        xspiDefines->XSPI_NOR_444_READ_DUMMY_CYCLES = 0;
    }

    /* QSPI flashes don't need dummy cycles for CMD reads in quad mode (yet) */
    xspiDefines->XSPI_NOR_QUAD_CMD_READ_DUMMY_CYCLES = 0U;

    /* Erase types : Of all the ones supported, smallest would be sector and largest would be block */
    /* This will set the erase command and type from BFPT. Will be updated with 4BAIT and SMPT parsing */
    OSPI_Sfdp_getEraseSizes(bfpt, xspiDefines);

    /* Check for JESD216A. That has only 9 DWORDS */
    if(numDwords > OSPI_SFDP_BFPT_MAX_DWORDS_JESD216)
    {
        /* Page size and timeouts */
        uint32_t count = 0U;
        uint32_t unit = 0U; /* All time units in μs */
        
        /* Page size */
        xspiDefines->XSPI_NOR_PAGE_SIZE = (1 << (OSPI_SFDP_GET_BITFIELD(bfpt->pageSizeTimes, 4, 7)));

        /* Page program timeout */
        if(OSPI_SFDP_GET_BITFIELD(bfpt->pageSizeTimes, 13, 13) == 1)
        {
            unit = 64;
        }
        else
        {
            unit = 8;
        }

        count = OSPI_SFDP_GET_BITFIELD(bfpt->pageSizeTimes, 8, 12);

        xspiDefines->XSPI_NOR_PAGE_PROG_TIMEOUT = (count+1)*unit;

        /* Chip Erase / Bulk Erase Timeout */
        switch(OSPI_SFDP_GET_BITFIELD(bfpt->pageSizeTimes, 29, 30))
        {
            case 0:
                unit = 16 * 1000; /* 16 ms */
                break;

            case 1:
                unit = 256 * 1000; /* 256 ms */
                break;

            case 2:
                unit = 4 * 1000 * 1000; /* 4 s */
                break;

            case 3:
                unit = 64 * 1000 * 1000; /* 64 s */
                break;

            default:
                unit = 0U; /* Should not hit */
                break;
        }

        count = OSPI_SFDP_GET_BITFIELD(bfpt->pageSizeTimes, 24, 28);

        xspiDefines->XSPI_NOR_BULK_ERASE_TIMEOUT = (count+1)*unit;
        xspiDefines->XSPI_NOR_WRR_WRITE_TIMEOUT = 10*xspiDefines->XSPI_NOR_PAGE_PROG_TIMEOUT;

        /* Quad Enable Requirement. TODO: Later change this to pickup a function pointer accordingly */
        xspiDefines->qeType = OSPI_SFDP_GET_BITFIELD(bfpt->holdResetQeXip, 20, 22);

        uint32_t bitPos = 0U;
        uint32_t field = OSPI_SFDP_GET_BITFIELD(bfpt->holdResetQeXip, 4, 8);

        for(bitPos = 0U; bitPos < 5; bitPos++)
        {
            if(((field >> bitPos) & 0x01)==1)
            {
                xspiDefines->seq444Enable[bitPos] = 1;
            }
            else
            {
                xspiDefines->seq444Enable[bitPos] = 0;
            }
        }

        field = OSPI_SFDP_GET_BITFIELD(bfpt->holdResetQeXip, 0, 3);

        for(bitPos = 0U; bitPos < 4; bitPos++)
        {
            if(((field >> bitPos) & 0x01)==1)
            {
                xspiDefines->seq444Disable[bitPos] = 1;
            }
            else
            {
                xspiDefines->seq444Disable[bitPos] = 0;    
            }
        }

        /* Soft Reset check */
        if(OSPI_SFDP_GET_BITFIELD(bfpt->fourByteAddressVNvStatusReg, 12, 12) == 1)
        {
            xspiDefines->XSPI_NOR_CMD_RSTEN = 0x66;
            xspiDefines->XSPI_NOR_CMD_RSTMEM = 0x99;
        }

        /* Check for JESD216B. That has only 16 DWORDS */
        if(numDwords > OSPI_SFDP_BFPT_MAX_DWORDS_JESD216B)
        {
            /* Byte order */
            xspiDefines->byteOrder = OSPI_SFDP_GET_BITFIELD(bfpt->dqsByteOrderCmdExt, 31, 31);

            /* 8D mode command extension */
            uint32_t cmdExt = OSPI_SFDP_GET_BITFIELD(bfpt->dqsByteOrderCmdExt, 29, 30);

            switch(cmdExt)
            {
                case 0x00:
                    xspiDefines->cmdExtType = OSPI_CMD_EXT_TYPE_REPEAT;
                    break;

                case 0x01:
                    xspiDefines->cmdExtType = OSPI_CMD_EXT_TYPE_INVERSE;
                    break;

                default:
                    xspiDefines->cmdExtType = OSPI_CMD_EXT_TYPE_NONE;
                    break;
            }

            /* TODO: Octal Enable Sequence */

        }
        else
        {
            /* JEDS216B, parsing stops here */
        }
    }
    else
    {
        /* JEDS216A, parsing stops here. */
    }

    return status;
}

/* Sector Map Parameter Table */
int32_t OSPI_Sfdp_parseSmpt(OSPI_SfdpSectorMapParamTable *smpt, OSPI_GenericXspiDevDefines *xspiDefines, uint32_t numDwords)
{
    int32_t status = SystemP_SUCCESS;

    /* TODO: Complete region mapping according to sector map selection */

    return status;
}

/* Status, Control and Configuration Registers Table */
int32_t OSPI_Sfdp_parseSccr(OSPI_SfdpSCCRParamTable *sccr, OSPI_GenericXspiDevDefines *xspiDefines, uint32_t numDwords)
{
    int32_t status = SystemP_SUCCESS;

    /* Address offset to volatile and non-volatile registers */
    xspiDefines->XSPI_NOR_VREG_OFFSET = sccr->dwords[0];
    xspiDefines->XSPI_NOR_NVREG_OFFSET = sccr->dwords[1];

    /* Bit location of WIP and WEL */
    xspiDefines->XSPI_NOR_SR_WIP = (1 << (OSPI_SFDP_GET_BITFIELD(sccr->dwords[4], 24, 26)));
    xspiDefines->XSPI_NOR_SR_WEL = (1 << (OSPI_SFDP_GET_BITFIELD(sccr->dwords[5], 24, 26)));

    /* Set CMDs for RDREG and WRREG */
    xspiDefines->XSPI_NOR_CMD_RDREG = 0x65;
    xspiDefines->XSPI_NOR_CMD_WRREG = 0x71;

    /* Set the dummy cycle and 8D, 4D mode register addresses */
    if(OSPI_SFDP_GET_BITFIELD(sccr->dwords[8], 28, 28) == 1)
    {
        xspiDefines->XSPI_NOR_DUMMY_CYCLE_CFG_ADDR = OSPI_SFDP_GET_BITFIELD(sccr->dwords[8], 16, 23);
    }

    /* QPI Mode */
    if(OSPI_SFDP_GET_BITFIELD(sccr->dwords[13], 31, 31) == 1)
    {
        xspiDefines->XSPI_NOR_QUAD_MODE_CFG_ADDR = OSPI_SFDP_GET_BITFIELD(sccr->dwords[13], 16, 23);
        xspiDefines->XSPI_NOR_QUAD_MODE_CFG_BIT_LOCATION = OSPI_SFDP_GET_BITFIELD(sccr->dwords[13], 24, 26);
    }

    /* DDR Octal SPI Mode */
    if(OSPI_SFDP_GET_BITFIELD(sccr->dwords[21], 31, 31) == 1)
    {
        xspiDefines->XSPI_NOR_DDR_OCTAL_MODE_CFG_ADDR = OSPI_SFDP_GET_BITFIELD(sccr->dwords[21], 16, 23);
        xspiDefines->XSPI_NOR_DDR_OCTAL_MODE_CFG_BIT_LOCATION = OSPI_SFDP_GET_BITFIELD(sccr->dwords[21], 24, 26);
    }

    /* Bit pattern for 444 mode read dummy cycle */
    xspiDefines->XSPI_NOR_444_READ_DUMMY_CYCLES_LC = OSPI_Sfdp_getDummyBitPattern(sccr, xspiDefines->XSPI_NOR_444_READ_DUMMY_CYCLES);

    return status;
}

/* 4 Byte Addressing Instructions Table */
int32_t OSPI_Sfdp_parse4bait(OSPI_Sfdp4ByteAddressingParamTable *fourBait, OSPI_GenericXspiDevDefines *xspiDefines, uint32_t numDwords)
{
    int32_t status = SystemP_SUCCESS;

    /* Erase CMDs for 4 byte addressing mode */
    uint8_t blkEraseCmd, sectEraseCmd;

    if(xspiDefines->supportedEraseTypes[0] < 4)
    {
        sectEraseCmd = (fourBait->dwords[1] >> (xspiDefines->supportedEraseTypes[0]*8)) & 0xFF;
    }
    else
    {
        sectEraseCmd = 0U;
    }

    if(xspiDefines->supportedEraseTypes[1] < 4)
    {
        blkEraseCmd = (fourBait->dwords[1] >> (xspiDefines->supportedEraseTypes[1]*8)) & 0xFF;
    }
    else
    {
        blkEraseCmd = 0U;
    }

    xspiDefines->XSPI_NOR_CMD_SECTOR_ERASE_4B = sectEraseCmd;
    xspiDefines->XSPI_NOR_CMD_BLOCK_ERASE_4B = blkEraseCmd;

    /* 4 Byte Page Program check */
    if(OSPI_SFDP_GET_BITFIELD(fourBait->dwords[0], 6, 6) == 1)
    {
        xspiDefines->XSPI_NOR_CMD_PAGE_PROG_4B = 0x12;
    }
    else
    {
        xspiDefines->XSPI_NOR_CMD_PAGE_PROG_4B = 0x02;
    }

    return status;
}

/* XSPI Profile 1 Table */
int32_t OSPI_Sfdp_parseXpt1(OSPI_SfdpXspiProfile1ParamTable *xpt1, OSPI_GenericXspiDevDefines *xspiDefines, uint32_t numDwords)
{
    int32_t status = SystemP_SUCCESS;

    /* Get the fast read command for 8D-8D-8D mode */
    xspiDefines->XSPI_NOR_CMD_888_DDR_READ = OSPI_SFDP_GET_BITFIELD(xpt1->dwords[0], 8, 15);

    /* Address type for 8D SFDP, LSB 0 or MSB 0 */
    if(OSPI_SFDP_GET_BITFIELD(xpt1->dwords[0], 31, 31) == 0)
    {
        xspiDefines->XSPI_NOR_OCTAL_RDSFDP_ADDR_TYPE = OSPI_SFDP_OCTAL_READ_ADDR_MSB_0;
    }
    else
    {
        xspiDefines->XSPI_NOR_OCTAL_RDSFDP_ADDR_TYPE = OSPI_SFDP_OCTAL_READ_ADDR_LSB_0;
    }
    
    /* Dummy cycles for 8D SFDP */
    if(OSPI_SFDP_GET_BITFIELD(xpt1->dwords[0], 30, 30) == 0)
    {
        xspiDefines->XSPI_NOR_OCTAL_RDSFDP_DUMMY_CYCLE = 8;
    }
    else
    {
        xspiDefines->XSPI_NOR_OCTAL_RDSFDP_DUMMY_CYCLE = 20;
    }

    /* Dummy Cycles for RDSR and REG READ CMDs */
    if(OSPI_SFDP_GET_BITFIELD(xpt1->dwords[0], 28, 28) == 1)
    {
        xspiDefines->XSPI_NOR_OCTAL_DDR_RDSR_DUMMY_CYCLE = 8;
    }
    else
    {
        xspiDefines->XSPI_NOR_OCTAL_DDR_RDSR_DUMMY_CYCLE = 4;
    }

    if(OSPI_SFDP_GET_BITFIELD(xpt1->dwords[0], 26, 26) == 1)
    {
        xspiDefines->XSPI_NOR_OCTAL_DDR_RDVREG_DUMMY_CYCLE = 8;
    }
    else
    {
        xspiDefines->XSPI_NOR_OCTAL_DDR_RDVREG_DUMMY_CYCLE = 4;
    }

    if(OSPI_SFDP_GET_BITFIELD(xpt1->dwords[0], 25, 25) == 1)
    {
        xspiDefines->XSPI_NOR_OCTAL_DDR_RDNVREG_DUMMY_CYCLE = 8;
    }
    else
    {
        xspiDefines->XSPI_NOR_OCTAL_DDR_RDNVREG_DUMMY_CYCLE = 4;
    }

    /* Address bytes needed in 8D for RDSR and REG READ CMDs */
    if(OSPI_SFDP_GET_BITFIELD(xpt1->dwords[0], 29, 29) == 1)
    {
        xspiDefines->XSPI_NOR_OCTAL_DDR_RDSR_ADDR_BYTES = 4;
    }
    else
    {
        xspiDefines->XSPI_NOR_OCTAL_DDR_RDSR_ADDR_BYTES = 0;
    }

    if(OSPI_SFDP_GET_BITFIELD(xpt1->dwords[0], 27, 27) == 1)
    {
        xspiDefines->XSPI_NOR_OCTAL_DDR_RDREG_ADDR_BYTES = 4;
    }
    else
    {
        xspiDefines->XSPI_NOR_OCTAL_DDR_RDREG_ADDR_BYTES = 1;
    }

    if(OSPI_SFDP_GET_BITFIELD(xpt1->dwords[0], 23, 23) == 1)
    {
        xspiDefines->XSPI_NOR_OCTAL_DDR_WRREG_ADDR_BYTES = 4;
    }
    else
    {
        xspiDefines->XSPI_NOR_OCTAL_DDR_WRREG_ADDR_BYTES = 1;
    }

    /* Get dummy cycles for the fastest speed possible */
    uint32_t dummy = OSPI_SFDP_GET_BITFIELD(xpt1->dwords[3], 7, 11); /* 200 MHz */
    uint8_t lc = OSPI_SFDP_GET_BITFIELD(xpt1->dwords[3], 2, 6);

    if(dummy == 0)
    {
        dummy = OSPI_SFDP_GET_BITFIELD(xpt1->dwords[4], 27, 31); /* 166 MHz */
        lc = OSPI_SFDP_GET_BITFIELD(xpt1->dwords[4], 22, 26);
    }

    if(dummy == 0)
    {
        dummy = OSPI_SFDP_GET_BITFIELD(xpt1->dwords[4], 17, 21); /* 133 MHz */
        lc = OSPI_SFDP_GET_BITFIELD(xpt1->dwords[4], 12, 16);
    }

    if(dummy == 0)
    {
        dummy = OSPI_SFDP_GET_BITFIELD(xpt1->dwords[4], 7, 11); /* 100 MHz */
        lc = OSPI_SFDP_GET_BITFIELD(xpt1->dwords[3], 2, 6);
    }

    if(dummy == 0)
    {
        DebugP_logError("No dummy cycle found from xSPI Profile 1.0 table !!!\r\n");
    }
    else
    {
        /* Timing can mess up if dummy cycle is odd */
        if(dummy % 2 != 0)
        {
            dummy += 1;
            if(lc < 31)
            {
                lc += 1;
            }
        }
        xspiDefines->XSPI_NOR_OCTAL_READ_DUMMY_CYCLE = dummy;
        xspiDefines->XSPI_NOR_OCTAL_READ_DUMMY_CYCLE_LC = lc;
    }

    return status;
}

static void OSPI_Sfdp_getEraseSizes(OSPI_SfdpBasicFlashParamTable *bfpt, OSPI_GenericXspiDevDefines *xspiDefines)
{
    uint32_t i, blk = 0U, sector = 31U;
    uint32_t blkIdx = 4U;
    uint32_t sectorIdx = 4U;
    uint8_t eraseSizeN[4];
    uint8_t eraseCmd[4];

    eraseSizeN[0] = OSPI_SFDP_GET_BITFIELD(bfpt->eraseType_1_2, 0, 7);
    eraseSizeN[1] = OSPI_SFDP_GET_BITFIELD(bfpt->eraseType_1_2, 16, 23);
    eraseSizeN[2] = OSPI_SFDP_GET_BITFIELD(bfpt->eraseType_3_4, 0, 7);
    eraseSizeN[3] = OSPI_SFDP_GET_BITFIELD(bfpt->eraseType_3_4, 16, 23);

    eraseCmd[0] = OSPI_SFDP_GET_BITFIELD(bfpt->eraseType_1_2, 8, 15);
    eraseCmd[1] = OSPI_SFDP_GET_BITFIELD(bfpt->eraseType_1_2, 24, 31);
    eraseCmd[2] = OSPI_SFDP_GET_BITFIELD(bfpt->eraseType_3_4, 8, 15);
    eraseCmd[3] = OSPI_SFDP_GET_BITFIELD(bfpt->eraseType_3_4, 24, 31);

    for(i = 0; i < 4; i++)
    {
        if((eraseSizeN[i] != 0) && (blk < eraseSizeN[i]))
        {
            blk = eraseSizeN[i];
            blkIdx = i;
        }
    }
    
    for(i = 0; i < 4; i++)
    {
        if((eraseSizeN[i] != 0) && (sector > eraseSizeN[i]))
        {
            sector = eraseSizeN[i];
            sectorIdx = i;
        }
    }

    xspiDefines->XSPI_NOR_BLOCK_SIZE = (1 << blk);
    xspiDefines->XSPI_NOR_SECTOR_SIZE = (1 << sector);

    if(sectorIdx < 4)
    {
        xspiDefines->XSPI_NOR_CMD_SECTOR_ERASE_3B = eraseCmd[sectorIdx];
        xspiDefines->XSPI_NOR_CMD_SECTOR_ERASE_4B = eraseCmd[sectorIdx];
        xspiDefines->supportedEraseTypes[0] = sectorIdx;
    }
    else
    {
        xspiDefines->XSPI_NOR_CMD_SECTOR_ERASE_3B = 0;
        xspiDefines->XSPI_NOR_CMD_SECTOR_ERASE_4B = 0;
        xspiDefines->supportedEraseTypes[0] = 4;
    }

    if(blkIdx < 4)
    {
        xspiDefines->XSPI_NOR_CMD_BLOCK_ERASE_3B = eraseCmd[blkIdx];
        xspiDefines->XSPI_NOR_CMD_BLOCK_ERASE_4B = eraseCmd[blkIdx];

        xspiDefines->supportedEraseTypes[1] = blkIdx;
    }
    else
    {
        xspiDefines->XSPI_NOR_CMD_BLOCK_ERASE_3B = 0;
        xspiDefines->XSPI_NOR_CMD_BLOCK_ERASE_4B = 0;

        xspiDefines->supportedEraseTypes[1] = 4;
    }

    /* XSPI standard */
    xspiDefines->XSPI_NOR_CMD_BULK_ERASE = 0xC7;
}

static uint8_t OSPI_Sfdp_getDummyBitPattern(OSPI_SfdpSCCRParamTable *sccr, uint32_t dummyClk)
{
    uint8_t bitPattern = 0xFF;
    uint32_t dword, isDummy, subVal;

    /* Only support even dummy cycles */
    if((dummyClk % 2) != 0)
    {
        dummyClk += 1;
    }

    /* Find the right DWORD in SCCR */
    if((dummyClk) <= 30 && (dummyClk >= 22))
    {
        dword = sccr->dwords[10];
        subVal = 20;
    }
    else if((dummyClk) <= 20 && (dummyClk >= 12))
    {
        dword = sccr->dwords[11];
        subVal = 10;
    }
    else if((dummyClk) <= 10 && (dummyClk >= 2))
    {
        dword = sccr->dwords[12];
        subVal = 0;
    }
    else
    {
        dword = 0xFFFFFFFFU;
    }

    if(dword != 0xFFFFFFFFU)
    {
        uint32_t dummySupportBit = 3*(dummyClk-subVal)+1;
        isDummy = OSPI_SFDP_GET_BITFIELD(dword, dummySupportBit, dummySupportBit);

        if(isDummy)
        {
            bitPattern = OSPI_SFDP_GET_BITFIELD(dword, (dummySupportBit-5), (dummySupportBit-1));
        }
        else
        {
            bitPattern = 0xFF;
        }
    }

    return bitPattern;
}