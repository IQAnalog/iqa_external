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
// Created     : 06/10/2019                                                     *
//                                                                              *
// Description : Application to exercise calling IQ-Analog's APIs.              *
//                                                                              *
//*******************************************************************************

#include <stdio.h>
#include <unistd.h>

#include "xil_cache.h"
#include "xspi.h"
#include "spi_header.h"

#include "iqa_f1000_api.h"
#include "f1000_ioctl.h"
#include "riscv_serdes_tx.h"
#include "test_mem_pattern.h"

#define F1000_ID            1          // Index 1 since there is exactly 1 F1000 on the board

extern int spi_init(void);

//===============================================================================
// Local declarations
//===============================================================================

static int load_riscv_fw(int ep_bitmap, const unsigned char * fw, const uint32_t size);
static int serdes_init(int ep_id, int dir_mask, int mode, int fclk_mhz);

// Callback functions to be called during initialization
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
// Callback functions to be called during initialization
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

    // Wait for calibration routine to complete
    printf ( "\r\nWaiting for ADC Calibration to Complete...\r\n" );

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

    return IQA_ERROR_NONE;
}

//===============================================================================
// Initialize SerDes
//===============================================================================

static int serdes_init(int ep_id, int dir_mask, int mode, int fclk_mhz)
{
    IQA_STATUS    status = IQA_ERROR_NONE;

    usleep(20000);

    cb_dvdd_voltage(0.94);
    usleep(20000);

    cb_avdd_voltage(1.26);
    usleep(20000);

    // Set RF Synthesizers
    if (fclk_mhz == 6000)
    {
        printf("Set RF Synth 1 port B to 6 GHz: [rfsynth 1 -f b 6000]...\n");
        wait_for_key(' ');

        printf("Set RF Synth 2 port A to 300 MHz and port B to sysref: [echo /f1000/300_3.125_2pulse.txt > /dev/rfsynth2]...\n");
        wait_for_key(' ');
    }
    else
    {
        printf("Set RF Synth 1 port B to 4 GHz: [rfsynth 1 -f b 4000]...\n");
        wait_for_key(' ');

        printf("Set RF Synth 2 port A to 200 MHz and port B to sysref: [echo /f1000/200_2.0833_2pulse.txt > /dev/rfsynth2]...\n");
        wait_for_key(' ');
    }

    // Reset SerDes TX
    if ( dir_mask & SERDES_TX_INIT )
      status = iqa_jesd_tx_sw_reset ( F1000_ID, ep_id );

    usleep ( 20000 );

    status = iqa_serdes_init(F1000_ID, ep_id, (bool) mode, dir_mask);

    usleep(20000);

    cb_vref_level(true);

    return status;
}

//===============================================================================
// Load data pattern into test memory
//===============================================================================
static int load_test_mem(int f1000_id, int ep_id)
{
    int status = IQA_SUCCESS;
    int ordered = 1;
    int oi = 0;
    int oq = 1;
    int ii = 2;
    int iq = 3;
    int stream_offset[] = { 0x3000,  //OI = 0
                            0x2000,  //Oq = 1
                            0x7000,  //Ii = 2
                            0x6000}; //Iq = 3

    status = iqa_write_cap_memory ( f1000_id, ep_id, MEM_TEST_1, stream_offset[ii], ordered, (char*) test_mem_1_pattern_ii, 1024 );
    status = iqa_write_cap_memory ( f1000_id, ep_id, MEM_TEST_1, stream_offset[iq], ordered, (char*) test_mem_1_pattern_iq, 1024 );
    status = iqa_write_cap_memory ( f1000_id, ep_id, MEM_TEST_1, stream_offset[oi], ordered, (char*) test_mem_1_pattern_oi, 1024 );
    status = iqa_write_cap_memory ( f1000_id, ep_id, MEM_TEST_1, stream_offset[oq], ordered, (char*) test_mem_1_pattern_oq, 1024 );

    status = iqa_write_cap_memory ( f1000_id, ep_id, MEM_TEST_2, stream_offset[ii], ordered, (char*) test_mem_2_pattern_ii, 1024 );
    status = iqa_write_cap_memory ( f1000_id, ep_id, MEM_TEST_2, stream_offset[iq], ordered, (char*) test_mem_2_pattern_iq, 1024 );
    status = iqa_write_cap_memory ( f1000_id, ep_id, MEM_TEST_2, stream_offset[oi], ordered, (char*) test_mem_2_pattern_oi, 1024 );
    status = iqa_write_cap_memory ( f1000_id, ep_id, MEM_TEST_2, stream_offset[oq], ordered, (char*) test_mem_2_pattern_oq, 1024 );

    return status;
}

//===============================================================================
// Start SerDes
//===============================================================================

static int serdes_start(int f1000_id,int ep_id)
{
  int  status = IQA_SUCCESS;
  lane_fanout_t lane_dist;
  testmode_sel_t testmode;
  int i;

  for (i = 0; i < 8;i++)
      lane_dist.enable_mask_lane[i] = 0x0C0C;

  testmode.adc1 = 1;
  testmode.adc2 = 1;

  status = iqa_serdes_config_data_distribution(f1000_id, ep_id, lane_dist, testmode);
  return status;
}

//===============================================================================
// Generate sysref request by toggling GPIO
//===============================================================================

static int gen_sysref_req(int f1000_id, int ep_id)
{
  int status = IQA_SUCCESS;
  uint32_t epa_gpio0_en_reg = 0x1A101000; // EP-A RIDCV GPIO 0 control register
  uint32_t epa_gpio0_reg    = 0x1A101008; // EP-A RISCV GPIO 0 register

  // Enable RISC_V EPA GPIO_0 As Output
  status = iqa_write_register ( f1000_id, ep_id, epa_gpio0_en_reg, 1 );

  if (status != IQA_SUCCESS)
    return status;

  // Toggle RISC_V EPA GPIO_0 to generate SYSREF request
  status = iqa_write_register ( f1000_id, ep_id, epa_gpio0_reg, 1 );

  if (status != IQA_SUCCESS)
    return status;

  status = iqa_write_register ( f1000_id, ep_id, epa_gpio0_reg, 0 );

  return status;
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
    int         ep_id      = EP_B;
    int         dir_mask   = 1;       // 1 - TX init, 2 - RX init, 3 - TX & RX init
    int         mode       = 0;       // 0 - DDR, 1- SDR
    int         fclk_mhz   = 6000;    // 6GHz
    int         test_mem_1 = 1;
    int         test_mem_2 = 2;

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

    if (IQA_ERROR_NONE != status)
    {
        printf("Error reading chip id (%d)\r\n", status);
        return status;
    }

    printf("Chip ID = 0x%lX\r\n", chip_id);

    //=================================
    // Initialize Serdes TX
    //=================================

    status = serdes_init(ep_id, dir_mask, mode, fclk_mhz);

    if (status != IQA_ERROR_NONE)
    {
        printf ("\nSerdes TX fails.\n");
    }

    printf("\r\n*** Load RISC-V firmware ***\r\n");
    load_riscv_fw(ep_id, riscv_serdes_tx_ald, riscv_serdes_tx_ald_size);

    //=================================
    // Load data pattern into test memory
    //=================================

    printf("\r\n*** Load pattern into test memory ***\r\n");
    status = load_test_mem(F1000_ID, ep_id);

    if (status != IQA_SUCCESS)
    {
        printf ("\r\nLoad pattern into test memory fails.\r\n");
    }

    //=================================
    // Playback from test memory
    //=================================

    status = iqa_init_test_playback (F1000_ID, ep_id, test_mem_1, 256);
    status = iqa_init_test_playback (F1000_ID, ep_id, test_mem_2, 1024);

    //=================================
    // Start serdes operation
    //=================================

    printf("\r\n*** Start serdes TX ***\r\n");
    status = serdes_start(F1000_ID, ep_id);

    if (status != IQA_SUCCESS)
    {
        printf ("\r\nFailed iqa_serdes_config_data_distribution.\r\n");
    }

    //=================================
    // Prepare FPGA
    //=================================

    printf ("\r\nLoad and Play the Bit File in FPGA. Press Space bar When Done...\r\n");
    wait_for_key(' ');

    //=================================
    // Generate sysref request
    //=================================

    printf ("Sysref request generated.\r\n");
    status = gen_sysref_req(F1000_ID, EP_A);

    //=================================
    // End of Program
    //=================================

    Xil_ICacheDisable();
    Xil_DCacheDisable();

    printf("\r\n*** End of Program ***\r\n");

    return IQA_ERROR_NONE;
}

