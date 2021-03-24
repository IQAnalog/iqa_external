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
// File        : f1000_shadow_reg.h                                             *
// Author      : Tomas Gonzalez                                                 *
// Created     : 03/25/2020                                                     *
//                                                                              *
// Description:  Include header file for F1000 register and bit field access.   *
//                                                                              *
//*******************************************************************************

#ifndef _F1000_SHADOW_REG_H_
#define _F1000_SHADOW_REG_H_

#include "f1000_regs.h"
#include "f1000_mio_intf.h"
#include <iqa_f1000_api.h>
#include <stdbool.h>
#include <stddef.h>
#include <iqa_assert.h>

//===============================================================================
// Defines for runtime register and bit field access
//===============================================================================

//===============================================================================

#define F1000_FIELD(chiprev_, register_set_, register_, field_) \
    f1000_field_desc_get( \
      F1000_REG_ADDR(F1000_REGSET_BASE(chiprev_, register_set_), chiprev_, register_set_, register_), \
      F1000_FIELD_BITPOS(chiprev_, register_set_, register_, field_), \
      F1000_FIELD_BITLEN(chiprev_, register_set_, register_, field_))

#define F1000_FIELD_REG(chiprev_, register_set_, register_) \
    f1000_field_desc_get_reg( \
      F1000_REG_ADDR(F1000_REGSET_BASE(chiprev_, register_set_), chiprev_, register_set_, register_))

//===============================================================================

#define F1000_REGSET_ADDR_COMMON(chiprev_value_, regset_) \
  ((chiprev_value_) == 1 ? \
    F1000_REGSET_ADDR(F1000A, regset_) : \
    F1000_REGSET_ADDR(F1000B, regset_))

#define F1000_FIELD_COMMON_AB(chiprev_value_, regset_, reg_, field_a_, field_b_) \
  ((chiprev_value_) == 1 ? \
    F1000_FIELD(F1000A, regset_, reg_, field_a_) : \
    F1000_FIELD(F1000B, regset_, reg_, field_b_))

#define F1000_FIELD_COMMON(chiprev_value_, regset_, reg_, field_) \
  F1000_FIELD_COMMON_AB(chiprev_value_, regset_, reg_, field_, field_)

#define F1000_FIELD_REG_COMMON_AB(chiprev_value_, regset_, reg_a_, reg_b_) \
  ((chiprev_value_) == 1 ? \
    F1000_FIELD_REG(F1000A, regset_, reg_a_) : \
    F1000_FIELD_REG(F1000B, regset_, reg_b_))

#define F1000_FIELD_REG_COMMON(chiprev_value_, regset_, reg_) \
  F1000_FIELD_REG_COMMON_AB(chiprev_value_, regset_, reg_, reg_)

#define F1000_FIELD_REG_DAC_AB(chiprev_value_, dac_id_, reg_a_, reg_b_) \
  ((dac_id_) == DAC_1 ? \
    F1000_FIELD_REG_COMMON_AB(chiprev_value_, dac1_dig, reg_a_, reg_b_) : \
    F1000_FIELD_REG_COMMON_AB(chiprev_value_, dac2_dig, reg_a_, reg_b_))

#define F1000_FIELD_REG_DAC(chiprev_value_, dac_id_, reg_) \
  F1000_FIELD_REG_DAC_AB(chiprev_value_, dac_id_, reg_, reg_)

#define F1000_FIELD_DAC_AB(chiprev_value_, dac_id_, reg_, field_a_, field_b_) \
  ((dac_id_) == DAC_1 ? \
    F1000_FIELD_COMMON_AB(chiprev_value_, dac1_dig, reg_, field_a_, field_b_) : \
    F1000_FIELD_COMMON_AB(chiprev_value_, dac2_dig, reg_, field_a_, field_b_))

#define F1000_FIELD_DAC(chiprev_value_, dac_id_, reg_, field_) \
  F1000_FIELD_DAC_AB(chiprev_value_, dac_id_, reg_, field_, field_)

#define F1000_FIELD_REG_ADC_AB(chiprev_value_, adc_id_, reg_a_, reg_b_) \
  ((adc_id_) == ADC_1 ? \
    F1000_FIELD_REG_COMMON_AB(chiprev_value_, adc1_top, reg_a_, reg_b_) : \
    F1000_FIELD_REG_COMMON_AB(chiprev_value_, adc2_top, reg_a_, reg_b_))

#define F1000_FIELD_REG_ADC(chiprev_value_, adc_id_, reg_) \
  F1000_FIELD_REG_ADC_AB(chiprev_value_, adc_id_, reg_, reg_)

#define F1000_FIELD_ADC(chiprev_value_, adc_id_, reg_, field_) \
  ((adc_id_) == ADC_1 ? \
    F1000_FIELD_COMMON(chiprev_value_, adc1_top, reg_, field_) : \
    F1000_FIELD_COMMON(chiprev_value_, adc2_top, reg_, field_))

#define F1000_FIELD_ADC_DLL(chiprev_value_, regset_, field_) \
  ((chiprev_value_) == 1 ? \
    F1000_FIELD(F1000A, regset_, adc16_dll, field_) : \
    F1000_FIELD(F1000B, regset_, adc16_dll0, field_))

#define F1000_FIELD_DLL_CTRL_AB(chiprev_value_, regset_, field_a_, field_b_) \
  ((chiprev_value_) == 1 ? \
    F1000_FIELD(F1000A, regset_, dll_ctrl, field_a_) : \
    F1000_FIELD(F1000B, regset_, dll_ctrl0, field_b_))

#define F1000_FIELD_DLL_CTRL(chiprev_value_, regset_, field_) \
  F1000_FIELD_DLL_CTRL_AB(chiprev_value_, regset_, field_, field_)

//===============================================================================

#define F1000_FIELD_ADDR(reg_addr_, field_) \
    f1000_field_desc_get( \
      reg_addr_, \
      (field_).bitpos, \
      (field_).bitlen)

//===============================================================================
// Structures and typedefs
//===============================================================================

/**
 * \brief The f1000_field_desc_t structure contains a "descriptor" which uniquely
 *        identifies a bit field within an F1000 register.
 *
 * In addition to the bit fields defined in the f1000a_regset and f1000b_regset,
 * headers, the field descriptor can be initialized to represent any contiguous
 * set of bits within an F1000 register, (ie. up to 32-bits) so occasionally,
 * the f1000_field_desc_t is used to work with the entire register.
 * (@see F1000_FIELD_REG)
 */
typedef struct
{
  mpreg_addr_t reg_addr;
  uint8_t bitpos;
  uint8_t bitlen;
} f1000_field_desc_t;

typedef struct
{
  mpreg_addr_t reg_addr_rs2:30;
  mpreg_addr_t shadowed:1;
  mpreg_addr_t ep_a_or_b:1;
  mpreg_value_t value;
  mpreg_value_t change_mask;
} f1000_shadow_reg_entry_t;

typedef struct
{
  f1000_shadow_reg_entry_t *p_entries;
  uint_t entry_count;
  uint_t entry_max;
} f1000_shadow_reg_t;

/**
 * \brief The f1000_field_t structure contains a reference to a single field descriptor.
 *        @see f1000_field_desc_t
 *
 */
typedef struct
{
    f1000_field_desc_t field_desc;
    uint32_t value;
    f1000_mio_t *p_mio;
} f1000_field_t;

typedef struct
{
  mpreg_addr_t addr_base;
  uint8_t idx;
  uint8_t addr_count:7;
  uint8_t use_bit_vector:1;
  mpreg_value_t *p_reg_base;
  mpreg_value_t *p_mask_base;
  union
  {
    //MP_BIT_VECTOR_TYPE(bit_vector) *p_shadowed_regs;
    bool shadowed;
  } u;
} f1000_reg_rmw_txn_t;

//===============================================================================
// Function prototypes
//===============================================================================

f1000_field_desc_t f1000_field_desc_get(mpreg_addr_t reg_addr, uint8_t bitpos, uint8_t bitlen);

f1000_field_desc_t f1000_field_desc_get_reg(mpreg_addr_t reg_addr);

//===============================================================================

/**
 * \brief                  Get an f1000_field_desc_t context for a custom bit field
 *
 * The API returns an initialized f1000_field_desc_t built from a specified
 * sub bit field within the specified field_desc. This "sub field" is useful
 * for modifying a bit field within the specified field_desc. Some F1000 fields
 * have sub fields that are not formally named in register set definintions
 * header. The user may access fields using f1000_subfield_desc_t for read or
 * write.
 *
 * \param field_desc:      The field descriptor of the parent field
 * \param subfield_bitpos: The bit position within field_desc of the new subfield
 * \param subfield_bitlen: The bit length of the new subfield
 *
 * \returns                A f1000_field_desc_t for use in f1000_shadow_reg_ APIs
 *
 */

f1000_field_desc_t f1000_field_desc_get_sub(f1000_field_desc_t field_desc, uint8_t subfield_bitpos, uint8_t subfield_bitlen);

//===============================================================================

/**
 * \brief              Initialize an f1000_field_t context
 *
 * A f1000_field_t structure hold the value of a single field (or register)
 * as described by field_desc and provides simple read/write access.
 *
 * NOTE: f1000_field_get can be used immediately after this call as init
 * performs a read against the p_mio.
 *
 * NOTE: All F1000 I/O is performed at a register level. Bit field (field
 * descriptor) modifications are always performed with a read-modify-write
 * (rmw) transaction to ensure only the field's bits are effected.
 *
 * \param p_field:     The uninitialized f1000_field_t to be initialized
 * \param p_mio:       The initialized memory I/O structure that will
 *                     used to handle read/write operations
 * \param field_desc:  The field descriptior of the field to be read
 *
 * \returns            A result status of IQA_SUCCESS on successful read or
 *                     error code in the event memory I/O failed.
 *
 */

int f1000_field_init(f1000_field_t *p_field, f1000_mio_t *p_mio, const f1000_field_desc_t field_desc);

/**
 * \brief              Get the value of a field
 *
 * Use this API to obtain the bit field value from the f1000_field_t.
 *
 * \param p_field:     An initialized f1000_field_t holding the register value
 *
 * \returns            The field value
 *
 */

uint32_t f1000_field_get(f1000_field_t *p_field);

/**
 * \brief              Read the register associated with the f1000_field_t's
 *                     field descriptor.
 *
 * Use this API to re-read field value using the p_mio
 *
 * NOTE: All F1000 I/O is performed at a register level. Bit field (field
 * descriptor) modifications are always performed with a read-modify-write
 * (rmw) transaction to ensure only the field's bits are effected. All reads
 * and writes (including f1000_field_read) operate on registers.
 *
 * \param p_field:     An initialized f1000_field_t holding the register value
 *
 * \returns            A result status of IQA_SUCCESS on successful read or
 *                     error code in the event memory I/O failed.
 *
 */

int f1000_field_read(f1000_field_t *p_field);

/**
 * \brief              Write the shadowed register value to hardware (via mio)
 *
 * Since all F1000 I/O is performed at a register level. Bit field (field
 * descriptor) modifications are always performed on the shadow copy stored in the
 * f1000_field_t, using a read-modify-write (rmw) transaction to ensure only the
 * field's bits are effected. All reads and writes (including f1000_field_write)
 * operate on registers.
 *
 * \param p_field:     An initialized f1000_field_t holding the register value
 * \param value:       The new value for the field.
 *
 * \returns            A result status of IQA_SUCCESS on success or
 *                     error code in the event memory I/O failed.
 *
 */

int f1000_field_write(f1000_field_t *p_field, uint32_t value);

//===============================================================================

/**
 * \brief              Initialize an f1000_shadow_reg_t context
 *
 * A f1000_shadow_reg_t structure shadows registers to the F1000 to
 * simplify register set, register, and bit field read, modify, and
 * write access.
 *
 * \param p_set:       The uninitialized f1000_shadow_reg_t to be initialized
 * \param p_entries:   An array of f1000_shadow_reg_entry_t items to be used
 *                     in shadowing register and field writes to the p_set.
 * \param entry_count: The count of f1000_shadow_reg_entry_t items in p_entries.
 *                     (ie. the maximum number of shadowed registers stored by
 *                      the p_set before a flush is required)
 */

void f1000_shadow_reg_init(f1000_shadow_reg_t *p_set, f1000_shadow_reg_entry_t* p_entries, uint_t entry_count);

/**
 * \brief              Modify (set) a field value
 *
 * The field_desc parameter specified a bitfield (or register) by bit
 * position and bit length. Upon flush to hardware, the p_set will only
 * effect the bits associated with this field. The p_set may safely modify
 * fields before reading the registers as it will track the shadowed state
 * of each register and perform a read/modify/write transaction (rmw) upon
 * flush.
 *
 * \param p_set:       The initialized f1000_shadow_reg_t to be used
 * \param field_desc:  The field descriptior of the field to be changed
 * \param value:       The new value for the field.
 *
 */

void f1000_shadow_reg_set(f1000_shadow_reg_t *p_set, const f1000_field_desc_t field_desc, mpreg_value_t value);

/**
 * \brief              Invalidate a specified field's shadow state
 *
 * This causes the p_set to clear the shadow bit from a register that
 * is in the cache. A subsequent f1000_shadow_reg_flush will read the
 * register from hardware before performing any write.
 *
 * This is useful if a register changed outside of the use of the
 * f1000_shadow_reg_ API to ensure that a stale shadowed regsiter value
 * isn't written to the hardware.
 *
 * \param p_set:       The initialized f1000_shadow_reg_t to be used
 * \param field_desc:  The field descriptior containing the reg_addr of the
 *                     shadow entry to invalidate.
 *
 */

void f1000_shadow_reg_inv(f1000_shadow_reg_t *p_set, const f1000_field_desc_t field_desc);

/**
 * \brief              Read/modify/write a field value
 *
 * This API is equivelent to a f1000_shadow_reg_inv followed by a
 * f1000_shadow_reg_set. In other words, it forces an invalidation of
 * any shadowed regsiter entry and forces the p_set to read back the
 * contents of the register from hardware before writing out the field
 * modification.
 *
 * \param p_set:       The initialized f1000_shadow_reg_t to be used
 * \param field_desc:  The field descriptior of the field to be changed
 * \param value:       The new value for the field.
 *
 */

void f1000_shadow_reg_rmw(f1000_shadow_reg_t *p_set, const f1000_field_desc_t field_desc, mpreg_value_t value);

/**
 * \brief              Flush changes to hardware (via mio)
 *
 * This is the primary write function to effect changes to F1000
 * registers. Only registers altered via f1000_shadow_reg_set or
 * f1000_shadow_reg_rmw are written. The p_set keeps old entries
 * active indefintely to avoid unnecessary read back of recently
 * read register values.
 *
 * This means care must be taken when using f1000_shadow_reg_t with
 * registers that are used for communicating status from the F1000
 * to a host processor. (ie. registers that are not appropriate for
 * shadowing)
 *
 * \param p_set:       The initialized f1000_shadow_reg_t to be used
 * \param p_mio:       The initialized memory I/O structure that will
 *                     used to invoke read/modify/write sequeneces on
 *                     the dirty shadow entries within p_set
 *
 * \returns            A result status of IQA_SUCCESS on success or
 *                     error code in the event memory I/O failed.
 *
 */

int f1000_shadow_reg_flush(f1000_shadow_reg_t *p_set, f1000_mio_t *p_mio);

//===============================================================================

static inline mpreg_addr_t f1000_get_dac_regset_addr(uint8_t chip_rev, int dac_id)
{
  return dac_id == DAC_1 ?
      F1000_REGSET_ADDR_COMMON(chip_rev, dac1_dig) :
      F1000_REGSET_ADDR_COMMON(chip_rev, dac2_dig);
}

static inline mpreg_addr_t f1000_get_adc_regset_addr(uint8_t chip_rev, int adc_id)
{
  return adc_id == ADC_1 ?
      F1000_REGSET_ADDR_COMMON(chip_rev, adc1_top) :
      F1000_REGSET_ADDR_COMMON(chip_rev, adc2_top);
}

static inline f1000_field_desc_t f1000_get_ldo_ctrl_reg(uint8_t chip_rev, int dac_id, int ldo_id)
{
  switch (ldo_id)
  {
    case 0:
      return F1000_FIELD_REG_DAC(chip_rev, dac_id, ldo0_ctrl);
    case 1:
    default:
      return F1000_FIELD_REG_DAC(chip_rev, dac_id, ldo1_ctrl);
  }
}

//===============================================================================

#endif // _F1000_SHADOW_REG_H_

// vim: sw=2 ts=2 et cindent
