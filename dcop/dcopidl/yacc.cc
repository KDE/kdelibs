
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

#include <config.h>

// Workaround for a bison issue:
// bison.simple concludes from _GNU_SOURCE that stpcpy is available,
// while GNU string.h only exposes it if __USE_GNU is set.
#ifdef _GNU_SOURCE
#define __USE_GNU 1
#endif

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

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


#line 66 "yacc.yy"
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



#define	YYFINAL		366
#define	YYFLAG		-32768
#define	YYNTBASE	66

#define YYTRANSLATE(x) ((unsigned)(x) <= 319 ? yytranslate[x] : 118)

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
     0,     4,     5,     8,    13,    14,    16,    17,    24,    32,
    36,    40,    46,    47,    55,    60,    66,    69,    74,    82,
    91,    94,    96,    98,   100,   103,   104,   106,   108,   110,
   112,   114,   116,   118,   119,   123,   126,   129,   132,   134,
   138,   140,   145,   149,   151,   154,   158,   161,   163,   164,
   166,   168,   171,   174,   177,   180,   183,   186,   189,   195,
   200,   205,   210,   217,   222,   229,   236,   244,   251,   258,
   264,   268,   270,   274,   276,   278,   280,   283,   285,   287,
   289,   293,   297,   305,   315,   316,   318,   320,   323,   325,
   328,   331,   335,   338,   342,   345,   349,   352,   356,   358,
   360,   363,   365,   368,   370,   373,   376,   379,   381,   382,
   384,   388,   390,   392,   395,   398,   403,   410,   414,   416,
   419,   421,   425,   429,   432,   435,   437,   440,   444,   446,
   450,   453,   455,   456,   459,   465,   467,   469,   471,   473,
   478,   479,   481,   483,   485,   487,   489,   491,   498,   506,
   508,   512,   513,   518,   520,   524,   527,   533,   537,   543,
   551,   558,   562,   564,   566,   570,   575,   578,   579,   581,
   584,   585,   587,   591,   594,   597,   601,   607,   613,   619
};

static const short yyrhs[] = {    67,
    69,    66,     0,     0,     8,    67,     0,    38,    11,    66,
    13,     0,     0,    56,     0,     0,     9,    78,    82,    68,
    84,    16,     0,     9,     5,    78,    82,    68,    84,    16,
     0,     9,    78,    16,     0,    10,    78,    16,     0,    10,
    78,    82,    84,    16,     0,     0,    41,     5,    11,    70,
    66,    13,    83,     0,    42,    41,     5,    16,     0,    42,
     5,    50,     5,    16,     0,    37,    16,     0,    28,    98,
    78,    16,     0,    28,    10,    11,    71,    13,    78,    16,
     0,    28,    10,    78,    11,    71,    13,    78,    16,     0,
    23,   109,     0,   109,     0,   117,     0,    85,     0,   117,
    71,     0,     0,    45,     0,    46,     0,    20,     0,    18,
     0,    17,     0,    26,     0,    27,     0,     0,    73,    74,
    15,     0,    74,    15,     0,    57,    15,     0,    58,    15,
     0,     5,     0,     5,    50,    78,     0,    78,     0,    78,
    34,    99,    35,     0,   103,    17,    79,     0,    79,     0,
    80,    11,     0,    80,    31,    81,     0,    15,    81,     0,
    11,     0,     0,    16,     0,    13,     0,    90,    84,     0,
   109,    84,     0,    77,    84,     0,    85,    84,     0,    76,
    84,     0,    75,    84,     0,   117,    84,     0,    24,     9,
    78,    16,    84,     0,    24,    78,    16,    84,     0,    24,
   105,    16,    84,     0,     9,    78,    16,    84,     0,     9,
    78,    82,    84,    16,    84,     0,    10,    78,    16,    84,
     0,    10,    78,    82,    84,    16,    84,     0,    42,     5,
    50,     5,    16,    84,     0,    40,     5,    11,    86,    13,
     5,    16,     0,    40,     5,    11,    86,    13,    16,     0,
    40,    11,    86,    13,     5,    16,     0,    40,    11,    86,
    13,    16,     0,    87,    31,    86,     0,    87,     0,     5,
    49,    89,     0,     5,     0,     3,     0,     6,     0,    30,
     6,     0,    51,     0,    78,     0,    88,     0,    88,    29,
    88,     0,    88,    65,    88,     0,    28,    78,    34,    99,
    35,    78,    16,     0,    28,    78,    34,    99,    35,    50,
     5,    78,    16,     0,     0,    22,     0,    59,     0,    59,
    52,     0,    60,     0,    60,    52,     0,    59,    62,     0,
    59,    62,    52,     0,    59,    61,     0,    59,    61,    52,
     0,    60,    62,     0,    60,    62,    52,     0,    60,    61,
     0,    60,    61,    52,     0,    52,     0,    61,     0,    61,
    52,     0,    62,     0,    62,    52,     0,    55,     0,    59,
    55,     0,    60,    55,     0,    32,    93,     0,    32,     0,
     0,   100,     0,    94,    31,   100,     0,    92,     0,    78,
     0,    10,    78,     0,     9,    78,     0,    78,    34,    96,
    35,     0,    78,    34,    96,    35,    50,    78,     0,    97,
    31,    96,     0,    97,     0,    95,    93,     0,    95,     0,
    22,    95,    93,     0,    22,    95,    36,     0,    22,    95,
     0,    95,    36,     0,    95,     0,    95,    93,     0,    98,
    31,    99,     0,    98,     0,    98,    78,   101,     0,    98,
   101,     0,    44,     0,     0,    49,   102,     0,    49,    12,
    98,    14,   102,     0,     7,     0,    89,     0,     4,     0,
    72,     0,    78,    12,    94,    14,     0,     0,    21,     0,
    64,     0,    65,     0,    35,     0,    34,     0,    49,     0,
    98,    78,    12,    94,    14,    91,     0,    98,    63,   104,
    12,    94,    14,    91,     0,   102,     0,   102,    31,   106,
     0,     0,     5,    12,   106,    14,     0,   107,     0,   107,
    31,   108,     0,   105,   111,     0,    21,   105,    49,    51,
   111,     0,    21,   105,   111,     0,    78,    12,    94,    14,
   111,     0,    78,    12,    94,    14,    15,   108,   111,     0,
   103,    33,    78,    12,    14,   111,     0,    47,   105,   111,
     0,    11,     0,    16,     0,   110,   112,    13,     0,   110,
   112,    13,    16,     0,   113,   112,     0,     0,    16,     0,
    31,   116,     0,     0,     5,     0,     5,    49,   102,     0,
    93,     5,     0,   115,   114,     0,    98,   116,    16,     0,
    98,    78,    15,     6,    16,     0,    47,    98,     5,   101,
    16,     0,    48,    98,     5,   101,    16,     0,    98,     5,
    53,    89,    54,    16,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
   163,   166,   170,   174,   177,   183,   184,   188,   197,   206,
   209,   212,   215,   219,   227,   230,   233,   236,   239,   242,
   245,   248,   251,   254,   260,   261,   264,   264,   266,   266,
   266,   268,   268,   268,   271,   276,   284,   292,   304,   307,
   315,   321,   330,   334,   341,   345,   353,   357,   364,   367,
   371,   375,   379,   383,   387,   391,   395,   399,   403,   407,
   411,   415,   419,   423,   427,   431,   438,   439,   440,   441,
   445,   446,   450,   451,   455,   456,   457,   458,   459,   463,
   464,   465,   469,   479,   487,   491,   498,   499,   500,   501,
   502,   503,   504,   505,   506,   507,   508,   509,   510,   511,
   512,   513,   514,   515,   516,   517,   521,   522,   526,   530,
   531,   539,   540,   541,   542,   543,   549,   560,   564,   572,
   577,   586,   591,   598,   603,   608,   613,   621,   625,   632,
   641,   649,   658,   661,   664,   671,   674,   677,   680,   683,
   689,   690,   694,   694,   694,   694,   694,   698,   720,   731,
   732,   733,   738,   743,   744,   748,   752,   756,   760,   766,
   772,   778,   791,   798,   799,   800,   804,   805,   809,   813,
   814,   817,   818,   819,   822,   826,   827,   828,   829,   830
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
"default","value","virtual_qualifier","operator","function_header","values",
"init_item","init_list","function","function_begin","function_body","function_lines",
"function_line","Identifier_list_rest","Identifier_list_entry","Identifier_list",
"member", NULL
};
#endif

static const short yyr1[] = {     0,
    66,    66,    67,    67,    67,    68,    68,    69,    69,    69,
    69,    69,    70,    69,    69,    69,    69,    69,    69,    69,
    69,    69,    69,    69,    71,    71,    72,    72,    73,    73,
    73,    74,    74,    74,    75,    75,    76,    77,    78,    78,
    79,    79,    80,    80,    81,    81,    82,    82,    83,    83,
    84,    84,    84,    84,    84,    84,    84,    84,    84,    84,
    84,    84,    84,    84,    84,    84,    85,    85,    85,    85,
    86,    86,    87,    87,    88,    88,    88,    88,    88,    89,
    89,    89,    90,    90,    91,    91,    92,    92,    92,    92,
    92,    92,    92,    92,    92,    92,    92,    92,    92,    92,
    92,    92,    92,    92,    92,    92,    93,    93,    94,    94,
    94,    95,    95,    95,    95,    95,    95,    96,    96,    97,
    97,    98,    98,    98,    98,    98,    98,    99,    99,   100,
   100,   100,   101,   101,   101,   102,   102,   102,   102,   102,
   103,   103,   104,   104,   104,   104,   104,   105,   105,   106,
   106,   106,   107,   108,   108,   109,   109,   109,   109,   109,
   109,   109,   110,   111,   111,   111,   112,   112,   113,   114,
   114,   115,   115,   115,   116,   117,   117,   117,   117,   117
};

static const short yyr2[] = {     0,
     3,     0,     2,     4,     0,     1,     0,     6,     7,     3,
     3,     5,     0,     7,     4,     5,     2,     4,     7,     8,
     2,     1,     1,     1,     2,     0,     1,     1,     1,     1,
     1,     1,     1,     0,     3,     2,     2,     2,     1,     3,
     1,     4,     3,     1,     2,     3,     2,     1,     0,     1,
     1,     2,     2,     2,     2,     2,     2,     2,     5,     4,
     4,     4,     6,     4,     6,     6,     7,     6,     6,     5,
     3,     1,     3,     1,     1,     1,     2,     1,     1,     1,
     3,     3,     7,     9,     0,     1,     1,     2,     1,     2,
     2,     3,     2,     3,     2,     3,     2,     3,     1,     1,
     2,     1,     2,     1,     2,     2,     2,     1,     0,     1,
     3,     1,     1,     2,     2,     4,     6,     3,     1,     2,
     1,     3,     3,     2,     2,     1,     2,     3,     1,     3,
     2,     1,     0,     2,     5,     1,     1,     1,     1,     4,
     0,     1,     1,     1,     1,     1,     1,     6,     7,     1,
     3,     0,     4,     1,     3,     2,     5,     3,     5,     7,
     6,     3,     1,     1,     3,     4,     2,     0,     1,     2,
     0,     1,     3,     2,     2,     3,     5,     5,     5,     6
};

static const short yydefact[] = {     5,
     5,     0,   141,     3,     5,    39,     0,     0,   142,     0,
   141,     0,     0,     0,     0,     0,     0,     0,    99,   104,
    87,    89,   100,   102,     5,   113,    24,   112,   126,     0,
     0,     0,    22,    23,     0,     0,    39,   115,   114,     0,
     0,   113,     0,     0,   124,     0,    21,     0,     0,    17,
     0,     0,     0,     0,     0,     0,     0,     0,    88,   105,
    93,    91,    90,   106,    97,    95,   101,   103,     1,   109,
     0,   108,   125,   127,    39,     0,     0,     0,   171,     0,
     0,   163,   164,   168,   156,     4,    40,     0,    48,   141,
    10,     7,    11,    34,   115,   114,     0,     0,   158,   123,
   122,    26,   114,     0,     0,    74,     0,    72,    13,     0,
     0,    39,   162,   133,    94,    92,    98,    96,   132,     0,
   133,   110,   121,     0,   119,   107,     0,     0,   146,   145,
   147,   143,   144,     0,   109,     0,   174,     0,   175,   176,
     0,   169,     0,   168,     7,   142,    41,    44,     0,    47,
     0,     6,    34,     0,     0,    51,    31,    30,    29,     0,
    32,    33,     0,     0,     0,     0,    34,     0,    34,    34,
    34,     0,    34,    34,    34,    34,     0,     0,     0,     0,
    26,    26,    18,     0,     0,     0,     0,     5,     0,    15,
     0,     0,     0,     0,     0,   133,   131,   120,   116,     0,
    75,   138,    76,   136,     0,    27,    28,    78,   139,    79,
    80,   137,   173,    79,     0,   109,     0,     0,   172,   170,
     0,   165,   167,    34,     0,    45,   141,     0,     0,   115,
   114,     0,   113,     0,     0,     0,    37,    38,     0,    36,
    57,    56,    54,    12,    55,    52,    53,    58,   157,     0,
     0,     0,    25,     0,     0,    73,     0,    70,    71,     0,
    16,     0,   134,   178,   179,     0,   159,   111,   130,     0,
   118,    77,   109,     0,     0,     0,     0,    85,   177,     0,
   166,     0,   129,     0,    46,    43,     8,    34,    34,    34,
    34,   115,    34,    34,     0,     0,    35,   133,     0,     0,
     0,    68,    69,    49,     0,     0,   154,     0,   117,     0,
    81,    82,   180,    85,    86,   148,   161,     9,     0,    42,
    62,     0,    64,     0,    34,    60,    61,     0,     0,    19,
     0,    67,    50,    14,     0,   152,     0,   160,   140,   149,
   128,    34,    34,    59,     0,    34,    20,   135,   150,     0,
   155,    63,    65,     0,     0,    66,   152,   153,     0,    83,
   151,     0,    84,     0,     0,     0
};

static const short yydefgoto[] = {    35,
     3,   153,    25,   188,   179,   209,   167,   168,   169,   170,
   171,    26,   148,   149,   150,    92,   334,   172,   173,   107,
   108,   211,   212,   174,   316,    28,    78,   120,    29,   124,
   125,    30,   284,   122,   192,   349,    31,   134,    32,   350,
   307,   308,   175,    84,    85,   143,   144,   139,    79,    80,
   176
};

static const short yypact[] = {    97,
   108,    27,   237,-32768,   114,    67,   131,   146,   388,   167,
   103,   402,   144,   121,   175,    47,   388,   388,-32768,-32768,
   239,   252,   130,   135,    91,    89,-32768,-32768,    25,    23,
    86,   224,-32768,-32768,   188,   146,    40,   221,   223,   146,
   146,   180,    22,    49,   218,   388,-32768,   212,   146,-32768,
   205,   236,   232,   201,   259,    30,   224,   264,-32768,-32768,
   220,   228,-32768,-32768,   231,   238,-32768,-32768,-32768,   373,
   167,   263,-32768,-32768,    84,   105,   241,   300,   284,   303,
   146,-32768,-32768,   304,-32768,-32768,-32768,   291,-32768,   136,
-32768,   265,-32768,   313,-32768,-32768,   312,   274,-32768,-32768,
-32768,   354,   321,   320,   236,   293,   330,   314,-32768,   339,
   331,    57,-32768,   299,-32768,-32768,-32768,-32768,-32768,   152,
    35,-32768,   263,   316,   318,-32768,   487,    41,-32768,-32768,
-32768,-32768,-32768,   342,   373,   350,-32768,   106,-32768,-32768,
   345,-32768,   349,   304,   265,-32768,   324,-32768,    39,-32768,
   352,-32768,   313,   146,   146,-32768,-32768,-32768,-32768,   417,
-32768,-32768,   146,   361,   362,   364,   282,   365,   313,   313,
   313,   351,   313,   313,   313,   313,   224,   388,   368,   113,
   354,   354,-32768,   371,    41,    51,   236,   114,   369,-32768,
   477,   370,   372,   246,   373,   299,-32768,-32768,   337,   167,
-32768,-32768,-32768,-32768,   383,-32768,-32768,-32768,-32768,   378,
    37,-32768,-32768,-32768,   338,   373,   171,   375,   347,-32768,
   380,   384,-32768,   313,   388,-32768,   136,   146,   387,   260,
   271,   146,   145,   389,   374,   379,-32768,-32768,   403,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,   394,
   146,   404,-32768,   391,   176,-32768,   405,-32768,-32768,   407,
-32768,   388,-32768,-32768,-32768,   418,-32768,-32768,-32768,   146,
-32768,-32768,   373,    41,    41,   414,   177,   409,-32768,   224,
-32768,   420,   406,   410,-32768,-32768,-32768,   313,   313,   313,
   313,   422,   313,   313,   388,   436,-32768,   299,   426,   146,
   428,-32768,-32768,   430,   437,   440,   424,   224,-32768,   199,
-32768,-32768,-32768,   409,-32768,-32768,-32768,-32768,   388,-32768,
-32768,   442,-32768,   443,   313,-32768,-32768,   421,   444,-32768,
   449,-32768,-32768,-32768,   487,   487,   418,-32768,-32768,-32768,
-32768,   313,   313,-32768,    46,   313,-32768,-32768,   435,   439,
-32768,-32768,-32768,   462,   452,-32768,   487,-32768,   146,-32768,
-32768,   454,-32768,   471,   473,-32768
};

static const short yypgoto[] = {    18,
   474,   329,-32768,-32768,   129,-32768,-32768,   319,-32768,-32768,
-32768,    -7,   257,-32768,   261,   -26,-32768,   -94,   484,   -84,
-32768,    42,   -91,-32768,   181,-32768,   -14,  -129,    -2,   296,
-32768,     8,  -247,   302,  -107,  -123,   -78,-32768,     7,   141,
-32768,   162,   204,-32768,   -35,   356,-32768,-32768,-32768,   363,
    29
};


#define	YYLAST		538


static const short yytable[] = {    38,
    39,    42,    42,   213,    42,   217,   193,    45,    99,    42,
    42,   151,    94,   197,    74,    44,    43,   364,    43,    49,
   184,   113,    77,    57,    56,    58,     6,    75,    87,    88,
   101,    34,    95,    96,   112,    97,   215,     5,    42,     6,
   103,   104,    69,   201,     6,     6,   203,   328,    97,   226,
     6,    54,    57,    43,    72,   257,    72,   126,   229,    82,
    73,   145,    42,    42,    83,   274,   258,   263,   123,   227,
   205,   341,  -133,   141,   241,   242,   243,   121,   245,   246,
   247,   248,   147,   191,    76,    76,   277,    55,   269,    36,
    -2,   208,    76,   256,    42,   354,    -2,    98,     1,  -172,
    70,   275,   259,    -2,     1,   191,    36,     6,   198,   180,
   219,    40,    41,   196,  -172,     1,    36,    75,    81,   210,
   214,     1,    71,     9,    10,    51,    -2,    42,     2,   282,
   181,    52,   127,    36,     2,    37,   128,    72,   129,   130,
     6,   249,   121,   310,    72,     2,   230,   231,   151,    46,
     6,     2,   233,   131,    19,   235,   146,    20,   267,    50,
   293,    21,    22,    23,    24,   194,   234,    43,   132,   133,
    42,     6,   252,    42,    42,    40,    41,   214,    71,    53,
   301,    67,   195,   210,   278,   250,    68,    42,   180,   180,
   314,   302,    42,   321,   322,   323,   324,   123,   326,   327,
    86,   195,   121,   289,   291,   260,    33,   195,    42,   181,
   181,   348,   339,    71,    47,   105,     6,    42,    19,   147,
   147,    20,   102,   121,   292,    21,    22,    23,    24,   195,
   344,    89,   283,    89,    82,    90,    91,    90,    93,    83,
   106,     6,   109,   299,   317,     7,     8,   352,   353,    72,
   110,   356,   135,   100,    42,   136,    82,     9,    10,    11,
   266,    83,   309,   111,    12,    42,   214,   214,   114,   305,
    89,   115,   338,    13,    90,   288,    14,    15,    16,   116,
   121,    89,   117,    17,    18,    90,   290,    42,    19,   118,
    59,    20,   331,    60,    72,    21,    22,    23,    24,    61,
    62,    89,   283,    63,   137,    90,    64,   161,   162,   253,
   254,    42,    65,    66,   138,   311,   312,     6,   140,   142,
   152,   154,   155,   135,   177,   156,   283,   210,   210,   157,
   158,   182,   159,     9,    10,   183,   160,   355,   161,   162,
   163,   185,   186,   189,   187,  -141,   190,   191,   200,   210,
   199,   362,    14,   216,   164,   218,   221,   225,     6,    17,
    18,   222,    40,    41,    19,   236,   244,    20,   228,   165,
   166,    21,    22,    23,    24,    10,   237,     6,   238,   240,
   251,    40,    41,   255,   261,   264,   270,   265,   272,   273,
   279,   276,     6,   280,    10,   127,    40,    41,   298,   281,
   178,    18,   287,   300,   294,    19,     6,   295,    20,    10,
    40,    48,    21,    22,    23,    24,   119,   297,   136,   304,
   303,     6,   306,    10,    19,   232,    41,    20,   296,   313,
   315,    21,    22,    23,    24,   318,   319,   325,    10,    19,
   329,   330,    20,   332,   320,   333,    21,    22,    23,    24,
   335,   336,   358,    19,   337,   345,    20,   342,   343,   346,
    21,    22,    23,    24,   347,   357,   359,   360,    19,   363,
   365,    20,   366,   224,     4,    21,    22,    23,    24,   201,
   202,     6,   203,   204,   286,   239,    27,   285,   262,   201,
   202,     6,   203,   204,   340,   271,   268,   361,   351,   223,
   220,     0,     0,     0,     0,     0,   205,     0,     0,     0,
     0,     0,     0,     0,     0,     0,   205,     0,     0,     0,
     0,   206,   207,     0,     0,     0,     0,   208,     0,     0,
     0,   206,   207,     0,     0,     0,     0,   208
};

static const short yycheck[] = {     7,
     8,     9,    10,   127,    12,   135,   114,    10,    44,    17,
    18,    90,    39,   121,    29,     9,     9,     0,    11,    12,
   105,    57,    30,    17,    17,    18,     5,     5,    36,    37,
    45,     3,    40,    41,     5,    43,   128,    11,    46,     5,
    48,    49,    25,     3,     5,     5,     6,   295,    56,    11,
     5,     5,    46,    46,    32,     5,    32,    72,   153,    11,
    36,    88,    70,    71,    16,    29,    16,   191,    71,    31,
    30,   319,    16,    81,   169,   170,   171,    70,   173,   174,
   175,   176,    90,    49,    63,    63,   216,    41,   196,    50,
     0,    51,    63,   185,   102,    50,     0,    49,     8,    16,
    12,    65,   187,    13,     8,    49,    50,     5,   123,   102,
     5,     9,    10,   121,    31,     8,    50,     5,    33,   127,
   128,     8,    34,    21,    22,     5,    13,   135,    38,   224,
   102,    11,    49,    50,    38,     5,    53,    32,    34,    35,
     5,   177,   135,   273,    32,    38,   154,   155,   227,    47,
     5,    38,   160,    49,    52,   163,    21,    55,   194,    16,
    16,    59,    60,    61,    62,    14,   160,   160,    64,    65,
   178,     5,   180,   181,   182,     9,    10,   185,    34,     5,
     5,    52,    31,   191,    14,   178,    52,   195,   181,   182,
    14,    16,   200,   288,   289,   290,   291,   200,   293,   294,
    13,    31,   195,   230,   231,   188,     3,    31,   216,   181,
   182,   335,    14,    34,    11,    11,     5,   225,    52,   227,
   228,    55,    11,   216,   232,    59,    60,    61,    62,    31,
   325,    11,   225,    11,    11,    15,    16,    15,    16,    16,
     5,     5,    11,   251,   280,     9,    10,   342,   343,    32,
    50,   346,    12,    36,   262,    15,    11,    21,    22,    23,
    15,    16,   270,     5,    28,   273,   274,   275,     5,   262,
    11,    52,   308,    37,    15,    16,    40,    41,    42,    52,
   273,    11,    52,    47,    48,    15,    16,   295,    52,    52,
    52,    55,   300,    55,    32,    59,    60,    61,    62,    61,
    62,    11,   295,    52,     5,    15,    55,    26,    27,   181,
   182,   319,    61,    62,    31,   274,   275,     5,    16,    16,
    56,     9,    10,    12,    51,    13,   319,   335,   336,    17,
    18,    11,    20,    21,    22,    16,    24,   345,    26,    27,
    28,    49,    13,     5,    31,    33,    16,    49,    31,   357,
    35,   359,    40,    12,    42,     6,    12,    34,     5,    47,
    48,    13,     9,    10,    52,     5,    16,    55,    17,    57,
    58,    59,    60,    61,    62,    22,    15,     5,    15,    15,
    13,     9,    10,    13,    16,    16,    50,    16,     6,    12,
    16,    54,     5,    14,    22,    49,     9,    10,     5,    16,
    47,    48,    16,    13,    16,    52,     5,    34,    55,    22,
     9,    10,    59,    60,    61,    62,    44,    15,    15,    13,
    16,     5,     5,    22,    52,     9,    10,    55,    50,    16,
    22,    59,    60,    61,    62,    16,    31,    16,    22,    52,
     5,    16,    55,    16,    35,    16,    59,    60,    61,    62,
    14,    12,    14,    52,    31,    35,    55,    16,    16,    16,
    59,    60,    61,    62,    16,    31,     5,    16,    52,    16,
     0,    55,     0,   145,     1,    59,    60,    61,    62,     3,
     4,     5,     6,     7,   228,   167,     3,   227,    12,     3,
     4,     5,     6,     7,   314,   200,   195,   357,   337,   144,
   138,    -1,    -1,    -1,    -1,    -1,    30,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    30,    -1,    -1,    -1,
    -1,    45,    46,    -1,    -1,    -1,    -1,    51,    -1,    -1,
    -1,    45,    46,    -1,    -1,    -1,    -1,    51
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
#line 164 "yacc.yy"
{
	  ;
    break;}
case 3:
#line 171 "yacc.yy"
{
		printf("<INCLUDE>%s</INCLUDE>\n", yyvsp[-1]._str->latin1() );
	  ;
    break;}
case 4:
#line 175 "yacc.yy"
{
	  ;
    break;}
case 5:
#line 178 "yacc.yy"
{
          ;
    break;}
case 6:
#line 183 "yacc.yy"
{ yyval._int = 1; ;
    break;}
case 7:
#line 184 "yacc.yy"
{ yyval._int = 0; ;
    break;}
case 8:
#line 189 "yacc.yy"
{
	 	if (yyvsp[-2]._int)
			  printf("<CLASS>\n    <NAME>%s</NAME>\n%s%s</CLASS>\n", ( in_namespace + *yyvsp[-4]._str ).latin1(), yyvsp[-3]._str->latin1(), yyvsp[-1]._str->latin1() );
		// default C++ visibility specifier is 'private'
		dcop_area = 0;
		dcop_signal_area = 0;

	  ;
    break;}
case 9:
#line 198 "yacc.yy"
{
	 	if (yyvsp[-2]._int)
			  printf("<CLASS>\n    <NAME>%s</NAME>\n    <LINK_SCOPE>%s</LINK_SCOPE>\n%s%s</CLASS>\n", ( in_namespace + *yyvsp[-4]._str ).latin1(),yyvsp[-5]._str->latin1(),  yyvsp[-3]._str->latin1(), yyvsp[-1]._str->latin1() );
		// default C++ visibility specifier is 'private'
		dcop_area = 0;
		dcop_signal_area = 0;

	  ;
    break;}
case 10:
#line 207 "yacc.yy"
{
	  ;
    break;}
case 11:
#line 210 "yacc.yy"
{
	  ;
    break;}
case 12:
#line 213 "yacc.yy"
{
	  ;
    break;}
case 13:
#line 216 "yacc.yy"
{
                      in_namespace += *yyvsp[-1]._str; in_namespace += "::";
                  ;
    break;}
case 14:
#line 220 "yacc.yy"
{
                      int pos = in_namespace.findRev( "::", -3 );
                      if( pos >= 0 )
                          in_namespace = in_namespace.left( pos + 2 );
                      else
                          in_namespace = "";
                  ;
    break;}
case 15:
#line 228 "yacc.yy"
{
          ;
    break;}
case 16:
#line 231 "yacc.yy"
{
          ;
    break;}
case 17:
#line 234 "yacc.yy"
{
	  ;
    break;}
case 18:
#line 237 "yacc.yy"
{
	  ;
    break;}
case 19:
#line 240 "yacc.yy"
{
	  ;
    break;}
case 20:
#line 243 "yacc.yy"
{
	  ;
    break;}
case 21:
#line 246 "yacc.yy"
{
	  ;
    break;}
case 22:
#line 249 "yacc.yy"
{
	  ;
    break;}
case 23:
#line 252 "yacc.yy"
{
	  ;
    break;}
case 24:
#line 255 "yacc.yy"
{
	  ;
    break;}
case 35:
#line 272 "yacc.yy"
{
	  dcop_area = 0;
	  dcop_signal_area = 0;
	;
    break;}
case 36:
#line 277 "yacc.yy"
{
	  dcop_area = 0;
	  dcop_signal_area = 0;
	;
    break;}
case 37:
#line 285 "yacc.yy"
{
	  dcop_area = 1;
	  dcop_signal_area = 0;
	;
    break;}
case 38:
#line 293 "yacc.yy"
{
	  /*
	  A dcop signals area needs all dcop area capabilities,
	  e.g. parsing of function parameters.
	  */
	  dcop_area = 1;
	  dcop_signal_area = 1;
	;
    break;}
case 39:
#line 304 "yacc.yy"
{
	  yyval._str = yyvsp[0]._str;
	;
    break;}
case 40:
#line 307 "yacc.yy"
{
	   QString* tmp = new QString( "%1::%2" );
           *tmp = tmp->arg(*(yyvsp[-2]._str)).arg(*(yyvsp[0]._str));
           yyval._str = tmp;
	;
    break;}
case 41:
#line 316 "yacc.yy"
{
		QString* tmp = new QString( "    <SUPER>%1</SUPER>\n" );
		*tmp = tmp->arg( *(yyvsp[0]._str) );
		yyval._str = tmp;
	  ;
    break;}
case 42:
#line 322 "yacc.yy"
{
		QString* tmp = new QString( "    <SUPER>%1</SUPER>\n" );
		*tmp = tmp->arg( *(yyvsp[-3]._str) + "&lt" + *(yyvsp[-1]._str) + "&gt;" );
		yyval._str = tmp;
	  ;
    break;}
case 43:
#line 331 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 44:
#line 335 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 45:
#line 342 "yacc.yy"
{
		yyval._str = yyvsp[-1]._str;
	  ;
    break;}
case 46:
#line 346 "yacc.yy"
{
		/* $$ = $1; */
		yyval._str = new QString( *(yyvsp[-2]._str) + *(yyvsp[0]._str) );
	  ;
    break;}
case 47:
#line 354 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 48:
#line 358 "yacc.yy"
{
		yyval._str = new QString( "" );
	  ;
    break;}
case 49:
#line 365 "yacc.yy"
{
          ;
    break;}
case 51:
#line 372 "yacc.yy"
{
		yyval._str = new QString( "" );
	  ;
    break;}
case 52:
#line 376 "yacc.yy"
{
		yyval._str = new QString( *(yyvsp[-1]._str) + *(yyvsp[0]._str) );
	  ;
    break;}
case 53:
#line 380 "yacc.yy"
{
		yyval._str = new QString( *(yyvsp[-1]._str) + *(yyvsp[0]._str) );
	  ;
    break;}
case 54:
#line 384 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 55:
#line 388 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 56:
#line 392 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 57:
#line 396 "yacc.yy"
{	
	        yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 58:
#line 400 "yacc.yy"
{
 	        yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 59:
#line 404 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 60:
#line 408 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 61:
#line 412 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 62:
#line 416 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 63:
#line 420 "yacc.yy"
{
                yyval._str = yyvsp[0]._str;
          ;
    break;}
case 64:
#line 424 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 65:
#line 428 "yacc.yy"
{
                yyval._str = yyvsp[0]._str;
          ;
    break;}
case 66:
#line 432 "yacc.yy"
{
                yyval._str = yyvsp[0]._str;
          ;
    break;}
case 73:
#line 450 "yacc.yy"
{;
    break;}
case 74:
#line 451 "yacc.yy"
{;
    break;}
case 75:
#line 455 "yacc.yy"
{;
    break;}
case 76:
#line 456 "yacc.yy"
{;
    break;}
case 77:
#line 457 "yacc.yy"
{;
    break;}
case 78:
#line 458 "yacc.yy"
{;
    break;}
case 79:
#line 459 "yacc.yy"
{;
    break;}
case 80:
#line 463 "yacc.yy"
{;
    break;}
case 81:
#line 464 "yacc.yy"
{;
    break;}
case 82:
#line 465 "yacc.yy"
{;
    break;}
case 83:
#line 470 "yacc.yy"
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
case 84:
#line 480 "yacc.yy"
{
		if (dcop_area)
		  yyerror("scoped template typedefs are not supported in dcop areas!");
	  ;
    break;}
case 85:
#line 488 "yacc.yy"
{
		yyval._int = 0;
	  ;
    break;}
case 86:
#line 492 "yacc.yy"
{
		yyval._int = 1;
	  ;
    break;}
case 87:
#line 498 "yacc.yy"
{ yyval._str = new QString("signed int"); ;
    break;}
case 88:
#line 499 "yacc.yy"
{ yyval._str = new QString("signed int"); ;
    break;}
case 89:
#line 500 "yacc.yy"
{ yyval._str = new QString("unsigned int"); ;
    break;}
case 90:
#line 501 "yacc.yy"
{ yyval._str = new QString("unsigned int"); ;
    break;}
case 91:
#line 502 "yacc.yy"
{ yyval._str = new QString("signed short int"); ;
    break;}
case 92:
#line 503 "yacc.yy"
{ yyval._str = new QString("signed short int"); ;
    break;}
case 93:
#line 504 "yacc.yy"
{ yyval._str = new QString("signed long int"); ;
    break;}
case 94:
#line 505 "yacc.yy"
{ yyval._str = new QString("signed long int"); ;
    break;}
case 95:
#line 506 "yacc.yy"
{ yyval._str = new QString("unsigned short int"); ;
    break;}
case 96:
#line 507 "yacc.yy"
{ yyval._str = new QString("unsigned short int"); ;
    break;}
case 97:
#line 508 "yacc.yy"
{ yyval._str = new QString("unsigned long int"); ;
    break;}
case 98:
#line 509 "yacc.yy"
{ yyval._str = new QString("unsigned long int"); ;
    break;}
case 99:
#line 510 "yacc.yy"
{ yyval._str = new QString("int"); ;
    break;}
case 100:
#line 511 "yacc.yy"
{ yyval._str = new QString("long int"); ;
    break;}
case 101:
#line 512 "yacc.yy"
{ yyval._str = new QString("long int"); ;
    break;}
case 102:
#line 513 "yacc.yy"
{ yyval._str = new QString("short int"); ;
    break;}
case 103:
#line 514 "yacc.yy"
{ yyval._str = new QString("short int"); ;
    break;}
case 104:
#line 515 "yacc.yy"
{ yyval._str = new QString("char"); ;
    break;}
case 105:
#line 516 "yacc.yy"
{ yyval._str = new QString("signed char"); ;
    break;}
case 106:
#line 517 "yacc.yy"
{ yyval._str = new QString("unsigned char"); ;
    break;}
case 109:
#line 527 "yacc.yy"
{
		yyval._str = new QString( "" );
	  ;
    break;}
case 111:
#line 532 "yacc.yy"
{
		yyval._str = new QString( *(yyvsp[-2]._str) + *(yyvsp[0]._str) );
	  ;
    break;}
case 113:
#line 540 "yacc.yy"
{ yyval._str = yyvsp[0]._str; ;
    break;}
case 114:
#line 541 "yacc.yy"
{ yyval._str = yyvsp[0]._str; ;
    break;}
case 115:
#line 542 "yacc.yy"
{ yyval._str = yyvsp[0]._str; ;
    break;}
case 116:
#line 543 "yacc.yy"
{
		QString *tmp = new QString("%1&lt;%2&gt;");
		*tmp = tmp->arg(*(yyvsp[-3]._str));
		*tmp = tmp->arg(*(yyvsp[-1]._str));
		yyval._str = tmp;
	 ;
    break;}
case 117:
#line 549 "yacc.yy"
{
		QString *tmp = new QString("%1&lt;%2&gt;::%3");
		*tmp = tmp->arg(*(yyvsp[-5]._str));
		*tmp = tmp->arg(*(yyvsp[-3]._str));
		*tmp = tmp->arg(*(yyvsp[0]._str));
		yyval._str = tmp;
	 ;
    break;}
case 118:
#line 561 "yacc.yy"
{
	    yyval._str = new QString(*(yyvsp[-2]._str) + "," + *(yyvsp[0]._str));
	  ;
    break;}
case 119:
#line 565 "yacc.yy"
{
 	    yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 120:
#line 573 "yacc.yy"
{
	    if (dcop_area)
	      yyerror("in dcop areas are no pointers allowed");
	  ;
    break;}
case 121:
#line 578 "yacc.yy"
{
 	    yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 122:
#line 587 "yacc.yy"
{
	    if (dcop_area)
	      yyerror("in dcop areas are no pointers allowed");
	  ;
    break;}
case 123:
#line 591 "yacc.yy"
{
	     if (dcop_area) {
	  	QString* tmp = new QString("<TYPE  qleft=\"const\" qright=\"" AMP_ENTITY "\">%1</TYPE>");
		*tmp = tmp->arg( *(yyvsp[-1]._str) );
		yyval._str = tmp;
	     }
	  ;
    break;}
case 124:
#line 598 "yacc.yy"
{
		QString* tmp = new QString("<TYPE>%1</TYPE>");
		*tmp = tmp->arg( *(yyvsp[0]._str) );
		yyval._str = tmp;
	;
    break;}
case 125:
#line 603 "yacc.yy"
{
	     if (dcop_area)
		yyerror("in dcop areas are only const references allowed!");
	  ;
    break;}
case 126:
#line 608 "yacc.yy"
{
		QString* tmp = new QString("<TYPE>%1</TYPE>");
		*tmp = tmp->arg( *(yyvsp[0]._str) );
		yyval._str = tmp;
	;
    break;}
case 127:
#line 614 "yacc.yy"
{
	    if (dcop_area)
	      yyerror("in dcop areas are no pointers allowed");
	  ;
    break;}
case 128:
#line 622 "yacc.yy"
{
	    yyval._str = new QString(*(yyvsp[-2]._str) + "," + *(yyvsp[0]._str));
	  ;
    break;}
case 129:
#line 626 "yacc.yy"
{
 	    yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 130:
#line 633 "yacc.yy"
{
		if (dcop_area) {
		   QString* tmp = new QString("\n        <ARG>%1<NAME>%2</NAME></ARG>");
  		   *tmp = tmp->arg( *(yyvsp[-2]._str) );
  		   *tmp = tmp->arg( *(yyvsp[-1]._str) );
		   yyval._str = tmp;		
		} else yyval._str = new QString();
	  ;
    break;}
case 131:
#line 642 "yacc.yy"
{
		if (dcop_area) {
		   QString* tmp = new QString("\n        <ARG>%1</ARG>");
  		   *tmp = tmp->arg( *(yyvsp[-1]._str) );
		   yyval._str = tmp;		
		} else yyval._str = new QString();
	  ;
    break;}
case 132:
#line 650 "yacc.yy"
{
		if (dcop_area)
			yyerror("variable arguments not supported in dcop area.");
		yyval._str = new QString("");
	  ;
    break;}
case 133:
#line 659 "yacc.yy"
{
	  ;
    break;}
case 134:
#line 662 "yacc.yy"
{
	  ;
    break;}
case 135:
#line 665 "yacc.yy"
{
	  ;
    break;}
case 136:
#line 672 "yacc.yy"
{
          ;
    break;}
case 137:
#line 675 "yacc.yy"
{
          ;
    break;}
case 138:
#line 678 "yacc.yy"
{
          ;
    break;}
case 139:
#line 681 "yacc.yy"
{
          ;
    break;}
case 140:
#line 684 "yacc.yy"
{
          ;
    break;}
case 141:
#line 689 "yacc.yy"
{ yyval._int = 0; ;
    break;}
case 142:
#line 690 "yacc.yy"
{ yyval._int = 1; ;
    break;}
case 148:
#line 699 "yacc.yy"
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
                *tmp = tmp->arg( *(yyvsp[-2]._str) );
                
                QString tagname = (dcop_signal_area) ? "SIGNAL" : "FUNC";
                QString attr = (yyvsp[0]._int) ? " qual=\"const\"" : "";
                *tmp = tmp->arg( QString("%1%2").arg(tagname).arg(attr) );
                *tmp = tmp->arg( QString("%1").arg(tagname) );
		yyval._str = tmp;
   	     } else
	        yyval._str = new QString("");
	  ;
    break;}
case 149:
#line 721 "yacc.yy"
{
	     if (dcop_area)
		yyerror("operators aren't allowed in dcop areas!");
	     yyval._str = new QString("");
	  ;
    break;}
case 150:
#line 731 "yacc.yy"
{;
    break;}
case 151:
#line 732 "yacc.yy"
{;
    break;}
case 152:
#line 733 "yacc.yy"
{;
    break;}
case 153:
#line 738 "yacc.yy"
{;
    break;}
case 154:
#line 743 "yacc.yy"
{;
    break;}
case 155:
#line 744 "yacc.yy"
{;
    break;}
case 156:
#line 749 "yacc.yy"
{
	        yyval._str = yyvsp[-1]._str;
	  ;
    break;}
case 157:
#line 753 "yacc.yy"
{
		yyval._str = yyvsp[-3]._str;
	  ;
    break;}
case 158:
#line 757 "yacc.yy"
{
		yyval._str = yyvsp[-1]._str;
	  ;
    break;}
case 159:
#line 761 "yacc.yy"
{
	      /* The constructor */
	      assert(!dcop_area);
              yyval._str = new QString("");
	  ;
    break;}
case 160:
#line 767 "yacc.yy"
{
	      /* The constructor */
	      assert(!dcop_area);
              yyval._str = new QString("");
	  ;
    break;}
case 161:
#line 773 "yacc.yy"
{
	      /* The destructor */
  	      assert(!dcop_area);
              yyval._str = new QString("");
	  ;
    break;}
case 162:
#line 779 "yacc.yy"
{
              if (dcop_area) {
                 if (dcop_signal_area)
                     yyerror("DCOP signals cannot be static");
                 else
                     yyerror("DCOP functions cannot be static");
              } else {
                 yyval._str = new QString();
              }  
	  ;
    break;}
case 163:
#line 792 "yacc.yy"
{
		function_mode = 1;
	;
    break;}
case 167:
#line 804 "yacc.yy"
{;
    break;}
case 168:
#line 805 "yacc.yy"
{;
    break;}
case 172:
#line 817 "yacc.yy"
{;
    break;}
case 173:
#line 818 "yacc.yy"
{;
    break;}
case 174:
#line 819 "yacc.yy"
{;
    break;}
case 175:
#line 822 "yacc.yy"
{;
    break;}
case 176:
#line 826 "yacc.yy"
{;
    break;}
case 177:
#line 827 "yacc.yy"
{;
    break;}
case 178:
#line 828 "yacc.yy"
{;
    break;}
case 179:
#line 829 "yacc.yy"
{;
    break;}
case 180:
#line 830 "yacc.yy"
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
#line 833 "yacc.yy"


void dcopidlParse( const char *_code )
{
    dcopidlInitFlex( _code );
    yyparse();
}
