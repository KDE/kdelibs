
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



#define	YYFINAL		350
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
   134,   136,   139,   143,   146,   148,   149,   151,   153,   156,
   159,   162,   165,   168,   171,   177,   182,   187,   192,   197,
   204,   212,   219,   226,   232,   236,   238,   242,   244,   246,
   248,   251,   253,   255,   257,   261,   265,   273,   283,   290,
   291,   293,   295,   298,   300,   303,   306,   310,   313,   317,
   320,   324,   327,   331,   333,   335,   338,   340,   343,   345,
   348,   351,   354,   356,   357,   359,   363,   365,   367,   370,
   373,   378,   385,   389,   391,   394,   396,   400,   404,   407,
   410,   412,   415,   419,   421,   425,   428,   430,   431,   434,
   440,   442,   444,   446,   448,   453,   454,   456,   458,   460,
   462,   464,   466,   473,   481,   483,   485,   489,   491,   495,
   500,   502,   506,   509,   515,   519,   525,   533,   540,   544,
   546,   548,   552,   557,   560,   561,   563,   566,   567,   569,
   573,   576,   579,   583,   589,   595
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
     0,     5,     0,     5,    49,    75,     0,    75,     0,   100,
    17,    76,     0,    76,     0,    77,    11,     0,    77,    31,
    78,     0,    15,    78,     0,    11,     0,     0,    16,     0,
    13,     0,    87,    81,     0,   107,    81,     0,    82,    81,
     0,    74,    81,     0,    73,    81,     0,   115,    81,     0,
    24,     9,    75,    16,    81,     0,    24,    75,    16,    81,
     0,    24,   102,    16,    81,     0,     9,    75,    16,    81,
     0,    10,    75,    16,    81,     0,    42,     5,    49,     5,
    16,    81,     0,    40,     5,    11,    83,    13,     5,    16,
     0,    40,     5,    11,    83,    13,    16,     0,    40,    11,
    83,    13,     5,    16,     0,    40,    11,    83,    13,    16,
     0,    84,    31,    83,     0,    84,     0,     5,    48,    86,
     0,     5,     0,     3,     0,     6,     0,    30,     6,     0,
    50,     0,    75,     0,    85,     0,    85,    29,    85,     0,
    85,    63,    85,     0,    28,    75,    34,    96,    35,    75,
    16,     0,    28,    75,    34,    96,    35,    49,     5,    75,
    16,     0,    10,    75,    11,    64,    13,    16,     0,     0,
    22,     0,    57,     0,    57,    51,     0,    58,     0,    58,
    51,     0,    57,    60,     0,    57,    60,    51,     0,    57,
    59,     0,    57,    59,    51,     0,    58,    60,     0,    58,
    60,    51,     0,    58,    59,     0,    58,    59,    51,     0,
    51,     0,    59,     0,    59,    51,     0,    60,     0,    60,
    51,     0,    54,     0,    57,    54,     0,    58,    54,     0,
    32,    90,     0,    32,     0,     0,    97,     0,    91,    31,
    97,     0,    89,     0,    75,     0,    10,    75,     0,     9,
    75,     0,    75,    34,    93,    35,     0,    75,    34,    93,
    35,    49,    75,     0,    94,    31,    93,     0,    94,     0,
    92,    90,     0,    92,     0,    22,    92,    90,     0,    22,
    92,    36,     0,    22,    92,     0,    92,    36,     0,    92,
     0,    92,    90,     0,    95,    31,    96,     0,    95,     0,
    95,    75,    98,     0,    95,    98,     0,    44,     0,     0,
    48,    99,     0,    48,    12,    95,    14,    99,     0,     7,
     0,    86,     0,     4,     0,    70,     0,    75,    12,    91,
    14,     0,     0,    21,     0,    62,     0,    63,     0,    35,
     0,    34,     0,    48,     0,    95,    75,    12,    91,    14,
    88,     0,    95,    61,   101,    12,    91,    14,    88,     0,
    85,     0,    70,     0,     5,    12,    14,     0,   103,     0,
   103,    31,   104,     0,     5,    12,   104,    14,     0,   105,
     0,   105,    31,   106,     0,   102,   109,     0,    21,   102,
    48,    50,   109,     0,    21,   102,   109,     0,    75,    12,
    91,    14,   109,     0,    75,    12,    91,    14,    15,   106,
   109,     0,   100,    33,    75,    12,    14,   109,     0,    47,
   102,   109,     0,    11,     0,    16,     0,   108,   110,    13,
     0,   108,   110,    13,    16,     0,   111,   110,     0,     0,
    16,     0,    31,   114,     0,     0,     5,     0,     5,    48,
    99,     0,    90,     5,     0,   113,   112,     0,    95,   114,
    16,     0,    95,    75,    15,     6,    16,     0,    47,    95,
     5,    98,    16,     0,    95,     5,    52,    86,    53,    16,
     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
   152,   155,   158,   162,   165,   171,   172,   175,   180,   183,
   186,   189,   193,   201,   204,   207,   210,   213,   216,   219,
   222,   225,   228,   233,   234,   236,   236,   238,   238,   238,
   240,   240,   240,   243,   247,   253,   259,   262,   269,   277,
   281,   288,   292,   300,   304,   311,   314,   318,   322,   326,
   330,   334,   338,   342,   346,   350,   354,   358,   362,   366,
   372,   373,   374,   375,   378,   379,   382,   383,   386,   387,
   388,   389,   390,   393,   394,   395,   398,   408,   413,   418,
   422,   429,   430,   431,   432,   433,   434,   435,   436,   437,
   438,   439,   440,   441,   442,   443,   444,   445,   446,   447,
   448,   451,   452,   455,   459,   460,   468,   469,   470,   471,
   472,   478,   488,   492,   499,   504,   512,   517,   524,   529,
   534,   539,   547,   551,   557,   566,   574,   582,   585,   588,
   594,   597,   600,   603,   606,   612,   613,   616,   616,   616,
   616,   616,   619,   644,   652,   653,   654,   657,   658,   661,
   664,   665,   668,   672,   676,   680,   686,   692,   698,   705,
   711,   712,   713,   716,   717,   720,   723,   724,   726,   727,
   728,   730,   733,   734,   735,   736
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
    72,    72,    72,    73,    73,    74,    75,    75,    76,    77,
    77,    78,    78,    79,    79,    80,    80,    81,    81,    81,
    81,    81,    81,    81,    81,    81,    81,    81,    81,    81,
    82,    82,    82,    82,    83,    83,    84,    84,    85,    85,
    85,    85,    85,    86,    86,    86,    87,    87,    87,    88,
    88,    89,    89,    89,    89,    89,    89,    89,    89,    89,
    89,    89,    89,    89,    89,    89,    89,    89,    89,    89,
    89,    90,    90,    91,    91,    91,    92,    92,    92,    92,
    92,    92,    93,    93,    94,    94,    95,    95,    95,    95,
    95,    95,    96,    96,    97,    97,    97,    98,    98,    98,
    99,    99,    99,    99,    99,   100,   100,   101,   101,   101,
   101,   101,   102,   102,   103,   103,   103,   104,   104,   105,
   106,   106,   107,   107,   107,   107,   107,   107,   107,   108,
   109,   109,   109,   110,   110,   111,   112,   112,   113,   113,
   113,   114,   115,   115,   115,   115
};

static const short yyr2[] = {     0,
     3,     0,     2,     4,     0,     1,     0,     6,     3,     3,
     6,     0,     7,     4,     5,     2,     4,     7,     8,     2,
     1,     1,     1,     2,     0,     1,     1,     1,     1,     1,
     1,     1,     0,     3,     2,     2,     1,     3,     1,     3,
     1,     2,     3,     2,     1,     0,     1,     1,     2,     2,
     2,     2,     2,     2,     5,     4,     4,     4,     4,     6,
     7,     6,     6,     5,     3,     1,     3,     1,     1,     1,
     2,     1,     1,     1,     3,     3,     7,     9,     6,     0,
     1,     1,     2,     1,     2,     2,     3,     2,     3,     2,
     3,     2,     3,     1,     1,     2,     1,     2,     1,     2,
     2,     2,     1,     0,     1,     3,     1,     1,     2,     2,
     4,     6,     3,     1,     2,     1,     3,     3,     2,     2,
     1,     2,     3,     1,     3,     2,     1,     0,     2,     5,
     1,     1,     1,     1,     4,     0,     1,     1,     1,     1,
     1,     1,     6,     7,     1,     1,     3,     1,     3,     4,
     1,     3,     2,     5,     3,     5,     7,     6,     3,     1,
     1,     3,     4,     2,     0,     1,     2,     0,     1,     3,
     2,     2,     3,     5,     5,     6
};

static const short yydefact[] = {     5,
     5,     0,   136,     3,     5,    37,     0,     0,   137,     0,
   136,     0,     0,     0,     0,     0,     0,    94,    99,    82,
    84,    95,    97,     5,   108,    23,   107,   121,     0,     0,
     0,    21,    22,     0,     0,   110,   109,     0,     0,   108,
     0,     0,   119,     0,    20,     0,     0,    16,     0,     0,
     0,     0,     0,     0,     0,    83,   100,    88,    86,    85,
   101,    92,    90,    96,    98,     1,   104,     0,   103,   120,
   122,    37,     0,     0,     0,   168,     0,     0,   160,   161,
   165,   153,     4,    38,    45,   136,     9,     7,     5,    10,
   110,   109,     0,     0,   155,   118,   117,    25,   109,     0,
     0,    68,     0,    66,    12,     0,     0,    37,   159,    89,
    87,    93,    91,   127,     0,   128,   105,   116,     0,   114,
   102,     0,     0,   141,   140,   142,   138,   139,     0,   104,
     0,   171,     0,   172,   173,     0,   166,     0,   165,   137,
    39,    41,     0,    44,     0,     6,    33,     0,     0,     0,
     0,     0,    25,    25,    17,     0,     0,     0,     0,     5,
     0,    14,     0,     0,     0,     0,   128,   126,   115,   111,
     0,    69,   133,    70,   131,     0,    26,    27,    72,   134,
    73,    74,   132,   170,    73,     0,   104,     0,     0,   169,
   167,     0,   162,   164,    42,   136,     0,     0,     0,    48,
    30,    29,    28,     0,    31,    32,     0,     0,     0,    33,
     0,    33,    33,     0,    33,    33,    33,    33,     0,   154,
     0,     0,     0,    24,     0,     0,    67,     0,    64,    65,
     0,    15,     0,   129,   175,     0,   156,   106,   125,     0,
   113,    71,   104,     0,     0,     0,     0,    80,   174,     0,
   163,    43,    40,   110,   109,     0,   108,     0,     0,     0,
    36,     0,    35,    53,    52,     8,    51,    49,    50,    54,
    11,   128,     0,     0,     0,    62,    63,    46,     0,     0,
   151,     0,   112,     0,    75,    76,   176,    80,    81,   143,
   158,    33,     5,    33,   110,    33,    33,     0,     0,    34,
    18,     0,    61,    47,    13,     0,     0,     0,   157,   135,
   144,    58,     0,    59,    33,    56,    57,   124,     0,     0,
    19,   130,    37,   146,   145,   148,     0,   152,     0,    55,
     0,     0,    33,     0,     0,   150,    79,   123,     0,     0,
    60,   147,   149,     0,    77,     0,    78,     0,     0,     0
};

static const short yydefgoto[] = {    34,
     3,   147,    24,   160,   151,   180,   210,   211,   212,   213,
    40,   142,   143,   144,    88,   305,   214,   215,   103,   104,
   182,   183,   216,   290,    27,    75,   115,    28,   119,   120,
    29,   319,   117,   164,   184,    30,   129,    31,   326,   327,
   281,   282,   217,    81,    82,   138,   139,   134,    76,    77,
   218
};

static const short yypact[] = {    52,
    42,     7,   315,-32768,    83,    60,   134,   134,   198,   393,
    77,   355,    41,   154,   146,    48,   198,-32768,-32768,   171,
   200,   119,   166,    55,   106,-32768,-32768,    65,    15,   193,
   172,-32768,-32768,   179,   134,   218,   205,   134,   134,   202,
    12,    29,   229,   198,-32768,   180,   134,-32768,   227,   241,
   255,   221,   262,    20,   172,-32768,-32768,   222,   223,-32768,
-32768,   224,   231,-32768,-32768,-32768,   120,   393,   246,-32768,
-32768,   274,   113,   157,   279,   257,   286,   134,-32768,-32768,
   291,-32768,-32768,-32768,-32768,   122,-32768,   240,    83,-32768,
-32768,-32768,   296,   261,-32768,-32768,-32768,   337,   298,   305,
   241,   266,   314,   297,-32768,   326,   318,    46,-32768,-32768,
-32768,-32768,-32768,-32768,   127,    24,-32768,   246,   300,   308,
-32768,   451,    28,-32768,-32768,-32768,-32768,-32768,   328,   120,
   338,-32768,   118,-32768,-32768,   333,-32768,   335,   291,-32768,
-32768,-32768,    30,-32768,   332,-32768,   259,   340,   172,   198,
   341,   121,   337,   337,-32768,   345,    28,    99,   241,    83,
   334,-32768,   429,   347,   232,   120,   303,-32768,-32768,   312,
   393,-32768,-32768,-32768,-32768,   361,-32768,-32768,-32768,-32768,
   356,     8,-32768,-32768,-32768,   317,   120,   159,   362,   323,
-32768,   365,   366,-32768,-32768,   122,   134,   134,   134,-32768,
-32768,-32768,-32768,   371,-32768,-32768,   134,   378,   370,   236,
   372,   259,   259,   373,   259,   259,   259,   259,   374,-32768,
   381,   134,   377,-32768,   386,   141,-32768,   384,-32768,-32768,
   388,-32768,   198,-32768,-32768,   399,-32768,-32768,-32768,   134,
-32768,-32768,   120,    28,    28,   389,   167,   385,-32768,   172,
-32768,-32768,-32768,   392,   216,   134,    33,   394,   382,   368,
-32768,   396,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,   303,   402,   134,   403,-32768,-32768,   404,   407,   411,
   395,   172,-32768,   197,-32768,-32768,-32768,   385,-32768,-32768,
-32768,   259,    83,   259,   408,   259,   259,   198,   422,-32768,
-32768,   421,-32768,-32768,-32768,   451,   194,   399,-32768,-32768,
-32768,-32768,   425,-32768,   259,-32768,-32768,   409,   410,   423,
-32768,-32768,    36,-32768,-32768,   412,   428,-32768,   430,-32768,
   198,    25,   259,   434,   194,-32768,-32768,-32768,   444,   445,
-32768,-32768,-32768,   134,-32768,   446,-32768,   460,   463,-32768
};

static const short yypgoto[] = {     3,
   464,-32768,-32768,-32768,   144,  -197,-32768,   254,-32768,-32768,
    -3,   269,-32768,   271,-32768,-32768,  -110,   465,   -89,-32768,
  -223,   -69,-32768,   181,-32768,    -4,  -121,    -9,   299,-32768,
     2,   140,   306,   -88,  -161,   -80,-32768,    -2,-32768,   138,
-32768,   168,    53,-32768,   -32,   339,-32768,-32768,-32768,   344,
    13
};


#define	YYLAST		501


static const short yytable[] = {    25,
    43,   234,   348,    36,    37,   145,    42,    25,   188,    95,
    41,   156,    41,    47,    55,    33,     6,     5,    54,    72,
   285,   286,   109,    71,   108,    74,    66,   168,     6,     6,
   172,    84,     6,   174,    91,    92,   244,    93,    97,    79,
   195,    55,    99,   100,    80,    41,    69,   334,   296,     1,
    93,    -2,    52,   186,    -2,    32,    48,   176,   118,     1,
   196,  -128,     1,    45,   121,   247,    68,    -2,   116,   230,
   245,   163,    73,   339,   136,    73,    94,   179,   239,     2,
    73,     6,   141,   325,    35,    38,    39,   227,    53,     2,
     1,   148,     2,   163,    35,    -2,    69,     9,    10,   152,
    70,   264,   265,   228,   267,   268,   269,   270,    35,   324,
   153,   325,   167,   169,   229,   145,   220,    67,   181,   185,
     2,   284,   190,    44,     6,    72,     6,    18,    38,    39,
    19,   116,   237,    20,    21,    22,    23,   324,     6,    68,
   165,    10,   140,    25,   322,   275,   124,   125,   223,    69,
    51,   221,    69,   185,   152,   152,   276,   166,    49,   181,
   126,   118,   231,   114,    50,   153,   153,   116,   130,    64,
    18,   131,   248,    19,   127,   128,    20,    21,    22,    23,
   288,   312,    79,   314,     6,   316,   317,    80,   116,   166,
    98,    83,   141,   141,   254,   255,   172,   166,   323,   174,
   257,   258,     6,   259,   330,    41,    38,    39,    25,    25,
   310,    25,    25,    25,    25,    89,    65,   291,   273,    10,
    90,    56,   341,   176,    57,    78,   293,   166,    85,    58,
    59,   294,    86,    87,   279,    68,   283,   101,   177,   178,
   185,   185,    79,   179,   116,   102,   236,    80,    18,   309,
    60,    19,   295,    61,    20,    21,    22,    23,    62,    63,
    69,   205,   206,     6,    96,   105,   107,   198,   199,   106,
   302,   200,   110,   111,   112,   201,   202,    69,   203,     9,
    10,   113,   204,   132,   205,   206,   207,   133,    25,  -169,
    25,  -136,    25,    25,   146,   313,   224,   225,    14,   318,
   208,   135,   181,   185,  -169,    17,   137,   130,   154,    18,
   149,    25,    19,   157,   209,    20,    21,    22,    23,     6,
   155,   122,    35,     7,     8,   123,   158,   159,   340,    25,
   161,   185,   318,   162,   170,     9,    10,    11,   171,   187,
   346,     6,    12,   189,   192,    38,    39,   193,   197,   232,
   163,    13,   219,   222,    14,    15,    16,   226,    10,     6,
   240,    17,   235,    38,    46,    18,   242,   243,    19,   246,
   122,    20,    21,    22,    23,     6,    10,   249,   250,   256,
    39,   251,   260,   150,   261,   272,   263,    18,   266,   271,
    19,   131,    10,    20,    21,    22,    23,     6,   274,   277,
   278,    38,    39,   280,   287,    18,   289,   292,    19,   297,
   300,    20,    21,    22,    23,   298,   299,   301,   303,   304,
   306,    18,   307,   315,    19,   308,   320,    20,    21,    22,
    23,   172,   173,     6,   174,   175,   321,   329,   333,   331,
   233,   336,   335,    18,   332,   337,    19,   342,   344,    20,
    21,    22,    23,   172,   173,     6,   174,   175,   176,   349,
   345,   347,   350,   262,     4,   253,   252,    26,   311,   241,
   338,   238,   343,   177,   178,   328,   191,   194,   179,     0,
   176,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,   177,   178,     0,     0,     0,
   179
};

static const short yycheck[] = {     3,
    10,   163,     0,     7,     8,    86,     9,    11,   130,    42,
     9,   101,    11,    12,    17,     3,     5,    11,    17,     5,
   244,   245,    55,    28,     5,    29,    24,   116,     5,     5,
     3,    35,     5,     6,    38,    39,    29,    41,    43,    11,
    11,    44,    46,    47,    16,    44,    32,    12,    16,     8,
    54,     0,     5,   123,     0,     3,    16,    30,    68,     8,
    31,    16,     8,    11,    69,   187,    34,    13,    67,   159,
    63,    48,    61,    49,    78,    61,    48,    50,   167,    38,
    61,     5,    86,   307,    49,     9,    10,   157,    41,    38,
     8,    89,    38,    48,    49,    13,    32,    21,    22,    98,
    36,   212,   213,     5,   215,   216,   217,   218,    49,   307,
    98,   335,   116,   118,    16,   196,   149,    12,   122,   123,
    38,   243,     5,    47,     5,     5,     5,    51,     9,    10,
    54,   130,   165,    57,    58,    59,    60,   335,     5,    34,
    14,    22,    21,   147,   306,     5,    34,    35,   152,    32,
     5,   150,    32,   157,   153,   154,    16,    31,     5,   163,
    48,   171,   160,    44,    11,   153,   154,   166,    12,    51,
    51,    15,    14,    54,    62,    63,    57,    58,    59,    60,
    14,   292,    11,   294,     5,   296,   297,    16,   187,    31,
    11,    13,   196,   197,   198,   199,     3,    31,     5,     6,
   204,   204,     5,   207,   315,   204,     9,    10,   212,   213,
    14,   215,   216,   217,   218,    11,    51,   250,   222,    22,
    16,    51,   333,    30,    54,    33,    11,    31,    11,    59,
    60,    16,    15,    16,   233,    34,   240,    11,    45,    46,
   244,   245,    11,    50,   243,     5,    15,    16,    51,   282,
    51,    54,   256,    54,    57,    58,    59,    60,    59,    60,
    32,    26,    27,     5,    36,    11,     5,     9,    10,    49,
   274,    13,    51,    51,    51,    17,    18,    32,    20,    21,
    22,    51,    24,     5,    26,    27,    28,    31,   292,    16,
   294,    33,   296,   297,    55,   293,   153,   154,    40,   298,
    42,    16,   306,   307,    31,    47,    16,    12,    11,    51,
    50,   315,    54,    48,    56,    57,    58,    59,    60,     5,
    16,    48,    49,     9,    10,    52,    13,    31,   332,   333,
     5,   335,   331,    16,    35,    21,    22,    23,    31,    12,
   344,     5,    28,     6,    12,     9,    10,    13,    17,    16,
    48,    37,    13,    13,    40,    41,    42,    13,    22,     5,
    49,    47,    16,     9,    10,    51,     6,    12,    54,    53,
    48,    57,    58,    59,    60,     5,    22,    16,    14,     9,
    10,    16,     5,    47,    15,     5,    15,    51,    16,    16,
    54,    15,    22,    57,    58,    59,    60,     5,    13,    16,
    13,     9,    10,     5,    16,    51,    22,    16,    54,    16,
    15,    57,    58,    59,    60,    34,    49,    16,    16,    16,
    14,    51,    12,    16,    54,    31,     5,    57,    58,    59,
    60,     3,     4,     5,     6,     7,    16,    13,    16,    31,
    12,    14,    31,    51,    35,    16,    54,    14,     5,    57,
    58,    59,    60,     3,     4,     5,     6,     7,    30,     0,
    16,    16,     0,   210,     1,   197,   196,     3,   288,   171,
   331,   166,   335,    45,    46,   308,   133,   139,    50,    -1,
    30,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    45,    46,    -1,    -1,    -1,
    50
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
#line 278 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 41:
#line 282 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 42:
#line 289 "yacc.yy"
{
		yyval._str = yyvsp[-1]._str;
	  ;
    break;}
case 43:
#line 293 "yacc.yy"
{
		/* $$ = $1; */
		yyval._str = new QString( *(yyvsp[-2]._str) + *(yyvsp[0]._str) );
	  ;
    break;}
case 44:
#line 301 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 45:
#line 305 "yacc.yy"
{
		yyval._str = new QString( "" );
	  ;
    break;}
case 46:
#line 312 "yacc.yy"
{
          ;
    break;}
case 48:
#line 319 "yacc.yy"
{
		yyval._str = new QString( "" );
	  ;
    break;}
case 49:
#line 323 "yacc.yy"
{
		yyval._str = new QString( *(yyvsp[-1]._str) + *(yyvsp[0]._str) );
	  ;
    break;}
case 50:
#line 327 "yacc.yy"
{
		yyval._str = new QString( *(yyvsp[-1]._str) + *(yyvsp[0]._str) );
	  ;
    break;}
case 51:
#line 331 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 52:
#line 335 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 53:
#line 339 "yacc.yy"
{	
	        yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 54:
#line 343 "yacc.yy"
{
 	        yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 55:
#line 347 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 56:
#line 351 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 57:
#line 355 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 58:
#line 359 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 59:
#line 363 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 60:
#line 367 "yacc.yy"
{
                yyval._str = yyvsp[0]._str;
          ;
    break;}
case 67:
#line 382 "yacc.yy"
{;
    break;}
case 68:
#line 383 "yacc.yy"
{;
    break;}
case 69:
#line 386 "yacc.yy"
{;
    break;}
case 70:
#line 387 "yacc.yy"
{;
    break;}
case 71:
#line 388 "yacc.yy"
{;
    break;}
case 72:
#line 389 "yacc.yy"
{;
    break;}
case 73:
#line 390 "yacc.yy"
{;
    break;}
case 74:
#line 393 "yacc.yy"
{;
    break;}
case 75:
#line 394 "yacc.yy"
{;
    break;}
case 76:
#line 395 "yacc.yy"
{;
    break;}
case 77:
#line 399 "yacc.yy"
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
case 78:
#line 409 "yacc.yy"
{
		if (dcop_area)
		  yyerror("scoped template typedefs are not supported in dcop areas!");
	  ;
    break;}
case 79:
#line 414 "yacc.yy"
{
	  ;
    break;}
case 80:
#line 419 "yacc.yy"
{
		yyval._int = 0;
	  ;
    break;}
case 81:
#line 423 "yacc.yy"
{
		yyval._int = 1;
	  ;
    break;}
case 82:
#line 429 "yacc.yy"
{ yyval._str = new QString("signed int"); ;
    break;}
case 83:
#line 430 "yacc.yy"
{ yyval._str = new QString("signed int"); ;
    break;}
case 84:
#line 431 "yacc.yy"
{ yyval._str = new QString("unsigned int"); ;
    break;}
case 85:
#line 432 "yacc.yy"
{ yyval._str = new QString("unsigned int"); ;
    break;}
case 86:
#line 433 "yacc.yy"
{ yyval._str = new QString("signed short int"); ;
    break;}
case 87:
#line 434 "yacc.yy"
{ yyval._str = new QString("signed short int"); ;
    break;}
case 88:
#line 435 "yacc.yy"
{ yyval._str = new QString("signed long int"); ;
    break;}
case 89:
#line 436 "yacc.yy"
{ yyval._str = new QString("signed long int"); ;
    break;}
case 90:
#line 437 "yacc.yy"
{ yyval._str = new QString("unsigned short int"); ;
    break;}
case 91:
#line 438 "yacc.yy"
{ yyval._str = new QString("unsigned short int"); ;
    break;}
case 92:
#line 439 "yacc.yy"
{ yyval._str = new QString("unsigned long int"); ;
    break;}
case 93:
#line 440 "yacc.yy"
{ yyval._str = new QString("unsigned long int"); ;
    break;}
case 94:
#line 441 "yacc.yy"
{ yyval._str = new QString("int"); ;
    break;}
case 95:
#line 442 "yacc.yy"
{ yyval._str = new QString("long int"); ;
    break;}
case 96:
#line 443 "yacc.yy"
{ yyval._str = new QString("long int"); ;
    break;}
case 97:
#line 444 "yacc.yy"
{ yyval._str = new QString("short int"); ;
    break;}
case 98:
#line 445 "yacc.yy"
{ yyval._str = new QString("short int"); ;
    break;}
case 99:
#line 446 "yacc.yy"
{ yyval._str = new QString("char"); ;
    break;}
case 100:
#line 447 "yacc.yy"
{ yyval._str = new QString("signed char"); ;
    break;}
case 101:
#line 448 "yacc.yy"
{ yyval._str = new QString("unsigned char"); ;
    break;}
case 104:
#line 456 "yacc.yy"
{
		yyval._str = new QString( "" );
	  ;
    break;}
case 106:
#line 461 "yacc.yy"
{
		yyval._str = new QString( *(yyvsp[-2]._str) + *(yyvsp[0]._str) );
	  ;
    break;}
case 108:
#line 469 "yacc.yy"
{ yyval._str = yyvsp[0]._str; ;
    break;}
case 109:
#line 470 "yacc.yy"
{ yyval._str = yyvsp[0]._str; ;
    break;}
case 110:
#line 471 "yacc.yy"
{ yyval._str = yyvsp[0]._str; ;
    break;}
case 111:
#line 472 "yacc.yy"
{
		QString *tmp = new QString("%1&lt;%2&gt;");
		*tmp = tmp->arg(*(yyvsp[-3]._str));
		*tmp = tmp->arg(*(yyvsp[-1]._str));
		yyval._str = tmp;
	 ;
    break;}
case 112:
#line 478 "yacc.yy"
{
		QString *tmp = new QString("%1&lt;%2&gt;::%3");
		*tmp = tmp->arg(*(yyvsp[-5]._str));
		*tmp = tmp->arg(*(yyvsp[-3]._str));
		*tmp = tmp->arg(*(yyvsp[0]._str));
		yyval._str = tmp;
	 ;
    break;}
case 113:
#line 489 "yacc.yy"
{
	    yyval._str = new QString(*(yyvsp[-2]._str) + "," + *(yyvsp[0]._str));
	  ;
    break;}
case 114:
#line 493 "yacc.yy"
{
 	    yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 115:
#line 500 "yacc.yy"
{
	    if (dcop_area)
	      yyerror("in dcop areas are no pointers allowed");
	  ;
    break;}
case 116:
#line 505 "yacc.yy"
{
 	    yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 117:
#line 513 "yacc.yy"
{
	    if (dcop_area)
	      yyerror("in dcop areas are no pointers allowed");
	  ;
    break;}
case 118:
#line 517 "yacc.yy"
{
	     if (dcop_area) {
	  	QString* tmp = new QString("<TYPE  qleft=\"const\" qright=\"" AMP_ENTITY "\">%1</TYPE>");
		*tmp = tmp->arg( *(yyvsp[-1]._str) );
		yyval._str = tmp;
	     }
	  ;
    break;}
case 119:
#line 524 "yacc.yy"
{
		QString* tmp = new QString("<TYPE>%1</TYPE>");
		*tmp = tmp->arg( *(yyvsp[0]._str) );
		yyval._str = tmp;
	;
    break;}
case 120:
#line 529 "yacc.yy"
{
	     if (dcop_area)
		yyerror("in dcop areas are only const references allowed!");
	  ;
    break;}
case 121:
#line 534 "yacc.yy"
{
		QString* tmp = new QString("<TYPE>%1</TYPE>");
		*tmp = tmp->arg( *(yyvsp[0]._str) );
		yyval._str = tmp;
	;
    break;}
case 122:
#line 540 "yacc.yy"
{
	    if (dcop_area)
	      yyerror("in dcop areas are no pointers allowed");
	  ;
    break;}
case 123:
#line 548 "yacc.yy"
{
	    yyval._str = new QString(*(yyvsp[-2]._str) + "," + *(yyvsp[0]._str));
	  ;
    break;}
case 124:
#line 552 "yacc.yy"
{
 	    yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 125:
#line 558 "yacc.yy"
{
		if (dcop_area) {
		   QString* tmp = new QString("\n        <ARG>%1<NAME>%2</NAME></ARG>");
  		   *tmp = tmp->arg( *(yyvsp[-2]._str) );
  		   *tmp = tmp->arg( *(yyvsp[-1]._str) );
		   yyval._str = tmp;		
		} else yyval._str = new QString();
	  ;
    break;}
case 126:
#line 567 "yacc.yy"
{
		if (dcop_area) {
		   QString* tmp = new QString("\n        <ARG>%1</ARG>");
  		   *tmp = tmp->arg( *(yyvsp[-1]._str) );
		   yyval._str = tmp;		
		} else yyval._str = new QString();
	  ;
    break;}
case 127:
#line 575 "yacc.yy"
{
		if (dcop_area)
			yyerror("variable arguments not supported in dcop area.");
		yyval._str = new QString("");
	  ;
    break;}
case 128:
#line 583 "yacc.yy"
{
	  ;
    break;}
case 129:
#line 586 "yacc.yy"
{
	  ;
    break;}
case 130:
#line 589 "yacc.yy"
{
	  ;
    break;}
case 131:
#line 595 "yacc.yy"
{
          ;
    break;}
case 132:
#line 598 "yacc.yy"
{
          ;
    break;}
case 133:
#line 601 "yacc.yy"
{
          ;
    break;}
case 134:
#line 604 "yacc.yy"
{
          ;
    break;}
case 135:
#line 607 "yacc.yy"
{
          ;
    break;}
case 136:
#line 612 "yacc.yy"
{ yyval._int = 0; ;
    break;}
case 137:
#line 613 "yacc.yy"
{ yyval._int = 1; ;
    break;}
case 143:
#line 620 "yacc.yy"
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
case 144:
#line 645 "yacc.yy"
{
	     if (dcop_area)
		yyerror("operators aren't allowed in dcop areas!");
	     yyval._str = new QString("");
	  ;
    break;}
case 145:
#line 652 "yacc.yy"
{;
    break;}
case 146:
#line 653 "yacc.yy"
{;
    break;}
case 147:
#line 654 "yacc.yy"
{;
    break;}
case 148:
#line 657 "yacc.yy"
{;
    break;}
case 149:
#line 658 "yacc.yy"
{;
    break;}
case 150:
#line 661 "yacc.yy"
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
case 153:
#line 669 "yacc.yy"
{
	        yyval._str = yyvsp[-1]._str;
	  ;
    break;}
case 154:
#line 673 "yacc.yy"
{
		yyval._str = yyvsp[-3]._str;
	  ;
    break;}
case 155:
#line 677 "yacc.yy"
{
		yyval._str = yyvsp[-1]._str;
	  ;
    break;}
case 156:
#line 681 "yacc.yy"
{
	      /* The constructor */
	      assert(!dcop_area);
              yyval._str = new QString("");
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
	      /* The destructor */
  	      assert(!dcop_area);
              yyval._str = new QString("");
	  ;
    break;}
case 159:
#line 699 "yacc.yy"
{
		if (dcop_area)
		  yyerror("static is not allowed in dcop area!");
		yyval._str = new QString();
	  ;
    break;}
case 160:
#line 706 "yacc.yy"
{
		function_mode = 1;
	;
    break;}
case 164:
#line 716 "yacc.yy"
{;
    break;}
case 165:
#line 717 "yacc.yy"
{;
    break;}
case 169:
#line 726 "yacc.yy"
{;
    break;}
case 170:
#line 727 "yacc.yy"
{;
    break;}
case 171:
#line 728 "yacc.yy"
{;
    break;}
case 172:
#line 730 "yacc.yy"
{;
    break;}
case 173:
#line 733 "yacc.yy"
{;
    break;}
case 174:
#line 734 "yacc.yy"
{;
    break;}
case 175:
#line 735 "yacc.yy"
{;
    break;}
case 176:
#line 736 "yacc.yy"
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
#line 738 "yacc.yy"


void dcopidlParse( const char *_code )
{
    dcopidlInitFlex( _code );
    yyparse();
}
