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
//                                                                              *
// This file is released with "Government Purpose Rights" as defined            *
// in DFARS SUBPART 227.71, clause 252.227-7013.                                *
//                                                                              *
//*******************************************************************************

// Generated by RMM v3.1.4
// IQ-Analog Corp. 2013-2018.

#ifndef __F1000_EP_CORE_A_REGS__
#define __F1000_EP_CORE_A_REGS__

#ifndef __CHIP_ID__
#define __CHIP_ID__
#define CHIP_ID(x) (x)->chip_id_register.chip_id_u.chip_id
#define f1000_ep_core_a_regs_CHIP_ID(x) (x)->chip_id_register.chip_id_u.bits.chip_id
typedef volatile struct chip_id_s {
	union {
		struct {
			unsigned int chip_id : 32;
		} bits;
		uint32_t chip_id;
	} chip_id_u;
} chip_id_t;
#endif /* __CHIP_ID__ */

#ifndef __CONFIGURATION__
#define __CONFIGURATION__
#define CONFIGURATION(x) (x)->configuration_register.configuration_u.configuration
#define f1000_ep_core_a_regs_CORE_ID(x) (x)->configuration_register.configuration_u.bits.core_id
#define f1000_ep_core_a_regs_FETCH_ENABLE(x) (x)->configuration_register.configuration_u.bits.fetch_enable
#define f1000_ep_core_a_regs_LMFC_CNT_VALUE(x) (x)->configuration_register.configuration_u.bits.lmfc_cnt_value
typedef volatile struct configuration_s {
	union {
		struct {
			unsigned int core_id : 4;
			unsigned int fetch_enable : 1;
			unsigned int lmfc_cnt_value : 9;
		} bits;
		uint32_t configuration;
	} configuration_u;
} configuration_t;
#endif /* __CONFIGURATION__ */

#ifndef __DUMMY_BITS__
#define __DUMMY_BITS__
#define DUMMY_BITS(x) (x)->dummy_bits_register.dummy_bits_u.dummy_bits
#define f1000_ep_core_a_regs_DUMMY_BITS(x) (x)->dummy_bits_register.dummy_bits_u.bits.dummy_bits
typedef volatile struct dummy_bits_s {
	union {
		struct {
			unsigned int dummy_bits : 7;
		} bits;
		uint32_t dummy_bits;
	} dummy_bits_u;
} dummy_bits_t;
#endif /* __DUMMY_BITS__ */

#ifndef __LMFC_IRQ_CONTROL__
#define __LMFC_IRQ_CONTROL__
#define LMFC_IRQ_CONTROL(x) (x)->lmfc_irq_control_register.lmfc_irq_control_u.lmfc_irq_control
#define f1000_ep_core_a_regs_ENABLE_LMFC_COUNT(x) (x)->lmfc_irq_control_register.lmfc_irq_control_u.bits.enable_lmfc_count
#define f1000_ep_core_a_regs_LMFC_IRQ_COMPARE(x) (x)->lmfc_irq_control_register.lmfc_irq_control_u.bits.lmfc_irq_compare
typedef volatile struct lmfc_irq_control_s {
	union {
		struct {
			unsigned int enable_lmfc_count : 1;
			unsigned int lmfc_irq_compare : 16;
		} bits;
		uint32_t lmfc_irq_control;
	} lmfc_irq_control_u;
} lmfc_irq_control_t;
#endif /* __LMFC_IRQ_CONTROL__ */

#ifndef __LMFC_COUNTER__
#define __LMFC_COUNTER__
#define LMFC_COUNTER(x) (x)->lmfc_counter_register.lmfc_counter_u.lmfc_counter
#define f1000_ep_core_a_regs_LMFC_COUNTER(x) (x)->lmfc_counter_register.lmfc_counter_u.bits.lmfc_counter
typedef volatile struct lmfc_counter_s {
	union {
		struct {
			unsigned int lmfc_counter : 16;
		} bits;
		uint32_t lmfc_counter;
	} lmfc_counter_u;
} lmfc_counter_t;
#endif /* __LMFC_COUNTER__ */

#ifndef __LMFC_IRQ_ACK__
#define __LMFC_IRQ_ACK__
#define LMFC_IRQ_ACK(x) (x)->lmfc_irq_ack_register.lmfc_irq_ack_u.lmfc_irq_ack
#define f1000_ep_core_a_regs_LMFC_IRQ_ACK(x) (x)->lmfc_irq_ack_register.lmfc_irq_ack_u.bits.lmfc_irq_ack
typedef volatile struct lmfc_irq_ack_s {
	union {
		struct {
			unsigned int lmfc_irq_ack : 1;
		} bits;
		uint32_t lmfc_irq_ack;
	} lmfc_irq_ack_u;
} lmfc_irq_ack_t;
#endif /* __LMFC_IRQ_ACK__ */

typedef struct f1000_ep_core_a_regs_s
{
	chip_id_t chip_id_register;	// Address: 0x0
	configuration_t configuration_register;	// Address: 0x1
	dummy_bits_t dummy_bits_register;	// Address: 0x2
	lmfc_irq_control_t lmfc_irq_control_register;	// Address: 0x3
	lmfc_counter_t lmfc_counter_register;	// Address: 0x4
	lmfc_irq_ack_t lmfc_irq_ack_register;	// Address: 0x5
} f1000_ep_core_a_regs_t;

#endif /* __F1000_EP_CORE_A_REGS__ */
