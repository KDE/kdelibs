
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
#define	T_ACCESS	292
#define	T_ENUM	293
#define	T_NAMESPACE	294
#define	T_UNKNOWN	295
#define	T_TRIPLE_DOT	296
#define	T_TRUE	297
#define	T_FALSE	298
#define	T_STATIC	299
#define	T_EQUAL	300
#define	T_SCOPE	301
#define	T_NULL	302
#define	T_INT	303
#define	T_ARRAY_OPEN	304
#define	T_ARRAY_CLOSE	305
#define	T_CHAR	306
#define	T_DCOP	307
#define	T_DCOP_AREA	308
#define	T_SIGNED	309
#define	T_UNSIGNED	310
#define	T_LONG	311
#define	T_SHORT	312
#define	T_FUNOPERATOR	313
#define	T_MISCOPERATOR	314

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
extern int function_mode;

static int dcop_area = 0;

void dcopidlInitFlex( const char *_code );

void yyerror( const char *s )
{
	qDebug( "In line %i : %s", idl_line_no, s );
        exit(1);
	//   theParser->parse_error( idl_lexFile, s, idl_line_no );
}


#line 32 "yacc.yy"
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



#define	YYFINAL		280
#define	YYFLAG		-32768
#define	YYNTBASE	61

#define YYTRANSLATE(x) ((unsigned)(x) <= 314 ? yytranslate[x] : 107)

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
    57,    58,    59,    60
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
   239,   242,   245,   249,   251,   253,   255,   257,   260,   263,
   266,   268,   269,   271,   275,   277,   279,   282,   285,   290,
   297,   301,   305,   308,   311,   313,   316,   320,   322,   326,
   329,   331,   332,   335,   338,   341,   344,   350,   351,   353,
   355,   357,   359,   361,   368,   376,   378,   380,   384,   389,
   391,   395,   398,   404,   408,   414,   422,   429,   433,   435,
   437,   441,   446,   449,   450,   452,   455,   456,   459,   463,
   468
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
     0,    55,    57,     0,    55,    58,    49,     0,    49,     0,
    57,     0,    58,     0,    52,     0,    55,    52,     0,    56,
    52,     0,    32,    85,     0,    32,     0,     0,    90,     0,
    86,    31,    90,     0,    84,     0,    71,     0,    10,    71,
     0,     9,    71,     0,    71,    34,    89,    35,     0,    71,
    34,    89,    35,    47,    71,     0,    22,    87,    85,     0,
    22,    87,    36,     0,    22,    87,     0,    87,    36,     0,
    87,     0,    87,    85,     0,    88,    31,    89,     0,    88,
     0,    88,    71,    91,     0,    88,    91,     0,    42,     0,
     0,    46,     7,     0,    46,    81,     0,    46,     4,     0,
    46,    66,     0,    46,    71,    12,    86,    14,     0,     0,
    21,     0,    60,     0,    35,     0,    34,     0,    46,     0,
    88,    71,    12,    86,    14,    83,     0,    88,    59,    93,
    12,    86,    14,    83,     0,    80,     0,    95,     0,    95,
    31,    96,     0,     5,    12,    96,    14,     0,    97,     0,
    97,    31,    98,     0,    94,   101,     0,    21,    94,    46,
    48,   101,     0,    21,    94,   101,     0,    71,    12,    86,
    14,   101,     0,    71,    12,    86,    14,    15,    98,   101,
     0,    92,    33,    71,    12,    14,   101,     0,    45,    94,
   101,     0,    11,     0,    16,     0,   100,   102,    13,     0,
   100,   102,    13,    16,     0,   103,   102,     0,     0,    16,
     0,    31,   105,     0,     0,     5,   104,     0,    88,   105,
    16,     0,    45,    88,     5,    16,     0,    88,     5,    50,
    81,    51,    16,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
   122,   125,   128,   132,   135,   141,   142,   145,   150,   153,
   156,   159,   162,   165,   168,   171,   174,   177,   180,   185,
   186,   188,   188,   190,   190,   190,   192,   192,   192,   195,
   199,   205,   211,   214,   219,   227,   231,   238,   242,   250,
   254,   261,   265,   269,   273,   277,   281,   285,   289,   293,
   299,   300,   301,   304,   305,   308,   309,   312,   313,   314,
   315,   316,   319,   320,   323,   333,   340,   344,   351,   352,
   353,   354,   355,   356,   357,   358,   359,   360,   361,   364,
   365,   368,   372,   373,   380,   381,   382,   383,   384,   389,
   397,   402,   409,   414,   419,   424,   432,   436,   442,   450,
   456,   464,   467,   470,   473,   476,   479,   485,   486,   489,
   489,   489,   489,   492,   504,   511,   514,   515,   518,   521,
   522,   525,   529,   533,   537,   543,   549,   555,   562,   568,
   569,   570,   573,   574,   577,   580,   581,   583,   586,   587,
   588
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
    84,    84,    84,    84,    84,    84,    84,    84,    84,    85,
    85,    86,    86,    86,    87,    87,    87,    87,    87,    87,
    88,    88,    88,    88,    88,    88,    89,    89,    90,    90,
    90,    91,    91,    91,    91,    91,    91,    92,    92,    93,
    93,    93,    93,    94,    94,    95,    96,    96,    97,    98,
    98,    99,    99,    99,    99,    99,    99,    99,   100,   101,
   101,   101,   102,   102,   103,   104,   104,   105,   106,   106,
   106
};

static const short yyr2[] = {     0,
     3,     0,     2,     4,     0,     1,     0,     6,     3,     3,
     6,     6,     4,     7,     8,     2,     1,     1,     1,     2,
     0,     1,     1,     1,     1,     1,     1,     1,     0,     3,
     2,     2,     1,     3,     1,     3,     1,     2,     3,     2,
     1,     1,     2,     2,     2,     2,     2,     2,     5,     4,
     7,     6,     6,     3,     1,     3,     1,     1,     1,     2,
     1,     1,     1,     3,     7,     9,     0,     1,     1,     1,
     2,     2,     3,     1,     1,     1,     1,     2,     2,     2,
     1,     0,     1,     3,     1,     1,     2,     2,     4,     6,
     3,     3,     2,     2,     1,     2,     3,     1,     3,     2,
     1,     0,     2,     2,     2,     2,     5,     0,     1,     1,
     1,     1,     1,     6,     7,     1,     1,     3,     4,     1,
     3,     2,     5,     3,     5,     7,     6,     3,     1,     1,
     3,     4,     2,     0,     1,     2,     0,     2,     3,     4,
     6
};

static const short yydefact[] = {     5,
     5,     0,   108,     3,     5,    33,     0,     0,   109,     0,
   108,     0,     0,     0,     0,    74,    77,    69,    70,    75,
    76,     5,    86,    19,    85,    95,     0,     0,     0,    17,
    18,     0,     0,    88,    87,     0,     0,    86,     0,     0,
    93,     0,    16,     0,     0,     0,     0,     0,     0,     0,
    78,    72,    71,    79,     1,    82,     0,    81,    94,    96,
    33,     0,     0,     0,     0,   129,   130,   134,   122,     4,
    34,    41,   108,     9,     7,     5,    10,    88,    87,     0,
   124,    92,    91,    21,    87,     0,     0,    57,     0,    55,
     5,    33,   128,    73,   101,     0,   102,    83,    98,     0,
    80,     0,     0,   138,   112,   111,   113,   110,     0,    82,
   139,     0,   135,     0,   134,   109,    35,    37,     0,    40,
     0,     6,    29,     0,     0,     0,     0,     0,    21,    21,
    13,     0,     0,     0,     0,     0,   140,     0,     0,     0,
   102,   100,     0,    89,   137,   136,    58,    59,     0,    61,
    62,    63,     0,    82,     0,     0,   131,   133,    38,   108,
     0,    42,    26,    25,    24,     0,    27,    28,     0,     0,
    29,     0,    29,    29,     0,    29,    29,    29,    29,     0,
   123,     0,     0,   137,    20,     0,     0,    56,     0,    54,
     0,     0,   125,    84,   105,   103,    22,    23,   106,    62,
   104,    99,    97,     0,    60,     0,     0,     0,    67,     0,
   132,    39,    36,     0,     0,     0,    32,     0,    31,    47,
    46,     8,    45,    43,    44,    48,    11,     0,     0,     0,
     0,    52,    53,    12,     0,   120,     0,    82,    90,    64,
   141,    67,    68,   114,   127,     0,    29,     0,    30,    14,
     0,    51,     0,     0,   126,     0,   115,    29,    50,     0,
    15,   116,   117,     0,   121,   107,    49,     0,     0,   119,
     0,     0,   118,     0,    65,     0,    66,     0,     0,     0
};

static const short yydefgoto[] = {    32,
     3,   123,    22,   127,   199,   171,   172,   173,   174,    38,
   118,   119,   120,    75,   175,   176,    89,    90,   152,   153,
   177,   244,    25,    60,    96,    26,    27,   100,    98,   142,
    28,   109,    29,   263,   264,   236,   237,   178,    68,    69,
   114,   115,   104,    64,   179
};

static const short yypact[] = {    15,
    47,    20,   224,-32768,    75,   -26,    38,    38,   289,   309,
   134,   299,   104,    52,   289,-32768,-32768,    96,    33,-32768,
-32768,    12,     6,-32768,-32768,   149,     8,   127,   184,-32768,
-32768,    86,    38,   183,   194,    38,    38,    88,     9,    64,
   152,   289,-32768,   156,    38,   151,   167,   176,    14,   184,
-32768,-32768,   147,-32768,-32768,    69,   289,   165,-32768,-32768,
    66,    68,   206,   199,    38,-32768,-32768,   200,-32768,-32768,
-32768,-32768,    72,-32768,   166,    75,-32768,-32768,-32768,   172,
-32768,-32768,-32768,   279,   212,   219,   167,   190,   225,   208,
    75,    22,-32768,-32768,-32768,   105,    30,-32768,   209,   207,
-32768,   236,    23,-32768,-32768,-32768,-32768,-32768,   239,    69,
-32768,   242,-32768,   235,   200,-32768,-32768,-32768,    16,-32768,
   240,-32768,   204,   254,   184,   289,   255,   265,   279,   279,
-32768,   259,    23,   269,   167,   262,-32768,   191,    69,   365,
   232,-32768,   289,   238,   252,-32768,-32768,-32768,   280,-32768,
-32768,   261,   241,    69,   121,   277,   281,-32768,-32768,    72,
    38,-32768,-32768,-32768,-32768,   203,-32768,-32768,    38,   278,
   114,   285,   204,   204,   286,   204,   204,   204,   204,   287,
-32768,   290,    38,    58,-32768,   283,   101,-32768,   291,-32768,
   294,   300,-32768,-32768,-32768,-32768,-32768,-32768,-32768,   301,
-32768,-32768,-32768,    38,-32768,    23,   296,   128,   284,   184,
-32768,-32768,-32768,    38,   304,   282,-32768,   302,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,   306,   307,    38,
   310,-32768,-32768,-32768,   303,   298,   184,    69,-32768,-32768,
-32768,   284,-32768,-32768,-32768,   311,   204,   289,-32768,-32768,
   314,-32768,    23,   300,-32768,   133,-32768,   204,-32768,   297,
-32768,-32768,   308,   319,-32768,-32768,-32768,    11,    23,-32768,
   320,   324,-32768,    38,-32768,   326,-32768,   343,   349,-32768
};

static const short yypgoto[] = {    10,
   351,-32768,-32768,    21,-32768,-32768,   179,-32768,-32768,    -3,
   192,-32768,   202,-32768,  -113,   356,   -76,-32768,  -197,   -96,
-32768,   118,-32768,   -19,  -109,   353,    39,  -141,   234,   233,
   -70,-32768,    89,-32768,   106,-32768,   122,   135,-32768,   -33,
   263,-32768,-32768,   275,     3
};


#define	YYLAST		413


static const short yytable[] = {    23,
   155,   203,   121,    34,    35,    31,    81,    23,   240,   278,
   132,    -2,    61,     6,    -2,     6,    93,    56,    92,     1,
    33,    83,     1,    63,    -2,   147,   159,     6,   148,    71,
     5,    55,    78,    79,     6,    63,   188,   137,   101,    57,
    85,    86,     6,   201,   208,    63,   160,    39,     2,    39,
    45,     2,   149,    49,     1,   262,    48,   271,   190,   220,
   221,   112,   223,   224,   225,   226,    62,    62,    33,   117,
   150,   262,    62,     6,    66,   140,     6,    36,    37,    67,
    39,  -137,     1,     2,    54,   124,   129,    -2,   102,   121,
    10,   181,   116,   141,    97,    99,   102,    40,    70,   151,
   136,   105,   106,    50,   193,   231,   260,   103,    46,    80,
    95,     2,    33,   107,    47,   103,   232,    16,   138,    23,
    17,    57,   128,    18,    19,    20,    21,   108,   256,   151,
    50,   129,   129,   259,   209,   139,   200,    30,     6,   167,
   168,   242,    36,    37,   267,    43,   266,    51,    97,   185,
   186,   139,    52,    53,     9,    10,   117,   117,   139,    65,
     6,    87,   215,   139,   182,   216,    84,   128,   128,    23,
    23,    88,    23,    23,    23,    23,   245,    97,    42,   229,
    58,    99,    16,    58,    59,    17,    91,    82,    18,    19,
    20,    21,    97,    72,    66,    94,    58,    73,    74,    67,
   239,    66,   151,   255,    76,   192,    67,     6,     6,    77,
   246,   214,    36,    37,   111,   113,   162,   110,   122,   125,
   163,   164,   130,   165,     9,    10,   251,   166,     6,   167,
   168,   169,     7,     8,   131,   133,  -108,   134,   135,   143,
   145,   144,    13,    23,     9,    10,    11,   157,    15,   151,
   154,    12,    16,   156,    23,    17,   161,   170,    18,    19,
    20,    21,    13,    14,   272,   151,   180,   183,    15,   184,
   276,   187,    16,   189,   191,    17,    97,   140,    18,    19,
    20,    21,   102,     6,   204,   205,    99,    36,    37,   206,
   210,   207,   217,     6,   228,   230,   211,    36,    37,   219,
    10,   222,   227,     6,   235,   243,   233,    36,    44,   234,
    10,   241,   238,     6,   253,   248,   249,    36,    37,   247,
    10,   137,   250,   126,   274,   252,   258,    16,   254,   261,
    17,   268,   270,    18,    19,    20,    21,    16,   269,   275,
    17,   277,   279,    18,    19,    20,    21,    16,   280,   218,
    17,     4,   213,    18,    19,    20,    21,    16,    24,   257,
    17,   212,    41,    18,    19,    20,    21,   147,   195,     6,
   148,   196,   194,   202,   273,   265,   146,   158,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,   149,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,   197,   198,     0,
     0,     0,   150
};

static const short yycheck[] = {     3,
   110,   143,    73,     7,     8,     3,    40,    11,   206,     0,
    87,     0,     5,     5,     0,     5,    50,    12,     5,     8,
    47,    41,     8,    27,    13,     3,    11,     5,     6,    33,
    11,    22,    36,    37,     5,    39,   133,    16,    58,    34,
    44,    45,     5,   140,   154,    49,    31,     9,    37,    11,
    12,    37,    30,    15,     8,   253,     5,    47,   135,   173,
   174,    65,   176,   177,   178,   179,    59,    59,    47,    73,
    48,   269,    59,     5,    11,    46,     5,     9,    10,    16,
    42,    16,     8,    37,    52,    76,    84,    13,    31,   160,
    22,   125,    21,    97,    56,    57,    31,     9,    13,   103,
    91,    34,    35,    15,   138,     5,   248,    50,     5,    46,
    42,    37,    47,    46,    11,    50,    16,    49,    14,   123,
    52,    34,    84,    55,    56,    57,    58,    60,   238,   133,
    42,   129,   130,   247,    14,    31,   140,     3,     5,    26,
    27,    14,     9,    10,   258,    11,    14,    52,   110,   129,
   130,    31,    57,    58,    21,    22,   160,   161,    31,    33,
     5,    11,   166,    31,   126,   169,    11,   129,   130,   173,
   174,     5,   176,   177,   178,   179,   210,   139,    45,   183,
    32,   143,    49,    32,    36,    52,    11,    36,    55,    56,
    57,    58,   154,    11,    11,    49,    32,    15,    16,    16,
   204,    11,   206,   237,    11,    15,    16,     5,     5,    16,
   214,     9,     9,    10,    16,    16,    13,    12,    53,    48,
    17,    18,    11,    20,    21,    22,   230,    24,     5,    26,
    27,    28,     9,    10,    16,    46,    33,    13,    31,    31,
     5,    35,    39,   247,    21,    22,    23,    13,    45,   253,
    12,    28,    49,    12,   258,    52,    17,    54,    55,    56,
    57,    58,    39,    40,   268,   269,    13,    13,    45,     5,
   274,    13,    49,     5,    13,    52,   238,    46,    55,    56,
    57,    58,    31,     5,    47,     6,   248,     9,    10,    29,
    14,    51,    15,     5,     5,    13,    16,     9,    10,    15,
    22,    16,    16,     5,     5,    22,    16,     9,    10,    16,
    22,    16,    12,     5,    12,    34,    15,     9,    10,    16,
    22,    16,    16,    45,     5,    16,    16,    49,    31,    16,
    52,    35,    14,    55,    56,    57,    58,    49,    31,    16,
    52,    16,     0,    55,    56,    57,    58,    49,     0,   171,
    52,     1,   161,    55,    56,    57,    58,    49,     3,   242,
    52,   160,    10,    55,    56,    57,    58,     3,     4,     5,
     6,     7,   139,   141,   269,   254,   102,   115,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    30,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    43,    44,    -1,
    -1,    -1,    48
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
#line 123 "yacc.yy"
{
	  ;
    break;}
case 3:
#line 129 "yacc.yy"
{
		printf("<INCLUDE file=\"%s\"/>\n", yyvsp[-1]._str->latin1() );
	  ;
    break;}
case 4:
#line 133 "yacc.yy"
{
	  ;
    break;}
case 5:
#line 136 "yacc.yy"
{
          ;
    break;}
case 6:
#line 141 "yacc.yy"
{ yyval._int = 1; ;
    break;}
case 7:
#line 142 "yacc.yy"
{ yyval._int = 0; ;
    break;}
case 8:
#line 146 "yacc.yy"
{
	 	if (yyvsp[-2]._int)
		  printf("<CLASS name=\"%s\">\n%s%s</CLASS>\n", yyvsp[-4]._str->latin1(), yyvsp[-3]._str->latin1(), yyvsp[-1]._str->latin1() );
	  ;
    break;}
case 9:
#line 151 "yacc.yy"
{
	  ;
    break;}
case 10:
#line 154 "yacc.yy"
{
	  ;
    break;}
case 11:
#line 157 "yacc.yy"
{
	  ;
    break;}
case 12:
#line 160 "yacc.yy"
{
	  ;
    break;}
case 13:
#line 163 "yacc.yy"
{
	  ;
    break;}
case 14:
#line 166 "yacc.yy"
{
	  ;
    break;}
case 15:
#line 169 "yacc.yy"
{
	  ;
    break;}
case 16:
#line 172 "yacc.yy"
{
	  ;
    break;}
case 17:
#line 175 "yacc.yy"
{
	  ;
    break;}
case 18:
#line 178 "yacc.yy"
{
	  ;
    break;}
case 19:
#line 181 "yacc.yy"
{
	  ;
    break;}
case 30:
#line 196 "yacc.yy"
{
	  dcop_area = 0;
	;
    break;}
case 31:
#line 200 "yacc.yy"
{
	  dcop_area = 0;
	;
    break;}
case 32:
#line 206 "yacc.yy"
{
	  dcop_area = 1;
	;
    break;}
case 33:
#line 211 "yacc.yy"
{
	  yyval._str = yyvsp[0]._str;
	;
    break;}
case 34:
#line 214 "yacc.yy"
{
	   yyval._str = new QString( *(yyvsp[-2]._str) + *(yyvsp[0]._str) );
	;
    break;}
case 35:
#line 220 "yacc.yy"
{
		QString* tmp = new QString( "<SUPER name=\"%1\"/>\n" );
		*tmp = tmp->arg( *(yyvsp[0]._str) );
		yyval._str = tmp;
	  ;
    break;}
case 36:
#line 228 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 37:
#line 232 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 38:
#line 239 "yacc.yy"
{
		yyval._str = yyvsp[-1]._str;
	  ;
    break;}
case 39:
#line 243 "yacc.yy"
{
		/* $$ = $1; */
		yyval._str = new QString( *(yyvsp[-2]._str) + *(yyvsp[0]._str) );
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
		yyval._str = new QString( "" );
	  ;
    break;}
case 42:
#line 262 "yacc.yy"
{
		yyval._str = new QString( "" );
	  ;
    break;}
case 43:
#line 266 "yacc.yy"
{
		yyval._str = new QString( *(yyvsp[-1]._str) + *(yyvsp[0]._str) );
	  ;
    break;}
case 44:
#line 270 "yacc.yy"
{
		yyval._str = new QString( *(yyvsp[-1]._str) + *(yyvsp[0]._str) );
	  ;
    break;}
case 45:
#line 274 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 46:
#line 278 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 47:
#line 282 "yacc.yy"
{	
	        yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 48:
#line 286 "yacc.yy"
{
 	        yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 49:
#line 290 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 50:
#line 294 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 56:
#line 308 "yacc.yy"
{;
    break;}
case 57:
#line 309 "yacc.yy"
{;
    break;}
case 58:
#line 312 "yacc.yy"
{;
    break;}
case 59:
#line 313 "yacc.yy"
{;
    break;}
case 60:
#line 314 "yacc.yy"
{;
    break;}
case 61:
#line 315 "yacc.yy"
{;
    break;}
case 62:
#line 316 "yacc.yy"
{;
    break;}
case 63:
#line 319 "yacc.yy"
{;
    break;}
case 64:
#line 320 "yacc.yy"
{;
    break;}
case 65:
#line 324 "yacc.yy"
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
#line 334 "yacc.yy"
{
		if (dcop_area)
		  yyerror("scoped template typedefs are not supported in dcop areas!");
	  ;
    break;}
case 67:
#line 341 "yacc.yy"
{
		yyval._int = 0;
	  ;
    break;}
case 68:
#line 345 "yacc.yy"
{
		yyval._int = 1;
	  ;
    break;}
case 69:
#line 351 "yacc.yy"
{ yyval._str = new QString("signed int"); ;
    break;}
case 70:
#line 352 "yacc.yy"
{ yyval._str = new QString("unsigned int"); ;
    break;}
case 71:
#line 353 "yacc.yy"
{ yyval._str = new QString("signed short int"); ;
    break;}
case 72:
#line 354 "yacc.yy"
{ yyval._str = new QString("signed long int"); ;
    break;}
case 73:
#line 355 "yacc.yy"
{ yyval._str = new QString("signed short int"); ;
    break;}
case 74:
#line 356 "yacc.yy"
{ yyval._str = new QString("int"); ;
    break;}
case 75:
#line 357 "yacc.yy"
{ yyval._str = new QString("long int"); ;
    break;}
case 76:
#line 358 "yacc.yy"
{ yyval._str = new QString("short int"); ;
    break;}
case 77:
#line 359 "yacc.yy"
{ yyval._str = new QString("char"); ;
    break;}
case 78:
#line 360 "yacc.yy"
{ yyval._str = new QString("signed char"); ;
    break;}
case 79:
#line 361 "yacc.yy"
{ yyval._str = new QString("unsigned char"); ;
    break;}
case 82:
#line 369 "yacc.yy"
{
		yyval._str = new QString( "" );
	  ;
    break;}
case 84:
#line 374 "yacc.yy"
{
		yyval._str = new QString( *(yyvsp[-2]._str) + *(yyvsp[0]._str) );
	  ;
    break;}
case 86:
#line 381 "yacc.yy"
{ yyval._str = yyvsp[0]._str; ;
    break;}
case 87:
#line 382 "yacc.yy"
{ yyval._str = yyvsp[0]._str; ;
    break;}
case 88:
#line 383 "yacc.yy"
{ yyval._str = yyvsp[0]._str; ;
    break;}
case 89:
#line 384 "yacc.yy"
{
		QString *tmp = new QString("%1<%2>");
		tmp->arg(*(yyvsp[-3]._str));
		tmp->arg(*(yyvsp[-1]._str));
	 ;
    break;}
case 90:
#line 389 "yacc.yy"
{
		QString *tmp = new QString("%1<%2>::%3");
		tmp->arg(*(yyvsp[-5]._str));
		tmp->arg(*(yyvsp[-3]._str));
		tmp->arg(*(yyvsp[0]._str));
	 ;
    break;}
case 91:
#line 398 "yacc.yy"
{
	    if (dcop_area)
	      yyerror("in dcop areas are no pointers allowed");
	  ;
    break;}
case 92:
#line 402 "yacc.yy"
{
	     if (dcop_area) {
	  	QString* tmp = new QString(" type=\"%1\" qleft=\"const\" qright=\"" AMP_ENTITY "\"/>");
		*tmp = tmp->arg( *(yyvsp[-1]._str) );
		yyval._str = tmp;
	     }
	  ;
    break;}
case 93:
#line 409 "yacc.yy"
{
		QString* tmp = new QString(" type=\"%1\"/>");
		*tmp = tmp->arg( *(yyvsp[0]._str) );
		yyval._str = tmp;
	;
    break;}
case 94:
#line 414 "yacc.yy"
{
	     if (dcop_area)
		yyerror("in dcop areas are only const references allowed!");
	  ;
    break;}
case 95:
#line 419 "yacc.yy"
{
		QString* tmp = new QString(" type=\"%1\"/>");
		*tmp = tmp->arg( *(yyvsp[0]._str) );
		yyval._str = tmp;
	;
    break;}
case 96:
#line 425 "yacc.yy"
{
	    if (dcop_area)
	      yyerror("in dcop areas are no pointers allowed");
	  ;
    break;}
case 97:
#line 433 "yacc.yy"
{
	    yyval._str = new QString(*(yyvsp[-2]._str) + "," + *(yyvsp[0]._str));
	  ;
    break;}
case 98:
#line 437 "yacc.yy"
{
 	    yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 99:
#line 443 "yacc.yy"
{
		if (dcop_area) {
		   QString* tmp = new QString("<ARG name=\"%1\"" + *(yyvsp[-2]._str));
  		   *tmp = tmp->arg( *(yyvsp[-1]._str) );
		   yyval._str = tmp;		
		} else yyval._str = new QString();
	  ;
    break;}
case 100:
#line 451 "yacc.yy"
{
		if (dcop_area)
		  yyerror("in dcoparea you have to specify paramater names!");
		yyval._str = new QString();
	  ;
    break;}
case 101:
#line 457 "yacc.yy"
{
		if (dcop_area)
			yyerror("variable arguments not supported in dcop area!");
		yyval._str = new QString("");
	  ;
    break;}
case 102:
#line 465 "yacc.yy"
{
	  ;
    break;}
case 103:
#line 468 "yacc.yy"
{
	  ;
    break;}
case 104:
#line 471 "yacc.yy"
{
	  ;
    break;}
case 105:
#line 474 "yacc.yy"
{
	  ;
    break;}
case 106:
#line 477 "yacc.yy"
{
	  ;
    break;}
case 107:
#line 480 "yacc.yy"
{
	  ;
    break;}
case 108:
#line 485 "yacc.yy"
{ yyval._int = 0; ;
    break;}
case 109:
#line 486 "yacc.yy"
{ yyval._int = 1; ;
    break;}
case 114:
#line 493 "yacc.yy"
{
	     if (dcop_area) {
		QString* tmp = new QString("<FUNC name=\"%1\" qual=\"%4\"><RET%2%3</FUNC>\n");
		*tmp = tmp->arg( *(yyvsp[-4]._str) );
		*tmp = tmp->arg( *(yyvsp[-5]._str) );
		*tmp = tmp->arg( *(yyvsp[-2]._str) );
		*tmp = tmp->arg(yyvsp[0]._int ? "const" : "");
		yyval._str = tmp;
   	     } else
	        yyval._str = new QString("");
	  ;
    break;}
case 115:
#line 505 "yacc.yy"
{
	     if (dcop_area)
		yyerror("operators aren't allowed in dcop areas!");
	     yyval._str = new QString("");
	  ;
    break;}
case 116:
#line 511 "yacc.yy"
{;
    break;}
case 117:
#line 514 "yacc.yy"
{;
    break;}
case 118:
#line 515 "yacc.yy"
{;
    break;}
case 119:
#line 518 "yacc.yy"
{;
    break;}
case 120:
#line 521 "yacc.yy"
{;
    break;}
case 121:
#line 522 "yacc.yy"
{;
    break;}
case 122:
#line 526 "yacc.yy"
{
	        yyval._str = yyvsp[-1]._str;
	  ;
    break;}
case 123:
#line 530 "yacc.yy"
{
		yyval._str = yyvsp[-3]._str;
	  ;
    break;}
case 124:
#line 534 "yacc.yy"
{
		yyval._str = yyvsp[-1]._str;
	  ;
    break;}
case 125:
#line 538 "yacc.yy"
{
	      /* The constructor */
	      assert(!dcop_area);
              yyval._str = new QString("");
	  ;
    break;}
case 126:
#line 544 "yacc.yy"
{
	      /* The constructor */
	      assert(!dcop_area);
              yyval._str = new QString("");
	  ;
    break;}
case 127:
#line 550 "yacc.yy"
{
	      /* The destructor */
  	      assert(!dcop_area);
              yyval._str = new QString("");
	  ;
    break;}
case 128:
#line 556 "yacc.yy"
{
		if (dcop_area)
		  yyerror("static is not allowed in dcop area!");
		yyval._str = new QString();
	  ;
    break;}
case 129:
#line 563 "yacc.yy"
{
		function_mode = 1;
	;
    break;}
case 133:
#line 573 "yacc.yy"
{;
    break;}
case 134:
#line 574 "yacc.yy"
{;
    break;}
case 138:
#line 583 "yacc.yy"
{;
    break;}
case 139:
#line 586 "yacc.yy"
{;
    break;}
case 140:
#line 587 "yacc.yy"
{;
    break;}
case 141:
#line 588 "yacc.yy"
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
#line 590 "yacc.yy"


void dcopidlParse( const char *_code )
{
    printf("<!DOCTYPE DCOP-IDL><DCOP-IDL>\n");
    dcopidlInitFlex( _code );
    yyparse();
    printf("</DCOP-IDL>\n");
}
