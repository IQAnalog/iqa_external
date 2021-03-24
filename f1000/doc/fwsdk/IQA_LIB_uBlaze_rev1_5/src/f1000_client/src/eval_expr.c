#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "eval_expr.h"

typedef enum eval_node_type {
    EVAL_TYPE_NIL       = EVAL_VAL_NIL,
    EVAL_TYPE_INT       = EVAL_VAL_INT,
    EVAL_TYPE_FLOAT     = EVAL_VAL_FLOAT,
    EVAL_TYPE_STR       = EVAL_VAL_STR,
    EVAL_TYPE_ARRAY     = EVAL_VAL_ARRAY,

    EVAL_TYPE_COND      = 32,
    EVAL_TYPE_COMPARE_EQ,
    EVAL_TYPE_COMPARE_NE,
    EVAL_TYPE_COMPARE_LT,
    EVAL_TYPE_COMPARE_GT,
    EVAL_TYPE_COMPARE_LE,
    EVAL_TYPE_COMPARE_GE,
    EVAL_TYPE_BINARY_AND,
    EVAL_TYPE_BINARY_OR,
    EVAL_TYPE_LOGICAL_AND,
    EVAL_TYPE_LOGICAL_OR,
    EVAL_TYPE_SHIFT_LEFT,
    EVAL_TYPE_SHIFT_RIGHT,
    EVAL_TYPE_ADD,
    EVAL_TYPE_SUB,
    EVAL_TYPE_MUL,
    EVAL_TYPE_DIV,
    EVAL_TYPE_MOD,
    EVAL_TYPE_NOT,
    EVAL_TYPE_INVERT,
    EVAL_TYPE_NEGATE,
    EVAL_TYPE_CALL,
    EVAL_TYPE_INDEX,
    EVAL_TYPE_VAR,
    EVAL_TYPE_VECTOR,
} eval_node_type_t;

typedef struct eval_parser {
    const char *data;
    unsigned int pos;
    unsigned int end;
    uint32_t alloc;
    eval_node_t *nodes;
} eval_parser_t;

typedef struct eval_save {
    uint32_t alloc;
    unsigned int pos;
} eval_save_t;

static int eval_parse_expr(eval_parser_t *psr, eval_node_t **ret);
static int eval_parse_char(eval_parser_t *psr, char ch);
static int eval_parse_params(eval_parser_t *psr, int comma,
        int *count, eval_node_t **ret);

static int eval_alloc_node(eval_parser_t *psr, eval_node_t **ret)
{
    unsigned int bit;

    bit = __builtin_ffs(psr->alloc);
    if (bit == 0)
        return psr->pos + 1;

    psr->alloc &= ~(1 << (bit - 1));

    *ret = &psr->nodes[bit - 1];
    (*ret)->type = EVAL_TYPE_NIL;
    (*ret)->next = NULL;

    return 0;
}

static void eval_save(eval_parser_t *psr, eval_save_t *save)
{
    save->alloc = psr->alloc;
    save->pos = psr->pos;
}

#define EVAL_FATAL 0x80000000

static int eval_error(eval_parser_t *psr, eval_save_t *save, int rc)
{
    psr->alloc = save->alloc;
    psr->pos = save->pos;
    return rc;
}

static int eval_fatal(eval_parser_t *psr, eval_save_t *save, int rc)
{
    return eval_error(psr, save, rc | EVAL_FATAL);
}

static int eval_isfatal(int rc)
{
    return !!(rc & EVAL_FATAL);
}

//  comment => '#' [^\n]* '\n'
static int eval_parse_comment(eval_parser_t *psr)
{
    int rc;

    rc = eval_parse_char(psr, '#');
    if (rc != 0)
        return rc;

    while (psr->pos < psr->end)
    {
        psr->pos += 1;
        if (psr->data[psr->pos - 1] == '\n')
            break;
    }

    return 0;
}

//  -       => ([\n\r ]+ | comment)*
static int eval_parse_whitespace(eval_parser_t *psr)
{
    unsigned int st = psr->pos;

    while (psr->pos < psr->end)
    {
        switch (psr->data[psr->pos])
        {
            case '\n':
            case '\r':
            case ' ':
                psr->pos += 1;
                break;

            case '#':
                eval_parse_comment(psr);
                break;

            default:
                if (psr->pos > st)
                    return 0;

                return psr->pos + 1;
        }
    }

    return psr->pos + 1;
}

static int eval_peek(eval_parser_t *psr, int *ch)
{
    if (psr->pos >= psr->end)
        return psr->pos + 1;

    *ch = psr->data[psr->pos];

    return 0;
}

static int eval_parse_char(eval_parser_t *psr, char ch)
{
    if (psr->pos >= psr->end)
        return psr->pos + 1;

    if (psr->data[psr->pos] == ch)
    {
        psr->pos += 1;
        return 0;
    }

    return psr->pos + 1;
}

static int eval_parse_char_w(eval_parser_t *psr, char ch)
{
    int rc;

    rc = eval_parse_char(psr, ch);
    if (rc == 0)
        eval_parse_whitespace(psr);

    return rc;
}

static int eval_parse_text(eval_parser_t *psr, const char *str)
{
    int c = psr->pos;
    int i = 0;

    while (c < psr->end && psr->data[c] == str[i])
    {
        i += 1;
        c += 1;
    }

    if (str[i] == 0)
    {
        psr->pos = c;
        return 0;
    }

    return psr->pos + 1;
}

static int eval_parse_text_w(eval_parser_t *psr, const char *str)
{
    int rc;

    rc = eval_parse_text(psr, str);
    if (rc == 0)
        eval_parse_whitespace(psr);

    return rc;
}

//  builtin => ("nil" | "true" | "false") -
static int eval_parse_builtin(eval_parser_t *psr, eval_node_t **ret)
{
    eval_node_type_t type;
    eval_save_t save;
    int value;
    int rc;

    eval_save(psr, &save);

    if ((rc = eval_parse_text_w(psr, "nil")) == 0)
    {
        type = EVAL_TYPE_NIL;
    }
    else if ((rc = eval_parse_text_w(psr, "false")) == 0)
    {
        type = EVAL_TYPE_INT;
        value = 0;
    }
    else if ((rc = eval_parse_text_w(psr, "true")) == 0)
    {
        type = EVAL_TYPE_INT;
        value = 1;
    }
    else
    {
        return psr->pos + 1;
    }

    rc = eval_alloc_node(psr, ret);
    if (rc != 0)
        return eval_error(psr, &save, rc);

    (*ret)->type = type;
    (*ret)->u.i_val = value;

    return 0;
}

//  str     => '"' ([^"\\]+ | '\\"')* '"' -
//           | "'" ([^'\\]+ | "\\'")* "'" -
static int eval_parse_str(eval_parser_t *psr, eval_node_t **ret)
{
    eval_save_t save;
    int qchar;
    int esc;
    int rc;
    int c;

    eval_save(psr, &save);

    qchar = '"';
    rc = eval_parse_char(psr, qchar);
    if (rc != 0)
    {
        qchar = '\'';
        rc = eval_parse_char(psr, qchar);
        if (rc != 0)
            return rc;
    }

    esc = 0;
    for (c = psr->pos; c < psr->end; ++c)
    {
        if (!esc)
        {
            if (psr->data[c] == qchar)
                break;
            else if (psr->data[c] == '\\')
                esc = 1;
        }
        else
        {
            esc = 0;
        }
    }

    if (c == psr->end)
        return eval_error(psr, &save, psr->pos + 1);

    rc = eval_alloc_node(psr, ret);
    if (rc != 0)
        return eval_error(psr, &save, rc);

    (*ret)->type = EVAL_TYPE_STR;
    (*ret)->u.var.str = psr->data + psr->pos;
    (*ret)->u.var.len = c - psr->pos;

    psr->pos = c + 1;

    eval_parse_whitespace(psr);

    return 0;
}

//  expon   => [eE] [+-] [0-9]+
//  int     => '-'? ([1-9][0-9]* | [0][xX][0-9a-fA-F]+ | [0][0-7]*) expon? -
static int eval_parse_int(eval_parser_t *psr, eval_node_t **ret)
{
    eval_save_t save;
    int64_t val;
    int hexok = 0;
    int rc;
    int ch;
    int s;

    eval_save(psr, &save);

    eval_parse_char(psr, '-');

    s = psr->pos;
    if (eval_parse_char(psr, '0') == 0)
    {
        if (eval_parse_char(psr, 'x') == 0)
            hexok = 1;
        else if (eval_parse_char(psr, 'X') == 0)
            hexok = 1;
    }

    if (hexok)
        s += 1;

    while ((rc = eval_peek(psr, &ch)) == 0)
    {
        if (!((ch >= '0' && ch <= '9') ||
              (hexok && ((ch >= 'A' && ch <= 'F') || (ch >= 'a' && ch <= 'f')))))
            break;
        psr->pos += 1;
    }

    if (eval_parse_char(psr, '.') == 0)
        return eval_error(psr, &save, psr->pos + 1);

    if (psr->pos == s)
        return eval_error(psr, &save, psr->pos + 1);

    if (eval_peek(psr, &ch) == 0 && (ch == 'e' || ch == 'E'))
    {
        psr->pos += 1;
        if (eval_peek(psr, &ch) == 0 && (ch == '-' || ch == '+'))
            psr->pos += 1;

        while ((rc = eval_peek(psr, &ch)) == 0)
        {
            if (!((ch >= '0' && ch <= '9') || (ch == '.')))
                break;
            psr->pos += 1;
        }
    }

    val = strtoll(psr->data + save.pos, NULL, 0);

    eval_parse_whitespace(psr);

    rc = eval_alloc_node(psr, ret);
    if (rc != 0)
        return eval_error(psr, &save, rc);

    (*ret)->type = EVAL_TYPE_INT;
    (*ret)->u.i_val = val;

    return 0;
}

//  float   => '-'? ([0] | [1-9][0-9]*) ('f' | '.' [0-9]* expon? 'f'?) -
static int eval_parse_float(eval_parser_t *psr, eval_node_t **ret)
{
    eval_save_t save;
    double val;
    int rc;
    int ch;
    int s;

    eval_save(psr, &save);

    eval_parse_char(psr, '-');

    s = psr->pos;

    while ((rc = eval_peek(psr, &ch)) == 0)
    {
        if (!((ch >= '0' && ch <= '9') || (ch == '.')))
            break;
        psr->pos += 1;
    }

    if (psr->pos == s)
        return eval_error(psr, &save, psr->pos + 1);

    if (eval_peek(psr, &ch) == 0 && (ch == 'e' || ch == 'E'))
    {
        psr->pos += 1;
        if (eval_peek(psr, &ch) == 0 && (ch == '-' || ch == '+'))
            psr->pos += 1;

        while ((rc = eval_peek(psr, &ch)) == 0)
        {
            if (!((ch >= '0' && ch <= '9') || (ch == '.')))
                break;
            psr->pos += 1;
        }
    }

    val = strtod(psr->data + save.pos, NULL);
    if (eval_parse_char_w(psr, 'f') != 0)
        eval_parse_whitespace(psr);

    rc = eval_alloc_node(psr, ret);
    if (rc != 0)
        return eval_error(psr, &save, rc);

    (*ret)->type = EVAL_TYPE_FLOAT;
    (*ret)->u.f_val = val;

    return 0;
}

//  var     => [A-Za-z$%._][A-Za-z0-9._]* -
static int eval_parse_var(eval_parser_t *psr, eval_node_t **ret)
{
    eval_save_t save;
    int rc;
    int ch;

    eval_save(psr, &save);

    rc = eval_peek(psr, &ch);
    if (rc != 0)
        return rc;

    if (!((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') ||
          (ch == '$' || ch == '%' || ch == '.' || ch == '_')))
        return psr->pos + 1;

    psr->pos += 1;
    while ((rc = eval_peek(psr, &ch)) == 0)
    {
        if (!((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') ||
              (ch >= '0' && ch <= '9') || (ch == '.' || ch == '_')))
            break;
        psr->pos += 1;
    }

    rc = eval_alloc_node(psr, ret);
    if (rc != 0)
        return eval_error(psr, &save, rc);

    (*ret)->type = EVAL_TYPE_VAR;
    (*ret)->u.var.str = psr->data + save.pos;
    (*ret)->u.var.len = psr->pos - save.pos;

    eval_parse_whitespace(psr);

    return 0;
}

//  array   => '[' - expr* ']' -
//           | '{' - params? '}' -
static int eval_parse_array(eval_parser_t *psr, eval_node_t **ret)
{
    eval_node_t *rhs;
    eval_save_t save;
    eval_node_type_t type;
    int len;
    int rc;

    eval_save(psr, &save);

    if (eval_parse_char_w(psr, '{') == 0)
    {
        rc = eval_parse_params(psr, 1, &len, &rhs);
        if (rc != 0)
        {
            if (eval_isfatal(rc))
                return eval_error(psr, &save, rc);
            rhs = NULL;
            len = 0;
        }

        // dangling comma
        eval_parse_char_w(psr, ',');

        rc = eval_parse_char_w(psr, '}');
        if (rc != 0)
            return eval_fatal(psr, &save, rc);

        type = EVAL_TYPE_ARRAY;
    }
    else if (eval_parse_char_w(psr, '[') == 0)
    {
        rc = eval_parse_params(psr, 0, &len, &rhs);
        if (rc != 0)
        {
            if (eval_isfatal(rc))
                return eval_error(psr, &save, rc);
            rhs = NULL;
            len = 0;
        }

        rc = eval_parse_char_w(psr, ']');
        if (rc != 0)
            return eval_fatal(psr, &save, rc);

        type = EVAL_TYPE_VECTOR;
    }
    else
    {
        return eval_error(psr, &save, psr->pos + 1);
    }

    rc = eval_alloc_node(psr, ret);
    if (rc != 0)
        return eval_error(psr, &save, rc);

    (*ret)->type = type;
    (*ret)->u.array.head = rhs;
    (*ret)->u.array.len = len;

    return 0;
}

//  group   => '(' - expr ')' -
static int eval_parse_group(eval_parser_t *psr, eval_node_t **ret)
{
    eval_save_t save;
    int rc;

    eval_save(psr, &save);

    rc = eval_parse_char_w(psr, '(');
    if (rc != 0)
        return eval_error(psr, &save, rc);

    rc = eval_parse_expr(psr, ret);
    if (rc != 0)
        return eval_fatal(psr, &save, rc);

    rc = eval_parse_char_w(psr, ')');
    if (rc != 0)
        return eval_fatal(psr, &save, rc);

    return 0;
}

//  const   => float | int | str | builtin
static int eval_parse_const(eval_parser_t *psr, eval_node_t **ret)
{
    int rc;

    if ((rc = eval_parse_int(psr, ret)) == 0)
        return 0;
    else if ((rc = eval_parse_float(psr, ret)) == 0)
        return 0;
    else if ((rc = eval_parse_str(psr, ret)) == 0)
        return 0;
    else if ((rc = eval_parse_builtin(psr, ret)) == 0)
        return 0;

    return rc;
}

//  primary => const | group | array | var
static int eval_parse_primary(eval_parser_t *psr, eval_node_t **ret)
{
    int rc;

    if ((rc = eval_parse_group(psr, ret)) == 0)
        return 0;
    else if ((rc = eval_parse_array(psr, ret)) == 0)
        return 0;
    else if ((rc = eval_parse_const(psr, ret)) == 0)
        return 0;
    else if ((rc = eval_parse_var(psr, ret)) == 0)
        return 0;

    return rc;
}

//  params  => expr (',' - expr)*
static int eval_parse_params(eval_parser_t *psr, int comma,
        int *count, eval_node_t **ret)
{
    eval_node_t *lhs;
    eval_node_t **tail;
    eval_save_t save;
    int num = 1;
    int rc;

    eval_save(psr, &save);

    rc = eval_parse_expr(psr, &lhs);
    if (rc != 0)
        return eval_error(psr, &save, rc);

    tail = &lhs->next;

    while (!comma || eval_parse_char_w(psr, ',') == 0)
    {
        eval_node_t *rhs;

        rc = eval_parse_expr(psr, &rhs);
        if (rc != 0)
        {
            if (comma || eval_isfatal(rc))
                return eval_error(psr, &save, rc);
            break;
        }

        *tail = rhs;
        tail = &rhs->next;
        num += 1;
    }

    if (count)
        *count = num;
    *tail = NULL;
    *ret = lhs;

    return 0;
}

//  postfix => primary (
//             ('(' - params? ')' -) |
//             ('[' - expr ']' -) |
//           )*
static int eval_parse_postfix(eval_parser_t *psr, eval_node_t **ret)
{
    eval_node_t *lhs;
    eval_node_t *rhs;
    eval_save_t save;
    int rc;

    eval_save(psr, &save);

    rc = eval_parse_primary(psr, &lhs);
    if (rc != 0)
        return eval_error(psr, &save, rc);

    *ret = lhs;

    if (eval_parse_char_w(psr, '(') == 0)
    {
        rc = eval_parse_params(psr, 1, NULL, &rhs);
        if (rc != 0)
        {
            if (eval_isfatal(rc))
                return eval_error(psr, &save, rc);
            rhs = NULL;
        }

        rc = eval_parse_char_w(psr, ')');
        if (rc != 0)
            return eval_fatal(psr, &save, rc);

        rc = eval_alloc_node(psr, ret);
        if (rc != 0)
            return eval_error(psr, &save, rc);

        (*ret)->type = EVAL_TYPE_CALL;
        (*ret)->u.binary.lhs = lhs;
        (*ret)->u.binary.rhs = rhs;
    }
    else if (eval_parse_char_w(psr, '[') == 0)
    {
        rc = eval_parse_expr(psr, &rhs);
        if (rc != 0)
            return eval_fatal(psr, &save, rc);

        rc = eval_parse_char_w(psr, ']');
        if (rc != 0)
            return eval_fatal(psr, &save, rc);

        rc = eval_alloc_node(psr, ret);
        if (rc != 0)
            return eval_error(psr, &save, rc);

        (*ret)->type = EVAL_TYPE_INDEX;
        (*ret)->u.binary.lhs = lhs;
        (*ret)->u.binary.rhs = rhs;
    }

    return 0;
}

//  unary   => '!' - unary
//           | '~' - unary
//           | '-' - unary
//           | postfix
static int eval_parse_unary(eval_parser_t *psr, eval_node_t **ret)
{
    eval_node_t *rhs;
    eval_save_t save;
    eval_node_type_t type;
    int rc = 0;

    eval_save(psr, &save);

    rhs = NULL;
    if (eval_parse_char_w(psr, '!') == 0)
    {
        type = EVAL_TYPE_NOT;
        rc = eval_parse_unary(psr, &rhs);
    }
    else if (eval_parse_char_w(psr, '~') == 0)
    {
        type = EVAL_TYPE_INVERT;
        rc = eval_parse_unary(psr, &rhs);
    }
    else if (eval_parse_char_w(psr, '-') == 0)
    {
        type = EVAL_TYPE_NEGATE;
        rc = eval_parse_unary(psr, &rhs);
    }

    if (rc != 0)
        return eval_error(psr, &save, rc);

    if (rhs != NULL)
    {
        rc = eval_alloc_node(psr, ret);
        if (rc != 0)
            return eval_error(psr, &save, rc);

        (*ret)->type = type;
        (*ret)->u.unary.rhs = rhs;
    }
    else
    {
        return eval_parse_postfix(psr, ret);
    }

    return 0;
}

//  mul     => unary (
//             ('*' - unary) |
//             ('/' - unary) |
//             ('%' - unary)
//           )*
static int eval_parse_mul(eval_parser_t *psr, eval_node_t **ret)
{
    eval_node_t *lhs;
    eval_save_t save;
    eval_node_type_t type;
    int rc;

    eval_save(psr, &save);

    rc = eval_parse_unary(psr, &lhs);
    if (rc != 0)
        return eval_error(psr, &save, rc);

    *ret = lhs;
    while (lhs != NULL)
    {
        eval_node_t *rhs = NULL;

        if (eval_parse_char_w(psr, '*') == 0)
        {
            type = EVAL_TYPE_MUL;
            rc = eval_parse_unary(psr, &rhs);
        }
        else if (eval_parse_char_w(psr, '/') == 0)
        {
            type = EVAL_TYPE_DIV;
            rc = eval_parse_unary(psr, &rhs);
        }
        else if (eval_parse_char_w(psr, '%') == 0)
        {
            type = EVAL_TYPE_MOD;
            rc = eval_parse_unary(psr, &rhs);
        }

        if (rc != 0)
            return eval_error(psr, &save, rc);

        if (rhs != NULL)
        {
            rc = eval_alloc_node(psr, ret);
            if (rc != 0)
                return eval_error(psr, &save, rc);

            (*ret)->type = type;
            (*ret)->u.binary.lhs = lhs;
            (*ret)->u.binary.rhs = rhs;
            lhs = *ret;
        }
        else
        {
            lhs = NULL;
        }
    }

    return 0;
}

//  add     => mul (('+' - mul) | ('-' - mul))*
static int eval_parse_add(eval_parser_t *psr, eval_node_t **ret)
{
    eval_node_t *lhs;
    eval_save_t save;
    eval_node_type_t type;
    int rc;

    eval_save(psr, &save);

    rc = eval_parse_mul(psr, &lhs);
    if (rc != 0)
        return eval_error(psr, &save, rc);

    *ret = lhs;
    while (lhs != NULL)
    {
        eval_node_t *rhs = NULL;

        if (eval_parse_char_w(psr, '+') == 0)
        {
            type = EVAL_TYPE_ADD;
            rc = eval_parse_mul(psr, &rhs);
        }
        else if (eval_parse_char_w(psr, '-') == 0)
        {
            type = EVAL_TYPE_SUB;
            rc = eval_parse_mul(psr, &rhs);
        }

        if (rc != 0)
            return eval_error(psr, &save, rc);

        if (rhs != NULL)
        {
            rc = eval_alloc_node(psr, ret);
            if (rc != 0)
                return eval_error(psr, &save, rc);

            (*ret)->type = type;
            (*ret)->u.binary.lhs = lhs;
            (*ret)->u.binary.rhs = rhs;
            lhs = *ret;
        }
        else
        {
            lhs = NULL;
        }
    }

    return 0;
}

//  shift   => add (('<<' - add) | ('>>' - add))*
static int eval_parse_shift(eval_parser_t *psr, eval_node_t **ret)
{
    eval_node_t *lhs;
    eval_save_t save;
    eval_node_type_t type;
    int rc;

    eval_save(psr, &save);

    rc = eval_parse_add(psr, &lhs);
    if (rc != 0)
        return eval_error(psr, &save, rc);

    *ret = lhs;

    while (lhs != NULL)
    {
        eval_node_t *rhs = NULL;

        if (eval_parse_text_w(psr, "<<") == 0)
        {
            type = EVAL_TYPE_SHIFT_LEFT;
            rc = eval_parse_add(psr, &rhs);
        }
        else if (eval_parse_text_w(psr, ">>") == 0)
        {
            type = EVAL_TYPE_SHIFT_RIGHT;
            rc = eval_parse_add(psr, &rhs);
        }

        if (rc != 0)
            return eval_error(psr, &save, rc);

        if (rhs != NULL)
        {
            rc = eval_alloc_node(psr, ret);
            if (rc != 0)
                return eval_error(psr, &save, rc);

            (*ret)->type = type;
            (*ret)->u.binary.lhs = lhs;
            (*ret)->u.binary.rhs = rhs;
            lhs = *ret;
        }
        else
        {
            lhs = NULL;
        }
    }

    return 0;
}

//  rel     => shift (
//             ('==' - shift) |
//             ('!=' - shift) |
//             ('<=' - shift) |
//             ('>=' - shift) |
//             ('<'  - shift) |
//             ('>'  - shift)
//           )?
static int eval_parse_relative(eval_parser_t *psr, eval_node_t **ret)
{
    eval_node_t *lhs;
    eval_node_t *rhs;
    eval_save_t save;
    eval_node_type_t type;
    int rc;

    eval_save(psr, &save);

    rc = eval_parse_shift(psr, &lhs);
    if (rc != 0)
        return eval_error(psr, &save, rc);

    *ret = lhs;
    rhs = NULL;

    if (eval_parse_text_w(psr, "==") == 0)
    {
        type = EVAL_TYPE_COMPARE_EQ;
        rc = eval_parse_shift(psr, &rhs);
    }
    else if (eval_parse_text_w(psr, "!=") == 0)
    {
        type = EVAL_TYPE_COMPARE_NE;
        rc = eval_parse_shift(psr, &rhs);
    }
    else if (eval_parse_text_w(psr, "<=") == 0)
    {
        type = EVAL_TYPE_COMPARE_LE;
        rc = eval_parse_shift(psr, &rhs);
    }
    else if (eval_parse_text_w(psr, ">=") == 0)
    {
        type = EVAL_TYPE_COMPARE_GE;
        rc = eval_parse_shift(psr, &rhs);
    }
    else if (eval_parse_text_w(psr, "<") == 0)
    {
        type = EVAL_TYPE_COMPARE_LT;
        rc = eval_parse_shift(psr, &rhs);
    }
    else if (eval_parse_text_w(psr, ">") == 0)
    {
        type = EVAL_TYPE_COMPARE_GT;
        rc = eval_parse_shift(psr, &rhs);
    }

    if (rc != 0)
        return eval_error(psr, &save, rc);

    if (rhs != NULL)
    {
        rc = eval_alloc_node(psr, ret);
        if (rc != 0)
            return eval_error(psr, &save, rc);

        (*ret)->type = type;
        (*ret)->u.binary.lhs = lhs;
        (*ret)->u.binary.rhs = rhs;
    }

    return 0;
}

//  b_and   => rel   ('&' - rel)*
static int eval_parse_binary_and(eval_parser_t *psr, eval_node_t **ret)
{
    eval_node_t *lhs;
    eval_save_t save;
    unsigned int s;
    int rc;

    eval_save(psr, &save);

    rc = eval_parse_relative(psr, &lhs);
    if (rc != 0)
        return eval_error(psr, &save, rc);

    *ret = lhs;
    s = psr->pos;

    while (eval_parse_char(psr, '&') == 0)
    {
        eval_node_t *rhs;

        if (eval_parse_char_w(psr, '&') == 0)
        {
            psr->pos = s;
            break;
        }
        eval_parse_whitespace(psr);

        rc = eval_parse_relative(psr, &rhs);
        if (rc != 0)
            return eval_error(psr, &save, rc);

        rc = eval_alloc_node(psr, ret);
        if (rc != 0)
            return eval_error(psr, &save, rc);

        (*ret)->type = EVAL_TYPE_BINARY_AND;
        (*ret)->u.binary.lhs = lhs;
        (*ret)->u.binary.rhs = rhs;
        lhs = *ret;
        s = psr->pos;
    }

    return 0;
}

//  b_or    => b_and  ('|' - b_and)*
static int eval_parse_binary_or(eval_parser_t *psr, eval_node_t **ret)
{
    eval_node_t *lhs;
    eval_save_t save;
    unsigned int s;
    int rc;

    eval_save(psr, &save);

    rc = eval_parse_binary_and(psr, &lhs);
    if (rc != 0)
        return eval_error(psr, &save, rc);

    *ret = lhs;
    s = psr->pos;

    while (eval_parse_char(psr, '|') == 0)
    {
        eval_node_t *rhs;

        if (eval_parse_char_w(psr, '|') == 0)
        {
            psr->pos = s;
            break;
        }
        eval_parse_whitespace(psr);

        rc = eval_parse_binary_and(psr, &rhs);
        if (rc != 0)
            return eval_error(psr, &save, rc);

        rc = eval_alloc_node(psr, ret);
        if (rc != 0)
            return eval_error(psr, &save, rc);

        (*ret)->type = EVAL_TYPE_BINARY_OR;
        (*ret)->u.binary.lhs = lhs;
        (*ret)->u.binary.rhs = rhs;
        lhs = *ret;
        s = psr->pos;
    }

    return 0;
}

//  l_and   => b_or  ('&&' - b_or)*
static int eval_parse_logical_and(eval_parser_t *psr, eval_node_t **ret)
{
    eval_node_t *lhs;
    eval_save_t save;
    int rc;

    eval_save(psr, &save);

    rc = eval_parse_binary_or(psr, &lhs);
    if (rc != 0)
        return eval_error(psr, &save, rc);

    *ret = lhs;

    while (eval_parse_text_w(psr, "&&") == 0)
    {
        eval_node_t *rhs;

        rc = eval_parse_binary_or(psr, &rhs);
        if (rc != 0)
            return eval_error(psr, &save, rc);

        rc = eval_alloc_node(psr, ret);
        if (rc != 0)
            return eval_error(psr, &save, rc);

        (*ret)->type = EVAL_TYPE_LOGICAL_AND;
        (*ret)->u.binary.lhs = lhs;
        (*ret)->u.binary.rhs = rhs;
        lhs = *ret;
    }

    return 0;
}

//  l_or    => l_and ('||' - l_and)*
static int eval_parse_logical_or(eval_parser_t *psr, eval_node_t **ret)
{
    eval_node_t *lhs;
    eval_save_t save;
    int rc;

    eval_save(psr, &save);

    rc = eval_parse_logical_and(psr, &lhs);
    if (rc != 0)
        return eval_error(psr, &save, rc);

    *ret = lhs;

    while (eval_parse_text_w(psr, "||") == 0)
    {
        eval_node_t *rhs;

        rc = eval_parse_logical_and(psr, &rhs);
        if (rc != 0)
            return eval_error(psr, &save, rc);

        rc = eval_alloc_node(psr, ret);
        if (rc != 0)
            return eval_error(psr, &save, rc);

        (*ret)->type = EVAL_TYPE_LOGICAL_OR;
        (*ret)->u.binary.lhs = lhs;
        (*ret)->u.binary.rhs = rhs;
        lhs = *ret;
    }

    return 0;
}

//  cond    => l_or ('?' - expr? ':' - cond)?
static int eval_parse_cond(eval_parser_t *psr, eval_node_t **ret)
{
    eval_node_t *cond;
    eval_save_t save;
    int rc;

    eval_save(psr, &save);

    rc = eval_parse_logical_or(psr, &cond);
    if (rc != 0)
        return eval_error(psr, &save, rc);

    *ret = cond;

    if (eval_parse_char_w(psr, '?') == 0)
    {
        eval_node_t *t_val;
        eval_node_t *f_val;

        rc = eval_parse_expr(psr, &t_val);
        if (rc != 0)
        {
            if (eval_isfatal(rc))
                return eval_error(psr, &save, rc);
            t_val = cond;
        }

        rc = eval_parse_cond(psr, &f_val);
        if (rc != 0)
            return eval_error(psr, &save, rc);

        rc = eval_alloc_node(psr, ret);
        if (rc != 0)
            return eval_error(psr, &save, rc);

        (*ret)->type = EVAL_TYPE_COND;
        (*ret)->u.cond.cond = cond;
        (*ret)->u.cond.t_val = t_val;
        (*ret)->u.cond.t_val = f_val;
    }

    return 0;
}

//  expr    => cond
static int eval_parse_expr(eval_parser_t *psr, eval_node_t **ret)
{
    return eval_parse_cond(psr, ret);
}

static int eval_compare(eval_ctx_t *ctx, const eval_result_t *lhs, const eval_result_t *rhs, int *res)
{
    int rc;
    int r;

    switch (lhs->type)
    {
    case EVAL_TYPE_FLOAT:
        if (lhs->u.f_val > rhs->u.f_val)
            *res = 1;
        else if (lhs->u.f_val < rhs->u.f_val)
            *res = -1;
        else
            *res = 0;
        return 0;

    case EVAL_TYPE_INT:
        *res = lhs->u.i_val - rhs->u.i_val;
        return 0;

    case EVAL_TYPE_NIL:
        *res = 0;
        return 0;

    case EVAL_TYPE_STR:
        r = lhs->u.str.len;
        if (rhs->u.str.len < r)
            r = rhs->u.str.len;

        *res = strncmp(lhs->u.str.str, rhs->u.str.str, r);
        if (*res == 0)
            *res = lhs->u.str.len - rhs->u.str.len;

        return 0;

    case EVAL_TYPE_ARRAY:
        {
            eval_node_t *ln = lhs->u.array.head;
            eval_node_t *rn = rhs->u.array.head;
            eval_result_t xlhs;
            eval_result_t xrhs;
            while (ln && rn)
            {
                rc = eval_solve_node(ctx, ln, &xlhs);
                if (rc != 0)
                    return rc;
                rc = eval_solve_node(ctx, rn, &xrhs);
                if (rc != 0)
                    return rc;
                rc = eval_compare(ctx, &xlhs, &xrhs, res);
                if (rc != 0)
                    return rc;
                if (*res != 0)
                    return 0;
            }
            if (ln)
                *res = 1;
            else if (rn)
                *res = -1;
            else
                *res = 0;
        }
        return 0;

    default:
        return -1;
    }
}

static int eval_simple_integer(int64_t val, eval_result_t *ret)
{
    ret->type = EVAL_VAL_INT;
    ret->u.i_val = val;
    return 0;
}

static int eval_simple_float(double val, eval_result_t *ret)
{
    ret->type = EVAL_VAL_FLOAT;
    ret->u.f_val = val;
    return 0;
}

static int eval_boolean(const eval_result_t *n)
{
    switch (n->type)
    {
    case EVAL_VAL_FLOAT:
        return n->u.f_val != 0;

    case EVAL_VAL_INT:
        return n->u.i_val != 0;

    case EVAL_VAL_NIL:
        return 0;

    case EVAL_VAL_STR:
        return n->u.str.len != 0;

    case EVAL_VAL_ARRAY:
        return n->u.array.len != 0;

    default:
        return 0;
    }
}

static int eval_splat(eval_ctx_t *ctx, const eval_result_t *n)
{
    int len;

    if (n->type != EVAL_VAL_STR)
        return -1;

    len = n->u.str.len;
    if (len > sizeof(ctx->priv.splatbuf) - ctx->priv.splat)
        len = sizeof(ctx->priv.splatbuf) - ctx->priv.splat;

    memcpy(ctx->priv.splatbuf + ctx->priv.splat, n->u.str.str, len);
    ctx->priv.splat += len;

    return 0;
}

int eval_solve_node(eval_ctx_t *ctx, const eval_node_t *n, eval_result_t *ret)
{
    const eval_node_t *p;
    eval_result_t lhs;
    eval_result_t rhs;
    int rc;
    int r;

    switch (n->type)
    {
    case EVAL_TYPE_COND:
    case EVAL_TYPE_VECTOR:
    case EVAL_TYPE_VAR:
    case EVAL_TYPE_CALL:
        // delayed evaluation
        break;

    case EVAL_TYPE_LOGICAL_AND:
    case EVAL_TYPE_LOGICAL_OR:
        rc = eval_solve_node(ctx, n->u.binary.lhs, &lhs);
        if (rc != 0)
            return rc;

        // rhs delayed eval
        break;

    case EVAL_TYPE_COMPARE_EQ:
    case EVAL_TYPE_COMPARE_NE:
    case EVAL_TYPE_COMPARE_LT:
    case EVAL_TYPE_COMPARE_GT:
    case EVAL_TYPE_COMPARE_LE:
    case EVAL_TYPE_COMPARE_GE:
    case EVAL_TYPE_BINARY_AND:
    case EVAL_TYPE_BINARY_OR:
    case EVAL_TYPE_SHIFT_LEFT:
    case EVAL_TYPE_SHIFT_RIGHT:
    case EVAL_TYPE_ADD:
    case EVAL_TYPE_SUB:
    case EVAL_TYPE_MUL:
    case EVAL_TYPE_DIV:
    case EVAL_TYPE_MOD:
        rc = eval_solve_node(ctx, n->u.binary.lhs, &lhs);
        if (rc != 0)
            return rc;

        rc = eval_solve_node(ctx, n->u.binary.rhs, &rhs);
        if (rc != 0)
            return rc;

        if (lhs.type != rhs.type)
        {
            if (lhs.type == EVAL_VAL_FLOAT && rhs.type == EVAL_VAL_INT)
            {
                rhs.type = EVAL_VAL_FLOAT;
                rhs.u.f_val = rhs.u.i_val;
            }
            else if (rhs.type == EVAL_VAL_FLOAT && lhs.type == EVAL_VAL_INT)
            {
                lhs.type = EVAL_VAL_FLOAT;
                lhs.u.f_val = rhs.u.i_val;
            }
            else
            {
                return -1;
            }
        }
        break;

    case EVAL_TYPE_INDEX:
        rc = eval_solve_node(ctx, n->u.binary.rhs, &rhs);
        if (rc != 0)
            return rc;
        break;

    case EVAL_TYPE_NOT:
    case EVAL_TYPE_INVERT:
    case EVAL_TYPE_NEGATE:
        rc = eval_solve_node(ctx, n->u.unary.rhs, &rhs);
        if (rc != 0)
            return rc;
        break;

    case EVAL_TYPE_FLOAT:
        ret->type = n->type;
        ret->u.f_val = n->u.f_val;
        return 0;

    case EVAL_TYPE_INT:
        ret->type = n->type;
        ret->u.i_val = n->u.i_val;
        return 0;

    case EVAL_TYPE_NIL:
        ret->type = n->type;
        return 0;

    case EVAL_TYPE_STR:
        ret->type = n->type;
        ret->u.str = n->u.var;
        return 0;

    case EVAL_TYPE_ARRAY:
        ret->type = n->type;
        ret->u.array.head = n->u.array.head;
        ret->u.array.len = n->u.array.len;
        return 0;
    }

    switch (n->type)
    {
    case EVAL_TYPE_COND:
        rc = eval_solve_node(ctx, n->u.cond.cond, &lhs);
        if (rc != 0)
            return rc;

        if (eval_boolean(&lhs))
            return eval_solve_node(ctx, n->u.cond.t_val, ret);

        return eval_solve_node(ctx, n->u.cond.f_val, ret);

    case EVAL_TYPE_COMPARE_EQ:
        rc = eval_compare(ctx, &lhs, &rhs, &r);
        return (rc != 0) ? rc : eval_simple_integer(r == 0, ret);

    case EVAL_TYPE_COMPARE_NE:
        rc = eval_compare(ctx, &lhs, &rhs, &r);
        return (rc != 0) ? rc : eval_simple_integer(r != 0, ret);

    case EVAL_TYPE_COMPARE_LT:
        rc = eval_compare(ctx, &lhs, &rhs, &r);
        return (rc != 0) ? rc : eval_simple_integer(r < 0, ret);

    case EVAL_TYPE_COMPARE_GT:
        rc = eval_compare(ctx, &lhs, &rhs, &r);
        return (rc != 0) ? rc : eval_simple_integer(r > 0, ret);

    case EVAL_TYPE_COMPARE_LE:
        rc = eval_compare(ctx, &lhs, &rhs, &r);
        return (rc != 0) ? rc : eval_simple_integer(r <= 0, ret);

    case EVAL_TYPE_COMPARE_GE:
        rc = eval_compare(ctx, &lhs, &rhs, &r);
        return (rc != 0) ? rc : eval_simple_integer(r >= 0, ret);

    case EVAL_TYPE_BINARY_AND:
        if (lhs.type != EVAL_VAL_INT)
            return -1;
        return eval_simple_integer(lhs.u.i_val & rhs.u.i_val, ret);

    case EVAL_TYPE_BINARY_OR:
        if (lhs.type != EVAL_VAL_INT)
            return -1;
        return eval_simple_integer(lhs.u.i_val | rhs.u.i_val, ret);

    case EVAL_TYPE_LOGICAL_AND:
        if (!eval_boolean(&lhs))
            return eval_simple_integer(0, ret);

        rc = eval_solve_node(ctx, n->u.binary.rhs, &rhs);
        if (rc != 0)
            return rc;

        return eval_simple_integer(eval_boolean(&rhs), ret);

    case EVAL_TYPE_LOGICAL_OR:
        if (eval_boolean(&lhs))
            return eval_simple_integer(1, ret);

        rc = eval_solve_node(ctx, n->u.binary.rhs, &rhs);
        if (rc != 0)
            return rc;

        return eval_simple_integer(eval_boolean(&rhs), ret);

    case EVAL_TYPE_SHIFT_LEFT:
        if (lhs.type != EVAL_VAL_INT)
            return -1;
        return eval_simple_integer(lhs.u.i_val << rhs.u.i_val, ret);

    case EVAL_TYPE_SHIFT_RIGHT:
        if (lhs.type != EVAL_VAL_INT)
            return -1;
        return eval_simple_integer(lhs.u.i_val >> rhs.u.i_val, ret);

    case EVAL_TYPE_ADD:
        switch (lhs.type)
        {
        case EVAL_VAL_INT:
            return eval_simple_integer(lhs.u.i_val + rhs.u.i_val, ret);
        case EVAL_VAL_FLOAT:
            return eval_simple_float(lhs.u.f_val + rhs.u.f_val, ret);
        case EVAL_VAL_STR:
            r = ctx->priv.splat;
            eval_splat(ctx, &lhs);
            eval_splat(ctx, &rhs);
            ret->type = EVAL_VAL_STR;
            ret->u.str.str = ctx->priv.splatbuf + r;
            ret->u.str.len = ctx->priv.splat - r;
            return 0;
        case EVAL_VAL_ARRAY:
            return -1;
        default:
            return -1;
        }
        break;

    case EVAL_TYPE_SUB:
        switch (lhs.type)
        {
        case EVAL_VAL_INT:
            return eval_simple_integer(lhs.u.i_val - rhs.u.i_val, ret);
        case EVAL_VAL_FLOAT:
            return eval_simple_float(lhs.u.f_val - rhs.u.f_val, ret);
        default:
            return -1;
        }
        break;

    case EVAL_TYPE_MUL:
        switch (lhs.type)
        {
        case EVAL_VAL_INT:
            return eval_simple_integer(lhs.u.i_val * rhs.u.i_val, ret);
        case EVAL_VAL_FLOAT:
            return eval_simple_float(lhs.u.f_val * rhs.u.f_val, ret);
        default:
            return -1;
        }
        break;

    case EVAL_TYPE_DIV:
        switch (lhs.type)
        {
        case EVAL_VAL_INT:
            return eval_simple_integer(lhs.u.i_val / rhs.u.i_val, ret);
        case EVAL_VAL_FLOAT:
            return eval_simple_float(lhs.u.f_val / rhs.u.f_val, ret);
        default:
            return -1;
        }
        break;

    case EVAL_TYPE_MOD:
        switch (lhs.type)
        {
        case EVAL_VAL_INT:
            return eval_simple_integer(lhs.u.i_val % rhs.u.i_val, ret);
        case EVAL_VAL_FLOAT:
            ret->type = EVAL_VAL_FLOAT;
            ret->u.f_val = lhs.u.f_val - (int64_t)(lhs.u.f_val - rhs.u.f_val) * rhs.u.f_val;
            return 0;
        default:
            return -1;
        }
        break;

    case EVAL_TYPE_NOT:
        return eval_simple_integer(!eval_boolean(&rhs), ret);

    case EVAL_TYPE_INVERT:
        if (rhs.type != EVAL_VAL_INT)
            return -1;
        return eval_simple_integer(~rhs.u.i_val, ret);

    case EVAL_TYPE_NEGATE:
        switch (rhs.type)
        {
        case EVAL_TYPE_INT:
            return eval_simple_integer(-rhs.u.i_val, ret);
        case EVAL_TYPE_FLOAT:
            return eval_simple_float(-rhs.u.f_val, ret);
        default:
            return -1;
        }
        break;

    case EVAL_TYPE_CALL:
        if (ctx->vptbl == NULL || ctx->vptbl->call == NULL)
            return -1;

        if (n->u.binary.lhs->type != EVAL_TYPE_VAR)
            return -1;

        lhs.type = EVAL_VAL_STR;
        lhs.u.str = n->u.binary.lhs->u.var;

        rhs.type = EVAL_VAL_ARRAY;
        rhs.u.array.head = n->u.binary.rhs;
        rhs.u.array.len = 0;
        p = n->u.binary.rhs;

        for (; p != NULL; p = p->next)
            rhs.u.array.len += 1;

        return ctx->vptbl->call(ctx, &lhs.u.str, &rhs, ret);

    case EVAL_TYPE_INDEX:
        // If this is a var index, and the index method is implemented, use it
        if ((n->u.binary.lhs->type == EVAL_TYPE_VAR) &&
            (ctx->vptbl != NULL && ctx->vptbl->index != NULL))
        {
            lhs.type = EVAL_VAL_STR;
            lhs.u.str = n->u.binary.lhs->u.var;

            return ctx->vptbl->index(ctx, &lhs.u.str, &rhs, ret);
        }

        // No such luck, see if we can index a string or array
        rc = eval_solve_node(ctx, n->u.binary.lhs, &lhs);
        if (rc != 0)
            return rc;

        if (rhs.type != EVAL_VAL_INT)
            return -1;

        switch (lhs.type)
        {
        case EVAL_TYPE_STR:
            r = rhs.u.i_val;
            if (r < 0)
                r = lhs.u.str.len + r;
            if (r < 0 || r > lhs.u.str.len)
                return -1;
            return eval_simple_integer(lhs.u.str.str[r], ret);
        case EVAL_TYPE_ARRAY:
            r = rhs.u.i_val;
            if (r < 0)
                r = lhs.u.array.len + r;
            if (r < 0 || r > lhs.u.array.len)
                return -1;
            p = lhs.u.array.head;
            for (; r > 0; --r)
                p = p->next;
            return eval_solve_node(ctx, p, ret);
        default:
            return -1;
        }
        break;

    case EVAL_TYPE_VAR:
        if (ctx->vptbl == NULL || ctx->vptbl->lookup == NULL)
            return -1;
        lhs.type = EVAL_VAL_STR;
        lhs.u.str = n->u.var;
        return ctx->vptbl->lookup(ctx, &lhs.u.str, ret);

    case EVAL_TYPE_VECTOR:
        if (n->u.array.len == 0)
        {
            ret->type = EVAL_VAL_ARRAY;
            ret->u.array.len = 0;
            ret->u.array.head = NULL;
            return 0;
        }

        r = ctx->priv.splat;
        p = n->u.array.head;
        for (; p != NULL; p = p->next)
        {
            int save_splat = ctx->priv.splat;
            rc = eval_solve_node(ctx, p, &rhs);
            if (rc != 0)
                return rc;
            ctx->priv.splat = save_splat;
            if (rhs.type == EVAL_VAL_STR)
                eval_splat(ctx, &rhs);
        }

        if (r == ctx->priv.splat)
        {
            ret->type = EVAL_VAL_ARRAY;
            ret->u.array.head = n->u.array.head;
            ret->u.array.len = n->u.array.len;
            return 0;
        }
        else
        {
            ret->type = EVAL_VAL_STR;
            ret->u.str.str = ctx->priv.splatbuf + r;
            ret->u.str.len = ctx->priv.splat - r;
        }
        return 0;

    case EVAL_TYPE_FLOAT:
    case EVAL_TYPE_INT:
    case EVAL_TYPE_NIL:
    case EVAL_TYPE_STR:
    case EVAL_TYPE_ARRAY:
        // Already handled
        return -1;
    }

    return -1;
}

int eval_solve(eval_ctx_t *ctx, const char *expr, unsigned int len, eval_result_t *ret)
{
    eval_parser_t psr;
    eval_node_t *node;
    int rc;

    if (len == (unsigned int)-1)
        len = strlen(expr);

    ctx->priv.splat = 0;

    psr.data = expr;
    psr.end = len;
    psr.pos = 0;
    psr.alloc = 0xffffffff;
    psr.nodes = ctx->priv.nodes;

    rc = eval_parse_expr(&psr, &node);
    rc &= ~EVAL_FATAL;
    if (rc != 0)
        return rc;

    if (psr.pos != psr.end)
        return psr.pos + 1;

    rc = eval_solve_node(ctx, node, ret);
    if (rc != 0)
        return 1;

    return 0;
}

int eval_process_args(eval_ctx_t *ctx,
        const eval_result_t *params, const char *fmt, ...)
{
    eval_node_t *param;
    eval_result_t temp;
    va_list args;
    int count = 0;
    char *str;
    int ch;
    int rc;

    param = params->u.array.head;

    va_start(args, fmt);
    for (; (ch = *fmt) != 0; ++fmt)
    {
        if (param == NULL)
            goto error_short;

        rc = eval_solve_node(ctx, param, &temp);
        if (rc != 0)
            goto error_short;

        switch (ch)
        {
        case 'i':
        case 'd':
            if (temp.type != EVAL_VAL_INT)
                goto error_short;
            *va_arg(args, int *) = temp.u.i_val;
            break;
        case 'I':
        case 'D':
            if (temp.type != EVAL_VAL_INT)
                goto error_short;
            *va_arg(args, int64_t *) = temp.u.i_val;
            break;
        case 'f':
            if (temp.type == EVAL_VAL_FLOAT)
                *va_arg(args, float *) = temp.u.f_val;
            else if (temp.type == EVAL_VAL_INT)
                *va_arg(args, float *) = temp.u.i_val;
            else
                goto error_short;
            break;
        case 'F':
            if (temp.type == EVAL_VAL_FLOAT)
                *va_arg(args, double *) = temp.u.f_val;
            else if (temp.type == EVAL_VAL_INT)
                *va_arg(args, double *) = temp.u.i_val;
            else
                goto error_short;
            break;
        case 's':
            if (temp.type != EVAL_VAL_STR)
                goto error_short;
            *va_arg(args, eval_str_t *) = temp.u.str;
            break;
        case 'z':
            if (temp.type != EVAL_VAL_STR)
                goto error_short;
            str = malloc(1 + temp.u.str.len);
            if (str == NULL)
                goto error_short;
            memcpy(str, temp.u.str.str, temp.u.str.len);
            str[temp.u.str.len] = 0;
            *va_arg(args, char **) = str;
            break;
        }

        count += 1;
        param = param->next;
    }
error_short:
    va_end(args);

    return count;
}

int eval_str_eq(const eval_str_t *str, const char *match)
{
    return strncmp(str->str, match, str->len) == 0;
}

#ifdef EVAL_TEST_IMPL

#include <stdio.h>
#include <readline/readline.h>

void print_node(eval_ctx_t *ctx, const eval_result_t *n, int term)
{
    eval_node_t *node;
    eval_result_t res;
    char buf[512];
    char *E = term ? "\n" : ",";
    char *T;

    switch (n->type)
    {
    case EVAL_VAL_FLOAT:
        T = term ? "F:" : "";
        printf("%s %lg%s", T, n->u.f_val, E);
        break;
    case EVAL_VAL_INT:
        T = term ? "I:" : "";
        printf("%s %ld%s", T, n->u.i_val, E);
        break;
    case EVAL_VAL_NIL:
        T = term ? "N:" : "";
        printf("%s nil%s", T, E);
        break;
    case EVAL_VAL_STR:
        T = term ? "S:" : "";
        memcpy(buf, n->u.str.str, n->u.str.len);
        buf[n->u.str.len] = 0;
        printf("%s \"%s\"%s", T, buf, E);
        break;
    case EVAL_VAL_ARRAY:
        T = term ? "A:" : "";
        printf("%s [%d]{ ", T, n->u.array.len);

        for (node = n->u.array.head; node != NULL; node = node->next)
        {
            eval_solve_node(ctx, node, &res);
            print_node(ctx, &res, 0);
        }

        printf(" }%s", E);
        break;
    }

}

static int env_call(eval_ctx_t *ctx,
        const eval_str_t *func,
        const eval_result_t *params, eval_result_t *ret)
{
    int rc;

    if (eval_str_eq(func, "sub"))
    {
        eval_str_t str;
        int start;
        int end;

        rc = eval_process_args(ctx, params, "sii", &str, &start, &end);
        if (rc != 3)
            return -1;

        if (start < 0)
            start = 0;
        if (end < 0)
            end = 0;
        if (end > str.len)
            end = str.len;
        if (start > end)
            start = end;

        ret->type = EVAL_VAL_STR;
        ret->u.str.str = str.str + start;
        ret->u.str.len = end - start;
        return 0;
    }
    else if (eval_str_eq(func, "atoi"))
    {
        char *str;

        rc = eval_process_args(ctx, params, "z", &str);
        if (rc != 1)
            return -1;

        ret->type = EVAL_VAL_INT;
        ret->u.i_val = strtol(str, 0, 0);
	    free(str);
        return 0;
    }

    return -1;
}

static int env_lookup(eval_ctx_t *ctx, const eval_str_t *key,
        eval_result_t *res)
{
    char buf[512];
    char *env;

    memcpy(buf, key->str, key->len);
    buf[key->len] = 0;

    env = getenv(buf);
    if (env == NULL)
        return -1;

    res->type = EVAL_VAL_STR;
    res->u.str.str = env;
    res->u.str.len = strlen(env);

    return 0;
}

static const struct eval_vptbl env_vptbl = {
    env_call,
    env_lookup,
};

int main(int argc, char **argv)
{
    eval_result_t res;
    eval_ctx_t ctx;
    char *line;
    int rc;
    int i;

    ctx.vptbl = &env_vptbl;

    while ((line = readline("E> ")) != NULL)
    {
        rc = eval_solve(&ctx, line, strlen(line), &res);
        if (rc != 0)
        {
            printf("   ");
            for (i = 0; i < rc - 1; ++i)
                printf(" ");
            printf("^\n");
            printf("unable to solve expression\n");
            continue;
        }
        print_node(&ctx, &res, 1);
    }

    return 0;
}

#endif /* EVAL_TEST_IMPL */

// vim: sw=4 ts=4 et
