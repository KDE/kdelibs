/* A Bison parser, made by GNU Bison 1.875.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002 Free Software Foundation, Inc.

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
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     NULLTOKEN = 258,
     TRUETOKEN = 259,
     FALSETOKEN = 260,
     STRING = 261,
     NUMBER = 262,
     BREAK = 263,
     CASE = 264,
     DEFAULT = 265,
     FOR = 266,
     NEW = 267,
     VAR = 268,
     CONTINUE = 269,
     FUNCTION = 270,
     RETURN = 271,
     VOID = 272,
     DELETE = 273,
     IF = 274,
     THIS = 275,
     DO = 276,
     WHILE = 277,
     ELSE = 278,
     IN = 279,
     INSTANCEOF = 280,
     TYPEOF = 281,
     SWITCH = 282,
     WITH = 283,
     RESERVED = 284,
     THROW = 285,
     TRY = 286,
     CATCH = 287,
     FINALLY = 288,
     EQEQ = 289,
     NE = 290,
     STREQ = 291,
     STRNEQ = 292,
     LE = 293,
     GE = 294,
     OR = 295,
     AND = 296,
     PLUSPLUS = 297,
     MINUSMINUS = 298,
     LSHIFT = 299,
     RSHIFT = 300,
     URSHIFT = 301,
     PLUSEQUAL = 302,
     MINUSEQUAL = 303,
     MULTEQUAL = 304,
     DIVEQUAL = 305,
     LSHIFTEQUAL = 306,
     RSHIFTEQUAL = 307,
     URSHIFTEQUAL = 308,
     ANDEQUAL = 309,
     MODEQUAL = 310,
     XOREQUAL = 311,
     OREQUAL = 312,
     IDENT = 313,
     AUTOPLUSPLUS = 314,
     AUTOMINUSMINUS = 315
   };
#endif
#define NULLTOKEN 258
#define TRUETOKEN 259
#define FALSETOKEN 260
#define STRING 261
#define NUMBER 262
#define BREAK 263
#define CASE 264
#define DEFAULT 265
#define FOR 266
#define NEW 267
#define VAR 268
#define CONTINUE 269
#define FUNCTION 270
#define RETURN 271
#define VOID 272
#define DELETE 273
#define IF 274
#define THIS 275
#define DO 276
#define WHILE 277
#define ELSE 278
#define IN 279
#define INSTANCEOF 280
#define TYPEOF 281
#define SWITCH 282
#define WITH 283
#define RESERVED 284
#define THROW 285
#define TRY 286
#define CATCH 287
#define FINALLY 288
#define EQEQ 289
#define NE 290
#define STREQ 291
#define STRNEQ 292
#define LE 293
#define GE 294
#define OR 295
#define AND 296
#define PLUSPLUS 297
#define MINUSMINUS 298
#define LSHIFT 299
#define RSHIFT 300
#define URSHIFT 301
#define PLUSEQUAL 302
#define MINUSEQUAL 303
#define MULTEQUAL 304
#define DIVEQUAL 305
#define LSHIFTEQUAL 306
#define RSHIFTEQUAL 307
#define URSHIFTEQUAL 308
#define ANDEQUAL 309
#define MODEQUAL 310
#define XOREQUAL 311
#define OREQUAL 312
#define IDENT 313
#define AUTOPLUSPLUS 314
#define AUTOMINUSMINUS 315




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
#line 50 "grammar.y"
typedef union YYSTYPE {
  int                 ival;
  double              dval;
  UString             *ustr;
  Identifier          *ident;
  Node                *node;
  StatementNode       *stat;
  ParameterNode       *param;
  FunctionBodyNode    *body;
  FuncDeclNode        *func;
  FunctionBodyNode    *prog;
  AssignExprNode      *init;
  SourceElementsNode  *srcs;
  StatListNode        *slist;
  ArgumentsNode       *args;
  ArgumentListNode    *alist;
  VarDeclNode         *decl;
  VarDeclListNode     *vlist;
  CaseBlockNode       *cblk;
  ClauseListNode      *clist;
  CaseClauseNode      *ccl;
  ElementNode         *elm;
  Operator            op;
  PropertyValueNode   *plist;
  PropertyNode        *pnode;
  CatchNode           *cnode;
  FinallyNode         *fnode;
} YYSTYPE;
/* Line 1242 of yacc.c.  */
#line 185 "grammar.tab.h"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE kjsyylval;

#if ! defined (YYLTYPE) && ! defined (YYLTYPE_IS_DECLARED)
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


