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
// File        : iqa_f1000_jesd.h                                               *
// Author      : KP                                                             *
// Created     : 05/13/2019                                                     *
//                                                                              *
// Description:  Include file for IQA library JESD functions.                   *
//                                                                              *
//*******************************************************************************

#ifndef __INCLUDE_IQA_F1000_JESD_H
#define __INCLUDE_IQA_F1000_JESD_H

//===============================================================================
// Define macros used by the routines
//===============================================================================

// Status bit mask defines for iqa_jesd_tx_get_lane_status API
#define  IQA_JESD_TX_LANE_STATUS_FSM_STATE           0x007
#define  IQA_JESD_TX_LANE_STATUS_CGS_LOCKED          0x008
#define  IQA_JESD_TX_LANE_STATUS_CGS_DONE            0x010
#define  IQA_JESD_TX_LANE_STATUS_ILAS_DONE           0x020
#define  IQA_JESD_TX_LANE_STATUS_ILAS_START          0x040
#define  IQA_JESD_TX_LANE_STATUS_DATA_STATE          0x080
#define  IQA_JESD_TX_LANE_STATUS_FRAMER_RELEASE      0x100

//===============================================================================
// Function prototypes
//===============================================================================

//===============================================================================
// Perform software reset on JESD TX link layer (ADC path).
//
// This will also disable all lanes in the TX block.  Lanes must 
// be re-enabled after calling this API.
//===============================================================================
int iqa_jesd_tx_sw_reset  ( int f1000_id, int ep_id );

//===============================================================================
// Enable JESD TX link layer (ADC path) lanes.
//===============================================================================
int iqa_jesd_tx_lane_enable ( int f1000_id, int ep_id, int lane_mask );

//===============================================================================
// Enable or disable the JESD TX link / core.
//===============================================================================
int iqa_jesd_tx_link_enable ( int f1000_id, int ep_id, int enable );

//===============================================================================
// Get the lane status for the specified lane.
//===============================================================================
int iqa_jesd_tx_get_lane_status ( int f1000_id, int ep_id, int lane, uint32_t *pStatus );

//===============================================================================
// Perform software reset on JESD RX link layer (DAC path).
//
// This will also disable all lanes in the RX block.  Lanes must 
// be re-enabled after calling this API.
//===============================================================================
int iqa_jesd_rx_sw_reset  ( int f1000_id, int ep_id );

//===============================================================================
// Enable JESD RX link layer (DAC path) lanes.
//===============================================================================
int iqa_jesd_rx_lane_enable ( int f1000_id, int ep_id, int lane_mask );

//===============================================================================
// Enable or disable the JESD RX link / core.
//===============================================================================
int iqa_jesd_rx_link_enable ( int f1000_id, int ep_id, int enable );

//===============================================================================
// Get the lane status for the specified lane.
//===============================================================================
int iqa_jesd_rx_get_lane_status ( int f1000_id, int ep_id, int lane, uint32_t *pStatus );

#endif  // __INCLUDE_IQA_F1000_JESD_H

// vim:  sw=3 ts=3 et
//
