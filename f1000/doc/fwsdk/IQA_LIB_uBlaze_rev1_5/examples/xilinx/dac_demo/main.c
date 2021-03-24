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
// Modified    : 04/14/2020                                                     *
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

#define F1000_ID            1          // Index 1 since there is exactly 1 F1000 on the board

extern int spi_init(void);
extern void gen_sine_wave(double cf, uint16_t *buf, int samples);

//===============================================================================
// Local declarations
//===============================================================================

static uint16_t sine_buf[sizeof(uint16_t)* 8192];

static int init_chip(int ep_id, int adc_id, int dac_id);

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
    printf ( "\r\n Connect & disconnect ATEST to 3.8V quickly. Press space bar when done...\r\n" );
    fflush(stdout);
    fflush(stdin);

    wait_for_key(' ');
    sleep(3);
}

//===============================================================================
// Initialize chip
//===============================================================================

static int init_chip(int ep_id, int    adc_id, int dac_id)
{
    IQA_STATUS  status;
    uint32_t    data;
    bool        is_rev_a;

    printf("\r\n*** Initialize Chip ***\r\n");

    is_rev_a = iqa_chip_is_rev_a ( F1000_ID );

    // Note: For F1000 rev A EVM, use 0.94 for dvdd. Use 0.85 for F1000 rev B/C EVM.
    cb_dvdd_voltage(0.94);

//    if ( is_rev_a )
//    {
//        printf("Set RF Synthesizer power level to 16: [rfsynth 1 2d; <& 0xFFC0 | 0x1_0000>...\n");
//        wait_for_key(' ');
//    }

    if ( is_rev_a ) 
    {
        status = iqa_init_analog ( F1000_ID, ep_id, adc_id, dac_id, 6000000000, &cb_dac );
    }
    else
    {
        // Note: For F1000 revision B, only DAC has been initialized. Use ADC_NONE instead.
        status = iqa_init_analog ( F1000_ID, ep_id, ADC_NONE, dac_id, 6000000000, &cb_dac );
    }

    if (status != IQA_SUCCESS)
    {
        printf ("\nFailed to initialize chip (%d).\n", status);
    }

    if ( is_rev_a ) 
    {
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
    }
    else
    {
        printf("DAC successfully powered up!\r\n");
    }

    return status;
}

//===============================================================================
// DAC Playback
//===============================================================================

static int dac_playback(int ep_id, int dac_id, uint32_t samples)
{
    IQA_STATUS    status;
    int i;
    int offset = 0;
    int samples_wrote = samples;
    FAR uint16_t *buf = &sine_buf[0];

    printf("Sine wave samples requested : %ld\r\n", samples);

    // Process 8k samples at a time
    while(samples_wrote > 0)
    {
        i = samples_wrote > 8192 ? 8192 : samples_wrote;

        // Generate a 500 MHz sine wave
        gen_sine_wave(500, buf, i);

        printf("Sine wave samples generated : %d\r\n", i);

        // Write samples to memory
        status = iqa_write_cap_memory ( F1000_ID, ep_id, dac_id == 1 ? MEM_DAC_1 : MEM_DAC_2, offset, 1, (char *) buf, i );
        samples_wrote -= i;
        offset += i*2;
    }

    // Start playback
    status = iqa_init_dac_playback ( F1000_ID, ep_id, dac_id, samples);

    printf("DAC Playback Done\r\n");

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
    int         ep_id;
    int         adc_id;
    int         dac_id;
    uint32_t    samples;    // Number of samples

    // Using EP-B, ADC 2, DAC 2, with 32K samples
    ep_id       = EP_B;
    adc_id      = ADC_NONE;
    dac_id      = DAC_2;
    samples     = 32768;

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

    if (IQA_SUCCESS != status)
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
    // DAC Playback
    //=================================

    printf("\r\n*** DAC Playback ***\r\n");
    status = dac_playback(ep_id, dac_id, samples);

    if (status != IQA_SUCCESS)
    {
        printf ("\nDAC Playback fails.\n");
    }

    Xil_ICacheDisable();
    Xil_DCacheDisable();

    printf("\r\n*** End of Program ***\r\n");

    return IQA_SUCCESS;
}
