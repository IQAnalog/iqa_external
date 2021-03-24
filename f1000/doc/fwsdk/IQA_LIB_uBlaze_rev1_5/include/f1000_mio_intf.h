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
// File        : f1000_mio_intf.h                                               *
// Author      : Courtney Cavin                                                 *
// Created     : 03/26/2020                                                     *
//                                                                              *
// Description:  Include header file for F1000 mio interface implementations.   *
//                                                                              *
//*******************************************************************************

#ifndef _F1000_MIO_INTF_H_
#define _F1000_MIO_INTF_H_

#include "f1000_mio.h"

#if defined(CONFIG_LIB_IQA_RISCV_IPC)
//===============================================================================
// Direct MMIO interface
//===============================================================================

typedef struct
{
  f1000_mio_t mio;
} f1000_mio_direct_t;

f1000_mio_t *f1000_mio_direct_init(f1000_mio_direct_t *p_direct);
#endif // defined(CONFIG_LIB_IQA_RISCV_IPC)

#if defined(CONFIG_LIB_IQA_HOST_IPC)
//===============================================================================
// SPI interface
//===============================================================================
typedef struct
{
  f1000_mio_t mio;
} f1000_mio_spi_t;

f1000_mio_t *f1000_mio_spi_init(f1000_mio_spi_t *p_spi, int f1000_id, int ep_id);

//===============================================================================
// Host IQA interface
//===============================================================================
typedef struct
{
  f1000_mio_t mio;
} f1000_mio_host_t;

f1000_mio_t *f1000_mio_host_init(f1000_mio_host_t *p_host, int f1000_id, int ep_id);
#endif // defined(CONFIG_LIB_IQA_HOST_IPC)

#if defined(CONFIG_LIB_IQA_TRACE_HOST_MIO)
//===============================================================================
// Trace interface
//===============================================================================
typedef struct
{
  f1000_mio_t mio;
  f1000_mio_t *p_mio;
} f1000_mio_trace_t;

f1000_mio_t *f1000_mio_trace_init(f1000_mio_trace_t *p_trace, f1000_mio_t *p_mio);
#endif // defined(CONFIG_LIB_IQA_TRACE_HOST_MIO)

#endif // _F1000_MIO_INTF_H_

// vim: sw=2 ts=2 et cindent
