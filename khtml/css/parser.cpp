/* A Bison parser, made from parser.y
   by GNU bison 1.35.  */

#define YYBISON 1  /* Identify Bison output.  */

#define yyparse cssyyparse
#define yylex cssyylex
#define yyerror cssyyerror
#define yylval cssyylval
#define yychar cssyychar
#define yydebug cssyydebug
#define yynerrs cssyynerrs
# define	S	257
# define	SGML_CD	258
# define	INCLUDES	259
# define	DASHMATCH	260
# define	BEGINSWITH	261
# define	ENDSWITH	262
# define	CONTAINS	263
# define	STRING	264
# define	IDENT	265
# define	HASH	266
# define	IMPORT_SYM	267
# define	PAGE_SYM	268
# define	MEDIA_SYM	269
# define	FONT_FACE_SYM	270
# define	CHARSET_SYM	271
# define	NAMESPACE_SYM	272
# define	KONQ_RULE_SYM	273
# define	KONQ_DECLS_SYM	274
# define	KONQ_VALUE_SYM	275
# define	IMPORTANT_SYM	276
# define	QEMS	277
# define	EMS	278
# define	EXS	279
# define	PXS	280
# define	CMS	281
# define	MMS	282
# define	INS	283
# define	PTS	284
# define	PCS	285
# define	DEGS	286
# define	RADS	287
# define	GRADS	288
# define	MSECS	289
# define	SECS	290
# define	HERZ	291
# define	KHERZ	292
# define	DIMEN	293
# define	PERCENTAGE	294
# define	NUMBER	295
# define	URI	296
# define	FUNCTION	297
# define	UNICODERANGE	298

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

#line 79 "parser.y"
#ifndef YYSTYPE
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
} yystype;
# define YYSTYPE yystype
# define YYSTYPE_IS_TRIVIAL 1
#endif
#line 100 "parser.y"


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


#ifndef YYDEBUG
# define YYDEBUG 0
#endif



#define	YYFINAL		284
#define	YYFLAG		-32768
#define	YYNTBASE	62

/* YYTRANSLATE(YYLEX) -- Bison token number corresponding to YYLEX. */
#define YYTRANSLATE(x) ((unsigned)(x) <= 298 ? yytranslate[x] : 118)

/* YYTRANSLATE[YYLEX] -- Bison token number corresponding to YYLEX. */
static const char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    59,    17,    54,    53,    56,    14,    60,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    13,    52,
       2,    58,    55,     2,    61,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    15,     2,    57,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    50,    16,    51,     2,     2,     2,     2,
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
       2,     2,     2,     2,     2,     2,     1,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49
};

#if YYDEBUG
static const short yyprhs[] =
{
       0,     0,     6,     9,    12,    15,    22,    28,    34,    35,
      38,    39,    42,    45,    46,    52,    56,    60,    61,    65,
      72,    76,    80,    81,    84,    91,    93,    94,    97,    98,
     102,   104,   106,   108,   110,   112,   114,   116,   118,   119,
     121,   123,   128,   131,   139,   140,   144,   147,   151,   155,
     159,   163,   166,   169,   170,   172,   174,   180,   182,   187,
     190,   192,   196,   199,   202,   206,   209,   212,   214,   216,
     219,   222,   224,   226,   228,   231,   234,   236,   238,   240,
     242,   245,   248,   250,   253,   258,   267,   269,   271,   273,
     275,   277,   279,   281,   283,   286,   293,   295,   298,   300,
     304,   306,   310,   314,   320,   325,   330,   337,   343,   346,
     349,   350,   352,   356,   359,   362,   365,   366,   368,   371,
     374,   377,   380,   383,   385,   387,   390,   393,   396,   399,
     402,   405,   408,   411,   414,   417,   420,   423,   426,   429,
     432,   435,   438,   441,   444,   450,   453,   457,   461,   464,
     470,   474,   476
};
static const short yyrhs[] =
{
      68,    67,    69,    71,    75,     0,    63,    66,     0,    64,
      66,     0,    65,    66,     0,    24,    50,    66,    87,    66,
      51,     0,    25,    50,    66,   103,    51,     0,    26,    50,
      66,   108,    51,     0,     0,    66,     3,     0,     0,    67,
       4,     0,    67,     3,     0,     0,    22,    66,    10,    66,
      52,     0,    22,     1,   116,     0,    22,     1,    52,     0,
       0,    69,    70,    67,     0,    18,    66,    77,    66,    78,
      52,     0,    18,     1,   116,     0,    18,     1,    52,     0,
       0,    72,    67,     0,    23,    66,    74,    77,    66,    52,
       0,    11,     0,     0,    73,    66,     0,     0,    75,    76,
      67,     0,    87,     0,    80,     0,    83,     0,    84,     0,
     115,     0,   114,     0,    10,     0,    47,     0,     0,    79,
       0,    82,     0,    79,    53,    66,    82,     0,    79,     1,
       0,    20,    66,    79,    50,    66,    81,    51,     0,     0,
      81,    87,    66,     0,    11,    66,     0,    19,     1,   116,
       0,    19,     1,    52,     0,    21,     1,   116,     0,    21,
       1,    52,     0,    54,    66,     0,    55,    66,     0,     0,
      56,     0,    54,     0,    88,    50,    66,   103,    51,     0,
      89,     0,    88,    53,    66,    89,     0,    88,     1,     0,
      90,     0,    89,    85,    90,     0,    89,     1,     0,    91,
      66,     0,    91,    94,    66,     0,    94,    66,     0,    92,
      93,     0,    93,     0,    16,     0,    11,    16,     0,    17,
      16,     0,    11,     0,    17,     0,    95,     0,    94,    95,
       0,    94,     1,     0,    12,     0,    96,     0,    99,     0,
     102,     0,    14,    11,     0,    92,    98,     0,    98,     0,
      11,    66,     0,    15,    66,    97,    57,     0,    15,    66,
      97,   100,    66,   101,    66,    57,     0,    58,     0,     5,
       0,     6,     0,     7,     0,     8,     0,     9,     0,    11,
       0,    10,     0,    13,    11,     0,    13,    48,    66,    11,
      66,    59,     0,   105,     0,   104,   105,     0,   104,     0,
       1,   117,     1,     0,     1,     0,   105,    52,    66,     0,
       1,    52,    66,     0,     1,   117,     1,    52,    66,     0,
     104,   105,    52,    66,     0,   104,     1,    52,    66,     0,
     104,     1,   117,     1,    52,    66,     0,   106,    13,    66,
     108,   107,     0,    11,    66,     0,    27,    66,     0,     0,
     110,     0,   108,   109,   110,     0,   108,     1,     0,    60,
      66,     0,    53,    66,     0,     0,   111,     0,    86,   111,
       0,    10,    66,     0,    11,    66,     0,    47,    66,     0,
      49,    66,     0,   113,     0,   112,     0,    46,    66,     0,
      45,    66,     0,    31,    66,     0,    32,    66,     0,    33,
      66,     0,    34,    66,     0,    35,    66,     0,    36,    66,
       0,    37,    66,     0,    38,    66,     0,    39,    66,     0,
      40,    66,     0,    41,    66,     0,    42,    66,     0,    43,
      66,     0,    29,    66,     0,    28,    66,     0,    30,    66,
       0,    44,    66,     0,    48,    66,   108,    59,    66,     0,
      12,    66,     0,    61,     1,   116,     0,    61,     1,    52,
       0,     1,   116,     0,    50,     1,   117,     1,    51,     0,
      50,     1,    51,     0,   116,     0,   117,     1,   116,     0
};

#endif

#if YYDEBUG
/* YYRLINE[YYN] -- source line where rule number YYN was defined. */
static const short yyrline[] =
{
       0,   234,   236,   237,   238,   241,   248,   254,   279,   281,
     284,   286,   287,   290,   292,   297,   298,   301,   303,   313,
     324,   327,   333,   334,   338,   342,   345,   347,   350,   352,
     362,   364,   365,   366,   367,   368,   371,   373,   376,   380,
     384,   389,   393,   400,   415,   417,   426,   448,   452,   457,
     461,   466,   468,   469,   472,   474,   477,   497,   511,   525,
     531,   535,   550,   556,   561,   566,   573,   575,   583,   585,
     586,   590,   607,   610,   615,   623,   629,   636,   637,   638,
     641,   650,   652,   658,   678,   684,   692,   696,   699,   702,
     705,   708,   713,   715,   718,   724,   732,   736,   741,   744,
     750,   758,   762,   768,   774,   779,   785,   793,   816,   823,
     825,   828,   833,   846,   852,   856,   859,   864,   866,   867,
     868,   874,   875,   876,   878,   883,   885,   886,   887,   888,
     889,   890,   891,   892,   893,   894,   895,   896,   897,   898,
     899,   900,   901,   902,   906,   921,   928,   935,   943,   969,
     971,   974,   976
};
#endif


#if (YYDEBUG) || defined YYERROR_VERBOSE

/* YYTNAME[TOKEN_NUM] -- String name of the token TOKEN_NUM. */
static const char *const yytname[] =
{
  "$", "error", "$undefined.", "S", "SGML_CD", "INCLUDES", "DASHMATCH", 
  "BEGINSWITH", "ENDSWITH", "CONTAINS", "STRING", "IDENT", "HASH", "':'", 
  "'.'", "'['", "'|'", "'*'", "IMPORT_SYM", "PAGE_SYM", "MEDIA_SYM", 
  "FONT_FACE_SYM", "CHARSET_SYM", "NAMESPACE_SYM", "KONQ_RULE_SYM", 
  "KONQ_DECLS_SYM", "KONQ_VALUE_SYM", "IMPORTANT_SYM", "QEMS", "EMS", 
  "EXS", "PXS", "CMS", "MMS", "INS", "PTS", "PCS", "DEGS", "RADS", 
  "GRADS", "MSECS", "SECS", "HERZ", "KHERZ", "DIMEN", "PERCENTAGE", 
  "NUMBER", "URI", "FUNCTION", "UNICODERANGE", "'{'", "'}'", "';'", "','", 
  "'+'", "'>'", "'-'", "']'", "'='", "')'", "'/'", "'@'", "stylesheet", 
  "konq_rule", "konq_decls", "konq_value", "maybe_space", "maybe_sgml", 
  "maybe_charset", "import_list", "import", "maybe_namespace", 
  "namespace", "ns_prefix", "maybe_ns_prefix", "rule_list", "rule", 
  "string_or_uri", "maybe_media_list", "media_list", "media", 
  "ruleset_list", "medium", "page", "font_face", "combinator", 
  "unary_operator", "ruleset", "selector_list", "selector", 
  "simple_selector", "ns_element", "ns_selector", "element_name", 
  "specifier_list", "specifier", "class", "ns_attrib_id", "attrib_id", 
  "attrib", "match", "ident_or_string", "pseudo", "declaration_list", 
  "decl_list", "declaration", "property", "prio", "expr", "operator", 
  "term", "unary_term", "function", "hexcolor", "invalid_at", 
  "invalid_rule", "invalid_block", "invalid_block_list", 0
};
#endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives. */
static const short yyr1[] =
{
       0,    62,    62,    62,    62,    63,    64,    65,    66,    66,
      67,    67,    67,    68,    68,    68,    68,    69,    69,    70,
      70,    70,    71,    71,    72,    73,    74,    74,    75,    75,
      76,    76,    76,    76,    76,    76,    77,    77,    78,    78,
      79,    79,    79,    80,    81,    81,    82,    83,    83,    84,
      84,    85,    85,    85,    86,    86,    87,    88,    88,    88,
      89,    89,    89,    90,    90,    90,    91,    91,    92,    92,
      92,    93,    93,    94,    94,    94,    95,    95,    95,    95,
      96,    97,    97,    98,    99,    99,   100,   100,   100,   100,
     100,   100,   101,   101,   102,   102,   103,   103,   103,   103,
     103,   104,   104,   104,   104,   104,   104,   105,   106,   107,
     107,   108,   108,   108,   109,   109,   109,   110,   110,   110,
     110,   110,   110,   110,   110,   111,   111,   111,   111,   111,
     111,   111,   111,   111,   111,   111,   111,   111,   111,   111,
     111,   111,   111,   111,   112,   113,   114,   114,   115,   116,
     116,   117,   117
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN. */
static const short yyr2[] =
{
       0,     5,     2,     2,     2,     6,     5,     5,     0,     2,
       0,     2,     2,     0,     5,     3,     3,     0,     3,     6,
       3,     3,     0,     2,     6,     1,     0,     2,     0,     3,
       1,     1,     1,     1,     1,     1,     1,     1,     0,     1,
       1,     4,     2,     7,     0,     3,     2,     3,     3,     3,
       3,     2,     2,     0,     1,     1,     5,     1,     4,     2,
       1,     3,     2,     2,     3,     2,     2,     1,     1,     2,
       2,     1,     1,     1,     2,     2,     1,     1,     1,     1,
       2,     2,     1,     2,     4,     8,     1,     1,     1,     1,
       1,     1,     1,     1,     2,     6,     1,     2,     1,     3,
       1,     3,     3,     5,     4,     4,     6,     5,     2,     2,
       0,     1,     3,     2,     2,     2,     0,     1,     2,     2,
       2,     2,     2,     1,     1,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     5,     2,     3,     3,     2,     5,
       3,     1,     3
};

/* YYDEFACT[S] -- default rule to reduce with in state S when YYTABLE
   doesn't specify something else to do.  Zero means the default is an
   error. */
static const short yydefact[] =
{
      13,     0,     0,     0,     0,     8,     8,     8,    10,     0,
       0,     8,     8,     8,     2,     3,     4,    17,     0,    16,
      15,     9,     8,     0,     0,     0,    12,    11,    22,     0,
       0,    71,    76,     0,     0,     8,    68,    72,     8,     0,
       0,    60,     8,     0,    67,     0,    73,    77,    78,    79,
     100,     8,     0,     0,    96,     0,     8,     8,     8,     8,
       8,     8,     8,     8,     8,     8,     8,     8,     8,     8,
       8,     8,     8,     8,     8,     8,     8,     8,     8,     8,
       8,    55,    54,     0,     0,   111,   117,   124,   123,     0,
       8,    10,    28,    10,   150,   151,     0,    14,    69,    94,
       8,    80,     0,    70,     0,    59,     8,     8,    62,     8,
       8,     0,    63,     0,    71,    72,    66,    75,    65,    74,
       8,     0,   108,     6,     0,    97,     8,     8,   119,   120,
     145,   141,   140,   142,   127,   128,   129,   130,   131,   132,
     133,   134,   135,   136,   137,   138,   139,   143,   126,   125,
     121,     0,   122,   118,   113,     7,     8,     8,     0,     0,
       0,    26,    18,     0,    23,     0,     0,     8,     0,     0,
       0,    82,     5,     0,     0,    51,    52,    61,    64,   102,
      99,     8,     0,     8,   101,     0,     0,   115,   114,   112,
      21,    20,    36,    37,     8,    25,     8,     0,     0,     0,
       8,     0,     0,    10,    31,    32,    33,    30,    35,    34,
     149,   152,     8,    83,     8,    81,    87,    88,    89,    90,
      91,    84,    86,     8,     0,     0,     8,   105,     0,   104,
       0,     8,    38,    27,     8,   148,     0,     0,     0,     0,
      29,     0,     0,    56,   103,     8,     8,   107,   144,     8,
       0,     0,    40,     0,    48,    47,     0,    50,    49,   147,
     146,    95,    93,    92,     8,   106,   109,    46,    19,    42,
       8,    24,     8,     0,     0,    44,    85,    41,     0,    43,
       8,    45,     0,     0,     0
};

static const short yydefgoto[] =
{
     282,     5,     6,     7,   213,    17,     8,    28,    91,    92,
      93,   196,   197,   163,   203,   194,   250,   251,   204,   278,
     252,   205,   206,   111,    83,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,   170,   171,    48,   223,   264,
      49,    52,    53,    54,    55,   247,    84,   158,    85,    86,
      87,    88,   208,   209,    95,    96
};

static const short yypact[] =
{
      98,   165,   -10,     4,    60,-32768,-32768,-32768,-32768,   119,
       6,-32768,-32768,-32768,    49,    49,    49,   150,   120,-32768,
  -32768,-32768,-32768,   310,    19,   411,-32768,-32768,     5,   146,
      33,   141,-32768,    91,   124,-32768,-32768,   151,-32768,    34,
     173,-32768,   193,   159,-32768,   148,-32768,-32768,-32768,-32768,
     127,-32768,   121,     2,   131,   187,-32768,-32768,-32768,-32768,
  -32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
  -32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
  -32768,-32768,-32768,   480,   308,-32768,-32768,-32768,-32768,    93,
  -32768,-32768,-32768,-32768,-32768,-32768,   208,-32768,-32768,-32768,
  -32768,-32768,    15,-32768,    35,-32768,-32768,-32768,-32768,-32768,
  -32768,   223,    49,   148,-32768,-32768,-32768,-32768,    49,-32768,
  -32768,   212,    49,-32768,   142,   172,-32768,-32768,    49,    49,
      49,    49,    49,    49,    49,    49,    49,    49,    49,    49,
      49,    49,    49,    49,    49,    49,    49,    49,    49,    49,
      49,   411,    49,-32768,-32768,-32768,-32768,-32768,   451,   160,
     101,    16,   150,   117,   150,   217,    22,   141,   151,   231,
      40,-32768,-32768,    19,   310,    49,    49,-32768,    49,    49,
     191,-32768,   245,-32768,    49,   411,   359,    49,    49,-32768,
  -32768,-32768,-32768,-32768,-32768,-32768,-32768,   103,   201,   253,
  -32768,   269,   271,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
  -32768,-32768,-32768,    49,-32768,-32768,-32768,-32768,-32768,-32768,
  -32768,-32768,-32768,-32768,   222,   203,-32768,    49,   197,    49,
     254,-32768,    90,    49,-32768,-32768,   200,    90,   209,   210,
     150,    21,   104,-32768,    49,-32768,-32768,-32768,    49,-32768,
     224,    38,-32768,    48,-32768,-32768,    42,-32768,-32768,-32768,
  -32768,-32768,-32768,-32768,-32768,    49,    49,    49,-32768,-32768,
  -32768,-32768,-32768,    26,    90,    49,-32768,-32768,   130,-32768,
  -32768,    49,   274,   275,-32768
};

static const short yypgoto[] =
{
  -32768,-32768,-32768,-32768,    -1,   -76,-32768,-32768,-32768,-32768,
  -32768,-32768,-32768,-32768,-32768,    80,-32768,    41,-32768,-32768,
      30,-32768,-32768,-32768,-32768,  -162,-32768,   106,   204,-32768,
     214,   268,   270,   -31,-32768,-32768,   161,-32768,-32768,-32768,
  -32768,   144,-32768,   276,-32768,-32768,  -143,-32768,   170,   248,
  -32768,-32768,-32768,-32768,    -7,   -43
};


#define	YYLAST		526


static const short yytable[] =
{
      10,   207,    20,   124,    14,    15,    16,   121,   186,    21,
      23,    24,    25,    51,   119,   162,    22,   164,    21,    21,
      50,    30,    21,    89,    21,    21,   167,   195,    90,    21,
      51,    36,   168,   212,   102,   105,    21,   104,    21,   269,
      11,   112,   230,   269,   118,   216,   217,   218,   219,   220,
     122,    21,    21,   -98,    12,   128,   129,   130,   131,   132,
     133,   134,   135,   136,   137,   138,   139,   140,   141,   142,
     143,   144,   145,   146,   147,   148,   149,   150,   151,   152,
     261,   182,   119,   276,   106,    97,   172,   107,   160,   161,
     -39,   270,   272,    21,   159,   270,    -8,   221,   222,   166,
     271,   249,    99,    -8,    21,   173,   174,    21,   175,   176,
      13,   192,   178,   192,   262,   263,   280,    -1,   198,   179,
       1,    29,     2,     3,     4,   184,   185,   240,    31,    32,
      33,    34,    35,    36,    37,   101,   199,   200,   201,   100,
      -8,    31,    32,    33,    34,    35,    36,    37,   193,   117,
     193,    -8,   191,    26,    27,   187,   188,    98,   211,    -8,
      32,    33,    34,    35,    -8,    -8,     9,   103,    -8,    18,
     114,    19,   123,   211,   108,    -8,   115,    18,   202,   120,
     227,   279,   229,   126,   -53,   -53,   -53,   -53,   -53,   -53,
     -53,   235,    18,   232,   181,   233,    18,    94,    -8,   237,
     127,    -8,    -8,    -8,   108,    32,    33,    34,    35,   165,
      18,   241,   190,   180,   -53,   -53,   -53,   -53,   -53,   -53,
     -53,   211,   242,   -57,   183,   244,   -57,   109,   110,   255,
     248,   258,   260,   253,    31,    32,    33,    34,    35,    36,
      37,    18,   214,   226,   265,   266,   228,    18,   267,   245,
      18,    18,   254,   -58,   236,   154,   -58,   109,   110,    18,
      18,   257,   259,   273,  -116,  -116,  -116,    18,   210,   274,
     238,   275,   239,   243,   283,   284,   268,   234,   256,   281,
     225,   246,  -116,  -116,  -116,  -116,  -116,  -116,  -116,  -116,
    -116,  -116,  -116,  -116,  -116,  -116,  -116,  -116,  -116,  -116,
    -116,  -116,  -116,  -116,   277,  -110,  -110,   156,  -116,   154,
    -116,   116,   113,    21,   157,   177,   169,   224,  -116,  -116,
    -116,    31,    32,    33,    34,    35,    36,    37,   189,   125,
     215,   153,     0,     0,     0,     0,  -116,  -116,  -116,  -116,
    -116,  -116,  -116,  -116,  -116,  -116,  -116,  -116,  -116,  -116,
    -116,  -116,  -116,  -116,  -116,  -116,  -116,  -116,     0,   155,
     154,   156,  -116,     0,  -116,     0,     0,     0,   157,  -116,
    -116,  -116,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  -116,  -116,  -116,
    -116,  -116,  -116,  -116,  -116,  -116,  -116,  -116,  -116,  -116,
    -116,  -116,  -116,  -116,  -116,  -116,  -116,  -116,  -116,     0,
       0,     0,   156,  -116,    21,  -116,     0,     0,   231,   157,
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

static const short yycheck[] =
{
       1,   163,     9,     1,     5,     6,     7,    50,   151,     3,
      11,    12,    13,    11,    45,    91,    10,    93,     3,     3,
       1,    22,     3,    18,     3,     3,    11,    11,    23,     3,
      11,    16,    17,    11,    35,     1,     3,    38,     3,     1,
      50,    42,   185,     1,    45,     5,     6,     7,     8,     9,
      51,     3,     3,    51,    50,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      59,   124,   113,    57,    50,    52,    51,    53,    89,    90,
      52,    53,    50,     3,     1,    53,     3,    57,    58,   100,
      52,    11,    11,    10,     3,   106,   107,     3,   109,   110,
      50,    10,   113,    10,    10,    11,   278,     0,     1,   120,
      22,     1,    24,    25,    26,   126,   127,   203,    11,    12,
      13,    14,    15,    16,    17,    11,    19,    20,    21,    48,
      47,    11,    12,    13,    14,    15,    16,    17,    47,     1,
      47,     3,   159,     3,     4,   156,   157,    16,   165,    11,
      12,    13,    14,    15,    16,    17,     1,    16,     3,    50,
      11,    52,    51,   180,     1,    10,    17,    50,    61,    52,
     181,    51,   183,    52,    11,    12,    13,    14,    15,    16,
      17,   198,    50,   194,    52,   196,    50,    51,    50,   200,
      13,    53,    54,    55,     1,    12,    13,    14,    15,     1,
      50,   212,    52,     1,    11,    12,    13,    14,    15,    16,
      17,   228,   223,    50,    52,   226,    53,    54,    55,   236,
     231,   238,   239,   234,    11,    12,    13,    14,    15,    16,
      17,    50,    11,    52,   245,   246,     1,    50,   249,    52,
      50,    50,    52,    50,     1,     1,    53,    54,    55,    50,
      50,    52,    52,   264,    10,    11,    12,    50,    51,   270,
       1,   272,     1,    51,     0,     0,    52,   197,   237,   280,
     174,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,   274,    51,    52,    53,    54,     1,
      56,    43,    42,     3,    60,   111,   102,   173,    10,    11,
      12,    11,    12,    13,    14,    15,    16,    17,   158,    53,
     169,    83,    -1,    -1,    -1,    -1,    28,    29,    30,    31,
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
#line 3 "/usr/share/bison/bison.simple"

/* Skeleton output parser for bison,

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002 Free Software
   Foundation, Inc.

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

/* This is the parser code that is written into each bison parser when
   the %semantic_parser declaration is not specified in the grammar.
   It was written by Richard Stallman by simplifying the hairy parser
   used when %semantic_parser is specified.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

#ifndef YYPARSE_RETURN_TYPE
#define YYPARSE_RETURN_TYPE int
#endif

#if ! defined (yyoverflow) || defined (YYERROR_VERBOSE)

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# if YYSTACK_USE_ALLOCA
#  define YYSTACK_ALLOC alloca
# else
#  ifndef YYSTACK_USE_ALLOCA
#   if defined (alloca) || defined (_ALLOCA_H)
#    define YYSTACK_ALLOC alloca
#   else
#    ifdef __GNUC__
#     define YYSTACK_ALLOC __builtin_alloca
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning. */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
# else
#  if defined (__STDC__) || defined (__cplusplus)
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   define YYSIZE_T size_t
#  endif
#  define YYSTACK_ALLOC malloc
#  define YYSTACK_FREE free
# endif
#endif /* ! defined (yyoverflow) || defined (YYERROR_VERBOSE) */


#if (! defined (yyoverflow) \
     && (! defined (__cplusplus) \
	 || ((YYLTYPE_IS_TRIVIAL || ! YYLSP_NEEDED) && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  short yyss;
  YYSTYPE yyvs;
# if YYLSP_NEEDED
  YYLTYPE yyls;
# endif
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAX (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# if YYLSP_NEEDED
#  define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short) + sizeof (YYSTYPE) + sizeof (YYLTYPE))	\
      + 2 * YYSTACK_GAP_MAX)
# else
#  define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short) + sizeof (YYSTYPE))				\
      + YYSTACK_GAP_MAX)
# endif

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  register YYSIZE_T yyi;		\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (0)
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAX;	\
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (0)

#endif


#if ! defined (YYSIZE_T) && defined (__SIZE_TYPE__)
# define YYSIZE_T __SIZE_TYPE__
#endif
#if ! defined (YYSIZE_T) && defined (size_t)
# define YYSIZE_T size_t
#endif
#if ! defined (YYSIZE_T)
# if defined (__STDC__) || defined (__cplusplus)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# endif
#endif
#if ! defined (YYSIZE_T)
# define YYSIZE_T unsigned int
#endif

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		-2
#define YYEOF		0
#define YYACCEPT	goto yyacceptlab
#define YYABORT 	goto yyabortlab
#define YYERROR		goto yyerrlab1
/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */
#define YYFAIL		goto yyerrlab
#define YYRECOVERING()  (!!yyerrstatus)
#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yychar1 = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { 								\
      yyerror ("syntax error: cannot back up");			\
      YYERROR;							\
    }								\
while (0)

#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Compute the default location (before the actions
   are run).

   When YYLLOC_DEFAULT is run, CURRENT is set the location of the
   first token.  By default, to implement support for ranges, extend
   its range to the last symbol.  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)       	\
   Current.last_line   = Rhs[N].last_line;	\
   Current.last_column = Rhs[N].last_column;
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#if YYPURE
# if YYLSP_NEEDED
#  ifdef YYLEX_PARAM
#   define YYLEX		yylex (&yylval, &yylloc, YYLEX_PARAM)
#  else
#   define YYLEX		yylex (&yylval, &yylloc)
#  endif
# else /* !YYLSP_NEEDED */
#  ifdef YYLEX_PARAM
#   define YYLEX		yylex (&yylval, YYLEX_PARAM)
#  else
#   define YYLEX		yylex (&yylval)
#  endif
# endif /* !YYLSP_NEEDED */
#else /* !YYPURE */
# define YYLEX			yylex ()
#endif /* !YYPURE */


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (0)
/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
#endif /* !YYDEBUG */

/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   SIZE_MAX < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#if YYMAXDEPTH == 0
# undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif

#ifdef YYERROR_VERBOSE

# ifndef yystrlen
#  if defined (__GLIBC__) && defined (_STRING_H)
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
#   if defined (__STDC__) || defined (__cplusplus)
yystrlen (const char *yystr)
#   else
yystrlen (yystr)
     const char *yystr;
#   endif
{
  register const char *yys = yystr;

  while (*yys++ != '\0')
    continue;

  return yys - yystr - 1;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined (__GLIBC__) && defined (_STRING_H) && defined (_GNU_SOURCE)
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
#   if defined (__STDC__) || defined (__cplusplus)
yystpcpy (char *yydest, const char *yysrc)
#   else
yystpcpy (yydest, yysrc)
     char *yydest;
     const char *yysrc;
#   endif
{
  register char *yyd = yydest;
  register const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif
#endif

#line 319 "/usr/share/bison/bison.simple"


/* The user can define YYPARSE_PARAM as the name of an argument to be passed
   into yyparse.  The argument should have type void *.
   It should actually point to an object.
   Grammar actions can access the variable by casting it
   to the proper pointer type.  */

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
#  define YYPARSE_PARAM_ARG void *YYPARSE_PARAM
#  define YYPARSE_PARAM_DECL
# else
#  define YYPARSE_PARAM_ARG YYPARSE_PARAM
#  define YYPARSE_PARAM_DECL void *YYPARSE_PARAM;
# endif
#else /* !YYPARSE_PARAM */
# define YYPARSE_PARAM_ARG
# define YYPARSE_PARAM_DECL
#endif /* !YYPARSE_PARAM */

/* Prevent warning if -Wstrict-prototypes.  */
#ifdef __GNUC__
# ifdef YYPARSE_PARAM
YYPARSE_RETURN_TYPE yyparse (void *);
# else
YYPARSE_RETURN_TYPE yyparse (void);
# endif
#endif

/* YY_DECL_VARIABLES -- depending whether we use a pure parser,
   variables are global, or local to YYPARSE.  */

#define YY_DECL_NON_LSP_VARIABLES			\
/* The lookahead symbol.  */				\
int yychar;						\
							\
/* The semantic value of the lookahead symbol. */	\
YYSTYPE yylval;						\
							\
/* Number of parse errors so far.  */			\
int yynerrs;

#if YYLSP_NEEDED
# define YY_DECL_VARIABLES			\
YY_DECL_NON_LSP_VARIABLES			\
						\
/* Location data for the lookahead symbol.  */	\
YYLTYPE yylloc;
#else
# define YY_DECL_VARIABLES			\
YY_DECL_NON_LSP_VARIABLES
#endif


/* If nonreentrant, generate the variables here. */

#if !YYPURE
YY_DECL_VARIABLES
#endif  /* !YYPURE */

YYPARSE_RETURN_TYPE
yyparse (YYPARSE_PARAM_ARG)
     YYPARSE_PARAM_DECL
{
  /* If reentrant, generate the variables here. */
#if YYPURE
  YY_DECL_VARIABLES
#endif  /* !YYPURE */

  register int yystate;
  register int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Lookahead token as an internal (translated) token number.  */
  int yychar1 = 0;

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack. */
  short	yyssa[YYINITDEPTH];
  short *yyss = yyssa;
  register short *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  register YYSTYPE *yyvsp;

#if YYLSP_NEEDED
  /* The location stack.  */
  YYLTYPE yylsa[YYINITDEPTH];
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;
#endif

#if YYLSP_NEEDED
# define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)
#else
# define YYPOPSTACK   (yyvsp--, yyssp--)
#endif

  YYSIZE_T yystacksize = YYINITDEPTH;


  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
#if YYLSP_NEEDED
  YYLTYPE yyloc;
#endif

  /* When reducing, the number of symbols on the RHS of the reduced
     rule. */
  int yylen;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;
#if YYLSP_NEEDED
  yylsp = yyls;
#endif
  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed. so pushing a state here evens the stacks.
     */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyssp >= yyss + yystacksize - 1)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack. Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	short *yyss1 = yyss;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  */
# if YYLSP_NEEDED
	YYLTYPE *yyls1 = yyls;
	/* This used to be a conditional around just the two extra args,
	   but that might be undefined if yyoverflow is a macro.  */
	yyoverflow ("parser stack overflow",
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yyls1, yysize * sizeof (*yylsp),
		    &yystacksize);
	yyls = yyls1;
# else
	yyoverflow ("parser stack overflow",
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yystacksize);
# endif
	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyoverflowlab;
# else
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
	goto yyoverflowlab;
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;

      {
	short *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyoverflowlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);
# if YYLSP_NEEDED
	YYSTACK_RELOCATE (yyls);
# endif
# undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
#if YYLSP_NEEDED
      yylsp = yyls + yysize - 1;
#endif

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyssp >= yyss + yystacksize - 1)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
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
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (yychar <= 0)		/* This means end of input. */
    {
      yychar1 = 0;
      yychar = YYEOF;		/* Don't call YYLEX any more */

      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yychar1 = YYTRANSLATE (yychar);

#if YYDEBUG
     /* We have to keep this `#if YYDEBUG', since we use variables
	which are defined only if `YYDEBUG' is set.  */
      if (yydebug)
	{
	  YYFPRINTF (stderr, "Next token is %d (%s",
		     yychar, yytname[yychar1]);
	  /* Give the individual parser a way to print the precise
	     meaning of a token, for further debugging info.  */
# ifdef YYPRINT
	  YYPRINT (stderr, yychar, yylval);
# endif
	  YYFPRINTF (stderr, ")\n");
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
  YYDPRINTF ((stderr, "Shifting token %d (%s), ",
	      yychar, yytname[yychar1]));

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;
#if YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  yystate = yyn;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to the semantic value of
     the lookahead token.  This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];

#if YYLSP_NEEDED
  /* Similarly for the default location.  Let the user run additional
     commands if for instance locations are ranges.  */
  yyloc = yylsp[1-yylen];
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
#endif

#if YYDEBUG
  /* We have to keep this `#if YYDEBUG', since we use variables which
     are defined only if `YYDEBUG' is set.  */
  if (yydebug)
    {
      int yyi;

      YYFPRINTF (stderr, "Reducing via rule %d (line %d), ",
		 yyn, yyrline[yyn]);

      /* Print the symbols being reduced, and their result.  */
      for (yyi = yyprhs[yyn]; yyrhs[yyi] > 0; yyi++)
	YYFPRINTF (stderr, "%s ", yytname[yyrhs[yyi]]);
      YYFPRINTF (stderr, " -> %s\n", yytname[yyr1[yyn]]);
    }
#endif

  switch (yyn) {

case 5:
#line 242 "parser.y"
{
        CSSParser *p = static_cast<CSSParser *>(parser);
	p->rule = yyvsp[-2].rule;
    ;
    break;}
case 6:
#line 249 "parser.y"
{
	/* can be empty */
    ;
    break;}
case 7:
#line 255 "parser.y"
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
#line 292 "parser.y"
{
#ifdef CSS_DEBUG
     kdDebug( 6080 ) << "charset rule: " << qString(yyvsp[-2].string) << endl;
#endif
 ;
    break;}
case 18:
#line 303 "parser.y"
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
#line 314 "parser.y"
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
#line 324 "parser.y"
{
        yyval.rule = 0;
    ;
    break;}
case 21:
#line 327 "parser.y"
{
        yyval.rule = 0;
    ;
    break;}
case 26:
#line 346 "parser.y"
{ yyval.string.string = 0; yyval.string.length = 0; ;
    break;}
case 29:
#line 352 "parser.y"
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
#line 377 "parser.y"
{
	yyval.mediaList = 0;
    ;
    break;}
case 40:
#line 385 "parser.y"
{
	yyval.mediaList = new MediaListImpl();
	yyval.mediaList->appendMedium( domString(yyvsp[0].string) );
    ;
    break;}
case 41:
#line 389 "parser.y"
{
	yyval.mediaList = yyvsp[-3].mediaList;
	yyval.mediaList->appendMedium( domString(yyvsp[0].string) );
    ;
    break;}
case 42:
#line 393 "parser.y"
{
	delete yyvsp[-1].mediaList;
	yyval.mediaList = 0;
    ;
    break;}
case 43:
#line 401 "parser.y"
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
#line 416 "parser.y"
{ yyval.ruleList = 0; ;
    break;}
case 45:
#line 417 "parser.y"
{
      yyval.ruleList = yyvsp[-2].ruleList;
      if ( yyvsp[-1].rule ) {
	  if ( !yyval.ruleList ) yyval.ruleList = new CSSRuleListImpl();
	  yyval.ruleList->append( yyvsp[-1].rule );
      }
  ;
    break;}
case 46:
#line 427 "parser.y"
{
      yyval.string = yyvsp[-1].string;
  ;
    break;}
case 47:
#line 449 "parser.y"
{
      yyval.rule = 0;
    ;
    break;}
case 48:
#line 452 "parser.y"
{
      yyval.rule = 0;
    ;
    break;}
case 49:
#line 458 "parser.y"
{
      yyval.rule = 0;
    ;
    break;}
case 50:
#line 461 "parser.y"
{
      yyval.rule = 0;
    ;
    break;}
case 51:
#line 467 "parser.y"
{ yyval.relation = CSSSelector::Sibling; ;
    break;}
case 52:
#line 468 "parser.y"
{ yyval.relation = CSSSelector::Child; ;
    break;}
case 53:
#line 469 "parser.y"
{ yyval.relation = CSSSelector::Descendant; ;
    break;}
case 54:
#line 473 "parser.y"
{ yyval.val = -1; ;
    break;}
case 55:
#line 474 "parser.y"
{ yyval.val = 1; ;
    break;}
case 56:
#line 478 "parser.y"
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
#line 498 "parser.y"
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
#line 511 "parser.y"
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
#line 525 "parser.y"
{
	delete yyvsp[-1].selectorList;
	yyval.selectorList = 0;
    ;
    break;}
case 60:
#line 532 "parser.y"
{
	yyval.selector = yyvsp[0].selector;
    ;
    break;}
case 61:
#line 535 "parser.y"
{
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
    ;
    break;}
case 62:
#line 550 "parser.y"
{
	delete yyvsp[-1].selector;
	yyval.selector = 0;
    ;
    break;}
case 63:
#line 557 "parser.y"
{
	yyval.selector = new CSSSelector();
	yyval.selector->tag = yyvsp[-1].element;
    ;
    break;}
case 64:
#line 561 "parser.y"
{
	yyval.selector = yyvsp[-1].selector;
        if ( yyval.selector )
	    yyval.selector->tag = yyvsp[-2].element;
    ;
    break;}
case 65:
#line 566 "parser.y"
{
	yyval.selector = yyvsp[-1].selector;
        if ( yyval.selector )
	    yyval.selector->tag = 0xffffffff;
    ;
    break;}
case 66:
#line 574 "parser.y"
{ yyval.element = (yyvsp[-1].ns<<16) | yyvsp[0].element; ;
    break;}
case 67:
#line 575 "parser.y"
{
        /* according to the specs this one matches all namespaces if no
	   default namespace has been specified otherwise the default namespace */
	CSSParser *p = static_cast<CSSParser *>(parser);
	yyval.element = (p->defaultNamespace<<16) | yyvsp[0].element;
    ;
    break;}
case 68:
#line 584 "parser.y"
{ yyval.ns = 0; ;
    break;}
case 69:
#line 585 "parser.y"
{ yyval.ns = 1; /* #### insert correct namespace id here */ ;
    break;}
case 70:
#line 586 "parser.y"
{ yyval.ns = 0xffff; ;
    break;}
case 71:
#line 591 "parser.y"
{
	CSSParser *p = static_cast<CSSParser *>(parser);
	DOM::DocumentImpl *doc = p->document();
	QString tag = qString(yyvsp[0].string);
	if ( doc ) {
	    if (doc->isHTMLDocument())
		tag = tag.lower();
	    const DOMString dtag(tag);
	    yyval.element = doc->elementNames()->getId(dtag.implementation(), false);
	} else {
	    yyval.element = khtml::getTagID(tag.lower().ascii(), tag.length());
	    // this case should never happen - only when loading
	    // the default stylesheet - which must not contain unknown tags
// 	    assert($$ != 0);
	}
    ;
    break;}
case 72:
#line 607 "parser.y"
{ yyval.element = 0xffff; ;
    break;}
case 73:
#line 611 "parser.y"
{
	yyval.selector = yyvsp[0].selector;
	yyval.selector->nonCSSHint = static_cast<CSSParser *>(parser)->nonCSSHint;
    ;
    break;}
case 74:
#line 615 "parser.y"
{
	yyval.selector = yyvsp[-1].selector;
        CSSSelector *end = yyvsp[-1].selector;
        while( end->tagHistory )
            end = end->tagHistory;
        end->relation = CSSSelector::SubSelector;
        end->tagHistory = yyvsp[0].selector;
    ;
    break;}
case 75:
#line 623 "parser.y"
{
	delete yyvsp[-1].selector;
	yyval.selector = 0;
;
    break;}
case 76:
#line 630 "parser.y"
{
	yyval.selector = new CSSSelector();
	yyval.selector->match = CSSSelector::Id;
	yyval.selector->attr = ATTR_ID;
	yyval.selector->value = domString(yyvsp[0].string);
    ;
    break;}
case 80:
#line 642 "parser.y"
{
	yyval.selector = new CSSSelector();
	yyval.selector->match = CSSSelector::List;
	yyval.selector->attr = ATTR_CLASS;
	yyval.selector->value = domString(yyvsp[0].string);
    ;
    break;}
case 81:
#line 651 "parser.y"
{ yyval.attribute = (yyvsp[-1].ns<<16) | yyvsp[0].attribute; ;
    break;}
case 82:
#line 652 "parser.y"
{
	/* opposed to elements, these only match for non namespaced attributes */
	yyval.attribute = yyvsp[0].attribute;
    ;
    break;}
case 83:
#line 659 "parser.y"
{
	CSSParser *p = static_cast<CSSParser *>(parser);
	DOM::DocumentImpl *doc = p->document();

	QString attr = qString(yyvsp[-1].string);
	if ( doc ) {
	    if (doc->isHTMLDocument())
		attr = attr.lower();
	    const DOMString dattr(attr);
	    yyval.attribute = doc->attrNames()->getId(dattr.implementation(), false);
	} else {
	    yyval.attribute = khtml::getAttrID(attr.lower().ascii(), attr.length());
	    // this case should never happen - only when loading
	    // the default stylesheet - which must not contain unknown attributes
	    assert(yyval.attribute != 0);
	    }
    ;
    break;}
case 84:
#line 679 "parser.y"
{
	yyval.selector = new CSSSelector();
	yyval.selector->attr = yyvsp[-1].attribute;
	yyval.selector->match = CSSSelector::Set;
    ;
    break;}
case 85:
#line 684 "parser.y"
{
	yyval.selector = new CSSSelector();
	yyval.selector->attr = yyvsp[-5].attribute;
	yyval.selector->match = (CSSSelector::Match)yyvsp[-4].val;
	yyval.selector->value = domString(yyvsp[-2].string);
    ;
    break;}
case 86:
#line 693 "parser.y"
{
	yyval.val = CSSSelector::Exact;
    ;
    break;}
case 87:
#line 696 "parser.y"
{
	yyval.val = CSSSelector::List;
    ;
    break;}
case 88:
#line 699 "parser.y"
{
	yyval.val = CSSSelector::Hyphen;
    ;
    break;}
case 89:
#line 702 "parser.y"
{
	yyval.val = CSSSelector::Begin;
    ;
    break;}
case 90:
#line 705 "parser.y"
{
	yyval.val = CSSSelector::End;
    ;
    break;}
case 91:
#line 708 "parser.y"
{
	yyval.val = CSSSelector::Contain;
    ;
    break;}
case 94:
#line 719 "parser.y"
{
	yyval.selector = new CSSSelector();
	yyval.selector->match = CSSSelector::Pseudo;
	yyval.selector->value = domString(yyvsp[0].string);
    ;
    break;}
case 95:
#line 724 "parser.y"
{
	yyval.selector = new CSSSelector();
	yyval.selector->match = CSSSelector::Pseudo;
	yyval.selector->_pseudoType = CSSSelector::PseudoFunction;
	yyval.selector->value = domString(yyvsp[-2].string);
    ;
    break;}
case 96:
#line 733 "parser.y"
{
	yyval.ok = yyvsp[0].ok;
    ;
    break;}
case 97:
#line 736 "parser.y"
{
	yyval.ok = yyvsp[-1].ok;
	if ( yyvsp[0].ok )
	    yyval.ok = yyvsp[0].ok;
    ;
    break;}
case 98:
#line 741 "parser.y"
{
	yyval.ok = yyvsp[0].ok;
    ;
    break;}
case 99:
#line 744 "parser.y"
{
	yyval.ok = false;
#ifdef CSS_DEBUG
	kdDebug( 6080 ) << "skipping bogus declaration" << endl;
#endif
    ;
    break;}
case 100:
#line 750 "parser.y"
{
	yyval.ok = false;
#ifdef CSS_DEBUG
	kdDebug( 6080 ) << "skipping all declarations" << endl;
#endif
    ;
    break;}
case 101:
#line 759 "parser.y"
{
	yyval.ok = yyvsp[-2].ok;
    ;
    break;}
case 102:
#line 762 "parser.y"
{
	yyval.ok = false;
#ifdef CSS_DEBUG
	kdDebug( 6080 ) << "skipping bogus declaration" << endl;
#endif
    ;
    break;}
case 103:
#line 768 "parser.y"
{
	yyval.ok = false;
#ifdef CSS_DEBUG
	kdDebug( 6080 ) << "skipping bogus declaration" << endl;
#endif
    ;
    break;}
case 104:
#line 774 "parser.y"
{
	yyval.ok = yyvsp[-3].ok;
	if ( yyvsp[-2].ok )
	    yyval.ok = yyvsp[-2].ok;
    ;
    break;}
case 105:
#line 779 "parser.y"
{
	yyval.ok = yyvsp[-3].ok;
#ifdef CSS_DEBUG
	kdDebug( 6080 ) << "skipping bogus declaration" << endl;
#endif
    ;
    break;}
case 106:
#line 785 "parser.y"
{
	yyval.ok = yyvsp[-5].ok;
#ifdef CSS_DEBUG
	kdDebug( 6080 ) << "skipping bogus declaration" << endl;
#endif
    ;
    break;}
case 107:
#line 794 "parser.y"
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
case 108:
#line 817 "parser.y"
{
	QString str = qString(yyvsp[-1].string);
	yyval.prop_id = getPropertyID( str.lower().latin1(), str.length() );
    ;
    break;}
case 109:
#line 824 "parser.y"
{ yyval.b = true; ;
    break;}
case 110:
#line 825 "parser.y"
{ yyval.b = false; ;
    break;}
case 111:
#line 829 "parser.y"
{
	yyval.valueList = new ValueList;
	yyval.valueList->addValue( yyvsp[0].value );
    ;
    break;}
case 112:
#line 833 "parser.y"
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
case 113:
#line 846 "parser.y"
{
	delete yyvsp[-1].valueList;
	yyval.valueList = 0;
    ;
    break;}
case 114:
#line 853 "parser.y"
{
	yyval.tok = '/';
    ;
    break;}
case 115:
#line 856 "parser.y"
{
	yyval.tok = ',';
    ;
    break;}
case 116:
#line 859 "parser.y"
{
        yyval.tok = 0;
  ;
    break;}
case 117:
#line 865 "parser.y"
{ yyval.value = yyvsp[0].value; ;
    break;}
case 118:
#line 866 "parser.y"
{ yyval.value = yyvsp[0].value; yyval.value.fValue *= yyvsp[-1].val; ;
    break;}
case 119:
#line 867 "parser.y"
{ yyval.value.id = 0; yyval.value.string = yyvsp[-1].string; yyval.value.unit = CSSPrimitiveValue::CSS_STRING; ;
    break;}
case 120:
#line 868 "parser.y"
{
      QString str = qString( yyvsp[-1].string );
      yyval.value.id = getValueID( str.lower().latin1(), str.length() );
      yyval.value.unit = CSSPrimitiveValue::CSS_IDENT;
      yyval.value.string = yyvsp[-1].string;
  ;
    break;}
case 121:
#line 874 "parser.y"
{ yyval.value.id = 0; yyval.value.string = yyvsp[-1].string; yyval.value.unit = CSSPrimitiveValue::CSS_URI; ;
    break;}
case 122:
#line 875 "parser.y"
{ yyval.value.id = 0; yyval.value.iValue = 0; yyval.value.unit = CSSPrimitiveValue::CSS_UNKNOWN;/* ### */ ;
    break;}
case 123:
#line 876 "parser.y"
{ yyval.value.id = 0; yyval.value.string = yyvsp[0].string; yyval.value.unit = CSSPrimitiveValue::CSS_RGBCOLOR; ;
    break;}
case 124:
#line 878 "parser.y"
{
      yyval.value = yyvsp[0].value;
  ;
    break;}
case 125:
#line 884 "parser.y"
{ yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_NUMBER; ;
    break;}
case 126:
#line 885 "parser.y"
{ yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_PERCENTAGE; ;
    break;}
case 127:
#line 886 "parser.y"
{ yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_PX; ;
    break;}
case 128:
#line 887 "parser.y"
{ yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_CM; ;
    break;}
case 129:
#line 888 "parser.y"
{ yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_MM; ;
    break;}
case 130:
#line 889 "parser.y"
{ yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_IN; ;
    break;}
case 131:
#line 890 "parser.y"
{ yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_PT; ;
    break;}
case 132:
#line 891 "parser.y"
{ yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_PC; ;
    break;}
case 133:
#line 892 "parser.y"
{ yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_DEG; ;
    break;}
case 134:
#line 893 "parser.y"
{ yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_RAD; ;
    break;}
case 135:
#line 894 "parser.y"
{ yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_GRAD; ;
    break;}
case 136:
#line 895 "parser.y"
{ yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_MS; ;
    break;}
case 137:
#line 896 "parser.y"
{ yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_S; ;
    break;}
case 138:
#line 897 "parser.y"
{ yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_HZ; ;
    break;}
case 139:
#line 898 "parser.y"
{ yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_KHZ; ;
    break;}
case 140:
#line 899 "parser.y"
{ yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_EMS; ;
    break;}
case 141:
#line 900 "parser.y"
{ yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = Value::Q_EMS; ;
    break;}
case 142:
#line 901 "parser.y"
{ yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_EXS; ;
    break;}
case 143:
#line 902 "parser.y"
{ yyval.value.id = 0; yyval.value.string = yyvsp[-1].string; yyval.value.unit = CSSPrimitiveValue::CSS_DIMENSION ;
    break;}
case 144:
#line 907 "parser.y"
{
      Function *f = new Function;
      f->name = yyvsp[-4].string;
      f->args = yyvsp[-2].valueList;
      yyval.value.id = 0;
      yyval.value.unit = Value::Function;
      yyval.value.function = f;
  ;
    break;}
case 145:
#line 922 "parser.y"
{ yyval.string = yyvsp[-1].string; ;
    break;}
case 146:
#line 929 "parser.y"
{
	yyval.rule = 0;
#ifdef CSS_DEBUG
	kdDebug( 6080 ) << "skipped invalid @-rule" << endl;
#endif
    ;
    break;}
case 147:
#line 935 "parser.y"
{
	yyval.rule = 0;
#ifdef CSS_DEBUG
	kdDebug( 6080 ) << "skipped invalid @-rule" << endl;
#endif
    ;
    break;}
case 148:
#line 944 "parser.y"
{
	yyval.rule = 0;
#ifdef CSS_DEBUG
	kdDebug( 6080 ) << "skipped invalid rule" << endl;
#endif
    ;
    break;}
}

#line 709 "/usr/share/bison/bison.simple"


  yyvsp -= yylen;
  yyssp -= yylen;
#if YYLSP_NEEDED
  yylsp -= yylen;
#endif

#if YYDEBUG
  if (yydebug)
    {
      short *yyssp1 = yyss - 1;
      YYFPRINTF (stderr, "state stack now");
      while (yyssp1 != yyssp)
	YYFPRINTF (stderr, " %d", *++yyssp1);
      YYFPRINTF (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;
#if YYLSP_NEEDED
  *++yylsp = yyloc;
#endif

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
  if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTBASE];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;

#ifdef YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (yyn > YYFLAG && yyn < YYLAST)
	{
	  YYSIZE_T yysize = 0;
	  char *yymsg;
	  int yyx, yycount;

	  yycount = 0;
	  /* Start YYX at -YYN if negative to avoid negative indexes in
	     YYCHECK.  */
	  for (yyx = yyn < 0 ? -yyn : 0;
	       yyx < (int) (sizeof (yytname) / sizeof (char *)); yyx++)
	    if (yycheck[yyx + yyn] == yyx)
	      yysize += yystrlen (yytname[yyx]) + 15, yycount++;
	  yysize += yystrlen ("parse error, unexpected ") + 1;
	  yysize += yystrlen (yytname[YYTRANSLATE (yychar)]);
	  yymsg = (char *) YYSTACK_ALLOC (yysize);
	  if (yymsg != 0)
	    {
	      char *yyp = yystpcpy (yymsg, "parse error, unexpected ");
	      yyp = yystpcpy (yyp, yytname[YYTRANSLATE (yychar)]);

	      if (yycount < 5)
		{
		  yycount = 0;
		  for (yyx = yyn < 0 ? -yyn : 0;
		       yyx < (int) (sizeof (yytname) / sizeof (char *));
		       yyx++)
		    if (yycheck[yyx + yyn] == yyx)
		      {
			const char *yyq = ! yycount ? ", expecting " : " or ";
			yyp = yystpcpy (yyp, yyq);
			yyp = yystpcpy (yyp, yytname[yyx]);
			yycount++;
		      }
		}
	      yyerror (yymsg);
	      YYSTACK_FREE (yymsg);
	    }
	  else
	    yyerror ("parse error; also virtual memory exhausted");
	}
      else
#endif /* defined (YYERROR_VERBOSE) */
	yyerror ("parse error");
    }
  goto yyerrlab1;


/*--------------------------------------------------.
| yyerrlab1 -- error raised explicitly by an action |
`--------------------------------------------------*/
yyerrlab1:
  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      /* return failure if at end of input */
      if (yychar == YYEOF)
	YYABORT;
      YYDPRINTF ((stderr, "Discarding token %d (%s).\n",
		  yychar, yytname[yychar1]));
      yychar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */

  yyerrstatus = 3;		/* Each real token shifted decrements this */

  goto yyerrhandle;


/*-------------------------------------------------------------------.
| yyerrdefault -- current state does not do anything special for the |
| error token.                                                       |
`-------------------------------------------------------------------*/
yyerrdefault:
#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */

  /* If its default is to accept any token, ok.  Otherwise pop it.  */
  yyn = yydefact[yystate];
  if (yyn)
    goto yydefault;
#endif


/*---------------------------------------------------------------.
| yyerrpop -- pop the current state because it cannot handle the |
| error token                                                    |
`---------------------------------------------------------------*/
yyerrpop:
  if (yyssp == yyss)
    YYABORT;
  yyvsp--;
  yystate = *--yyssp;
#if YYLSP_NEEDED
  yylsp--;
#endif

#if YYDEBUG
  if (yydebug)
    {
      short *yyssp1 = yyss - 1;
      YYFPRINTF (stderr, "Error: state stack now");
      while (yyssp1 != yyssp)
	YYFPRINTF (stderr, " %d", *++yyssp1);
      YYFPRINTF (stderr, "\n");
    }
#endif

/*--------------.
| yyerrhandle.  |
`--------------*/
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

  YYDPRINTF ((stderr, "Shifting error token, "));

  *++yyvsp = yylval;
#if YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

/*---------------------------------------------.
| yyoverflowab -- parser overflow comes here.  |
`---------------------------------------------*/
yyoverflowlab:
  yyerror ("parser stack overflow");
  yyresult = 2;
  /* Fall through.  */

yyreturn:
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  return yyresult;
}
#line 979 "parser.y"


