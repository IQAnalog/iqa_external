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
// File        : iqa_f1000_chip.h                                               *
// Author      : KP                                                             *
// Created     : 11/08/2018                                                     *
//                                                                              *
// Description:  Include file with chip specific prototypes and defines.        *
//                                                                              *
//*******************************************************************************

#ifndef __INCLUDE_IQA_F1000_CHIP_H
#define __INCLUDE_IQA_F1000_CHIP_H

#include "iqa_f1000_embedded_controller.h"

//===============================================================================
// Defines
//===============================================================================

// Macros to be used with iqa_adc_get_cal_status
#define  IQA_ADC_CAL_STATUS_DONE(x)             ((x) & 0x001) ///< ADC Calibration Process done
#define  IQA_ADC_CAL_STATUS_LSB_ERR_COUNT(x)    (((x) >> 1) & 0xff) ///< ADC cal status LSB error count
#define  IQA_ADC_CAL_STATUS_MSB_ERR_COUNT(x)    (((x) >> 9) & 0x7f) ///< ADC cal status MSB error count
#define  IQA_ADC_CAL_STATUS_TDC_VDD_FAIL(x)     (((x) >> 16) & 0x1) ///< ADC cal status failed TDC VDD

// DLL type
typedef enum
{
    ADC_DLL,
    DAC_DLL,
    SERDES_DLL
} dll_type_enum_t;

// DSP rate mode
typedef enum
{
  MODE_250MSPS = 0,               ///< Rate mode: 250 Mega Samples per Second
  MODE_500MSPS,                   ///< Rate mode: 500 Mega Samples per Second
  MODE_1GSPS,                     ///< Rate mode: 1 Giga Samples per Second
  MODE_2GSPS,                     ///< Rate mode: 2 Giga Samples per Second
  MODE_4GSPS                      ///< Rate mode: 4 Giga Samples per Second
} rate_mode_t;

//===============================================================================
// Structure types
//===============================================================================

/**\brief This structure contains callback functions which will be called during DAC wakeup
 *
 */

typedef struct cb_dac_s
{

  /** Callback function pointer for AVNN Switch handler. The
  * caller should implement the functionality in system level. The
  * IQA API calls this callback as needed.
  *
  */
  void ( * avnn_sw ) ( void );

  /** This callback enables the AVNN power rail. The caller should implement
   * the functionality in system level. The IQA API calls this callback as
   * needed.
   * <b>Note: The AVNN SW should be active between 10ms-20ms </b>
   *
   */
  void ( * avnn_en ) ( void );

  /** This callback sets the AVDD level. The caller needs to implement
   * the functionality in system level. The IQA API calls this callback as
   * needed.
   *
   * \param low_high bool : The IQA API calls with 0 (Low) or 1 (High) . The
   *                        Low corresponds to 1.35V and High Corresponds to 1.6V
   *
   */
  void ( * avdd_level ) ( bool low_high );

  /** This callback sets the AVDD voltage. The caller needs to implement
  * the functionality in system level. The IQA API calls this callback as
  * needed.
  *
  * \param v float : The IQA API calls with the Voltage value.
  *
  */
  void ( * avdd_voltage ) ( float v );

  /** This callback sets the DVDD voltage. The caller needs to implement
   * the functionality in system level. The IQA API calls this callback as
   * needed.
   *
   * \param v float : The IQA API calls with 0.8V nominal.
   *
   */
  void ( * dvdd_voltage ) ( float v );

  /** This callback sets the VREF level. The caller needs to implement
   * the functionality in system level. The IQA API calls this callback as
   * needed.
   *
   * \param low_high bool : The IQA API calls with 0 (Low) or 1 (High) . The
   *                        Low corresponds to 0.1V and High corresponds to 1.82V
   *
   */
  void ( * vref_level ) ( bool low_high );

  /** This callback sets the vref voltage. The caller needs to implement
  * the functionality in system level. The IQA API calls this callback as
  * needed.
  *
  * \param v uint16_t : The vref voltage value that the IQA API calls with.
  *
  */
  void ( * vref_voltage ) ( uint16_t v );

  /** This callback sets the ATEST switch. The caller needs to implement
  * the functionality in system level. The IQA API calls this callback as
  * needed.
  *
  */
  void ( * atest_sw ) ( void );
} cb_dac_t;

//===============================================================================
// Function prototypes
//===============================================================================

/// @see iqa_init in iqa_f1000_embedded_controller.h

bool iqa_chip_is_rev_a ( int f1000_id );

bool iqa_chip_is_rev_b ( int f1000_id );

int iqa_init_chip    ( int f1000_id, int ep_id, const cb_dac_t * cb );

int iqa_init_analog  ( int f1000_id, int ep_id, int adc_id, int dac_id,
                       uint64_t fclk, const cb_dac_t * cb );
int iqa_init_adc_dsp ( int f1000_id, int ep_id, int adc_id, int band_id,
                       uint32_t * band_freq, uint32_t * initial_phase,
                       rate_mode_t rate_mode,int frequency );

int iqa_adc_dsp_enable ( int f1000_id, int ep_id, int adc_bitmap, int band_bitmap,
                         rate_mode_t rate_mode, int frequency );

int iqa_help_dll_lock ( int f1000_id, int ep_id, int id, dll_type_enum_t dll_type, int optimize );

int iqa_set_adc_dsp_freq ( int f1000_id, int ep_id, int adc_id, int band_id,
                           uint32_t * band_freq, uint32_t * initial_phase, int frame_sync );

int iqa_set_adc_dsp_rate_mode ( int f1000_id, int ep_id, int adc_id,
                                int band_id, rate_mode_t rate_mode, int frequency );

void iqa_adc_dsp_start_repeat_playback ( int f1000_id, int ep_id, int adc_id,
                                         uint16_t sample_size );

void iqa_adc_dsp_stop_repeat_playback ( int f1000_id, int ep_id, int adc_id,
                                        uint16_t sample_size );

int iqa_set_dac_dsp_rate_mode ( int f1000_id, int ep_id, int dac_id, int band_id,
                                rate_mode_t rate_mode, int nco_index );

int iqa_set_dac_dsp_freq ( int f1000_id, int ep_id, int adc_id, int band_id,
                           uint32_t * band_freq, uint32_t * initial_phase,
                           int frame_sync );

int iqa_dac_dsp_enable ( int f1000_id, int ep_id, int dac_bitmap, int band_bitmap,
                         rate_mode_t rate_mode, int nco_index );

int iqa_init_dac_dsp ( int f1000_id, int ep_id, int dac_id, int band_id,
                       uint32_t * band_freq, uint32_t * initial_phase,
                       rate_mode_t rate_mode, int nco_index );

int iqa_read_chip_id ( int f1000_id, int ep_id, uint32_t * chip_id );

int iqa_read_chip_rev ( int f1000_id, int ep_id, uint8_t * chip_rev );

int iqa_adc_cal_restart ( int f1000_id, int ep_id );

int iqa_adc_get_cal_status ( int f1000_id, int ep_id, int adc_id,
                             uint32_t *status );

int iqa_reset_chip ( int f1000_id );

int iqa_set_baud_rate ( int f1000_id, int ep_id, uint64_t fclk, int baudrate );

int   iqa_prbs_config ( int f1000_id, int ep_id, int dir_mask,int prbs_seq_type,
                        int lane_mask, char *tx_prbs_framer_code,
                        uint32_t  tx_prbs_framer_code_size,
                        char *rx_prbs_framer_code,
                        uint32_t  rx_prbs_framer_code_size);


#endif  // __INCLUDE_IQA_F1000_CHIP_H

// vim:  sw=3 ts=3 et
//
