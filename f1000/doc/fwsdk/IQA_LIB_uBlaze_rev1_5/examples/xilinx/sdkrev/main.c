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
// File        : main.c                                                         *
// Author      : RL                                                             *
// Created     : 07/08/2019                                                     *
//                                                                              *
// Description : Application to exercise calling IQ-Analog library's APIs.      *
//                                                                              *
//*******************************************************************************

#include <stdio.h>

#include "iqa_f1000_api.h"

//===============================================================================
// Main
//===============================================================================

int main ( int argc, FAR char *argv[] )
{
    //=================================
    // Get SDK revision number
    //=================================

    printf("\r\nSDK Revision %s\r\n", iqa_get_revision());

    printf("\r\n*** End of Program ***\r\n");

    return IQA_SUCCESS;
}
