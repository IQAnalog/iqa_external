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
// File        : iqa_f1000_memory.h                                             *
// Author      : KP                                                             *
// Created     : 11/08/2018                                                     *
//                                                                              *
// Description : Include file with memory access specific prototypes and defines*
//                                                                              *
//*******************************************************************************

#ifndef __INCLUDE_IQA_F1000_MEMORY_H
#define __INCLUDE_IQA_F1000_MEMORY_H

// Define the memory base addresses

#define  MEM_ADC_1_BASE        0x50000000
#define  MEM_ADC_2_BASE        0x60000000
#define  MEM_DAC_1_BASE        0x70000000
#define  MEM_DAC_2_BASE        0x80000000
#define  MEM_TEST_1_CAP_BASE   0xA0000000
#define  MEM_TEST_2_CAP_BASE   0xB0000000
#define  MEM_TEST_1_PLAY_BASE  0xC0000000
#define  MEM_TEST_2_PLAY_BASE  0xD0000000

//===============================================================================
// Function prototypes
//===============================================================================

int iqa_read_cap_memory  ( int f1000_id, int ep_id, int mem_id, int offset, int ordered,
                           char * buf, uint32_t size );
int iqa_read_cap_memory_legacy ( int f1000_id, int ep_id, int mem_id, int offset,
                                 int ordered, char * buf, uint32_t samples );
int iqa_write_cap_memory ( int f1000_id, int ep_id, int mem_id, int offset, int ordeed,
                           char * buf, uint32_t size );

int iqa_init_adc_capture ( int f1000_id, int ep_id, int adc_id, int capture_size);
int iqa_init_dac_playback ( int f1000_id, int ep_id, int dac_id, int playback_size);

int iqa_init_test_playback ( int f1000_id, int ep_id,
                            int test_mem_id, int playback_size);

#endif  // __INCLUDE_IQA_F1000_MEMORY_H

// vim:  sw=3 ts=3 et
//
