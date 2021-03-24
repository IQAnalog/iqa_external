/****************************************************************************
 * eval.c
 *
 *   Copyright (C) 2013-2014 Ken Pettit. All rights reserved.
 *   Author: Ken Pettit <pettitkd@gmail.com>
 *
 *   This file contains routines for setting and evaluation variables.
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

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <math.h>
#include "eval.h"

/*
================================================================================
Evaluaton and evaluation helper prototypes
================================================================================
*/
int64_t     hextoi( char* hex);
static int  GetType(char);                   /* Get type based on character */
static void Push(EvalDef_t *pEv, char);      /* Push to stack */
static char Pop(EvalDef_t *pEv);             /* Pop from stack */
static int  GetPrec(unsigned char);          /* Get the precedence of operator */

/*
================================================================================
expr_FindVariable:   Searches the variable space for the specified var.  This 
                        will first search the local variable space of any active 
                        subroutine (if subroutines are compiled in), and if not 
                        found, will search global space next.
================================================================================
*/
int expr_FindVariable(EvalDef_t* pEv, const char *name, void *pContext, 
                        FloatType_t *value)
{
  int         found;
  int64_t     ivalue;

  /* Search for name in equates table and variable database */

  found = pEv->pValLookup(pEv->pCtx, name, 0, &ivalue);
  if (found)
  {
      *value = (FloatType_t) ivalue;
      return 1;
  }

  /* Search for the name in the function database */

  ivalue = pEv->pFunLookup(pEv->pCtx, name);
  if (ivalue != -1)
  {
     *value = (FloatType_t) ivalue;
     return 1;
  }

  return 0;
}

/*
================================================================================
InfixToPostfix: Convert an infix expression to a postfix expression so it can
                be evaluated.  This take into account order of precedence and
                embedded parenthesis.
================================================================================
*/
static int InfixToPostfix(EvalDef_t *pEv, const char *pInfix)
{
  unsigned char   i, p, l, type, prec;
  unsigned char   last = 0;
  unsigned char   next, lastch, op;
  unsigned char   inQuote = 0, parenLevel, isSub = 0;

  i = p = 0;
  pEv->top = -1;
  pEv->bytesEvaled = 0;
  l = strlen(pInfix);
  parenLevel = 0;

  /* Loop for all characters in the infix string */

  while(i < l)
    {
      /* Test for terminating '#' operator */

      if (pInfix[i] == '#' && (parenLevel == 0))
        break;

      if (inQuote)
        {
          /* Add the characters un-modified until end quote found */

          if (pInfix[i] == '\\')
            {
              /* Handle escape characters */

              i++;
              if (pInfix[i] == 't')
                pEv->postfix[p++] = '\t';
              else if (pInfix[i] == 'n')
                pEv->postfix[p++] = '\n';
              else if (pInfix[i] == 'r')
                pEv->postfix[p++] = '\r';
              else if (pInfix[i] == '"')
                pEv->postfix[p++] = '"';
            }
          else if (pInfix[i] == '"')
            {
              /* Process end of quote */

              pEv->postfix[p++] = '"';
              inQuote = FALSE;
            }
          else
            pEv->postfix[p++] = pInfix[i];

          i++;
          continue;
        }

      /* Keep track of parenthesis level */

      if (pInfix[i] == '(')
        parenLevel++;
      else if (pInfix[i] == ')')
        {
          parenLevel--;
          if (parenLevel == 0 && isSub)
            {
              i++;
              break;
            }
        }

      /* Get type of this character */

      if ((pInfix[i] == '<' && pInfix[i+1] == '<') ||
          (pInfix[i] == '>' && pInfix[i+1] == '>'))
        type = OPERATOR;
      else
        type = GetType(pInfix[i]);

      switch(type)
        {
          case LP:
            Push(pEv, pInfix[i]);
            break;
          case RP:
            while(pEv->top > -1 && (next = Pop(pEv)) != '(')
              pEv->postfix[p++] = next;
            break;
          case LB:
            Push(pEv, ' ');
            while(isalnum(pEv->postfix[p-1]) || pEv->postfix[p-1]=='_')
              Push(pEv, pEv->postfix[--p]);
            //while(p && pEv->postfix[p-1] != ' ')
            //  Push(pEv, pEv->postfix[--p]);
            Push(pEv, pInfix[i]);
            break;
          case RB:
            while(pEv->top > -1 && (next = Pop(pEv)) != '[')
              pEv->postfix[p++] = next;
            pEv->postfix[p++] = '[';
            pEv->postfix[p++] = ']';
            break;
          case OPERAND:
            /* Test for special operands */

            if (last != OPERAND)
              {
                /* Test for "%lo" command */

                if (strncmp((char *) "%lo", &pInfix[i], 3) == 0)
                  {
                     Push(pEv, FUN_LO);
                     i += 2;
                     break;
                  }

                /* Test for "%hi" command */

                if (strncmp((char *) "%hi", &pInfix[i], 3) == 0)
                  {
                     Push(pEv, FUN_HI);
                     i += 2;
                     break;
                  }
              }

            if (pInfix[i] == '"')
              inQuote = TRUE;

            if (!(pInfix[i] == ' ' && lastch == ' '))
              pEv->postfix[p++] = pInfix[i];
            lastch = pInfix[i];
            break;
          case OPERATOR:
            if ((last == OPERATOR || last == '?' || last == LP) && pInfix[i] == '-')
              {
                /* Urinay negate */

                Push(pEv, NEGATE);
                break;
              }
            if ((last == OPERAND) && (lastch != ' '))
              Push(pEv, ' ');

            /* Test for special operators */

            op = pInfix[i];
            if (op == '<' && pInfix[i+1] == '=')
              {
                op = COND_LTE;
                i++;
              }
            else if (op == '<' && pInfix[i+1] == '>')
              {
                op = COND_NE;
                i++;
              }
            else if (op == '>' && pInfix[i+1] == '=')
              {
                op = COND_GTE;
                i++;
              }
            else if (op == '!' && pInfix[i+1] == '=')
              {
                op = COND_NE;
                i++;
              }
            else if (op == '<' && pInfix[i+1] == '<')
              {
                op = LEFT_SHIFT;
                i++;
              }
            else if (op == '>' && pInfix[i+1] == '>')
              {
                op = RIGHT_SHIFT;
                i++;
              }
            else if (op == '|' && pInfix[i+1] == '|')
              {
                op = LOGICAL_OR;
                i++;
              }
            else if (op == '&' && pInfix[i+1] == '&')
              {
                op = LOGICAL_AND;
                i++;
              }
            else if (op == '=' && pInfix[i+1] == '=')
              {
                op = COND_EQ;
                i++;
              }

            // Get order of precidence
            prec = GetPrec(op);

            // Process stack until order of precidence says stop
            while(pEv->top > -1 && prec <= GetPrec(pEv->stack.ch[pEv->top]))
              pEv->postfix[p++] = Pop(pEv);
            Push(pEv, op);
            break;
        }

      /* Set the last operand type */

      if (!(type == OPERAND && lastch == ' '))
        last = type;
      i++;
    }
  while(pEv->top > -1)
    pEv->postfix[p++] = Pop(pEv);
  pEv->postfix[p] = '\0';

  /* Return pointer to next un-parsed byte */
  
  pEv->bytesEvaled = i;
  return i;
}

static int GetType(char sym)
{
  switch(sym)
    {
      case '(':
        return(LP);
      case ')':
        return(RP);
      case '[':
        return(LB);
      case ']':
        return(RB);
      case '+':
      case '-':
      case '*':
      case '/':
      case '=':
      case '<':
      case '>':
      case '!':
      case '&':
      case '|':
      case '^':
        return(OPERATOR);
      default :
        return(OPERAND);
    }
}

static void Push(EvalDef_t *pEv, char sym)
{
  if (pEv->top > N)
    {
      STACK_DEBUG_PRINT("\nStack is full\n");
      return;
    }
  else
    pEv->stack.ch[++pEv->top] = sym;
}

static char Pop(EvalDef_t *pEv)
{
  if (pEv->top <= -1)
    {
      STACK_DEBUG_PRINT("\nStack is empty\n");
      return 0;
    }
  else
    return(pEv->stack.ch[pEv->top--]);
}

static void PushF(EvalDef_t *pEv, StackType_t sym)
{
  if (pEv->top > STACK_SIZE)
    {
      STACK_DEBUG_PRINT("\nStack is full\n");
      return;
    }
  else
    pEv->stack.s[++pEv->top] = sym;
}

static StackType_t PopF(EvalDef_t *pEv)
{
  StackType_t  temp;

  if (pEv->top <= -1)
    {
      STACK_DEBUG_PRINT("\nStack is empty\n");
      temp.u.f = 0.0;
      temp.type = STACK_TYPE_FLOAT;
      return temp;
    }
  else
    return(pEv->stack.s[pEv->top--]);
}

/*
================================================================================
GetPrec:  Returns the mathematical order of precidence for the specified
          operator.  This gives us information about when to Push / Pop the
          operation stack.
================================================================================
*/
static int GetPrec(unsigned char sym)
{
  switch (sym)
    {
      case '(':
        return(LPP);
      case '+':
        return(AP);
      case '-':
        return(SUBP);
      case '*':
        return(MP);
      case '/':
        return(DP);
      case '%':
        return(REMP);
      case '[':
        return(LBP);
      case '^':
        return(EXPP);
      case RIGHT_SHIFT:
      case LEFT_SHIFT:
        return(SHIFT);
      case '<':
      case '>':
      case COND_LT:
      case COND_LTE:
      case COND_GT:
      case COND_GTE:
        return(GT);
      case COND_EQ:
      case COND_NE:
      case '=':
        return EQ;
      case '!':
        return NOT;
      case LOGICAL_AND:
        return LAND;
      case LOGICAL_OR:
        return LOR;
      case FUN_HI:
      case FUN_LO:
        return FUN;
      default :
        return(NONE);
    }
}

/*
================================================================================
expr_Evaluate:   Evaluate arguments so it results in a floating point value

returns:  TRUE if evaluation results in a value
          FALSE if expression cannot be reduced to a value
================================================================================
*/
int expr_Evaluate(EvalDef_t* pEv, FAR const char* pExpr, unsigned char isatty, 
                FloatType_t *result, void *pContext)
{
  unsigned char   len;
  StackType_t     a, b, temp;
  unsigned char   i;
  FloatType_t     value;

  /* Call infix to postfix conversion routine */

  InfixToPostfix(pEv, pExpr);

  /* Calculate the value based on postfix conversion */

  len = strlen(pEv->postfix);
  pEv->postfix[len] = '#';

  for (i = 0; pEv->postfix[i] != '#'; i++)
    {
      /* Initialize temp to zero */

      temp.type = STACK_TYPE_FLOAT;
      temp.u.f = 0.0;

      /* Skip whitespace in postfix array */

      if (pEv->postfix[i] == ' ')
        continue;

      /* Convert numeric values */

      if ((pEv->postfix[i] <= '9' && pEv->postfix[i] >= '0') ||
          (pEv->postfix[i] == '.' && 
              !(isalpha(pEv->postfix[i-1]) ||
                isalpha(pEv->postfix[i+1])
               )   
          )
         )
        {
          /* Convert ASCII to decimal */

          len = 0;
          while ((pEv->postfix[i] <= '9' && pEv->postfix[i] >= '0') ||
                 pEv->postfix[i] == '.' || pEv->postfix[i] == 'x' ||
                 (pEv->postfix[i] >= 'a' && pEv->postfix[i] <= 'f') ||
                 (pEv->postfix[i] >= 'A' && pEv->postfix[i] <= 'F'))
            pEv->floatStr[len++] = pEv->postfix[i++];
          i--;
          pEv->floatStr[len] ='\0';

          /* Push it to the stack */

          if (strncmp(pEv->floatStr, "0x", 2) == 0)
              temp.u.f = hextoi(pEv->floatStr);
          else
              temp.u.f = atof(pEv->floatStr);

          temp.type = STACK_TYPE_FLOAT;
          PushF(pEv, temp);
        }

      else if ((pEv->postfix[i] >= 'a' && pEv->postfix[i] <= 'z') ||
               ((pEv->postfix[i] >= 'A' && pEv->postfix[i] <= 'Z')) ||
               pEv->postfix[i] == '_' || pEv->postfix[i] == '.' ||
               pEv->postfix[i] == '$')
        {
          /* It is a variable.  Pull it from the postfix string and get its value */

          len = 0;
          while ((pEv->postfix[i] >= 'a' && pEv->postfix[i] <= 'z') ||
                 ((pEv->postfix[i] >= 'A' && pEv->postfix[i] <= 'Z')) ||
                 ((pEv->postfix[i] >= '0' && pEv->postfix[i] <= '9')) ||
                 pEv->postfix[i] == '_' || pEv->postfix[i] == ':' ||
                 pEv->postfix[i] == '.' || pEv->postfix[i] == '$')
            {
              /* Limit our variables to MAX_VAR_LEN bytes */

              if (len < sizeof(pEv->floatStr))
                pEv->floatStr[len++] = pEv->postfix[i++];
              else
                i++;
            }

          i--;

          pEv->floatStr[len++] = '\0';

          /* Now try to find the variable */

          if (expr_FindVariable(pEv, pEv->floatStr, pContext, &value))
            {
              /* Now push the variable to the stack */
              temp.u.f = value;

              PushF(pEv, temp);
            }
          else
            {
               return 0;
            }
        }

      /* Test for variable array */

      else if (pEv->postfix[i] == '[' && pEv->postfix[i+1] == ']')
        {
          if (pEv->top <= -1)
            return 0;

          /* Increment past [] */

          i += 2;
          while (pEv->postfix[i] == ' ')
            i++;

          /* It is a variable.  Pull it from the postfix string and get its value */

          len = 0;
          while ((pEv->postfix[i] >= 'a' && pEv->postfix[i] <= 'z') ||
                 ((pEv->postfix[i] >= 'A' && pEv->postfix[i] <= 'Z')) ||
                 ((pEv->postfix[i] >= '0' && pEv->postfix[i] <= '9')) ||
                 pEv->postfix[i] == '_' || pEv->postfix[i] == ':' ||
                 pEv->postfix[i] == '.')
            {
              /* Limit our variables to MAX_VAR_LEN bytes */

              if (len < sizeof(pEv->floatStr))
                pEv->floatStr[len++] = pEv->postfix[i++];
              else
                i++;
            }
          i--;

          pEv->floatStr[len++] = '[';
          pEv->floatStr[len] = '\0';

          /* Get array index value from stack */
          a = PopF(pEv);
          sprintf(&pEv->floatStr[len], "%d", (int) a.u.f);
          len = strlen(pEv->floatStr);
          pEv->floatStr[len++] = ']';
          pEv->floatStr[len++] = '\0';

          /* Now try to find the variable */

          if (expr_FindVariable(pEv, pEv->floatStr, pContext, &value))
            {
              /* Now push the variable to the stack */
              temp.u.f = value;

              PushF(pEv, temp);
            }
          else
            {
               return 0;
            }
        }
      else
        {
          /* Process operator */
          if (pEv->top <= -1)
            return 0;

          a = PopF(pEv);
          if (pEv->postfix[i] != NEGATE && pEv->postfix[i] != FUN_HI &&
                pEv->postfix[i] != FUN_LO)
            {
              if (pEv->top <= -1)
                return 0;
              b = PopF(pEv);
            }

          switch ((unsigned char) pEv->postfix[i])
            {
              case '+':
                if (b.type == STACK_TYPE_FLOAT && a.type == STACK_TYPE_FLOAT)
                  temp.u.f = b.u.f + a.u.f;
                else
                  {
                    /* TODO: Add string concatenation */
                  }
                break;
              case '-':
                temp.u.f = b.u.f - a.u.f;
                break;
              case '*':
                temp.u.f = b.u.f * a.u.f;
                break;
              case '|':
                temp.u.f = (int) b.u.f | (int) a.u.f;
                break;
              case '&':
                temp.u.f = (int) b.u.f & (int) a.u.f;
                break;
              case '/':
                temp.u.f = b.u.f / a.u.f;
                break;
              case LEFT_SHIFT:
                temp.u.f = (FloatType_t) ((unsigned int) b.u.f << (unsigned int) a.u.f);
                break;
              case RIGHT_SHIFT:
                temp.u.f = (FloatType_t) ((unsigned int) b.u.f >> (unsigned int) a.u.f);
                break;
              case NEGATE:
                temp.u.f = -a.u.f;
                break;
              case '<':
              case COND_LT:
                if (b.type == STACK_TYPE_FLOAT && a.type == STACK_TYPE_FLOAT)
                  temp.u.f = b.u.f < a.u.f;
                
                break;
              case '>':
              case COND_GT:
                if (b.type == STACK_TYPE_FLOAT && a.type == STACK_TYPE_FLOAT)
                  temp.u.f = b.u.f > a.u.f;

                break;
              case COND_GTE:
                if (b.type == STACK_TYPE_FLOAT && a.type == STACK_TYPE_FLOAT)
                  temp.u.f = b.u.f >= a.u.f;

                break;
              case COND_LTE:
                if (b.type == STACK_TYPE_FLOAT && a.type == STACK_TYPE_FLOAT)
                  temp.u.f = b.u.f <= a.u.f;

                break;
              case COND_NE:
              case COND_LG:
                if (b.type == STACK_TYPE_FLOAT && a.type == STACK_TYPE_FLOAT)
                  temp.u.f = b.u.f != a.u.f;
                break;
              case COND_EQ:
                if (b.type == STACK_TYPE_FLOAT && a.type == STACK_TYPE_FLOAT)
                  temp.u.f = b.u.f == a.u.f;
                break;
              case LOGICAL_AND:
                temp.u.f = b.u.f != 0.0 && a.u.f != 0.0;
                break;
              case LOGICAL_OR:
                temp.u.f = b.u.f != 0.0 || a.u.f != 0.0;
                break;
              case FUN_LO:
                temp.u.f = (float) (((uint32_t) a.u.f) & 0xFFFF);
                break;
              case FUN_HI:
                temp.u.f = (float) (((uint32_t) a.u.f) >> 16);
                break;
              case '=':
                if (b.type == STACK_TYPE_FLOAT && a.type == STACK_TYPE_FLOAT)
                  temp.u.f = b.u.f == a.u.f;
                break;
              case '^':
                temp.u.f = pow (b.u.f , a.u.f);
                break;
              default:
                /* Unknown operator!!! */

                if (isatty)
                  {
                    /* Print the error */

                    if (pEv->postfix[i] == '"')
                      printf("Unknown operator: '%c' (no string vars support)\r\n",
                                 pEv->postfix[i]);
                    else
                      printf("Unknown operator: '%c'\r\n", pEv->postfix[i]);
                    *result = 0.0;
                    return 0;
                  }
            }

          /* Push result back to stack */

          PushF(pEv, temp);
        }
    }

  /* Get the final result and return it */

  if (pEv->top <= -1)
    return 0;
  temp = PopF(pEv);
  *result = temp.u.f;
  return 1;
}
