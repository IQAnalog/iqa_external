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

#ifndef __INCLUDE_F1000_IPC_INTERNAL_H
#define __INCLUDE_F1000_IPC_INTERNAL_H

#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <iqa_assert.h>

//===============================================================================
// Defines
//===============================================================================

//===============================================================================
// Defines for the Host to RISC-V communication channel
//===============================================================================

#define F1000_IPC_CMD_TABLE(entry_) \
  /*     enum,                         id,     resp, argument and response registers */ \
  entry_(F1000_IPC_CMD_PING,           1,      2,    VALID_REG(IPC_REG_CMD)) \
  entry_(F1000_IPC_CMD_GET_REV,        2,      1,    VALID_REG(IPC_REG_CMD) | VALID_REG(IPC_REG_REP1)) \
  entry_(F1000_IPC_CMD_GEN_INT,        3,      9,    VALID_REG(IPC_REG_CMD)) \
  entry_(F1000_IPC_CMD_DISABLE_INT,    4,      9,    VALID_REG(IPC_REG_CMD)) \
  entry_(F1000_IPC_CMD_GET_ADC_STATUS, 5,      4,    VALID_REG(IPC_REG_CMD) | VALID_REG(IPC_REG_REP1) | VALID_REG(IPC_REG_REP2)) \
  entry_(F1000_IPC_CMD_RESET_RX,       0x8000, 1,    0) \
  entry_(F1000_IPC_CMD_TERMINATE,      0xf501, 3,    VALID_REG(IPC_REG_CMD))

#define F1000_IPC_DECLARE_CMD_ENUM_ENTRY(name_, id_, resp_, valid_regs_) \
  name_,

#define F1000_IPC_DECLARE_CMD_STRUCT_ENTRY(name_, id_, resp_, valid_regs_) \
{ id_, resp_, valid_regs_ },

#define F1000_IPC_DECLARE_CMD_SWITCH_ENTRY(name_, id_, resp_, valid_regs_) \
  case id_: return name_;

#define F1000_IPC_DECLARE_CMD_FUNC_PROTO_ENTRY(name_, id_, resp_, valid_regs_) \
  extern void name_##_func ( cmn_ipc_riscv_req_t *pReq );

#define F1000_IPC_DECLARE_CMD_FUNC_ENTRY(name_, id_, resp_, valid_regs_) \
  name_##_func,

#define F1000_IPC_DECLARE_CMD_TABLE(table_) \
  typedef struct cmn_ipc_cmd_s \
{ \
  uint32_t command_id; \
  uint32_t resp_id; \
  cmn_ipc_reg_mask_t valid_regs; \
} cmn_ipc_cmd_t; \
typedef enum \
{ \
  F1000_IPC_CMD_NONE = 0, \
  table_(F1000_IPC_DECLARE_CMD_ENUM_ENTRY) \
  F1000_IPC_CMD_MAX \
} cmn_ipc_cmd_id_enum_t; \
cmn_ipc_cmd_t const * cmn_get_cmd_entry(cmn_ipc_cmd_id_enum_t id)

#define F1000_IPC_DEFINE_CMD_TABLE(table_) \
  static cmn_ipc_cmd_t const gp_cmd_table[] = \
{ \
  table_(F1000_IPC_DECLARE_CMD_STRUCT_ENTRY) \
}; \
static cmn_ipc_cmd_id_enum_t cmn_get_cmd_from_command_id(uint32_t command_id) \
{ \
  switch (command_id) \
  { \
    table_(F1000_IPC_DECLARE_CMD_SWITCH_ENTRY) \
  } \
  return F1000_IPC_CMD_NONE; \
} \
cmn_ipc_cmd_t const * cmn_get_cmd_entry(cmn_ipc_cmd_id_enum_t id) \
{ \
  IQA_ASSERT(id > F1000_IPC_CMD_NONE && id < F1000_IPC_CMD_MAX); \
  return &gp_cmd_table[id - F1000_IPC_CMD_NONE - 1]; \
} \
typedef void F1000_IPC_DEFINE_CMD_TABLE_dummy

#define F1000_IPC_DEFINE_REQ_TABLE(table_) \
  table_(F1000_IPC_DECLARE_CMD_FUNC_PROTO_ENTRY) \
  static cmn_ipc_riscv_req_fptr_t const gp_req_table[] = \
{ \
  table_(F1000_IPC_DECLARE_CMD_FUNC_ENTRY) \
}

#define F1000_IPC_CMD(id_) \
  cmn_get_cmd_entry(id_)->command_id

#define F1000_IPC_RESP(id_) \
  cmn_get_cmd_entry(id_)->resp_id

#define F1000_ATTEN_TERMINATE_KEY       0x5a5aF703
#define F1000_RESP_UNKNOWN              100

#define VALID_REG(reg_id_)              (1U << (reg_id_))

//===============================================================================
// Structure types
//===============================================================================

typedef enum
{
  IPC_REG_CMD,
  IPC_REG_ARG1,
  IPC_REG_ARG2,
  IPC_REG_ARG3,
  IPC_REG_ARG4,
  IPC_REG_REP0,
  IPC_REG_REP1,
  IPC_REG_REP2,
  IPC_REG_REP3,
  IPC_REG_REP4,
  IPC_REG_MAX
} cmn_ipc_reg_enum_t;

typedef unsigned int cmn_ipc_reg_mask_t;

typedef struct f1000_ctx_s f1000_ctx_t;

F1000_IPC_DECLARE_CMD_TABLE(F1000_IPC_CMD_TABLE);

//===============================================================================
// Function prototypes
//===============================================================================

// \brief Get the F1000 register address for the specified reg_id
static inline uint32_t cmn_get_reg_addr ( f1000_ctx_t *pCtx, cmn_ipc_reg_enum_t reg_id );

#endif  // __INCLUDE_F1000_IPC_INTERNAL_H

// vim: sw=2 ts=2 et cindent
