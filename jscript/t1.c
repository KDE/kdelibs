
/*  A Bison parser, made from t1.y
 by  GNU Bison version 1.25
  */

#define YYBISON 1  /* Identify Bison output.  */

#define	FUNCTION	258
#define	IF	259
#define	ELSE	260
#define	IN	261
#define	WITH	262
#define	WHILE	263
#define	FOR	264
#define	SHIFT_LEFT	265
#define	SHIFT_RIGHT	266
#define	EQ	267
#define	NEQ	268
#define	OR	269
#define	AND	270
#define	THIS	271
#define	B_NULL	272
#define	FLOAT	273
#define	B_TRUE	274
#define	B_FALSE	275
#define	NEW	276
#define	DELETE	277
#define	BREAK	278
#define	CONTINUE	279
#define	RETURN	280
#define	VAR	281
#define	PP	282
#define	MM	283
#define	STRING	284
#define	LEQ	285
#define	GEQ	286
#define	MAS	287
#define	DAS	288
#define	AAS	289
#define	SAS	290
#define	PAS	291
#define	RAS	292
#define	BAAS	293
#define	BOAS	294
#define	NUM	295
#define	IDENTIFIER	296

#line 1 "t1.y"


#include "bison2cpp.h"

void initFlex( const char *_code );


#line 9 "t1.y"
typedef union
{
     int vali;
     double vald;
     char *name;
     void *ptr;
} YYSTYPE;
#include <stdio.h>

#ifndef __cplusplus
#ifndef __STDC__
#define const
#endif
#endif



#define	YYFINAL		200
#define	YYFLAG		-32768
#define	YYNTBASE	64

#define YYTRANSLATE(x) ((unsigned)(x) <= 296 ? yytranslate[x] : 100)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,    44,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,    54,     2,    42,
    43,    59,    57,    45,    58,    61,    60,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,    51,    48,    55,
    49,    56,    50,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
    62,     2,    63,    53,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,    46,    52,    47,     2,     2,     2,     2,     2,
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
     2,     2,     2,     2,     2,     1,     2,     3,     4,     5,
     6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
    16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
    26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
    36,    37,    38,    39,    40,    41
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     1,     4,    11,    13,    16,    17,    19,    21,    25,
    29,    30,    33,    35,    37,    41,    47,    51,    59,    67,
    73,    76,    79,    85,    89,    91,    94,    95,    97,   101,
   104,   107,   110,   112,   114,   118,   120,   124,   125,   127,
   129,   132,   134,   138,   142,   146,   150,   154,   158,   162,
   166,   170,   172,   178,   180,   184,   186,   190,   192,   196,
   198,   202,   204,   208,   210,   214,   218,   220,   224,   228,
   232,   236,   238,   242,   246,   248,   252,   256,   258,   262,
   266,   268,   271,   274,   277,   280,   283,   286,   289,   293,
   295,   297,   302,   306,   308,   313,   318,   320,   324,   325,
   327,   329,   333,   337,   339,   341,   343,   345,   347,   349,
   351
};

static const short yyrhs[] = {    -1,
    64,    65,     0,     3,    41,    42,    66,    43,    68,     0,
    69,     0,     1,    44,     0,     0,    67,     0,    41,     0,
    41,    45,    67,     0,    46,    69,    47,     0,     0,    70,
    69,     0,    68,     0,    48,     0,     4,    72,    70,     0,
     4,    72,    70,     5,    70,     0,     8,    72,    70,     0,
    73,    48,    78,    48,    78,    43,    70,     0,    74,    48,
    78,    48,    78,    43,    70,     0,    74,     6,    79,    43,
    70,     0,    23,    71,     0,    24,    71,     0,     7,    42,
    79,    43,    70,     0,    25,    78,    71,     0,    68,     0,
    75,    71,     0,     0,    48,     0,    42,    79,    43,     0,
     9,    42,     0,    73,    75,     0,    26,    76,     0,    80,
     0,    77,     0,    77,    45,    76,     0,    41,     0,    41,
    49,    80,     0,     0,    79,     0,    80,     0,    80,    79,
     0,    81,     0,    81,    49,    80,     0,    81,    32,    80,
     0,    81,    33,    80,     0,    81,    34,    80,     0,    81,
    35,    80,     0,    81,    36,    80,     0,    81,    37,    80,
     0,    81,    38,    80,     0,    81,    39,    80,     0,    82,
     0,    82,    50,    80,    51,    80,     0,    83,     0,    83,
    14,    82,     0,    84,     0,    84,    15,    83,     0,    85,
     0,    85,    52,    84,     0,    86,     0,    86,    53,    85,
     0,    87,     0,    87,    54,    86,     0,    88,     0,    88,
    12,    87,     0,    88,    13,    87,     0,    89,     0,    88,
    55,    89,     0,    88,    56,    89,     0,    88,    30,    89,
     0,    88,    31,    89,     0,    90,     0,    90,    10,    89,
     0,    90,    11,    89,     0,    91,     0,    91,    57,    90,
     0,    91,    58,    90,     0,    92,     0,    92,    59,    91,
     0,    92,    60,    91,     0,    95,     0,    58,    92,     0,
    27,    95,     0,    28,    95,     0,    95,    27,     0,    95,
    28,     0,    21,    93,     0,    22,    95,     0,    16,    61,
    94,     0,    94,     0,    41,     0,    41,    42,    97,    43,
     0,    41,    61,    94,     0,    96,     0,    96,    62,    79,
    63,     0,    96,    42,    97,    43,     0,    99,     0,    95,
    61,    41,     0,     0,    98,     0,    80,     0,    80,    45,
    98,     0,    42,    79,    43,     0,    41,     0,    40,     0,
    18,     0,    29,     0,    20,     0,    19,     0,    17,     0,
    16,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
    91,    92,    95,    96,    97,   100,   101,   104,   105,   108,
   111,   112,   113,   116,   117,   118,   119,   120,   121,   122,
   123,   124,   125,   126,   127,   128,   131,   132,   135,   138,
   141,   144,   145,   148,   149,   152,   153,   156,   157,   160,
   161,   164,   165,   166,   167,   168,   169,   170,   171,   172,
   173,   176,   177,   180,   181,   184,   185,   188,   189,   192,
   193,   196,   197,   200,   201,   202,   205,   206,   207,   208,
   209,   212,   213,   214,   217,   218,   219,   222,   223,   224,
   227,   228,   229,   230,   231,   232,   233,   234,   237,   238,
   241,   242,   243,   246,   247,   248,   251,   252,   255,   256,
   259,   260,   263,   264,   265,   266,   267,   268,   269,   270,
   271
};
#endif


#if YYDEBUG != 0 || defined (YYERROR_VERBOSE)

static const char * const yytname[] = {   "$","error","$undefined.","FUNCTION",
"IF","ELSE","IN","WITH","WHILE","FOR","SHIFT_LEFT","SHIFT_RIGHT","EQ","NEQ",
"OR","AND","THIS","B_NULL","FLOAT","B_TRUE","B_FALSE","NEW","DELETE","BREAK",
"CONTINUE","RETURN","VAR","PP","MM","STRING","LEQ","GEQ","MAS","DAS","AAS","SAS",
"PAS","RAS","BAAS","BOAS","NUM","IDENTIFIER","'('","')'","'\\n'","','","'{'",
"'}'","';'","'='","'?'","':'","'|'","'^'","'&'","'<'","'>'","'+'","'-'","'*'",
"'/'","'.'","'['","']'","input","element","parameterListOpt","parameterList",
"compoundStatement","statements","statement","semiOpt","condition","forParen",
"forBegin","variablesOrExpression","variables","variable","expressionOpt","expression",
"assignmentExpression","conditionalExpression","orExpression","andExpression",
"bitwiseOrExpression","bitwiseXorExpression","bitwiseAndExpression","equalityExpression",
"relationalExpression","shiftExpression","additiveExpression","multiplicativeExpression",
"unaryExpression","constructor","constructorCall","simpleExpression","memberExpression",
"argumentListOpt","argumentList","primaryExpression", NULL
};
#endif

static const short yyr1[] = {     0,
    64,    64,    65,    65,    65,    66,    66,    67,    67,    68,
    69,    69,    69,    70,    70,    70,    70,    70,    70,    70,
    70,    70,    70,    70,    70,    70,    71,    71,    72,    73,
    74,    75,    75,    76,    76,    77,    77,    78,    78,    79,
    79,    80,    80,    80,    80,    80,    80,    80,    80,    80,
    80,    81,    81,    82,    82,    83,    83,    84,    84,    85,
    85,    86,    86,    87,    87,    87,    88,    88,    88,    88,
    88,    89,    89,    89,    90,    90,    90,    91,    91,    91,
    92,    92,    92,    92,    92,    92,    92,    92,    93,    93,
    94,    94,    94,    95,    95,    95,    96,    96,    97,    97,
    98,    98,    99,    99,    99,    99,    99,    99,    99,    99,
    99
};

static const short yyr2[] = {     0,
     0,     2,     6,     1,     2,     0,     1,     1,     3,     3,
     0,     2,     1,     1,     3,     5,     3,     7,     7,     5,
     2,     2,     5,     3,     1,     2,     0,     1,     3,     2,
     2,     2,     1,     1,     3,     1,     3,     0,     1,     1,
     2,     1,     3,     3,     3,     3,     3,     3,     3,     3,
     3,     1,     5,     1,     3,     1,     3,     1,     3,     1,
     3,     1,     3,     1,     3,     3,     1,     3,     3,     3,
     3,     1,     3,     3,     1,     3,     3,     1,     3,     3,
     1,     2,     2,     2,     2,     2,     2,     2,     3,     1,
     1,     4,     3,     1,     4,     4,     1,     3,     0,     1,
     1,     3,     3,     1,     1,     1,     1,     1,     1,     1,
     1
};

static const short yydefact[] = {     1,
     0,     0,     0,     0,     0,     0,     0,   111,   110,   106,
   109,   108,     0,     0,    27,    27,    38,     0,     0,     0,
   107,   105,   104,     0,    11,    14,     0,     2,    13,     4,
    11,     0,     0,    27,    33,    42,    52,    54,    56,    58,
    60,    62,    64,    67,    72,    75,    78,    81,    94,    97,
     5,     0,     0,     0,     0,     0,    30,     0,    91,    87,
    90,    88,    28,    21,    22,    27,    39,    40,    36,    32,
    34,    83,    84,     0,     0,    82,    12,    38,    31,     0,
    38,    26,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,    85,
    86,     0,    99,     0,     6,     0,    25,    15,     0,    17,
     0,    99,     0,    24,    41,     0,     0,   103,    10,     0,
     0,     0,    44,    45,    46,    47,    48,    49,    50,    51,
    43,     0,    55,    57,    59,    61,    63,    65,    66,    70,
    71,    68,    69,    73,    74,    76,    77,    79,    80,    98,
   101,     0,   100,     0,     8,     0,     7,    29,     0,     0,
    89,     0,    93,    37,    35,    38,     0,    38,     0,     0,
    96,    95,     0,     0,    16,    23,    92,     0,    20,     0,
    53,   102,     9,     3,     0,     0,    18,    19,     0,     0
};

static const short yydefgoto[] = {     1,
    28,   166,   167,   117,    30,    31,    64,    54,    32,    33,
    34,    70,    71,    66,    67,    35,    36,    37,    38,    39,
    40,    41,    42,    43,    44,    45,    46,    47,    60,    61,
    48,    49,   162,   163,    50
};

static const short yypact[] = {-32768,
   129,   -12,     4,    18,    21,    18,    24,-32768,-32768,-32768,
-32768,-32768,     0,     2,    37,    37,   238,    55,     2,     2,
-32768,-32768,-32768,   238,   172,-32768,   238,-32768,-32768,-32768,
   172,   205,     3,    37,-32768,    45,    52,    93,    94,    56,
    58,    59,    -2,-32768,    16,   -10,    33,   -15,   -27,-32768,
-32768,    70,   238,   172,   238,   172,-32768,    53,   -28,-32768,
-32768,    54,-32768,-32768,-32768,    37,-32768,   238,    67,-32768,
    74,    54,    54,    77,    75,-32768,-32768,   238,-32768,   238,
   238,-32768,   238,   238,   238,   238,   238,   238,   238,   238,
   238,   238,   238,   238,   238,   238,   238,   238,   238,   238,
   238,   238,   238,   238,   238,   238,   238,   238,   238,-32768,
-32768,    80,   238,   238,    82,    81,-32768,   121,    84,-32768,
    87,   238,    87,-32768,-32768,   238,    55,-32768,-32768,    83,
    91,    92,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,    88,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
    90,    98,-32768,    79,   114,   118,-32768,-32768,   172,   172,
-32768,   122,-32768,-32768,-32768,   238,   172,   238,   238,   238,
-32768,-32768,    82,   120,-32768,-32768,-32768,   124,-32768,   125,
-32768,-32768,-32768,-32768,   172,   172,-32768,-32768,   173,-32768
};

static const short yypgoto[] = {-32768,
-32768,-32768,   -11,    -1,    -8,   -52,    -9,   168,-32768,-32768,
   146,    57,-32768,   -75,   -19,   -16,-32768,    89,   108,   109,
   107,   110,   -40,-32768,   -14,    -7,    -4,   158,-32768,   -83,
    36,-32768,    64,    25,-32768
};


#define	YYLAST		296


static const short yytable[] = {    29,
    68,   118,   130,   120,    74,   132,    65,    68,    80,    98,
    99,   110,   111,   122,   113,    58,    75,     8,     9,    10,
    11,    12,    77,    29,    82,   104,   105,   100,   101,    29,
    21,    51,   123,   116,   114,   119,    68,   171,    68,   173,
    59,    22,    23,    24,    52,   112,   106,   107,   125,    62,
    81,    68,   102,   103,    72,    73,   124,   148,   149,    53,
   131,    68,    55,    68,    68,    57,   133,   134,   135,   136,
   137,   138,   139,   140,   141,   142,    83,    84,    85,    86,
    87,    88,    89,    90,    63,   150,   151,   152,   153,   154,
   155,   108,   109,    91,   164,    69,   161,    68,   156,   157,
   188,    92,   190,   158,   159,   161,    93,    95,    94,   174,
    96,   115,    97,   121,   112,   126,   185,   186,   127,   128,
   160,   129,   165,   168,   189,   169,   170,    59,   199,     2,
   176,     3,     4,   177,   180,     5,     6,     7,   179,   178,
   181,   182,   197,   198,     8,     9,    10,    11,    12,    13,
    14,    15,    16,    17,    18,    19,    20,    21,   183,    68,
   184,    68,   191,   161,   187,    25,   195,   196,    22,    23,
    24,   193,   200,    56,    25,     4,    26,    79,     5,     6,
     7,   143,   194,   175,    76,   172,    27,     8,     9,    10,
    11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
    21,   144,   146,   145,   192,     0,   147,     0,     0,     0,
     0,    22,    23,    24,     0,     0,     0,    25,     0,    26,
     8,     9,    10,    11,    12,    13,    14,     0,     0,    27,
    18,    19,    20,    21,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,    22,    23,    24,     0,     0,     0,
     0,     0,    78,     8,     9,    10,    11,    12,    13,    14,
     0,     0,    27,     0,    19,    20,    21,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,    22,    23,    24,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,    27
};

static const short yycheck[] = {     1,
    17,    54,    78,    56,    24,    81,    16,    24,     6,    12,
    13,    27,    28,    42,    42,    16,    25,    16,    17,    18,
    19,    20,    31,    25,    34,    10,    11,    30,    31,    31,
    29,    44,    61,    53,    62,    55,    53,   121,    55,   123,
    41,    40,    41,    42,    41,    61,    57,    58,    68,    14,
    48,    68,    55,    56,    19,    20,    66,    98,    99,    42,
    80,    78,    42,    80,    81,    42,    83,    84,    85,    86,
    87,    88,    89,    90,    91,    92,    32,    33,    34,    35,
    36,    37,    38,    39,    48,   100,   101,   102,   103,   104,
   105,    59,    60,    49,   114,    41,   113,   114,   106,   107,
   176,    50,   178,   108,   109,   122,    14,    52,    15,   126,
    53,    42,    54,    61,    61,    49,   169,   170,    45,    43,
    41,    47,    41,    43,   177,     5,    43,    41,     0,     1,
    48,     3,     4,    43,    45,     7,     8,     9,    51,    48,
    43,    63,   195,   196,    16,    17,    18,    19,    20,    21,
    22,    23,    24,    25,    26,    27,    28,    29,    45,   176,
    43,   178,   179,   180,    43,    46,    43,    43,    40,    41,
    42,   183,     0,     6,    46,     4,    48,    32,     7,     8,
     9,    93,   184,   127,    27,   122,    58,    16,    17,    18,
    19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
    29,    94,    96,    95,   180,    -1,    97,    -1,    -1,    -1,
    -1,    40,    41,    42,    -1,    -1,    -1,    46,    -1,    48,
    16,    17,    18,    19,    20,    21,    22,    -1,    -1,    58,
    26,    27,    28,    29,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    40,    41,    42,    -1,    -1,    -1,
    -1,    -1,    48,    16,    17,    18,    19,    20,    21,    22,
    -1,    -1,    58,    -1,    27,    28,    29,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    40,    41,    42,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    58
};
/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
#line 3 "/usr/share/bison.simple"

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
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

#ifndef alloca
#ifdef __GNUC__
#define alloca __builtin_alloca
#else /* not GNU C.  */
#if (!defined (__STDC__) && defined (sparc)) || defined (__sparc__) || defined (__sparc) || defined (__sgi)
#include <alloca.h>
#else /* not sparc */
#if defined (MSDOS) && !defined (__TURBOC__)
#include <malloc.h>
#else /* not MSDOS, or __TURBOC__ */
#if defined(_AIX)
#include <malloc.h>
 #pragma alloca
#else /* not MSDOS, __TURBOC__, or _AIX */
#ifdef __hpux
#ifdef __cplusplus
extern "C" {
void *alloca (unsigned int);
};
#else /* not __cplusplus */
void *alloca ();
#endif /* not __cplusplus */
#endif /* __hpux */
#endif /* not _AIX */
#endif /* not MSDOS, or __TURBOC__ */
#endif /* not sparc.  */
#endif /* not GNU C.  */
#endif /* alloca not defined.  */

/* This is the parser code that is written into each bison parser
  when the %semantic_parser declaration is not specified in the grammar.
  It was written by Richard Stallman by simplifying the hairy parser
  used when %semantic_parser is specified.  */

/* Note: there must be only one dollar sign in this file.
   It is replaced by the list of actions, each action
   as one case of the switch.  */

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		-2
#define YYEOF		0
#define YYACCEPT	return(0)
#define YYABORT 	return(1)
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

#ifndef YYPARSE_RETURN_TYPE
#define YYPARSE_RETURN_TYPE int
#endif

/* Prevent warning if -Wstrict-prototypes.  */
#ifdef __GNUC__
YYPARSE_RETURN_TYPE yyparse (void);
#endif

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
     int count;
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
__yy_memcpy (char *to, char *from, int count)
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#endif
#endif

#line 196 "/usr/share/bison.simple"

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
	  return 2;
	}
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;
      yyss = (short *) alloca (yystacksize * sizeof (*yyssp));
      __yy_memcpy ((char *)yyss, (char *)yyss1, size * sizeof (*yyssp));
      yyvs = (YYSTYPE *) alloca (yystacksize * sizeof (*yyvsp));
      __yy_memcpy ((char *)yyvs, (char *)yyvs1, size * sizeof (*yyvsp));
#ifdef YYLSP_NEEDED
      yyls = (YYLTYPE *) alloca (yystacksize * sizeof (*yylsp));
      __yy_memcpy ((char *)yyls, (char *)yyls1, size * sizeof (*yylsp));
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

case 2:
#line 92 "t1.y"
{ printf("!!!!! Adding code !!!!!\n"); jsAppendCode( yyvsp[0].ptr ); ;
    break;}
case 3:
#line 95 "t1.y"
{ printf("Function: '%s'\n",yyvsp[-4].name ); yyval.ptr = newJSFunction( yyvsp[-4].name, yyvsp[-2].ptr, yyvsp[0].ptr ); ;
    break;}
case 4:
#line 96 "t1.y"
{ printf("Statement\n"); yyval.ptr = yyvsp[0].ptr; ;
    break;}
case 5:
#line 97 "t1.y"
{ yyerrok; yyval.ptr = 0L; ;
    break;}
case 6:
#line 100 "t1.y"
{ yyval.ptr = 0L; ;
    break;}
case 7:
#line 101 "t1.y"
{ yyval.ptr = yyvsp[0].ptr; ;
    break;}
case 8:
#line 104 "t1.y"
{ printf("param '%s'\n", yyvsp[0].name); yyval.ptr = newJSParameter( yyvsp[0].name, 0L ); ;
    break;}
case 9:
#line 105 "t1.y"
{ printf("param '%s\n", yyvsp[-2].name); yyval.ptr = newJSParameter( yyvsp[-2].name, yyvsp[0].ptr ); ;
    break;}
case 10:
#line 108 "t1.y"
{ yyval.ptr = yyvsp[-1].ptr; ;
    break;}
case 11:
#line 111 "t1.y"
{ yyval.ptr = 0L; ;
    break;}
case 12:
#line 112 "t1.y"
{ printf(""); yyval.ptr = newJSStatement( yyvsp[-1].ptr, yyvsp[0].ptr ); ;
    break;}
case 13:
#line 113 "t1.y"
{ printf(""); yyval.ptr = yyvsp[0].ptr; ;
    break;}
case 14:
#line 116 "t1.y"
{ printf(""); yyval.ptr = 0L; ;
    break;}
case 15:
#line 117 "t1.y"
{ printf("Simple IF\n"); yyval.ptr = 0L; ;
    break;}
case 16:
#line 118 "t1.y"
{ printf("Complex IF\n"); yyval.ptr = 0L; ;
    break;}
case 17:
#line 119 "t1.y"
{ printf(""); yyval.ptr = 0L; ;
    break;}
case 18:
#line 120 "t1.y"
{ printf(""); yyval.ptr = 0L; ;
    break;}
case 19:
#line 121 "t1.y"
{ printf(""); yyval.ptr = 0L; ;
    break;}
case 20:
#line 122 "t1.y"
{ printf(""); yyval.ptr = 0L; ;
    break;}
case 21:
#line 123 "t1.y"
{ printf(""); yyval.ptr = 0L; ;
    break;}
case 22:
#line 124 "t1.y"
{ printf(""); yyval.ptr = 0L; ;
    break;}
case 23:
#line 125 "t1.y"
{ printf(""); yyval.ptr = 0L; ;
    break;}
case 24:
#line 126 "t1.y"
{ printf(""); yyval.ptr = 0L; ;
    break;}
case 25:
#line 127 "t1.y"
{ printf(""); yyval.ptr = yyvsp[0].ptr; ;
    break;}
case 26:
#line 128 "t1.y"
{ printf(""); yyval.ptr = yyvsp[-1].ptr; ;
    break;}
case 28:
#line 132 "t1.y"
{ printf(""); ;
    break;}
case 29:
#line 135 "t1.y"
{ printf("Condition\n"); ;
    break;}
case 30:
#line 138 "t1.y"
{ printf(""); ;
    break;}
case 31:
#line 141 "t1.y"
{ printf(""); ;
    break;}
case 32:
#line 144 "t1.y"
{ printf(""); yyval.ptr = 0L; ;
    break;}
case 33:
#line 145 "t1.y"
{ printf(""); yyval.ptr = yyvsp[0].ptr; ;
    break;}
case 34:
#line 148 "t1.y"
{ printf(""); ;
    break;}
case 35:
#line 149 "t1.y"
{ printf(""); ;
    break;}
case 36:
#line 152 "t1.y"
{ printf("Var: '%s'\n", yyvsp[0].name); ;
    break;}
case 37:
#line 153 "t1.y"
{ printf("Var with Assignment: '%s'\n", yyvsp[-2].name); ;
    break;}
case 38:
#line 156 "t1.y"
{ yyval.ptr = 0L; ;
    break;}
case 39:
#line 157 "t1.y"
{ printf(""); yyval.ptr = yyvsp[0].ptr; ;
    break;}
case 40:
#line 160 "t1.y"
{ printf(""); yyval.ptr = yyvsp[0].ptr; ;
    break;}
case 41:
#line 161 "t1.y"
{ printf(""); ;
    break;}
case 42:
#line 164 "t1.y"
{ printf(""); yyval.ptr = yyvsp[0].ptr; ;
    break;}
case 43:
#line 165 "t1.y"
{ printf("Assignment ( = )\n"); yyval.ptr = newJSAssignment( OP_ASSIGN, yyvsp[-2].ptr, yyvsp[0].ptr ); ;
    break;}
case 44:
#line 166 "t1.y"
{ printf("Assignment ( *= )\n"); ;
    break;}
case 45:
#line 167 "t1.y"
{ printf("Assignment ( /= )\n"); ;
    break;}
case 46:
#line 168 "t1.y"
{ printf("Assignment ( += )\n"); ;
    break;}
case 47:
#line 169 "t1.y"
{ printf("Assignment ( -= )\n"); ;
    break;}
case 48:
#line 170 "t1.y"
{ printf("Assignment ( ^= )\n"); ;
    break;}
case 49:
#line 171 "t1.y"
{ printf("Assignment ( %= )\n"); ;
    break;}
case 50:
#line 172 "t1.y"
{ printf("Assignment ( &= )\n"); ;
    break;}
case 51:
#line 173 "t1.y"
{ printf("Assignment ( |= )\n"); ;
    break;}
case 52:
#line 176 "t1.y"
{ printf(""); yyval.ptr = yyvsp[0].ptr; ;
    break;}
case 53:
#line 177 "t1.y"
{ printf(""); ;
    break;}
case 54:
#line 180 "t1.y"
{ printf(""); yyval.ptr = yyvsp[0].ptr; ;
    break;}
case 55:
#line 181 "t1.y"
{ printf(""); yyval.ptr = newJSBinaryOperator( OP_OR, yyvsp[-2].ptr, yyvsp[0].ptr ); ;
    break;}
case 56:
#line 184 "t1.y"
{ printf(""); yyval.ptr = yyvsp[0].ptr; ;
    break;}
case 57:
#line 185 "t1.y"
{ printf(""); yyval.ptr = newJSBinaryOperator( OP_AND, yyvsp[-2].ptr, yyvsp[0].ptr ); ;
    break;}
case 58:
#line 188 "t1.y"
{ printf(""); yyval.ptr = yyvsp[0].ptr; ;
    break;}
case 59:
#line 189 "t1.y"
{ printf(""); yyval.ptr = newJSBinaryOperator( OP_BOR, yyvsp[-2].ptr, yyvsp[0].ptr ); ;
    break;}
case 60:
#line 192 "t1.y"
{ printf(""); yyval.ptr = yyvsp[0].ptr; ;
    break;}
case 61:
#line 193 "t1.y"
{ printf(""); yyval.ptr = newJSBinaryOperator( OP_BXOR, yyvsp[-2].ptr, yyvsp[0].ptr ); ;
    break;}
case 62:
#line 196 "t1.y"
{ printf(""); yyval.ptr = yyvsp[0].ptr; ;
    break;}
case 63:
#line 197 "t1.y"
{ printf(""); yyval.ptr = newJSBinaryOperator( OP_BAND, yyvsp[-2].ptr, yyvsp[0].ptr ); ;
    break;}
case 64:
#line 200 "t1.y"
{ printf(""); yyval.ptr = yyvsp[0].ptr; ;
    break;}
case 65:
#line 201 "t1.y"
{ printf(""); yyval.ptr = newJSBinaryOperator( OP_EQ, yyvsp[-2].ptr, yyvsp[0].ptr ); ;
    break;}
case 66:
#line 202 "t1.y"
{ printf(""); yyval.ptr = newJSBinaryOperator( OP_NEQ, yyvsp[-2].ptr, yyvsp[0].ptr ); ;
    break;}
case 67:
#line 205 "t1.y"
{ printf(""); yyval.ptr = yyvsp[0].ptr; ;
    break;}
case 68:
#line 206 "t1.y"
{ printf(""); yyval.ptr = newJSBinaryOperator( OP_LT, yyvsp[-2].ptr, yyvsp[0].ptr ); ;
    break;}
case 69:
#line 207 "t1.y"
{ printf(""); yyval.ptr = newJSBinaryOperator( OP_GT, yyvsp[-2].ptr, yyvsp[0].ptr ); ;
    break;}
case 70:
#line 208 "t1.y"
{ printf(""); yyval.ptr = newJSBinaryOperator( OP_LEQ, yyvsp[-2].ptr, yyvsp[0].ptr ); ;
    break;}
case 71:
#line 209 "t1.y"
{ printf(""); yyval.ptr = newJSBinaryOperator( OP_GEQ, yyvsp[-2].ptr, yyvsp[0].ptr ); ;
    break;}
case 72:
#line 212 "t1.y"
{ printf(""); yyval.ptr = yyvsp[0].ptr; ;
    break;}
case 73:
#line 213 "t1.y"
{ printf(""); yyval.ptr = newJSBinaryOperator( OP_SL, yyvsp[-2].ptr, yyvsp[0].ptr ); ;
    break;}
case 74:
#line 214 "t1.y"
{ printf(""); yyval.ptr = newJSBinaryOperator( OP_SR, yyvsp[-2].ptr, yyvsp[0].ptr );;
    break;}
case 75:
#line 217 "t1.y"
{ printf(""); yyval.ptr = yyvsp[0].ptr; ;
    break;}
case 76:
#line 218 "t1.y"
{ printf("Add ( + )\n"); yyval.ptr = newJSBinaryOperator( OP_ADD, yyvsp[-2].ptr, yyvsp[0].ptr ); ;
    break;}
case 77:
#line 219 "t1.y"
{ printf("Sub ( - )\n"); yyval.ptr = newJSBinaryOperator( OP_SUB, yyvsp[-2].ptr, yyvsp[0].ptr ); ;
    break;}
case 78:
#line 222 "t1.y"
{ printf(""); yyval.ptr = yyvsp[0].ptr; ;
    break;}
case 79:
#line 223 "t1.y"
{ printf("Mul ( * )\n"); yyval.ptr = newJSBinaryOperator( OP_MUL, yyvsp[-2].ptr, yyvsp[0].ptr ); ;
    break;}
case 80:
#line 224 "t1.y"
{ printf("Div ( / )\n"); yyval.ptr = newJSBinaryOperator( OP_DIV, yyvsp[-2].ptr, yyvsp[0].ptr ); ;
    break;}
case 81:
#line 227 "t1.y"
{ printf(""); yyval.ptr = yyvsp[0].ptr; ;
    break;}
case 82:
#line 228 "t1.y"
{ printf("Unary Minus\n"); ;
    break;}
case 83:
#line 229 "t1.y"
{ printf("++ Prefix\n"); ;
    break;}
case 84:
#line 230 "t1.y"
{ printf("-- Prefix\n"); ;
    break;}
case 85:
#line 231 "t1.y"
{ printf("Postfix ++\n"); ;
    break;}
case 86:
#line 232 "t1.y"
{ printf("Postfix --\n"); ;
    break;}
case 87:
#line 233 "t1.y"
{ printf("new\n"); yyval.ptr = yyvsp[0].ptr; ;
    break;}
case 88:
#line 234 "t1.y"
{ printf("delete\n"); ;
    break;}
case 89:
#line 237 "t1.y"
{ printf(""); yyval.ptr = 0L; ;
    break;}
case 90:
#line 238 "t1.y"
{ printf(""); yyval.ptr = yyvsp[0].ptr; ;
    break;}
case 91:
#line 241 "t1.y"
{ printf(""); yyval.ptr = newJSConstructorCall( newJSIdentifier( yyvsp[0].name ) , 0L ); ;
    break;}
case 92:
#line 242 "t1.y"
{ printf(""); yyval.ptr = newJSConstructorCall( newJSIdentifier( yyvsp[-3].name ), yyvsp[-1].ptr ); ;
    break;}
case 93:
#line 243 "t1.y"
{ printf(""); yyval.ptr = 0L; ;
    break;}
case 94:
#line 246 "t1.y"
{ printf(""); yyval.ptr = yyvsp[0].ptr; ;
    break;}
case 95:
#line 247 "t1.y"
{ printf("[ ]\n"); yyval.ptr = newJSArrayAccess( yyvsp[-3].ptr, yyvsp[-1].ptr ); ;
    break;}
case 96:
#line 248 "t1.y"
{ printf("Function Call\n"); yyval.ptr = newJSFunctionCall( yyvsp[-3].ptr, yyvsp[-1].ptr ); ;
    break;}
case 97:
#line 251 "t1.y"
{ printf(""); yyval.ptr = yyvsp[0].ptr; ;
    break;}
case 98:
#line 252 "t1.y"
{ printf("Member ( '%s' )\n", yyvsp[0].name ); yyval.ptr = newJSMember( yyvsp[-2].ptr, yyvsp[0].name ); ;
    break;}
case 99:
#line 255 "t1.y"
{ yyval.ptr = 0L; ;
    break;}
case 100:
#line 256 "t1.y"
{ printf("ArgumentList\n"); yyval.ptr = yyvsp[0].ptr; ;
    break;}
case 101:
#line 259 "t1.y"
{ printf("Argument\n"); yyval.ptr = newJSArgument( yyvsp[0].ptr, 0L ); ;
    break;}
case 102:
#line 260 "t1.y"
{ printf("Argument (cont)\n"); yyval.ptr = newJSArgument( yyvsp[-2].ptr, yyvsp[0].ptr ); ;
    break;}
case 103:
#line 263 "t1.y"
{ printf("Paranthesis\n"); yyval.ptr = yyvsp[-1].ptr; ;
    break;}
case 104:
#line 264 "t1.y"
{ printf("ID '%s'\n",yyvsp[0].name); yyval.ptr = newJSIdentifier( yyvsp[0].name ); ;
    break;}
case 105:
#line 265 "t1.y"
{ printf("NUM\n"); yyval.ptr = newJSInteger( yyvsp[0].vali ); ;
    break;}
case 106:
#line 266 "t1.y"
{ printf(""); yyval.ptr = newJSFloat( yyvsp[0].vald ); ;
    break;}
case 107:
#line 267 "t1.y"
{ printf(""); yyval.ptr = newJSString( yyvsp[0].name ); ;
    break;}
case 108:
#line 268 "t1.y"
{ printf(""); yyval.ptr = newJSBool( 0 ); ;
    break;}
case 109:
#line 269 "t1.y"
{ printf(""); yyval.ptr = newJSBool( 1 ); ;
    break;}
case 110:
#line 270 "t1.y"
{ printf(""); yyval.ptr = newJSNull(); ;
    break;}
case 111:
#line 271 "t1.y"
{ printf(""); yyval.ptr = newJSThis(); ;
    break;}
}
   /* the action file gets copied in in place of this dollarsign */
#line 498 "/usr/share/bison.simple"

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
}
#line 276 "t1.y"


yyerror ( char *s )  /* Called by yyparse on error */
{
    printf ("ERROR: %s\n", s);
}

void mainParse( const char *_code )
{
    initFlex( _code );
    yyparse();
}






