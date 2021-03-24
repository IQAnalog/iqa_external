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
#include "xspi.h"
#include "xil_cache.h"
#include "spi_header.h"

#include "iqa_f1000_api.h"
#include "f1000_ioctl.h"

#define F1000_ID            1          // Index 1 since there is exactly 1 F1000 on the board

extern int spi_init(void);

//===============================================================================
// Main
//===============================================================================

int main ( int argc, FAR char *argv[] )
{
    IQA_STATUS  status;
    uint32_t    chip_id;
    uint8_t     chip_rev;

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
    // Read Chip ID
    //=================================

    printf("\r\n*** Read Chip ID ***\r\n");

    status = iqa_read_chip_id ( F1000_ID, EP_A, &chip_id );

    if (IQA_SUCCESS != status)
    {
        printf("Error reading chip id (%d)\r\n", status);
        return status;
    }

    printf("Chip ID = 0x%lX\r\n", chip_id);

    //=================================
    // Read Chip Revision
    //=================================

    printf("\r\n*** Read Chip Revision ***\r\n");

    status = iqa_read_chip_rev ( F1000_ID, EP_A, &chip_rev );

    if (IQA_SUCCESS != status)
    {
        printf("Error reading chip rev (%d)\r\n", status);
        return status;
    }

    printf("Chip rev = 0x%x ", chip_rev);

    if ( iqa_chip_is_rev_a( F1000_ID ) )
        printf("(rev A)\r\n");

    else if ( iqa_chip_is_rev_b( F1000_ID ) )
        printf("(rev B)\r\n");

    else
        printf("(rev unknown)\r\n");

    Xil_ICacheDisable();
    Xil_DCacheDisable();

    printf("\r\n*** End of Program ***\r\n");

    return IQA_SUCCESS;
}
