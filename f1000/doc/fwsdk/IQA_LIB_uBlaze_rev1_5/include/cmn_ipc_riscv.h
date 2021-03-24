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

#ifndef __INCLUDE_F1000_IPC_RISCV_H
#define __INCLUDE_F1000_IPC_RISCV_H

#if defined(CONFIG_LIB_IQA_RISCV_IPC)

#include "cmn_ipc_internal.h"

//===============================================================================
// Defines for the Host to RISC-V communication channel
//===============================================================================

#define F1000_IPC_DECLARE_FUNC(name_, pReq_) \
  void name_##_func ( cmn_ipc_riscv_req_t *pReq_ )

//===============================================================================
// Structure types
//===============================================================================

typedef enum
{
  IRQ_HOST,
  IRQ_OOB
} cmn_ipc_riscv_irq_enum_t;

typedef struct cmn_ipc_riscv_req_s cmn_ipc_riscv_req_t;

typedef struct cmn_ipc_riscv_s cmn_ipc_riscv_t;

typedef void ( * cmn_ipc_riscv_req_fptr_t ) ( cmn_ipc_riscv_req_t *pReq );

struct cmn_ipc_riscv_req_s
{
  cmn_ipc_riscv_t *pIpc;
  cmn_ipc_riscv_irq_enum_t irq_type;
  cmn_ipc_cmd_t const *pCmd;
  cmn_ipc_riscv_req_fptr_t pfHandler;
  cmn_ipc_riscv_req_t *pNext;
};

struct cmn_ipc_riscv_s
{
  f1000_ctx_t *pCtx;
  cmn_ipc_riscv_req_t *pFirst;
  uint8_t requests_alloc;
  cmn_ipc_riscv_req_t requests[2];
};

//===============================================================================
// Function prototypes
//===============================================================================

// \brief Direct register access (RISCV side only)
static inline volatile uint32_t * cmn_get_riscv_reg_ptr ( f1000_ctx_t *pCtx, cmn_ipc_reg_enum_t reg_id )
{
  return (uint32_t*) cmn_get_reg_addr ( pCtx, reg_id );
}

// \brief Initialize the IPC structure
void cmn_ipc_riscv_init ( cmn_ipc_riscv_t *pIpc, f1000_ctx_t *pCtx );

// \brief Allocate a request object/envelope from an ec generated interrupt
bool cmn_ipc_riscv_poll ( cmn_ipc_riscv_t *pIpc, cmn_ipc_riscv_irq_enum_t irq_type );

cmn_ipc_riscv_req_t * cmn_ipc_riscv_pop ( cmn_ipc_riscv_t *pIpc );

void cmn_ipc_riscv_dispatch ( cmn_ipc_riscv_req_t* pReq );

void cmn_ipc_riscv_req_free ( cmn_ipc_riscv_req_t *pReq );

// \brief Extract request arg
static inline uint32_t cmn_ipc_riscv_req1 ( cmn_ipc_riscv_req_t *pReq )
{
  return *cmn_get_riscv_reg_ptr ( pReq->pIpc->pCtx, IPC_REG_ARG1 );
}

// \brief Set response
void cmn_ipc_riscv_resp ( cmn_ipc_riscv_req_t *pReq, cmn_ipc_reg_enum_t reg_id, uint32_t value );

// \brief Set status response
static inline void cmn_ipc_riscv_resp0 ( cmn_ipc_riscv_req_t *pReq, uint32_t value )
{
  cmn_ipc_riscv_resp ( pReq, IPC_REG_REP0, value );
}

// \brief Set arg 1 response
static inline void cmn_ipc_riscv_resp1 ( cmn_ipc_riscv_req_t *pReq, uint32_t value )
{
  cmn_ipc_riscv_resp ( pReq, IPC_REG_REP1, value );
}

#endif // defined(CONFIG_LIB_IQA_RISCV_IPC)

#endif  // __INCLUDE_IQA_IPC_RISCV_H

// vim: sw=2 ts=2 et cindent
