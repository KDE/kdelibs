
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
#define	BEGINSWITH	261
#define	ENDSWITH	262
#define	CONTAINS	263
#define	STRING	264
#define	IDENT	265
#define	HASH	266
#define	IMPORT_SYM	267
#define	PAGE_SYM	268
#define	MEDIA_SYM	269
#define	FONT_FACE_SYM	270
#define	CHARSET_SYM	271
#define	NAMESPACE_SYM	272
#define	KONQ_RULE_SYM	273
#define	KONQ_DECLS_SYM	274
#define	KONQ_VALUE_SYM	275
#define	IMPORTANT_SYM	276
#define	QEMS	277
#define	EMS	278
#define	EXS	279
#define	PXS	280
#define	CMS	281
#define	MMS	282
#define	INS	283
#define	PTS	284
#define	PCS	285
#define	DEGS	286
#define	RADS	287
#define	GRADS	288
#define	MSECS	289
#define	SECS	290
#define	HERZ	291
#define	KHERZ	292
#define	DIMEN	293
#define	PERCENTAGE	294
#define	NUMBER	295
#define	URI	296
#define	FUNCTION	297
#define	UNICODERANGE	298

#line 1 "parser.y"


/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2002-2003 Lars Knoll (knoll@kde.org)
 *  Copyright (c) 2003 Apple Computer
 *  Copyright (C) 2003 Dirk Mueller (mueller@kde.org)
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

// turn off inlining to void warning with newer gcc
#undef __inline
#define __inline
#include "cssproperties.c"
#include "cssvalues.c"
#undef __inline

int DOM::getPropertyID(const char *tagStr, int len)
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

#line 85 "parser.y"
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
    unsigned int attribute;
    unsigned int element;
    unsigned int ns;
    CSSSelector::Relation relation;
    bool b;
    char tok;
    Value value;
    ValueList *valueList;
} YYSTYPE;
#line 106 "parser.y"


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



#define	YYFINAL		285
#define	YYFLAG		-32768
#define	YYNTBASE	62

#define YYTRANSLATE(x) ((unsigned)(x) <= 298 ? yytranslate[x] : 118)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
    59,    17,    54,    53,    56,    14,    60,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,    13,    52,     2,
    58,    55,     2,    61,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
    15,     2,    57,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,    50,    16,    51,     2,     2,     2,     2,     2,
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
     7,     8,     9,    10,    11,    12,    18,    19,    20,    21,
    22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
    32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
    42,    43,    44,    45,    46,    47,    48,    49
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     6,     9,    12,    15,    22,    28,    34,    35,    38,
    39,    42,    45,    46,    52,    56,    60,    61,    65,    72,
    76,    80,    81,    84,    91,    93,    94,    97,    98,   102,
   104,   106,   108,   110,   112,   114,   116,   118,   119,   121,
   123,   128,   131,   139,   140,   144,   147,   151,   155,   159,
   163,   166,   169,   170,   172,   174,   180,   182,   187,   190,
   192,   196,   199,   202,   206,   209,   212,   214,   216,   219,
   222,   224,   226,   228,   231,   234,   236,   238,   240,   242,
   245,   248,   250,   253,   258,   267,   269,   271,   273,   275,
   277,   279,   281,   283,   286,   290,   296,   298,   301,   303,
   307,   309,   313,   317,   323,   328,   333,   340,   346,   349,
   352,   353,   355,   359,   362,   365,   368,   369,   371,   374,
   377,   380,   383,   386,   388,   390,   393,   396,   399,   402,
   405,   408,   411,   414,   417,   420,   423,   426,   429,   432,
   435,   438,   441,   444,   447,   453,   456,   460,   464,   467,
   473,   477,   479
};

static const short yyrhs[] = {    68,
    67,    69,    71,    75,     0,    63,    66,     0,    64,    66,
     0,    65,    66,     0,    24,    50,    66,    87,    66,    51,
     0,    25,    50,    66,   103,    51,     0,    26,    50,    66,
   108,    51,     0,     0,    66,     3,     0,     0,    67,     4,
     0,    67,     3,     0,     0,    22,    66,    10,    66,    52,
     0,    22,     1,   116,     0,    22,     1,    52,     0,     0,
    69,    70,    67,     0,    18,    66,    77,    66,    78,    52,
     0,    18,     1,   116,     0,    18,     1,    52,     0,     0,
    72,    67,     0,    23,    66,    74,    77,    66,    52,     0,
    11,     0,     0,    73,    66,     0,     0,    75,    76,    67,
     0,    87,     0,    80,     0,    83,     0,    84,     0,   115,
     0,   114,     0,    10,     0,    47,     0,     0,    79,     0,
    82,     0,    79,    53,    66,    82,     0,    79,     1,     0,
    20,    66,    79,    50,    66,    81,    51,     0,     0,    81,
    87,    66,     0,    11,    66,     0,    19,     1,   116,     0,
    19,     1,    52,     0,    21,     1,   116,     0,    21,     1,
    52,     0,    54,    66,     0,    55,    66,     0,     0,    56,
     0,    54,     0,    88,    50,    66,   103,    51,     0,    89,
     0,    88,    53,    66,    89,     0,    88,     1,     0,    90,
     0,    89,    85,    90,     0,    89,     1,     0,    91,    66,
     0,    91,    94,    66,     0,    94,    66,     0,    92,    93,
     0,    93,     0,    16,     0,    11,    16,     0,    17,    16,
     0,    11,     0,    17,     0,    95,     0,    94,    95,     0,
    94,     1,     0,    12,     0,    96,     0,    99,     0,   102,
     0,    14,    11,     0,    92,    98,     0,    98,     0,    11,
    66,     0,    15,    66,    97,    57,     0,    15,    66,    97,
   100,    66,   101,    66,    57,     0,    58,     0,     5,     0,
     6,     0,     7,     0,     8,     0,     9,     0,    11,     0,
    10,     0,    13,    11,     0,    13,    13,    11,     0,    13,
    48,    66,    90,    59,     0,   105,     0,   104,   105,     0,
   104,     0,     1,   117,     1,     0,     1,     0,   105,    52,
    66,     0,     1,    52,    66,     0,     1,   117,     1,    52,
    66,     0,   104,   105,    52,    66,     0,   104,     1,    52,
    66,     0,   104,     1,   117,     1,    52,    66,     0,   106,
    13,    66,   108,   107,     0,    11,    66,     0,    27,    66,
     0,     0,   110,     0,   108,   109,   110,     0,   108,     1,
     0,    60,    66,     0,    53,    66,     0,     0,   111,     0,
    86,   111,     0,    10,    66,     0,    11,    66,     0,    47,
    66,     0,    49,    66,     0,   113,     0,   112,     0,    46,
    66,     0,    45,    66,     0,    31,    66,     0,    32,    66,
     0,    33,    66,     0,    34,    66,     0,    35,    66,     0,
    36,    66,     0,    37,    66,     0,    38,    66,     0,    39,
    66,     0,    40,    66,     0,    41,    66,     0,    42,    66,
     0,    43,    66,     0,    29,    66,     0,    28,    66,     0,
    30,    66,     0,    44,    66,     0,    48,    66,   108,    59,
    66,     0,    12,    66,     0,    61,     1,   116,     0,    61,
     1,    52,     0,     1,   116,     0,    50,     1,   117,     1,
    51,     0,    50,     1,    51,     0,   116,     0,   117,     1,
   116,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
   240,   242,   243,   244,   247,   254,   260,   285,   287,   290,
   292,   293,   296,   298,   303,   304,   307,   309,   319,   330,
   333,   339,   340,   344,   348,   351,   353,   356,   358,   368,
   370,   371,   372,   373,   374,   377,   379,   382,   386,   390,
   395,   399,   405,   419,   421,   430,   452,   456,   461,   465,
   470,   472,   473,   476,   478,   481,   501,   515,   529,   535,
   539,   560,   566,   571,   576,   583,   585,   593,   595,   596,
   600,   621,   624,   629,   639,   645,   652,   653,   654,   657,
   666,   668,   674,   698,   704,   712,   716,   719,   722,   725,
   728,   733,   735,   738,   744,   750,   758,   762,   767,   770,
   776,   784,   788,   794,   800,   805,   811,   819,   844,   851,
   853,   856,   861,   874,   880,   884,   887,   892,   894,   895,
   896,   902,   903,   904,   906,   911,   913,   914,   915,   916,
   917,   918,   919,   920,   921,   922,   923,   924,   925,   926,
   927,   928,   929,   930,   934,   949,   956,   963,   971,   997,
   999,  1002,  1004
};
#endif


#if YYDEBUG != 0 || defined (YYERROR_VERBOSE)

static const char * const yytname[] = {   "$","error","$undefined.","S","SGML_CD",
"INCLUDES","DASHMATCH","BEGINSWITH","ENDSWITH","CONTAINS","STRING","IDENT","HASH",
"':'","'.'","'['","'|'","'*'","IMPORT_SYM","PAGE_SYM","MEDIA_SYM","FONT_FACE_SYM",
"CHARSET_SYM","NAMESPACE_SYM","KONQ_RULE_SYM","KONQ_DECLS_SYM","KONQ_VALUE_SYM",
"IMPORTANT_SYM","QEMS","EMS","EXS","PXS","CMS","MMS","INS","PTS","PCS","DEGS",
"RADS","GRADS","MSECS","SECS","HERZ","KHERZ","DIMEN","PERCENTAGE","NUMBER","URI",
"FUNCTION","UNICODERANGE","'{'","'}'","';'","','","'+'","'>'","'-'","']'","'='",
"')'","'/'","'@'","stylesheet","konq_rule","konq_decls","konq_value","maybe_space",
"maybe_sgml","maybe_charset","import_list","import","maybe_namespace","namespace",
"ns_prefix","maybe_ns_prefix","rule_list","rule","string_or_uri","maybe_media_list",
"media_list","media","ruleset_list","medium","page","font_face","combinator",
"unary_operator","ruleset","selector_list","selector","simple_selector","ns_element",
"ns_selector","element_name","specifier_list","specifier","class","ns_attrib_id",
"attrib_id","attrib","match","ident_or_string","pseudo","declaration_list","decl_list",
"declaration","property","prio","expr","operator","term","unary_term","function",
"hexcolor","invalid_at","invalid_rule","invalid_block","invalid_block_list", NULL
};
#endif

static const short yyr1[] = {     0,
    62,    62,    62,    62,    63,    64,    65,    66,    66,    67,
    67,    67,    68,    68,    68,    68,    69,    69,    70,    70,
    70,    71,    71,    72,    73,    74,    74,    75,    75,    76,
    76,    76,    76,    76,    76,    77,    77,    78,    78,    79,
    79,    79,    80,    81,    81,    82,    83,    83,    84,    84,
    85,    85,    85,    86,    86,    87,    88,    88,    88,    89,
    89,    89,    90,    90,    90,    91,    91,    92,    92,    92,
    93,    93,    94,    94,    94,    95,    95,    95,    95,    96,
    97,    97,    98,    99,    99,   100,   100,   100,   100,   100,
   100,   101,   101,   102,   102,   102,   103,   103,   103,   103,
   103,   104,   104,   104,   104,   104,   104,   105,   106,   107,
   107,   108,   108,   108,   109,   109,   109,   110,   110,   110,
   110,   110,   110,   110,   110,   111,   111,   111,   111,   111,
   111,   111,   111,   111,   111,   111,   111,   111,   111,   111,
   111,   111,   111,   111,   112,   113,   114,   114,   115,   116,
   116,   117,   117
};

static const short yyr2[] = {     0,
     5,     2,     2,     2,     6,     5,     5,     0,     2,     0,
     2,     2,     0,     5,     3,     3,     0,     3,     6,     3,
     3,     0,     2,     6,     1,     0,     2,     0,     3,     1,
     1,     1,     1,     1,     1,     1,     1,     0,     1,     1,
     4,     2,     7,     0,     3,     2,     3,     3,     3,     3,
     2,     2,     0,     1,     1,     5,     1,     4,     2,     1,
     3,     2,     2,     3,     2,     2,     1,     1,     2,     2,
     1,     1,     1,     2,     2,     1,     1,     1,     1,     2,
     2,     1,     2,     4,     8,     1,     1,     1,     1,     1,
     1,     1,     1,     2,     3,     5,     1,     2,     1,     3,
     1,     3,     3,     5,     4,     4,     6,     5,     2,     2,
     0,     1,     3,     2,     2,     2,     0,     1,     2,     2,
     2,     2,     2,     1,     1,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     5,     2,     3,     3,     2,     5,
     3,     1,     3
};

static const short yydefact[] = {    13,
     0,     0,     0,     0,     8,     8,     8,    10,     0,     0,
     8,     8,     8,     2,     3,     4,    17,     0,    16,    15,
     9,     8,     0,     0,     0,    12,    11,    22,     0,     0,
    71,    76,     0,     0,     8,    68,    72,     8,     0,     0,
    60,     8,     0,    67,     0,    73,    77,    78,    79,   101,
     8,     0,     0,    97,     0,     8,     8,     8,     8,     8,
     8,     8,     8,     8,     8,     8,     8,     8,     8,     8,
     8,     8,     8,     8,     8,     8,     8,     8,     8,     8,
    55,    54,     0,     0,   112,   118,   125,   124,     0,     8,
    10,    28,    10,   151,   152,     0,    14,    69,    94,     0,
     8,    80,     0,    70,     0,    59,     8,     8,    62,     8,
     8,     0,    63,     0,    71,    72,    66,    75,    65,    74,
     8,     0,   109,     6,     0,    98,     8,     8,   120,   121,
   146,   142,   141,   143,   128,   129,   130,   131,   132,   133,
   134,   135,   136,   137,   138,   139,   140,   144,   127,   126,
   122,     0,   123,   119,   114,     7,     8,     8,     0,     0,
     0,    26,    18,     0,    23,     0,    95,     0,     8,     0,
     0,     0,    82,     5,     0,     0,    51,    52,    61,    64,
   103,   100,     8,     0,     8,   102,     0,     0,   116,   115,
   113,    21,    20,    36,    37,     8,    25,     8,     0,     0,
     0,     8,     0,     0,    10,    31,    32,    33,    30,    35,
    34,   150,   153,     0,    83,     8,    81,    87,    88,    89,
    90,    91,    84,    86,     8,     0,     0,     8,   106,     0,
   105,     0,     8,    38,    27,     8,   149,     0,     0,     0,
     0,    29,    96,     0,    56,   104,     8,     8,   108,   145,
     8,     0,     0,    40,     0,    48,    47,     0,    50,    49,
   148,   147,    93,    92,     8,   107,   110,    46,    19,    42,
     8,    24,     8,     0,     0,    44,    85,    41,     0,    43,
     8,    45,     0,     0,     0
};

static const short yydefgoto[] = {   283,
     5,     6,     7,   215,    17,     8,    28,    91,    92,    93,
   198,   199,   164,   205,   196,   252,   253,   206,   279,   254,
   207,   208,   112,    83,    38,    39,    40,    41,    42,    43,
    44,    45,    46,    47,   172,   173,    48,   225,   265,    49,
    52,    53,    54,    55,   249,    84,   159,    85,    86,    87,
    88,   210,   211,    95,    96
};

static const short yypact[] = {   217,
   235,   -22,    -8,    46,-32768,-32768,-32768,-32768,     1,   112,
-32768,-32768,-32768,   111,   111,   111,   176,   104,-32768,-32768,
-32768,-32768,   202,    22,   412,-32768,-32768,     8,   227,     0,
   108,-32768,   129,   128,-32768,-32768,   135,-32768,    34,   157,
-32768,   261,   124,-32768,   149,-32768,-32768,-32768,-32768,    73,
-32768,    96,    92,   103,   154,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,   481,   309,-32768,-32768,-32768,-32768,    98,-32768,
-32768,-32768,-32768,-32768,-32768,   184,-32768,-32768,-32768,   195,
-32768,-32768,   237,-32768,    35,-32768,-32768,-32768,-32768,-32768,
-32768,   246,   111,   149,-32768,-32768,-32768,-32768,   111,-32768,
-32768,   208,   111,-32768,    94,   169,-32768,-32768,   111,   111,
   111,   111,   111,   111,   111,   111,   111,   111,   111,   111,
   111,   111,   111,   111,   111,   111,   111,   111,   111,   111,
   111,   412,   111,-32768,-32768,-32768,-32768,-32768,   452,   131,
   101,    29,   176,   117,   176,   262,-32768,   202,   108,   135,
   215,    40,-32768,-32768,    22,   202,   111,   111,-32768,   111,
   111,   144,-32768,   236,-32768,   111,   412,   360,   111,   111,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,   102,   194,
   251,-32768,   254,   267,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,   220,   111,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,   230,   175,-32768,   111,   148,
   111,   255,-32768,    91,   111,-32768,-32768,   170,    91,   199,
   219,   176,-32768,    19,-32768,   111,-32768,-32768,-32768,   111,
-32768,   253,    38,-32768,    33,-32768,-32768,    42,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,   111,   111,   111,-32768,-32768,
-32768,-32768,-32768,    24,    91,   111,-32768,-32768,     3,-32768,
-32768,   111,   314,   316,-32768
};

static const short yypgoto[] = {-32768,
-32768,-32768,-32768,    -1,   -84,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,   118,-32768,    79,-32768,-32768,    47,
-32768,-32768,-32768,-32768,  -163,-32768,   147,   -88,-32768,   221,
   282,   284,   -32,-32768,-32768,   156,-32768,-32768,-32768,-32768,
   153,-32768,   276,-32768,-32768,   -33,-32768,   171,   248,-32768,
-32768,-32768,-32768,    -7,   -42
};


#define	YYLAST		527


static const short yytable[] = {    10,
   209,    20,    21,    14,    15,    16,   163,   122,   165,    23,
    24,    25,   120,    31,    32,    33,    34,    35,    36,    37,
    30,    21,    50,   179,    21,    89,    21,    11,   263,   264,
    90,    21,    51,   103,   106,    21,   105,    21,   270,   197,
   113,    12,   270,   119,   218,   219,   220,   221,   222,   123,
    18,    97,    19,   280,   129,   130,   131,   132,   133,   134,
   135,   136,   137,   138,   139,   140,   141,   142,   143,   144,
   145,   146,   147,   148,   149,   150,   151,   152,   153,   214,
   277,   120,   184,   107,   272,   174,   108,   161,   162,   -39,
   271,   273,   125,    21,   271,    13,   223,   224,   160,   168,
    -8,   251,    51,    21,    29,   175,   176,    -8,   177,   178,
   194,   194,   180,    21,    21,   281,    -1,   200,   188,   181,
   242,    22,    18,    98,   121,   186,   187,    31,    32,    33,
    34,    35,    36,    37,   115,   201,   202,   203,   102,    99,
   116,   100,   -99,    18,    -8,   183,   124,   195,   195,   118,
   104,    -8,   193,   232,   127,   189,   190,   109,   213,    -8,
    32,    33,    34,    35,    -8,    -8,   128,   -53,   -53,   -53,
   -53,   -53,   -53,   -53,   213,   109,   101,   204,    26,    27,
    18,   229,   192,   231,   166,   -53,   -53,   -53,   -53,   -53,
   -53,   -53,   237,    18,   234,   228,   235,    18,    -8,   247,
   239,    -8,    -8,    -8,    21,   167,   -57,    -8,   182,   -57,
   110,   111,    31,    32,    33,    34,    35,    36,    37,    18,
   185,   256,   213,   244,   -58,   216,   246,   -58,   110,   111,
   257,   250,   260,   262,   255,     9,   230,    -8,     1,    21,
     2,     3,     4,    18,    -8,   266,   267,   169,    18,   268,
   259,   238,    36,   170,   240,   155,    31,    32,    33,    34,
    35,    36,    37,   274,  -117,  -117,  -117,   241,    18,   275,
   261,   276,    32,    33,    34,    35,    18,    94,   243,   282,
   245,   248,  -117,  -117,  -117,  -117,  -117,  -117,  -117,  -117,
  -117,  -117,  -117,  -117,  -117,  -117,  -117,  -117,  -117,  -117,
  -117,  -117,  -117,  -117,   269,  -111,  -111,   157,  -117,   155,
  -117,    18,   212,   284,   158,   285,   236,   258,  -117,  -117,
  -117,   278,   227,   171,   117,   114,   217,   226,   126,   191,
   154,     0,     0,     0,     0,     0,  -117,  -117,  -117,  -117,
  -117,  -117,  -117,  -117,  -117,  -117,  -117,  -117,  -117,  -117,
  -117,  -117,  -117,  -117,  -117,  -117,  -117,  -117,     0,   156,
   155,   157,  -117,     0,  -117,     0,     0,     0,   158,  -117,
  -117,  -117,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,  -117,  -117,  -117,
  -117,  -117,  -117,  -117,  -117,  -117,  -117,  -117,  -117,  -117,
  -117,  -117,  -117,  -117,  -117,  -117,  -117,  -117,  -117,     0,
     0,     0,   157,  -117,    21,  -117,     0,     0,   233,   158,
     0,    56,    57,    58,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,    59,
    60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
    70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
    80,    56,    57,    58,     0,    81,     0,    82,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,    59,
    60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
    70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
    80,     0,     0,     0,     0,    81,     0,    82,    59,    60,
    61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
    71,    72,    73,    74,    75,    76,    77
};

static const short yycheck[] = {     1,
   164,     9,     3,     5,     6,     7,    91,    50,    93,    11,
    12,    13,    45,    11,    12,    13,    14,    15,    16,    17,
    22,     3,     1,   112,     3,    18,     3,    50,    10,    11,
    23,     3,    11,    35,     1,     3,    38,     3,     1,    11,
    42,    50,     1,    45,     5,     6,     7,     8,     9,    51,
    50,    52,    52,    51,    56,    57,    58,    59,    60,    61,
    62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
    72,    73,    74,    75,    76,    77,    78,    79,    80,   168,
    57,   114,   125,    50,    52,    51,    53,    89,    90,    52,
    53,    50,     1,     3,    53,    50,    57,    58,     1,   101,
     3,    11,    11,     3,     1,   107,   108,    10,   110,   111,
    10,    10,   114,     3,     3,   279,     0,     1,   152,   121,
   205,    10,    50,    16,    52,   127,   128,    11,    12,    13,
    14,    15,    16,    17,    11,    19,    20,    21,    11,    11,
    17,    13,    51,    50,    47,    52,    51,    47,    47,     1,
    16,     3,   160,   187,    52,   157,   158,     1,   166,    11,
    12,    13,    14,    15,    16,    17,    13,    11,    12,    13,
    14,    15,    16,    17,   182,     1,    48,    61,     3,     4,
    50,   183,    52,   185,     1,    11,    12,    13,    14,    15,
    16,    17,   200,    50,   196,    52,   198,    50,    50,    52,
   202,    53,    54,    55,     3,    11,    50,    59,     1,    53,
    54,    55,    11,    12,    13,    14,    15,    16,    17,    50,
    52,    52,   230,   225,    50,    11,   228,    53,    54,    55,
   238,   233,   240,   241,   236,     1,     1,     3,    22,     3,
    24,    25,    26,    50,    10,   247,   248,    11,    50,   251,
    52,     1,    16,    17,     1,     1,    11,    12,    13,    14,
    15,    16,    17,   265,    10,    11,    12,     1,    50,   271,
    52,   273,    12,    13,    14,    15,    50,    51,    59,   281,
    51,    27,    28,    29,    30,    31,    32,    33,    34,    35,
    36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
    46,    47,    48,    49,    52,    51,    52,    53,    54,     1,
    56,    50,    51,     0,    60,     0,   199,   239,    10,    11,
    12,   275,   176,   103,    43,    42,   171,   175,    53,   159,
    83,    -1,    -1,    -1,    -1,    -1,    28,    29,    30,    31,
    32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
    42,    43,    44,    45,    46,    47,    48,    49,    -1,    51,
     1,    53,    54,    -1,    56,    -1,    -1,    -1,    60,    10,
    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    28,    29,    30,
    31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
    41,    42,    43,    44,    45,    46,    47,    48,    49,    -1,
    -1,    -1,    53,    54,     3,    56,    -1,    -1,    59,    60,
    -1,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    28,
    29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
    39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
    49,    10,    11,    12,    -1,    54,    -1,    56,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    28,
    29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
    39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
    49,    -1,    -1,    -1,    -1,    54,    -1,    56,    28,    29,
    30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
    40,    41,    42,    43,    44,    45,    46
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
#line 248 "parser.y"
{
        CSSParser *p = static_cast<CSSParser *>(parser);
	p->rule = yyvsp[-2].rule;
    ;
    break;}
case 6:
#line 255 "parser.y"
{
	/* can be empty */
    ;
    break;}
case 7:
#line 261 "parser.y"
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
#line 298 "parser.y"
{
#ifdef CSS_DEBUG
     kdDebug( 6080 ) << "charset rule: " << qString(yyvsp[-2].string) << endl;
#endif
 ;
    break;}
case 18:
#line 309 "parser.y"
{
     CSSParser *p = static_cast<CSSParser *>(parser);
     if ( yyvsp[-1].rule && p->styleElement && p->styleElement->isCSSStyleSheet() ) {
	 p->styleElement->append( yyvsp[-1].rule );
     } else {
	 delete yyvsp[-1].rule;
     }
 ;
    break;}
case 19:
#line 320 "parser.y"
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
case 20:
#line 330 "parser.y"
{
        yyval.rule = 0;
    ;
    break;}
case 21:
#line 333 "parser.y"
{
        yyval.rule = 0;
    ;
    break;}
case 26:
#line 352 "parser.y"
{ yyval.string.string = 0; yyval.string.length = 0; ;
    break;}
case 29:
#line 358 "parser.y"
{
     CSSParser *p = static_cast<CSSParser *>(parser);
     if ( yyvsp[-1].rule && p->styleElement && p->styleElement->isCSSStyleSheet() ) {
	 p->styleElement->append( yyvsp[-1].rule );
     } else {
	 delete yyvsp[-1].rule;
     }
 ;
    break;}
case 38:
#line 383 "parser.y"
{
	yyval.mediaList = 0;
    ;
    break;}
case 40:
#line 391 "parser.y"
{
	yyval.mediaList = new MediaListImpl();
	yyval.mediaList->appendMedium( domString(yyvsp[0].string) );
    ;
    break;}
case 41:
#line 395 "parser.y"
{
	yyval.mediaList = yyvsp[-3].mediaList;
	yyval.mediaList->appendMedium( domString(yyvsp[0].string) );
    ;
    break;}
case 42:
#line 399 "parser.y"
{
	delete yyvsp[-1].mediaList;
	yyval.mediaList = 0;
    ;
    break;}
case 43:
#line 406 "parser.y"
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
case 44:
#line 420 "parser.y"
{ yyval.ruleList = 0; ;
    break;}
case 45:
#line 421 "parser.y"
{
      yyval.ruleList = yyvsp[-2].ruleList;
      if ( yyvsp[-1].rule ) {
	  if ( !yyval.ruleList ) yyval.ruleList = new CSSRuleListImpl();
	  yyval.ruleList->append( yyvsp[-1].rule );
      }
  ;
    break;}
case 46:
#line 431 "parser.y"
{
      yyval.string = yyvsp[-1].string;
  ;
    break;}
case 47:
#line 453 "parser.y"
{
      yyval.rule = 0;
    ;
    break;}
case 48:
#line 456 "parser.y"
{
      yyval.rule = 0;
    ;
    break;}
case 49:
#line 462 "parser.y"
{
      yyval.rule = 0;
    ;
    break;}
case 50:
#line 465 "parser.y"
{
      yyval.rule = 0;
    ;
    break;}
case 51:
#line 471 "parser.y"
{ yyval.relation = CSSSelector::Sibling; ;
    break;}
case 52:
#line 472 "parser.y"
{ yyval.relation = CSSSelector::Child; ;
    break;}
case 53:
#line 473 "parser.y"
{ yyval.relation = CSSSelector::Descendant; ;
    break;}
case 54:
#line 477 "parser.y"
{ yyval.val = -1; ;
    break;}
case 55:
#line 478 "parser.y"
{ yyval.val = 1; ;
    break;}
case 56:
#line 482 "parser.y"
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
case 57:
#line 502 "parser.y"
{
	if ( yyvsp[0].selector ) {
	    yyval.selectorList = new QPtrList<CSSSelector>;
            yyval.selectorList->setAutoDelete( true );
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
case 58:
#line 515 "parser.y"
{
	if ( yyvsp[-3].selectorList && yyvsp[0].selector ) {
	    yyval.selectorList = yyvsp[-3].selectorList;
	    yyval.selectorList->append( yyvsp[0].selector );
#ifdef CSS_DEBUG
	    kdDebug( 6080 ) << "   got simple selector:" << endl;
	    yyvsp[0].selector->print();
#endif
	} else {
	    delete yyvsp[-3].selectorList;
	    delete yyvsp[0].selector;
	    yyval.selectorList = 0;
	}
    ;
    break;}
case 59:
#line 529 "parser.y"
{
	delete yyvsp[-1].selectorList;
	yyval.selectorList = 0;
    ;
    break;}
case 60:
#line 536 "parser.y"
{
	yyval.selector = yyvsp[0].selector;
    ;
    break;}
case 61:
#line 539 "parser.y"
{
	if ( !yyvsp[-2].selector || !yyvsp[0].selector ) {
	    delete yyvsp[-2].selector;
	    delete yyvsp[0].selector;
	    yyval.selector = 0;
	} else {
	    yyval.selector = yyvsp[0].selector;
	    CSSSelector *end = yyvsp[0].selector;
	    while( end->tagHistory )
		end = end->tagHistory;
	    end->relation = yyvsp[-1].relation;
	    end->tagHistory = yyvsp[-2].selector;
	    if ( yyvsp[-1].relation == CSSSelector::Descendant ||
		 yyvsp[-1].relation == CSSSelector::Child ) {
		CSSParser *p = static_cast<CSSParser *>(parser);
		DOM::DocumentImpl *doc = p->document();
		if ( doc )
		    doc->setUsesDescendantRules(true);
	    }
	}
    ;
    break;}
case 62:
#line 560 "parser.y"
{
	delete yyvsp[-1].selector;
	yyval.selector = 0;
    ;
    break;}
case 63:
#line 567 "parser.y"
{
	yyval.selector = new CSSSelector();
	yyval.selector->tag = yyvsp[-1].element;
    ;
    break;}
case 64:
#line 571 "parser.y"
{
	yyval.selector = yyvsp[-1].selector;
        if ( yyval.selector )
	    yyval.selector->tag = yyvsp[-2].element;
    ;
    break;}
case 65:
#line 576 "parser.y"
{
	yyval.selector = yyvsp[-1].selector;
        if ( yyval.selector )
	    yyval.selector->tag = 0xffffffff;
    ;
    break;}
case 66:
#line 584 "parser.y"
{ yyval.element = (yyvsp[-1].ns<<16) | yyvsp[0].element; ;
    break;}
case 67:
#line 585 "parser.y"
{
        /* according to the specs this one matches all namespaces if no
	   default namespace has been specified otherwise the default namespace */
	CSSParser *p = static_cast<CSSParser *>(parser);
	yyval.element = (p->defaultNamespace<<16) | yyvsp[0].element;
    ;
    break;}
case 68:
#line 594 "parser.y"
{ yyval.ns = 0; ;
    break;}
case 69:
#line 595 "parser.y"
{ yyval.ns = 1; /* #### insert correct namespace id here */ ;
    break;}
case 70:
#line 596 "parser.y"
{ yyval.ns = 0xffff; ;
    break;}
case 71:
#line 601 "parser.y"
{
	CSSParser *p = static_cast<CSSParser *>(parser);
	DOM::DocumentImpl *doc = p->document();
	QString tag = qString(yyvsp[0].string);
	if ( doc ) {
	    if (doc->isHTMLDocument())
		tag = tag.lower();
	    const DOMString dtag(tag);
#if APPLE_CHANGES
            yyval.element = doc->tagId(0, dtag.implementation(), false);
#else
	    yyval.element = doc->elementNames()->getId(dtag.implementation(), false);
#endif
	} else {
	    yyval.element = khtml::getTagID(tag.lower().ascii(), tag.length());
	    // this case should never happen - only when loading
	    // the default stylesheet - which must not contain unknown tags
// 	    assert($$ != 0);
	}
    ;
    break;}
case 72:
#line 621 "parser.y"
{ yyval.element = 0xffff; ;
    break;}
case 73:
#line 625 "parser.y"
{
	yyval.selector = yyvsp[0].selector;
	yyval.selector->nonCSSHint = static_cast<CSSParser *>(parser)->nonCSSHint;
    ;
    break;}
case 74:
#line 629 "parser.y"
{
	yyval.selector = yyvsp[-1].selector;
	if ( yyval.selector ) {
            CSSSelector *end = yyvsp[-1].selector;
            while( end->tagHistory )
                end = end->tagHistory;
            end->relation = CSSSelector::SubSelector;
            end->tagHistory = yyvsp[0].selector;
	}
    ;
    break;}
case 75:
#line 639 "parser.y"
{
	delete yyvsp[-1].selector;
	yyval.selector = 0;
    ;
    break;}
case 76:
#line 646 "parser.y"
{
	yyval.selector = new CSSSelector();
	yyval.selector->match = CSSSelector::Id;
	yyval.selector->attr = ATTR_ID;
	yyval.selector->value = domString(yyvsp[0].string);
    ;
    break;}
case 80:
#line 658 "parser.y"
{
	yyval.selector = new CSSSelector();
	yyval.selector->match = CSSSelector::List;
	yyval.selector->attr = ATTR_CLASS;
	yyval.selector->value = domString(yyvsp[0].string);
    ;
    break;}
case 81:
#line 667 "parser.y"
{ yyval.attribute = (yyvsp[-1].ns<<16) | yyvsp[0].attribute; ;
    break;}
case 82:
#line 668 "parser.y"
{
	/* opposed to elements, these only match for non namespaced attributes */
	yyval.attribute = yyvsp[0].attribute;
    ;
    break;}
case 83:
#line 675 "parser.y"
{
	CSSParser *p = static_cast<CSSParser *>(parser);
	DOM::DocumentImpl *doc = p->document();

	QString attr = qString(yyvsp[-1].string);
	if ( doc ) {
	    if (doc->isHTMLDocument())
		attr = attr.lower();
	    const DOMString dattr(attr);
#if APPLE_CHANGES
            yyval.attribute = doc->attrId(0, dattr.implementation(), false);
#else
	    yyval.attribute = doc->attrNames()->getId(dattr.implementation(), false);
#endif
	} else {
	    yyval.attribute = khtml::getAttrID(attr.lower().ascii(), attr.length());
	    // this case should never happen - only when loading
	    // the default stylesheet - which must not contain unknown attributes
	    assert(yyval.attribute != 0);
	    }
    ;
    break;}
case 84:
#line 699 "parser.y"
{
	yyval.selector = new CSSSelector();
	yyval.selector->attr = yyvsp[-1].attribute;
	yyval.selector->match = CSSSelector::Set;
    ;
    break;}
case 85:
#line 704 "parser.y"
{
	yyval.selector = new CSSSelector();
	yyval.selector->attr = yyvsp[-5].attribute;
	yyval.selector->match = (CSSSelector::Match)yyvsp[-4].val;
	yyval.selector->value = domString(yyvsp[-2].string);
    ;
    break;}
case 86:
#line 713 "parser.y"
{
	yyval.val = CSSSelector::Exact;
    ;
    break;}
case 87:
#line 716 "parser.y"
{
	yyval.val = CSSSelector::List;
    ;
    break;}
case 88:
#line 719 "parser.y"
{
	yyval.val = CSSSelector::Hyphen;
    ;
    break;}
case 89:
#line 722 "parser.y"
{
	yyval.val = CSSSelector::Begin;
    ;
    break;}
case 90:
#line 725 "parser.y"
{
	yyval.val = CSSSelector::End;
    ;
    break;}
case 91:
#line 728 "parser.y"
{
	yyval.val = CSSSelector::Contain;
    ;
    break;}
case 94:
#line 739 "parser.y"
{
	yyval.selector = new CSSSelector();
	yyval.selector->match = CSSSelector::Pseudo;
	yyval.selector->value = domString(yyvsp[0].string);
    ;
    break;}
case 95:
#line 745 "parser.y"
{
	yyval.selector = new CSSSelector();
	yyval.selector->match = CSSSelector::Pseudo;
        yyval.selector->value = domString(yyvsp[0].string);
    ;
    break;}
case 96:
#line 750 "parser.y"
{
        yyval.selector = new CSSSelector();
        yyval.selector->match = CSSSelector::Pseudo;
        yyval.selector->simpleSelector = yyvsp[-1].selector;
        yyval.selector->value = domString(yyvsp[-3].string);
    ;
    break;}
case 97:
#line 759 "parser.y"
{
	yyval.ok = yyvsp[0].ok;
    ;
    break;}
case 98:
#line 762 "parser.y"
{
	yyval.ok = yyvsp[-1].ok;
	if ( yyvsp[0].ok )
	    yyval.ok = yyvsp[0].ok;
    ;
    break;}
case 99:
#line 767 "parser.y"
{
	yyval.ok = yyvsp[0].ok;
    ;
    break;}
case 100:
#line 770 "parser.y"
{
	yyval.ok = false;
#ifdef CSS_DEBUG
	kdDebug( 6080 ) << "skipping bogus declaration" << endl;
#endif
    ;
    break;}
case 101:
#line 776 "parser.y"
{
	yyval.ok = false;
#ifdef CSS_DEBUG
	kdDebug( 6080 ) << "skipping all declarations" << endl;
#endif
    ;
    break;}
case 102:
#line 785 "parser.y"
{
	yyval.ok = yyvsp[-2].ok;
    ;
    break;}
case 103:
#line 788 "parser.y"
{
	yyval.ok = false;
#ifdef CSS_DEBUG
	kdDebug( 6080 ) << "skipping bogus declaration" << endl;
#endif
    ;
    break;}
case 104:
#line 794 "parser.y"
{
	yyval.ok = false;
#ifdef CSS_DEBUG
	kdDebug( 6080 ) << "skipping bogus declaration" << endl;
#endif
    ;
    break;}
case 105:
#line 800 "parser.y"
{
	yyval.ok = yyvsp[-3].ok;
	if ( yyvsp[-2].ok )
	    yyval.ok = yyvsp[-2].ok;
    ;
    break;}
case 106:
#line 805 "parser.y"
{
	yyval.ok = yyvsp[-3].ok;
#ifdef CSS_DEBUG
	kdDebug( 6080 ) << "skipping bogus declaration" << endl;
#endif
    ;
    break;}
case 107:
#line 811 "parser.y"
{
	yyval.ok = yyvsp[-5].ok;
#ifdef CSS_DEBUG
	kdDebug( 6080 ) << "skipping bogus declaration" << endl;
#endif
    ;
    break;}
case 108:
#line 820 "parser.y"
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
	} else {
            delete yyvsp[-1].valueList;
        }
	delete p->valueList;
	p->valueList = 0;
    ;
    break;}
case 109:
#line 845 "parser.y"
{
	QString str = qString(yyvsp[-1].string);
	yyval.prop_id = getPropertyID( str.lower().latin1(), str.length() );
    ;
    break;}
case 110:
#line 852 "parser.y"
{ yyval.b = true; ;
    break;}
case 111:
#line 853 "parser.y"
{ yyval.b = false; ;
    break;}
case 112:
#line 857 "parser.y"
{
	yyval.valueList = new ValueList;
	yyval.valueList->addValue( yyvsp[0].value );
    ;
    break;}
case 113:
#line 861 "parser.y"
{
	yyval.valueList = yyvsp[-2].valueList;
	if ( yyval.valueList ) {
	    if ( yyvsp[-1].tok ) {
		Value v;
		v.id = 0;
		v.unit = Value::Operator;
		v.iValue = yyvsp[-1].tok;
		yyval.valueList->addValue( v );
	    }
	    yyval.valueList->addValue( yyvsp[0].value );
	}
    ;
    break;}
case 114:
#line 874 "parser.y"
{
	delete yyvsp[-1].valueList;
	yyval.valueList = 0;
    ;
    break;}
case 115:
#line 881 "parser.y"
{
	yyval.tok = '/';
    ;
    break;}
case 116:
#line 884 "parser.y"
{
	yyval.tok = ',';
    ;
    break;}
case 117:
#line 887 "parser.y"
{
        yyval.tok = 0;
  ;
    break;}
case 118:
#line 893 "parser.y"
{ yyval.value = yyvsp[0].value; ;
    break;}
case 119:
#line 894 "parser.y"
{ yyval.value = yyvsp[0].value; yyval.value.fValue *= yyvsp[-1].val; ;
    break;}
case 120:
#line 895 "parser.y"
{ yyval.value.id = 0; yyval.value.string = yyvsp[-1].string; yyval.value.unit = CSSPrimitiveValue::CSS_STRING; ;
    break;}
case 121:
#line 896 "parser.y"
{
      QString str = qString( yyvsp[-1].string );
      yyval.value.id = getValueID( str.lower().latin1(), str.length() );
      yyval.value.unit = CSSPrimitiveValue::CSS_IDENT;
      yyval.value.string = yyvsp[-1].string;
  ;
    break;}
case 122:
#line 902 "parser.y"
{ yyval.value.id = 0; yyval.value.string = yyvsp[-1].string; yyval.value.unit = CSSPrimitiveValue::CSS_URI; ;
    break;}
case 123:
#line 903 "parser.y"
{ yyval.value.id = 0; yyval.value.iValue = 0; yyval.value.unit = CSSPrimitiveValue::CSS_UNKNOWN;/* ### */ ;
    break;}
case 124:
#line 904 "parser.y"
{ yyval.value.id = 0; yyval.value.string = yyvsp[0].string; yyval.value.unit = CSSPrimitiveValue::CSS_RGBCOLOR; ;
    break;}
case 125:
#line 906 "parser.y"
{
      yyval.value = yyvsp[0].value;
  ;
    break;}
case 126:
#line 912 "parser.y"
{ yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_NUMBER; ;
    break;}
case 127:
#line 913 "parser.y"
{ yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_PERCENTAGE; ;
    break;}
case 128:
#line 914 "parser.y"
{ yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_PX; ;
    break;}
case 129:
#line 915 "parser.y"
{ yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_CM; ;
    break;}
case 130:
#line 916 "parser.y"
{ yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_MM; ;
    break;}
case 131:
#line 917 "parser.y"
{ yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_IN; ;
    break;}
case 132:
#line 918 "parser.y"
{ yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_PT; ;
    break;}
case 133:
#line 919 "parser.y"
{ yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_PC; ;
    break;}
case 134:
#line 920 "parser.y"
{ yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_DEG; ;
    break;}
case 135:
#line 921 "parser.y"
{ yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_RAD; ;
    break;}
case 136:
#line 922 "parser.y"
{ yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_GRAD; ;
    break;}
case 137:
#line 923 "parser.y"
{ yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_MS; ;
    break;}
case 138:
#line 924 "parser.y"
{ yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_S; ;
    break;}
case 139:
#line 925 "parser.y"
{ yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_HZ; ;
    break;}
case 140:
#line 926 "parser.y"
{ yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_KHZ; ;
    break;}
case 141:
#line 927 "parser.y"
{ yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_EMS; ;
    break;}
case 142:
#line 928 "parser.y"
{ yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = Value::Q_EMS; ;
    break;}
case 143:
#line 929 "parser.y"
{ yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_EXS; ;
    break;}
case 144:
#line 930 "parser.y"
{ yyval.value.id = 0; yyval.value.string = yyvsp[-1].string; yyval.value.unit = CSSPrimitiveValue::CSS_DIMENSION; ;
    break;}
case 145:
#line 935 "parser.y"
{
      Function *f = new Function;
      f->name = yyvsp[-4].string;
      f->args = yyvsp[-2].valueList;
      yyval.value.id = 0;
      yyval.value.unit = Value::Function;
      yyval.value.function = f;
  ;
    break;}
case 146:
#line 950 "parser.y"
{ yyval.string = yyvsp[-1].string; ;
    break;}
case 147:
#line 957 "parser.y"
{
	yyval.rule = 0;
#ifdef CSS_DEBUG
	kdDebug( 6080 ) << "skipped invalid @-rule" << endl;
#endif
    ;
    break;}
case 148:
#line 963 "parser.y"
{
	yyval.rule = 0;
#ifdef CSS_DEBUG
	kdDebug( 6080 ) << "skipped invalid @-rule" << endl;
#endif
    ;
    break;}
case 149:
#line 972 "parser.y"
{
	yyval.rule = 0;
#ifdef CSS_DEBUG
	kdDebug( 6080 ) << "skipped invalid rule" << endl;
#endif
    ;
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
#line 1007 "parser.y"


