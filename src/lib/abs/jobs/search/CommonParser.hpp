#ifndef CommonParserH
#define CommonParserH
/*
// =============================================================================
// ABS - Advanced Batch System
// -----------------------------------------------------------------------------
//    Copyright (C) 2013      Petr Kulhanek, kulhanek@chemi.muni.cz
//
//     This program is free software; you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation; either version 2 of the License, or
//     (at your option) any later version.
//
//     This program is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU General Public License along
//     with this program; if not, write to the Free Software Foundation, Inc.,
//     51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
// =============================================================================
*/

/* operators ---------------------------------------------------------------- */

enum SOperator {
    O_NONE,
    O_NOT,
    O_AND,
    O_OR,
    O_LT,
    O_LE,
    O_GT,
    O_GE,
    O_EQ,
    O_NE
};

/* object types ------------------------------------------------------------- */

typedef struct {
    int         Position;
} SGrammar;

typedef struct {
    int         Position;
    int         Number;
} SInteger;

typedef struct {
    int         Position;
    char        String[100];
} SString;

/* parser types ------------------------------------------------------------- */

enum SType {
    /* nodes -------- */
    T_NCPUS,
    T_NFREECPUS,
    T_NGPUS,
    T_NFREEGPUS,
    T_MEM,
    T_FREEMEM,
    T_GPUMEM,
    T_NAME,
    T_PROPS,
    T_STATE,
    /* jobs -------- */
    T_ID,
    T_TITLE,
    T_QUEUE,
    T_NNODES,
    T_USER,
    T_PATH,
    T_ST,
    T_BS
};

struct SSelection {
    enum SType          Type;
    enum SOperator      Operator;
    int                 IValue;
    char                SValue[100];
};

struct SExpression {
    enum SOperator          Operator;
    struct SSelection*      Selection;
    struct SExpression*     LeftExpression;
    struct SExpression*     RightExpression;
};

/* global variables --------------------------------------------------------- */
extern int                 LexPosition;        /* position in a mask during lexical analysis */
extern struct SExpression* TopExpression;      /* top expression to be evaluated */

/* global functions --------------------------------------------------------- */
#ifdef __cplusplus
extern "C" {
#endif

    /* init mask parser for new job */
    extern int init_mask(void);

    /* parse job mask */
    extern int parse_job_mask(const char* p_mask);

    /* parse node mask */
    extern int parse_node_mask(const char* p_mask);

    /* free data associated with parsed mask */
    extern int free_mask_tree(void);

    /* get top node of mask expression */
    extern struct SExpression* get_expression_tree(void);

    /* print mask expression */
    extern int print_expression_tree(struct SExpression* p_expr);

    /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

    extern int yyjoberror(const char* p_error);
    extern int yynodeerror(const char* p_error);
    extern int pperror(const char* p_error,int position);

    /* helper functions */
    extern struct SSelection*  AllocateSelectionByString(enum SType type,enum SOperator op,char* str);
    extern struct SSelection*  AllocateSelectionByNumber(enum SType type,enum SOperator op,int num);

    extern struct SExpression* AllocateExpression(void);

    /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#ifdef __cplusplus
}
#endif

#endif
