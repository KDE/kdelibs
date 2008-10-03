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
     UNIMPORTANT_TOK = 258,
     S = 259,
     SGML_CD = 260,
     INCLUDES = 261,
     DASHMATCH = 262,
     BEGINSWITH = 263,
     ENDSWITH = 264,
     CONTAINS = 265,
     STRING = 266,
     IDENT = 267,
     NTH = 268,
     HASH = 269,
     HEXCOLOR = 270,
     IMPORT_SYM = 271,
     PAGE_SYM = 272,
     MEDIA_SYM = 273,
     FONT_FACE_SYM = 274,
     CHARSET_SYM = 275,
     NAMESPACE_SYM = 276,
     KHTML_RULE_SYM = 277,
     KHTML_DECLS_SYM = 278,
     KHTML_VALUE_SYM = 279,
     KHTML_MEDIAQUERY_SYM = 280,
     IMPORTANT_SYM = 281,
     MEDIA_ONLY = 282,
     MEDIA_NOT = 283,
     MEDIA_AND = 284,
     QEMS = 285,
     EMS = 286,
     EXS = 287,
     PXS = 288,
     CMS = 289,
     MMS = 290,
     INS = 291,
     PTS = 292,
     PCS = 293,
     DEGS = 294,
     RADS = 295,
     GRADS = 296,
     MSECS = 297,
     SECS = 298,
     HERZ = 299,
     KHERZ = 300,
     DPI = 301,
     DPCM = 302,
     DIMEN = 303,
     PERCENTAGE = 304,
     FLOAT = 305,
     INTEGER = 306,
     URI = 307,
     FUNCTION = 308,
     NOTFUNCTION = 309,
     UNICODERANGE = 310
   };
#endif
/* Tokens.  */
#define UNIMPORTANT_TOK 258
#define S 259
#define SGML_CD 260
#define INCLUDES 261
#define DASHMATCH 262
#define BEGINSWITH 263
#define ENDSWITH 264
#define CONTAINS 265
#define STRING 266
#define IDENT 267
#define NTH 268
#define HASH 269
#define HEXCOLOR 270
#define IMPORT_SYM 271
#define PAGE_SYM 272
#define MEDIA_SYM 273
#define FONT_FACE_SYM 274
#define CHARSET_SYM 275
#define NAMESPACE_SYM 276
#define KHTML_RULE_SYM 277
#define KHTML_DECLS_SYM 278
#define KHTML_VALUE_SYM 279
#define KHTML_MEDIAQUERY_SYM 280
#define IMPORTANT_SYM 281
#define MEDIA_ONLY 282
#define MEDIA_NOT 283
#define MEDIA_AND 284
#define QEMS 285
#define EMS 286
#define EXS 287
#define PXS 288
#define CMS 289
#define MMS 290
#define INS 291
#define PTS 292
#define PCS 293
#define DEGS 294
#define RADS 295
#define GRADS 296
#define MSECS 297
#define SECS 298
#define HERZ 299
#define KHERZ 300
#define DPI 301
#define DPCM 302
#define DIMEN 303
#define PERCENTAGE 304
#define FLOAT 305
#define INTEGER 306
#define URI 307
#define FUNCTION 308
#define NOTFUNCTION 309
#define UNICODERANGE 310




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE

{
    CSSRuleImpl *rule;
    CSSSelector *selector;
    QList<CSSSelector*> *selectorList;
    bool ok;
    MediaListImpl *mediaList;
    CSSMediaRuleImpl *mediaRule;
    CSSRuleListImpl *ruleList;
    ParseString string;
    float val;
    int prop_id;
    unsigned int attribute;
    unsigned int element;
    CSSSelector::Relation relation;
    CSSSelector::Match match;
    bool b;
    char tok;
    Value value;
    ValueList *valueList;

    khtml::MediaQuery* mediaQuery;
    khtml::MediaQueryExp* mediaQueryExp;
    QList<khtml::MediaQueryExp*>* mediaQueryExpList;
    khtml::MediaQuery::Restrictor mediaQueryRestrictor;
}
/* Line 1489 of yacc.c.  */

	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



