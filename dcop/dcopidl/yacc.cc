
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



#define	YYFINAL		321
#define	YYFLAG		-32768
#define	YYNTBASE	63

#define YYTRANSLATE(x) ((unsigned)(x) <= 316 ? yytranslate[x] : 113)

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
   159,   162,   165,   168,   171,   177,   182,   187,   194,   202,
   209,   216,   220,   222,   226,   228,   230,   232,   235,   237,
   239,   241,   245,   253,   263,   270,   271,   273,   275,   277,
   280,   283,   287,   290,   293,   297,   300,   302,   304,   306,
   308,   311,   314,   317,   319,   320,   322,   326,   328,   330,
   333,   336,   341,   348,   352,   354,   358,   362,   365,   368,
   370,   373,   377,   379,   383,   386,   388,   389,   392,   398,
   400,   402,   404,   406,   411,   412,   414,   416,   418,   420,
   422,   429,   437,   439,   441,   445,   450,   452,   456,   459,
   465,   469,   475,   483,   490,   494,   496,   498,   502,   507,
   510,   511,   513,   516,   517,   520,   524,   530
};

static const short yyrhs[] = {    64,
    66,    63,     0,     0,     8,    64,     0,    38,    11,    63,
    13,     0,     0,    55,     0,     0,     9,    74,    78,    65,
    80,    16,     0,     9,    74,    16,     0,    10,    74,    16,
     0,    10,    74,    11,    63,    13,    16,     0,     0,    41,
     5,    11,    67,    63,    13,    79,     0,    42,    41,     5,
    16,     0,    42,     5,    49,     5,    16,     0,    37,    16,
     0,    28,    93,    74,    16,     0,    28,    10,    11,    68,
    13,    74,    16,     0,    28,    10,    74,    11,    68,    13,
    74,    16,     0,    23,   105,     0,   105,     0,   112,     0,
    81,     0,   112,    68,     0,     0,    45,     0,    46,     0,
    20,     0,    18,     0,    17,     0,    26,     0,    27,     0,
     0,    70,    71,    15,     0,    71,    15,     0,    56,    15,
     0,     5,     0,     5,    49,    74,     0,    74,     0,    98,
    17,    75,     0,    75,     0,    76,    11,     0,    76,    31,
    77,     0,    15,    77,     0,    11,     0,     0,    16,     0,
    13,     0,    86,    80,     0,   105,    80,     0,    81,    80,
     0,    73,    80,     0,    72,    80,     0,   112,    80,     0,
    24,     9,    74,    16,    80,     0,    24,    74,    16,    80,
     0,     9,    74,    16,    80,     0,    42,     5,    49,     5,
    16,    80,     0,    40,     5,    11,    82,    13,     5,    16,
     0,    40,     5,    11,    82,    13,    16,     0,    40,    11,
    82,    13,     5,    16,     0,    83,    31,    82,     0,    83,
     0,     5,    48,    85,     0,     5,     0,     3,     0,     6,
     0,    30,     6,     0,    50,     0,    74,     0,    84,     0,
    84,    29,    84,     0,    28,    74,    34,    94,    35,    74,
    16,     0,    28,    74,    34,    94,    35,    49,     5,    74,
    16,     0,    10,    74,    11,    63,    13,    16,     0,     0,
    22,     0,    57,     0,    58,     0,    57,    60,     0,    57,
    59,     0,    57,    60,    51,     0,    58,    60,     0,    58,
    59,     0,    58,    60,    51,     0,    58,    51,     0,    51,
     0,    59,     0,    60,     0,    54,     0,    57,    54,     0,
    58,    54,     0,    32,    89,     0,    32,     0,     0,    95,
     0,    90,    31,    95,     0,    88,     0,    74,     0,    10,
    74,     0,     9,    74,     0,    74,    34,    92,    35,     0,
    74,    34,    92,    35,    49,    74,     0,    91,    31,    92,
     0,    91,     0,    22,    91,    89,     0,    22,    91,    36,
     0,    22,    91,     0,    91,    36,     0,    91,     0,    91,
    89,     0,    93,    31,    94,     0,    93,     0,    93,    74,
    96,     0,    93,    96,     0,    44,     0,     0,    48,    97,
     0,    48,    12,    93,    14,    97,     0,     7,     0,    85,
     0,     4,     0,    69,     0,    74,    12,    90,    14,     0,
     0,    21,     0,    62,     0,    35,     0,    34,     0,    48,
     0,    93,    74,    12,    90,    14,    87,     0,    93,    61,
    99,    12,    90,    14,    87,     0,    84,     0,   101,     0,
   101,    31,   102,     0,     5,    12,   102,    14,     0,   103,
     0,   103,    31,   104,     0,   100,   107,     0,    21,   100,
    48,    50,   107,     0,    21,   100,   107,     0,    74,    12,
    90,    14,   107,     0,    74,    12,    90,    14,    15,   104,
   107,     0,    98,    33,    74,    12,    14,   107,     0,    47,
   100,   107,     0,    11,     0,    16,     0,   106,   108,    13,
     0,   106,   108,    13,    16,     0,   109,   108,     0,     0,
    16,     0,    31,   111,     0,     0,     5,   110,     0,    93,
   111,    16,     0,    47,    93,     5,    96,    16,     0,    93,
     5,    52,    85,    53,    16,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
   150,   153,   156,   160,   163,   169,   170,   173,   178,   181,
   184,   187,   191,   199,   202,   205,   208,   211,   214,   217,
   220,   223,   226,   231,   232,   234,   234,   236,   236,   236,
   238,   238,   238,   241,   245,   251,   257,   260,   267,   275,
   279,   286,   290,   298,   302,   309,   312,   316,   320,   324,
   328,   332,   336,   340,   344,   348,   352,   356,   362,   363,
   364,   367,   368,   371,   372,   375,   376,   377,   378,   379,
   382,   383,   386,   396,   401,   406,   410,   417,   418,   419,
   420,   421,   422,   423,   424,   425,   426,   427,   428,   429,
   430,   431,   434,   435,   438,   442,   443,   450,   451,   452,
   453,   454,   460,   469,   473,   478,   483,   490,   495,   500,
   505,   513,   517,   523,   532,   540,   548,   551,   554,   560,
   563,   566,   569,   572,   578,   579,   582,   582,   582,   582,
   585,   610,   617,   620,   621,   624,   627,   628,   631,   635,
   639,   643,   649,   655,   661,   668,   674,   675,   676,   679,
   680,   683,   686,   687,   689,   692,   693,   694
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
"asterisks","params","type_name","type_name_list","type","type_list","param",
"default","default_value","virtual_qualifier","operator","function_header","argument",
"arguments","init_item","init_list","function","function_begin","function_body",
"function_lines","function_line","Identifier_list_rest","Identifier_list","member", NULL
};
#endif

static const short yyr1[] = {     0,
    63,    63,    64,    64,    64,    65,    65,    66,    66,    66,
    66,    67,    66,    66,    66,    66,    66,    66,    66,    66,
    66,    66,    66,    68,    68,    69,    69,    70,    70,    70,
    71,    71,    71,    72,    72,    73,    74,    74,    75,    76,
    76,    77,    77,    78,    78,    79,    79,    80,    80,    80,
    80,    80,    80,    80,    80,    80,    80,    80,    81,    81,
    81,    82,    82,    83,    83,    84,    84,    84,    84,    84,
    85,    85,    86,    86,    86,    87,    87,    88,    88,    88,
    88,    88,    88,    88,    88,    88,    88,    88,    88,    88,
    88,    88,    89,    89,    90,    90,    90,    91,    91,    91,
    91,    91,    91,    92,    92,    93,    93,    93,    93,    93,
    93,    94,    94,    95,    95,    95,    96,    96,    96,    97,
    97,    97,    97,    97,    98,    98,    99,    99,    99,    99,
   100,   100,   101,   102,   102,   103,   104,   104,   105,   105,
   105,   105,   105,   105,   105,   106,   107,   107,   107,   108,
   108,   109,   110,   110,   111,   112,   112,   112
};

static const short yyr2[] = {     0,
     3,     0,     2,     4,     0,     1,     0,     6,     3,     3,
     6,     0,     7,     4,     5,     2,     4,     7,     8,     2,
     1,     1,     1,     2,     0,     1,     1,     1,     1,     1,
     1,     1,     0,     3,     2,     2,     1,     3,     1,     3,
     1,     2,     3,     2,     1,     0,     1,     1,     2,     2,
     2,     2,     2,     2,     5,     4,     4,     6,     7,     6,
     6,     3,     1,     3,     1,     1,     1,     2,     1,     1,
     1,     3,     7,     9,     6,     0,     1,     1,     1,     2,
     2,     3,     2,     2,     3,     2,     1,     1,     1,     1,
     2,     2,     2,     1,     0,     1,     3,     1,     1,     2,
     2,     4,     6,     3,     1,     3,     3,     2,     2,     1,
     2,     3,     1,     3,     2,     1,     0,     2,     5,     1,
     1,     1,     1,     4,     0,     1,     1,     1,     1,     1,
     6,     7,     1,     1,     3,     4,     1,     3,     2,     5,
     3,     5,     7,     6,     3,     1,     1,     3,     4,     2,
     0,     1,     2,     0,     2,     3,     5,     6
};

static const short yydefact[] = {     5,
     5,     0,   125,     3,     5,    37,     0,     0,   126,     0,
   125,     0,     0,     0,     0,     0,     0,    87,    90,    78,
    79,    88,    89,     5,    99,    23,    98,   110,     0,     0,
     0,    21,    22,     0,     0,   101,   100,     0,     0,    99,
     0,     0,   108,     0,    20,     0,     0,    16,     0,     0,
     0,     0,     0,     0,     0,    91,    81,    80,    86,    92,
    84,    83,     1,    95,     0,    94,   109,   111,    37,     0,
     0,     0,     0,   146,   147,   151,   139,     4,    38,    45,
   125,     9,     7,     5,    10,   101,   100,     0,   141,   107,
   106,    25,   100,     0,     0,    65,     0,    63,    12,     0,
     0,    37,   145,    82,    85,   116,     0,   117,    96,   105,
     0,    93,     0,     0,   155,   129,   128,   130,   127,     0,
    95,   156,     0,   152,     0,   151,   126,    39,    41,     0,
    44,     0,     6,    33,     0,     0,     0,     0,     0,    25,
    25,    17,     0,     0,     0,     0,     5,     0,    14,     0,
     0,     0,     0,   117,   115,     0,   102,   154,   153,    66,
    67,     0,    69,    70,    71,     0,    95,     0,     0,   148,
   150,    42,   125,     0,     0,     0,    48,    30,    29,    28,
     0,    31,    32,     0,     0,     0,    33,     0,    33,    33,
     0,    33,    33,    33,    33,     0,   140,     0,     0,   154,
    24,     0,     0,    64,     0,    62,     0,    15,   122,   120,
     0,    26,    27,   123,    70,   121,   118,   157,     0,   142,
    97,   114,   104,     0,    68,     0,     0,     0,    76,     0,
   149,    43,    40,   101,   100,     0,     0,     0,     0,    36,
     0,    35,    53,    52,     8,    51,    49,    50,    54,    11,
   117,     0,     0,     0,    60,    61,    46,     0,    95,     0,
   137,     0,   103,    72,   158,    76,    77,   131,   144,    33,
     5,     0,    33,     0,     0,    34,    18,     0,    59,    47,
    13,     0,     0,     0,     0,   143,   132,    57,     0,    33,
    56,   113,     0,     0,    19,   119,   124,   133,   134,     0,
   138,     0,    55,     0,     0,    33,     0,   136,    75,   112,
     0,     0,    58,   135,     0,    73,     0,    74,     0,     0,
     0
};

static const short yydefgoto[] = {    34,
     3,   134,    24,   147,   138,   214,   187,   188,   189,   190,
    40,   129,   130,   131,    83,   281,   191,   192,    97,    98,
   165,   216,   193,   268,    27,    68,   107,    28,   111,    29,
   293,   109,   151,   217,    30,   120,    31,   299,   300,   261,
   262,   194,    76,    77,   125,   126,   115,    72,   195
};

static const short yypact[] = {    31,
    72,    75,   268,-32768,   138,   -26,    23,    23,   308,   346,
    55,   328,   103,    36,   194,    40,   308,-32768,-32768,   150,
   183,-32768,-32768,    21,    28,-32768,-32768,   161,     7,   121,
    26,-32768,-32768,   193,    23,   168,   201,    23,    23,   134,
    11,    68,   196,   308,-32768,    88,    23,-32768,   225,   209,
   233,   192,   243,    13,    26,-32768,-32768,   200,-32768,-32768,
-32768,   202,-32768,    78,   346,   228,-32768,-32768,    76,   140,
   250,   249,    23,-32768,-32768,   252,-32768,-32768,-32768,-32768,
   177,-32768,   211,   138,-32768,-32768,-32768,   219,-32768,-32768,
-32768,   289,   260,   258,   209,   227,   267,   251,-32768,   278,
   269,     8,-32768,-32768,-32768,-32768,    77,    25,-32768,   253,
   257,-32768,   281,   115,-32768,-32768,-32768,-32768,-32768,   276,
    78,-32768,   283,-32768,   280,   252,-32768,-32768,-32768,    64,
-32768,   284,-32768,   198,   287,    26,   308,   294,   292,   289,
   289,-32768,   301,   115,   311,   209,   138,   304,-32768,   155,
   305,   248,    78,   275,-32768,   346,   282,   293,-32768,-32768,
-32768,   323,-32768,-32768,   303,   286,    78,   113,   320,   319,
-32768,-32768,   177,    23,    23,    23,-32768,-32768,-32768,-32768,
   230,-32768,-32768,    23,   336,   327,   203,   329,   198,   198,
   337,   198,   198,   198,   198,   338,-32768,   340,    23,    51,
-32768,   339,    17,-32768,   341,-32768,   345,-32768,-32768,-32768,
   308,-32768,-32768,-32768,   348,-32768,-32768,-32768,   356,-32768,
-32768,-32768,-32768,    23,-32768,   115,   347,   149,   342,    26,
-32768,-32768,-32768,   358,   364,    23,   360,   343,   331,-32768,
   363,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
   275,   365,    23,   367,-32768,-32768,   368,   375,    78,   378,
   361,    26,-32768,-32768,-32768,   342,-32768,-32768,-32768,   198,
   138,   377,   198,   308,   386,-32768,-32768,   379,-32768,-32768,
-32768,   366,   163,   115,   356,-32768,-32768,-32768,   381,   198,
-32768,   370,   372,   382,-32768,-32768,-32768,-32768,   371,   385,
-32768,   392,-32768,   308,    22,   198,   115,-32768,-32768,-32768,
   404,   394,-32768,-32768,    23,-32768,   397,-32768,   414,   415,
-32768
};

static const short yypgoto[] = {     1,
   416,-32768,-32768,-32768,   106,-32768,-32768,   231,-32768,-32768,
    -3,   245,-32768,   247,-32768,-32768,  -140,   418,   -85,-32768,
  -217,   -18,-32768,   156,-32768,    91,  -119,    -7,   270,     2,
   119,   271,   -91,   143,   -75,-32768,    80,-32768,   120,-32768,
   144,   153,-32768,   -35,   302,-32768,-32768,   317,    12
};


#define	YYLAST		430


static const short yytable[] = {    25,
   319,   168,    43,    36,    37,   132,    89,    25,   264,   143,
    41,    69,    41,    47,    33,     6,   155,   102,    54,   103,
    -2,   254,    35,  -117,    63,    71,     6,     6,     1,     6,
    -2,    79,   255,    -2,    86,    87,    74,    71,     1,    64,
    49,    75,    93,    94,    52,    41,    50,   228,   243,   244,
    71,   246,   247,   248,   249,   150,    35,   110,     2,     6,
   206,    65,   222,    38,    39,   108,   298,    70,     2,   123,
   311,    70,   150,    70,   172,     9,    10,   128,    74,     1,
    53,   113,     6,    75,   135,     5,    38,    39,    42,   298,
   152,  -154,     6,   139,   173,   166,    55,   132,    92,    10,
   197,    44,   114,   140,   154,    18,   113,   153,    19,     2,
   164,    20,    21,    22,    23,    88,   220,   160,    48,     6,
   161,   106,   108,    55,    35,   204,   229,   114,    18,   288,
    25,    19,   291,    91,    20,    21,    22,    23,   198,   283,
   164,   139,   139,   153,   162,     1,   215,   207,   110,   303,
    -2,   140,   140,    73,   108,    32,   112,   160,   209,     6,
   161,   210,   266,    45,   163,   313,   211,    65,   108,   128,
   128,   234,   235,   116,   117,     2,   297,   237,    80,   153,
   238,     6,    81,    82,   162,    25,    25,   118,    25,    25,
    25,    25,    66,   153,   269,   252,    67,   127,    51,   212,
   213,   119,     6,    56,   163,    78,   175,   176,    57,    58,
   177,    84,   258,    96,   178,   179,    85,   180,     9,    10,
   263,   181,   164,   182,   183,   184,   286,    66,   182,   183,
  -125,    90,   272,    59,     6,    95,    60,    14,   236,   185,
   100,    61,    62,    99,    17,   201,   202,   101,    18,   278,
   104,    19,   105,   186,    20,    21,    22,    23,    74,    66,
   108,   121,   219,    75,   122,   133,    25,   124,   136,    25,
   141,   289,     6,   142,   144,   292,     7,     8,   215,   145,
   164,   146,   148,   156,   149,   158,    25,   167,     9,    10,
    11,   157,   170,     6,   169,    12,   200,    38,    39,   196,
   174,   312,    25,   164,    13,   292,   199,    14,    15,    16,
    10,   317,     6,   203,    17,   205,    38,    39,    18,   208,
   218,    19,   150,   113,    20,    21,    22,    23,   225,    10,
   224,   226,     6,   230,   231,   137,    38,    46,   227,    18,
   239,   240,    19,   242,   251,    20,    21,    22,    23,    10,
     6,   253,   245,   250,    38,    39,   256,   257,    18,   259,
   260,    19,   265,   267,    20,    21,    22,    23,   160,   209,
     6,   161,   210,   270,   271,   273,   274,   276,    18,   275,
   277,    19,   279,   280,    20,    21,    22,    23,   282,   284,
   294,   285,   290,   302,   295,   162,    18,   306,   308,    19,
   304,   307,    20,    21,    22,    23,   305,   309,   315,   316,
   212,   213,   318,   320,   321,   163,     4,   241,   233,   232,
    26,   287,   310,   221,   296,   223,   314,   171,   301,   159
};

static const short yycheck[] = {     3,
     0,   121,    10,     7,     8,    81,    42,    11,   226,    95,
     9,     5,    11,    12,     3,     5,   108,     5,    17,    55,
     0,     5,    49,    16,    24,    29,     5,     5,     8,     5,
     0,    35,    16,    13,    38,    39,    11,    41,     8,    12,
     5,    16,    46,    47,     5,    44,    11,   167,   189,   190,
    54,   192,   193,   194,   195,    48,    49,    65,    38,     5,
   146,    34,   154,     9,    10,    64,   284,    61,    38,    73,
    49,    61,    48,    61,    11,    21,    22,    81,    11,     8,
    41,    31,     5,    16,    84,    11,     9,    10,     9,   307,
    14,    16,     5,    92,    31,   114,    17,   173,    11,    22,
   136,    47,    52,    92,   108,    51,    31,    31,    54,    38,
   114,    57,    58,    59,    60,    48,   152,     3,    16,     5,
     6,    44,   121,    44,    49,   144,    14,    52,    51,   270,
   134,    54,   273,    43,    57,    58,    59,    60,   137,   259,
   144,   140,   141,    31,    30,     8,   150,   147,   156,   290,
    13,   140,   141,    33,   153,     3,    66,     3,     4,     5,
     6,     7,    14,    11,    50,   306,    12,    34,   167,   173,
   174,   175,   176,    34,    35,    38,    14,   181,    11,    31,
   184,     5,    15,    16,    30,   189,   190,    48,   192,   193,
   194,   195,    32,    31,   230,   199,    36,    21,     5,    45,
    46,    62,     5,    54,    50,    13,     9,    10,    59,    60,
    13,    11,   211,     5,    17,    18,    16,    20,    21,    22,
   224,    24,   226,    26,    27,    28,   262,    32,    26,    27,
    33,    36,   236,    51,     5,    11,    54,    40,     9,    42,
    49,    59,    60,    11,    47,   140,   141,     5,    51,   253,
    51,    54,    51,    56,    57,    58,    59,    60,    11,    32,
   259,    12,    15,    16,    16,    55,   270,    16,    50,   273,
    11,   271,     5,    16,    48,   274,     9,    10,   282,    13,
   284,    31,     5,    31,    16,     5,   290,    12,    21,    22,
    23,    35,    13,     5,    12,    28,     5,     9,    10,    13,
    17,   305,   306,   307,    37,   304,    13,    40,    41,    42,
    22,   315,     5,    13,    47,     5,     9,    10,    51,    16,
    16,    54,    48,    31,    57,    58,    59,    60,     6,    22,
    49,    29,     5,    14,    16,    47,     9,    10,    53,    51,
     5,    15,    54,    15,     5,    57,    58,    59,    60,    22,
     5,    13,    16,    16,     9,    10,    16,    13,    51,    12,
     5,    54,    16,    22,    57,    58,    59,    60,     3,     4,
     5,     6,     7,    16,    11,    16,    34,    15,    51,    49,
    16,    54,    16,    16,    57,    58,    59,    60,    14,    12,
     5,    31,    16,    13,    16,    30,    51,    16,    14,    54,
    31,    31,    57,    58,    59,    60,    35,    16,     5,    16,
    45,    46,    16,     0,     0,    50,     1,   187,   174,   173,
     3,   266,   304,   153,   282,   156,   307,   126,   285,   113
};
/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
#line 3 "/usr/lib/bison.simple"
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

#line 217 "/usr/lib/bison.simple"

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
#line 151 "yacc.yy"
{
	  ;
    break;}
case 3:
#line 157 "yacc.yy"
{
		printf("<INCLUDE>%s</INCLUDE>\n", yyvsp[-1]._str->latin1() );
	  ;
    break;}
case 4:
#line 161 "yacc.yy"
{
	  ;
    break;}
case 5:
#line 164 "yacc.yy"
{
          ;
    break;}
case 6:
#line 169 "yacc.yy"
{ yyval._int = 1; ;
    break;}
case 7:
#line 170 "yacc.yy"
{ yyval._int = 0; ;
    break;}
case 8:
#line 174 "yacc.yy"
{
	 	if (yyvsp[-2]._int)
			  printf("<CLASS>\n    <NAME>%s</NAME>\n%s%s</CLASS>\n", ( in_namespace + *yyvsp[-4]._str ).latin1(), yyvsp[-3]._str->latin1(), yyvsp[-1]._str->latin1() );
	  ;
    break;}
case 9:
#line 179 "yacc.yy"
{
	  ;
    break;}
case 10:
#line 182 "yacc.yy"
{
	  ;
    break;}
case 11:
#line 185 "yacc.yy"
{
	  ;
    break;}
case 12:
#line 188 "yacc.yy"
{
                      in_namespace += *yyvsp[-1]._str; in_namespace += "::";
                  ;
    break;}
case 13:
#line 192 "yacc.yy"
{
                      int pos = in_namespace.findRev( "::", -3 );
                      if( pos >= 0 )
                          in_namespace = in_namespace.left( pos + 2 );
                      else
                          in_namespace = "";
                  ;
    break;}
case 14:
#line 200 "yacc.yy"
{
          ;
    break;}
case 15:
#line 203 "yacc.yy"
{
          ;
    break;}
case 16:
#line 206 "yacc.yy"
{
	  ;
    break;}
case 17:
#line 209 "yacc.yy"
{
	  ;
    break;}
case 18:
#line 212 "yacc.yy"
{
	  ;
    break;}
case 19:
#line 215 "yacc.yy"
{
	  ;
    break;}
case 20:
#line 218 "yacc.yy"
{
	  ;
    break;}
case 21:
#line 221 "yacc.yy"
{
	  ;
    break;}
case 22:
#line 224 "yacc.yy"
{
	  ;
    break;}
case 23:
#line 227 "yacc.yy"
{
	  ;
    break;}
case 34:
#line 242 "yacc.yy"
{
	  dcop_area = 0;
	;
    break;}
case 35:
#line 246 "yacc.yy"
{
	  dcop_area = 0;
	;
    break;}
case 36:
#line 252 "yacc.yy"
{
	  dcop_area = 1;
	;
    break;}
case 37:
#line 257 "yacc.yy"
{
	  yyval._str = yyvsp[0]._str;
	;
    break;}
case 38:
#line 260 "yacc.yy"
{
	   QString* tmp = new QString( "%1::%2" );
           *tmp = tmp->arg(*(yyvsp[-2]._str)).arg(*(yyvsp[0]._str));
           yyval._str = tmp;
	;
    break;}
case 39:
#line 268 "yacc.yy"
{
		QString* tmp = new QString( "    <SUPER>%1</SUPER>\n" );
		*tmp = tmp->arg( *(yyvsp[0]._str) );
		yyval._str = tmp;
	  ;
    break;}
case 40:
#line 276 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 41:
#line 280 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 42:
#line 287 "yacc.yy"
{
		yyval._str = yyvsp[-1]._str;
	  ;
    break;}
case 43:
#line 291 "yacc.yy"
{
		/* $$ = $1; */
		yyval._str = new QString( *(yyvsp[-2]._str) + *(yyvsp[0]._str) );
	  ;
    break;}
case 44:
#line 299 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 45:
#line 303 "yacc.yy"
{
		yyval._str = new QString( "" );
	  ;
    break;}
case 46:
#line 310 "yacc.yy"
{
          ;
    break;}
case 48:
#line 317 "yacc.yy"
{
		yyval._str = new QString( "" );
	  ;
    break;}
case 49:
#line 321 "yacc.yy"
{
		yyval._str = new QString( *(yyvsp[-1]._str) + *(yyvsp[0]._str) );
	  ;
    break;}
case 50:
#line 325 "yacc.yy"
{
		yyval._str = new QString( *(yyvsp[-1]._str) + *(yyvsp[0]._str) );
	  ;
    break;}
case 51:
#line 329 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 52:
#line 333 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 53:
#line 337 "yacc.yy"
{	
	        yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 54:
#line 341 "yacc.yy"
{
 	        yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 55:
#line 345 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 56:
#line 349 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 57:
#line 353 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 58:
#line 357 "yacc.yy"
{
                yyval._str = yyvsp[0]._str;
          ;
    break;}
case 64:
#line 371 "yacc.yy"
{;
    break;}
case 65:
#line 372 "yacc.yy"
{;
    break;}
case 66:
#line 375 "yacc.yy"
{;
    break;}
case 67:
#line 376 "yacc.yy"
{;
    break;}
case 68:
#line 377 "yacc.yy"
{;
    break;}
case 69:
#line 378 "yacc.yy"
{;
    break;}
case 70:
#line 379 "yacc.yy"
{;
    break;}
case 71:
#line 382 "yacc.yy"
{;
    break;}
case 72:
#line 383 "yacc.yy"
{;
    break;}
case 73:
#line 387 "yacc.yy"
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
case 74:
#line 397 "yacc.yy"
{
		if (dcop_area)
		  yyerror("scoped template typedefs are not supported in dcop areas!");
	  ;
    break;}
case 75:
#line 402 "yacc.yy"
{
	  ;
    break;}
case 76:
#line 407 "yacc.yy"
{
		yyval._int = 0;
	  ;
    break;}
case 77:
#line 411 "yacc.yy"
{
		yyval._int = 1;
	  ;
    break;}
case 78:
#line 417 "yacc.yy"
{ yyval._str = new QString("signed int"); ;
    break;}
case 79:
#line 418 "yacc.yy"
{ yyval._str = new QString("unsigned int"); ;
    break;}
case 80:
#line 419 "yacc.yy"
{ yyval._str = new QString("signed short int"); ;
    break;}
case 81:
#line 420 "yacc.yy"
{ yyval._str = new QString("signed long int"); ;
    break;}
case 82:
#line 421 "yacc.yy"
{ yyval._str = new QString("signed short int"); ;
    break;}
case 83:
#line 422 "yacc.yy"
{ yyval._str = new QString("unsigned short int"); ;
    break;}
case 84:
#line 423 "yacc.yy"
{ yyval._str = new QString("unsigned long int"); ;
    break;}
case 85:
#line 424 "yacc.yy"
{ yyval._str = new QString("unsigned short int"); ;
    break;}
case 86:
#line 425 "yacc.yy"
{ yyval._str = new QString("unsigned int"); ;
    break;}
case 87:
#line 426 "yacc.yy"
{ yyval._str = new QString("int"); ;
    break;}
case 88:
#line 427 "yacc.yy"
{ yyval._str = new QString("long int"); ;
    break;}
case 89:
#line 428 "yacc.yy"
{ yyval._str = new QString("short int"); ;
    break;}
case 90:
#line 429 "yacc.yy"
{ yyval._str = new QString("char"); ;
    break;}
case 91:
#line 430 "yacc.yy"
{ yyval._str = new QString("signed char"); ;
    break;}
case 92:
#line 431 "yacc.yy"
{ yyval._str = new QString("unsigned char"); ;
    break;}
case 95:
#line 439 "yacc.yy"
{
		yyval._str = new QString( "" );
	  ;
    break;}
case 97:
#line 444 "yacc.yy"
{
		yyval._str = new QString( *(yyvsp[-2]._str) + *(yyvsp[0]._str) );
	  ;
    break;}
case 99:
#line 451 "yacc.yy"
{ yyval._str = yyvsp[0]._str; ;
    break;}
case 100:
#line 452 "yacc.yy"
{ yyval._str = yyvsp[0]._str; ;
    break;}
case 101:
#line 453 "yacc.yy"
{ yyval._str = yyvsp[0]._str; ;
    break;}
case 102:
#line 454 "yacc.yy"
{
		QString *tmp = new QString("%1&lt;%2&gt;");
		*tmp = tmp->arg(*(yyvsp[-3]._str));
		*tmp = tmp->arg(*(yyvsp[-1]._str));
		yyval._str = tmp;
	 ;
    break;}
case 103:
#line 460 "yacc.yy"
{
		QString *tmp = new QString("%1&lt;%2&gt;::%3");
		*tmp = tmp->arg(*(yyvsp[-5]._str));
		*tmp = tmp->arg(*(yyvsp[-3]._str));
		*tmp = tmp->arg(*(yyvsp[0]._str));
		yyval._str = tmp;
	 ;
    break;}
case 104:
#line 470 "yacc.yy"
{
	    yyval._str = new QString(*(yyvsp[-2]._str) + "," + *(yyvsp[0]._str));
	  ;
    break;}
case 105:
#line 474 "yacc.yy"
{
 	    yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 106:
#line 479 "yacc.yy"
{
	    if (dcop_area)
	      yyerror("in dcop areas are no pointers allowed");
	  ;
    break;}
case 107:
#line 483 "yacc.yy"
{
	     if (dcop_area) {
	  	QString* tmp = new QString("<TYPE  qleft=\"const\" qright=\"" AMP_ENTITY "\">%1</TYPE>");
		*tmp = tmp->arg( *(yyvsp[-1]._str) );
		yyval._str = tmp;
	     }
	  ;
    break;}
case 108:
#line 490 "yacc.yy"
{
		QString* tmp = new QString("<TYPE>%1</TYPE>");
		*tmp = tmp->arg( *(yyvsp[0]._str) );
		yyval._str = tmp;
	;
    break;}
case 109:
#line 495 "yacc.yy"
{
	     if (dcop_area)
		yyerror("in dcop areas are only const references allowed!");
	  ;
    break;}
case 110:
#line 500 "yacc.yy"
{
		QString* tmp = new QString("<TYPE>%1</TYPE>");
		*tmp = tmp->arg( *(yyvsp[0]._str) );
		yyval._str = tmp;
	;
    break;}
case 111:
#line 506 "yacc.yy"
{
	    if (dcop_area)
	      yyerror("in dcop areas are no pointers allowed");
	  ;
    break;}
case 112:
#line 514 "yacc.yy"
{
	    yyval._str = new QString(*(yyvsp[-2]._str) + "," + *(yyvsp[0]._str));
	  ;
    break;}
case 113:
#line 518 "yacc.yy"
{
 	    yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 114:
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
case 115:
#line 533 "yacc.yy"
{
		if (dcop_area) {
		   QString* tmp = new QString("\n        <ARG>%1</ARG>");
  		   *tmp = tmp->arg( *(yyvsp[-1]._str) );
		   yyval._str = tmp;		
		} else yyval._str = new QString();
	  ;
    break;}
case 116:
#line 541 "yacc.yy"
{
		if (dcop_area)
			yyerror("variable arguments not supported in dcop area.");
		yyval._str = new QString("");
	  ;
    break;}
case 117:
#line 549 "yacc.yy"
{
	  ;
    break;}
case 118:
#line 552 "yacc.yy"
{
	  ;
    break;}
case 119:
#line 555 "yacc.yy"
{
	  ;
    break;}
case 120:
#line 561 "yacc.yy"
{
          ;
    break;}
case 121:
#line 564 "yacc.yy"
{
          ;
    break;}
case 122:
#line 567 "yacc.yy"
{
          ;
    break;}
case 123:
#line 570 "yacc.yy"
{
          ;
    break;}
case 124:
#line 573 "yacc.yy"
{
          ;
    break;}
case 125:
#line 578 "yacc.yy"
{ yyval._int = 0; ;
    break;}
case 126:
#line 579 "yacc.yy"
{ yyval._int = 1; ;
    break;}
case 131:
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
case 132:
#line 611 "yacc.yy"
{
	     if (dcop_area)
		yyerror("operators aren't allowed in dcop areas!");
	     yyval._str = new QString("");
	  ;
    break;}
case 133:
#line 617 "yacc.yy"
{;
    break;}
case 134:
#line 620 "yacc.yy"
{;
    break;}
case 135:
#line 621 "yacc.yy"
{;
    break;}
case 136:
#line 624 "yacc.yy"
{;
    break;}
case 137:
#line 627 "yacc.yy"
{;
    break;}
case 138:
#line 628 "yacc.yy"
{;
    break;}
case 139:
#line 632 "yacc.yy"
{
	        yyval._str = yyvsp[-1]._str;
	  ;
    break;}
case 140:
#line 636 "yacc.yy"
{
		yyval._str = yyvsp[-3]._str;
	  ;
    break;}
case 141:
#line 640 "yacc.yy"
{
		yyval._str = yyvsp[-1]._str;
	  ;
    break;}
case 142:
#line 644 "yacc.yy"
{
	      /* The constructor */
	      assert(!dcop_area);
              yyval._str = new QString("");
	  ;
    break;}
case 143:
#line 650 "yacc.yy"
{
	      /* The constructor */
	      assert(!dcop_area);
              yyval._str = new QString("");
	  ;
    break;}
case 144:
#line 656 "yacc.yy"
{
	      /* The destructor */
  	      assert(!dcop_area);
              yyval._str = new QString("");
	  ;
    break;}
case 145:
#line 662 "yacc.yy"
{
		if (dcop_area)
		  yyerror("static is not allowed in dcop area!");
		yyval._str = new QString();
	  ;
    break;}
case 146:
#line 669 "yacc.yy"
{
		function_mode = 1;
	;
    break;}
case 150:
#line 679 "yacc.yy"
{;
    break;}
case 151:
#line 680 "yacc.yy"
{;
    break;}
case 155:
#line 689 "yacc.yy"
{;
    break;}
case 156:
#line 692 "yacc.yy"
{;
    break;}
case 157:
#line 693 "yacc.yy"
{;
    break;}
case 158:
#line 694 "yacc.yy"
{;
    break;}
}
   /* the action file gets copied in in place of this dollarsign */
#line 543 "/usr/lib/bison.simple"

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
