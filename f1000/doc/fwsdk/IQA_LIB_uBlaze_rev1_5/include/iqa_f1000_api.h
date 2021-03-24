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
// File        : iqa_f1000_api.h                                                *
// Author      : RL                                                             *
// Created     : 11/08/2018                                                     *
//                                                                              *
// Description:  Main include header file for IQA F1000 API framework.          *
//                                                                              *
//*******************************************************************************

#ifndef __INCLUDE_IQA_F1000_API_H
#define __INCLUDE_IQA_F1000_API_H

///@cond
#ifdef __cplusplus
#define EXTERN extern "C"
extern "C" {
#else
#define EXTERN extern
#endif

#ifndef FAR
#define FAR
#endif


#include    <sys/types.h>
#include    <assert.h>
#include    <stdint.h>
#include    <stdbool.h>
#include    <string.h>
#include    <stdio.h>

//===============================================================================
// Include sub-header files
//===============================================================================

#include    "iqa_ald.h"
#include    "iqa_f1000_chip.h"
#include    "iqa_f1000_embedded_controller.h"
#include    "iqa_f1000_hal.h"
#include    "iqa_f1000_jesd.h"
#include    "iqa_f1000_memory.h"
#include    "iqa_f1000_merlin_etb.h"
#include    "iqa_f1000_register.h"
#include    "iqa_f1000_serdes.h"
#include    "iqa_revision.h"
///@endcond
//===============================================================================
// Define commonly used values/constants
//===============================================================================

// F1000 chip revision
#define     F1000_REV_A                 (0x01)              ///< F1000 revision A
#define     F1000_REV_B                 (0x02)              ///< F1000 revision B

// Element Processor index. Index should allow bitmap value so multiple EPs can be selected
#define     EP_MIN                      (0x01)              ///< Minimum Element Processor number
#define     EP_A                        (0x01)              ///< Element Processor A
#define     EP_B                        (0x02)              ///< Element Processor B
///@cond
#define     EP_MAX                      EP_B                ///< Maximum Element Processor number


#define     EP_BITMAP_MIN               (EP_A)              ///< Minimum Element Processor bitmap
#define     EP_BITMAP_MAX               (EP_A | EP_B)       ///< Maximum Element Processor bitmap (both EP_A and EP_B)

//supported clock Frequencies
#define     FCLK_MIN                    4000000000UL
#define     FCLK_MAX                    8000000000UL
///@endcond

// ADC index. Index should allow bitmap value so multiple ADCs can be selected
///@cond
#define     ADC_MIN                     (0x00)              ///< Minimum ADC number
///@endcond
#define     ADC_NONE                    (0x00)              ///< No ADC
#define     ADC_1                       (0x01)              ///< ADC 1
#define     ADC_2                       (0x02)              ///< ADC 2
///@cond
#define     ADC_MAX                     ADC_2               ///< Maximum ADC number


#define     ADC_BITMAP_MIN              (0)                 ///< Minimum ADC bitmap (no ADC selected)
#define     ADC_BITMAP_MAX              (ADC_1 | ADC_2)     ///< Maximum ADC bitmap (all ADCs)


// DAC index. Index should allow bitmap value so multiple DACs can be selected
#define     DAC_MIN                     (0x00)              ///< Minimum DAC number
///@endcond
#define     DAC_NONE                    (0x00)              ///< No DAC
#define     DAC_1                       (0x01)              ///< DAC 1
#define     DAC_2                       (0x02)              ///< DAC 2
///@cond
#define     DAC_MAX                     DAC_2               ///< Maximum DAC number

#define     DAC_BITMAP_MIN              (0)                 ///< Minimum DAC bitmap (no DAC selected)
#define     DAC_BITMAP_MAX              (DAC_1 | DAC_2)     ///< Maximum DAC bitmap (all DACs)

///< ADC Band index
#define     BAND_MIN                    (0x01)              ///< Minimum band number
///@endcond
#define     BAND_1                      (0x01)              ///< ADC/DAC Band 1
#define     BAND_2                      (0x02)              ///< ADC/DAC Band 2
///@cond
#define     BAND_MAX                    BAND_2              ///< Maximum band number

#define     BAND_BITMAP_MIN             (0)                 ///< Minimum BAND bitmap
#define     BAND_BITMAP_MAX             (BAND_1 | BAND_2)   ///< Maximum BAND bitmap (all BANDs)

// NCO index
#define     NCO_MIN                     1                   ///< Minimum NCO index
///@cond
#define     NCO_1                       1                   ///< NCO 1
#define     NCO_2                       2                   ///< NCO 2
#define     NCO_3                       3                   ///< NCO 3
#define     NCO_4                       4                   ///< NCO 4
#define     NCO_5                       5                   ///< NCO 5
#define     NCO_6                       6                   ///< NCO 6
#define     NCO_7                       7                   ///< NCO 7
#define     NCO_8                       8                   ///< NCO 8
///@cond
#define     NCO_MAX                     NCO_8               ///< Maximum NCO index

// SerDes RX/TX Lane index
#define     LANE_MIN                    0                   ///< Minimum LANE index
///@endcond
#define     LANE_0                      0                   ///< LANE 0 ID
#define     LANE_1                      1                   ///< LANE 1 ID
#define     LANE_2                      2                   ///< LANE 2 ID
#define     LANE_3                      3                   ///< LANE 3 ID
#define     LANE_4                      4                   ///< LANE 4 ID
#define     LANE_5                      5                   ///< LANE 5 ID
#define     LANE_6                      6                   ///< LANE 6 ID
#define     LANE_7                      7                   ///< LANE 7 ID
///@cond
#define     LANE_MAX                    LANE_7              ///< Maximum LANE index
///@endcond
//SerDes Mode
#define     SERDES_MODE_DDR             0                   ///<DDR Mode for SERDES Initialization
#define     SERDES_MODE_CDR             1                   ///<CDR Mode for SERDES Initialization

//Serdes Init Direction Mask
///@cond
#define     SERDES_DIR_MASK_MIN         0x1
///@endcond
#define     SERDES_TX_INIT              0x1                 ///<Initialize only Serdes Transmit
#define     SERDES_RX_INIT              0x2                 ///<Initialize only Serdes Receive
///@cond
#define     SERDES_DIR_MASK_MAX         0x3

//PRBS Init Direction Mask
///@cond
#define     PRBS_DIR_MASK_MIN           0x1
///@endcond
#define     PRBS_TX_INIT                0x1                 ///<Initialize only PRBS Generator for Transmit
#define     PRBS_RX_INIT                0x2                 ///<Initialize only PRBS Generator for Receive
///@cond
#define     PRBS_DIR_MASK_MAX         	0x3


#define     PRBS_SEQUENCE_MIN           4
///@endcond
#define     PRBS_SEQUENCE7              4                 ///<PRBS generating monic polynomial 7
#define     PRBS_SEQUENCE15             5                 ///<PRBS generating monic polynomial 15
#define     PRBS_SEQUENCE23             6                 ///<PRBS generating monic polynomial 23
#define     PRBS_SEQUENCE31             7                 ///<PRBS generating monic polynomial 31
///@cond
#define     PRBS_SEQUENCE_MAX           7

// Memory ID values
#define     MEM_ID_MIN                  0
///@endcond
#define     MEM_ADC_1                   0                   ///< ADC/DAC 1 memory in 10-bit ADC mode
#define     MEM_DAC_1                   1                   ///< ADC/DAC 1 memory in 8-bit DAC mode
#define     MEM_ADC_2                   2                   ///< ADC/DAC 2 memory in 10-bit ADC mode
#define     MEM_DAC_2                   3                   ///< ADC/DAC 2 memory in 8-bit DAC mode
///@cond
#define     MEM_TEST_1                  4                   ///< Test 1 memory
#define     MEM_TEST_2                  5                   ///< Test 2 memory

#define     MEM_ID_MAX                  5
///@endcond

//===============================================================================
// Error codes
//===============================================================================
#define     IQA_STATUS                  int                 ///< IQA API Status data type

#define     IQA_SUCCESS                   0                 ///< Function execution success
#define     IQA_ERROR_NONE                0

#define     IQA_ERROR_PARAM              10                 ///< One of the Parameters is wrong
#define     IQA_ERROR_READ_INVALID       11                 ///< Invalid read
#define     IQA_ERROR_AGAIN              12                 ///< Try again

#define     IQA_ERROR_F1000_ID          100                 ///< Invalid F1000 ID
#define     IQA_ERROR_EP_ID             101                 ///< Invalid Element Processor ID
#define     IQA_ERROR_EP_OPEN           102                 ///< Unable to open Element processor
#define     IQA_ERROR_EP_READ           103                 ///< Unable to Read from the Element Processor
#define     IQA_ERROR_EP_WRITE          104
#define     IQA_ERROR_MEM_ID            105                 ///< Invalid Memory Id
#define     IQA_ERROR_NO_RESPONSE       106                 ///< No Response from F1000
#define     IQA_ERROR_ADC_INACTIVE      107
#define     IQA_ERROR_DAC_INIT          108                 ///< DAC Initialization Failed
#define     IQA_ERROR_F1000_REV         109                 ///< Incorrect F1000 Revision
#define     IQA_ERROR_DLL_LOCK          110                 ///< DLL failed to lock

#define     IQA_ERROR_FILE              200                 ///< Unable to open the file
#define     IQA_ERROR_FILE_FORMAT       201                 ///< Invalid File format
#define     IQA_ERROR_DOWNLOAD_FW       210                 ///< Error Downloading the Firmware
#define     IQA_ERROR_NO_MEMORY         220                 ///< Error no Memory
#define     IQA_ERROR_NULL_POINTER      221                 ///< Invalid address or Null Pointer

#define     IQA_ERROR_SPI_INIT          300                 ///< SPI initialization error

// SPI related
#define     SPI_BLOCKING                true                ///< SPI execution uses blocking operation
#define     SPI_NON_BLOCKING            false               ///< SPI execution uses non-blocking operation; i.e. interrupt driven

#define     SPI_LOCKING                 true                ///< Lock SPI bus for operation
#define     SPI_NON_LOCKING             false               ///< Don't lock SPI bus for operation

//===============================================================================
// Macros
//===============================================================================

// MIN and MAX macros

#define     MIN_DAC_PLAYBACK_SAMPLES    0x40            ///< Minimum number of DAC playback samples per transaction
#define     MAX_DAC_PLAYBACK_SAMPLES    0x8000          ///< Maximum number of DAC playback samples per transaction

#define     MIN_ADC_CAPTURE_SAMPLES     0x40          ///< Minimum number of ADC capture samples per transaction
#define     MAX_ADC_CAPTURE_SAMPLES     0x8000        ///< Maximum number of ADC capture samples per transaction

///@cond
#ifndef MIN
#  define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

#ifndef MAX
#  define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif

#ifndef NULL
#define NULL   ((void *) 0)
#endif

#undef EXTERN
#ifdef __cplusplus
}
#endif
///@endcond
#endif  // __INCLUDE_IQA_F1000_API_H

// vim:  sw=3 ts=3 et
