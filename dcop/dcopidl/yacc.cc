
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



#define	YYFINAL		360
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
   580,   583,   584,   586,   590,   593,   596,   600,   606,   612
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
     0,     9,    77,    81,    83,    16,    83,     0,    10,    77,
    16,    83,     0,    10,    77,    81,    83,    16,    83,     0,
    42,     5,    49,     5,    16,    83,     0,    40,     5,    11,
    85,    13,     5,    16,     0,    40,     5,    11,    85,    13,
    16,     0,    40,    11,    85,    13,     5,    16,     0,    40,
    11,    85,    13,    16,     0,    86,    31,    85,     0,    86,
     0,     5,    48,    88,     0,     5,     0,     3,     0,     6,
     0,    30,     6,     0,    50,     0,    77,     0,    87,     0,
    87,    29,    87,     0,    87,    64,    87,     0,    28,    77,
    34,    98,    35,    77,    16,     0,    28,    77,    34,    98,
    35,    49,     5,    77,    16,     0,     0,    22,     0,    58,
     0,    58,    51,     0,    59,     0,    59,    51,     0,    58,
    61,     0,    58,    61,    51,     0,    58,    60,     0,    58,
    60,    51,     0,    59,    61,     0,    59,    61,    51,     0,
    59,    60,     0,    59,    60,    51,     0,    51,     0,    60,
     0,    60,    51,     0,    61,     0,    61,    51,     0,    54,
     0,    58,    54,     0,    59,    54,     0,    32,    92,     0,
    32,     0,     0,    99,     0,    93,    31,    99,     0,    91,
     0,    77,     0,    10,    77,     0,     9,    77,     0,    77,
    34,    95,    35,     0,    77,    34,    95,    35,    49,    77,
     0,    96,    31,    95,     0,    96,     0,    94,    92,     0,
    94,     0,    22,    94,    92,     0,    22,    94,    36,     0,
    22,    94,     0,    94,    36,     0,    94,     0,    94,    92,
     0,    97,    31,    98,     0,    97,     0,    97,    77,   100,
     0,    97,   100,     0,    44,     0,     0,    48,   101,     0,
    48,    12,    97,    14,   101,     0,     7,     0,    88,     0,
     4,     0,    71,     0,    77,    12,    93,    14,     0,     0,
    21,     0,    63,     0,    64,     0,    35,     0,    34,     0,
    48,     0,    97,    77,    12,    93,    14,    90,     0,    97,
    62,   103,    12,    93,    14,    90,     0,    87,     0,    71,
     0,     5,    12,    14,     0,   105,     0,   105,    31,   106,
     0,     5,    12,   106,    14,     0,   107,     0,   107,    31,
   108,     0,   104,   111,     0,    21,   104,    48,    50,   111,
     0,    21,   104,   111,     0,    77,    12,    93,    14,   111,
     0,    77,    12,    93,    14,    15,   108,   111,     0,   102,
    33,    77,    12,    14,   111,     0,    47,   104,   111,     0,
    11,     0,    16,     0,   110,   112,    13,     0,   110,   112,
    13,    16,     0,   113,   112,     0,     0,    16,     0,    31,
   116,     0,     0,     5,     0,     5,    48,   101,     0,    92,
     5,     0,   115,   114,     0,    97,   116,    16,     0,    97,
    77,    15,     6,    16,     0,    47,    97,     5,   100,    16,
     0,    97,     5,    52,    88,    53,    16,     0
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
   378,   382,   386,   390,   394,   400,   401,   402,   403,   406,
   407,   410,   411,   414,   415,   416,   417,   418,   421,   422,
   423,   426,   436,   443,   447,   454,   455,   456,   457,   458,
   459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
   469,   470,   471,   472,   473,   476,   477,   480,   484,   485,
   493,   494,   495,   496,   497,   503,   513,   517,   524,   529,
   537,   542,   549,   554,   559,   564,   572,   576,   582,   591,
   599,   607,   610,   613,   619,   622,   625,   628,   631,   637,
   638,   641,   641,   641,   641,   641,   644,   675,   683,   684,
   685,   688,   689,   692,   695,   696,   699,   703,   707,   711,
   717,   723,   729,   741,   747,   748,   749,   752,   753,   756,
   759,   760,   762,   763,   764,   766,   769,   770,   771,   772
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
    83,    83,    83,    83,    83,    84,    84,    84,    84,    85,
    85,    86,    86,    87,    87,    87,    87,    87,    88,    88,
    88,    89,    89,    90,    90,    91,    91,    91,    91,    91,
    91,    91,    91,    91,    91,    91,    91,    91,    91,    91,
    91,    91,    91,    91,    91,    92,    92,    93,    93,    93,
    94,    94,    94,    94,    94,    94,    95,    95,    96,    96,
    97,    97,    97,    97,    97,    97,    98,    98,    99,    99,
    99,   100,   100,   100,   101,   101,   101,   101,   101,   102,
   102,   103,   103,   103,   103,   103,   104,   104,   105,   105,
   105,   106,   106,   107,   108,   108,   109,   109,   109,   109,
   109,   109,   109,   110,   111,   111,   111,   112,   112,   113,
   114,   114,   115,   115,   115,   116,   117,   117,   117,   117
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
     2,     0,     1,     3,     2,     2,     3,     5,     5,     6
};

static const short yydefact[] = {     5,
     5,     0,   140,     3,     5,    38,     0,     0,   141,     0,
   140,     0,     0,     0,     0,     0,     0,    98,   103,    86,
    88,    99,   101,     5,   112,    23,   111,   125,     0,     0,
     0,    21,    22,     0,     0,   114,   113,     0,     0,   112,
     0,     0,   123,     0,    20,     0,     0,    16,     0,     0,
     0,     0,     0,     0,     0,    87,   104,    92,    90,    89,
   105,    96,    94,   100,   102,     1,   108,     0,   107,   124,
   126,    38,     0,     0,     0,   172,     0,     0,   164,   165,
   169,   157,     4,    39,    47,   140,     9,     7,    10,    33,
   114,   113,     0,     0,   159,   122,   121,    25,   113,     0,
     0,    73,     0,    71,    12,     0,     0,    38,   163,    93,
    91,    97,    95,   131,     0,   132,   109,   120,     0,   118,
   106,     0,     0,   145,   144,   146,   142,   143,     0,   108,
     0,   175,     0,   176,   177,     0,   170,     0,   169,   141,
    40,    43,     0,    46,     0,     6,    33,     0,     0,    50,
    30,    29,    28,     0,    31,    32,     0,     0,     0,     0,
    33,     0,    33,    33,    33,     0,    33,    33,    33,    33,
     0,     0,     0,     0,    25,    25,    17,     0,     0,     0,
     0,     5,     0,    14,     0,     0,     0,     0,   132,   130,
   119,   115,     0,    74,   137,    75,   135,     0,    26,    27,
    77,   138,    78,    79,   136,   174,    78,     0,   108,     0,
     0,   173,   171,     0,   166,   168,     0,    44,   140,     0,
     0,   114,   113,     0,   112,     0,     0,     0,    36,    37,
     0,    35,    56,    55,    53,    11,    54,    51,    52,    57,
   158,     0,     0,     0,    24,     0,     0,    72,     0,    69,
    70,     0,    15,     0,   133,   179,     0,   160,   110,   129,
     0,   117,    76,   108,     0,     0,     0,     0,    84,   178,
     0,   167,   128,     0,    45,    42,     8,    33,    33,    33,
    33,   114,    33,    33,     0,     0,    34,   132,     0,     0,
     0,    67,    68,    48,     0,     0,   155,     0,   116,     0,
    80,    81,   180,    84,    85,   147,   162,     0,    41,    61,
     0,    63,     0,    33,    59,    60,     0,     0,    18,     0,
    66,    49,    13,     0,     0,     0,   161,   139,   148,   127,
    33,    33,    58,     0,    33,    19,   134,    38,   150,   149,
   152,     0,   156,    62,    64,     0,     0,    65,     0,     0,
   154,     0,    82,   151,   153,     0,    83,     0,     0,     0
};

static const short yydefgoto[] = {    34,
     3,   147,    24,   182,   173,   202,   161,   162,   163,   164,
   165,    25,   142,   143,   144,    88,   323,   166,   167,   103,
   104,   204,   205,   168,   306,    27,    75,   115,    28,   119,
   120,    29,   274,   117,   186,   206,    30,   129,    31,   341,
   342,   297,   298,   169,    81,    82,   138,   139,   134,    76,
    77,   170
};

static const short yypact[] = {    42,
    38,    52,   354,-32768,   149,    63,   138,   138,   415,    59,
   216,   434,   111,   103,   140,    51,   415,-32768,-32768,   239,
   295,   101,   115,   126,    91,-32768,-32768,   195,    20,   141,
   219,-32768,-32768,   205,   138,    43,   234,   138,   138,   181,
    31,    90,   276,   415,-32768,   217,   138,-32768,   185,   224,
   221,   197,   264,    36,   219,-32768,-32768,   193,   237,-32768,
-32768,   243,   246,-32768,-32768,-32768,   375,    59,   255,-32768,
-32768,   128,   250,   241,   298,   279,   303,   138,-32768,-32768,
   306,-32768,-32768,-32768,-32768,   202,-32768,   256,-32768,   311,
-32768,-32768,   313,   273,-32768,-32768,-32768,   394,   315,   314,
   224,   286,   323,   309,-32768,   336,   326,   157,-32768,-32768,
-32768,-32768,-32768,-32768,   122,    25,-32768,   255,   312,   317,
-32768,   125,    72,-32768,-32768,-32768,-32768,-32768,   338,   375,
   346,-32768,    40,-32768,-32768,   342,-32768,   344,   306,-32768,
   327,-32768,   129,-32768,   343,-32768,   311,   138,   138,-32768,
-32768,-32768,-32768,   455,-32768,-32768,   138,   361,   358,   359,
   162,   363,   311,   311,   311,   365,   311,   311,   311,   311,
   219,   415,   366,   119,   394,   394,-32768,   370,    72,   203,
   224,   149,   371,-32768,   236,   372,   257,   375,   341,-32768,
-32768,   337,    59,-32768,-32768,-32768,-32768,   384,-32768,-32768,
-32768,-32768,   380,    71,-32768,-32768,-32768,   340,   375,   132,
   382,   352,-32768,   388,   390,-32768,   415,-32768,   202,   138,
   391,   280,   291,   138,    28,   393,   376,   362,-32768,-32768,
   402,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,   413,   138,   406,-32768,   409,   204,-32768,   407,-32768,
-32768,   414,-32768,   415,-32768,-32768,   423,-32768,-32768,-32768,
   138,-32768,-32768,   375,    72,    72,   416,   180,   408,-32768,
   219,-32768,   400,   403,-32768,-32768,-32768,   311,   311,   311,
   311,   424,   311,   311,   415,   437,-32768,   341,   430,   138,
   431,-32768,-32768,   433,   436,   439,   426,   219,-32768,   183,
-32768,-32768,-32768,   408,-32768,-32768,-32768,   415,-32768,-32768,
   442,-32768,   443,   311,-32768,-32768,   427,   445,-32768,   447,
-32768,-32768,-32768,   125,   259,   423,-32768,-32768,-32768,-32768,
   311,   311,-32768,    48,   311,-32768,-32768,    21,-32768,-32768,
   440,   453,-32768,-32768,-32768,   463,   454,-32768,   458,   259,
-32768,   138,-32768,-32768,-32768,   462,-32768,   479,   480,-32768
};

static const short yypgoto[] = {    11,
   481,-32768,-32768,-32768,   104,  -229,-32768,   320,-32768,-32768,
-32768,    -7,   263,-32768,   265,   -31,-32768,   -80,   483,   -86,
-32768,  -239,   -85,-32768,   186,-32768,   -14,  -110,    -3,   294,
-32768,     7,  -242,   301,  -108,  -176,   -63,-32768,     4,-32768,
   146,-32768,   165,    46,-32768,   -38,   360,-32768,-32768,-32768,
   364,     9
};


#define	YYLAST		516


static const short yytable[] = {    36,
    37,    40,    40,    95,    40,    90,    43,   190,   255,    40,
   358,    33,    42,    71,   178,    41,   109,    41,    47,   210,
    55,    74,   145,    54,    72,   301,   302,    84,    97,     6,
    91,    92,   349,    93,    66,     6,    40,   208,    99,   100,
   108,    -2,   317,   283,   212,     1,    93,    55,    32,     1,
    41,    69,     6,    85,   121,    52,    45,    86,    87,    40,
    40,    68,     5,     6,   118,   330,   221,    38,    39,    35,
   136,    69,   185,   116,   194,     2,     6,   196,   141,     2,
   260,    73,   233,   234,   235,   340,   237,   238,   239,   240,
    40,    53,    73,   248,   251,   339,   346,    73,   268,   265,
    79,   198,    67,   191,   174,    80,   175,    49,   189,    18,
   340,    35,    19,    50,   203,   207,    20,    21,    22,    23,
   339,   201,    40,    72,    68,    -2,    48,   194,   195,     6,
   196,   197,   241,     1,   266,   187,   116,    94,    -2,   218,
   222,   223,     6,  -173,    51,   269,   225,   337,   258,   227,
    69,    64,   188,   300,   198,   145,     1,   226,  -173,   219,
    41,    -2,   188,     2,    40,    65,   244,    40,    40,   199,
   200,   207,  -132,    78,   201,   122,    35,   203,   242,   123,
    40,   174,   174,   175,   175,    40,     2,   155,   156,   118,
   279,   281,   252,   304,   116,   101,   328,   310,   311,   312,
   313,    40,   315,   316,   185,    35,     6,   249,   291,    40,
   188,   141,   141,   188,    68,   116,   282,    83,   250,   292,
     6,     6,   140,   273,    38,    39,    69,    98,   102,    79,
    70,   105,   307,   333,    80,   289,     9,    10,   194,   195,
     6,   196,   197,   110,    85,   106,    40,   254,    86,    89,
   344,   345,   130,   299,   348,   131,    40,   207,   207,   327,
   295,   194,    44,   338,   196,   198,    18,    79,   107,    19,
   116,   257,    80,    20,    21,    22,    23,    40,   245,   246,
   199,   200,   320,   124,   125,   201,    69,   111,   198,    56,
    85,   273,    57,   112,    86,   278,   113,   126,    58,    59,
    40,    85,   132,   199,   200,    86,   280,    69,   201,   133,
   146,    96,   127,   128,   273,     6,   203,   207,   135,   148,
   149,   137,   171,   150,   130,   176,   347,   151,   152,   177,
   153,     9,    10,   179,   154,   180,   155,   156,   157,   181,
   183,   184,   207,  -140,   356,    60,   192,   193,    61,   209,
    14,   211,   158,   214,    62,    63,   215,    17,     6,   220,
   217,    18,     7,     8,    19,   228,   159,   160,    20,    21,
    22,    23,   229,   230,     9,    10,    11,   232,   243,     6,
   236,    12,   247,    38,    39,   261,   253,   256,   185,   263,
    13,   264,   267,    14,    15,    16,    10,   270,     6,   122,
    17,   271,    38,    39,    18,   272,   277,    19,   284,   285,
   286,    20,    21,    22,    23,    10,   287,   288,   114,     6,
   131,   290,   293,    38,    39,    18,   294,   296,    19,   305,
   308,   303,    20,    21,    22,    23,    10,   309,     6,   314,
   172,   318,    38,    46,    18,   319,   321,    19,   322,   324,
   325,    20,    21,    22,    23,    10,   326,   331,   332,     6,
   335,   334,   336,   224,    39,    18,   351,   352,    19,   353,
   350,   354,    20,    21,    22,    23,    10,   357,   359,   360,
   231,     4,   276,   275,    18,    26,   262,    19,   259,   329,
   343,    20,    21,    22,    23,   355,   213,     0,   216,     0,
     0,     0,     0,     0,     0,    18,     0,     0,    19,     0,
     0,     0,    20,    21,    22,    23
};

static const short yycheck[] = {     7,
     8,     9,    10,    42,    12,    37,    10,   116,   185,    17,
     0,     3,     9,    28,   101,     9,    55,    11,    12,   130,
    17,    29,    86,    17,     5,   265,   266,    35,    43,     5,
    38,    39,    12,    41,    24,     5,    44,   123,    46,    47,
     5,     0,   285,    16,     5,     8,    54,    44,     3,     8,
    44,    32,     5,    11,    69,     5,    11,    15,    16,    67,
    68,    34,    11,     5,    68,   308,   147,     9,    10,    49,
    78,    32,    48,    67,     3,    38,     5,     6,    86,    38,
   189,    62,   163,   164,   165,   325,   167,   168,   169,   170,
    98,    41,    62,   179,   181,   325,    49,    62,   209,    29,
    11,    30,    12,   118,    98,    16,    98,     5,   116,    51,
   350,    49,    54,    11,   122,   123,    58,    59,    60,    61,
   350,    50,   130,     5,    34,     0,    16,     3,     4,     5,
     6,     7,   171,     8,    64,    14,   130,    48,    13,    11,
   148,   149,     5,    16,     5,    14,   154,   324,   187,   157,
    32,    51,    31,   264,    30,   219,     8,   154,    31,    31,
   154,    13,    31,    38,   172,    51,   174,   175,   176,    45,
    46,   179,    16,    33,    50,    48,    49,   185,   172,    52,
   188,   175,   176,   175,   176,   193,    38,    26,    27,   193,
   222,   223,   182,    14,   188,    11,    14,   278,   279,   280,
   281,   209,   283,   284,    48,    49,     5,     5,     5,   217,
    31,   219,   220,    31,    34,   209,   224,    13,    16,    16,
     5,     5,    21,   217,     9,    10,    32,    11,     5,    11,
    36,    11,   271,   314,    16,   243,    21,    22,     3,     4,
     5,     6,     7,    51,    11,    49,   254,    12,    15,    16,
   331,   332,    12,   261,   335,    15,   264,   265,   266,   298,
   254,     3,    47,     5,     6,    30,    51,    11,     5,    54,
   264,    15,    16,    58,    59,    60,    61,   285,   175,   176,
    45,    46,   290,    34,    35,    50,    32,    51,    30,    51,
    11,   285,    54,    51,    15,    16,    51,    48,    60,    61,
   308,    11,     5,    45,    46,    15,    16,    32,    50,    31,
    55,    36,    63,    64,   308,     5,   324,   325,    16,     9,
    10,    16,    50,    13,    12,    11,   334,    17,    18,    16,
    20,    21,    22,    48,    24,    13,    26,    27,    28,    31,
     5,    16,   350,    33,   352,    51,    35,    31,    54,    12,
    40,     6,    42,    12,    60,    61,    13,    47,     5,    17,
    34,    51,     9,    10,    54,     5,    56,    57,    58,    59,
    60,    61,    15,    15,    21,    22,    23,    15,    13,     5,
    16,    28,    13,     9,    10,    49,    16,    16,    48,     6,
    37,    12,    53,    40,    41,    42,    22,    16,     5,    48,
    47,    14,     9,    10,    51,    16,    16,    54,    16,    34,
    49,    58,    59,    60,    61,    22,    15,     5,    44,     5,
    15,    13,    16,     9,    10,    51,    13,     5,    54,    22,
    31,    16,    58,    59,    60,    61,    22,    35,     5,    16,
    47,     5,     9,    10,    51,    16,    16,    54,    16,    14,
    12,    58,    59,    60,    61,    22,    31,    16,    16,     5,
    16,    35,    16,     9,    10,    51,    14,     5,    54,    16,
    31,    14,    58,    59,    60,    61,    22,    16,     0,     0,
   161,     1,   220,   219,    51,     3,   193,    54,   188,   304,
   326,    58,    59,    60,    61,   350,   133,    -1,   139,    -1,
    -1,    -1,    -1,    -1,    -1,    51,    -1,    -1,    54,    -1,
    -1,    -1,    58,    59,    60,    61
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
case 64:
#line 391 "yacc.yy"
{
                yyval._str = yyvsp[0]._str;
          ;
    break;}
case 65:
#line 395 "yacc.yy"
{
                yyval._str = yyvsp[0]._str;
          ;
    break;}
case 72:
#line 410 "yacc.yy"
{;
    break;}
case 73:
#line 411 "yacc.yy"
{;
    break;}
case 74:
#line 414 "yacc.yy"
{;
    break;}
case 75:
#line 415 "yacc.yy"
{;
    break;}
case 76:
#line 416 "yacc.yy"
{;
    break;}
case 77:
#line 417 "yacc.yy"
{;
    break;}
case 78:
#line 418 "yacc.yy"
{;
    break;}
case 79:
#line 421 "yacc.yy"
{;
    break;}
case 80:
#line 422 "yacc.yy"
{;
    break;}
case 81:
#line 423 "yacc.yy"
{;
    break;}
case 82:
#line 427 "yacc.yy"
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
#line 437 "yacc.yy"
{
		if (dcop_area)
		  yyerror("scoped template typedefs are not supported in dcop areas!");
	  ;
    break;}
case 84:
#line 444 "yacc.yy"
{
		yyval._int = 0;
	  ;
    break;}
case 85:
#line 448 "yacc.yy"
{
		yyval._int = 1;
	  ;
    break;}
case 86:
#line 454 "yacc.yy"
{ yyval._str = new QString("signed int"); ;
    break;}
case 87:
#line 455 "yacc.yy"
{ yyval._str = new QString("signed int"); ;
    break;}
case 88:
#line 456 "yacc.yy"
{ yyval._str = new QString("unsigned int"); ;
    break;}
case 89:
#line 457 "yacc.yy"
{ yyval._str = new QString("unsigned int"); ;
    break;}
case 90:
#line 458 "yacc.yy"
{ yyval._str = new QString("signed short int"); ;
    break;}
case 91:
#line 459 "yacc.yy"
{ yyval._str = new QString("signed short int"); ;
    break;}
case 92:
#line 460 "yacc.yy"
{ yyval._str = new QString("signed long int"); ;
    break;}
case 93:
#line 461 "yacc.yy"
{ yyval._str = new QString("signed long int"); ;
    break;}
case 94:
#line 462 "yacc.yy"
{ yyval._str = new QString("unsigned short int"); ;
    break;}
case 95:
#line 463 "yacc.yy"
{ yyval._str = new QString("unsigned short int"); ;
    break;}
case 96:
#line 464 "yacc.yy"
{ yyval._str = new QString("unsigned long int"); ;
    break;}
case 97:
#line 465 "yacc.yy"
{ yyval._str = new QString("unsigned long int"); ;
    break;}
case 98:
#line 466 "yacc.yy"
{ yyval._str = new QString("int"); ;
    break;}
case 99:
#line 467 "yacc.yy"
{ yyval._str = new QString("long int"); ;
    break;}
case 100:
#line 468 "yacc.yy"
{ yyval._str = new QString("long int"); ;
    break;}
case 101:
#line 469 "yacc.yy"
{ yyval._str = new QString("short int"); ;
    break;}
case 102:
#line 470 "yacc.yy"
{ yyval._str = new QString("short int"); ;
    break;}
case 103:
#line 471 "yacc.yy"
{ yyval._str = new QString("char"); ;
    break;}
case 104:
#line 472 "yacc.yy"
{ yyval._str = new QString("signed char"); ;
    break;}
case 105:
#line 473 "yacc.yy"
{ yyval._str = new QString("unsigned char"); ;
    break;}
case 108:
#line 481 "yacc.yy"
{
		yyval._str = new QString( "" );
	  ;
    break;}
case 110:
#line 486 "yacc.yy"
{
		yyval._str = new QString( *(yyvsp[-2]._str) + *(yyvsp[0]._str) );
	  ;
    break;}
case 112:
#line 494 "yacc.yy"
{ yyval._str = yyvsp[0]._str; ;
    break;}
case 113:
#line 495 "yacc.yy"
{ yyval._str = yyvsp[0]._str; ;
    break;}
case 114:
#line 496 "yacc.yy"
{ yyval._str = yyvsp[0]._str; ;
    break;}
case 115:
#line 497 "yacc.yy"
{
		QString *tmp = new QString("%1&lt;%2&gt;");
		*tmp = tmp->arg(*(yyvsp[-3]._str));
		*tmp = tmp->arg(*(yyvsp[-1]._str));
		yyval._str = tmp;
	 ;
    break;}
case 116:
#line 503 "yacc.yy"
{
		QString *tmp = new QString("%1&lt;%2&gt;::%3");
		*tmp = tmp->arg(*(yyvsp[-5]._str));
		*tmp = tmp->arg(*(yyvsp[-3]._str));
		*tmp = tmp->arg(*(yyvsp[0]._str));
		yyval._str = tmp;
	 ;
    break;}
case 117:
#line 514 "yacc.yy"
{
	    yyval._str = new QString(*(yyvsp[-2]._str) + "," + *(yyvsp[0]._str));
	  ;
    break;}
case 118:
#line 518 "yacc.yy"
{
 	    yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 119:
#line 525 "yacc.yy"
{
	    if (dcop_area)
	      yyerror("in dcop areas are no pointers allowed");
	  ;
    break;}
case 120:
#line 530 "yacc.yy"
{
 	    yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 121:
#line 538 "yacc.yy"
{
	    if (dcop_area)
	      yyerror("in dcop areas are no pointers allowed");
	  ;
    break;}
case 122:
#line 542 "yacc.yy"
{
	     if (dcop_area) {
	  	QString* tmp = new QString("<TYPE  qleft=\"const\" qright=\"" AMP_ENTITY "\">%1</TYPE>");
		*tmp = tmp->arg( *(yyvsp[-1]._str) );
		yyval._str = tmp;
	     }
	  ;
    break;}
case 123:
#line 549 "yacc.yy"
{
		QString* tmp = new QString("<TYPE>%1</TYPE>");
		*tmp = tmp->arg( *(yyvsp[0]._str) );
		yyval._str = tmp;
	;
    break;}
case 124:
#line 554 "yacc.yy"
{
	     if (dcop_area)
		yyerror("in dcop areas are only const references allowed!");
	  ;
    break;}
case 125:
#line 559 "yacc.yy"
{
		QString* tmp = new QString("<TYPE>%1</TYPE>");
		*tmp = tmp->arg( *(yyvsp[0]._str) );
		yyval._str = tmp;
	;
    break;}
case 126:
#line 565 "yacc.yy"
{
	    if (dcop_area)
	      yyerror("in dcop areas are no pointers allowed");
	  ;
    break;}
case 127:
#line 573 "yacc.yy"
{
	    yyval._str = new QString(*(yyvsp[-2]._str) + "," + *(yyvsp[0]._str));
	  ;
    break;}
case 128:
#line 577 "yacc.yy"
{
 	    yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 129:
#line 583 "yacc.yy"
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
#line 592 "yacc.yy"
{
		if (dcop_area) {
		   QString* tmp = new QString("\n        <ARG>%1</ARG>");
  		   *tmp = tmp->arg( *(yyvsp[-1]._str) );
		   yyval._str = tmp;		
		} else yyval._str = new QString();
	  ;
    break;}
case 131:
#line 600 "yacc.yy"
{
		if (dcop_area)
			yyerror("variable arguments not supported in dcop area.");
		yyval._str = new QString("");
	  ;
    break;}
case 132:
#line 608 "yacc.yy"
{
	  ;
    break;}
case 133:
#line 611 "yacc.yy"
{
	  ;
    break;}
case 134:
#line 614 "yacc.yy"
{
	  ;
    break;}
case 135:
#line 620 "yacc.yy"
{
          ;
    break;}
case 136:
#line 623 "yacc.yy"
{
          ;
    break;}
case 137:
#line 626 "yacc.yy"
{
          ;
    break;}
case 138:
#line 629 "yacc.yy"
{
          ;
    break;}
case 139:
#line 632 "yacc.yy"
{
          ;
    break;}
case 140:
#line 637 "yacc.yy"
{ yyval._int = 0; ;
    break;}
case 141:
#line 638 "yacc.yy"
{ yyval._int = 1; ;
    break;}
case 147:
#line 645 "yacc.yy"
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
#line 676 "yacc.yy"
{
	     if (dcop_area)
		yyerror("operators aren't allowed in dcop areas!");
	     yyval._str = new QString("");
	  ;
    break;}
case 149:
#line 683 "yacc.yy"
{;
    break;}
case 150:
#line 684 "yacc.yy"
{;
    break;}
case 151:
#line 685 "yacc.yy"
{;
    break;}
case 152:
#line 688 "yacc.yy"
{;
    break;}
case 153:
#line 689 "yacc.yy"
{;
    break;}
case 154:
#line 692 "yacc.yy"
{;
    break;}
case 155:
#line 695 "yacc.yy"
{;
    break;}
case 156:
#line 696 "yacc.yy"
{;
    break;}
case 157:
#line 700 "yacc.yy"
{
	        yyval._str = yyvsp[-1]._str;
	  ;
    break;}
case 158:
#line 704 "yacc.yy"
{
		yyval._str = yyvsp[-3]._str;
	  ;
    break;}
case 159:
#line 708 "yacc.yy"
{
		yyval._str = yyvsp[-1]._str;
	  ;
    break;}
case 160:
#line 712 "yacc.yy"
{
	      /* The constructor */
	      assert(!dcop_area);
              yyval._str = new QString("");
	  ;
    break;}
case 161:
#line 718 "yacc.yy"
{
	      /* The constructor */
	      assert(!dcop_area);
              yyval._str = new QString("");
	  ;
    break;}
case 162:
#line 724 "yacc.yy"
{
	      /* The destructor */
  	      assert(!dcop_area);
              yyval._str = new QString("");
	  ;
    break;}
case 163:
#line 730 "yacc.yy"
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
#line 742 "yacc.yy"
{
		function_mode = 1;
	;
    break;}
case 168:
#line 752 "yacc.yy"
{;
    break;}
case 169:
#line 753 "yacc.yy"
{;
    break;}
case 173:
#line 762 "yacc.yy"
{;
    break;}
case 174:
#line 763 "yacc.yy"
{;
    break;}
case 175:
#line 764 "yacc.yy"
{;
    break;}
case 176:
#line 766 "yacc.yy"
{;
    break;}
case 177:
#line 769 "yacc.yy"
{;
    break;}
case 178:
#line 770 "yacc.yy"
{;
    break;}
case 179:
#line 771 "yacc.yy"
{;
    break;}
case 180:
#line 772 "yacc.yy"
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
#line 774 "yacc.yy"


void dcopidlParse( const char *_code )
{
    dcopidlInitFlex( _code );
    yyparse();
}
