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
// File        : iqa_f1000_hal.h                                                *
// Author      : RL                                                             *
// Created     : 11/08/2018                                                     *
//                                                                              *
// Description:  Include header file for IQA F1000 HAL layer.                   *
//                                                                              *
//*******************************************************************************

#ifndef __INCLUDE_IQA_F1000_HAL_H
#define __INCLUDE_IQA_F1000_HAL_H

//===============================================================================
// Function prototypes
//===============================================================================

int iqa_hal_spi_read  ( int f1000_id, int ep_id, uint32_t addr, uint32_t * buf, uint32_t size, bool block, bool lock );
int iqa_hal_spi_write ( int f1000_id, int ep_id, uint32_t addr, uint32_t * buf, uint32_t size, bool block, bool lock );
int iqa_hal_spi_ioctl ( int f1000_id, int ep_id, int command, uint32_t addr, char * buf, uint32_t size, bool block, bool lock );

#endif  // __INCLUDE_IQA_F1000_HAL_H

// vim:  sw=3 ts=3 et
