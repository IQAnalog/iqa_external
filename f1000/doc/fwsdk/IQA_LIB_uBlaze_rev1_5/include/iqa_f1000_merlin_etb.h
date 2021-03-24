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
// File        : iqa_f1000_merlin_etb.h                                         *
// Author      : RL                                                             *
// Created     : 01/10/2020                                                     *
//                                                                              *
// Description:  Include file for APIs for F1000 Merlin Embedded Trace Buffer.  *
//                                                                              *
//*******************************************************************************

#ifndef __INCLUDE_IQA_F1000_MERLIN_ETB_H
#define __INCLUDE_IQA_F1000_MERLIN_ETB_H

//===============================================================================
// Defines
//===============================================================================
#define MERLIN_OOB                  0
#define MERLIN_TX                   1
#define MERLIN_RX                   2

#define NUM_TRACE_LINE              1024

//===============================================================================
// Function prototypes
//===============================================================================
int iqa_merlin_etb_attach  ( int f1000_id, int ep_id );
int iqa_merlin_etb_debug   ( int f1000_id, int ep_id, int type, int lane );
int iqa_merlin_etb_reset   ( int f1000_id, int ep_id );
int iqa_merlin_etb_init    ( int f1000_id, int ep_id );
int iqa_merlin_etb_trigger ( int f1000_id, int ep_id );
int iqa_merlin_etb_load    ( int f1000_id, int ep_id );
int iqa_merlin_etb_get     ( int f1000_id, int ep_id, int line, char * buf, int len, int formatted );
int iqa_merlin_etb_detach  ( int f1000_id, int ep_id );

#endif  // __INCLUDE_IQA_F1000_MERLIN_ETB_H

// vim:  sw=3 ts=3 et
//
