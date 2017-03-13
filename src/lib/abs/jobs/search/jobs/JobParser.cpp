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

#include "JobParser.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <ErrorSystem.hpp>

using namespace std;

//------------------------------------------------------------------------------

int                 LexPosition     = 0;
struct SExpression* TopExpression   = NULL;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// track info about allocation
vector<SSelection*>     SelectionAllocations;
vector<SExpression*>    ExpressionAllocations;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

bool print_expression(FILE* p_fout,struct SExpression* p_expr);
bool print_selection(FILE* p_fout,struct SSelection* p_sel);
bool print_list(FILE* p_fout,struct SList* p_list);

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

// int main(void)
// {
//  init_mask();
//  parse_mask("(ncpus == 5)&&(props != cl_cluster)||(props == hyper*)");
//  print_expression_tree(get_expression_tree());
//  free_mask_tree();
//  ErrorSystem.PrintErrors();
// }

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

int init_job_mask(void)
{
    free_job_mask_tree();
    JobLexPosition = 1;
    TopJobExpression = NULL;
    return(0);
}

//------------------------------------------------------------------------------

struct SExpression* get_job_expression_tree(void) {
    return(TopJobExpression);
}

//------------------------------------------------------------------------------

int free_job_mask_tree(void)
{
    for(unsigned int i=0; i < SelectionAllocations.size(); i++) {
        delete SelectionAllocations[i];
    }
    SelectionAllocations.clear();

    for(unsigned int i=0; i < ExpressionAllocations.size(); i++) {
        delete ExpressionAllocations[i];
    }
    ExpressionAllocations.clear();

    LexPosition = 0;
    TopExpression = NULL;

    return(0);
}

//------------------------------------------------------------------------------

int print_job_expression_tree(struct SExpression* p_expr)
{
    if(print_expression(stdout,p_expr) != true) return(-1);
    return(0);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool print_job_expression(FILE* p_fout,struct SExpression* p_expr)
{
    if(p_expr == NULL) {
        fprintf(p_fout,"<- NULL expr");
        return(false);
    }

    fprintf(p_fout,"EXP(");
    if(p_expr->Selection != NULL) {
        if(print_selection(p_fout,p_expr->Selection) == false) return(false);
    } else {
        switch(p_expr->Operator) {
            case O_AND:
                if(print_expression(p_fout,p_expr->LeftExpression) == false) return(false);
                fprintf(p_fout," && ");
                if(print_expression(p_fout,p_expr->RightExpression) == false) return(false);
                break;
            case O_OR:
                if(print_expression(p_fout,p_expr->LeftExpression) == false) return(false);
                fprintf(p_fout," || ");
                if(print_expression(p_fout,p_expr->RightExpression) == false) return(false);
                break;
            case O_NOT:
                fprintf(p_fout,"! ");
                if(print_expression(p_fout,p_expr->RightExpression) == false) return(false);
                break;                

            default:
                fprintf(p_fout,"<- unknown operator");
                return(false);
        };
    }
    fprintf(p_fout,")");
    return(true);
}

//------------------------------------------------------------------------------

bool print_job_selection(FILE* p_fout,struct SSelection* p_sel)
{
    if(p_sel == NULL) {
        fprintf(p_fout,"<- NULL selection");
        return(false);
    }

    switch(p_sel->Type) {
        case T_NCPUS:
            fprintf(p_fout,"(ncpus");
            break;
        case T_NGPUS:
            fprintf(p_fout,"(ngpus");
            break;
        case T_NAME:
            fprintf(p_fout,"(name");
            break;
        case T_PROPS:
            fprintf(p_fout,"(props");
            break;
        case T_STATE:
            fprintf(p_fout,"(state");
            break;
        default:
            fprintf(p_fout,"<- unknown selector type");
            return(false);
    };

    switch(p_sel->Operator) {
        case O_LT:
            fprintf(p_fout," < ");
            break;
        case O_LE:
            fprintf(p_fout," <= ");
            break;
        case O_GT:
            fprintf(p_fout," > ");
            break;
        case O_GE:
            fprintf(p_fout," >= ");
            break;
        case O_EQ:
            fprintf(p_fout," == ");
            break;
        case O_NE:
            fprintf(p_fout," != ");
            break;
        default:
            fprintf(p_fout,"<- unknown selector operator");
            return(false);
    };

    switch(p_sel->Type) {
        case T_NCPUS:
        case T_NGPUS:
            fprintf(p_fout,"%d)",p_sel->IValue);
            break;
        case T_NAME:
        case T_PROPS:
        case T_STATE:
            fprintf(p_fout,"%s)",p_sel->SValue);
            break;
        default:
            fprintf(p_fout,"<- unknown selector value");
            return(false);
    };

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

int yyerror(const char* p_error)
{
    CSmallString error;
    error << p_error << " (pos: " << LexPosition << ")";
    ES_ERROR(error);
    return(0);
}

//------------------------------------------------------------------------------

int pperror(const char* p_error,int position)
{
    CSmallString error;
    error << position << ": " << p_error;
    ES_ERROR(error);
    return(0);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

struct SSelection* AllocateSelectionByString(enum SType type,enum SOperator op,char* str) {
    struct SSelection* p_sel = new struct SSelection;

    p_sel->Type = type;
    p_sel->Operator = op;
    strncpy(p_sel->SValue,str,100);

    SelectionAllocations.push_back(p_sel);
    return(p_sel);
}

//------------------------------------------------------------------------------

struct SSelection* AllocateSelectionByNumber(enum SType type,enum SOperator op,int num) {
    struct SSelection* p_sel = new struct SSelection;

    p_sel->Type = type;
    p_sel->Operator = op;
    p_sel->IValue = num;

    SelectionAllocations.push_back(p_sel);
    return(p_sel);
}

//------------------------------------------------------------------------------

struct SExpression* AllocateJobExpression(void) {
    struct SExpression* p_expr = new struct SExpression;

    p_expr->Operator = O_NONE;
    p_expr->Selection = NULL;
    p_expr->LeftExpression = NULL;
    p_expr->RightExpression = NULL;

    ExpressionAllocations.push_back(p_expr);
    return(p_expr);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================
