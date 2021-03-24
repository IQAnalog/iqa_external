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
// File        : iqa_f1000_register.h                                           *
// Author      : KP                                                             *
// Created     : 11/08/2018                                                     *
//                                                                              *
// Description : Include file with register access specific prototypes and      *
//               defines.                                                       *
//                                                                              *
//*******************************************************************************

#ifndef __INCLUDE_IQA_F1000_REGISTER_H
#define __INCLUDE_IQA_F1000_REGISTER_H

//===============================================================================
// Function prototypes
//===============================================================================

int iqa_read_register    ( int f1000_id, int ep_id, uint32_t addr, uint32_t * value );
int iqa_write_register   ( int f1000_id, int ep_id, uint32_t addr, uint32_t   value );
int iqa_rdmodwr_register ( int f1000_id, int ep_id, uint32_t addr, uint32_t mask, uint32_t value );

#endif  // __INCLUDE_IQA_F1000_REGISTER_H

// vim:  sw=3 ts=3 et
//
