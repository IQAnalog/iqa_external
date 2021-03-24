//*******************************************************************************
//      __  __                                                                  *
//     /  \/  \                                                                 *
//         \   \                                                                *
//    I Q - A N A L O G                                                         *
//           \   \            IQ-Analog Corp                                    *
//            \__/\__/       www.iqanalog.com                                   *
//                                                                              *
//------------------------------------------------------------------------------*
//                                                                              *
// Copyright (C) IQ-Analog Corp. All rights reserved.                           *
//                                                                              *
//------------------------------------------------------------------------------*
// CONFIDENTIAL                                                                 *
//------------------------------------------------------------------------------*
//                                                                              *
// File        : iqa_hal_spi.c                                                  *
// Author      : RL                                                             *
// Created     : 11/15/2018                                                     *
//                                                                              *
// Description : Routines to manage SPI access through the HAL layer.           *
//                                                                              *
//*******************************************************************************

///@cond
#include <stdio.h>
#include <nuttx/spi/spi.h>
#include <sys/ioctl.h>
#include <string.h>
#include <fcntl.h>

#include "arch/board/board.h"
///@endcond

#include "iqa_f1000_api.h"
#include "f1000_ioctl.h"

//-------------------------------------------------------------------------------
// Structure for F1000 device
//-------------------------------------------------------------------------------
struct f1000_dev_s
{
    struct spi_dev_s * spi_dev;

    // Other fields for F1000 can be added here
};

// Declare how many F1000 on the board.
// IQ-Analog F1000 EVM has only one F1000 in the hardware.
#define NUM_F1000   1

struct f1000_dev_s          f1000_dev[NUM_F1000];
struct f1000_iostruct_s     f1000_req[NUM_F1000];

static pthread_mutex_t g_hal_spi_mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 ***********************************************************************************
 *
 * \brief This is HAL layer API to read data from SPI slave device.
 *
 *  Note: This routine is expected to be customized per board configuration.
 *        For example, items such as number of F1000 installed, interface to F1000 (SPI slave device)
 *
 * \param f1000_id : F1000 device to communicate with
 * \param ep_id : Value that represents the Element Processor ID
 *                Element Processor | Value
 *                :----------------:| :---------------:
 *                        A         | 0x1
 *                        B         | 0x2
 * \param addr : Address to be read from
 * \param buf : pointer to buffer for retrieved data
 * \param size : number of bytes to be read
 * \param block : Select blocking or non-blocking call
 *                Blocking     | Value
 *                :-----------:| :---------------:
 *                SPI_BLOCKING     | true
 *                Non-SPI_BLOCKING | false
 * \param lock : Select whether SPI bus should be locked for operation
 *                Locking         | Value
 *                :-------------: | :---------------:
 *                SPI_LOCKING     | true (lock SPI interface to allow only access to slave device)
 *                SPI_NON_LOCKING | false (allows more than one master to access the slave device)
 *
 * \return int : IQA_SUCCESS if no error. Error value if error occurs.
 * \sa iqa_hal_spi_ioctl
 * \sa iqa_hal_spi_write
 *
 ***************************************************************************************************
 */
int iqa_hal_spi_read ( int f1000_id, int ep_id, uint32_t addr, uint32_t * buf, uint32_t size, bool block, bool lock )
{
    ASSERT ( f1000_id >= 1 && f1000_id <= NUM_F1000 );
    ASSERT ( ep_id >= EP_MIN && ep_id <= EP_MAX );

    int         fd = -1;
    char *      token;
    char        cmdbuf[128];
    uint32_t    value= 0xFFFFFFFF;
    uint32_t  * pbuf;
    IQA_STATUS  ret = IQA_SUCCESS;

    pthread_mutex_lock(&g_hal_spi_mutex);

    // Open F1000 EP-A or EP-B device
    fd = open( (ep_id == EP_A ? "/dev/f1000_epa" : "/dev/f1000_epb"), O_RDWR);
    if (fd < 0)
    {
        ret = -IQA_ERROR_EP_OPEN;
        printf("Unable to open Element Processor (%d).\r\n", ret);
        goto iqa_hal_spi_read_complete;
    }

    // Read 32-bit data at a time
    pbuf = buf;
    for (int i = 0; i < size/4; i++)
    {
        // Write the "read" command and address to be read to the device
        sprintf(cmdbuf, "r%08X\r\n", addr + i*4);

        write(fd, cmdbuf, strlen(cmdbuf));

        // Read the file data
        ssize_t nbytesread = read(fd, cmdbuf, sizeof(cmdbuf));

        if (nbytesread <= 0)
        {
            ret = -IQA_ERROR_EP_READ;
            printf("Unable to read from Element Processor (%d).\r\n", ret);

            close(fd);
            goto iqa_hal_spi_read_complete;
        }

        // parse the chars read in format "REG AAAAAAAA = VVVVVVVV"

        // Get address
        token = strtok(cmdbuf, "=");

        // Get value
        token = strtok(NULL, " =\r\n");
        sscanf(token, "%x", &value);

        *pbuf++ = value;
    }


    // Close device
    close(fd);

iqa_hal_spi_read_complete:

    pthread_mutex_unlock(&g_hal_spi_mutex);
    return ret;
}


/**
 ***********************************************************************************
 *
 * \brief This is HAL layer API to write data to SPI slave device.
 *
 *  Note: This routine is expected to be customized per board configuration.
 *        For example, items such as number of F1000 installed, interface to F1000 (SPI slave device)
 *
 * \param f1000_id : F1000 device to communicate with
 * \param ep_id : Value that represents the Element Processor ID
 *                Element Processor | Value
 *                :----------------:| :---------------:
 *                        A         | 0x1
 *                        B         | 0x2
 * \param addr : Address to write data to
 * \param buf : pointer to buffer for data to be written
 * \param size : number of bytes to be written
 * \param block : Select blocking or non-blocking call
 *                Blocking     | Value
 *                :-----------:| :---------------:
 *                SPI_BLOCKING     | true
 *                Non-SPI_BLOCKING | false
 * \param lock : Select whether SPI bus should be locked for operation
 *                Locking         | Value
 *                :-------------: | :---------------:
 *                SPI_LOCKING     | true (lock SPI interface to allow only access to slave device)
 *                SPI_NON_LOCKING | false (allows more than one master to access the slave device)
 *
 * \return int : IQA_SUCCESS if no error. Error value if error occurs.
 * \sa iqa_hal_spi_ioctl
 * \sa iqa_hal_spi_read
 *
 ***************************************************************************************************
 */
int iqa_hal_spi_write ( int f1000_id, int ep_id, uint32_t addr, uint32_t * buf, uint32_t size, bool block, bool lock )
{
    ASSERT ( f1000_id >= 1 && f1000_id <= NUM_F1000 );
    ASSERT ( ep_id >= EP_MIN && ep_id <= EP_MAX );

    int         fd = -1;
    char        cmdbuf[128];
    uint32_t    count;
    uint32_t  * pbuf;
    struct f1000_iostruct_s * pios;
    IQA_STATUS  ret = IQA_SUCCESS;

    pthread_mutex_lock(&g_hal_spi_mutex);

    // Open F1000 EP-A or EP-B device
    fd = open( (ep_id == EP_A ? "/dev/f1000_epa" : "/dev/f1000_epb"), O_WRONLY);
    if (fd < 0)
    {
        ret = -IQA_ERROR_EP_OPEN;
        printf("Unable to open Element Processor (%d).\r\n", ret);
        goto iqa_hal_spi_write_complete;
    }

    if (size > 4)
    {
      pios         = &f1000_req[0];   // There is only 1 F1000 so fill in the params for its structure
      pios->cmd    = CMD_WRITE_MEMORY;
      pios->addr   = addr;
      pios->len    = size;
      pios->buf    = (char *) buf;
      ret = ioctl(fd, CMD_WRITE_MEMORY, (unsigned long) pios);

      close(fd);
      goto iqa_hal_spi_write_complete;
    }
    else
    {
        // Write to addr with 32-bits data at a time
        pbuf = buf;
        count = 0;
        while (size >= 4)
        {
            sprintf(cmdbuf, "w%8X=%8X\r\n", addr+count, *pbuf++);
            write(fd, cmdbuf, strlen(cmdbuf));
            size -= 4;
            count += 4;
        }
    }

    // TODO: Need to handle length not divisible into 4 bytes

    // Close device
    close(fd);

iqa_hal_spi_write_complete:
    pthread_mutex_unlock(&g_hal_spi_mutex);
    return ret;
}

/**
 ***********************************************************************************
 *
 * \brief This is HAL layer API to perform IO control to SPI slave device.
 *
 *  Note: This routine is expected to be customized per board configuration.
 *        For example, items such as number of F1000 installed, interface to F1000 (SPI slave device)
 *
 * \param f1000_id : F1000 device to communicate with
 * \param ep_id : Value that represents the Element Processor ID
 *                Element Processor | Value
 *                :----------------:| :---------------:
 *                        A         | 0x1
 *                        B         | 0x2
 * \param command : Command for the device to execute
 * \param addr : Address to write data to
 * \param buf : pointer to buffer for data to be written
 * \param size : number of bytes to be written
 * \param block : Select blocking or non-blocking call
 *                Blocking     | Value
 *                :-----------:| :---------------:
 *                SPI_BLOCKING     | true
 *                Non-SPI_BLOCKING | false
 * \param lock : Select whether SPI bus should be locked for operation
 *                Locking         | Value
 *                :-------------: | :---------------:
 *                SPI_LOCKING     | true (lock SPI interface to allow only access to slave device)
 *                SPI_NON_LOCKING | false (allows more than one master to access the slave device)
 *
 * \return int : IQA_SUCCESS if no error. Error value if error occurs.
 * \sa iqa_hal_spi_read
 * \sa iqa_hal_spi_write
 *
 ***************************************************************************************************
 */
int iqa_hal_spi_ioctl ( int f1000_id, int ep_id, int command, uint32_t addr, char * buf, uint32_t size, bool block, bool lock )
{
    struct f1000_iostruct_s * pios;
    int                       fd = -1;
    IQA_STATUS                ret = IQA_SUCCESS;

    ASSERT ( f1000_id >= 1 && f1000_id <= NUM_F1000 );
    ASSERT ( ep_id >= EP_MIN && ep_id <= EP_MAX );

    pthread_mutex_lock(&g_hal_spi_mutex);

    // Open F1000 EP-A or EP-B device
    fd = open( (ep_id == EP_A ? "/dev/f1000_epa" : "/dev/f1000_epb"), O_WRONLY);
    if (fd < 0)
    {
        ret = -IQA_ERROR_EP_OPEN;
        printf("Unable to open Element Processor (%d).\r\n", ret);
        goto iqa_hal_spi_ioctl_complete;
    }

    pios         = &f1000_req[0];   // There is only 1 F1000 so fill in the params for its structure
    pios->cmd    = command;
    pios->addr   = addr;
    pios->len    = size;
    pios->buf    = buf;

    if ( CMD_DOWNLOAD_FW == command )
    {
        // Determine file type, ELF, SLM or binary
        pios->fwtype = FWTYPE_BINARY;
    }

    ioctl(fd, command, (unsigned long) pios);
    close(fd);

iqa_hal_spi_ioctl_complete:
    pthread_mutex_unlock(&g_hal_spi_mutex);
    return IQA_SUCCESS;
}

// vim:  sw=3 ts=3 et
