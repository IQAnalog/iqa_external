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
// Author      : Rudy Liang                                                     *
// Created     : 06/12/2020                                                     *
//                                                                              *
// Description : Application to exercise calling IQ-Analog's APIs.              *
//                                                                              *
//*******************************************************************************

#include <stdio.h>
#include <unistd.h>

#include "xil_cache.h"
#include "xspi.h"

#include "f1000.hxx"
#include "iqa_f1000_api.h"
#include "f1000_ioctl.h"
#include "riscv_fw.h"
#include "riscv_b_fw.h"
#include "merlin_b_fw.h"

#define F1000_ID            1          // Index 1 since there is exactly 1 F1000 on the board

extern int spi_init(void);

//===============================================================================
// Local declarations
//===============================================================================

static int init_chip(int ep_id, int adc_id, int dac_id);
static int init_adc_dsp ( int f1000_id, int ep_id, int adc_id );
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

    printf("\r\n*** Initialize Chip ***\r\n");

    printf("Set RF Synthesizer power level to 16: [rfsynth 1 2d; <& 0xFFC0 | 0x1_0000>...\n");
    wait_for_key(' ');

    status = iqa_init_analog ( F1000_ID, ep_id, adc_id, dac_id, 6000000000, &cb_dac );

    if (status != IQA_SUCCESS)
    {
        printf ("\nFailed to initialize chip (%d).\n", status);
    }

    return status;
}

//===============================================================================
// Initialize ADC DSP
//===============================================================================

static int init_adc_dsp ( int f1000_id, int ep_id, int adc_id )
{
  int       status = IQA_SUCCESS;

  // Default frequencies to be loaded into ADC DSP

  uint32_t band_freq[] = { 0x2000000,     //   8/2^10 * 2^32
                           0x2c00000,     //  11/2^10 * 2^32
                           0x3400000,     //  13/2^10 * 2^32
                           0x4000000,     //  16/2^10 * 2^32
                           0x5400000,     //  21/2^10 * 2^32
                           0x8000000,     //  32/2^10 * 2^32
                           0x10000000,    //  64/2^10 * 2^32
                           0x20000000     // 128/2^10 * 2^32
                         };

  // 8 initial phase frequencies to be loaded into ADC DSP
  uint32_t initial_phase[] = { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 };

  status |= iqa_init_adc_dsp ( f1000_id, ep_id, adc_id, BAND_1, band_freq,
                               initial_phase, MODE_250MSPS, 7 );     // freq selected = 7
  status |= iqa_init_adc_dsp ( f1000_id, ep_id, adc_id, BAND_2, band_freq,
                               initial_phase, MODE_250MSPS, 7 );     // freq selected = 7

  return status;
}

//===============================================================================
// Load firmware into RISC-V cores
//===============================================================================

static int load_riscv_fw(int ep_bitmap, const unsigned char * fw, const uint32_t size)
{
    #define FIRMWARE_REVISION_LEN   12*sizeof(char)

    char                    fw_revision[FIRMWARE_REVISION_LEN];
    iqa_fw_download_state_t state = {0};
    IQA_STATUS              status;

    // Put Element Processor in freeze mode

    status = iqa_ec_freeze ( F1000_ID, ep_bitmap );

    if (IQA_ERROR_NONE != status)
    {
        printf("Error putting device into freeze mode for download.\r\n");
        return status;
    }

    // Download firmware into Element Processor

    status = iqa_ec_load_firmware ( F1000_ID, ep_bitmap, (char *) fw, (uint32_t) size, &state );

    if (IQA_ERROR_NONE != status)
    {
        printf("Error downloading firmware (%d)\r\n", status);
        return status;
    }

    // Unfreeze Element Processor

    status = iqa_ec_unfreeze ( F1000_ID, ep_bitmap );

    if (IQA_ERROR_NONE != status)
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
        if (status != IQA_ERROR_NONE)
        {
            printf ("ERROR!!  EP A RISC-V ping test fail!\r\n");
            return status;
        }
    }

    if (ep_bitmap & EP_B)
    {
        status = iqa_ec_ping ( F1000_ID, EP_B );
        if (status != IQA_ERROR_NONE)
        {
            printf ("ERROR!!  EP B RISC-V ping test fail!\r\n");
            return status;
        }
    }

    // Get the Firmware Revision number
    if (ep_bitmap & EP_A)
    {
        status = iqa_ec_get_rev ( F1000_ID, EP_A, fw_revision, FIRMWARE_REVISION_LEN );
        if (status != IQA_ERROR_NONE)
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
        if (status != IQA_ERROR_NONE)
        {
            printf ("ERROR!!  Unable to get EP B firmware version!\r\n");
            return status;
        }
        else
        {
            printf("EP B, RISC-V FW Version: %s\r\n", fw_revision);
        }
    }

    return IQA_ERROR_NONE;
}


//===============================================================================
// Check ADC calibration completion
//===============================================================================

static int check_adc_cal_status(int ep_bitmap, int adc_bitmap)
{
    #define REG_OOPS2    0x40014004        // Register OOP2 for calibration status for F1000_A

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
    IQA_STATUS  status = IQA_SUCCESS;
    uint32_t    chip_id;
    int         fclk_mhz;
    int         ep_id;
    int         adc_id;
    int         dac_id;
    int         band_id;
    int         run_mode = 0;       // Merlin not running in standalone mode
    int         optimize = 0;       // Not using optimize option for serdes DLL lock
    int         frame_sync;         // 1 to issue frame sync after programming DSP, 0 to skip
    int         rx_lane_id;         // Which RX lane to be enabled (0-7)
    uint8_t     rx_lane_bitmap;     // Bitmap of which RX lanes to be enabled

    // Using 6GHz clock, EP-B, ADC 2, No DAC
    fclk_mhz    = 6000;
    ep_id       = EP_B;
    adc_id      = ADC_2;
    dac_id      = DAC_NONE;

    // NOTE: Define which RX lane to be enabled
    rx_lane_id  = 1;

    // Create bitmap of which RX lanes to be enabled
    rx_lane_bitmap = 1 << rx_lane_id;

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

    // Turning off the signal generator is only needed for F1000 rev A silicon
    if ( iqa_chip_is_rev_a(F1000_ID) )
    {
        printf("\r\nMake sure signal generator is OFF...\r\n");
        wait_for_key(' ');
    }

    printf("\r\n*** Load RISC-V firmware ***\r\n");
    if (iqa_chip_is_rev_a(F1000_ID))
        status = load_riscv_fw(ep_id, riscv_fw, riscv_fw_size);
    else
        status = load_riscv_fw(ep_id, riscv_b_fw, riscv_b_fw_size);

    if (status != IQA_SUCCESS)
    {
        printf ( "RISC-V firmware loading failed.\r\n" );
        return status;
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
    // Initialize Serdes TX & RX
    //=================================

    printf ( "\r\n*** Initialize serdes ***\r\n" );

    // Configure RF Synthesizer 2 based on input clock freq
    if ( fclk_mhz == 6000 )
    {
        printf("Set RF Synth 2 port A to 300 MHz and port B to sysref: [echo /f1000/300_3.125_3pulse.txt > /dev/rfsynth2]...\r\n");
    }
    else
    {
        printf("Set RF Synth 2 port A to 200 MHz and port B to sysref: [echo /f1000/200_2.0833_3pulse.txt > /dev/rfsynth2]...\r\n");
    }
    wait_for_key(' ');

    // Initialize serdes TX
    status = iqa_serdes_init ( F1000_ID, ep_id, SERDES_MODE_DDR, SERDES_TX_INIT | SERDES_RX_INIT );

    if (status != IQA_SUCCESS)
    {
        printf("Serdes initialization failed (%d)\r\n", status);
        return status;
    }

    usleep ( 20000 );

    //=================================
    // Enable all Serdes TX lanes
    //=================================

    int lane_mask = ( 1 << 8 ) - 1;
    status = iqa_jesd_tx_lane_enable ( F1000_ID, ep_id, lane_mask );

    if (status != IQA_SUCCESS)
    {
        printf("Failed to Enable Lanes in EP-%s (%d)\r\n", ep_id  == EP_A  ? "A" : "B", status);
        return status;
    }

    // API returns 1 for DLL locked, 0 for unlocked, negative number for error
    status = iqa_help_dll_lock ( F1000_ID, ep_id, 0, SERDES_DLL, optimize );

    if ( status == 0 )
    {
        printf ( "\nSerdes DLL NOT LOCKED\r\n" );
        return -IQA_ERROR_DLL_LOCK;
    }
    else if ( status < IQA_SUCCESS )
    {
        printf ( "\nError in getting serdes DLL to lock, status %d\r\n", status );
        return status;
    }

    //=================================
    // Initialize ADC DSP
    //=================================

    printf ("\r\n*** Initialize ADC DSP ***\r\n");

    status = init_adc_dsp ( F1000_ID, ep_id, adc_id );

    if (status != IQA_SUCCESS)
    {
        printf("ADC DSP initialization failed.\r\n");
        return status;
    }

    //=================================
    // Set user defined NCO Frequency Tuning Word
    //=================================

    // 48Msps
    uint32_t band_freq[] = { 0x3bbbbbc,    // 700M
                             0x8000000,    // 1.5G
                             0xaaaaaab,    // 2.0G
                             0xccccccd,    // 2.4G
                             0xe666666,    // 2.7G
                             0xf777777,    // 2.9G
                             0x10000000,   // 3.0G
                             0x10888888    // 3.1G
                           };

    // 8 initial phases to be loaded into ADC DSP
    uint32_t initial_phase[] = { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 };

    band_id    = 1;

    // Skip frame sync until the last call to iqa_set_adc_dsp_freq()
    frame_sync = 0;

    status = iqa_set_adc_dsp_freq ( F1000_ID, ep_id, adc_id, band_id, band_freq, initial_phase, frame_sync );
    if ( status != IQA_SUCCESS )
    {
      printf ( "Failed to set ADC DSP frequency for EP-%c, ADC-%d, Band-%d, status %d\r\n", 'A' + ep_id - 1, adc_id, band_id, status );
      return status;
    }

    band_id    = 2;

    // Issue frame sync in the last call to iqa_set_adc_dsp_freq() so the freq take effect
    frame_sync = 1;

    status = iqa_set_adc_dsp_freq ( F1000_ID, ep_id, adc_id, band_id, band_freq, initial_phase, 1 );
    if ( status != IQA_SUCCESS )
    {
      printf ( "Failed to set ADC DSP frequency for EP-%c, ADC-%d, Band-%d, status %d\r\n", 'A' + ep_id - 1, adc_id, band_id, status );
      return status;
    }

    //=================================
    // Disable RX Link
    //=================================

    status = iqa_jesd_rx_link_enable ( F1000_ID, ep_id, 0 );

    if ( status != IQA_SUCCESS )
    {
      printf ( "Failed to Disable RX link in EP-%c, status %d\r\n", 'A' + ep_id - 1, status );
      return status;
    }

    //=================================
    // Enable RX Lanes
    //=================================

    printf("\r\nEnabling Serdes RX lanes 0x%x\r\n", rx_lane_bitmap);

    status = iqa_jesd_rx_lane_enable ( F1000_ID, ep_id, rx_lane_bitmap );

    if ( status != IQA_SUCCESS )
    {
      printf ( "Failed to Enable RX Lanes in EP-%c, status %d\r\n", 'A' + ep_id - 1, status );
      return status;
    }

    //=================================
    // Software reset OOB Processor
    //=================================

    status = iqa_oob_sw_reset ( F1000_ID, ep_id );
    if ( status != IQA_SUCCESS )
    {
      printf ( "Failed to reset OOB processor for EP-%c, status %d\r\n", 'A' + ep_id - 1, status );
      return status;
    }

    //=================================
    // Load Merlin firmware
    //=================================

    printf("\r\n*** Download Merlin firmware into EP-A & EP-B ***\r\n");
    status = iqa_merlin_load_firmware ( F1000_ID, EP_A | EP_B, (char *) merlin_b_fw,
                                          (uint32_t) merlin_b_fw_size, run_mode );

    if ( status != IQA_SUCCESS )
    {
        printf("Download Merlin firmware failed.\r\n");
        return status;
    }

    printf ("Bytes downloaded = %ld (0x%lx)\r\n", merlin_b_fw_size, merlin_b_fw_size);

    printf ("\r\nLoad bit file into FPGA. Press space bar when done...\r\n");
    wait_for_key(' ');

    //=================================
    // Enable RX Link
    //=================================

    status = iqa_jesd_rx_link_enable ( F1000_ID, ep_id, 1 );

    if ( status != IQA_SUCCESS )
    {
      printf ( "Failed to Enable RX link in EP-%c, status %d\r\n", 'A' + ep_id - 1, status );
      return status;
    }

    printf ( "\r\nADC -> DSP -> SERDES TX %s\r\n", (status == IQA_SUCCESS ? "PASSED" : "FAILED") );

    //=================================
    // End of Program
    //=================================

    Xil_ICacheDisable();
    Xil_DCacheDisable();

    printf("\r\n*** End of Program ***\r\n");

    return IQA_SUCCESS;
}
