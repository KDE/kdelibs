
/*  A Bison parser, made from yacc.yy
    by GNU Bison version 1.28  */

#define YYBISON 1  /* Identify Bison output.  */

#define	T_CHARACTER_LITERAL	257
#define	T_DOUBLE_LITERAL	258
#define	T_IDENTIFIER	259
#define	T_INTEGER_LITERAL	260
#define	T_CHAR_LITERAL	261
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
#define	T_PRIVATE	274
#define	T_VIRTUAL	275
#define	T_CONST	276
#define	T_RETURN	277
#define	T_SIGNAL	278
#define	T_SLOT	279
#define	T_TYPEDEF	280
#define	T_COMMA	281
#define	T_ASTERISK	282
#define	T_TILDE	283
#define	T_LESS	284
#define	T_GREATER	285
#define	T_AMPERSAND	286
#define	T_ACCESS	287
#define	T_ENUM	288
#define	T_NAMESPACE	289
#define	T_UNKNOWN	290
#define	T_TRUE	291
#define	T_FALSE	292
#define	T_STATIC	293
#define	T_EQUAL	294
#define	T_SCOPE	295
#define	T_NULL	296
#define	T_DCOP	297
#define	T_DCOP_AREA	298
#define	T_SIGNED	299
#define	T_UNSIGNED	300
#define	T_FUNOPERATOR	301
#define	T_MISCOPERATOR	302

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



#define	YYFINAL		228
#define	YYFLAG		-32768
#define	YYNTBASE	49

#define YYTRANSLATE(x) ((unsigned)(x) <= 302 ? yytranslate[x] : 85)

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
    47,    48
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     4,     5,     8,     9,    11,    12,    19,    23,    27,
    34,    41,    46,    48,    50,    52,    54,    56,    58,    60,
    62,    64,    66,    67,    71,    74,    77,    79,    83,    85,
    89,    91,    94,    98,   101,   103,   105,   108,   111,   114,
   117,   120,   123,   131,   138,   145,   149,   151,   155,   159,
   161,   169,   170,   172,   174,   176,   178,   181,   183,   184,
   186,   190,   193,   195,   198,   202,   209,   215,   221,   226,
   228,   231,   235,   239,   241,   245,   248,   249,   252,   255,
   258,   261,   264,   267,   270,   276,   277,   279,   281,   283,
   285,   287,   294,   302,   305,   311,   315,   321,   328,   336,
   338,   342,   347,   352,   360,   367,   377,   383,   384,   388
};

static const short yyrhs[] = {    50,
    52,    49,     0,     0,     9,    50,     0,     0,    43,     0,
     0,    10,    58,    62,    51,    63,    17,     0,    10,    58,
    17,     0,    11,    58,    17,     0,    11,    58,    12,    49,
    14,    17,     0,    35,     5,    12,    49,    14,    17,     0,
    26,    70,    58,    17,     0,    81,     0,    84,     0,    64,
     0,    37,     0,    38,     0,    20,     0,    19,     0,    18,
     0,    24,     0,    25,     0,     0,    54,    55,    16,     0,
    55,    16,     0,    44,    16,     0,     5,     0,     5,    41,
    58,     0,    58,     0,    78,    18,    59,     0,    59,     0,
    60,    12,     0,    60,    27,    61,     0,    16,    61,     0,
    12,     0,    14,     0,    67,    63,     0,    81,    63,     0,
    64,    63,     0,    57,    63,     0,    56,    63,     0,    84,
    63,     0,    34,     5,    12,    65,    14,     5,    17,     0,
    34,     5,    12,    65,    14,    17,     0,    34,    12,    65,
    14,     5,    17,     0,    66,    27,    65,     0,    66,     0,
     5,    40,     6,     0,     5,    40,     7,     0,     5,     0,
    26,    58,    30,    75,    31,    58,    17,     0,     0,    22,
     0,    45,     0,    46,     0,    74,     0,    28,    71,     0,
    28,     0,     0,    76,     0,    72,    27,    76,     0,    69,
    58,     0,    58,     0,    73,    32,     0,    22,    73,    32,
     0,    22,    58,    30,    75,    31,    32,     0,    58,    30,
    75,    31,    32,     0,    58,    30,    75,    31,    71,     0,
    58,    30,    75,    31,     0,    73,     0,    73,    71,     0,
    22,    73,    71,     0,    74,    27,    75,     0,    74,     0,
    74,    58,    77,     0,    74,    77,     0,     0,    40,     8,
     0,    40,     3,     0,    40,     4,     0,    40,     6,     0,
    40,    42,     0,    40,    58,     0,    40,    53,     0,    40,
    58,    13,    72,    15,     0,     0,    21,     0,    48,     0,
    31,     0,    30,     0,    40,     0,    70,    58,    13,    72,
    15,    68,     0,    70,    47,    79,    13,    72,    15,    68,
     0,    80,    82,     0,    21,    80,    40,    42,    82,     0,
    21,    80,    82,     0,    58,    13,    72,    15,    82,     0,
    78,    29,    58,    13,    15,    82,     0,    39,    70,    58,
    13,    72,    15,    82,     0,    17,     0,    12,    83,    14,
     0,    12,    83,    14,    17,     0,    23,    58,    17,    83,
     0,    23,    58,    13,    72,    15,    17,    83,     0,    23,
    58,    33,     5,    17,    83,     0,    23,    58,    33,     5,
    13,    72,    15,    17,    83,     0,     5,    40,     5,    17,
    83,     0,     0,    70,     5,    17,     0,    39,    70,     5,
    17,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
   110,   114,   117,   121,   127,   128,   131,   136,   139,   142,
   145,   148,   151,   154,   157,   161,   161,   163,   163,   163,
   165,   165,   165,   168,   172,   178,   184,   187,   192,   200,
   204,   211,   215,   223,   227,   234,   238,   242,   246,   250,
   254,   258,   264,   265,   266,   269,   270,   273,   274,   275,
   278,   290,   294,   301,   302,   305,   313,   314,   317,   321,
   325,   332,   333,   336,   340,   347,   354,   358,   362,   367,
   372,   377,   384,   391,   397,   405,   413,   416,   419,   422,
   425,   428,   431,   434,   437,   443,   444,   447,   447,   447,
   447,   450,   462,   470,   474,   478,   482,   488,   494,   502,
   503,   504,   507,   508,   509,   510,   511,   512,   516,   517
};
#endif


#if YYDEBUG != 0 || defined (YYERROR_VERBOSE)

static const char * const yytname[] = {   "$","error","$undefined.","T_CHARACTER_LITERAL",
"T_DOUBLE_LITERAL","T_IDENTIFIER","T_INTEGER_LITERAL","T_CHAR_LITERAL","T_STRING_LITERAL",
"T_INCLUDE","T_CLASS","T_STRUCT","T_LEFT_CURLY_BRACKET","T_LEFT_PARANTHESIS",
"T_RIGHT_CURLY_BRACKET","T_RIGHT_PARANTHESIS","T_COLON","T_SEMICOLON","T_PUBLIC",
"T_PROTECTED","T_PRIVATE","T_VIRTUAL","T_CONST","T_RETURN","T_SIGNAL","T_SLOT",
"T_TYPEDEF","T_COMMA","T_ASTERISK","T_TILDE","T_LESS","T_GREATER","T_AMPERSAND",
"T_ACCESS","T_ENUM","T_NAMESPACE","T_UNKNOWN","T_TRUE","T_FALSE","T_STATIC",
"T_EQUAL","T_SCOPE","T_NULL","T_DCOP","T_DCOP_AREA","T_SIGNED","T_UNSIGNED",
"T_FUNOPERATOR","T_MISCOPERATOR","main","includes","dcoptag","declaration","bool_value",
"nodcop_area","sigslot","nodcop_area_begin","dcop_area_begin","Identifier","super_class_name",
"super_class","super_classes","class_header","body","enum","enum_list","enum_item",
"typedef","const_qualifier","prequalifier","return","asterisks","params","type_name",
"type","type_list","param","default","virtual_qualifier","operator","function_header",
"function","function_body","function_lines","member", NULL
};
#endif

static const short yyr1[] = {     0,
    49,    49,    50,    50,    51,    51,    52,    52,    52,    52,
    52,    52,    52,    52,    52,    53,    53,    54,    54,    54,
    55,    55,    55,    56,    56,    57,    58,    58,    59,    60,
    60,    61,    61,    62,    62,    63,    63,    63,    63,    63,
    63,    63,    64,    64,    64,    65,    65,    66,    66,    66,
    67,    68,    68,    69,    69,    70,    71,    71,    72,    72,
    72,    73,    73,    74,    74,    74,    74,    74,    74,    74,
    74,    74,    75,    75,    76,    76,    77,    77,    77,    77,
    77,    77,    77,    77,    77,    78,    78,    79,    79,    79,
    79,    80,    80,    81,    81,    81,    81,    81,    81,    82,
    82,    82,    83,    83,    83,    83,    83,    83,    84,    84
};

static const short yyr2[] = {     0,
     3,     0,     2,     0,     1,     0,     6,     3,     3,     6,
     6,     4,     1,     1,     1,     1,     1,     1,     1,     1,
     1,     1,     0,     3,     2,     2,     1,     3,     1,     3,
     1,     2,     3,     2,     1,     1,     2,     2,     2,     2,
     2,     2,     7,     6,     6,     3,     1,     3,     3,     1,
     7,     0,     1,     1,     1,     1,     2,     1,     0,     1,
     3,     2,     1,     2,     3,     6,     5,     5,     4,     1,
     2,     3,     3,     1,     3,     2,     0,     2,     2,     2,
     2,     2,     2,     2,     5,     0,     1,     1,     1,     1,
     1,     6,     7,     2,     5,     3,     5,     6,     7,     1,
     3,     4,     4,     7,     6,     9,     5,     0,     3,     4
};

static const short yydefact[] = {     4,
     4,    86,     3,    27,     0,     0,    87,     0,     0,     0,
     0,     0,    54,    55,     4,    63,    15,     0,     0,    70,
    56,     0,     0,    13,    14,     0,     0,     0,    63,     0,
     0,    63,     0,     0,     0,     0,     0,     0,     1,    59,
     0,    62,    27,     0,     0,    58,    64,    71,     0,   108,
   100,    94,    28,    35,    86,     8,     6,     4,     9,     0,
    96,     0,    65,    72,     0,     0,    50,     0,    47,     4,
    27,     0,     0,    77,    60,    74,     0,   109,    90,    89,
    91,    88,     0,    59,    57,     0,     0,     0,     0,    87,
    29,    31,     0,    34,     0,     5,    23,     0,     0,     0,
    12,     0,     0,     0,     0,     0,   110,    59,     0,     0,
     0,    77,    76,     0,    69,    59,     0,     0,     0,     0,
   101,    32,    86,     0,    36,    20,    19,    18,    21,    22,
     0,     0,    23,     0,    23,    23,     0,    23,    23,    23,
    23,     0,    95,     0,     0,    48,    49,     0,    46,     0,
     0,    97,    61,    79,    80,    81,    78,    16,    17,    82,
    84,    83,    75,    73,    67,    68,     0,    52,     0,     0,
    59,   108,     0,   102,    33,    30,     0,    26,     0,    25,
    41,    40,     7,    39,    37,    38,    42,    10,    66,     0,
    44,    45,    11,     0,    59,    52,    53,    92,    98,   108,
     0,   103,     0,     0,    24,    43,    99,     0,    93,   107,
     0,    59,   108,     0,    85,   108,     0,   105,     0,   104,
     0,     0,   108,    51,   106,     0,     0,     0
};

static const short yydefgoto[] = {    39,
     2,    97,    15,   161,   133,   134,   135,   136,    29,    92,
    93,    94,    57,   137,   138,    68,    69,   139,   198,    18,
    19,    48,    73,    20,    21,    77,    75,   113,    22,    83,
    23,   140,    52,    89,   141
};

static const short yypact[] = {    22,
    18,    71,-32768,   -11,    36,    36,    15,    20,    15,   108,
    46,    15,-32768,-32768,    98,    13,-32768,    36,     7,   116,
-32768,    35,   147,-32768,-32768,    36,   129,   150,    40,    10,
    23,    50,   119,    36,    77,    86,   112,   130,-32768,    15,
    15,-32768,     1,    54,   137,   141,-32768,-32768,    36,    51,
-32768,-32768,-32768,-32768,    29,-32768,   136,   157,-32768,   163,
-32768,    15,-32768,-32768,   155,    86,   166,   175,   180,   157,
    27,   195,    72,     8,-32768,   182,   181,-32768,-32768,-32768,
-32768,-32768,   197,    15,-32768,   198,   173,    36,   200,-32768,
-32768,-32768,    76,-32768,   201,-32768,   156,   202,   147,   184,
-32768,   204,   177,   215,    86,   207,-32768,    15,   147,    15,
   149,   183,-32768,    15,   160,    15,    81,   209,   217,    16,
   208,-32768,    29,    36,-32768,-32768,-32768,-32768,-32768,-32768,
    36,   210,   179,   211,   156,   156,   212,   156,   156,   156,
   156,   213,-32768,   196,   106,-32768,-32768,   214,-32768,   216,
    99,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,   219,-32768,-32768,-32768,-32768,   100,   218,   147,   220,
    15,    51,   229,-32768,-32768,-32768,   205,-32768,   222,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,   224,
-32768,-32768,-32768,   147,    15,   218,-32768,-32768,-32768,    51,
   103,-32768,    87,    15,-32768,-32768,-32768,   104,-32768,-32768,
   225,    15,    51,   221,-32768,    51,   113,-32768,    36,-32768,
   226,   227,    51,-32768,-32768,   236,   239,-32768
};

static const short yypgoto[] = {     9,
   244,-32768,-32768,-32768,-32768,   114,-32768,-32768,    -2,   122,
-32768,   125,-32768,    58,   247,   -47,-32768,-32768,    55,-32768,
   151,   -25,   -70,   242,   -39,   -55,   143,   142,   -45,-32768,
   248,   254,   -26,  -161,   255
};


#define	YYLAST		257


static const short yytable[] = {    16,
    74,    76,    27,    28,    61,    32,   100,    64,   226,    95,
   202,    43,     4,   117,     4,    42,    45,    78,   102,     4,
    85,    -2,    76,    53,     4,    40,     1,    45,   171,    26,
     1,    65,   172,     4,    50,    72,     8,   151,   210,    51,
     4,    26,    41,   107,    74,   167,    86,   111,   173,    90,
    37,   218,    91,    44,   220,    87,    44,   149,   164,    13,
    14,   225,    60,    49,    13,    14,    98,    26,    74,    41,
    74,   112,   143,    88,    76,     4,    74,    95,   106,    62,
     5,     6,   152,    79,    80,   120,   109,   122,    66,   166,
    67,     7,     8,    81,    16,   168,     9,    -2,   110,   212,
   201,    82,   123,   213,    10,    11,     1,   110,   162,    12,
   190,    -2,    35,   194,   196,    13,    14,   211,   215,    36,
    91,    91,   191,    70,   208,   110,   110,   221,   177,   110,
   110,    74,    16,    16,    71,    16,    16,    16,    16,   110,
    54,   217,   199,    46,    55,    56,    46,    47,   214,    84,
    63,   154,   155,     4,   156,    74,   157,    30,    50,    34,
     4,    58,    38,    51,    76,     1,    59,   207,    46,   125,
    -2,   101,    74,   126,   127,   128,     7,     8,    96,   129,
   130,   131,   146,   147,   -86,   158,   159,    46,   104,    10,
   160,   165,   181,   182,    12,   184,   185,   186,   187,   132,
    13,    14,   129,   130,    99,   103,   105,   108,   114,   116,
   118,   115,   119,   121,   144,   142,   222,   145,   124,   148,
   150,   170,   111,   169,   174,   178,   180,   189,   183,   188,
   192,   195,   193,   203,   204,   227,   200,   205,   228,   197,
   206,   216,   223,   224,     3,   176,   179,   175,    17,    33,
   209,   219,   153,   163,    31,    24,    25
};

static const short yycheck[] = {     2,
    40,    41,     5,     6,    31,     8,    62,    33,     0,    55,
   172,     5,     5,    84,     5,    18,    19,    17,    66,     5,
    46,     0,    62,    26,     5,    13,     9,    30,    13,    41,
     9,    34,    17,     5,    12,    38,    22,   108,   200,    17,
     5,    41,    30,    17,    84,   116,    49,    40,    33,    21,
     5,   213,    55,    47,   216,     5,    47,   105,   114,    45,
    46,   223,    40,    29,    45,    46,    58,    41,   108,    30,
   110,    74,    99,    23,   114,     5,   116,   123,    70,    30,
    10,    11,   109,    30,    31,    88,    15,    12,    12,   115,
     5,    21,    22,    40,    97,    15,    26,     0,    27,    13,
   171,    48,    27,    17,    34,    35,     9,    27,   111,    39,
     5,    14,     5,    15,    15,    45,    46,    15,    15,    12,
   123,   124,    17,    12,   195,    27,    27,    15,   131,    27,
    27,   171,   135,   136,     5,   138,   139,   140,   141,    27,
    12,   212,   169,    28,    16,    17,    28,    32,   204,    13,
    32,     3,     4,     5,     6,   195,     8,     7,    12,     9,
     5,    12,    12,    17,   204,     9,    17,   194,    28,    14,
    14,    17,   212,    18,    19,    20,    21,    22,    43,    24,
    25,    26,     6,     7,    29,    37,    38,    28,    14,    34,
    42,    32,   135,   136,    39,   138,   139,   140,   141,    44,
    45,    46,    24,    25,    42,    40,    27,    13,    27,    13,
    13,    31,    40,    14,    31,    14,   219,    14,    18,     5,
    14,     5,    40,    15,    17,    16,    16,    32,    17,    17,
    17,    13,    17,     5,    30,     0,    17,    16,     0,    22,
    17,    17,    17,    17,     1,   124,   133,   123,     2,     8,
   196,    31,   110,   112,     7,     2,     2
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
#line 111 "yacc.yy"
{
	     dcop_area = 0; // reset
	  ;
    break;}
case 3:
#line 118 "yacc.yy"
{
		printf("<INCLUDE file=\"%s\"/>\n", yyvsp[-1]._str->latin1() );
	  ;
    break;}
case 4:
#line 122 "yacc.yy"
{
          ;
    break;}
case 5:
#line 127 "yacc.yy"
{ yyval._int = 1; ;
    break;}
case 6:
#line 128 "yacc.yy"
{ yyval._int = 0; ;
    break;}
case 7:
#line 132 "yacc.yy"
{
	 	if (yyvsp[-2]._int)
		  printf("<CLASS name=\"%s\">\n%s%s</CLASS>\n", yyvsp[-4]._str->latin1(), yyvsp[-3]._str->latin1(), yyvsp[-1]._str->latin1() );
	  ;
    break;}
case 8:
#line 137 "yacc.yy"
{
	  ;
    break;}
case 9:
#line 140 "yacc.yy"
{
	  ;
    break;}
case 10:
#line 143 "yacc.yy"
{
	  ;
    break;}
case 11:
#line 146 "yacc.yy"
{
	  ;
    break;}
case 12:
#line 149 "yacc.yy"
{
	  ;
    break;}
case 13:
#line 152 "yacc.yy"
{
	  ;
    break;}
case 14:
#line 155 "yacc.yy"
{
	  ;
    break;}
case 15:
#line 158 "yacc.yy"
{
	  ;
    break;}
case 24:
#line 169 "yacc.yy"
{
	  dcop_area = 0;
	;
    break;}
case 25:
#line 173 "yacc.yy"
{
	  dcop_area = 0;
	;
    break;}
case 26:
#line 179 "yacc.yy"
{
	  dcop_area = 1;
	;
    break;}
case 27:
#line 184 "yacc.yy"
{
	  yyval._str = yyvsp[0]._str;
	;
    break;}
case 28:
#line 187 "yacc.yy"
{
	   yyval._str = new QString( *(yyvsp[-2]._str) + *(yyvsp[0]._str) );
	;
    break;}
case 29:
#line 193 "yacc.yy"
{
		QString* tmp = new QString( "<SUPER name=\"%1\"/>\n" );
		*tmp = tmp->arg( *(yyvsp[0]._str) );
		yyval._str = tmp;
	  ;
    break;}
case 30:
#line 201 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 31:
#line 205 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 32:
#line 212 "yacc.yy"
{
		yyval._str = yyvsp[-1]._str;
	  ;
    break;}
case 33:
#line 216 "yacc.yy"
{
		/* $$ = $1; */
		yyval._str = new QString( *(yyvsp[-2]._str) + *(yyvsp[0]._str) );
	  ;
    break;}
case 34:
#line 224 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 35:
#line 228 "yacc.yy"
{
		yyval._str = new QString( "" );
	  ;
    break;}
case 36:
#line 235 "yacc.yy"
{
		yyval._str = new QString( "" );
	  ;
    break;}
case 37:
#line 239 "yacc.yy"
{
		yyval._str = new QString( *(yyvsp[-1]._str) + *(yyvsp[0]._str) );
	  ;
    break;}
case 38:
#line 243 "yacc.yy"
{
		yyval._str = new QString( *(yyvsp[-1]._str) + *(yyvsp[0]._str) );
	  ;
    break;}
case 39:
#line 247 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 40:
#line 251 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 41:
#line 255 "yacc.yy"
{	
	        yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 42:
#line 258 "yacc.yy"
{
 	        yyval._str = yyvsp[0]._str;
	;
    break;}
case 48:
#line 273 "yacc.yy"
{;
    break;}
case 49:
#line 274 "yacc.yy"
{;
    break;}
case 50:
#line 275 "yacc.yy"
{;
    break;}
case 51:
#line 279 "yacc.yy"
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
case 52:
#line 291 "yacc.yy"
{
		yyval._int = 0;
	  ;
    break;}
case 53:
#line 295 "yacc.yy"
{
		yyval._int = 1;
	  ;
    break;}
case 54:
#line 301 "yacc.yy"
{ yyval._str = new QString("signed"); ;
    break;}
case 55:
#line 302 "yacc.yy"
{ yyval._str = new QString("unsigned"); ;
    break;}
case 56:
#line 306 "yacc.yy"
{
		QString* tmp = new QString("<RET %1");
		*tmp = tmp->arg( *(yyvsp[0]._str) );
		yyval._str = tmp;
	  ;
    break;}
case 59:
#line 318 "yacc.yy"
{
		yyval._str = new QString( "" );
	  ;
    break;}
case 60:
#line 322 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 61:
#line 326 "yacc.yy"
{
		yyval._str = new QString( *(yyvsp[-2]._str) + *(yyvsp[0]._str) );
	  ;
    break;}
case 62:
#line 332 "yacc.yy"
{ yyval._str = new QString( *(yyvsp[-1]._str) + *(yyvsp[0]._str) ); ;
    break;}
case 63:
#line 333 "yacc.yy"
{ yyval._str = yyvsp[0]._str; ;
    break;}
case 64:
#line 336 "yacc.yy"
{
	     if (dcop_area)
		yyerror("in dcop areas are only const references allowed!");
	  ;
    break;}
case 65:
#line 340 "yacc.yy"
{
	     if (dcop_area) {
	  	QString* tmp = new QString(" type=\"%1\" qleft=\"const\" qright=\"" AMP_ENTITY "\"/>");
		*tmp = tmp->arg( *(yyvsp[-1]._str) );
		yyval._str = tmp;
	     }
	  ;
    break;}
case 66:
#line 347 "yacc.yy"
{
		if (dcop_area) {
		  QString* tmp = new QString(" type=\"%1<%2>\" qleft=\"const\" qright=\"" AMP_ENTITY "\"/>");
		  *tmp = tmp->arg( *(yyvsp[-4]._str) ).arg( *(yyvsp[-2]._str) );
		  yyval._str = tmp;
		}
	;
    break;}
case 67:
#line 354 "yacc.yy"
{
	    if (dcop_area)
	      yyerror("in dcop areas are only const references allowed!");	
	;
    break;}
case 68:
#line 358 "yacc.yy"
{
	    if (dcop_area)
	      yyerror("in dcop areas are no pointers allowed!");
	;
    break;}
case 69:
#line 362 "yacc.yy"
{
	     QString* tmp = new QString(" type=\"%1<%2>\" qleft=\"\" qright=\"\"/>");
	     *tmp = tmp->arg( *(yyvsp[-3]._str) ).arg( *(yyvsp[-1]._str) );
	     yyval._str = tmp;
	;
    break;}
case 70:
#line 367 "yacc.yy"
{
		QString* tmp = new QString(" type=\"%1\"/>");
		*tmp = tmp->arg( *(yyvsp[0]._str) );
		yyval._str = tmp;
	;
    break;}
case 71:
#line 373 "yacc.yy"
{
	    if (dcop_area)
	      yyerror("in dcop areas are no pointers allowed");
	  ;
    break;}
case 72:
#line 378 "yacc.yy"
{
	    if (dcop_area)
	      yyerror("in dcop areas are no pointers allowed");
	  ;
    break;}
case 73:
#line 385 "yacc.yy"
{
	    if (dcop_area)
		yyerror("two argument templates in dcop area currently not supported!");

	    yyval._str = new QString(*(yyvsp[-2]._str) + "," + *(yyvsp[0]._str));
	  ;
    break;}
case 74:
#line 392 "yacc.yy"
{
 	    yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 75:
#line 398 "yacc.yy"
{
		if (dcop_area) {
		   QString* tmp = new QString("<ARG name=\"%1\"" + *(yyvsp[-2]._str));
  		   *tmp = tmp->arg( *(yyvsp[-1]._str) );
		   yyval._str = tmp;		
		} else yyval._str = new QString();
	  ;
    break;}
case 76:
#line 406 "yacc.yy"
{
		if (dcop_area)
		  yyerror("in dcoparea you have to specify paramater names!");
		yyval._str = new QString();
	  ;
    break;}
case 77:
#line 414 "yacc.yy"
{
	  ;
    break;}
case 78:
#line 417 "yacc.yy"
{
	  ;
    break;}
case 79:
#line 420 "yacc.yy"
{
	  ;
    break;}
case 80:
#line 423 "yacc.yy"
{
	  ;
    break;}
case 81:
#line 426 "yacc.yy"
{
	  ;
    break;}
case 82:
#line 429 "yacc.yy"
{
	  ;
    break;}
case 83:
#line 432 "yacc.yy"
{
	  ;
    break;}
case 84:
#line 435 "yacc.yy"
{
	  ;
    break;}
case 85:
#line 438 "yacc.yy"
{
	  ;
    break;}
case 86:
#line 443 "yacc.yy"
{ yyval._int = 0; ;
    break;}
case 87:
#line 444 "yacc.yy"
{ yyval._int = 1; ;
    break;}
case 92:
#line 451 "yacc.yy"
{
	     if (dcop_area) {
		QString* tmp = new QString("<FUNC name=\"%1\" qual=\"%4\">%2%3</FUNC>\n");
		*tmp = tmp->arg( *(yyvsp[-4]._str) );
		*tmp = tmp->arg( *(yyvsp[-5]._str) );
		*tmp = tmp->arg( *(yyvsp[-2]._str) );
		*tmp = tmp->arg(yyvsp[0]._int ? "const" : "");
		yyval._str = tmp;
   	     } else
	        yyval._str = new QString("");
	  ;
    break;}
case 93:
#line 463 "yacc.yy"
{
	     if (dcop_area)
		yyerror("operators aren't allowed in dcop areas!");
	     yyval._str = new QString("");
	  ;
    break;}
case 94:
#line 471 "yacc.yy"
{
	        yyval._str = yyvsp[-1]._str;
	  ;
    break;}
case 95:
#line 475 "yacc.yy"
{
		yyval._str = yyvsp[-3]._str;
	  ;
    break;}
case 96:
#line 479 "yacc.yy"
{
		yyval._str = yyvsp[-1]._str;
	  ;
    break;}
case 97:
#line 483 "yacc.yy"
{
	      /* The constructor */
	      assert(!dcop_area);
              yyval._str = new QString("");
	  ;
    break;}
case 98:
#line 489 "yacc.yy"
{
	      /* The destructor */
  	      assert(!dcop_area);
              yyval._str = new QString("");
	  ;
    break;}
case 99:
#line 495 "yacc.yy"
{
		if (dcop_area)
		  yyerror("static is not allowed in dcop area!");
		yyval._str = new QString();
	  ;
    break;}
case 103:
#line 507 "yacc.yy"
{;
    break;}
case 104:
#line 508 "yacc.yy"
{;
    break;}
case 105:
#line 509 "yacc.yy"
{;
    break;}
case 106:
#line 510 "yacc.yy"
{;
    break;}
case 107:
#line 511 "yacc.yy"
{;
    break;}
case 108:
#line 512 "yacc.yy"
{;
    break;}
case 109:
#line 516 "yacc.yy"
{;
    break;}
case 110:
#line 517 "yacc.yy"
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
#line 519 "yacc.yy"


void dcopidlParse( const char *_code )
{
    printf("<!DOCTYPE DCOP-IDL><DCOP-IDL>\n");
    dcopidlInitFlex( _code );
    yyparse();
    printf("</DCOP-IDL>\n");
}
