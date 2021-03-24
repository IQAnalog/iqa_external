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
// File        : client.c                                                       *
// Author      : KDP                                                            *
// Created     : 11/08/2018                                                     *
//                                                                              *
// Description:  F1000 client application for testing APIs and running          *
//               scripts.                                                       *
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
#include <sys/select.h>
#include <inttypes.h>

#include "client.h"
#include "f1000_command_code.h"

const int gMaxPktSize = 2048;

#ifndef offsetof
#define offsetof(type, md) ((unsigned long)&((type *)0)->md)
#endif

#ifndef container_of
#define container_of(ptr, type, member) \
((type *)((char *)(ptr) - offsetof(type, member)))
#endif

/*
================================================================================
Local data prototypes
================================================================================
*/
typedef void (*client_func_t)(client_t *pClient, int argc, char *argv[]);

typedef struct
{
  const char      *name;
  client_func_t   func;
  int             is_script;
  const char      *help;
} client_cmd_t;

typedef int (*client_var_func_t)(client_t *, const char *name, script_var_t *);

typedef struct
{
    const char          *name;
    client_var_func_t   func;
} client_var_accessor_t;

typedef struct client_file_io_s
{
  client_io_t io;
  FILE        *fp;
} client_file_io_t;

typedef struct client_mem_io_s
{
  client_io_t io;
  char        *pData;
  int         pos;
  int         size;
} client_mem_io_t;

/*
================================================================================
Function prototypes
================================================================================
*/

static void  client_adc_mem(client_t *pClient, int argc, char *argv[]);
static void  client_adc_cap(client_t *pClient, int argc, char *argv[]);
static void  client_call(client_t *pClient, int argc, char *argv[]);
static void  client_cap_once(client_t *pClient, int argc, char *argv[]);
static void  client_disp(client_t *pClient, int argc, char *argv[]);
static void  client_echo(client_t *pClient, int argc, char *argv[]);
static void  client_else(client_t *pClient, int argc, char *argv[]);
static void  client_elseif(client_t *pClient, int argc, char *argv[]);
static void  client_ep(client_t *pClient, int argc, char *argv[]);
static void  client_end(client_t *pClient, int argc, char *argv[]);
static void  client_endfun(client_t *pClient, int argc, char *argv[]);
static void  client_endif(client_t *pClient, int argc, char *argv[]);
static void  client_fclose(client_t *pClient, int argc, char *argv[]);
static void  client_fopen(client_t *pClient, int argc, char *argv[]);
static void  client_for(client_t *pClient, int argc, char *argv[]);
static void  client_function(client_t *pClient, int argc, char *argv[]);
static void  client_get_cap(client_t *pClient, int argc, char *argv[]);
void         client_help(client_t *pClient, int argc, char *argv[]);
static void  client_if(client_t *pClient, int argc, char *argv[]);
static void  client_logger(client_t *pClient, int argc, char *argv[]);
static void  client_mem(client_t *pClient, int argc, char *argv[]);
static void  client_rfsynth(client_t *pClient, int argc, char *argv[]);
static void  client_reg(client_t *pClient, int argc, char *argv[]);
static void  client_repeat(client_t *pClient, int argc, char *argv[]);
static void  client_run(client_t *pClient, int argc, char *argv[]);
static void  client_system(client_t *pClient, int argc, char *argv[]);
static void  client_set(client_t *pClient, int argc, char *argv[]);
static void  client_print(client_t *pClient, int argc, char *argv[]);
static void  client_quit(client_t *pClient, int argc, char *argv[]);
static void  client_pause(client_t *pClient, int argc, char *argv[]);
static void  client_var(client_t *pClient, int argc, char *argv[]);
static void  client_while(client_t *pClient, int argc, char *argv[]);
static void  client_initchip(client_t *pClient, int argc, char *argv[]);

static int   client_var_acc_defined(client_t *pClient, const char *name, script_var_t *pRes);
static int   client_var_acc_undefined(client_t *pClient, const char *name, script_var_t *pRes);

static void  client_file_io_seek(client_io_t *io, int pos);
static int   client_file_io_tell(client_io_t *io);
static char *client_file_io_gets(client_io_t *io, char *line, int size);
static int   client_file_io_read(client_io_t *io, char *buf, int size);
static int   client_file_io_write(client_io_t *io, const char *buf, int size);

static void  client_mem_io_seek(client_io_t *io, int pos);
static int   client_mem_io_tell(client_io_t *io);
static char *client_mem_io_gets(client_io_t *io, char *line, int size);
static int   client_mem_io_read(client_io_t *io, char *buf, int size);
static int   client_mem_io_write(client_io_t *io, const char *buf, int size);

static void  client_del_function(client_t *pClient, script_fn_t *pFn);
static void  client_call_function(client_t *pClient, script_var_t *pResult,
    int argc, char *argv[]);

static void  client_input_function(client_t *pClient, script_var_t *pResult,
    int argc, char *argv[]);

static int   client_parse_args(char *mutable_line, char **cmd_argv, int cmd_argmax);
static void  client_run_io_script(client_t *pClient, const char *script,
    script_var_t *pParams, client_io_t *io);


//static script_var_t * client_find_var(client_t *pClient, const char *name);

const char g_init_chip_help[] = "<rfsynthmask> (0 = None, 1 = RF Synth1,2 = RF Synth2 ,3 = RF Synth1&2)\n"
                                "<rfsynthport> (1 = Port A, 2 = Port B)\n"
                                "<adcmask> (0 = None,1 = ADC1,2 = ADC2 ,3 = ADC1&2)\n"
                                "<dacmask> (0 = None ,1 = DAC1,2 = DAC2 ,3 = DAC1&2)\n"
                                "fclk [freq in Mhz]\n";

/*
================================================================================
Private data
================================================================================
*/
static const client_cmd_t g_cmds[] =
{
  { "adc_cap",      &client_adc_cap,      0, "initiate ADC capture" },
  { "adc_mem",      &client_adc_mem,      0, "read ADC memory" },
  { "call",         &client_call,         1, "call function" },
  { "cap_once",     &client_cap_once,     0, "capture DAC waveform once" },
  { "disp",         &client_disp,         0, "display messages to the console" },
  { "dac_mem",      &client_dac_mem,      0, "write DAC memory" },
  { "dac_play",     &client_dac_play,     0, "initiate DAC playback" },
  { "echo",         &client_echo,         1, "echo string"  },
  { "elif",         &client_elseif,       1, "elseif condition"  },
  { "else",         &client_else,         1, "else condition"  },
  { "elseif",       &client_elseif,       1, "elseif condition"  },
  { "ep",           &client_ep,           0, "set the EP"  },
  { "end",          &client_end,          1, "terminate a loop" },
  { "endfun",       &client_endfun,       1, "terminate a function" },
  { "endif",        &client_endif,        1, "terminate a loop" },
  { "fclose",       &client_fclose,       1, "close a file" },
  { "fopen",        &client_fopen,        1, "open a file for write" },
  { "for",          &client_for,          1, "execute a for loop" },
  { "function",     &client_function,     1, "define a function" },
  { "get_cap",      &client_get_cap,      0, "capture DAC waveform once" },
  { "help",         &client_help,         0, "provide help"  },
  { "if",           &client_if,           1, "conditional execution" },
  { "init_chip",    &client_initchip,     0, "initialize f1000 chip and sub systems" },
  { "local",        &client_var,          1, "set a script variable value" },
  { "log",          &client_logger,       0, "log all server APIs to file"  },
  { "mem",          &client_mem,          0, "define a raw access byte access memory"  },
  { "mem16",        &client_mem,          0, "define a raw access 16-bit access memory"  },
  { "mem32",        &client_mem,          0, "define a raw access 32-bit access memory"  },
  { "?",            &client_print,        0, "print reg by name"  },
  { "p",            &client_print,        0, "print reg by name"  },
  { "p/x",          &client_print,        0, "print reg by name"  },
  { "print",        &client_print,        0, "print reg by name"  },
  { "pause",        &client_pause,        1, "pause the specified number of seconds"  },
  { "quit",         &client_quit,         0, "quit the program"  },
  { "q",            &client_quit,         0, "quit the program"  },
  { "reg",          &client_reg,          0, "read/write register value"  },
  { "reg16",        &client_reg,          0, "read/write register value"  },
  { "repeat",       &client_repeat,       0, "repeat execute a command"  },
  { "rfsynth",      &client_rfsynth,      0, "read or write rfsynth registers"  },
  { "run",          &client_run,          0, "run a script"  },
  { "s",            &client_set,          0, "set reg by name"  },
  { "set",          &client_set,          0, "set reg by name"  },
  { "system",       &client_system,       0, "run 'system' command on target"  },
  { "var",          &client_var,          1, "set a script variable value" },
  { "while",        &client_while,        1, "execute a while loop" },
};

static const int g_cmd_count = sizeof(g_cmds) / sizeof(client_cmd_t);

static const client_var_accessor_t g_var_accessors[] =
{
  { "defined",      &client_var_acc_defined },
  { "undefined",    &client_var_acc_undefined },
};

static const int g_var_accessor_count =
    sizeof(g_var_accessors) / sizeof(client_var_accessor_t);

static const client_io_vptbl_t client_file_io_vptbl =
{
  client_file_io_seek,
  client_file_io_tell,
  client_file_io_gets,
  client_file_io_read,
  client_file_io_write,
};

static const client_io_vptbl_t client_mem_io_vptbl =
{
  client_mem_io_seek,
  client_mem_io_tell,
  client_mem_io_gets,
  client_mem_io_read,
  client_mem_io_write,
};

/*
================================================================================
io seek wrapper
================================================================================
*/
static inline void client_io_seek(client_io_t *io, int pos)
{
  io->vptbl->seek(io, pos);
}

/*
================================================================================
io tell wrapper
================================================================================
*/
static inline int client_io_tell(client_io_t *io)
{
  return io->vptbl->tell(io);
}

/*
================================================================================
io gets wrapper
================================================================================
*/
static inline char *client_io_gets(client_io_t *io, char *line, int size)
{
  return io->vptbl->gets(io, line, size);
}

/*
================================================================================
io read wrapper
================================================================================
*/
static inline int client_io_read(client_io_t *io, char *buf, int size)
{
  return io->vptbl->read(io, buf, size);
}

/*
================================================================================
file io seek
================================================================================
*/
static void client_file_io_seek(client_io_t *io, int pos)
{
  FILE *fp = container_of(io, client_file_io_t, io)->fp;
  fseek(fp, pos, SEEK_SET);
}

/*
================================================================================
file io tell
================================================================================
*/
static int client_file_io_tell(client_io_t *io)
{
  FILE *fp = container_of(io, client_file_io_t, io)->fp;
  return ftell(fp);
}

/*
================================================================================
file io gets
================================================================================
*/
static char *client_file_io_gets(client_io_t *io, char *line, int size)
{
  FILE *fp = container_of(io, client_file_io_t, io)->fp;
  return fgets(line, size, fp);
}

/*
================================================================================
file io read
================================================================================
*/
static int client_file_io_read(client_io_t *io, char *buf, int size)
{
  FILE *fp = container_of(io, client_file_io_t, io)->fp;
  return fread(buf, 1, size, fp);
}

/*
================================================================================
file io write
================================================================================
*/
static int client_file_io_write(client_io_t *io, const char *buf, int size)
{
  FILE *fp = container_of(io, client_file_io_t, io)->fp;
  return fwrite(buf, 1, size, fp);
}

/*
================================================================================
mem io seek
================================================================================
*/
static void client_mem_io_seek(client_io_t *io, int pos)
{
  client_mem_io_t *mem = container_of(io, client_mem_io_t, io);
  if (pos < 0)
    pos = 0;
  if (pos > mem->size)
    pos = mem->size;
  mem->pos = pos;
}

/*
================================================================================
mem io tell
================================================================================
*/
static int client_mem_io_tell(client_io_t *io)
{
  client_mem_io_t *mem = container_of(io, client_mem_io_t, io);
  return mem->pos;
}

/*
================================================================================
mem io gets
================================================================================
*/
static char *client_mem_io_gets(client_io_t *io, char *line, int size)
{
  client_mem_io_t *mem = container_of(io, client_mem_io_t, io);
  int pos = 0;
  int ch = 0;

  while (ch != '\n' && mem->pos < mem->size && pos + 1 < size)
  {
    ch = mem->pData[mem->pos++];
    line[pos++] = ch;
  }

  if (pos == 0 && mem->pos == mem->size)
    return NULL;

  line[pos] = 0;

  return line;
}

/*
================================================================================
mem io read
================================================================================
*/
static int client_mem_io_read(client_io_t *io, char *buf, int size)
{
  client_mem_io_t *mem = container_of(io, client_mem_io_t, io);
  int rem;

  rem = mem->size - mem->pos;
  if (size > rem)
    size = rem;

  if (size > 0)
  {
    memcpy(buf, mem->pData + mem->pos, size);
    mem->pos += size;
  }

  return size;
}

/*
================================================================================
mem io write
================================================================================
*/
static int client_mem_io_write(client_io_t *io, const char *buf, int size)
{
  client_mem_io_t *mem = container_of(io, client_mem_io_t, io);
  int rem;

  rem = mem->size - mem->pos;
  if (size > rem)
    size = rem;

  if (size > 0)
  {
    memcpy(mem->pData + mem->pos, buf, size);
    mem->pos += size;
  }

  return size;
}

/*
================================================================================
hextoi
================================================================================
*/
int64_t hextoi(char *str)
{
  int64_t ret;
  int     is_hex;
  int     x, len;

  is_hex = 0;
  if (strncmp(str, "0x", 2) == 0)
    sscanf(&str[2], "%lx", &ret);
  else
  {
    len = strlen(str);
    for (x = 0; x < len; x++)
    {
      if ((str[x] >= 'A' && str[x] <= 'F') ||
          (str[x] >= 'a' && str[x] <= 'f'))
      {
        is_hex = 1;
        break;
      }
    }

    if (is_hex)
      sscanf(str, "%lx", &ret);
    else
      ret = atoi(str);
  }

  return ret;
}

/*
================================================================================
Put unordered samples back in ordered

sampples: Ttoal samples in the buffer to be reordered.
chunk_size: This MUST be the same size as each read from f1000_server's request
            until samples size is reached. This is important since such
            chunk size affects how the memory is being read thus reordering
            needs to be done with the same size.
            Currently the request size from f1000_server is 256 bytes.
================================================================================
*/
static void adc_ordermem(client_t *pClient, int samples, int chunk_size)
{
  FAR uint32_t   *dptr = NULL;
  int            bytes_per_sram;
  int            bytes_per_word;
  int            num_srams = 16;
  int            s, x, r;
  FAR char       *buffer;
  int            running_samples = samples;
  int            offset = 0;
  int            num_rows;
  int            pong;
  int            sram_off = 0;

  /* Need a temporary buffer for remapping */
  buffer = ( FAR char * ) calloc ( samples * 2, sizeof ( char ) );

  if ( buffer == NULL )
  {
    client_printf(pClient, "Error allocating memory");
    return;
  }

  /* skip first 8 chars for command id and status */
  memcpy((void *)buffer, (const void *)pClient->rxdbuff+8, (size_t)samples*2);

  /* Points to destination buffer */
  dptr = ( uint32_t * )(pClient->rxdbuff + 8);

  num_rows = chunk_size / ( num_srams * 8 );

  /*
  If there are samples that fit in less than a row , then allocate
  1 row. We are not dealing with partial rows here.
  */
  if ( ( samples > 0 ) && ( num_rows == 0 ) )
    num_rows = 1;

  /* Calculate size per sram and word */
  bytes_per_sram = ( chunk_size * 2 ) >> 4;
  bytes_per_word = bytes_per_sram >> 2;

  if ( ( bytes_per_sram > 0 ) && ( bytes_per_word == 0 ) )
    bytes_per_word = 1;

  /* Reorder chunk_size data per loop until all samples are processed */
  while (running_samples > 0)
  {
      /* Loop for all rows */
      for ( r = 0; r < num_rows; r++ )
      {
        /* Loop for ping-pong */
        for ( pong = 2; pong >= 0; pong -= 2 )
        {
          /* Loop for all srams */
          for ( sram_off = 0, s = 0; s < num_srams; s++ )
          {
            /* Write Word 0 */
            *dptr++ = * ( ( uint32_t * ) &buffer[offset + sram_off + pong * bytes_per_word + r * 4] );

            /* Write Word 1 */
            *dptr++ = * ( ( uint32_t * ) &buffer[offset + sram_off + ( pong + 1 ) * bytes_per_word + r * 4] );

            sram_off += bytes_per_sram;
          }
        }
      }
      running_samples -= chunk_size;
      offset          += chunk_size * 2;
  }
  free(buffer);

  /* For ADC memory, two 10-bit samples are packed in the
   * lower 20 bits of a 32-bit word.  We must unpack them.
   */
  dptr = ( uint32_t * ) (char *)(pClient->rxdbuff + 8);

  for ( x = 0; x < samples / 2; x++ )
  {
    uint32_t temp;

    /* Reorder the samples */
    temp = *dptr;
    *dptr = ( temp & ((uint32_t)0x3ff) ) | ( ( ( temp & ((uint32_t)0x000FFC00 )) << 6) & 0xFFFF0000 );
    dptr++;
  }
}

/*
================================================================================
common read from socket routine
================================================================================
*/
int client_read(client_t *pClient, int rx_expected)
{
  int    rx_size;
  int    n;
  struct pollfd pfd;

  if (pClient->debug && !pClient->quiet)
    client_printf(pClient, "waiting for read data from server..\n");
  rx_size    = 0;
  pfd.fd     = pClient->sockfd;
  pfd.events = POLLIN;
  while (rx_size < rx_expected)
  {
    if (poll(&pfd, 1, 50000) == 0)
    {
      if (!pClient->quiet)
        client_printf(pClient, "ERROR: read timed out.  Received %d bytes, expected %d (%d diff)\n",
            rx_size, rx_expected, rx_expected-rx_size);
      return rx_size;
    }

    n = read(pClient->sockfd, &pClient->rxdbuff[rx_size], sizeof(pClient->rxdbuff)-rx_size);
    if (n <= 0)
    {
      if (!pClient->quiet)
        client_printf(pClient, "Error reading data from socket");
      close(pClient->sockfd);
      pClient->sockfd = 0;
      return rx_size;
    }

    /* Accumulate data */
    rx_size += n;
  }

  if (pClient->debug && !pClient->quiet)
      client_printf(pClient, "Have API reply data from F1000 server %d bytes\n", rx_size);
  return rx_size;
}

/*
================================================================================
common read from socket routine
================================================================================
*/
int client_read_exact(client_t *pClient, int rx_expected)
{
  int    rx_size;
  int    n;
  struct pollfd pfd;

  if (pClient->debug && !pClient->quiet)
    client_printf(pClient, "waiting for read data from server..\n");
  rx_size    = 0;
  pfd.fd     = pClient->sockfd;
  pfd.events = POLLIN;
  while (rx_size < rx_expected)
  {
    if (poll(&pfd, 1, 50000) == 0)
    {
      if (!pClient->quiet)
        client_printf(pClient, "ERROR: read timed out.  Received %d bytes, expected %d (%d diff)\n",
            rx_size, rx_expected, rx_expected-rx_size);
      return rx_size;
    }

    n = read(pClient->sockfd, &pClient->rxdbuff[rx_size], rx_expected-rx_size);
    if (n < 0)
    {
      if (!pClient->quiet)
        perror("Error reading data from socket");
      return rx_size;
    }

    /* Accumulate data */
    rx_size += n;
  }

  if (pClient->debug && !pClient->quiet)
    client_printf(pClient, "Have API reply data from F1000 server %d bytes\n", rx_size);
  return rx_size;
}

/*
================================================================================
Quit command
================================================================================
*/
static void client_quit(client_t *pClient, int argc, char *argv[])
{
  if (argc < 1)
  {
    client_printf(pClient, "usage: %s\n", argv[0]);
    return;
  }

  pClient->quit = 1;
}

/*
================================================================================
EP command
================================================================================
*/
static void client_ep(client_t *pClient, int argc, char *argv[])
{
  char    str[64];

  /* Test if we should execute based on if processing */
  if (!ifproc_should_process(&pClient->ifProc))
    return;

  if (argc < 2)
  {
    client_printf(pClient, "usage: ep [a|b]\n");
    client_printf(pClient, "   or: ep push [a|b]\n");
    client_printf(pClient, "   or: ep pop\n");
    return;
  }

  if (strcmp(argv[1], "push") == 0 && argc == 3)
  {
    argv++;
    pClient->epStack = (pClient->epStack << 1) | (pClient->ep_id - 1);
  }
  else if (strcmp(argv[1], "pop") == 0)
  {
    pClient->ep_id = (pClient->epStack & 1) + 1;
    pClient->epStack >>= 1;
    return;
  }

  if (strcmp(argv[1], "b") == 0 || strcmp(argv[1], "2") == 0)
    pClient->ep_id = 2;
  else if (strcmp(argv[1], "a") == 0 || strcmp(argv[1], "1") == 0)
    pClient->ep_id = 1;
  else
    client_printf(pClient, "ep: invalid use; check usage\n");

  /* If this is a TCL connection type, then tell TCL script the EP */
  if (pClient->port == 5001)
  {
    sprintf(str, "e %d\r\n", pClient->ep_id);
    write(pClient->sockfd, str, strlen(str));
  }
}

/*
================================================================================
rfsynth command
================================================================================
*/
static void client_rfsynth(client_t *pClient, int argc, char *argv[])
{
  int       rf_id;
  uint32_t  rf_addr;
  uint32_t  rf_val;
  uint32_t  *ptr;
  uint32_t  tx_size;
  uint32_t  rx_size;

  /* Test if we should execute based on if processing */
  if (!ifproc_should_process(&pClient->ifProc))
    return;

  if (argc < 3)
  {
    client_printf(pClient, "usage: rfsynth [1|2] addr\r\n");
    client_printf(pClient, "   or: rfsynth [1|2] addr value\r\n");
    return;
  }

  /* Test if we have an open connection */
  if (pClient->sockfd == 0)
  {
    client_printf(pClient, "Please open a connection first\n");
    return;
  }

  ptr = (uint32_t *) pClient->txdbuff;

  /* Get RFsynth id and address */
  rf_id = atoi(argv[1]);
  rf_addr = atoi(argv[2]);

  /* Copy API and f1000/ep id to txdbuff */
  if (argc == 3)
    *ptr++ = IQA_READ_RF_SYNTHESIZER;
  else
    *ptr++ = IQA_WRITE_RF_SYNTHESIZER;
  *ptr++ = rf_id;
  *ptr++ = rf_addr;

  tx_size = 12;
  rx_size = 8;
  if (argc == 3)
  {
    rf_val = atoi(argv[3]);
    *ptr++ = rf_val;
    tx_size = 16;
    rx_size = 12;
  }

  /* Write to the socket */
  write(pClient->sockfd, pClient->txdbuff, tx_size);

  /* Read response */
  client_read(pClient, rx_size);
}

/*
================================================================================
F1000 init_chip:  This initializes the analog portions of the chip.

  bit 0-3   : RF Synthesizer Mask
  bit 4-7   : RF Synthesizer Port
  bit 8-15  : DAC Mask
  bit 16-23 : ADC Mask
================================================================================
*/
static void client_initchip(client_t *pClient, int argc, char *argv[])
{
  uint32_t  init_mask=0;
  int       fclk;
  uint32_t  tx_size;
  uint32_t  rx_size;
  uint32_t  *ptr;

  /* Validate the number of arguments */
  if (argc < 5)
  {
    client_printf(pClient, "%s", g_init_chip_help);
    return;
  }

  tx_size = 20;
  rx_size = 8;

  /* Populate the TX buffer */
  ptr = (uint32_t *) pClient->txdbuff;

  *ptr++ = IQA_INIT_CHIP;
  *ptr++ = pClient->f1000_id;
  *ptr++ = pClient->ep_id;

  /* Set the init mask accordingly */
  // rfSynth
  if (atoi(argv[1]) & 0x1)
  {
    INIT_RFSYNTH1(init_mask);
  }
  if (atoi(argv[1])& 0x2)
  {
    INIT_RFSYNTH2(init_mask);
  }

  /* RFSynth Port */
  if (atoi(argv[2]) & 0x1)
  {
    SELECT_RFSYNTH_PORTA(init_mask);
  }
  if (atoi(argv[2])& 0x2)
  {
    SELECT_RFSYNTH_PORTB(init_mask);
  }

  /* ADC */
  if (atoi(argv[3] )& 0x1)
  {
    INIT_ADC1(init_mask);
  }
  if (atoi(argv[3] )& 0x2)
  {
    INIT_ADC2(init_mask);
  }

  /* DAC */
  if (atoi(argv[4] )& 0x1)
  {
    INIT_DAC1(init_mask);
  }
  if (atoi(argv[4])& 0x2)
  {
    INIT_DAC2(init_mask);
  }

  *ptr++ = init_mask;

  /* fclk in Mhz */
  fclk = atoi(argv[5]);
  *ptr++ = fclk;

  /* Write to the socket */
  write(pClient->sockfd, pClient->txdbuff, tx_size);

  /* Read response */
  client_read(pClient, rx_size);

  return;
}
/*
================================================================================
Pause
================================================================================
*/
static void client_pause(client_t *pClient, int argc, char *argv[])
{
  int     pause_period;

  /* Test if we should execute based on if processing */
  if (!ifproc_should_process(&pClient->ifProc))
    return;

  if (argc < 2)
  {
    client_printf(pClient, "usage: pause msec\n");
    return;
  }

  /* Get pause perod */
  pause_period = atof(argv[1]) * 1000;

  usleep(pause_period);
}

/*
================================================================================
Repeat execute a command
================================================================================
*/
static void client_repeat(client_t *pClient, int argc, char *argv[])
{
  int   repeat_count;
  int   x, c;
  int   is_script;

  /* Test if we should execute based on if processing */
  if (!ifproc_should_process(&pClient->ifProc))
    return;

  if (argc < 3)
  {
    client_printf(pClient, "usage: repeat count cmd [args...]\n");
    return;
  }

  /* Get repeat count */
  repeat_count = atoi(argv[1]);

  for (x = 2; x < argc; x++)
    argv[x-2] = argv[x];
  argc -= 2;

  /* Set script mode so we don't add to history */
  is_script = pClient->is_script;
  pClient->is_script = 1;

  /* Search through all known commands */
  for (x = 0; x < g_cmd_count; x++)
  {
    /* Test for script termination */
    if (pClient->terminate_script)
      break;

    /* Compare the next function */
    if (strcmp(argv[0], g_cmds[x].name) == 0)
    {
      /* Call the function */
      for (c = 0; c < repeat_count; c++)
        g_cmds[x].func(pClient, argc, argv);
      break;
    }
  }

  pClient->is_script = is_script;
}

/*
================================================================================
client_read_register_tcl:

This routine performs a register read.
================================================================================
*/
static int client_read_register_tcl(client_t *pClient, uint32_t chip_id,
      uint32_t ep_id, uint32_t reg_addr, uint32_t *reg_val)
{
  char      str[64];
  int       rx_size;
  char *    ptr;

  // Issue the register read command
  sprintf(str,"r 0x%08X\n", reg_addr);
  write(pClient->sockfd, str, strlen(str));

  /* Read response.  This will be an 8-digit hex:  0xABCD1234\n */
  rx_size = 12;
  if (client_read(pClient, rx_size) != rx_size)
    return -1;

  /* Convert return string value to integer */
  ptr = (char *) pClient->rxdbuff;
  sscanf(&ptr[2], "%x", reg_val);
  return 0;
}

/*
================================================================================
client_write_register_tcl:

This routine performs a register write.
================================================================================
*/
static int client_write_register_tcl(client_t *pClient, uint32_t chip_id,
      uint32_t ep_id, uint32_t reg_addr, uint32_t reg_val)
{
  char      str[64];
  int       rx_size;

  // Issue the register read command
  sprintf(str,"w 0x%08X 0x%08X\n", reg_addr, reg_val);
  write(pClient->sockfd, str, strlen(str));

  /* Read response.  This will be:  OK\n */
  rx_size = 4;
  if (client_read(pClient, rx_size) != rx_size)
    return -1;

  return 0;
}

/*
================================================================================
client_read_register:

This routine performs a register read.
================================================================================
*/
int client_read_register(client_t *pClient, uint32_t f1000_id,
      uint32_t ep_id, uint32_t reg_addr, uint32_t *reg_val)
{
  uint32_t  *ptr;
  uint32_t  tx_size;
  uint32_t  rx_size;
  uint32_t  api;

  /* Test for TCL server connection tyoe */
  if (pClient->port == 5001)
    return client_read_register_tcl(pClient, f1000_id, ep_id, reg_addr, reg_val);

  /* Copy API and f1000/ep id to txdbuff */
  api     = IQA_READ_REG;
  tx_size = 16;
  rx_size = 12;

  /* Populate the TX buffer */

  ptr = (uint32_t *) pClient->txdbuff;
  *ptr++ = api;
  *ptr++ = pClient->f1000_id;
  *ptr++ = pClient->ep_id;
  *ptr++ = reg_addr;

  /* Write to the socket */
  write(pClient->sockfd, pClient->txdbuff, tx_size);

  /* Read response */
  if (client_read(pClient, rx_size) != rx_size)
    return -1;

  ptr = (uint32_t *) pClient->rxdbuff;
  *reg_val = *++ptr;
  return 0;
}

/*
================================================================================
client_write_register:

This routine performs a register write.
================================================================================
*/
int client_write_register(client_t *pClient, uint32_t f1000_id,
      uint32_t ep_id, uint32_t reg_addr, uint32_t reg_val)
{
  uint32_t  *ptr;
  uint32_t  tx_size;
  uint32_t  rx_size;
  uint32_t  api;

  /* Test for TCL server connection tyoe */
  if (pClient->port == 5001)
    return client_write_register_tcl(pClient, f1000_id, ep_id, reg_addr, reg_val);

  api     = IQA_WRITE_REG;
  tx_size = 20;
  rx_size = 8;

  /* Populate the TX buffer */

  ptr = (uint32_t *) pClient->txdbuff;
  *ptr++ = api;
  *ptr++ = pClient->f1000_id;
  *ptr++ = pClient->ep_id;
  *ptr++ = reg_addr;
  *ptr++ = reg_val;

  /* Write to the socket */
  write(pClient->sockfd, pClient->txdbuff, tx_size);

  /* Read response */
  if (client_read(pClient, rx_size) != rx_size)
    return -1;

  return 0;
}

/*
================================================================================
client_get_chip_revision:

This routine gets the current revsion of the F1000 device.
================================================================================
*/

int client_get_chip_revision(client_t *pClient, uint32_t f1000_id,
                             uint32_t *rev)
{
  uint32_t  *ptr;
  uint32_t  tx_size;
  uint32_t  rx_size;
  uint32_t  api;

  /* Test for TCL connection type */
  if (pClient->port == 5001)
  {
    if (client_readreg(pClient, 0x40000000, rev) == -1)
      return -1;
    *rev &= 0xF;
    return 0;
  }

  /* Copy API and f1000/ep id to txdbuff */
  api     = EP_READ_CHIP_REVISION;
  tx_size = 12;
  rx_size = 12;

  /* Populate the TX buffer */

  ptr = (uint32_t *) pClient->txdbuff;
  *ptr++ = api;
  *ptr++ = pClient->f1000_id;
  *ptr++ = pClient->ep_id;

  /* Write to the socket */
  write(pClient->sockfd, pClient->txdbuff, tx_size);

  /* Read response */
  if (client_read(pClient, rx_size) != rx_size)
    return -1;

  ptr = (uint32_t *) pClient->rxdbuff;
  *rev = *++ptr;
  return 0;
}


/*
================================================================================
format text
================================================================================
*/
static int client_format_text(client_t *pClient, int quiet, char *buf, int buf_len,
    int argc, char *argv[])
{
  int         out_len;
  int         x, i;
  int         bin_len;
  int         bin_under;
  uint32_t    bin_mask;
  int         mask_len;
  int         len;
  int         argn;
  char        *ptr;
  char        fmt[16];
  int         fmtlen;
  char        fmtChar;
  eval_result_t value;

  /* Start at beginning of format string */
  ptr = argv[1];
  argn = 2;
  if (ptr[0] == '"')
  {
    ptr += 1;
    len = strlen(ptr);
    if (len > 0 && ptr[len - 1] == '"')
    {
      len -= 1;
      ptr[len] = 0;
    }
  }
  else
  {
    len = strlen(ptr);
  }
  out_len = 0;

  for (x = 0; x < len; x++)
  {
    /* Test for '%' format specifier */
    if (ptr[x] == '%')
    {
      fmtlen = 0;
      fmt[fmtlen++] = '%';

      /* Skip size info */
      while (isdigit(ptr[x+1]))
      {
        fmt[fmtlen++] = ptr[x+1];
        x++;
      }

      /* Get the format character */
      x++;
      fmt[fmtlen++] = ptr[x];
      fmt[fmtlen] = 0;
      fmtChar = ptr[x];
      bin_under = 0;
      if (fmtChar == '_' && ptr[x+1] == 'b')
      {
        bin_under = 1;
        fmtChar = ptr[++x];
      }

#if 0
      if (fmtChar == 'u')
      {
        fmtChar = ptr[++x];
        fmt[fmtlen++] = fmtChar;
        fmt[fmtlen] = 0;
      }
#endif

      /* Test for '%%' */
      if (fmtChar == '%')
      {
        out_len += snprintf(buf + out_len, buf_len - out_len, "%%");
        continue;
      }

      if (argn >= argc)
      {
        if (!quiet)
          client_printf(pClient, "Insufficient number of arguments for format\n");
        return -1;
      }

      /* Evauate the next argument */
      if (eval_solve(pClient->pEvalCtx, argv[argn], -1, &value))
      {
        if (!quiet)
          client_printf(pClient, "Error evaluating expression '%s'\n",
              argv[argn]);
        return -1;
      }

      switch (fmtChar)
      {
      case 'b': /* Binary */
        if (value.type != EVAL_VAL_INT)
        {
          if (!quiet)
            client_printf(pClient, "Invalid type for 'b' format '%s'\n",
                argv[argn]);
          return -1;
        }
        /* Find binary string length */
        bin_len = -1;
        if (isdigit(fmt[1]))
        {
          bin_len = atoi(&fmt[1]);
        }
        bin_mask = 0x80000000;
        mask_len = 32;
        while ((bin_mask & value.u.i_val) == 0 && mask_len > bin_len &&
                bin_mask != 0)
        {
          bin_mask >>= 1;
          mask_len--;
        }

        /* We need to print binary specially */
        while (mask_len > 0)
        {
          char ch;
          ch = (value.u.i_val & bin_mask) ? '1' : '0';
          out_len += snprintf(buf + out_len, buf_len - out_len, "%c", ch);

          bin_mask >>= 1;
          mask_len--;
          if (bin_under && mask_len && ((mask_len % 8) == 0))
            out_len += snprintf(buf + out_len, buf_len - out_len, "_");
        }

        /* Advance to next arg */
        argn++;
        break;

      case 's': /* String */
        switch (value.type)
        {
        case EVAL_VAL_STR:
          strcpy(fmt + fmtlen - 1, ".*s");
          out_len += snprintf(buf + out_len, buf_len - out_len, fmt,
              value.u.str.len, value.u.str.str);
          break;

        case EVAL_VAL_INT:
          out_len += snprintf(buf + out_len, buf_len - out_len, "%" PRId64,
              value.u.i_val);
          break;

        case EVAL_VAL_FLOAT:
          out_len += snprintf(buf + out_len, buf_len - out_len, "%f",
              value.u.f_val);
          break;

        default:
          out_len += snprintf(buf + out_len, buf_len - out_len, "NaN");
          break;
        }

        argn++;
        break;

      case 'c':
        if (value.type != EVAL_VAL_INT)
        {
          if (!quiet)
            client_printf(pClient, "Invalid type for '%c' format '%s'\n",
                fmtChar, argv[argn]);
          return -1;
        }
        out_len += snprintf(buf + out_len, buf_len - out_len, fmt, (char)value.u.i_val);
        argn++;
        break;

      case 'd':
      case 'i':
      case 'o':
      case 'u':
      case 'x':
      case 'X':
        if (value.type != EVAL_VAL_INT)
        {
          if (!quiet)
            client_printf(pClient, "Invalid type for '%c' format '%s'\n",
                fmtChar, argv[argn]);
          return -1;
        }

        /* Properly format for value width (use correct number of 'l's) */
        fmtlen -= 1;
        for (i = 0; PRId64[i]; ++i)
          fmt[fmtlen++] = PRId64[i];
        fmt[fmtlen] = 0;
        fmt[fmtlen - 1] = fmtChar;

        out_len += snprintf(buf + out_len, buf_len - out_len, fmt, value.u.i_val);
        argn++;
        break;

      case 'a':
      case 'A':
      case 'e':
      case 'E':
      case 'f':
      case 'F':
      case 'g':
      case 'G':
        if (value.type != EVAL_VAL_FLOAT)
        {
          if (!quiet)
            client_printf(pClient, "Invalid type for '%c' format '%s'\n",
                fmtChar, argv[argn]);
          return -1;
        }
        out_len += snprintf(buf + out_len, buf_len - out_len, fmt, value.u.f_val);
        argn++;
        break;

      default:
        if (!quiet)
          client_printf(pClient, "Invalid format char '%c'\n", fmtChar);
        return -1;
      }
    }

    /* Test for '\' character */
    else if (ptr[x] == '\\')
    {
      /* Get special character */
      fmtChar = ptr[++x];
      if (fmtChar == 'r')
        fmtChar = '\r';
      else if (fmtChar == 'n')
        fmtChar = '\n';
      else if (fmtChar == 't')
        fmtChar = '\t';
      else if (fmtChar == '\\')
        fmtChar = '\\';

      out_len += snprintf(buf + out_len, buf_len - out_len, "%c", fmtChar);
    }

    else
    {
      out_len += snprintf(buf + out_len, buf_len - out_len, "%c", ptr[x]);
    }
  }

  return out_len;

}

/*
================================================================================
print command
================================================================================
*/
static void client_print(client_t *pClient, int argc, char *argv[])
{
  int   x;
  int   print_as_int16 = 0;
  f1000_regs_ctx_t  *pReg;
  script_var_t      *pVar;

  /* Test if we should execute based on if processing */
  if (!ifproc_should_process(&pClient->ifProc))
    return;

  /* Test if enough args */
  if (argc < 2)
  {
    client_printf(pClient, "usage: print var\n");
    client_printf(pClient, "   or: print format [args...]\n");
    return;
  }

  if (argc == 3 && strcmp(argv[2], "int16") ==0)
    print_as_int16 = 1;

  /* Test if we have an open connection */
  if (pClient->sockfd != 0)
  {
    /* Use f1000_reg context to print a register by name */
    pReg = pClient->pRegs;
    if (pReg)
    {
      pReg->argc = argc;
      for (x = 0; x < argc; x++)
        pReg->argv[x] = argv[x];

      /* Test for simple register printing */
      if (f1000_regs_is_register(pClient->pRegs, argv[1]))
      {
        f1000_regs_cmd_print(pReg, pClient->fd, print_as_int16);
        return;
      }
    }
  }

  /* First test if this is a variable or a constant */
  pVar = client_find_var(pClient, argv[1]);
  if (pVar || isdigit(argv[1][0]))
  {
    if (pVar != NULL)
    {
      switch (pVar->type)
      {
        case SCRIPT_VAR_INT:
          if (strcmp(argv[0], "p/x") == 0 || strcmp(argv[0], "?") == 0)
          {
            if (pClient->fd == NULL)
              client_printf(pClient, "0x%x\n", pVar->u.i_val);
            else
              fprintf(pClient->fd, "0x%x\n", pVar->u.i_val);
          }
          else
          {
            if (pClient->fd == NULL)
              client_printf(pClient, "%d\n", pVar->u.i_val);
            else
              fprintf(pClient->fd, "%d\n", pVar->u.i_val);
          }
          break;
        case SCRIPT_VAR_QWORD:
          if (strcmp(argv[0], "p/x") == 0 || strcmp(argv[0], "?") == 0)
          {
            if (pClient->fd == NULL)
              client_printf(pClient, "0x%" PRIx64 "\n", pVar->u.q_val);
            else
              fprintf(pClient->fd, "0x%" PRIx64 "\n", pVar->u.q_val);
          }
          else
          {
            if (pClient->fd == NULL)
              client_printf(pClient, "%" PRId64 "\n", pVar->u.q_val);
            else
              fprintf(pClient->fd, "%" PRId64 "\n", pVar->u.q_val);
          }
          break;

        case SCRIPT_VAR_STRING:
          if (pClient->fd == NULL)
            client_printf(pClient, "%s\n", pVar->u.s_val);
          else
            fprintf(pClient->fd, "%s\n", pVar->u.s_val);
          break;

        case SCRIPT_VAR_NAN:
          if (pClient->fd == NULL)
            client_printf(pClient, "NaN\n");
          else
            fprintf(pClient->fd, "NaN\n");
          break;

      }
    }
    else
    {
      client_printf(pClient, "%s\n", argv[1]);
    }
  }

  /* Test for print using format */
  if (argv[1][0] == '"')
  {
    char        buf[512];

    x = client_format_text(pClient, 0, buf, sizeof(buf), argc, argv);
    if (x > 0)
    {
      if (pClient->fd != NULL)
        fwrite(buf, 1, x, pClient->fd);
      else
        client_printf(pClient, "%s", buf);
    }
  }
}

/*
================================================================================
set command
================================================================================
*/
static void client_set(client_t *pClient, int argc, char *argv[])
{
  int   x;
  f1000_regs_ctx_t  *pReg;

  /* Test if we should execute based on if processing */
  if (!ifproc_should_process(&pClient->ifProc))
    return;

  if (argc < 2)
  {
    client_printf(pClient, "usage: set reg = value\n");
    return;
  }

  /* Test if we have an open connection */
  if (pClient->sockfd == 0)
  {
    client_printf(pClient, "Please open a connection first\n");
    return;
  }

  /* Use f1000_reg context to set a register by name */
  pReg = pClient->pRegs;
  if (pReg)
  {
    pReg->argc = argc;
    for (x = 0; x < argc; x++)
      pReg->argv[x] = argv[x];

    f1000_regs_cmd_set(pReg);
  }
}

/*
================================================================================
echo command
================================================================================
*/
static void client_echo(client_t *pClient, int argc, char *argv[])
{
  /* Test if we should execute based on if processing */
  if (!ifproc_should_process(&pClient->ifProc))
    return;

  client_printf(pClient, "%s\n", argv[1]);
}

/*
================================================================================
readreg command
================================================================================
*/
static int client_readreg_tcl(client_t *pClient, uint32_t reg_addr, uint32_t *val)
{
  char      str[64];
  int       rx_size;
  char *    ptr;

  // Issue the register read command
  sprintf(str,"r 0x%08X\n", reg_addr);
  write(pClient->sockfd, str, strlen(str));

  *val = 0xCAFEBABE;

  /* Read response.  This will be an 8-digit hex:  0xABCD1234\n */
  rx_size = 12;
  if (client_read(pClient, rx_size) != rx_size)
    return -1;

  /* Convert return string value to integer */
  ptr = (char *) pClient->rxdbuff;
  sscanf(&ptr[2], "%x", val);
  return 0;
}

/*
================================================================================
writereg command
================================================================================
*/
static int client_writereg_tcl(client_t *pClient, uint32_t reg_addr, uint32_t val)
{
  char      str[64];
  int       rx_size;

  // Issue the register read command
  sprintf(str,"w 0x%08X 0x%08X\n", reg_addr, val);
  write(pClient->sockfd, str, strlen(str));

  /* Read response.  This will be:  OK\n */
  rx_size = 4;
  if (client_read(pClient, rx_size) != rx_size)
    return -1;

  return 0;
}

/*
================================================================================
readreg command
================================================================================
*/
int client_readreg(client_t *pClient, uint32_t reg_addr, uint32_t *val)
{
  uint32_t  api;
  uint32_t  tx_size;
  uint32_t  rx_size;
  uint32_t  *ptr;
  int       ret;
  int       retry;

  if (pClient->sockfd == 0)
  {
    *val = 0xCAFEBABE;
    return -1;
  }

  /* Test for TCL type connection */
  if (pClient->port == 5001)
    return client_readreg_tcl(pClient, reg_addr, val);

  for (retry = 0; retry < 6; retry++)
  {
    api     = IQA_READ_REG;
    tx_size = 16;
    rx_size = 12;

    /* Populate the TX buffer */

    ptr = (uint32_t *) pClient->txdbuff;
    *ptr++ = api;
    *ptr++ = pClient->f1000_id;
    *ptr++ = pClient->ep_id;
    *ptr++ = reg_addr;

    /* Write to the socket */
    write(pClient->sockfd, pClient->txdbuff, tx_size);

    /* Read response */
    ret = client_read(pClient, rx_size);
    if (ret == rx_size)
      break;
  }

  if (retry == 6)
    return -1;

  /* Get the register value */
  ptr = (uint32_t *) pClient->rxdbuff;
  *val = *++ptr;
  return 0;
}

/*
================================================================================
writereg command
================================================================================
*/
int client_writereg(client_t *pClient, uint32_t reg_addr, uint32_t val)
{
  uint32_t  api;
  uint32_t  tx_size;
  uint32_t  rx_size;
  uint32_t  *ptr;
  int       ret;

  /* Test for TCL type connection */
  if (pClient->port == 5001)
    return client_writereg_tcl(pClient, reg_addr, val);

  api     = IQA_WRITE_REG;
  tx_size = 20;
  rx_size = 8;

  /* Populate the TX buffer */

  ptr = (uint32_t *) pClient->txdbuff;
  *ptr++ = api;
  *ptr++ = pClient->f1000_id;
  *ptr++ = pClient->ep_id;
  *ptr++ = reg_addr;
  *ptr++ = val;

  /* Write to the socket */
  write(pClient->sockfd, pClient->txdbuff, tx_size);

  /* Read response */
  ret = client_read(pClient, rx_size);
  if (ret != rx_size)
    return -1;

  return 0;
}

/*
================================================================================
readreg command
================================================================================
*/
static void client_reg(client_t *pClient, int argc, char *argv[])
{
  uint32_t  reg_addr;
  uint32_t  reg_val;
  uint32_t  *ptr;
  uint32_t  tx_size;
  uint32_t  rx_size;
  uint32_t  api;
  int       is_reg16;
  int       ret;
  eval_result_t value;

  /* Test if we should execute based on if processing */
  if (!ifproc_should_process(&pClient->ifProc))
    return;

  if (argc < 2)
  {
    client_printf(pClient, "usage: reg addr\r\n");
    client_printf(pClient, "   or: reg addr = value\r\n");
    return;
  }

  /* Test if we have an open connection */
  if (pClient->sockfd == 0)
  {
    client_printf(pClient, "reg: Please open a connection first\n");
    return;
  }

  is_reg16 = (strcmp(argv[0], "reg16") == 0);

  /* Get RFsynth id and address */
  ret = eval_solve(pClient->pEvalCtx, argv[1], -1, &value);
  if (ret != 0)
  {
    client_printf(pClient, "Error evaluating expression '%s'\n", argv[1]);
    return;
  }

  if (value.type != EVAL_VAL_INT)
  {
    client_printf(pClient, "Invalid address type for: '%s'\n", argv[1]);
    return;
  }
  reg_addr = value.u.i_val;

  /* Copy API and f1000/ep id to txdbuff */
  if (argc == 2)
  {
    api     = IQA_READ_REG;
    tx_size = 16;
    rx_size = 12;
    reg_val = 0;
  }
  else
  {
    api     = IQA_WRITE_REG;
    tx_size = 20;
    rx_size = 8;
    ret = eval_solve(pClient->pEvalCtx, argv[3], strlen(argv[3]), &value);
    if (ret != 0)
    {
      client_printf(pClient, "Error evaluating expression '%s'\n", argv[3]);
      return;
    }

    if (value.type != EVAL_VAL_INT)
    {
      client_printf(pClient, "Invalid value type for: '%s'\n", argv[3]);
      return;
    }

    reg_val = value.u.i_val;
  }

  /* Populate the TX buffer */

  ptr = (uint32_t *) pClient->txdbuff;
  *ptr++ = api;
  *ptr++ = pClient->f1000_id;
  *ptr++ = pClient->ep_id;
  *ptr++ = reg_addr;
  *ptr++ = reg_val;

  /* Test for TCL connection type */
  if (pClient->port == 5001)
  {
    api = IQA_READ_REG;
    if (argc == 2)
      client_readreg_tcl(pClient, reg_addr, &reg_val);
    else
      client_writereg_tcl(pClient, reg_addr, reg_val);
  }
  else
  {
    /* Write to the socket */
    write(pClient->sockfd, pClient->txdbuff, tx_size);

    /* Read response */
    client_read(pClient, rx_size);
    if (api == IQA_READ_REG)
    {
      ptr = (uint32_t *) pClient->rxdbuff;
      reg_val = *++ptr;
    }
  }

  if (api == IQA_READ_REG)
  {
    if (pClient->fd == NULL)
    {
      if (is_reg16)
      {
        int32_t sreg_val = reg_val;
        if (reg_val & 0x8000)
          sreg_val |= 0xFFFF0000;
        client_printf(pClient, "%d\n", sreg_val);
      }
      else
        client_printf(pClient, "0x%08X = 0x%08X\n", reg_addr, reg_val);
    }
    else
    {
      if (is_reg16)
      {
        int32_t sreg_val = reg_val;
        if (reg_val & 0x8000)
          sreg_val |= 0xFFFF0000;
        fprintf(pClient->fd, "%d\n", sreg_val);
      }
      else
        fprintf(pClient->fd, "0x%08X\n", reg_val);
    }
  }
}

/*
================================================================================
Capture DAC waveform once
================================================================================
*/
static void client_cap_once(client_t *pClient, int argc, char *argv[])
{
  char *ptr;
  int   tx_size;

  /* Test if we should execute based on if processing */
  if (!ifproc_should_process(&pClient->ifProc))
    return;

  if (argc < 1)
  {
    client_printf(pClient, "usage: cap_once\n");
    return;
  }

  ptr = pClient->txdbuff;

  /* Copy API and f1000/ep id to txdbuff */
  *((uint32_t *) ptr) = IQA_CAP_ONCE;
  ptr += 4;

  /* Write to the socket */
  tx_size = 4;
  write(pClient->sockfd, pClient->txdbuff, tx_size);

  /* Read response */
  client_read(pClient, 8);
}

/*
================================================================================
Capture DAC waveform once
================================================================================
*/
static void client_get_cap(client_t *pClient, int argc, char *argv[])
{
  char *ptr;
  int   tx_size;
  int   n;
  int   samples;
  int   rx_expected, rx_received;
  int   rx_remaining;
  struct pollfd pfd;
  char  *hex_file = NULL;
  char  *txt_file = NULL;

  /* Test if we should execute based on if processing */
  if (!ifproc_should_process(&pClient->ifProc))
    return;

  if (argc < 2)
  {
    client_printf(pClient, "usage: get_cap filename\n");
    return;
  }

  if (argc == 3)
  {
    hex_file = argv[1];
    txt_file = argv[2];
  }
  else
  {
    txt_file = argv[1];
  }

  ptr = pClient->txdbuff;

  /* Copy API and f1000/ep id to txdbuff */
  *((uint32_t *) ptr) = IQA_GET_DAC_CAP;

  /* Write to the socket */
  tx_size = 4;
  write(pClient->sockfd, pClient->txdbuff, tx_size);

  /* Read 8 bytes from the server */
  int rx_size = 0;
  while (rx_size < 8)
  {
    if (poll(&pfd, 1, 25000) == 0)
    {
      if (!pClient->quiet)
        client_printf(pClient, "ERROR: read timed out.  Received %d bytes, expected %d (%d diff)\n",
          rx_size, 8, 8-rx_size);
      return;
    }

    n = read(pClient->sockfd, &pClient->rxdbuff[rx_size], 8);
    if (n < 0)
    {
      if (!pClient->quiet)
        perror("Error reading data from socket");
      return;
    }

    /* Accumulate data */
    rx_size += n;
  }

  if (rx_size == 8)
  {
    /* Get the sample count */
    samples = *((uint32_t *) &pClient->rxdbuff[4]);
  }

  /* Test for empty buffer */
  if (samples == 0)
  {
    if (!pClient->quiet)
      client_printf(pClient, "No buffer captured.  Please issue 'cap_once' first\n");

    /* Read the error code */
    client_read(pClient, 4);
    return;
  }

  if (pClient->debug && !pClient->quiet)
    client_printf(pClient, "Awaiting %d samples\n", samples);
  rx_expected = samples*2;
  rx_received = client_read(pClient, rx_expected);

  /* Write to a file if requested */
  FILE* fd;

  /* Try to open the file(s) */
  if ((fd = fopen(txt_file, "w+")) != NULL)
  {
    int16_t *pSamp = (int16_t *) pClient->rxdbuff;
    int x;

    for (x = 0; x < samples; x++)
    {
      if (hex_file == NULL)
      {
        fprintf(fd, "0x%04X   %d\n", *pSamp, *pSamp);
      }
      else
      {
        fprintf(fd, "%d\n", *pSamp);
      }
      pSamp++;
    }
    fclose(fd);
  }
  else
  {
    client_printf(pClient, "Error opening %s\n", txt_file);
  }

  /* Test if separate hex file specified */
  if (hex_file != NULL)
  {
    if ((fd = fopen(hex_file, "w+")) != NULL)
    {
      int16_t *pSamp = (int16_t *) pClient->rxdbuff;
      int x;

      for (x = 0; x < samples; x++)
      {
        fprintf(fd, "0x%04X\n", *pSamp);
        pSamp++;
      }
      fclose(fd);
    }
    else
    {
      client_printf(pClient, "Error opening %s\n", txt_file);
    }
  }

  rx_expected = 4;
  rx_remaining = rx_received - samples * 2;
  if (rx_remaining < rx_expected)
  {
    rx_expected -= rx_remaining;
    if (pClient->debug)
      client_printf(pClient, "Awaiting %d remaining error code bytes\n", rx_expected);

    /* Read the final bytes of the error code */
    client_read(pClient, rx_expected);
  }
}

/*
================================================================================
help command
================================================================================
*/
void client_help(client_t *pClient, int argc, char *argv[])
{
  int   x;

  if (argc == 0)
  {
    client_printf(pClient, "usage: help [command]\n");
    return;
  }

  if (argc == 1)
  {
    /* Print help for all commands */
    for (x = 0; x < g_cmd_count; x++)
    {
      if (!g_cmds[x].is_script)
        client_printf(pClient, "%-15s%s\n", g_cmds[x].name, g_cmds[x].help);
    }
  }
  else
  {
    /* Find the specific commmand and provide usage info */
    for (x = 0; x < g_cmd_count; x++)
    {
      if (strcmp(argv[1], g_cmds[x].name) == 0)
      {
        char *cmd_argv[] = { argv[1] };
        client_printf(pClient, "\r\n%s:  %s\n", argv[1], g_cmds[x].help);
        g_cmds[x].func(pClient, 0, cmd_argv);
        return;
      }
    }

    client_printf(pClient, "unknown command: %s\n", argv[1]);
  }
}

/*
================================================================================
adc_cap command
================================================================================
*/
static void client_adc_cap(client_t *pClient, int argc, char *argv[])
{
    uint32_t  *ptr;
    int       tx_size;
    int       n;
    int       rx_expected;
    uint32_t  adc_id;
    uint32_t  samples;

  /* Test if we should execute based on if processing */
  if (!ifproc_should_process(&pClient->ifProc))
    return;

  /* Validate the arguments */
  if (argc < 3)
  {
    client_printf(pClient, "usage: adc_cap [1 | 2] samples\n");
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
  *ptr++ = F1000_ADC_DSP_START_CAPTURE;
  *ptr++ = pClient->f1000_id;
  *ptr++ = pClient->ep_id;

  /* Process arguments */
  adc_id  = atoi(argv[1]);         // ADC ID
  samples = atoi(argv[2]);         // Sample count

  /* Add API parameters */
  *ptr++ = adc_id;
  *ptr++ = samples;

  /* Write to the socket */
  tx_size = (char *) ptr - &pClient->txdbuff[0];
  n = write(pClient->sockfd, pClient->txdbuff, tx_size);
  if (n < 0)
  {
  	perror("Error transmitting to server");
  	return;
  }

  /* Perform read from server */
  rx_expected = 8;
  client_read(pClient, rx_expected);

  /* If we have additional parameters, then call adc_mem automatically */
  if (argc > 3)
    client_adc_mem(pClient, argc, argv);
}

/*
================================================================================
adc_mem command
================================================================================
*/
static void client_adc_mem(client_t *pClient, int argc, char *argv[])
{
    uint32_t  *ptr;
    int       n;
    int       tx_size;
    int       rx_expected;
    int       rx_received;
    int       plot = 0;
    int       fft  = 0;
    int       ordered = 1;
    uint32_t  adc_id;
    uint32_t  samples;
    char      *filename = NULL;

  /* Test if we should execute based on if processing */
  if (!ifproc_should_process(&pClient->ifProc))
    return;

  /* Validate the arguments */
  if (argc < 3)
  {
    client_printf(pClient, "usage: adc_mem [1 | 2] samples [plot | fft | filename] [unordered]\n");
    return;
  }

  /* Test if we have an open connection */
  if (pClient->sockfd == 0)
  {
    client_printf(pClient, "Please open a connection first\n");
    return;
  }

  if (argc >= 4)
  {
    /* Test for plot request */
    if (strncmp(argv[3], "plot", 4) == 0)
    {
      if (strcmp(argv[3], "plot") == 0)
        plot = atoi(argv[2]);
      else
        plot = atoi(&argv[3][5]);
    }
    /* Test for fft request */
    else if (strcmp(argv[3], "fft") == 0)
    {
      fft = 1;
    }
    else
    {
      /* Writing to a file */
      filename= argv[3];
    }
  }

  /* Check if unordered data is requested */
  if (argc == 5 && strcmp(argv[4], "unordered") == 0)
  {
    ordered = 0;
  }

  memset(pClient->rxdbuff, 0, sizeof(pClient->rxdbuff));
  memset(pClient->txdbuff, 0, sizeof(pClient->txdbuff));
  ptr = (uint32_t *) pClient->txdbuff;

  /* Copy API and f1000/ep id to txdbuff */
  *ptr++ = ordered ? F1000_ADC_READ_MEM_ORDERED : F1000_ADC_READ_MEM;
  *ptr++ = pClient->f1000_id;
  *ptr++ = pClient->ep_id;

  /* Process arguments */
  adc_id  = atoi(argv[1]);         // ADC ID
  samples = atoi(argv[2]);         // Sample count

  /* Add API parameters */
  *ptr++ = adc_id;
  *ptr++ = samples;

  /* Write to the socket */
  tx_size = (char *) ptr - &pClient->txdbuff[0];
  n = write(pClient->sockfd, pClient->txdbuff, tx_size);
  if (n < 0)
  {
  	perror("Error transmitting to server");
  	return;
  }


  /* Perform read from server. Add 12 bytes additional data from f1000_server. */
  rx_expected = samples*2 + 12;
  rx_received = client_read(pClient, rx_expected);

  if (!ordered)
  {
    /* chunk_size MUST be the same size as requested size from f1000_server,
       which is 256 bytes.
    */
    adc_ordermem(pClient, samples, 256);
  }

  /* Write to a file if requested */
  if (filename != NULL)
  {
    FILE    *fd;
    int16_t *pPtr;
    int     x;

    pPtr = (int16_t *) (pClient->rxdbuff + 8);

    /* Try to open the file */
    if ((fd = fopen(filename, "w")) != NULL)
    {
      for (x = 0; x < rx_received; x+= 2)
      {
        if ((*pPtr) & 0x200)
          (*pPtr) |= 0xFC00;
        fprintf(fd, "%d\n", *pPtr++);
      }
      fclose(fd);
    }
  }
  else if (plot > 0 || fft > 0)
  {
    FILE    *fd;
    int16_t *pPtr;
    int     x;

    pPtr = (int16_t *) (pClient->rxdbuff + 8);
    if (fft > 0)
      plot = samples;

    /* Try to open the file */
    if ((fd = fopen("/tmp/client_samp.txt", "w")) != NULL)
    {
      for (x = 0; x < plot; x++)
      {
        if ((*pPtr) & 0x200)
          (*pPtr) |= 0xFC00;
        fprintf(fd, "%d\n", *pPtr++);
      }
      fclose(fd);
    }

    // Plot using the external gnuplot script
    if (fft == 0)
    {
      /* Plot the data using gnuplot */
      system("gnuplot -c scripts/gnuplot.s");
    }
    else
    {
      /* Run an fft using octave and then plot the results */
      system("octave scripts/octave.m");
      system("gnuplot -c scripts/gnuplot_fft.s");
    }
  }
}

/*
================================================================================
system command
================================================================================
*/
static void client_system(client_t *pClient, int argc, char *argv[])
{
  uint32_t  *ptr;
  int       n;
  int       tx_size;
  int       rx_expected;
  char      *cmd;
  int       len;

  /* Test if we should execute based on if processing */
  if (!ifproc_should_process(&pClient->ifProc))
    return;

  /* Validate the arguments */
  if (argc < 2)
  {
    client_printf(pClient, "usage: %s command\n", argv[0]);
    return;
  }

  /* Test if we have an open connection */
  if (pClient->sockfd == 0)
  {
    client_printf(pClient, "Please open a connection first\n");
    return;
  }

  /* Test if connection type is TCL and simply return. System not supported */
  if (pClient->port == 5001)
  {
    client_printf(pClient, "The 'system' command is not supported with TCL interface\n");
    return;
  }

  cmd = argv[1];
  if (cmd[0] == '"')
  {
    cmd += 1;
    len = strlen(cmd);
    if (len > 0 && cmd[len - 1] == '"')
    {
      cmd[len - 1] = 0;
      len -= 1;
    }
  }
  else
  {
    len = strlen(cmd);
  }

  ptr = (uint32_t *) pClient->txdbuff;

  /* Copy API and f1000/ep id to txdbuff */
  *ptr++ = IQA_SYSTEM;
  strcpy((char *) ptr, cmd);

  /* Write to the socket */
  tx_size = len + 1 + 4;
  if (pClient->debug && !pClient->quiet)
    client_printf(pClient, "Sending %d bytes\n", tx_size);
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

/*
================================================================================
Find specified variable in our linked list
================================================================================
*/
script_var_t * client_find_var(client_t *pClient, const char *name)
{
  script_var_t    *pVar;

  /* Start at the list head for locals*/
  pVar = pClient->pLocals;
  while (pVar != NULL)
  {
    /* Test if this variable matches */
    if (strcmp(pVar->name, name) == 0)
      return pVar;

    /* Get pointer to next item in list */
    pVar = pVar->pNext;
  }

  /* Start at the list head */
  pVar = pClient->pVars;
  while (pVar != NULL)
  {
    /* Test if this variable matches */
    if (strcmp(pVar->name, name) == 0)
      return pVar;

    /* Get pointer to next item in list */
    pVar = pVar->pNext;
  }

  if (pClient->pAuxVarGet != NULL)
  {
    pVar = &pClient->auxScriptVar;
    if (pClient->pAuxVarGet(pClient->pAuxCtx, name, pVar) == 0)
    {
        pVar->flags |= SCRIPT_VAR_FLAG_CONST;
        return pVar;
    }
  }

  return NULL;
}

/*
================================================================================
Create the specified variable, or return pointer to existing var
================================================================================
*/
static script_var_t * client_make_var(client_t *pClient, char *name,
    script_var_t **pLocals, int shadow)
{
  script_var_t  *pVar;

  /* Test if this variable already exists */
  if (!shadow)
      pVar = client_find_var(pClient, name);
  else
      pVar = NULL;

  /* Allocate new variable if it doesn't exist */
  if (pVar == NULL)
  {
    pVar = (script_var_t *) malloc(sizeof(script_var_t));
    if (pVar == NULL)
    {
      client_printf(pClient, "Out of memory allocating variable %s\n", name);
      return NULL;
    }

    /* Initialize the new variable */
    pVar->type = SCRIPT_VAR_NAN;
    pVar->flags = 0;
    pVar->pNext = NULL;
    pVar->u.s_val = NULL;
    strcpy(pVar->name, name);

    /* Add to our linked list */
    if (pLocals != NULL)
    {
      pVar->flags |= SCRIPT_VAR_FLAG_LOCAL;
      pVar->pNext = *pLocals;
      *pLocals = pVar;
    }
    else
    {
      pVar->pNext = pClient->pVars;
      pClient->pVars = pVar;
    }
  }
  else if (pVar->flags & SCRIPT_VAR_FLAG_CONST)
  {
    client_printf(pClient, "Invalid write to const variable %s\n", name);
    return NULL;
  }

  return pVar;
}

/*
================================================================================
Delete the specified variable's data
================================================================================
*/
static void client_del_var_data(client_t *pClient, script_var_t *pVar)
{
  switch (pVar->type)
  {
    case SCRIPT_VAR_STRING:
      free(pVar->u.s_val);
      break;

    case SCRIPT_VAR_FUNC:
      client_del_function(pClient, pVar->u.fn_val);
      break;
  }
  pVar->type = SCRIPT_VAR_NAN;
}

/*
================================================================================
Delete the specified variable
================================================================================
*/
static void client_del_var(client_t *pClient, script_var_t *pVar)
{
  client_del_var_data(pClient, pVar);
  free(pVar);
}

/*
================================================================================
Convert specified variable to script structure
================================================================================
*/
static int client_eval_from_var(client_t *pClient, eval_result_t *pRes, const script_var_t *pVar)
{
  switch (pVar->type)
  {
  case SCRIPT_VAR_INT:
    pRes->type = EVAL_VAL_INT;
    pRes->u.i_val = (uint32_t) pVar->u.i_val;
    break;

  case SCRIPT_VAR_QWORD:
    pRes->type = EVAL_VAL_INT;
    pRes->u.i_val = (uint64_t) pVar->u.q_val;
    break;

  case SCRIPT_VAR_FLOAT:
    pRes->type = EVAL_VAL_FLOAT;
    pRes->u.f_val = pVar->u.f_val;
    break;

  case SCRIPT_VAR_STRING:
    pRes->type = EVAL_VAL_STR;
    pRes->u.str.str = pVar->u.s_val;
    pRes->u.str.len = strlen(pVar->u.s_val);
    break;

  default:
    return -1;
  }

  return 0;
}

/*
================================================================================
Convert specified variable to script structure
================================================================================
*/
static int client_var_from_eval(client_t *pClient, int quiet, script_var_t *pOut,
    const eval_result_t *pIn)
{
  /* Keep a copy of the data to prevent use after free */
  script_var_t tmp = *pOut;

  switch (pIn->type)
  {
  case EVAL_VAL_NIL:
    pOut->type = SCRIPT_VAR_NAN;
    break;

  case EVAL_VAL_INT:
    pOut->type = SCRIPT_VAR_INT;
    pOut->u.i_val = pIn->u.i_val;
    break;

  case EVAL_VAL_FLOAT:
    pOut->type = SCRIPT_VAR_FLOAT;
    pOut->u.f_val = pIn->u.f_val;
    break;

  case EVAL_VAL_STR:
    pOut->type = SCRIPT_VAR_STRING;
    pOut->u.s_val = malloc(1 + pIn->u.str.len);
    if (pOut->u.s_val == NULL)
    {
      client_printf(pClient, "Out of memory allocating string\n");
      pOut->type = SCRIPT_VAR_NAN;
      return -1;
    }
    memcpy(pOut->u.s_val, pIn->u.str.str, pIn->u.str.len);
    pOut->u.s_val[pIn->u.str.len] = 0;
    break;

  default:
    client_printf(pClient, "Unsupported variable type\n");
    pOut->type = SCRIPT_VAR_NAN;
    return -1;
  }

  /* Delete prev data, if any */
  client_del_var_data(pClient, &tmp);

  return 0;
}

/*
================================================================================
var command
================================================================================
*/
static void client_var(client_t *pClient, int argc, char *argv[])
{
  int           val_index;
  char          *pStr;
  script_var_t  *pVar;

  /* Test if we should execute based on if processing */
  if (!ifproc_should_process(&pClient->ifProc))
    return;

  if (argc < 2)
  {
    client_printf(pClient, "Usage: %s name = val\r\n", argv[0]);
    return;
  }

  /* Test for name=value format */
  val_index = 3;
  if ((pStr = strchr(argv[1], '=')) != NULL)
  {
    *pStr++ = 0;
    while (*pStr == ' ' || *pStr == '"')
      pStr++;
    argv[argc++] = "=";
    argv[argc++] = pStr;
  }
  else
  {
    if (argv[2] == NULL)
    {
      client_printf(pClient, "Usage: %s name = val\r\n", argv[0]);
      return;
    }
    if (strcmp(argv[2], "=") == 0)
      val_index = 3;
  }

  if (argc < 4)
  {
    client_printf(pClient, "Usage: %s name = val\r\n", argv[0]);
    return;
  }

  /* Create the variable */
  if (argv[0][0] == 'l') /* Local explicitly specified */
    pVar = client_make_var(pClient, argv[1], &pClient->pLocals, 1);
  else
    pVar = client_make_var(pClient, argv[1], NULL, 0);
  if (pVar == NULL)
    return;

  /* Test if assigning from a reg address */
  else if (strncmp(argv[val_index], "reg ", 4) == 0)
  {
    /* Evaluate the argument to 'reg' */
    eval_result_t value;
    const char *  expr;
    uint32_t      reg_val;
    uint32_t      addr;

    expr = &argv[val_index][4];
    if (eval_solve(pClient->pEvalCtx, expr, -1, &value) != 0)
    {
      client_printf(pClient, "error evaluating expression '%s'\n", expr);
      return;
    }

    if (value.type != EVAL_VAL_INT)
    {
      client_printf(pClient, "Invalid register address for '%s'\n", expr);
      return;
    }

    /* Now we have the address.  Read the register */
    addr = (uint32_t) value.u.i_val;
    if (client_readreg(pClient, addr, &reg_val))
    {
      client_printf(pClient, "Error reading memory address 0x%08X\n", addr);
      return;
    }

    /* Delete prev data, if any */
    client_del_var_data(pClient, pVar);

    pVar->u.i_val = reg_val;
    pVar->type = SCRIPT_VAR_INT;
  }
  else if (strncmp(argv[val_index], "call ", 5) == 0)
  {
    char  *call_argv[20];
    int   call_argc;

    call_argc = client_parse_args(argv[val_index], call_argv, 20);
    client_call_function(pClient, pVar, call_argc, call_argv);
  }
  else if (strncmp(argv[val_index], "input ", 5) == 0)
  {
    char  *call_argv[20];
    int   call_argc;

    call_argc = client_parse_args(argv[val_index], call_argv, 20);
    client_input_function(pClient, pVar, call_argc, call_argv);
  }

  /* Else we must evaluate it */
  else
  {
    eval_result_t value;
    int ret;

    ret = eval_solve(pClient->pEvalCtx, argv[val_index], -1, &value);
    if (ret != 0)
    {
      client_printf(pClient, "Unable to evaluate expression\n");
      pVar->type = SCRIPT_VAR_NAN;
      return;
    }

    if (client_var_from_eval(pClient, 0, pVar, &value) != 0)
      return;
  }
#if 0
  if (f1000_regs_is_register(pClient->pRegs, argv[val_index]))
  {
    uint32_t value;

    /* Read the register value */
    if (f1000_regs_readreg(pClient->pRegs, argv[val_index], &value) == 0)
    {
      pVar->u.i_val = value;
      pVar->type = SCRIPT_VAR_INT;
    }
    else
      pVar->type = SCRIPT_VAR_NAN;
  }
  else if (argv[val_index][0] >= '1' && argv[val_index][0] <= '9')
  {
    pVar->u.i_val = atoi(argv[val_index]);
    pVar->type = SCRIPT_VAR_INT;
  }
  else if (strncmp(argv[val_index], "0x", 2) == 0)
  {
    pVar->u.i_val = hextoi(argv[val_index]);
    pVar->type = SCRIPT_VAR_INT;
  }
#endif
}

/*
================================================================================
mem command
================================================================================
*/
static void client_mem(client_t *pClient, int argc, char *argv[])
{
  int           val_index;
  char          *pStr;
  script_mem_t  *pMem;
  eval_result_t value;

  /* Test if we should execute based on if processing */
  if (!ifproc_should_process(&pClient->ifProc))
    return;

  if (argc < 2)
  {
    client_printf(pClient, "Usage: %s name = addr\r\n", argv[0]);
    return;
  }

  /* Test for name=value format */
  val_index = 3;
  if ((pStr = strchr(argv[1], '=')) != NULL)
  {
    *pStr++ = 0;
    while (*pStr == ' ' || *pStr == '"')
      pStr++;
    argv[argc++] = "=";
    argv[argc++] = pStr;
  }
  else
  {
    if (argv[2] == NULL)
    {
      client_printf(pClient, "Usage: %s name = addr\r\n", argv[0]);
      return;
    }
    if (strcmp(argv[2], "=") == 0)
      val_index = 3;
  }

  if (argc < 4)
  {
    client_printf(pClient, "Usage: %s name = addr\r\n", argv[0]);
    return;
  }

  /* Evaluate the address */
  if (eval_solve(pClient->pEvalCtx, argv[val_index], -1, &value) != 0)
  {
    client_printf(pClient, "Error evaluating expression '%s'\n", argv[val_index]);
    return;
  }

  if (value.type != EVAL_VAL_INT)
  {
    client_printf(pClient, "Invalid type in: '%s'\n", argv[val_index]);
    return;
  }

  /* Create the memory */
  pMem = (script_mem_t *) malloc(sizeof(script_mem_t));
  if (pMem == NULL)
  {
    client_printf(pClient, "Out of memory allocating memory struct\n");
        return;
  }

  /* Initialze the new struct */
  strcpy(pMem->name, argv[1]);
  pMem->addr = value.u.i_val;
  pMem->type = SCRIPT_MEM_BYTE;
  if (strcmp(argv[0], "mem16") == 0)
    pMem->type = SCRIPT_MEM_16BIT;
  else if (strcmp(argv[0], "mem32") == 0)
    pMem->type = SCRIPT_MEM_32BIT;

  /* Add to our linked list */
  pMem->pNext = pClient->pMem;
  pClient->pMem = pMem;
}

/*
================================================================================
fclose command
================================================================================
*/
static void client_fclose(client_t *pClient, int argc, char *argv[])
{
  /* Test if we should execute based on if processing */
  if (!ifproc_should_process(&pClient->ifProc))
    return;

  if (argc < 1)
  {
    client_printf(pClient, "usage: fclose\n");
    return;
  }

  if (pClient->fd != NULL)
    fclose(pClient->fd);
  pClient->fd = NULL;
}

/*
================================================================================
fopen command
================================================================================
*/
static void client_fopen(client_t *pClient, int argc, char *argv[])
{
  /* Test if we should execute based on if processing */
  if (!ifproc_should_process(&pClient->ifProc))
    return;

  if (argc < 3)
  {
    client_printf(pClient, "usage: fopen filename [w|a]\n");
    return;
  }

  /* Test if file already open */
  if (pClient->fd != NULL)
  {
    client_printf(pClient, "Another file already open!\n");
    return;
  }

  /* Try to open the file */
  pClient->fd = fopen(argv[1], argv[2]);
  if (pClient->fd == NULL)
  {
    client_printf(pClient, "Error opening file '%s'\n", argv[1]);
  }
}

/*
================================================================================
while command
================================================================================
*/
static void client_while(client_t *pClient, int argc, char *argv[])
{
  script_loop_t *pLoop;

  if (argc < 2)
  {
    client_printf(pClient, "usage: while expression\n");
    return;
  }

  if (pClient->pActiveScript == NULL)
  {
    client_printf(pClient, "WHILE statement only valid from a script\n");
    return;
  }

  /* Create a loop control variable */
  pLoop = malloc(sizeof(*pLoop));
  if (pLoop == NULL)
  {
    client_printf(pClient, "Out of memory creating loop control\n");
    return;
  }

  pLoop->type           = SCRIPT_LOOP_WHILE;
  pLoop->lineno         = pClient->pActiveScript->lineno;
  pLoop->loopControl    = malloc(strlen(argv[1]) + 1);
  pLoop->loopVar        = NULL;
  pLoop->loopIndex      = 0;
  if (pLoop->loopControl == NULL)
  {
    free(pLoop);
    client_printf(pClient, "Error allocating loop control\n");
    return;
  }
  strcpy(pLoop->loopControl, argv[1]);

  pLoop->block.type     = SCRIPT_BLOCK_LOOP;
  pLoop->block.filePos  = client_io_tell(pClient->pActiveScript->io);

  pClient->blockStack[pClient->blockIndex++] = &pLoop->block;
  ifproc_if(&pClient->ifProc, pLoop->loopControl, pClient->pActiveScript->lineno);
}

/*
================================================================================
for command
================================================================================
*/
static void client_for(client_t *pClient, int argc, char *argv[])
{
  int           val_index;
  int           x;
  char          *pStr;
  script_var_t  *pVar;
  script_var_t  *pEvVar;
  script_loop_t *pLoop;
  char          ch;

  /* Test if we have enough arguments */
  if (argc < 3)
  {
    client_printf(pClient, "Usage: for var = start[:step]:end\r\n");
    return;
  }

  /* Test for name=value format */
  val_index = 3;
  if ((pStr = strchr(argv[1], '=')) != NULL)
  {
    *pStr++ = 0;
    while (*pStr == ' ' || *pStr == '"')
      pStr++;
    argv[argc++] = "=";
    argv[argc++] = pStr;
  }
  else
  {
    if (argv[2] == NULL)
    {
      client_printf(pClient, "Usage: for var = start[:step]:end\r\n");
      return;
    }
    if (strcmp(argv[2], "=") == 0)
      val_index = 3;
  }

  /* Test if we have enough arguments */
  if (argc < 4)
  {
    client_printf(pClient, "Usage: for var = start[:step]:end\r\n");
    return;
  }

  /* Test if a script is running */
  if (pClient->pActiveScript == NULL)
  {
    client_printf(pClient, "'for' loops only valid when running a script file\n");
    return;
  }

  /* Create the variable */
  pVar = client_make_var(pClient, argv[1], &pClient->pLocals, 1);
  if (pVar == NULL)
    return;

  /* Create a loop control variable */
  pLoop = malloc(sizeof(*pLoop));
  if (pLoop == NULL)
  {
    client_printf(pClient, "Out of memory creating loop control\n");
    return;
  }

  /* Initialize the loop control */
  pLoop->type          = SCRIPT_LOOP_FOR;
  pLoop->lineno        = pClient->pActiveScript->lineno;
  pLoop->loopVar       = pVar;
  pLoop->loopControl   = malloc(strlen(argv[val_index]) + 1);
  pLoop->loopIndex     = 0;
  pLoop->block.filePos = client_io_tell(pClient->pActiveScript->io);
  pLoop->block.type    = SCRIPT_BLOCK_LOOP;
  if (pLoop->loopControl == NULL)
  {
    client_printf(pClient, "Error allocating loop control\n");
    return;
  }
  strcpy(pLoop->loopControl, argv[val_index]);

  /* Get initial variable value */
  pVar->type = SCRIPT_VAR_INT;

  /* Test for list control type */
  if (pLoop->loopControl[0] == '[')
  {
    /* Get value of 1st item in list */
    pVar->u.i_val = atoi(&pLoop->loopControl[1]);

    ch = pLoop->loopControl[pLoop->loopIndex];
    while (ch != ' ' && ch != ']')
    {
      pLoop->loopIndex++;
      ch = pLoop->loopControl[pLoop->loopIndex];
    }

    /* Skip whitespace */
    while (ch == ' ')
    {
      pLoop->loopIndex++;
      ch = pLoop->loopControl[pLoop->loopIndex];
    }
  }

  /* Test if a simple variable name supplied */
  else if ((pEvVar = client_find_var(pClient, pLoop->loopControl)) != NULL)
  {
    /* Control info is in a variable */
  }
  else
  {
    int   segment = 0;

    /* Must be start:stop or start:step:stop format */
    //pVar->u.i_val = atoi(pLoop->loopControl);

    /* Get step and stop values */
    pStr = pLoop->loopControl;
    for (x = 0; x <= strlen(pLoop->loopControl); x++)
    {
      eval_result_t value;
      int           ret;
      int           len;

      /* Find ':' control separator */
      if (pLoop->loopControl[x] == ':' || pLoop->loopControl[x] == '\0')
      {
        /* Separator found.  Get next value */
        len = &pLoop->loopControl[x] - pStr;
        ret = eval_solve(pClient->pEvalCtx, pStr, len, &value);
        if (ret != 0)
        {
          /* Error evaluating expression */
          client_printf(pClient, "Error evaluating expression '%.*s'\n",
              len, pStr);
          return;
        }

        if (value.type != EVAL_VAL_INT)
        {
          client_printf(pClient, "Invalid type for '%.*s; expected integer\n",
              len, pStr);
          return;
        }

        /* Populate pLoop control */
        if (segment == 0)
          pVar->u.i_val = (int) value.u.i_val;
        else if (segment == 1)
        {
          pLoop->forStop = (int) value.u.i_val;

          if (pLoop->forStop >= pVar->u.i_val)
            pLoop->forStep = 1;
          else
            pLoop->forStep = -1;
        }
        else
        {
          pLoop->forStep = pLoop->forStop;
          pLoop->forStop = (int) value.u.i_val;
        }

        segment++;
        pStr = &pLoop->loopControl[x+1];

#if 0
        /* Test for a 2nd ':' */
        if ((pStr = strchr(&pLoop->loopControl[x], ':')) == NULL)
        {
          pLoop->forStop = val;
          if (val >= pVar->u.i_val)
            pLoop->forStep = 1;
          else
            pLoop->forStep = -1;
        }
        else
        {
          pLoop->forStep = val;
          pLoop->forStop = atoi(pStr+1);
        }

        break;
#endif
      }
    }
  }

  /* Add this loop control variable */
  //pClient->loopStack[pClient->loopIndex++] = pLoop;
  pClient->blockStack[pClient->blockIndex++] = &pLoop->block;
}

/*
================================================================================
end command for loops
================================================================================
*/
static void client_end_loop(client_t *pClient, int argc, char *argv[])
{
  script_var_t  *pVar;
  script_loop_t *pLoop;
  int           at_loop_end;

  /* Test if we have and active loop */
  if (pClient->blockIndex == 0)
    return;

  /* Get the loop control struct */
  at_loop_end = 0;
  pLoop       = (script_loop_t *) pClient->blockStack[pClient->blockIndex-1];
  pVar        = pLoop->loopVar;

  /* Test if this is a for loop */
  switch (pLoop->type)
  {
    case SCRIPT_LOOP_WHILE:
      at_loop_end = !ifproc_should_process(&pClient->ifProc);
      ifproc_endif(&pClient->ifProc, pClient->pActiveScript->lineno);
      if (!at_loop_end)
        ifproc_if(&pClient->ifProc, pLoop->loopControl, pClient->pActiveScript->lineno);
      break;

    case SCRIPT_LOOP_FOR:
      /* Test for list of values */
      if (pLoop->loopControl[0] == '[')
      {
        /* Get next value from the loopControl string */
        if (pLoop->loopControl[pLoop->loopIndex] == ']')
        {
          at_loop_end = 1;
        }
        else
        {
          /* Get value of next item in list */
          pVar->u.i_val = atoi(&pLoop->loopControl[pLoop->loopIndex]);

          /* Skip the item */
          while (pLoop->loopControl[pLoop->loopIndex] != ' ' &&
                 pLoop->loopControl[pLoop->loopIndex] != ']' &&
                 pLoop->loopControl[pLoop->loopIndex] != '\0')
          {
            pLoop->loopIndex++;
          }

          /* Skip whitespace */
          while (pLoop->loopControl[pLoop->loopIndex] == ' ')
            pLoop->loopIndex++;
        }
      }
      else
      {
        /* Step the variable value */
        pVar->u.i_val += pLoop->forStep;
        if (pLoop->forStep > 0 && pVar->u.i_val > pLoop->forStop)
          at_loop_end = 1;
        else if (pLoop->forStep < 0 && pVar->u.i_val < pLoop->forStop)
          at_loop_end = 1;
      }

      break;
  }

  /* Test if we are at the loop end and rewind FD if not */
  if (!at_loop_end)
  {
    client_io_seek(pClient->pActiveScript->io, pLoop->block.filePos);
    pClient->pActiveScript->lineno = pLoop->lineno;
  }

  /* If we are at the loop end, then remove the loop */
  if (at_loop_end)
  {
    free(pLoop->loopControl);
    free(pLoop);
    pClient->blockIndex--;
    pClient->blockStack[pClient->blockIndex] = NULL;
  }
}

/*
================================================================================
'if' statement 'end' command handler
================================================================================
*/
static void client_endif(client_t *pClient, int argc, char *argv[])
{
  if (argc < 1)
  {
    client_printf(pClient, "usage: endif\n");
    return;
  }

  ifproc_endif(&pClient->ifProc, pClient->pActiveScript->lineno);

  pClient->blockIndex--;
  free(pClient->blockStack[pClient->blockIndex]);
  pClient->blockStack[pClient->blockIndex] = NULL;
}

/*
================================================================================
end command
================================================================================
*/
static void client_end(client_t *pClient, int argc, char *argv[])
{
  script_block_t  *pBlock;

  if (argc < 1)
  {
    client_printf(pClient, "usage: end\n");
    return;
  }

  /* Test if we have and active block */
  if (pClient->blockIndex == 0)
    return;

  /* Get the block control struct */
  pBlock      = pClient->blockStack[pClient->blockIndex-1];
  if (pBlock == NULL)
    return;

  switch (pBlock->type)
  {
    case SCRIPT_BLOCK_LOOP:
      client_end_loop(pClient, argc, argv);
      break;

    case SCRIPT_BLOCK_IF:
      client_endif(pClient, argc, argv);
      break;

    case SCRIPT_BLOCK_FUNC:
      client_endfun(pClient, argc, argv);
      break;

    default:
      /* What to do... */
      break;
  }
}

/*
================================================================================
call function with result variable and parameters
================================================================================
*/
static int client_call_function_params(client_t *pClient, const char *fname,
    script_fn_t *pFn, script_var_t *pParams, script_var_t *pResult)
{
  script_var_t      *pVar;
  client_mem_io_t   io;

  io.io.vptbl   = &client_mem_io_vptbl;
  io.pos        = 0;
  io.size       = pFn->datalen;
  io.pData      = pFn->pData;

  /* If we have a result variable, make it as a local variable */
  if (pFn->rname[0])
  {
    pVar = client_make_var(pClient, pFn->rname, &pClient->pLocals, 1);
    if (pVar == NULL)
      return -1;
  }

  client_run_io_script(pClient, fname, pParams, &io.io);

  /* Take our result variable and either copy it out, or release it */
  if (pFn->rname[0])
  {
    /* Pop it from locals */
    pVar = pClient->pLocals;
    pClient->pLocals = pVar->pNext;
    if (pResult == NULL)
    {
      /* Nobody wants you... */
      client_del_var(pClient, pVar);
    }
    else
    {
      /* Copy it out */
      pResult->type = pVar->type;
      pResult->u = pVar->u;
      free(pVar);
    }
  }
  else if (pResult == NULL)
  {
    pResult->type = SCRIPT_VAR_NAN;
  }

  return 0;
}

/*
================================================================================
call function with result variable
================================================================================
*/
static void client_call_function(client_t *pClient, script_var_t *pResult,
    int argc, char *argv[])
{
  script_fn_t       *pFn;
  script_var_t      *pVar;
  script_var_t      *pParams;
  script_fn_param_t *pFnParam;
  char              *fname;
  int               argn;
  int               x;

  fname = argv[1];

  /* If we find a space, we split off the first parameter */
  x = strcspn(fname, " \t");
  if (fname[x] != 0)
  {
    argv[1] = fname + x + strspn(fname + x, " \t");
    fname[x] = 0;
    argn = 1;
  }
  else
  {
    argn = 2;
  }

  /* Locate the function */
  pVar = client_find_var(pClient, fname);
  if (pVar == NULL)
  {
    client_printf(pClient, "no such function '%s'\n", fname);
    return;
  }

  /* Make sure it's actually a function */
  if (pVar->type != SCRIPT_VAR_FUNC)
  {
    client_printf(pClient, "'%s' is not a function\n", fname);
    return;
  }

  pFn = pVar->u.fn_val;

  /* Create argument variables */
  pFnParam = pFn->pParameters;
  pParams = NULL;
  for (; argn < argc; ++argn)
  {
    eval_result_t value;

    if (pFnParam == NULL)
    {
      client_printf(pClient, "Too many arguments for function '%s'\n", fname);
      goto unwind_params;
    }

    if (eval_solve(pClient->pEvalCtx, argv[argn], -1, &value) != 0)
    {
      client_printf(pClient, "Unable to evaluate expression '%s'\n", argv[argn]);
      goto unwind_params;
    }

    pVar = client_make_var(pClient, pFnParam->name, &pParams, 1);
    if (pVar == NULL)
      goto unwind_params;

    if (client_var_from_eval(pClient, 0, pVar, &value) != 0)
      goto unwind_params;

    pFnParam = pFnParam->pNext;
  }

  if (client_call_function_params(pClient, fname, pFn, pParams, pResult) != 0)
    goto unwind_params;

  return;

unwind_params:
  while (pParams != NULL)
  {
    script_var_t *pNext = pParams->pNext;
    client_del_var(pClient, pParams);
    pParams = pNext;
  }
}

/*
================================================================================
call function command
================================================================================
*/
static void client_call(client_t *pClient, int argc, char *argv[])
{
  /* Test if we should execute based on if processing */
  if (!ifproc_should_process(&pClient->ifProc))
    return;

  if (argc < 2)
  {
    client_printf(pClient, "usage: call function [args..],\n");
    return;
  }

  client_call_function(pClient, NULL, argc, argv);
}

/*
================================================================================
delete function
================================================================================
*/
static void client_del_function(client_t *pClient, script_fn_t *pFn)
{
  while (pFn->pParameters != NULL)
  {
    script_fn_param_t *pNext = pFn->pParameters->pNext;
    free(pFn->pParameters);
    pFn->pParameters = pNext;
  }
  if (pFn->pData != NULL)
    free(pFn->pData);
  free(pFn);
}

/*
================================================================================
function command
================================================================================
*/
static void client_function(client_t *pClient, int argc, char *argv[])
{
  script_fn_param_t *pParam;
  script_fn_param_t **ppParams;
  script_var_t      *pVar;
  script_fn_t       *pFn;
  char              *ptr;
  char              name[64];
  int               count;
  int               len;

  if (argc < 2)
  {
    client_printf(pClient, "usage: function [var =] name(args)\n");
    return;
  }

  if (pClient->pActiveScript == NULL)
  {
    client_printf(pClient, "function only valid from a script\n");
    return;
  }

  /* Test if we should execute based on if processing */
  if (!ifproc_should_process(&pClient->ifProc))
  {
    pClient->terminate_script = 1;
    client_printf(pClient, "function only valid from outer scope\n");
    return;
  }

  /* Create a function object */
  pFn = malloc(sizeof(*pFn));
  if (pFn == NULL)
  {
    pClient->terminate_script = 1;
    client_printf(pClient, "Out of memory creating function object\n");
    return;
  }

  pFn->pParameters    = NULL;
  pFn->lineno         = pClient->pActiveScript->lineno;
  pFn->block.filePos  = client_io_tell(pClient->pActiveScript->io);
  pFn->block.type     = SCRIPT_BLOCK_FUNC;

  // expected:
  //   function res = name(args)
  //   function name(args)
  ptr = argv[1];

  // Grab the return variable name
  count = strcspn(ptr, "=( \t");
  len = count;
  if (len > sizeof(pFn->rname) - 1)
    len = sizeof(pFn->rname) - 1;
  strncpy(pFn->rname, ptr, len);
  pFn->rname[len] = 0;
  ptr += count;

  // Skip whitespace
  ptr += strspn(ptr, " \t");
  if (*ptr == '=')
  {
    ptr += 1;

    // Grab function name
    ptr += strspn(ptr, " \t");
    count = strcspn(ptr, "( \t");
    len = count;
    if (len > sizeof(name) - 1)
      len = sizeof(name) - 1;
    strncpy(name, ptr, len);
    name[len] = 0;

    pVar = client_make_var(pClient, name, NULL, 0);
    if (pVar == NULL)
    {
      pClient->terminate_script = 1;
      free(pFn);
      return;
    }
    ptr += count;
  }
  else if (*ptr == '(')
  {
    // There was no return var name, that was the function name
    pVar = client_make_var(pClient, pFn->rname, NULL, 0);
    if (pVar == NULL)
    {
      pClient->terminate_script = 1;
      free(pFn);
      return;
    }
    pFn->rname[0] = 0;
  }
  else
  {
    pClient->terminate_script = 1;
    client_printf(pClient, "Invalid function syntax\n");
    free(pFn);
    return;
  }
  ptr += strspn(ptr, " \t");

  /* Delete prev value, if any */
  client_del_var_data(pClient, pVar);
  pVar->type = SCRIPT_VAR_FUNC;
  pVar->u.fn_val = pFn;

  // Find parameter start
  if (*ptr != '(')
  {
    pClient->terminate_script = 1;
    pVar->type = SCRIPT_VAR_NAN;
    client_printf(pClient, "Invalid function syntax\n");
    free(pFn);
    return;
  }
  ptr += 1;

  // Grab parameter names
  ppParams = &pFn->pParameters;
  ptr += strspn(ptr, " \t");
  count = strcspn(ptr, " \t,)");
  while (count > 0)
  {
    pParam = malloc(sizeof(*pParam));
    if (pParam == NULL)
      break;

    *ppParams = pParam;
    ppParams = &pParam->pNext;
    pParam->pNext = NULL;

    len = count;
    if (len > sizeof(pParam->name) - 1)
      len = sizeof(pParam->name) - 1;

    strncpy(pParam->name, ptr, len);
    pParam->name[len] = 0;

    ptr += count;
    ptr += strspn(ptr, " \t");
    if (*ptr != ',')
      break;
    ptr += 1;
    ptr += strspn(ptr, " \t");
    count = strcspn(ptr, " \t,)");
  }

  if (*ptr != ')')
  {
    pClient->terminate_script = 1;
    pVar->type = SCRIPT_VAR_NAN;
    while (pFn->pParameters != NULL)
    {
      pParam = pFn->pParameters;
      pFn->pParameters = pParam->pNext;
      free(pParam);
    }
    free(pFn);
    return;
  }

  // Don't actually execute anything, just find the end
  ifproc_if(&pClient->ifProc, "0", pClient->pActiveScript->lineno);
  pClient->blockStack[pClient->blockIndex++] = &pFn->block;
}

/*
================================================================================
endfun command
================================================================================
*/
static void client_endfun(client_t *pClient, int argc, char *argv[])
{
  script_block_t    *pBlock;
  script_fn_t       *pFn;
  int               endPos;
  int               len;

  if (argc < 1)
  {
    client_printf(pClient, "usage: endfun\n");
    return;
  }

  /* Test if we have an active func */
  if (pClient->blockIndex == 0)
    return;

  /* Get the context */
  pBlock    = pClient->blockStack[pClient->blockIndex-1];

  /* Pop the block */
  pClient->blockIndex--;
  pClient->blockStack[pClient->blockIndex] = NULL;

  pFn       = (script_fn_t *)pBlock;
  ifproc_endif(&pClient->ifProc, pClient->pActiveScript->lineno);

  /* Save end position */
  endPos = client_io_tell(pClient->pActiveScript->io);

  /* Rewind to function start */
  client_io_seek(pClient->pActiveScript->io, pFn->block.filePos);

  /* Read in function data */
  len = endPos - pFn->block.filePos;
  pFn->pData = malloc(len);
  if (pFn->pData != NULL)
  {
    int nlcount = 0;

    len = client_io_read(pClient->pActiveScript->io, pFn->pData, len);
    /* Trim the "end" off */
    while (len > 0)
    {
        if (pFn->pData[len - 1] == '\n')
        {
            nlcount += 1;
            if (nlcount >= 2)
                break;
        }
        len -= 1;
    }
    pFn->datalen = len;
  }

  /* Restore position */
  client_io_seek(pClient->pActiveScript->io, endPos);
}

/*
================================================================================
if command
================================================================================
*/
static void client_if(client_t *pClient, int argc, char *argv[])
{
  script_block_t  *pBlock;

  if (argc < 2)
  {
    client_printf(pClient, "usage: if expression\n");
    return;
  }

  if (pClient->pActiveScript == NULL)
  {
    client_printf(pClient, "IF statment only valid from a script\n");
    return;
  }

  /* Add a new IF block to the block stack */
  pBlock = (script_block_t *) malloc(sizeof(script_block_t));
  pClient->blockStack[pClient->blockIndex++] = pBlock;
  pBlock->type = SCRIPT_BLOCK_IF;
  pBlock->filePos = client_io_tell(pClient->pActiveScript->io);

  /* Call the if processor */
  ifproc_if(&pClient->ifProc, argv[1], pClient->pActiveScript->lineno);
}

/*
================================================================================
else command
================================================================================
*/
static void client_else(client_t *pClient, int argc, char *argv[])
{
  if (argc < 1)
  {
    client_printf(pClient, "usage: %s\n", argv[0]);
    return;
  }

  if (pClient->pActiveScript == NULL)
  {
    client_printf(pClient, "ELSE statment only valid from a script\n");
    return;
  }

  ifproc_else(&pClient->ifProc, pClient->pActiveScript->lineno);
}

/*
================================================================================
elseif command
================================================================================
*/
static void client_elseif(client_t *pClient, int argc, char *argv[])
{
  if (argc < 2)
  {
    client_printf(pClient, "usage: %s expression\n", argv[0]);
    return;
  }

  if (pClient->pActiveScript == NULL)
  {
    client_printf(pClient, "ELSEIF statment only valid from a script\n");
    return;
  }

  ifproc_elif(&pClient->ifProc, argv[1], pClient->pActiveScript->lineno);
}

/*
================================================================================
input function with result variable
================================================================================
*/
static void client_input_function(client_t *pClient, script_var_t *pResult,
    int argc, char *argv[])
{
  int               len;
  int               typeStr = 0;
  char              str[512];

  /* Test if we should execute based on if processing */
  if (!ifproc_should_process(&pClient->ifProc))
    return;

  /* Validate we have an input function */
  if (pClient->pInputGet == NULL || pClient->printf == NULL)
    return;

  /* Validate we have enough args */
  if (argc < 2)
  {
    client_printf(pClient, "usage: input prompt\n");
    return;
  }

  /* Test if string var requested */
  if (argc > 3 && strcmp(argv[2], "'s'") == 0)
    typeStr = 1;

  /* Print the prompt */
  if (argv[1][0] == '"')
    snprintf(str, sizeof(str), "%s", &argv[1][1]);
  else
    snprintf(str, sizeof(str), "%s", argv[1]);
  len = strlen(str);
  if (len && str[len-1] == '"')
    str[len-1] = 0;

  client_printf(pClient, str);

  /* Get the input */
  if ((len = pClient->pInputGet(pClient->pInputCtx, str, sizeof(str)) > 0))
  {
    /* Provide result */
    if (pResult != NULL)
    {
      /* Copy it out */
      if (typeStr)
      {
        pResult->type = SCRIPT_VAR_STRING;
        pResult->u.s_val = malloc(len + 1);
        strcpy(pResult->u.s_val, str);
      }
      else
      {
        /* Test for empty string */
        if (strlen(str) == 0)
        {
            pResult->type = SCRIPT_VAR_NAN;
        }
        else
        {
          eval_result_t value;

          /* Evaluate the input */
          if (eval_solve(pClient->pEvalCtx, str, -1, &value) != 0)
          {
            pResult->type = SCRIPT_VAR_NAN;
            client_printf(pClient, "Unable to evaluate '%s'\n", str);
            return;
          }

          if (client_var_from_eval(pClient, 0, pResult, &value) != 0)
            return;
        }
      }
    }
  }

  /* Terminate the line */
  client_printf(pClient, "\n");
}

/*
================================================================================
disp command
================================================================================
*/
static void client_disp(client_t *pClient, int argc, char *argv[])
{
  char *text;
  int len;
  int x;

  /* Test if we should execute based on if processing */
  if (!ifproc_should_process(&pClient->ifProc))
    return;

  if (argc < 2)
  {
    client_printf(pClient, "usage: disp items\n");
    return;
  }

  for (x = 1; x < argc; x++)
  {
    text = argv[x];
    if (text[0] == '"')
    {
      len = strlen(text);
      if (len > 0 && text[len - 1] == '"')
        text[len - 1] = 0;
      text += 1;
    }
    client_printf(pClient, "%s ", text);
  }
  client_printf(pClient, "\n");
}

/*
================================================================================
open socket connection
================================================================================
*/
int client_open_connection(client_t *pClient)
{
  struct addrinfo hints;
  struct addrinfo *res;
  struct addrinfo *res_p;
  char port[16];
  int rc;

  snprintf(port, sizeof(port), "%d", pClient->port);

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_ADDRCONFIG;
  hints.ai_protocol = 0;

  if (pClient->sockfd != 0)
    close(pClient->sockfd);

  rc = getaddrinfo(pClient->server_ip, port, &hints, &res);
  if (rc != 0)
  {
    client_printf(pClient, "Error : Could not get address info\n");
    pClient->sockfd = 0;
    return -ENOENT;
  }

  for (res_p = res; res_p != NULL; res_p = res_p->ai_next)
  {
    pClient->sockfd = socket(res_p->ai_family, res_p->ai_socktype, res_p->ai_protocol);
    if (pClient->sockfd < 0)
      continue;

    if (connect(pClient->sockfd, res_p->ai_addr, res_p->ai_addrlen) >= 0)
      break;

    close(pClient->sockfd);
  }

  freeaddrinfo(res);

  if (pClient->debug)
    client_printf(pClient, "socket created\n");

  if (res_p == NULL)
  {
    client_printf(pClient, "Error : Connect Failed\n");
    pClient->sockfd = 0;
    return -ENOENT;
  }

  /* Test for TCL connection type */
  if (pClient->port == 5001)
  {
    char    str[32];
    /* Read the telnet hello message */
    client_read(pClient, 28);
    
    /* Set the F1000_id interface number */
    sprintf(str, "i %d\r\n", pClient->f1000_id - 1);
    write(pClient->sockfd, str, strlen(str));
  }

  /* Initialize the F1000 regs context */
  if (f1000_regs_init(pClient, pClient->printf, pClient->pPrintfCtx,
      pClient->printfColors) != 0)
  {
    /* Error initializing */
    client_printf(pClient, "Failed to init f1000 regs\n");
    return -ENOENT;
  }
 
  client_printf(pClient, "\r\nconnected to server\n");

  return 0;
}

/*
================================================================================
logger command
================================================================================
*/
static void client_logger(client_t *pClient, int argc, char *argv[])
{
  uint32_t      * ptr;
  int             tx_size;
  int             n;
  char          * filename;
  fd_set          fds;
  int             ret;
  int             rx_actual;
  FILE          * fd;
  int             x;
  uint32_t        f1000_id;
  uint32_t        ep_id;
  uint32_t        cmd_id;
  uint32_t        addr;
  uint32_t        value;
  char            buf[1024];
  int             echo = 0;

  /* Test if we should execute based on if processing */
  if (!ifproc_should_process(&pClient->ifProc))
    return;

  /* Validate the arguments */
  if (argc < 2)
  {
    client_printf(pClient, "usage: logger filename [echo]\n");
    return;
  }

  if (argc == 3 && strcmp(argv[2], "echo") == 0)
    echo = 1;

  memset(pClient->rxdbuff, 0, sizeof(pClient->rxdbuff));
  memset(pClient->txdbuff, 0, sizeof(pClient->txdbuff));
  ptr = (uint32_t *) pClient->txdbuff;

  /* Copy API and f1000/ep id to txdbuff */
  *ptr++ = IQA_LOGGER;

  /* Write to the socket to activate LOGGER mode */
  tx_size = 4;
  n = write(pClient->sockfd, pClient->txdbuff, tx_size);
  if (n < 0)
  {
  	perror("Error transmitting to server");
  	return;
  }

  /* Read the response */
  rx_actual = client_read(pClient, 8);
  if (rx_actual != 8)
  {
    return;
  }

  /* Try to open the file */
  filename = argv[1];
  if ((fd = fopen(filename, "w+")) == NULL)
  {
    client_printf(pClient, "Error opening %s\n", filename);

    /* Terminate LOGGER mode */
    n = write(pClient->sockfd, pClient->txdbuff, tx_size);
    client_read(pClient, 4);
    return;
  }

  if (pClient->debug)
    client_printf(pClient, "Awaiting log entries\n");

  /* Loop until told to quit at the keyboard (or the socket closes) */
  while (1)
  {
    /* Poll the socket for data */
    FD_ZERO(&fds);
    FD_SET(0, &fds);
    FD_SET(pClient->sockfd, &fds);

    ret = select(pClient->sockfd+1, &fds, NULL, NULL, NULL);

    if (ret == -1)
    {
      /* Test for EINTR */
      if (errno == EINTR)
        continue;

      n = write(pClient->sockfd, pClient->txdbuff, tx_size);
      client_read(pClient, 8);
      return;
    }

    /* Test for data from the socket */
    if (FD_ISSET(pClient->sockfd, &fds))
    {
      /* Read 12 bytes of header */
      rx_actual = client_read(pClient, 12);
      if (rx_actual >= 12)
      {
        uint32_t  logger_id;
        uint32_t  api_id;
        int       len;

        if (pClient->debug)
        {
          ptr = (uint32_t *) pClient->rxdbuff;
          for (x = 0; x < 8; x++)
            printf("  0x%08X\n", *ptr++);
        }

        ptr = (uint32_t *) pClient->rxdbuff;

        logger_id = *ptr++;
        api_id    = *ptr++;
        len       = *ptr++;

        if (rx_actual < 12 + len)
        {
          int   rx_new;
          memcpy(buf, &pClient->rxdbuff[12], rx_actual-len);
          rx_new += client_read_exact(pClient, 12 + len - rx_actual);
          memcpy(&buf[rx_actual], pClient->rxdbuff, rx_new);
        }
        else
          memcpy(buf, &pClient->rxdbuff[12], len);

        if (logger_id != IQA_LOG_ENTRY)
        {
          /* Hmm, some other thing recieved */
          client_printf(pClient, "Unexpected ID during logging: 0x%08X\n", logger_id);
        }
        else
        {
          /* Read remaining data for this API */
          //if (rx_actual == len)
          {
            /* Log data to the file */

            ptr = (uint32_t *) buf;
            cmd_id = *ptr++;
            char *sys = (char *) ptr;
            f1000_id = *ptr++;
            ep_id = *ptr++;
            addr = *ptr++;
            value = *ptr++;
            (void) f1000_id;
            (void) cmd_id;

            switch (api_id)
            {
              case IQA_READ_REG:
                fprintf(fd, "EP%c read  0x%08X: 0x%08X\n", 'A' + ep_id - 1, addr, value);
                if (echo)
                  printf("EP%c read  0x%08X: 0x%08X\n", 'A' + ep_id - 1, addr, value);
                break;

              case IQA_WRITE_REG:
                fprintf(fd, "EP%c write 0x%08x= 0x%08X\n", 'A' + ep_id - 1, addr, value);
                if (echo)
                  printf("EP%c write 0x%08x= 0x%08X\n", 'A' + ep_id - 1, addr, value);
                break;

              case IQA_SYSTEM:
                *(sys + len) = 0;
                fprintf(fd, "system(\"%s\")\n", sys);
                if (echo)
                  printf("system(\"%s\")\n", sys);
                break;
            }
          }
        }
      }
    }

    /* Test for data from the socket */
    if (FD_ISSET(0, &fds))
    {
      break;
    }
  }

  /* Terminate LOGGER mode */
  n = write(pClient->sockfd, pClient->txdbuff, tx_size);
  client_read(pClient, 4);
}

/*
================================================================================
Substitute variables from input line
================================================================================
*/
void client_subst_vars(client_t *pClient, const char *line,
              char *subst_line, int len)
{
  int           x, y;
  int           dest = 0;
  int           paren_level = 0;
  char          var_name[256];
  int           name_idx = 0;
  char          var_subst[256];
  char          ch_open;
  char          ch_close;
  script_var_t *pVar;

  /* Loop through source line */
  for (x = 0; line[x] != 0; x++)
  {
    if ((line[x] == '$' && line[x+1] == '(') || (line[x] == '[' && line[x+1] != 0))
    {
      paren_level++;

      if (line[x] == '$')
      {
        x++;
        ch_open = '(';
        ch_close = ')';
      }
      else
      {
        subst_line[dest++] = '[';
        ch_open = '[';
        ch_close = ']';
      }

      /* Get variable name */
      while (paren_level > 0 && line[x+1] != 0)
      {
        x++;
        var_name[name_idx++] = line[x];
        if (line[x] == ch_open)
          paren_level++;
        else if (line[x] == ch_close)
          paren_level--;
      }

      /* We have the variable name.  Do variable substitution on it */
      var_name[name_idx-1] = 0;
      client_subst_vars(pClient, var_name, var_subst, sizeof(var_subst));
      name_idx = 0;

      /* Find the variable */
      if (paren_level == 0)
        pVar = client_find_var(pClient, var_subst);
      else
        pVar = NULL;

      if (pVar == NULL)
      {
        /* Just copy directly */
        for (y = 0; var_subst[y] != 0; y++)
        {
          subst_line[dest++] = var_subst[y];

          /* Test for max len */
          if (dest+1 == len)
          {
            subst_line[dest] = 0;
            return;
          }
        }
      }
      else
      {
        /* Substitute with the variable value */
        switch (pVar->type)
        {
        case SCRIPT_VAR_STRING:
          dest += snprintf(subst_line + dest, len - dest, "%s", pVar->u.s_val);
          break;

        case SCRIPT_VAR_INT:
          dest += snprintf(subst_line + dest, len - dest, "%d", pVar->u.i_val);
          break;

        case SCRIPT_VAR_QWORD:
          dest += snprintf(subst_line + dest, len - dest, "%" PRId64, pVar->u.q_val);
          break;

        case SCRIPT_VAR_FLOAT:
          dest += snprintf(subst_line + dest, len - dest, "%f", pVar->u.f_val);
          break;

        case SCRIPT_VAR_NAN:
          dest += snprintf(subst_line + dest, len - dest, "NaN");
          break;

        default:
          break;
        }
      }

      /* Terminate the bracket if ch_open == '[' */
      if (ch_open == '[')
        --x;
    }
    else
    {
      subst_line[dest++] = line[x];

      /* Test for max len */
      if (dest+1 == len)
      {
        subst_line[dest] = 0;
        return;
      }
    }
  }

  subst_line[dest] = 0;
}

/*
================================================================================
Parse arguments
================================================================================
*/
static int client_parse_args(char *mutable_line, char **cmd_argv, int cmd_argmax)
{
  int       sep;
  int       inquote;
  int       brkt_level;
  char      brkt_stack[16];
  int       cmd_argc;
  int       args_done;
  char      *arg;

  /* Build argv array */
  cmd_argv[0] = mutable_line;
  while(*cmd_argv[0] == ' ' || *cmd_argv[0] == '\t')
    cmd_argv[0]++;
  cmd_argc = 1;
  args_done = 0;

  /* Find all arguments */
  arg = cmd_argv[0];
  inquote = 0;
  brkt_level = 0;
  sep = ' ';
  while (cmd_argc < cmd_argmax && !args_done)
  {
    while (inquote || brkt_level || (*arg != sep && *arg != '\0' &&
          *arg != '\t' && *arg != '#' && *arg != '%' && *arg != '=' &&
          *arg != ','))
    {
      /* Always break at end of line */
      if (*arg == '\0')
        break;

      /* Test for quote character */
      if (*arg == '"')
      {
        inquote ^= 1;
      }

      /* Test for brackets */
      else if (!inquote)
      {
        if (brkt_level < sizeof(brkt_level))
        {
          if (*arg == '[')
            brkt_stack[brkt_level++] = ']';
          else if (*arg == '(')
            brkt_stack[brkt_level++] = ')';
        }

        if (brkt_level > 0)
        {
          if (*arg == brkt_stack[brkt_level - 1])
            brkt_level--;
        }
      }

      arg++;
    }

    /* Test for comment */
    if (*arg == '#' || *arg == '%')
    {
      *arg = '\0';
      break;
    }

    /* Test for '=' character */
    if (*arg == '=' && *(arg-1) != sep && *(arg-1) != 0 &&
        *arg != ',')
    //if (*arg == '=' && *(arg-1) != ' ' && *(arg-1) != 0 &&
        //*(arg-1) != '>' && *(arg-1) != '<' && *(arg-1) != '!')
    {
      /* Terminate the previous arg */
      if (*(arg+1) == '=')
      {
        cmd_argv[cmd_argc++] = "==";
        arg++;
      }
      else
      {
        cmd_argv[cmd_argc++] = "=";
        args_done = 1;
      }
    }

    if (*arg != '\0')
    {
      /* NULL terminate the arg */
      if (*arg == '=')
      {
#if 0
          if (*(arg+1) == '=')
          {
            cmd_argv[cmd_argc-1] = "==";
            arg++;
          }
          else if (*(arg-1) == '!')
          {
            cmd_argv[cmd_argc-1] = "!=";
            arg++;
          }
          else if (*(arg-1) == '<')
          {
            cmd_argv[cmd_argc-1] = "<=";
            arg++;
          }
          else if (*(arg-1) == '>')
          {
            cmd_argv[cmd_argc-1] = ">=";
            arg++;
          }
          else
          {
#endif
          cmd_argv[cmd_argc-1] = "=";
          args_done = 1;
        //  }
      }

      *arg = '\0';
      arg++;
    }

    /* Skip whitespace between args */
    while (*arg == ' ' || *arg == ',' || *arg == '\t')
      arg++;

    /* Test if another argument given */
    if (*arg == '\0' || *arg == '#' || *arg == '%')
    {
      /* No, break the loop */
      break;
    }

    /* Get pointer to next arg */
    cmd_argv[cmd_argc++] = arg;

    /* Test for quote arg */
    if (*arg == '"')
    {
      /* Skip the quote and mark that we are in quote */
      arg++;
      inquote = 1;
    }

    /* For "if" statement, skip splitting of args */

    if (cmd_argc == 2)
    {
      if (strcmp(cmd_argv[0], "if") == 0 ||
          strcmp(cmd_argv[0], "elif") == 0 ||
          strcmp(cmd_argv[0], "elseif") == 0 ||
          strcmp(cmd_argv[0], "function") == 0 ||
          strcmp(cmd_argv[0], "while") == 0 ||
          strcmp(cmd_argv[0], "echo") == 0 ||
          strcmp(cmd_argv[0], "system") == 0 ||
          strcmp(cmd_argv[0], "pause") == 0)
      {
        args_done = 1;
      }

      if (strcmp(cmd_argv[0], "reg") == 0 ||
          strcmp(cmd_argv[0], "reg16") == 0)
        sep = 255;
    }

    else if (cmd_argc == 3)
    {
      if (strcmp(cmd_argv[0], "print") == 0 ||
          strcmp(cmd_argv[0], "call") == 0 ||
          strcmp(cmd_argv[0], "run") == 0 ||
          strcmp(cmd_argv[0], "p") == 0)
        sep = ',';
    }
  }
  return cmd_argc;
}

/*
================================================================================
Process the next line of input (either from keyboard or file)
================================================================================
*/
int client_process_line(client_t *pClient, char *line, int script)
{
  int       len;
  int       x;
  int       cmd_argc;
  char      *cmd_argv[20];
  char      subst_line[512];

  /* Skip whitespace */
  while (*line == ' ')
    line++;

  /* Remove trailing whitespace and semicolon */
  len = strlen(line);
  while (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r' ||
      line[len-1] == ' ' || line[len-1] == ';'))
  {
    line[--len] = '\0';
  }

  /* Test for zero length line */
  if (len == 0)
    return 0;

  /* Test for comment line */
  if (line[0] == '#' || line[0] == '%')
    return 0;

  /* If not running  a script, then add to cmd history */
  if (!pClient->is_script)
    add_history(line);

  /* Do variable substitution */
  client_subst_vars(pClient, line, subst_line, sizeof(subst_line));

  /* Test for external command processing */
  if (subst_line[0] == '!' && !(subst_line[1] == '-' || isdigit(subst_line[1]))) {
      FILE *fp;

      if (!ifproc_should_process(&pClient->ifProc))
        return 0;

      fp = popen(subst_line + 1, "r");
      if (fp == NULL) {
          client_printf(pClient, "error: unable to execute command\n");
          return -1;
      }
      while (fgets(subst_line, sizeof(subst_line), fp)) {
          client_printf(pClient, "%s", subst_line);
      }
      return pclose(fp);
  }

  /* Build argv array */
  cmd_argc = client_parse_args(subst_line, cmd_argv, 20);

  /* See if this is a function call */
  if (strchr(cmd_argv[0], '(') != NULL)
  {
    eval_result_t value;

    if (cmd_argc != 1)
    {
      client_printf(pClient, "error: bad syntax after '%s'\n", cmd_argv[0]);
      return -1;
    }

    if (eval_solve(pClient->pEvalCtx, cmd_argv[0], -1, &value) != 0)
    {
      client_printf(pClient, "error: bad expression '%s'\n", cmd_argv[0]);
      return -1;
    }

    return 1;
  }

  /* Search through all known commands */
  for (x = 0; x < g_cmd_count; x++)
  {
    /* Compare the next function */
    if (strcmp(cmd_argv[0], g_cmds[x].name) == 0)
    {
      /* Call the function */
      g_cmds[x].func(pClient, cmd_argc, cmd_argv);
      return 1;
    }
  }

  /* Test if we should execute based on if processing */
  if (!ifproc_should_process(&pClient->ifProc))
    return 1;

  /* Test for unknown command */
  if (x == g_cmd_count)
  {
    /* Test for simple variable assignment format */
    if (cmd_argc > 1 && strcmp(cmd_argv[1], "=") == 0)
    {
      int x;

      /* Shift arguments up one */
      for (x = cmd_argc; x > 0; x--)
      {
        cmd_argv[x] = cmd_argv[x-1];
      }
      cmd_argc++;

      /* Check if lval is an F1000 register */
      if (f1000_regs_is_register(pClient->pRegs, cmd_argv[0]))
      {
        cmd_argv[0] = "s";
        client_set(pClient, cmd_argc, cmd_argv);
      }
      else
      {
        /* Must be a variable assignment */
        cmd_argv[0] = "var";
        client_var(pClient, cmd_argc, cmd_argv);
      }

      return 1;
    }

    else if (pClient->pAuxCmdProc)
    {
      return pClient->pAuxCmdProc(pClient->pAuxCtx, cmd_argc, cmd_argv);
    }

    /* Test for function call format */

    /* Else it is unknown */
    else
    {
      client_printf(pClient, "error: unknown command '%s'\n", cmd_argv[0]);
      return -1;
    }
  }

  return -1;
}

/*
================================================================================
Returns TRUE if a script is running
================================================================================
*/
int client_is_script_running(client_t *pClient)
{
  return pClient->is_script;
}

/*
================================================================================
Terminates a running script
================================================================================
*/
void client_terminate_script(client_t *pClient)
{
  /* Ensure we are running a script */
  if (pClient->is_script)
  {
    /* Set the terminate_script variable */
    pClient->terminate_script = 1;
  }
}

/*
================================================================================
Run the specified script by io
================================================================================
*/
static void client_run_io_script(client_t *pClient, const char *script,
    script_var_t *pParams, client_io_t *io)
{
  char  line[512];
  int   is_script;
  script_def_t* pPrevScript;
  script_def_t* pNewScript;
  script_var_t* pLocals;
  eval_ctx_t   *pEvalCtx;
  script_var_t *pVar;

  /* Get previous script mode settings */
  pEvalCtx    = pClient->pEvalCtx;
  pLocals     = pClient->pLocals;
  is_script   = pClient->is_script;
  pPrevScript = pClient->pActiveScript;
  if (is_script == 0)
    pClient->terminate_script = 0;

  /* Setup the new script structure */
  pNewScript  = (script_def_t *) malloc(sizeof(script_def_t));
  if (pNewScript == NULL)
  {
    client_printf(pClient, "Error allocating script memory\n");
    return;
  }

  /* Swap to new evaluation context */
  pNewScript->evalCtx.vptbl     = pClient->evalCtx.vptbl;
  pNewScript->evalCtx.userdata  = pClient->evalCtx.userdata;
  pClient->pEvalCtx             = &pNewScript->evalCtx;

  pClient->scriptStack[pClient->scriptIndex++] = pNewScript;
  pNewScript->io       = io;
  pNewScript->lineno   = 0;
  strcpy(pNewScript->script_file, script);

  /* Prepend any parameters onto our locals */
  if (pParams)
  {
    pVar = pParams;
    while (pVar->pNext)
      pVar = pVar->pNext;
    pVar->pNext = pClient->pLocals;
    pClient->pLocals = pParams;
  }

  /* Set script mode */
  pClient->is_script     = 1;
  pClient->pActiveScript = pNewScript;
  ifproc_set_filename(&pClient->ifProc, pClient->pActiveScript->script_file);

  /* Read all lines from the file and process them */
  while (client_io_gets(io, line, sizeof(line)) != NULL)
  {
    /* Test for script termination */
    if (pClient->terminate_script)
      break;

    pNewScript->lineno++;
    client_process_line(pClient, line, 1);
  }

  /* Restore previous script */
  pClient->pEvalCtx = pEvalCtx;
  pClient->scriptIndex--;
  pClient->is_script = is_script;
  pClient->pActiveScript = pPrevScript;
  if (pClient->pActiveScript)
    ifproc_set_filename(&pClient->ifProc, pClient->pActiveScript->script_file);

  /* Pop & free local variables */
  pVar = pClient->pLocals;
  while (pVar != pLocals)
  {
    script_var_t *pNext = pVar->pNext;

    client_del_var(pClient, pVar);
    pVar = pNext;
  }
  pClient->pLocals = pLocals;

  /* If script was terminated, then print message */
  if (is_script == 0 && pClient->terminate_script)
  {
    client_printf(pClient, "\r\nScript '%s' terminated on line %d\n",
        script, pNewScript->lineno);
  }

  /* Free the script memory */
  free(pNewScript);
}


/*
================================================================================
Run the specified script
================================================================================
*/
static int client_run_script_internal(client_t *pClient, int quiet,
    char *script, int argc, char *argv[])
{
  FILE  *fd;
  char  line[512];
  int   x;
  script_var_t *pVar;
  script_var_t *pParams;
  client_file_io_t io;

  /* Try to open the file */
  if ((fd = fopen(script, "r")) == NULL)
  {
    for (x = 0; x < pClient->numScriptPath; x++)
    {
      snprintf(line, sizeof(line), "%s/%s", pClient->scriptPath[x], script);
      if ((fd = fopen(line, "r")) != NULL)
        break;
    }

    /* Test if file found in any of the script paths */
    if (fd == NULL)
    {
      if (!quiet)
        client_printf(pClient, "Error %d opening script file %s\n", errno, script);
      return -1;
    }
  }

  /* Create argument variables */
  pParams = NULL;
  for (x = 1; x < argc; ++x)
  {
    eval_result_t   value;
    char            name[16];

    snprintf(name, sizeof(name), "$%d", x);

    pVar = client_make_var(pClient, name, &pParams, 1);
    if (pVar == NULL)
      goto unwind_params;

    if (eval_solve(pClient->pEvalCtx, argv[x], -1, &value) == 0)
    {
      if (client_var_from_eval(pClient, quiet, pVar, &value) != 0)
        goto unwind_params;
    }
    else
    {
      /* Just use the string directly */
      pVar->u.s_val = strdup(argv[x]);
      if (pVar->u.s_val == NULL)
      {
        if (!quiet)
          client_printf(pClient, "Out of memory allocating string\n");
        goto unwind_params;
      }
      pVar->type = SCRIPT_VAR_STRING;
    }

    pVar->flags |= SCRIPT_VAR_FLAG_CONST;
  }

  io.io.vptbl = &client_file_io_vptbl;
  io.fp = fd;

  client_run_io_script(pClient, script, pParams, &io.io);

  /* Close the file */
  fclose(fd);

  return 0;

unwind_params:
  while (pParams != NULL)
  {
    script_var_t *pNext = pParams->pNext;
    client_del_var(pClient, pParams);
    pParams = pNext;
  }
  fclose(fd);

  return -1;
}

/*
================================================================================
Run the specified script
================================================================================
*/
void client_run_script(client_t *pClient, char *script, int argc, char *argv[])
{
  client_run_script_internal(pClient, 0, script, argc, argv);
}

/*
================================================================================
Name: client_var_lookup

Description:
  Callback function to perform variable lookup during expression evaluation.
================================================================================
*/
static int client_var_lookup(client_t *pClient, const char *str, script_var_t *pRes)
{
  script_var_t  *pVar;
  int           x;
  int           len;
  int           ret;

  /* Test if it is a register */
  if (f1000_regs_is_register(pClient->pRegs, str))
  {
    uint32_t reg_value;

    pRes->type = SCRIPT_VAR_INT;
    /* Read the register value */
    if (f1000_regs_readreg(pClient->pRegs, str, &reg_value) == 0)
      pRes->u.i_val = reg_value;
    else
      pRes->u.i_val = 0xDEADBEEF;

    /* Indicate variable found */
    return 1;
  }

  /* Test if this is a named variable */
  pVar = client_find_var(pClient, str);
  if (pVar)
  {
    *pRes = *pVar;
    /* Indicate variable found */
    return 1;
  }

  /* Test if this is a variable accessor */
  len = strlen(str);
  for (x = 0; x < g_var_accessor_count; x++)
  {
    const client_var_accessor_t *pVarAcc = &g_var_accessors[x];
    int alen = 1 + strlen(pVarAcc->name);
    if (len > alen && str[len - alen] == '.' &&
        strcmp(str + 1 + len - alen, pVarAcc->name) == 0)
    {
      char  name[128];

      strncpy(name, str, sizeof(name) - 1);
      name[sizeof(name) - 1] = 0;
      if (len - alen < sizeof(name) - 1)
          name[len - alen] = 0;

      ret = pVarAcc->func(pClient, name, pRes);
      if (ret)
        return ret;
    }
  }

  return 0;
}

static int client_eval_lookup(eval_ctx_t *pCtx, const eval_str_t *str,
    eval_result_t *pRes)
{
  client_t      *pClient = (client_t *)pCtx->userdata;
  char          name[128];
  script_var_t  value;

  if (str->len >= sizeof(name))
    return -1;

  memcpy(name, str->str, str->len);
  name[str->len] = 0;

  if (!client_var_lookup(pClient, name, &value))
    return -1;

  if (client_eval_from_var(pClient, pRes, &value) != 0)
    pRes->type = EVAL_VAL_NIL;

  return 0;
}

static int client_eval_index(eval_ctx_t *pCtx, const eval_str_t *str,
    const eval_result_t *idx, eval_result_t *pRes)
{
  client_t      *pClient = (client_t *)pCtx->userdata;
  char          name[128];
  script_var_t  value;
  script_mem_t  *pMem;

  if (idx->type != EVAL_VAL_INT)
  {
    client_printf(pClient, "Indexing with non-integral unsupported\n");
    return -1;
  }

  /* Memory access? Search for memory name in our memory database. */
  for (pMem = pClient->pMem; pMem != NULL; pMem = pMem->pNext)
  {
    /* Test if this entry matches */
    if (eval_str_eq(str, pMem->name))
    {
      uint32_t  addr;
      uint32_t  reg_val;
      uint32_t  offset;

      /* Match found.  Get the address */
      offset = idx->u.i_val;
      if (pMem->type == SCRIPT_MEM_16BIT)
        offset <<= 1;
      else if (pMem->type == SCRIPT_MEM_32BIT)
        offset <<= 2;
      addr = pMem->addr + offset;

      /* Read the memory location */
      if (client_readreg(pClient, addr, &reg_val) != 0)
      {
        client_printf(pClient, "Error reading memory address 0x%08X\n", addr);
        return 0;
      }

      if (pMem->type == SCRIPT_MEM_16BIT)
      {
        reg_val &= 0xFFFF;
        if (reg_val & 0x8000)
        {
          reg_val |= 0xFFFF0000;
        }
      }
      else if (pMem->type == SCRIPT_MEM_BYTE)
        reg_val &= 0xFF;

      pRes->type = EVAL_VAL_INT;
      pRes->u.i_val = reg_val;

      return 0;
    }
  }

  if (str->len >= sizeof(name))
    return -1;

  memcpy(name, str->str, str->len);
  name[str->len] = 0;

  /* Maybe it's a string index? */
  if (client_var_lookup(pClient, name, &value))
  {
    unsigned int len;

    if (value.type != SCRIPT_VAR_STRING)
      return -1;

    len = strlen(value.u.s_val);
    if (idx->u.i_val < 0 || idx->u.i_val >= len)
      return -1;

    pRes->type = EVAL_VAL_INT;
    pRes->u.i_val = value.u.s_val[idx->u.i_val];

    return 0;
  }

  return -1;
}


/*
================================================================================
Name: client_eval_script_call

Description:
  Callback function to perform function call during expression evaluation.
================================================================================
*/
static int client_eval_script_call(eval_ctx_t *pCtx, const char *fname,
    script_fn_t *pFn, const eval_result_t *pEvalParams, eval_result_t *pRes)
{
  client_t *pClient = (client_t *)pCtx->userdata;
  const eval_node_t *pParam = pEvalParams->u.array.head;
  script_fn_param_t *pFnParam;
  script_var_t *pParams;
  script_var_t *pVar;

  /* Create argument variables */
  pFnParam = pFn->pParameters;
  pParams = NULL;
  for (; pParam != NULL; pParam = pParam->next)
  {
    eval_result_t eval;

    if (pFnParam == NULL)
    {
      client_printf(pClient, "Too many arguments for function '%s'\n", fname);
      goto unwind_params;
    }

    if (eval_solve_node(pClient->pEvalCtx, pParam, &eval) != 0)
    {
      client_printf(pClient, "Unable to resolve parameter\n");
      goto unwind_params;
    }

    pVar = client_make_var(pClient, pFnParam->name, &pParams, 1);
    if (pVar == NULL)
      goto unwind_params;

    if (client_var_from_eval(pClient, 0, pVar, &eval) != 0)
      goto unwind_params;

    pFnParam = pFnParam->pNext;
  }

  pVar = client_make_var(pClient, "ans", NULL, 0);
  if (pVar == NULL)
    goto unwind_params;

  if (client_call_function_params(pClient, fname, pFn, pParams, pVar) != 0)
    goto unwind_params;

  if (client_eval_from_var(pClient, pRes, pVar) != 0)
    pRes->type = EVAL_VAL_NIL;

  return 0;

unwind_params:
  while (pParams != NULL)
  {
    script_var_t *pNext = pParams->pNext;
    client_del_var(pClient, pParams);
    pParams = pNext;
  }
  return -1;
}

/*
================================================================================
Name: client_eval_call

Description:
  Callback function to perform function call during expression evaluation.
================================================================================
*/
static int client_eval_call(eval_ctx_t *pCtx, const eval_str_t *func,
    const eval_result_t *pEvalParams, eval_result_t *pRes)
{
  client_t *pClient = (client_t *)pCtx->userdata;
  script_var_t *pVar;
  char name[256];
  int rc;

  if (eval_str_eq(func, "str2num"))
  {
    char *str;

    rc = eval_process_args(pCtx, pEvalParams, "z", &str);
    if (rc != 1)
        return -1;

    pRes->type = EVAL_VAL_INT;
    pRes->u.i_val = strtol(str, 0, 0);
    free(str);

    return 0;
  }
  else if (eval_str_eq(func, "hex2dec"))
  {
    char *str;

    rc = eval_process_args(pCtx, pEvalParams, "z", &str);
    if (rc != 1)
        return -1;

    pRes->type = EVAL_VAL_INT;
    pRes->u.i_val = strtol(str, 0, 16);
    free(str);

    return 0;
  }
  else if (eval_str_eq(func, "substr"))
  {
    eval_str_t str;
    int start;
    int end;

    rc = eval_process_args(pCtx, pEvalParams, "sii", &str, &start, &end);
    if (rc < 2)
        return -1;

    if (rc == 2)
      end = str.len;
    else if (end < 0)
      end = str.len + end;

    if (start < 0)
      start = str.len + start;

    if (end > str.len)
      end = str.len;

    if (start < 0 || end < 0 || start > str.len || end - start < 0)
        start = end = 0;

    pRes->type = EVAL_VAL_STR;
    pRes->u.str.str = str.str + start;
    pRes->u.str.len = end - start;

    return 0;
  }
  else if (eval_str_eq(func, "typename"))
  {
    eval_result_t res;
    const char *str;

    if (pEvalParams->u.array.len != 1)
      return -1;

    if (eval_solve_node(pClient->pEvalCtx, pEvalParams->u.array.head, &res) != 0)
      return -1;

    switch (res.type)
    {
    case EVAL_VAL_NIL:    str = "nil"; break;
    case EVAL_VAL_INT:    str = "int"; break;
    case EVAL_VAL_ARRAY:  str = "array"; break;
    case EVAL_VAL_STR:    str = "str"; break;
    case EVAL_VAL_FLOAT:  str = "float"; break;
    default:              str = "unk"; break;
    }

    pRes->type = EVAL_VAL_STR;
    pRes->u.str.str = str;
    pRes->u.str.len = strlen(str);

    return 0;
  }
  else if (eval_str_eq(func, "len"))
  {
    eval_result_t res;
    unsigned int len;

    if (pEvalParams->u.array.len != 1)
      return -1;

    if (eval_solve_node(pClient->pEvalCtx, pEvalParams->u.array.head, &res) != 0)
      return -1;

    switch (res.type)
    {
    case EVAL_VAL_NIL:    len = 0; break;
    case EVAL_VAL_INT:    len = 1; break;
    case EVAL_VAL_ARRAY:  len = res.u.array.len; break;
    case EVAL_VAL_STR:    len = res.u.str.len; break;
    case EVAL_VAL_FLOAT:  len = 1; break;
    default:              len = 0; break;
    }

    pRes->type = EVAL_VAL_INT;
    pRes->u.i_val = len;

    return 0;
  }

  if (func->len >= sizeof(name))
    return -1;

  memcpy(name, func->str, func->len);
  name[func->len] = 0;

  /* See if the function is already loaded */
  pVar = client_find_var(pClient, name);

  /* If we didn't find the variable, look in our search-path for a script */
  if (pVar == NULL)
  {
    char *args[] = { name };

    /* Add our file extension */
    snprintf(name + func->len, sizeof(name) - func->len, ".%s", "lm");

    /* Run the script */
    rc = client_run_script_internal(pClient, 1, name, 1, args);

    /* Restore our function name */
    name[func->len] = 0;

    if (rc == 0)
    {
      /* We ran the script successfully.  Hopefully our function showed up. */
      pVar = client_find_var(pClient, name);
    }
  }

  if (pVar != NULL && pVar->type == SCRIPT_VAR_FUNC)
  {
    return client_eval_script_call(pCtx, name, pVar->u.fn_val, pEvalParams, pRes);
  }
  else
  {
    client_printf(pClient, "No such function '%s'\n", name);
  }

  return -1;
}

/*
================================================================================
Name: client_var_acc_defined

Description:
  Determine if variable is defined
================================================================================
*/
static int client_var_acc_defined(client_t *pClient, const char *name, script_var_t *pRes)
{
  script_var_t value;

  pRes->type = SCRIPT_VAR_INT;
  pRes->u.i_val = client_var_lookup(pClient, name, &value);
  return 1;
}

/*
================================================================================
Name: client_var_acc_undefined

Description:
  Determine if variable is undefined
================================================================================
*/
static int client_var_acc_undefined(client_t *pClient, const char *name, script_var_t *pRes)
{
  script_var_t value;

  pRes->type = SCRIPT_VAR_INT;
  pRes->u.i_val = !client_var_lookup(pClient, name, &value);
  return 1;
}

/*
================================================================================
Name: client_ifproc_var_lookup

Description:
  Callback function for ifproc processor to determine if a variable exists
  and get it's value.
================================================================================
*/
static int client_ifproc_var_lookup(void *pCtx, const char *name, ifproc_value_t *pValue)
{
  int             ret;
  script_var_t    value;

  ret = client_var_lookup(pCtx, name, &value);
  if (ret)
  {
    if (value.type == SCRIPT_VAR_INT)
      *pValue = value.u.i_val;
    else if (value.type == SCRIPT_VAR_QWORD)
      *pValue = value.u.q_val;
    else
      ret = 0;
  }
  return ret;
}

/*
================================================================================
Name: client_ifproc_evaluate

Description:
  Callback function to evauate an expression during ifproc processing.
================================================================================
*/
static int client_ifproc_evaluate(void *pCtx, const char *expr, ifproc_value_t *pValue)
{
  int           ret;
  eval_result_t value;
  client_t*     pClient = (client_t *) pCtx;

  ret = eval_solve(pClient->pEvalCtx, expr, -1, &value);
  if (ret != 0)
    return 0;

  switch (value.type)
  {
  case EVAL_VAL_INT:
    *pValue = value.u.i_val;
    return 1;

  case EVAL_VAL_FLOAT:
    *pValue = value.u.f_val != 0;
    return 1;

  case EVAL_VAL_STR:
    *pValue = value.u.str.len != 0;
    return 1;

  case EVAL_VAL_ARRAY:
    *pValue = value.u.array.len != 0;
    return 1;

  case EVAL_VAL_NIL:
    *pValue = 0;
    return 1;
  }

  return 0;
}

/*
================================================================================
Run command ... run a script
================================================================================
*/
static void client_run(client_t *pClient, int argc, char *argv[])
{
  char  *fname;
  int   argn = 1;
  int   x;

  /* Test if we should execute based on if processing */
  if (!ifproc_should_process(&pClient->ifProc))
    return;

  /* run the specified file */
  if (argc < 2)
  {
    client_printf(pClient, "usage: run script [args...]\n");
    return;
  }

  fname = argv[1];

  /* If we find a space, we split off the first parameter */
  x = strcspn(fname, " \t");
  if (fname[x] != 0)
  {
    argv[1] = fname + x + strspn(fname + x, " \t");
    fname[x] = 0;
    argn = 0;
  }

  client_run_script_internal(pClient, 0, fname, argc - argn, argv + argn);
}

/*
================================================================================
Initialize a client struct with callback functions
================================================================================
*/
void client_init_callbacks(client_t *pClient)
{
  static const struct eval_vptbl client_eval_callbacks =
  {
    client_eval_call,
    client_eval_lookup,
    client_eval_index,
  };
  /* Initialize the IF processor */
  ifproc_init(&pClient->ifProc, &client_ifproc_var_lookup,
              &client_ifproc_evaluate, pClient);

  /* Initialize the Evaluation Def */
  pClient->evalCtx.vptbl    = &client_eval_callbacks;
  pClient->evalCtx.userdata = pClient;
  pClient->pEvalCtx         = &pClient->evalCtx;
}

/*
================================================================================
De-initialize a client struct
================================================================================
*/
void client_deinit(client_t *pClient)
{
  /* De-initialize the F1000 regs context */
  if (pClient->pRegs != NULL)
    f1000_regs_deinit(pClient);

  /* Clear the readline history */
  clear_history();

  while (pClient->pVars)
  {
    script_var_t *pNext = pClient->pVars->pNext;
    client_del_var(pClient, pClient->pVars);
    pClient->pVars = pNext;
  }

  while (pClient->pMem)
  {
    script_mem_t  *pNext = pClient->pMem->pNext;
    free(pClient->pMem);
    pClient->pMem = pNext;
  }
}

/*
================================================================================
Returns the number of commands in our command table
================================================================================
*/
int client_get_cmd_count(void)
{
  return g_cmd_count;
}

/*
================================================================================
De-initialize a client struct
================================================================================
*/
const char * client_get_cmd_text(int cmd_no)
{
  if (cmd_no < g_cmd_count)
    return g_cmds[cmd_no].name;

  return NULL;
}

/*
================================================================================
Lookup client variable and format a string using the given format
================================================================================
*/
int client_get_expr_string(client_t *pClient, const char *pExpr,
      const char *fmt, char *valStr, int maxLen)
{
  char *args[] = { "print", (char *)fmt, (char *)pExpr };
  int len;

  len = client_format_text(pClient, 1, valStr, maxLen, 3, args);

  return len > 0;
}

// vim: et sw=2 ts=2 cindent
