
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
#define	T_PRIVATE	273
#define	T_VIRTUAL	274
#define	T_CONST	275
#define	T_RETURN	276
#define	T_SIGNAL	277
#define	T_SLOT	278
#define	T_TYPEDEF	279
#define	T_COMMA	280
#define	T_ASTERISK	281
#define	T_TILDE	282
#define	T_LESS	283
#define	T_GREATER	284
#define	T_AMPERSAND	285
#define	T_ACCESS	286
#define	T_ENUM	287
#define	T_UNKNOWN	288
#define	T_TRUE	289
#define	T_FALSE	290
#define	T_STATIC	291
#define	T_EQUAL	292
#define	T_SCOPE	293
#define	T_NULL	294
#define	T_DCOP	295
#define	T_DCOP_AREA	296
#define	T_SIGNED	297
#define	T_UNSIGNED	298

#line 1 "yacc.yy"


#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include <qstring.h>

#define AMP_ENTITY "&amp;"
#define YYERROR_VERBOSE

extern int yylex();

// extern QString idl_lexFile;
extern int idl_line_no;

static int dcop_area = 0;

void dcopidlInitFlex( const char *_code );

void yyerror( const char *s )
{
	qDebug( "In line %i : %s", idl_line_no, s );
        exit(1);
	//   theParser->parse_error( idl_lexFile, s, idl_line_no );
}


#line 31 "yacc.yy"
typedef union
{
  long   _int;
  QString        *_str;
  ushort          _char;
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



#define	YYFINAL		203
#define	YYFLAG		-32768
#define	YYNTBASE	45

#define YYTRANSLATE(x) ((unsigned)(x) <= 298 ? yytranslate[x] : 73)

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
    37,    38,    39,    40,    41,    42,    43,    44
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     3,     6,     7,    15,    22,    27,    32,    33,    35,
    37,    39,    41,    43,    45,    47,    48,    52,    55,    57,
    61,    63,    67,    70,    72,    75,    79,    82,    84,    86,
    89,    92,    95,    98,   101,   109,   111,   114,   118,   123,
   124,   126,   128,   132,   134,   136,   138,   141,   145,   150,
   157,   159,   166,   172,   175,   179,   186,   192,   193,   195,
   199,   205,   208,   212,   218,   225,   233,   237,   238,   241,
   244,   247,   250,   253,   256,   259,   270,   276,   282,   292,
   301,   309,   318,   320,   324,   329,   334,   341,   351,   357,
   358,   362
};

static const short yyrhs[] = {    46,
    47,     0,     8,    46,     0,     0,     9,    53,    57,    41,
    58,    16,    47,     0,     9,    53,    57,    58,    16,    47,
     0,     9,    53,    16,    45,     0,    10,    53,    16,    45,
     0,     0,    35,     0,    36,     0,    19,     0,    18,     0,
    17,     0,    23,     0,    24,     0,     0,    49,    50,    15,
     0,    42,    15,     0,     5,     0,     5,    39,    53,     0,
    53,     0,    20,    17,    54,     0,    17,    54,     0,    54,
     0,    55,    11,     0,    55,    26,    56,     0,    15,    56,
     0,    11,     0,    13,     0,    59,    58,     0,    69,    58,
     0,    52,    58,     0,    51,    58,     0,    72,    58,     0,
    25,    53,    29,    60,    30,    53,    16,     0,    53,     0,
    53,    27,     0,    53,    26,    60,     0,    53,    27,    26,
    60,     0,     0,    21,     0,    53,     0,    53,    26,    62,
     0,    43,     0,    44,     0,    53,     0,    63,    53,     0,
    21,    53,    31,     0,    53,    29,    62,    30,     0,    21,
    53,    29,    62,    30,    31,     0,    65,     0,    21,    53,
    29,    62,    30,    27,     0,    53,    29,    62,    30,    27,
     0,    53,    27,     0,    21,    53,    27,     0,    53,    29,
    62,    27,    30,    27,     0,    53,    29,    62,    27,    30,
     0,     0,    67,     0,    66,    26,    67,     0,    21,    53,
    31,    53,    68,     0,    53,    68,     0,    53,    53,    68,
     0,    53,    29,    62,    30,    53,     0,    53,    29,    62,
    30,    31,    53,     0,    21,    53,    29,    62,    30,    31,
    53,     0,    65,    53,    68,     0,     0,    38,     7,     0,
    38,     3,     0,    38,     4,     0,    38,     6,     0,    38,
    40,     0,    38,    53,     0,    38,    48,     0,    20,    64,
    53,    12,    66,    14,    61,    38,    40,    70,     0,    53,
    12,    66,    14,    70,     0,    28,    53,    12,    14,    70,
     0,    64,    53,    12,    66,    14,    61,    38,    40,    70,
     0,    20,    64,    53,    12,    66,    14,    61,    70,     0,
    64,    53,    12,    66,    14,    61,    70,     0,    37,    64,
    53,    12,    66,    14,    61,    70,     0,    16,     0,    11,
    71,    13,     0,    11,    71,    13,    16,     0,    22,    53,
    16,    71,     0,    22,    53,    32,     5,    16,    71,     0,
    22,    53,    32,     5,    12,    66,    14,    16,    71,     0,
     5,    38,     5,    16,    71,     0,     0,    64,     5,    16,
     0,    37,    64,     5,    16,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
   102,   109,   113,   119,   123,   126,   129,   132,   135,   135,
   137,   137,   137,   139,   139,   139,   142,   148,   154,   157,
   163,   173,   177,   181,   188,   192,   200,   204,   211,   215,
   219,   223,   227,   231,   237,   250,   256,   261,   267,   275,
   279,   286,   290,   297,   298,   301,   307,   313,   319,   325,
   331,   339,   340,   341,   342,   343,   344,   348,   352,   356,
   363,   370,   377,   383,   389,   395,   401,   409,   412,   415,
   418,   421,   424,   427,   430,   434,   446,   452,   458,   467,
   476,   488,   497,   498,   499,   502,   503,   504,   505,   506,
   510,   511
};
#endif


#if YYDEBUG != 0 || defined (YYERROR_VERBOSE)

static const char * const yytname[] = {   "$","error","$undefined.","T_CHARACTER_LITERAL",
"T_DOUBLE_LITERAL","T_IDENTIFIER","T_INTEGER_LITERAL","T_STRING_LITERAL","T_INCLUDE",
"T_CLASS","T_STRUCT","T_LEFT_CURLY_BRACKET","T_LEFT_PARANTHESIS","T_RIGHT_CURLY_BRACKET",
"T_RIGHT_PARANTHESIS","T_COLON","T_SEMICOLON","T_PUBLIC","T_PROTECTED","T_PRIVATE",
"T_VIRTUAL","T_CONST","T_RETURN","T_SIGNAL","T_SLOT","T_TYPEDEF","T_COMMA","T_ASTERISK",
"T_TILDE","T_LESS","T_GREATER","T_AMPERSAND","T_ACCESS","T_ENUM","T_UNKNOWN",
"T_TRUE","T_FALSE","T_STATIC","T_EQUAL","T_SCOPE","T_NULL","T_DCOP","T_DCOP_AREA",
"T_SIGNED","T_UNSIGNED","main","includes","rest","bool_value","nodcop_area",
"sigslot","nodcop_area_begin","dcop_area_begin","Identifier","super_class_name",
"super_class","super_classes","class_header","body","typedef","typedef_params",
"qualifier","return_params","prequalifier","return","pointer_type","params",
"param","default","function","function_body","function_lines","member", NULL
};
#endif

static const short yyr1[] = {     0,
    45,    46,    46,    47,    47,    47,    47,    47,    48,    48,
    49,    49,    49,    50,    50,    50,    51,    52,    53,    53,
    54,    55,    55,    55,    56,    56,    57,    57,    58,    58,
    58,    58,    58,    58,    59,    60,    60,    60,    60,    61,
    61,    62,    62,    63,    63,    64,    64,    64,    64,    64,
    64,    65,    65,    65,    65,    65,    65,    66,    66,    66,
    67,    67,    67,    67,    67,    67,    67,    68,    68,    68,
    68,    68,    68,    68,    68,    69,    69,    69,    69,    69,
    69,    69,    70,    70,    70,    71,    71,    71,    71,    71,
    72,    72
};

static const short yyr2[] = {     0,
     2,     2,     0,     7,     6,     4,     4,     0,     1,     1,
     1,     1,     1,     1,     1,     0,     3,     2,     1,     3,
     1,     3,     2,     1,     2,     3,     2,     1,     1,     2,
     2,     2,     2,     2,     7,     1,     2,     3,     4,     0,
     1,     1,     3,     1,     1,     1,     2,     3,     4,     6,
     1,     6,     5,     2,     3,     6,     5,     0,     1,     3,
     5,     2,     3,     5,     6,     7,     3,     0,     2,     2,
     2,     2,     2,     2,     2,    10,     5,     5,     9,     8,
     7,     8,     1,     3,     4,     4,     6,     9,     5,     0,
     3,     4
};

static const short yydefact[] = {     3,
     3,     8,     2,     0,     0,     1,    19,     0,     0,     0,
    28,     0,     3,     0,     3,    20,     0,     0,    21,    24,
     0,    27,     6,    29,    13,    12,    11,     0,     0,     0,
     0,     0,     0,     0,    44,    45,    16,     0,     0,    46,
     0,     0,     0,     0,    51,     0,     0,     7,    23,     0,
    25,     0,    46,     0,     0,     0,     0,     0,     0,    18,
    14,    15,     0,    33,    32,    58,    54,     0,     8,    30,
    47,    19,     0,    31,    34,    22,    26,     0,    55,     0,
    48,     0,     0,    19,     0,     8,    17,     0,    68,     0,
     0,    59,    42,     0,     5,    91,    58,    58,     0,    36,
     0,     0,    92,    58,     4,     0,     0,     0,    68,    62,
    68,     0,     0,     0,     0,    49,     0,     0,     0,     0,
    37,     0,    90,    83,    78,     0,     0,     0,     0,    70,
    71,    72,    69,     9,    10,    73,    75,    74,    63,    67,
    77,    60,    43,    57,    53,    40,    40,    52,    50,    38,
     0,     0,     0,     0,     0,    40,     0,    68,     0,    56,
    41,     0,     0,    39,    35,     0,     0,    84,     0,     0,
    61,     0,    64,     0,    81,     0,    80,     0,    90,     0,
    85,    82,     0,    65,     0,     0,    90,    86,     0,    66,
    79,    76,    89,    58,    90,     0,    87,     0,    90,    88,
     0,     0,     0
};

static const short yydefgoto[] = {    23,
     2,     6,   137,    37,    63,    38,    39,    40,    20,    21,
    22,    14,    41,    42,   101,   162,    94,    43,    44,    45,
    91,    92,   110,    46,   125,   155,    47
};

static const short yypact[] = {    23,
    23,   102,-32768,    58,    58,-32768,    31,   115,    82,    58,
-32768,    97,    23,   116,    23,-32768,    58,   134,-32768,-32768,
    71,-32768,-32768,-32768,-32768,-32768,-32768,     0,    58,    58,
    58,     0,   156,   157,-32768,-32768,   141,   156,   156,    78,
   162,   156,    58,   175,-32768,   156,   156,-32768,-32768,    58,
-32768,    97,   119,    58,   111,   153,   171,   186,   176,-32768,
-32768,-32768,   179,-32768,-32768,    12,-32768,    58,   102,-32768,
-32768,    -9,   183,-32768,-32768,-32768,-32768,   185,-32768,    58,
-32768,    58,   189,    21,   192,   102,-32768,    58,    42,    58,
     6,-32768,   180,    65,-32768,-32768,    12,    12,   177,   144,
   178,     7,-32768,    12,-32768,   125,    58,   182,   173,-32768,
   173,     7,    12,    58,   184,   188,    47,    87,   112,    58,
   187,    58,    52,-32768,-32768,    94,    58,    58,    95,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,   193,-32768,   191,   191,-32768,-32768,-32768,
    58,   200,   181,    58,   208,   191,   194,   173,    50,-32768,
-32768,     3,    18,-32768,-32768,   218,    36,   209,     7,   118,
-32768,    58,-32768,   190,-32768,   195,-32768,   210,    52,   222,
-32768,-32768,    58,-32768,     7,     7,    52,-32768,   150,-32768,
-32768,-32768,-32768,    12,    52,   101,-32768,   212,    52,-32768,
   229,   231,-32768
};

static const short yypgoto[] = {    91,
   232,    10,-32768,-32768,-32768,-32768,-32768,    -4,    -1,-32768,
   196,-32768,   163,-32768,   -98,   -28,   -69,-32768,   135,   -62,
   -95,   121,   -99,-32768,   -97,  -112,-32768
};


#define	YYLAST		248


static const short yytable[] = {     8,
     9,   117,   118,    90,     7,    16,    96,    19,   126,   139,
    99,   140,    19,   123,   141,    49,     7,   123,   124,   112,
    29,   150,   124,    53,    55,    56,    57,    53,   123,    10,
     1,   113,    88,   124,    90,    90,   103,   129,    71,    73,
   174,    90,    35,    36,   143,    19,     7,    19,    76,    78,
    90,   179,   164,    85,     7,   176,   153,   157,   171,    10,
   146,    89,     7,    93,   175,   177,   188,   180,    67,    10,
   107,   182,   113,   154,   193,    93,   145,   100,    95,   108,
   172,    51,   197,   106,   109,   111,   200,   191,   192,    66,
   201,   115,    89,    89,   116,   105,    52,    15,   196,    89,
   147,     7,    93,   138,    67,    48,    68,   156,    89,    93,
     4,     5,   113,    17,   198,   100,    18,   152,   163,   113,
     7,   115,    93,   158,   159,    11,   113,   169,    24,    12,
    13,    90,    25,    26,    27,    28,    29,    79,   148,    80,
    30,    81,   149,    31,   148,    67,   100,    68,   183,   167,
    50,    79,    32,   127,   173,   128,    33,    34,    35,    36,
     7,   194,    54,    61,    62,   195,    58,   184,    24,   120,
   121,    60,    25,    26,    27,    28,    29,    69,   190,    72,
    30,    82,    83,    31,   130,   131,     7,   132,   133,    89,
    84,    86,    32,    87,    97,    59,    98,    34,    35,    36,
    64,    65,   102,   104,    70,   114,   119,   122,    74,    75,
   108,   161,   151,   144,   145,   165,   134,   135,   166,   160,
   168,   136,   178,   170,   181,   187,   189,   199,   202,   185,
   203,     0,     3,   142,   186,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,    77
};

static const short yycheck[] = {     4,
     5,    97,    98,    66,     5,    10,    16,    12,   104,   109,
    80,   111,    17,    11,   112,    17,     5,    11,    16,    14,
    21,   120,    16,    28,    29,    30,    31,    32,    11,    39,
     8,    26,    21,    16,    97,    98,    16,   107,    43,    44,
    38,   104,    43,    44,   114,    50,     5,    52,    50,    54,
   113,    16,   151,    58,     5,    38,     5,   127,   158,    39,
    14,    66,     5,    68,   162,   163,   179,    32,    27,    39,
    29,   169,    26,    22,   187,    80,    27,    82,    69,    38,
    31,    11,   195,    88,    89,    90,   199,   185,   186,    12,
     0,    27,    97,    98,    30,    86,    26,    16,   194,   104,
    14,     5,   107,   108,    27,    15,    29,    14,   113,   114,
     9,    10,    26,    17,    14,   120,    20,   122,   147,    26,
     5,    27,   127,   128,    30,    11,    26,   156,    13,    15,
    16,   194,    17,    18,    19,    20,    21,    27,    27,    29,
    25,    31,    31,    28,    27,    27,   151,    29,    31,   154,
    17,    27,    37,    29,   159,    31,    41,    42,    43,    44,
     5,    12,    28,    23,    24,    16,    32,   172,    13,    26,
    27,    15,    17,    18,    19,    20,    21,    16,   183,     5,
    25,    29,    12,    28,     3,     4,     5,     6,     7,   194,
     5,    16,    37,    15,    12,    33,    12,    42,    43,    44,
    38,    39,    14,    12,    42,    26,    30,    30,    46,    47,
    38,    21,    26,    30,    27,    16,    35,    36,    38,    27,
    13,    40,     5,    30,    16,    16,     5,    16,     0,    40,
     0,    -1,     1,   113,    40,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    52
};
/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
#line 3 "/usr/share/misc/bison.simple"
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

#line 217 "/usr/share/misc/bison.simple"

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
#line 103 "yacc.yy"
{
	     dcop_area = 0; // reset
	  ;
    break;}
case 2:
#line 110 "yacc.yy"
{
		printf("<INCLUDE file=\"%s\"/>\n", yyvsp[-1]._str->latin1() );
	  ;
    break;}
case 3:
#line 114 "yacc.yy"
{
          ;
    break;}
case 4:
#line 120 "yacc.yy"
{
		printf("<CLASS name=\"%s\">\n%s%s</CLASS>\n", yyvsp[-5]._str->latin1(), yyvsp[-4]._str->latin1(), yyvsp[-2]._str->latin1() );
	  ;
    break;}
case 5:
#line 124 "yacc.yy"
{
	  ;
    break;}
case 6:
#line 127 "yacc.yy"
{
	  ;
    break;}
case 7:
#line 130 "yacc.yy"
{
	  ;
    break;}
case 8:
#line 132 "yacc.yy"
{;
    break;}
case 17:
#line 143 "yacc.yy"
{
	  dcop_area = 0;
	;
    break;}
case 18:
#line 149 "yacc.yy"
{
	  dcop_area = 1;
	;
    break;}
case 19:
#line 154 "yacc.yy"
{
	  yyval._str = yyvsp[0]._str;
	;
    break;}
case 20:
#line 157 "yacc.yy"
{
	   yyval._str = new QString( *(yyvsp[-2]._str) + *(yyvsp[0]._str) );
	;
    break;}
case 21:
#line 164 "yacc.yy"
{
		QString* tmp = new QString( "<SUPER name=\"%1\"/>\n" );
		*tmp = tmp->arg( *(yyvsp[0]._str) );
		yyval._str = tmp;
	  ;
    break;}
case 22:
#line 174 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 23:
#line 178 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 24:
#line 182 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 25:
#line 189 "yacc.yy"
{
		yyval._str = yyvsp[-1]._str;
	  ;
    break;}
case 26:
#line 193 "yacc.yy"
{
		/* $$ = $1; */
		yyval._str = new QString( *(yyvsp[-2]._str) + *(yyvsp[0]._str) );
	  ;
    break;}
case 27:
#line 201 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 28:
#line 205 "yacc.yy"
{
		yyval._str = new QString( "" );
	  ;
    break;}
case 29:
#line 212 "yacc.yy"
{
		yyval._str = new QString( "" );
	  ;
    break;}
case 30:
#line 216 "yacc.yy"
{
		yyval._str = new QString( *(yyvsp[-1]._str) + *(yyvsp[0]._str) );
	  ;
    break;}
case 31:
#line 220 "yacc.yy"
{
		yyval._str = new QString( *(yyvsp[-1]._str) + *(yyvsp[0]._str) );
	  ;
    break;}
case 32:
#line 224 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 33:
#line 228 "yacc.yy"
{	
	        yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 34:
#line 231 "yacc.yy"
{
 	        yyval._str = yyvsp[0]._str;
	;
    break;}
case 35:
#line 238 "yacc.yy"
{
		if (dcop_area) {
 		  QString* tmp = new QString("<TYPEDEF name=\"%1\" template=\"%2\">%3</TYPEDEF>\n");
		  *tmp = tmp->arg( *(yyvsp[-1]._str) ).arg( *(yyvsp[-5]._str) ).arg( *(yyvsp[-3]._str) );
		  yyval._str = tmp;
		} else {
		  yyval._str = new QString("");
		}
	  ;
    break;}
case 36:
#line 251 "yacc.yy"
{
		QString* tmp = new QString("<PARAM type=\"%1\"/>");
		*tmp = tmp->arg( *(yyvsp[0]._str) );
		yyval._str = tmp;
	  ;
    break;}
case 37:
#line 257 "yacc.yy"
{
		if (dcop_area)
		  yyerror("pointers are not allowed in dcop area!");
	  ;
    break;}
case 38:
#line 262 "yacc.yy"
{
		QString* tmp = new QString("<PARAM type=\"%1\"/>%2");
		*tmp = tmp->arg( *(yyvsp[-2]._str) ).arg( *(yyvsp[0]._str) );
		yyval._str = tmp;
	  ;
    break;}
case 39:
#line 268 "yacc.yy"
{
		if (dcop_area)
		  yyerror("pointers are not allowed in dcop area!");
	  ;
    break;}
case 40:
#line 276 "yacc.yy"
{
		yyval._str = new QString( "" );
	  ;
    break;}
case 41:
#line 280 "yacc.yy"
{
		yyval._str = new QString( "const" );
	  ;
    break;}
case 42:
#line 287 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 43:
#line 291 "yacc.yy"
{
		yyval._str = new QString( *(yyvsp[-2]._str) + "," + *(yyvsp[0]._str) );
	  ;
    break;}
case 44:
#line 297 "yacc.yy"
{ yyval._str = new QString("signed"); ;
    break;}
case 45:
#line 298 "yacc.yy"
{ yyval._str = new QString("unsigned"); ;
    break;}
case 46:
#line 302 "yacc.yy"
{
		QString* tmp = new QString("<RET type=\"%1\"/>");
		*tmp = tmp->arg( *(yyvsp[0]._str) );
		yyval._str = tmp;		
	  ;
    break;}
case 47:
#line 308 "yacc.yy"
{
		QString* tmp = new QString("<RET type=\"%1%2\"/>");
		*tmp = tmp->arg( *(yyvsp[-1]._str) ).arg( *(yyvsp[0]._str) );
		yyval._str = tmp;		
	  ;
    break;}
case 48:
#line 314 "yacc.yy"
{
		QString* tmp = new QString("<RET type=\"%1\" qleft=\"const\" qright=\"" AMP_ENTITY "\"/>");
		*tmp = tmp->arg( *(yyvsp[-1]._str) );
		yyval._str = tmp;		
	  ;
    break;}
case 49:
#line 320 "yacc.yy"
{
		QString* tmp = new QString("<RET type=\"%1<%2>\"/>");
		*tmp = tmp->arg( *(yyvsp[-3]._str) ).arg( *(yyvsp[-1]._str) );
		yyval._str = tmp;		
	  ;
    break;}
case 50:
#line 326 "yacc.yy"
{
		QString* tmp = new QString("<RET type=\"%1<%2>\" qleft=\"const\" qright=\"" AMP_ENTITY "\"/>");
		*tmp = tmp->arg( *(yyvsp[-4]._str) ).arg( *(yyvsp[-2]._str) );
		yyval._str = tmp;		
	  ;
    break;}
case 51:
#line 332 "yacc.yy"
{
	 	if (dcop_area)
	           yyerror("pointers are not allowed in kdcop areas!");
	  ;
    break;}
case 52:
#line 339 "yacc.yy"
{;
    break;}
case 53:
#line 340 "yacc.yy"
{;
    break;}
case 54:
#line 341 "yacc.yy"
{;
    break;}
case 55:
#line 342 "yacc.yy"
{;
    break;}
case 56:
#line 343 "yacc.yy"
{;
    break;}
case 57:
#line 344 "yacc.yy"
{;
    break;}
case 58:
#line 349 "yacc.yy"
{
		yyval._str = new QString( "" );
	  ;
    break;}
case 59:
#line 353 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 60:
#line 357 "yacc.yy"
{
		yyval._str = new QString( *(yyvsp[-2]._str) + *(yyvsp[0]._str) );
	  ;
    break;}
case 61:
#line 364 "yacc.yy"
{
		QString* tmp = new QString("<ARG name=\"%1\" type=\"%2\" qleft=\"const\" qright=\"" AMP_ENTITY "\"/>");
		*tmp = tmp->arg( *(yyvsp[-1]._str) );
		*tmp = tmp->arg( *(yyvsp[-3]._str) );
		yyval._str = tmp;		
	  ;
    break;}
case 62:
#line 371 "yacc.yy"
{
		if (dcop_area) {
		  yyerror("in dcoparea you have to specify paramater names!");
		}
		yyval._str = 0;
	  ;
    break;}
case 63:
#line 378 "yacc.yy"
{
		QString* tmp = new QString("<ARG name=\"%1\" type=\"%2\"/>");
		*tmp = tmp->arg( *(yyvsp[-1]._str) ).arg( *(yyvsp[-2]._str) );
		yyval._str = tmp;		
	  ;
    break;}
case 64:
#line 384 "yacc.yy"
{
		QString* tmp = new QString("<ARG name=\"%1\" type=\"%2<%3>\"/>");
		*tmp = tmp->arg( *(yyvsp[0]._str) ).arg( *(yyvsp[-4]._str) ).arg( *(yyvsp[-2]._str) );
		yyval._str = tmp;		
	  ;
    break;}
case 65:
#line 390 "yacc.yy"
{
		QString* tmp = new QString("<ARG name=\"%1\" type=\"%2<%3>\" qright=\"" AMP_ENTITY "\"/>");
		*tmp = tmp->arg( *(yyvsp[0]._str) ).arg( *(yyvsp[-5]._str) ).arg( *(yyvsp[-3]._str) );
		yyval._str = tmp;		
	  ;
    break;}
case 66:
#line 396 "yacc.yy"
{
		QString* tmp = new QString("<ARG name=\"%1\" type=\"%1<%2>\" qleft=\"const\" qright=\"" AMP_ENTITY "\"/>");
		*tmp = tmp->arg( *(yyvsp[0]._str) ).arg( *(yyvsp[-5]._str) ).arg( *(yyvsp[-3]._str) );
		yyval._str = tmp;		
	  ;
    break;}
case 67:
#line 401 "yacc.yy"
{
	       if (dcop_area)
	           yyerror("pointers are not allowed in kdcop areas!");
	       yyval._str = new QString("");
	;
    break;}
case 68:
#line 410 "yacc.yy"
{
	  ;
    break;}
case 69:
#line 413 "yacc.yy"
{
	  ;
    break;}
case 70:
#line 416 "yacc.yy"
{
	  ;
    break;}
case 71:
#line 419 "yacc.yy"
{
	  ;
    break;}
case 72:
#line 422 "yacc.yy"
{
	  ;
    break;}
case 73:
#line 425 "yacc.yy"
{
	  ;
    break;}
case 74:
#line 428 "yacc.yy"
{
	  ;
    break;}
case 76:
#line 435 "yacc.yy"
{
	     if (dcop_area) {
		QString* tmp = new QString("<FUNC name=\"%1\" qual=\"%4\">%2%3</FUNC>\n");
		*tmp = tmp->arg( *(yyvsp[-7]._str) );
		*tmp = tmp->arg( *(yyvsp[-8]._str) );
		*tmp = tmp->arg( *(yyvsp[-5]._str) );
		*tmp = tmp->arg( *(yyvsp[-3]._str) );
		yyval._str = tmp;
   	     } else
	        yyval._str = new QString("");
	  ;
    break;}
case 77:
#line 447 "yacc.yy"
{
	      /* The constructor */
	      assert(!dcop_area);
              yyval._str = new QString("");
	  ;
    break;}
case 78:
#line 453 "yacc.yy"
{
	      /* The destructor */
  	      assert(!dcop_area);
              yyval._str = new QString("");
	  ;
    break;}
case 79:
#line 459 "yacc.yy"
{
	     if (dcop_area) {
		QString* tmp = new QString("<FUNC name=\"%1\" qual=\"%4\">%2%3</FUNC>\n");
		*tmp = tmp->arg( *(yyvsp[-7]._str) ).arg( *(yyvsp[-8]._str) ).arg( *(yyvsp[-5]._str) ).arg( *(yyvsp[-3]._str) );
		yyval._str = tmp;
	     } else
	        yyval._str = new QString("");
	  ;
    break;}
case 80:
#line 468 "yacc.yy"
{
	     if (dcop_area) {
		QString* tmp = new QString("<FUNC name=\"%1\" qual=\"%4\">%2%3</FUNC>\n");
		*tmp = tmp->arg( *(yyvsp[-5]._str) ).arg( *(yyvsp[-6]._str) ).arg( *(yyvsp[-3]._str) ).arg( *(yyvsp[-1]._str) );
		yyval._str = tmp;
	     } else
	        yyval._str = new QString("");
	  ;
    break;}
case 81:
#line 477 "yacc.yy"
{
	     if (dcop_area) {
		QString* tmp = new QString("<FUNC name=\"%1\" qual=\"%4\">%2%3</FUNC>\n");
		*tmp = tmp->arg( *(yyvsp[-5]._str) );
		*tmp = tmp->arg( *(yyvsp[-6]._str) );
		*tmp = tmp->arg( *(yyvsp[-3]._str) );
		*tmp = tmp->arg( *(yyvsp[-1]._str) );
		yyval._str = tmp;
	     } else
	        yyval._str = new QString("");
	  ;
    break;}
case 82:
#line 489 "yacc.yy"
{
		if (dcop_area)
		  yyerror("static is not allowed in dcop area!");
		yyval._str = new QString();
	  ;
    break;}
case 86:
#line 502 "yacc.yy"
{;
    break;}
case 87:
#line 503 "yacc.yy"
{;
    break;}
case 88:
#line 504 "yacc.yy"
{;
    break;}
case 89:
#line 505 "yacc.yy"
{;
    break;}
case 90:
#line 506 "yacc.yy"
{;
    break;}
case 91:
#line 510 "yacc.yy"
{;
    break;}
case 92:
#line 511 "yacc.yy"
{;
    break;}
}
   /* the action file gets copied in in place of this dollarsign */
#line 543 "/usr/share/misc/bison.simple"

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
#line 513 "yacc.yy"


void dcopidlParse( const char *_code )
{
    printf("<!DOCTYPE DCOP-IDL><DCOP-IDL>\n");
    dcopidlInitFlex( _code );
    yyparse();
    printf("</DCOP-IDL>\n");
}
