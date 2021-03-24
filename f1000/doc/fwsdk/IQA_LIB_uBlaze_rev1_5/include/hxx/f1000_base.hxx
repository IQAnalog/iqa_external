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

#ifndef __F1000_BASE_ADDRESS__
#define __F1000_BASE_ADDRESS__

#define REG_RISCV_UART                0x1a100000
#define REG_FPGA                      0x20000000
#define REG_EP_A                      0x40000000
#define REG_ADC_DATA_CTRL             0x40001000
#define REG_ADC1_MEM_CAP              0x40002000
#define REG_ADC2_MEM_CAP              0x40003000
#define REG_DAC1_MEM_PLAY             0x40004000
#define REG_DAC2_MEM_PLAY             0x40005000
#define REG_ADC1_DSP                  0x40006000
#define REG_ADC2_DSP                  0x40007000
#define REG_DAC1_DSP                  0x40008000
#define REG_DAC2_DSP                  0x40009000
#define REG_RX_LL_CORE                0x4000a000
#define REG_RX_CDR                    0x4000a200
#define REG_RX_ETB                    0x4000a400
#define REG_RX_LL_LANE0               0x4000a800
#define REG_RX_MERLIN_CTRL0           0x4000a840
#define REG_RX_FRAMER0                0x4000a870
#define REG_RX_LL_LANE1               0x4000a880
#define REG_RX_MERLIN_CTRL1           0x4000a8c0
#define REG_RX_FRAMER1                0x4000a8f0
#define REG_RX_LL_LANE2               0x4000a900
#define REG_RX_MERLIN_CTRL2           0x4000a940
#define REG_RX_FRAMER2                0x4000a970
#define REG_RX_LL_LANE3               0x4000a980
#define REG_RX_MERLIN_CTRL3           0x4000a9c0
#define REG_RX_FRAMER3                0x4000a9f0
#define REG_RX_LL_LANE4               0x4000aa00
#define REG_RX_MERLIN_CTRL4           0x4000aa40
#define REG_RX_FRAMER4                0x4000aa70
#define REG_RX_LL_LANE5               0x4000aa80
#define REG_RX_MERLIN_CTRL5           0x4000aac0
#define REG_RX_FRAMER5                0x4000aaf0
#define REG_RX_LL_LANE6               0x4000ab00
#define REG_RX_MERLIN_CTRL6           0x4000ab40
#define REG_RX_FRAMER6                0x4000ab70
#define REG_RX_LL_LANE7               0x4000ab80
#define REG_RX_MERLIN_CTRL7           0x4000abc0
#define REG_RX_FRAMER7                0x4000abf0
#define REG_TX_LL_CORE                0x4000b000
#define REG_TX_SAT                    0x4000b200
#define REG_TX_ETB                    0x4000b400
#define REG_TX_DRV                    0x4000b600
#define REG_TX_LL_LANE0               0x4000b800
#define REG_TX_MERLIN_CTRL0           0x4000b840
#define REG_TX_FRAMER0                0x4000b870
#define REG_TX_LL_LANE1               0x4000b880
#define REG_TX_MERLIN_CTRL1           0x4000b8c0
#define REG_TX_FRAMER1                0x4000b8f0
#define REG_TX_LL_LANE2               0x4000b900
#define REG_TX_MERLIN_CTRL2           0x4000b940
#define REG_TX_FRAMER2                0x4000b970
#define REG_TX_LL_LANE3               0x4000b980
#define REG_TX_MERLIN_CTRL3           0x4000b9c0
#define REG_TX_FRAMER3                0x4000b9f0
#define REG_TX_LL_LANE4               0x4000ba00
#define REG_TX_MERLIN_CTRL4           0x4000ba40
#define REG_TX_FRAMER4                0x4000ba70
#define REG_TX_LL_LANE5               0x4000ba80
#define REG_TX_MERLIN_CTRL5           0x4000bac0
#define REG_TX_FRAMER5                0x4000baf0
#define REG_TX_LL_LANE6               0x4000bb00
#define REG_TX_MERLIN_CTRL6           0x4000bb40
#define REG_TX_FRAMER6                0x4000bb70
#define REG_TX_LL_LANE7               0x4000bb80
#define REG_TX_MERLIN_CTRL7           0x4000bbc0
#define REG_TX_FRAMER7                0x4000bbf0
#define REG_TEST_DSP1_MEM_CAP         0x4000c000
#define REG_TEST_DSP2_MEM_CAP         0x4000d000
#define REG_TEST_DSP1_MEM_PLAY        0x4000e000
#define REG_TEST_DSP2_MEM_PLAY        0x4000f000
#define REG_ADC1_POLAR_CTRL           0x40010000
#define REG_DAC1_POLAR_CTRL           0x40011000
#define REG_ADC2_POLAR_CTRL           0x40012000
#define REG_DAC2_POLAR_CTRL           0x40013000
#define REG_GP                        0x40014000
#define REG_RAM_PARAM                 0x40015000
#define REG_PAD_CTRL                  0x40016000
#define REG_OOB_PROC                  0x40017000
#define REG_OOB_MERLIN                0x40017200
#define REG_OOB_ETB                   0x40017400
#define REG_OOB_AXIFACE               0x40017600
#define REG_ADC1_CH0                  0x41000000
#define REG_ADC1_CH1                  0x41001000
#define REG_ADC1_CH2                  0x41002000
#define REG_ADC1_CH3                  0x41003000
#define REG_ADC1_CH4                  0x41004000
#define REG_ADC1_CH5                  0x41005000
#define REG_ADC1_CH6                  0x41006000
#define REG_ADC1_CH7                  0x41007000
#define REG_ADC1_CH8                  0x41008000
#define REG_ADC1_CH9                  0x41009000
#define REG_ADC1_CH10                 0x4100a000
#define REG_ADC1_CH11                 0x4100b000
#define REG_ADC1_CH12                 0x4100c000
#define REG_ADC1_CH13                 0x4100d000
#define REG_ADC1_CH14                 0x4100e000
#define REG_ADC1_CH15                 0x4100f000
#define REG_ADC1_CH16                 0x41010000
#define REG_ADC1_CH17                 0x41011000
#define REG_ADC1_TI_CAL               0x41012000
#define REG_ADC1_TOP                  0x41013000
#define REG_ADC2_CH0                  0x41100000
#define REG_ADC2_CH1                  0x41101000
#define REG_ADC2_CH2                  0x41102000
#define REG_ADC2_CH3                  0x41103000
#define REG_ADC2_CH4                  0x41104000
#define REG_ADC2_CH5                  0x41105000
#define REG_ADC2_CH6                  0x41106000
#define REG_ADC2_CH7                  0x41107000
#define REG_ADC2_CH8                  0x41108000
#define REG_ADC2_CH9                  0x41109000
#define REG_ADC2_CH10                 0x4110a000
#define REG_ADC2_CH11                 0x4110b000
#define REG_ADC2_CH12                 0x4110c000
#define REG_ADC2_CH13                 0x4110d000
#define REG_ADC2_CH14                 0x4110e000
#define REG_ADC2_CH15                 0x4110f000
#define REG_ADC2_CH16                 0x41110000
#define REG_ADC2_CH17                 0x41111000
#define REG_ADC2_TI_CAL               0x41112000
#define REG_ADC2_TOP                  0x41113000
#define REG_DAC1_DIG                  0x42000000
#define REG_DAC2_DIG                  0x42100000
#define REG_EP_B                      0x40000000
#endif /* __F1000_BASE_ADDRESS__ */
