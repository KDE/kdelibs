
/*  A Bison parser, made from yacc.yy
    by GNU Bison version 1.28  */

#define YYBISON 1  /* Identify Bison output.  */

#define	T_STRUCT	257
#define	T_ENUM	258
#define	T_INTERFACE	259
#define	T_MODULE	260
#define	T_VOID	261
#define	T_LEFT_CURLY_BRACKET	262
#define	T_RIGHT_CURLY_BRACKET	263
#define	T_LEFT_PARANTHESIS	264
#define	T_RIGHT_PARANTHESIS	265
#define	T_LESS	266
#define	T_GREATER	267
#define	T_EQUAL	268
#define	T_SEMICOLON	269
#define	T_COLON	270
#define	T_COMMA	271
#define	T_IDENTIFIER	272
#define	T_QUALIFIED_IDENTIFIER	273
#define	T_INTEGER_LITERAL	274
#define	T_UNKNOWN	275
#define	T_BOOLEAN	276
#define	T_STRING	277
#define	T_LONG	278
#define	T_BYTE	279
#define	T_OBJECT	280
#define	T_SEQUENCE	281
#define	T_AUDIO	282
#define	T_FLOAT	283
#define	T_IN	284
#define	T_OUT	285
#define	T_STREAM	286
#define	T_MULTI	287
#define	T_ATTRIBUTE	288
#define	T_READONLY	289
#define	T_ASYNC	290
#define	T_ONEWAY	291
#define	T_DEFAULT	292

#line 22 "yacc.yy"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "core.h"
#include "namespace.h"

using namespace std;
using namespace Arts;

extern int idl_line_no;
extern string idl_filename;

extern int yylex();
extern void mcopidlInitFlex( const char *_code );
extern void addEnumTodo( EnumDef *edef );
extern void addStructTodo( TypeDef *type );
extern void addInterfaceTodo( InterfaceDef *iface );

void yyerror( const char *s )
{
	printf( "%s:%i: %s\n", idl_filename.c_str(), idl_line_no, s );
    exit(1);
	//   theParser->parse_error( idl_lexFile, s, idl_line_no );
}


#line 51 "yacc.yy"
typedef union
{
  // generic data types
  long		_int;
  char*		_str;
  unsigned short	_char;
  double	_float;

  vector<char *> *_strs;

  // types
  vector<TypeComponent *> *_typeComponentSeq;
  TypeComponent* _typeComponent;

  // enums
  vector<EnumComponent *> *_enumComponentSeq;

  // interfaces
  InterfaceDef *_interfaceDef;

  ParamDef* _paramDef;
  vector<ParamDef *> *_paramDefSeq;

  MethodDef* _methodDef;
  vector<MethodDef *> *_methodDefSeq;

  AttributeDef* _attributeDef;
  vector<AttributeDef *> *_attributeDefSeq;
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



#define	YYFINAL		129
#define	YYFLAG		-32768
#define	YYNTBASE	39

#define YYTRANSLATE(x) ((unsigned)(x) <= 292 ? yytranslate[x] : 73)

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
    37,    38
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     2,     4,     7,     9,    11,    13,    15,    16,    24,
    25,    33,    35,    37,    39,    43,    47,    53,    54,    63,
    65,    68,    69,    70,    79,    81,    84,    87,    90,    92,
    98,   100,   102,   104,   106,   108,   110,   117,   121,   123,
   126,   128,   131,   134,   138,   141,   145,   153,   155,   158,
   160,   164,   167,   169,   173,   175,   179,   181,   183,   185,
   190,   192,   197,   199,   201,   203,   205,   207,   209,   211,
   213,   215,   217
};

static const short yyrhs[] = {    40,
     0,    72,     0,    41,    40,     0,    42,     0,    48,     0,
    51,     0,    44,     0,     0,     3,    18,    43,     8,    69,
     9,    15,     0,     0,     4,    46,    45,     8,    47,     9,
    15,     0,    18,     0,    72,     0,    18,     0,    18,    14,
    20,     0,    47,    17,    18,     0,    47,    17,    18,    14,
    20,     0,     0,     5,    18,    49,    50,     8,    54,     9,
    15,     0,    72,     0,    16,    67,     0,     0,     0,     6,
    18,    52,     8,    40,     9,    53,    15,     0,    72,     0,
    62,    54,     0,    55,    54,     0,    60,    54,     0,    59,
     0,    56,    34,    70,    66,    15,     0,    72,     0,    35,
     0,    72,     0,    37,     0,    72,     0,    38,     0,    58,
    61,    70,    32,    66,    15,     0,    38,    66,    15,     0,
    30,     0,    30,    33,     0,    31,     0,    31,    33,     0,
    36,    30,     0,    36,    30,    33,     0,    36,    31,     0,
    36,    31,    33,     0,    57,    70,    18,    10,    63,    11,
    15,     0,    72,     0,    65,    64,     0,    72,     0,    64,
    17,    65,     0,    70,    18,     0,    18,     0,    66,    17,
    18,     0,    68,     0,    67,    17,    68,     0,    18,     0,
    19,     0,    72,     0,    70,    66,    15,    69,     0,    71,
     0,    27,    12,    71,    13,     0,    22,     0,    23,     0,
    24,     0,    25,     0,    26,     0,    28,     0,    29,     0,
     7,     0,    18,     0,    19,     0,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
   120,   122,   122,   124,   124,   124,   124,   126,   129,   140,
   143,   155,   155,   157,   164,   170,   178,   185,   187,   207,
   209,   211,   214,   217,   219,   223,   228,   239,   245,   247,
   261,   263,   266,   268,   271,   273,   276,   289,   294,   296,
   297,   298,   299,   300,   301,   302,   306,   316,   321,   328,
   333,   342,   357,   359,   361,   363,   365,   369,   375,   379,
   398,   399,   412,   414,   415,   416,   417,   418,   419,   420,
   421,   425,   431
};
#endif


#if YYDEBUG != 0 || defined (YYERROR_VERBOSE)

static const char * const yytname[] = {   "$","error","$undefined.","T_STRUCT",
"T_ENUM","T_INTERFACE","T_MODULE","T_VOID","T_LEFT_CURLY_BRACKET","T_RIGHT_CURLY_BRACKET",
"T_LEFT_PARANTHESIS","T_RIGHT_PARANTHESIS","T_LESS","T_GREATER","T_EQUAL","T_SEMICOLON",
"T_COLON","T_COMMA","T_IDENTIFIER","T_QUALIFIED_IDENTIFIER","T_INTEGER_LITERAL",
"T_UNKNOWN","T_BOOLEAN","T_STRING","T_LONG","T_BYTE","T_OBJECT","T_SEQUENCE",
"T_AUDIO","T_FLOAT","T_IN","T_OUT","T_STREAM","T_MULTI","T_ATTRIBUTE","T_READONLY",
"T_ASYNC","T_ONEWAY","T_DEFAULT","aidlfile","definitions","definition","structdef",
"@1","enumdef","@2","enumname","enumbody","interfacedef","@3","inheritedinterfaces",
"moduledef","@4","@5","classbody","attributedef","maybereadonly","maybeoneway",
"maybedefault","streamdef","defaultdef","direction","methoddef","paramdefs",
"paramdefs1","paramdef","identifierlist","interfacelist","interfacelistelem",
"structbody","type","simpletype","epsilon", NULL
};
#endif

static const short yyr1[] = {     0,
    39,    40,    40,    41,    41,    41,    41,    43,    42,    45,
    44,    46,    46,    47,    47,    47,    47,    49,    48,    50,
    50,    52,    53,    51,    54,    54,    54,    54,    55,    55,
    56,    56,    57,    57,    58,    58,    59,    60,    61,    61,
    61,    61,    61,    61,    61,    61,    62,    63,    63,    64,
    64,    65,    66,    66,    67,    67,    68,    68,    69,    69,
    70,    70,    71,    71,    71,    71,    71,    71,    71,    71,
    71,    71,    72
};

static const short yyr2[] = {     0,
     1,     1,     2,     1,     1,     1,     1,     0,     7,     0,
     7,     1,     1,     1,     3,     3,     5,     0,     8,     1,
     2,     0,     0,     8,     1,     2,     2,     2,     1,     5,
     1,     1,     1,     1,     1,     1,     6,     3,     1,     2,
     1,     2,     2,     3,     2,     3,     7,     1,     2,     1,
     3,     2,     1,     3,     1,     3,     1,     1,     1,     4,
     1,     4,     1,     1,     1,     1,     1,     1,     1,     1,
     1,     1,     0
};

static const short yydefact[] = {    73,
     0,    73,     0,     0,     1,    73,     4,     7,     5,     6,
     2,     8,    12,    10,    13,    18,    22,     3,     0,     0,
    73,     0,    73,     0,     0,     0,    20,    73,    70,    71,
    72,    63,    64,    65,    66,    67,     0,    68,    69,     0,
     0,    61,    59,    14,     0,    57,    58,    21,    55,    73,
     0,     0,     0,    53,     0,     0,     0,     0,     0,    32,
    34,    36,     0,    73,     0,     0,     0,    29,    73,    73,
    33,    23,     0,     9,    73,     0,    15,    11,    16,    56,
     0,     0,    27,     0,     0,    39,    41,     0,     0,    28,
    26,     0,    62,    60,    54,     0,    38,    19,     0,     0,
    40,    42,    43,    45,     0,    24,    17,     0,    73,    44,
    46,     0,    30,     0,    73,     0,    48,     0,     0,    49,
    50,    52,    37,    47,     0,    51,     0,     0,     0
};

static const short yydefgoto[] = {   127,
     5,     6,     7,    19,     8,    20,    14,    45,     9,    21,
    26,    10,    22,    92,    63,    64,    65,    66,    67,    68,
    69,    89,    70,   114,   120,   115,    55,    48,    49,    40,
    41,    42,    71
};

static const short yypact[] = {    29,
     4,    25,    45,    46,-32768,    29,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,    49,    57,
    52,    61,    23,    60,    40,    71,-32768,    29,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,    68,-32768,-32768,    72,
    64,-32768,-32768,    69,    -2,-32768,-32768,    67,-32768,   -25,
    76,    48,    73,-32768,     3,    66,    74,    75,    40,-32768,
-32768,    64,    78,   -25,    56,    23,   -22,-32768,   -25,   -25,
    -5,-32768,    79,-32768,    23,    77,-32768,-32768,    80,-32768,
    21,    81,-32768,    23,    82,    58,    65,    31,    23,-32768,
-32768,    84,-32768,-32768,-32768,    83,-32768,-32768,    64,    87,
-32768,-32768,    85,    86,    70,-32768,-32768,    22,    23,-32768,
-32768,    64,-32768,    90,-32768,    88,-32768,    39,    89,    91,
-32768,-32768,-32768,-32768,    23,-32768,   105,   107,-32768
};

static const short yypgoto[] = {-32768,
    -1,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,   -53,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,   -15,   -59,-32768,    53,    36,
   -65,    62,     0
};


#define	YYLAST		119


static const short yytable[] = {    11,
    85,    15,    81,   -25,    18,    11,    57,    86,    87,    60,
    83,    61,    62,    88,    58,    90,    91,    75,    99,    76,
    27,    12,    43,   105,   -35,   -35,    51,    11,   -31,    29,
   -35,     1,     2,     3,     4,    97,   113,    76,    76,   108,
    30,    31,    13,   116,    32,    33,    34,    35,    36,    37,
    38,    39,   118,   123,    29,    76,    23,    46,    47,   116,
   103,   104,    16,    17,    24,    30,    31,    25,    28,    32,
    33,    34,    35,    36,    43,    38,    39,    44,    50,    52,
    53,    54,    56,    59,    72,    77,    82,    74,    78,    84,
   101,    93,    79,    96,    95,    98,   109,   102,   106,   100,
   119,   112,   107,   124,   128,   122,   129,   125,   117,   126,
    94,    80,     0,    73,   121,     0,     0,   110,   111
};

static const short yycheck[] = {     0,
    66,     2,    62,     9,     6,     6,     9,    30,    31,    35,
    64,    37,    38,    36,    17,    69,    70,    15,    84,    17,
    21,    18,    23,    89,    30,    31,    28,    28,    34,     7,
    36,     3,     4,     5,     6,    15,    15,    17,    17,    99,
    18,    19,    18,   109,    22,    23,    24,    25,    26,    27,
    28,    29,   112,    15,     7,    17,     8,    18,    19,   125,
    30,    31,    18,    18,     8,    18,    19,    16,     8,    22,
    23,    24,    25,    26,    75,    28,    29,    18,     8,    12,
     9,    18,    14,    17,     9,    20,     9,    15,    15,    34,
    33,    13,    18,    14,    18,    15,    10,    33,    15,    18,
    11,    32,    20,    15,     0,    18,     0,    17,   109,   125,
    75,    59,    -1,    52,   115,    -1,    -1,    33,    33
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

case 8:
#line 127 "yacc.yy"
{ ModuleHelper::define(yyvsp[0]._str); ;
    break;}
case 9:
#line 132 "yacc.yy"
{
        char *qualified = ModuleHelper::qualify(yyvsp[-5]._str);
		addStructTodo(new TypeDef(qualified,*yyvsp[-2]._typeComponentSeq));
		free(qualified);
	    free(yyvsp[-5]._str);
	  ;
    break;}
case 10:
#line 141 "yacc.yy"
{ ModuleHelper::define(yyvsp[0]._str); ;
    break;}
case 11:
#line 146 "yacc.yy"
{
	    char *qualified = ModuleHelper::qualify(yyvsp[-5]._str);
	  	addEnumTodo(new EnumDef(qualified,*yyvsp[-2]._enumComponentSeq));
		free(qualified);
		free(yyvsp[-5]._str);
		delete yyvsp[-2]._enumComponentSeq;
	  ;
    break;}
case 12:
#line 155 "yacc.yy"
{ yyval._str = yyvsp[0]._str; ;
    break;}
case 13:
#line 155 "yacc.yy"
{ yyval._str = strdup("_anonymous_"); ;
    break;}
case 14:
#line 159 "yacc.yy"
{
	  	yyval._enumComponentSeq = new vector<EnumComponent *>;
		yyval._enumComponentSeq->push_back(new EnumComponent(yyvsp[0]._str,0));
		free(yyvsp[0]._str);
	  ;
    break;}
case 15:
#line 165 "yacc.yy"
{
	  	yyval._enumComponentSeq = new vector<EnumComponent *>;
		yyval._enumComponentSeq->push_back(new EnumComponent(yyvsp[-2]._str,yyvsp[0]._int));
		free(yyvsp[-2]._str);
	  ;
    break;}
case 16:
#line 171 "yacc.yy"
{
	  	EnumComponent *last = (*yyvsp[-2]._enumComponentSeq)[yyvsp[-2]._enumComponentSeq->size()-1];

		yyval._enumComponentSeq = yyvsp[-2]._enumComponentSeq;
		yyval._enumComponentSeq->push_back(new EnumComponent(yyvsp[0]._str,last->value+1));
		free(yyvsp[0]._str);
	  ;
    break;}
case 17:
#line 179 "yacc.yy"
{
		yyval._enumComponentSeq = yyvsp[-4]._enumComponentSeq;
		yyval._enumComponentSeq->push_back(new EnumComponent(yyvsp[-2]._str,yyvsp[0]._int));
		free(yyvsp[-2]._str);
	  ;
    break;}
case 18:
#line 186 "yacc.yy"
{ ModuleHelper::define(yyvsp[0]._str); ;
    break;}
case 19:
#line 191 "yacc.yy"
{
	    vector<char *>::iterator ii;
		for(ii=yyvsp[-4]._strs->begin(); ii != yyvsp[-4]._strs->end(); ii++)
		{
			yyvsp[-2]._interfaceDef->inheritedInterfaces.push_back(*ii);
			free(*ii);
		}
		delete yyvsp[-4]._strs;
        char *qualified = ModuleHelper::qualify(yyvsp[-6]._str);
		yyvsp[-2]._interfaceDef->name = qualified;
		free(qualified);
		free(yyvsp[-6]._str);
	  	addInterfaceTodo(yyvsp[-2]._interfaceDef);
	  ;
    break;}
case 20:
#line 208 "yacc.yy"
{ yyval._strs = new vector<char *>; ;
    break;}
case 21:
#line 209 "yacc.yy"
{ yyval._strs = yyvsp[0]._strs; ;
    break;}
case 22:
#line 212 "yacc.yy"
{ ModuleHelper::enter(yyvsp[0]._str); free(yyvsp[0]._str); ;
    break;}
case 23:
#line 215 "yacc.yy"
{ ModuleHelper::leave(); ;
    break;}
case 25:
#line 220 "yacc.yy"
{
	  	yyval._interfaceDef = new InterfaceDef();
	  ;
    break;}
case 26:
#line 224 "yacc.yy"
{
		yyval._interfaceDef = yyvsp[0]._interfaceDef;
		yyval._interfaceDef->methods.insert(yyval._interfaceDef->methods.begin(),yyvsp[-1]._methodDef);
	  ;
    break;}
case 27:
#line 229 "yacc.yy"
{
	    yyval._interfaceDef = yyvsp[0]._interfaceDef;
	    yyval._interfaceDef->attributes.insert(yyval._interfaceDef->attributes.begin(),yyvsp[-1]._attributeDefSeq->begin(),yyvsp[-1]._attributeDefSeq->end());
	    if (((*yyvsp[-1]._attributeDefSeq)[0])->flags & streamDefault) {
	      vector<std::string> sv;
	      for (vector<AttributeDef *>::iterator i=yyvsp[-1]._attributeDefSeq->begin(); i!=yyvsp[-1]._attributeDefSeq->end(); i++)
	        sv.push_back((*i)->name);
	      yyval._interfaceDef->defaultPorts.insert(yyval._interfaceDef->defaultPorts.begin(),sv.begin(),sv.end());
	    }
	  ;
    break;}
case 28:
#line 240 "yacc.yy"
{
	    yyval._interfaceDef = yyvsp[0]._interfaceDef;
	    yyval._interfaceDef->defaultPorts.insert(yyval._interfaceDef->defaultPorts.begin(),yyvsp[-1]._strs->begin(),yyvsp[-1]._strs->end());
	  ;
    break;}
case 30:
#line 248 "yacc.yy"
{
	    // 16 == attribute
		vector<char *>::iterator i;
		yyval._attributeDefSeq = new vector<AttributeDef *>;
		for(i=yyvsp[-1]._strs->begin();i != yyvsp[-1]._strs->end();i++)
		{
	  	  yyval._attributeDefSeq->push_back(new AttributeDef((*i),yyvsp[-2]._str,(AttributeType)(yyvsp[-4]._int + 16)));
		  free(*i);
		}
		delete yyvsp[-1]._strs;
	  ;
    break;}
case 31:
#line 262 "yacc.yy"
{ yyval._int = 1+2; /* in&out (read & write) */ ;
    break;}
case 32:
#line 263 "yacc.yy"
{ yyval._int = 2; /* out (readonly) */ ;
    break;}
case 33:
#line 267 "yacc.yy"
{ yyval._int = methodTwoway; ;
    break;}
case 34:
#line 268 "yacc.yy"
{ yyval._int = methodOneway; ;
    break;}
case 35:
#line 272 "yacc.yy"
{ yyval._int = 0; ;
    break;}
case 36:
#line 273 "yacc.yy"
{ yyval._int = streamDefault; ;
    break;}
case 37:
#line 277 "yacc.yy"
{
	    // 8 == stream
		vector<char *>::iterator i;
		yyval._attributeDefSeq = new vector<AttributeDef *>;
		for(i=yyvsp[-1]._strs->begin();i != yyvsp[-1]._strs->end();i++)
		{
	  	  yyval._attributeDefSeq->push_back(new AttributeDef((*i),yyvsp[-3]._str,(AttributeType)((yyvsp[-4]._int|yyvsp[-5]._int) + 8)));
		  free(*i);
		}
		delete yyvsp[-1]._strs;
	  ;
    break;}
case 38:
#line 290 "yacc.yy"
{
	  	yyval._strs = yyvsp[-1]._strs;
	  ;
    break;}
case 39:
#line 295 "yacc.yy"
{ yyval._int = streamIn; ;
    break;}
case 40:
#line 296 "yacc.yy"
{ yyval._int = streamIn|streamMulti; ;
    break;}
case 41:
#line 297 "yacc.yy"
{ yyval._int = streamOut; ;
    break;}
case 42:
#line 298 "yacc.yy"
{ yyval._int = streamOut|streamMulti; ;
    break;}
case 43:
#line 299 "yacc.yy"
{ yyval._int = streamAsync|streamIn; ;
    break;}
case 44:
#line 300 "yacc.yy"
{ yyval._int =streamAsync|streamIn|streamMulti  ;
    break;}
case 45:
#line 301 "yacc.yy"
{ yyval._int = streamAsync|streamOut; ;
    break;}
case 46:
#line 302 "yacc.yy"
{ yyval._int = streamAsync|streamOut|streamMulti; ;
    break;}
case 47:
#line 309 "yacc.yy"
{
	  	yyval._methodDef = new MethodDef(yyvsp[-4]._str,yyvsp[-5]._str,(MethodType)yyvsp[-6]._int,*yyvsp[-2]._paramDefSeq);
		free(yyvsp[-4]._str);
		free(yyvsp[-5]._str);
	  ;
    break;}
case 48:
#line 318 "yacc.yy"
{
	  	yyval._paramDefSeq = new vector<ParamDef *>;
	  ;
    break;}
case 49:
#line 322 "yacc.yy"
{
	  	yyval._paramDefSeq = yyvsp[0]._paramDefSeq;
		yyval._paramDefSeq->insert(yyval._paramDefSeq->begin(),yyvsp[-1]._paramDef);
	  ;
    break;}
case 50:
#line 330 "yacc.yy"
{
	  	yyval._paramDefSeq = new vector<ParamDef *>;
	  ;
    break;}
case 51:
#line 334 "yacc.yy"
{
	  	yyval._paramDefSeq = yyvsp[-2]._paramDefSeq;
		yyval._paramDefSeq->push_back(yyvsp[0]._paramDef);
		//$$->insert($$->begin(),$3);
	  ;
    break;}
case 52:
#line 343 "yacc.yy"
{
	  	yyval._paramDef = new ParamDef(string(yyvsp[-1]._str),string(yyvsp[0]._str));
		free(yyvsp[-1]._str);
		free(yyvsp[0]._str);
	  ;
    break;}
case 53:
#line 358 "yacc.yy"
{ yyval._strs = new vector<char *>; yyval._strs->push_back(yyvsp[0]._str); ;
    break;}
case 54:
#line 359 "yacc.yy"
{ yyval._strs = yyvsp[-2]._strs; yyval._strs->push_back(yyvsp[0]._str); ;
    break;}
case 55:
#line 362 "yacc.yy"
{ yyval._strs = new vector<char *>; yyval._strs->push_back(yyvsp[0]._str); ;
    break;}
case 56:
#line 363 "yacc.yy"
{ yyval._strs = yyvsp[-2]._strs; yyval._strs->push_back(yyvsp[0]._str); ;
    break;}
case 57:
#line 365 "yacc.yy"
{
		yyval._str = ModuleHelper::qualify(yyvsp[0]._str);
		free(yyvsp[0]._str);
	  ;
    break;}
case 58:
#line 369 "yacc.yy"
{
		yyval._str = ModuleHelper::qualify(yyvsp[0]._str);
		free(yyvsp[0]._str);
	  ;
    break;}
case 59:
#line 375 "yacc.yy"
{
		// is empty by default
		yyval._typeComponentSeq = new vector<TypeComponent *>;
	  ;
    break;}
case 60:
#line 379 "yacc.yy"
{
	    yyval._typeComponentSeq = yyvsp[0]._typeComponentSeq;
		vector<char *>::reverse_iterator i;
		for(i = yyvsp[-2]._strs->rbegin();i != yyvsp[-2]._strs->rend();i++)
		{
		  char *identifier = *i;

		  yyval._typeComponentSeq->insert(yyval._typeComponentSeq->begin(),new TypeComponent(yyvsp[-3]._str,identifier));
		  free(identifier);
		}
		delete yyvsp[-2]._strs;
	  ;
    break;}
case 62:
#line 400 "yacc.yy"
{
	  	// a sequence<long> is for instance coded as *long

		// alloc new size: add one for the null byte and one for the '*' char
	    char *result = (char *)malloc(strlen(yyvsp[-1]._str)+2);
		result[0] = '*';
		strcpy(&result[1],yyvsp[-1]._str);
		free(yyvsp[-1]._str);  /* fails */

	  	yyval._str = result;
	  ;
    break;}
case 63:
#line 413 "yacc.yy"
{ yyval._str = strdup("boolean"); ;
    break;}
case 64:
#line 414 "yacc.yy"
{ yyval._str = strdup("string"); ;
    break;}
case 65:
#line 415 "yacc.yy"
{ yyval._str = strdup("long"); ;
    break;}
case 66:
#line 416 "yacc.yy"
{ yyval._str = strdup("byte"); ;
    break;}
case 67:
#line 417 "yacc.yy"
{ yyval._str = strdup("object"); ;
    break;}
case 68:
#line 418 "yacc.yy"
{ yyval._str = strdup("float"); ;
    break;}
case 69:
#line 419 "yacc.yy"
{ yyval._str = strdup("float"); ;
    break;}
case 70:
#line 420 "yacc.yy"
{ yyval._str = strdup("void"); ;
    break;}
case 71:
#line 421 "yacc.yy"
{
		yyval._str = ModuleHelper::qualify(yyvsp[0]._str);
		free(yyvsp[0]._str);
	  ;
    break;}
case 72:
#line 425 "yacc.yy"
{
		yyval._str = ModuleHelper::qualify(yyvsp[0]._str);
		free(yyvsp[0]._str);
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
#line 432 "yacc.yy"


void mcopidlParse( const char *_code )
{
    mcopidlInitFlex( _code );
    yyparse();
}
