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
// File        : gen_sine_wave.c                                                *
// Author      : ND                                                             *
// Created     : 06/20/2019                                                     *
//                                                                              *
// Description : Generate a sine wave for the desired frequency.                *
//                                                                              *
//*******************************************************************************

#include <stdio.h>
#include <math.h>

void gen_sine_wave(double cf, uint16_t *buf, int samples)
{
  double    DAC_RATE = 48000.0; /* 48GSPS or 48000MSPS */
  double    ns       = cf * samples / DAC_RATE;
  int       num_sine = round(ns);
  double    pi = 3.141592653589793;
  double    stepper = 2.0 * pi / samples;
  double    step = 0.0;
  double    scale = 0.90;
  int       full_scale = 127;
  int       offset = 128;
  int16_t   value = 0;
  int       waves = num_sine;
  int       report_once = 1;
  int       n;

  /* Create a square wave with appropriate periodicity */
  for (n = 0; n < samples; n++)
  {
    /* Calculate next value */
    value = (uint16_t) (sin(step) * full_scale * scale + offset);
    if (value > 255 || value < 0)
    {
      /* Clip the value */
      value = value > 255 ? 255 : 0;

      if (report_once)
      {
        printf("Waveform clipped\n");
        report_once = 0;
      }
    }

    /* Write the sample to the buffer */
    *buf++ = value;

    /* Update the step */
    step += stepper * waves;
  }
}

