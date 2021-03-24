/****************************************************************************
 * eval.h
 *
 *   Copyright (C) 2013-2014 Ken Pettit. All rights reserved.
 *   Author: Ken Pettit <pettitkd@gmail.com>
 *
 *   This is a private header file defining internal structures and
 *   contants used by uCBASIC.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/*
=========================================
Compiler specific library routine defines
=========================================
*/

typedef double                  FloatType_t;

/*
=======================================================================================
Defines for infix expression evaluation
=======================================================================================
*/
#define N               256
#define STACK_SIZE      16
#define LP              10
#define RP              20
#define LB              '['
#define RB              ']'
#define OPERATOR        30
#define OPERAND         40
#define NEGATE          '~'
#define COND_EQ         0x80
#define COND_LT         0x81
#define COND_LTE        0x82
#define COND_GT         0x83
#define COND_GTE        0x84
#define COND_NE         0x85
#define COND_LG         0x86
#define LEFT_SHIFT      0x87
#define RIGHT_SHIFT     0x88
#define LOGICAL_OR      0x89
#define LOGICAL_AND     0x8A
#define FUN_HI          0x8B
#define FUN_LO          0x8C

/* Left parentheses precedence. Minimum of all */

#define LPP             0       /* Left Parenthesis precedence */
#define LBP             1       /* Left Bracket precedence */
#define AP              8       /* Addition precedence */
#define SUBP            AP      /* Subtraction precedence */
#define MP              9       /* Muliplication precedence */
#define DP              MP      /* Division precedence */
#define REMP            10      /* Remainder precedence */
#define EXPP            12      /* Remainder precedence */
#define SHIFT           7       /* Shift precedence */
#define GT              6       /* GT, GTE, LT, LTE precedence */
#define EQ              5       /* EQ, NE precedence */
#define LAND            4       /* Logical AND */
#define LOR             3       /* Logical OR precedence */
#define NOT             11      /* NOT precedence */
#define FUN             13      /* Function precedence */
#define NONE            15

#define STACK_DEBUG_PRINT(x)    printf(x)

#ifndef TRUE
#define TRUE    1
#endif

#ifndef FALSE
#define FALSE   0
#endif

#ifndef FAR
#define FAR
#endif

/*
=================================================
Define what a variable looks like
=================================================
*/
typedef struct
{
    int         type;
    int         equate_or_label;
    int         index;              // Index into equate or label table
} VarDef_t;

#define VAR_TYPE_STRING     0
#define VAR_TYPE_INT        1
#define VAR_TYPE_FLOAT      2

typedef int (*val_lookup_t)(void *pCtx, const char *ptr, 
                            char replace, int64_t *value);
typedef int (*fun_lookup_t)(void *pCtx, const char *ptr);

/*
=================================================
Definitions for evaluation
=================================================
*/
typedef struct StackType
{
  unsigned char     type;
  union 
  {
    FloatType_t       f;
    const FAR char *  s;
  } u;
} StackType_t;

#define STACK_TYPE_FLOAT  0
#define STACK_TYPE_STRING 1
#define STACK_TYPE_CONST  2

typedef struct EvalDef
{
  char              postfix[N+1];
  union
  {
    unsigned char   ch[N];
    StackType_t     s[STACK_SIZE];
  } stack;
  int               top;
  int               bytesEvaled;
  char              funcEvalMode;
  char              floatStr[128];

  val_lookup_t      pValLookup;
  fun_lookup_t      pFunLookup;
  void*             pCtx;
} EvalDef_t;


int expr_Evaluate(EvalDef_t* pEv, const char* pExpr, unsigned char report_error, FloatType_t *result,
                    void* pContext);

