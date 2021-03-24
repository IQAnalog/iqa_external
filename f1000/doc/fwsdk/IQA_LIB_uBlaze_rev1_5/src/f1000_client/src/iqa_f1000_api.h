//*******************************************************************************
//      __  __                                                                  *
//     /  \/  \                                                                 *
//         \   \                                                                *
//    I Q - A N A L O G                                                         *
//           \   \            IQ-Analog Corp                                    *
//            \__/\__/       www.iqanalog.com                                   *
//                                                                              *
// -----------------------------------------------------------------------------*
//
// Copyright (C) IQ-Analog Corp. All rights reserved.
//
// ------------------------------------------------------------------------------
//  CONFIDENTIAL
// ------------------------------------------------------------------------------
//
// File        : iqa_f1000_api.h
// Author      : RL
// Created     : 11/08/2018
//
// Description:  Main include header file for IQA F1000 API framework.
//
//*******************************************************************************

#ifndef __INCLUDE_IQA_F1000_API_H
#define __INCLUDE_IQA_F1000_API_H

#ifdef __cplusplus
#define EXTERN extern "C"
extern "C" {
#else
#define EXTERN extern
#endif

#include <sys/types.h>
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>

typedef uint8_t uchar8;
typedef uint32_t uint32;
typedef uint16_t uint16;

//#define true    1
//#define false   0

//===============================================================================
// Include sub-header files
//===============================================================================
//#include    "f1000.hxx"
//#include    "iqa_f1000_chip.h"
//#include    "iqa_f1000_riscv.h"
//#include    "iqa_f1000_memory.h"
//#include    "iqa_f1000_hal.h"

//===============================================================================
// API Command codes
//===============================================================================
#define IQA_READ_REG                         0x00000001
#define IQA_WRITE_REG                        0x00000002
#define F1000_ADC_ENABLE                     0x00000003
#define F1000_DAC_ENABLE                     0x00000004
#define F1000_ADC_DSP_START_CAPTURE          0x00000005
#define F1000_ADC_READ_MEM                   0x00000006
#define F1000_ADC_DSP_START_PLAYBACK         0x00000007
#define F1000_DAC_DSP_START_CAPTURE          0x00000008
#define F1000_DAC_READ_MEM                   0x00000009
#define F1000_DAC_DSP_START_PLAYBACK         0x0000000A
#define F1000_DAC_DSP_LOAD_WAVEFORM          0x0000000B
#define EP_READ_CHIP_REVISION                0x0000000C
#define EP_READ_PROCESSOR_ID                 0x0000000D
#define EP_ENABLE_PROCESSOR                  0x0000000E
#define EP_READ_CHIP_ID                      0x0000000F
#define IQA_READ_RF_SYNTHESIZER              0x00000010
#define IQA_WRITE_RF_SYNTHESIZER             0x00000011
#define IQA_MATLAB_TEST3PARAMS               0x00000012
#define IQA_MATLAB_TEST4PARAMS               0x00000013
#define IQA_SYSTEM                           0x00000014

#define NCO_INIT                             0x00000015
#define NCO_SELECT_COEFFICIENT               0x00000016
#define F1000_TEST_CAPTURE_MEM_WRITE         0x00000017
#define F1000_TEST_CAPTURE_MEM_READ          0x00000018
#define F1000_DSP_SET_IQ_COEFFS              0x00000019
#define F1000_DSP_GET_IQ_COEFFS              0x0000001A
#define F1000_DSP_RX_SET_BEAM_BANDWIDTH      0x0000001B
#define F1000_DSP_TX_SET_BEAM_BANDWIDTH      0x0000001C
#define F1000_DSP_SET_RATE_MODE              0x0000001D
#define F1000_DSP_GET_RATE_MODE              0x0000001E
#define F1000_CAPTURE_MEM_READ               0x0000001F
#define F1000_CAPTURE_MEM_WRITE              0x00000020
#define F1000_TX_DIGITAL_MEM_READ            0x00000021
#define F1000_TX_DIGITAL_MEM_WRITE           0x00000022

#define F1000_EP_CAPTURE_MEM_READ            0x00000023
#define F1000_EP_CAPTURE_MEM_WRITE           0x00000024
#define F1000_EP_TEST_CAPTURE_MEM_READ       0x00000025
#define F1000_EP_TEST_CAPTURE_MEM_WRITE      0x00000026
#define F1000_EP_RX_DIGITAL_MEM_READ         0x00000027
#define F1000_EP_RX_DIGITAL_MEM_WRITE        0x00000028
#define F1000_EP_OOB_MEM_READ                0x00000029
#define F1000_EP_OOB_MEM_WRITE               0x0000002A
#define F1000_EC_MEM_READ                    0x0000002B
#define F1000_EC_MEM_WRITE                   0x0000002C
#define F1000_SET_NUM                        0x0000002D
#define F1000_JESD_RX_LINK_SW_RESET          0x0000002E
#define F1000_JESD_RX_LINK_SET_FRAME_SIZE    0x0000002F
#define F1000_JESD_RX_LINK_GET_FRAME_SIZE    0x00000030
#define F1000_JESD_RX_LINK_SET_MULTIFRAME_SIZE    0x00000031
#define F1000_JESD_RX_LINK_GET_MULTIFRAME_SIZE    0x00000032
#define F1000_JESD_RX_LINK_SET_SUBCLASS      0x00000033
#define F1000_JESD_RX_LINK_GET_SUBCLASS      0x00000034
#define F1000_JESD_RX_LINK_ENABLE_DYNAMIC_RECONFIG          0x00000035
#define F1000_JESD_RX_LINK_DISABLE_DYNAMIC_RECONFIG         0x00000036
#define F1000_JESD_RX_LINK_GET_DYNAMIC_RECONFIG             0x00000037
#define F1000_JESD_RX_LINK_ENABLE_SYSREF_MONITOR            0x00000038
#define F1000_JESD_RX_LINK_DISABLE_SYSREF_MONITOR           0x00000039
#define F1000_JESD_RX_LINK_GET_SYSREF_MONITOR               0x0000003A
#define F1000_JESD_RX_LINK_ENABLE_SYSREF_REALIGN            0x0000003B
#define F1000_JESD_RX_LINK_DISABLE_SYSREF_REALIGN           0x0000003C
#define F1000_JESD_RX_LINK_GET_SYSREF_REALIGN               0x0000003D
#define F1000_JESD_RX_LINK_FORCE_LANES_RESYNC               0x0000003E
#define F1000_JESD_RX_LINK_ENABLE_LANES_SYNC                0x0000003F
#define F1000_JESD_RX_LINK_DISABLE_LANES_SYNC               0x00000040
#define F1000_JESD_RX_LINK_GET_LANES_SYNC                   0x00000041
#define F1000_JESD_RX_LINK_ENABLE_SHIFT_SIGNAL              0x00000042
#define F1000_JESD_RX_LINK_DISABLE_SHIFT_SIGNAL             0x00000043
#define F1000_JESD_RX_LINK_GET_SHIFT_SIGNAL                 0x00000044
#define F1000_JESD_RX_LINK_SET_FIFO_THRESHOLD               0x00000045
#define F1000_JESD_RX_LINK_GET_FIFO_THRESHOLD               0x00000046
#define F1000_JESD_RX_LINK_SET_ENABLE_LINKS                 0x00000047
#define F1000_JESD_RX_LINK_GET_ENABLE_LINKS                 0x00000048
#define F1000_JESD_RX_LINK_SET_K_COMMA_LOCK_THRESHOLD       0x00000049
#define F1000_JESD_RX_LINK_GET_K_COMMA_LOCK_THRESHOLD       0x0000004A
#define F1000_JESD_RX_LINK_SET_K_COMMA_RESYNC_THRESHOLD     0x0000004B
#define F1000_JESD_RX_LINK_GET_K_COMMA_RESYNC_THRESHOLD     0x0000004C
#define F1000_JESD_RX_LINK_SET_ALIGN_DELAY                  0x0000004D
#define F1000_JESD_RX_LINK_GET_ALIGN_DELAY                  0x0000004F
#define F1000_JESD_RX_LINK_SET_CGS_CHECK_CLEAR_COUNT        0x00000050
#define F1000_JESD_RX_LINK_GET_CGS_CHECK_CLEAR_COUNT        0x00000051
#define F1000_JESD_RX_LINK_ENABLE_CGS_FSM                   0x00000052
#define F1000_JESD_RX_LINK_DISABLE_CGS_FSM                  0x00000053
#define F1000_JESD_RX_LINK_GET_CGS_FSM_STATE                0x00000054
#define F1000_JESD_RX_LINK_SET_FCLK_DELAY                   0X00000055
#define F1000_JESD_RX_LINK_GET_FCLK_DELAY                   0X00000056
#define F1000_JESD_RX_LINK_GET_SYSREF_MISALIGN_COUNT        0x00000057
#define F1000_JESD_RX_LANE_ENABLE                           0x00000058
#define F1000_JESD_RX_LINK_SET_ILAS_MULTIFRAME              0x00000059
#define F1000_JESD_RX_LINK_GET_ILAS_MULTIFRAME              0x0000005A
#define F1000_JESD_RX_LINK_ENABLE_ILAS_STATE_MACHINE        0x0000005B
#define F1000_JESD_RX_LINK_DISABLE_ILAS_STATE_MACHINE       0x0000005C
#define F1000_JESD_RX_LINK_GET_ILAS_STATE_MACHINE           0x0000005D
#define F1000_JESD_RX_LINK_ENABLE_ILAS_ALIGN_ERROR_CHECK    0x0000005E
#define F1000_JESD_RX_LINK_DISABLE_ILAS_ALIGN_ERROR_CHECK   0x0000005F
#define F1000_JESD_RX_LINK_ENABLE_ILAS_ALIGN_MONITOR        0x00000060
#define F1000_JESD_RX_LINK_DISABLE_ILAS_ALIGN_MONITOR       0x00000061
#define F1000_JESD_RX_LINK_ENABLE_AUTO_REALIGN              0x00000062
#define F1000_JESD_RX_LINK_DISABLE_AUTO_REALIGN             0x00000063
#define F1000_JESD_RX_LINK_ENABLE_SINGLE_LANE_MODE          0x00000064
#define F1000_JESD_RX_LINK_DISABLE_SINGLE_LANE_MODE         0x00000065
#define F1000_JESD_RX_LINK_SET_DYNAMIC_CONFIG               0x00000066
#define F1000_JESD_RX_LINK_GET_DYNAMIC_CONFIG               0x00000067
#define F1000_JESD_RX_LINK_SET_DAC_DEVICE_PER_LINK          0x00000068
#define F1000_JESD_RX_LINK_GET_DAC_DEVICE_PER_LINK          0x00000069
#define F1000_JESD_RX_LINK_SET_CONVERTER_RES                0x0000006A
#define F1000_JESD_RX_LINK_GET_CONVERTER_RES                0x0000006B
#define F1000_JESD_RX_LINK_SET_SAMPLE_BITS                  0x0000006C
#define F1000_JESD_RX_LINK_GET_SAMPLE_BITS                  0x0000006D
#define F1000_JESD_RX_LINK_SET_FRAME_SAMPLES                0x0000006E
#define F1000_JESD_RX_LINK_GET_FRAME_SAMPLES                0x0000006F
#define F1000_JESD_RX_IS_LMFC_ALIGNED                       0x00000070
#define F1000_JESD_RX_IS_FCLK_LOCKED                        0x00000071
#define F1000_JESD_RX_IS_LINK0_LANES_IN_DATA_STATE          0x00000072
#define F1000_JESD_RX_IS_LINK1_LANES_IN_DATA_STATE          0x00000073
#define F1000_JESD_RX_LINK_MUX_OUT                          0x00000074
#define F1000_JESD_RX_GET_K_SYMBOL_COUNT                    0x00000075

#define F1000_ADC_READ_MEM_ORDERED                          0x00000076
#define F1000_DAC_READ_MEM_ORDERED                          0x00000077

#define IQA_LOGGER                           0x00010000
#define IQA_LOG_ENTRY                        0x00010001
#define IQA_CAP_ONCE                         0x00010002
#define IQA_GET_DAC_CAP                      0x00010003


//===============================================================================
// Define commonly used values/constants
//===============================================================================

// Element Processor index. Index should allow bitmap value so multiple EPs can be selected
#define     EP_MIN                      (0x01)  // Minimum Element Processor number
#define     EP_A                        (0x01)  // Element Processor A
#define     EP_B                        (0x02)  // Element Processor B
#define     EP_MAX                      EP_B    // Maximum Element Processor number

#define     EP_BITMAP_MIN               (EP_A)          // Minimum Element Processor bitmap
#define     EP_BITMAP_MAX               (EP_A | EP_B)   // Maximum Element Processor bitmap (both EP_A and EP_B)

// ADC index
#define     ADC_MIN                     1       // Minimum ADC number
#define     ADC_1                       1       // ADC 1
#define     ADC_2                       2       // ADC 2
#define     ADC_MAX                     ADC_2   // Maximum ADC number

// DAC index
#define     DAC_MIN                     1       // Minimum DAC number
#define     DAC_1                       1       // DAC 1
#define     DAC_2                       2       // DAC 2
#define     DAC_MAX                     DAC_2   // Maximum DAC number


// ADC Band index
#define     BAND_MIN                    1       // Minimum band number
#define     BAND_1                      1       // ADC Band 1
#define     BAND_2                      2       // ADC Band 2
#define     BAND_MAX                    BAND_2  // Maximum band number

// NCO index
#define     NCO_MIN                     1       // Minimum NCO index
#define     NCO_1                       1       // NCO 1
#define     NCO_2                       2       // NCO 2
#define     NCO_3                       3       // NCO 3
#define     NCO_4                       4       // NCO 4
#define     NCO_5                       5       // NCO 5
#define     NCO_6                       6       // NCO 6
#define     NCO_7                       7       // NCO 7
#define     NCO_8                       8       // NCO 8
#define     NCO_MAX                     NCO_8   // Maximum NCO index

// SerDes RX/TX Lane index
#define     LANE_MIN                    0       // Minimum LANE index
#define     LANE_0                      0       // LANE 0
#define     LANE_1                      1       // LANE 1
#define     LANE_2                      2       // LANE 2
#define     LANE_3                      3       // LANE 3
#define     LANE_4                      4       // LANE 4
#define     LANE_5                      5       // LANE 5
#define     LANE_6                      6       // LANE 6
#define     LANE_7                      7       // LANE 7
#define     LANE_MAX                    LANE_7  // Maximum LANE index

//===============================================================================
// Error codes
//===============================================================================
#define     IQA_STATUS                  int
#define     IQA_SUCCESS                   0
#define     IQA_ERROR_NONE                0


#define     IQA_ERROR_F1000_ID          100
#define     IQA_ERROR_EP_ID             101
#define     IQA_ERROR_EP_OPEN           102

#define     IQA_ERROR_SPI_INIT          300

// SPI related
#define     SPI_BLOCKING                true        // SPI execution uses blocking operation
#define     SPI_NON_BLOCKING            false       // SPI execution uses non-blocking operation; i.e. interrupt driven

#define     SPI_LOCKING                 true        // Lock SPI bus for operation
#define     SPI_NON_LOCKING             false       // Don't lock SPI bus for operation
#define     API_ERROR                   0xFFFFFFFF


#undef EXTERN
#ifdef __cplusplus
}
#endif

#endif  // __INCLUDE_IQA_F1000_API_H

// vim:  sw=3 ts=3 et
