
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
#define	T_SIGNAL	276
#define	T_SLOT	277
#define	T_TYPEDEF	278
#define	T_COMMA	279
#define	T_ASTERISK	280
#define	T_TILDE	281
#define	T_LESS	282
#define	T_GREATER	283
#define	T_AMPERSAND	284
#define	T_ENUM	285
#define	T_UNKNOWN	286
#define	T_EQUAL	287
#define	T_SCOPE	288
#define	T_NULL	289
#define	T_DCOP	290
#define	T_DCOP_AREA	291

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



#define	YYFINAL		148
#define	YYFLAG		-32768
#define	YYNTBASE	38

#define YYTRANSLATE(x) ((unsigned)(x) <= 291 ? yytranslate[x] : 61)

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
    37
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     3,     6,     7,    14,    19,    24,    26,    28,    30,
    32,    34,    35,    39,    42,    44,    48,    51,    53,    56,
    60,    63,    65,    67,    70,    73,    76,    79,    82,    90,
    92,    96,    97,    99,   101,   105,   107,   111,   116,   123,
   125,   132,   138,   141,   142,   144,   148,   154,   158,   164,
   171,   179,   182,   183,   186,   189,   192,   195,   198,   203,
   214,   220,   226,   236,   245,   253
};

static const short yyrhs[] = {    39,
    40,     0,     8,    39,     0,     0,     9,     5,    48,    36,
    49,    16,     0,     9,     5,    16,    38,     0,    10,     5,
    16,    38,     0,    19,     0,    18,     0,    17,     0,    22,
     0,    23,     0,     0,    41,    42,    15,     0,    37,    15,
     0,     5,     0,    20,    17,    45,     0,    17,    45,     0,
    45,     0,    46,    11,     0,    46,    25,    47,     0,    15,
    47,     0,    11,     0,    13,     0,    50,    49,     0,    59,
    49,     0,    44,    49,     0,    43,    49,     0,    60,    49,
     0,    24,     5,    28,    51,    29,     5,    16,     0,     5,
     0,     5,    25,    51,     0,     0,    21,     0,     5,     0,
     5,    25,    53,     0,     5,     0,    21,     5,    30,     0,
     5,    28,    53,    29,     0,    21,     5,    28,    53,    29,
    30,     0,    55,     0,    21,     5,    28,    53,    29,    26,
     0,     5,    28,    53,    29,    26,     0,     5,    26,     0,
     0,    57,     0,    56,    25,    57,     0,    21,     5,    30,
     5,    58,     0,     5,     5,    58,     0,     5,    28,    53,
    29,     5,     0,     5,    28,    53,    29,    30,     5,     0,
    21,     5,    28,    53,    29,    30,     5,     0,    55,     5,
     0,     0,    33,     7,     0,    33,     3,     0,    33,     4,
     0,    33,     6,     0,    33,    35,     0,    33,     5,    34,
     5,     0,    20,    54,     5,    12,    56,    14,    52,    33,
    35,    16,     0,     5,    12,    56,    14,    16,     0,    27,
     5,    12,    14,    16,     0,    54,     5,    12,    56,    14,
    52,    33,    35,    16,     0,    20,    54,     5,    12,    56,
    14,    52,    16,     0,    54,     5,    12,    56,    14,    52,
    16,     0,    54,    16,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
    93,   100,   104,   110,   114,   117,   122,   122,   122,   124,
   124,   124,   127,   133,   138,   147,   151,   155,   162,   166,
   174,   178,   185,   189,   193,   197,   201,   205,   211,   220,
   226,   235,   239,   246,   250,   257,   263,   269,   275,   281,
   289,   290,   291,   294,   298,   302,   309,   316,   322,   328,
   334,   340,   348,   351,   354,   357,   360,   363,   366,   372,
   384,   390,   396,   405,   414,   429
};
#endif


#if YYDEBUG != 0 || defined (YYERROR_VERBOSE)

static const char * const yytname[] = {   "$","error","$undefined.","T_CHARACTER_LITERAL",
"T_DOUBLE_LITERAL","T_IDENTIFIER","T_INTEGER_LITERAL","T_STRING_LITERAL","T_INCLUDE",
"T_CLASS","T_STRUCT","T_LEFT_CURLY_BRACKET","T_LEFT_PARANTHESIS","T_RIGHT_CURLY_BRACKET",
"T_RIGHT_PARANTHESIS","T_COLON","T_SEMICOLON","T_PUBLIC","T_PROTECTED","T_PRIVATE",
"T_VIRTUAL","T_CONST","T_SIGNAL","T_SLOT","T_TYPEDEF","T_COMMA","T_ASTERISK",
"T_TILDE","T_LESS","T_GREATER","T_AMPERSAND","T_ENUM","T_UNKNOWN","T_EQUAL",
"T_SCOPE","T_NULL","T_DCOP","T_DCOP_AREA","main","includes","rest","nodcop_area",
"sigslot","nodcop_area_begin","dcop_area_begin","super_class_name","super_class",
"super_classes","class_header","body","typedef","typedef_params","qualifier",
"return_params","return","pointer_type","params","param","default","function",
"member", NULL
};
#endif

static const short yyr1[] = {     0,
    38,    39,    39,    40,    40,    40,    41,    41,    41,    42,
    42,    42,    43,    44,    45,    46,    46,    46,    47,    47,
    48,    48,    49,    49,    49,    49,    49,    49,    50,    51,
    51,    52,    52,    53,    53,    54,    54,    54,    54,    54,
    55,    55,    55,    56,    56,    56,    57,    57,    57,    57,
    57,    57,    58,    58,    58,    58,    58,    58,    58,    59,
    59,    59,    59,    59,    59,    60
};

static const short yyr2[] = {     0,
     2,     2,     0,     6,     4,     4,     1,     1,     1,     1,
     1,     0,     3,     2,     1,     3,     2,     1,     2,     3,
     2,     1,     1,     2,     2,     2,     2,     2,     7,     1,
     3,     0,     1,     1,     3,     1,     3,     4,     6,     1,
     6,     5,     2,     0,     1,     3,     5,     3,     5,     6,
     7,     2,     0,     2,     2,     2,     2,     2,     4,    10,
     5,     5,     9,     8,     7,     2
};

static const short yydefact[] = {     3,
     3,     0,     2,     0,     0,     1,     0,     0,    22,     0,
     3,     0,     3,    15,     0,     0,    18,     0,    21,     5,
     0,     6,    17,     0,    19,     0,    36,    23,     9,     8,
     7,     0,     0,     0,     0,     0,    12,     0,     0,     0,
     0,     0,    40,     0,     0,    16,    20,    44,    43,     0,
    36,     0,     0,     0,     0,    14,    10,    11,     0,    27,
    26,     4,    24,     0,    66,    25,    28,     0,     0,     0,
     0,    45,    34,     0,     0,     0,    37,     0,     0,    13,
    44,    53,     0,     0,    52,     0,     0,     0,    38,    44,
     0,    30,     0,     0,     0,     0,    48,     0,     0,     0,
    61,    46,    35,    42,     0,     0,     0,     0,    62,    32,
    55,    56,     0,    57,    54,    58,     0,     0,    53,    32,
    41,    39,    31,     0,    33,     0,     0,    49,     0,     0,
    47,     0,    29,    65,     0,    59,    50,     0,    64,     0,
     0,    51,     0,    63,    60,     0,     0,     0
};

static const short yydefgoto[] = {    20,
     2,     6,    37,    59,    38,    39,    17,    18,    19,    12,
    40,    41,    93,   126,    74,    42,    43,    71,    72,    97,
    44,    45
};

static const short yypact[] = {     1,
     1,    72,-32768,    14,    42,-32768,    62,    55,-32768,    32,
     1,   -24,     1,-32768,    52,    69,-32768,    33,-32768,-32768,
    -3,-32768,-32768,    52,-32768,    32,    15,-32768,-32768,-32768,
-32768,    25,    85,    86,    87,    59,    66,    -3,    -3,    77,
    -3,    40,-32768,    -3,    -3,-32768,-32768,    27,-32768,    89,
    34,    90,    37,    68,    88,-32768,-32768,-32768,    82,-32768,
-32768,-32768,-32768,    91,-32768,-32768,-32768,     3,    93,    94,
    36,-32768,    76,    73,    92,    89,-32768,   100,    95,-32768,
    27,    74,    89,    57,-32768,    96,    27,    89,    80,    27,
    79,    97,    81,    98,    39,     0,-32768,    84,    89,   106,
-32768,-32768,-32768,-32768,    41,    53,   100,   110,-32768,    99,
-32768,-32768,    83,-32768,-32768,-32768,    -4,   101,    74,    99,
-32768,-32768,-32768,   102,-32768,    -5,   111,-32768,   114,    54,
-32768,     7,-32768,-32768,   103,-32768,-32768,   116,-32768,   104,
   107,-32768,   108,-32768,-32768,   125,   126,-32768
};

static const short yypgoto[] = {    38,
   127,-32768,-32768,-32768,-32768,-32768,    44,-32768,   105,-32768,
    31,-32768,    20,     9,   -63,   109,   -48,   -27,    45,    16,
-32768,-32768
};


#define	YYLAST		141


static const short yytable[] = {    70,
   128,    27,   111,   112,   113,   114,   115,    82,     1,    28,
   134,    21,    91,    29,    30,    31,    32,    33,     7,    98,
    34,   104,   139,    35,   103,   129,    48,   135,    49,    51,
    83,    68,    70,    36,   116,   118,    14,   146,    70,   140,
    49,    70,    50,    25,    64,    33,     8,    69,    15,    86,
    22,    16,   110,    95,   120,    65,    14,    26,    23,    49,
    87,    50,   105,    87,    76,    87,    77,    46,    60,    61,
    13,    63,     9,    56,    66,    67,    10,    11,   121,   121,
     4,     5,   122,   138,    99,    24,   100,    57,    58,    53,
    54,    55,    62,    73,    75,    78,    80,    84,    85,    79,
    88,    89,    81,    90,    92,   104,    96,   106,    94,   108,
   119,   101,   117,   109,   124,   136,   127,   133,   137,   125,
   142,   107,   144,   145,   147,   148,   123,     3,   132,   130,
    47,   102,     0,     0,   131,     0,     0,   141,   143,     0,
    52
};

static const short yycheck[] = {    48,
     5,     5,     3,     4,     5,     6,     7,     5,     8,    13,
    16,    36,    76,    17,    18,    19,    20,    21,     5,    83,
    24,    26,    16,    27,    88,    30,    12,    33,    26,     5,
    28,     5,    81,    37,    35,    99,     5,     0,    87,    33,
    26,    90,    28,    11,     5,    21,     5,    21,    17,    14,
    13,    20,    14,    81,    14,    16,     5,    25,    15,    26,
    25,    28,    90,    25,    28,    25,    30,    24,    38,    39,
    16,    41,    11,    15,    44,    45,    15,    16,    26,    26,
     9,    10,    30,    30,    28,    17,    30,    22,    23,     5,
     5,     5,    16,     5,     5,    28,    15,     5,     5,    12,
    25,    29,    12,    12,     5,    26,    33,    29,    14,    29,
     5,    16,    29,    16,     5,     5,    34,    16,     5,    21,
     5,    25,    16,    16,     0,     0,   107,     1,   120,    29,
    26,    87,    -1,    -1,   119,    -1,    -1,    35,    35,    -1,
    32
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
#line 94 "yacc.yy"
{
	     dcop_area = 0; // reset
	  ;
    break;}
case 2:
#line 101 "yacc.yy"
{
		printf("<INCLUDE file=\"%s\"/>\n", yyvsp[-1]._str->latin1() );
	  ;
    break;}
case 3:
#line 105 "yacc.yy"
{
          ;
    break;}
case 4:
#line 111 "yacc.yy"
{
		printf("<CLASS name=\"%s\">\n%s\n%s</CLASS>\n", yyvsp[-4]._str->latin1(), yyvsp[-3]._str->latin1(), yyvsp[-1]._str->latin1() );
	  ;
    break;}
case 5:
#line 115 "yacc.yy"
{
	  ;
    break;}
case 6:
#line 118 "yacc.yy"
{
	  ;
    break;}
case 13:
#line 128 "yacc.yy"
{
	  dcop_area = 0;
	;
    break;}
case 14:
#line 134 "yacc.yy"
{
	  dcop_area = 1;
	;
    break;}
case 15:
#line 139 "yacc.yy"
{
		QString* tmp = new QString( "<SUPER name=\"%1\"/>" );
		*tmp = tmp->arg( *(yyvsp[0]._str) );
		yyval._str = tmp;
	  ;
    break;}
case 16:
#line 148 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 17:
#line 152 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 18:
#line 156 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 19:
#line 163 "yacc.yy"
{
		yyval._str = yyvsp[-1]._str;
	  ;
    break;}
case 20:
#line 167 "yacc.yy"
{
		/* $$ = $1; */
		yyval._str = new QString( *(yyvsp[-2]._str) + *(yyvsp[0]._str) );
	  ;
    break;}
case 21:
#line 175 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 22:
#line 179 "yacc.yy"
{
		yyval._str = new QString( "" );
	  ;
    break;}
case 23:
#line 186 "yacc.yy"
{
		yyval._str = new QString( "" );
	  ;
    break;}
case 24:
#line 190 "yacc.yy"
{
		yyval._str = new QString( *(yyvsp[-1]._str) + *(yyvsp[0]._str) );
	  ;
    break;}
case 25:
#line 194 "yacc.yy"
{
		yyval._str = new QString( *(yyvsp[-1]._str) + *(yyvsp[0]._str) );
	  ;
    break;}
case 26:
#line 198 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 27:
#line 202 "yacc.yy"
{	
	        yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 28:
#line 205 "yacc.yy"
{
 	        yyval._str = yyvsp[0]._str;
	;
    break;}
case 29:
#line 212 "yacc.yy"
{
		QString* tmp = new QString("<TYPEDEF name=\"%1\" template=\"%2\">%3</TYPEDEF>\n");
		*tmp = tmp->arg( *(yyvsp[-1]._str) ).arg( *(yyvsp[-5]._str) ).arg( *(yyvsp[-3]._str) );
		yyval._str = tmp;
	  ;
    break;}
case 30:
#line 221 "yacc.yy"
{
		QString* tmp = new QString("<PARAM type=\"%1\"/>");
		*tmp = tmp->arg( *(yyvsp[0]._str) );
		yyval._str = tmp;
	  ;
    break;}
case 31:
#line 227 "yacc.yy"
{
		QString* tmp = new QString("<PARAM type=\"%1\"/>%2");
		*tmp = tmp->arg( *(yyvsp[-2]._str) ).arg( *(yyvsp[0]._str) );
		yyval._str = tmp;
	  ;
    break;}
case 32:
#line 236 "yacc.yy"
{
		yyval._str = new QString( "" );
	  ;
    break;}
case 33:
#line 240 "yacc.yy"
{
		yyval._str = new QString( "const" );
	  ;
    break;}
case 34:
#line 247 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 35:
#line 251 "yacc.yy"
{
		yyval._str = new QString( *(yyvsp[-2]._str) + "," + *(yyvsp[0]._str) );
	  ;
    break;}
case 36:
#line 258 "yacc.yy"
{
		QString* tmp = new QString("<RET type=\"%1\"/>");
		*tmp = tmp->arg( *(yyvsp[0]._str) );
		yyval._str = tmp;		
	  ;
    break;}
case 37:
#line 264 "yacc.yy"
{
		QString* tmp = new QString("<RET type=\"%1\" qleft=\"const\" qright=\"" AMP_ENTITY "\"/>");
		*tmp = tmp->arg( *(yyvsp[-1]._str) );
		yyval._str = tmp;		
	  ;
    break;}
case 38:
#line 270 "yacc.yy"
{
		QString* tmp = new QString("<RET type=\"%1<%2>\"/>");
		*tmp = tmp->arg( *(yyvsp[-3]._str) ).arg( *(yyvsp[-1]._str) );
		yyval._str = tmp;		
	  ;
    break;}
case 39:
#line 276 "yacc.yy"
{
		QString* tmp = new QString("<RET type=\"%1<%2>\" qleft=\"const\" qright=\"" AMP_ENTITY "\"/>");
		*tmp = tmp->arg( *(yyvsp[-4]._str) ).arg( *(yyvsp[-2]._str) );
		yyval._str = tmp;		
	  ;
    break;}
case 40:
#line 282 "yacc.yy"
{
	 	if (dcop_area)
	           yyerror("pointers are not allowed in kdcop areas!");
	  ;
    break;}
case 41:
#line 289 "yacc.yy"
{;
    break;}
case 42:
#line 290 "yacc.yy"
{;
    break;}
case 43:
#line 291 "yacc.yy"
{;
    break;}
case 44:
#line 295 "yacc.yy"
{
		yyval._str = new QString( "" );
	  ;
    break;}
case 45:
#line 299 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 46:
#line 303 "yacc.yy"
{
		yyval._str = new QString( *(yyvsp[-2]._str) + *(yyvsp[0]._str) );
	  ;
    break;}
case 47:
#line 310 "yacc.yy"
{
		QString* tmp = new QString("<ARG name=\"%1\" type=\"%2\" qleft=\"const\" qright=\"" AMP_ENTITY "\"/>");
		*tmp = tmp->arg( *(yyvsp[-1]._str) );
		*tmp = tmp->arg( *(yyvsp[-3]._str) );
		yyval._str = tmp;		
	  ;
    break;}
case 48:
#line 317 "yacc.yy"
{
		QString* tmp = new QString("<ARG name=\"%1\" type=\"%2\"/>");
		*tmp = tmp->arg( *(yyvsp[-1]._str) ).arg( *(yyvsp[-2]._str) );
		yyval._str = tmp;		
	  ;
    break;}
case 49:
#line 323 "yacc.yy"
{
		QString* tmp = new QString("<ARG name=\"%1\" type=\"%2<%3>\"/>");
		*tmp = tmp->arg( *(yyvsp[0]._str) ).arg( *(yyvsp[-4]._str) ).arg( *(yyvsp[-2]._str) );
		yyval._str = tmp;		
	  ;
    break;}
case 50:
#line 329 "yacc.yy"
{
		QString* tmp = new QString("<ARG name=\"%1\" type=\"%2<%3>\" qright=\"" AMP_ENTITY "\"/>");
		*tmp = tmp->arg( *(yyvsp[0]._str) ).arg( *(yyvsp[-5]._str) ).arg( *(yyvsp[-3]._str) );
		yyval._str = tmp;		
	  ;
    break;}
case 51:
#line 335 "yacc.yy"
{
		QString* tmp = new QString("<ARG name=\"%1\" type=\"%1<%2>\" qleft=\"const\" qright=\"" AMP_ENTITY "\"/>");
		*tmp = tmp->arg( *(yyvsp[0]._str) ).arg( *(yyvsp[-5]._str) ).arg( *(yyvsp[-3]._str) );
		yyval._str = tmp;		
	  ;
    break;}
case 52:
#line 340 "yacc.yy"
{
	       if (dcop_area)
	           yyerror("pointers are not allowed in kdcop areas!");
	       yyval._str = new QString("");
	;
    break;}
case 53:
#line 349 "yacc.yy"
{
	  ;
    break;}
case 54:
#line 352 "yacc.yy"
{
	  ;
    break;}
case 55:
#line 355 "yacc.yy"
{
	  ;
    break;}
case 56:
#line 358 "yacc.yy"
{
	  ;
    break;}
case 57:
#line 361 "yacc.yy"
{
	  ;
    break;}
case 58:
#line 364 "yacc.yy"
{
	  ;
    break;}
case 59:
#line 367 "yacc.yy"
{
	  ;
    break;}
case 60:
#line 373 "yacc.yy"
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
case 61:
#line 385 "yacc.yy"
{
	      /* The constructor */
	      assert(!dcop_area);
              yyval._str = new QString("");
	  ;
    break;}
case 62:
#line 391 "yacc.yy"
{
	    /* The destructor */
	    assert(!dcop_area);
            yyval._str = new QString("");
	  ;
    break;}
case 63:
#line 397 "yacc.yy"
{
	     if (dcop_area) {
		QString* tmp = new QString("<FUNC name=\"%1\" qual=\"%4\">%2%3</FUNC>\n");
		*tmp = tmp->arg( *(yyvsp[-7]._str) ).arg( *(yyvsp[-8]._str) ).arg( *(yyvsp[-5]._str) ).arg( *(yyvsp[-3]._str) );
		yyval._str = tmp;
	     } else
	        yyval._str = new QString("");
	  ;
    break;}
case 64:
#line 406 "yacc.yy"
{
	     if (dcop_area) {
		QString* tmp = new QString("<FUNC name=\"%1\" qual=\"%4\">%2%3</FUNC>\n");
		*tmp = tmp->arg( *(yyvsp[-5]._str) ).arg( *(yyvsp[-6]._str) ).arg( *(yyvsp[-3]._str) ).arg( *(yyvsp[-1]._str) );
		yyval._str = tmp;
	     } else
	        yyval._str = new QString("");
	  ;
    break;}
case 65:
#line 415 "yacc.yy"
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
case 66:
#line 429 "yacc.yy"
{
	   debug("member %s", (yyvsp[-1]._str)->ascii());
	;
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
#line 434 "yacc.yy"


void dcopidlParse( const char *_code )
{
    printf("<!DOCTYPE DCOP-IDL><DCOP-IDL>\n");
    dcopidlInitFlex( _code );
    yyparse();
    printf("</DCOP-IDL>\n");
}
