
/*  A Bison parser, made from yacc.yy
 by  GNU Bison version 1.25
  */

#define YYBISON 1  /* Identify Bison output.  */

#define	T_CHARACTER_LITERAL	258
#define	T_DOUBLE_LITERAL	259
#define	T_IDENTIFIER	260
#define	T_INTEGER_LITERAL	261
#define	T_STRING_LITERAL	262
#define	T_INCLUDE	263
#define	T_CLASS	264
#define	T_STRUCT	265
#define	T_LEFT_CURLY_BRACKET	266
#define	T_LEFT_PARANTHESIS	267
#define	T_RIGHT_CURLY_BRACKET	268
#define	T_RIGHT_PARANTHESIS	269
#define	T_COLON	270
#define	T_SEMICOLON	271
#define	T_PUBLIC	272
#define	T_PROTECTED	273
#define	T_TRIPE_DOT	274
#define	T_PRIVATE	275
#define	T_VIRTUAL	276
#define	T_CONST	277
#define	T_INLINE	278
#define	T_FRIEND	279
#define	T_RETURN	280
#define	T_SIGNAL	281
#define	T_SLOT	282
#define	T_TYPEDEF	283
#define	T_PLUS	284
#define	T_MINUS	285
#define	T_COMMA	286
#define	T_ASTERISK	287
#define	T_TILDE	288
#define	T_LESS	289
#define	T_GREATER	290
#define	T_AMPERSAND	291
#define	T_EXTERN	292
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
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

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

void dcopidlInitFlex( const char *_code );

void yyerror( const char *s )
{
	qDebug( "In line %i : %s", idl_line_no, s );
        exit(1);
	//   theParser->parse_error( idl_lexFile, s, idl_line_no );
}


#line 54 "yacc.yy"
typedef union
{
  long   _int;
  QString        *_str;
  unsigned short          _char;
  double _float;
} YYSTYPE;
#include <stdio.h>

#ifndef __cplusplus
#ifndef __STDC__
#define const
#endif
#endif



#define	YYFINAL		284
#define	YYFLAG		-32768
#define	YYNTBASE	61

#define YYTRANSLATE(x) ((unsigned)(x) <= 315 ? yytranslate[x] : 107)

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
     2,     2,     2,     2,     2,     1,     2,     3,     4,     5,
     6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
    16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
    26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
    36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
    46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
    56,    57,    58,    59,    60
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     4,     5,     8,    13,    14,    16,    17,    24,    28,
    32,    39,    46,    51,    59,    68,    71,    73,    75,    77,
    80,    81,    83,    85,    87,    89,    91,    93,    95,    96,
   100,   103,   106,   108,   112,   114,   118,   120,   123,   127,
   130,   132,   134,   137,   140,   143,   146,   149,   152,   158,
   163,   171,   178,   185,   189,   191,   195,   197,   199,   201,
   204,   206,   208,   210,   214,   222,   232,   233,   235,   237,
   239,   242,   245,   249,   252,   255,   259,   262,   264,   266,
   268,   270,   273,   276,   279,   281,   282,   284,   288,   290,
   292,   295,   298,   303,   310,   314,   318,   321,   324,   326,
   329,   333,   335,   339,   342,   344,   345,   348,   351,   354,
   357,   363,   364,   366,   368,   370,   372,   374,   381,   389,
   391,   393,   397,   402,   404,   408,   411,   417,   421,   427,
   435,   442,   446,   448,   450,   454,   459,   462,   463,   465,
   468,   469,   472,   476,   481
};

static const short yyrhs[] = {    62,
    64,    61,     0,     0,     8,    62,     0,    37,    11,    61,
    13,     0,     0,    53,     0,     0,     9,    71,    75,    63,
    76,    16,     0,     9,    71,    16,     0,    10,    71,    16,
     0,    10,    71,    11,    61,    13,    16,     0,    40,     5,
    11,    61,    13,    16,     0,    28,    88,    71,    16,     0,
    28,    10,    11,    65,    13,    71,    16,     0,    28,    10,
    71,    11,    65,    13,    71,    16,     0,    23,    99,     0,
    99,     0,   106,     0,    77,     0,   106,    65,     0,     0,
    43,     0,    44,     0,    20,     0,    18,     0,    17,     0,
    26,     0,    27,     0,     0,    67,    68,    15,     0,    68,
    15,     0,    54,    15,     0,     5,     0,     5,    47,    71,
     0,    71,     0,    92,    17,    72,     0,    72,     0,    73,
    11,     0,    73,    31,    74,     0,    15,    74,     0,    11,
     0,    13,     0,    82,    76,     0,    99,    76,     0,    77,
    76,     0,    70,    76,     0,    69,    76,     0,   106,    76,
     0,    24,     9,    71,    16,    76,     0,    24,    71,    16,
    76,     0,    39,     5,    11,    78,    13,     5,    16,     0,
    39,     5,    11,    78,    13,    16,     0,    39,    11,    78,
    13,     5,    16,     0,    79,    31,    78,     0,    79,     0,
     5,    46,    81,     0,     5,     0,     3,     0,     6,     0,
    30,     6,     0,    48,     0,    71,     0,    80,     0,    80,
    29,    80,     0,    28,    71,    34,    89,    35,    71,    16,
     0,    28,    71,    34,    89,    35,    47,     5,    71,    16,
     0,     0,    22,     0,    55,     0,    56,     0,    55,    58,
     0,    55,    57,     0,    55,    58,    49,     0,    56,    58,
     0,    56,    57,     0,    56,    58,    49,     0,    56,    49,
     0,    49,     0,    57,     0,    58,     0,    52,     0,    55,
    52,     0,    56,    52,     0,    32,    85,     0,    32,     0,
     0,    90,     0,    86,    31,    90,     0,    84,     0,    71,
     0,    10,    71,     0,     9,    71,     0,    71,    34,    89,
    35,     0,    71,    34,    89,    35,    47,    71,     0,    22,
    87,    85,     0,    22,    87,    36,     0,    22,    87,     0,
    87,    36,     0,    87,     0,    87,    85,     0,    88,    31,
    89,     0,    88,     0,    88,    71,    91,     0,    88,    91,
     0,    42,     0,     0,    46,     7,     0,    46,    81,     0,
    46,     4,     0,    46,    66,     0,    46,    71,    12,    86,
    14,     0,     0,    21,     0,    60,     0,    35,     0,    34,
     0,    46,     0,    88,    71,    12,    86,    14,    83,     0,
    88,    59,    93,    12,    86,    14,    83,     0,    80,     0,
    95,     0,    95,    31,    96,     0,     5,    12,    96,    14,
     0,    97,     0,    97,    31,    98,     0,    94,   101,     0,
    21,    94,    46,    48,   101,     0,    21,    94,   101,     0,
    71,    12,    86,    14,   101,     0,    71,    12,    86,    14,
    15,    98,   101,     0,    92,    33,    71,    12,    14,   101,
     0,    45,    94,   101,     0,    11,     0,    16,     0,   100,
   102,    13,     0,   100,   102,    13,    16,     0,   103,   102,
     0,     0,    16,     0,    31,   105,     0,     0,     5,   104,
     0,    88,   105,    16,     0,    45,    88,     5,    16,     0,
    88,     5,    50,    81,    51,    16,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
   144,   147,   150,   154,   157,   163,   164,   167,   172,   175,
   178,   181,   184,   187,   190,   193,   196,   199,   202,   207,
   208,   210,   210,   212,   212,   212,   214,   214,   214,   217,
   221,   227,   233,   236,   241,   249,   253,   260,   264,   272,
   276,   283,   287,   291,   295,   299,   303,   307,   311,   315,
   321,   322,   323,   326,   327,   330,   331,   334,   335,   336,
   337,   338,   341,   342,   345,   355,   362,   366,   373,   374,
   375,   376,   377,   378,   379,   380,   381,   382,   383,   384,
   385,   386,   387,   390,   391,   394,   398,   399,   406,   407,
   408,   409,   410,   415,   423,   428,   435,   440,   445,   450,
   458,   462,   468,   477,   483,   491,   494,   497,   500,   503,
   506,   512,   513,   516,   516,   516,   516,   519,   544,   551,
   554,   555,   558,   561,   562,   565,   569,   573,   577,   583,
   589,   595,   602,   608,   609,   610,   613,   614,   617,   620,
   621,   623,   626,   627,   628
};
#endif


#if YYDEBUG != 0 || defined (YYERROR_VERBOSE)

static const char * const yytname[] = {   "$","error","$undefined.","T_CHARACTER_LITERAL",
"T_DOUBLE_LITERAL","T_IDENTIFIER","T_INTEGER_LITERAL","T_STRING_LITERAL","T_INCLUDE",
"T_CLASS","T_STRUCT","T_LEFT_CURLY_BRACKET","T_LEFT_PARANTHESIS","T_RIGHT_CURLY_BRACKET",
"T_RIGHT_PARANTHESIS","T_COLON","T_SEMICOLON","T_PUBLIC","T_PROTECTED","T_TRIPE_DOT",
"T_PRIVATE","T_VIRTUAL","T_CONST","T_INLINE","T_FRIEND","T_RETURN","T_SIGNAL",
"T_SLOT","T_TYPEDEF","T_PLUS","T_MINUS","T_COMMA","T_ASTERISK","T_TILDE","T_LESS",
"T_GREATER","T_AMPERSAND","T_EXTERN","T_ACCESS","T_ENUM","T_NAMESPACE","T_UNKNOWN",
"T_TRIPLE_DOT","T_TRUE","T_FALSE","T_STATIC","T_EQUAL","T_SCOPE","T_NULL","T_INT",
"T_ARRAY_OPEN","T_ARRAY_CLOSE","T_CHAR","T_DCOP","T_DCOP_AREA","T_SIGNED","T_UNSIGNED",
"T_LONG","T_SHORT","T_FUNOPERATOR","T_MISCOPERATOR","main","includes","dcoptag",
"declaration","member_list","bool_value","nodcop_area","sigslot","nodcop_area_begin",
"dcop_area_begin","Identifier","super_class_name","super_class","super_classes",
"class_header","body","enum","enum_list","enum_item","number","int_expression",
"typedef","const_qualifier","int_type","asterisks","params","type_name","type",
"type_list","param","default","virtual_qualifier","operator","function_header",
"argument","arguments","init_item","init_list","function","function_begin","function_body",
"function_lines","function_line","Identifier_list_rest","Identifier_list","member", NULL
};
#endif

static const short yyr1[] = {     0,
    61,    61,    62,    62,    62,    63,    63,    64,    64,    64,
    64,    64,    64,    64,    64,    64,    64,    64,    64,    65,
    65,    66,    66,    67,    67,    67,    68,    68,    68,    69,
    69,    70,    71,    71,    72,    73,    73,    74,    74,    75,
    75,    76,    76,    76,    76,    76,    76,    76,    76,    76,
    77,    77,    77,    78,    78,    79,    79,    80,    80,    80,
    80,    80,    81,    81,    82,    82,    83,    83,    84,    84,
    84,    84,    84,    84,    84,    84,    84,    84,    84,    84,
    84,    84,    84,    85,    85,    86,    86,    86,    87,    87,
    87,    87,    87,    87,    88,    88,    88,    88,    88,    88,
    89,    89,    90,    90,    90,    91,    91,    91,    91,    91,
    91,    92,    92,    93,    93,    93,    93,    94,    94,    95,
    96,    96,    97,    98,    98,    99,    99,    99,    99,    99,
    99,    99,   100,   101,   101,   101,   102,   102,   103,   104,
   104,   105,   106,   106,   106
};

static const short yyr2[] = {     0,
     3,     0,     2,     4,     0,     1,     0,     6,     3,     3,
     6,     6,     4,     7,     8,     2,     1,     1,     1,     2,
     0,     1,     1,     1,     1,     1,     1,     1,     0,     3,
     2,     2,     1,     3,     1,     3,     1,     2,     3,     2,
     1,     1,     2,     2,     2,     2,     2,     2,     5,     4,
     7,     6,     6,     3,     1,     3,     1,     1,     1,     2,
     1,     1,     1,     3,     7,     9,     0,     1,     1,     1,
     2,     2,     3,     2,     2,     3,     2,     1,     1,     1,
     1,     2,     2,     2,     1,     0,     1,     3,     1,     1,
     2,     2,     4,     6,     3,     3,     2,     2,     1,     2,
     3,     1,     3,     2,     1,     0,     2,     2,     2,     2,
     5,     0,     1,     1,     1,     1,     1,     6,     7,     1,
     1,     3,     4,     1,     3,     2,     5,     3,     5,     7,
     6,     3,     1,     1,     3,     4,     2,     0,     1,     2,
     0,     2,     3,     4,     6
};

static const short yydefact[] = {     5,
     5,     0,   112,     3,     5,    33,     0,     0,   113,     0,
   112,     0,     0,     0,     0,    78,    81,    69,    70,    79,
    80,     5,    90,    19,    89,    99,     0,     0,     0,    17,
    18,     0,     0,    92,    91,     0,     0,    90,     0,     0,
    97,     0,    16,     0,     0,     0,     0,     0,     0,     0,
    82,    72,    71,    77,    83,    75,    74,     1,    86,     0,
    85,    98,   100,    33,     0,     0,     0,     0,   133,   134,
   138,   126,     4,    34,    41,   112,     9,     7,     5,    10,
    92,    91,     0,   128,    96,    95,    21,    91,     0,     0,
    57,     0,    55,     5,    33,   132,    73,    76,   105,     0,
   106,    87,   102,     0,    84,     0,     0,   142,   116,   115,
   117,   114,     0,    86,   143,     0,   139,     0,   138,   113,
    35,    37,     0,    40,     0,     6,    29,     0,     0,     0,
     0,     0,    21,    21,    13,     0,     0,     0,     0,     0,
   144,     0,     0,     0,   106,   104,     0,    93,   141,   140,
    58,    59,     0,    61,    62,    63,     0,    86,     0,     0,
   135,   137,    38,   112,     0,    42,    26,    25,    24,     0,
    27,    28,     0,     0,    29,     0,    29,    29,     0,    29,
    29,    29,    29,     0,   127,     0,     0,   141,    20,     0,
     0,    56,     0,    54,     0,     0,   129,    88,   109,   107,
    22,    23,   110,    62,   108,   103,   101,     0,    60,     0,
     0,     0,    67,     0,   136,    39,    36,     0,     0,     0,
    32,     0,    31,    47,    46,     8,    45,    43,    44,    48,
    11,     0,     0,     0,     0,    52,    53,    12,     0,   124,
     0,    86,    94,    64,   145,    67,    68,   118,   131,     0,
    29,     0,    30,    14,     0,    51,     0,     0,   130,     0,
   119,    29,    50,     0,    15,   120,   121,     0,   125,   111,
    49,     0,     0,   123,     0,     0,   122,     0,    65,     0,
    66,     0,     0,     0
};

static const short yydefgoto[] = {    32,
     3,   127,    22,   131,   203,   175,   176,   177,   178,    38,
   122,   123,   124,    78,   179,   180,    92,    93,   156,   157,
   181,   248,    25,    63,   100,    26,    27,   104,   102,   146,
    28,   113,    29,   267,   268,   240,   241,   182,    71,    72,
   118,   119,   108,    67,   183
};

static const short yypact[] = {    78,
    42,    12,   228,-32768,    18,    25,   123,   123,   283,    96,
    35,   293,   125,   145,   283,-32768,-32768,   154,   -20,-32768,
-32768,   127,    15,-32768,-32768,   184,     1,    81,   193,-32768,
-32768,   160,   123,   229,   203,   123,   123,   156,     9,   122,
   191,   283,-32768,   189,   123,   211,   219,   231,    17,   193,
-32768,-32768,   197,-32768,-32768,-32768,   206,-32768,    61,   283,
   220,-32768,-32768,    27,   109,   246,   245,   123,-32768,-32768,
   255,-32768,-32768,-32768,-32768,    30,-32768,   223,    18,-32768,
-32768,-32768,   224,-32768,-32768,-32768,   269,   270,   266,   219,
   241,   276,   259,    18,    79,-32768,-32768,-32768,-32768,   111,
    23,-32768,   265,   262,-32768,   296,   117,-32768,-32768,-32768,
-32768,-32768,   294,    61,-32768,   295,-32768,   297,   255,-32768,
-32768,-32768,    77,-32768,   291,-32768,   208,   298,   193,   283,
   299,   304,   269,   269,-32768,   300,   117,   311,   219,   306,
-32768,   284,    61,   153,   271,-32768,   283,   273,   292,-32768,
-32768,-32768,   316,-32768,-32768,   301,   277,    61,   132,   315,
   317,-32768,-32768,    30,   123,-32768,-32768,-32768,-32768,   234,
-32768,-32768,   123,   319,   166,   321,   208,   208,   327,   208,
   208,   208,   208,   328,-32768,   326,   123,    71,-32768,   324,
   171,-32768,   330,-32768,   331,   347,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,   341,-32768,-32768,-32768,   123,-32768,   117,
   338,   157,   333,   193,-32768,-32768,-32768,   123,   340,   323,
-32768,   343,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,   344,   345,   123,   346,-32768,-32768,-32768,   351,   334,
   193,    61,-32768,-32768,-32768,   333,-32768,-32768,-32768,   348,
   208,   283,-32768,-32768,   350,-32768,   117,   347,-32768,   158,
-32768,   208,-32768,   332,-32768,-32768,   337,   355,-32768,-32768,
-32768,     5,   117,-32768,   354,   356,-32768,   123,-32768,   357,
-32768,   370,   371,-32768
};

static const short yypgoto[] = {     3,
   373,-32768,-32768,    69,-32768,-32768,   200,-32768,-32768,    -3,
   212,-32768,   214,-32768,  -162,   376,   -77,-32768,  -198,    54,
-32768,   130,-32768,    88,  -105,   372,    52,  -145,   237,   236,
   -65,-32768,    39,-32768,   110,-32768,   126,    36,-32768,   -33,
   267,-32768,-32768,   279,    -2
};


#define	YYLAST		386


static const short yytable[] = {    23,
    31,   207,   282,    34,    35,    64,    84,    23,   159,     6,
   125,   244,   136,     6,   224,   225,    96,   227,   228,   229,
   230,    95,     5,    66,    58,     1,    59,     6,    54,    74,
    -2,    55,    81,    82,     6,    66,    56,    57,    30,     6,
    88,    89,  -141,    36,    37,    66,    43,    40,    60,     1,
   120,   275,   212,    50,     2,     9,    10,   106,   266,    65,
    39,   194,    39,    45,   116,     6,    49,    65,   144,    36,
    37,    33,   121,    33,   266,    65,   107,    -2,     2,    42,
    50,   128,    10,    16,   133,     1,    17,   163,   263,    18,
    19,    20,    21,    39,   141,   185,   140,   145,   125,   271,
     6,   106,    99,   155,    36,    37,   264,   164,   197,    16,
   101,   103,    17,    68,     2,    18,    19,    20,    21,   151,
   107,     6,   152,    23,   142,    33,    -2,     6,    86,    46,
   133,   133,    69,   155,     1,    47,   260,    70,   132,    -2,
   204,   143,   109,   110,    16,   213,   153,    17,   105,    48,
    18,    19,    20,    21,   111,   151,   199,     6,   152,   200,
   121,   121,   143,     2,   154,   101,   219,    83,   112,   220,
   246,   270,    73,    23,    23,   235,    23,    23,    23,    23,
   249,   186,   153,   233,   132,   132,   236,   143,   143,    60,
   192,   171,   172,     6,   101,   201,   202,   205,   103,    87,
   154,   189,   190,    69,   243,    51,   155,   259,    70,   101,
    52,    53,     6,    79,   250,    61,    36,    37,    80,    62,
   166,    90,    61,    91,   167,   168,    85,   169,     9,    10,
   255,   170,     6,   171,   172,   173,     7,     8,     6,    75,
  -112,    94,   218,    76,    77,    97,    13,    23,     9,    10,
    11,    61,    15,   155,    98,    12,    16,   114,    23,    17,
   115,   174,    18,    19,    20,    21,    13,    14,   276,   155,
   117,   129,    15,     6,   280,   126,    16,    36,    37,    17,
   134,   135,    18,    19,    20,    21,   137,     6,   138,   139,
    10,    36,    37,   101,    69,   147,   148,     6,   196,    70,
   149,    36,    44,   103,    10,   158,   160,   165,   188,   161,
   184,   187,   191,   130,    10,   193,   144,    16,   195,   208,
    17,   209,   106,    18,    19,    20,    21,   211,   214,   210,
   232,    16,   215,   221,    17,   223,   234,    18,    19,    20,
    21,    16,   226,   231,    17,   237,   238,    18,    19,    20,
    21,   239,   242,   245,   247,   251,   252,   253,   278,   141,
   254,   256,   257,   262,   258,   265,   272,   273,   274,   283,
   284,   279,   281,     4,   222,   261,   217,   216,    24,   198,
   206,    41,   277,   269,   150,   162
};

static const short yycheck[] = {     3,
     3,   147,     0,     7,     8,     5,    40,    11,   114,     5,
    76,   210,    90,     5,   177,   178,    50,   180,   181,   182,
   183,     5,    11,    27,    22,     8,    12,     5,    49,    33,
    13,    52,    36,    37,     5,    39,    57,    58,     3,     5,
    44,    45,    16,     9,    10,    49,    11,     9,    34,     8,
    21,    47,   158,    15,    37,    21,    22,    31,   257,    59,
     9,   139,    11,    12,    68,     5,    15,    59,    46,     9,
    10,    47,    76,    47,   273,    59,    50,     0,    37,    45,
    42,    79,    22,    49,    87,     8,    52,    11,   251,    55,
    56,    57,    58,    42,    16,   129,    94,   101,   164,   262,
     5,    31,    42,   107,     9,    10,   252,    31,   142,    49,
    59,    60,    52,    33,    37,    55,    56,    57,    58,     3,
    50,     5,     6,   127,    14,    47,     0,     5,    41,     5,
   133,   134,    11,   137,     8,    11,   242,    16,    87,    13,
   144,    31,    34,    35,    49,    14,    30,    52,    61,     5,
    55,    56,    57,    58,    46,     3,     4,     5,     6,     7,
   164,   165,    31,    37,    48,   114,   170,    46,    60,   173,
    14,    14,    13,   177,   178,     5,   180,   181,   182,   183,
   214,   130,    30,   187,   133,   134,    16,    31,    31,    34,
   137,    26,    27,     5,   143,    43,    44,   144,   147,    11,
    48,   133,   134,    11,   208,    52,   210,   241,    16,   158,
    57,    58,     5,    11,   218,    32,     9,    10,    16,    36,
    13,    11,    32,     5,    17,    18,    36,    20,    21,    22,
   234,    24,     5,    26,    27,    28,     9,    10,     5,    11,
    33,    11,     9,    15,    16,    49,    39,   251,    21,    22,
    23,    32,    45,   257,    49,    28,    49,    12,   262,    52,
    16,    54,    55,    56,    57,    58,    39,    40,   272,   273,
    16,    48,    45,     5,   278,    53,    49,     9,    10,    52,
    11,    16,    55,    56,    57,    58,    46,     5,    13,    31,
    22,     9,    10,   242,    11,    31,    35,     5,    15,    16,
     5,     9,    10,   252,    22,    12,    12,    17,     5,    13,
    13,    13,    13,    45,    22,     5,    46,    49,    13,    47,
    52,     6,    31,    55,    56,    57,    58,    51,    14,    29,
     5,    49,    16,    15,    52,    15,    13,    55,    56,    57,
    58,    49,    16,    16,    52,    16,    16,    55,    56,    57,
    58,     5,    12,    16,    22,    16,    34,    15,     5,    16,
    16,    16,    12,    16,    31,    16,    35,    31,    14,     0,
     0,    16,    16,     1,   175,   246,   165,   164,     3,   143,
   145,    10,   273,   258,   106,   119
};
/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
#line 3 "/usr/share/bison.simple"

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
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

#ifndef alloca
#ifdef __GNUC__
#define alloca __builtin_alloca
#else /* not GNU C.  */
#if (!defined (__STDC__) && defined (sparc)) || defined (__sparc__) || defined (__sparc) || defined (__sgi)
#include <alloca.h>
#else /* not sparc */
#if defined (MSDOS) && !defined (__TURBOC__)
#include <malloc.h>
#else /* not MSDOS, or __TURBOC__ */
#if defined(_AIX)
#include <malloc.h>
 #pragma alloca
#else /* not MSDOS, __TURBOC__, or _AIX */
#ifdef __hpux
#ifdef __cplusplus
extern "C" {
void *alloca (unsigned int);
};
#else /* not __cplusplus */
void *alloca ();
#endif /* not __cplusplus */
#endif /* __hpux */
#endif /* not _AIX */
#endif /* not MSDOS, or __TURBOC__ */
#endif /* not sparc.  */
#endif /* not GNU C.  */
#endif /* alloca not defined.  */

/* This is the parser code that is written into each bison parser
  when the %semantic_parser declaration is not specified in the grammar.
  It was written by Richard Stallman by simplifying the hairy parser
  used when %semantic_parser is specified.  */

/* Note: there must be only one dollar sign in this file.
   It is replaced by the list of actions, each action
   as one case of the switch.  */

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		-2
#define YYEOF		0
#define YYACCEPT	return(0)
#define YYABORT 	return(1)
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

#ifndef YYPARSE_RETURN_TYPE
#define YYPARSE_RETURN_TYPE int
#endif

/* Prevent warning if -Wstrict-prototypes.  */
#ifdef __GNUC__
YYPARSE_RETURN_TYPE yyparse (void);
#endif

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
     int count;
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
__yy_memcpy (char *to, char *from, int count)
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#endif
#endif

#line 196 "/usr/share/bison.simple"

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
	  return 2;
	}
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;
      yyss = (short *) alloca (yystacksize * sizeof (*yyssp));
      __yy_memcpy ((char *)yyss, (char *)yyss1, size * sizeof (*yyssp));
      yyvs = (YYSTYPE *) alloca (yystacksize * sizeof (*yyvsp));
      __yy_memcpy ((char *)yyvs, (char *)yyvs1, size * sizeof (*yyvsp));
#ifdef YYLSP_NEEDED
      yyls = (YYLTYPE *) alloca (yystacksize * sizeof (*yylsp));
      __yy_memcpy ((char *)yyls, (char *)yyls1, size * sizeof (*yylsp));
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
#line 145 "yacc.yy"
{
	  ;
    break;}
case 3:
#line 151 "yacc.yy"
{
		printf("<INCLUDE>%s</INCLUDE>\n", yyvsp[-1]._str->latin1() );
	  ;
    break;}
case 4:
#line 155 "yacc.yy"
{
	  ;
    break;}
case 5:
#line 158 "yacc.yy"
{
          ;
    break;}
case 6:
#line 163 "yacc.yy"
{ yyval._int = 1; ;
    break;}
case 7:
#line 164 "yacc.yy"
{ yyval._int = 0; ;
    break;}
case 8:
#line 168 "yacc.yy"
{
	 	if (yyvsp[-2]._int)
			  printf("<CLASS>\n    <NAME>%s</NAME>\n%s%s</CLASS>\n", yyvsp[-4]._str->latin1(), yyvsp[-3]._str->latin1(), yyvsp[-1]._str->latin1() );
	  ;
    break;}
case 9:
#line 173 "yacc.yy"
{
	  ;
    break;}
case 10:
#line 176 "yacc.yy"
{
	  ;
    break;}
case 11:
#line 179 "yacc.yy"
{
	  ;
    break;}
case 12:
#line 182 "yacc.yy"
{
	  ;
    break;}
case 13:
#line 185 "yacc.yy"
{
	  ;
    break;}
case 14:
#line 188 "yacc.yy"
{
	  ;
    break;}
case 15:
#line 191 "yacc.yy"
{
	  ;
    break;}
case 16:
#line 194 "yacc.yy"
{
	  ;
    break;}
case 17:
#line 197 "yacc.yy"
{
	  ;
    break;}
case 18:
#line 200 "yacc.yy"
{
	  ;
    break;}
case 19:
#line 203 "yacc.yy"
{
	  ;
    break;}
case 30:
#line 218 "yacc.yy"
{
	  dcop_area = 0;
	;
    break;}
case 31:
#line 222 "yacc.yy"
{
	  dcop_area = 0;
	;
    break;}
case 32:
#line 228 "yacc.yy"
{
	  dcop_area = 1;
	;
    break;}
case 33:
#line 233 "yacc.yy"
{
	  yyval._str = yyvsp[0]._str;
	;
    break;}
case 34:
#line 236 "yacc.yy"
{
	   yyval._str = new QString( *(yyvsp[-2]._str) + *(yyvsp[0]._str) );
	;
    break;}
case 35:
#line 242 "yacc.yy"
{
		QString* tmp = new QString( "    <SUPER>%1</SUPER>\n" );
		*tmp = tmp->arg( *(yyvsp[0]._str) );
		yyval._str = tmp;
	  ;
    break;}
case 36:
#line 250 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 37:
#line 254 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 38:
#line 261 "yacc.yy"
{
		yyval._str = yyvsp[-1]._str;
	  ;
    break;}
case 39:
#line 265 "yacc.yy"
{
		/* $$ = $1; */
		yyval._str = new QString( *(yyvsp[-2]._str) + *(yyvsp[0]._str) );
	  ;
    break;}
case 40:
#line 273 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 41:
#line 277 "yacc.yy"
{
		yyval._str = new QString( "" );
	  ;
    break;}
case 42:
#line 284 "yacc.yy"
{
		yyval._str = new QString( "" );
	  ;
    break;}
case 43:
#line 288 "yacc.yy"
{
		yyval._str = new QString( *(yyvsp[-1]._str) + *(yyvsp[0]._str) );
	  ;
    break;}
case 44:
#line 292 "yacc.yy"
{
		yyval._str = new QString( *(yyvsp[-1]._str) + *(yyvsp[0]._str) );
	  ;
    break;}
case 45:
#line 296 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 46:
#line 300 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 47:
#line 304 "yacc.yy"
{	
	        yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 48:
#line 308 "yacc.yy"
{
 	        yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 49:
#line 312 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 50:
#line 316 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 56:
#line 330 "yacc.yy"
{;
    break;}
case 57:
#line 331 "yacc.yy"
{;
    break;}
case 58:
#line 334 "yacc.yy"
{;
    break;}
case 59:
#line 335 "yacc.yy"
{;
    break;}
case 60:
#line 336 "yacc.yy"
{;
    break;}
case 61:
#line 337 "yacc.yy"
{;
    break;}
case 62:
#line 338 "yacc.yy"
{;
    break;}
case 63:
#line 341 "yacc.yy"
{;
    break;}
case 64:
#line 342 "yacc.yy"
{;
    break;}
case 65:
#line 346 "yacc.yy"
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
case 66:
#line 356 "yacc.yy"
{
		if (dcop_area)
		  yyerror("scoped template typedefs are not supported in dcop areas!");
	  ;
    break;}
case 67:
#line 363 "yacc.yy"
{
		yyval._int = 0;
	  ;
    break;}
case 68:
#line 367 "yacc.yy"
{
		yyval._int = 1;
	  ;
    break;}
case 69:
#line 373 "yacc.yy"
{ yyval._str = new QString("signed int"); ;
    break;}
case 70:
#line 374 "yacc.yy"
{ yyval._str = new QString("unsigned int"); ;
    break;}
case 71:
#line 375 "yacc.yy"
{ yyval._str = new QString("signed short int"); ;
    break;}
case 72:
#line 376 "yacc.yy"
{ yyval._str = new QString("signed long int"); ;
    break;}
case 73:
#line 377 "yacc.yy"
{ yyval._str = new QString("signed short int"); ;
    break;}
case 74:
#line 378 "yacc.yy"
{ yyval._str = new QString("unsigned short int"); ;
    break;}
case 75:
#line 379 "yacc.yy"
{ yyval._str = new QString("unsigned long int"); ;
    break;}
case 76:
#line 380 "yacc.yy"
{ yyval._str = new QString("unsigned short int"); ;
    break;}
case 77:
#line 381 "yacc.yy"
{ yyval._str = new QString("unsigned int"); ;
    break;}
case 78:
#line 382 "yacc.yy"
{ yyval._str = new QString("int"); ;
    break;}
case 79:
#line 383 "yacc.yy"
{ yyval._str = new QString("long int"); ;
    break;}
case 80:
#line 384 "yacc.yy"
{ yyval._str = new QString("short int"); ;
    break;}
case 81:
#line 385 "yacc.yy"
{ yyval._str = new QString("char"); ;
    break;}
case 82:
#line 386 "yacc.yy"
{ yyval._str = new QString("signed char"); ;
    break;}
case 83:
#line 387 "yacc.yy"
{ yyval._str = new QString("unsigned char"); ;
    break;}
case 86:
#line 395 "yacc.yy"
{
		yyval._str = new QString( "" );
	  ;
    break;}
case 88:
#line 400 "yacc.yy"
{
		yyval._str = new QString( *(yyvsp[-2]._str) + *(yyvsp[0]._str) );
	  ;
    break;}
case 90:
#line 407 "yacc.yy"
{ yyval._str = yyvsp[0]._str; ;
    break;}
case 91:
#line 408 "yacc.yy"
{ yyval._str = yyvsp[0]._str; ;
    break;}
case 92:
#line 409 "yacc.yy"
{ yyval._str = yyvsp[0]._str; ;
    break;}
case 93:
#line 410 "yacc.yy"
{
		QString *tmp = new QString("%1<%2>");
		tmp->arg(*(yyvsp[-3]._str));
		tmp->arg(*(yyvsp[-1]._str));
	 ;
    break;}
case 94:
#line 415 "yacc.yy"
{
		QString *tmp = new QString("%1<%2>::%3");
		tmp->arg(*(yyvsp[-5]._str));
		tmp->arg(*(yyvsp[-3]._str));
		tmp->arg(*(yyvsp[0]._str));
	 ;
    break;}
case 95:
#line 424 "yacc.yy"
{
	    if (dcop_area)
	      yyerror("in dcop areas are no pointers allowed");
	  ;
    break;}
case 96:
#line 428 "yacc.yy"
{
	     if (dcop_area) {
	  	QString* tmp = new QString("<TYPE  qleft=\"const\" qright=\"" AMP_ENTITY "\">%1</TYPE>");
		*tmp = tmp->arg( *(yyvsp[-1]._str) );
		yyval._str = tmp;
	     }
	  ;
    break;}
case 97:
#line 435 "yacc.yy"
{
		QString* tmp = new QString("<TYPE>%1</TYPE>");
		*tmp = tmp->arg( *(yyvsp[0]._str) );
		yyval._str = tmp;
	;
    break;}
case 98:
#line 440 "yacc.yy"
{
	     if (dcop_area)
		yyerror("in dcop areas are only const references allowed!");
	  ;
    break;}
case 99:
#line 445 "yacc.yy"
{
		QString* tmp = new QString("<TYPE>%1</TYPE>");
		*tmp = tmp->arg( *(yyvsp[0]._str) );
		yyval._str = tmp;
	;
    break;}
case 100:
#line 451 "yacc.yy"
{
	    if (dcop_area)
	      yyerror("in dcop areas are no pointers allowed");
	  ;
    break;}
case 101:
#line 459 "yacc.yy"
{
	    yyval._str = new QString(*(yyvsp[-2]._str) + "," + *(yyvsp[0]._str));
	  ;
    break;}
case 102:
#line 463 "yacc.yy"
{
 	    yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 103:
#line 469 "yacc.yy"
{
		if (dcop_area) {
		   QString* tmp = new QString("\n        <ARG>%1<NAME>%2</NAME></ARG>");
  		   *tmp = tmp->arg( *(yyvsp[-2]._str) );
  		   *tmp = tmp->arg( *(yyvsp[-1]._str) );
		   yyval._str = tmp;		
		} else yyval._str = new QString();
	  ;
    break;}
case 104:
#line 478 "yacc.yy"
{
		if (dcop_area)
		  yyerror("in dcoparea you have to specify paramater names!");
		yyval._str = new QString();
	  ;
    break;}
case 105:
#line 484 "yacc.yy"
{
		if (dcop_area)
			yyerror("variable arguments not supported in dcop area!");
		yyval._str = new QString("");
	  ;
    break;}
case 106:
#line 492 "yacc.yy"
{
	  ;
    break;}
case 107:
#line 495 "yacc.yy"
{
	  ;
    break;}
case 108:
#line 498 "yacc.yy"
{
	  ;
    break;}
case 109:
#line 501 "yacc.yy"
{
	  ;
    break;}
case 110:
#line 504 "yacc.yy"
{
	  ;
    break;}
case 111:
#line 507 "yacc.yy"
{
	  ;
    break;}
case 112:
#line 512 "yacc.yy"
{ yyval._int = 0; ;
    break;}
case 113:
#line 513 "yacc.yy"
{ yyval._int = 1; ;
    break;}
case 118:
#line 520 "yacc.yy"
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
case 119:
#line 545 "yacc.yy"
{
	     if (dcop_area)
		yyerror("operators aren't allowed in dcop areas!");
	     yyval._str = new QString("");
	  ;
    break;}
case 120:
#line 551 "yacc.yy"
{;
    break;}
case 121:
#line 554 "yacc.yy"
{;
    break;}
case 122:
#line 555 "yacc.yy"
{;
    break;}
case 123:
#line 558 "yacc.yy"
{;
    break;}
case 124:
#line 561 "yacc.yy"
{;
    break;}
case 125:
#line 562 "yacc.yy"
{;
    break;}
case 126:
#line 566 "yacc.yy"
{
	        yyval._str = yyvsp[-1]._str;
	  ;
    break;}
case 127:
#line 570 "yacc.yy"
{
		yyval._str = yyvsp[-3]._str;
	  ;
    break;}
case 128:
#line 574 "yacc.yy"
{
		yyval._str = yyvsp[-1]._str;
	  ;
    break;}
case 129:
#line 578 "yacc.yy"
{
	      /* The constructor */
	      assert(!dcop_area);
              yyval._str = new QString("");
	  ;
    break;}
case 130:
#line 584 "yacc.yy"
{
	      /* The constructor */
	      assert(!dcop_area);
              yyval._str = new QString("");
	  ;
    break;}
case 131:
#line 590 "yacc.yy"
{
	      /* The destructor */
  	      assert(!dcop_area);
              yyval._str = new QString("");
	  ;
    break;}
case 132:
#line 596 "yacc.yy"
{
		if (dcop_area)
		  yyerror("static is not allowed in dcop area!");
		yyval._str = new QString();
	  ;
    break;}
case 133:
#line 603 "yacc.yy"
{
		function_mode = 1;
	;
    break;}
case 137:
#line 613 "yacc.yy"
{;
    break;}
case 138:
#line 614 "yacc.yy"
{;
    break;}
case 142:
#line 623 "yacc.yy"
{;
    break;}
case 143:
#line 626 "yacc.yy"
{;
    break;}
case 144:
#line 627 "yacc.yy"
{;
    break;}
case 145:
#line 628 "yacc.yy"
{;
    break;}
}
   /* the action file gets copied in in place of this dollarsign */
#line 498 "/usr/share/bison.simple"

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
}
#line 630 "yacc.yy"


void dcopidlParse( const char *_code )
{
    dcopidlInitFlex( _code );
    yyparse();
}
