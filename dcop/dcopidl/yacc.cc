
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
#define	T_SHIFT	317

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



#define	YYFINAL		353
#define	YYFLAG		-32768
#define	YYNTBASE	64

#define YYTRANSLATE(x) ((unsigned)(x) <= 317 ? yytranslate[x] : 116)

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
    57,    58,    59,    60,    61,    62,    63
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     4,     5,     8,    13,    14,    16,    17,    24,    28,
    32,    39,    40,    48,    53,    59,    62,    67,    75,    84,
    87,    89,    91,    93,    96,    97,    99,   101,   103,   105,
   107,   109,   111,   112,   116,   119,   122,   124,   128,   130,
   135,   139,   141,   144,   148,   151,   153,   154,   156,   158,
   161,   164,   167,   170,   173,   176,   182,   187,   192,   197,
   202,   209,   217,   224,   231,   237,   241,   243,   247,   249,
   251,   253,   256,   258,   260,   262,   266,   270,   278,   288,
   295,   296,   298,   300,   303,   305,   308,   311,   315,   318,
   322,   325,   329,   332,   336,   338,   340,   343,   345,   348,
   350,   353,   356,   359,   361,   362,   364,   368,   370,   372,
   375,   378,   383,   390,   394,   396,   399,   401,   405,   409,
   412,   415,   417,   420,   424,   426,   430,   433,   435,   436,
   439,   445,   447,   449,   451,   453,   458,   459,   461,   463,
   465,   467,   469,   471,   478,   486,   488,   490,   494,   496,
   500,   505,   507,   511,   514,   520,   524,   530,   538,   545,
   549,   551,   553,   557,   562,   565,   566,   568,   571,   572,
   574,   578,   581,   584,   588,   594,   600
};

static const short yyrhs[] = {    65,
    67,    64,     0,     0,     8,    65,     0,    38,    11,    64,
    13,     0,     0,    55,     0,     0,     9,    75,    79,    66,
    81,    16,     0,     9,    75,    16,     0,    10,    75,    16,
     0,    10,    75,    11,    64,    13,    16,     0,     0,    41,
     5,    11,    68,    64,    13,    80,     0,    42,    41,     5,
    16,     0,    42,     5,    49,     5,    16,     0,    37,    16,
     0,    28,    95,    75,    16,     0,    28,    10,    11,    69,
    13,    75,    16,     0,    28,    10,    75,    11,    69,    13,
    75,    16,     0,    23,   107,     0,   107,     0,   115,     0,
    82,     0,   115,    69,     0,     0,    45,     0,    46,     0,
    20,     0,    18,     0,    17,     0,    26,     0,    27,     0,
     0,    71,    72,    15,     0,    72,    15,     0,    56,    15,
     0,     5,     0,     5,    49,    75,     0,    75,     0,    75,
    34,    96,    35,     0,   100,    17,    76,     0,    76,     0,
    77,    11,     0,    77,    31,    78,     0,    15,    78,     0,
    11,     0,     0,    16,     0,    13,     0,    87,    81,     0,
   107,    81,     0,    82,    81,     0,    74,    81,     0,    73,
    81,     0,   115,    81,     0,    24,     9,    75,    16,    81,
     0,    24,    75,    16,    81,     0,    24,   102,    16,    81,
     0,     9,    75,    16,    81,     0,    10,    75,    16,    81,
     0,    42,     5,    49,     5,    16,    81,     0,    40,     5,
    11,    83,    13,     5,    16,     0,    40,     5,    11,    83,
    13,    16,     0,    40,    11,    83,    13,     5,    16,     0,
    40,    11,    83,    13,    16,     0,    84,    31,    83,     0,
    84,     0,     5,    48,    86,     0,     5,     0,     3,     0,
     6,     0,    30,     6,     0,    50,     0,    75,     0,    85,
     0,    85,    29,    85,     0,    85,    63,    85,     0,    28,
    75,    34,    96,    35,    75,    16,     0,    28,    75,    34,
    96,    35,    49,     5,    75,    16,     0,    10,    75,    11,
    64,    13,    16,     0,     0,    22,     0,    57,     0,    57,
    51,     0,    58,     0,    58,    51,     0,    57,    60,     0,
    57,    60,    51,     0,    57,    59,     0,    57,    59,    51,
     0,    58,    60,     0,    58,    60,    51,     0,    58,    59,
     0,    58,    59,    51,     0,    51,     0,    59,     0,    59,
    51,     0,    60,     0,    60,    51,     0,    54,     0,    57,
    54,     0,    58,    54,     0,    32,    90,     0,    32,     0,
     0,    97,     0,    91,    31,    97,     0,    89,     0,    75,
     0,    10,    75,     0,     9,    75,     0,    75,    34,    93,
    35,     0,    75,    34,    93,    35,    49,    75,     0,    94,
    31,    93,     0,    94,     0,    92,    90,     0,    92,     0,
    22,    92,    90,     0,    22,    92,    36,     0,    22,    92,
     0,    92,    36,     0,    92,     0,    92,    90,     0,    95,
    31,    96,     0,    95,     0,    95,    75,    98,     0,    95,
    98,     0,    44,     0,     0,    48,    99,     0,    48,    12,
    95,    14,    99,     0,     7,     0,    86,     0,     4,     0,
    70,     0,    75,    12,    91,    14,     0,     0,    21,     0,
    62,     0,    63,     0,    35,     0,    34,     0,    48,     0,
    95,    75,    12,    91,    14,    88,     0,    95,    61,   101,
    12,    91,    14,    88,     0,    85,     0,    70,     0,     5,
    12,    14,     0,   103,     0,   103,    31,   104,     0,     5,
    12,   104,    14,     0,   105,     0,   105,    31,   106,     0,
   102,   109,     0,    21,   102,    48,    50,   109,     0,    21,
   102,   109,     0,    75,    12,    91,    14,   109,     0,    75,
    12,    91,    14,    15,   106,   109,     0,   100,    33,    75,
    12,    14,   109,     0,    47,   102,   109,     0,    11,     0,
    16,     0,   108,   110,    13,     0,   108,   110,    13,    16,
     0,   111,   110,     0,     0,    16,     0,    31,   114,     0,
     0,     5,     0,     5,    48,    99,     0,    90,     5,     0,
   113,   112,     0,    95,   114,    16,     0,    95,    75,    15,
     6,    16,     0,    47,    95,     5,    98,    16,     0,    95,
     5,    52,    86,    53,    16,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
   152,   155,   158,   162,   165,   171,   172,   175,   180,   183,
   186,   189,   193,   201,   204,   207,   210,   213,   216,   219,
   222,   225,   228,   233,   234,   236,   236,   238,   238,   238,
   240,   240,   240,   243,   247,   253,   259,   262,   269,   275,
   283,   287,   294,   298,   306,   310,   317,   320,   324,   328,
   332,   336,   340,   344,   348,   352,   356,   360,   364,   368,
   372,   378,   379,   380,   381,   384,   385,   388,   389,   392,
   393,   394,   395,   396,   399,   400,   401,   404,   414,   419,
   424,   428,   435,   436,   437,   438,   439,   440,   441,   442,
   443,   444,   445,   446,   447,   448,   449,   450,   451,   452,
   453,   454,   457,   458,   461,   465,   466,   474,   475,   476,
   477,   478,   484,   494,   498,   505,   510,   518,   523,   530,
   535,   540,   545,   553,   557,   563,   572,   580,   588,   591,
   594,   600,   603,   606,   609,   612,   618,   619,   622,   622,
   622,   622,   622,   625,   650,   658,   659,   660,   663,   664,
   667,   670,   671,   674,   678,   682,   686,   692,   698,   704,
   711,   717,   718,   719,   722,   723,   726,   729,   730,   732,
   733,   734,   736,   739,   740,   741,   742
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
"T_SHIFT","main","includes","dcoptag","declaration","@1","member_list","bool_value",
"nodcop_area","sigslot","nodcop_area_begin","dcop_area_begin","Identifier","super_class_name",
"super_class","super_classes","class_header","opt_semicolon","body","enum","enum_list",
"enum_item","number","int_expression","typedef","const_qualifier","int_type",
"asterisks","params","type_name","templ_type_list","templ_type","type","type_list",
"param","default","default_value","virtual_qualifier","operator","function_header",
"argument","arguments","init_item","init_list","function","function_begin","function_body",
"function_lines","function_line","Identifier_list_rest","Identifier_list_entry",
"Identifier_list","member", NULL
};
#endif

static const short yyr1[] = {     0,
    64,    64,    65,    65,    65,    66,    66,    67,    67,    67,
    67,    68,    67,    67,    67,    67,    67,    67,    67,    67,
    67,    67,    67,    69,    69,    70,    70,    71,    71,    71,
    72,    72,    72,    73,    73,    74,    75,    75,    76,    76,
    77,    77,    78,    78,    79,    79,    80,    80,    81,    81,
    81,    81,    81,    81,    81,    81,    81,    81,    81,    81,
    81,    82,    82,    82,    82,    83,    83,    84,    84,    85,
    85,    85,    85,    85,    86,    86,    86,    87,    87,    87,
    88,    88,    89,    89,    89,    89,    89,    89,    89,    89,
    89,    89,    89,    89,    89,    89,    89,    89,    89,    89,
    89,    89,    90,    90,    91,    91,    91,    92,    92,    92,
    92,    92,    92,    93,    93,    94,    94,    95,    95,    95,
    95,    95,    95,    96,    96,    97,    97,    97,    98,    98,
    98,    99,    99,    99,    99,    99,   100,   100,   101,   101,
   101,   101,   101,   102,   102,   103,   103,   103,   104,   104,
   105,   106,   106,   107,   107,   107,   107,   107,   107,   107,
   108,   109,   109,   109,   110,   110,   111,   112,   112,   113,
   113,   113,   114,   115,   115,   115,   115
};

static const short yyr2[] = {     0,
     3,     0,     2,     4,     0,     1,     0,     6,     3,     3,
     6,     0,     7,     4,     5,     2,     4,     7,     8,     2,
     1,     1,     1,     2,     0,     1,     1,     1,     1,     1,
     1,     1,     0,     3,     2,     2,     1,     3,     1,     4,
     3,     1,     2,     3,     2,     1,     0,     1,     1,     2,
     2,     2,     2,     2,     2,     5,     4,     4,     4,     4,
     6,     7,     6,     6,     5,     3,     1,     3,     1,     1,
     1,     2,     1,     1,     1,     3,     3,     7,     9,     6,
     0,     1,     1,     2,     1,     2,     2,     3,     2,     3,
     2,     3,     2,     3,     1,     1,     2,     1,     2,     1,
     2,     2,     2,     1,     0,     1,     3,     1,     1,     2,
     2,     4,     6,     3,     1,     2,     1,     3,     3,     2,
     2,     1,     2,     3,     1,     3,     2,     1,     0,     2,
     5,     1,     1,     1,     1,     4,     0,     1,     1,     1,
     1,     1,     1,     6,     7,     1,     1,     3,     1,     3,
     4,     1,     3,     2,     5,     3,     5,     7,     6,     3,
     1,     1,     3,     4,     2,     0,     1,     2,     0,     1,
     3,     2,     2,     3,     5,     5,     6
};

static const short yydefact[] = {     5,
     5,     0,   137,     3,     5,    37,     0,     0,   138,     0,
   137,     0,     0,     0,     0,     0,     0,    95,   100,    83,
    85,    96,    98,     5,   109,    23,   108,   122,     0,     0,
     0,    21,    22,     0,     0,   111,   110,     0,     0,   109,
     0,     0,   120,     0,    20,     0,     0,    16,     0,     0,
     0,     0,     0,     0,     0,    84,   101,    89,    87,    86,
   102,    93,    91,    97,    99,     1,   105,     0,   104,   121,
   123,    37,     0,     0,     0,   169,     0,     0,   161,   162,
   166,   154,     4,    38,    46,   137,     9,     7,     5,    10,
   111,   110,     0,     0,   156,   119,   118,    25,   110,     0,
     0,    69,     0,    67,    12,     0,     0,    37,   160,    90,
    88,    94,    92,   128,     0,   129,   106,   117,     0,   115,
   103,     0,     0,   142,   141,   143,   139,   140,     0,   105,
     0,   172,     0,   173,   174,     0,   167,     0,   166,   138,
    39,    42,     0,    45,     0,     6,    33,     0,     0,     0,
     0,     0,    25,    25,    17,     0,     0,     0,     0,     5,
     0,    14,     0,     0,     0,     0,   129,   127,   116,   112,
     0,    70,   134,    71,   132,     0,    26,    27,    73,   135,
    74,    75,   133,   171,    74,     0,   105,     0,     0,   170,
   168,     0,   163,   165,     0,    43,   137,     0,     0,     0,
    49,    30,    29,    28,     0,    31,    32,     0,     0,     0,
    33,     0,    33,    33,     0,    33,    33,    33,    33,     0,
   155,     0,     0,     0,    24,     0,     0,    68,     0,    65,
    66,     0,    15,     0,   130,   176,     0,   157,   107,   126,
     0,   114,    72,   105,     0,     0,     0,     0,    81,   175,
     0,   164,   125,     0,    44,    41,   111,   110,     0,   109,
     0,     0,     0,    36,     0,    35,    54,    53,     8,    52,
    50,    51,    55,    11,   129,     0,     0,     0,    63,    64,
    47,     0,     0,   152,     0,   113,     0,    76,    77,   177,
    81,    82,   144,   159,     0,    40,    33,     5,    33,   111,
    33,    33,     0,     0,    34,    18,     0,    62,    48,    13,
     0,     0,     0,   158,   136,   145,   124,    59,     0,    60,
    33,    57,    58,     0,     0,    19,   131,    37,   147,   146,
   149,     0,   153,     0,    56,     0,    33,     0,     0,   151,
    80,     0,     0,    61,   148,   150,     0,    78,     0,    79,
     0,     0,     0
};

static const short yydefgoto[] = {    34,
     3,   147,    24,   160,   151,   180,   211,   212,   213,   214,
    40,   142,   143,   144,    88,   310,   215,   216,   103,   104,
   182,   183,   217,   293,    27,    75,   115,    28,   119,   120,
    29,   254,   117,   164,   184,    30,   129,    31,   331,   332,
   284,   285,   218,    81,    82,   138,   139,   134,    76,    77,
   219
};

static const short yypact[] = {    61,
   107,    41,   359,-32768,   191,    16,    35,    35,   415,   127,
   117,   426,    79,   162,   153,    40,   415,-32768,-32768,   207,
   230,   119,   138,    58,   122,-32768,-32768,   232,     9,   146,
   214,-32768,-32768,   211,    35,   277,   225,    35,    35,   192,
    13,    95,   271,   415,-32768,   177,    35,-32768,   224,   245,
   246,   210,   255,    25,   214,-32768,-32768,   219,   231,-32768,
-32768,   234,   244,-32768,-32768,-32768,   218,   127,   251,-32768,
-32768,    98,   217,   194,   292,   275,   294,    35,-32768,-32768,
   295,-32768,-32768,-32768,-32768,    32,-32768,   258,   191,-32768,
-32768,-32768,   302,   265,-32768,-32768,-32768,   393,   305,   301,
   245,   272,   306,   290,-32768,   317,   307,    82,-32768,-32768,
-32768,-32768,-32768,-32768,   128,    24,-32768,   251,   291,   293,
-32768,   241,    57,-32768,-32768,-32768,-32768,-32768,   313,   218,
   321,-32768,    52,-32768,-32768,   316,-32768,   318,   295,-32768,
   296,-32768,   130,-32768,   312,-32768,   332,   319,   214,   415,
   322,    89,   393,   393,-32768,   325,    57,   175,   245,   191,
   323,-32768,    43,   324,   289,   218,   298,-32768,-32768,   299,
   127,-32768,-32768,-32768,-32768,   337,-32768,-32768,-32768,-32768,
   335,     4,-32768,-32768,-32768,   304,   218,   134,   339,   303,
-32768,   347,   346,-32768,   415,-32768,    32,    35,    35,    35,
-32768,-32768,-32768,-32768,   436,-32768,-32768,    35,   361,   352,
    83,   355,   332,   332,   357,   332,   332,   332,   332,   360,
-32768,   370,    35,   362,-32768,   365,   196,-32768,   368,-32768,
-32768,   372,-32768,   415,-32768,-32768,   388,-32768,-32768,-32768,
    35,-32768,-32768,   218,    57,    57,   378,   141,   373,-32768,
   214,-32768,   366,   369,-32768,-32768,   389,   238,    35,   135,
   391,   374,   363,-32768,   394,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,   298,   395,    35,   398,-32768,-32768,
   405,   408,   411,   396,   214,-32768,   169,-32768,-32768,-32768,
   373,-32768,-32768,-32768,   415,-32768,   332,   191,   332,   410,
   332,   332,   415,   423,-32768,-32768,   413,-32768,-32768,-32768,
   241,   187,   388,-32768,-32768,-32768,-32768,-32768,   417,-32768,
   332,-32768,-32768,   397,   418,-32768,-32768,    27,-32768,-32768,
   402,   424,-32768,   427,-32768,    29,   332,   425,   187,-32768,
-32768,   437,   433,-32768,-32768,-32768,    35,-32768,   438,-32768,
   455,   456,-32768
};

static const short yypgoto[] = {     3,
   458,-32768,-32768,-32768,    54,  -211,-32768,   249,-32768,-32768,
    -3,   259,-32768,   264,-32768,-32768,  -197,   459,   -95,-32768,
  -221,   -67,-32768,   172,-32768,   -15,  -119,    -9,   297,-32768,
    68,  -187,   310,   -85,  -154,   -74,-32768,    -2,-32768,   125,
-32768,   152,   228,-32768,   -32,   328,-32768,-32768,-32768,   338,
    -1
};


#define	YYLAST		496


static const short yytable[] = {    25,
    43,    33,   351,    36,    37,   156,    42,    25,   235,    95,
   188,   145,    71,    72,    55,   267,   268,     6,   270,   271,
   272,   273,   109,   288,   289,    74,    66,    97,     6,   108,
   168,    84,   245,     6,    91,    92,     6,    93,   338,     6,
    69,    55,    99,   100,    52,   172,   173,     6,   174,   175,
    93,     5,   140,   121,   234,   186,   190,    -2,   118,   172,
    -2,     6,   174,   231,    35,     1,   246,   248,     1,    73,
    -2,   163,   176,    73,   136,    35,    41,   342,    41,    47,
    53,   240,   141,    69,    54,    73,   176,   177,   178,   228,
   330,   148,   179,    72,    48,     2,   153,  -129,     2,   318,
   329,   320,   169,   322,   323,    79,   179,   317,   206,   207,
    80,    41,   167,  -170,     1,   324,   221,   330,   181,   185,
    69,     6,   145,   335,   287,    38,    39,   329,  -170,   163,
    35,     6,   238,    67,   116,    38,    39,     9,    10,   344,
   196,   165,    94,    25,     2,   122,    35,   249,   224,   123,
   301,   153,   153,   185,   291,    68,   327,    51,   166,   181,
   197,   118,   232,    44,   166,   152,    49,    18,    68,    64,
    19,   166,    50,    20,    21,    22,    23,    18,    78,   229,
    19,     6,   315,    20,    21,    22,    23,    98,    65,   172,
   230,   328,   174,   141,   141,   257,   258,   116,     1,   166,
   278,   260,   261,    -2,   262,   130,   225,   226,   131,    25,
    25,   279,    25,    25,    25,    25,   176,   222,   294,   276,
   152,   152,     6,    83,    79,    68,    38,    39,     2,    80,
    32,   177,   178,   116,   101,    89,   179,   286,    45,    10,
    90,   185,   185,   172,   173,     6,   174,   175,   298,   102,
   124,   125,   314,   299,   116,   300,   105,    56,   106,   107,
    57,   114,   253,    69,   126,    58,    59,    70,    18,   110,
   176,    19,    41,   307,    20,    21,    22,    23,   127,   128,
    60,   111,    69,    61,   112,   177,   178,    85,    62,    63,
   179,    86,    87,    25,   113,    25,   132,    25,    25,    79,
   319,   282,    69,   237,    80,   133,    96,   181,   185,   135,
   137,   116,   146,   130,   149,   154,   155,    25,   158,   157,
   159,   161,   162,   171,   187,   170,   189,   192,   198,   195,
   193,   220,   343,    25,   223,   185,     6,   227,   233,   236,
   199,   200,   243,   349,   201,   163,   244,   241,   202,   203,
   122,   204,     9,    10,   250,   205,   247,   206,   207,   208,
   251,   252,   253,     6,  -137,   263,   264,     7,     8,   266,
   253,    14,   269,   209,   275,   274,   131,   277,    17,     9,
    10,    11,    18,   280,   281,    19,    12,   210,    20,    21,
    22,    23,   283,   290,   292,    13,   295,     6,    14,    15,
    16,    38,    39,   296,   297,    17,   302,   303,   305,    18,
   306,   304,    19,   308,    10,    20,    21,    22,    23,     6,
   309,   311,   312,    38,    39,   321,   313,   325,   326,   334,
     6,   336,   339,   337,    38,    46,    10,   340,   345,   150,
     6,   347,   341,    18,   259,    39,    19,    10,   348,    20,
    21,    22,    23,   350,   352,   353,   256,    10,     4,   265,
   255,    26,   316,   346,   333,    18,   194,   242,    19,     0,
   191,    20,    21,    22,    23,   239,    18,     0,     0,    19,
     0,     0,    20,    21,    22,    23,    18,     0,     0,    19,
     0,     0,    20,    21,    22,    23
};

static const short yycheck[] = {     3,
    10,     3,     0,     7,     8,   101,     9,    11,   163,    42,
   130,    86,    28,     5,    17,   213,   214,     5,   216,   217,
   218,   219,    55,   245,   246,    29,    24,    43,     5,     5,
   116,    35,    29,     5,    38,    39,     5,    41,    12,     5,
    32,    44,    46,    47,     5,     3,     4,     5,     6,     7,
    54,    11,    21,    69,    12,   123,     5,     0,    68,     3,
     0,     5,     6,   159,    49,     8,    63,   187,     8,    61,
    13,    48,    30,    61,    78,    49,     9,    49,    11,    12,
    41,   167,    86,    32,    17,    61,    30,    45,    46,   157,
   312,    89,    50,     5,    16,    38,    98,    16,    38,   297,
   312,   299,   118,   301,   302,    11,    50,   295,    26,    27,
    16,    44,   116,    16,     8,   303,   149,   339,   122,   123,
    32,     5,   197,   321,   244,     9,    10,   339,    31,    48,
    49,     5,   165,    12,    67,     9,    10,    21,    22,   337,
    11,    14,    48,   147,    38,    48,    49,    14,   152,    52,
    16,   153,   154,   157,    14,    34,   311,     5,    31,   163,
    31,   171,   160,    47,    31,    98,     5,    51,    34,    51,
    54,    31,    11,    57,    58,    59,    60,    51,    33,     5,
    54,     5,    14,    57,    58,    59,    60,    11,    51,     3,
    16,     5,     6,   197,   198,   199,   200,   130,     8,    31,
     5,   205,   205,    13,   208,    12,   153,   154,    15,   213,
   214,    16,   216,   217,   218,   219,    30,   150,   251,   223,
   153,   154,     5,    13,    11,    34,     9,    10,    38,    16,
     3,    45,    46,   166,    11,    11,    50,   241,    11,    22,
    16,   245,   246,     3,     4,     5,     6,     7,    11,     5,
    34,    35,   285,    16,   187,   259,    11,    51,    49,     5,
    54,    44,   195,    32,    48,    59,    60,    36,    51,    51,
    30,    54,   205,   277,    57,    58,    59,    60,    62,    63,
    51,    51,    32,    54,    51,    45,    46,    11,    59,    60,
    50,    15,    16,   297,    51,   299,     5,   301,   302,    11,
   298,   234,    32,    15,    16,    31,    36,   311,   312,    16,
    16,   244,    55,    12,    50,    11,    16,   321,    13,    48,
    31,     5,    16,    31,    12,    35,     6,    12,    17,    34,
    13,    13,   336,   337,    13,   339,     5,    13,    16,    16,
     9,    10,     6,   347,    13,    48,    12,    49,    17,    18,
    48,    20,    21,    22,    16,    24,    53,    26,    27,    28,
    14,    16,   295,     5,    33,     5,    15,     9,    10,    15,
   303,    40,    16,    42,     5,    16,    15,    13,    47,    21,
    22,    23,    51,    16,    13,    54,    28,    56,    57,    58,
    59,    60,     5,    16,    22,    37,    31,     5,    40,    41,
    42,     9,    10,    35,    16,    47,    16,    34,    15,    51,
    16,    49,    54,    16,    22,    57,    58,    59,    60,     5,
    16,    14,    12,     9,    10,    16,    31,     5,    16,    13,
     5,    35,    31,    16,     9,    10,    22,    14,    14,    47,
     5,     5,    16,    51,     9,    10,    54,    22,    16,    57,
    58,    59,    60,    16,     0,     0,   198,    22,     1,   211,
   197,     3,   291,   339,   313,    51,   139,   171,    54,    -1,
   133,    57,    58,    59,    60,   166,    51,    -1,    -1,    54,
    -1,    -1,    57,    58,    59,    60,    51,    -1,    -1,    54,
    -1,    -1,    57,    58,    59,    60
};
/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
#line 3 "/usr/share/bison/bison.simple"
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

#line 217 "/usr/share/bison/bison.simple"

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
#line 153 "yacc.yy"
{
	  ;
    break;}
case 3:
#line 159 "yacc.yy"
{
		printf("<INCLUDE>%s</INCLUDE>\n", yyvsp[-1]._str->latin1() );
	  ;
    break;}
case 4:
#line 163 "yacc.yy"
{
	  ;
    break;}
case 5:
#line 166 "yacc.yy"
{
          ;
    break;}
case 6:
#line 171 "yacc.yy"
{ yyval._int = 1; ;
    break;}
case 7:
#line 172 "yacc.yy"
{ yyval._int = 0; ;
    break;}
case 8:
#line 176 "yacc.yy"
{
	 	if (yyvsp[-2]._int)
			  printf("<CLASS>\n    <NAME>%s</NAME>\n%s%s</CLASS>\n", ( in_namespace + *yyvsp[-4]._str ).latin1(), yyvsp[-3]._str->latin1(), yyvsp[-1]._str->latin1() );
	  ;
    break;}
case 9:
#line 181 "yacc.yy"
{
	  ;
    break;}
case 10:
#line 184 "yacc.yy"
{
	  ;
    break;}
case 11:
#line 187 "yacc.yy"
{
	  ;
    break;}
case 12:
#line 190 "yacc.yy"
{
                      in_namespace += *yyvsp[-1]._str; in_namespace += "::";
                  ;
    break;}
case 13:
#line 194 "yacc.yy"
{
                      int pos = in_namespace.findRev( "::", -3 );
                      if( pos >= 0 )
                          in_namespace = in_namespace.left( pos + 2 );
                      else
                          in_namespace = "";
                  ;
    break;}
case 14:
#line 202 "yacc.yy"
{
          ;
    break;}
case 15:
#line 205 "yacc.yy"
{
          ;
    break;}
case 16:
#line 208 "yacc.yy"
{
	  ;
    break;}
case 17:
#line 211 "yacc.yy"
{
	  ;
    break;}
case 18:
#line 214 "yacc.yy"
{
	  ;
    break;}
case 19:
#line 217 "yacc.yy"
{
	  ;
    break;}
case 20:
#line 220 "yacc.yy"
{
	  ;
    break;}
case 21:
#line 223 "yacc.yy"
{
	  ;
    break;}
case 22:
#line 226 "yacc.yy"
{
	  ;
    break;}
case 23:
#line 229 "yacc.yy"
{
	  ;
    break;}
case 34:
#line 244 "yacc.yy"
{
	  dcop_area = 0;
	;
    break;}
case 35:
#line 248 "yacc.yy"
{
	  dcop_area = 0;
	;
    break;}
case 36:
#line 254 "yacc.yy"
{
	  dcop_area = 1;
	;
    break;}
case 37:
#line 259 "yacc.yy"
{
	  yyval._str = yyvsp[0]._str;
	;
    break;}
case 38:
#line 262 "yacc.yy"
{
	   QString* tmp = new QString( "%1::%2" );
           *tmp = tmp->arg(*(yyvsp[-2]._str)).arg(*(yyvsp[0]._str));
           yyval._str = tmp;
	;
    break;}
case 39:
#line 270 "yacc.yy"
{
		QString* tmp = new QString( "    <SUPER>%1</SUPER>\n" );
		*tmp = tmp->arg( *(yyvsp[0]._str) );
		yyval._str = tmp;
	  ;
    break;}
case 40:
#line 276 "yacc.yy"
{
		QString* tmp = new QString( "    <SUPER>%1</SUPER>\n" );
		*tmp = tmp->arg( *(yyvsp[-3]._str) + "&lt" + *(yyvsp[-1]._str) + "&gt;" );
		yyval._str = tmp;
	  ;
    break;}
case 41:
#line 284 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 42:
#line 288 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 43:
#line 295 "yacc.yy"
{
		yyval._str = yyvsp[-1]._str;
	  ;
    break;}
case 44:
#line 299 "yacc.yy"
{
		/* $$ = $1; */
		yyval._str = new QString( *(yyvsp[-2]._str) + *(yyvsp[0]._str) );
	  ;
    break;}
case 45:
#line 307 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 46:
#line 311 "yacc.yy"
{
		yyval._str = new QString( "" );
	  ;
    break;}
case 47:
#line 318 "yacc.yy"
{
          ;
    break;}
case 49:
#line 325 "yacc.yy"
{
		yyval._str = new QString( "" );
	  ;
    break;}
case 50:
#line 329 "yacc.yy"
{
		yyval._str = new QString( *(yyvsp[-1]._str) + *(yyvsp[0]._str) );
	  ;
    break;}
case 51:
#line 333 "yacc.yy"
{
		yyval._str = new QString( *(yyvsp[-1]._str) + *(yyvsp[0]._str) );
	  ;
    break;}
case 52:
#line 337 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 53:
#line 341 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 54:
#line 345 "yacc.yy"
{	
	        yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 55:
#line 349 "yacc.yy"
{
 	        yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 56:
#line 353 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 57:
#line 357 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 58:
#line 361 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 59:
#line 365 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 60:
#line 369 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 61:
#line 373 "yacc.yy"
{
                yyval._str = yyvsp[0]._str;
          ;
    break;}
case 68:
#line 388 "yacc.yy"
{;
    break;}
case 69:
#line 389 "yacc.yy"
{;
    break;}
case 70:
#line 392 "yacc.yy"
{;
    break;}
case 71:
#line 393 "yacc.yy"
{;
    break;}
case 72:
#line 394 "yacc.yy"
{;
    break;}
case 73:
#line 395 "yacc.yy"
{;
    break;}
case 74:
#line 396 "yacc.yy"
{;
    break;}
case 75:
#line 399 "yacc.yy"
{;
    break;}
case 76:
#line 400 "yacc.yy"
{;
    break;}
case 77:
#line 401 "yacc.yy"
{;
    break;}
case 78:
#line 405 "yacc.yy"
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
case 79:
#line 415 "yacc.yy"
{
		if (dcop_area)
		  yyerror("scoped template typedefs are not supported in dcop areas!");
	  ;
    break;}
case 80:
#line 420 "yacc.yy"
{
	  ;
    break;}
case 81:
#line 425 "yacc.yy"
{
		yyval._int = 0;
	  ;
    break;}
case 82:
#line 429 "yacc.yy"
{
		yyval._int = 1;
	  ;
    break;}
case 83:
#line 435 "yacc.yy"
{ yyval._str = new QString("signed int"); ;
    break;}
case 84:
#line 436 "yacc.yy"
{ yyval._str = new QString("signed int"); ;
    break;}
case 85:
#line 437 "yacc.yy"
{ yyval._str = new QString("unsigned int"); ;
    break;}
case 86:
#line 438 "yacc.yy"
{ yyval._str = new QString("unsigned int"); ;
    break;}
case 87:
#line 439 "yacc.yy"
{ yyval._str = new QString("signed short int"); ;
    break;}
case 88:
#line 440 "yacc.yy"
{ yyval._str = new QString("signed short int"); ;
    break;}
case 89:
#line 441 "yacc.yy"
{ yyval._str = new QString("signed long int"); ;
    break;}
case 90:
#line 442 "yacc.yy"
{ yyval._str = new QString("signed long int"); ;
    break;}
case 91:
#line 443 "yacc.yy"
{ yyval._str = new QString("unsigned short int"); ;
    break;}
case 92:
#line 444 "yacc.yy"
{ yyval._str = new QString("unsigned short int"); ;
    break;}
case 93:
#line 445 "yacc.yy"
{ yyval._str = new QString("unsigned long int"); ;
    break;}
case 94:
#line 446 "yacc.yy"
{ yyval._str = new QString("unsigned long int"); ;
    break;}
case 95:
#line 447 "yacc.yy"
{ yyval._str = new QString("int"); ;
    break;}
case 96:
#line 448 "yacc.yy"
{ yyval._str = new QString("long int"); ;
    break;}
case 97:
#line 449 "yacc.yy"
{ yyval._str = new QString("long int"); ;
    break;}
case 98:
#line 450 "yacc.yy"
{ yyval._str = new QString("short int"); ;
    break;}
case 99:
#line 451 "yacc.yy"
{ yyval._str = new QString("short int"); ;
    break;}
case 100:
#line 452 "yacc.yy"
{ yyval._str = new QString("char"); ;
    break;}
case 101:
#line 453 "yacc.yy"
{ yyval._str = new QString("signed char"); ;
    break;}
case 102:
#line 454 "yacc.yy"
{ yyval._str = new QString("unsigned char"); ;
    break;}
case 105:
#line 462 "yacc.yy"
{
		yyval._str = new QString( "" );
	  ;
    break;}
case 107:
#line 467 "yacc.yy"
{
		yyval._str = new QString( *(yyvsp[-2]._str) + *(yyvsp[0]._str) );
	  ;
    break;}
case 109:
#line 475 "yacc.yy"
{ yyval._str = yyvsp[0]._str; ;
    break;}
case 110:
#line 476 "yacc.yy"
{ yyval._str = yyvsp[0]._str; ;
    break;}
case 111:
#line 477 "yacc.yy"
{ yyval._str = yyvsp[0]._str; ;
    break;}
case 112:
#line 478 "yacc.yy"
{
		QString *tmp = new QString("%1&lt;%2&gt;");
		*tmp = tmp->arg(*(yyvsp[-3]._str));
		*tmp = tmp->arg(*(yyvsp[-1]._str));
		yyval._str = tmp;
	 ;
    break;}
case 113:
#line 484 "yacc.yy"
{
		QString *tmp = new QString("%1&lt;%2&gt;::%3");
		*tmp = tmp->arg(*(yyvsp[-5]._str));
		*tmp = tmp->arg(*(yyvsp[-3]._str));
		*tmp = tmp->arg(*(yyvsp[0]._str));
		yyval._str = tmp;
	 ;
    break;}
case 114:
#line 495 "yacc.yy"
{
	    yyval._str = new QString(*(yyvsp[-2]._str) + "," + *(yyvsp[0]._str));
	  ;
    break;}
case 115:
#line 499 "yacc.yy"
{
 	    yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 116:
#line 506 "yacc.yy"
{
	    if (dcop_area)
	      yyerror("in dcop areas are no pointers allowed");
	  ;
    break;}
case 117:
#line 511 "yacc.yy"
{
 	    yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 118:
#line 519 "yacc.yy"
{
	    if (dcop_area)
	      yyerror("in dcop areas are no pointers allowed");
	  ;
    break;}
case 119:
#line 523 "yacc.yy"
{
	     if (dcop_area) {
	  	QString* tmp = new QString("<TYPE  qleft=\"const\" qright=\"" AMP_ENTITY "\">%1</TYPE>");
		*tmp = tmp->arg( *(yyvsp[-1]._str) );
		yyval._str = tmp;
	     }
	  ;
    break;}
case 120:
#line 530 "yacc.yy"
{
		QString* tmp = new QString("<TYPE>%1</TYPE>");
		*tmp = tmp->arg( *(yyvsp[0]._str) );
		yyval._str = tmp;
	;
    break;}
case 121:
#line 535 "yacc.yy"
{
	     if (dcop_area)
		yyerror("in dcop areas are only const references allowed!");
	  ;
    break;}
case 122:
#line 540 "yacc.yy"
{
		QString* tmp = new QString("<TYPE>%1</TYPE>");
		*tmp = tmp->arg( *(yyvsp[0]._str) );
		yyval._str = tmp;
	;
    break;}
case 123:
#line 546 "yacc.yy"
{
	    if (dcop_area)
	      yyerror("in dcop areas are no pointers allowed");
	  ;
    break;}
case 124:
#line 554 "yacc.yy"
{
	    yyval._str = new QString(*(yyvsp[-2]._str) + "," + *(yyvsp[0]._str));
	  ;
    break;}
case 125:
#line 558 "yacc.yy"
{
 	    yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 126:
#line 564 "yacc.yy"
{
		if (dcop_area) {
		   QString* tmp = new QString("\n        <ARG>%1<NAME>%2</NAME></ARG>");
  		   *tmp = tmp->arg( *(yyvsp[-2]._str) );
  		   *tmp = tmp->arg( *(yyvsp[-1]._str) );
		   yyval._str = tmp;		
		} else yyval._str = new QString();
	  ;
    break;}
case 127:
#line 573 "yacc.yy"
{
		if (dcop_area) {
		   QString* tmp = new QString("\n        <ARG>%1</ARG>");
  		   *tmp = tmp->arg( *(yyvsp[-1]._str) );
		   yyval._str = tmp;		
		} else yyval._str = new QString();
	  ;
    break;}
case 128:
#line 581 "yacc.yy"
{
		if (dcop_area)
			yyerror("variable arguments not supported in dcop area.");
		yyval._str = new QString("");
	  ;
    break;}
case 129:
#line 589 "yacc.yy"
{
	  ;
    break;}
case 130:
#line 592 "yacc.yy"
{
	  ;
    break;}
case 131:
#line 595 "yacc.yy"
{
	  ;
    break;}
case 132:
#line 601 "yacc.yy"
{
          ;
    break;}
case 133:
#line 604 "yacc.yy"
{
          ;
    break;}
case 134:
#line 607 "yacc.yy"
{
          ;
    break;}
case 135:
#line 610 "yacc.yy"
{
          ;
    break;}
case 136:
#line 613 "yacc.yy"
{
          ;
    break;}
case 137:
#line 618 "yacc.yy"
{ yyval._int = 0; ;
    break;}
case 138:
#line 619 "yacc.yy"
{ yyval._int = 1; ;
    break;}
case 144:
#line 626 "yacc.yy"
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
case 145:
#line 651 "yacc.yy"
{
	     if (dcop_area)
		yyerror("operators aren't allowed in dcop areas!");
	     yyval._str = new QString("");
	  ;
    break;}
case 146:
#line 658 "yacc.yy"
{;
    break;}
case 147:
#line 659 "yacc.yy"
{;
    break;}
case 148:
#line 660 "yacc.yy"
{;
    break;}
case 149:
#line 663 "yacc.yy"
{;
    break;}
case 150:
#line 664 "yacc.yy"
{;
    break;}
case 151:
#line 667 "yacc.yy"
{;
    break;}
case 152:
#line 670 "yacc.yy"
{;
    break;}
case 153:
#line 671 "yacc.yy"
{;
    break;}
case 154:
#line 675 "yacc.yy"
{
	        yyval._str = yyvsp[-1]._str;
	  ;
    break;}
case 155:
#line 679 "yacc.yy"
{
		yyval._str = yyvsp[-3]._str;
	  ;
    break;}
case 156:
#line 683 "yacc.yy"
{
		yyval._str = yyvsp[-1]._str;
	  ;
    break;}
case 157:
#line 687 "yacc.yy"
{
	      /* The constructor */
	      assert(!dcop_area);
              yyval._str = new QString("");
	  ;
    break;}
case 158:
#line 693 "yacc.yy"
{
	      /* The constructor */
	      assert(!dcop_area);
              yyval._str = new QString("");
	  ;
    break;}
case 159:
#line 699 "yacc.yy"
{
	      /* The destructor */
  	      assert(!dcop_area);
              yyval._str = new QString("");
	  ;
    break;}
case 160:
#line 705 "yacc.yy"
{
		if (dcop_area)
		  yyerror("static is not allowed in dcop area!");
		yyval._str = new QString();
	  ;
    break;}
case 161:
#line 712 "yacc.yy"
{
		function_mode = 1;
	;
    break;}
case 165:
#line 722 "yacc.yy"
{;
    break;}
case 166:
#line 723 "yacc.yy"
{;
    break;}
case 170:
#line 732 "yacc.yy"
{;
    break;}
case 171:
#line 733 "yacc.yy"
{;
    break;}
case 172:
#line 734 "yacc.yy"
{;
    break;}
case 173:
#line 736 "yacc.yy"
{;
    break;}
case 174:
#line 739 "yacc.yy"
{;
    break;}
case 175:
#line 740 "yacc.yy"
{;
    break;}
case 176:
#line 741 "yacc.yy"
{;
    break;}
case 177:
#line 742 "yacc.yy"
{;
    break;}
}
   /* the action file gets copied in in place of this dollarsign */
#line 543 "/usr/share/bison/bison.simple"

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
#line 744 "yacc.yy"


void dcopidlParse( const char *_code )
{
    dcopidlInitFlex( _code );
    yyparse();
}
