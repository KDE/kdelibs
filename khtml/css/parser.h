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
     HASH = 268,
     IMPORT_SYM = 269,
     PAGE_SYM = 270,
     MEDIA_SYM = 271,
     FONT_FACE_SYM = 272,
     CHARSET_SYM = 273,
     NAMESPACE_SYM = 274,
     KHTML_RULE_SYM = 275,
     KHTML_DECLS_SYM = 276,
     KHTML_VALUE_SYM = 277,
     IMPORTANT_SYM = 278,
     QEMS = 279,
     EMS = 280,
     EXS = 281,
     PXS = 282,
     CMS = 283,
     MMS = 284,
     INS = 285,
     PTS = 286,
     PCS = 287,
     DEGS = 288,
     RADS = 289,
     GRADS = 290,
     MSECS = 291,
     SECS = 292,
     HERZ = 293,
     KHERZ = 294,
     DIMEN = 295,
     PERCENTAGE = 296,
     NUMBER = 297,
     URI = 298,
     FUNCTION = 299,
     UNICODERANGE = 300
   };
#endif
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
#define HASH 268
#define IMPORT_SYM 269
#define PAGE_SYM 270
#define MEDIA_SYM 271
#define FONT_FACE_SYM 272
#define CHARSET_SYM 273
#define NAMESPACE_SYM 274
#define KHTML_RULE_SYM 275
#define KHTML_DECLS_SYM 276
#define KHTML_VALUE_SYM 277
#define IMPORTANT_SYM 278
#define QEMS 279
#define EMS 280
#define EXS 281
#define PXS 282
#define CMS 283
#define MMS 284
#define INS 285
#define PTS 286
#define PCS 287
#define DEGS 288
#define RADS 289
#define GRADS 290
#define MSECS 291
#define SECS 292
#define HERZ 293
#define KHERZ 294
#define DIMEN 295
#define PERCENTAGE 296
#define NUMBER 297
#define URI 298
#define FUNCTION 299
#define UNICODERANGE 300




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





