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
// File        : f1000_mio.h                                                    *
// Author      : Courtney Cavin                                                 *
// Created     : 03/26/2020                                                     *
//                                                                              *
// Description:  Include header file for F1000 mio interface.                   *
//                                                                              *
//*******************************************************************************

#ifndef _F1000_MIO_H_
#define _F1000_MIO_H_

#include "f1000_regs.h"

//===============================================================================
// Defines for mio interface
//===============================================================================

typedef struct f1000_mio f1000_mio_t;

typedef struct f1000_mio_vptbl
{
  int (*read) (f1000_mio_t *p_mio, mpreg_addr_t addr, mpreg_value_t *p_base, unsigned int count);
  int (*write)(f1000_mio_t *p_mio, mpreg_addr_t addr, const mpreg_value_t *p_base, unsigned int count);
  int (*rmw)  (f1000_mio_t *p_mio, mpreg_addr_t addr, mpreg_value_t old_value, mpbitfield_t *p_bitfield);
} f1000_mio_vptbl_t;

struct f1000_mio
{
  const f1000_mio_vptbl_t *p_vptbl;
  uint8_t chip_rev;
  uint8_t f1000_id;
  uint8_t ep_id;
};

static inline void f1000_mio_init(f1000_mio_t *p_mio,
    uint8_t chip_rev, int f1000_id, int ep_id, const f1000_mio_vptbl_t *p_vptbl)
{
  p_mio->p_vptbl = p_vptbl;
  p_mio->chip_rev = chip_rev;
  p_mio->f1000_id = f1000_id;
  p_mio->ep_id = ep_id;
}

//===============================================================================
// Function prototypes
//===============================================================================

static inline int f1000_mio_read(f1000_mio_t *p_mio, mpreg_addr_t addr,
   mpreg_value_t *p_base, unsigned int count)
{
   return p_mio->p_vptbl->read(p_mio, addr, p_base, count);
}

static inline int f1000_mio_write(f1000_mio_t *p_mio, mpreg_addr_t addr,
   const mpreg_value_t *p_base, unsigned int count)
{
   return p_mio->p_vptbl->write(p_mio, addr, p_base, count);
}

int f1000_mio_rmw(f1000_mio_t *p_mio, mpreg_addr_t addr,
    mpreg_value_t old_value, mpbitfield_t *p_bitfield);

static inline int f1000_mio_reg_read(f1000_mio_t *p_mio, mpreg_addr_t addr,
    mpreg_value_t *p_value)
{
  return f1000_mio_read(p_mio, addr, p_value, 1);
}

static inline int f1000_mio_reg_write(f1000_mio_t *p_mio, mpreg_addr_t addr,
    mpreg_value_t value)
{
  return f1000_mio_write(p_mio, addr, &value, 1);
}

int f1000_mio_reg_rmw(f1000_mio_t *p_mio, mpreg_addr_t addr,
    mpreg_value_t mask, mpreg_value_t value);

int f1000_mio_field_read(f1000_mio_t *p_mio, mpreg_addr_t addr,
    uint8_t bitpos, uint8_t bitlen, mpreg_value_t *p_value);

#define F1000_MIO_REG_ADDR(p_mio_, regset_, reg_) \
  F1000_REV_REG_ADDR((p_mio_)->chip_rev, regset_, reg_)

#define F1000_MIO_FIELD_BITPOS(p_mio_, regset_, reg_, field_) \
  F1000_REV_FIELD_BITPOS((p_mio_)->chip_rev, regset_, reg_, field_)

#define F1000_MIO_FIELD_BITLEN(p_mio_, regset_, reg_, field_) \
  F1000_REV_FIELD_BITLEN((p_mio_)->chip_rev, regset_, reg_, field_)

#define F1000_MIO_FIELD_BITMASK(p_mio_, regset_, reg_, field_) \
  F1000_REV_FIELD_BITMASK((p_mio_)->chip_rev, regset_, reg_, field_)

#define F1000_MIO_FIELD_RMW(p_mio_, regset_, reg_, field_, value_) \
  f1000_mio_reg_rmw(p_mio_, \
    F1000_MIO_REG_ADDR(p_mio_, regset_, reg_), \
    F1000_MIO_FIELD_BITMASK(p_mio_, regset_, reg_, field_), \
    ((value_) << F1000_MIO_FIELD_BITPOS(p_mio_, regset_, reg_, field_)))

#define F1000_MIO_FIELD_READ(p_mio_, regset_, reg_, field_, p_value_) \
  f1000_mio_field_read(p_mio_, \
    F1000_MIO_REG_ADDR(p_mio_, regset_, reg_), \
    F1000_MIO_FIELD_BITPOS(p_mio_, regset_, reg_, field_), \
    F1000_MIO_FIELD_BITLEN(p_mio_, regset_, reg_, field_), \
    p_value_)

#define F1000_MIO_REG_READ(p_mio_, regset_, reg_, p_value_) \
  f1000_mio_reg_read(p_mio_, \
    F1000_MIO_REG_ADDR(p_mio_, regset_, reg_), \
    p_value_, 1)

#define F1000_MIO_REG_WRITE(p_mio_, regset_, reg_, value_) \
  f1000_mio_reg_write(p_mio_, \
    F1000_MIO_REG_ADDR(p_mio_, regset_, reg_), \
    value_)

#endif // _F1000_MIO_H_

// vim: sw=2 ts=2 et
