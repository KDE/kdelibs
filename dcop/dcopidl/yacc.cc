
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
#define	T_MUTABLE	302
#define	T_EQUAL	303
#define	T_SCOPE	304
#define	T_NULL	305
#define	T_INT	306
#define	T_ARRAY_OPEN	307
#define	T_ARRAY_CLOSE	308
#define	T_CHAR	309
#define	T_DCOP	310
#define	T_DCOP_AREA	311
#define	T_DCOP_SIGNAL_AREA	312
#define	T_SIGNED	313
#define	T_UNSIGNED	314
#define	T_LONG	315
#define	T_SHORT	316
#define	T_FUNOPERATOR	317
#define	T_MISCOPERATOR	318
#define	T_SHIFT	319

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



#define	YYFINAL		365
#define	YYFLAG		-32768
#define	YYNTBASE	66

#define YYTRANSLATE(x) ((unsigned)(x) <= 319 ? yytranslate[x] : 119)

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
    57,    58,    59,    60,    61,    62,    63,    64,    65
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     4,     5,     8,    13,    14,    16,    17,    24,    28,
    32,    38,    39,    47,    52,    58,    61,    66,    74,    83,
    86,    88,    90,    92,    95,    96,    98,   100,   102,   104,
   106,   108,   110,   111,   115,   118,   121,   124,   126,   130,
   132,   137,   141,   143,   146,   150,   153,   155,   156,   158,
   160,   163,   166,   169,   172,   175,   178,   181,   187,   192,
   197,   202,   209,   214,   221,   228,   236,   243,   250,   256,
   260,   262,   266,   268,   270,   272,   275,   277,   279,   281,
   285,   289,   297,   307,   308,   310,   312,   315,   317,   320,
   323,   327,   330,   334,   337,   341,   344,   348,   350,   352,
   355,   357,   360,   362,   365,   368,   371,   373,   374,   376,
   380,   382,   384,   387,   390,   395,   402,   406,   408,   411,
   413,   417,   421,   424,   427,   429,   432,   436,   438,   442,
   445,   447,   448,   451,   457,   459,   461,   463,   465,   470,
   471,   473,   475,   477,   479,   481,   483,   490,   498,   500,
   502,   506,   508,   512,   517,   519,   523,   526,   532,   536,
   542,   550,   557,   561,   563,   565,   569,   574,   577,   578,
   580,   583,   584,   586,   590,   593,   596,   600,   606,   612,
   618
};

static const short yyrhs[] = {    67,
    69,    66,     0,     0,     8,    67,     0,    38,    11,    66,
    13,     0,     0,    56,     0,     0,     9,    78,    82,    68,
    84,    16,     0,     9,    78,    16,     0,    10,    78,    16,
     0,    10,    78,    82,    84,    16,     0,     0,    41,     5,
    11,    70,    66,    13,    83,     0,    42,    41,     5,    16,
     0,    42,     5,    50,     5,    16,     0,    37,    16,     0,
    28,    98,    78,    16,     0,    28,    10,    11,    71,    13,
    78,    16,     0,    28,    10,    78,    11,    71,    13,    78,
    16,     0,    23,   110,     0,   110,     0,   118,     0,    85,
     0,   118,    71,     0,     0,    45,     0,    46,     0,    20,
     0,    18,     0,    17,     0,    26,     0,    27,     0,     0,
    73,    74,    15,     0,    74,    15,     0,    57,    15,     0,
    58,    15,     0,     5,     0,     5,    50,    78,     0,    78,
     0,    78,    34,    99,    35,     0,   103,    17,    79,     0,
    79,     0,    80,    11,     0,    80,    31,    81,     0,    15,
    81,     0,    11,     0,     0,    16,     0,    13,     0,    90,
    84,     0,   110,    84,     0,    77,    84,     0,    85,    84,
     0,    76,    84,     0,    75,    84,     0,   118,    84,     0,
    24,     9,    78,    16,    84,     0,    24,    78,    16,    84,
     0,    24,   105,    16,    84,     0,     9,    78,    16,    84,
     0,     9,    78,    82,    84,    16,    84,     0,    10,    78,
    16,    84,     0,    10,    78,    82,    84,    16,    84,     0,
    42,     5,    50,     5,    16,    84,     0,    40,     5,    11,
    86,    13,     5,    16,     0,    40,     5,    11,    86,    13,
    16,     0,    40,    11,    86,    13,     5,    16,     0,    40,
    11,    86,    13,    16,     0,    87,    31,    86,     0,    87,
     0,     5,    49,    89,     0,     5,     0,     3,     0,     6,
     0,    30,     6,     0,    51,     0,    78,     0,    88,     0,
    88,    29,    88,     0,    88,    65,    88,     0,    28,    78,
    34,    99,    35,    78,    16,     0,    28,    78,    34,    99,
    35,    50,     5,    78,    16,     0,     0,    22,     0,    59,
     0,    59,    52,     0,    60,     0,    60,    52,     0,    59,
    62,     0,    59,    62,    52,     0,    59,    61,     0,    59,
    61,    52,     0,    60,    62,     0,    60,    62,    52,     0,
    60,    61,     0,    60,    61,    52,     0,    52,     0,    61,
     0,    61,    52,     0,    62,     0,    62,    52,     0,    55,
     0,    59,    55,     0,    60,    55,     0,    32,    93,     0,
    32,     0,     0,   100,     0,    94,    31,   100,     0,    92,
     0,    78,     0,    10,    78,     0,     9,    78,     0,    78,
    34,    96,    35,     0,    78,    34,    96,    35,    50,    78,
     0,    97,    31,    96,     0,    97,     0,    95,    93,     0,
    95,     0,    22,    95,    93,     0,    22,    95,    36,     0,
    22,    95,     0,    95,    36,     0,    95,     0,    95,    93,
     0,    98,    31,    99,     0,    98,     0,    98,    78,   101,
     0,    98,   101,     0,    44,     0,     0,    49,   102,     0,
    49,    12,    98,    14,   102,     0,     7,     0,    89,     0,
     4,     0,    72,     0,    78,    12,    94,    14,     0,     0,
    21,     0,    64,     0,    65,     0,    35,     0,    34,     0,
    49,     0,    98,    78,    12,    94,    14,    91,     0,    98,
    63,   104,    12,    94,    14,    91,     0,    88,     0,    72,
     0,     5,    12,    14,     0,   106,     0,   106,    31,   107,
     0,     5,    12,   107,    14,     0,   108,     0,   108,    31,
   109,     0,   105,   112,     0,    21,   105,    49,    51,   112,
     0,    21,   105,   112,     0,    78,    12,    94,    14,   112,
     0,    78,    12,    94,    14,    15,   109,   112,     0,   103,
    33,    78,    12,    14,   112,     0,    47,   105,   112,     0,
    11,     0,    16,     0,   111,   113,    13,     0,   111,   113,
    13,    16,     0,   114,   113,     0,     0,    16,     0,    31,
   117,     0,     0,     5,     0,     5,    49,   102,     0,    93,
     5,     0,   116,   115,     0,    98,   117,    16,     0,    98,
    78,    15,     6,    16,     0,    47,    98,     5,   101,    16,
     0,    48,    98,     5,   101,    16,     0,    98,     5,    53,
    89,    54,    16,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
   155,   158,   161,   165,   168,   174,   175,   178,   183,   186,
   189,   192,   196,   204,   207,   210,   213,   216,   219,   222,
   225,   228,   231,   236,   237,   239,   239,   241,   241,   241,
   243,   243,   243,   246,   251,   258,   264,   270,   273,   280,
   286,   294,   298,   305,   309,   317,   321,   328,   331,   335,
   339,   343,   347,   351,   355,   359,   363,   367,   371,   375,
   379,   383,   387,   391,   395,   401,   402,   403,   404,   407,
   408,   411,   412,   415,   416,   417,   418,   419,   422,   423,
   424,   427,   437,   444,   448,   455,   456,   457,   458,   459,
   460,   461,   462,   463,   464,   465,   466,   467,   468,   469,
   470,   471,   472,   473,   474,   477,   478,   481,   485,   486,
   494,   495,   496,   497,   498,   504,   514,   518,   525,   530,
   538,   543,   550,   555,   560,   565,   573,   577,   583,   592,
   600,   608,   611,   614,   620,   623,   626,   629,   632,   638,
   639,   642,   642,   642,   642,   642,   645,   676,   684,   685,
   686,   689,   690,   693,   696,   697,   700,   704,   708,   712,
   718,   724,   730,   742,   748,   749,   750,   753,   754,   757,
   760,   761,   763,   764,   765,   767,   770,   771,   772,   773,
   774
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
"T_USING","T_UNKNOWN","T_TRIPLE_DOT","T_TRUE","T_FALSE","T_STATIC","T_MUTABLE",
"T_EQUAL","T_SCOPE","T_NULL","T_INT","T_ARRAY_OPEN","T_ARRAY_CLOSE","T_CHAR",
"T_DCOP","T_DCOP_AREA","T_DCOP_SIGNAL_AREA","T_SIGNED","T_UNSIGNED","T_LONG",
"T_SHORT","T_FUNOPERATOR","T_MISCOPERATOR","T_SHIFT","main","includes","dcoptag",
"declaration","@1","member_list","bool_value","nodcop_area","sigslot","nodcop_area_begin",
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
    66,    66,    67,    67,    67,    68,    68,    69,    69,    69,
    69,    70,    69,    69,    69,    69,    69,    69,    69,    69,
    69,    69,    69,    71,    71,    72,    72,    73,    73,    73,
    74,    74,    74,    75,    75,    76,    77,    78,    78,    79,
    79,    80,    80,    81,    81,    82,    82,    83,    83,    84,
    84,    84,    84,    84,    84,    84,    84,    84,    84,    84,
    84,    84,    84,    84,    84,    85,    85,    85,    85,    86,
    86,    87,    87,    88,    88,    88,    88,    88,    89,    89,
    89,    90,    90,    91,    91,    92,    92,    92,    92,    92,
    92,    92,    92,    92,    92,    92,    92,    92,    92,    92,
    92,    92,    92,    92,    92,    93,    93,    94,    94,    94,
    95,    95,    95,    95,    95,    95,    96,    96,    97,    97,
    98,    98,    98,    98,    98,    98,    99,    99,   100,   100,
   100,   101,   101,   101,   102,   102,   102,   102,   102,   103,
   103,   104,   104,   104,   104,   104,   105,   105,   106,   106,
   106,   107,   107,   108,   109,   109,   110,   110,   110,   110,
   110,   110,   110,   111,   112,   112,   112,   113,   113,   114,
   115,   115,   116,   116,   116,   117,   118,   118,   118,   118,
   118
};

static const short yyr2[] = {     0,
     3,     0,     2,     4,     0,     1,     0,     6,     3,     3,
     5,     0,     7,     4,     5,     2,     4,     7,     8,     2,
     1,     1,     1,     2,     0,     1,     1,     1,     1,     1,
     1,     1,     0,     3,     2,     2,     2,     1,     3,     1,
     4,     3,     1,     2,     3,     2,     1,     0,     1,     1,
     2,     2,     2,     2,     2,     2,     2,     5,     4,     4,
     4,     6,     4,     6,     6,     7,     6,     6,     5,     3,
     1,     3,     1,     1,     1,     2,     1,     1,     1,     3,
     3,     7,     9,     0,     1,     1,     2,     1,     2,     2,
     3,     2,     3,     2,     3,     2,     3,     1,     1,     2,
     1,     2,     1,     2,     2,     2,     1,     0,     1,     3,
     1,     1,     2,     2,     4,     6,     3,     1,     2,     1,
     3,     3,     2,     2,     1,     2,     3,     1,     3,     2,
     1,     0,     2,     5,     1,     1,     1,     1,     4,     0,
     1,     1,     1,     1,     1,     1,     6,     7,     1,     1,
     3,     1,     3,     4,     1,     3,     2,     5,     3,     5,
     7,     6,     3,     1,     1,     3,     4,     2,     0,     1,
     2,     0,     1,     3,     2,     2,     3,     5,     5,     5,
     6
};

static const short yydefact[] = {     5,
     5,     0,   140,     3,     5,    38,     0,     0,   141,     0,
   140,     0,     0,     0,     0,     0,     0,     0,    98,   103,
    86,    88,    99,   101,     5,   112,    23,   111,   125,     0,
     0,     0,    21,    22,     0,     0,   114,   113,     0,     0,
   112,     0,     0,   123,     0,    20,     0,     0,    16,     0,
     0,     0,     0,     0,     0,     0,     0,    87,   104,    92,
    90,    89,   105,    96,    94,   100,   102,     1,   108,     0,
   107,   124,   126,    38,     0,     0,     0,   172,     0,     0,
   164,   165,   169,   157,     4,    39,    47,   140,     9,     7,
    10,    33,   114,   113,     0,     0,   159,   122,   121,    25,
   113,     0,     0,    73,     0,    71,    12,     0,     0,    38,
   163,   132,    93,    91,    97,    95,   131,     0,   132,   109,
   120,     0,   118,   106,     0,     0,   145,   144,   146,   142,
   143,     0,   108,     0,   175,     0,   176,   177,     0,   170,
     0,   169,   141,    40,    43,     0,    46,     0,     6,    33,
     0,     0,    50,    30,    29,    28,     0,    31,    32,     0,
     0,     0,     0,    33,     0,    33,    33,    33,     0,    33,
    33,    33,    33,     0,     0,     0,     0,    25,    25,    17,
     0,     0,     0,     0,     5,     0,    14,     0,     0,     0,
     0,     0,   132,   130,   119,   115,     0,    74,   137,    75,
   135,     0,    26,    27,    77,   138,    78,    79,   136,   174,
    78,     0,   108,     0,     0,   173,   171,     0,   166,   168,
     0,    44,   140,     0,     0,   114,   113,     0,   112,     0,
     0,     0,    36,    37,     0,    35,    56,    55,    53,    11,
    54,    51,    52,    57,   158,     0,     0,     0,    24,     0,
     0,    72,     0,    69,    70,     0,    15,     0,   133,   179,
   180,     0,   160,   110,   129,     0,   117,    76,   108,     0,
     0,     0,     0,    84,   178,     0,   167,   128,     0,    45,
    42,     8,    33,    33,    33,    33,   114,    33,    33,     0,
     0,    34,   132,     0,     0,     0,    67,    68,    48,     0,
     0,   155,     0,   116,     0,    80,    81,   181,    84,    85,
   147,   162,     0,    41,    61,     0,    63,     0,    33,    59,
    60,     0,     0,    18,     0,    66,    49,    13,     0,     0,
     0,   161,   139,   148,   127,    33,    33,    58,     0,    33,
    19,   134,    38,   150,   149,   152,     0,   156,    62,    64,
     0,     0,    65,     0,     0,   154,     0,    82,   151,   153,
     0,    83,     0,     0,     0
};

static const short yydefgoto[] = {    35,
     3,   150,    25,   185,   176,   206,   164,   165,   166,   167,
   168,    26,   145,   146,   147,    90,   328,   169,   170,   105,
   106,   208,   209,   171,   311,    28,    77,   118,    29,   122,
   123,    30,   279,   120,   189,   210,    31,   132,    32,   346,
   347,   302,   303,   172,    83,    84,   141,   142,   137,    78,
    79,   173
};

static const short yypact[] = {    39,
   190,    42,   360,-32768,   103,    10,   102,   102,   401,   104,
   220,   422,   106,    45,   131,    37,   401,   401,-32768,-32768,
   252,   257,   108,   122,    91,   112,-32768,-32768,   222,    29,
   156,   227,-32768,-32768,   182,   102,   258,   274,   102,   102,
   174,    13,    48,   261,   401,-32768,   177,   102,-32768,   200,
   226,   223,   186,   240,    17,   227,   241,-32768,-32768,   205,
   208,-32768,-32768,   213,   224,-32768,-32768,-32768,   381,   104,
   245,-32768,-32768,    90,    93,   123,   273,   253,   280,   102,
-32768,-32768,   286,-32768,-32768,-32768,-32768,    49,-32768,   249,
-32768,   316,-32768,-32768,   296,   264,-32768,-32768,-32768,   239,
   306,   304,   226,   278,   315,   299,-32768,   326,   319,    82,
-32768,   290,-32768,-32768,-32768,-32768,-32768,   196,    19,-32768,
   245,   317,   320,-32768,   173,    66,-32768,-32768,-32768,-32768,
-32768,   341,   381,   348,-32768,    23,-32768,-32768,   354,-32768,
   359,   286,-32768,   333,-32768,   138,-32768,   362,-32768,   316,
   102,   102,-32768,-32768,-32768,-32768,   442,-32768,-32768,   102,
   375,   369,   370,   160,   372,   316,   316,   316,   373,   316,
   316,   316,   316,   227,   401,   379,   115,   239,   239,-32768,
   380,    66,   157,   226,   103,   378,-32768,   463,   382,   383,
   330,   381,   290,-32768,-32768,   345,   104,-32768,-32768,-32768,
-32768,   390,-32768,-32768,-32768,-32768,   392,    65,-32768,-32768,
-32768,   351,   381,   201,   393,   364,-32768,   400,   402,-32768,
   401,-32768,    49,   102,   408,   344,   346,   102,    99,   410,
   394,   366,-32768,-32768,   414,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,   412,   102,   415,-32768,   421,
   191,-32768,   419,-32768,-32768,   424,-32768,   401,-32768,-32768,
-32768,   433,-32768,-32768,-32768,   102,-32768,-32768,   381,    66,
    66,   423,   206,   426,-32768,   227,-32768,   418,   411,-32768,
-32768,-32768,   316,   316,   316,   316,   429,   316,   316,   401,
   445,-32768,   290,   438,   102,   439,-32768,-32768,   441,   444,
   447,   434,   227,-32768,   219,-32768,-32768,-32768,   426,-32768,
-32768,-32768,   401,-32768,-32768,   455,-32768,   456,   316,-32768,
-32768,   443,   457,-32768,   460,-32768,-32768,-32768,   173,   265,
   433,-32768,-32768,-32768,-32768,   316,   316,-32768,    25,   316,
-32768,-32768,    24,-32768,-32768,   448,   466,-32768,-32768,-32768,
   480,   470,-32768,   473,   265,-32768,   102,-32768,-32768,-32768,
   472,-32768,   489,   490,-32768
};

static const short yypgoto[] = {    12,
   491,-32768,-32768,-32768,    44,  -207,-32768,   327,-32768,-32768,
-32768,    -7,   271,-32768,   275,   -34,-32768,   -84,   493,   -94,
-32768,  -245,   -82,-32768,   197,-32768,   -13,  -118,    -3,   302,
-32768,    34,  -188,   308,   -92,  -174,   -71,-32768,     4,-32768,
   150,-32768,   176,    54,-32768,   -37,   368,-32768,-32768,-32768,
   376,     5
};


#define	YYLAST		514


static const short yytable[] = {    37,
    38,    41,    41,    92,    41,    97,    44,    34,   181,    41,
    41,   363,    43,   259,   214,    73,   148,     6,   111,   190,
    56,   110,    76,     6,   306,   307,   194,   216,    86,     6,
    99,    93,    94,    74,    95,   354,    68,    41,    -2,   101,
   102,    53,    42,   212,    42,    48,     1,    95,    56,    50,
    55,    57,     5,     6,    71,    51,    33,   124,    81,    36,
    71,    41,    41,    82,    46,   225,   121,   188,   198,   143,
     6,   200,   139,    36,   351,    75,     2,    54,    42,    75,
   144,   237,   238,   239,   345,   241,   242,   243,   244,   255,
    -2,    75,    41,   270,   273,   202,    96,  -132,     1,   252,
   265,   322,   119,    -2,   178,  -173,     6,   195,     6,   345,
     1,   193,    39,    40,   288,    -2,   205,   207,   211,    74,
  -173,    49,   344,    69,   335,    41,   127,   128,     2,   271,
   188,    36,    70,   177,   133,    52,   245,   134,   125,    36,
     2,   129,   126,   226,   227,    70,    71,   344,   222,   229,
   305,   148,   231,   263,   342,    19,   130,   131,    20,    66,
   230,   253,    21,    22,    23,    24,   119,    41,   223,   248,
    41,    41,   254,    67,   211,   198,   199,     6,   200,   201,
   207,     6,   178,   178,    41,   158,   159,   100,    80,    41,
    42,   284,   286,   121,    85,   296,   256,     1,   315,   316,
   317,   318,   202,   320,   321,    41,   297,    70,   246,   191,
   103,   177,   177,    41,   274,   144,   144,   203,   204,   309,
   287,   249,   250,   205,     6,   119,   192,     2,    39,    40,
   104,   192,   333,   107,   338,   108,   192,    81,   312,   294,
     9,    10,    82,     6,   109,   112,   119,    39,    40,   192,
    41,   349,   350,    71,   278,   353,   113,    72,   304,   114,
    10,    41,   211,   211,   115,   332,    45,   198,    87,   343,
   200,    19,    88,    89,    20,   116,    71,   135,    21,    22,
    23,    24,    41,   136,    87,   175,    18,   325,    88,    91,
    19,   300,    71,    20,   202,   138,    98,    21,    22,    23,
    24,   140,   119,    58,   149,    41,    59,   133,    62,   203,
   204,    63,    60,    61,   174,   205,   179,    64,    65,   180,
     6,   207,   211,   278,   151,   152,   182,   183,   153,   184,
   186,   352,   154,   155,   187,   156,     9,    10,   188,   157,
    81,   158,   159,   160,   262,    82,   278,   211,  -140,   361,
   197,   196,   213,   215,    87,    14,    87,   161,    88,   283,
    88,   285,    17,    18,     6,   218,   221,    19,     7,     8,
    20,   219,   162,   163,    21,    22,    23,    24,   224,   232,
     9,    10,    11,   233,   234,     6,   236,    12,   240,    39,
    40,   247,   251,   257,   266,   268,    13,   260,   261,    14,
    15,    16,    10,   269,   272,     6,    17,    18,   275,    39,
    40,    19,   125,   276,    20,   291,   293,   277,    21,    22,
    23,    24,    10,   282,   117,   289,     6,   290,   292,   134,
    39,    47,    19,   295,   298,    20,   299,   301,   308,    21,
    22,    23,    24,    10,   319,   314,     6,   310,   313,   323,
   228,    40,    19,   324,   326,    20,   327,   329,   330,    21,
    22,    23,    24,    10,   331,   198,   199,     6,   200,   201,
   336,   337,   340,    19,   258,   341,    20,   339,   355,   356,
    21,    22,    23,    24,   357,   358,   359,   362,   364,   365,
   235,     4,   202,    19,   281,    27,    20,   280,   267,   264,
    21,    22,    23,    24,   360,   334,   348,   203,   204,   220,
     0,   217,     0,   205
};

static const short yycheck[] = {     7,
     8,     9,    10,    38,    12,    43,    10,     3,   103,    17,
    18,     0,     9,   188,   133,    29,    88,     5,    56,   112,
    17,     5,    30,     5,   270,   271,   119,     5,    36,     5,
    44,    39,    40,     5,    42,    12,    25,    45,     0,    47,
    48,     5,     9,   126,    11,    12,     8,    55,    45,     5,
    17,    18,    11,     5,    32,    11,     3,    71,    11,    50,
    32,    69,    70,    16,    11,   150,    70,    49,     3,    21,
     5,     6,    80,    50,    50,    63,    38,    41,    45,    63,
    88,   166,   167,   168,   330,   170,   171,   172,   173,   184,
     0,    63,   100,    29,   213,    30,    49,    16,     8,   182,
   193,   290,    69,    13,   100,    16,     5,   121,     5,   355,
     8,   119,     9,    10,    16,    13,    51,   125,   126,     5,
    31,    16,   330,    12,   313,   133,    34,    35,    38,    65,
    49,    50,    34,   100,    12,     5,   174,    15,    49,    50,
    38,    49,    53,   151,   152,    34,    32,   355,    11,   157,
   269,   223,   160,   191,   329,    52,    64,    65,    55,    52,
   157,     5,    59,    60,    61,    62,   133,   175,    31,   177,
   178,   179,    16,    52,   182,     3,     4,     5,     6,     7,
   188,     5,   178,   179,   192,    26,    27,    11,    33,   197,
   157,   226,   227,   197,    13,     5,   185,     8,   283,   284,
   285,   286,    30,   288,   289,   213,    16,    34,   175,    14,
    11,   178,   179,   221,    14,   223,   224,    45,    46,    14,
   228,   178,   179,    51,     5,   192,    31,    38,     9,    10,
     5,    31,    14,    11,   319,    50,    31,    11,   276,   247,
    21,    22,    16,     5,     5,     5,   213,     9,    10,    31,
   258,   336,   337,    32,   221,   340,    52,    36,   266,    52,
    22,   269,   270,   271,    52,   303,    47,     3,    11,     5,
     6,    52,    15,    16,    55,    52,    32,     5,    59,    60,
    61,    62,   290,    31,    11,    47,    48,   295,    15,    16,
    52,   258,    32,    55,    30,    16,    36,    59,    60,    61,
    62,    16,   269,    52,    56,   313,    55,    12,    52,    45,
    46,    55,    61,    62,    51,    51,    11,    61,    62,    16,
     5,   329,   330,   290,     9,    10,    49,    13,    13,    31,
     5,   339,    17,    18,    16,    20,    21,    22,    49,    24,
    11,    26,    27,    28,    15,    16,   313,   355,    33,   357,
    31,    35,    12,     6,    11,    40,    11,    42,    15,    16,
    15,    16,    47,    48,     5,    12,    34,    52,     9,    10,
    55,    13,    57,    58,    59,    60,    61,    62,    17,     5,
    21,    22,    23,    15,    15,     5,    15,    28,    16,     9,
    10,    13,    13,    16,    50,     6,    37,    16,    16,    40,
    41,    42,    22,    12,    54,     5,    47,    48,    16,     9,
    10,    52,    49,    14,    55,    50,     5,    16,    59,    60,
    61,    62,    22,    16,    44,    16,     5,    34,    15,    15,
     9,    10,    52,    13,    16,    55,    13,     5,    16,    59,
    60,    61,    62,    22,    16,    35,     5,    22,    31,     5,
     9,    10,    52,    16,    16,    55,    16,    14,    12,    59,
    60,    61,    62,    22,    31,     3,     4,     5,     6,     7,
    16,    16,    16,    52,    12,    16,    55,    35,    31,    14,
    59,    60,    61,    62,     5,    16,    14,    16,     0,     0,
   164,     1,    30,    52,   224,     3,    55,   223,   197,   192,
    59,    60,    61,    62,   355,   309,   331,    45,    46,   142,
    -1,   136,    -1,    51
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

case 1:
#line 156 "yacc.yy"
{
	  ;
    break;}
case 3:
#line 162 "yacc.yy"
{
		printf("<INCLUDE>%s</INCLUDE>\n", yyvsp[-1]._str->latin1() );
	  ;
    break;}
case 4:
#line 166 "yacc.yy"
{
	  ;
    break;}
case 5:
#line 169 "yacc.yy"
{
          ;
    break;}
case 6:
#line 174 "yacc.yy"
{ yyval._int = 1; ;
    break;}
case 7:
#line 175 "yacc.yy"
{ yyval._int = 0; ;
    break;}
case 8:
#line 179 "yacc.yy"
{
	 	if (yyvsp[-2]._int)
			  printf("<CLASS>\n    <NAME>%s</NAME>\n%s%s</CLASS>\n", ( in_namespace + *yyvsp[-4]._str ).latin1(), yyvsp[-3]._str->latin1(), yyvsp[-1]._str->latin1() );
	  ;
    break;}
case 9:
#line 184 "yacc.yy"
{
	  ;
    break;}
case 10:
#line 187 "yacc.yy"
{
	  ;
    break;}
case 11:
#line 190 "yacc.yy"
{
	  ;
    break;}
case 12:
#line 193 "yacc.yy"
{
                      in_namespace += *yyvsp[-1]._str; in_namespace += "::";
                  ;
    break;}
case 13:
#line 197 "yacc.yy"
{
                      int pos = in_namespace.findRev( "::", -3 );
                      if( pos >= 0 )
                          in_namespace = in_namespace.left( pos + 2 );
                      else
                          in_namespace = "";
                  ;
    break;}
case 14:
#line 205 "yacc.yy"
{
          ;
    break;}
case 15:
#line 208 "yacc.yy"
{
          ;
    break;}
case 16:
#line 211 "yacc.yy"
{
	  ;
    break;}
case 17:
#line 214 "yacc.yy"
{
	  ;
    break;}
case 18:
#line 217 "yacc.yy"
{
	  ;
    break;}
case 19:
#line 220 "yacc.yy"
{
	  ;
    break;}
case 20:
#line 223 "yacc.yy"
{
	  ;
    break;}
case 21:
#line 226 "yacc.yy"
{
	  ;
    break;}
case 22:
#line 229 "yacc.yy"
{
	  ;
    break;}
case 23:
#line 232 "yacc.yy"
{
	  ;
    break;}
case 34:
#line 247 "yacc.yy"
{
	  dcop_area = 0;
	  dcop_signal_area = 0;
	;
    break;}
case 35:
#line 252 "yacc.yy"
{
	  dcop_area = 0;
	  dcop_signal_area = 0;
	;
    break;}
case 36:
#line 259 "yacc.yy"
{
	  dcop_area = 1;
	;
    break;}
case 37:
#line 265 "yacc.yy"
{
	  dcop_signal_area = 1;
	;
    break;}
case 38:
#line 270 "yacc.yy"
{
	  yyval._str = yyvsp[0]._str;
	;
    break;}
case 39:
#line 273 "yacc.yy"
{
	   QString* tmp = new QString( "%1::%2" );
           *tmp = tmp->arg(*(yyvsp[-2]._str)).arg(*(yyvsp[0]._str));
           yyval._str = tmp;
	;
    break;}
case 40:
#line 281 "yacc.yy"
{
		QString* tmp = new QString( "    <SUPER>%1</SUPER>\n" );
		*tmp = tmp->arg( *(yyvsp[0]._str) );
		yyval._str = tmp;
	  ;
    break;}
case 41:
#line 287 "yacc.yy"
{
		QString* tmp = new QString( "    <SUPER>%1</SUPER>\n" );
		*tmp = tmp->arg( *(yyvsp[-3]._str) + "&lt" + *(yyvsp[-1]._str) + "&gt;" );
		yyval._str = tmp;
	  ;
    break;}
case 42:
#line 295 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 43:
#line 299 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 44:
#line 306 "yacc.yy"
{
		yyval._str = yyvsp[-1]._str;
	  ;
    break;}
case 45:
#line 310 "yacc.yy"
{
		/* $$ = $1; */
		yyval._str = new QString( *(yyvsp[-2]._str) + *(yyvsp[0]._str) );
	  ;
    break;}
case 46:
#line 318 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 47:
#line 322 "yacc.yy"
{
		yyval._str = new QString( "" );
	  ;
    break;}
case 48:
#line 329 "yacc.yy"
{
          ;
    break;}
case 50:
#line 336 "yacc.yy"
{
		yyval._str = new QString( "" );
	  ;
    break;}
case 51:
#line 340 "yacc.yy"
{
		yyval._str = new QString( *(yyvsp[-1]._str) + *(yyvsp[0]._str) );
	  ;
    break;}
case 52:
#line 344 "yacc.yy"
{
		yyval._str = new QString( *(yyvsp[-1]._str) + *(yyvsp[0]._str) );
	  ;
    break;}
case 53:
#line 348 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 54:
#line 352 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 55:
#line 356 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 56:
#line 360 "yacc.yy"
{	
	        yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 57:
#line 364 "yacc.yy"
{
 	        yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 58:
#line 368 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 59:
#line 372 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 60:
#line 376 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 61:
#line 380 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 62:
#line 384 "yacc.yy"
{
                yyval._str = yyvsp[0]._str;
          ;
    break;}
case 63:
#line 388 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 64:
#line 392 "yacc.yy"
{
                yyval._str = yyvsp[0]._str;
          ;
    break;}
case 65:
#line 396 "yacc.yy"
{
                yyval._str = yyvsp[0]._str;
          ;
    break;}
case 72:
#line 411 "yacc.yy"
{;
    break;}
case 73:
#line 412 "yacc.yy"
{;
    break;}
case 74:
#line 415 "yacc.yy"
{;
    break;}
case 75:
#line 416 "yacc.yy"
{;
    break;}
case 76:
#line 417 "yacc.yy"
{;
    break;}
case 77:
#line 418 "yacc.yy"
{;
    break;}
case 78:
#line 419 "yacc.yy"
{;
    break;}
case 79:
#line 422 "yacc.yy"
{;
    break;}
case 80:
#line 423 "yacc.yy"
{;
    break;}
case 81:
#line 424 "yacc.yy"
{;
    break;}
case 82:
#line 428 "yacc.yy"
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
case 83:
#line 438 "yacc.yy"
{
		if (dcop_area)
		  yyerror("scoped template typedefs are not supported in dcop areas!");
	  ;
    break;}
case 84:
#line 445 "yacc.yy"
{
		yyval._int = 0;
	  ;
    break;}
case 85:
#line 449 "yacc.yy"
{
		yyval._int = 1;
	  ;
    break;}
case 86:
#line 455 "yacc.yy"
{ yyval._str = new QString("signed int"); ;
    break;}
case 87:
#line 456 "yacc.yy"
{ yyval._str = new QString("signed int"); ;
    break;}
case 88:
#line 457 "yacc.yy"
{ yyval._str = new QString("unsigned int"); ;
    break;}
case 89:
#line 458 "yacc.yy"
{ yyval._str = new QString("unsigned int"); ;
    break;}
case 90:
#line 459 "yacc.yy"
{ yyval._str = new QString("signed short int"); ;
    break;}
case 91:
#line 460 "yacc.yy"
{ yyval._str = new QString("signed short int"); ;
    break;}
case 92:
#line 461 "yacc.yy"
{ yyval._str = new QString("signed long int"); ;
    break;}
case 93:
#line 462 "yacc.yy"
{ yyval._str = new QString("signed long int"); ;
    break;}
case 94:
#line 463 "yacc.yy"
{ yyval._str = new QString("unsigned short int"); ;
    break;}
case 95:
#line 464 "yacc.yy"
{ yyval._str = new QString("unsigned short int"); ;
    break;}
case 96:
#line 465 "yacc.yy"
{ yyval._str = new QString("unsigned long int"); ;
    break;}
case 97:
#line 466 "yacc.yy"
{ yyval._str = new QString("unsigned long int"); ;
    break;}
case 98:
#line 467 "yacc.yy"
{ yyval._str = new QString("int"); ;
    break;}
case 99:
#line 468 "yacc.yy"
{ yyval._str = new QString("long int"); ;
    break;}
case 100:
#line 469 "yacc.yy"
{ yyval._str = new QString("long int"); ;
    break;}
case 101:
#line 470 "yacc.yy"
{ yyval._str = new QString("short int"); ;
    break;}
case 102:
#line 471 "yacc.yy"
{ yyval._str = new QString("short int"); ;
    break;}
case 103:
#line 472 "yacc.yy"
{ yyval._str = new QString("char"); ;
    break;}
case 104:
#line 473 "yacc.yy"
{ yyval._str = new QString("signed char"); ;
    break;}
case 105:
#line 474 "yacc.yy"
{ yyval._str = new QString("unsigned char"); ;
    break;}
case 108:
#line 482 "yacc.yy"
{
		yyval._str = new QString( "" );
	  ;
    break;}
case 110:
#line 487 "yacc.yy"
{
		yyval._str = new QString( *(yyvsp[-2]._str) + *(yyvsp[0]._str) );
	  ;
    break;}
case 112:
#line 495 "yacc.yy"
{ yyval._str = yyvsp[0]._str; ;
    break;}
case 113:
#line 496 "yacc.yy"
{ yyval._str = yyvsp[0]._str; ;
    break;}
case 114:
#line 497 "yacc.yy"
{ yyval._str = yyvsp[0]._str; ;
    break;}
case 115:
#line 498 "yacc.yy"
{
		QString *tmp = new QString("%1&lt;%2&gt;");
		*tmp = tmp->arg(*(yyvsp[-3]._str));
		*tmp = tmp->arg(*(yyvsp[-1]._str));
		yyval._str = tmp;
	 ;
    break;}
case 116:
#line 504 "yacc.yy"
{
		QString *tmp = new QString("%1&lt;%2&gt;::%3");
		*tmp = tmp->arg(*(yyvsp[-5]._str));
		*tmp = tmp->arg(*(yyvsp[-3]._str));
		*tmp = tmp->arg(*(yyvsp[0]._str));
		yyval._str = tmp;
	 ;
    break;}
case 117:
#line 515 "yacc.yy"
{
	    yyval._str = new QString(*(yyvsp[-2]._str) + "," + *(yyvsp[0]._str));
	  ;
    break;}
case 118:
#line 519 "yacc.yy"
{
 	    yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 119:
#line 526 "yacc.yy"
{
	    if (dcop_area)
	      yyerror("in dcop areas are no pointers allowed");
	  ;
    break;}
case 120:
#line 531 "yacc.yy"
{
 	    yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 121:
#line 539 "yacc.yy"
{
	    if (dcop_area)
	      yyerror("in dcop areas are no pointers allowed");
	  ;
    break;}
case 122:
#line 543 "yacc.yy"
{
	     if (dcop_area) {
	  	QString* tmp = new QString("<TYPE  qleft=\"const\" qright=\"" AMP_ENTITY "\">%1</TYPE>");
		*tmp = tmp->arg( *(yyvsp[-1]._str) );
		yyval._str = tmp;
	     }
	  ;
    break;}
case 123:
#line 550 "yacc.yy"
{
		QString* tmp = new QString("<TYPE>%1</TYPE>");
		*tmp = tmp->arg( *(yyvsp[0]._str) );
		yyval._str = tmp;
	;
    break;}
case 124:
#line 555 "yacc.yy"
{
	     if (dcop_area)
		yyerror("in dcop areas are only const references allowed!");
	  ;
    break;}
case 125:
#line 560 "yacc.yy"
{
		QString* tmp = new QString("<TYPE>%1</TYPE>");
		*tmp = tmp->arg( *(yyvsp[0]._str) );
		yyval._str = tmp;
	;
    break;}
case 126:
#line 566 "yacc.yy"
{
	    if (dcop_area)
	      yyerror("in dcop areas are no pointers allowed");
	  ;
    break;}
case 127:
#line 574 "yacc.yy"
{
	    yyval._str = new QString(*(yyvsp[-2]._str) + "," + *(yyvsp[0]._str));
	  ;
    break;}
case 128:
#line 578 "yacc.yy"
{
 	    yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 129:
#line 584 "yacc.yy"
{
		if (dcop_area) {
		   QString* tmp = new QString("\n        <ARG>%1<NAME>%2</NAME></ARG>");
  		   *tmp = tmp->arg( *(yyvsp[-2]._str) );
  		   *tmp = tmp->arg( *(yyvsp[-1]._str) );
		   yyval._str = tmp;		
		} else yyval._str = new QString();
	  ;
    break;}
case 130:
#line 593 "yacc.yy"
{
		if (dcop_area) {
		   QString* tmp = new QString("\n        <ARG>%1</ARG>");
  		   *tmp = tmp->arg( *(yyvsp[-1]._str) );
		   yyval._str = tmp;		
		} else yyval._str = new QString();
	  ;
    break;}
case 131:
#line 601 "yacc.yy"
{
		if (dcop_area)
			yyerror("variable arguments not supported in dcop area.");
		yyval._str = new QString("");
	  ;
    break;}
case 132:
#line 609 "yacc.yy"
{
	  ;
    break;}
case 133:
#line 612 "yacc.yy"
{
	  ;
    break;}
case 134:
#line 615 "yacc.yy"
{
	  ;
    break;}
case 135:
#line 621 "yacc.yy"
{
          ;
    break;}
case 136:
#line 624 "yacc.yy"
{
          ;
    break;}
case 137:
#line 627 "yacc.yy"
{
          ;
    break;}
case 138:
#line 630 "yacc.yy"
{
          ;
    break;}
case 139:
#line 633 "yacc.yy"
{
          ;
    break;}
case 140:
#line 638 "yacc.yy"
{ yyval._int = 0; ;
    break;}
case 141:
#line 639 "yacc.yy"
{ yyval._int = 1; ;
    break;}
case 147:
#line 646 "yacc.yy"
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
case 148:
#line 677 "yacc.yy"
{
	     if (dcop_area)
		yyerror("operators aren't allowed in dcop areas!");
	     yyval._str = new QString("");
	  ;
    break;}
case 149:
#line 684 "yacc.yy"
{;
    break;}
case 150:
#line 685 "yacc.yy"
{;
    break;}
case 151:
#line 686 "yacc.yy"
{;
    break;}
case 152:
#line 689 "yacc.yy"
{;
    break;}
case 153:
#line 690 "yacc.yy"
{;
    break;}
case 154:
#line 693 "yacc.yy"
{;
    break;}
case 155:
#line 696 "yacc.yy"
{;
    break;}
case 156:
#line 697 "yacc.yy"
{;
    break;}
case 157:
#line 701 "yacc.yy"
{
	        yyval._str = yyvsp[-1]._str;
	  ;
    break;}
case 158:
#line 705 "yacc.yy"
{
		yyval._str = yyvsp[-3]._str;
	  ;
    break;}
case 159:
#line 709 "yacc.yy"
{
		yyval._str = yyvsp[-1]._str;
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
	      /* The constructor */
	      assert(!dcop_area);
              yyval._str = new QString("");
	  ;
    break;}
case 162:
#line 725 "yacc.yy"
{
	      /* The destructor */
  	      assert(!dcop_area);
              yyval._str = new QString("");
	  ;
    break;}
case 163:
#line 731 "yacc.yy"
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
case 164:
#line 743 "yacc.yy"
{
		function_mode = 1;
	;
    break;}
case 168:
#line 753 "yacc.yy"
{;
    break;}
case 169:
#line 754 "yacc.yy"
{;
    break;}
case 173:
#line 763 "yacc.yy"
{;
    break;}
case 174:
#line 764 "yacc.yy"
{;
    break;}
case 175:
#line 765 "yacc.yy"
{;
    break;}
case 176:
#line 767 "yacc.yy"
{;
    break;}
case 177:
#line 770 "yacc.yy"
{;
    break;}
case 178:
#line 771 "yacc.yy"
{;
    break;}
case 179:
#line 772 "yacc.yy"
{;
    break;}
case 180:
#line 773 "yacc.yy"
{;
    break;}
case 181:
#line 774 "yacc.yy"
{;
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
#line 776 "yacc.yy"


void dcopidlParse( const char *_code )
{
    dcopidlInitFlex( _code );
    yyparse();
}
