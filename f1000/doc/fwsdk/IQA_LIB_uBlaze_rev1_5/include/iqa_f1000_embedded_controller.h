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
// File        : iqa_f1000_embedded_controller.h                                *
// Author      : RL                                                             *
// Created     : 11/14/2018                                                     *
//                                                                              *
// Description:  Include header file for F1000 embedded controllers including   *
// the RISC-V processor and the Merlin framer / OOB processors.                 *
//                                                                              *
// Communication with the RISC-V will be done via general purpose registers,    *
// one for command and another for status.                                      *
//                                                                              *
// Mailbox is an area reserved in RISC-V's data memory for communications with  *
// the embedded controller.                                                     *
//                                                                              *
// A later release may use a Mailbox scheme where an INBOX area is used to      *
// send data to the RISC-V and an OUTBOX area to send data back to the host.    *
//                                                                              *
//*******************************************************************************

#ifndef __INCLUDE_IQA_F1000_EMBEDDED_CONTROLLER_H
#define __INCLUDE_IQA_F1000_EMBEDDED_CONTROLLER_H

#include <cmn_ipc.h>

//===============================================================================
// Defines for RISC-V embedded controller
//===============================================================================
#define     RISCV_MEMSIZE_CODE              (256 * 1024)    // 256KB Instructions RAM
#define     RISCV_MEMSIZE_DATA              (128 * 1024)    // 128KB Total Data RAM. Available data is (data ram size less mailbox size).

//===============================================================================
// Defines for the Host to RISC-V communication channel
//===============================================================================

#define F1000_RISCV_INT_GEN_REG         0x1a104008    /* Write 0x10000000 */
#define F1000_RISCV_COMM_IRQ_NUM        28            /* IRQ 28 for Host comm */

//===============================================================================
// Structure types
//===============================================================================

//===============================================================================
// Structure to track download status when download data is presented in
// chunks.  If multiple F1000's are programmed at the same time from a common
// set of code, then each F1000 must have a unique instance of this struct 
// passed to the iqa_load_firmware routine.
//===============================================================================
typedef struct iqa_fw_download_state_s
{
  uint32_t    addr;
  uint32_t    len;
  uint32_t    carryover_len;
  char        carryover[8];
} iqa_fw_download_state_t;

//===============================================================================
// Function prototypes
//===============================================================================

//===============================================================================
// Freeze the RISC-V processor.  This basically stops the instruction
// FETCH pipeline without resetting the controller.
//===============================================================================
int iqa_ec_freeze ( int f1000_id, int ep_bitmap );

//===============================================================================
// Unfreeze the RISC-V processor.
//===============================================================================
int iqa_ec_unfreeze ( int f1000_id, int ep_bitmap );

//===============================================================================
// Ping the Embedded Controller (RISC-V).
//
// This will validate the RISC-V is running properly once the firmware has
// been dowloaded.
//===============================================================================
int iqa_ec_ping ( int f1000_id, int ep_bitmap );

//===============================================================================
// Get the revision string of the downloaded RISC-V firmware.
//===============================================================================
int iqa_ec_get_rev ( int f1000_id, int ep_bitmap, char *pRev, int size );

//===============================================================================
// Load the RISC-V firmware and reset the PC to begin execution
// of the newly loaded firmware.
//
// The FW to be loaded will be provided by IQA in binary format.  The
// host must store the binary file (in FLASH, etc.).
//===============================================================================
int iqa_ec_load_firmware ( int f1000_id, int ep_bitmap, char * buf, uint32_t size,
                           iqa_fw_download_state_t *pState );

//===============================================================================
// Load the Merlin Framer / OOB processor code into the processors.
//
// The FW to be loaded will be provided by IQA in binary format.  The
// host must store the binary file (in FLASH, etc.) and provide a pointer
// to that data, along with the size.
//===============================================================================
int iqa_merlin_load_firmware ( int f1000_id, int ep_bitmap, char * buf,
                               uint32_t size, int standalone );

//===============================================================================
// Software reset of OOB Processor
//===============================================================================
int iqa_oob_sw_reset ( int f1000_id, int ep_id );

#endif  // __INCLUDE_IQA_F1000_EMBEDDED_CONTROLLER_H

