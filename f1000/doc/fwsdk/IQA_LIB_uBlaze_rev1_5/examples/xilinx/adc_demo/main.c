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
// Copyright (C) 2018-2020 IQ-Analog Corp. All rights reserved.                 *
//                                                                              *
//------------------------------------------------------------------------------*
// IQ-Analog CONFIDENTIAL                                                       *
//------------------------------------------------------------------------------*
// This file is released with "Government Purpose Rights" as defined            *
// in DFARS SUBPART 227.71, clause 252.227-7013.                                *
//------------------------------------------------------------------------------*
//                                                                              *
// File        : main.c                                                         *
// Author      : Rudy Liang                                                     *
// Created     : 06/10/2019                                                     *
// Updated     : 04/27/2020                                                     *
//                                                                              *
// Description : Application to exercise calling IQ-Analog's APIs.              *
//               Note: This is for F1000 revision B only.                       *
//                                                                              *
//*******************************************************************************

#include <stdio.h>
#include <unistd.h>

#include "xil_cache.h"
#include "xspi.h"

#include "f1000.hxx"
#include "iqa_f1000_api.h"
#include "f1000_ioctl.h"
#include "riscv_b_fw.h"

#define F1000_ID            1          // Index 1 since there is exactly 1 F1000 on the board

extern int spi_init(void);
extern int dump_mem_data(int f1000_id, int ep_id, uint32_t adc_id, uint32_t samples);

//===============================================================================
// Local declarations
//===============================================================================

static int init_chip(int ep_id, int adc_id, int dac_id);
static int load_riscv_fw(int ep_bitmap, const unsigned char * fw, const uint32_t size);

// Callback functions to be called from DAC wakeup routine
static void cb_avnn_sw(void);
static void cb_avnn_en(void);
static void cb_avdd_level(bool low_high);
static void cb_avdd_voltage(float v);
static void cb_dvdd_voltage(float v);
static void cb_vref_level(bool low_high);
static void cb_vref_voltage(uint16_t v);
static void cb_atest_sw(void);

const cb_dac_t cb_dac =
{
    cb_avnn_sw,
    cb_avnn_en,
    cb_avdd_level,
    cb_avdd_voltage,
    cb_dvdd_voltage,
    cb_vref_level,
    cb_vref_voltage,
    cb_atest_sw
};

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
// Callback functions to be called from DAC wakeup routine
//===============================================================================

static void cb_avnn_sw(void)
{
    printf("cb_avnn_sw(): [echo 1 > /dev/gpio/avnn_sw]...\n");
    wait_for_key(' ');
}

static void cb_avnn_en(void)
{
    printf("cb_avnn_en(): [power avnn on]...\n");
    wait_for_key(' ');
}

static void cb_avdd_level(bool low_high)
{
    printf("cb_avdd_level(%s): [power avdd %s]...\n", (low_high == 0) ? "low" : "high", (low_high == 0) ? "1.35" : "1.6");
    wait_for_key(' ');
}

static void cb_avdd_voltage(float v)
{
    printf("cb_avdd_voltage(%.2f): [power avdd %.2f]...\n", v, v);
    wait_for_key(' ');
}

static void cb_dvdd_voltage(float v)
{
    printf("cb_dvdd_voltage(%.2f): [power dvdd %.2f]...\n", v, v);
    wait_for_key(' ');
}

static void cb_vref_level(bool low_high)
{
    printf("cb_vref_level(%s): [echo %s > /dev/vref]...\n", (low_high == 0) ? "low" : "high", (low_high == 0) ? "10" : "182");
    wait_for_key(' ');
}

static void cb_vref_voltage(uint16_t v)
{
    printf("cb_vref_voltage(%d): [echo %d > /dev/vref]...\n", v, v);
    wait_for_key(' ');
}

static void cb_atest_sw(void)
{
    printf ("\r\ncb_atest_sw: Short the Board and Press Space bar...\n");
    fflush(stdout);
    fflush(stdin);

    wait_for_key(' ');
    sleep(3);
}

//===============================================================================
// Initialize chip
//===============================================================================

static int init_chip(int ep_id, int adc_id, int dac_id)
{
    IQA_STATUS  status;
    uint32_t    data;

    printf("\r\n*** Initialize Chip ***\r\n");

    // Note: For F1000 rev A EVM, use 0.94 for dvdd. Use 0.85 for F1000 rev B/C EVM.
    cb_dvdd_voltage(0.94);

    printf("Set RF Synthesizer power level to 16: [rfsynth 1 2d; <& 0xFFC0 | 0x1_0000>...\n");
    wait_for_key(' ');

    status = iqa_init_analog ( F1000_ID, ep_id, adc_id, dac_id, 6000000000, &cb_dac );

    if (status != IQA_SUCCESS)
    {
        printf ("\nFailed to initialize chip (%d).\n", status);
    }

    // Set the avdd voltage to 1.25 for ADC
    cb_avdd_voltage(1.25);

    // Check DAC power up status
    if (dac_id != DAC_NONE)
    {
        iqa_read_register(F1000_ID, ep_id, DAC2_DIG_DLL_STATUS, &data);

        // Check DLL power up status
        if ((data & 2) == 2)
        {
            printf("DAC successfully powered up!\r\n");
            status = IQA_SUCCESS;
        }
        else
        {
            printf("DAC power-up failed.\r\n");
            status = -IQA_ERROR_DAC_INIT;
        }
    }

    return status;
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

    return IQA_SUCCESS;
}

//===============================================================================
// Check ADC calibration completion
//===============================================================================

static int check_adc_cal_status(int ep_bitmap, int adc_bitmap)
{
    IQA_STATUS  status = IQA_SUCCESS;
    uint32_t    adc_cal_started;
    uint32_t    all_cal_done;
    int         seconds;

    // Wait for calibration routine to complete
    printf ( "\r\nWaiting for ADC Calibration to Complete...\r\n" );

    seconds = 0;
    if ( iqa_chip_is_rev_a(F1000_ID) )
    {
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
    }
    else // F1000 rev B or newer
    {
        do
        {
            sleep ( 1 );
            seconds++;

            all_cal_done = 1;
            for ( int ep_id = EP_A; ep_id <= EP_B && all_cal_done; ep_id++ )
            {
                if ( ep_id & ep_bitmap )
                {
                    for ( int adc_id = ADC_1; adc_id <= ADC_2 && all_cal_done; adc_id++ )
                    {
                        if ( adc_id & adc_bitmap )
                        {
                            status = iqa_adc_get_cal_status( F1000_ID, ep_id, adc_id, &all_cal_done );

                            if ( status != IQA_SUCCESS )
                                return status;
                        }
                    }
                }
            }
        } while ( ! ( all_cal_done ) && ( seconds < 15 ) );
    }

    if ( seconds >= 15 )
    {
      printf ( "ADC Calibration Failure.\r\n" );
      return -IQA_ERROR_ADC_INACTIVE;
    }

    return IQA_ERROR_NONE;
}

//===============================================================================
//
// Main
//
//===============================================================================

int main ( int argc, FAR char *argv[] )
{
    IQA_STATUS  status;
    uint32_t    chip_id;
    int         ep_id;
    int         adc_id;
    int         dac_id;
    uint32_t    samples;

    // Using EP-A, ADC 1, No DAC, with 32K samples
    ep_id       = EP_A;
    adc_id      = ADC_1;
    dac_id      = DAC_NONE;
    samples     = 32768;    // Number of samples

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

    status = iqa_read_chip_id ( F1000_ID, ep_id, &chip_id );

    if (status != IQA_SUCCESS)
    {
        printf("Error reading chip id (%d)\r\n", status);
        return status;
    }

    printf("Chip ID = 0x%lX\r\n", chip_id);

    //=================================
    // Initialize chip
    //=================================

    status = init_chip(ep_id, adc_id, dac_id);

    if (status != IQA_SUCCESS)
    {
        printf("Chip initialization failed.\r\n");
        return status;
    }

    //=================================
    // Load RISC-V firmware
    //=================================

    printf("\r\n*** Loading RISC-V firmware ***\r\n");
    status = load_riscv_fw(ep_id, riscv_b_fw, riscv_b_fw_size);

    if (status != IQA_SUCCESS)
    {
        printf ( "ADC Calibration Failed.\r\n" );
    }

    //=================================
    // Check ADC calibration completion
    //=================================
    status = check_adc_cal_status(ep_id, adc_id);

    if (status != IQA_SUCCESS)
    {
        printf ( "ADC calibration completion failed.\r\n" );
        return status;
    }

    //=================================
    // ADC Capture
    //=================================

    printf("\r\n*** ADC Capture ***\r\n");
    status = iqa_init_adc_capture ( F1000_ID, ep_id, adc_id, samples );

    if (status != IQA_SUCCESS)
    {
        printf ("Failed to capture ADC data into memory.\r\n");
    }

    printf("\r\n*** Dump Memory Data ***\r\n");
    status = dump_mem_data ( F1000_ID, ep_id, adc_id, samples );

    if (status != IQA_SUCCESS)
    {
        printf ("Failed to read ADC memory.\r\n");
    }

    Xil_ICacheDisable();
    Xil_DCacheDisable();

    printf("\r\n*** End of Program ***\r\n");

    return IQA_SUCCESS;
}
