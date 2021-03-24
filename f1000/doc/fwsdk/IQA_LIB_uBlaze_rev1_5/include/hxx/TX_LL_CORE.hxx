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

// Generated by RMM 3.3
// IQ-Analog Corp. 2013-2018.

#ifndef __TX_LL_CORE_HXX__
#define __TX_LL_CORE_HXX__

#define TX_LL_CORE_TX_LINK_CONFIG            (REG_TX_LL_CORE+0x0)
#define TX_LL_CORE_TX_LINK_ENABLE            (REG_TX_LL_CORE+0x4)
#define TX_LL_CORE_TX_SYSREF_MISALIGN_COUNT  (REG_TX_LL_CORE+0x8)
#define TX_LL_CORE_TX_LANE_ENABLE            (REG_TX_LL_CORE+0xc)
#define TX_LL_CORE_TX_SCRAMBLE_CONFIG        (REG_TX_LL_CORE+0x10)
#define TX_LL_CORE_TX_DEBUG                  (REG_TX_LL_CORE+0x14)
#define TX_LL_CORE_TX_FCLK_STATUS            (REG_TX_LL_CORE+0x18)
#define TX_LL_CORE_TX_CGS_ILAS_CONFIG_1      (REG_TX_LL_CORE+0x1c)
#define TX_LL_CORE_TX_CGS_ILAS_CONFIG_2      (REG_TX_LL_CORE+0x20)
#define TX_LL_CORE_TX_CGS_ILAS_CONFIG_3      (REG_TX_LL_CORE+0x24)
#define TX_LL_CORE_TX_CGS_ILAS_CONFIG_4      (REG_TX_LL_CORE+0x28)
#define TX_LL_CORE_TX_SYNC_CONFIG            (REG_TX_LL_CORE+0x2c)
#define TX_LL_CORE_TX_SYNC_ERRORS_LINK0_1    (REG_TX_LL_CORE+0x30)
#define TX_LL_CORE_TX_SYNC_ERRORS_LINK0_2    (REG_TX_LL_CORE+0x34)
#define TX_LL_CORE_TX_SYNC_ERRORS_LINK0_3    (REG_TX_LL_CORE+0x38)
#define TX_LL_CORE_TX_ALIGN_DELAY            (REG_TX_LL_CORE+0x3c)
#define TX_LL_CORE_LINKLAYER_IN_2_REG        (REG_TX_LL_CORE+0x40)
#define TX_LL_CORE_TX_ATEST_CTRL             (REG_TX_LL_CORE+0x44)
//F1000 REV B Specific - START
#define TX_LL_CORE_INJECT_TX_BIT_ERROR            (REG_TX_LL_CORE+0x48)
#define TX_LL_CORE_LANE0_TX_BIT_ERROR_MASK_DELAY  (REG_TX_LL_CORE+0x4c)
#define TX_LL_CORE_LANE1_TX_BIT_ERROR_MASK_DELAY  (REG_TX_LL_CORE+0x50)
#define TX_LL_CORE_LANE2_TX_BIT_ERROR_MASK_DELAY  (REG_TX_LL_CORE+0x54)
#define TX_LL_CORE_LANE3_TX_BIT_ERROR_MASK_DELAY  (REG_TX_LL_CORE+0x58)
#define TX_LL_CORE_LANE4_TX_BIT_ERROR_MASK_DELAY  (REG_TX_LL_CORE+0x5c)
#define TX_LL_CORE_LANE5_TX_BIT_ERROR_MASK_DELAY  (REG_TX_LL_CORE+0x60)
#define TX_LL_CORE_LANE6_TX_BIT_ERROR_MASK_DELAY  (REG_TX_LL_CORE+0x64)
#define TX_LL_CORE_LANE7_TX_BIT_ERROR_MASK_DELAY  (REG_TX_LL_CORE+0x68)
#define TX_LL_CORE_LANE_IF_CTRL                   (REG_TX_LL_CORE+0x6c)
//F1000 REV B Specific - END

#endif /* __TX_LL_CORE_HXX__ */