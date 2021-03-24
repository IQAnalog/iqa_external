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
// File        : f1000_regs.c                                                   *
// Author      : KDP                                                            *
// Created     : 11/08/2018                                                     *
//                                                                              *
// Description:  F1000 register interface routines.                             *
//                                                                              *
//*******************************************************************************

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
#include <stdarg.h>

#include "client.h"

/***************************************************************************
* Macro Definitions
****************************************************************************/

#define F1000_REV_ADDR        0x40000000
#define F1000_REV_A_ID        0x01
#define F1000_REV_B_ID        0x02

#define F1000_REV_A_INDEX     0
#define F1000_REV_B_INDEX     1
#define F1000_REV_MAX_INDEX   2

#define     F1000_ID                1

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

int client_read_register(client_t *pClient, uint32_t f1000_id, 
      uint32_t ep_id, uint32_t addr, uint32_t *val);
int client_write_register(client_t *pClient, uint32_t f1000_id, 
      uint32_t ep_id, uint32_t addr, uint32_t val);

/****************************************************************************
 * Private Data
 ****************************************************************************/

static const char *gpMapfilename[] = {
				"regs/f1000_register_map.csv",
                         	"regs/rev_b/f1000b_register_map.csv",
};

/****************************************************************************
 * Public Data
 ****************************************************************************/

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * f1000_regs_cmd_printf
 *
 * Print string to command window
 ****************************************************************************/

int cmd_printf(void *pPrintfCtx, FAR const char *fmt, ...)
{
  va_list ap;
  int     ret;

  va_start(ap, fmt);
  ret = vfprintf(stdout, fmt, ap);
  va_end(ap);

  return ret;
}

/****************************************************************************
 * f1000_regs_src_printf
 *
 * Print string to source window
 ****************************************************************************/

static int src_printf(void *pCtx, FAR const char *fmt, ...)
{
  va_list ap;
  int     ret;

  va_start(ap, fmt);
  ret = vfprintf(stdout, fmt, ap);
  va_end(ap);

  return ret;
}

/****************************************************************************
 * f1000_regs_free_regdef
 *
 * Free memory used by the register definition file.
 ****************************************************************************/

static void f1000_regs_free_regdef(f1000_regs_ctx_t *pCtx)
{
  f1000_reg_def_t   *pReg;
  f1000_reg_def_t   *pNextReg;
  f1000_field_def_t *pField;
  f1000_field_def_t *pNextField;

  /* Loop through each register definition and free it */
  pReg = pCtx->pFirstRegDef;
  while (pReg)
  {
    /* Get pointer to next item */
    pNextReg = (f1000_reg_def_t *) pReg->listitem.pNext;

    /* Loop for all field definitions in this reg and free them */
    pField = pReg->pFirstFieldDef;
    while (pField)
    {
      /* Get pointer to next item */
      pNextField = (f1000_field_def_t *) pField->listitem.pNext;

      /* Free the memory */
      free (pField);

      /* Point to next item for loop iteration */
      pField = pNextField;
    }

    /* Free the memory */
    free(pReg);

    /* Point to next item for next loop iteration */
    pReg = pNextReg;
  }

  /* Mark the memory as freed */
  pCtx->pFirstRegDef = NULL;
}

/****************************************************************************
 * f1000_regs_read_register
 *
 * Read the specified register from the F1000 chip
 ****************************************************************************/

static uint32_t f1000_regs_read_register(f1000_regs_ctx_t *pCtx, uint32_t addr)
{
  int       ret;
  uint32_t  val;
  client_t  *pClient = pCtx->pClient;

  /* Perform the read */
  ret = client_read_register(pClient, pClient->f1000_id, pClient->ep_id, addr, &val);

  /* Test for error */
  if (ret != OK)
  {
    pCtx->cmd_printf(pCtx->pPrintfCtx, "Error reading register 0x%08X\r\n", addr);
    val = 0xDEADBEEF;
  }

  return val;
}

/****************************************************************************
 * f1000_regs_get_chip_revision
 *
 * Read the chip revision from the F1000 chip
 ****************************************************************************/

static int f1000_regs_get_chip_revision(f1000_regs_ctx_t *pCtx, uint32_t *rev)
{
  int       ret;
  client_t  *pClient = pCtx->pClient;

  /* Perform the read */
  ret = client_read_register(pClient, pClient->f1000_id, pClient->ep_id, F1000_REV_ADDR, rev);

  /* Test for error */
  if (ret != OK)
  {
    pCtx->cmd_printf(pCtx->pPrintfCtx, "Error reading chip revision\r\n");
    *rev = 0xDEADBEEF;
  }
  else
  {
    *rev &= 0xff;
  }

  return ret;
}

/****************************************************************************
 * f1000_regs_list_insert
 *
 * Insert list item into a list alphapetically.
 ****************************************************************************/

static void f1000_regs_list_insert(f1000_sort_list_t **pListHead,
                                  f1000_sort_list_t *pListItem)
{
  f1000_sort_list_t *pCmp;
  f1000_sort_list_t *pPrev;

  pListItem->pNext = NULL;
  if (*pListHead == NULL)
  {
    *pListHead = pListItem;
  }
  else
  {
    /* Start with first item in list for comparison */

    pCmp = *pListHead;
    pPrev = pCmp;
    while (pListItem && pCmp)
    {
      /* Compare with this item */

      if (strcasecmp(pListItem->name, pCmp->name) < 0)
      {
        /* Insert prior to this item (*/

        pListItem->pNext = pCmp;
        if (pCmp == *pListHead)
        {
          *pListHead = pListItem;
        }
        else
        {
          pPrev->pNext = pListItem;
        }

        pListItem = NULL;
      }
      else
      {
        /* Advance to next item in list */

        pPrev = pCmp;
        pCmp = pCmp->pNext;
      }
    }

    /* If we reached the end of the list, then we must insert 
     * at the end because we are last alphabetically
     */

    if (pListItem != NULL)
    {

      pPrev->pNext = pListItem;
    }
  }
}

/****************************************************************************
 * f1000_regs_parse_mapfile
 *
 * This routine reads and parses the f1000_register_map file to determine 
 * the name and base address of all known register blocks.
 ****************************************************************************/

static int f1000_regs_parse_mapfile(f1000_regs_ctx_t *pCtx)
{
  int   fd;
  char  *line;
  char  *new_line;
  char  *token;
  int   len;
  int   namelen;
  f1000_reg_block_t *pBlock;
  client_t  *pClient = pCtx->pClient;
  uint32_t rev;
  int   x;

  /* Try to allocate a buffer */
  pCtx->buffer = (char *) malloc(pCtx->buflen);
  if (pCtx->buffer == NULL)
  {
    return -ENOMEM;
  }

  if (pClient == NULL)
  {
    return -ENOSYS;
  }
  /*Get the revision using the F1000 API*/
  f1000_regs_get_chip_revision(pCtx, &rev);

  /* Try to open the file */
  if ((fd = open( rev == F1000_REV_A_ID ? gpMapfilename[F1000_REV_A_INDEX]:
            gpMapfilename[F1000_REV_B_INDEX], O_RDONLY)) == -1)
  {
    pCtx->cmd_printf(pCtx->pPrintfCtx, "Unable to open %s or\n", gpMapfilename[F1000_REV_A_INDEX]);
    pCtx->cmd_printf(pCtx->pPrintfCtx, "Unable to open %s\n", gpMapfilename[F1000_REV_B_INDEX]);
    free(pCtx->buffer);
    return -ENOSYS;
  }

  /* Read all data */

  len = read(fd, pCtx->buffer, pCtx->buflen);
  line = pCtx->buffer;
  for (x = 0; x < sizeof(pCtx->level_len) / sizeof(pCtx->level_len[0]); ++x)
  {
    pCtx->level_len[x] = 0;
  }

  /* Loop for all lines in the file */
  while (line != NULL)
  {
    /* Find end of line and terminate */
    new_line = line;
    while (new_line < pCtx->buffer+len && *new_line != '\n')
    {
      new_line++;
    }
    *new_line++ = 0;

    /* Test if first line is a header */
    if (strncmp(line, "Address", 7) == 0)
    {
      line = new_line;
      continue;
    }

    /* Allocate a new f1000_reg_block */

    pBlock = (f1000_reg_block_t *) malloc(sizeof(f1000_reg_block_t));
    if (pBlock == NULL)
    {
      close(fd);
      return -ENOMEM;
    }
    memset(pBlock, 0, sizeof(*pBlock));
    pBlock->listitem.name = pBlock->name;

    /* Get the register address */

    token = strtok(line, "x, ");
    token = strtok(NULL, "x, ");
    sscanf(token, "%x", &pBlock->baseaddr);

    /* Get the register block name */

    token = strtok(NULL, ", ");
    strncpy(pBlock->name, token, sizeof(pBlock->name));
    namelen = strlen(pBlock->name);
    if (namelen > pCtx->level_len[1])
      pCtx->level_len[1] = namelen;

    /* Get the filename */

    token = strtok(NULL, ", ");
    strncpy(pBlock->filename, token, sizeof(pBlock->filename));

    /* Insert the block into our list in alphabetical name order */
    f1000_regs_list_insert((f1000_sort_list_t **) &pCtx->pFirstRegBlock, 
                           &pBlock->listitem);

    /* Update line pointer */

    if (new_line >= pCtx->buffer + len)
    {
      line = NULL;
    }
    else
    {
      line = new_line;
    }
  }

  close(fd);
  return OK;
}

/****************************************************************************
 * f1000_regs_open_level2
 *
 * Find and open the register description file (level 2 tab expansion)
 ****************************************************************************/

static int f1000_regs_open_level2(f1000_regs_ctx_t *pCtx, char *reg_arg)
{
  char              str[128];
  int               len, x;
  int               fd;
  int               toklen;
  char              *line;
  char              *saveptr;
  char              *token;
  f1000_reg_block_t *pBlock;
  f1000_reg_def_t   *pReg;
  f1000_field_def_t *pField;
  uint32_t rev;


  /* Free the register definition memory if allocated */
  f1000_regs_free_regdef(pCtx);

  /* Get the reg_block name */
  strncpy(pCtx->reg_block, reg_arg, sizeof(pCtx->reg_block));
  for (x = 0; pCtx->reg_block[x] != '.' && pCtx->reg_block[x] != '\0'; )
    x++;
  pCtx->reg_block[x] = '\0';

  /* Find the reg_block name in our list of known reg maps */
  pReg = NULL;
  pField = NULL;
  pBlock = pCtx->pFirstRegBlock;
  pCtx->level_len[2] = 0;

  /*
  Get the chip revision using the F1000 API to load corresponding csv files
  */
  f1000_regs_get_chip_revision(pCtx, &rev);

  while (pBlock)
  {
    /* Test for match with this item */
    if (strcasecmp(pBlock->name, pCtx->reg_block) == 0)
    {
      /* Found, try to open and parse the file specified */

      sprintf(str, rev == F1000_REV_A_ID ?"regs/csv/%s":
          "regs/rev_b/csv/%s" , pBlock->filename);
      fd = open(str, O_RDONLY);
      if (fd == -1)
        return -ENOSYS;

      /* Read data into our context buffer */
      len = read(fd, pCtx->buffer, pCtx->buflen);

      line = pCtx->buffer;
      for (x = 0; x < len; x++)
      {
        /* Search for line ending */
        if (pCtx->buffer[x] == '\n')
        {
          /* NULL terminate the line and process it */
          pCtx->buffer[x] = '\0';

          /* Test for zero length line */
          if (strlen(line) == 0)
          {
            /* Close the previous register definition */
            pReg = NULL;
          }
          else if (line[0] == '#')
          {
            // Do nothing for comment line
          }
          else if (pReg)
          {
            /* Process the line as a field definition */
            pField = (f1000_field_def_t *) malloc(sizeof(*pField));
            if (pField == NULL)
            {
              close(fd);
              return -ENOMEM;
            }
            memset(pField, 0, sizeof(*pField));

            /* Get register attribute */
            if (*line != ',')
            {
              token = strtok_r(line, ",", &saveptr);
            }

            /* Get bit definition */
            if (*line == ',')
              token = strtok_r(line, ",", &saveptr);
            else
              token = strtok_r(NULL, ",", &saveptr);
            pField->high_bit = atoi(++token);
            pField->low_bit = pField->high_bit;
            while (*token != '\0')
            {
              /* Increment to next character */
              token++;

              /* Test for ':' in declaration */
              if (*token == ':')
              {
                pField->low_bit = atoi(++token);
                break;
              }
            }

            /* Get the field name */
            token = strtok_r(NULL, ",", &saveptr);
            pField->listitem.name = token;
            toklen = strlen(token);
            if (toklen > pCtx->level_len[3])
              pCtx->level_len[3] = toklen;

            /* Get the field description */
            pField->desc = saveptr;

            /* Remove default value by searching backward
             * from end of token in case there are embedded
             * commas enclosed in quotes.
             */
            toklen = strlen(saveptr);
            while (toklen > 0 && saveptr[toklen] != ',')
              toklen--;
            saveptr[toklen] = '\0';

            /* Insert into the list */
            f1000_regs_list_insert((f1000_sort_list_t **) &pReg->pFirstFieldDef,
               &pField->listitem);
          }
          else
          {
            /* Process the line as a register definition */
            pReg = (f1000_reg_def_t *) malloc(sizeof(*pReg));
            if (pReg == NULL)
            {
              close(fd);
              return -ENOMEM;
            }
            memset(pReg, 0, sizeof(*pReg));

            /* Get register offset */
            token = strtok_r(line, ",", &saveptr);
            pReg->offset = atoi(token);

            /* Get number of bits in the register */
            token = strtok_r(NULL, ",", &saveptr);
            pReg->num_bits = atoi(token);

            /* Get register name */
            token = strtok_r(NULL, ",", &saveptr);
            pReg->listitem.name = token;
            toklen = strlen(token);
            if (toklen > pCtx->level_len[2])
              pCtx->level_len[2] = toklen;

            /* Get register RW / RO type */
            pReg->rw = strtok_r(NULL, ",", &saveptr);

            /* Insert into the list */
            f1000_regs_list_insert((f1000_sort_list_t **) &pCtx->pFirstRegDef,
               &pReg->listitem);
          }

          /* Get pointer to the next line */
          line = &pCtx->buffer[x+1];
        }
      }

      /* Close the file */
      close(fd);
      return OK;
    }

    pBlock = (f1000_reg_block_t *) pBlock->listitem.pNext;
  }

  return -ENOSYS;
}

/****************************************************************************
 * f1000_regs_find_block_def
 *
 * Find pointer to the specificed register block name.
 ****************************************************************************/

static f1000_reg_block_t *f1000_regs_find_block_def(f1000_regs_ctx_t *pCtx,
          char *block)
{
  f1000_reg_block_t *pBlock;

  /* Start with first item in the list */
  pBlock = pCtx->pFirstRegBlock;
  while (pBlock)
  {
    /* Test the next item */
    if (strcasecmp(pBlock->name, block) == 0)
    {
      /* We found the block definiton.  Now ensure that file
       * is loaded so we can access the registers.
       */
      if (pCtx->pFirstRegDef == NULL || strcasecmp(pCtx->reg_block, block) != 0)
      {
        f1000_regs_open_level2(pCtx, block);
        if (pCtx->pFirstRegDef == NULL)
          return NULL;
      }

      return pBlock;
    }

    /* Point to next item */
    pBlock = (f1000_reg_block_t *) pBlock->listitem.pNext;
  }

  /* Not found, return NULL */
  return pBlock;
}

/****************************************************************************
 * f1000_regs_find_reg_def
 *
 * Find pointer to the specificed register block name.
 ****************************************************************************/

static f1000_reg_def_t *f1000_regs_find_reg_def(f1000_regs_ctx_t *pCtx,
          char *reg)
{
  f1000_reg_def_t *pReg;

  /* Start with first item in the list */
  pReg = pCtx->pFirstRegDef;
  while (pReg)
  {
    /* Test the next item */
    if (strcasecmp(pReg->listitem.name, reg) == 0)
    {
      return pReg;
    }

    /* Point to next item */
    pReg = (f1000_reg_def_t *) pReg->listitem.pNext;
  }

  /* Not found, return NULL */
  return pReg;
}

/****************************************************************************
 * f1000_regs_print_block_regs
 *
 * Print all registers in the 
 ****************************************************************************/

static void f1000_regs_print_block_regs(f1000_regs_ctx_t *pCtx, 
              f1000_reg_block_t *pBlock, FILE *fd,
              int print_as_int16)
{
  f1000_reg_def_t   *pReg;
  uint32_t          reg_val;
  int               len, maxlen;
  char              fmt[32];

  /* Find length of longest register name */
  pReg = pCtx->pFirstRegDef;
  maxlen = 0;
  while (pReg)
  {
    /* Test length of this item */
    len = strlen(pReg->listitem.name);
    if (len > maxlen)
      maxlen = len;

    /* Get pointer to next register */
    pReg = (f1000_reg_def_t *) pReg->listitem.pNext;
  }

  /* Create print format */
  sprintf(fmt, "%%-%ds(0x%%08x):  0x%%08x\r\n", maxlen + 2);

  /* Get pointer to first register */
  pReg = pCtx->pFirstRegDef;
  while (pReg)
  {
    /* Get the register value */
    reg_val = f1000_regs_read_register(pCtx, pBlock->baseaddr + pReg->offset * 4);

    /* Print the item */
    if (fd == NULL)
      pCtx->src_printf(pCtx->pPrintfCtx, fmt, pReg->listitem.name, pBlock->baseaddr + pReg->offset * 4, reg_val);
    else
    {
      if (print_as_int16)
      {
        int32_t   sreg_val = reg_val;
        if (reg_val & 0x8000)
          sreg_val |= 0xFFFF0000;
        fprintf(fd, "%d\n", sreg_val);
      }
      else
        fprintf(fd, "0x%08X\n", reg_val);
    }

    /* Get pointer to next register */
    pReg = (f1000_reg_def_t *) pReg->listitem.pNext;
  }
}

/****************************************************************************
 * f1000_regs_print_matching_regs
 *
 * Print all registers in the 
 ****************************************************************************/

static void f1000_regs_print_matching_regs(f1000_regs_ctx_t *pCtx, 
              f1000_reg_block_t *pBlock, char *reg, FILE* fd,
              int print_as_int16)
{
  f1000_reg_def_t   *pReg;
  uint32_t          reg_val;
  int               len, maxlen;
  int               namelen;
  char              fmt[32];

  /* Find length of longest register name */
  pReg = pCtx->pFirstRegDef;
  maxlen = 0;
  namelen = strlen(reg);
  while (pReg)
  {
    if (strncasecmp(pReg->listitem.name, reg, namelen) == 0)
    {
      /* Test length of this item */
      len = strlen(pReg->listitem.name);
      if (len > maxlen)
        maxlen = len;
    }

    /* Get pointer to next register */
    pReg = (f1000_reg_def_t *) pReg->listitem.pNext;
  }

  /* Create print format */
  sprintf(fmt, "%%-%ds(0x%%08x):  0x%%08x\r\n", maxlen + 2);

  /* Get pointer to first register */
  pReg = pCtx->pFirstRegDef;
  while (pReg)
  {
    if (strncasecmp(pReg->listitem.name, reg, namelen) == 0)
    {
      /* Get the register value */
      reg_val = f1000_regs_read_register(pCtx, pBlock->baseaddr + pReg->offset * 4);
      
      /* Print the item */
      if (fd == NULL)
        pCtx->src_printf(pCtx->pPrintfCtx, fmt, pReg->listitem.name, pBlock->baseaddr + pReg->offset * 4, reg_val);
      else
      {
        if (print_as_int16)
        {
          int32_t   sreg_val = reg_val;
          if (reg_val & 0x8000)
            sreg_val |= 0xFFFF0000;
          fprintf(fd, "%d\n", sreg_val);
        }
        else
          fprintf(fd, "0x%08x\n", reg_val);
      }
    }

    /* Get pointer to next register */
    pReg = (f1000_reg_def_t *) pReg->listitem.pNext;
  }
}

/****************************************************************************
 * f1000_regs_set_default_colors
 *
 * Configure the terminal for "defaultname" colors
 ****************************************************************************/

static void f1000_regs_set_default_colors(f1000_regs_ctx_t *pCtx)
{
#ifndef STANDALONE
  struct termcurses_colors_s colors;

  /* Initialize colors */
  colors.color_mask = TCURS_COLOR_FG;
  colors.fg_red   = 0xff;
  colors.fg_green = 0xff;
  colors.fg_blue  = 0xff;
  termcurses_setcolors(pCtx->tcurs, &colors);
#else
  if (pCtx->printfColors)
    pCtx->cmd_printf(pCtx->pPrintfCtx, "\\c0");
#endif
}

/****************************************************************************
 * f1000_regs_set_name_colors
 *
 * Configure the terminal for "name" colors
 ****************************************************************************/

static void f1000_regs_set_name_colors(f1000_regs_ctx_t *pCtx)
{
#ifndef STANDALONE
  struct termcurses_colors_s colors;

  /* Initialize colors */
  colors.color_mask = TCURS_COLOR_FG;
  colors.fg_red   = 0x20;
  colors.fg_green = 0xc0;
  colors.fg_blue  = 0x20;
  termcurses_setcolors(pCtx->tcurs, &colors);
#else
  if (pCtx->printfColors)
    pCtx->cmd_printf(pCtx->pPrintfCtx, "\\c2");
#endif
}

/****************************************************************************
 * f1000_regs_set_field_colors
 *
 * Configure the terminal for "field" colors
 ****************************************************************************/

static void f1000_regs_set_field_colors(f1000_regs_ctx_t *pCtx)
{
#ifndef STANDALONE
  struct termcurses_colors_s colors;

  /* Initialize colors */
  colors.color_mask = TCURS_COLOR_FG;
  colors.fg_red   = 0xa0;
  colors.fg_green = 0xa0;
  colors.fg_blue  = 0xff;
  termcurses_setcolors(pCtx->tcurs, &colors);
#else
  if (pCtx->printfColors)
    pCtx->cmd_printf(pCtx->pPrintfCtx, "\\c5");
#endif
}

/****************************************************************************
 * f1000_regs_set_value_colors
 *
 * Configure the terminal for "value" colors
 ****************************************************************************/

static void f1000_regs_set_value_colors(f1000_regs_ctx_t *pCtx)
{
#ifndef STANDALONE
  struct termcurses_colors_s colors;

  /* Initialize colors */
  colors.color_mask = TCURS_COLOR_FG;
  colors.fg_red   = 0xff;
  colors.fg_green = 0xff;
  colors.fg_blue  = 0;
  termcurses_setcolors(pCtx->tcurs, &colors);
#else
  if (pCtx->printfColors)
    pCtx->cmd_printf(pCtx->pPrintfCtx, "\\c4");
#endif
}

/****************************************************************************
 * f1000_regs_calc_field_mask
 *
 * Calculate the AND mask for the specified field
 ****************************************************************************/

static uint32_t f1000_regs_calc_field_mask(f1000_field_def_t *pField)
{
  int      mask;
  uint32_t x;

  mask = 1;
  for (x = pField->low_bit; x < pField->high_bit; x++)
  {
    mask = (mask << 1) | 1;
  }

  return mask;
}

/****************************************************************************
 * f1000_regs_set_value
 *
 * Find the specified field by name in the given register block.
 ****************************************************************************/

static void f1000_regs_set_value(f1000_regs_ctx_t *pCtx, 
              f1000_reg_block_t *pBlock,
              f1000_reg_def_t *pReg, f1000_field_def_t *pField,
              uint32_t value)
{
  uint32_t    temp;
  int         ret;
  uint32_t    mask;
  uint32_t    addr;
  client_t    *pClient = pCtx->pClient;

  /* Get the register address */
  addr = pBlock->baseaddr + pReg->offset * 4;

  /* If the field is given, then do a read / modifiy / write */
  if (pField != NULL)
  {
    /* First do a read */
    ret = client_read_register(pClient, pClient->f1000_id, pClient->ep_id, addr, &temp);
    if (ret != 0)
    {
      pCtx->cmd_printf(pCtx->pPrintfCtx, "Error reading from register 0x%08X\r\n", addr);
      return;
    }

    /* Perform register masking and set new value */
    mask = f1000_regs_calc_field_mask(pField) << pField->low_bit;
    temp &= ~mask;
    temp |= (value << pField->low_bit) & mask;

    /* Now write the new value back to the register */
    ret = client_write_register(pClient, pClient->f1000_id, pClient->ep_id, addr, temp);
    if (ret != 0)
    {
      pCtx->cmd_printf(pCtx->pPrintfCtx, "Error writing to register 0x%08X\r\n", addr);
      return;
    }
  }
  else
  {
    /* Do a simple register write */
    ret = client_write_register(pClient, pClient->f1000_id, pClient->ep_id, addr, value);
    if (ret != 0)
    {
      pCtx->cmd_printf(pCtx->pPrintfCtx, "Error writing to register 0x%08X\r\n", addr);
      return;
    }
  }
}

/****************************************************************************
 * f1000_regs_find_field
 *
 * Find the specified field by name in the given register block.
 ****************************************************************************/

static f1000_field_def_t *f1000_regs_find_field(f1000_regs_ctx_t *pCtx, 
              f1000_reg_def_t *pReg, char *field)
{
  f1000_field_def_t *pField;

  pField = pReg->pFirstFieldDef;
  while (pField)
  {
    if (strcasecmp(pField->listitem.name, field) == 0)
    {
      /* Field found. */
      return pField;
    }

    /* Get pointer to next field */
    pField = (f1000_field_def_t *) pField->listitem.pNext;
  }

  /* Not found */
  return NULL;
}

/****************************************************************************
 * f1000_regs_print_matching_fields
 *
 * Print all bit fields in the given register
 ****************************************************************************/

static void f1000_regs_print_matching_fields(f1000_regs_ctx_t *pCtx, 
              f1000_reg_block_t *pBlock, f1000_reg_def_t *pReg, 
              char *field, int hex, FILE *fd, int print_as_int16)
{
  f1000_field_def_t *pField;
  uint32_t          reg_val;
  uint32_t          field_val;
  uint32_t          mask;
  int               len, maxlen;
  int               namelen;
  char              fmt[32];
  char              bitstr[16];

  /* Find length of longest register name */
  pField = pReg->pFirstFieldDef;
  maxlen = 0;
  namelen = strlen(field);
  while (pField)
  {
    if (*field == '\0' || strncasecmp(pField->listitem.name, field, namelen) == 0)
    {
      /* Test length of this item */
      len = strlen(pField->listitem.name);
      if (len > maxlen)
        maxlen = len;
    }

    /* Get pointer to next field */
    pField = (f1000_field_def_t *) pField->listitem.pNext;
  }
  sprintf(fmt, "%%-%ds", maxlen + 2);

  /* Print the register name */
  reg_val = f1000_regs_read_register(pCtx, pBlock->baseaddr + pReg->offset * 4);
  if (fd == NULL)
  {
    pCtx->src_printf(pCtx->pPrintfCtx, "\r\n%s ", pReg->listitem.name,  reg_val);
    fflush(stdout);
  }

  /* Print the register address */
  f1000_regs_set_field_colors(pCtx);
  if (fd == NULL)
  {
    pCtx->src_printf(pCtx->pPrintfCtx, "(0x%08x) = ", pBlock->baseaddr + pReg->offset * 4);
    fflush(stdout);
  }

  /* Print the register value */
  f1000_regs_set_value_colors(pCtx);
  if (fd == NULL)
  {
    pCtx->src_printf(pCtx->pPrintfCtx, "0x%08x\r\n", reg_val);
  }
  else
  {
    if (hex)
      fprintf(fd, "0x%08X\n", reg_val);
    else if (print_as_int16)
    {
      int32_t   sreg_val = reg_val;
      if (reg_val & 0x8000)
        sreg_val |= 0xFFFF0000;
      fprintf(fd, "%d\n", sreg_val);
    }
    else
      fprintf(fd, "%d\n", reg_val);
    return;
  }

  /* Get pointer to first register */
  pField = pReg->pFirstFieldDef;
  while (pField)
  {
    if (*field == '\0' || strncasecmp(pField->listitem.name, field, namelen) == 0)
    {
      /* Calculate the field value based on the bit position */
      mask = f1000_regs_calc_field_mask(pField);
      field_val = (reg_val >> pField->low_bit) & mask;

      /* Create the bit field definition string */
      if (pField->low_bit == pField->high_bit)
        sprintf(bitstr, "[%d]", pField->high_bit);
      else
        sprintf(bitstr, "[%d:%d]", pField->high_bit, pField->low_bit);

      /* Print the item */
      f1000_regs_set_name_colors(pCtx);
      pCtx->src_printf(pCtx->pPrintfCtx, fmt, pField->listitem.name);
      fflush(stdout);

      /* Print the bit definition */
      f1000_regs_set_field_colors(pCtx);
      pCtx->src_printf(pCtx->pPrintfCtx, "%-10s", bitstr);
      fflush(stdout);

      f1000_regs_set_value_colors(pCtx);
      if (hex)
        pCtx->src_printf(pCtx->pPrintfCtx, "0x%x\r\n", field_val);
      else
        pCtx->src_printf(pCtx->pPrintfCtx, "%d\r\n", field_val);
    }

    /* Get pointer to next register */
    pField = (f1000_field_def_t *) pField->listitem.pNext;
  }

  /* Restore colors */
  f1000_regs_set_default_colors(pCtx);
}

/****************************************************************************
 * f1000_regs_get_field_value
 *
 * Print all bit fields in the given register
 ****************************************************************************/

static int f1000_regs_get_field_value(f1000_regs_ctx_t *pCtx, 
              f1000_reg_block_t *pBlock, f1000_reg_def_t *pReg, 
              char *field, uint32_t *value)
{
  f1000_field_def_t *pField;
  uint32_t          reg_val;
  uint32_t          field_val;
  uint32_t          mask;

  /* Read the register value */
  reg_val = f1000_regs_read_register(pCtx, pBlock->baseaddr + pReg->offset * 4);

  /* Get pointer to first register */
  pField = pReg->pFirstFieldDef;
  while (pField)
  {
    if (strcasecmp(pField->listitem.name, field) == 0)
    {
      /* Calculate the field value based on the bit position */
      mask = f1000_regs_calc_field_mask(pField);
      field_val = (reg_val >> pField->low_bit) & mask;

      *value = field_val;
      return 0;
    }

    /* Get pointer to next register */
    pField = (f1000_field_def_t *) pField->listitem.pNext;
  }

  *value = 0xDEADBEEF;
  return -1;
}

/****************************************************************************
 * f1000_regs_readreg
 *
 * Read a register value
 ****************************************************************************/

int f1000_regs_readreg(f1000_regs_ctx_t *pCtx, const char *name, uint32_t *value)
{
  char              block[256];
  char              *reg;
  char              *field;
  f1000_reg_block_t *pBlock;
  f1000_reg_def_t   *pReg;

  /* Find pointers to register block, name and field */
  strncpy(block, name, sizeof(block));
  reg = block;
  while (*reg != '\0' && *reg != '.')
    reg++;
  if (*reg == '.')
  {
    *reg++ = '\0';
  }

  field = reg;
  while (*field != '\0' && *field != '.')
    field++;
  if (*field == '.')
  {
    *field++ = '\0';
  }

  /* Find the register block definition being accessed */
  pBlock = f1000_regs_find_block_def(pCtx, block);
  if (pBlock == NULL)
  {
    return -1;
  }

  /* If no register given, then print all regs and return */
  if (*reg == '\0')
  {
    return -1;
  }

  /* Find the register definition */
  pReg = f1000_regs_find_reg_def(pCtx, reg);
  if (pReg == NULL)
  {
    return -1;
  }

  if (*field == '\0')
  {
    /* Return the whole register */
    *value = f1000_regs_read_register(pCtx, pBlock->baseaddr + pReg->offset * 4);
    return 0;
  }
  else
  {
    /* Return only the field value */
    return f1000_regs_get_field_value(pCtx, pBlock, pReg, field, value);
  }
}

/****************************************************************************
 * f1000_regs_cmd_print
 *
 * Handle the "print" command
 ****************************************************************************/

int f1000_regs_cmd_print(f1000_regs_ctx_t *pCtx, FILE* fd,
                          int print_as_int16)
{
  char              *block;
  char              *reg;
  char              *field;
  int               hex;
  f1000_reg_block_t *pBlock;
  f1000_reg_def_t   *pReg;

  /* Test if enough args provided */
  if (pCtx->argc < 2)
    return 0xDEADBEEF;

  /* Determine if print is hex or decimal */
  hex = strcmp(pCtx->argv[0], "p") != 0;

  /* Find pointers to register block, name and field */
  block = pCtx->argv[1];
  reg = block;
  while (*reg != '\0' && *reg != '.')
    reg++;
  if (*reg == '.')
  {
    *reg++ = '\0';
  }

  field = reg;
  while (*field != '\0' && *field != '.')
    field++;
  if (*field == '.')
  {
    *field++ = '\0';
  }

  /* Find the register block definition being accessed */
  pBlock = f1000_regs_find_block_def(pCtx, block);
  if (pBlock == NULL)
  {
    pCtx->cmd_printf(pCtx->pPrintfCtx, "Unknown block definition %s\n", block);
    return 0;
  }

  /* If no register given, then print all regs and return */
  if (*reg == '\0')
  {
    f1000_regs_print_block_regs(pCtx, pBlock, fd,
        print_as_int16);
    return 1;
  }

  /* Find the register definition */
  pReg = f1000_regs_find_reg_def(pCtx, reg);
  if (pReg == NULL)
  {
    /* Should we print all registers that match? */
    f1000_regs_print_matching_regs(pCtx, pBlock, reg, fd,
        print_as_int16);
    return 1;
  }

  /* Print the matching fields */
  f1000_regs_print_matching_fields(pCtx, pBlock, pReg, field, hex, fd, 
                                   print_as_int16);
  return 1;
}

/****************************************************************************
 * f1000_regs_cmd_set
 *
 * Handle the "set" command
 ****************************************************************************/

void f1000_regs_cmd_set(f1000_regs_ctx_t *pCtx)
{
  char              *block;
  char              *reg;
  char              *field;
  char              *pStr;
  uint32_t          value;
  f1000_reg_block_t *pBlock;
  f1000_field_def_t *pField;
  f1000_reg_def_t   *pReg;

  /* Test if enough args provided */
  if (pCtx->argc < 2)
    return;

  /* Test for reg=val format */
  if (pCtx->argc == 2)
  {
    /* Find '=' in the 2nd arg */
    if ((pStr = strchr(pCtx->argv[1], '=')) != NULL)
    {
      *pStr++ = 0;
      if (*pStr != 0)
      {
        pCtx->argv[2] = pStr;
        pCtx->argc++;
      }
    } 
  }

  /* Test for reg = val format */
  if (pCtx->argc > 2 && strcmp(pCtx->argv[2], "=") == 0)
  {
    pCtx->argv[2] = pCtx->argv[3];
    pCtx->argc--;
  }

  /* Test that 3 args were given */
  if (pCtx->argc < 3)
  {
    pCtx->cmd_printf(pCtx->pPrintfCtx, "Please provide register / field value\r\n");
    return;
  }

  /* Find pointers to register block, name and field */
  block = pCtx->argv[1];
  reg = block;
  while (*reg != '\0' && *reg != '.')
    reg++;
  if (*reg == '.')
  {
    *reg++ = '\0';
  }

  field = reg;
  while (*field != '\0' && *field != '.')
    field++;
  if (*field == '.')
  {
    *field++ = '\0';
  }

  /* Find the register block definition being accessed */
  pBlock = f1000_regs_find_block_def(pCtx, block);
  if (pBlock == NULL)
  {
    pCtx->cmd_printf(pCtx->pPrintfCtx, "Unknown block definition %s\r\n", block);
    return;
  }

  /* If no register given, then report error */
  if (*reg == '\0')
  {
    pCtx->cmd_printf(pCtx->pPrintfCtx, "Please specify a register within %s\r\n", block);
    return;
  }

  /* Find the register definition */
  pReg = f1000_regs_find_reg_def(pCtx, reg);
  if (pReg == NULL)
  {
    /* Print report of no matching register */
    pCtx->cmd_printf(pCtx->pPrintfCtx, "Unknown register: %s\r\n", reg);
    return;
  }

  /* Test if a field was specified */
  pField = NULL;
  if (*field != '\0')
  {
    /* Find the field definition */
    pField = f1000_regs_find_field(pCtx, pReg, field);

    if (pField == NULL)
    {
      pCtx->cmd_printf(pCtx->pPrintfCtx, "Unknown field %s\r\n", field);
      return;
    }
  }

  /* Get the value to set the register / field to */
  if (strncasecmp(pCtx->argv[2], "0x", 2) == 0)
  {
    sscanf(&pCtx->argv[2][2], "%x", &value);
  }
  else
    value = atoi(pCtx->argv[2]);

  /* Perform the register / field set */
  f1000_regs_set_value(pCtx, pBlock, pReg, pField, value);
}

/****************************************************************************
 * f1000_regs_is_register
 *
 * Return TRUE is given name is a register
 ****************************************************************************/

int f1000_regs_is_register(f1000_regs_ctx_t *pCtx, const char *name)
{
  char              *block;
  char              *reg;
  char              *field;
  char              *pName;
  f1000_reg_block_t *pBlock;
  f1000_field_def_t *pField;
  f1000_reg_def_t   *pReg;

  /* Ensure the context is valid */
  if (pCtx == NULL)
    return 0;

  pName = malloc(strlen(name) + 1);
  strcpy(pName, name);

  /* Find pointers to register block, name and field */
  block = pName;
  reg = block;
  while (*reg != '\0' && *reg != '.')
    reg++;
  if (*reg == '.')
  {
    *reg++ = '\0';
  }

  field = reg;
  while (*field != '\0' && *field != '.')
    field++;
  if (*field == '.')
  {
    *field++ = '\0';
  }

  /* Find the register block definition being accessed */
  pBlock = f1000_regs_find_block_def(pCtx, block);
  if (pBlock == NULL)
  {
    free(pName);
    return 0;
  }

  /* If no register given, then report error */
  if (*reg == '\0')
  {
    free(pName);
    return 0;
  }

  /* Find the register definition */
  pReg = f1000_regs_find_reg_def(pCtx, reg);
  if (pReg == NULL)
  {
    free(pName);
    return 0;
  }

  /* Test if a field was specified */
  pField = NULL;
  if (*field == '\0')
  {
    return 1;
  }

  /* Find the field definition */
  pField = f1000_regs_find_field(pCtx, pReg, field);
  if (pField == NULL)
  {
    free(pName);
    return 0;
  }

  free(pName);
  return 1;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * f1000_regs_init
 ****************************************************************************/

int f1000_regs_init(client_t *pClient, f1000_regs_printf_t pPrintf, 
                    void *pPrintfCtx, int printfColors)
{
  f1000_regs_ctx_t  *pCtx;

  /* Allocate a context */
  pCtx = (f1000_regs_ctx_t *) malloc(sizeof(f1000_regs_ctx_t));
  if (pCtx == NULL)
  {
    pCtx->cmd_printf(pCtx->pPrintfCtx, "Out of memory allocating context\n");
    return ENOMEM;
  }

  pClient->pRegs = pCtx;

  /* Parse the register map file */
  pCtx->pFirstRegBlock = NULL;
  pCtx->pFirstRegDef   = NULL;
  pCtx->buflen         = 12000;
  pCtx->pClient        = pClient;
  if (pPrintf == NULL)
  {
    pCtx->cmd_printf   = &cmd_printf;
    pCtx->src_printf   = &src_printf;
    pCtx->pPrintfCtx   = pCtx;
    pCtx->printfColors = 0;
  }
  else
  {
    pCtx->cmd_printf    = pPrintf;
    pCtx->src_printf    = pPrintf;
    pCtx->pPrintfCtx    = pPrintfCtx;
    pCtx->printfColors  = printfColors;
  }

  if (f1000_regs_parse_mapfile(pCtx) != OK)
    return -1;

  return 0;
}

/****************************************************************************
 * f1000_regs_deinit
 ****************************************************************************/

int f1000_regs_deinit(client_t *pClient)
{
  f1000_regs_ctx_t  *pCtx;
  f1000_reg_block_t *pBlock;
  f1000_reg_block_t *pNext;

  pCtx = pClient->pRegs;

  /* Delete all entries in the context */

  pBlock = pCtx->pFirstRegBlock;
  while (pBlock)
  {
    pNext = (f1000_reg_block_t *) pBlock->listitem.pNext;
    free(pBlock);
    pBlock = pNext;
  }

  /* Free other memory used */
  f1000_regs_free_regdef(pCtx);
  free(pCtx->buffer);
  free(pCtx);

  return OK;
}

typedef struct rmm_iterator_s
{
  client_t          *pClient;
  f1000_regs_ctx_t  *pCtx;  
  f1000_reg_block_t *pBlock;
  f1000_reg_def_t   *pReg;
  f1000_field_def_t *pField;
  char              str[512];
} rmm_iterator_t;

/****************************************************************************
 * f1000_regs_iterate_begin
 ****************************************************************************/

void * f1000_regs_iterate_begin(client_t *pClient, const char *name)
{
  f1000_regs_ctx_t  *pCtx;
  int             len;
  int             x;
  int             c;
  int             dot_count;
  rmm_iterator_t  *pIt;

  /* Test for NULL context */
  if (pClient == NULL || pClient->pRegs == NULL)
    return NULL;

  /* Get pointer to the f1000_regs context */
  pCtx = pClient->pRegs;

  /* Allocate a new itertor */
  pIt = (rmm_iterator_t *) malloc(sizeof(rmm_iterator_t));
  pIt->pCtx     = pCtx;
  pIt->pClient  = pClient;
  pIt->pReg     = NULL;
  pIt->pField   = NULL;

  /* Check for 1st level top module */
  len = strlen(name);
  dot_count = 0;
  for (x = 0; x < len; x++)
    if (name[x] == '.')
      dot_count++;

  /* Check for empty name ... we return all top level entries */
  if (name == NULL || *name == 0 || dot_count == 0)
  {
    pIt->pBlock = pCtx->pFirstRegBlock;
    return pIt;
  }

  if (dot_count == 1)
  {
    /* Top level and register name given.  Get get the top level block name */
    for (x = 0; x < len && name[x] != '.'; x++)
    {
      pIt->str[x] = name[x];
    } 
    pIt->str[x] = 0;

    // Get first register
    f1000_regs_open_level2(pCtx, pIt->str);
    if (pCtx->pFirstRegDef == NULL)
    {
      free(pIt);
      return NULL;
    }
    pIt->pReg = pCtx->pFirstRegDef;
    return pIt;
  }
  else if (dot_count == 2)
  {
    /* Top level and register name given.  Get get the top level block name */
    for (x = 0; x < len && name[x] != '.'  ; x++)
    {
      pIt->str[x] = name[x];
    } 
    pIt->str[x] = 0;

    // Get first register
    f1000_regs_open_level2(pCtx, pIt->str);
    if (pCtx->pFirstRegDef == NULL)
    {
      free(pIt);
      return NULL;
    }

    /* Get the register name */
    for (++x, c=0; x < len && name[x] != '.'; x++)
    {
      pIt->str[c++] = name[x];
    }
    pIt->str[c] = 0;

    pIt->pReg = f1000_regs_find_reg_def(pIt->pCtx, pIt->str);
    if (pIt->pReg == NULL)
    {
      /* Register not found */
      free(pIt);
      return NULL; 
    }    
    pIt->pField = pIt->pReg->pFirstFieldDef;
    return pIt;
  }

  return NULL;
}

/****************************************************************************
 * f1000_regs_iterate_next
 ****************************************************************************/

void * f1000_regs_iterate_next(void *it)
{
  rmm_iterator_t  *pIt;

  /* Get the next register from the given iterator */
  if (it == NULL)
    return NULL;

  /* Get pointer to the f1000_regs context */
  pIt = (rmm_iterator_t *) it;

  /* Test if we are at top level */
  if (pIt->pBlock && !pIt->pReg)
  {
    // Get next block
    pIt->pBlock = (f1000_reg_block_t *) pIt->pBlock->listitem.pNext;
    if (pIt->pBlock == NULL)
    {
      free(pIt);
      it = NULL;
    }
  }
  else if (pIt->pReg && !pIt->pField)
  {
    // Get next register
    pIt->pReg = (f1000_reg_def_t *) pIt->pReg->listitem.pNext;
    if (pIt->pReg == NULL)
    {
      free(pIt);
      it = NULL;
    }
  }
  else
  {
    /* Get next field */
    pIt->pField = (f1000_field_def_t *) pIt->pField->listitem.pNext;
    if (pIt->pField == NULL)
    {
      free(pIt);
      it = NULL;
    }
  }

  return it;
}

/****************************************************************************
 * f1000_regs_iterate_name
 ****************************************************************************/

const char * f1000_regs_iterate_name(void *it)
{
  rmm_iterator_t  *pIt;

  /* Get the next register from the given iterator */
  if (it == NULL)
    return "";

  /* Get pointer to the f1000_regs context */
  pIt = (rmm_iterator_t *) it;

  /* Test for top level block */
  if (pIt->pBlock && !pIt->pReg)
  {
    /* Copy the block name to the iterator string */
    strncpy(pIt->str, pIt->pBlock->name, sizeof(pIt->str));
    strcat(pIt->str, ".");
    return pIt->str;
  }

  /* Test for register level itertor */
  else if (pIt->pReg && !pIt->pField)
  {
    strncpy(pIt->str, pIt->pReg->listitem.name, sizeof(pIt->str));
    strcat(pIt->str, ".");
    return pIt->str;
  }

  /* Test for field level itertor */
  else if (pIt->pReg && pIt->pField)
  {
    strncpy(pIt->str, pIt->pField->listitem.name, sizeof(pIt->str));
    return pIt->str;
  }

  return "";
}

// vim: ts=2 sw=2
