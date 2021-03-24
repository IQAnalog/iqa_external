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
// File        : f1000_command_code.h
// Author      : Rudy Liang
// Created     : 1/14/2019
//
// Description:  Include header file for Command IDs for the F1000 server,
//               Command Decoder, and Client application.
//
//*******************************************************************************

#ifndef __INCLUDE_IQA_F1000_COMMAND_CODE_H
#define __INCLUDE_IQA_F1000_COMMAND_CODE_H

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
#define IQA_READ_CHIP_ID                     0x0000000F
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

#define IQA_INIT_CHIP                                       0x00000078


#define IQA_LOGGER                           0x00010000
#define IQA_LOG_ENTRY                        0x00010001
#define IQA_CAP_ONCE                         0x00010002
#define IQA_GET_DAC_CAP                      0x00010003

#endif  // __INCLUDE_IQA_F1000_COMMAND_CODE_H
