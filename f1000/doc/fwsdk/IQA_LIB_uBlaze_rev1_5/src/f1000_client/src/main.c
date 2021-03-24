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
// File        : main.c                                                       *
// Author      : KDP                                                            *
// Created     : 11/08/2018                                                     *
//                                                                              *
// Description:  F1000 client application for testing APIs and running          *
//               scripts.                                                       *
//                                                                              *
//*******************************************************************************

#define VERSION "1.2"

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
#include <sys/select.h>
#include <stdarg.h>

#include "client.h"

/*
================================================================================
Simple printf based client printf
================================================================================
*/
int client_stdout_printf(void *pCtx, const char *fmt, ...)
{
  va_list   ap;
  int       ret;

  va_start(ap, fmt);
  ret = vprintf(fmt, ap);
  va_end(ap);
  
  return ret;
}

/*
================================================================================
Main client entry point
================================================================================
*/
int main(int argc , char *argv[])
{
  int       ret;
  int       opt;
  char      *line;
  char      *server_ip = NULL;
  char      *script = NULL;
  int       port = 5000;
  int       debug = 0;
  int       quiet = 0;
  int       f1000_id = 1;
  client_t  client;

  /* Process arguments */
  while ((opt = getopt(argc, argv, "dhi:p:r:s:q")) != -1) 
  {
    switch (opt) 
    {
    // Server IP
    case 's':
      server_ip = optarg;
      break;

    // Run script
    case 'r':
      script = optarg;
      break;

    // Port number
    case 'p':
      port = atoi(optarg);
      break;

    // Debug mode
    case 'd':
      debug = 1;
      break;

    // Debug mode
    case 'i':
      f1000_id = atoi(optarg);
      break;

    // Quiet mode
    case 'q':
      quiet = 1;
      break;

    default: /* '?' */
      fprintf(stderr, "Usage: %s [-s server_ip] [-r script] [host] [script]\n",
              argv[0]);
      exit(EXIT_FAILURE);
    }
  }

  /* Set server IP with remaining arg */
  if (optind < argc && server_ip == NULL)
  {
    server_ip = argv[optind];
    optind++;
  }

  /* Set script with remaining arg */
  if (optind < argc && script == NULL)
    script = argv[optind];

  /* Test if we have a server IP */
  if (server_ip == NULL)
  {
    printf("Please provide the server IP\n");
    exit(EXIT_FAILURE);
  }

  /* Initialize the client structure */
  client.quit           = 0;
  client.f1000_id       = f1000_id;
  client.ep_id          = 1;
  client.server_ip      = server_ip;
  client.port           = port;
  client.sockfd         = 0;
  client.debug          = debug;
  client.quiet          = quiet;
  client.pVars          = NULL;
  client.pLocals        = NULL;
  client.pMem           = NULL;
  client.fd             = NULL;
  client.blockIndex     = 0;
  client.printf         = &client_stdout_printf;
  client.pPrintfCtx     = NULL;
  client.printfColors   = 0;

  client.scriptIndex    = 0;
  client.pActiveScript  = NULL;
  client.numScriptPath  = 0;

  client.pAuxCmdProc    = NULL;
  client.pAuxCtx        = NULL;
  client.pAuxVarGet     = NULL;
  client.pInputGet      = NULL;
  client.pInputCtx      = NULL;

  client_init_callbacks(&client);

  /* Connect to server */
  ret = client_open_connection(&client);
  if (ret != 0)
    return ret;

  printf("Client version %s\n", VERSION);

  /* Configure readline history */
  using_history();

  /* Test if we are running a script or interactive */
  if (script != NULL)
  {
    /* Run the specified script file */
    client_run_script(&client, script, 1, &script);
  }
  else
  {
    /* Loop until quit */
    while(!client.quit)
    {
      /* Get input line */
      line = readline("client> ");

      client.is_script = 0;
      client_process_line(&client, line, 0);
    }
  }

  if (client.debug)
    printf("closing socket\n");
  close(client.sockfd);
  f1000_regs_deinit(&client);
  return 0;
}

