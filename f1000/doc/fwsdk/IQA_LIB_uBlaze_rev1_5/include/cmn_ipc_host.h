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

#ifndef __INCLUDE_F1000_IPC_HOST_H
#define __INCLUDE_F1000_IPC_HOST_H

#if defined(CONFIG_LIB_IQA_HOST_IPC)

#include "cmn_ipc_internal.h"

//===============================================================================
// Defines for the Host to RISC-V communication channel
//===============================================================================

//===============================================================================
// Structure types
//===============================================================================

typedef struct cmn_ipc_host_s cmn_ipc_host_t;

typedef struct cmn_ipc_host_req_s
{
  cmn_ipc_host_t *pIpc;
  cmn_ipc_reg_mask_t valid_regs;
  cmn_ipc_cmd_id_enum_t id;
  //uint32_t pReq[4];
  //uint32_t pResp[4];
} cmn_ipc_host_req_t;

typedef unsigned int cmn_ipc_req_id_t;

struct cmn_ipc_host_s
{
  f1000_ctx_t *pCtx;
  int ep_id;
  cmn_ipc_host_req_t requests[1];
};

//===============================================================================
// Function prototypes
//===============================================================================

// \brief Obtain the IPC instance for the specified F1000 and Element Processor
cmn_ipc_host_t* cmn_get_host_ipc ( int f1000_id, int ep_id );

// \brief Initialize pIpc from pCtx and ep_id
void cmn_ipc_host_init ( cmn_ipc_host_t *pIpc, f1000_ctx_t *pCtx, int ep_id );

// \brief Allocate a request object/envelope
cmn_ipc_host_req_t * cmn_ipc_host_req_alloc ( cmn_ipc_host_t *pIpc );

// \brief Release an allocated request object/envelope
void cmn_ipc_host_req_free ( cmn_ipc_host_req_t *pReq );

// \brief Host register read
uint32_t cmn_ipc_host_req_read_reg ( cmn_ipc_host_req_t *pReq, cmn_ipc_reg_enum_t reg_id );

// \brief Host register read
uint32_t cmn_ipc_host_read_reg ( cmn_ipc_host_t *pIpc, cmn_ipc_reg_enum_t reg_id );

// \brief Host register write
int cmn_ipc_host_write_reg ( cmn_ipc_host_req_t *pReq, cmn_ipc_reg_enum_t reg_id, uint32_t value);

// \brief Format request
static inline void cmn_ipc_host_req0 ( cmn_ipc_host_req_t *pReq, cmn_ipc_cmd_id_enum_t id )
{
  pReq->id = id;
}

// \brief Issue a request to the ep
int cmn_ipc_host_req ( cmn_ipc_host_req_t *pReq );

// \brief Extract response
static inline uint32_t cmn_ipc_host_resp0 ( cmn_ipc_host_req_t *pReq )
{
  return cmn_ipc_host_req_read_reg ( pReq, IPC_REG_REP0 );
}

// \brief Extract response
static inline uint32_t cmn_ipc_host_resp1 ( cmn_ipc_host_req_t *pReq )
{
  return cmn_ipc_host_req_read_reg ( pReq, IPC_REG_REP1 );
}

// \brief Extract response
static inline uint32_t cmn_ipc_host_resp2 ( cmn_ipc_host_req_t *pReq )
{
  return cmn_ipc_host_req_read_reg ( pReq, IPC_REG_REP2 );
}

#endif // defined(CONFIG_LIB_IQA_HOST_IPC)

#endif  // __INCLUDE_F1000_IPC_HOST_H

// vim: sw=2 ts=2 et cindent
