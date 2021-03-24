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
// File        : cmn_ipc.h                                                      *
// Author      : TG                                                             *
// Created     : 02/03/2020                                                     *
//                                                                              *
// Description:  Include header file for F1000 IPC with RISC-V element          *
// controllers.                                                                  *
//
// Request (host)  => Response (riscv)
// Host Processor  => Element Controller A (EP_A)
// Host Processor  => Element Controller B (EP_B)
//
// API sequence:
// ec
//    => cmn_ipc_host_req
//       => cmn_ipc_riscv_req[A or B]
//          ec: ep IRQ
//          => cmn_ipc_req.req (registereed callback in ep)
//          ep: ec IRQ
//       ~cmn_ipc_riscv_req
//    ~cmn_ipc_host_req
//
//                                                                              *
//*******************************************************************************

#ifndef __INCLUDE_IQA_IPC_H
#define __INCLUDE_IQA_IPC_H

#include "cmn_ipc_riscv.h"
#include "cmn_ipc_host.h"
#include "cmn_ipc_internal.h"

//===============================================================================
// Defines for the Host to RISC-V communication channel
//===============================================================================

//===============================================================================
// Structure types
//===============================================================================

/**\brief This structure contains context information for the f1000 and IPC
 * state between the other internal RISCV core.
 */

struct f1000_ctx_s
{
  /** This identifies the F1000 chip being accessed in APIs that
   * require the f1000_ctx_t context.
   */
  int f1000_id;

  /** F1000 chip revision
  */
  uint8_t chip_rev;

  /** Argument registers used in IPC communication
  */
  uint32_t const *pArgReg;

#if defined(CONFIG_LIB_IQA_HOST_IPC)
  /** IPC contexts for EP_A and EP_B
  */
  cmn_ipc_host_t ipcCtx[2];
#endif // defined(CONFIG_LIB_IQA_HOST_IPC)
};

//===============================================================================
// Function prototypes
//===============================================================================

// \brief Get (and initialize) the F1000 IPC context
f1000_ctx_t* cmn_get_ctx ( int f1000_id );

// \brief Get the F1000 chipset revision (either 1 or 2)
static inline uint8_t cmn_get_rev ( int f1000_id )
{
  return cmn_get_ctx ( f1000_id )->chip_rev;
}

// \brief Get the physical address of the specified IPC register
static inline uint32_t cmn_get_reg_addr ( f1000_ctx_t *pCtx, cmn_ipc_reg_enum_t reg_id )
{
  IQA_ASSERT(reg_id < IPC_REG_MAX);
  return pCtx->pArgReg[reg_id];
}

#endif  // __INCLUDE_IQA_IPC_H

// vim: sw=2 ts=2 et cindent
