/* A Bison parser, made by GNU Bison 3.0.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2013 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_YYNODE_NODEMASK_TAB_H_INCLUDED
# define YY_YYNODE_NODEMASK_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yynodedebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    STRING = 258,
    INUMBER = 259,
    NCPUS = 260,
    NFREECPUS = 261,
    NGPUS = 262,
    NFREEGPUS = 263,
    NAME = 264,
    PROPS = 265,
    MEM = 266,
    FREEMEM = 267,
    ST = 268,
    BS = 269,
    LT = 270,
    LE = 271,
    GT = 272,
    GE = 273,
    NE = 274,
    EQ = 275,
    NOT = 276,
    AND = 277,
    OR = 278,
    RBRA = 279,
    LBRA = 280,
    ERROR = 281
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE YYSTYPE;
union YYSTYPE
{
#line 16 "NodeMask.y" /* yacc.c:1909  */

    SGrammar                gValue;
    SInteger                iValue;
    SString                 sValue;
    enum SType              tValue;
    struct SSelection*      selValue;
    struct SExpression*     exprValue;
    

#line 91 "NodeMask.tab.h" /* yacc.c:1909  */
};
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yynodelval;

int yynodeparse (void);

#endif /* !YY_YYNODE_NODEMASK_TAB_H_INCLUDED  */
