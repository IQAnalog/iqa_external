////////////////////////////////////////////////////////
//  Expression BNF
////////////////////////////////////////////////////////
//  expr    => cond
//  cond    => l_or  ('?' - expr? ':' - cond)?
//  l_or    => l_and ('||' - l_and)*
//  l_and   => b_or  ('&&' - b_or)*
//  b_or    => b_and ('|' - b_and)*
//  b_and   => rel   ('&' - rel)*
//  rel     => shift (
//             ('==' - shift) |
//             ('!=' - shift) |
//             ('<=' - shift) |
//             ('>=' - shift) |
//             ('<'  - shift) |
//             ('>'  - shift)
//           )?
//  shift   => add (('<<' - add) | ('>>' - add))*
//  add     => mul (('+' - mul) | ('-' - mul))*
//  mul     => unary (
//             ('*' - unary) |
//             ('/' - unary) |
//             ('%' - unary)
//           )*
//  unary   => '!' - unary
//           | '~' - unary
//           | '-' - unary
//           | postfix
//  postfix => primary (
//             ('(' - params? ')' -) |
//             ('[' - expr ']' -)
//           )*
//  params  => expr (',' - expr)*
//  primary => const | group | array | var
//  const   => float | int | str | builtin
//  group   => '(' - expr ')' -
//  array   => '[' - expr* ']' -
//           | '{' - params? '}' -
//  var     => [A-Za-z$%._][A-Za-z0-9._]* -
//  float   => '-'? ([0] | [1-9][0-9]*) ('f' | '.' [0-9]* expon? 'f'?) -
//  int     => '-'? ([1-9][0-9]* | [0][xX][0-9a-fA-F]+ | [0][0-7]*) expon? -
//  expon   => [eE] [+-]? [0-9]+
//  str     => '"' ([^"\\]+ | '\\"')* '"' -
//           | "'" ([^'\\]+ | "\\'")* "'" -
//  builtin => ("nil" | "true" | "false") -
//  comment => '#' [^\n]* '\n'
//  -       => ([\n\r ]+ | comment)*
////////////////////////////////////////////////////////

#ifndef _L_EVAL_EXPR_H_
#define _L_EVAL_EXPR_H_

#include <stdint.h>

struct eval_ctx;
struct eval_node;

typedef struct eval_node eval_node_t;
typedef struct eval_ctx eval_ctx_t;

/** String type */
typedef struct eval_str {
    const char *str;    /**< Unterminated string */
    unsigned int len;   /**< String length */
} eval_str_t;

/** Value type */
typedef enum eval_val_type {
    EVAL_VAL_NIL,       /**< Nil/null */
    EVAL_VAL_INT,       /**< Integer */
    EVAL_VAL_FLOAT,     /**< Floating-point number */
    EVAL_VAL_STR,       /**< String */
    EVAL_VAL_ARRAY,     /**< Array */
} eval_val_type_t;

/** Result value type */
typedef struct eval_result {
    eval_val_type_t type;       /**< Type of value */

    /** Union of value types */
    union {
        eval_str_t str;         /**< String */
        struct {
            /** Array list head pointer
             * Items should be iterated using the `next` pointer, and resolved
             * with @sa eval_solve_node for evaluation.
             */
            struct eval_node *head;
            unsigned int len;   /**< Array item count */
        } array;                /**< Array */
        double f_val;           /**< Floating-point value */
        int64_t i_val;          /**< Integral value */
    } u;
} eval_result_t;

/** Internal value node
 * Only usable member is `next`.  Should be resolved with @sa eval_solve_node
 * for evaluation.
 */
struct eval_node {
    int type;               /**< Private node type */
    eval_node_t *next;      /**< Pointer to next item in list */
    /** Private type union */
    union {
        struct {
            eval_node_t *cond;
            eval_node_t *t_val;
            eval_node_t *f_val;
        } cond;
        struct {
            eval_node_t *lhs;
            eval_node_t *rhs;
        } binary;
        struct {
            eval_node_t *rhs;
        } unary;
        eval_str_t var;
        struct {
            eval_node_t *head;
            unsigned int len;
        } array;
        double f_val;
        int64_t i_val;
    } u;
};

/** Context virtual pointer table.
 * All methods are optional, and can be NULL.
 */
struct eval_vptbl {
    /** Function callback method.
     * @param ctx Evaluation context.
     * @param func Function name.
     * @param params Function parameter array.
     * @param ret Output result.
     * @return 0 on success; !0 on error.
     */
    int (* call)(eval_ctx_t *ctx, const eval_str_t *func,
            const eval_result_t *params, eval_result_t *ret);

    /** Variable lookup method.
     * @param ctx Evaluation context.
     * @param key Variable name.
     * @param ret Output result.
     * @return 0 on success; !0 on error
     */
    int (* lookup)(eval_ctx_t *ctx, const eval_str_t *key, eval_result_t *ret);

    /** Variable index method.
     * @param ctx Evaluation context.
     * @param key Variable name.
     * @param idx Variable index.
     * @param ret Output result.
     * @return 0 on success; !0 on error
     */
    int (* index)(eval_ctx_t *ctx, const eval_str_t *key,
            const eval_result_t *idx, eval_result_t *ret);
};

/** Evaluation context. */
struct eval_ctx {
    const struct eval_vptbl *vptbl;     /**< Virtual pointer table */
    void *userdata;                     /**< Opaque userdata pointer */

    /** Private internal structure */
    struct {
        unsigned int splat;
        char splatbuf[512];
        eval_node_t nodes[32];
    } priv;
};

/** Solve expression.
 * Solves the provided expression, if possible.  `ctx.vptbl` should be
 * initialized.  Result depends on `ctx` and `expr` data, and any use of
 * the resulting data should avoid overwriting that data.
 * @param ctx Evaluation context.
 * @param expr Expression text.
 * @param len Expression text length; -1 for automatic strlen.
 * @param ret Output expression result.
 * @return 0 on success; expression text error position + 1 on error
 */
int eval_solve(eval_ctx_t *ctx, const char *expr, unsigned int len, eval_result_t *ret);

/** Resolve expression node.
 * For use on array results.  Result will reuse the existing data from the
 * `ctx` and `eval` * arguments to @sa eval_solve.
 * @param ctx Evaluation context.
 * @param n Evaluation node.
 * @param ret Output expression result.
 * @return 0 on success; !0 on error
 */
int eval_solve_node(eval_ctx_t *ctx, const eval_node_t *n, eval_result_t *ret);

/** Process function arguments.
 * For use on function parameter array.
 *
 * Format characters:
 *  CHARS  TYPE         NOTES
 *   i,d   int          Downcasted from int64_t
 *   I,D   int64_t
 *   f     float
 *   F     double
 *   s     eval_str_t
 *   z     char *       Null-terminated, allocated.  Release with 'free'
 *
 * Example:
 * ```c
 *     struct eval_str str;
 *     int a, b, rc;
 *     rc = eval_process_args(ctx, params, "sii", &str, &a, &b);
 *     if (rc != 3)
 *        // handle short parameter list
 * ```
 *
 * @param ctx Evaluation context.
 * @param params Parameter argument array.
 * @param fmt Parameter format string.
 * @param ... List of data pointers matching format string.
 * @return Number of arguments processed successfully
 */
int eval_process_args(eval_ctx_t *ctx,
        const eval_result_t *params, const char *fmt, ...);

/** Compare evaluation string with null-terminated string.
 * @param str Evaluation string.
 * @param match Null-terminated string.
 * @return 1 if equal; 0 if not
 */
int eval_str_eq(const eval_str_t *str, const char *match);

#endif /* _L_EVAL_EXPR_H_ */
