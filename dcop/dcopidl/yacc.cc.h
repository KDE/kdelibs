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
     T_UNIMPORTANT = 258,
     T_CHARACTER_LITERAL = 259,
     T_DOUBLE_LITERAL = 260,
     T_IDENTIFIER = 261,
     T_INTEGER_LITERAL = 262,
     T_STRING_LITERAL = 263,
     T_INCLUDE = 264,
     T_CLASS = 265,
     T_STRUCT = 266,
     T_LEFT_CURLY_BRACKET = 267,
     T_LEFT_PARANTHESIS = 268,
     T_RIGHT_CURLY_BRACKET = 269,
     T_RIGHT_PARANTHESIS = 270,
     T_COLON = 271,
     T_SEMICOLON = 272,
     T_PUBLIC = 273,
     T_PROTECTED = 274,
     T_TRIPE_DOT = 275,
     T_PRIVATE = 276,
     T_VIRTUAL = 277,
     T_CONST = 278,
     T_INLINE = 279,
     T_FRIEND = 280,
     T_RETURN = 281,
     T_SIGNAL = 282,
     T_SLOT = 283,
     T_TYPEDEF = 284,
     T_PLUS = 285,
     T_MINUS = 286,
     T_COMMA = 287,
     T_ASTERISK = 288,
     T_TILDE = 289,
     T_LESS = 290,
     T_GREATER = 291,
     T_AMPERSAND = 292,
     T_EXTERN = 293,
     T_EXTERN_C = 294,
     T_ACCESS = 295,
     T_ENUM = 296,
     T_NAMESPACE = 297,
     T_USING = 298,
     T_UNKNOWN = 299,
     T_TRIPLE_DOT = 300,
     T_TRUE = 301,
     T_FALSE = 302,
     T_STATIC = 303,
     T_MUTABLE = 304,
     T_EQUAL = 305,
     T_SCOPE = 306,
     T_NULL = 307,
     T_INT = 308,
     T_ARRAY_OPEN = 309,
     T_ARRAY_CLOSE = 310,
     T_CHAR = 311,
     T_DCOP = 312,
     T_DCOP_AREA = 313,
     T_DCOP_SIGNAL_AREA = 314,
     T_SIGNED = 315,
     T_UNSIGNED = 316,
     T_LONG = 317,
     T_SHORT = 318,
     T_FUNOPERATOR = 319,
     T_MISCOPERATOR = 320,
     T_SHIFT = 321
   };
#endif
#define T_UNIMPORTANT 258
#define T_CHARACTER_LITERAL 259
#define T_DOUBLE_LITERAL 260
#define T_IDENTIFIER 261
#define T_INTEGER_LITERAL 262
#define T_STRING_LITERAL 263
#define T_INCLUDE 264
#define T_CLASS 265
#define T_STRUCT 266
#define T_LEFT_CURLY_BRACKET 267
#define T_LEFT_PARANTHESIS 268
#define T_RIGHT_CURLY_BRACKET 269
#define T_RIGHT_PARANTHESIS 270
#define T_COLON 271
#define T_SEMICOLON 272
#define T_PUBLIC 273
#define T_PROTECTED 274
#define T_TRIPE_DOT 275
#define T_PRIVATE 276
#define T_VIRTUAL 277
#define T_CONST 278
#define T_INLINE 279
#define T_FRIEND 280
#define T_RETURN 281
#define T_SIGNAL 282
#define T_SLOT 283
#define T_TYPEDEF 284
#define T_PLUS 285
#define T_MINUS 286
#define T_COMMA 287
#define T_ASTERISK 288
#define T_TILDE 289
#define T_LESS 290
#define T_GREATER 291
#define T_AMPERSAND 292
#define T_EXTERN 293
#define T_EXTERN_C 294
#define T_ACCESS 295
#define T_ENUM 296
#define T_NAMESPACE 297
#define T_USING 298
#define T_UNKNOWN 299
#define T_TRIPLE_DOT 300
#define T_TRUE 301
#define T_FALSE 302
#define T_STATIC 303
#define T_MUTABLE 304
#define T_EQUAL 305
#define T_SCOPE 306
#define T_NULL 307
#define T_INT 308
#define T_ARRAY_OPEN 309
#define T_ARRAY_CLOSE 310
#define T_CHAR 311
#define T_DCOP 312
#define T_DCOP_AREA 313
#define T_DCOP_SIGNAL_AREA 314
#define T_SIGNED 315
#define T_UNSIGNED 316
#define T_LONG 317
#define T_SHORT 318
#define T_FUNOPERATOR 319
#define T_MISCOPERATOR 320
#define T_SHIFT 321




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
#line 67 "yacc.yy"
typedef union YYSTYPE {
  long   _int;
  QString        *_str;
  unsigned short          _char;
  double _float;
} YYSTYPE;
/* Line 1285 of yacc.c.  */
#line 176 "yacc.hh"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;



