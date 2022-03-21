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
 *  \ingroup DRV_OSPI_MODULE
 *  \defgroup DRV_OSPI_SFDP_API_MODULE SFDP API for single pin mode
 *
 *   These APIs are used to parse various SFDP parameter tables read from a NOR OSPI flash
 *
 *  @{
 */

/**
 *  \file ospi_sfdp.h
 *
 *  \brief OSPI Driver SFDP API/interface file.
 */

#ifndef OSPI_SFDP_H_
#define OSPI_SFDP_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdint.h>
#include <kernel/dpl/SystemP.h>
#include <kernel/dpl/HwiP.h>
#include <kernel/dpl/SemaphoreP.h>
#include <drivers/hw_include/csl_types.h>
#include <drivers/hw_include/cslr_ospi.h>

#ifdef __cplusplus
extern "C" {
#endif

/* SFDP Signature DWORD */
#define OSPI_SFDP_SIGNATURE     (0x50444653U)

/* SFDP Revisions */
#define OSPI_SFDP_JESD216_MAJOR     (1U)
#define OSPI_SFDP_JESD216_MINOR     (0U)
#define OSPI_SFDP_JESD216A_MINOR    (5U)
#define OSPI_SFDP_JESD216B_MINOR    (6U)
#define OSPI_SFDP_JESD216C_MINOR    (7U)
#define OSPI_SFDP_JESD216D_MINOR    (8U)

/* SFDP Maximum numbers of parameter headers (only JEDEC ones included)*/
#define OSPI_SFDP_NPH_MAX       (13U)

/* SFDP offsets */
#define OSPI_SFDP_HEADER_START_OFFSET        (0x00U)
#define OSPI_SFDP_FIRST_PARAM_HEADER_OFFSET  (0x08U)
#define OSPI_SFDP_SECOND_PARAM_HEADER_OFFSET (0x10U)

/* SFDP Parameter table IDs supported by JEDEC*/
#define OSPI_SFDP_BASIC_PARAM_TABLE_ID            (0xFF00)
#define OSPI_SFDP_SECTOR_MAP_TABLE_ID             (0xFF81)
#define OSPI_SFDP_RPMC_TABLE_ID                   (0xFF03)
#define OSPI_SFDP_4BYTE_ADDR_INSTR_TABLE_ID       (0xFF84)
#define OSPI_SFDP_XSPI_PROFILE_TABLE_ID           (0xFF05)
#define OSPI_SFDP_XSPI_PROFILE_2_TABLE_ID         (0xFF06)
#define OSPI_SFDP_SCCR_TABLE_ID                   (0xFF87)
#define OSPI_SFDP_SCCR_MULTISPI_OFFSETS_TABLE_ID  (0xFF88)
#define OSPI_SFDP_SCCR_XSPI_PROFILE_2_TABLE_ID    (0xFF09)
#define OSPI_SFDP_OCTAL_CMD_SEQ_TABLE_ID          (0xFF0A)
#define OSPI_SFDP_LONG_LATENCY_NVM_MSP_TABLE_ID   (0xFF8B)
#define OSPI_SFDP_QUAD_IO_WITH_DS_TABLE_ID        (0xFF0C)
#define OSPI_SFDP_QUAD_CMD_SEQ_TABLE_ID           (0xFF8D)

/* SFDP Number of DWORDS in BFPT for different revisions */
#define OSPI_SFDP_BFPT_MAX_DWORDS_JESD216         (9)
#define OSPI_SFDP_BFPT_MAX_DWORDS_JESD216B        (16)

/* SFDP read command address type in 8D */
#define OSPI_SFDP_OCTAL_READ_ADDR_MSB_0           (0)
#define OSPI_SFDP_OCTAL_READ_ADDR_LSB_0           (1)

typedef struct OSPI_SfdpMainHeader_s
{
    uint32_t signature;
    /*  SFDP Signature. "SFDP" in ASCII */

    uint8_t minorRev;
    /* SFDP minor revision number */
    
    uint8_t majorRev;
    /* SFDP minor revision number */

    uint8_t numParamHeaders;
    /* Number of param headers. Zero-based. 0 means 1 param header */
    
    uint8_t accessProtocol;
    /* SFDP Access Protocol */

} OSPI_SfdpMainHeader;

typedef struct OSPI_SfdpParamHeader_s
{   
    uint8_t paramIdLsb;
    /* LSB of parameter ID */

    uint8_t paramTableMinorRev;
    /* Minor revision number of parameter table */

    uint8_t paramTableMajorRev;
    /* Major revision number of parameter table */

    uint8_t paramTableLength;
    /* Number of DWORDs in the parameter table. One based field. So 1 means 1 DWORD */

    uint8_t paramTablePtr[3];
    /* Start of parameter header's corresponding table. This address should be DWORD aligned */

    uint8_t paramIdMsb;
    /* MSB of parameter ID */

} OSPI_SfdpParamHeader;

typedef struct OSPI_SfdpBasicFlashParamTable_s
{
    uint32_t dtrQFRNumAddr;
    /* 1st DWORD - has info about DTR support, number of address bytes and Quad and Dual Fast read support */

    uint32_t memoryDensity;
    /* 2nd DWORD - flash size. If b31 is 0, 30:0 is size in bits. If b31 is 1, 30:0 is N, where 2^N is size in bits */

    uint32_t fastRead_114_144_WMI;
    /* 3rd DWORD - 1-1-4 and 1-4-4 fast read wait states, mode bit clocks and instruction */

    uint32_t fastRead_112_122_WMI;
    /* 4th DWORD - 1-1-2 and 1-2-2 fast read wait states, mode bit clocks and instruction */

    uint32_t fastReadSupport_222_444;
    /* 5th DWORD - has info on whether 2-2-2 and 4-4-4 mode are supported */

    uint32_t fastRead_222_WMI;
    /* 6th DWORD - 2-2-2 fast read wait states, mode bit clocks and instruction */

    uint32_t fastRead_444_WMI;
    /* 7th DWORD - 4-4-4 fast read wait states, mode bit clocks and instruction */

    uint32_t eraseType_1_2;
    /* 8th DWORD - erase types 1 and 2 : Sizes and their instructions */

    uint32_t eraseType_3_4;
    /* 9th DWORD - erase types 3 and 4 : Sizes and their instructions */

    uint32_t eraseTimes;
    /* 10th DWORD - erase times for all 4 types of erases */

    uint32_t pageSizeTimes;
    /* 11th DWORD - pageSize, chip erase time, page program time, byte program time */

    uint32_t suspendResumeSupport;
    /* 12th DWORD - suspend/resume support, intervals, latency etc */

    uint32_t suspendResumeInstr;
    /* 13th DWORD - suspend/resume instructions */

    uint32_t deepPdStatusPoll;
    /* 14th DWORD - Deep power down support, instructions, status polling supported modes */

    uint32_t holdResetQeXip;
    /* 15th DWORD - hold and reset details, quad enable requirements, 0-4-4 and 4-4-4 mode enabling */

    uint32_t fourByteAddressVNvStatusReg;
    /* 16th DWORD - 4 byte addressing mode entry/exit, support, soft reset sequences, volatile and non-volatile status register support */  

    uint32_t fastRead_118_188_WMI;
    /* 17th DWORD - 1-1-8 and 1-8-8 fast read wait states, mode bit clocks and instruction */

    uint32_t dqsByteOrderCmdExt;
    /* 18th DWORD - DQS support, byte order, command extension type in 8D mode */

    uint32_t OeXip;
    /* 19th DWORD - Octal enable requirements, 0-8-8 mode support, entry and exit, 8-8-8 enable disable sequence */

    uint32_t maxClocks;
    /* 20th DWORD - Maximum operational speed for 4-4-4 and 8-8-8 modes */

} OSPI_SfdpBasicFlashParamTable;

typedef struct OSPI_SfdpXspiProfile1ParamTable_s
{
    uint32_t dwords[5];

} OSPI_SfdpXspiProfile1ParamTable;

typedef struct OSPI_SfdpSectorMapParamTable_s
{
    uint32_t dwords[2];
    
} OSPI_SfdpSectorMapParamTable;

typedef struct OSPI_SfdpSCCRParamTable_s
{
    uint32_t dwords[28];

} OSPI_SfdpSCCRParamTable;

typedef struct OSPI_Sfdp4ByteAddressingParamTable_s
{
    uint32_t dwords[2];

} OSPI_Sfdp4ByteAddressingParamTable;

/**
 *  \brief OSPI SFDP Structure
 */

typedef struct OSPI_SfdpHeader_s
{
    OSPI_SfdpMainHeader      sfdpHeader;
    /* The SFDP header */

    OSPI_SfdpParamHeader firstParamHeader;
    /* First and mandatory parameter table header */
    
} OSPI_SfdpHeader;

/* ========================================================================== */
/*                             Function Definitions                           */
/* ========================================================================== */

/**
 *  \brief  This function returns the name of the parameter table given the table ID
 *
 *  \param  paramTableId 16 bit ID of the parameter table
 *
 *  \return The name of the parameter table as a string
 */
char* OSPI_Sfdp_getParameterTableName(uint32_t paramTableId);

/**
 *  \brief  This function returns the Parameter Table Pointer (PTP) of the parameter table given the parameter header
 *
 *  \param  paramHeader Parameter header for the table
 *
 *  \return PTP as a uint32_t address
 */
uint32_t OSPI_Sfdp_getPtp(OSPI_SfdpParamHeader *paramHeader);

/**
 *  \brief  This function parses the Basic Flash Parameter Table (BFPT) and fills the xspiDevDefines structure with the parsed information
 *
 *  \param  bfpt         Pointer to the BFPT (allocated by user)
 *  \param  xspiDefines  Pointer to the generic xSPI device definitions structure (allocated by user)
 *  \param  numDwords    Number of double words actually present in the table (this has to be read from the param header and passed)
 *
 *  \return SystemP_SUCCESS if parsing is successful, otherwise failure.
 */
int32_t OSPI_Sfdp_parseBfpt(OSPI_SfdpBasicFlashParamTable *bfpt, OSPI_GenericXspiDevDefines *xspiDefines, uint32_t numDwords);

/**
 *  \brief  This function parses the xSPI Flash Profile 1.0 Table and fills the xspiDevDefines structure with the parsed information
 *
 *  \param  xpt1         Pointer to the xSPI Flash Profile 1.0 Table (allocated by user)
 *  \param  xspiDefines  Pointer to the generic xSPI device definitions structure (allocated by user)
 *  \param  numDwords    Number of double words actually present in the table (this has to be read from the param header and passed)
 *
 *  \return SystemP_SUCCESS if parsing is successful, otherwise failure.
 */
int32_t OSPI_Sfdp_parseXpt1(OSPI_SfdpXspiProfile1ParamTable *xpt1, OSPI_GenericXspiDevDefines *xspiDefines, uint32_t numDwords);

/**
 *  \brief  This function parses the 4 Byte Addressing Information Table (4BAIT) and fills the xspiDevDefines structure with the parsed information
 *
 *  \param  fourBait     Pointer to the 4BAIT (allocated by user)
 *  \param  xspiDefines  Pointer to the generic xSPI device definitions structure (allocated by user)
 *  \param  numDwords    Number of double words actually present in the table (this has to be read from the param header and passed)
 *
 *  \return SystemP_SUCCESS if parsing is successful, otherwise failure.
 */
int32_t OSPI_Sfdp_parse4bait(OSPI_Sfdp4ByteAddressingParamTable *fourBait, OSPI_GenericXspiDevDefines *xspiDefines, uint32_t numDwords);

/**
 *  \brief  This function parses the Status, Control and Configuration Registers (SCCR) Table and fills the xspiDevDefines structure with the parsed information
 *
 *  \param  sccr         Pointer to the SCCR Table (allocated by user)
 *  \param  xspiDefines  Pointer to the generic xSPI device definitions structure (allocated by user)
 *  \param  numDwords    Number of double words actually present in the table (this has to be read from the param header and passed)
 *
 *  \return SystemP_SUCCESS if parsing is successful, otherwise failure.
 */
int32_t OSPI_Sfdp_parseSccr(OSPI_SfdpSCCRParamTable *sccr, OSPI_GenericXspiDevDefines *xspiDefines, uint32_t numDwords);

/**
 *  \brief  This function parses the Sector Map Parameter Table (SMPT) and fills the xspiDevDefines structure with the parsed information
 *
 *  \param  smpt         Pointer to the SMPT (allocated by user)
 *  \param  xspiDefines  Pointer to the generic xSPI device definitions structure (allocated by user)
 *  \param  numDwords    Number of double words actually present in the table (this has to be read from the param header and passed)
 *
 *  \return SystemP_SUCCESS if parsing is successful, otherwise failure.
 */
int32_t OSPI_Sfdp_parseSmpt(OSPI_SfdpSectorMapParamTable *smpt, OSPI_GenericXspiDevDefines *xspiDefines, uint32_t numDwords);

/** @} */
#ifdef __cplusplus
}
#endif

#endif /* OSPI_SFDP_H_ */