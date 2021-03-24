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
// File        : iqa_assert.h                                                   *
// Author      : Tomas Gonzalez                                                 *
// Created     : 04/23/2020                                                     *
//                                                                              *
// Description:  Include header file for tracing runtime debug asserts.         *
//                                                                              *
//*******************************************************************************

#ifndef _IQA_ASSERT_H_
#define _IQA_ASSERT_H_

#ifndef IQA_ASSERT

#include <assert.h>
#include <stdio.h>

#define IQA_ASSERT(expr_) \
    do \
    { \
        if (!(expr_)) \
        { \
            printf("ASSERT failure at %s:%d\r\n", __FILE__, __LINE__); \
            assert(false); \
            while (1) { } \
        } \
    } while (0)

#endif // IQA_ASSERT

#endif // _IQA_ASSERT_H_

// vim: sw=2 ts=2 et cindent
