
/*  A Bison parser, made from yacc.yy
 by  GNU Bison version 1.27
  */

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
#define	T_VIRTUAL	272
#define	T_CONST	273
#define	T_TYPEDEF	274
#define	T_COMMA	275
#define	T_LESS	276
#define	T_GREATER	277
#define	T_AMPERSAND	278
#define	T_ENUM	279
#define	T_UNKNOWN	280
#define	T_EQUAL	281
#define	T_SCOPE	282
#define	T_NULL	283
#define	T_DCOP	284
#define	T_DCOP_AREA	285

#line 1 "yacc.yy"


#include <stdlib.h>
#include <stdio.h>

#include <qstring.h>

#define AMP_ENTITY "&amp;"
#define YYERROR_VERBOSE
 
extern int yylex();

// extern QString idl_lexFile;
extern int idl_line_no;

void dcopidlInitFlex( const char *_code );

void yyerror( const char *s )
{
	qDebug( "In line %i : %s", idl_line_no, s );
        exit(1);
	//   theParser->parse_error( idl_lexFile, s, idl_line_no );
}


#line 28 "yacc.yy"
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



#define	YYFINAL		118
#define	YYFLAG		-32768
#define	YYNTBASE	32

#define YYTRANSLATE(x) ((unsigned)(x) <= 285 ? yytranslate[x] : 49)

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
    27,    28,    29,    30,    31
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     3,     6,     7,    14,    19,    24,    26,    30,    33,
    35,    38,    42,    45,    47,    49,    52,    55,    59,    67,
    69,    73,    74,    76,    78,    82,    84,    88,    93,   100,
   101,   103,   107,   113,   117,   123,   130,   138,   139,   142,
   145,   148,   151,   154,   159,   170,   180,   189
};

static const short yyrhs[] = {    33,
    34,     0,     8,    33,     0,     0,     9,     5,    38,    30,
    39,    16,     0,     9,     5,    16,    32,     0,    10,     5,
    16,    32,     0,     5,     0,    18,    17,    35,     0,    17,
    35,     0,    35,     0,    36,    11,     0,    36,    21,    37,
     0,    15,    37,     0,    11,     0,    13,     0,    40,    39,
     0,    48,    39,     0,    31,    15,    39,     0,    20,     5,
    22,    41,    23,     5,    16,     0,     5,     0,     5,    21,
    41,     0,     0,    19,     0,     5,     0,     5,    21,    43,
     0,     5,     0,    19,     5,    24,     0,     5,    22,    43,
    23,     0,    19,     5,    22,    43,    23,    24,     0,     0,
    46,     0,    45,    21,    46,     0,    19,     5,    24,     5,
    47,     0,     5,     5,    47,     0,     5,    22,    43,    23,
     5,     0,     5,    22,    43,    23,    24,     5,     0,    19,
     5,    22,    43,    23,    24,     5,     0,     0,    27,     7,
     0,    27,     3,     0,    27,     4,     0,    27,     6,     0,
    27,    29,     0,    27,     5,    28,     5,     0,    18,    44,
     5,    12,    45,    14,    42,    27,    29,    16,     0,    44,
     5,    12,    45,    14,    42,    27,    29,    16,     0,    18,
    44,     5,    12,    45,    14,    42,    16,     0,    44,     5,
    12,    45,    14,    42,    16,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
    84,    90,    94,   100,   104,   107,   113,   122,   126,   130,
   137,   141,   149,   153,   160,   164,   168,   172,   179,   188,
   194,   203,   207,   214,   218,   225,   231,   237,   243,   252,
   256,   260,   267,   274,   280,   286,   292,   301,   304,   307,
   310,   313,   316,   319,   325,   334,   340,   346
};
#endif


#if YYDEBUG != 0 || defined (YYERROR_VERBOSE)

static const char * const yytname[] = {   "$","error","$undefined.","T_CHARACTER_LITERAL",
"T_DOUBLE_LITERAL","T_IDENTIFIER","T_INTEGER_LITERAL","T_STRING_LITERAL","T_INCLUDE",
"T_CLASS","T_STRUCT","T_LEFT_CURLY_BRACKET","T_LEFT_PARANTHESIS","T_RIGHT_CURLY_BRACKET",
"T_RIGHT_PARANTHESIS","T_COLON","T_SEMICOLON","T_PUBLIC","T_VIRTUAL","T_CONST",
"T_TYPEDEF","T_COMMA","T_LESS","T_GREATER","T_AMPERSAND","T_ENUM","T_UNKNOWN",
"T_EQUAL","T_SCOPE","T_NULL","T_DCOP","T_DCOP_AREA","main","includes","rest",
"super_class_name","super_class","super_classes","class_header","body","typedef",
"typedef_params","qualifier","return_params","return","params","param","default",
"function", NULL
};
#endif

static const short yyr1[] = {     0,
    32,    33,    33,    34,    34,    34,    35,    36,    36,    36,
    37,    37,    38,    38,    39,    39,    39,    39,    40,    41,
    41,    42,    42,    43,    43,    44,    44,    44,    44,    45,
    45,    45,    46,    46,    46,    46,    46,    47,    47,    47,
    47,    47,    47,    47,    48,    48,    48,    48
};

static const short yyr2[] = {     0,
     2,     2,     0,     6,     4,     4,     1,     3,     2,     1,
     2,     3,     2,     1,     1,     2,     2,     3,     7,     1,
     3,     0,     1,     1,     3,     1,     3,     4,     6,     0,
     1,     3,     5,     3,     5,     6,     7,     0,     2,     2,
     2,     2,     2,     4,    10,     9,     8,     7
};

static const short yydefact[] = {     3,
     3,     0,     2,     0,     0,     1,     0,     0,    14,     0,
     3,     0,     3,     7,     0,     0,    10,     0,    13,     5,
     0,     6,     9,     0,    11,     0,    26,    15,     0,     0,
     0,     0,     0,     0,     0,     0,     8,    12,     0,     0,
     0,     0,     0,     4,    16,     0,    17,    24,     0,     0,
     0,    27,     0,    18,    30,     0,    28,    30,     0,    20,
     0,     0,     0,     0,    31,    25,     0,     0,     0,     0,
    38,     0,     0,    22,     0,    22,    29,    21,     0,     0,
    34,     0,     0,     0,    23,     0,    32,     0,    19,    40,
    41,     0,    42,    39,    43,     0,     0,    38,    48,     0,
    47,     0,     0,    35,     0,     0,    33,     0,     0,    44,
    36,     0,    46,    45,    37,     0,     0,     0
};

static const short yydefgoto[] = {    20,
     2,     6,    17,    18,    19,    12,    33,    34,    61,    86,
    49,    35,    64,    65,    81,    36
};

static const short yypact[] = {    30,
    30,     2,-32768,     3,    25,-32768,    40,    37,-32768,    23,
    30,     9,    30,-32768,    44,    41,-32768,    24,-32768,-32768,
    -4,-32768,-32768,    44,-32768,    23,    38,-32768,    -2,    56,
    57,    48,    49,    -4,    59,    -4,-32768,-32768,    61,    62,
    28,    46,    -4,-32768,-32768,    58,-32768,    50,    51,    60,
    61,-32768,    64,-32768,    -1,    61,-32768,    -1,    52,    55,
    54,    -3,    68,    -8,-32768,-32768,    33,    63,    64,    73,
    53,    61,    35,    65,    -1,    65,-32768,-32768,    66,    19,
-32768,    67,    61,    74,-32768,    15,-32768,    17,-32768,-32768,
-32768,    69,-32768,-32768,-32768,     5,    70,    53,-32768,    71,
-32768,    72,    76,-32768,    78,    75,-32768,    79,    80,-32768,
-32768,    81,-32768,-32768,-32768,    85,    88,-32768
};

static const short yypgoto[] = {     7,
    90,-32768,    22,-32768,    77,-32768,     0,-32768,    20,    16,
   -51,    82,    36,    27,     6,-32768
};


#define	YYLAST		111


static const short yytable[] = {    59,
    27,    71,    27,    62,    66,    74,   116,     7,    28,   104,
     4,     5,    75,    29,    30,    31,    30,    63,    72,    22,
    82,    90,    91,    92,    93,    94,    32,    14,   105,     8,
    99,    97,   101,    45,    25,    47,    23,     1,    21,    15,
    16,   100,    54,   102,    26,    37,    76,    95,    14,    51,
     9,    52,    13,    75,    10,    11,    83,    24,    84,    39,
    41,    42,    43,    46,    44,    48,    50,    53,    60,    55,
    56,    58,    73,    57,    68,    69,    70,    79,    98,    80,
   110,    89,   111,    85,   117,   115,    77,   118,    78,    96,
     3,    88,   106,    67,   113,   114,   103,     0,   112,   108,
   109,    87,    38,   107,     0,     0,     0,     0,     0,     0,
    40
};

static const short yycheck[] = {    51,
     5,     5,     5,     5,    56,    14,     0,     5,    13,     5,
     9,    10,    21,    18,    19,    20,    19,    19,    22,    13,
    72,     3,     4,     5,     6,     7,    31,     5,    24,     5,
    16,    83,    16,    34,    11,    36,    15,     8,    30,    17,
    18,    27,    43,    27,    21,    24,    14,    29,     5,    22,
    11,    24,    16,    21,    15,    16,    22,    17,    24,    22,
     5,     5,    15,     5,    16,     5,     5,    22,     5,    12,
    21,    12,     5,    23,    23,    21,    23,     5,     5,    27,
     5,    16,     5,    19,     0,     5,    24,     0,    69,    23,
     1,    76,    23,    58,    16,    16,    28,    -1,    24,    29,
    29,    75,    26,    98,    -1,    -1,    -1,    -1,    -1,    -1,
    29
};
/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
#line 3 "/opt/local/share/bison.simple"
/* This file comes from bison-1.27.  */

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

#line 216 "/opt/local/share/bison.simple"

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
#line 85 "yacc.yy"
{
	  ;
    break;}
case 2:
#line 91 "yacc.yy"
{
		printf("<INCLUDE file=\"%s\"/>\n", yyvsp[-1]._str->latin1() );
	  ;
    break;}
case 3:
#line 95 "yacc.yy"
{
          ;
    break;}
case 4:
#line 101 "yacc.yy"
{
		printf("<CLASS name=\"%s\">\n%s\n%s</CLASS>\n", yyvsp[-4]._str->latin1(), yyvsp[-3]._str->latin1(), yyvsp[-1]._str->latin1() );
	  ;
    break;}
case 5:
#line 105 "yacc.yy"
{
	  ;
    break;}
case 6:
#line 108 "yacc.yy"
{
	  ;
    break;}
case 7:
#line 114 "yacc.yy"
{
		QString* tmp = new QString( "<SUPER name=\"%1\"/>" );
		*tmp = tmp->arg( *(yyvsp[0]._str) );
		yyval._str = tmp;
	  ;
    break;}
case 8:
#line 123 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 9:
#line 127 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 10:
#line 131 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 11:
#line 138 "yacc.yy"
{
		yyval._str = yyvsp[-1]._str;
	  ;
    break;}
case 12:
#line 142 "yacc.yy"
{
		/* $$ = $1; */
		yyval._str = new QString( *(yyvsp[-2]._str) + *(yyvsp[0]._str) );
	  ;
    break;}
case 13:
#line 150 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 14:
#line 154 "yacc.yy"
{
		yyval._str = new QString( "" );
	  ;
    break;}
case 15:
#line 161 "yacc.yy"
{
		yyval._str = new QString( "" );
	  ;
    break;}
case 16:
#line 165 "yacc.yy"
{
		yyval._str = new QString( *(yyvsp[-1]._str) + *(yyvsp[0]._str) );
	  ;
    break;}
case 17:
#line 169 "yacc.yy"
{
		yyval._str = new QString( *(yyvsp[-1]._str) + *(yyvsp[0]._str) );
	  ;
    break;}
case 18:
#line 173 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 19:
#line 180 "yacc.yy"
{
		QString* tmp = new QString("<TYPEDEF name=\"%1\" template=\"%2\">%3</TYPEDEF>\n");
		*tmp = tmp->arg( *(yyvsp[-1]._str) ).arg( *(yyvsp[-5]._str) ).arg( *(yyvsp[-3]._str) );
		yyval._str = tmp;
	  ;
    break;}
case 20:
#line 189 "yacc.yy"
{
		QString* tmp = new QString("<PARAM type=\"%1\"/>");
		*tmp = tmp->arg( *(yyvsp[0]._str) );
		yyval._str = tmp;
	  ;
    break;}
case 21:
#line 195 "yacc.yy"
{
		QString* tmp = new QString("<PARAM type=\"%1\"/>%2");
		*tmp = tmp->arg( *(yyvsp[-2]._str) ).arg( *(yyvsp[0]._str) );
		yyval._str = tmp;
	  ;
    break;}
case 22:
#line 204 "yacc.yy"
{
		yyval._str = new QString( "" );
	  ;
    break;}
case 23:
#line 208 "yacc.yy"
{
		yyval._str = new QString( "const" );
	  ;
    break;}
case 24:
#line 215 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 25:
#line 219 "yacc.yy"
{
		yyval._str = new QString( *(yyvsp[-2]._str) + "," + *(yyvsp[0]._str) );
	  ;
    break;}
case 26:
#line 226 "yacc.yy"
{
		QString* tmp = new QString("<RET type=\"%1\"/>");
		*tmp = tmp->arg( *(yyvsp[0]._str) );
		yyval._str = tmp;		
	  ;
    break;}
case 27:
#line 232 "yacc.yy"
{
		QString* tmp = new QString("<RET type=\"%1\" qleft=\"const\" qright=\"" AMP_ENTITY "\"/>");
		*tmp = tmp->arg( *(yyvsp[-1]._str) );
		yyval._str = tmp;		
	  ;
    break;}
case 28:
#line 238 "yacc.yy"
{
		QString* tmp = new QString("<RET type=\"%1<%2>\"/>");
		*tmp = tmp->arg( *(yyvsp[-3]._str) ).arg( *(yyvsp[-1]._str) );
		yyval._str = tmp;		
	  ;
    break;}
case 29:
#line 244 "yacc.yy"
{
		QString* tmp = new QString("<RET type=\"%1<%2>\" qleft=\"const\" qright=\"" AMP_ENTITY "\"/>");
		*tmp = tmp->arg( *(yyvsp[-4]._str) ).arg( *(yyvsp[-2]._str) );
		yyval._str = tmp;		
	  ;
    break;}
case 30:
#line 253 "yacc.yy"
{
		yyval._str = new QString( "" );
	  ;
    break;}
case 31:
#line 257 "yacc.yy"
{
		yyval._str = yyvsp[0]._str;
	  ;
    break;}
case 32:
#line 261 "yacc.yy"
{
		yyval._str = new QString( *(yyvsp[-2]._str) + *(yyvsp[0]._str) );
	  ;
    break;}
case 33:
#line 268 "yacc.yy"
{
		QString* tmp = new QString("<ARG name=\"%1\" type=\"%2\" qleft=\"const\" qright=\"" AMP_ENTITY "\"/>");
		*tmp = tmp->arg( *(yyvsp[-1]._str) );
		*tmp = tmp->arg( *(yyvsp[-3]._str) );
		yyval._str = tmp;		
	  ;
    break;}
case 34:
#line 275 "yacc.yy"
{
		QString* tmp = new QString("<ARG name=\"%1\" type=\"%2\"/>");
		*tmp = tmp->arg( *(yyvsp[-1]._str) ).arg( *(yyvsp[-2]._str) );
		yyval._str = tmp;		
	  ;
    break;}
case 35:
#line 281 "yacc.yy"
{
		QString* tmp = new QString("<ARG name=\"%1\" type=\"%2<%3>\"/>");
		*tmp = tmp->arg( *(yyvsp[0]._str) ).arg( *(yyvsp[-4]._str) ).arg( *(yyvsp[-2]._str) );
		yyval._str = tmp;		
	  ;
    break;}
case 36:
#line 287 "yacc.yy"
{
		QString* tmp = new QString("<ARG name=\"%1\" type=\"%2<%3>\" qright=\"" AMP_ENTITY "\"/>");
		*tmp = tmp->arg( *(yyvsp[0]._str) ).arg( *(yyvsp[-5]._str) ).arg( *(yyvsp[-3]._str) );
		yyval._str = tmp;		
	  ;
    break;}
case 37:
#line 293 "yacc.yy"
{
		QString* tmp = new QString("<ARG name=\"%1\" type=\"%1<%2>\" qleft=\"const\" qright=\"" AMP_ENTITY "\"/>");
		*tmp = tmp->arg( *(yyvsp[0]._str) ).arg( *(yyvsp[-5]._str) ).arg( *(yyvsp[-3]._str) );
		yyval._str = tmp;		
	  ;
    break;}
case 38:
#line 302 "yacc.yy"
{
	  ;
    break;}
case 39:
#line 305 "yacc.yy"
{
	  ;
    break;}
case 40:
#line 308 "yacc.yy"
{
	  ;
    break;}
case 41:
#line 311 "yacc.yy"
{
	  ;
    break;}
case 42:
#line 314 "yacc.yy"
{
	  ;
    break;}
case 43:
#line 317 "yacc.yy"
{
	  ;
    break;}
case 44:
#line 320 "yacc.yy"
{
	  ;
    break;}
case 45:
#line 326 "yacc.yy"
{
		QString* tmp = new QString("<FUNC name=\"%1\" qual=\"%4\">%2%3</FUNC>\n");
		*tmp = tmp->arg( *(yyvsp[-7]._str) );
		*tmp = tmp->arg( *(yyvsp[-8]._str) );
		*tmp = tmp->arg( *(yyvsp[-5]._str) );
		*tmp = tmp->arg( *(yyvsp[-3]._str) );
		yyval._str = tmp;
	  ;
    break;}
case 46:
#line 335 "yacc.yy"
{
		QString* tmp = new QString("<FUNC name=\"%1\" qual=\"%4\">%2%3</FUNC>\n");
		*tmp = tmp->arg( *(yyvsp[-7]._str) ).arg( *(yyvsp[-8]._str) ).arg( *(yyvsp[-5]._str) ).arg( *(yyvsp[-3]._str) );
		yyval._str = tmp;
	  ;
    break;}
case 47:
#line 341 "yacc.yy"
{
		QString* tmp = new QString("<FUNC name=\"%1\" qual=\"%4\">%2%3</FUNC>\n");
		*tmp = tmp->arg( *(yyvsp[-5]._str) ).arg( *(yyvsp[-6]._str) ).arg( *(yyvsp[-3]._str) ).arg( *(yyvsp[-1]._str) );
		yyval._str = tmp;
	  ;
    break;}
case 48:
#line 347 "yacc.yy"
{
		QString* tmp = new QString("<FUNC name=\"%1\" qual=\"%4\">%2%3</FUNC>\n");
		*tmp = tmp->arg( *(yyvsp[-5]._str) );
		*tmp = tmp->arg( *(yyvsp[-6]._str) );
		*tmp = tmp->arg( *(yyvsp[-3]._str) );
		*tmp = tmp->arg( *(yyvsp[-1]._str) );
		yyval._str = tmp;
	  ;
    break;}
}
   /* the action file gets copied in in place of this dollarsign */
#line 542 "/opt/local/share/bison.simple"

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
#line 357 "yacc.yy"


void dcopidlParse( const char *_code )
{
    printf("<!DOCTYPE DCOP-IDL><DCOP-IDL>\n");
    dcopidlInitFlex( _code );
    yyparse();
    printf("</DCOP-IDL>\n");
}
