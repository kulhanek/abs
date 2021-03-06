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

#include <CommonParser.hpp>
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

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

int init_mask(void)
{
    free_mask_tree();
    LexPosition = 1;
    TopExpression = NULL;
    return(0);
}

//------------------------------------------------------------------------------

struct SExpression* get_expression_tree(void) {
    return(TopExpression);
}

//------------------------------------------------------------------------------

int free_mask_tree(void)
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

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

int yyjoberror(const char* p_error)
{
    CSmallString error;
    error << p_error << " (pos: " << LexPosition << ")";
    ES_ERROR(error);
    return(0);
}

//------------------------------------------------------------------------------

int yynodeerror(const char* p_error)
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

struct SExpression* AllocateExpression(void) {
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
