
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
#define	T_UNKNOWN	296
#define	T_TRIPLE_DOT	297
#define	T_TRUE	298
#define	T_FALSE	299
#define	T_STATIC	300
#define	T_EQUAL	301
#define	T_SCOPE	302
#define	T_NULL	303
#define	T_INT	304
#define	T_ARRAY_OPEN	305
#define	T_ARRAY_CLOSE	306
#define	T_CHAR	307
#define	T_DCOP	308
#define	T_DCOP_AREA	309
#define	T_SIGNED	310
#define	T_UNSIGNED	311
#define	T_LONG	312
#define	T_SHORT	313
#define	T_FUNOPERATOR	314
#define	T_MISCOPERATOR	315

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

void dcopidlInitFlex( const char *_code );

void yyerror( const char *s )
{
	qDebug( "In line %i : %s", idl_line_no, s );
        exit(1);
	//   theParser->parse_error( idl_lexFile, s, idl_line_no );
}


#line 55 "yacc.yy"
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



#define	YYFINAL		305
#define	YYFLAG		-32768
#define	YYNTBASE	62

#define YYTRANSLATE(x) ((unsigned)(x) <= 315 ? yytranslate[x] : 110)

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
    57,    58,    59,    60,    61
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     4,     5,     8,    13,    14,    16,    17,    24,    28,
    32,    39,    46,    49,    54,    62,    71,    74,    76,    78,
    80,    83,    84,    86,    88,    90,    92,    94,    96,    98,
    99,   103,   106,   109,   111,   115,   117,   121,   123,   126,
   130,   133,   135,   137,   140,   143,   146,   149,   152,   155,
   161,   166,   171,   179,   186,   193,   197,   199,   203,   205,
   207,   209,   212,   214,   216,   218,   222,   230,   240,   247,
   248,   250,   252,   254,   257,   260,   264,   267,   270,   274,
   277,   279,   281,   283,   285,   288,   291,   294,   296,   297,
   299,   303,   305,   307,   310,   313,   318,   325,   329,   331,
   335,   339,   342,   345,   347,   350,   354,   356,   360,   363,
   365,   366,   369,   375,   377,   379,   381,   383,   388,   389,
   391,   393,   395,   397,   399,   406,   414,   416,   418,   422,
   427,   429,   433,   436,   442,   446,   452,   460,   467,   471,
   473,   475,   479,   484,   487,   488,   490,   493,   494,   497,
   501,   507
};

static const short yyrhs[] = {    63,
    65,    62,     0,     0,     8,    63,     0,    38,    11,    62,
    13,     0,     0,    54,     0,     0,     9,    72,    76,    64,
    77,    16,     0,     9,    72,    16,     0,    10,    72,    16,
     0,    10,    72,    11,    62,    13,    16,     0,    41,     5,
    11,    62,    13,    16,     0,    37,    16,     0,    28,    90,
    72,    16,     0,    28,    10,    11,    66,    13,    72,    16,
     0,    28,    10,    72,    11,    66,    13,    72,    16,     0,
    23,   102,     0,   102,     0,   109,     0,    78,     0,   109,
    66,     0,     0,    44,     0,    45,     0,    20,     0,    18,
     0,    17,     0,    26,     0,    27,     0,     0,    68,    69,
    15,     0,    69,    15,     0,    55,    15,     0,     5,     0,
     5,    48,    72,     0,    72,     0,    95,    17,    73,     0,
    73,     0,    74,    11,     0,    74,    31,    75,     0,    15,
    75,     0,    11,     0,    13,     0,    83,    77,     0,   102,
    77,     0,    78,    77,     0,    71,    77,     0,    70,    77,
     0,   109,    77,     0,    24,     9,    72,    16,    77,     0,
    24,    72,    16,    77,     0,     9,    72,    16,    77,     0,
    40,     5,    11,    79,    13,     5,    16,     0,    40,     5,
    11,    79,    13,    16,     0,    40,    11,    79,    13,     5,
    16,     0,    80,    31,    79,     0,    80,     0,     5,    47,
    82,     0,     5,     0,     3,     0,     6,     0,    30,     6,
     0,    49,     0,    72,     0,    81,     0,    81,    29,    81,
     0,    28,    72,    34,    91,    35,    72,    16,     0,    28,
    72,    34,    91,    35,    48,     5,    72,    16,     0,    10,
    72,    11,    62,    13,    16,     0,     0,    22,     0,    56,
     0,    57,     0,    56,    59,     0,    56,    58,     0,    56,
    59,    50,     0,    57,    59,     0,    57,    58,     0,    57,
    59,    50,     0,    57,    50,     0,    50,     0,    58,     0,
    59,     0,    53,     0,    56,    53,     0,    57,    53,     0,
    32,    86,     0,    32,     0,     0,    92,     0,    87,    31,
    92,     0,    85,     0,    72,     0,    10,    72,     0,     9,
    72,     0,    72,    34,    89,    35,     0,    72,    34,    89,
    35,    48,    72,     0,    88,    31,    89,     0,    88,     0,
    22,    88,    86,     0,    22,    88,    36,     0,    22,    88,
     0,    88,    36,     0,    88,     0,    88,    86,     0,    90,
    31,    91,     0,    90,     0,    90,    72,    93,     0,    90,
    93,     0,    43,     0,     0,    47,    94,     0,    47,    12,
    90,    14,    94,     0,     7,     0,    82,     0,     4,     0,
    67,     0,    72,    12,    87,    14,     0,     0,    21,     0,
    61,     0,    35,     0,    34,     0,    47,     0,    90,    72,
    12,    87,    14,    84,     0,    90,    60,    96,    12,    87,
    14,    84,     0,    81,     0,    98,     0,    98,    31,    99,
     0,     5,    12,    99,    14,     0,   100,     0,   100,    31,
   101,     0,    97,   104,     0,    21,    97,    47,    49,   104,
     0,    21,    97,   104,     0,    72,    12,    87,    14,   104,
     0,    72,    12,    87,    14,    15,   101,   104,     0,    95,
    33,    72,    12,    14,   104,     0,    46,    97,   104,     0,
    11,     0,    16,     0,   103,   105,    13,     0,   103,   105,
    13,    16,     0,   106,   105,     0,     0,    16,     0,    31,
   108,     0,     0,     5,   107,     0,    90,   108,    16,     0,
    46,    90,     5,    93,    16,     0,    90,     5,    51,    82,
    52,    16,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
   147,   150,   153,   157,   160,   166,   167,   170,   175,   178,
   181,   184,   187,   190,   193,   196,   199,   202,   205,   208,
   213,   214,   216,   216,   218,   218,   218,   220,   220,   220,
   223,   227,   233,   239,   242,   249,   257,   261,   268,   272,
   280,   284,   291,   295,   299,   303,   307,   311,   315,   319,
   323,   327,   333,   334,   335,   338,   339,   342,   343,   346,
   347,   348,   349,   350,   353,   354,   357,   367,   372,   377,
   381,   388,   389,   390,   391,   392,   393,   394,   395,   396,
   397,   398,   399,   400,   401,   402,   405,   406,   409,   413,
   414,   421,   422,   423,   424,   425,   431,   440,   444,   449,
   454,   461,   466,   471,   476,   484,   488,   494,   503,   511,
   519,   522,   525,   531,   534,   537,   540,   543,   549,   550,
   553,   553,   553,   553,   556,   581,   588,   591,   592,   595,
   598,   599,   602,   606,   610,   614,   620,   626,   632,   639,
   645,   646,   647,   650,   651,   654,   657,   658,   660,   663,
   664,   665
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
"T_UNKNOWN","T_TRIPLE_DOT","T_TRUE","T_FALSE","T_STATIC","T_EQUAL","T_SCOPE",
"T_NULL","T_INT","T_ARRAY_OPEN","T_ARRAY_CLOSE","T_CHAR","T_DCOP","T_DCOP_AREA",
"T_SIGNED","T_UNSIGNED","T_LONG","T_SHORT","T_FUNOPERATOR","T_MISCOPERATOR",
"main","includes","dcoptag","declaration","member_list","bool_value","nodcop_area",
"sigslot","nodcop_area_begin","dcop_area_begin","Identifier","super_class_name",
"super_class","super_classes","class_header","body","enum","enum_list","enum_item",
"number","int_expression","typedef","const_qualifier","int_type","asterisks",
"params","type_name","type_name_list","type","type_list","param","default","default_value",
"virtual_qualifier","operator","function_header","argument","arguments","init_item",
"init_list","function","function_begin","function_body","function_lines","function_line",
"Identifier_list_rest","Identifier_list","member", NULL
};
#endif

static const short yyr1[] = {     0,
    62,    62,    63,    63,    63,    64,    64,    65,    65,    65,
    65,    65,    65,    65,    65,    65,    65,    65,    65,    65,
    66,    66,    67,    67,    68,    68,    68,    69,    69,    69,
    70,    70,    71,    72,    72,    73,    74,    74,    75,    75,
    76,    76,    77,    77,    77,    77,    77,    77,    77,    77,
    77,    77,    78,    78,    78,    79,    79,    80,    80,    81,
    81,    81,    81,    81,    82,    82,    83,    83,    83,    84,
    84,    85,    85,    85,    85,    85,    85,    85,    85,    85,
    85,    85,    85,    85,    85,    85,    86,    86,    87,    87,
    87,    88,    88,    88,    88,    88,    88,    89,    89,    90,
    90,    90,    90,    90,    90,    91,    91,    92,    92,    92,
    93,    93,    93,    94,    94,    94,    94,    94,    95,    95,
    96,    96,    96,    96,    97,    97,    98,    99,    99,   100,
   101,   101,   102,   102,   102,   102,   102,   102,   102,   103,
   104,   104,   104,   105,   105,   106,   107,   107,   108,   109,
   109,   109
};

static const short yyr2[] = {     0,
     3,     0,     2,     4,     0,     1,     0,     6,     3,     3,
     6,     6,     2,     4,     7,     8,     2,     1,     1,     1,
     2,     0,     1,     1,     1,     1,     1,     1,     1,     0,
     3,     2,     2,     1,     3,     1,     3,     1,     2,     3,
     2,     1,     1,     2,     2,     2,     2,     2,     2,     5,
     4,     4,     7,     6,     6,     3,     1,     3,     1,     1,
     1,     2,     1,     1,     1,     3,     7,     9,     6,     0,
     1,     1,     1,     2,     2,     3,     2,     2,     3,     2,
     1,     1,     1,     1,     2,     2,     2,     1,     0,     1,
     3,     1,     1,     2,     2,     4,     6,     3,     1,     3,
     3,     2,     2,     1,     2,     3,     1,     3,     2,     1,
     0,     2,     5,     1,     1,     1,     1,     4,     0,     1,
     1,     1,     1,     1,     6,     7,     1,     1,     3,     4,
     1,     3,     2,     5,     3,     5,     7,     6,     3,     1,
     1,     3,     4,     2,     0,     1,     2,     0,     2,     3,
     5,     6
};

static const short yydefact[] = {     5,
     5,     0,   119,     3,     5,    34,     0,     0,   120,     0,
   119,     0,     0,     0,     0,     0,    81,    84,    72,    73,
    82,    83,     5,    93,    20,    92,   104,     0,     0,     0,
    18,    19,     0,     0,    95,    94,     0,     0,    93,     0,
     0,   102,     0,    17,     0,     0,    13,     0,     0,     0,
     0,     0,    85,    75,    74,    80,    86,    78,    77,     1,
    89,     0,    88,   103,   105,    34,     0,     0,     0,     0,
   140,   141,   145,   133,     4,    35,    42,   119,     9,     7,
     5,    10,    95,    94,     0,   135,   101,   100,    22,    94,
     0,     0,    59,     0,    57,     5,    34,   139,    76,    79,
   110,     0,   111,    90,    99,     0,    87,     0,     0,   149,
   123,   122,   124,   121,     0,    89,   150,     0,   146,     0,
   145,   120,    36,    38,     0,    41,     0,     6,    30,     0,
     0,     0,     0,     0,    22,    22,    14,     0,     0,     0,
     0,     0,     0,     0,     0,     0,   111,   109,     0,    96,
   148,   147,    60,    61,     0,    63,    64,    65,     0,    89,
     0,     0,   142,   144,    39,   119,     0,     0,     0,    43,
    27,    26,    25,     0,    28,    29,     0,     0,    30,     0,
    30,    30,     0,    30,    30,    30,    30,     0,   134,     0,
     0,   148,    21,     0,     0,    58,     0,    56,     0,   116,
   114,     0,    23,    24,   117,    64,   115,   112,   151,     0,
   136,    91,   108,    98,     0,    62,     0,     0,     0,    70,
     0,   143,    40,    37,    95,    94,     0,     0,     0,    33,
     0,    32,    48,    47,     8,    46,    44,    45,    49,    11,
   111,     0,     0,     0,    54,    55,    12,     0,    89,     0,
   131,     0,    97,    66,   152,    70,    71,   125,   138,    30,
     5,     0,    30,     0,    31,    15,     0,    53,     0,     0,
     0,     0,   137,   126,    52,     0,    30,    51,   107,     0,
    16,   113,   118,   127,   128,     0,   132,     0,    50,     0,
     0,     0,   130,    69,   106,     0,     0,   129,     0,    67,
     0,    68,     0,     0,     0
};

static const short yydefgoto[] = {    33,
     3,   129,    23,   133,   205,   179,   180,   181,   182,    39,
   124,   125,   126,    80,   183,   184,    94,    95,   158,   207,
   185,   258,    26,    65,   102,    27,   106,    28,   280,   104,
   144,   208,    29,   115,    30,   285,   286,   251,   252,   186,
    73,    74,   120,   121,   110,    69,   187
};

static const short yypact[] = {    72,
    87,    34,   185,-32768,    36,    35,   100,   100,   195,   223,
    31,   319,    92,   127,   112,   195,-32768,-32768,   -26,   205,
-32768,-32768,   149,    -6,-32768,-32768,    84,     2,   114,   182,
-32768,-32768,   120,   100,   254,   218,   100,   100,   138,     6,
   103,   183,   195,-32768,   180,   100,-32768,   188,   191,   207,
    10,   182,-32768,-32768,   171,-32768,-32768,-32768,   177,-32768,
   301,   223,   145,-32768,-32768,    70,   176,   204,   214,   100,
-32768,-32768,   220,-32768,-32768,-32768,-32768,    17,-32768,   193,
    36,-32768,-32768,-32768,   190,-32768,-32768,-32768,   102,   238,
   243,   191,   225,   248,   236,    36,   154,-32768,-32768,-32768,
-32768,    78,    24,-32768,   244,   250,-32768,   282,    73,-32768,
-32768,-32768,-32768,-32768,   280,   301,-32768,   283,-32768,   287,
   220,-32768,-32768,-32768,   104,-32768,   276,-32768,   281,   291,
   182,   195,   299,   310,   102,   102,-32768,   303,    73,   314,
   191,   307,   343,   306,   302,   301,   278,-32768,   223,   284,
   295,-32768,-32768,-32768,   324,-32768,-32768,   304,   290,   301,
   161,   321,   327,-32768,-32768,    17,   100,   100,   100,-32768,
-32768,-32768,-32768,   241,-32768,-32768,   100,   330,   251,   337,
   281,   281,   340,   281,   281,   281,   281,   345,-32768,   348,
   100,   125,-32768,   349,    88,-32768,   347,-32768,   350,-32768,
-32768,   195,-32768,-32768,-32768,   352,-32768,-32768,-32768,   360,
-32768,-32768,-32768,-32768,   100,-32768,    73,   351,   166,   346,
   182,-32768,-32768,-32768,   354,   363,   100,   355,   356,-32768,
   364,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
   278,   365,   100,   366,-32768,-32768,-32768,   369,   301,   368,
   353,   182,-32768,-32768,-32768,   346,-32768,-32768,-32768,   281,
    36,   370,   281,   195,-32768,-32768,   373,-32768,   124,   172,
    73,   360,-32768,-32768,-32768,   372,   281,-32768,   362,   359,
-32768,-32768,-32768,-32768,   367,   377,-32768,   379,-32768,   195,
    21,    73,-32768,-32768,-32768,   391,   381,-32768,   100,-32768,
   383,-32768,   400,   401,-32768
};

static const short yypgoto[] = {     1,
   402,-32768,-32768,   148,-32768,-32768,   226,-32768,-32768,    -3,
   235,-32768,   240,-32768,  -126,   404,   -78,-32768,  -207,   -92,
-32768,   152,-32768,   -12,  -103,    -8,   255,     7,   119,   264,
   -82,   142,   -75,-32768,    30,-32768,   121,-32768,   140,    91,
-32768,   -32,   293,-32768,-32768,   308,     9
};


#define	YYLAST		416


static const short yytable[] = {    24,
   303,    42,   127,    35,    36,    61,    66,    24,    86,   254,
     6,    32,   161,   138,    97,    40,   159,    40,    46,    98,
   148,     6,    51,    60,    68,     6,    53,    62,     6,    88,
    76,    54,    55,    83,    84,     6,    68,   122,    41,    37,
    38,    90,    91,     1,     5,    52,   196,    68,    -2,    40,
   107,     9,    10,   105,   233,   234,   219,   236,   237,   238,
   239,    67,   198,   284,   213,    67,   118,   103,   296,    67,
   143,    -2,    52,     2,   123,   153,    43,     6,   154,     1,
    17,   130,    34,    18,   284,  -148,    19,    20,    21,    22,
   127,   145,   244,    31,     1,   134,   142,   135,   189,   147,
   108,    44,   155,   245,     6,   157,     6,    47,   146,     2,
    37,    38,   211,    71,   165,    63,    50,    34,    72,    64,
   109,   156,   103,    10,     2,    24,   153,   200,     6,   154,
   201,    48,    75,   275,   166,   157,   278,    49,   190,   206,
   105,   134,   134,   135,   135,   270,    70,   132,    -2,    85,
   289,    17,   103,   155,    18,   108,     1,    19,    20,    21,
    22,    -2,   123,   123,   225,   226,   103,   203,   204,  -111,
   228,    62,   156,   229,   220,   109,    63,    24,    24,   256,
    24,    24,    24,    24,     6,   283,     2,   242,   259,     6,
    89,   146,    71,     7,     8,    93,   146,    72,    92,     6,
   143,    34,   146,    37,    38,     9,    10,    11,   248,   111,
   112,   253,    12,   157,    63,   116,    10,    96,    87,   273,
    99,    13,   113,   262,    14,    15,   100,     6,    81,   117,
    16,    37,    38,    82,    17,   119,   114,    18,   131,   267,
    19,    20,    21,    22,    17,     6,   128,    18,   136,   227,
    19,    20,    21,    22,    56,   103,    24,    57,   137,    24,
   140,   276,    58,    59,    77,   206,   141,   157,    78,    79,
   279,   139,    17,    24,   149,    18,   175,   176,    19,    20,
    21,    22,   193,   194,   150,     6,   151,   297,   157,   168,
   169,   160,   167,   170,   162,   301,   279,   171,   172,   163,
   173,     9,    10,   188,   174,     6,   175,   176,   177,    37,
    38,   191,    71,  -119,   192,   195,   210,    72,   197,   199,
    14,   209,    10,     6,   143,   108,    16,    37,    45,   216,
    17,   215,   217,    18,   221,   178,    19,    20,    21,    22,
    10,   218,   222,   101,   230,   153,   200,     6,   154,   201,
    17,   232,   241,    18,   202,   235,    19,    20,    21,    22,
   240,   243,   246,   249,   250,   247,   255,   257,    17,   260,
   263,    18,   155,   261,    19,    20,    21,    22,   265,   271,
   266,   268,   269,   272,   288,   277,   203,   204,   281,   264,
   293,   156,   290,   291,   294,   299,   300,   292,   302,   304,
   305,   224,     4,   214,   231,   223,    25,   274,   295,   212,
   282,   287,   298,   164,     0,   152
};

static const short yycheck[] = {     3,
     0,    10,    78,     7,     8,    12,     5,    11,    41,   217,
     5,     3,   116,    92,     5,     9,   109,    11,    12,    52,
   103,     5,    16,    23,    28,     5,    53,    34,     5,    42,
    34,    58,    59,    37,    38,     5,    40,    21,     9,     9,
    10,    45,    46,     8,    11,    16,   139,    51,    13,    43,
    63,    21,    22,    62,   181,   182,   160,   184,   185,   186,
   187,    60,   141,   271,   147,    60,    70,    61,    48,    60,
    47,     0,    43,    38,    78,     3,    46,     5,     6,     8,
    50,    81,    48,    53,   292,    16,    56,    57,    58,    59,
   166,    14,     5,     3,     8,    89,    96,    89,   131,   103,
    31,    11,    30,    16,     5,   109,     5,    16,    31,    38,
     9,    10,   145,    11,    11,    32,     5,    48,    16,    36,
    51,    49,   116,    22,    38,   129,     3,     4,     5,     6,
     7,     5,    13,   260,    31,   139,   263,    11,   132,   143,
   149,   135,   136,   135,   136,   249,    33,    46,     0,    47,
   277,    50,   146,    30,    53,    31,     8,    56,    57,    58,
    59,    13,   166,   167,   168,   169,   160,    44,    45,    16,
   174,    34,    49,   177,    14,    51,    32,   181,   182,    14,
   184,   185,   186,   187,     5,    14,    38,   191,   221,     5,
    11,    31,    11,     9,    10,     5,    31,    16,    11,     5,
    47,    48,    31,     9,    10,    21,    22,    23,   202,    34,
    35,   215,    28,   217,    32,    12,    22,    11,    36,   252,
    50,    37,    47,   227,    40,    41,    50,     5,    11,    16,
    46,     9,    10,    16,    50,    16,    61,    53,    49,   243,
    56,    57,    58,    59,    50,     5,    54,    53,    11,     9,
    56,    57,    58,    59,    50,   249,   260,    53,    16,   263,
    13,   261,    58,    59,    11,   269,    31,   271,    15,    16,
   264,    47,    50,   277,    31,    53,    26,    27,    56,    57,
    58,    59,   135,   136,    35,     5,     5,   291,   292,     9,
    10,    12,    17,    13,    12,   299,   290,    17,    18,    13,
    20,    21,    22,    13,    24,     5,    26,    27,    28,     9,
    10,    13,    11,    33,     5,    13,    15,    16,     5,    13,
    40,    16,    22,     5,    47,    31,    46,     9,    10,     6,
    50,    48,    29,    53,    14,    55,    56,    57,    58,    59,
    22,    52,    16,    43,    15,     3,     4,     5,     6,     7,
    50,    15,     5,    53,    12,    16,    56,    57,    58,    59,
    16,    13,    16,    12,     5,    16,    16,    22,    50,    16,
    16,    53,    30,    11,    56,    57,    58,    59,    15,    12,
    16,    16,    14,    31,    13,    16,    44,    45,    16,    34,
    14,    49,    31,    35,    16,     5,    16,    31,    16,     0,
     0,   167,     1,   149,   179,   166,     3,   256,   290,   146,
   269,   272,   292,   121,    -1,   108
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
#line 148 "yacc.yy"
{
	  ;
    break;}
case 3:
#line 154 "yacc.yy"
{
		printf("<INCLUDE>%s</INCLUDE>\n", yyvsp[-1]._str->latin1() );
	  ;
    break;}
case 4:
#line 158 "yacc.yy"
{
	  ;
    break;}
case 5:
#line 161 "yacc.yy"
{
          ;
    break;}
case 6:
#line 166 "yacc.yy"
{ yyval._int = 1; ;
    break;}
case 7:
#line 167 "yacc.yy"
{ yyval._int = 0; ;
    break;}
case 8:
#line 171 "yacc.yy"
{
	 	if (yyvsp[-2]._int)
			  printf("<CLASS>\n    <NAME>%s</NAME>\n%s%s</CLASS>\n", yyvsp[-4]._str->latin1(), yyvsp[-3]._str->latin1(), yyvsp[-1]._str->latin1() );
	  ;
    break;}
case 9:
#line 176 "yacc.yy"
{
	  ;
    break;}
case 10:
#line 179 "yacc.yy"
{
	  ;
    break;}
case 11:
#line 182 "yacc.yy"
{
	  ;
    break;}
case 12:
#line 185 "yacc.yy"
{
	  ;
    break;}
case 13:
#line 188 "yacc.yy"
{
	  ;
    break;}
case 14:
#line 191 "yacc.yy"
{
	  ;
    break;}
case 15:
#line 194 "yacc.yy"
{
	  ;
    break;}
case 16:
#line 197 "yacc.yy"
{
	  ;
    break;}
case 17:
#line 200 "yacc.yy"
{
	  ;
    break;}
case 18:
#line 203 "yacc.yy"
{
	  ;
    break;}
case 19:
#line 206 "yacc.yy"
{
	  ;
    break;}
case 20:
#line 209 "yacc.yy"
{
	  ;
    break;}
case 31:
#line 224 "yacc.yy"
{
	  dcop_area = 0;
	;
    break;}
case 32:
#line 228 "yacc.yy"
{
	  dcop_area = 0;
	;
    break;}
case 33:
#line 234 "yacc.yy"
{
	  dcop_area = 1;
	;
    break;}
case 34:
#line 239 "yacc.yy"
{
	  yyval._str = yyvsp[0]._str;
	;
    break;}
case 35:
#line 242 "yacc.yy"
{
	   QString* tmp = new QString( "%1::%2" );
           *tmp = tmp->arg(*(yyvsp[-2]._str)).arg(*(yyvsp[0]._str));
           yyval._str = tmp;
	;
    break;}
case 36:
#line 250 "yacc.yy"
{
		QString* tmp = new QString( "    <SUPER>%1</SUPER>\n" );
		*tmp = tmp->arg( *(yyvsp[0]._str) );
		yyval._str = tmp;
	  ;
    break;}
case 37:
#line 258 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 38:
#line 262 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 39:
#line 269 "yacc.yy"
{
		yyval._str = yyvsp[-1]._str;
	  ;
    break;}
case 40:
#line 273 "yacc.yy"
{
		/* $$ = $1; */
		yyval._str = new QString( *(yyvsp[-2]._str) + *(yyvsp[0]._str) );
	  ;
    break;}
case 41:
#line 281 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 42:
#line 285 "yacc.yy"
{
		yyval._str = new QString( "" );
	  ;
    break;}
case 43:
#line 292 "yacc.yy"
{
		yyval._str = new QString( "" );
	  ;
    break;}
case 44:
#line 296 "yacc.yy"
{
		yyval._str = new QString( *(yyvsp[-1]._str) + *(yyvsp[0]._str) );
	  ;
    break;}
case 45:
#line 300 "yacc.yy"
{
		yyval._str = new QString( *(yyvsp[-1]._str) + *(yyvsp[0]._str) );
	  ;
    break;}
case 46:
#line 304 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 47:
#line 308 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 48:
#line 312 "yacc.yy"
{	
	        yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 49:
#line 316 "yacc.yy"
{
 	        yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 50:
#line 320 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 51:
#line 324 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 52:
#line 328 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 58:
#line 342 "yacc.yy"
{;
    break;}
case 59:
#line 343 "yacc.yy"
{;
    break;}
case 60:
#line 346 "yacc.yy"
{;
    break;}
case 61:
#line 347 "yacc.yy"
{;
    break;}
case 62:
#line 348 "yacc.yy"
{;
    break;}
case 63:
#line 349 "yacc.yy"
{;
    break;}
case 64:
#line 350 "yacc.yy"
{;
    break;}
case 65:
#line 353 "yacc.yy"
{;
    break;}
case 66:
#line 354 "yacc.yy"
{;
    break;}
case 67:
#line 358 "yacc.yy"
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
case 68:
#line 368 "yacc.yy"
{
		if (dcop_area)
		  yyerror("scoped template typedefs are not supported in dcop areas!");
	  ;
    break;}
case 69:
#line 373 "yacc.yy"
{
	  ;
    break;}
case 70:
#line 378 "yacc.yy"
{
		yyval._int = 0;
	  ;
    break;}
case 71:
#line 382 "yacc.yy"
{
		yyval._int = 1;
	  ;
    break;}
case 72:
#line 388 "yacc.yy"
{ yyval._str = new QString("signed int"); ;
    break;}
case 73:
#line 389 "yacc.yy"
{ yyval._str = new QString("unsigned int"); ;
    break;}
case 74:
#line 390 "yacc.yy"
{ yyval._str = new QString("signed short int"); ;
    break;}
case 75:
#line 391 "yacc.yy"
{ yyval._str = new QString("signed long int"); ;
    break;}
case 76:
#line 392 "yacc.yy"
{ yyval._str = new QString("signed short int"); ;
    break;}
case 77:
#line 393 "yacc.yy"
{ yyval._str = new QString("unsigned short int"); ;
    break;}
case 78:
#line 394 "yacc.yy"
{ yyval._str = new QString("unsigned long int"); ;
    break;}
case 79:
#line 395 "yacc.yy"
{ yyval._str = new QString("unsigned short int"); ;
    break;}
case 80:
#line 396 "yacc.yy"
{ yyval._str = new QString("unsigned int"); ;
    break;}
case 81:
#line 397 "yacc.yy"
{ yyval._str = new QString("int"); ;
    break;}
case 82:
#line 398 "yacc.yy"
{ yyval._str = new QString("long int"); ;
    break;}
case 83:
#line 399 "yacc.yy"
{ yyval._str = new QString("short int"); ;
    break;}
case 84:
#line 400 "yacc.yy"
{ yyval._str = new QString("char"); ;
    break;}
case 85:
#line 401 "yacc.yy"
{ yyval._str = new QString("signed char"); ;
    break;}
case 86:
#line 402 "yacc.yy"
{ yyval._str = new QString("unsigned char"); ;
    break;}
case 89:
#line 410 "yacc.yy"
{
		yyval._str = new QString( "" );
	  ;
    break;}
case 91:
#line 415 "yacc.yy"
{
		yyval._str = new QString( *(yyvsp[-2]._str) + *(yyvsp[0]._str) );
	  ;
    break;}
case 93:
#line 422 "yacc.yy"
{ yyval._str = yyvsp[0]._str; ;
    break;}
case 94:
#line 423 "yacc.yy"
{ yyval._str = yyvsp[0]._str; ;
    break;}
case 95:
#line 424 "yacc.yy"
{ yyval._str = yyvsp[0]._str; ;
    break;}
case 96:
#line 425 "yacc.yy"
{
		QString *tmp = new QString("%1&lt;%2&gt;");
		*tmp = tmp->arg(*(yyvsp[-3]._str));
		*tmp = tmp->arg(*(yyvsp[-1]._str));
		yyval._str = tmp;
	 ;
    break;}
case 97:
#line 431 "yacc.yy"
{
		QString *tmp = new QString("%1&lt;%2&gt;::%3");
		*tmp = tmp->arg(*(yyvsp[-5]._str));
		*tmp = tmp->arg(*(yyvsp[-3]._str));
		*tmp = tmp->arg(*(yyvsp[0]._str));
		yyval._str = tmp;
	 ;
    break;}
case 98:
#line 441 "yacc.yy"
{
	    yyval._str = new QString(*(yyvsp[-2]._str) + "," + *(yyvsp[0]._str));
	  ;
    break;}
case 99:
#line 445 "yacc.yy"
{
 	    yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 100:
#line 450 "yacc.yy"
{
	    if (dcop_area)
	      yyerror("in dcop areas are no pointers allowed");
	  ;
    break;}
case 101:
#line 454 "yacc.yy"
{
	     if (dcop_area) {
	  	QString* tmp = new QString("<TYPE  qleft=\"const\" qright=\"" AMP_ENTITY "\">%1</TYPE>");
		*tmp = tmp->arg( *(yyvsp[-1]._str) );
		yyval._str = tmp;
	     }
	  ;
    break;}
case 102:
#line 461 "yacc.yy"
{
		QString* tmp = new QString("<TYPE>%1</TYPE>");
		*tmp = tmp->arg( *(yyvsp[0]._str) );
		yyval._str = tmp;
	;
    break;}
case 103:
#line 466 "yacc.yy"
{
	     if (dcop_area)
		yyerror("in dcop areas are only const references allowed!");
	  ;
    break;}
case 104:
#line 471 "yacc.yy"
{
		QString* tmp = new QString("<TYPE>%1</TYPE>");
		*tmp = tmp->arg( *(yyvsp[0]._str) );
		yyval._str = tmp;
	;
    break;}
case 105:
#line 477 "yacc.yy"
{
	    if (dcop_area)
	      yyerror("in dcop areas are no pointers allowed");
	  ;
    break;}
case 106:
#line 485 "yacc.yy"
{
	    yyval._str = new QString(*(yyvsp[-2]._str) + "," + *(yyvsp[0]._str));
	  ;
    break;}
case 107:
#line 489 "yacc.yy"
{
 	    yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 108:
#line 495 "yacc.yy"
{
		if (dcop_area) {
		   QString* tmp = new QString("\n        <ARG>%1<NAME>%2</NAME></ARG>");
  		   *tmp = tmp->arg( *(yyvsp[-2]._str) );
  		   *tmp = tmp->arg( *(yyvsp[-1]._str) );
		   yyval._str = tmp;		
		} else yyval._str = new QString();
	  ;
    break;}
case 109:
#line 504 "yacc.yy"
{
		if (dcop_area) {
		   QString* tmp = new QString("\n        <ARG>%1</ARG>");
  		   *tmp = tmp->arg( *(yyvsp[-1]._str) );
		   yyval._str = tmp;		
		} else yyval._str = new QString();
	  ;
    break;}
case 110:
#line 512 "yacc.yy"
{
		if (dcop_area)
			yyerror("variable arguments not supported in dcop area.");
		yyval._str = new QString("");
	  ;
    break;}
case 111:
#line 520 "yacc.yy"
{
	  ;
    break;}
case 112:
#line 523 "yacc.yy"
{
	  ;
    break;}
case 113:
#line 526 "yacc.yy"
{
	  ;
    break;}
case 114:
#line 532 "yacc.yy"
{
          ;
    break;}
case 115:
#line 535 "yacc.yy"
{
          ;
    break;}
case 116:
#line 538 "yacc.yy"
{
          ;
    break;}
case 117:
#line 541 "yacc.yy"
{
          ;
    break;}
case 118:
#line 544 "yacc.yy"
{
          ;
    break;}
case 119:
#line 549 "yacc.yy"
{ yyval._int = 0; ;
    break;}
case 120:
#line 550 "yacc.yy"
{ yyval._int = 1; ;
    break;}
case 125:
#line 557 "yacc.yy"
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
case 126:
#line 582 "yacc.yy"
{
	     if (dcop_area)
		yyerror("operators aren't allowed in dcop areas!");
	     yyval._str = new QString("");
	  ;
    break;}
case 127:
#line 588 "yacc.yy"
{;
    break;}
case 128:
#line 591 "yacc.yy"
{;
    break;}
case 129:
#line 592 "yacc.yy"
{;
    break;}
case 130:
#line 595 "yacc.yy"
{;
    break;}
case 131:
#line 598 "yacc.yy"
{;
    break;}
case 132:
#line 599 "yacc.yy"
{;
    break;}
case 133:
#line 603 "yacc.yy"
{
	        yyval._str = yyvsp[-1]._str;
	  ;
    break;}
case 134:
#line 607 "yacc.yy"
{
		yyval._str = yyvsp[-3]._str;
	  ;
    break;}
case 135:
#line 611 "yacc.yy"
{
		yyval._str = yyvsp[-1]._str;
	  ;
    break;}
case 136:
#line 615 "yacc.yy"
{
	      /* The constructor */
	      assert(!dcop_area);
              yyval._str = new QString("");
	  ;
    break;}
case 137:
#line 621 "yacc.yy"
{
	      /* The constructor */
	      assert(!dcop_area);
              yyval._str = new QString("");
	  ;
    break;}
case 138:
#line 627 "yacc.yy"
{
	      /* The destructor */
  	      assert(!dcop_area);
              yyval._str = new QString("");
	  ;
    break;}
case 139:
#line 633 "yacc.yy"
{
		if (dcop_area)
		  yyerror("static is not allowed in dcop area!");
		yyval._str = new QString();
	  ;
    break;}
case 140:
#line 640 "yacc.yy"
{
		function_mode = 1;
	;
    break;}
case 144:
#line 650 "yacc.yy"
{;
    break;}
case 145:
#line 651 "yacc.yy"
{;
    break;}
case 149:
#line 660 "yacc.yy"
{;
    break;}
case 150:
#line 663 "yacc.yy"
{;
    break;}
case 151:
#line 664 "yacc.yy"
{;
    break;}
case 152:
#line 665 "yacc.yy"
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
#line 667 "yacc.yy"


void dcopidlParse( const char *_code )
{
    dcopidlInitFlex( _code );
    yyparse();
}
