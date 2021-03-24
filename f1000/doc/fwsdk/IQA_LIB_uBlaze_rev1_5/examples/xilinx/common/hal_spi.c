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
// Copyright (C) 2018-2019 IQ-Analog Corp. All rights reserved.                 *
//                                                                              *
//------------------------------------------------------------------------------*
// IQ-Analog CONFIDENTIAL                                                       *
//------------------------------------------------------------------------------*
// This file is released with "Government Purpose Rights" as defined            *
// in DFARS SUBPART 227.71, clause 252.227-7013.                                *
//------------------------------------------------------------------------------*
//                                                                              *
// File        : hal_spi.c                                                      *
// Author      : RL                                                             *
// Created     : 06/10/2019                                                     *
//                                                                              *
// Description : Custom HAL layer implementation of SPI access to device.       *
//                                                                              *
//*******************************************************************************

#include <assert.h>
#include "xspi.h"
#include "xspi_l.h"

#include "iqa_f1000_api.h"
#include "f1000_ioctl.h"

// Declare how many F1000 on the board. IQ-Analog F1000 EVM has one F1000.
#define NUM_F1000   1

//===============================================================================
// External declaration
//===============================================================================

extern void f1000_select_ep(int dev);
extern u32 f1000_spi_write(u32 addr, u32 data);
extern u32 f1000_spi_read(u32 addr, u32 data);

/**
 ***************************************************************************************************
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
    assert ( f1000_id >= 1 && f1000_id <= NUM_F1000 );
    assert ( ep_id >= EP_MIN && ep_id <= EP_MAX );

    uint32_t i;

    // Select Element Processor A or B
    f1000_select_ep(ep_id);

    // Read data from device, 4 bytes each time
    for (i = 0; i < (size+3) / 4; i++)
    {
        *(buf + i) = f1000_spi_read(addr + i * 4, 0);
    }

    return IQA_SUCCESS;
}

/**
 ***************************************************************************************************
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
    assert ( f1000_id >= 1 && f1000_id <= NUM_F1000 );
    assert ( ep_id >= EP_MIN && ep_id <= EP_MAX );

    uint32_t i;

    // Select Element Processor A or B
    f1000_select_ep(ep_id);

    // Write data to device, 4 bytes each time
    for (i = 0; i < (size+3) / 4; i++)
    {
        f1000_spi_write(addr + i * 4, *(buf + i));
    }

    return IQA_SUCCESS;
}

/**
 ***************************************************************************************************
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
    assert ( f1000_id >= 1 && f1000_id <= NUM_F1000 );
    assert ( ep_id >= EP_MIN && ep_id <= EP_MAX );

    uint32_t     i;

    // Select Element Processor A or B
    f1000_select_ep(ep_id);

    switch (command)
    {
        case CMD_DOWNLOAD_FW:
        case CMD_WRITE_MEMORY:

            // Write data to device, 4 bytes each time
            for (i = 0; i < (size+3) / 4; i++)
            {
                f1000_spi_write(addr + i*4, *(uint32_t *)(buf + i*4));
            }
            break;

        case CMD_READ_MEMORY:

            // Read data from device, 4 bytes each time
            for (i = 0; i < (size+3) / 4; i++)
            {
                *(uint32_t *)(buf + i*4) = f1000_spi_read(addr + i*4, 0);
            }

            break;

        default:
            return -IQA_ERROR_PARAM;
    }

    return IQA_SUCCESS;
}
