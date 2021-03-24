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
// File        : main.c                                                         *
// Author      : RL                                                             *
// Created     : 06/10/2019                                                     *
//                                                                              *
// Description : Application to exercise calling IQ-Analog library's APIs.      *
//                                                                              *
//*******************************************************************************

#include <stdio.h>
#include <unistd.h>
#include "xspi.h"
#include "xil_cache.h"
#include "spi_header.h"

#include "iqa_f1000_api.h"
#include "merlin_fw.h"

#define F1000_ID          1        // Index 1 since there is exactly 1 F1000 on the board

extern int spi_init(void);

//===============================================================================
// load_merlin_fw
//===============================================================================

static int load_merlin_fw(int ep_id)
{
    #define REG_MERLIN_OOB_AXIFACE_CONTROL 0x40017600
    #define REG_GP_OOPS1                   0x40014000

    const int     standalone = 1;   // free running; i.e. not synchronized to the JESD-204B Multiframe Clock
    IQA_STATUS  status;
    uint32_t    chip_id;
    uint32_t    data;

    // Enable OOB AXI Bus access (READ and WRITE)
    status = iqa_write_register ( F1000_ID, ep_id, REG_MERLIN_OOB_AXIFACE_CONTROL, 0x3F );

    if (IQA_SUCCESS != status)
    {
        printf("Error writing to Merlin AXI register (%d)\r\n", status);
        return status;
    }

    // Clear register that Merlin firmware writes its output to
    iqa_write_register ( F1000_ID, EP_A, REG_GP_OOPS1, 0 );

    // Firmware will read chip id, shift left 4 bits and write its output to the OOPS1 register
    status = iqa_merlin_load_firmware ( F1000_ID, ep_id, (char *) merlin_fw, (uint32_t) merlin_fw_size, standalone );

    if (IQA_SUCCESS != status)
    {
        printf("Error downloading firmware into Merlin (%d)\r\n", status);
        return status;
    }

    printf("Bytes downloaded = %ld\r\n", merlin_fw_size);

    // Read from OOPS1 register where the output is
    iqa_read_register ( F1000_ID, EP_A, REG_GP_OOPS1, &data );

    // Read chip ID for comparison
    iqa_read_chip_id ( F1000_ID, EP_A, &chip_id );

    printf("Merlin firmware output: 0x%lX, Expected value: 0x%lX (%s)\r\n", data, (chip_id << 4), (data == (chip_id <<4)) ? "Pass" : "FAIL");

    return IQA_SUCCESS;
}

//===============================================================================
// Main
//===============================================================================

int main ( int argc, FAR char *argv[] )
{
    Xil_ICacheEnable();
    Xil_DCacheEnable();

    //=================================
    // Initialize SPI bus
    //=================================

    printf("\r\n*** Initialize SPI bus ***\r\n");
    if (spi_init() != XST_SUCCESS)
    {
        printf("Error initialize SPI\r\n");
        return -1;
    }
    printf("SPI initialized successfully.\r\n");

    //=================================
    // Load Merlin Firmware
    //=================================

    printf("\r\n*** Load Merlin firmware ***\r\n");
    load_merlin_fw(EP_A);

    Xil_ICacheDisable();
    Xil_DCacheDisable();

    printf("\r\n*** End of Program ***\r\n");

    return IQA_SUCCESS;
}
