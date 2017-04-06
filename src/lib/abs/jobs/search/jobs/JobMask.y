/* =============================================================================
ABS Node Mask Lexical Analyzer
      Copyright (c) 2013 Petr Kulhanek, kulhanek@chemi.muni.cz
============================================================================= */

%{
#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdio.h>
#include <string.h>
#include "CommonParser.hpp"
#define YYERROR_VERBOSE
%}

%union{
    SGrammar                gValue;
    SInteger                iValue;
    SString                 sValue;
    enum SType              tValue;
    struct SSelection*      selValue;
    struct SExpression*     exprValue;
    };

/* recognized tokens -------------------------------------------------------- */
%token <sValue> STRING
%token <iValue> INUMBER
%token <gValue> ID NCPUS NGPUS NNODES TITLE QUEUE USER PATH ST BS
%token <gValue> LT LE GT GE NE EQ
%token <gValue> NOT AND OR
%token <gValue> RBRA LBRA
%token <gValue> ERROR

%type <tValue>    iselector
%type <tValue>    sselector
%type <selValue>  iselection
%type <selValue>  sselection
%type <selValue>  selection
%type <exprValue> expr
%type <exprValue> job_mask

/* priority of operators ---------------------------------------------------- */
%left OR
%left AND
%left NOT
%left LT LE GT GE NE EQ

/* defined grammar ---------------------------------------------------------- */
%%

job_mask:
    expr                { TopExpression = $1; $$ = $1; }
    ;

expr:
/* SELECTION ---------------------------------------------------------------- */

    selection {
        struct SExpression* p_expr = AllocateExpression();
        if( p_expr == NULL ){
            yyerror("unable to allocate memory for the expression");
            YYERROR;
            }
        p_expr->Selection = $1;
        $$ = p_expr;
        }

/* LOGICAL OPERATORS -------------------------------------------------------- */

    | expr AND expr {
        struct SExpression* p_expr = AllocateExpression();
        if( p_expr == NULL ){
            yyerror("unable to allocate memory for the expression");
            YYERROR;
            }
        p_expr->Operator = O_AND; 
        p_expr->LeftExpression = $1;
        p_expr->RightExpression = $3;
        $$ = p_expr;
        }
    | expr OR expr {
        struct SExpression* p_expr = AllocateExpression();
        if( p_expr == NULL ){
            yyerror("unable to allocate memory for the expression");
            YYERROR;
            }
        p_expr->Operator = O_OR;
        p_expr->LeftExpression = $1;
        p_expr->RightExpression = $3;
        $$ = p_expr;
        }
    | NOT expr {
        struct SExpression* p_expr = AllocateExpression();
        if( p_expr == NULL ){
            yyerror("unable to allocate memory for the expression");
            YYERROR;
            }
        p_expr->Operator = O_NOT;
        p_expr->LeftExpression = NULL;
        p_expr->RightExpression = $2;
        $$ = p_expr;
        }

/* BRACKETS ----------------------------------------------------------------- */

    | LBRA expr RBRA {
        $$ = $2;
        }
    ;

/* SELECTIONS * ------------------------------------------------------------- */

selection:
    iselection {
            $$ = $1;
        }
    | sselection {
            $$ = $1;
        }
    ;

iselection:
    iselector LT INUMBER {
        struct SSelection* p_selection = AllocateSelectionByNumber($1,O_LT,$3.Number);
        if( p_selection == NULL ){
            yyerror("unable to allocate memory for selection");
            YYERROR;
            }
        $$ = p_selection;
        }

    | iselector LE INUMBER {
        struct SSelection* p_selection = AllocateSelectionByNumber($1,O_LE,$3.Number);
        if( p_selection == NULL ){
            yyerror("unable to allocate memory for selection");
            YYERROR;
            }
        $$ = p_selection;
        }

    | iselector GT INUMBER {
        struct SSelection* p_selection = AllocateSelectionByNumber($1,O_GT,$3.Number);
        if( p_selection == NULL ){
            yyerror("unable to allocate memory for selection");
            YYERROR;
            }
        $$ = p_selection;
        }

    | iselector GE INUMBER {
        struct SSelection* p_selection = AllocateSelectionByNumber($1,O_GE,$3.Number);
        if( p_selection == NULL ){
            yyerror("unable to allocate memory for selection");
            YYERROR;
            }
        $$ = p_selection;
        }

    | iselector EQ INUMBER {
        struct SSelection* p_selection = AllocateSelectionByNumber($1,O_EQ,$3.Number);
        if( p_selection == NULL ){
            yyerror("unable to allocate memory for selection");
            YYERROR;
            }
        $$ = p_selection;
        }

    | iselector NE INUMBER {
        struct SSelection* p_selection = AllocateSelectionByNumber($1,O_NE,$3.Number);
        if( p_selection == NULL ){
            yyerror("unable to allocate memory for selection");
            YYERROR;
            }
        $$ = p_selection;
        }
    ;

sselection:
    sselector EQ STRING {
        struct SSelection* p_selection = AllocateSelectionByString($1,O_EQ,$3.String);
        if( p_selection == NULL ){
            yyerror("unable to allocate memory for selection");
            YYERROR;
            }
        $$ = p_selection;
        }

    | sselector NE STRING {
        struct SSelection* p_selection = AllocateSelectionByString($1,O_NE,$3.String);
        if( p_selection == NULL ){
            yyerror("unable to allocate memory for selection");
            YYERROR;
            }
        $$ = p_selection;
        }
    ;

iselector:
    ID {
        enum SType selt = T_ID;
        $$ = selt;
        }
    | NCPUS {
        enum SType selt = T_NCPUS;
        $$ = selt;
        }
    | NGPUS {
        enum SType selt = T_NGPUS;
        $$ = selt;
        }
    | NNODES {
        enum SType selt = T_NNODES;
        $$ = selt;
        }
    ;

sselector:
    TITLE {
        enum SType selt = T_TITLE;
        $$ = selt;
        }
    | QUEUE {
        enum SType selt = T_QUEUE;
        $$ = selt;
        }
    | USER {
        enum SType selt = T_USER;
        $$ = selt;
        }
    | PATH {
        enum SType selt = T_PATH;
        $$ = selt;
        }
    | ST {
        enum SType selt = T_ST;
        $$ = selt;
        }
    | BS {
        enum SType selt = T_BS;
        $$ = selt;
        }
    ;

%%
/* ========================================================================== */

int parse_job_mask(const char* p_mask)
{
    yyjob_scan_string(p_mask);
    int rvalue = yyjobparse();
    yyjoblex_destroy();
    return(rvalue);
}

