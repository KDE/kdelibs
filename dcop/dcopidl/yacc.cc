
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



#define	YYFINAL		340
#define	YYFLAG		-32768
#define	YYNTBASE	63

#define YYTRANSLATE(x) ((unsigned)(x) <= 316 ? yytranslate[x] : 115)

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
   342,   344,   346,   349,   352,   357,   364,   368,   370,   373,
   375,   379,   383,   386,   389,   391,   394,   398,   400,   404,
   407,   409,   410,   413,   419,   421,   423,   425,   427,   432,
   433,   435,   437,   439,   441,   443,   450,   458,   460,   462,
   466,   468,   472,   477,   479,   483,   486,   492,   496,   502,
   510,   517,   521,   523,   525,   529,   534,   537,   538,   540,
   543,   544,   546,   549,   552,   556,   562,   568
};

static const short yyrhs[] = {    64,
    66,    63,     0,     0,     8,    64,     0,    38,    11,    63,
    13,     0,     0,    55,     0,     0,     9,    74,    78,    65,
    80,    16,     0,     9,    74,    16,     0,    10,    74,    16,
     0,    10,    74,    11,    63,    13,    16,     0,     0,    41,
     5,    11,    67,    63,    13,    79,     0,    42,    41,     5,
    16,     0,    42,     5,    49,     5,    16,     0,    37,    16,
     0,    28,    94,    74,    16,     0,    28,    10,    11,    68,
    13,    74,    16,     0,    28,    10,    74,    11,    68,    13,
    74,    16,     0,    23,   106,     0,   106,     0,   114,     0,
    81,     0,   114,    68,     0,     0,    45,     0,    46,     0,
    20,     0,    18,     0,    17,     0,    26,     0,    27,     0,
     0,    70,    71,    15,     0,    71,    15,     0,    56,    15,
     0,     5,     0,     5,    49,    74,     0,    74,     0,    99,
    17,    75,     0,    75,     0,    76,    11,     0,    76,    31,
    77,     0,    15,    77,     0,    11,     0,     0,    16,     0,
    13,     0,    86,    80,     0,   106,    80,     0,    81,    80,
     0,    73,    80,     0,    72,    80,     0,   114,    80,     0,
    24,     9,    74,    16,    80,     0,    24,    74,    16,    80,
     0,    24,   101,    16,    80,     0,     9,    74,    16,    80,
     0,    10,    74,    16,    80,     0,    42,     5,    49,     5,
    16,    80,     0,    40,     5,    11,    82,    13,     5,    16,
     0,    40,     5,    11,    82,    13,    16,     0,    40,    11,
    82,    13,     5,    16,     0,    40,    11,    82,    13,    16,
     0,    83,    31,    82,     0,    83,     0,     5,    48,    85,
     0,     5,     0,     3,     0,     6,     0,    30,     6,     0,
    50,     0,    74,     0,    84,     0,    84,    29,    84,     0,
    28,    74,    34,    95,    35,    74,    16,     0,    28,    74,
    34,    95,    35,    49,     5,    74,    16,     0,    10,    74,
    11,    63,    13,    16,     0,     0,    22,     0,    57,     0,
    58,     0,    57,    60,     0,    57,    59,     0,    57,    60,
    51,     0,    58,    60,     0,    58,    59,     0,    58,    60,
    51,     0,    58,    51,     0,    51,     0,    59,     0,    60,
     0,    54,     0,    57,    54,     0,    58,    54,     0,    32,
    89,     0,    32,     0,     0,    96,     0,    90,    31,    96,
     0,    88,     0,    74,     0,    10,    74,     0,     9,    74,
     0,    74,    34,    92,    35,     0,    74,    34,    92,    35,
    49,    74,     0,    93,    31,    92,     0,    93,     0,    91,
    89,     0,    91,     0,    22,    91,    89,     0,    22,    91,
    36,     0,    22,    91,     0,    91,    36,     0,    91,     0,
    91,    89,     0,    94,    31,    95,     0,    94,     0,    94,
    74,    97,     0,    94,    97,     0,    44,     0,     0,    48,
    98,     0,    48,    12,    94,    14,    98,     0,     7,     0,
    85,     0,     4,     0,    69,     0,    74,    12,    90,    14,
     0,     0,    21,     0,    62,     0,    35,     0,    34,     0,
    48,     0,    94,    74,    12,    90,    14,    87,     0,    94,
    61,   100,    12,    90,    14,    87,     0,    84,     0,    69,
     0,     5,    12,    14,     0,   102,     0,   102,    31,   103,
     0,     5,    12,   103,    14,     0,   104,     0,   104,    31,
   105,     0,   101,   108,     0,    21,   101,    48,    50,   108,
     0,    21,   101,   108,     0,    74,    12,    90,    14,   108,
     0,    74,    12,    90,    14,    15,   105,   108,     0,    99,
    33,    74,    12,    14,   108,     0,    47,   101,   108,     0,
    11,     0,    16,     0,   107,   109,    13,     0,   107,   109,
    13,    16,     0,   110,   109,     0,     0,    16,     0,    31,
   113,     0,     0,     5,     0,    89,     5,     0,   112,   111,
     0,    94,   113,    16,     0,    94,    74,    15,     6,    16,
     0,    47,    94,     5,    97,    16,     0,    94,     5,    52,
    85,    53,    16,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
   151,   154,   157,   161,   164,   170,   171,   174,   179,   182,
   185,   188,   192,   200,   203,   206,   209,   212,   215,   218,
   221,   224,   227,   232,   233,   235,   235,   237,   237,   237,
   239,   239,   239,   242,   246,   252,   258,   261,   268,   276,
   280,   287,   291,   299,   303,   310,   313,   317,   321,   325,
   329,   333,   337,   341,   345,   349,   353,   357,   361,   365,
   371,   372,   373,   374,   377,   378,   381,   382,   385,   386,
   387,   388,   389,   392,   393,   396,   406,   411,   416,   420,
   427,   428,   429,   430,   431,   432,   433,   434,   435,   436,
   437,   438,   439,   440,   441,   444,   445,   448,   452,   453,
   461,   462,   463,   464,   465,   471,   481,   485,   492,   497,
   505,   510,   517,   522,   527,   532,   540,   544,   550,   559,
   567,   575,   578,   581,   587,   590,   593,   596,   599,   605,
   606,   609,   609,   609,   609,   612,   637,   645,   646,   647,
   650,   651,   654,   657,   658,   661,   665,   669,   673,   679,
   685,   691,   698,   704,   705,   706,   709,   710,   713,   716,
   717,   719,   720,   722,   725,   726,   727,   728
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
"asterisks","params","type_name","templ_type_list","templ_type","type","type_list",
"param","default","default_value","virtual_qualifier","operator","function_header",
"argument","arguments","init_item","init_list","function","function_begin","function_body",
"function_lines","function_line","Identifier_list_rest","Identifier_list_entry",
"Identifier_list","member", NULL
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
    91,    91,    91,    91,    91,    91,    92,    92,    93,    93,
    94,    94,    94,    94,    94,    94,    95,    95,    96,    96,
    96,    97,    97,    97,    98,    98,    98,    98,    98,    99,
    99,   100,   100,   100,   100,   101,   101,   102,   102,   102,
   103,   103,   104,   105,   105,   106,   106,   106,   106,   106,
   106,   106,   107,   108,   108,   108,   109,   109,   110,   111,
   111,   112,   112,   113,   114,   114,   114,   114
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
     1,     1,     2,     2,     4,     6,     3,     1,     2,     1,
     3,     3,     2,     2,     1,     2,     3,     1,     3,     2,
     1,     0,     2,     5,     1,     1,     1,     1,     4,     0,
     1,     1,     1,     1,     1,     6,     7,     1,     1,     3,
     1,     3,     4,     1,     3,     2,     5,     3,     5,     7,
     6,     3,     1,     1,     3,     4,     2,     0,     1,     2,
     0,     1,     2,     2,     3,     5,     5,     6
};

static const short yydefact[] = {     5,
     5,     0,   130,     3,     5,    37,     0,     0,   131,     0,
   130,     0,     0,     0,     0,     0,     0,    90,    93,    81,
    82,    91,    92,     5,   102,    23,   101,   115,     0,     0,
     0,    21,    22,     0,     0,   104,   103,     0,     0,   102,
     0,     0,   113,     0,    20,     0,     0,    16,     0,     0,
     0,     0,     0,     0,     0,    94,    84,    83,    89,    95,
    87,    86,     1,    98,     0,    97,   114,   116,    37,     0,
     0,     0,   161,     0,     0,   153,   154,   158,   146,     4,
    38,    45,   130,     9,     7,     5,    10,   104,   103,     0,
     0,   148,   112,   111,    25,   103,     0,     0,    68,     0,
    66,    12,     0,     0,    37,   152,    85,    88,   121,     0,
   122,    99,   110,     0,   108,    96,     0,   134,   133,   135,
   132,     0,    98,     0,   163,     0,   164,   165,     0,   159,
     0,   158,   131,    39,    41,     0,    44,     0,     6,    33,
     0,     0,     0,     0,     0,    25,    25,    17,     0,     0,
     0,     0,     5,     0,    14,     0,     0,     0,     0,   122,
   120,   109,   105,     0,    69,    70,     0,    72,    73,    74,
     0,    98,     0,     0,   162,   160,     0,   155,   157,    42,
   130,     0,     0,     0,    48,    30,    29,    28,     0,    31,
    32,     0,     0,     0,    33,     0,    33,    33,     0,    33,
    33,    33,    33,     0,   147,     0,     0,     0,    24,     0,
     0,    67,     0,    64,    65,     0,    15,   127,   125,     0,
    26,    27,   128,    73,   126,   123,   167,     0,   149,   100,
   119,     0,   107,    71,     0,     0,     0,    79,   166,     0,
   156,    43,    40,   104,   103,     0,   102,     0,     0,     0,
    36,     0,    35,    53,    52,     8,    51,    49,    50,    54,
    11,   122,     0,     0,     0,    62,    63,    46,     0,    98,
     0,   144,     0,   106,    75,   168,    79,    80,   136,   151,
    33,     5,    33,   104,    33,    33,     0,     0,    34,    18,
     0,    61,    47,    13,     0,     0,     0,     0,   150,   137,
    58,     0,    59,    33,    56,    57,   118,     0,     0,    19,
   124,   129,    37,   139,   138,   141,     0,   145,     0,    55,
     0,     0,    33,     0,     0,   143,    78,   117,     0,     0,
    60,   140,   142,     0,    76,     0,    77,     0,     0,     0
};

static const short yydefgoto[] = {    34,
     3,   140,    24,   153,   144,   223,   195,   196,   197,   198,
    40,   135,   136,   137,    85,   294,   199,   200,   100,   101,
   170,   225,   201,   279,    27,    72,   110,    28,   114,   115,
    29,   308,   112,   157,   226,    30,   122,    31,   316,   317,
   272,   273,   202,    78,    79,   131,   132,   127,    73,    74,
   203
};

static const short yypact[] = {    45,
    44,    22,   205,-32768,    54,     9,   105,   105,   348,   112,
   248,   364,   115,    29,   134,    24,   348,-32768,-32768,   142,
   180,-32768,-32768,    41,    94,-32768,-32768,   184,    10,    62,
   113,-32768,-32768,   139,   105,   196,   197,   105,   105,   143,
    13,    39,   212,   348,-32768,   198,   105,-32768,   157,   231,
   230,   202,   249,    17,   113,-32768,-32768,   204,-32768,-32768,
-32768,   209,-32768,    76,   112,   234,-32768,-32768,    60,    84,
   206,   262,   237,   255,   105,-32768,-32768,   257,-32768,-32768,
-32768,-32768,   133,-32768,   220,    54,-32768,-32768,-32768,   264,
   224,-32768,-32768,-32768,   334,   266,   263,   231,   233,   271,
   256,-32768,   281,   272,    15,-32768,-32768,-32768,-32768,   126,
    25,-32768,   234,   258,   259,-32768,   187,-32768,-32768,-32768,
-32768,   279,    76,   290,-32768,    43,-32768,-32768,   285,-32768,
   287,   257,-32768,-32768,-32768,    28,-32768,   286,-32768,   308,
   291,   113,   348,   296,    79,   334,   334,-32768,   297,   187,
   146,   231,    54,   282,-32768,   407,   295,   208,    76,   267,
-32768,-32768,   265,   112,-32768,-32768,   306,-32768,-32768,   298,
   280,    76,   127,   300,-32768,-32768,   310,   321,-32768,-32768,
   133,   105,   105,   105,-32768,-32768,-32768,-32768,   374,-32768,
-32768,   105,   333,   325,   149,   327,   308,   308,   329,   308,
   308,   308,   308,   330,-32768,   342,   105,   336,-32768,   339,
   168,-32768,   338,-32768,-32768,   347,-32768,-32768,-32768,   348,
-32768,-32768,-32768,   337,-32768,-32768,-32768,   356,-32768,-32768,
-32768,   105,-32768,-32768,   187,   355,   129,   341,-32768,   113,
-32768,-32768,-32768,   359,   219,   105,   110,   360,   343,   323,
-32768,   363,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,   267,   366,   105,   371,-32768,-32768,   373,   376,    76,
   368,   367,   113,-32768,-32768,-32768,   341,-32768,-32768,-32768,
   308,    54,   308,   379,   308,   308,   348,   392,-32768,-32768,
   384,-32768,-32768,-32768,   435,   136,   424,   356,-32768,-32768,
-32768,   388,-32768,   308,-32768,-32768,   372,   369,   393,-32768,
-32768,-32768,    11,-32768,-32768,   385,   403,-32768,   404,-32768,
   348,    20,   308,   412,   424,-32768,-32768,-32768,   430,   420,
-32768,-32768,-32768,   105,-32768,   427,-32768,   444,   445,-32768
};

static const short yypgoto[] = {     3,
   446,-32768,-32768,-32768,   103,  -209,-32768,   251,-32768,-32768,
    -3,   268,-32768,   270,-32768,-32768,   -98,   452,   -82,-32768,
  -229,   -60,-32768,   171,-32768,   -19,  -111,    -9,   292,-32768,
     2,   128,   299,   -83,   164,   -66,-32768,    -7,-32768,   135,
-32768,   163,    90,-32768,   -35,   331,-32768,-32768,-32768,   340,
    18
};


#define	YYLAST		485


static const short yytable[] = {    25,
    43,    42,   338,    36,    37,   275,    92,    25,    68,    55,
    41,   173,    41,    47,    69,   149,   138,     6,    54,   106,
    33,   105,   324,    94,     6,    71,    63,   161,    52,     6,
  -122,    81,     5,    49,    88,    89,    55,    90,   180,    50,
    -2,    66,    96,    97,    -2,    41,   116,   175,     1,    76,
    90,     1,     1,    -2,    77,   113,   171,    35,   181,    35,
   237,     1,   156,    35,    53,   111,    -2,   315,   329,   215,
    70,   129,   156,    70,    66,  -162,   231,    70,     2,   134,
     6,     2,     2,    69,    38,    39,    91,   314,   141,   212,
  -162,     2,    32,   162,    75,   315,   145,    10,   254,   255,
    45,   257,   258,   259,   260,    64,   205,   160,    35,     6,
    66,   117,   146,   169,   138,   314,     6,   118,   119,   109,
    38,    39,   229,    76,   111,   285,    18,    65,    77,    19,
    48,   120,    20,    21,    22,    23,    25,     6,    51,   158,
   238,   208,   277,    65,   206,   121,   169,   145,   145,   312,
   213,    80,   224,   133,   113,   216,   159,   159,   296,   159,
   111,   214,    18,   146,   146,    19,   159,    98,    20,    21,
    22,    23,   265,   111,   190,   191,    65,   134,   134,   244,
   245,   248,   301,   266,   303,   247,   305,   306,   249,   165,
    41,     6,   166,    25,    25,    56,    25,    25,    25,    25,
    57,    58,     6,   263,   280,   320,    82,    86,    95,     6,
    83,    84,    87,     7,     8,    66,   167,   123,    76,    67,
   124,   269,   228,    77,   331,     9,    10,    11,   274,   282,
    59,   169,    12,    60,   283,    99,   168,   299,    61,    62,
   102,    13,   284,    66,    14,    15,    16,    93,   209,   210,
   103,    17,     6,   104,   107,    18,    38,    39,    19,   108,
   291,    20,    21,    22,    23,    66,   125,   126,     9,    10,
   128,   111,   130,   142,   139,   123,   147,    25,   148,    25,
   150,    25,    25,   151,   302,   154,   152,   155,   307,   164,
   172,   224,   163,   169,    44,   174,   177,   217,    18,   178,
    25,    19,   182,   204,    20,    21,    22,    23,   207,   211,
   227,   234,     6,   232,   156,   239,   183,   184,   330,    25,
   185,   169,   307,   240,   186,   187,   235,   188,     9,    10,
   336,   189,   236,   190,   191,   192,   241,   250,     6,   251,
  -130,   253,    38,    39,   256,   261,   262,    14,   270,   193,
   124,   264,     6,   267,    17,    10,    38,    39,    18,   268,
   271,    19,   278,   194,    20,    21,    22,    23,     6,    10,
   276,   288,    38,    46,   281,   286,   287,   289,     6,   297,
   143,   290,   246,    39,    18,    10,   292,    19,   293,   295,
    20,    21,    22,    23,   304,    10,   309,   298,    18,   310,
   319,    19,   321,   322,    20,    21,    22,    23,   323,   165,
   218,     6,   166,   219,    18,   325,   326,    19,   220,   327,
    20,    21,    22,    23,    18,   332,   165,    19,   313,   166,
    20,    21,    22,    23,   334,   335,   167,   165,   218,     6,
   166,   219,   337,   339,   340,   252,     4,   300,   328,   243,
   242,   221,   222,   167,    26,   233,   168,   230,   311,   333,
   318,     0,   179,     0,   167,   176,     0,     0,   221,   222,
     0,     0,     0,   168,     0,     0,     0,     0,     0,   221,
   222,     0,     0,     0,   168
};

static const short yycheck[] = {     3,
    10,     9,     0,     7,     8,   235,    42,    11,    28,    17,
     9,   123,    11,    12,     5,    98,    83,     5,    17,    55,
     3,     5,    12,    43,     5,    29,    24,   111,     5,     5,
    16,    35,    11,     5,    38,    39,    44,    41,    11,    11,
     0,    32,    46,    47,     0,    44,    66,     5,     8,    11,
    54,     8,     8,    13,    16,    65,   117,    49,    31,    49,
   172,     8,    48,    49,    41,    64,    13,   297,    49,   152,
    61,    75,    48,    61,    32,    16,   160,    61,    38,    83,
     5,    38,    38,     5,     9,    10,    48,   297,    86,   150,
    31,    38,     3,   113,    33,   325,    95,    22,   197,   198,
    11,   200,   201,   202,   203,    12,   142,   111,    49,     5,
    32,    52,    95,   117,   181,   325,     5,    34,    35,    44,
     9,    10,   158,    11,   123,    16,    51,    34,    16,    54,
    16,    48,    57,    58,    59,    60,   140,     5,     5,    14,
    14,   145,    14,    34,   143,    62,   150,   146,   147,    14,
     5,    13,   156,    21,   164,   153,    31,    31,   270,    31,
   159,    16,    51,   146,   147,    54,    31,    11,    57,    58,
    59,    60,     5,   172,    26,    27,    34,   181,   182,   183,
   184,   189,   281,    16,   283,   189,   285,   286,   192,     3,
   189,     5,     6,   197,   198,    54,   200,   201,   202,   203,
    59,    60,     5,   207,   240,   304,    11,    11,    11,     5,
    15,    16,    16,     9,    10,    32,    30,    12,    11,    36,
    15,   220,    15,    16,   323,    21,    22,    23,   232,    11,
    51,   235,    28,    54,    16,     5,    50,   273,    59,    60,
    11,    37,   246,    32,    40,    41,    42,    36,   146,   147,
    49,    47,     5,     5,    51,    51,     9,    10,    54,    51,
   264,    57,    58,    59,    60,    32,     5,    31,    21,    22,
    16,   270,    16,    50,    55,    12,    11,   281,    16,   283,
    48,   285,   286,    13,   282,     5,    31,    16,   287,    31,
    12,   295,    35,   297,    47,     6,    12,    16,    51,    13,
   304,    54,    17,    13,    57,    58,    59,    60,    13,    13,
    16,     6,     5,    49,    48,    16,     9,    10,   322,   323,
    13,   325,   321,    14,    17,    18,    29,    20,    21,    22,
   334,    24,    53,    26,    27,    28,    16,     5,     5,    15,
    33,    15,     9,    10,    16,    16,     5,    40,    12,    42,
    15,    13,     5,    16,    47,    22,     9,    10,    51,    13,
     5,    54,    22,    56,    57,    58,    59,    60,     5,    22,
    16,    49,     9,    10,    16,    16,    34,    15,     5,    12,
    47,    16,     9,    10,    51,    22,    16,    54,    16,    14,
    57,    58,    59,    60,    16,    22,     5,    31,    51,    16,
    13,    54,    31,    35,    57,    58,    59,    60,    16,     3,
     4,     5,     6,     7,    51,    31,    14,    54,    12,    16,
    57,    58,    59,    60,    51,    14,     3,    54,     5,     6,
    57,    58,    59,    60,     5,    16,    30,     3,     4,     5,
     6,     7,    16,     0,     0,   195,     1,   277,   321,   182,
   181,    45,    46,    30,     3,   164,    50,   159,   295,   325,
   298,    -1,   132,    -1,    30,   126,    -1,    -1,    45,    46,
    -1,    -1,    -1,    50,    -1,    -1,    -1,    -1,    -1,    45,
    46,    -1,    -1,    -1,    50
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
#line 152 "yacc.yy"
{
	  ;
    break;}
case 3:
#line 158 "yacc.yy"
{
		printf("<INCLUDE>%s</INCLUDE>\n", yyvsp[-1]._str->latin1() );
	  ;
    break;}
case 4:
#line 162 "yacc.yy"
{
	  ;
    break;}
case 5:
#line 165 "yacc.yy"
{
          ;
    break;}
case 6:
#line 170 "yacc.yy"
{ yyval._int = 1; ;
    break;}
case 7:
#line 171 "yacc.yy"
{ yyval._int = 0; ;
    break;}
case 8:
#line 175 "yacc.yy"
{
	 	if (yyvsp[-2]._int)
			  printf("<CLASS>\n    <NAME>%s</NAME>\n%s%s</CLASS>\n", ( in_namespace + *yyvsp[-4]._str ).latin1(), yyvsp[-3]._str->latin1(), yyvsp[-1]._str->latin1() );
	  ;
    break;}
case 9:
#line 180 "yacc.yy"
{
	  ;
    break;}
case 10:
#line 183 "yacc.yy"
{
	  ;
    break;}
case 11:
#line 186 "yacc.yy"
{
	  ;
    break;}
case 12:
#line 189 "yacc.yy"
{
                      in_namespace += *yyvsp[-1]._str; in_namespace += "::";
                  ;
    break;}
case 13:
#line 193 "yacc.yy"
{
                      int pos = in_namespace.findRev( "::", -3 );
                      if( pos >= 0 )
                          in_namespace = in_namespace.left( pos + 2 );
                      else
                          in_namespace = "";
                  ;
    break;}
case 14:
#line 201 "yacc.yy"
{
          ;
    break;}
case 15:
#line 204 "yacc.yy"
{
          ;
    break;}
case 16:
#line 207 "yacc.yy"
{
	  ;
    break;}
case 17:
#line 210 "yacc.yy"
{
	  ;
    break;}
case 18:
#line 213 "yacc.yy"
{
	  ;
    break;}
case 19:
#line 216 "yacc.yy"
{
	  ;
    break;}
case 20:
#line 219 "yacc.yy"
{
	  ;
    break;}
case 21:
#line 222 "yacc.yy"
{
	  ;
    break;}
case 22:
#line 225 "yacc.yy"
{
	  ;
    break;}
case 23:
#line 228 "yacc.yy"
{
	  ;
    break;}
case 34:
#line 243 "yacc.yy"
{
	  dcop_area = 0;
	;
    break;}
case 35:
#line 247 "yacc.yy"
{
	  dcop_area = 0;
	;
    break;}
case 36:
#line 253 "yacc.yy"
{
	  dcop_area = 1;
	;
    break;}
case 37:
#line 258 "yacc.yy"
{
	  yyval._str = yyvsp[0]._str;
	;
    break;}
case 38:
#line 261 "yacc.yy"
{
	   QString* tmp = new QString( "%1::%2" );
           *tmp = tmp->arg(*(yyvsp[-2]._str)).arg(*(yyvsp[0]._str));
           yyval._str = tmp;
	;
    break;}
case 39:
#line 269 "yacc.yy"
{
		QString* tmp = new QString( "    <SUPER>%1</SUPER>\n" );
		*tmp = tmp->arg( *(yyvsp[0]._str) );
		yyval._str = tmp;
	  ;
    break;}
case 40:
#line 277 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 41:
#line 281 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 42:
#line 288 "yacc.yy"
{
		yyval._str = yyvsp[-1]._str;
	  ;
    break;}
case 43:
#line 292 "yacc.yy"
{
		/* $$ = $1; */
		yyval._str = new QString( *(yyvsp[-2]._str) + *(yyvsp[0]._str) );
	  ;
    break;}
case 44:
#line 300 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 45:
#line 304 "yacc.yy"
{
		yyval._str = new QString( "" );
	  ;
    break;}
case 46:
#line 311 "yacc.yy"
{
          ;
    break;}
case 48:
#line 318 "yacc.yy"
{
		yyval._str = new QString( "" );
	  ;
    break;}
case 49:
#line 322 "yacc.yy"
{
		yyval._str = new QString( *(yyvsp[-1]._str) + *(yyvsp[0]._str) );
	  ;
    break;}
case 50:
#line 326 "yacc.yy"
{
		yyval._str = new QString( *(yyvsp[-1]._str) + *(yyvsp[0]._str) );
	  ;
    break;}
case 51:
#line 330 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 52:
#line 334 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 53:
#line 338 "yacc.yy"
{	
	        yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 54:
#line 342 "yacc.yy"
{
 	        yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 55:
#line 346 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 56:
#line 350 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 57:
#line 354 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 58:
#line 358 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 59:
#line 362 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 60:
#line 366 "yacc.yy"
{
                yyval._str = yyvsp[0]._str;
          ;
    break;}
case 67:
#line 381 "yacc.yy"
{;
    break;}
case 68:
#line 382 "yacc.yy"
{;
    break;}
case 69:
#line 385 "yacc.yy"
{;
    break;}
case 70:
#line 386 "yacc.yy"
{;
    break;}
case 71:
#line 387 "yacc.yy"
{;
    break;}
case 72:
#line 388 "yacc.yy"
{;
    break;}
case 73:
#line 389 "yacc.yy"
{;
    break;}
case 74:
#line 392 "yacc.yy"
{;
    break;}
case 75:
#line 393 "yacc.yy"
{;
    break;}
case 76:
#line 397 "yacc.yy"
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
#line 407 "yacc.yy"
{
		if (dcop_area)
		  yyerror("scoped template typedefs are not supported in dcop areas!");
	  ;
    break;}
case 78:
#line 412 "yacc.yy"
{
	  ;
    break;}
case 79:
#line 417 "yacc.yy"
{
		yyval._int = 0;
	  ;
    break;}
case 80:
#line 421 "yacc.yy"
{
		yyval._int = 1;
	  ;
    break;}
case 81:
#line 427 "yacc.yy"
{ yyval._str = new QString("signed int"); ;
    break;}
case 82:
#line 428 "yacc.yy"
{ yyval._str = new QString("unsigned int"); ;
    break;}
case 83:
#line 429 "yacc.yy"
{ yyval._str = new QString("signed short int"); ;
    break;}
case 84:
#line 430 "yacc.yy"
{ yyval._str = new QString("signed long int"); ;
    break;}
case 85:
#line 431 "yacc.yy"
{ yyval._str = new QString("signed short int"); ;
    break;}
case 86:
#line 432 "yacc.yy"
{ yyval._str = new QString("unsigned short int"); ;
    break;}
case 87:
#line 433 "yacc.yy"
{ yyval._str = new QString("unsigned long int"); ;
    break;}
case 88:
#line 434 "yacc.yy"
{ yyval._str = new QString("unsigned short int"); ;
    break;}
case 89:
#line 435 "yacc.yy"
{ yyval._str = new QString("unsigned int"); ;
    break;}
case 90:
#line 436 "yacc.yy"
{ yyval._str = new QString("int"); ;
    break;}
case 91:
#line 437 "yacc.yy"
{ yyval._str = new QString("long int"); ;
    break;}
case 92:
#line 438 "yacc.yy"
{ yyval._str = new QString("short int"); ;
    break;}
case 93:
#line 439 "yacc.yy"
{ yyval._str = new QString("char"); ;
    break;}
case 94:
#line 440 "yacc.yy"
{ yyval._str = new QString("signed char"); ;
    break;}
case 95:
#line 441 "yacc.yy"
{ yyval._str = new QString("unsigned char"); ;
    break;}
case 98:
#line 449 "yacc.yy"
{
		yyval._str = new QString( "" );
	  ;
    break;}
case 100:
#line 454 "yacc.yy"
{
		yyval._str = new QString( *(yyvsp[-2]._str) + *(yyvsp[0]._str) );
	  ;
    break;}
case 102:
#line 462 "yacc.yy"
{ yyval._str = yyvsp[0]._str; ;
    break;}
case 103:
#line 463 "yacc.yy"
{ yyval._str = yyvsp[0]._str; ;
    break;}
case 104:
#line 464 "yacc.yy"
{ yyval._str = yyvsp[0]._str; ;
    break;}
case 105:
#line 465 "yacc.yy"
{
		QString *tmp = new QString("%1&lt;%2&gt;");
		*tmp = tmp->arg(*(yyvsp[-3]._str));
		*tmp = tmp->arg(*(yyvsp[-1]._str));
		yyval._str = tmp;
	 ;
    break;}
case 106:
#line 471 "yacc.yy"
{
		QString *tmp = new QString("%1&lt;%2&gt;::%3");
		*tmp = tmp->arg(*(yyvsp[-5]._str));
		*tmp = tmp->arg(*(yyvsp[-3]._str));
		*tmp = tmp->arg(*(yyvsp[0]._str));
		yyval._str = tmp;
	 ;
    break;}
case 107:
#line 482 "yacc.yy"
{
	    yyval._str = new QString(*(yyvsp[-2]._str) + "," + *(yyvsp[0]._str));
	  ;
    break;}
case 108:
#line 486 "yacc.yy"
{
 	    yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 109:
#line 493 "yacc.yy"
{
	    if (dcop_area)
	      yyerror("in dcop areas are no pointers allowed");
	  ;
    break;}
case 110:
#line 498 "yacc.yy"
{
 	    yyval._str = yyvsp[0]._str;
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
#line 510 "yacc.yy"
{
	     if (dcop_area) {
	  	QString* tmp = new QString("<TYPE  qleft=\"const\" qright=\"" AMP_ENTITY "\">%1</TYPE>");
		*tmp = tmp->arg( *(yyvsp[-1]._str) );
		yyval._str = tmp;
	     }
	  ;
    break;}
case 113:
#line 517 "yacc.yy"
{
		QString* tmp = new QString("<TYPE>%1</TYPE>");
		*tmp = tmp->arg( *(yyvsp[0]._str) );
		yyval._str = tmp;
	;
    break;}
case 114:
#line 522 "yacc.yy"
{
	     if (dcop_area)
		yyerror("in dcop areas are only const references allowed!");
	  ;
    break;}
case 115:
#line 527 "yacc.yy"
{
		QString* tmp = new QString("<TYPE>%1</TYPE>");
		*tmp = tmp->arg( *(yyvsp[0]._str) );
		yyval._str = tmp;
	;
    break;}
case 116:
#line 533 "yacc.yy"
{
	    if (dcop_area)
	      yyerror("in dcop areas are no pointers allowed");
	  ;
    break;}
case 117:
#line 541 "yacc.yy"
{
	    yyval._str = new QString(*(yyvsp[-2]._str) + "," + *(yyvsp[0]._str));
	  ;
    break;}
case 118:
#line 545 "yacc.yy"
{
 	    yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 119:
#line 551 "yacc.yy"
{
		if (dcop_area) {
		   QString* tmp = new QString("\n        <ARG>%1<NAME>%2</NAME></ARG>");
  		   *tmp = tmp->arg( *(yyvsp[-2]._str) );
  		   *tmp = tmp->arg( *(yyvsp[-1]._str) );
		   yyval._str = tmp;		
		} else yyval._str = new QString();
	  ;
    break;}
case 120:
#line 560 "yacc.yy"
{
		if (dcop_area) {
		   QString* tmp = new QString("\n        <ARG>%1</ARG>");
  		   *tmp = tmp->arg( *(yyvsp[-1]._str) );
		   yyval._str = tmp;		
		} else yyval._str = new QString();
	  ;
    break;}
case 121:
#line 568 "yacc.yy"
{
		if (dcop_area)
			yyerror("variable arguments not supported in dcop area.");
		yyval._str = new QString("");
	  ;
    break;}
case 122:
#line 576 "yacc.yy"
{
	  ;
    break;}
case 123:
#line 579 "yacc.yy"
{
	  ;
    break;}
case 124:
#line 582 "yacc.yy"
{
	  ;
    break;}
case 125:
#line 588 "yacc.yy"
{
          ;
    break;}
case 126:
#line 591 "yacc.yy"
{
          ;
    break;}
case 127:
#line 594 "yacc.yy"
{
          ;
    break;}
case 128:
#line 597 "yacc.yy"
{
          ;
    break;}
case 129:
#line 600 "yacc.yy"
{
          ;
    break;}
case 130:
#line 605 "yacc.yy"
{ yyval._int = 0; ;
    break;}
case 131:
#line 606 "yacc.yy"
{ yyval._int = 1; ;
    break;}
case 136:
#line 613 "yacc.yy"
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
case 137:
#line 638 "yacc.yy"
{
	     if (dcop_area)
		yyerror("operators aren't allowed in dcop areas!");
	     yyval._str = new QString("");
	  ;
    break;}
case 138:
#line 645 "yacc.yy"
{;
    break;}
case 139:
#line 646 "yacc.yy"
{;
    break;}
case 140:
#line 647 "yacc.yy"
{;
    break;}
case 141:
#line 650 "yacc.yy"
{;
    break;}
case 142:
#line 651 "yacc.yy"
{;
    break;}
case 143:
#line 654 "yacc.yy"
{;
    break;}
case 144:
#line 657 "yacc.yy"
{;
    break;}
case 145:
#line 658 "yacc.yy"
{;
    break;}
case 146:
#line 662 "yacc.yy"
{
	        yyval._str = yyvsp[-1]._str;
	  ;
    break;}
case 147:
#line 666 "yacc.yy"
{
		yyval._str = yyvsp[-3]._str;
	  ;
    break;}
case 148:
#line 670 "yacc.yy"
{
		yyval._str = yyvsp[-1]._str;
	  ;
    break;}
case 149:
#line 674 "yacc.yy"
{
	      /* The constructor */
	      assert(!dcop_area);
              yyval._str = new QString("");
	  ;
    break;}
case 150:
#line 680 "yacc.yy"
{
	      /* The constructor */
	      assert(!dcop_area);
              yyval._str = new QString("");
	  ;
    break;}
case 151:
#line 686 "yacc.yy"
{
	      /* The destructor */
  	      assert(!dcop_area);
              yyval._str = new QString("");
	  ;
    break;}
case 152:
#line 692 "yacc.yy"
{
		if (dcop_area)
		  yyerror("static is not allowed in dcop area!");
		yyval._str = new QString();
	  ;
    break;}
case 153:
#line 699 "yacc.yy"
{
		function_mode = 1;
	;
    break;}
case 157:
#line 709 "yacc.yy"
{;
    break;}
case 158:
#line 710 "yacc.yy"
{;
    break;}
case 162:
#line 719 "yacc.yy"
{;
    break;}
case 163:
#line 720 "yacc.yy"
{;
    break;}
case 164:
#line 722 "yacc.yy"
{;
    break;}
case 165:
#line 725 "yacc.yy"
{;
    break;}
case 166:
#line 726 "yacc.yy"
{;
    break;}
case 167:
#line 727 "yacc.yy"
{;
    break;}
case 168:
#line 728 "yacc.yy"
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
#line 730 "yacc.yy"


void dcopidlParse( const char *_code )
{
    dcopidlInitFlex( _code );
    yyparse();
}
