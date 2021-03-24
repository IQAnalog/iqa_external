//*******************************************************************************
//      __  __                                                                  *
//     /  \/  \                                                                 *
//         \   \                                                                *
//    I Q - A N A L O G                                                         *
//           \   \            IQ-Analog Corp                                    *
//            \__/\__/       www.iqanalog.com                                   *
//                                                                              *
// -----------------------------------------------------------------------------*
//
// Copyright (C) IQ-Analog Corp. All rights reserved.
//
// ------------------------------------------------------------------------------
//  CONFIDENTIAL
// ------------------------------------------------------------------------------
//
// File        : client.h
// Author      : KP
// Created     : 02/12/2019
//
// Description:  Include file for f1000 client
//
//*******************************************************************************

#ifndef __APPS_F1000_CLIENT_CLIENT_H
#define __APPS_F1000_CLIENT_CLIENT_H

#include "f1000_regs.h"
#include "eval_expr.h"
#include "ifproc.h"

/*
================================================================================
Structure definitions
================================================================================
*/

#define SCRIPT_BLOCK_LOOP       0
#define SCRIPT_BLOCK_IF         1
#define SCRIPT_BLOCK_SWITCH     2
#define SCRIPT_BLOCK_FUNC       3

#define SCRIPT_VAR_INT          0
#define SCRIPT_VAR_FLOAT        1
#define SCRIPT_VAR_STRING       2
#define SCRIPT_VAR_NAN          3
#define SCRIPT_VAR_QWORD        4
#define SCRIPT_VAR_FUNC         5

#define SCRIPT_VAR_FLAG_CONST   (1 << 0)
#define SCRIPT_VAR_FLAG_LOCAL   (1 << 1)

#define SCRIPT_LOOP_FOR         0
#define SCRIPT_LOOP_WHILE       1

#define SCRIPT_MEM_BYTE         0
#define SCRIPT_MEM_16BIT        1
#define SCRIPT_MEM_32BIT        2

//Macros to Set individual Modules in init_chip command
#define SET(x,y)                x |= (1 << y)

//Bits 0-3 for RF Synthesizer
#define INIT_RFSYNTH1(x)        SET(x,0)
#define INIT_RFSYNTH2(x)        SET(x,1)

//Bits 4-7 RF Synthesizer Port
#define SELECT_RFSYNTH_PORTA(x) SET(x,4)
#define SELECT_RFSYNTH_PORTB(x) SET(x,5)

//Bits 8-15 for DAC
#define INIT_DAC1(x)            SET(x,8)
#define INIT_DAC2(x)            SET(x,9)
//Bits 16-23 for ADC
#define INIT_ADC1(x)            SET(x,16)
#define INIT_ADC2(x)            SET(x,17)

typedef struct script_block_s
{
  int                 type;
  int                 filePos;
} script_block_t;

typedef struct script_fn_param_s
{
  char                      name[64];
  struct script_fn_param_s  *pNext;
} script_fn_param_t;

typedef struct script_fn_s
{
  script_block_t      block;
  char                rname[64];
  int                 lineno;
  script_fn_param_t * pParameters;
  char *              pData;
  int                 datalen;
} script_fn_t;

typedef struct script_var_s
{
  char                name[64];
  int                 type;
  uint32_t            flags;
  union
  {
    uint64_t          q_val;
    uint32_t          i_val;
    float             f_val;
    char *            s_val;
    script_fn_t *     fn_val;
  } u;
  struct script_var_s *pNext;
} script_var_t;

typedef struct script_mem_s
{
  char                name[64];
  int                 type;
  uint32_t            addr;
  struct script_mem_s *pNext;
} script_mem_t;

typedef struct script_loop_s
{
  script_block_t      block;
  script_var_t        *loopVar;
  int                 type;
  char                *loopControl;
  int                 loopIndex;
  int                 forStop;
  int                 forStep;
  int                 lineno;
} script_loop_t;

typedef struct script_if_s
{
  script_block_t      block;
} script_if_t;

typedef struct client_io_s client_io_t;
typedef struct client_io_vptbl_s
{
  void  (* seek)(client_io_t *io, int pos);
  int   (* tell)(client_io_t *io);
  char *(* gets)(client_io_t *io, char *line, int size);
  int   (* read)(client_io_t *io, char *buf, int size);
  int   (* write)(client_io_t *io, const char *buf, int size);
} client_io_vptbl_t;

struct client_io_s
{
  const client_io_vptbl_t *vptbl;
};

typedef struct script_def_s
{
  /* Script file pointer, linono, etc. */
  client_io_t       *io;
  int               lineno;
  char              script_file[512];

  /* Block stack for nexted if, for, switch, etc. */
  int               blockIndex;
  script_block_t    *blockStack[20];

  /* Evaluation context */
  eval_ctx_t        evalCtx;

  /* Script if-then-else exectution control */
} script_def_t;


typedef int (*client_printf_t)(void *pCtx, const char *fmt, ...);
typedef int (*client_auxcmdproc_t)(void *pCtx, int argc, char *argv[]);
typedef int (*client_auxvarget_t)(void *pCtx, const char *name, script_var_t *pVar);
typedef int (*client_inputget_t)(void *pCtx, char *pBuf, int maxLen);

typedef struct client_s
{
  /* EVM Server connection control */
  char              *server_ip;
  int               port;
  int               sockfd;
  char              rxdbuff[1024 * 256];
  char              txdbuff[1024 * 256];

  /* Program execution control */
  int               debug;
  int               quiet;
  int               quit;
  int               is_script;
  int               terminate_script;

  /* Output file descriptor */
  FILE*             fd;

  /* F1000 and EP selection */
  uint32_t          f1000_id;
  uint32_t          ep_id;

  /* Block stack for nexted if, for, switch, etc. */
  int               blockIndex;
  script_block_t    *blockStack[20];

  /* Loop stack for nested loops */
//  int               loopIndex;
//  script_loop_t     *loopStack[20];

  /* F1000 register lookup and evaluation control */
  f1000_regs_ctx_t  *pRegs;
  script_var_t      *pVars;
  script_var_t      *pLocals;
  script_mem_t      *pMem;
  eval_ctx_t        *pEvalCtx;
  eval_ctx_t        evalCtx;

  /* Script execution control */
  script_def_t      *scriptStack[20];
  int               scriptIndex;
  script_def_t      *pActiveScript;
  char              *scriptPath[10];
  int               numScriptPath;

  /* EP bit stack */
  uint32_t          epStack;

  /* If processing context */
  ifproc_ctx_t      ifProc;

  /* Print function */
  client_printf_t   printf;
  int               printfColors;
  void*             pPrintfCtx;

  /* Aux Command Processor (external to client.c */
  client_auxcmdproc_t   pAuxCmdProc;
  void*                 pAuxCtx;
  client_auxvarget_t    pAuxVarGet;
  script_var_t          auxScriptVar;
  client_inputget_t     pInputGet;
  void*                 pInputCtx;
} client_t;

extern const int gMaxPktSize;

/*
================================================================================
Function prototypes
================================================================================
*/

int         client_read(client_t *pClient, int rx_expected);
void        client_dac_mem(client_t *pClient, int argc, char *argv[]);
void        client_dac_play(client_t *pClient, int argc, char *argv[]);
void        client_init_callbacks(client_t *pClient);
void        client_deinit(client_t *pClient);
void        client_run_script(client_t *pClient, char *script, int argc, char *argv[]);
int         client_is_script_running(client_t *pClient);
void        client_terminate_script(client_t *pClient);
int         client_process_line(client_t *pClient, char *line, int script);
int         client_open_connection(client_t *pClient);
int         client_readreg(client_t *pClient, uint32_t reg_addr, uint32_t *val);
int         client_writereg(client_t *pClient, uint32_t reg_addr, uint32_t val);
int         client_get_cmd_count(void);
void        client_help(client_t *pClient, int argc, char *argv[]);
const char *client_get_cmd_text(int cmd_no);
script_var_t * client_find_var(client_t *pClient, const char *name);
void        client_subst_vars(client_t *pClient, const char *line,
                              char *subst_line, int len);
int         client_get_expr_string(client_t *pClient, const char *pExpr,
                                   const char *fmt, char *valStr, int maxLen);

/*
================================================================================
client print function wrapper
================================================================================
*/
#define client_printf(client_, ...) \
  (client_)->printf((client_)->pPrintfCtx, __VA_ARGS__)


#endif /*  __APPS_F1000_CLIENT_CLIENT_H */

// vim:  sw=2 ts=2 et
