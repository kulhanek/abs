#ifndef JobParserH
#define JobParserH
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

enum SJobOperator {
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
} SJobGrammar;

typedef struct {
    int         Position;
    int         Number;
} SJobInteger;

typedef struct {
    int         Position;
    char        String[100];
} SJobString;

/* parser types ------------------------------------------------------------- */

enum SJobType {
    T_NCPUS,
    T_NFREECPUS,
    T_NGPUS,
    T_NFREEGPUS,
    T_NAME,
    T_PROPS,
    T_STATE
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

    /* parse mask */
    extern int parse_mask(const char* p_mask);

    /* free data associated with parsed mask */
    extern int free_mask_tree(void);

    /* get top node of mask expression */
    extern struct SExpression* get_expression_tree(void);

    /* print mask expression */
    extern int print_expression_tree(struct SExpression* p_expr);

    /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

    extern int yyerror(const char* p_error);
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
