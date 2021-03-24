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

#include "xil_cache.h"
#include "xspi.h"
//#include "spi_header.h"

#include "iqa_f1000_api.h"
#include "f1000_ioctl.h"
#include "riscv_fw.h"

#define F1000_ID                1                // Index 1 since there is exactly 1 F1000 on the board
#define REG_OOPS2                0x40014004        // Register for calibration status
#define FIRMWARE_REVISION_LEN   12*sizeof(char)

extern int spi_init(void);

//===============================================================================
//
// Private Functions Implementation
//
//===============================================================================

static void wait_for_key(char c)
{
    while (inbyte() != c)
        ;
}

//===============================================================================
// Load firmware into RISC-V cores
//===============================================================================
static int load_riscv_fw(int ep_bitmap, const unsigned char * fw, const uint32_t size)
{
    #define REG_OOPS2               0x40014004        // Register OOP2 for calibration status
    #define FIRMWARE_REVISION_LEN   12*sizeof(char)

    char                    fw_revision[FIRMWARE_REVISION_LEN];
    iqa_fw_download_state_t state = {0};
    IQA_STATUS              status;
    uint32_t                adc_cal_started;
    int                        seconds;

    // Put Element Processor in freeze mode

    status = iqa_ec_freeze ( F1000_ID, ep_bitmap );

    if (IQA_SUCCESS != status)
    {
        printf("Error putting device into freeze mode for download.\r\n");
        return status;
    }

    // Download firmware into Element Processor

    status = iqa_ec_load_firmware ( F1000_ID, ep_bitmap, (char *) fw, (uint32_t) size, &state );

    if (IQA_SUCCESS != status)
    {
        printf("Error downloading firmware (%d)\r\n", status);
        return status;
    }

    // Unfreeze Element Processor

    status = iqa_ec_unfreeze ( F1000_ID, ep_bitmap );

    if (IQA_SUCCESS != status)
    {
        printf("Error putting device into unfreeze mode after download.\r\n");
        return status;
    }

    printf("Bytes downloaded = %ld\r\n", size);

    /* Give the RISC-V core time to startup */
    usleep(1000000);

    // Check if firmware is running
    if (ep_bitmap & EP_A)
    {
        status = iqa_ec_ping ( F1000_ID, EP_A );
        if (status != IQA_SUCCESS)
        {
            printf ("ERROR!!  EP A RISC-V ping test fail!\r\n");
            return status;
        }
    }

    if (ep_bitmap & EP_B)
    {
        status = iqa_ec_ping ( F1000_ID, EP_B );
        if (status != IQA_SUCCESS)
        {
            printf ("ERROR!!  EP B RISC-V ping test fail!\r\n");
            return status;
        }
    }

    // Get the Firmware Revision number
    if (ep_bitmap & EP_A)
    {
        status = iqa_ec_get_rev ( F1000_ID, EP_A, fw_revision, FIRMWARE_REVISION_LEN );
        if (status != IQA_SUCCESS)
        {
            printf ("ERROR!!  Unable to get EP A firmware version!\r\n");
            return status;
        }
        else
        {
            printf("EP A, RISC-V FW Version: %s\r\n", fw_revision);
        }
    }

    if (ep_bitmap & EP_B)
    {
        status = iqa_ec_get_rev ( F1000_ID, EP_B, fw_revision, FIRMWARE_REVISION_LEN );
        if (status != IQA_SUCCESS)
        {
            printf ("ERROR!!  Unable to get EP B firmware version!\r\n");
            return status;
        }
        else
        {
            printf("EP B, RISC-V FW Version: %s\r\n", fw_revision);
        }
    }

    // Wait for calibration routine to complete
    printf ( "Waiting for ADC Calibration to Complete...\r\n" );

    seconds = 0;
    do
    {
      sleep ( 1 );
      seconds++;

      if (ep_bitmap & EP_A)
          status = iqa_read_register ( F1000_ID, EP_A, REG_OOPS2,
                                       ( uint32_t* ) &adc_cal_started );
      else if (ep_bitmap & EP_B)
          status = iqa_read_register ( F1000_ID, EP_B, REG_OOPS2,
                                       ( uint32_t* ) &adc_cal_started );
    }
    while ( ! ( adc_cal_started & 0x80 ) && ( seconds < 15 ) );

    if ( seconds >= 15 )
    {
      return -IQA_ERROR_ADC_INACTIVE;
    }

    return IQA_SUCCESS;
}

//===============================================================================
// Main
//===============================================================================

int main ( int argc, FAR char *argv[] )
{
    IQA_STATUS    status;

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
    // Load RISC-V Firmware
    //=================================

    printf("\r\nMake sure signal generator is OFF. Hit space bar to continue...\n");
    wait_for_key(' ');

    printf("\r\n*** Load RISC-V firmware ***\r\n");
    status = load_riscv_fw(EP_A | EP_B, riscv_fw, riscv_fw_size);

    if (status != IQA_SUCCESS)
    {
        printf ( "RISC-V firmware loading failed.\r\n" );
    }

    Xil_ICacheDisable();
    Xil_DCacheDisable();

    printf("\r\n*** End of Program ***\r\n");

    return IQA_SUCCESS;
}
