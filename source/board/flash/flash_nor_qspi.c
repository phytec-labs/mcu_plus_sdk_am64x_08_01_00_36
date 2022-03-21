/*
 * Copyright (c) 2021, Texas Instruments Incorporated
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
 *
 */


#include <board/flash.h>

#define FLASH_QSPI_ID_CODE_SIZE_MAX (8U)

#define QSPI_NOR_SR_WIP             (1U << 0U)
#define QSPI_NOR_SR_WEL             (1U << 1U)

static int32_t Flash_norQspiErase(Flash_Config *config, uint32_t blkNum);
static int32_t Flash_norQspiRead(Flash_Config *config, uint32_t offset, uint8_t *buf, uint32_t len);
static int32_t Flash_norQspiWrite(Flash_Config *config, uint32_t offset, uint8_t *buf, uint32_t len);
static int32_t Flash_norQspiOpen(Flash_Config *config, Flash_Params *params);
static void Flash_norQspiClose(Flash_Config *config);

/* NOR function table for NOR QSPI interface implementation */
Flash_Fxns gFlashNorQspiFxns = {

    .openFxn = Flash_norQspiOpen,
    .closeFxn = Flash_norQspiClose,
    .readFxn = Flash_norQspiRead,
    .writeFxn = Flash_norQspiWrite,
    .eraseFxn = Flash_norQspiErase,
};

int32_t QSPI_norFlashCmdRead(QSPI_Handle handle, uint8_t cmd, uint32_t cmdAddr, uint8_t *rxBuf, uint32_t rxLen)
{
    int32_t status = SystemP_SUCCESS;

    QSPI_ReadCmdParams rdParams;
    QSPI_readCmdParams_init(&rdParams);
    rdParams.cmd       = cmd;
    rdParams.cmdAddr   = cmdAddr;
    rdParams.rxDataBuf = rxBuf;
    rdParams.rxDataLen = rxLen;

    status += QSPI_readCmd(handle, &rdParams);

    return status;
}

int32_t QSPI_norFlashCmdWrite(QSPI_Handle handle, uint8_t cmd, uint32_t cmdAddr, uint8_t *txBuf, uint32_t txLen)
{
    int32_t status = SystemP_SUCCESS;

    QSPI_WriteCmdParams wrParams;
    QSPI_writeCmdParams_init(&wrParams);
    wrParams.cmd        = cmd;
    wrParams.cmdAddr    = cmdAddr;
    wrParams.txDataBuf  = txBuf;
    wrParams.txDataLen  = txLen;
    status += QSPI_writeCmd(handle, &wrParams);

    return status;
}

int32_t QSPI_norFlashWriteEnableLatched(QSPI_Handle handle, Flash_NorQspiDevDefines *devDefines, uint32_t timeOut)
{
    int32_t status = SystemP_SUCCESS;
    uint8_t readStatus = 0U;
    uint8_t cmd;

    cmd = devDefines->NOR_CMD_RDSR1;

    do
    {
        status = QSPI_norFlashCmdRead(handle, cmd, QSPI_CMD_INVALID_ADDR, &readStatus, 1);
        if(status != SystemP_SUCCESS)
        {
            break;
        }
        if((readStatus & QSPI_NOR_SR_WEL) != 0)
        {
            break;
        }
        timeOut--;
        if(timeOut == 0)
        {
            status = SystemP_TIMEOUT;
            break;
        }
    } while(1);

    if((readStatus & QSPI_NOR_SR_WEL) != 0)
    {
        status = SystemP_SUCCESS;
    }
    else
    {
        status = SystemP_FAILURE;
    }

    return status;
}

int32_t QSPI_norFlashWaitReady(QSPI_Handle handle, Flash_NorQspiDevDefines *devDefines, uint32_t timeOut)
{
    int32_t status = SystemP_SUCCESS;
    uint8_t readStatus = 0U;
    uint8_t cmd;

    cmd = devDefines->NOR_CMD_RDSR1;

    do
    {
        status = QSPI_norFlashCmdRead(handle, cmd, QSPI_CMD_INVALID_ADDR, &readStatus, 1);
        if(status != SystemP_SUCCESS)
        {
            break;
        }
        if((readStatus & QSPI_NOR_SR_WIP) == 0)
        {
            break;
        }
        timeOut--;
        if(timeOut == 0)
        {
            status = SystemP_TIMEOUT;
            break;
        }
    } while(1);

    if((readStatus & QSPI_NOR_SR_WIP) == 0)
    {
        status = SystemP_SUCCESS;
    }
    else
    {
        status = SystemP_FAILURE;
    }

    return status;
}

int32_t QSPI_norFlashQuadReadEnable(Flash_Config *config)
{
    int32_t status = SystemP_SUCCESS;
    Flash_NorQspiObject *obj = (Flash_NorQspiObject *)(config->object);
    Flash_NorQspiDevDefines *devDefines = (Flash_NorQspiDevDefines *)config->devDefines;
    uint8_t readreg1 = 0U;
    uint8_t readreg2 = 0U;
    status = QSPI_norFlashCmdWrite(obj->qspiHandle, devDefines->NOR_CMD_WREN, QSPI_CMD_INVALID_ADDR, NULL, 0);
    status += QSPI_norFlashWriteEnableLatched(obj->qspiHandle, devDefines, devDefines->NOR_WRR_WRITE_TIMEOUT);

    if (status == SystemP_SUCCESS)
    {
        status = QSPI_norFlashCmdRead(obj->qspiHandle, devDefines->NOR_CMD_RDSR1, QSPI_CMD_INVALID_ADDR, &readreg1, 1);
        status += QSPI_norFlashCmdRead(obj->qspiHandle, devDefines->NOR_CMD_RDSR2, QSPI_CMD_INVALID_ADDR, &readreg2, 1);

        readreg2 |= 0x2;
        uint16_t regdata = 0;
        regdata |= readreg2;
        regdata = (regdata << 8) | readreg1;

        status += QSPI_norFlashCmdWrite(obj->qspiHandle, devDefines->NOR_CMD_WRSR, QSPI_CMD_INVALID_ADDR, (uint8_t *)&regdata, 2);
        status += QSPI_norFlashWaitReady(obj->qspiHandle, devDefines, devDefines->NOR_PAGE_PROG_TIMEOUT);
    }

    return status;
}

static int32_t Flash_norQspiOpen(Flash_Config *config, Flash_Params *params)
{
    int32_t status = SystemP_SUCCESS;
    Flash_NorQspiObject *obj = (Flash_NorQspiObject *)(config->object);
    Flash_NorQspiDevDefines *devDefines = (Flash_NorQspiDevDefines *)config->devDefines;

    obj->qspiHandle = QSPI_getHandle(config->attrs->driverInstance);

    if(obj->qspiHandle==NULL)
    {
        status = SystemP_FAILURE;
    }
    if(status == SystemP_SUCCESS)
    {
        uint8_t cmd;
        /* Reset the Flash */
        cmd = devDefines->NOR_CMD_RSTEN;
        QSPI_norFlashCmdWrite(obj->qspiHandle, cmd, QSPI_CMD_INVALID_ADDR, NULL, 0);

        cmd = devDefines->NOR_CMD_RST;
        QSPI_norFlashCmdWrite(obj->qspiHandle, cmd, QSPI_CMD_INVALID_ADDR, NULL, 0);

        status = QSPI_norFlashWaitReady(obj->qspiHandle, devDefines, devDefines->NOR_WRR_WRITE_TIMEOUT);

        QSPI_setWriteCmd(obj->qspiHandle, devDefines->NOR_PAGE_PROG);

        QSPI_setReadCmd(obj->qspiHandle, devDefines->NOR_CMD_QUAD_READ);

        QSPI_setAddressByteCount(obj->qspiHandle, 3);

        QSPI_setDummyBitCount(obj->qspiHandle, devDefines->NOR_QUAD_READ_DUMMY_CYCLE);

        status = QSPI_norFlashQuadReadEnable(config);
    }
    return (status);
}

static void Flash_norQspiClose(Flash_Config *config)
{
    Flash_NorQspiObject *obj = (Flash_NorQspiObject *)(config->object);

    obj->qspiHandle = NULL;

    /* QSPI Driver will be closed outside flash */

    return;
}


static int32_t Flash_norQspiRead(Flash_Config *config, uint32_t offset, uint8_t *buf, uint32_t len)
{
    int32_t status = SystemP_SUCCESS;
    Flash_NorQspiObject *obj = (Flash_NorQspiObject *)(config->object);
    Flash_Attrs *attrs = config->attrs;

    /* Validate address input */
    if ((offset + len) > (attrs->blockCount*attrs->pageCount*attrs->pageSize))
    {
        status = SystemP_FAILURE;
    }
    if (status == SystemP_SUCCESS)
    {
        QSPI_Transaction transaction;

        QSPI_transaction_init(&transaction);
        transaction.addrOffset = offset;
        transaction.buf = (void *)buf;
        transaction.count = len;
        status = QSPI_readMemMapMode(obj->qspiHandle, &transaction);
    }
    return status;
}

static int32_t Flash_norQspiWrite(Flash_Config *config, uint32_t offset, uint8_t *buf, uint32_t len)
{
    int32_t status = SystemP_SUCCESS;
    Flash_NorQspiObject *obj = (Flash_NorQspiObject *)(config->object);
    Flash_NorQspiDevDefines *devDefines = (Flash_NorQspiDevDefines *)config->devDefines;
    Flash_Attrs *attrs = config->attrs;

    /* Validate address input */
    if ((offset + len) > (attrs->blockCount*attrs->pageCount*attrs->pageSize))
    {
        status = SystemP_FAILURE;
    }
    /* Check offset alignment */
    if(0 != (offset % attrs->pageSize))
    {
        status = SystemP_FAILURE;
    }
    if(status == SystemP_SUCCESS)
    {
        uint32_t pageSize, chunkLen, actual;
        uint8_t cmdWren = devDefines->NOR_CMD_WREN;
        QSPI_Transaction transaction;

        pageSize = devDefines->NOR_PAGE_SIZE;
        chunkLen = pageSize;

        for (actual = 0; actual < len; actual += chunkLen)
        {
            status = QSPI_norFlashCmdWrite(obj->qspiHandle, cmdWren, QSPI_CMD_INVALID_ADDR, NULL, 0);

            if(status == SystemP_SUCCESS)
            {
                status = QSPI_norFlashWriteEnableLatched(obj->qspiHandle, devDefines, devDefines->NOR_WRR_WRITE_TIMEOUT);
            }

            if(status == SystemP_SUCCESS)
            {
                /* Send Page Program command */
                if((len - actual) < (pageSize))
                {
                    chunkLen = (len - actual);
                }
                else
                {
                    chunkLen = pageSize;
                }

                QSPI_transaction_init(&transaction);
                transaction.addrOffset = offset;
                transaction.buf = (void *)(buf + actual);
                transaction.count = chunkLen;
                status = QSPI_writeConfigMode(obj->qspiHandle, &transaction);
            }

            if(status == SystemP_SUCCESS)
            {
                status = QSPI_norFlashWaitReady(obj->qspiHandle, devDefines, devDefines->NOR_PAGE_PROG_TIMEOUT);
            }

            if(status == SystemP_SUCCESS)
            {
                offset += chunkLen;
            }
            else
            {
                break;
            }
        }
    }

    return status;
}

static int32_t Flash_norQspiErase(Flash_Config *config, uint32_t blkNum)
{
    int32_t status = SystemP_SUCCESS;
    Flash_NorQspiObject *obj = (Flash_NorQspiObject *)(config->object);
    Flash_NorQspiDevDefines *devDefines = (Flash_NorQspiDevDefines *)config->devDefines;

    uint8_t  cmd = QSPI_CMD_INVALID_OPCODE;
    uint32_t cmdAddr = QSPI_CMD_INVALID_ADDR;
    uint8_t  cmdWren = devDefines->NOR_CMD_WREN;

    if (blkNum == (uint32_t)(-1))
    {
        cmd    = devDefines->NOR_CMD_BULK_ERASE;
    }
    else
    {
        cmdAddr = blkNum * config->attrs->pageCount * config->attrs->pageSize;
        cmd = devDefines->NOR_CMD_BLOCK_ERASE;

        if (blkNum >= config->attrs->blockCount)
        {
            status = SystemP_FAILURE;
        }
    }
    if(status == SystemP_SUCCESS)
    {
        status = QSPI_norFlashWaitReady(obj->qspiHandle, devDefines, devDefines->NOR_WRR_WRITE_TIMEOUT);
    }
    if(status == SystemP_SUCCESS)
    {
        status = QSPI_norFlashCmdWrite(obj->qspiHandle, cmdWren, QSPI_CMD_INVALID_ADDR, NULL, 0);
    }
    if(status == SystemP_SUCCESS)
    {
        status = QSPI_norFlashWaitReady(obj->qspiHandle, devDefines, devDefines->NOR_WRR_WRITE_TIMEOUT);
    }
    if(status == SystemP_SUCCESS)
    {
        status = QSPI_norFlashCmdWrite(obj->qspiHandle, cmd, cmdAddr, NULL, 0);
    }
    if(status == SystemP_SUCCESS)
    {
        status = QSPI_norFlashWaitReady(obj->qspiHandle, devDefines, devDefines->NOR_BULK_ERASE_TIMEOUT);
    }

    return status;
}
