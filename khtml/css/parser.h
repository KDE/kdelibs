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
     S = 258,
     SGML_CD = 259,
     INCLUDES = 260,
     DASHMATCH = 261,
     BEGINSWITH = 262,
     ENDSWITH = 263,
     CONTAINS = 264,
     STRING = 265,
     IDENT = 266,
     HASH = 267,
     IMPORT_SYM = 268,
     PAGE_SYM = 269,
     MEDIA_SYM = 270,
     FONT_FACE_SYM = 271,
     CHARSET_SYM = 272,
     NAMESPACE_SYM = 273,
     KHTML_RULE_SYM = 274,
     KHTML_DECLS_SYM = 275,
     KHTML_VALUE_SYM = 276,
     IMPORTANT_SYM = 277,
     QEMS = 278,
     EMS = 279,
     EXS = 280,
     PXS = 281,
     CMS = 282,
     MMS = 283,
     INS = 284,
     PTS = 285,
     PCS = 286,
     DEGS = 287,
     RADS = 288,
     GRADS = 289,
     MSECS = 290,
     SECS = 291,
     HERZ = 292,
     KHERZ = 293,
     DIMEN = 294,
     PERCENTAGE = 295,
     NUMBER = 296,
     URI = 297,
     FUNCTION = 298,
     UNICODERANGE = 299
   };
#endif
#define S 258
#define SGML_CD 259
#define INCLUDES 260
#define DASHMATCH 261
#define BEGINSWITH 262
#define ENDSWITH 263
#define CONTAINS 264
#define STRING 265
#define IDENT 266
#define HASH 267
#define IMPORT_SYM 268
#define PAGE_SYM 269
#define MEDIA_SYM 270
#define FONT_FACE_SYM 271
#define CHARSET_SYM 272
#define NAMESPACE_SYM 273
#define KHTML_RULE_SYM 274
#define KHTML_DECLS_SYM 275
#define KHTML_VALUE_SYM 276
#define IMPORTANT_SYM 277
#define QEMS 278
#define EMS 279
#define EXS 280
#define PXS 281
#define CMS 282
#define MMS 283
#define INS 284
#define PTS 285
#define PCS 286
#define DEGS 287
#define RADS 288
#define GRADS 289
#define MSECS 290
#define SECS 291
#define HERZ 292
#define KHERZ 293
#define DIMEN 294
#define PERCENTAGE 295
#define NUMBER 296
#define URI 297
#define FUNCTION 298
#define UNICODERANGE 299




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)

typedef union YYSTYPE {
    CSSRuleImpl *rule;
    CSSSelector *selector;
    QPtrList<CSSSelector> *selectorList;
    bool ok;
    MediaListImpl *mediaList;
    CSSMediaRuleImpl *mediaRule;
    CSSRuleListImpl *ruleList;
    ParseString string;
    float val;
    int prop_id;
    unsigned int attribute;
    unsigned int element;
    unsigned int ns;
    CSSSelector::Relation relation;
    bool b;
    char tok;
    Value value;
    ValueList *valueList;
} YYSTYPE;
/* Line 1248 of yacc.c.  */

# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif





