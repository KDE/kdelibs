
/*  A Bison parser, made from parser.y
    by GNU Bison version 1.28  */

#define YYBISON 1  /* Identify Bison output.  */

#define yyparse cssyyparse
#define yylex cssyylex
#define yyerror cssyyerror
#define yylval cssyylval
#define yychar cssyychar
#define yydebug cssyydebug
#define yynerrs cssyynerrs
#define	S	257
#define	SGML_CD	258
#define	INCLUDES	259
#define	DASHMATCH	260
#define	STRING	261
#define	IDENT	262
#define	HASH	263
#define	IMPORT_SYM	264
#define	PAGE_SYM	265
#define	MEDIA_SYM	266
#define	FONT_FACE_SYM	267
#define	CHARSET_SYM	268
#define	KONQ_RULE_SYM	269
#define	KONQ_DECLS_SYM	270
#define	KONQ_VALUE_SYM	271
#define	IMPORTANT_SYM	272
#define	EMS	273
#define	EXS	274
#define	PXS	275
#define	CMS	276
#define	MMS	277
#define	INS	278
#define	PTS	279
#define	PCS	280
#define	DEGS	281
#define	RADS	282
#define	GRADS	283
#define	MSECS	284
#define	SECS	285
#define	HERZ	286
#define	KHERZ	287
#define	DIMEN	288
#define	PERCENTAGE	289
#define	NUMBER	290
#define	URI	291
#define	FUNCTION	292
#define	UNICODERANGE	293

#line 1 "parser.y"


/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2002-2003 Lars Knoll (knoll@kde.org)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  $Id$
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <string.h>
#include <stdlib.h>

#include <dom/dom_string.h>
#include <xml/dom_docimpl.h>
#include <css/css_ruleimpl.h>
#include <css/css_stylesheetimpl.h>
#include <css/css_valueimpl.h>
#include <misc/htmlhashes.h>
#include "cssparser.h"

#include <assert.h>
#include <kdebug.h>
// #define CSS_DEBUG

using namespace DOM;

//
// The following file defines the function
//     const struct props *findProp(const char *word, int len)
//
// with 'props->id' a CSS property in the range from CSS_PROP_MIN to
// (and including) CSS_PROP_TOTAL-1
#include "cssproperties.c"
#include "cssvalues.c"

static inline int DOM::getPropertyID(const char *tagStr, int len)
{
    const struct props *propsPtr = findProp(tagStr, len);
    if (!propsPtr)
        return 0;

    return propsPtr->id;
}

static inline int getValueID(const char *tagStr, int len)
{
    const struct css_value *val = findValue(tagStr, len);
    if (!val)
        return 0;

    return val->id;
}


#define YYDEBUG 0
#define YYMAXDEPTH 0
#define YYPARSE_PARAM parser

#line 79 "parser.y"
typedef union {
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
    int attribute;
    int element;
    CSSSelector::Relation relation;
    bool b;
    char tok;
    Value value;
    ValueList *valueList;
} YYSTYPE;
#line 99 "parser.y"


static inline int cssyyerror(const char *x ) {
#ifdef CSS_DEBUG
    qDebug( x );
#else
    Q_UNUSED( x );
#endif
    return 1;
}

static int cssyylex( YYSTYPE *yylval ) {
    return CSSParser::current()->lex( yylval );
}


#include <stdio.h>

#ifndef __cplusplus
#ifndef __STDC__
#define const
#endif
#endif



#define	YYFINAL		231
#define	YYFLAG		-32768
#define	YYNTBASE	55

#define YYTRANSLATE(x) ((unsigned)(x) <= 293 ? yytranslate[x] : 102)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
    53,    50,    47,    46,    49,    11,    54,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,    10,    45,     2,
    52,    48,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
    12,     2,    51,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,    43,     2,    44,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     1,     3,     4,     5,     6,
     7,     8,     9,    13,    14,    15,    16,    17,    18,    19,
    20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
    30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
    40,    41,    42
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     5,     8,    11,    14,    21,    27,    33,    34,    37,
    38,    41,    44,    45,    51,    52,    56,    57,    61,    63,
    65,    67,    69,    71,    74,    77,    80,    86,    90,    97,
    99,   101,   102,   104,   109,   117,   119,   124,   125,   129,
   132,   136,   140,   143,   146,   147,   149,   151,   157,   159,
   164,   166,   170,   173,   177,   180,   182,   184,   186,   189,
   191,   193,   195,   197,   200,   203,   208,   217,   219,   221,
   223,   225,   227,   230,   237,   239,   242,   244,   247,   251,
   255,   260,   265,   271,   274,   277,   278,   280,   284,   287,
   290,   291,   293,   296,   299,   302,   305,   308,   310,   312,
   315,   318,   321,   324,   327,   330,   333,   336,   339,   342,
   345,   348,   351,   354,   357,   360,   363,   369
};

static const short yyrhs[] = {    61,
    60,    62,    63,     0,    56,    59,     0,    57,    59,     0,
    58,    59,     0,    18,    43,    59,    78,    59,    44,     0,
    19,    43,    59,    91,    44,     0,    20,    43,    59,    96,
    44,     0,     0,    59,     3,     0,     0,    60,     4,     0,
    60,     3,     0,     0,    17,    59,     7,    59,    45,     0,
     0,    62,    67,    60,     0,     0,    63,    64,    60,     0,
    78,     0,    70,     0,    74,     0,    75,     0,    65,     0,
     1,    66,     0,     1,    45,     0,     1,    44,     0,    43,
     1,    66,     1,    44,     0,    43,     1,    44,     0,    13,
    59,    68,    59,    69,    45,     0,     7,     0,    40,     0,
     0,    73,     0,    69,    46,    59,    73,     0,    15,    59,
    71,    43,    59,    72,    44,     0,    73,     0,    69,    46,
    59,    73,     0,     0,    72,    78,    59,     0,     8,    59,
     0,    14,     1,    44,     0,    16,     1,    44,     0,    47,
    59,     0,    48,    59,     0,     0,    49,     0,    47,     0,
    79,    43,    59,    91,    44,     0,    80,     0,    79,    46,
    59,    80,     0,    81,     0,    80,    76,    81,     0,    82,
    59,     0,    82,    83,    59,     0,    83,    59,     0,     8,
     0,    50,     0,    84,     0,    83,    84,     0,     9,     0,
    85,     0,    87,     0,    90,     0,    11,     8,     0,     8,
    59,     0,    12,    59,    86,    51,     0,    12,    59,    86,
    88,    59,    89,    59,    51,     0,    52,     0,     5,     0,
     6,     0,     8,     0,     7,     0,    10,     8,     0,    10,
    41,    59,     8,    59,    53,     0,    93,     0,    92,    93,
     0,    92,     0,     1,    66,     0,    93,    45,    59,     0,
     1,    45,    59,     0,    92,    93,    45,    59,     0,    92,
     1,    45,    59,     0,    94,    10,    59,    96,    95,     0,
     8,    59,     0,    21,    59,     0,     0,    98,     0,    96,
    97,    98,     0,    54,    59,     0,    46,    59,     0,     0,
    99,     0,    77,    99,     0,     7,    59,     0,     8,    59,
     0,    40,    59,     0,    42,    59,     0,   101,     0,   100,
     0,    39,    59,     0,    38,    59,     0,    24,    59,     0,
    25,    59,     0,    26,    59,     0,    27,    59,     0,    28,
    59,     0,    29,    59,     0,    30,    59,     0,    31,    59,
     0,    32,    59,     0,    33,    59,     0,    34,    59,     0,
    35,    59,     0,    36,    59,     0,    22,    59,     0,    23,
    59,     0,    41,    59,    96,    53,    59,     0,     9,    59,
     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
   217,   219,   220,   221,   224,   231,   237,   262,   264,   267,
   269,   270,   273,   275,   282,   284,   295,   297,   307,   309,
   310,   311,   312,   315,   322,   328,   336,   338,   341,   354,
   356,   359,   363,   367,   374,   388,   393,   400,   402,   411,
   433,   439,   445,   447,   448,   451,   453,   456,   476,   489,
   502,   506,   513,   518,   522,   528,   545,   550,   555,   563,
   570,   571,   572,   575,   584,   604,   610,   618,   622,   625,
   630,   632,   635,   641,   649,   653,   658,   661,   669,   673,
   679,   684,   692,   715,   722,   724,   727,   732,   745,   749,
   752,   757,   759,   760,   761,   767,   768,   769,   771,   776,
   778,   779,   780,   781,   782,   783,   784,   785,   786,   787,
   788,   789,   790,   791,   792,   793,   797,   812
};
#endif


#if YYDEBUG != 0 || defined (YYERROR_VERBOSE)

static const char * const yytname[] = {   "$","error","$undefined.","S","SGML_CD",
"INCLUDES","DASHMATCH","STRING","IDENT","HASH","':'","'.'","'['","IMPORT_SYM",
"PAGE_SYM","MEDIA_SYM","FONT_FACE_SYM","CHARSET_SYM","KONQ_RULE_SYM","KONQ_DECLS_SYM",
"KONQ_VALUE_SYM","IMPORTANT_SYM","EMS","EXS","PXS","CMS","MMS","INS","PTS","PCS",
"DEGS","RADS","GRADS","MSECS","SECS","HERZ","KHERZ","DIMEN","PERCENTAGE","NUMBER",
"URI","FUNCTION","UNICODERANGE","'{'","'}'","';'","','","'+'","'>'","'-'","'*'",
"']'","'='","')'","'/'","stylesheet","konq_rule","konq_decls","konq_value","maybe_space",
"maybe_sgml","maybe_charset","import_list","rule_list","rule","invalid_rule",
"invalid_block","import","string_or_uri","media_list","media","media_list2",
"ruleset_list","medium","page","font_face","combinator","unary_operator","ruleset",
"selector_list","selector","simple_selector","element_name","specifier_list",
"specifier","class","attrib_id","attrib","match","ident_or_string","pseudo",
"declaration_list","decl_list","declaration","property","prio","expr","operator",
"term","unary_term","function","hexcolor", NULL
};
#endif

static const short yyr1[] = {     0,
    55,    55,    55,    55,    56,    57,    58,    59,    59,    60,
    60,    60,    61,    61,    62,    62,    63,    63,    64,    64,
    64,    64,    64,    65,    65,    65,    66,    66,    67,    68,
    68,    69,    69,    69,    70,    71,    71,    72,    72,    73,
    74,    75,    76,    76,    76,    77,    77,    78,    79,    79,
    80,    80,    81,    81,    81,    82,    82,    83,    83,    84,
    84,    84,    84,    85,    86,    87,    87,    88,    88,    88,
    89,    89,    90,    90,    91,    91,    91,    91,    92,    92,
    92,    92,    93,    94,    95,    95,    96,    96,    97,    97,
    97,    98,    98,    98,    98,    98,    98,    98,    98,    99,
    99,    99,    99,    99,    99,    99,    99,    99,    99,    99,
    99,    99,    99,    99,    99,    99,   100,   101
};

static const short yyr2[] = {     0,
     4,     2,     2,     2,     6,     5,     5,     0,     2,     0,
     2,     2,     0,     5,     0,     3,     0,     3,     1,     1,
     1,     1,     1,     2,     2,     2,     5,     3,     6,     1,
     1,     0,     1,     4,     7,     1,     4,     0,     3,     2,
     3,     3,     2,     2,     0,     1,     1,     5,     1,     4,
     1,     3,     2,     3,     2,     1,     1,     1,     2,     1,
     1,     1,     1,     2,     2,     4,     8,     1,     1,     1,
     1,     1,     2,     6,     1,     2,     1,     2,     3,     3,
     4,     4,     5,     2,     2,     0,     1,     3,     2,     2,
     0,     1,     2,     2,     2,     2,     2,     1,     1,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     5,     2
};

static const short yydefact[] = {    13,
     8,     0,     0,     0,     8,     8,     8,    10,     0,     8,
     8,     8,     2,     3,     4,    15,     9,     8,     0,     0,
     0,    12,    11,    17,     0,    56,    60,     0,     0,     8,
    57,     8,     0,    45,    51,     8,     8,    58,    61,    62,
    63,     0,     8,     0,     0,    75,     0,     8,     8,     8,
     8,     8,     8,     8,     8,     8,     8,     8,     8,     8,
     8,     8,     8,     8,     8,     8,     8,     8,     8,     8,
    47,    46,     0,    91,    87,    92,    99,    98,     8,     0,
    10,    14,    73,     8,    64,     0,     0,     8,     8,     8,
     8,     0,    53,     8,    55,    59,     0,     8,    78,    84,
     6,     0,    76,     8,     8,    94,    95,   118,   115,   116,
   102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
   112,   113,   114,   101,   100,    96,     0,    97,    93,     7,
     8,     8,     0,     0,     0,     0,     8,     0,    10,    23,
    20,    21,    22,    19,    16,     0,     8,     0,     5,     0,
     0,    43,    44,    52,    54,     0,    80,     8,     8,    79,
     0,    91,    90,    89,    88,    30,    31,     8,    26,    25,
    24,     0,    32,     0,    18,     8,    65,    69,    70,    66,
    68,     8,     0,    45,    28,     0,    82,    81,    91,     8,
    32,    41,     8,     0,     0,    33,    42,     0,     0,    48,
     0,     8,    83,   117,     0,    33,    40,     8,     8,    74,
    72,    71,     8,    27,    85,    29,     8,     0,    38,     0,
     0,    34,     0,    67,    34,    35,     8,    39,     0,     0,
     0
};

static const short yydefgoto[] = {   229,
     5,     6,     7,     9,    16,     8,    24,    80,   139,   140,
    99,    81,   168,   194,   141,   195,   223,   196,   142,   143,
    92,    73,    32,    33,    34,    35,    36,    37,    38,    39,
   148,    40,   182,   213,    41,    44,    45,    46,    47,   203,
    74,   133,    75,    76,    77,    78
};

static const short yypact[] = {   155,
-32768,   -39,   -26,   -17,-32768,-32768,-32768,-32768,    73,-32768,
-32768,-32768,     8,     8,     8,    87,-32768,-32768,    25,   130,
   202,-32768,-32768,    59,    26,-32768,-32768,    70,    74,-32768,
-32768,-32768,   141,   112,-32768,   169,   169,-32768,-32768,-32768,
-32768,   146,-32768,    53,    69,    56,   114,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,   266,   -32,-32768,-32768,-32768,-32768,-32768,    94,
-32768,-32768,-32768,-32768,-32768,   131,    37,-32768,-32768,-32768,
-32768,   111,     8,   169,     8,-32768,   129,-32768,-32768,     8,
-32768,    62,   104,-32768,-32768,     8,     8,     8,     8,     8,
     8,     8,     8,     8,     8,     8,     8,     8,     8,     8,
     8,     8,     8,     8,     8,     8,   202,     8,-32768,-32768,
-32768,-32768,   238,    89,   125,   150,-32768,   156,-32768,-32768,
-32768,-32768,-32768,-32768,    87,   140,-32768,    36,-32768,   130,
    25,     8,     8,-32768,     8,    97,     8,-32768,-32768,     8,
   202,   -30,     8,     8,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,   120,   144,   132,    87,-32768,     8,-32768,-32768,-32768,
-32768,-32768,   142,   119,-32768,   189,     8,     8,    91,-32768,
   144,-32768,-32768,   147,   149,   151,-32768,    15,    12,-32768,
   152,-32768,-32768,     8,   137,-32768,     8,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,     8,-32768,-32768,   144,     8,    18,
   144,   157,   106,-32768,-32768,-32768,-32768,     8,   195,   198,
-32768
};

static const short yypgoto[] = {-32768,
-32768,-32768,-32768,    -5,   -73,-32768,-32768,-32768,-32768,-32768,
  -126,-32768,-32768,    10,-32768,-32768,-32768,   -93,-32768,-32768,
-32768,-32768,   -77,-32768,    48,   110,-32768,   170,   -27,-32768,
-32768,-32768,-32768,-32768,-32768,    57,-32768,   168,-32768,-32768,
   -88,-32768,    81,   143,-32768,-32768
};


#define	YYLAST		305


static const short yytable[] = {    13,
    14,    15,   144,    10,    19,    20,    21,   145,   171,    96,
    17,   130,    25,   131,    17,   131,    11,    17,   211,   212,
    17,   132,   190,   132,    86,    12,    87,    17,    17,   186,
    93,    95,    26,    27,    28,    29,    30,   100,   162,    17,
   178,   179,   106,   107,   108,   109,   110,   111,   112,   113,
   114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
   124,   125,   126,   127,   128,   175,    96,   210,   224,   102,
    82,    79,   189,   134,    31,    17,    43,    83,   146,    18,
   149,    85,   150,   151,   152,   153,   180,   181,   155,    22,
    23,    17,   157,    -1,   135,   166,   101,   206,   160,   161,
   104,    26,    27,    28,    29,    30,   158,   136,   137,   138,
    84,   202,   -77,    26,    27,    28,    29,    30,    26,    27,
    28,    29,    30,   105,   222,   163,   164,   225,   167,   156,
    42,   173,    17,    17,   -86,   -86,   131,    43,   147,    97,
   185,   177,    17,    31,   132,   227,    17,   176,   159,   226,
   172,   193,   187,   188,   -49,    31,   174,   -49,    90,    91,
    31,   -50,   191,   192,   -50,    90,    91,    97,   169,   170,
   198,     1,     2,     3,     4,   197,   199,    27,    28,    29,
    30,   216,   217,    88,   204,   200,    89,   207,    97,   201,
    98,   209,   208,   -36,   230,   214,   215,   231,   184,   -37,
   205,   154,   218,   219,    17,    94,   183,   220,    48,    49,
    50,   221,   103,   165,     0,   129,     0,     0,     0,     0,
     0,   228,     0,    51,    52,    53,    54,    55,    56,    57,
    58,    59,    60,    61,    62,    63,    64,    65,     0,    66,
    67,    68,    69,    70,    48,    49,    50,     0,    71,     0,
    72,     0,     0,     0,     0,     0,     0,     0,     0,    51,
    52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
    62,    63,    64,    65,     0,    66,    67,    68,    69,    70,
     0,     0,     0,     0,    71,     0,    72,    51,    52,    53,
    54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
    64,    65,     0,    66,    67
};

static const short yycheck[] = {     5,
     6,     7,    80,    43,    10,    11,    12,    81,   135,    37,
     3,    44,    18,    46,     3,    46,    43,     3,     7,     8,
     3,    54,    53,    54,    30,    43,    32,     3,     3,   156,
    36,    37,     8,     9,    10,    11,    12,    43,   127,     3,
     5,     6,    48,    49,    50,    51,    52,    53,    54,    55,
    56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
    66,    67,    68,    69,    70,   139,    94,    53,    51,     1,
    45,    13,   161,    79,    50,     3,     8,     8,    84,     7,
    44,     8,    88,    89,    90,    91,    51,    52,    94,     3,
     4,     3,    98,     0,     1,     7,    44,   191,   104,   105,
    45,     8,     9,    10,    11,    12,    45,    14,    15,    16,
    41,    21,    44,     8,     9,    10,    11,    12,     8,     9,
    10,    11,    12,    10,   218,   131,   132,   221,    40,     1,
     1,   137,     3,     3,    44,    45,    46,     8,     8,    43,
    44,   147,     3,    50,    54,   223,     3,     8,    45,    44,
     1,     8,   158,   159,    43,    50,     1,    46,    47,    48,
    50,    43,   168,    44,    46,    47,    48,    43,    44,    45,
   176,    17,    18,    19,    20,    44,   182,     9,    10,    11,
    12,    45,    46,    43,   190,    44,    46,   193,    43,     1,
    45,    43,    46,    43,     0,    44,   202,     0,   151,    43,
   191,    92,   208,   209,     3,    36,   150,   213,     7,     8,
     9,   217,    45,   133,    -1,    73,    -1,    -1,    -1,    -1,
    -1,   227,    -1,    22,    23,    24,    25,    26,    27,    28,
    29,    30,    31,    32,    33,    34,    35,    36,    -1,    38,
    39,    40,    41,    42,     7,     8,     9,    -1,    47,    -1,
    49,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    22,
    23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
    33,    34,    35,    36,    -1,    38,    39,    40,    41,    42,
    -1,    -1,    -1,    -1,    47,    -1,    49,    22,    23,    24,
    25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
    35,    36,    -1,    38,    39
};
#define YYPURE 1

/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
#line 3 "/usr/share/bison.simple"
/* This file comes from bison-1.28.  */

/* Skeleton output parser for bison,
   Copyright (C) 1984, 1989, 1990 Free Software Foundation, Inc.

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

/* This is the parser code that is written into each bison parser
  when the %semantic_parser declaration is not specified in the grammar.
  It was written by Richard Stallman by simplifying the hairy parser
  used when %semantic_parser is specified.  */

#ifndef YYPARSE_RETURN_TYPE
#define YYPARSE_RETURN_TYPE int
#endif


#ifndef YYSTACK_USE_ALLOCA
#ifdef alloca
#define YYSTACK_USE_ALLOCA
#else /* alloca not defined */
#ifdef __GNUC__
#define YYSTACK_USE_ALLOCA
#define alloca __builtin_alloca
#else /* not GNU C.  */
#if (!defined (__STDC__) && defined (sparc)) || defined (__sparc__) || defined (__sparc) || defined (__sgi) || (defined (__sun) && defined (__i386))
#define YYSTACK_USE_ALLOCA
#include <alloca.h>
#else /* not sparc */
/* We think this test detects Watcom and Microsoft C.  */
/* This used to test MSDOS, but that is a bad idea
   since that symbol is in the user namespace.  */
#if (defined (_MSDOS) || defined (_MSDOS_)) && !defined (__TURBOC__)
#if 0 /* No need for malloc.h, which pollutes the namespace;
	 instead, just don't use alloca.  */
#include <malloc.h>
#endif
#else /* not MSDOS, or __TURBOC__ */
#if defined(_AIX)
/* I don't know what this was needed for, but it pollutes the namespace.
   So I turned it off.   rms, 2 May 1997.  */
/* #include <malloc.h>  */
 #pragma alloca
#define YYSTACK_USE_ALLOCA
#else /* not MSDOS, or __TURBOC__, or _AIX */
#if 0
#ifdef __hpux /* haible@ilog.fr says this works for HPUX 9.05 and up,
		 and on HPUX 10.  Eventually we can turn this on.  */
#define YYSTACK_USE_ALLOCA
#define alloca __builtin_alloca
#endif /* __hpux */
#endif
#endif /* not _AIX */
#endif /* not MSDOS, or __TURBOC__ */
#endif /* not sparc */
#endif /* not GNU C */
#endif /* alloca not defined */
#endif /* YYSTACK_USE_ALLOCA not defined */

#ifdef YYSTACK_USE_ALLOCA
#define YYSTACK_ALLOC alloca
#else
#define YYSTACK_ALLOC malloc
#endif

/* Note: there must be only one dollar sign in this file.
   It is replaced by the list of actions, each action
   as one case of the switch.  */

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		-2
#define YYEOF		0
#define YYACCEPT	goto yyacceptlab
#define YYABORT 	goto yyabortlab
#define YYERROR		goto yyerrlab1
/* Like YYERROR except do call yyerror.
   This remains here temporarily to ease the
   transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */
#define YYFAIL		goto yyerrlab
#define YYRECOVERING()  (!!yyerrstatus)
#define YYBACKUP(token, value) \
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    { yychar = (token), yylval = (value);			\
      yychar1 = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { yyerror ("syntax error: cannot back up"); YYERROR; }	\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

#ifndef YYPURE
#define YYLEX		yylex()
#endif

#ifdef YYPURE
#ifdef YYLSP_NEEDED
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, &yylloc, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval, &yylloc)
#endif
#else /* not YYLSP_NEEDED */
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval)
#endif
#endif /* not YYLSP_NEEDED */
#endif

/* If nonreentrant, generate the variables here */

#ifndef YYPURE

int	yychar;			/*  the lookahead symbol		*/
YYSTYPE	yylval;			/*  the semantic value of the		*/
				/*  lookahead symbol			*/

#ifdef YYLSP_NEEDED
YYLTYPE yylloc;			/*  location data for the lookahead	*/
				/*  symbol				*/
#endif

int yynerrs;			/*  number of parse errors so far       */
#endif  /* not YYPURE */

#if YYDEBUG != 0
int yydebug;			/*  nonzero means print parse trace	*/
/* Since this is uninitialized, it does not stop multiple parsers
   from coexisting.  */
#endif

/*  YYINITDEPTH indicates the initial size of the parser's stacks	*/

#ifndef	YYINITDEPTH
#define YYINITDEPTH 200
#endif

/*  YYMAXDEPTH is the maximum size the stacks can grow to
    (effective only if the built-in stack extension method is used).  */

#if YYMAXDEPTH == 0
#undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
#define YYMAXDEPTH 10000
#endif

/* Define __yy_memcpy.  Note that the size argument
   should be passed with type unsigned int, because that is what the non-GCC
   definitions require.  With GCC, __builtin_memcpy takes an arg
   of type size_t, but it can handle unsigned int.  */

#if __GNUC__ > 1		/* GNU C and GNU C++ define this.  */
#define __yy_memcpy(TO,FROM,COUNT)	__builtin_memcpy(TO,FROM,COUNT)
#else				/* not GNU C or C++ */
#ifndef __cplusplus

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (to, from, count)
     char *to;
     char *from;
     unsigned int count;
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#else /* __cplusplus */

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (char *to, char *from, unsigned int count)
{
  register char *t = to;
  register char *f = from;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#endif
#endif

#line 222 "/usr/share/bison.simple"

/* The user can define YYPARSE_PARAM as the name of an argument to be passed
   into yyparse.  The argument should have type void *.
   It should actually point to an object.
   Grammar actions can access the variable by casting it
   to the proper pointer type.  */

#ifdef YYPARSE_PARAM
#ifdef __cplusplus
#define YYPARSE_PARAM_ARG void *YYPARSE_PARAM
#define YYPARSE_PARAM_DECL
#else /* not __cplusplus */
#define YYPARSE_PARAM_ARG YYPARSE_PARAM
#define YYPARSE_PARAM_DECL void *YYPARSE_PARAM;
#endif /* not __cplusplus */
#else /* not YYPARSE_PARAM */
#define YYPARSE_PARAM_ARG
#define YYPARSE_PARAM_DECL
#endif /* not YYPARSE_PARAM */

/* Prevent warning if -Wstrict-prototypes.  */
#ifdef __GNUC__
#ifdef YYPARSE_PARAM
YYPARSE_RETURN_TYPE
yyparse (void *);
#else
YYPARSE_RETURN_TYPE
yyparse (void);
#endif
#endif

YYPARSE_RETURN_TYPE
yyparse(YYPARSE_PARAM_ARG)
     YYPARSE_PARAM_DECL
{
  register int yystate;
  register int yyn;
  register short *yyssp;
  register YYSTYPE *yyvsp;
  int yyerrstatus;	/*  number of tokens to shift before error messages enabled */
  int yychar1 = 0;		/*  lookahead token as an internal (translated) token number */

  short	yyssa[YYINITDEPTH];	/*  the state stack			*/
  YYSTYPE yyvsa[YYINITDEPTH];	/*  the semantic value stack		*/

  short *yyss = yyssa;		/*  refer to the stacks thru separate pointers */
  YYSTYPE *yyvs = yyvsa;	/*  to allow yyoverflow to reallocate them elsewhere */

#ifdef YYLSP_NEEDED
  YYLTYPE yylsa[YYINITDEPTH];	/*  the location stack			*/
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;

#define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)
#else
#define YYPOPSTACK   (yyvsp--, yyssp--)
#endif

  int yystacksize = YYINITDEPTH;
#ifndef YYSTACK_USE_ALLOCA
  int yyfree_stacks = 0;
#endif

#ifdef YYPURE
  int yychar;
  YYSTYPE yylval;
  int yynerrs;
#ifdef YYLSP_NEEDED
  YYLTYPE yylloc;
#endif
#endif

  YYSTYPE yyval;		/*  the variable used to return		*/
				/*  semantic values from the action	*/
				/*  routines				*/

  int yylen;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Starting parse\n");
#endif

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss - 1;
  yyvsp = yyvs;
#ifdef YYLSP_NEEDED
  yylsp = yyls;
#endif

/* Push a new state, which is found in  yystate  .  */
/* In all cases, when you get here, the value and location stacks
   have just been pushed. so pushing a state here evens the stacks.  */
yynewstate:

  *++yyssp = yystate;

  if (yyssp >= yyss + yystacksize - 1)
    {
      /* Give user a chance to reallocate the stack */
      /* Use copies of these so that the &'s don't force the real ones into memory. */
      YYSTYPE *yyvs1 = yyvs;
      short *yyss1 = yyss;
#ifdef YYLSP_NEEDED
      YYLTYPE *yyls1 = yyls;
#endif

      /* Get the current used size of the three stacks, in elements.  */
      int size = yyssp - yyss + 1;

#ifdef yyoverflow
      /* Each stack pointer address is followed by the size of
	 the data in use in that stack, in bytes.  */
#ifdef YYLSP_NEEDED
      /* This used to be a conditional around just the two extra args,
	 but that might be undefined if yyoverflow is a macro.  */
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yyls1, size * sizeof (*yylsp),
		 &yystacksize);
#else
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yystacksize);
#endif

      yyss = yyss1; yyvs = yyvs1;
#ifdef YYLSP_NEEDED
      yyls = yyls1;
#endif
#else /* no yyoverflow */
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
	{
	  yyerror("parser stack overflow");
#ifndef YYSTACK_USE_ALLOCA
	  if (yyfree_stacks)
	    {
	      free (yyss);
	      free (yyvs);
#ifdef YYLSP_NEEDED
	      free (yyls);
#endif
	    }
#endif
	  return 2;
	}
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;
#ifndef YYSTACK_USE_ALLOCA
      yyfree_stacks = 1;
#endif
      yyss = (short *) YYSTACK_ALLOC (yystacksize * sizeof (*yyssp));
      __yy_memcpy ((char *)yyss, (char *)yyss1,
		   size * (unsigned int) sizeof (*yyssp));
      yyvs = (YYSTYPE *) YYSTACK_ALLOC (yystacksize * sizeof (*yyvsp));
      __yy_memcpy ((char *)yyvs, (char *)yyvs1,
		   size * (unsigned int) sizeof (*yyvsp));
#ifdef YYLSP_NEEDED
      yyls = (YYLTYPE *) YYSTACK_ALLOC (yystacksize * sizeof (*yylsp));
      __yy_memcpy ((char *)yyls, (char *)yyls1,
		   size * (unsigned int) sizeof (*yylsp));
#endif
#endif /* no yyoverflow */

      yyssp = yyss + size - 1;
      yyvsp = yyvs + size - 1;
#ifdef YYLSP_NEEDED
      yylsp = yyls + size - 1;
#endif

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Stack size increased to %d\n", yystacksize);
#endif

      if (yyssp >= yyss + yystacksize - 1)
	YYABORT;
    }

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Entering state %d\n", yystate);
#endif

  goto yybackup;
 yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* yychar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (yychar == YYEMPTY)
    {
#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Reading a token: ");
#endif
      yychar = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (yychar <= 0)		/* This means end of input. */
    {
      yychar1 = 0;
      yychar = YYEOF;		/* Don't call YYLEX any more */

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Now at end of input.\n");
#endif
    }
  else
    {
      yychar1 = YYTRANSLATE(yychar);

#if YYDEBUG != 0
      if (yydebug)
	{
	  fprintf (stderr, "Next token is %d (%s", yychar, yytname[yychar1]);
	  /* Give the individual parser a way to print the precise meaning
	     of a token, for further debugging info.  */
#ifdef YYPRINT
	  YYPRINT (stderr, yychar, yylval);
#endif
	  fprintf (stderr, ")\n");
	}
#endif
    }

  yyn += yychar1;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != yychar1)
    goto yydefault;

  yyn = yytable[yyn];

  /* yyn is what to do for this token type in this state.
     Negative => reduce, -yyn is rule number.
     Positive => shift, yyn is new state.
       New state is final state => don't bother to shift,
       just return success.
     0, or most negative number => error.  */

  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrlab;

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting token %d (%s), ", yychar, yytname[yychar1]);
#endif

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  /* count tokens shifted since error; after three, turn off error status.  */
  if (yyerrstatus) yyerrstatus--;

  yystate = yyn;
  goto yynewstate;

/* Do the default action for the current state.  */
yydefault:

  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;

/* Do a reduction.  yyn is the number of a rule to reduce with.  */
yyreduce:
  yylen = yyr2[yyn];
  if (yylen > 0)
    yyval = yyvsp[1-yylen]; /* implement default value of the action */

#if YYDEBUG != 0
  if (yydebug)
    {
      int i;

      fprintf (stderr, "Reducing via rule %d (line %d), ",
	       yyn, yyrline[yyn]);

      /* Print the symbols being reduced, and their result.  */
      for (i = yyprhs[yyn]; yyrhs[i] > 0; i++)
	fprintf (stderr, "%s ", yytname[yyrhs[i]]);
      fprintf (stderr, " -> %s\n", yytname[yyr1[yyn]]);
    }
#endif


  switch (yyn) {

case 5:
#line 225 "parser.y"
{
        CSSParser *p = static_cast<CSSParser *>(parser);
	p->rule = yyvsp[-2].rule;
    ;
    break;}
case 6:
#line 232 "parser.y"
{
	/* can be empty */
    ;
    break;}
case 7:
#line 238 "parser.y"
{
	CSSParser *p = static_cast<CSSParser *>(parser);
	if ( yyvsp[-1].valueList ) {
	    p->valueList = yyvsp[-1].valueList;
#ifdef CSS_DEBUG
	    kdDebug( 6080 ) << "   got property for " << p->id <<
		(p->important?" important":"")<< endl;
	    bool ok =
#endif
		p->parseValue( p->id, p->important );
#ifdef CSS_DEBUG
	    if ( !ok )
		kdDebug( 6080 ) << "     couldn't parse value!" << endl;
#endif
	}
#ifdef CSS_DEBUG
	else
	    kdDebug( 6080 ) << "     no value found!" << endl;
#endif
	delete p->valueList;
	p->valueList = 0;
    ;
    break;}
case 14:
#line 275 "parser.y"
{
#ifdef CSS_DEBUG
     kdDebug( 6080 ) << "charset rule: " << qString(yyvsp[-2].string) << endl;
#endif
 ;
    break;}
case 16:
#line 284 "parser.y"
{
     CSSParser *p = static_cast<CSSParser *>(parser);
     if ( yyvsp[-1].rule && p->styleElement && p->styleElement->isCSSStyleSheet() ) {
	 p->styleElement->append( yyvsp[-1].rule );
     } else {
	 delete yyvsp[-1].rule;
     }
 ;
    break;}
case 18:
#line 297 "parser.y"
{
     CSSParser *p = static_cast<CSSParser *>(parser);
     if ( yyvsp[-1].rule && p->styleElement && p->styleElement->isCSSStyleSheet() ) {
	 p->styleElement->append( yyvsp[-1].rule );
     } else {
	 delete yyvsp[-1].rule;
     }
 ;
    break;}
case 24:
#line 316 "parser.y"
{
	yyval.rule = 0;
#ifdef CSS_DEBUG
	kdDebug( 6080 ) << "skipped invalid rule" << endl;
#endif
    ;
    break;}
case 25:
#line 322 "parser.y"
{
	yyval.rule = 0;
#ifdef CSS_DEBUG
	kdDebug( 6080 ) << "skipped invalid rule" << endl;
#endif
    ;
    break;}
case 26:
#line 328 "parser.y"
{
	yyval.rule = 0;
#ifdef CSS_DEBUG
	kdDebug( 6080 ) << "skipped invalid rule" << endl;
#endif
    ;
    break;}
case 29:
#line 342 "parser.y"
{
#ifdef CSS_DEBUG
	kdDebug( 6080 ) << "@import: " << qString(yyvsp[-3].string) << endl;
#endif
	CSSParser *p = static_cast<CSSParser *>(parser);
	if ( p->styleElement && p->styleElement->isCSSStyleSheet() )
	    yyval.rule = new CSSImportRuleImpl( p->styleElement, domString(yyvsp[-3].string), yyvsp[-1].mediaList );
	else
	    yyval.rule = 0;
    ;
    break;}
case 32:
#line 360 "parser.y"
{
	yyval.mediaList = 0;
    ;
    break;}
case 33:
#line 363 "parser.y"
{
	yyval.mediaList = new MediaListImpl();
	yyval.mediaList->appendMedium( domString(yyvsp[0].string) );
    ;
    break;}
case 34:
#line 367 "parser.y"
{
	yyval.mediaList = yyvsp[-3].mediaList;
	if ( !yyval.mediaList ) yyval.mediaList = new MediaListImpl();
	yyval.mediaList->appendMedium( domString(yyvsp[0].string) );
    ;
    break;}
case 35:
#line 375 "parser.y"
{
	CSSParser *p = static_cast<CSSParser *>(parser);
	if ( yyvsp[-4].mediaList && yyvsp[-1].ruleList &&
	     p->styleElement && p->styleElement->isCSSStyleSheet() ) {
	    yyval.rule = new CSSMediaRuleImpl( static_cast<CSSStyleSheetImpl*>(p->styleElement), yyvsp[-4].mediaList, yyvsp[-1].ruleList );
	} else {
	    yyval.rule = 0;
	    delete yyvsp[-4].mediaList;
	    delete yyvsp[-1].ruleList;
	}
    ;
    break;}
case 36:
#line 389 "parser.y"
{
	yyval.mediaList = new MediaListImpl();
	yyval.mediaList->appendMedium( domString(yyvsp[0].string) );
    ;
    break;}
case 37:
#line 393 "parser.y"
{
	yyval.mediaList = yyvsp[-3].mediaList;
	yyval.mediaList->appendMedium( domString(yyvsp[0].string) );
    ;
    break;}
case 38:
#line 401 "parser.y"
{ yyval.ruleList = 0; ;
    break;}
case 39:
#line 402 "parser.y"
{
      yyval.ruleList = yyvsp[-2].ruleList;
      if ( yyvsp[-1].rule ) {
	  if ( !yyval.ruleList ) yyval.ruleList = new CSSRuleListImpl();
	  yyval.ruleList->append( yyvsp[-1].rule );
      }
  ;
    break;}
case 40:
#line 412 "parser.y"
{
      yyval.string = yyvsp[-1].string;
  ;
    break;}
case 41:
#line 434 "parser.y"
{
      yyval.rule = 0;
    ;
    break;}
case 42:
#line 440 "parser.y"
{
	yyval.rule = 0;
    ;
    break;}
case 43:
#line 446 "parser.y"
{ yyval.relation = CSSSelector::Sibling; ;
    break;}
case 44:
#line 447 "parser.y"
{ yyval.relation = CSSSelector::Child; ;
    break;}
case 45:
#line 448 "parser.y"
{ yyval.relation = CSSSelector::Descendant; ;
    break;}
case 46:
#line 452 "parser.y"
{ yyval.val = -1; ;
    break;}
case 47:
#line 453 "parser.y"
{ yyval.val = 1; ;
    break;}
case 48:
#line 457 "parser.y"
{
#ifdef CSS_DEBUG
	kdDebug( 6080 ) << "got ruleset" << endl << "  selector:" << endl;
#endif
	CSSParser *p = static_cast<CSSParser *>(parser);
	if ( yyvsp[-4].selectorList && yyvsp[-1].ok && p->numParsedProperties ) {
	    CSSStyleRuleImpl *rule = new CSSStyleRuleImpl( p->styleElement );
	    CSSStyleDeclarationImpl *decl = p->createStyleDeclaration( rule );
	    rule->setSelector( yyvsp[-4].selectorList );
	    rule->setDeclaration(decl);
	    yyval.rule = rule;
	} else {
	    yyval.rule = 0;
	    delete yyvsp[-4].selectorList;
	    p->clearProperties();
	}
    ;
    break;}
case 49:
#line 477 "parser.y"
{
	if ( yyvsp[0].selector ) {
	    yyval.selectorList = new QPtrList<CSSSelector>;
#ifdef CSS_DEBUG
	    kdDebug( 6080 ) << "   got simple selector:" << endl;
	    yyvsp[0].selector->print();
#endif
	    yyval.selectorList->append( yyvsp[0].selector );
	} else {
	    yyval.selectorList = 0;
	}
    ;
    break;}
case 50:
#line 489 "parser.y"
{
	yyval.selectorList = yyvsp[-3].selectorList;
	if ( yyvsp[0].selector ) {
	    if ( !yyval.selectorList ) yyval.selectorList = new QPtrList<CSSSelector>;
	    yyval.selectorList->append( yyvsp[0].selector );
#ifdef CSS_DEBUG
	    kdDebug( 6080 ) << "   got simple selector:" << endl;
	    yyvsp[0].selector->print();
#endif
	}
    ;
    break;}
case 51:
#line 503 "parser.y"
{
	yyval.selector = yyvsp[0].selector;
    ;
    break;}
case 52:
#line 506 "parser.y"
{
	yyval.selector = yyvsp[0].selector;
	yyval.selector->relation = yyvsp[-1].relation;
	yyval.selector->tagHistory = yyvsp[-2].selector;
    ;
    break;}
case 53:
#line 514 "parser.y"
{
	yyval.selector = new CSSSelector();
	yyval.selector->tag = yyvsp[-1].element;
    ;
    break;}
case 54:
#line 518 "parser.y"
{
	yyval.selector = yyvsp[-1].selector;
	yyval.selector->tag = yyvsp[-2].element;
    ;
    break;}
case 55:
#line 522 "parser.y"
{
	yyval.selector = yyvsp[-1].selector;
	yyval.selector->tag = -1;
    ;
    break;}
case 56:
#line 529 "parser.y"
{
	CSSParser *p = static_cast<CSSParser *>(parser);
	DOM::DocumentImpl *doc = p->document();
	QString tag = qString(yyvsp[0].string);
	if ( doc ) {
	    if (doc->isHTMLDocument())
		tag = tag.lower();
	    const DOMString dtag(tag);
	    yyval.element = doc->tagId(0, dtag.implementation(), false);
	} else {
	    yyval.element = khtml::getTagID(tag.lower().ascii(), tag.length());
	    // this case should never happen - only when loading
	    // the default stylesheet - which must not contain unknown tags
// 	    assert($$ != 0);
	}
    ;
    break;}
case 57:
#line 545 "parser.y"
{
	yyval.element = -1;
    ;
    break;}
case 58:
#line 551 "parser.y"
{
	yyval.selector = yyvsp[0].selector;
	yyval.selector->nonCSSHint = static_cast<CSSParser *>(parser)->nonCSSHint;
    ;
    break;}
case 59:
#line 555 "parser.y"
{
	yyval.selector = yyvsp[-1].selector;
	yyval.selector->nonCSSHint = static_cast<CSSParser *>(parser)->nonCSSHint;
	yyval.selector->relation = CSSSelector::SubSelector;
	yyval.selector->tagHistory = yyvsp[0].selector;
    ;
    break;}
case 60:
#line 564 "parser.y"
{
	yyval.selector = new CSSSelector();
	yyval.selector->match = CSSSelector::Id;
	yyval.selector->attr = ATTR_ID;
	yyval.selector->value = domString(yyvsp[0].string);
    ;
    break;}
case 64:
#line 576 "parser.y"
{
	yyval.selector = new CSSSelector();
	yyval.selector->match = CSSSelector::List;
	yyval.selector->attr = ATTR_CLASS;
	yyval.selector->value = domString(yyvsp[0].string);
    ;
    break;}
case 65:
#line 585 "parser.y"
{
	CSSParser *p = static_cast<CSSParser *>(parser);
	DOM::DocumentImpl *doc = p->document();

	QString attr = qString(yyvsp[-1].string);
	if ( doc ) {
	    if (doc->isHTMLDocument())
		attr = attr.lower();
	    const DOMString dattr(attr);
	    yyval.attribute = doc->attrId(0, dattr.implementation(), false);
	} else {
	    yyval.attribute = khtml::getAttrID(attr.lower().ascii(), attr.length());
	    // this case should never happen - only when loading
	    // the default stylesheet - which must not contain unknown attributes
	    assert(yyval.attribute != 0);
	    }
    ;
    break;}
case 66:
#line 605 "parser.y"
{
	yyval.selector = new CSSSelector();
	yyval.selector->attr = yyvsp[-1].attribute;
	yyval.selector->match = CSSSelector::Set;
    ;
    break;}
case 67:
#line 610 "parser.y"
{
	yyval.selector = new CSSSelector();
	yyval.selector->attr = yyvsp[-5].attribute;
	yyval.selector->match = (CSSSelector::Match)yyvsp[-4].val;
	yyval.selector->value = domString(yyvsp[-2].string);
    ;
    break;}
case 68:
#line 619 "parser.y"
{
	yyval.val = CSSSelector::Exact;
    ;
    break;}
case 69:
#line 622 "parser.y"
{
	yyval.val = CSSSelector::List;
    ;
    break;}
case 70:
#line 625 "parser.y"
{
	yyval.val = CSSSelector::Hyphen; /* ### ??? */
    ;
    break;}
case 73:
#line 636 "parser.y"
{
	yyval.selector = new CSSSelector();
	yyval.selector->match = CSSSelector::Pseudo;
	yyval.selector->value = domString(yyvsp[0].string);
    ;
    break;}
case 74:
#line 641 "parser.y"
{
	yyval.selector = new CSSSelector();
	yyval.selector->match = CSSSelector::Pseudo;
	yyval.selector->_pseudoType = CSSSelector::PseudoFunction;
	yyval.selector->value = domString(yyvsp[-2].string);
    ;
    break;}
case 75:
#line 650 "parser.y"
{
	yyval.ok = yyvsp[0].ok;
    ;
    break;}
case 76:
#line 653 "parser.y"
{
	yyval.ok = yyvsp[-1].ok;
	if ( yyvsp[0].ok )
	    yyval.ok = yyvsp[0].ok;
    ;
    break;}
case 77:
#line 658 "parser.y"
{
	yyval.ok = yyvsp[0].ok;
    ;
    break;}
case 78:
#line 661 "parser.y"
{
	yyval.ok = false;
#ifdef CSS_DEBUG
	kdDebug( 6080 ) << "skipping bogus declaration" << endl;
#endif
    ;
    break;}
case 79:
#line 670 "parser.y"
{
	yyval.ok = yyvsp[-2].ok;
    ;
    break;}
case 80:
#line 673 "parser.y"
{
	yyval.ok = false;
#ifdef CSS_DEBUG
	kdDebug( 6080 ) << "skipping bogus declaration" << endl;
#endif
    ;
    break;}
case 81:
#line 679 "parser.y"
{
	yyval.ok = yyvsp[-3].ok;
	if ( yyvsp[-2].ok )
	    yyval.ok = yyvsp[-2].ok;
    ;
    break;}
case 82:
#line 684 "parser.y"
{
	yyval.ok = yyvsp[-3].ok;
#ifdef CSS_DEBUG
	kdDebug( 6080 ) << "skipping bogus declaration" << endl;
#endif
    ;
    break;}
case 83:
#line 693 "parser.y"
{
	yyval.ok = false;
	CSSParser *p = static_cast<CSSParser *>(parser);
	if ( yyvsp[-4].prop_id && yyvsp[-1].valueList ) {
	    p->valueList = yyvsp[-1].valueList;
#ifdef CSS_DEBUG
	    kdDebug( 6080 ) << "   got property: " << yyvsp[-4].prop_id <<
		(yyvsp[0].b?" important":"")<< endl;
#endif
	    bool ok = p->parseValue( yyvsp[-4].prop_id, yyvsp[0].b );
	    if ( ok )
		yyval.ok = ok;
#ifdef CSS_DEBUG
	    else
		kdDebug( 6080 ) << "     couldn't parse value!" << endl;
#endif
	}
	delete p->valueList;
	p->valueList = 0;
    ;
    break;}
case 84:
#line 716 "parser.y"
{
	QString str = qString(yyvsp[-1].string);
	yyval.prop_id = getPropertyID( str.lower().latin1(), str.length() );
    ;
    break;}
case 85:
#line 723 "parser.y"
{ yyval.b = true; ;
    break;}
case 86:
#line 724 "parser.y"
{ yyval.b = false; ;
    break;}
case 87:
#line 728 "parser.y"
{
	yyval.valueList = new ValueList;
	yyval.valueList->addValue( yyvsp[0].value );
    ;
    break;}
case 88:
#line 732 "parser.y"
{
	yyval.valueList = yyvsp[-2].valueList;
	if ( yyvsp[-1].tok ) {
	    Value v;
	    v.id = 0;
	    v.unit = Value::Operator;
	    v.iValue = yyvsp[-1].tok;
	    yyval.valueList->addValue( v );
	}
	yyval.valueList->addValue( yyvsp[0].value );
    ;
    break;}
case 89:
#line 746 "parser.y"
{
	yyval.tok = '/';
    ;
    break;}
case 90:
#line 749 "parser.y"
{
	yyval.tok = ',';
    ;
    break;}
case 91:
#line 752 "parser.y"
{
        yyval.tok = 0;
  ;
    break;}
case 92:
#line 758 "parser.y"
{ yyval.value = yyvsp[0].value; ;
    break;}
case 93:
#line 759 "parser.y"
{ yyval.value = yyvsp[0].value; yyval.value.fValue *= yyvsp[-1].val; ;
    break;}
case 94:
#line 760 "parser.y"
{ yyval.value.id = 0; yyval.value.string = yyvsp[-1].string; yyval.value.unit = CSSPrimitiveValue::CSS_STRING; ;
    break;}
case 95:
#line 761 "parser.y"
{
      QString str = qString( yyvsp[-1].string );
      yyval.value.id = getValueID( str.lower().latin1(), str.length() );
      yyval.value.unit = CSSPrimitiveValue::CSS_IDENT;
      yyval.value.string = yyvsp[-1].string;
  ;
    break;}
case 96:
#line 767 "parser.y"
{ yyval.value.id = 0; yyval.value.string = yyvsp[-1].string; yyval.value.unit = CSSPrimitiveValue::CSS_URI; ;
    break;}
case 97:
#line 768 "parser.y"
{ yyval.value.id = 0; yyval.value.iValue = 0; yyval.value.unit = CSSPrimitiveValue::CSS_UNKNOWN;/* ### */ ;
    break;}
case 98:
#line 769 "parser.y"
{ yyval.value.id = 0; yyval.value.string = yyvsp[0].string; yyval.value.unit = CSSPrimitiveValue::CSS_RGBCOLOR; ;
    break;}
case 99:
#line 771 "parser.y"
{
      yyval.value = yyvsp[0].value;
  ;
    break;}
case 100:
#line 777 "parser.y"
{ yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_NUMBER; ;
    break;}
case 101:
#line 778 "parser.y"
{ yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_PERCENTAGE; ;
    break;}
case 102:
#line 779 "parser.y"
{ yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_PX; ;
    break;}
case 103:
#line 780 "parser.y"
{ yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_CM; ;
    break;}
case 104:
#line 781 "parser.y"
{ yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_MM; ;
    break;}
case 105:
#line 782 "parser.y"
{ yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_IN; ;
    break;}
case 106:
#line 783 "parser.y"
{ yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_PT; ;
    break;}
case 107:
#line 784 "parser.y"
{ yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_PC; ;
    break;}
case 108:
#line 785 "parser.y"
{ yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_DEG; ;
    break;}
case 109:
#line 786 "parser.y"
{ yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_RAD; ;
    break;}
case 110:
#line 787 "parser.y"
{ yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_GRAD; ;
    break;}
case 111:
#line 788 "parser.y"
{ yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_MS; ;
    break;}
case 112:
#line 789 "parser.y"
{ yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_S; ;
    break;}
case 113:
#line 790 "parser.y"
{ yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_HZ; ;
    break;}
case 114:
#line 791 "parser.y"
{ yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_KHZ; ;
    break;}
case 115:
#line 792 "parser.y"
{ yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_EMS; ;
    break;}
case 116:
#line 793 "parser.y"
{ yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_EXS; ;
    break;}
case 117:
#line 798 "parser.y"
{
      Function *f = new Function;
      f->name = yyvsp[-4].string;
      f->args = yyvsp[-2].valueList;
      yyval.value.id = 0;
      yyval.value.unit = Value::Function;
      yyval.value.function = f;
  ;
    break;}
case 118:
#line 813 "parser.y"
{ yyval.string = yyvsp[-1].string; ;
    break;}
}
   /* the action file gets copied in in place of this dollarsign */
#line 554 "/usr/share/bison.simple"

  yyvsp -= yylen;
  yyssp -= yylen;
#ifdef YYLSP_NEEDED
  yylsp -= yylen;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;

#ifdef YYLSP_NEEDED
  yylsp++;
  if (yylen == 0)
    {
      yylsp->first_line = yylloc.first_line;
      yylsp->first_column = yylloc.first_column;
      yylsp->last_line = (yylsp-1)->last_line;
      yylsp->last_column = (yylsp-1)->last_column;
      yylsp->text = 0;
    }
  else
    {
      yylsp->last_line = (yylsp+yylen-1)->last_line;
      yylsp->last_column = (yylsp+yylen-1)->last_column;
    }
#endif

  /* Now "shift" the result of the reduction.
     Determine what state that goes to,
     based on the state we popped back to
     and the rule number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
  if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTBASE];

  goto yynewstate;

yyerrlab:   /* here on detecting error */

  if (! yyerrstatus)
    /* If not already recovering from an error, report this error.  */
    {
      ++yynerrs;

#ifdef YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (yyn > YYFLAG && yyn < YYLAST)
	{
	  int size = 0;
	  char *msg;
	  int x, count;

	  count = 0;
	  /* Start X at -yyn if nec to avoid negative indexes in yycheck.  */
	  for (x = (yyn < 0 ? -yyn : 0);
	       x < (sizeof(yytname) / sizeof(char *)); x++)
	    if (yycheck[x + yyn] == x)
	      size += strlen(yytname[x]) + 15, count++;
	  msg = (char *) malloc(size + 15);
	  if (msg != 0)
	    {
	      strcpy(msg, "parse error");

	      if (count < 5)
		{
		  count = 0;
		  for (x = (yyn < 0 ? -yyn : 0);
		       x < (sizeof(yytname) / sizeof(char *)); x++)
		    if (yycheck[x + yyn] == x)
		      {
			strcat(msg, count == 0 ? ", expecting `" : " or `");
			strcat(msg, yytname[x]);
			strcat(msg, "'");
			count++;
		      }
		}
	      yyerror(msg);
	      free(msg);
	    }
	  else
	    yyerror ("parse error; also virtual memory exceeded");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror("parse error");
    }

  goto yyerrlab1;
yyerrlab1:   /* here on error raised explicitly by an action */

  if (yyerrstatus == 3)
    {
      /* if just tried and failed to reuse lookahead token after an error, discard it.  */

      /* return failure if at end of input */
      if (yychar == YYEOF)
	YYABORT;

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Discarding token %d (%s).\n", yychar, yytname[yychar1]);
#endif

      yychar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token
     after shifting the error token.  */

  yyerrstatus = 3;		/* Each real token shifted decrements this */

  goto yyerrhandle;

yyerrdefault:  /* current state does not do anything special for the error token. */

#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */
  yyn = yydefact[yystate];  /* If its default is to accept any token, ok.  Otherwise pop it.*/
  if (yyn) goto yydefault;
#endif

yyerrpop:   /* pop the current state because it cannot handle the error token */

  if (yyssp == yyss) YYABORT;
  yyvsp--;
  yystate = *--yyssp;
#ifdef YYLSP_NEEDED
  yylsp--;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "Error: state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

yyerrhandle:

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yyerrdefault;

  yyn += YYTERROR;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != YYTERROR)
    goto yyerrdefault;

  yyn = yytable[yyn];
  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrpop;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrpop;

  if (yyn == YYFINAL)
    YYACCEPT;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting error token, ");
#endif

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  yystate = yyn;
  goto yynewstate;

 yyacceptlab:
  /* YYACCEPT comes here.  */
#ifndef YYSTACK_USE_ALLOCA
  if (yyfree_stacks)
    {
      free (yyss);
      free (yyvs);
#ifdef YYLSP_NEEDED
      free (yyls);
#endif
    }
#endif
  return 0;

 yyabortlab:
  /* YYABORT comes here.  */
#ifndef YYSTACK_USE_ALLOCA
  if (yyfree_stacks)
    {
      free (yyss);
      free (yyvs);
#ifdef YYLSP_NEEDED
      free (yyls);
#endif
    }
#endif
  return 1;
}
#line 816 "parser.y"


