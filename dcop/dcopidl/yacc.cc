
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
#define	T_SIGNED	311
#define	T_UNSIGNED	312
#define	T_LONG	313
#define	T_SHORT	314
#define	T_FUNOPERATOR	315
#define	T_MISCOPERATOR	316

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

static QString in_namespace( "" );

void dcopidlInitFlex( const char *_code );

void yyerror( const char *s )
{
	qDebug( "In line %i : %s", idl_line_no, s );
        exit(1);
	//   theParser->parse_error( idl_lexFile, s, idl_line_no );
}


#line 57 "yacc.yy"
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



#define	YYFINAL		324
#define	YYFLAG		-32768
#define	YYNTBASE	63

#define YYTRANSLATE(x) ((unsigned)(x) <= 316 ? yytranslate[x] : 112)

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
    57,    58,    59,    60,    61,    62
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     4,     5,     8,    13,    14,    16,    17,    24,    28,
    32,    39,    40,    48,    53,    59,    62,    67,    75,    84,
    87,    89,    91,    93,    96,    97,    99,   101,   103,   105,
   107,   109,   111,   112,   116,   119,   122,   124,   128,   130,
   134,   136,   139,   143,   146,   148,   149,   151,   153,   156,
   159,   162,   165,   168,   171,   177,   182,   187,   192,   197,
   204,   212,   219,   226,   232,   236,   238,   242,   244,   246,
   248,   251,   253,   255,   257,   261,   269,   279,   286,   287,
   289,   291,   293,   296,   299,   303,   306,   309,   313,   316,
   318,   320,   322,   324,   327,   330,   333,   335,   336,   338,
   342,   344,   346,   349,   352,   357,   364,   368,   372,   375,
   378,   381,   383,   387,   389,   393,   396,   398,   399,   402,
   408,   410,   412,   414,   416,   421,   422,   424,   426,   428,
   430,   432,   439,   447,   449,   451,   455,   460,   462,   466,
   469,   475,   479,   485,   493,   500,   504,   506,   508,   512,
   517,   520,   521,   523,   526,   527,   530,   534,   540
};

static const short yyrhs[] = {    64,
    66,    63,     0,     0,     8,    64,     0,    38,    11,    63,
    13,     0,     0,    55,     0,     0,     9,    74,    78,    65,
    80,    16,     0,     9,    74,    16,     0,    10,    74,    16,
     0,    10,    74,    11,    63,    13,    16,     0,     0,    41,
     5,    11,    67,    63,    13,    79,     0,    42,    41,     5,
    16,     0,    42,     5,    49,     5,    16,     0,    37,    16,
     0,    28,    92,    74,    16,     0,    28,    10,    11,    68,
    13,    74,    16,     0,    28,    10,    74,    11,    68,    13,
    74,    16,     0,    23,   104,     0,   104,     0,   111,     0,
    81,     0,   111,    68,     0,     0,    45,     0,    46,     0,
    20,     0,    18,     0,    17,     0,    26,     0,    27,     0,
     0,    70,    71,    15,     0,    71,    15,     0,    56,    15,
     0,     5,     0,     5,    49,    74,     0,    74,     0,    97,
    17,    75,     0,    75,     0,    76,    11,     0,    76,    31,
    77,     0,    15,    77,     0,    11,     0,     0,    16,     0,
    13,     0,    86,    80,     0,   104,    80,     0,    81,    80,
     0,    73,    80,     0,    72,    80,     0,   111,    80,     0,
    24,     9,    74,    16,    80,     0,    24,    74,    16,    80,
     0,    24,    99,    16,    80,     0,     9,    74,    16,    80,
     0,    10,    74,    16,    80,     0,    42,     5,    49,     5,
    16,    80,     0,    40,     5,    11,    82,    13,     5,    16,
     0,    40,     5,    11,    82,    13,    16,     0,    40,    11,
    82,    13,     5,    16,     0,    40,    11,    82,    13,    16,
     0,    83,    31,    82,     0,    83,     0,     5,    48,    85,
     0,     5,     0,     3,     0,     6,     0,    30,     6,     0,
    50,     0,    74,     0,    84,     0,    84,    29,    84,     0,
    28,    74,    34,    93,    35,    74,    16,     0,    28,    74,
    34,    93,    35,    49,     5,    74,    16,     0,    10,    74,
    11,    63,    13,    16,     0,     0,    22,     0,    57,     0,
    58,     0,    57,    60,     0,    57,    59,     0,    57,    60,
    51,     0,    58,    60,     0,    58,    59,     0,    58,    60,
    51,     0,    58,    51,     0,    51,     0,    59,     0,    60,
     0,    54,     0,    57,    54,     0,    58,    54,     0,    32,
    89,     0,    32,     0,     0,    94,     0,    90,    31,    94,
     0,    88,     0,    74,     0,    10,    74,     0,     9,    74,
     0,    74,    34,    93,    35,     0,    74,    34,    93,    35,
    49,    74,     0,    22,    91,    89,     0,    22,    91,    36,
     0,    22,    91,     0,    91,    36,     0,    91,    89,     0,
    91,     0,    92,    31,    93,     0,    92,     0,    92,    74,
    95,     0,    92,    95,     0,    44,     0,     0,    48,    96,
     0,    48,    12,    92,    14,    96,     0,     7,     0,    85,
     0,     4,     0,    69,     0,    74,    12,    90,    14,     0,
     0,    21,     0,    62,     0,    35,     0,    34,     0,    48,
     0,    92,    74,    12,    90,    14,    87,     0,    92,    61,
    98,    12,    90,    14,    87,     0,    84,     0,   100,     0,
   100,    31,   101,     0,     5,    12,   101,    14,     0,   102,
     0,   102,    31,   103,     0,    99,   106,     0,    21,    99,
    48,    50,   106,     0,    21,    99,   106,     0,    74,    12,
    90,    14,   106,     0,    74,    12,    90,    14,    15,   103,
   106,     0,    97,    33,    74,    12,    14,   106,     0,    47,
    99,   106,     0,    11,     0,    16,     0,   105,   107,    13,
     0,   105,   107,    13,    16,     0,   108,   107,     0,     0,
    16,     0,    31,   110,     0,     0,     5,   109,     0,    92,
   110,    16,     0,    47,    92,     5,    95,    16,     0,    92,
     5,    52,    85,    53,    16,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
   149,   152,   155,   159,   162,   168,   169,   172,   177,   180,
   183,   186,   190,   198,   201,   204,   207,   210,   213,   216,
   219,   222,   225,   230,   231,   233,   233,   235,   235,   235,
   237,   237,   237,   240,   244,   250,   256,   259,   266,   274,
   278,   285,   289,   297,   301,   308,   311,   315,   319,   323,
   327,   331,   335,   339,   343,   347,   351,   355,   359,   363,
   369,   370,   371,   372,   375,   376,   379,   380,   383,   384,
   385,   386,   387,   390,   391,   394,   404,   409,   414,   418,
   425,   426,   427,   428,   429,   430,   431,   432,   433,   434,
   435,   436,   437,   438,   439,   442,   443,   447,   451,   452,
   459,   460,   461,   462,   463,   469,   478,   483,   490,   495,
   500,   505,   513,   517,   523,   532,   540,   548,   551,   554,
   560,   563,   566,   569,   572,   578,   579,   582,   582,   582,
   582,   585,   610,   617,   620,   621,   624,   627,   628,   631,
   635,   639,   643,   649,   655,   661,   668,   674,   675,   676,
   679,   680,   683,   686,   687,   689,   692,   693,   694
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
"T_DCOP_AREA","T_SIGNED","T_UNSIGNED","T_LONG","T_SHORT","T_FUNOPERATOR","T_MISCOPERATOR",
"main","includes","dcoptag","declaration","@1","member_list","bool_value","nodcop_area",
"sigslot","nodcop_area_begin","dcop_area_begin","Identifier","super_class_name",
"super_class","super_classes","class_header","opt_semicolon","body","enum","enum_list",
"enum_item","number","int_expression","typedef","const_qualifier","int_type",
"asterisks","params","type_name","type","type_list","param","default","default_value",
"virtual_qualifier","operator","function_header","argument","arguments","init_item",
"init_list","function","function_begin","function_body","function_lines","function_line",
"Identifier_list_rest","Identifier_list","member", NULL
};
#endif

static const short yyr1[] = {     0,
    63,    63,    64,    64,    64,    65,    65,    66,    66,    66,
    66,    67,    66,    66,    66,    66,    66,    66,    66,    66,
    66,    66,    66,    68,    68,    69,    69,    70,    70,    70,
    71,    71,    71,    72,    72,    73,    74,    74,    75,    76,
    76,    77,    77,    78,    78,    79,    79,    80,    80,    80,
    80,    80,    80,    80,    80,    80,    80,    80,    80,    80,
    81,    81,    81,    81,    82,    82,    83,    83,    84,    84,
    84,    84,    84,    85,    85,    86,    86,    86,    87,    87,
    88,    88,    88,    88,    88,    88,    88,    88,    88,    88,
    88,    88,    88,    88,    88,    89,    89,    90,    90,    90,
    91,    91,    91,    91,    91,    91,    92,    92,    92,    92,
    92,    92,    93,    93,    94,    94,    94,    95,    95,    95,
    96,    96,    96,    96,    96,    97,    97,    98,    98,    98,
    98,    99,    99,   100,   101,   101,   102,   103,   103,   104,
   104,   104,   104,   104,   104,   104,   105,   106,   106,   106,
   107,   107,   108,   109,   109,   110,   111,   111,   111
};

static const short yyr2[] = {     0,
     3,     0,     2,     4,     0,     1,     0,     6,     3,     3,
     6,     0,     7,     4,     5,     2,     4,     7,     8,     2,
     1,     1,     1,     2,     0,     1,     1,     1,     1,     1,
     1,     1,     0,     3,     2,     2,     1,     3,     1,     3,
     1,     2,     3,     2,     1,     0,     1,     1,     2,     2,
     2,     2,     2,     2,     5,     4,     4,     4,     4,     6,
     7,     6,     6,     5,     3,     1,     3,     1,     1,     1,
     2,     1,     1,     1,     3,     7,     9,     6,     0,     1,
     1,     1,     2,     2,     3,     2,     2,     3,     2,     1,
     1,     1,     1,     2,     2,     2,     1,     0,     1,     3,
     1,     1,     2,     2,     4,     6,     3,     3,     2,     2,
     2,     1,     3,     1,     3,     2,     1,     0,     2,     5,
     1,     1,     1,     1,     4,     0,     1,     1,     1,     1,
     1,     6,     7,     1,     1,     3,     4,     1,     3,     2,
     5,     3,     5,     7,     6,     3,     1,     1,     3,     4,
     2,     0,     1,     2,     0,     2,     3,     5,     6
};

static const short yydefact[] = {     5,
     5,     0,   126,     3,     5,    37,     0,     0,   127,     0,
   126,     0,     0,     0,     0,     0,     0,    90,    93,    81,
    82,    91,    92,     5,   102,    23,   101,   112,     0,     0,
     0,    21,    22,     0,     0,   104,   103,     0,     0,   102,
     0,     0,   109,     0,    20,     0,     0,    16,     0,     0,
     0,     0,     0,     0,     0,    94,    84,    83,    89,    95,
    87,    86,     1,    98,     0,    97,   110,   111,    37,     0,
     0,     0,     0,   147,   148,   152,   140,     4,    38,    45,
   126,     9,     7,     5,    10,   104,   103,     0,   142,   108,
   107,    25,   103,     0,     0,    68,     0,    66,    12,     0,
     0,    37,   146,    85,    88,   117,     0,   118,    99,   114,
     0,    96,     0,     0,   156,   130,   129,   131,   128,     0,
    98,   157,     0,   153,     0,   152,   127,    39,    41,     0,
    44,     0,     6,    33,     0,     0,     0,     0,     0,    25,
    25,    17,     0,     0,     0,     0,     5,     0,    14,     0,
     0,     0,     0,   118,   116,     0,   105,   155,   154,    69,
    70,     0,    72,    73,    74,     0,    98,     0,     0,   149,
   151,    42,   126,     0,     0,     0,    48,    30,    29,    28,
     0,    31,    32,     0,     0,     0,    33,     0,    33,    33,
     0,    33,    33,    33,    33,     0,   141,     0,     0,   155,
    24,     0,     0,    67,     0,    64,    65,     0,    15,   123,
   121,     0,    26,    27,   124,    73,   122,   119,   158,     0,
   143,   100,   115,   113,     0,    71,     0,     0,     0,    79,
     0,   150,    43,    40,   104,   103,     0,   102,     0,     0,
     0,    36,     0,    35,    53,    52,     8,    51,    49,    50,
    54,    11,   118,     0,     0,     0,    62,    63,    46,     0,
    98,     0,   138,     0,   106,    75,   159,    79,    80,   132,
   145,    33,     5,    33,   104,    33,    33,     0,     0,    34,
    18,     0,    61,    47,    13,     0,     0,     0,     0,   144,
   133,    58,     0,    59,    33,    56,    57,     0,     0,    19,
   120,   125,   134,   135,     0,   139,     0,    55,     0,    33,
     0,   137,    78,     0,     0,    60,   136,     0,    76,     0,
    77,     0,     0,     0
};

static const short yydefgoto[] = {    34,
     3,   134,    24,   147,   138,   215,   187,   188,   189,   190,
    40,   129,   130,   131,    83,   285,   191,   192,    97,    98,
   165,   217,   193,   270,    27,    68,   107,    28,    29,   111,
   109,   151,   218,    30,   120,    31,   304,   305,   263,   264,
   194,    76,    77,   125,   126,   115,    72,   195
};

static const short yypact[] = {    45,
    66,    39,   272,-32768,    87,   104,   172,   172,   352,    55,
   312,   368,   160,   125,   189,    51,   352,-32768,-32768,   108,
   106,-32768,-32768,    21,    90,-32768,-32768,   171,    10,   151,
   140,-32768,-32768,   197,   172,   184,   164,   172,   172,   168,
    11,    38,   179,   352,-32768,   174,   172,-32768,   202,   213,
   217,   180,   225,    12,   140,-32768,-32768,   182,-32768,-32768,
-32768,   185,-32768,   294,   352,   203,-32768,-32768,     9,    92,
   226,   221,   172,-32768,-32768,   224,-32768,-32768,-32768,-32768,
   112,-32768,   187,    87,-32768,-32768,-32768,   193,-32768,-32768,
-32768,   334,   233,   229,   213,   200,   234,   218,-32768,   246,
   238,    41,-32768,-32768,-32768,-32768,    77,    15,-32768,   230,
   227,-32768,   255,    25,-32768,-32768,-32768,-32768,-32768,   252,
   294,-32768,   253,-32768,   254,   224,-32768,-32768,-32768,    31,
-32768,   249,-32768,   199,   257,   140,   352,   259,   263,   334,
   334,-32768,   262,    25,    94,   213,    87,   268,-32768,   411,
   270,   190,   294,   231,-32768,   352,   239,   247,-32768,-32768,
-32768,   281,-32768,-32768,   260,   237,   294,   107,   277,   280,
-32768,-32768,   112,   172,   172,   172,-32768,-32768,-32768,-32768,
   391,-32768,-32768,   172,   292,   283,   102,   286,   199,   199,
   289,   199,   199,   199,   199,   295,-32768,   297,   172,    70,
-32768,   305,   132,-32768,   304,-32768,-32768,   311,-32768,-32768,
-32768,   352,-32768,-32768,-32768,   298,-32768,-32768,-32768,   320,
-32768,-32768,-32768,-32768,   172,-32768,    25,   319,   118,   306,
   140,-32768,-32768,-32768,   321,   177,   172,    82,   324,   293,
   287,-32768,   326,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,   231,   330,   172,   331,-32768,-32768,   333,   328,
   294,   338,   327,   140,-32768,-32768,-32768,   306,-32768,-32768,
-32768,   199,    87,   199,   339,   199,   199,   352,   355,-32768,
-32768,   348,-32768,-32768,-32768,   428,   121,    25,   320,-32768,
-32768,-32768,   354,-32768,   199,-32768,-32768,   340,   349,-32768,
-32768,-32768,-32768,   337,   362,-32768,   363,-32768,    19,   199,
    25,-32768,-32768,   375,   366,-32768,-32768,   172,-32768,   367,
-32768,   384,   386,-32768
};

static const short yypgoto[] = {     3,
   388,-32768,-32768,-32768,   -93,-32768,-32768,   208,-32768,-32768,
    -3,   223,-32768,   214,-32768,-32768,  -113,   395,   -77,-32768,
  -204,   -26,-32768,   131,-32768,    54,  -115,   392,     2,  -144,
   251,   -69,   119,   -80,-32768,    -7,-32768,    96,-32768,   135,
    30,-32768,   -33,   282,-32768,-32768,   307,     4
};


#define	YYLAST		478


static const short yytable[] = {    25,
   132,    42,   322,    36,    37,   168,    33,    25,    89,    55,
    41,   224,    41,    47,    69,     6,   102,   143,    54,     6,
    -2,   103,   266,     6,  -155,    71,    63,   160,     1,     6,
   161,    79,    32,    -2,    86,    87,    55,    71,   155,   113,
    45,   172,    93,    94,    -2,    41,   201,   202,    74,     5,
    71,   229,     1,    75,   162,    52,  -118,    35,     2,     6,
   114,   173,   150,    38,    39,   108,   110,   314,   207,   123,
    70,    70,    70,     1,   163,   245,   246,   128,   248,   249,
   250,   251,     2,   303,   223,    88,   135,   166,   150,    35,
   152,    53,   132,   139,     1,   140,    91,   276,   205,    -2,
   113,    64,   197,     2,   154,    18,   303,   153,    19,   206,
   164,    20,    21,    22,    23,    65,     6,   204,   221,   112,
   230,   114,   108,    65,     2,   116,   117,   182,   183,    49,
    25,   268,   127,   298,   302,    50,   256,   153,   198,   118,
   164,   139,   139,   140,   140,   287,   216,   257,   153,   208,
    74,   153,    35,   119,   108,    75,    59,   110,   292,    60,
   294,    56,   296,   297,    61,    62,    57,    58,   108,   128,
   128,   235,   236,   239,    84,    48,     6,   238,     6,    85,
   240,   308,    41,    73,    92,    25,    25,   273,    25,    25,
    25,    25,   274,    51,    80,   254,   316,   271,    81,    82,
    74,    65,    66,     6,   220,    75,    67,   175,   176,    78,
    66,   177,    95,   260,    90,   178,   179,    96,   180,     9,
    10,   265,   181,   164,   182,   183,   184,    99,   100,   101,
   290,  -126,   104,   275,    66,   105,   122,   121,    14,   124,
   185,   133,   136,   141,   142,    17,   145,   144,   146,    18,
   148,   282,    19,   149,   186,    20,    21,    22,    23,   158,
   156,   157,   108,   167,   169,   174,   170,   200,    25,   196,
    25,   199,    25,    25,   203,   293,     6,   113,   150,   110,
     7,     8,   216,   209,   164,   219,   226,   225,   227,   228,
   231,    25,     9,    10,    11,   232,   241,   242,     6,    12,
   244,   253,    38,    39,   247,   315,    25,   164,    13,   261,
   252,    14,    15,    16,   320,    10,     6,   255,    17,   258,
    38,    39,    18,   259,   262,    19,   278,   269,    20,    21,
    22,    23,     9,    10,   267,   279,   272,   106,     6,   277,
   280,   286,    38,    39,    18,   281,   283,    19,   284,   288,
    20,    21,    22,    23,   295,    10,     6,   289,    44,   299,
    38,    39,    18,   300,   310,    19,   307,   311,    20,    21,
    22,    23,     6,    10,   309,   312,    38,    46,   313,   318,
   137,   319,   321,   323,    18,   324,   233,    19,     4,    10,
    20,    21,    22,    23,   243,     6,   234,    26,   291,   237,
    39,    43,    18,   222,   301,    19,   317,   171,    20,    21,
    22,    23,    10,   160,   210,     6,   161,   211,    18,   159,
     0,    19,   212,   306,    20,    21,    22,    23,     0,     0,
   160,   210,     6,   161,   211,     0,     0,     0,     0,     0,
   162,    18,     0,     0,    19,     0,     0,    20,    21,    22,
    23,     0,     0,     0,     0,   213,   214,   162,     0,     0,
   163,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,   213,   214,     0,     0,     0,   163
};

static const short yycheck[] = {     3,
    81,     9,     0,     7,     8,   121,     3,    11,    42,    17,
     9,   156,    11,    12,     5,     5,     5,    95,    17,     5,
     0,    55,   227,     5,    16,    29,    24,     3,     8,     5,
     6,    35,     3,    13,    38,    39,    44,    41,   108,    31,
    11,    11,    46,    47,     0,    44,   140,   141,    11,    11,
    54,   167,     8,    16,    30,     5,    16,    49,    38,     5,
    52,    31,    48,     9,    10,    64,    65,    49,   146,    73,
    61,    61,    61,     8,    50,   189,   190,    81,   192,   193,
   194,   195,    38,   288,   154,    48,    84,   114,    48,    49,
    14,    41,   173,    92,     8,    92,    43,    16,     5,    13,
    31,    12,   136,    38,   108,    51,   311,    31,    54,    16,
   114,    57,    58,    59,    60,    34,     5,   144,   152,    66,
    14,    52,   121,    34,    38,    34,    35,    26,    27,     5,
   134,    14,    21,   278,    14,    11,     5,    31,   137,    48,
   144,   140,   141,   140,   141,   261,   150,    16,    31,   147,
    11,    31,    49,    62,   153,    16,    51,   156,   272,    54,
   274,    54,   276,   277,    59,    60,    59,    60,   167,   173,
   174,   175,   176,   181,    11,    16,     5,   181,     5,    16,
   184,   295,   181,    33,    11,   189,   190,    11,   192,   193,
   194,   195,    16,     5,    11,   199,   310,   231,    15,    16,
    11,    34,    32,     5,    15,    16,    36,     9,    10,    13,
    32,    13,    11,   212,    36,    17,    18,     5,    20,    21,
    22,   225,    24,   227,    26,    27,    28,    11,    49,     5,
   264,    33,    51,   237,    32,    51,    16,    12,    40,    16,
    42,    55,    50,    11,    16,    47,    13,    48,    31,    51,
     5,   255,    54,    16,    56,    57,    58,    59,    60,     5,
    31,    35,   261,    12,    12,    17,    13,     5,   272,    13,
   274,    13,   276,   277,    13,   273,     5,    31,    48,   278,
     9,    10,   286,    16,   288,    16,     6,    49,    29,    53,
    14,   295,    21,    22,    23,    16,     5,    15,     5,    28,
    15,     5,     9,    10,    16,   309,   310,   311,    37,    12,
    16,    40,    41,    42,   318,    22,     5,    13,    47,    16,
     9,    10,    51,    13,     5,    54,    34,    22,    57,    58,
    59,    60,    21,    22,    16,    49,    16,    44,     5,    16,
    15,    14,     9,    10,    51,    16,    16,    54,    16,    12,
    57,    58,    59,    60,    16,    22,     5,    31,    47,     5,
     9,    10,    51,    16,    16,    54,    13,    31,    57,    58,
    59,    60,     5,    22,    35,    14,     9,    10,    16,     5,
    47,    16,    16,     0,    51,     0,   173,    54,     1,    22,
    57,    58,    59,    60,   187,     5,   174,     3,   268,     9,
    10,    10,    51,   153,   286,    54,   311,   126,    57,    58,
    59,    60,    22,     3,     4,     5,     6,     7,    51,   113,
    -1,    54,    12,   289,    57,    58,    59,    60,    -1,    -1,
     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,
    30,    51,    -1,    -1,    54,    -1,    -1,    57,    58,    59,
    60,    -1,    -1,    -1,    -1,    45,    46,    30,    -1,    -1,
    50,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    45,    46,    -1,    -1,    -1,    50
};
/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
#line 3 "//usr/lib/bison.simple"
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

#line 217 "//usr/lib/bison.simple"

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
#line 150 "yacc.yy"
{
	  ;
    break;}
case 3:
#line 156 "yacc.yy"
{
		printf("<INCLUDE>%s</INCLUDE>\n", yyvsp[-1]._str->latin1() );
	  ;
    break;}
case 4:
#line 160 "yacc.yy"
{
	  ;
    break;}
case 5:
#line 163 "yacc.yy"
{
          ;
    break;}
case 6:
#line 168 "yacc.yy"
{ yyval._int = 1; ;
    break;}
case 7:
#line 169 "yacc.yy"
{ yyval._int = 0; ;
    break;}
case 8:
#line 173 "yacc.yy"
{
	 	if (yyvsp[-2]._int)
			  printf("<CLASS>\n    <NAME>%s</NAME>\n%s%s</CLASS>\n", ( in_namespace + *yyvsp[-4]._str ).latin1(), yyvsp[-3]._str->latin1(), yyvsp[-1]._str->latin1() );
	  ;
    break;}
case 9:
#line 178 "yacc.yy"
{
	  ;
    break;}
case 10:
#line 181 "yacc.yy"
{
	  ;
    break;}
case 11:
#line 184 "yacc.yy"
{
	  ;
    break;}
case 12:
#line 187 "yacc.yy"
{
                      in_namespace += *yyvsp[-1]._str; in_namespace += "::";
                  ;
    break;}
case 13:
#line 191 "yacc.yy"
{
                      int pos = in_namespace.findRev( "::", -3 );
                      if( pos >= 0 )
                          in_namespace = in_namespace.left( pos + 2 );
                      else
                          in_namespace = "";
                  ;
    break;}
case 14:
#line 199 "yacc.yy"
{
          ;
    break;}
case 15:
#line 202 "yacc.yy"
{
          ;
    break;}
case 16:
#line 205 "yacc.yy"
{
	  ;
    break;}
case 17:
#line 208 "yacc.yy"
{
	  ;
    break;}
case 18:
#line 211 "yacc.yy"
{
	  ;
    break;}
case 19:
#line 214 "yacc.yy"
{
	  ;
    break;}
case 20:
#line 217 "yacc.yy"
{
	  ;
    break;}
case 21:
#line 220 "yacc.yy"
{
	  ;
    break;}
case 22:
#line 223 "yacc.yy"
{
	  ;
    break;}
case 23:
#line 226 "yacc.yy"
{
	  ;
    break;}
case 34:
#line 241 "yacc.yy"
{
	  dcop_area = 0;
	;
    break;}
case 35:
#line 245 "yacc.yy"
{
	  dcop_area = 0;
	;
    break;}
case 36:
#line 251 "yacc.yy"
{
	  dcop_area = 1;
	;
    break;}
case 37:
#line 256 "yacc.yy"
{
	  yyval._str = yyvsp[0]._str;
	;
    break;}
case 38:
#line 259 "yacc.yy"
{
	   QString* tmp = new QString( "%1::%2" );
           *tmp = tmp->arg(*(yyvsp[-2]._str)).arg(*(yyvsp[0]._str));
           yyval._str = tmp;
	;
    break;}
case 39:
#line 267 "yacc.yy"
{
		QString* tmp = new QString( "    <SUPER>%1</SUPER>\n" );
		*tmp = tmp->arg( *(yyvsp[0]._str) );
		yyval._str = tmp;
	  ;
    break;}
case 40:
#line 275 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 41:
#line 279 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 42:
#line 286 "yacc.yy"
{
		yyval._str = yyvsp[-1]._str;
	  ;
    break;}
case 43:
#line 290 "yacc.yy"
{
		/* $$ = $1; */
		yyval._str = new QString( *(yyvsp[-2]._str) + *(yyvsp[0]._str) );
	  ;
    break;}
case 44:
#line 298 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 45:
#line 302 "yacc.yy"
{
		yyval._str = new QString( "" );
	  ;
    break;}
case 46:
#line 309 "yacc.yy"
{
          ;
    break;}
case 48:
#line 316 "yacc.yy"
{
		yyval._str = new QString( "" );
	  ;
    break;}
case 49:
#line 320 "yacc.yy"
{
		yyval._str = new QString( *(yyvsp[-1]._str) + *(yyvsp[0]._str) );
	  ;
    break;}
case 50:
#line 324 "yacc.yy"
{
		yyval._str = new QString( *(yyvsp[-1]._str) + *(yyvsp[0]._str) );
	  ;
    break;}
case 51:
#line 328 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 52:
#line 332 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 53:
#line 336 "yacc.yy"
{	
	        yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 54:
#line 340 "yacc.yy"
{
 	        yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 55:
#line 344 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 56:
#line 348 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 57:
#line 352 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 58:
#line 356 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 59:
#line 360 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 60:
#line 364 "yacc.yy"
{
                yyval._str = yyvsp[0]._str;
          ;
    break;}
case 67:
#line 379 "yacc.yy"
{;
    break;}
case 68:
#line 380 "yacc.yy"
{;
    break;}
case 69:
#line 383 "yacc.yy"
{;
    break;}
case 70:
#line 384 "yacc.yy"
{;
    break;}
case 71:
#line 385 "yacc.yy"
{;
    break;}
case 72:
#line 386 "yacc.yy"
{;
    break;}
case 73:
#line 387 "yacc.yy"
{;
    break;}
case 74:
#line 390 "yacc.yy"
{;
    break;}
case 75:
#line 391 "yacc.yy"
{;
    break;}
case 76:
#line 395 "yacc.yy"
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
case 77:
#line 405 "yacc.yy"
{
		if (dcop_area)
		  yyerror("scoped template typedefs are not supported in dcop areas!");
	  ;
    break;}
case 78:
#line 410 "yacc.yy"
{
	  ;
    break;}
case 79:
#line 415 "yacc.yy"
{
		yyval._int = 0;
	  ;
    break;}
case 80:
#line 419 "yacc.yy"
{
		yyval._int = 1;
	  ;
    break;}
case 81:
#line 425 "yacc.yy"
{ yyval._str = new QString("signed int"); ;
    break;}
case 82:
#line 426 "yacc.yy"
{ yyval._str = new QString("unsigned int"); ;
    break;}
case 83:
#line 427 "yacc.yy"
{ yyval._str = new QString("signed short int"); ;
    break;}
case 84:
#line 428 "yacc.yy"
{ yyval._str = new QString("signed long int"); ;
    break;}
case 85:
#line 429 "yacc.yy"
{ yyval._str = new QString("signed short int"); ;
    break;}
case 86:
#line 430 "yacc.yy"
{ yyval._str = new QString("unsigned short int"); ;
    break;}
case 87:
#line 431 "yacc.yy"
{ yyval._str = new QString("unsigned long int"); ;
    break;}
case 88:
#line 432 "yacc.yy"
{ yyval._str = new QString("unsigned short int"); ;
    break;}
case 89:
#line 433 "yacc.yy"
{ yyval._str = new QString("unsigned int"); ;
    break;}
case 90:
#line 434 "yacc.yy"
{ yyval._str = new QString("int"); ;
    break;}
case 91:
#line 435 "yacc.yy"
{ yyval._str = new QString("long int"); ;
    break;}
case 92:
#line 436 "yacc.yy"
{ yyval._str = new QString("short int"); ;
    break;}
case 93:
#line 437 "yacc.yy"
{ yyval._str = new QString("char"); ;
    break;}
case 94:
#line 438 "yacc.yy"
{ yyval._str = new QString("signed char"); ;
    break;}
case 95:
#line 439 "yacc.yy"
{ yyval._str = new QString("unsigned char"); ;
    break;}
case 97:
#line 444 "yacc.yy"
{ ;
    break;}
case 98:
#line 448 "yacc.yy"
{
		yyval._str = new QString( "" );
	  ;
    break;}
case 100:
#line 453 "yacc.yy"
{
		yyval._str = new QString( *(yyvsp[-2]._str) + *(yyvsp[0]._str) );
	  ;
    break;}
case 102:
#line 460 "yacc.yy"
{ yyval._str = yyvsp[0]._str; ;
    break;}
case 103:
#line 461 "yacc.yy"
{ yyval._str = yyvsp[0]._str; ;
    break;}
case 104:
#line 462 "yacc.yy"
{ yyval._str = yyvsp[0]._str; ;
    break;}
case 105:
#line 463 "yacc.yy"
{
		QString *tmp = new QString("%1&lt;%2&gt;");
		*tmp = tmp->arg(*(yyvsp[-3]._str));
		*tmp = tmp->arg(*(yyvsp[-1]._str));
		yyval._str = tmp;
	 ;
    break;}
case 106:
#line 469 "yacc.yy"
{
		QString *tmp = new QString("%1&lt;%2&gt;::%3");
		*tmp = tmp->arg(*(yyvsp[-5]._str));
		*tmp = tmp->arg(*(yyvsp[-3]._str));
		*tmp = tmp->arg(*(yyvsp[0]._str));
		yyval._str = tmp;
	 ;
    break;}
case 107:
#line 479 "yacc.yy"
{
	    if (dcop_area)
	      yyerror("in dcop areas are no pointers allowed");
	  ;
    break;}
case 108:
#line 483 "yacc.yy"
{
	     if (dcop_area) {
	  	QString* tmp = new QString("<TYPE  qleft=\"const\" qright=\"" AMP_ENTITY "\">%1</TYPE>");
		*tmp = tmp->arg( *(yyvsp[-1]._str) );
		yyval._str = tmp;
	     }
	  ;
    break;}
case 109:
#line 490 "yacc.yy"
{
		QString* tmp = new QString("<TYPE>%1</TYPE>");
		*tmp = tmp->arg( *(yyvsp[0]._str) );
		yyval._str = tmp;
	;
    break;}
case 110:
#line 495 "yacc.yy"
{
	     if (dcop_area)
		yyerror("in dcop areas are only const references allowed!");
	  ;
    break;}
case 111:
#line 501 "yacc.yy"
{
	    if (dcop_area)
	      yyerror("in dcop areas are no pointers allowed");
	  ;
    break;}
case 112:
#line 505 "yacc.yy"
{
		QString* tmp = new QString("<TYPE>%1</TYPE>");
		*tmp = tmp->arg( *(yyvsp[0]._str) );
		yyval._str = tmp;
	;
    break;}
case 113:
#line 514 "yacc.yy"
{
	    yyval._str = new QString(*(yyvsp[-2]._str) + "," + *(yyvsp[0]._str));
	  ;
    break;}
case 114:
#line 518 "yacc.yy"
{
 	    yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 115:
#line 524 "yacc.yy"
{
		if (dcop_area) {
		   QString* tmp = new QString("\n        <ARG>%1<NAME>%2</NAME></ARG>");
  		   *tmp = tmp->arg( *(yyvsp[-2]._str) );
  		   *tmp = tmp->arg( *(yyvsp[-1]._str) );
		   yyval._str = tmp;		
		} else yyval._str = new QString();
	  ;
    break;}
case 116:
#line 533 "yacc.yy"
{
		if (dcop_area) {
		   QString* tmp = new QString("\n        <ARG>%1</ARG>");
  		   *tmp = tmp->arg( *(yyvsp[-1]._str) );
		   yyval._str = tmp;		
		} else yyval._str = new QString();
	  ;
    break;}
case 117:
#line 541 "yacc.yy"
{
		if (dcop_area)
			yyerror("variable arguments not supported in dcop area.");
		yyval._str = new QString("");
	  ;
    break;}
case 118:
#line 549 "yacc.yy"
{
	  ;
    break;}
case 119:
#line 552 "yacc.yy"
{
	  ;
    break;}
case 120:
#line 555 "yacc.yy"
{
	  ;
    break;}
case 121:
#line 561 "yacc.yy"
{
          ;
    break;}
case 122:
#line 564 "yacc.yy"
{
          ;
    break;}
case 123:
#line 567 "yacc.yy"
{
          ;
    break;}
case 124:
#line 570 "yacc.yy"
{
          ;
    break;}
case 125:
#line 573 "yacc.yy"
{
          ;
    break;}
case 126:
#line 578 "yacc.yy"
{ yyval._int = 0; ;
    break;}
case 127:
#line 579 "yacc.yy"
{ yyval._int = 1; ;
    break;}
case 132:
#line 586 "yacc.yy"
{
	     if (dcop_area) {
		QString* tmp = 0;
		if ( yyvsp[0]._int )
			tmp = new QString(
				"    <FUNC qual=\"const\">\n"
				"        %2\n"
				"        <NAME>%1</NAME>"
				"%3\n"
				"     </FUNC>\n");
		else
			tmp = new QString(
				"    <FUNC>\n"
				"        %2\n"
				"        <NAME>%1</NAME>"
				"%3\n"
				"     </FUNC>\n");
		*tmp = tmp->arg( *(yyvsp[-4]._str) );
		*tmp = tmp->arg( *(yyvsp[-5]._str) );
		*tmp = tmp->arg( *(yyvsp[-2]._str) );
		yyval._str = tmp;
   	     } else
	        yyval._str = new QString("");
	  ;
    break;}
case 133:
#line 611 "yacc.yy"
{
	     if (dcop_area)
		yyerror("operators aren't allowed in dcop areas!");
	     yyval._str = new QString("");
	  ;
    break;}
case 134:
#line 617 "yacc.yy"
{;
    break;}
case 135:
#line 620 "yacc.yy"
{;
    break;}
case 136:
#line 621 "yacc.yy"
{;
    break;}
case 137:
#line 624 "yacc.yy"
{;
    break;}
case 138:
#line 627 "yacc.yy"
{;
    break;}
case 139:
#line 628 "yacc.yy"
{;
    break;}
case 140:
#line 632 "yacc.yy"
{
	        yyval._str = yyvsp[-1]._str;
	  ;
    break;}
case 141:
#line 636 "yacc.yy"
{
		yyval._str = yyvsp[-3]._str;
	  ;
    break;}
case 142:
#line 640 "yacc.yy"
{
		yyval._str = yyvsp[-1]._str;
	  ;
    break;}
case 143:
#line 644 "yacc.yy"
{
	      /* The constructor */
	      assert(!dcop_area);
              yyval._str = new QString("");
	  ;
    break;}
case 144:
#line 650 "yacc.yy"
{
	      /* The constructor */
	      assert(!dcop_area);
              yyval._str = new QString("");
	  ;
    break;}
case 145:
#line 656 "yacc.yy"
{
	      /* The destructor */
  	      assert(!dcop_area);
              yyval._str = new QString("");
	  ;
    break;}
case 146:
#line 662 "yacc.yy"
{
		if (dcop_area)
		  yyerror("static is not allowed in dcop area!");
		yyval._str = new QString();
	  ;
    break;}
case 147:
#line 669 "yacc.yy"
{
		function_mode = 1;
	;
    break;}
case 151:
#line 679 "yacc.yy"
{;
    break;}
case 152:
#line 680 "yacc.yy"
{;
    break;}
case 156:
#line 689 "yacc.yy"
{;
    break;}
case 157:
#line 692 "yacc.yy"
{;
    break;}
case 158:
#line 693 "yacc.yy"
{;
    break;}
case 159:
#line 694 "yacc.yy"
{;
    break;}
}
   /* the action file gets copied in in place of this dollarsign */
#line 543 "//usr/lib/bison.simple"

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
#line 696 "yacc.yy"


void dcopidlParse( const char *_code )
{
    dcopidlInitFlex( _code );
    yyparse();
}
