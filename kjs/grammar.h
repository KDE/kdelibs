/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

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

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     NULLTOKEN = 258,
     TRUETOKEN = 259,
     FALSETOKEN = 260,
     BREAK = 261,
     CASE = 262,
     DEFAULT = 263,
     FOR = 264,
     NEW = 265,
     VAR = 266,
     CONSTTOKEN = 267,
     CONTINUE = 268,
     FUNCTION = 269,
     RETURN = 270,
     VOIDTOKEN = 271,
     DELETETOKEN = 272,
     IF = 273,
     THISTOKEN = 274,
     DO = 275,
     WHILE = 276,
     INTOKEN = 277,
     INSTANCEOF = 278,
     TYPEOF = 279,
     SWITCH = 280,
     WITH = 281,
     RESERVED = 282,
     THROW = 283,
     TRY = 284,
     CATCH = 285,
     FINALLY = 286,
     DEBUGGER = 287,
     IMPORT = 288,
     IF_WITHOUT_ELSE = 289,
     ELSE = 290,
     EQEQ = 291,
     NE = 292,
     STREQ = 293,
     STRNEQ = 294,
     LE = 295,
     GE = 296,
     OR = 297,
     AND = 298,
     PLUSPLUS = 299,
     MINUSMINUS = 300,
     LSHIFT = 301,
     RSHIFT = 302,
     URSHIFT = 303,
     PLUSEQUAL = 304,
     MINUSEQUAL = 305,
     MULTEQUAL = 306,
     DIVEQUAL = 307,
     LSHIFTEQUAL = 308,
     RSHIFTEQUAL = 309,
     URSHIFTEQUAL = 310,
     ANDEQUAL = 311,
     MODEQUAL = 312,
     XOREQUAL = 313,
     OREQUAL = 314,
     NUMBER = 315,
     STRING = 316,
     IDENT = 317,
     AUTOPLUSPLUS = 318,
     AUTOMINUSMINUS = 319
   };
#endif
/* Tokens.  */
#define NULLTOKEN 258
#define TRUETOKEN 259
#define FALSETOKEN 260
#define BREAK 261
#define CASE 262
#define DEFAULT 263
#define FOR 264
#define NEW 265
#define VAR 266
#define CONSTTOKEN 267
#define CONTINUE 268
#define FUNCTION 269
#define RETURN 270
#define VOIDTOKEN 271
#define DELETETOKEN 272
#define IF 273
#define THISTOKEN 274
#define DO 275
#define WHILE 276
#define INTOKEN 277
#define INSTANCEOF 278
#define TYPEOF 279
#define SWITCH 280
#define WITH 281
#define RESERVED 282
#define THROW 283
#define TRY 284
#define CATCH 285
#define FINALLY 286
#define DEBUGGER 287
#define IMPORT 288
#define IF_WITHOUT_ELSE 289
#define ELSE 290
#define EQEQ 291
#define NE 292
#define STREQ 293
#define STRNEQ 294
#define LE 295
#define GE 296
#define OR 297
#define AND 298
#define PLUSPLUS 299
#define MINUSMINUS 300
#define LSHIFT 301
#define RSHIFT 302
#define URSHIFT 303
#define PLUSEQUAL 304
#define MINUSEQUAL 305
#define MULTEQUAL 306
#define DIVEQUAL 307
#define LSHIFTEQUAL 308
#define RSHIFTEQUAL 309
#define URSHIFTEQUAL 310
#define ANDEQUAL 311
#define MODEQUAL 312
#define XOREQUAL 313
#define OREQUAL 314
#define NUMBER 315
#define STRING 316
#define IDENT 317
#define AUTOPLUSPLUS 318
#define AUTOMINUSMINUS 319




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 59 "grammar.y"
{
  int                 ival;
  double              dval;
  UString             *ustr;
  Identifier          *ident;
  Node                *node;
  StatementNode       *stat;
  ParameterNode       *param;
  FunctionBodyNode    *body;
  FuncDeclNode        *func;
  FuncExprNode        *funcExpr;
  ProgramNode         *prog;
  AssignExprNode      *init;
  SourceElementsNode  *srcs;
  ArgumentsNode       *args;
  ArgumentListNode    *alist;
  VarDeclNode         *decl;
  VarDeclListNode     *vlist;
  CaseBlockNode       *cblk;
  ClauseListNode      *clist;
  CaseClauseNode      *ccl;
  ElementNode         *elm;
  Operator            op;
  PropertyListNode   *plist;
  PropertyNode       *pnode;
  PropertyNameNode   *pname;
  PackageNameNode     *pkgn;
}
/* Line 1489 of yacc.c.  */
#line 206 "grammar.tab.h"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE kjsyylval;

#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
} YYLTYPE;
# define yyltype YYLTYPE /* obsolescent; will be withdrawn */
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif

extern YYLTYPE kjsyylloc;
