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
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

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
     NOT = 258,
     EQ = 259,
     NEQ = 260,
     LEQ = 261,
     GEQ = 262,
     LE = 263,
     GR = 264,
     OR = 265,
     AND = 266,
     TOKEN_IN = 267,
     EXIST = 268,
     MAX = 269,
     MIN = 270,
     VAL_BOOL = 271,
     VAL_STRING = 272,
     VAL_ID = 273,
     VAL_NUM = 274,
     VAL_FLOAT = 275
   };
#endif
#define NOT 258
#define EQ 259
#define NEQ 260
#define LEQ 261
#define GEQ 262
#define LE 263
#define GR 264
#define OR 265
#define AND 266
#define TOKEN_IN 267
#define EXIST 268
#define MAX 269
#define MIN 270
#define VAL_BOOL 271
#define VAL_STRING 272
#define VAL_ID 273
#define VAL_NUM 274
#define VAL_FLOAT 275




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
#line 13 "yacc.y"
typedef union YYSTYPE {
     char valb;
     int vali;
     double vald;
     char *name;
     void *ptr;
} YYSTYPE;
/* Line 1240 of yacc.c.  */
#line 84 "yacc.tab.h"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE kiotraderlval;



