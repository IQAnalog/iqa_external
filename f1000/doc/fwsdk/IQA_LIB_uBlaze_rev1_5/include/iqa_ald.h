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
// File        : iqa_ald.h                                                      *
// Author      : RL                                                             *
// Created     : 05/15/2019                                                     *
//                                                                              *
// Description:  Include header file for IQ-Analog ALD file format.             *
//                                                                              *
//*******************************************************************************

#ifndef __INCLUDE_IQA_ALD_H
#define __INCLUDE_IQA_ALD_H

//===============================================================================
// Defines
//===============================================================================

#define FILE_SIGNATURE_LEN      12                  // Example: "IQA-ALD1.0 \n"
#define FILE_SIGNATURE_ROOT     "IQA-ALD"           // File signature string without version number
#define FILE_VERSION_NUMBER     "1.0 \n"
#define FILE_NOTE_HEADER        "Doc\n"

#define ADDRESS_LEN             sizeof(uint32_t)
#define DATA_LEN                sizeof(uint32_t)

#endif  // __INCLUDE_IQA_ALD_H
