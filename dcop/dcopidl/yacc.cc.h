/* A Bison parser, made by GNU Bison 1.875a.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003 Free Software Foundation, Inc.

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
     T_CHARACTER_LITERAL = 258,
     T_DOUBLE_LITERAL = 259,
     T_IDENTIFIER = 260,
     T_INTEGER_LITERAL = 261,
     T_STRING_LITERAL = 262,
     T_INCLUDE = 263,
     T_CLASS = 264,
     T_STRUCT = 265,
     T_LEFT_CURLY_BRACKET = 266,
     T_LEFT_PARANTHESIS = 267,
     T_RIGHT_CURLY_BRACKET = 268,
     T_RIGHT_PARANTHESIS = 269,
     T_COLON = 270,
     T_SEMICOLON = 271,
     T_PUBLIC = 272,
     T_PROTECTED = 273,
     T_TRIPE_DOT = 274,
     T_PRIVATE = 275,
     T_VIRTUAL = 276,
     T_CONST = 277,
     T_INLINE = 278,
     T_FRIEND = 279,
     T_RETURN = 280,
     T_SIGNAL = 281,
     T_SLOT = 282,
     T_TYPEDEF = 283,
     T_PLUS = 284,
     T_MINUS = 285,
     T_COMMA = 286,
     T_ASTERISK = 287,
     T_TILDE = 288,
     T_LESS = 289,
     T_GREATER = 290,
     T_AMPERSAND = 291,
     T_EXTERN = 292,
     T_EXTERN_C = 293,
     T_ACCESS = 294,
     T_ENUM = 295,
     T_NAMESPACE = 296,
     T_USING = 297,
     T_UNKNOWN = 298,
     T_TRIPLE_DOT = 299,
     T_TRUE = 300,
     T_FALSE = 301,
     T_STATIC = 302,
     T_MUTABLE = 303,
     T_EQUAL = 304,
     T_SCOPE = 305,
     T_NULL = 306,
     T_INT = 307,
     T_ARRAY_OPEN = 308,
     T_ARRAY_CLOSE = 309,
     T_CHAR = 310,
     T_DCOP = 311,
     T_DCOP_AREA = 312,
     T_DCOP_SIGNAL_AREA = 313,
     T_SIGNED = 314,
     T_UNSIGNED = 315,
     T_LONG = 316,
     T_SHORT = 317,
     T_FUNOPERATOR = 318,
     T_MISCOPERATOR = 319,
     T_SHIFT = 320
   };
#endif
#define T_CHARACTER_LITERAL 258
#define T_DOUBLE_LITERAL 259
#define T_IDENTIFIER 260
#define T_INTEGER_LITERAL 261
#define T_STRING_LITERAL 262
#define T_INCLUDE 263
#define T_CLASS 264
#define T_STRUCT 265
#define T_LEFT_CURLY_BRACKET 266
#define T_LEFT_PARANTHESIS 267
#define T_RIGHT_CURLY_BRACKET 268
#define T_RIGHT_PARANTHESIS 269
#define T_COLON 270
#define T_SEMICOLON 271
#define T_PUBLIC 272
#define T_PROTECTED 273
#define T_TRIPE_DOT 274
#define T_PRIVATE 275
#define T_VIRTUAL 276
#define T_CONST 277
#define T_INLINE 278
#define T_FRIEND 279
#define T_RETURN 280
#define T_SIGNAL 281
#define T_SLOT 282
#define T_TYPEDEF 283
#define T_PLUS 284
#define T_MINUS 285
#define T_COMMA 286
#define T_ASTERISK 287
#define T_TILDE 288
#define T_LESS 289
#define T_GREATER 290
#define T_AMPERSAND 291
#define T_EXTERN 292
#define T_EXTERN_C 293
#define T_ACCESS 294
#define T_ENUM 295
#define T_NAMESPACE 296
#define T_USING 297
#define T_UNKNOWN 298
#define T_TRIPLE_DOT 299
#define T_TRUE 300
#define T_FALSE 301
#define T_STATIC 302
#define T_MUTABLE 303
#define T_EQUAL 304
#define T_SCOPE 305
#define T_NULL 306
#define T_INT 307
#define T_ARRAY_OPEN 308
#define T_ARRAY_CLOSE 309
#define T_CHAR 310
#define T_DCOP 311
#define T_DCOP_AREA 312
#define T_DCOP_SIGNAL_AREA 313
#define T_SIGNED 314
#define T_UNSIGNED 315
#define T_LONG 316
#define T_SHORT 317
#define T_FUNOPERATOR 318
#define T_MISCOPERATOR 319
#define T_SHIFT 320




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
#line 67 "yacc.yy"
typedef union YYSTYPE {
  long   _int;
  QString        *_str;
  unsigned short          _char;
  double _float;
} YYSTYPE;
/* Line 1240 of yacc.c.  */
#line 174 "yacc.hh"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;



