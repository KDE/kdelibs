#ifndef ppdparser_cpp_h
#define ppdparser_cpp_h
/* A Bison parser, made by GNU Bison 1.875d.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004 Free Software Foundation, Inc.

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
     TRANSLATION = 258,
     OPENUI = 259,
     CLOSEUI = 260,
     OPENGROUP = 261,
     CLOSEGROUP = 262,
     DEFAULT = 263,
     KEYWORD = 264,
     OPTION = 265,
     STRINGPART = 266,
     QUOTED = 267,
     CONSTRAINT = 268,
     PAPERDIM = 269,
     IMGAREA = 270,
     FOODATA = 271,
     COMMENT = 272
   };
#endif
#define TRANSLATION 258
#define OPENUI 259
#define CLOSEUI 260
#define OPENGROUP 261
#define CLOSEGROUP 262
#define DEFAULT 263
#define KEYWORD 264
#define OPTION 265
#define STRINGPART 266
#define QUOTED 267
#define CONSTRAINT 268
#define PAPERDIM 269
#define IMGAREA 270
#define FOODATA 271
#define COMMENT 272




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE kdeprint_ppdlval;



#endif
