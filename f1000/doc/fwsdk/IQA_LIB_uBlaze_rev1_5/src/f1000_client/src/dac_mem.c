//*******************************************************************************
//      __  __                                                                  *
//     /  \/  \                                                                 *
//         \   \                                                                *
//    I Q - A N A L O G                                                         *
//           \   \            IQ-Analog Corp                                    *
//            \__/\__/       www.iqanalog.com                                   *
//                                                                              *
// -----------------------------------------------------------------------------*
//                                                                              *
// Copyright (C) IQ-Analog Corp. All rights reserved.                           *
//                                                                              *
// -----------------------------------------------------------------------------*
// CONFIDENTIAL                                                                 *
// -----------------------------------------------------------------------------*
//                                                                              *
// File        : dac_mem.c                                                       *
// Author      : KDP                                                            *
// Created     : 11/08/2018                                                     *
//                                                                              *
// Description:  Routines for performing DAC memory write and playback
//                                                                              *
//*******************************************************************************

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <poll.h>
#include <fcntl.h>
#include <math.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "client.h"
#include "f1000_command_code.h"

static const char *g_dac_mem_help[] =
{
  "[1 | 2] samples wave_type [arg]",
  "",
  "    where wave_type is:",
  "       square",
  "       ramp",
  "       sine (arg=number of waves)",
  "       file (arg=filename)",
};

const unsigned int g_dac_mem_help_size =
  sizeof(g_dac_mem_help) / sizeof(g_dac_mem_help[0]);

/*
================================================================================
Print dac_mem usage.
================================================================================
*/
static void client_dac_mem_usage(client_t *pClient)
{
  int x;

  client_printf(pClient, "usage:\n");
  for (x = 0; x < g_dac_mem_help_size; ++x)
  {
    client_printf(pClient, "%s\n", g_dac_mem_help[x]);
  }
}

/*
================================================================================
Load DAC mem waveform from a file.
================================================================================
*/
static void client_dac_mem_file(client_t *pClient, char * filename, 
                                uint16_t *buf, int samples)
{
  //int   fd;
  int   i;

  if (filename == NULL)
  {
    client_printf(pClient, "Please provide source filename as final argument\n");
    return;
  }

  /* Open the file */
  //fd = open(filename, O_RDONLY);
  FILE* fd = fopen(filename,"ra");
  // if (fd == -1)
  if (fd == NULL)
  {
    client_printf(pClient, "Can't open file %s\n", filename);
    return;
  }

  /* Read data from the file */
  for (i = 0; i < samples; i++)
  {
    char line[256];
    if (fgets(line,256,fd) == NULL)
	    break;
    int x = atoi(line);
    buf[i] = x;
  }

  fclose(fd);
}

/*
================================================================================
Create square wave for dac_mem command
================================================================================
*/
static void client_dac_mem_square(client_t *pClient, char * wave_arg,
                                  uint16_t *buf, int samples)
{
  int n;
  int count = 0;
  uint16_t value = 0;
  int period = 128;

  /* Test if periodicity specified */
  if (wave_arg != NULL)
    period = atoi(wave_arg);

  /* Create a square wave with appropriate periodicity */
  for (n = 0; n < samples; n++)
  {
    /* Set the next sample value */
    *buf++ = value;

    /* Toggle high/low based on period */
    if (++count == period / 2)
    {
      value ^= 0xFF;
      count = 0;
    }
  }
}

/*
================================================================================
Create sine wave for dac_mem command
================================================================================
*/
static void client_dac_mem_sine(client_t *pClient, char * wave_arg,
                                uint16_t *buf, int samples)
{
  double    pi = 3.141592653578;
  double    stepper = 2.0 * pi / samples;
  double    step = 0.0;
  double    scale = 0.90;
  int       full_scale = 127;
  int       offset = 128;
  int16_t   value = 0;
  int       waves = 1;
  int       report_once = 1;
  int       n;
  FILE		*fd;

  /* Test if wave count specified */
  if (wave_arg != NULL)
    waves = atoi(wave_arg);

  char filename[256];
  double cf = waves * 48000.0 / samples;
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  char date[128];
  strftime(date,sizeof(date)-1, "%d_%m_%Y_%H:%M",t);
  sprintf(filename,"DACDATA_%.3f_MHz_%s.txt",cf,date);

  /* Create a square wave with appropriate periodicity */
  fd = fopen(filename, "w");
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
        client_printf(pClient, "Waveform clipped\n");
        report_once = 0;
      }
    }

    /* Write the sample to the buffer */
    *buf++ = value;

    if (fd != NULL)
	    fprintf(fd, "%d\n", value);
    
    /* Update the step */
    step += stepper * waves;
  }
  fclose(fd);
}

/*
=============================================================================
Generates a sine wave at the desired center frequency
=============================================================================
*/
static int client_dac_mem_cf(client_t *pClient, char * wave_arg,
    uint16_t* buf, int samples)
{
  double    DAC_RATE = 48000.0; /* 48GSPS or 48000MSPS */
  double    cf       = atoi(wave_arg);
  double    ns       = cf * samples / DAC_RATE;
  int       num_sine = round(ns);
  char      new_wave_arg[256];

  sprintf(new_wave_arg, "%d", num_sine);
  client_printf(pClient, "DAC_MEM_CF is generating %f MHz as %d sine waves\n",
      cf, num_sine);
  client_dac_mem_sine(pClient, new_wave_arg, buf, samples);
  return 0;
}
/*
================================================================================
Create two-tone sine wave for dac_mem command
================================================================================
*/
static int client_dac_mem_twotone(client_t *pClient, char * wave_arg,
                                   uint16_t *buf, int samples)
{
  double    pi = 3.141592653578;
  double    stepper = 2.0 * pi / samples;
  double    step1 = 0.0;
  double    scale1 = 0.65;
  double    step2 = 0.0;
  double    scale2 = 0.30;
  int       full_scale = 127;
  int       offset = 128;
  uint16_t  value = 0;
  int       waves1 = 1;
  int       waves2 = 4;
  int       report_once = 1;
  int       n;
  char      *token;

  /* Test if wave count specified */
  if (wave_arg != NULL)
  {
    /* Tokenize the argument ... it should have 2 values */
    if (strchr(wave_arg, ',') == NULL)
    {
      client_printf(pClient, "Please specify argument as 'count1,count2'\n");
      return -EINVAL;
    }

    token = strtok(wave_arg, ",");
    waves1 = atoi(token);
    token = strtok(NULL, ",");
    waves2 = atoi(token);
  }

  /* Create a square wave with appropriate periodicity */
  for (n = 0; n < samples; n++)
  {
    /* Calculate next value */
    value = (uint16_t) (sin(step1) * full_scale * scale1 + 
                        sin(step2) * full_scale * scale2 + 
                        offset);
    if (value > 255)
    {
      /* Clip the value */
      value = 255;

      if (report_once)
      {
        client_printf(pClient, "Waveform clipped\n");
        report_once = 0;
      }
    }

    /* Write the sample to the buffer */
    *buf++ = value;
    
    /* Update the steps */
    step1 += stepper * waves1;
    step2 += stepper * waves2;
  }

  return 0;
}

/*
================================================================================
dac_mem command
================================================================================
*/
void client_dac_mem(client_t *pClient, int argc, char *argv[])
{
    uint32_t  *ptr;
    int       n;
    int       tx_size;
    int       tx_sent;
    int       rx_expected;
    int       pkt_count;
    uint32_t  dac_id;
    int       samples;
    char      *wave_arg = NULL;
    uint16_t  *buf;
    char      *wave_type;

   /* Validate the arguments */
    if (argc < 4)
    {
      client_dac_mem_usage(pClient);
      return;
    }

    /* Test if we have an open connection */
    if (pClient->sockfd == 0)
    {
      client_printf(pClient, "Please open a connection first\n");
      return;
    }

    memset(pClient->rxdbuff, 0, sizeof(pClient->rxdbuff));
    memset(pClient->txdbuff, 0, sizeof(pClient->txdbuff));
    ptr = (uint32_t *) pClient->txdbuff;

    /* Copy API and f1000/ep id to txdbuff */
    *ptr++ = F1000_DAC_DSP_LOAD_WAVEFORM;
    *ptr++ = pClient->f1000_id;
    *ptr++ = pClient->ep_id;

    /* Process arguments */
    dac_id  = atoi(argv[1]);         // ADC ID
    samples = atoi(argv[2]);         // Sample count
    wave_type = argv[3];             // Waveform type
    if (argc >= 5)
      wave_arg = argv[4];

    /* Add API parameters */
    *ptr++ = dac_id;
    *ptr++ = samples;

    /* Fill the tx buffer */
    buf = (uint16_t *) ptr;
    if (strcmp(wave_type, "file") == 0)
    {
      /* Load from the specified file */
      client_dac_mem_file(pClient, wave_arg, buf, samples);
    }
    else if (strcmp(wave_type, "square") == 0)
    {
      client_dac_mem_square(pClient, wave_arg, buf, samples);
    }
    else if (strcmp(wave_type, "sine") == 0 || strcmp(wave_type, "sin") == 0)
    {
      client_dac_mem_sine(pClient, wave_arg, buf, samples);
    }
    else if (strcmp(wave_type, "cf") == 0)
    {
      client_dac_mem_cf(pClient,wave_arg,buf,samples);
    }
    else if (strcmp(wave_type, "twotone") == 0 || strcmp(wave_type, "two_tone") == 0)
    {
      if (client_dac_mem_twotone(pClient, wave_arg, buf, samples) != 0)
        return;
    }
    else if (strcmp(wave_type, "ramp") == 0)
    {
      /* Create simple ramp pattern */
      for (n = 0; n < samples; n++)
      {
        *buf++ = (uint8_t) n;
      }
    }
    else
    {
      client_dac_mem_usage(pClient);
      return;
    }

    tx_size = 20 + samples * 2;
    tx_sent = 0;
    pkt_count = 0;
    client_printf(pClient, "Sending %d bytes\r\n", tx_size);

    /* Send them 512 bytes at a time */
    while (tx_sent < tx_size)
    {
      int bytes_to_send = tx_size-tx_sent;
      int subsend = 0;

      if (bytes_to_send > gMaxPktSize)
        bytes_to_send = gMaxPktSize;

      while (subsend < bytes_to_send)
      {
        n = write(pClient->sockfd, &pClient->txdbuff[tx_sent+subsend], 
            bytes_to_send-subsend);
        if (n <= 0)
        {
          client_printf(pClient, "Error transmitting to server: %s\n", strerror(errno));
          return;
        }
        subsend += n;

        if ((++pkt_count) % 8 == 0)
          usleep(100000);
      }

      tx_sent += bytes_to_send;
    }

    /* Perform read from server */
    rx_expected = 8;
    client_read(pClient, rx_expected);

    /* Test if we should play the waveform */
    if ((wave_arg != NULL && strcmp(wave_arg, "play") == 0) ||
        (argc == 6 && strcmp(argv[5], "play") == 0))
    {
      /* Call the routine to start the playback */

      client_dac_play(pClient, argc, argv);
    }
}

/*
================================================================================
dac_play command
================================================================================
*/
void client_dac_play(client_t *pClient, int argc, char *argv[])
{
    uint32_t  *ptr;
    int       tx_size;
    int       n;
    int       rx_expected;
    uint32_t  dac_id;
    uint32_t  samples;

    /* Test if we have an open connection */
    if (pClient->sockfd == 0)
    {
      client_printf(pClient, "Please open a connection first\n");
      return;
    }

    /* Validate the arguments */
    if (argc < 3)
    {
      client_printf(pClient, "usage: dac_play [1 | 2] samples\n");
      return;
    }

    memset(pClient->rxdbuff, 0, sizeof(pClient->rxdbuff));
    memset(pClient->txdbuff, 0, sizeof(pClient->txdbuff));
    ptr = (uint32_t *) pClient->txdbuff;

    /* Copy API and f1000/ep id to txdbuff */
    *ptr++ = F1000_DAC_DSP_START_PLAYBACK;
    *ptr++ = pClient->f1000_id;
    *ptr++ = pClient->ep_id;

    /* Process arguments */
    dac_id  = atoi(argv[1]);         // ADC ID
    samples = atoi(argv[2]);         // Sample count

    /* Add API parameters */
    *ptr++ = dac_id;
    *ptr++ = samples;

    /* Write to the socket */
    tx_size = (char *) ptr - &pClient->txdbuff[0];
    n = write(pClient->sockfd, pClient->txdbuff, tx_size);
    if (n < 0)
    {
      client_printf(pClient, "Error transmitting to server: %s\n", strerror(errno));
      return;
    }

    /* Perform read from server */
    rx_expected = 8;
    client_read(pClient, rx_expected);
}
