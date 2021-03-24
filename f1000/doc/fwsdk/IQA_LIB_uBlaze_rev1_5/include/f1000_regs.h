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
// File        : f1000_regs.h                                                   *
// Author      : Tomas Gonzalez                                                 *
// Created     : 02/10/2020                                                     *
//                                                                              *
// Description:  Include header file for F1000 register and bit field access.   *
//                                                                              *
//*******************************************************************************

#ifndef _F1000_REGS_H_
#define _F1000_REGS_H_

#include <stdint.h>
#include <string.h>
#include "f1000a_regset.h"
#include "f1000b_regset.h"

//===============================================================================
// Defines for common pre-processor conditionals
//===============================================================================

#ifdef offsetof
#define MP_OFFSET_OF(type, member) \
  offsetof(type, member)
#else /* offsetof */
#define MP_OFFSET_OF(type, member) \
  ((uintptr_t)(&((const type*)0)->member))
#endif /* offsetof */

#define MP_CONTAINER_OF(ptr, type, member) \
  ((type*) (void*) (((uint8_t*)(ptr)) - MP_OFFSET_OF(type, member)))

#define MP_CONCAT(a_, b_)        a_##b_
#define MP_PROBE(x_)             x_, 1,
#define MP_CHECK_N(x_, n_, ...)  n_
#define MP_CHECK(...)            MP_CHECK_N(__VA_ARGS__, 0,)
#define MP_IIF_0(t_, e_)         e_
#define MP_IIF_1(t_, e_)         t_
#define MP_IIF(c_)               MP_CONCAT(MP_IIF_, c_)
#define MP_BOOL_1                MP_PROBE(_)
#define MP_NOT_0                 MP_PROBE(_)

#define MP_BOOL(x_) \
  MP_CHECK(MP_CONCAT(MP_BOOL_, x_))

#define MP_NOT(x_) \
  MP_CHECK(MP_CONCAT(MP_NOT_, x_))

#define MP_IF(c_) \
  MP_IIF(MP_BOOL(c_))

#define MP_OR(e1_, e2_) \
  MP_IF(e1_)(1, MP_IF(e2_)(1, 0))

#define MP_AND_NOT(e1_, e2_) \
  MP_IF(e1_)(MP_IF(e2_)(0, 1), 0)

#define MP_ARRAY_SIZE(a) \
  ((size_t)(sizeof(a) / sizeof(*(a))))

#define MP_DECL_INLINE(rettype_, prefix_, name_) \
  static inline rettype_ MP_T(prefix_, name_)

#define MP_DECL(rettype_, prefix_, name_) \
  rettype_ MP_T(prefix_, name_)

#define MP_T(prefix_, name_) \
  prefix_##_##name_

//===============================================================================
// Defines for common bit field manipulation
//===============================================================================

#define BITS_PER_BYTE 8

#define MP_BIT_MASK(bitlen_) \
  (((1ULL << ((bitlen_) - 1U)) << 1U) - 1U)

#define MP_BIT_FIELD_BITMASK(bitpos_, bitlen_) \
  (MP_BIT_MASK(bitlen_) << (bitpos_))

#define MP_BIT_FIELD(value_, bitlen_) \
  ((value_) & MP_BIT_MASK(bitlen_))

#define MP_BIT_GET_FIELD(value_, bitpos_, bitlen_) \
  (MP_BIT_FIELD((value_) >> (bitpos_), bitlen_))

#define MP_BIT_CLEAR_FIELD(value_, bitpos_, bitlen_) \
  ((value_) & ~MP_BIT_FIELD_BITMASK(bitpos_, bitlen_))

#define MP_BIT_OR_FIELD(cleared_value_, bitpos_, bitlen_, field_value_) \
  ((cleared_value_) | ((field_value_) & MP_BIT_MASK(bitlen_)) << (bitpos_))

#define MP_BIT_SET_FIELD(value_, bitpos_, bitlen_, field_value_) \
  MP_BIT_OR_FIELD( \
    MP_BIT_CLEAR_FIELD(value_, bitpos_, bitlen_), \
    bitpos_, \
    bitlen_, \
    field_value_)

#define MP_BIT_VECTOR_DECLARE(prefix_, idx_type_, elem_type_, count_) \
  typedef struct \
  { \
    elem_type_ elems[count_]; \
  } MP_BIT_VECTOR_TYPE(prefix_); \
  MP_DECL_INLINE(void, prefix_, init) (MP_BIT_VECTOR_TYPE(prefix_) *p_vec) \
  { \
    memset(p_vec, 0, sizeof(*p_vec)); \
  } \
  MP_DECL_INLINE(elem_type_, prefix_, get) (MP_BIT_VECTOR_TYPE(prefix_) *p_vec, idx_type_ idx) \
  { \
    idx_type_ elem_idx = idx / (BITS_PER_BYTE * sizeof(elem_type_)); \
    idx_type_ bitpos = idx % (BITS_PER_BYTE * sizeof(elem_type_)); \
    IQA_ASSERT(elem_idx < MP_ARRAY_SIZE(p_vec->elems)); \
    return MP_BIT_GET_FIELD(p_vec->elems[elem_idx], bitpos, 1); \
  } \
  MP_DECL_INLINE(void, prefix_, set) (MP_BIT_VECTOR_TYPE(prefix_) *p_vec, idx_type_ idx, elem_type_ value) \
  { \
    idx_type_ elem_idx = idx / (BITS_PER_BYTE * sizeof(elem_type_)); \
    idx_type_ bitpos = idx % (BITS_PER_BYTE * sizeof(elem_type_)); \
    IQA_ASSERT(elem_idx < MP_ARRAY_SIZE(p_vec->elems)); \
    p_vec->elems[elem_idx] = MP_BIT_SET_FIELD( \
        p_vec->elems[elem_idx], \
        bitpos, \
        1, \
        value); \
  } \
  typedef void prefix_##_dummy_t

#define MP_BIT_VECTOR_TYPE(prefix_) \
  mpbitvector_##prefix_##_t

#define MP_BIT_VECTOR_COUNT(prefix_) \
  (sizeof(((MP_BIT_VECTOR_TYPE(prefix_)*) 0)->elems) * BITS_PER_BYTE)

//===============================================================================
// Structures and typedefs
//===============================================================================

typedef unsigned int uint_t;

typedef uint32_t mpreg_addr_t;

typedef uint32_t mpreg_value_t;

typedef struct
{
  mpreg_value_t value;
  mpreg_value_t change_mask;
} mpbitfield_t;

//===============================================================================
// Defines for compile-time register and bit field access
//===============================================================================

#define F1000_BASE 0x40000000
#define FPGA_BASE  0x20000000

#define F1000_REGSET_VALID(chiprev_, register_set_) \
  chiprev_##_##register_set_##_VALID

#define F1000_REGSET_MAP(chiprev_, entry_, ctx_) \
  chiprev_##_REGSET_MAP(entry_, ctx_)

#define F1000_REGSET_BASE(chiprev_, register_set_) \
  chiprev_##_##register_set_##_BASE

#define F1000_REGSET_COUNT(chiprev_, register_set_) \
  chiprev_##_##register_set_##_COUNT

#define F1000_REGSET_ADDR(chiprev_, register_set_) \
  chiprev_##_##register_set_##_OFFSET(F1000_REGSET_BASE(chiprev_, register_set_))

#define F1000_REGSET_OFFSET(f1000_base_, chiprev_, register_set_) \
  chiprev_##_##register_set_##_OFFSET(f1000_base_)

#define F1000_REGSET_REG_OFFSET(register_set_offset_, chiprev_, register_set_, register_) \
  chiprev_##_##register_set_##_##register_##_OFFSET(register_set_offset_)

#define F1000_REGSET_REG_PTR(register_set_offset_, chiprev_, register_set_, register_) \
  ((uint32_t *) F1000_REGSET_REG_OFFSET(register_set_offset_, chiprev_, register_set_, register_))

#define F1000_REG_VALID(chiprev_, register_set_, register_) \
  chiprev_##_##register_set_##_##register_##_VALID

#define F1000_REG_MAP(chiprev_, register_set_, entry_, ctx_) \
  chiprev_##_##register_set_##_REG_MAP(entry_, ctx_)

#define F1000_REG_INDEX(chiprev_, register_set_, register_) \
  chiprev_##_##register_set_##_##register_##_INDEX

#define F1000_REG_ADDR(f1000_base_, chiprev_, register_set_, register_) \
  F1000_REGSET_REG_OFFSET( \
    F1000_REGSET_OFFSET(f1000_base_, chiprev_, register_set_), \
    chiprev_, register_set_, register_)

#define F1000_REG_PTR(f1000_base_, chiprev_, register_set_, register_) \
  ((volatile uint32_t *) F1000_REG_ADDR(f1000_base_, chiprev_, register_set_, register_))

#define F1000_REG_NV_PTR(register_set_base_, chiprev_, register_set_, register_) \
  ((uint32_t *) F1000_REG_ADDR(register_set_base_, chiprev_, register_set_, register_))

#define F1000_FIELD_VALID(chiprev_, register_set_, register_, field_) \
  chiprev_##_##register_set_##_##register_##_##field_##_VALID

#define F1000_FIELD_MAP(chiprev_, register_set_, register_, entry_, ctx_) \
  chiprev_##_##register_set_##_##register_##_FIELD_MAP(entry_, ctx_)

#define F1000_FIELD_BITPOS(chiprev_, register_set_, register_, field_) \
  chiprev_##_##register_set_##_##register_##_##field_##_BITPOS

#define F1000_FIELD_BITLEN(chiprev_, register_set_, register_, field_) \
  chiprev_##_##register_set_##_##register_##_##field_##_BITLEN

#define F1000_FIELD_BITMASK(...) \
  MP_BIT_FIELD_BITMASK(F1000_FIELD_BITPOS(__VA_ARGS__), F1000_FIELD_BITLEN(__VA_ARGS__))

#define F1000_FIELD_GET(p_register_, chiprev_, register_set_, register_, field_) \
  MP_BIT_GET_FIELD( \
    *(p_register_), \
    F1000_FIELD_BITPOS(chiprev_, register_set_, register_, field_), \
    F1000_FIELD_BITLEN(chiprev_, register_set_, register_, field_))

#define F1000_FIELD_OR(cleared_register_value_, chiprev_, register_set_, register_, field_, field_value_) \
  MP_BIT_OR_FIELD( \
    cleared_register_value_, \
    F1000_FIELD_BITPOS(chiprev_, register_set_, register_, field_), \
    F1000_FIELD_BITLEN(chiprev_, register_set_, register_, field_), \
    field_value_)

#define F1000_FIELD_SET(p_register_, chiprev_, register_set_, register_, field_, field_value_) \
  *(p_register_) = MP_BIT_SET_FIELD( \
    *(p_register_), \
    F1000_FIELD_BITPOS(chiprev_, register_set_, register_, field_), \
    F1000_FIELD_BITLEN(chiprev_, register_set_, register_, field_), \
    field_value_)

//===============================================================================
// Defines for runtime register and bit field access
//===============================================================================

#define F1000_REV_REGSET_OFFSET(rtchiprev_, regset_) \
  ((rtchiprev_) == 1 ? \
    F1000_REGSET_OFFSET(0UL, F1000A, regset_) : \
    F1000_REGSET_OFFSET(0UL, F1000B, regset_))

#define F1000_REV_REG_ADDR(rtchiprev_, regset_, reg_) \
  ((rtchiprev_) == 1 ? \
    F1000_REG_ADDR(F1000_REGSET_BASE(F1000A, regset_), F1000A, regset_, reg_) : \
    F1000_REG_ADDR(F1000_REGSET_BASE(F1000B, regset_), F1000B, regset_, reg_))

#define F1000_REV_FIELD_BITPOS(rtchiprev_, regset_, reg_, field_) \
  ((rtchiprev_) == 1 ? \
    F1000_FIELD_BITPOS(F1000A, regset_, reg_, field_) : \
    F1000_FIELD_BITPOS(F1000B, regset_, reg_, field_))

#define F1000_REV_FIELD_BITLEN(rtchiprev_, regset_, reg_, field_) \
  ((rtchiprev_) == 1 ? \
    F1000_FIELD_BITLEN(F1000A, regset_, reg_, field_) : \
    F1000_FIELD_BITLEN(F1000B, regset_, reg_, field_))

#define F1000_REV_FIELD_BITMASK(rtchiprev_, regset_, reg_, field_) \
  ((rtchiprev_) == 1 ? \
    F1000_FIELD_BITMASK(F1000A, regset_, reg_, field_) : \
    F1000_FIELD_BITMASK(F1000B, regset_, reg_, field_))

//===============================================================================
// Function prototypes
//===============================================================================

#endif // _F1000_REGS_H_

// vim: sw=2 ts=2 et cindent
