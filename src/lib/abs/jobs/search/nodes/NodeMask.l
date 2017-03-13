/* =============================================================================
ABS Node Mask Lexical Analyzer
      Copyright (c) 2013 Petr Kulhanek, kulhanek@chemi.muni.cz
============================================================================= */

%{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "CommonParser.hpp"
#include "NodeMask.tab.h"
%}

%option noyywrap
newline       [\n]
whitesp       [ \t]+
string        [a-zA-Z\/\#*?\[\].\-\+_]+[a-zA-Z0-9\/\#*?,.\[\]\-\+_]*
inumber       [0-9]+

/* rules ==================================================================== */
%%

{inumber} {
    yynodelval.iValue.Position = LexPosition;
    if( sscanf(yynodetext, "%d", &yynodelval.iValue.Number) != 1 ){
        pperror("unable to convert string to integer",LexPosition);
        }
    LexPosition += yynodeleng;
    return INUMBER;
    }

"ncpus"     { yynodelval.gValue.Position = LexPosition; LexPosition += 5; return NCPUS; }
"nfreecpus" { yynodelval.gValue.Position = LexPosition; LexPosition += 5; return NFREECPUS; }
"ngpus"     { yynodelval.gValue.Position = LexPosition; LexPosition += 5; return NGPUS; }
"nfreegpus" { yynodelval.gValue.Position = LexPosition; LexPosition += 5; return NFREEGPUS; }
"name"      { yynodelval.gValue.Position = LexPosition; LexPosition += 4; return NAME; }
"props"     { yynodelval.gValue.Position = LexPosition; LexPosition += 5; return PROPS; }
"state"     { yynodelval.gValue.Position = LexPosition; LexPosition += 5; return STATE; }

"<"       { yynodelval.gValue.Position = LexPosition; LexPosition += 1; return LT; }
"<="      { yynodelval.gValue.Position = LexPosition; LexPosition += 2; return LE; }
">"       { yynodelval.gValue.Position = LexPosition; LexPosition += 1; return GT; }
">="      { yynodelval.gValue.Position = LexPosition; LexPosition += 2; return GE; }
"=="      { yynodelval.gValue.Position = LexPosition; LexPosition += 2; return EQ; }
"!="      { yynodelval.gValue.Position = LexPosition; LexPosition += 2; return NE; }

"&&"      { yynodelval.gValue.Position = LexPosition; LexPosition+=2; return AND; }
"||"      { yynodelval.gValue.Position = LexPosition; LexPosition+=2; return OR; }
"!"       { yynodelval.gValue.Position = LexPosition; LexPosition+=1; return NOT; }

"("       { yynodelval.gValue.Position = LexPosition; LexPosition++; return LBRA; }
")"       { yynodelval.gValue.Position = LexPosition; LexPosition++; return RBRA; }

{string}  {
    yynodelval.sValue.Position = LexPosition;
    if( yynodeleng >= 100 ){
        pperror("name is too long (max 100 characters)",LexPosition);
        }
    memset(yynodelval.sValue.String,0,100);
    strncpy(yynodelval.sValue.String,yynodetext,yynodeleng);
    LexPosition += yynodeleng;
    return STRING;
    }

{whitesp} { LexPosition += yynodeleng;}

{newline} {
            pperror("new line character is not allowed in mask specification",LexPosition);
            return ERROR;
            }
