/*
 * $Id: assemble.cpp,v 1.15 2013/02/08 00:07:52 kpettit1 Exp $
 *
 * Copyright 2010 Ken Pettit
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.

 
 VTAssembler:		This class defines an 8085 Macro Assembler for the
					VirtualT project.

========================================================================
*/

#ifndef PREPROC_H
#define PREPROC_H

#define  PREPROC_NESTED_IF_DEPTH       32

#define  IF_STAT_PROCESS               1
#define  IF_STAT_DONT_PROCESS          2
#define  IF_STAT_NESTED_DONT_PROCESS   3
#define  IF_STAT_EVAL_ERROR            4

#define  PREPROC_ERR_NONE              0
#define  PREPROC_ERR_NEST_TOO_DEEP     1
#define  PREPROC_ERR_EVAL_ERROR        2

#define  INST_IF                       1
#define  INST_ELIF                     2

typedef  double ifproc_value_t;

typedef int (*ifproc_get_value_func_t)(void *pCtx, const char *name, ifproc_value_t *pValue);

typedef struct
{
  int                       if_depth;
  char                      if_stat[PREPROC_NESTED_IF_DEPTH];
  int                       lineno;
  int                       last_if_else_line;
  int                       last_if_else_is_if;
  char                      sFilename[256];
  char                      sError[256];

  ifproc_get_value_func_t   pFindSymbolFunc;
  ifproc_get_value_func_t   pEvalExprFunc;
  void*                     pOpaque;

} ifproc_ctx_t;

int  ifproc_init(ifproc_ctx_t *pCtx, ifproc_get_value_func_t pFindSymbolFunc,
                 ifproc_get_value_func_t pEvalExprFunc, void *pOpaque);
int  ifproc_ifdef(ifproc_ctx_t *pCtx, const char* name, int lineno);
int  ifproc_ifndef(ifproc_ctx_t *pCtx, const char* name, int lineno);
int  ifproc_if(ifproc_ctx_t *pCtx, const char *expr, int lineno);
int  ifproc_elif(ifproc_ctx_t *pCtx, const char *expr, int lineno);
int  ifproc_else(ifproc_ctx_t* pCtx, int lineno);
int  ifproc_endif(ifproc_ctx_t* pCtx, int lineno);
void ifproc_set_filename(ifproc_ctx_t *pCtx, const char *filename);
int  ifproc_should_process(ifproc_ctx_t *pCtx);

#endif   // PREPROC_H

