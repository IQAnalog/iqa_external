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
// File        : iqa_f1000_serdes.h                                             *
// Author      : ND                                                             *
// Created     : 05/14/2019                                                     *
//                                                                              *
// Description:  Include header file for IQA F1000 SerDes functions.            *
//                                                                              *
//*******************************************************************************

#ifndef __INCLUDE_IQA_F1000_SERDES_H
#define __INCLUDE_IQA_F1000_SERDES_H

#define MAX_SERDES_LANES      8

//SERDES Modes
#define SERDES_DDR_MODE       0
#define SERDES_SDR_MODE       1

/**\brief This structure contains array of control mask for 8 SERDES Lanes
 *
 */
typedef struct lane_fanout
{
  uint16_t  enable_mask_lane[MAX_SERDES_LANES]; ///< Array  that is populated with the lane mask value for eight lanes
} lane_fanout_t;

/**\brief This structure contains adc select option for ADC DSP data distribution
 *
 */
typedef struct test_mode_sel
{
  int adc1; ///< 0 or 1
  int adc2; /// 0 or 1
}testmode_sel_t;

int iqa_jesd_rx_get_lane_status ( int f1000_id, int ep_id, int lane,
                                    uint32_t *pStatus );
int iqa_jesd_rx_lane_enable ( int f1000_id, int ep_id, int lane_mask );
int iqa_jesd_rx_link_enable ( int f1000_id, int ep_id, int enable );
int iqa_jesd_rx_sw_reset ( int f1000_id, int ep_id );
int iqa_jesd_tx_get_lane_status ( int f1000_id, int ep_id, int lane,
                                    uint32_t *pStatus );
int iqa_jesd_tx_lane_enable ( int f1000_id, int ep_id, int lane_mask );
int iqa_jesd_tx_link_enable ( int f1000_id, int ep_id, int enable );
int iqa_jesd_tx_sw_reset ( int f1000_id, int ep_id );
int iqa_serdes_init(int f1000_id,int ep_id,bool mode, int dir_mask);
int iqa_serdes_config_data_distribution(int f1000_id,int ep_id,
                                        lane_fanout_t lane_dist,
                                        testmode_sel_t testmode);
int iqa_serdes_rx_cal_lane_offset ( int f1000_id, int ep_id, int lane_id, int ntest );

#endif // __INCLUDE_IQA_F1000_SERDES_H
