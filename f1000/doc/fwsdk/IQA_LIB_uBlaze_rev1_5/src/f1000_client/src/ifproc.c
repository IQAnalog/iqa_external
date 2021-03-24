/*
 * $Id: ifproc.c,v 1.15 2013/02/08 00:07:52 kpettit1 Exp $
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

 
 ifproc:		This C context defines a ifprocessor framework for 
					keeping track of nested #ifdef / #if / #elif / #endif
               expressions

========================================================================
*/

#include		"ifproc.h"
#include		<math.h>
#include		<string.h>
#include		<stdio.h>
#include		<stdlib.h>
#ifndef WIN32
#include 		<unistd.h>
#endif

/*
============================================================================
Routine to search the parent application for the given symbol

Return:  TRUE if symbol found, FALSE otherwise
============================================================================
*/
static int ifproc_find_symbol(ifproc_ctx_t *pCtx, const char *name, 
            ifproc_value_t *pValue)
{
  // Call the callback function in our context to search for the symbol
  return pCtx->pFindSymbolFunc(pCtx->pOpaque, name, pValue);
}

static int ifproc_internal_ifdef(ifproc_ctx_t *pCtx, const char* name, int negate, int lineno)
{
  int             defined;
  ifproc_value_t  value;

  // Save line number
  pCtx->last_if_else_line = lineno;
  pCtx->last_if_else_is_if = 1;

  // First push results from previous IF/ELSE/ENDIF operation so
  // we generate "nested if" conditions.  Initialize condition to
  // EVAL_ERROR in case the condition does not evaluate
  if ((pCtx->if_stat[pCtx->if_depth] == IF_STAT_DONT_PROCESS) ||
    (pCtx->if_stat[pCtx->if_depth] == IF_STAT_NESTED_DONT_PROCESS) ||
    (pCtx->if_stat[pCtx->if_depth] == IF_STAT_EVAL_ERROR))
  {
    pCtx->if_stat[++pCtx->if_depth] = IF_STAT_NESTED_DONT_PROCESS;
  }
  else
    pCtx->if_stat[++pCtx->if_depth] = IF_STAT_EVAL_ERROR;

  // Ensure our #ifdef stack depth hasn't overflowed
  if (pCtx->if_depth >= sizeof(pCtx->if_stat))
  {
    pCtx->if_depth--;
    sprintf(pCtx->sError, "Error in line %d(%s):  Too many nested ifs", 
            lineno, (char *) pCtx->sFilename);
    return 1;
  }
  else if (pCtx->if_stat[pCtx->if_depth] == IF_STAT_EVAL_ERROR)
  {
    pCtx->if_stat[pCtx->if_depth] = IF_STAT_DONT_PROCESS;
    defined = ifproc_find_symbol(pCtx, name, &value);
    if ((defined && !negate) || (!defined && negate))
      pCtx->if_stat[pCtx->if_depth] = IF_STAT_PROCESS;
  }

  return 0;
}

/*
============================================================================
This function is called when an if directive is encountered
============================================================================
*/
static int ifproc_internal_if(ifproc_ctx_t *pCtx, const char *expr, int inst, int lineno)
{
  ifproc_value_t		value;

  pCtx->last_if_else_line = lineno;
  pCtx->last_if_else_is_if = 1;

  // First push results from previous IF/ELSE/ENDIF operation so
  // we generate "nested if" conditions.  Initialize condition to
  // EVAL_ERROR in case the condition does not evaluate
  if (inst == INST_IF)
  {
     // Process if instrution
     if ((pCtx->if_stat[pCtx->if_depth] == IF_STAT_DONT_PROCESS) ||
        (pCtx->if_stat[pCtx->if_depth] == IF_STAT_NESTED_DONT_PROCESS) ||
        (pCtx->if_stat[pCtx->if_depth] == IF_STAT_EVAL_ERROR))
     {
       pCtx->if_stat[++pCtx->if_depth] = IF_STAT_NESTED_DONT_PROCESS;
     }
     else
       pCtx->if_stat[++pCtx->if_depth] = IF_STAT_EVAL_ERROR;
     
     if (pCtx->if_depth >= sizeof(pCtx->if_stat))
     {
       pCtx->if_depth--;
       sprintf(pCtx->sError, "Error in line %d(%s):  Too many nested ifs",
          lineno, (char *) pCtx->sFilename);
       return 1;
     }
  }
  else
  {
    // Process elif instruction
    if (pCtx->if_depth == 0)
    {
      sprintf(pCtx->sError, "Error in line %d(%s):  ELSE without a matching IF",
         lineno, (char *) pCtx->sFilename);
      return 1;
    }

    if (pCtx->if_stat[pCtx->if_depth] == IF_STAT_PROCESS)
      pCtx->if_stat[pCtx->if_depth] = IF_STAT_NESTED_DONT_PROCESS;
    else
      if (pCtx->if_stat[pCtx->if_depth] == IF_STAT_DONT_PROCESS)
        pCtx->if_stat[pCtx->if_depth] = IF_STAT_EVAL_ERROR;
  }

  // Determine if both equations can be evaluated
  if (pCtx->if_stat[pCtx->if_depth] == IF_STAT_EVAL_ERROR)
  {
    if (pCtx->pEvalExprFunc(pCtx->pOpaque, expr, &value))
    {
      // Initialize to DONT_PROCESS
      pCtx->if_stat[pCtx->if_depth] = IF_STAT_DONT_PROCESS;
       
      if (value != 0)
        pCtx->if_stat[pCtx->if_depth] = IF_STAT_PROCESS;
    }
  }		

  if (pCtx->if_stat[pCtx->if_depth] == IF_STAT_EVAL_ERROR)
  {
    sprintf(pCtx->sError, "Error in line %d(%s):  Expression %s cannot be evaluated",
          lineno, pCtx->sFilename, expr);
    return 1;
  }

  return 0;
}

/*
============================================================================
This is the ifprocessor function to handle conditional IFDEF statements

Return:  Zero if no error, non-zero error code
============================================================================
*/
int ifproc_ifdef(ifproc_ctx_t *pCtx, const char* name, int lineno)
{
  return ifproc_internal_ifdef(pCtx, name, 0, lineno);
}

/*
============================================================================
This is the ifprocessor function to handle conditional IFNDEF statements
============================================================================
*/
int ifproc_ifndef(ifproc_ctx_t *pCtx, const char* name, int lineno)
{
  return ifproc_internal_ifdef(pCtx, name, 1, lineno);
}

/*
============================================================================
This is the ifprocessor function to handle conditional IF statements
============================================================================
*/
int ifproc_if(ifproc_ctx_t *pCtx, const char *expr, int lineno)
{
  return ifproc_internal_if(pCtx, expr, INST_IF, lineno);
}

/*
============================================================================
This is the ifprocessor function to handle conditional ELIF statements
============================================================================
*/
int ifproc_elif(ifproc_ctx_t *pCtx, const char *expr, int lineno)
{
  return ifproc_internal_if(pCtx, expr, INST_ELIF, lineno);
}

/*
============================================================================
This is the ifprocessor function to handle conditional ELSE statements

Return:  Zero if no error, non-zero error code
============================================================================
*/
int ifproc_else(ifproc_ctx_t* pCtx, int lineno)
{
  pCtx->last_if_else_line = lineno;
  pCtx->last_if_else_is_if = 0;

  // Process ELSE statement during parsing.  First insure else has a matching if
  if (pCtx->if_depth == 0)
  {
    sprintf(pCtx->sError, "Error in line %d(%s):  ELSE without a matching IF",
          lineno, (char *) pCtx->sFilename);
    return 1;
  }

  // Now check if the active IF statement is not a NESTED_DONT_PROCESS.
  // If it isn't then change the state of the  assembly
  if (pCtx->if_stat[pCtx->if_depth] == IF_STAT_PROCESS)
    pCtx->if_stat[pCtx->if_depth] = IF_STAT_DONT_PROCESS;
  else
    if (pCtx->if_stat[pCtx->if_depth] == IF_STAT_DONT_PROCESS)
       pCtx->if_stat[pCtx->if_depth] = IF_STAT_PROCESS;

  return 0;
}

/*
============================================================================
This function is called when an endif directive is encountered
============================================================================
*/
int ifproc_endif(ifproc_ctx_t* pCtx, int lineno)
{
   // Process ENDIF statement during parsing.  First insure else has a matching if
   if (pCtx->if_depth == 0)
   {
      sprintf(pCtx->sError, "Error in line %d(%s):  ENDIF without a matching IF", 
            lineno, (char *) pCtx->sFilename);
      return 1;
   }

   // Pop If from stack
   pCtx->if_depth--;
   return 0;
}

/*
============================================================================
This function should be called first to initialize the context
============================================================================
*/
int ifproc_init(ifproc_ctx_t *pCtx, ifproc_get_value_func_t pFindSymbolFunc,
                  ifproc_get_value_func_t pEvalExprFunc, void *pOpaque)
{
   pCtx->if_depth = 0;
   pCtx->if_stat[0] = IF_STAT_PROCESS;
   pCtx->last_if_else_is_if = 0;
   pCtx->sFilename[0] = 0;

   pCtx->pOpaque = pOpaque;
   pCtx->pFindSymbolFunc = pFindSymbolFunc;
   pCtx->pEvalExprFunc = pEvalExprFunc;

   return 0;
}

/*
============================================================================
This function should be called any time the filename being parsed changes.
============================================================================
*/
void ifproc_set_filename(ifproc_ctx_t *pCtx, const char *filename)
{
   strncpy(pCtx->sFilename, filename, sizeof(pCtx->sFilename));
}

/*
============================================================================
This function returns TRUE (1) if the current PROCESS mode indicates
source lines should be processed based on nested #if/#ifdef/#else/#endif
status.
============================================================================
*/
int ifproc_should_process(ifproc_ctx_t *pCtx)
{
   return pCtx->if_stat[pCtx->if_depth] == IF_STAT_PROCESS;
}

