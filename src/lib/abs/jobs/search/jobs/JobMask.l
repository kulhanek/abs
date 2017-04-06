/* =============================================================================
ABS Node Mask Lexical Analyzer
      Copyright (c) 2013 Petr Kulhanek, kulhanek@chemi.muni.cz
============================================================================= */

%{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "CommonParser.hpp"
#include "JobMask.tab.h"
%}

%option noyywrap
newline       [\n]
whitesp       [ \t]+
string        [a-zA-Z\/\#*?\[\].\-\+_]+[a-zA-Z0-9\/\#*?,.\[\]\-\+_]*
inumber       [0-9]+

/* rules ==================================================================== */
%%

{inumber} {
    yyjoblval.iValue.Position = LexPosition;
    if( sscanf(yyjobtext, "%d", &yyjoblval.iValue.Number) != 1 ){
        pperror("unable to convert string to integer",LexPosition);
        }
    LexPosition += yyjobleng;
    return INUMBER;
    }

"id"        { yyjoblval.gValue.Position = LexPosition; LexPosition += 2; return ID; }
"ncpus"     { yyjoblval.gValue.Position = LexPosition; LexPosition += 5; return NCPUS; }
"ngpus"     { yyjoblval.gValue.Position = LexPosition; LexPosition += 5; return NGPUS; }
"nnodes"    { yyjoblval.gValue.Position = LexPosition; LexPosition += 6; return NNODES; }
"title"     { yyjoblval.gValue.Position = LexPosition; LexPosition += 5; return TITLE; }
"queue"     { yyjoblval.gValue.Position = LexPosition; LexPosition += 5; return QUEUE; }
"user"      { yyjoblval.gValue.Position = LexPosition; LexPosition += 4; return USER; }
"path"      { yyjoblval.gValue.Position = LexPosition; LexPosition += 4; return PATH; }
"st"      { yyjoblval.gValue.Position = LexPosition; LexPosition += 2; return ST; }
"bs"      { yyjoblval.gValue.Position = LexPosition; LexPosition += 2; return BS; }

"<"       { yyjoblval.gValue.Position = LexPosition; LexPosition += 1; return LT; }
"<="      { yyjoblval.gValue.Position = LexPosition; LexPosition += 2; return LE; }
">"       { yyjoblval.gValue.Position = LexPosition; LexPosition += 1; return GT; }
">="      { yyjoblval.gValue.Position = LexPosition; LexPosition += 2; return GE; }
"=="      { yyjoblval.gValue.Position = LexPosition; LexPosition += 2; return EQ; }
"!="      { yyjoblval.gValue.Position = LexPosition; LexPosition += 2; return NE; }

"&&"      { yyjoblval.gValue.Position = LexPosition; LexPosition+=2; return AND; }
"||"      { yyjoblval.gValue.Position = LexPosition; LexPosition+=2; return OR; }
"!"       { yyjoblval.gValue.Position = LexPosition; LexPosition+=1; return NOT; }

"("       { yyjoblval.gValue.Position = LexPosition; LexPosition++; return LBRA; }
")"       { yyjoblval.gValue.Position = LexPosition; LexPosition++; return RBRA; }

{string}  {
    yyjoblval.sValue.Position = LexPosition;
    if( yyjobleng >= 100 ){
        pperror("name is too long (max 100 characters)",LexPosition);
        }
    memset(yyjoblval.sValue.String,0,100);
    strncpy(yyjoblval.sValue.String,yyjobtext,yyjobleng);
    LexPosition += yyjobleng;
    return STRING;
    }

{whitesp} { LexPosition += yyjobleng;}

{newline} {
            pperror("new line character is not allowed in mask specification",LexPosition);
            return ERROR;
            }
