
/*  A Bison parser, made from yacc.yy
    by GNU Bison version 1.28  */

#define YYBISON 1  /* Identify Bison output.  */

#define	T_CHARACTER_LITERAL	257
#define	T_DOUBLE_LITERAL	258
#define	T_IDENTIFIER	259
#define	T_INTEGER_LITERAL	260
#define	T_STRING_LITERAL	261
#define	T_INCLUDE	262
#define	T_CLASS	263
#define	T_STRUCT	264
#define	T_LEFT_CURLY_BRACKET	265
#define	T_LEFT_PARANTHESIS	266
#define	T_RIGHT_CURLY_BRACKET	267
#define	T_RIGHT_PARANTHESIS	268
#define	T_COLON	269
#define	T_SEMICOLON	270
#define	T_PUBLIC	271
#define	T_PROTECTED	272
#define	T_TRIPE_DOT	273
#define	T_PRIVATE	274
#define	T_VIRTUAL	275
#define	T_CONST	276
#define	T_INLINE	277
#define	T_FRIEND	278
#define	T_RETURN	279
#define	T_SIGNAL	280
#define	T_SLOT	281
#define	T_TYPEDEF	282
#define	T_PLUS	283
#define	T_MINUS	284
#define	T_COMMA	285
#define	T_ASTERISK	286
#define	T_TILDE	287
#define	T_LESS	288
#define	T_GREATER	289
#define	T_AMPERSAND	290
#define	T_EXTERN	291
#define	T_EXTERN_C	292
#define	T_ACCESS	293
#define	T_ENUM	294
#define	T_NAMESPACE	295
#define	T_USING	296
#define	T_UNKNOWN	297
#define	T_TRIPLE_DOT	298
#define	T_TRUE	299
#define	T_FALSE	300
#define	T_STATIC	301
#define	T_EQUAL	302
#define	T_SCOPE	303
#define	T_NULL	304
#define	T_INT	305
#define	T_ARRAY_OPEN	306
#define	T_ARRAY_CLOSE	307
#define	T_CHAR	308
#define	T_DCOP	309
#define	T_DCOP_AREA	310
#define	T_DCOP_SIGNAL_AREA	311
#define	T_SIGNED	312
#define	T_UNSIGNED	313
#define	T_LONG	314
#define	T_SHORT	315
#define	T_FUNOPERATOR	316
#define	T_MISCOPERATOR	317
#define	T_SHIFT	318

#line 1 "yacc.yy"

/*****************************************************************
Copyright (c) 1999 Torben Weis <weis@kde.org>
Copyright (c) 2000 Matthias Ettrich <ettrich@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include <config.h>
#include <qstring.h>

#define AMP_ENTITY "&amp;"
#define YYERROR_VERBOSE

extern int yylex();

// extern QString idl_lexFile;
extern int idl_line_no;
extern int function_mode;

static int dcop_area = 0;
static int dcop_signal_area = 0;

static QString in_namespace( "" );

void dcopidlInitFlex( const char *_code );

void yyerror( const char *s )
{
	qDebug( "In line %i : %s", idl_line_no, s );
        exit(1);
	//   theParser->parse_error( idl_lexFile, s, idl_line_no );
}


#line 58 "yacc.yy"
typedef union
{
  long   _int;
  QString        *_str;
  unsigned short          _char;
  double _float;
} YYSTYPE;
#ifndef YYDEBUG
#define YYDEBUG 1
#endif

#include <stdio.h>

#ifndef __cplusplus
#ifndef __STDC__
#define const
#endif
#endif



#define	YYFINAL		356
#define	YYFLAG		-32768
#define	YYNTBASE	65

#define YYTRANSLATE(x) ((unsigned)(x) <= 318 ? yytranslate[x] : 118)

static const char yytranslate[] = {     0,
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
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     1,     3,     4,     5,     6,
     7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
    17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
    27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
    37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
    47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
    57,    58,    59,    60,    61,    62,    63,    64
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     4,     5,     8,    13,    14,    16,    17,    24,    28,
    32,    38,    39,    47,    52,    58,    61,    66,    74,    83,
    86,    88,    90,    92,    95,    96,    98,   100,   102,   104,
   106,   108,   110,   111,   115,   118,   121,   124,   126,   130,
   132,   137,   141,   143,   146,   150,   153,   155,   156,   158,
   160,   163,   166,   169,   172,   175,   178,   181,   187,   192,
   197,   202,   207,   214,   222,   229,   236,   242,   246,   248,
   252,   254,   256,   258,   261,   263,   265,   267,   271,   275,
   283,   293,   300,   301,   303,   305,   308,   310,   313,   316,
   320,   323,   327,   330,   334,   337,   341,   343,   345,   348,
   350,   353,   355,   358,   361,   364,   366,   367,   369,   373,
   375,   377,   380,   383,   388,   395,   399,   401,   404,   406,
   410,   414,   417,   420,   422,   425,   429,   431,   435,   438,
   440,   441,   444,   450,   452,   454,   456,   458,   463,   464,
   466,   468,   470,   472,   474,   476,   483,   491,   493,   495,
   499,   501,   505,   510,   512,   516,   519,   525,   529,   535,
   543,   550,   554,   556,   558,   562,   567,   570,   571,   573,
   576,   577,   579,   583,   586,   589,   593,   599,   605
};

static const short yyrhs[] = {    66,
    68,    65,     0,     0,     8,    66,     0,    38,    11,    65,
    13,     0,     0,    55,     0,     0,     9,    77,    81,    67,
    83,    16,     0,     9,    77,    16,     0,    10,    77,    16,
     0,    10,    77,    81,    83,    16,     0,     0,    41,     5,
    11,    69,    65,    13,    82,     0,    42,    41,     5,    16,
     0,    42,     5,    49,     5,    16,     0,    37,    16,     0,
    28,    97,    77,    16,     0,    28,    10,    11,    70,    13,
    77,    16,     0,    28,    10,    77,    11,    70,    13,    77,
    16,     0,    23,   109,     0,   109,     0,   117,     0,    84,
     0,   117,    70,     0,     0,    45,     0,    46,     0,    20,
     0,    18,     0,    17,     0,    26,     0,    27,     0,     0,
    72,    73,    15,     0,    73,    15,     0,    56,    15,     0,
    57,    15,     0,     5,     0,     5,    49,    77,     0,    77,
     0,    77,    34,    98,    35,     0,   102,    17,    78,     0,
    78,     0,    79,    11,     0,    79,    31,    80,     0,    15,
    80,     0,    11,     0,     0,    16,     0,    13,     0,    89,
    83,     0,   109,    83,     0,    76,    83,     0,    84,    83,
     0,    75,    83,     0,    74,    83,     0,   117,    83,     0,
    24,     9,    77,    16,    83,     0,    24,    77,    16,    83,
     0,    24,   104,    16,    83,     0,     9,    77,    16,    83,
     0,    10,    77,    16,    83,     0,    42,     5,    49,     5,
    16,    83,     0,    40,     5,    11,    85,    13,     5,    16,
     0,    40,     5,    11,    85,    13,    16,     0,    40,    11,
    85,    13,     5,    16,     0,    40,    11,    85,    13,    16,
     0,    86,    31,    85,     0,    86,     0,     5,    48,    88,
     0,     5,     0,     3,     0,     6,     0,    30,     6,     0,
    50,     0,    77,     0,    87,     0,    87,    29,    87,     0,
    87,    64,    87,     0,    28,    77,    34,    98,    35,    77,
    16,     0,    28,    77,    34,    98,    35,    49,     5,    77,
    16,     0,    10,    77,    11,    65,    13,    16,     0,     0,
    22,     0,    58,     0,    58,    51,     0,    59,     0,    59,
    51,     0,    58,    61,     0,    58,    61,    51,     0,    58,
    60,     0,    58,    60,    51,     0,    59,    61,     0,    59,
    61,    51,     0,    59,    60,     0,    59,    60,    51,     0,
    51,     0,    60,     0,    60,    51,     0,    61,     0,    61,
    51,     0,    54,     0,    58,    54,     0,    59,    54,     0,
    32,    92,     0,    32,     0,     0,    99,     0,    93,    31,
    99,     0,    91,     0,    77,     0,    10,    77,     0,     9,
    77,     0,    77,    34,    95,    35,     0,    77,    34,    95,
    35,    49,    77,     0,    96,    31,    95,     0,    96,     0,
    94,    92,     0,    94,     0,    22,    94,    92,     0,    22,
    94,    36,     0,    22,    94,     0,    94,    36,     0,    94,
     0,    94,    92,     0,    97,    31,    98,     0,    97,     0,
    97,    77,   100,     0,    97,   100,     0,    44,     0,     0,
    48,   101,     0,    48,    12,    97,    14,   101,     0,     7,
     0,    88,     0,     4,     0,    71,     0,    77,    12,    93,
    14,     0,     0,    21,     0,    63,     0,    64,     0,    35,
     0,    34,     0,    48,     0,    97,    77,    12,    93,    14,
    90,     0,    97,    62,   103,    12,    93,    14,    90,     0,
    87,     0,    71,     0,     5,    12,    14,     0,   105,     0,
   105,    31,   106,     0,     5,    12,   106,    14,     0,   107,
     0,   107,    31,   108,     0,   104,   111,     0,    21,   104,
    48,    50,   111,     0,    21,   104,   111,     0,    77,    12,
    93,    14,   111,     0,    77,    12,    93,    14,    15,   108,
   111,     0,   102,    33,    77,    12,    14,   111,     0,    47,
   104,   111,     0,    11,     0,    16,     0,   110,   112,    13,
     0,   110,   112,    13,    16,     0,   113,   112,     0,     0,
    16,     0,    31,   116,     0,     0,     5,     0,     5,    48,
   101,     0,    92,     5,     0,   115,   114,     0,    97,   116,
    16,     0,    97,    77,    15,     6,    16,     0,    47,    97,
     5,   100,    16,     0,    97,     5,    52,    88,    53,    16,
     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
   154,   157,   160,   164,   167,   173,   174,   177,   182,   185,
   188,   191,   195,   203,   206,   209,   212,   215,   218,   221,
   224,   227,   230,   235,   236,   238,   238,   240,   240,   240,
   242,   242,   242,   245,   250,   257,   263,   269,   272,   279,
   285,   293,   297,   304,   308,   316,   320,   327,   330,   334,
   338,   342,   346,   350,   354,   358,   362,   366,   370,   374,
   378,   382,   386,   392,   393,   394,   395,   398,   399,   402,
   403,   406,   407,   408,   409,   410,   413,   414,   415,   418,
   428,   433,   438,   442,   449,   450,   451,   452,   453,   454,
   455,   456,   457,   458,   459,   460,   461,   462,   463,   464,
   465,   466,   467,   468,   471,   472,   475,   479,   480,   488,
   489,   490,   491,   492,   498,   508,   512,   519,   524,   532,
   537,   544,   549,   554,   559,   567,   571,   577,   586,   594,
   602,   605,   608,   614,   617,   620,   623,   626,   632,   633,
   636,   636,   636,   636,   636,   639,   670,   678,   679,   680,
   683,   684,   687,   690,   691,   694,   698,   702,   706,   712,
   718,   724,   736,   742,   743,   744,   747,   748,   751,   754,
   755,   757,   758,   759,   761,   764,   765,   766,   767
};
#endif


#if YYDEBUG != 0 || defined (YYERROR_VERBOSE)

static const char * const yytname[] = {   "$","error","$undefined.","T_CHARACTER_LITERAL",
"T_DOUBLE_LITERAL","T_IDENTIFIER","T_INTEGER_LITERAL","T_STRING_LITERAL","T_INCLUDE",
"T_CLASS","T_STRUCT","T_LEFT_CURLY_BRACKET","T_LEFT_PARANTHESIS","T_RIGHT_CURLY_BRACKET",
"T_RIGHT_PARANTHESIS","T_COLON","T_SEMICOLON","T_PUBLIC","T_PROTECTED","T_TRIPE_DOT",
"T_PRIVATE","T_VIRTUAL","T_CONST","T_INLINE","T_FRIEND","T_RETURN","T_SIGNAL",
"T_SLOT","T_TYPEDEF","T_PLUS","T_MINUS","T_COMMA","T_ASTERISK","T_TILDE","T_LESS",
"T_GREATER","T_AMPERSAND","T_EXTERN","T_EXTERN_C","T_ACCESS","T_ENUM","T_NAMESPACE",
"T_USING","T_UNKNOWN","T_TRIPLE_DOT","T_TRUE","T_FALSE","T_STATIC","T_EQUAL",
"T_SCOPE","T_NULL","T_INT","T_ARRAY_OPEN","T_ARRAY_CLOSE","T_CHAR","T_DCOP",
"T_DCOP_AREA","T_DCOP_SIGNAL_AREA","T_SIGNED","T_UNSIGNED","T_LONG","T_SHORT",
"T_FUNOPERATOR","T_MISCOPERATOR","T_SHIFT","main","includes","dcoptag","declaration",
"@1","member_list","bool_value","nodcop_area","sigslot","nodcop_area_begin",
"dcop_area_begin","dcop_signal_area_begin","Identifier","super_class_name","super_class",
"super_classes","class_header","opt_semicolon","body","enum","enum_list","enum_item",
"number","int_expression","typedef","const_qualifier","int_type","asterisks",
"params","type_name","templ_type_list","templ_type","type","type_list","param",
"default","default_value","virtual_qualifier","operator","function_header","argument",
"arguments","init_item","init_list","function","function_begin","function_body",
"function_lines","function_line","Identifier_list_rest","Identifier_list_entry",
"Identifier_list","member", NULL
};
#endif

static const short yyr1[] = {     0,
    65,    65,    66,    66,    66,    67,    67,    68,    68,    68,
    68,    69,    68,    68,    68,    68,    68,    68,    68,    68,
    68,    68,    68,    70,    70,    71,    71,    72,    72,    72,
    73,    73,    73,    74,    74,    75,    76,    77,    77,    78,
    78,    79,    79,    80,    80,    81,    81,    82,    82,    83,
    83,    83,    83,    83,    83,    83,    83,    83,    83,    83,
    83,    83,    83,    84,    84,    84,    84,    85,    85,    86,
    86,    87,    87,    87,    87,    87,    88,    88,    88,    89,
    89,    89,    90,    90,    91,    91,    91,    91,    91,    91,
    91,    91,    91,    91,    91,    91,    91,    91,    91,    91,
    91,    91,    91,    91,    92,    92,    93,    93,    93,    94,
    94,    94,    94,    94,    94,    95,    95,    96,    96,    97,
    97,    97,    97,    97,    97,    98,    98,    99,    99,    99,
   100,   100,   100,   101,   101,   101,   101,   101,   102,   102,
   103,   103,   103,   103,   103,   104,   104,   105,   105,   105,
   106,   106,   107,   108,   108,   109,   109,   109,   109,   109,
   109,   109,   110,   111,   111,   111,   112,   112,   113,   114,
   114,   115,   115,   115,   116,   117,   117,   117,   117
};

static const short yyr2[] = {     0,
     3,     0,     2,     4,     0,     1,     0,     6,     3,     3,
     5,     0,     7,     4,     5,     2,     4,     7,     8,     2,
     1,     1,     1,     2,     0,     1,     1,     1,     1,     1,
     1,     1,     0,     3,     2,     2,     2,     1,     3,     1,
     4,     3,     1,     2,     3,     2,     1,     0,     1,     1,
     2,     2,     2,     2,     2,     2,     2,     5,     4,     4,
     4,     4,     6,     7,     6,     6,     5,     3,     1,     3,
     1,     1,     1,     2,     1,     1,     1,     3,     3,     7,
     9,     6,     0,     1,     1,     2,     1,     2,     2,     3,
     2,     3,     2,     3,     2,     3,     1,     1,     2,     1,
     2,     1,     2,     2,     2,     1,     0,     1,     3,     1,
     1,     2,     2,     4,     6,     3,     1,     2,     1,     3,
     3,     2,     2,     1,     2,     3,     1,     3,     2,     1,
     0,     2,     5,     1,     1,     1,     1,     4,     0,     1,
     1,     1,     1,     1,     1,     6,     7,     1,     1,     3,
     1,     3,     4,     1,     3,     2,     5,     3,     5,     7,
     6,     3,     1,     1,     3,     4,     2,     0,     1,     2,
     0,     1,     3,     2,     2,     3,     5,     5,     6
};

static const short yydefact[] = {     5,
     5,     0,   139,     3,     5,    38,     0,     0,   140,     0,
   139,     0,     0,     0,     0,     0,     0,    97,   102,    85,
    87,    98,   100,     5,   111,    23,   110,   124,     0,     0,
     0,    21,    22,     0,     0,   113,   112,     0,     0,   111,
     0,     0,   122,     0,    20,     0,     0,    16,     0,     0,
     0,     0,     0,     0,     0,    86,   103,    91,    89,    88,
   104,    95,    93,    99,   101,     1,   107,     0,   106,   123,
   125,    38,     0,     0,     0,   171,     0,     0,   163,   164,
   168,   156,     4,    39,    47,   139,     9,     7,    10,    33,
   113,   112,     0,     0,   158,   121,   120,    25,   112,     0,
     0,    71,     0,    69,    12,     0,     0,    38,   162,    92,
    90,    96,    94,   130,     0,   131,   108,   119,     0,   117,
   105,     0,     0,   144,   143,   145,   141,   142,     0,   107,
     0,   174,     0,   175,   176,     0,   169,     0,   168,   140,
    40,    43,     0,    46,     0,     6,    33,     0,     0,    50,
    30,    29,    28,     0,    31,    32,     0,     0,     0,     0,
    33,     0,    33,    33,    33,     0,    33,    33,    33,    33,
     0,     0,     0,     0,    25,    25,    17,     0,     0,     0,
     0,     5,     0,    14,     0,     0,     0,     0,   131,   129,
   118,   114,     0,    72,   136,    73,   134,     0,    26,    27,
    75,   137,    76,    77,   135,   173,    76,     0,   107,     0,
     0,   172,   170,     0,   165,   167,     0,    44,   139,     0,
     0,   113,   112,     0,   111,     0,     0,     0,    36,    37,
     0,    35,    56,    55,    53,    11,    54,    51,    52,    57,
   157,     0,     0,     0,    24,     0,     0,    70,     0,    67,
    68,     0,    15,     0,   132,   178,     0,   159,   109,   128,
     0,   116,    74,   107,     0,     0,     0,     0,    83,   177,
     0,   166,   127,     0,    45,    42,     8,    33,     5,    33,
   113,    33,    33,     0,     0,    34,   131,     0,     0,     0,
    65,    66,    48,     0,     0,   154,     0,   115,     0,    78,
    79,   179,    83,    84,   146,   161,     0,    41,    61,     0,
    62,    33,    59,    60,     0,     0,    18,     0,    64,    49,
    13,     0,     0,     0,   160,   138,   147,   126,     0,    58,
     0,    33,    19,   133,    38,   149,   148,   151,     0,   155,
    82,     0,     0,    63,     0,     0,   153,     0,    80,   150,
   152,     0,    81,     0,     0,     0
};

static const short yydefgoto[] = {    34,
     3,   147,    24,   182,   173,   202,   161,   162,   163,   164,
   165,    40,   142,   143,   144,    88,   321,   166,   167,   103,
   104,   204,   205,   168,   305,    27,    75,   115,    28,   119,
   120,    29,   274,   117,   186,   206,    30,   129,    31,   338,
   339,   296,   297,   169,    81,    82,   138,   139,   134,    76,
    77,   170
};

static const short yypact[] = {    65,
   103,    34,    67,-32768,    97,   -16,   117,   117,   353,   400,
   184,   372,   183,    41,   213,    26,   353,-32768,-32768,   172,
   230,   177,   179,    56,    28,-32768,-32768,   111,    17,   194,
   218,-32768,-32768,   224,   117,   199,   235,   117,   117,   221,
    20,    52,   216,   353,-32768,   208,   117,-32768,   236,   254,
   253,   219,   269,    22,   218,-32768,-32768,   238,   241,-32768,
-32768,   242,   243,-32768,-32768,-32768,   244,   400,   265,-32768,
-32768,   100,   161,    81,   271,   268,   284,   117,-32768,-32768,
   285,-32768,-32768,-32768,-32768,   129,-32768,   251,-32768,   313,
-32768,-32768,   295,   258,-32768,-32768,-32768,   339,   302,   298,
   254,   267,   304,   290,-32768,   319,   309,    75,-32768,-32768,
-32768,-32768,-32768,-32768,   149,    23,-32768,   265,   292,   301,
-32768,   459,    36,-32768,-32768,-32768,-32768,-32768,   324,   244,
   332,-32768,   110,-32768,-32768,   330,-32768,   334,   285,-32768,
   316,-32768,   101,-32768,   335,-32768,   313,   117,   117,-32768,
-32768,-32768,-32768,   386,-32768,-32768,   117,   346,   341,   342,
   174,   344,   313,   313,   313,   338,   313,   313,   313,   313,
   218,   353,   352,   124,   339,   339,-32768,   355,    36,   169,
   254,    97,   350,-32768,   266,   360,   245,   244,   331,-32768,
-32768,   329,   400,-32768,-32768,-32768,-32768,   374,-32768,-32768,
-32768,-32768,   371,    73,-32768,-32768,-32768,   336,   244,   167,
   368,   337,-32768,   373,   376,-32768,   353,-32768,   129,   117,
   385,   387,   225,   117,   143,   390,   354,   366,-32768,-32768,
   401,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,   397,   117,   402,-32768,   405,   192,-32768,   403,-32768,
-32768,   407,-32768,   353,-32768,-32768,   416,-32768,-32768,-32768,
   117,-32768,-32768,   244,    36,    36,   406,   173,   412,-32768,
   218,-32768,   393,   394,-32768,-32768,-32768,   313,    97,   313,
   419,   313,   313,   353,   431,-32768,   331,   423,   117,   425,
-32768,-32768,   426,   424,   436,   418,   218,-32768,   176,-32768,
-32768,-32768,   412,-32768,-32768,-32768,   353,-32768,-32768,   430,
-32768,   313,-32768,-32768,   415,   437,-32768,   439,-32768,-32768,
-32768,   459,   422,   416,-32768,-32768,-32768,-32768,   440,-32768,
    29,   313,-32768,-32768,    68,-32768,-32768,   438,   443,-32768,
-32768,   465,   455,-32768,   460,   422,-32768,   117,-32768,-32768,
-32768,   457,-32768,   475,   476,-32768
};

static const short yypgoto[] = {     6,
   477,-32768,-32768,-32768,   107,  -193,-32768,   318,-32768,-32768,
-32768,    -3,   257,-32768,   261,   444,-32768,  -110,   479,   -83,
-32768,  -249,   -94,-32768,   180,-32768,   -19,  -128,    -7,   291,
-32768,     3,  -115,   297,   -97,  -184,   -79,-32768,     4,-32768,
   140,-32768,   163,   200,-32768,   -32,   349,-32768,-32768,-32768,
   357,     8
};


#define	YYLAST		509


static const short yytable[] = {    25,
   255,   210,    43,    36,    37,   354,   145,    25,    71,    95,
    33,    41,    42,    41,    47,   300,   301,   178,   190,    54,
    55,    72,   109,    97,     6,    74,   108,     6,   208,    66,
    52,    84,    35,     6,    91,    92,   221,    93,   194,    67,
     6,   196,    99,   100,     5,    49,    41,    55,    69,   121,
    93,    50,   233,   234,   235,    -2,   237,   238,   239,   240,
   118,    68,    79,     1,    -2,   198,    53,    80,    -2,   116,
   185,     6,     1,   337,   136,     7,     8,   342,    73,   345,
   268,    73,   141,    73,   248,   201,    25,     9,    10,    11,
  -131,   260,   130,     2,    12,   131,   337,   251,   191,    94,
   174,   265,     2,    13,     1,   175,    14,    15,    16,    -2,
     1,   218,   189,    17,   212,  -172,    35,    18,   203,   207,
    19,     6,   185,    35,    20,    21,    22,    23,    72,   336,
  -172,   219,   116,     6,     2,   299,   266,   334,   241,   145,
     2,    69,    69,    25,   222,   223,    70,   122,    35,   140,
   225,   123,   336,   227,   258,    69,    41,   226,   282,    25,
    25,    25,   187,    25,    25,    25,    25,   309,   315,   311,
   244,   313,   314,   249,   242,   207,    68,   174,   174,   188,
   269,   203,   175,   175,   250,   118,   303,   252,     6,   326,
   116,   328,    38,    39,   124,   125,   290,   188,    48,   155,
   156,   330,    32,   188,     9,    10,   188,   291,   126,    85,
    45,   116,     6,    86,    87,   141,   141,    51,    98,   273,
   281,   344,    56,   127,   128,    57,    78,    64,    79,    65,
    44,    58,    59,    80,    18,   279,    83,    19,   306,   288,
   280,    20,    21,    22,    23,    85,   101,    69,     6,    86,
    89,    96,    38,    39,    68,    79,   294,   298,   102,   257,
    80,   207,   207,   105,   325,    10,   116,   106,   194,   195,
     6,   196,   197,   107,    25,   132,    25,   254,    25,    25,
    60,   245,   246,    61,   310,   318,   273,   114,   110,    62,
    63,   111,   112,   113,    18,   198,    69,    19,   133,   135,
   137,    20,    21,    22,    23,   146,   130,   171,    25,   273,
   199,   200,   176,   177,   179,   201,   180,     6,   203,   207,
   181,   148,   149,   183,   184,   150,   192,   343,    25,   151,
   152,   193,   153,     9,    10,   209,   154,   211,   155,   156,
   157,   214,   207,     6,   352,  -139,   215,    38,    39,   217,
   228,   220,    14,   236,   158,   229,   230,     6,   232,    17,
    10,    38,    39,    18,   243,   253,    19,   247,   159,   160,
    20,    21,    22,    23,    10,   256,     6,   261,   185,   263,
    38,    46,   264,   270,   122,   172,   271,   284,   267,    18,
     6,   272,    19,    10,   224,    39,    20,    21,    22,    23,
   277,   287,   278,    18,     6,   283,    19,    10,    38,    39,
    20,    21,    22,    23,   285,   286,   131,   289,   292,   293,
   295,   302,    18,   307,   194,    19,   335,   196,   308,    20,
    21,    22,    23,   304,   312,   316,    18,   322,   317,    19,
   319,   320,   329,    20,    21,    22,    23,   323,   324,   331,
    18,   198,   332,    19,   333,   341,   347,    20,    21,    22,
    23,   194,   195,     6,   196,   197,   199,   200,   346,   348,
   349,   201,   353,   350,   355,   356,   276,     4,   231,   275,
    90,    26,   327,   262,   259,   351,   340,   216,   198,   213,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,   199,   200,     0,     0,     0,   201
};

static const short yycheck[] = {     3,
   185,   130,    10,     7,     8,     0,    86,    11,    28,    42,
     3,     9,     9,    11,    12,   265,   266,   101,   116,    17,
    17,     5,    55,    43,     5,    29,     5,     5,   123,    24,
     5,    35,    49,     5,    38,    39,   147,    41,     3,    12,
     5,     6,    46,    47,    11,     5,    44,    44,    32,    69,
    54,    11,   163,   164,   165,     0,   167,   168,   169,   170,
    68,    34,    11,     8,     0,    30,    41,    16,    13,    67,
    48,     5,     8,   323,    78,     9,    10,    49,    62,    12,
   209,    62,    86,    62,   179,    50,    90,    21,    22,    23,
    16,   189,    12,    38,    28,    15,   346,   181,   118,    48,
    98,    29,    38,    37,     8,    98,    40,    41,    42,    13,
     8,    11,   116,    47,     5,    16,    49,    51,   122,   123,
    54,     5,    48,    49,    58,    59,    60,    61,     5,   323,
    31,    31,   130,     5,    38,   264,    64,   322,   171,   219,
    38,    32,    32,   147,   148,   149,    36,    48,    49,    21,
   154,    52,   346,   157,   187,    32,   154,   154,    16,   163,
   164,   165,    14,   167,   168,   169,   170,   278,   284,   280,
   174,   282,   283,     5,   172,   179,    34,   175,   176,    31,
    14,   185,   175,   176,    16,   193,    14,   182,     5,    14,
   188,   307,     9,    10,    34,    35,     5,    31,    16,    26,
    27,   312,     3,    31,    21,    22,    31,    16,    48,    11,
    11,   209,     5,    15,    16,   219,   220,     5,    11,   217,
   224,   332,    51,    63,    64,    54,    33,    51,    11,    51,
    47,    60,    61,    16,    51,    11,    13,    54,   271,   243,
    16,    58,    59,    60,    61,    11,    11,    32,     5,    15,
    16,    36,     9,    10,    34,    11,   254,   261,     5,    15,
    16,   265,   266,    11,   297,    22,   264,    49,     3,     4,
     5,     6,     7,     5,   278,     5,   280,    12,   282,   283,
    51,   175,   176,    54,   279,   289,   284,    44,    51,    60,
    61,    51,    51,    51,    51,    30,    32,    54,    31,    16,
    16,    58,    59,    60,    61,    55,    12,    50,   312,   307,
    45,    46,    11,    16,    48,    50,    13,     5,   322,   323,
    31,     9,    10,     5,    16,    13,    35,   331,   332,    17,
    18,    31,    20,    21,    22,    12,    24,     6,    26,    27,
    28,    12,   346,     5,   348,    33,    13,     9,    10,    34,
     5,    17,    40,    16,    42,    15,    15,     5,    15,    47,
    22,     9,    10,    51,    13,    16,    54,    13,    56,    57,
    58,    59,    60,    61,    22,    16,     5,    49,    48,     6,
     9,    10,    12,    16,    48,    47,    14,    34,    53,    51,
     5,    16,    54,    22,     9,    10,    58,    59,    60,    61,
    16,     5,    16,    51,     5,    16,    54,    22,     9,    10,
    58,    59,    60,    61,    49,    15,    15,    13,    16,    13,
     5,    16,    51,    31,     3,    54,     5,     6,    35,    58,
    59,    60,    61,    22,    16,     5,    51,    14,    16,    54,
    16,    16,    13,    58,    59,    60,    61,    12,    31,    35,
    51,    30,    16,    54,    16,    16,    14,    58,    59,    60,
    61,     3,     4,     5,     6,     7,    45,    46,    31,     5,
    16,    50,    16,    14,     0,     0,   220,     1,   161,   219,
    37,     3,   303,   193,   188,   346,   324,   139,    30,   133,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    45,    46,    -1,    -1,    -1,    50
};
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

#line 217 "/usr/share/bison.simple"

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
int yyparse (void *);
#else
int yyparse (void);
#endif
#endif

int
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
  int yyfree_stacks = 0;

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
	  if (yyfree_stacks)
	    {
	      free (yyss);
	      free (yyvs);
#ifdef YYLSP_NEEDED
	      free (yyls);
#endif
	    }
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

case 1:
#line 155 "yacc.yy"
{
	  ;
    break;}
case 3:
#line 161 "yacc.yy"
{
		printf("<INCLUDE>%s</INCLUDE>\n", yyvsp[-1]._str->latin1() );
	  ;
    break;}
case 4:
#line 165 "yacc.yy"
{
	  ;
    break;}
case 5:
#line 168 "yacc.yy"
{
          ;
    break;}
case 6:
#line 173 "yacc.yy"
{ yyval._int = 1; ;
    break;}
case 7:
#line 174 "yacc.yy"
{ yyval._int = 0; ;
    break;}
case 8:
#line 178 "yacc.yy"
{
	 	if (yyvsp[-2]._int)
			  printf("<CLASS>\n    <NAME>%s</NAME>\n%s%s</CLASS>\n", ( in_namespace + *yyvsp[-4]._str ).latin1(), yyvsp[-3]._str->latin1(), yyvsp[-1]._str->latin1() );
	  ;
    break;}
case 9:
#line 183 "yacc.yy"
{
	  ;
    break;}
case 10:
#line 186 "yacc.yy"
{
	  ;
    break;}
case 11:
#line 189 "yacc.yy"
{
	  ;
    break;}
case 12:
#line 192 "yacc.yy"
{
                      in_namespace += *yyvsp[-1]._str; in_namespace += "::";
                  ;
    break;}
case 13:
#line 196 "yacc.yy"
{
                      int pos = in_namespace.findRev( "::", -3 );
                      if( pos >= 0 )
                          in_namespace = in_namespace.left( pos + 2 );
                      else
                          in_namespace = "";
                  ;
    break;}
case 14:
#line 204 "yacc.yy"
{
          ;
    break;}
case 15:
#line 207 "yacc.yy"
{
          ;
    break;}
case 16:
#line 210 "yacc.yy"
{
	  ;
    break;}
case 17:
#line 213 "yacc.yy"
{
	  ;
    break;}
case 18:
#line 216 "yacc.yy"
{
	  ;
    break;}
case 19:
#line 219 "yacc.yy"
{
	  ;
    break;}
case 20:
#line 222 "yacc.yy"
{
	  ;
    break;}
case 21:
#line 225 "yacc.yy"
{
	  ;
    break;}
case 22:
#line 228 "yacc.yy"
{
	  ;
    break;}
case 23:
#line 231 "yacc.yy"
{
	  ;
    break;}
case 34:
#line 246 "yacc.yy"
{
	  dcop_area = 0;
	  dcop_signal_area = 0;
	;
    break;}
case 35:
#line 251 "yacc.yy"
{
	  dcop_area = 0;
	  dcop_signal_area = 0;
	;
    break;}
case 36:
#line 258 "yacc.yy"
{
	  dcop_area = 1;
	;
    break;}
case 37:
#line 264 "yacc.yy"
{
	  dcop_signal_area = 1;
	;
    break;}
case 38:
#line 269 "yacc.yy"
{
	  yyval._str = yyvsp[0]._str;
	;
    break;}
case 39:
#line 272 "yacc.yy"
{
	   QString* tmp = new QString( "%1::%2" );
           *tmp = tmp->arg(*(yyvsp[-2]._str)).arg(*(yyvsp[0]._str));
           yyval._str = tmp;
	;
    break;}
case 40:
#line 280 "yacc.yy"
{
		QString* tmp = new QString( "    <SUPER>%1</SUPER>\n" );
		*tmp = tmp->arg( *(yyvsp[0]._str) );
		yyval._str = tmp;
	  ;
    break;}
case 41:
#line 286 "yacc.yy"
{
		QString* tmp = new QString( "    <SUPER>%1</SUPER>\n" );
		*tmp = tmp->arg( *(yyvsp[-3]._str) + "&lt" + *(yyvsp[-1]._str) + "&gt;" );
		yyval._str = tmp;
	  ;
    break;}
case 42:
#line 294 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 43:
#line 298 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 44:
#line 305 "yacc.yy"
{
		yyval._str = yyvsp[-1]._str;
	  ;
    break;}
case 45:
#line 309 "yacc.yy"
{
		/* $$ = $1; */
		yyval._str = new QString( *(yyvsp[-2]._str) + *(yyvsp[0]._str) );
	  ;
    break;}
case 46:
#line 317 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 47:
#line 321 "yacc.yy"
{
		yyval._str = new QString( "" );
	  ;
    break;}
case 48:
#line 328 "yacc.yy"
{
          ;
    break;}
case 50:
#line 335 "yacc.yy"
{
		yyval._str = new QString( "" );
	  ;
    break;}
case 51:
#line 339 "yacc.yy"
{
		yyval._str = new QString( *(yyvsp[-1]._str) + *(yyvsp[0]._str) );
	  ;
    break;}
case 52:
#line 343 "yacc.yy"
{
		yyval._str = new QString( *(yyvsp[-1]._str) + *(yyvsp[0]._str) );
	  ;
    break;}
case 53:
#line 347 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 54:
#line 351 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 55:
#line 355 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 56:
#line 359 "yacc.yy"
{	
	        yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 57:
#line 363 "yacc.yy"
{
 	        yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 58:
#line 367 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 59:
#line 371 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 60:
#line 375 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 61:
#line 379 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 62:
#line 383 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 63:
#line 387 "yacc.yy"
{
                yyval._str = yyvsp[0]._str;
          ;
    break;}
case 70:
#line 402 "yacc.yy"
{;
    break;}
case 71:
#line 403 "yacc.yy"
{;
    break;}
case 72:
#line 406 "yacc.yy"
{;
    break;}
case 73:
#line 407 "yacc.yy"
{;
    break;}
case 74:
#line 408 "yacc.yy"
{;
    break;}
case 75:
#line 409 "yacc.yy"
{;
    break;}
case 76:
#line 410 "yacc.yy"
{;
    break;}
case 77:
#line 413 "yacc.yy"
{;
    break;}
case 78:
#line 414 "yacc.yy"
{;
    break;}
case 79:
#line 415 "yacc.yy"
{;
    break;}
case 80:
#line 419 "yacc.yy"
{
		if (dcop_area) {
 		  QString* tmp = new QString("<TYPEDEF name=\"%1\" template=\"%2\"><PARAM %3</TYPEDEF>\n");
		  *tmp = tmp->arg( *(yyvsp[-1]._str) ).arg( *(yyvsp[-5]._str) ).arg( *(yyvsp[-3]._str) );
		  yyval._str = tmp;
		} else {
		  yyval._str = new QString("");
		}
	  ;
    break;}
case 81:
#line 429 "yacc.yy"
{
		if (dcop_area)
		  yyerror("scoped template typedefs are not supported in dcop areas!");
	  ;
    break;}
case 82:
#line 434 "yacc.yy"
{
	  ;
    break;}
case 83:
#line 439 "yacc.yy"
{
		yyval._int = 0;
	  ;
    break;}
case 84:
#line 443 "yacc.yy"
{
		yyval._int = 1;
	  ;
    break;}
case 85:
#line 449 "yacc.yy"
{ yyval._str = new QString("signed int"); ;
    break;}
case 86:
#line 450 "yacc.yy"
{ yyval._str = new QString("signed int"); ;
    break;}
case 87:
#line 451 "yacc.yy"
{ yyval._str = new QString("unsigned int"); ;
    break;}
case 88:
#line 452 "yacc.yy"
{ yyval._str = new QString("unsigned int"); ;
    break;}
case 89:
#line 453 "yacc.yy"
{ yyval._str = new QString("signed short int"); ;
    break;}
case 90:
#line 454 "yacc.yy"
{ yyval._str = new QString("signed short int"); ;
    break;}
case 91:
#line 455 "yacc.yy"
{ yyval._str = new QString("signed long int"); ;
    break;}
case 92:
#line 456 "yacc.yy"
{ yyval._str = new QString("signed long int"); ;
    break;}
case 93:
#line 457 "yacc.yy"
{ yyval._str = new QString("unsigned short int"); ;
    break;}
case 94:
#line 458 "yacc.yy"
{ yyval._str = new QString("unsigned short int"); ;
    break;}
case 95:
#line 459 "yacc.yy"
{ yyval._str = new QString("unsigned long int"); ;
    break;}
case 96:
#line 460 "yacc.yy"
{ yyval._str = new QString("unsigned long int"); ;
    break;}
case 97:
#line 461 "yacc.yy"
{ yyval._str = new QString("int"); ;
    break;}
case 98:
#line 462 "yacc.yy"
{ yyval._str = new QString("long int"); ;
    break;}
case 99:
#line 463 "yacc.yy"
{ yyval._str = new QString("long int"); ;
    break;}
case 100:
#line 464 "yacc.yy"
{ yyval._str = new QString("short int"); ;
    break;}
case 101:
#line 465 "yacc.yy"
{ yyval._str = new QString("short int"); ;
    break;}
case 102:
#line 466 "yacc.yy"
{ yyval._str = new QString("char"); ;
    break;}
case 103:
#line 467 "yacc.yy"
{ yyval._str = new QString("signed char"); ;
    break;}
case 104:
#line 468 "yacc.yy"
{ yyval._str = new QString("unsigned char"); ;
    break;}
case 107:
#line 476 "yacc.yy"
{
		yyval._str = new QString( "" );
	  ;
    break;}
case 109:
#line 481 "yacc.yy"
{
		yyval._str = new QString( *(yyvsp[-2]._str) + *(yyvsp[0]._str) );
	  ;
    break;}
case 111:
#line 489 "yacc.yy"
{ yyval._str = yyvsp[0]._str; ;
    break;}
case 112:
#line 490 "yacc.yy"
{ yyval._str = yyvsp[0]._str; ;
    break;}
case 113:
#line 491 "yacc.yy"
{ yyval._str = yyvsp[0]._str; ;
    break;}
case 114:
#line 492 "yacc.yy"
{
		QString *tmp = new QString("%1&lt;%2&gt;");
		*tmp = tmp->arg(*(yyvsp[-3]._str));
		*tmp = tmp->arg(*(yyvsp[-1]._str));
		yyval._str = tmp;
	 ;
    break;}
case 115:
#line 498 "yacc.yy"
{
		QString *tmp = new QString("%1&lt;%2&gt;::%3");
		*tmp = tmp->arg(*(yyvsp[-5]._str));
		*tmp = tmp->arg(*(yyvsp[-3]._str));
		*tmp = tmp->arg(*(yyvsp[0]._str));
		yyval._str = tmp;
	 ;
    break;}
case 116:
#line 509 "yacc.yy"
{
	    yyval._str = new QString(*(yyvsp[-2]._str) + "," + *(yyvsp[0]._str));
	  ;
    break;}
case 117:
#line 513 "yacc.yy"
{
 	    yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 118:
#line 520 "yacc.yy"
{
	    if (dcop_area)
	      yyerror("in dcop areas are no pointers allowed");
	  ;
    break;}
case 119:
#line 525 "yacc.yy"
{
 	    yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 120:
#line 533 "yacc.yy"
{
	    if (dcop_area)
	      yyerror("in dcop areas are no pointers allowed");
	  ;
    break;}
case 121:
#line 537 "yacc.yy"
{
	     if (dcop_area) {
	  	QString* tmp = new QString("<TYPE  qleft=\"const\" qright=\"" AMP_ENTITY "\">%1</TYPE>");
		*tmp = tmp->arg( *(yyvsp[-1]._str) );
		yyval._str = tmp;
	     }
	  ;
    break;}
case 122:
#line 544 "yacc.yy"
{
		QString* tmp = new QString("<TYPE>%1</TYPE>");
		*tmp = tmp->arg( *(yyvsp[0]._str) );
		yyval._str = tmp;
	;
    break;}
case 123:
#line 549 "yacc.yy"
{
	     if (dcop_area)
		yyerror("in dcop areas are only const references allowed!");
	  ;
    break;}
case 124:
#line 554 "yacc.yy"
{
		QString* tmp = new QString("<TYPE>%1</TYPE>");
		*tmp = tmp->arg( *(yyvsp[0]._str) );
		yyval._str = tmp;
	;
    break;}
case 125:
#line 560 "yacc.yy"
{
	    if (dcop_area)
	      yyerror("in dcop areas are no pointers allowed");
	  ;
    break;}
case 126:
#line 568 "yacc.yy"
{
	    yyval._str = new QString(*(yyvsp[-2]._str) + "," + *(yyvsp[0]._str));
	  ;
    break;}
case 127:
#line 572 "yacc.yy"
{
 	    yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 128:
#line 578 "yacc.yy"
{
		if (dcop_area) {
		   QString* tmp = new QString("\n        <ARG>%1<NAME>%2</NAME></ARG>");
  		   *tmp = tmp->arg( *(yyvsp[-2]._str) );
  		   *tmp = tmp->arg( *(yyvsp[-1]._str) );
		   yyval._str = tmp;		
		} else yyval._str = new QString();
	  ;
    break;}
case 129:
#line 587 "yacc.yy"
{
		if (dcop_area) {
		   QString* tmp = new QString("\n        <ARG>%1</ARG>");
  		   *tmp = tmp->arg( *(yyvsp[-1]._str) );
		   yyval._str = tmp;		
		} else yyval._str = new QString();
	  ;
    break;}
case 130:
#line 595 "yacc.yy"
{
		if (dcop_area)
			yyerror("variable arguments not supported in dcop area.");
		yyval._str = new QString("");
	  ;
    break;}
case 131:
#line 603 "yacc.yy"
{
	  ;
    break;}
case 132:
#line 606 "yacc.yy"
{
	  ;
    break;}
case 133:
#line 609 "yacc.yy"
{
	  ;
    break;}
case 134:
#line 615 "yacc.yy"
{
          ;
    break;}
case 135:
#line 618 "yacc.yy"
{
          ;
    break;}
case 136:
#line 621 "yacc.yy"
{
          ;
    break;}
case 137:
#line 624 "yacc.yy"
{
          ;
    break;}
case 138:
#line 627 "yacc.yy"
{
          ;
    break;}
case 139:
#line 632 "yacc.yy"
{ yyval._int = 0; ;
    break;}
case 140:
#line 633 "yacc.yy"
{ yyval._int = 1; ;
    break;}
case 146:
#line 640 "yacc.yy"
{
	     if (dcop_area || dcop_signal_area) {
		QString* tmp = 0;
                tmp = new QString(
                        "    <%4>\n"
                        "        %2\n"
                        "        <NAME>%1</NAME>"
                        "%3\n"
                        "     </%5>\n");
		*tmp = tmp->arg( *(yyvsp[-4]._str) );
		*tmp = tmp->arg( *(yyvsp[-5]._str) );
                if (yyvsp[0]._int) {
                   *tmp = tmp->arg( *(yyvsp[-2]._str) + " qual=\"const\">" );
                } else {
                   *tmp = tmp->arg( *(yyvsp[-2]._str) );
                }
                
                QString tagname = "";
                QString attr = "";
                if (dcop_signal_area) {
                   tagname = "SIGNAL";
                } else {
                   tagname = "FUNC";
                }
                *tmp = tmp->arg( QString("%1%2").arg(tagname).arg(attr) );
                *tmp = tmp->arg( QString("%1").arg(tagname) );
		yyval._str = tmp;
   	     } else
	        yyval._str = new QString("");
	  ;
    break;}
case 147:
#line 671 "yacc.yy"
{
	     if (dcop_area)
		yyerror("operators aren't allowed in dcop areas!");
	     yyval._str = new QString("");
	  ;
    break;}
case 148:
#line 678 "yacc.yy"
{;
    break;}
case 149:
#line 679 "yacc.yy"
{;
    break;}
case 150:
#line 680 "yacc.yy"
{;
    break;}
case 151:
#line 683 "yacc.yy"
{;
    break;}
case 152:
#line 684 "yacc.yy"
{;
    break;}
case 153:
#line 687 "yacc.yy"
{;
    break;}
case 154:
#line 690 "yacc.yy"
{;
    break;}
case 155:
#line 691 "yacc.yy"
{;
    break;}
case 156:
#line 695 "yacc.yy"
{
	        yyval._str = yyvsp[-1]._str;
	  ;
    break;}
case 157:
#line 699 "yacc.yy"
{
		yyval._str = yyvsp[-3]._str;
	  ;
    break;}
case 158:
#line 703 "yacc.yy"
{
		yyval._str = yyvsp[-1]._str;
	  ;
    break;}
case 159:
#line 707 "yacc.yy"
{
	      /* The constructor */
	      assert(!dcop_area);
              yyval._str = new QString("");
	  ;
    break;}
case 160:
#line 713 "yacc.yy"
{
	      /* The constructor */
	      assert(!dcop_area);
              yyval._str = new QString("");
	  ;
    break;}
case 161:
#line 719 "yacc.yy"
{
	      /* The destructor */
  	      assert(!dcop_area);
              yyval._str = new QString("");
	  ;
    break;}
case 162:
#line 725 "yacc.yy"
{
              if (dcop_area) {
                 if (dcop_signal_area)
                     yyval._str = yyvsp[-1]._str;
                 else
                     yyerror("static is not allowed in dcop area!");
              } else {
                 yyval._str = new QString();
              }  
	  ;
    break;}
case 163:
#line 737 "yacc.yy"
{
		function_mode = 1;
	;
    break;}
case 167:
#line 747 "yacc.yy"
{;
    break;}
case 168:
#line 748 "yacc.yy"
{;
    break;}
case 172:
#line 757 "yacc.yy"
{;
    break;}
case 173:
#line 758 "yacc.yy"
{;
    break;}
case 174:
#line 759 "yacc.yy"
{;
    break;}
case 175:
#line 761 "yacc.yy"
{;
    break;}
case 176:
#line 764 "yacc.yy"
{;
    break;}
case 177:
#line 765 "yacc.yy"
{;
    break;}
case 178:
#line 766 "yacc.yy"
{;
    break;}
case 179:
#line 767 "yacc.yy"
{;
    break;}
}
   /* the action file gets copied in in place of this dollarsign */
#line 543 "/usr/share/bison.simple"

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
  if (yyfree_stacks)
    {
      free (yyss);
      free (yyvs);
#ifdef YYLSP_NEEDED
      free (yyls);
#endif
    }
  return 0;

 yyabortlab:
  /* YYABORT comes here.  */
  if (yyfree_stacks)
    {
      free (yyss);
      free (yyvs);
#ifdef YYLSP_NEEDED
      free (yyls);
#endif
    }
  return 1;
}
#line 769 "yacc.yy"


void dcopidlParse( const char *_code )
{
    dcopidlInitFlex( _code );
    yyparse();
}
