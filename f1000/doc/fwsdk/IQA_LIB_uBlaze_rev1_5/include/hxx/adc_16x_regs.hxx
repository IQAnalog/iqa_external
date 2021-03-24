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

#ifndef __ADC_16X_REGS__
#define __ADC_16X_REGS__

#ifndef __ADC16_PWR__
#define __ADC16_PWR__
#define ADC16_PWR(x) (x)->adc16_pwr_register.adc16_pwr_u.adc16_pwr
#define ADC_TOP_EN(x) (x)->adc16_pwr_register.adc16_pwr_u.bits.en
#define ADC_TOP_DLL_CLK_EN(x) (x)->adc16_pwr_register.adc16_pwr_u.bits.dll_clk_en
#define ADC_TOP_TEST_PAT_EN(x) (x)->adc16_pwr_register.adc16_pwr_u.bits.test_pat_en
#define ADC_TOP_TEST_PAT_SEL(x) (x)->adc16_pwr_register.adc16_pwr_u.bits.test_pat_sel
typedef volatile struct adc16_pwr_s {
	union {
		struct {
			unsigned int en : 9;
			unsigned int dll_clk_en : 1;
			unsigned int test_pat_en : 1;
			unsigned int test_pat_sel : 2;
		} bits;
		uint32_t adc16_pwr;
	} adc16_pwr_u;
} adc16_pwr_t;
#endif /* __ADC16_PWR__ */

#ifndef __ADC16_MUX__
#define __ADC16_MUX__
#define ADC16_MUX(x) (x)->adc16_mux_register.adc16_mux_u.adc16_mux
#define ADC_TOP_SWEEP_DIR(x) (x)->adc16_mux_register.adc16_mux_u.bits.sweep_dir
#define ADC_TOP_EN_MUX_CTRL(x) (x)->adc16_mux_register.adc16_mux_u.bits.en_mux_ctrl
#define ADC_TOP_DAT_OUT_SEL_REG(x) (x)->adc16_mux_register.adc16_mux_u.bits.dat_out_sel_reg
#define ADC_TOP_FORCE_SKIP_MUX(x) (x)->adc16_mux_register.adc16_mux_u.bits.force_skip_mux
#define ADC_TOP_ILV_MODE(x) (x)->adc16_mux_register.adc16_mux_u.bits.ilv_mode
#define ADC_TOP_EN_MUX(x) (x)->adc16_mux_register.adc16_mux_u.bits.en_mux
typedef volatile struct adc16_mux_s {
	union {
		struct {
			unsigned int sweep_dir : 8;
			unsigned int en_mux_ctrl : 8;
			unsigned int dat_out_sel_reg : 8;
			unsigned int force_skip_mux : 1;
			unsigned int ilv_mode : 2;
			unsigned int en_mux : 1;
		} bits;
		uint32_t adc16_mux;
	} adc16_mux_u;
} adc16_mux_t;
#endif /* __ADC16_MUX__ */

#ifndef __ADC16_CALDAC__
#define __ADC16_CALDAC__
#define ADC16_CALDAC(x) (x)->adc16_caldac_register.adc16_caldac_u.adc16_caldac
#define ADC_TOP_CAL_DAC_DAT(x) (x)->adc16_caldac_register.adc16_caldac_u.bits.cal_dac_dat
#define ADC_TOP_CAL_DAC_TRIGGER(x) (x)->adc16_caldac_register.adc16_caldac_u.bits.cal_dac_trigger
#define ADC_TOP_CAL_DAC_CTRL(x) (x)->adc16_caldac_register.adc16_caldac_u.bits.cal_dac_ctrl
typedef volatile struct adc16_caldac_s {
	union {
		struct {
			int cal_dac_dat : 12;
			unsigned int cal_dac_trigger : 1;
			unsigned int cal_dac_ctrl : 16;
		} bits;
		uint32_t adc16_caldac;
	} adc16_caldac_u;
} adc16_caldac_t;
#endif /* __ADC16_CALDAC__ */

#ifndef __ADC16_DLL__
#define __ADC16_DLL__
#define ADC16_DLL(x) (x)->adc16_dll_register.adc16_dll_u.adc16_dll
#define ADC_TOP_DLL_CTRL(x) (x)->adc16_dll_register.adc16_dll_u.bits.dll_ctrl
#define ADC_TOP_DLL_RST_N(x) (x)->adc16_dll_register.adc16_dll_u.bits.dll_rst_n
#define ADC_TOP_DLL_CP_BIAS_CTRL(x) (x)->adc16_dll_register.adc16_dll_u.bits.dll_cp_bias_ctrl
#define ADC_TOP_DLL_HF_SEL(x) (x)->adc16_dll_register.adc16_dll_u.bits.dll_hf_sel
#define ADC_TOP_DLL_VC_CTRL(x) (x)->adc16_dll_register.adc16_dll_u.bits.dll_vc_ctrl
#define ADC_TOP_DLL_CAP_CTRL(x) (x)->adc16_dll_register.adc16_dll_u.bits.dll_cap_ctrl
#define ADC_TOP_CLK_CM_CTRL(x) (x)->adc16_dll_register.adc16_dll_u.bits.clk_cm_ctrl
typedef volatile struct adc16_dll_s {
	union {
		struct {
			unsigned int dll_ctrl : 8;
			unsigned int dll_rst_n : 1;
			unsigned int dll_cp_bias_ctrl : 2;
			unsigned int dll_hf_sel : 1;
			unsigned int dll_vc_ctrl : 4;
			unsigned int dll_cap_ctrl : 3;
			unsigned int clk_cm_ctrl : 2;
		} bits;
		uint32_t adc16_dll;
	} adc16_dll_u;
} adc16_dll_t;
#endif /* __ADC16_DLL__ */

#ifndef __ADC16_DLL_STATUS__
#define __ADC16_DLL_STATUS__
#define ADC16_DLL_STATUS(x) (x)->adc16_dll_status_register.adc16_dll_status_u.adc16_dll_status
#define ADC_TOP_DLL_STATUS(x) (x)->adc16_dll_status_register.adc16_dll_status_u.bits.dll_status
typedef volatile struct adc16_dll_status_s {
	union {
		struct {
			unsigned int dll_status : 4;
		} bits;
		uint32_t adc16_dll_status;
	} adc16_dll_status_u;
} adc16_dll_status_t;
#endif /* __ADC16_DLL_STATUS__ */

#ifndef __ADC16_BIAS__
#define __ADC16_BIAS__
#define ADC16_BIAS(x) (x)->adc16_bias_register.adc16_bias_u.adc16_bias
#define ADC_TOP_BIAS_CTRL(x) (x)->adc16_bias_register.adc16_bias_u.bits.bias_ctrl
#define ADC_TOP_GP_CTRL(x) (x)->adc16_bias_register.adc16_bias_u.bits.gp_ctrl
typedef volatile struct adc16_bias_s {
	union {
		struct {
			unsigned int bias_ctrl : 4;
			unsigned int gp_ctrl : 8;
		} bits;
		uint32_t adc16_bias;
	} adc16_bias_u;
} adc16_bias_t;
#endif /* __ADC16_BIAS__ */

#ifndef __ADC16_LDO1__
#define __ADC16_LDO1__
#define ADC16_LDO1(x) (x)->adc16_ldo1_register.adc16_ldo1_u.adc16_ldo1
#define ADC_TOP_LDO_FINE_VAL1(x) (x)->adc16_ldo1_register.adc16_ldo1_u.bits.ldo_fine_val1
#define ADC_TOP_LDO_FINE_EN1(x) (x)->adc16_ldo1_register.adc16_ldo1_u.bits.ldo_fine_en1
#define ADC_TOP_LDO_FINE_RATE1(x) (x)->adc16_ldo1_register.adc16_ldo1_u.bits.ldo_fine_rate1
#define ADC_TOP_LDO_CTRL1(x) (x)->adc16_ldo1_register.adc16_ldo1_u.bits.ldo_ctrl1
typedef volatile struct adc16_ldo1_s {
	union {
		struct {
			int ldo_fine_val1 : 8;
			unsigned int ldo_fine_en1 : 1;
			unsigned int ldo_fine_rate1 : 2;
			unsigned int ldo_ctrl1 : 8;
		} bits;
		uint32_t adc16_ldo1;
	} adc16_ldo1_u;
} adc16_ldo1_t;
#endif /* __ADC16_LDO1__ */

#ifndef __ADC16_LDO2__
#define __ADC16_LDO2__
#define ADC16_LDO2(x) (x)->adc16_ldo2_register.adc16_ldo2_u.adc16_ldo2
#define ADC_TOP_LDO_FINE_VAL2(x) (x)->adc16_ldo2_register.adc16_ldo2_u.bits.ldo_fine_val2
#define ADC_TOP_LDO_FINE_EN2(x) (x)->adc16_ldo2_register.adc16_ldo2_u.bits.ldo_fine_en2
#define ADC_TOP_LDO_FINE_RATE2(x) (x)->adc16_ldo2_register.adc16_ldo2_u.bits.ldo_fine_rate2
#define ADC_TOP_LDO_CTRL2(x) (x)->adc16_ldo2_register.adc16_ldo2_u.bits.ldo_ctrl2
typedef volatile struct adc16_ldo2_s {
	union {
		struct {
			int ldo_fine_val2 : 8;
			unsigned int ldo_fine_en2 : 1;
			unsigned int ldo_fine_rate2 : 2;
			unsigned int ldo_ctrl2 : 8;
		} bits;
		uint32_t adc16_ldo2;
	} adc16_ldo2_u;
} adc16_ldo2_t;
#endif /* __ADC16_LDO2__ */

#ifndef __ADC16_LDO3__
#define __ADC16_LDO3__
#define ADC16_LDO3(x) (x)->adc16_ldo3_register.adc16_ldo3_u.adc16_ldo3
#define ADC_TOP_LDO_FINE_VAL3(x) (x)->adc16_ldo3_register.adc16_ldo3_u.bits.ldo_fine_val3
#define ADC_TOP_LDO_FINE_EN3(x) (x)->adc16_ldo3_register.adc16_ldo3_u.bits.ldo_fine_en3
#define ADC_TOP_LDO_FINE_RATE3(x) (x)->adc16_ldo3_register.adc16_ldo3_u.bits.ldo_fine_rate3
#define ADC_TOP_LDO_CTRL3(x) (x)->adc16_ldo3_register.adc16_ldo3_u.bits.ldo_ctrl3
typedef volatile struct adc16_ldo3_s {
	union {
		struct {
			int ldo_fine_val3 : 8;
			unsigned int ldo_fine_en3 : 1;
			unsigned int ldo_fine_rate3 : 2;
			unsigned int ldo_ctrl3 : 8;
		} bits;
		uint32_t adc16_ldo3;
	} adc16_ldo3_u;
} adc16_ldo3_t;
#endif /* __ADC16_LDO3__ */

#ifndef __ADC16_DIGI_CLK__
#define __ADC16_DIGI_CLK__
#define ADC16_DIGI_CLK(x) (x)->adc16_digi_clk_register.adc16_digi_clk_u.adc16_digi_clk
#define ADC_TOP_CK_SYNC(x) (x)->adc16_digi_clk_register.adc16_digi_clk_u.bits.ck_sync
#define ADC_TOP_CK_PH(x) (x)->adc16_digi_clk_register.adc16_digi_clk_u.bits.ck_ph
#define ADC_TOP_SYNC(x) (x)->adc16_digi_clk_register.adc16_digi_clk_u.bits.sync
typedef volatile struct adc16_digi_clk_s {
	union {
		struct {
			unsigned int ck_sync : 1;
			unsigned int ck_ph : 4;
			unsigned int sync : 1;
		} bits;
		uint32_t adc16_digi_clk;
	} adc16_digi_clk_u;
} adc16_digi_clk_t;
#endif /* __ADC16_DIGI_CLK__ */

#ifndef __ADC16_SH__
#define __ADC16_SH__
#define ADC16_SH(x) (x)->adc16_sh_register.adc16_sh_u.adc16_sh
#define ADC_TOP_SAH_EN(x) (x)->adc16_sh_register.adc16_sh_u.bits.sah_en
#define ADC_TOP_SAH_CTRL(x) (x)->adc16_sh_register.adc16_sh_u.bits.sah_ctrl
typedef volatile struct adc16_sh_s {
	union {
		struct {
			unsigned int sah_en : 9;
			unsigned int sah_ctrl : 4;
		} bits;
		uint32_t adc16_sh;
	} adc16_sh_u;
} adc16_sh_t;
#endif /* __ADC16_SH__ */

#ifndef __ADC16_ATEST__
#define __ADC16_ATEST__
#define ADC16_ATEST(x) (x)->adc16_atest_register.adc16_atest_u.adc16_atest
#define ADC_TOP_ATEST_CTRL(x) (x)->adc16_atest_register.adc16_atest_u.bits.atest_ctrl
#define ADC_TOP_SAH_ATEST_EN(x) (x)->adc16_atest_register.adc16_atest_u.bits.sah_atest_en
#define ADC_TOP_DLL_ATEST_SEL(x) (x)->adc16_atest_register.adc16_atest_u.bits.dll_atest_sel
typedef volatile struct adc16_atest_s {
	union {
		struct {
			unsigned int atest_ctrl : 16;
			unsigned int sah_atest_en : 1;
			unsigned int dll_atest_sel : 3;
		} bits;
		uint32_t adc16_atest;
	} adc16_atest_u;
} adc16_atest_t;
#endif /* __ADC16_ATEST__ */

#ifndef __SW_CAL_CTRL__
#define __SW_CAL_CTRL__
#define ADC_TOP_SW_CAL_EN(x) (x)->sw_cal_ctrl_register.sw_cal_ctrl_u.bits.sw_cal_en
#define ADC_TOP_SW_CAL_TRIG(x) (x)->sw_cal_ctrl_register.sw_cal_ctrl_u.bits.sw_cal_trig
#define SW_CAL_CTRL(x) (x)->sw_cal_ctrl_register.sw_cal_ctrl_u.sw_cal_ctrl
#define ADC_TOP_SW_CAL_CTRL(x) (x)->sw_cal_ctrl_register.sw_cal_ctrl_u.bits.sw_cal_ctrl
typedef volatile struct sw_cal_ctrl_s {
	union {
		struct {
			unsigned int sw_cal_en : 1;
			unsigned int sw_cal_trig : 1;
			unsigned int sw_cal_ctrl : 6;
		} bits;
		uint32_t sw_cal_ctrl;
	} sw_cal_ctrl_u;
} sw_cal_ctrl_t;
#endif /* __SW_CAL_CTRL__ */

#ifndef __SW_CAL_STATUS__
#define __SW_CAL_STATUS__
#define SW_CAL_STATUS(x) (x)->sw_cal_status_register.sw_cal_status_u.sw_cal_status
#define ADC_TOP_SW_CAL_STATUS(x) (x)->sw_cal_status_register.sw_cal_status_u.bits.sw_cal_status
typedef volatile struct sw_cal_status_s {
	union {
		struct {
			unsigned int sw_cal_status : 8;
		} bits;
		uint32_t sw_cal_status;
	} sw_cal_status_u;
} sw_cal_status_t;
#endif /* __SW_CAL_STATUS__ */

#ifndef __SYNC_STATUS__
#define __SYNC_STATUS__
#define SYNC_STATUS(x) (x)->sync_status_register.sync_status_u.sync_status
#define ADC_TOP_SYNC_STATUS(x) (x)->sync_status_register.sync_status_u.bits.sync_status
typedef volatile struct sync_status_s {
	union {
		struct {
			unsigned int sync_status : 8;
		} bits;
		uint32_t sync_status;
	} sync_status_u;
} sync_status_t;
#endif /* __SYNC_STATUS__ */

typedef struct adc_16x_regs_s
{
	adc16_pwr_t adc16_pwr_register;	// Address: 0x0
	adc16_mux_t adc16_mux_register;	// Address: 0x1
	adc16_caldac_t adc16_caldac_register;	// Address: 0x2
	adc16_dll_t adc16_dll_register;	// Address: 0x3
	adc16_dll_status_t adc16_dll_status_register;	// Address: 0x4
	adc16_bias_t adc16_bias_register;	// Address: 0x5
	adc16_ldo1_t adc16_ldo1_register;	// Address: 0x6
	adc16_ldo2_t adc16_ldo2_register;	// Address: 0x7
	adc16_ldo3_t adc16_ldo3_register;	// Address: 0x8
	adc16_digi_clk_t adc16_digi_clk_register;	// Address: 0x9
	adc16_sh_t adc16_sh_register;	// Address: 0xa
	adc16_atest_t adc16_atest_register;	// Address: 0xb
	sw_cal_ctrl_t sw_cal_ctrl_register;	// Address: 0xc
	sw_cal_status_t sw_cal_status_register;	// Address: 0xd
	sync_status_t sync_status_register;	// Address: 0xe
} adc_16x_regs_t;

#endif /* __ADC_16X_REGS__ */
