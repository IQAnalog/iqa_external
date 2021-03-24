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

#ifndef __MONZA_RX_LINK_REGS__
#define __MONZA_RX_LINK_REGS__

#ifndef __RX_LANE_CGS_STATUS__
#define __RX_LANE_CGS_STATUS__
#define RX_LANE_CGS_STATUS(x) (x)->rx_lane_cgs_status_register.rx_lane_cgs_status_u.rx_lane_cgs_status
#define monza_rx_link_regs_CGS_LOCK(x) (x)->rx_lane_cgs_status_register.rx_lane_cgs_status_u.bits.cgs_lock
#define monza_rx_link_regs_CGS_DATA(x) (x)->rx_lane_cgs_status_register.rx_lane_cgs_status_u.bits.cgs_data
#define monza_rx_link_regs_CGS_FSM_STATE(x) (x)->rx_lane_cgs_status_register.rx_lane_cgs_status_u.bits.cgs_fsm_state
#define monza_rx_link_regs_CGS_K_DET(x) (x)->rx_lane_cgs_status_register.rx_lane_cgs_status_u.bits.cgs_k_det
#define monza_rx_link_regs_DECODED_8B_DATA_COMB(x) (x)->rx_lane_cgs_status_register.rx_lane_cgs_status_u.bits.decoded_8b_data_comb
typedef volatile struct rx_lane_cgs_status_s {
	union {
		struct {
			unsigned int cgs_lock : 1;
			unsigned int cgs_data : 1;
			unsigned int cgs_fsm_state : 4;
			unsigned int cgs_k_det : 1;
			unsigned int decoded_8b_data_comb : 9;
		} bits;
		uint32_t rx_lane_cgs_status;
	} rx_lane_cgs_status_u;
} rx_lane_cgs_status_t;
#endif /* __RX_LANE_CGS_STATUS__ */

#ifndef __RX_LANE_COUNTS__
#define __RX_LANE_COUNTS__
#define RX_LANE_COUNTS(x) (x)->rx_lane_counts_register.rx_lane_counts_u.rx_lane_counts
#define monza_rx_link_regs_K_COUNT(x) (x)->rx_lane_counts_register.rx_lane_counts_u.bits.k_count
#define monza_rx_link_regs_ERR_COUNT(x) (x)->rx_lane_counts_register.rx_lane_counts_u.bits.err_count
#define monza_rx_link_regs_MISALIGN_CNT(x) (x)->rx_lane_counts_register.rx_lane_counts_u.bits.misalign_cnt
#define monza_rx_link_regs_REALIGN_CNT(x) (x)->rx_lane_counts_register.rx_lane_counts_u.bits.realign_cnt
typedef volatile struct rx_lane_counts_s {
	union {
		struct {
			unsigned int k_count : 8;
			unsigned int err_count : 8;
			unsigned int misalign_cnt : 8;
			unsigned int realign_cnt : 8;
		} bits;
		uint32_t rx_lane_counts;
	} rx_lane_counts_u;
} rx_lane_counts_t;
#endif /* __RX_LANE_COUNTS__ */

#ifndef __RX_LANE_CONFIG__
#define __RX_LANE_CONFIG__
#define RX_LANE_CONFIG(x) (x)->rx_lane_config_register.rx_lane_config_u.rx_lane_config
#define monza_rx_link_regs_LANE_LINK(x) (x)->rx_lane_config_register.rx_lane_config_u.bits.lane_link
#define monza_rx_link_regs_LANE_ID(x) (x)->rx_lane_config_register.rx_lane_config_u.bits.lane_id
#define monza_rx_link_regs_TEST_REPEAT_K_EN(x) (x)->rx_lane_config_register.rx_lane_config_u.bits.test_repeat_k_en
#define monza_rx_link_regs_TEST_REPEAT_D21_5_EN(x) (x)->rx_lane_config_register.rx_lane_config_u.bits.test_repeat_d21_5_en
#define monza_rx_link_regs_DISPARITY_REG_IN(x) (x)->rx_lane_config_register.rx_lane_config_u.bits.disparity_reg_in
#define monza_rx_link_regs_EN_DISPARITY_FB(x) (x)->rx_lane_config_register.rx_lane_config_u.bits.en_disparity_fb
#define monza_rx_link_regs_LD_DISPARITY_REG(x) (x)->rx_lane_config_register.rx_lane_config_u.bits.ld_disparity_reg
#define monza_rx_link_regs_ALIGN_REPORT_ONLY(x) (x)->rx_lane_config_register.rx_lane_config_u.bits.align_report_only
typedef volatile struct rx_lane_config_s {
	union {
		struct {
			unsigned int lane_link : 1;
			unsigned int lane_id : 5;
			unsigned int test_repeat_k_en : 1;
			unsigned int test_repeat_d21_5_en : 1;
			unsigned int disparity_reg_in : 1;
			unsigned int en_disparity_fb : 1;
			unsigned int ld_disparity_reg : 1;
			unsigned int align_report_only : 1;
		} bits;
		uint32_t rx_lane_config;
	} rx_lane_config_u;
} rx_lane_config_t;
#endif /* __RX_LANE_CONFIG__ */

#ifndef __RX_TOTAL_ERRORS__
#define __RX_TOTAL_ERRORS__
#define RX_TOTAL_ERRORS(x) (x)->rx_total_errors_register.rx_total_errors_u.rx_total_errors
#define monza_rx_link_regs_LANE_TOTAL_ERRORS(x) (x)->rx_total_errors_register.rx_total_errors_u.bits.lane_total_errors
#define monza_rx_link_regs_SYNC_REQ_CNT(x) (x)->rx_total_errors_register.rx_total_errors_u.bits.sync_req_cnt
typedef volatile struct rx_total_errors_s {
	union {
		struct {
			unsigned int lane_total_errors : 16;
			unsigned int sync_req_cnt : 8;
		} bits;
		uint32_t rx_total_errors;
	} rx_total_errors_u;
} rx_total_errors_t;
#endif /* __RX_TOTAL_ERRORS__ */

#ifndef __RX_ILAS_STATUS__
#define __RX_ILAS_STATUS__
#define RX_ILAS_STATUS(x) (x)->rx_ilas_status_register.rx_ilas_status_u.rx_ilas_status
#define monza_rx_link_regs_ILAS_CFG_LID(x) (x)->rx_ilas_status_register.rx_ilas_status_u.bits.ilas_cfg_lid
#define monza_rx_link_regs_ILAS_CHECKSUM_ERROR(x) (x)->rx_ilas_status_register.rx_ilas_status_u.bits.ilas_checksum_error
#define monza_rx_link_regs_ILAS_FSM_STATE(x) (x)->rx_ilas_status_register.rx_ilas_status_u.bits.ilas_fsm_state
#define monza_rx_link_regs_ILAS_LID_MISMATCH(x) (x)->rx_ilas_status_register.rx_ilas_status_u.bits.ilas_lid_mismatch
#define monza_rx_link_regs_ILAS_ALIGNED(x) (x)->rx_ilas_status_register.rx_ilas_status_u.bits.ilas_aligned
#define monza_rx_link_regs_DISPARITY_REG(x) (x)->rx_ilas_status_register.rx_ilas_status_u.bits.disparity_reg
#define monza_rx_link_regs_A_DET_COMB(x) (x)->rx_ilas_status_register.rx_ilas_status_u.bits.a_det_comb
#define monza_rx_link_regs_R_DET_COMB(x) (x)->rx_ilas_status_register.rx_ilas_status_u.bits.r_det_comb
#define monza_rx_link_regs_F_DET_COMB(x) (x)->rx_ilas_status_register.rx_ilas_status_u.bits.f_det_comb
typedef volatile struct rx_ilas_status_s {
	union {
		struct {
			unsigned int ilas_cfg_lid : 5;
			unsigned int ilas_checksum_error : 1;
			unsigned int ilas_fsm_state : 4;
			unsigned int ilas_lid_mismatch : 1;
			unsigned int ilas_aligned : 1;
			unsigned int disparity_reg : 1;
			unsigned int a_det_comb : 1;
			unsigned int r_det_comb : 1;
			unsigned int f_det_comb : 1;
		} bits;
		uint32_t rx_ilas_status;
	} rx_ilas_status_u;
} rx_ilas_status_t;
#endif /* __RX_ILAS_STATUS__ */

#ifndef __RX_MANUAL_CONTROL__
#define __RX_MANUAL_CONTROL__
#define RX_MANUAL_CONTROL(x) (x)->rx_manual_control_register.rx_manual_control_u.rx_manual_control
#define monza_rx_link_regs_MANUAL_ENABLE(x) (x)->rx_manual_control_register.rx_manual_control_u.bits.manual_enable
#define monza_rx_link_regs_MANUAL_FIFO_RESET(x) (x)->rx_manual_control_register.rx_manual_control_u.bits.manual_fifo_reset
#define monza_rx_link_regs_MANUAL_RX_CGS_LOCKED(x) (x)->rx_manual_control_register.rx_manual_control_u.bits.manual_rx_cgs_locked
#define monza_rx_link_regs_MANUAL_RX_CGS_DONE(x) (x)->rx_manual_control_register.rx_manual_control_u.bits.manual_rx_cgs_done
#define monza_rx_link_regs_BIT_REVERSE(x) (x)->rx_manual_control_register.rx_manual_control_u.bits.bit_reverse
#define monza_rx_link_regs_MAN_LINK_LAYER_OUT(x) (x)->rx_manual_control_register.rx_manual_control_u.bits.man_link_layer_out
#define monza_rx_link_regs_INJECT_LINK_LAYER_OUT(x) (x)->rx_manual_control_register.rx_manual_control_u.bits.inject_link_layer_out
#define monza_rx_link_regs_CGS_FIFO_RST_DISABLE(x) (x)->rx_manual_control_register.rx_manual_control_u.bits.cgs_fifo_rst_disable
#define monza_rx_link_regs_PIPELINE_20B_REVERSE(x) (x)->rx_manual_control_register.rx_manual_control_u.bits.pipeline_20b_reverse
#define monza_rx_link_regs_PIPELINE_20B_FREEZE(x) (x)->rx_manual_control_register.rx_manual_control_u.bits.pipeline_20b_freeze
typedef volatile struct rx_manual_control_s {
	union {
		struct {
			unsigned int manual_enable : 1;
			unsigned int manual_fifo_reset : 1;
			unsigned int manual_rx_cgs_locked : 1;
			unsigned int manual_rx_cgs_done : 1;
			unsigned int bit_reverse : 1;
			unsigned int man_link_layer_out : 9;
			unsigned int inject_link_layer_out : 1;
			unsigned int cgs_fifo_rst_disable : 1;
			unsigned int pipeline_20b_reverse : 1;
			unsigned int pipeline_20b_freeze : 1;
		} bits;
		uint32_t rx_manual_control;
	} rx_manual_control_u;
} rx_manual_control_t;
#endif /* __RX_MANUAL_CONTROL__ */

#ifndef __RX_LANE_D21_5_COUNTS__
#define __RX_LANE_D21_5_COUNTS__
#define RX_LANE_D21_5_COUNTS(x) (x)->rx_lane_d21_5_counts_register.rx_lane_d21_5_counts_u.rx_lane_d21_5_counts
#define monza_rx_link_regs_D21_5_COUNT(x) (x)->rx_lane_d21_5_counts_register.rx_lane_d21_5_counts_u.bits.d21_5_count
#define monza_rx_link_regs_LANE_ALIGN_TAP(x) (x)->rx_lane_d21_5_counts_register.rx_lane_d21_5_counts_u.bits.lane_align_tap
#define monza_rx_link_regs_S_DET_COMB(x) (x)->rx_lane_d21_5_counts_register.rx_lane_d21_5_counts_u.bits.s_det_comb
#define monza_rx_link_regs_Q_DET_COMB(x) (x)->rx_lane_d21_5_counts_register.rx_lane_d21_5_counts_u.bits.q_det_comb
#define monza_rx_link_regs_ERR_DET_COMB(x) (x)->rx_lane_d21_5_counts_register.rx_lane_d21_5_counts_u.bits.err_det_comb
#define monza_rx_link_regs_DISP_ERR_DET_COMB(x) (x)->rx_lane_d21_5_counts_register.rx_lane_d21_5_counts_u.bits.disp_err_det_comb
typedef volatile struct rx_lane_d21_5_counts_s {
	union {
		struct {
			unsigned int d21_5_count : 8;
			unsigned int lane_align_tap : 4;
			unsigned int s_det_comb : 1;
			unsigned int q_det_comb : 1;
			unsigned int err_det_comb : 1;
			unsigned int disp_err_det_comb : 1;
		} bits;
		uint32_t rx_lane_d21_5_counts;
	} rx_lane_d21_5_counts_u;
} rx_lane_d21_5_counts_t;
#endif /* __RX_LANE_D21_5_COUNTS__ */

#ifndef __RX_LANE_NON_K_CNT4TEST__
#define __RX_LANE_NON_K_CNT4TEST__
#define RX_LANE_NON_K_CNT4TEST(x) (x)->rx_lane_non_k_cnt4test_register.rx_lane_non_k_cnt4test_u.rx_lane_non_k_cnt4test
#define monza_rx_link_regs_NON_K_CNT4TEST(x) (x)->rx_lane_non_k_cnt4test_register.rx_lane_non_k_cnt4test_u.bits.non_k_cnt4test
#define monza_rx_link_regs_DISPARITY_COMB(x) (x)->rx_lane_non_k_cnt4test_register.rx_lane_non_k_cnt4test_u.bits.disparity_comb
#define monza_rx_link_regs_NON_D21_5_CNT4TEST(x) (x)->rx_lane_non_k_cnt4test_register.rx_lane_non_k_cnt4test_u.bits.non_d21_5_cnt4test
#define monza_rx_link_regs_TOGGLE_DISPARITY(x) (x)->rx_lane_non_k_cnt4test_register.rx_lane_non_k_cnt4test_u.bits.toggle_disparity
typedef volatile struct rx_lane_non_k_cnt4test_s {
	union {
		struct {
			unsigned int non_k_cnt4test : 15;
			unsigned int disparity_comb : 1;
			unsigned int non_d21_5_cnt4test : 15;
			unsigned int toggle_disparity : 1;
		} bits;
		uint32_t rx_lane_non_k_cnt4test;
	} rx_lane_non_k_cnt4test_u;
} rx_lane_non_k_cnt4test_t;
#endif /* __RX_LANE_NON_K_CNT4TEST__ */

#ifndef __SERDES_20B_PIPELINE__
#define __SERDES_20B_PIPELINE__
#define SERDES_20B_PIPELINE(x) (x)->serdes_20b_pipeline_register.serdes_20b_pipeline_u.serdes_20b_pipeline
#define monza_rx_link_regs_PIPELINE_20B_DATA(x) (x)->serdes_20b_pipeline_register.serdes_20b_pipeline_u.bits.pipeline_20b_data
typedef volatile struct serdes_20b_pipeline_s {
	union {
		struct {
			unsigned int pipeline_20b_data : 20;
		} bits;
		uint32_t serdes_20b_pipeline;
	} serdes_20b_pipeline_u;
} serdes_20b_pipeline_t;
#endif /* __SERDES_20B_PIPELINE__ */

#ifndef __RX_CONFIG0_DATA__
#define __RX_CONFIG0_DATA__
#define RX_CONFIG0_DATA(x) (x)->rx_config0_data_register.rx_config0_data_u.rx_config0_data
#define monza_rx_link_regs_CONFIG0(x) (x)->rx_config0_data_register.rx_config0_data_u.bits.config0
typedef volatile struct rx_config0_data_s {
	union {
		struct {
			unsigned int config0 : 16;
		} bits;
		uint32_t rx_config0_data;
	} rx_config0_data_u;
} rx_config0_data_t;
#endif /* __RX_CONFIG0_DATA__ */

#ifndef __RX_CONFIG1_DATA__
#define __RX_CONFIG1_DATA__
#define RX_CONFIG1_DATA(x) (x)->rx_config1_data_register.rx_config1_data_u.rx_config1_data
#define monza_rx_link_regs_CONFIG1(x) (x)->rx_config1_data_register.rx_config1_data_u.bits.config1
typedef volatile struct rx_config1_data_s {
	union {
		struct {
			unsigned int config1 : 16;
		} bits;
		uint32_t rx_config1_data;
	} rx_config1_data_u;
} rx_config1_data_t;
#endif /* __RX_CONFIG1_DATA__ */

#ifndef __RX_CONFIG2_DATA__
#define __RX_CONFIG2_DATA__
#define RX_CONFIG2_DATA(x) (x)->rx_config2_data_register.rx_config2_data_u.rx_config2_data
#define monza_rx_link_regs_CONFIG2(x) (x)->rx_config2_data_register.rx_config2_data_u.bits.config2
typedef volatile struct rx_config2_data_s {
	union {
		struct {
			unsigned int config2 : 16;
		} bits;
		uint32_t rx_config2_data;
	} rx_config2_data_u;
} rx_config2_data_t;
#endif /* __RX_CONFIG2_DATA__ */

#ifndef __RX_CONFIG3_DATA__
#define __RX_CONFIG3_DATA__
#define RX_CONFIG3_DATA(x) (x)->rx_config3_data_register.rx_config3_data_u.rx_config3_data
#define monza_rx_link_regs_CONFIG3(x) (x)->rx_config3_data_register.rx_config3_data_u.bits.config3
typedef volatile struct rx_config3_data_s {
	union {
		struct {
			unsigned int config3 : 16;
		} bits;
		uint32_t rx_config3_data;
	} rx_config3_data_u;
} rx_config3_data_t;
#endif /* __RX_CONFIG3_DATA__ */

#ifndef __RX_CONFIG4_DATA__
#define __RX_CONFIG4_DATA__
#define RX_CONFIG4_DATA(x) (x)->rx_config4_data_register.rx_config4_data_u.rx_config4_data
#define monza_rx_link_regs_CONFIG4(x) (x)->rx_config4_data_register.rx_config4_data_u.bits.config4
typedef volatile struct rx_config4_data_s {
	union {
		struct {
			unsigned int config4 : 16;
		} bits;
		uint32_t rx_config4_data;
	} rx_config4_data_u;
} rx_config4_data_t;
#endif /* __RX_CONFIG4_DATA__ */

#ifndef __RX_CONFIG5_DATA__
#define __RX_CONFIG5_DATA__
#define RX_CONFIG5_DATA(x) (x)->rx_config5_data_register.rx_config5_data_u.rx_config5_data
#define monza_rx_link_regs_CONFIG5(x) (x)->rx_config5_data_register.rx_config5_data_u.bits.config5
typedef volatile struct rx_config5_data_s {
	union {
		struct {
			unsigned int config5 : 16;
		} bits;
		uint32_t rx_config5_data;
	} rx_config5_data_u;
} rx_config5_data_t;
#endif /* __RX_CONFIG5_DATA__ */

#ifndef __RX_CONFIG6_DATA__
#define __RX_CONFIG6_DATA__
#define RX_CONFIG6_DATA(x) (x)->rx_config6_data_register.rx_config6_data_u.rx_config6_data
#define monza_rx_link_regs_CONFIG6(x) (x)->rx_config6_data_register.rx_config6_data_u.bits.config6
typedef volatile struct rx_config6_data_s {
	union {
		struct {
			unsigned int config6 : 16;
		} bits;
		uint32_t rx_config6_data;
	} rx_config6_data_u;
} rx_config6_data_t;
#endif /* __RX_CONFIG6_DATA__ */

typedef struct monza_rx_link_regs_s
{
	rx_lane_cgs_status_t rx_lane_cgs_status_register;	// Address: 0x0
	rx_lane_counts_t rx_lane_counts_register;	// Address: 0x1
	rx_lane_config_t rx_lane_config_register;	// Address: 0x2
	rx_total_errors_t rx_total_errors_register;	// Address: 0x3
	rx_ilas_status_t rx_ilas_status_register;	// Address: 0x4
	rx_manual_control_t rx_manual_control_register;	// Address: 0x5
	rx_lane_d21_5_counts_t rx_lane_d21_5_counts_register;	// Address: 0x6
	rx_lane_non_k_cnt4test_t rx_lane_non_k_cnt4test_register;	// Address: 0x7
	serdes_20b_pipeline_t serdes_20b_pipeline_register;	// Address: 0x8
	rx_config0_data_t rx_config0_data_register;	// Address: 0x9
	rx_config1_data_t rx_config1_data_register;	// Address: 0xa
	rx_config2_data_t rx_config2_data_register;	// Address: 0xb
	rx_config3_data_t rx_config3_data_register;	// Address: 0xc
	rx_config4_data_t rx_config4_data_register;	// Address: 0xd
	rx_config5_data_t rx_config5_data_register;	// Address: 0xe
	rx_config6_data_t rx_config6_data_register;	// Address: 0xf
} monza_rx_link_regs_t;

#endif /* __MONZA_RX_LINK_REGS__ */