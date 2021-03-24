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
// File        : f1000_regs.h                                                       *
// Author      : KDP                                                            *
// Created     : 11/08/2018                                                     *
//                                                                              *
// Description:  F1000 register access routines.
//                                                                              *
//*******************************************************************************

#ifndef _APPS_F1000_CLIENT_SRC_REGS_H_
#define _APPS_F1000_CLIENT_SRC_REGS_H_

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdint.h>

#ifndef STANDALONE
#include <graphics/curses.h>
#include <system/termcurses.h>
#include <../tui/Tui.h>
//#include "RegsSrc.h"
#endif

/****************************************************************************
 * Definitions
 ****************************************************************************/

#define   REGS_HISTORY_MAX 16

#ifndef FAR
#define FAR
#endif

#ifndef OK
#define OK 0
#endif

/****************************************************************************
 * Private Types
 ****************************************************************************/

typedef struct client_s client_t;

typedef int (*f1000_regs_printf_t)(void *pCtx, const char *fmt, ...);

typedef struct f1000_sort_list_s
{
  char                      *name;
  struct f1000_sort_list_s  *pNext;
} f1000_sort_list_t;

typedef struct f1000_field_def_s
{
  f1000_sort_list_t         listitem;
  char                      *desc;
  uint32_t                  low_bit;
  uint32_t                  high_bit;
} f1000_field_def_t;

typedef struct f1000_reg_def_s
{
  f1000_sort_list_t         listitem;
  uint32_t                  offset;
  uint32_t                  num_bits;
  char                      *rw;
  f1000_field_def_t         *pFirstFieldDef;
} f1000_reg_def_t;

typedef struct f1000_reg_block_s
{
  f1000_sort_list_t         listitem;
  char                      name[20];
  uint32_t                  baseaddr;
  char                      filename[32];
} f1000_reg_block_t;

typedef struct f1000_regs_ctx_s
{
  f1000_reg_block_t         *pFirstRegBlock;    /* Pointer to first register block */
  f1000_reg_def_t           *pFirstRegDef;
  f1000_reg_def_t           *pSelectedReg;
  char                      *buffer;
  int                       buflen;
  int                       argc;
  char *                    argv[20];
  int                       level_len[4];
  char                      reg_block[64];
  f1000_regs_printf_t       cmd_printf;
  f1000_regs_printf_t       src_printf;
  void *                    pPrintfCtx;
  int												printfColors;

  client_t                  *pClient;
#if 0
  char                      *history[REGS_HISTORY_MAX];
  int                       history_new;
  int                       history_back;
  int                       level_len[4];
  char                      line[200];
  int                       linelen;
  int                       lineindex;
  int                       level;
  int                       ep;
  int                       f1000_id;
  int                       quit;
#endif
} f1000_regs_ctx_t;

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * f1000_regs_init
 *
 * Initialize the f1000_regs context
 ****************************************************************************/

int f1000_regs_init(client_t *pClient, f1000_regs_printf_t, void *pPrintfCtx,
                    int printfColors);

/****************************************************************************
 * f1000_regs_deinit
 *
 * De-Initialize the f1000_regs context
 ****************************************************************************/

int f1000_regs_deinit(client_t *pClient);

/****************************************************************************
 * f1000_regs_cmd_print
 *
 * Print an f1000 register by name
 ****************************************************************************/

int f1000_regs_cmd_print(f1000_regs_ctx_t *pCtx, FILE *fd,
                         int print_as_int16);

/****************************************************************************
 * f1000_regs_cmd_set
 *
 * Set an f1000 register by name
 ****************************************************************************/

void f1000_regs_cmd_set(f1000_regs_ctx_t *pCtx);

/****************************************************************************
 * f1000_regs_is_register
 *
 * Return TRUE if given name is a known register
 ****************************************************************************/

int f1000_regs_is_register(f1000_regs_ctx_t *pCtx, const char *name);

/****************************************************************************
 * f1000_regs_readreg
 *
 * Return value of register or field
 ****************************************************************************/

int f1000_regs_readreg(f1000_regs_ctx_t *pCtx, const char *name, uint32_t *value);

/****************************************************************************
 * f1000_regs_iterate_begin
 *
 * Returns an iterator to the first RMM register matching the name.
 ****************************************************************************/

void * f1000_regs_iterate_begin(client_t *pClient, const char *name);

/****************************************************************************
 * f1000_regs_iterate_next
 *
 * Iterate to next item in the list after the given iterator.  Returns a 
 * void * pointer to the next iterator or NULL if at the end.
 ****************************************************************************/

void * f1000_regs_iterate_next(void *it);

/****************************************************************************
 * f1000_regs_iterate_name
 *
 * Returns the name associated with the given iterator.
 ****************************************************************************/

const char * f1000_regs_iterate_name(void *it);

#endif /* _APPS_F1000_CLIENT_SRC_REGS_H_ */

// vim: ts=2 sw=2
