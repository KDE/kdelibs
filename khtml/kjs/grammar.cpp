
/*  A Bison parser, made from grammar.y
 by  GNU Bison version 1.25
  */

#define YYBISON 1  /* Identify Bison output.  */

#define yyparse kjsyyparse
#define yylex kjsyylex
#define yyerror kjsyyerror
#define yylval kjsyylval
#define yychar kjsyychar
#define yydebug kjsyydebug
#define yynerrs kjsyynerrs
#define	LF	258
#define	NULLTOKEN	259
#define	TRUETOKEN	260
#define	FALSETOKEN	261
#define	STRING	262
#define	DECIMAL	263
#define	INTEGER	264
#define	REGEXP	265
#define	BREAK	266
#define	CASE	267
#define	DEFAULT	268
#define	FOR	269
#define	NEW	270
#define	VAR	271
#define	CONTINUE	272
#define	FUNCTION	273
#define	RETURN	274
#define	VOID	275
#define	DELETE	276
#define	IF	277
#define	THIS	278
#define	DO	279
#define	WHILE	280
#define	ELSE	281
#define	IN	282
#define	INSTANCEOF	283
#define	TYPEOF	284
#define	SWITCH	285
#define	WITH	286
#define	RESERVED	287
#define	THROW	288
#define	TRY	289
#define	CATCH	290
#define	FINALLY	291
#define	DEBUG	292
#define	EQEQ	293
#define	NE	294
#define	STREQ	295
#define	STRNEQ	296
#define	LE	297
#define	GE	298
#define	OR	299
#define	AND	300
#define	PLUSPLUS	301
#define	MINUSMINUS	302
#define	LSHIFT	303
#define	RSHIFT	304
#define	URSHIFT	305
#define	PLUSEQUAL	306
#define	MINUSEQUAL	307
#define	MULTEQUAL	308
#define	DIVEQUAL	309
#define	LSHIFTEQUAL	310
#define	RSHIFTEQUAL	311
#define	URSHIFTEQUAL	312
#define	ANDEQUAL	313
#define	MODEQUAL	314
#define	XOREQUAL	315
#define	OREQUAL	316
#define	DOUBLE	317
#define	IDENT	318

#line 1 "grammar.y"


/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999-2000 Harri Porten (porten@kde.org)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "global.h"
#include "kjs.h"
#include "nodes.h"
#include "lexer.h"

extern int yylex();
int yyerror (const char *);

using namespace KJS;


#line 34 "grammar.y"
typedef union {
  int                 ival;
  double              dval;
  CString             *cstr;
  UString             *ustr;
  void                *rxp; /* TODO */
  Node                *node;
  StatementNode       *stat;
  ParameterNode       *param;
  FuncDeclNode        *func;
  ProgramNode         *prog;
  AssignExprNode      *init;
  SourceElementNode   *src;
  SourceElementsNode  *srcs;
  StatListNode        *slist;
  ArgumentsNode       *args;
  ArgumentListNode    *alist;
  VarDeclNode         *decl;
  VarDeclListNode     *vlist;
  CaseBlockNode       *cblk;
  ClauseListNode      *clist;
  CaseClauseNode      *ccl;
  Operator            op;
} YYSTYPE;
#include <stdio.h>

#ifndef __cplusplus
#ifndef __STDC__
#define const
#endif
#endif



#define	YYFINAL		303
#define	YYFLAG		-32768
#define	YYNTBASE	88

#define YYTRANSLATE(x) ((unsigned)(x) <= 318 ? yytranslate[x] : 144)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,    73,     2,     2,     2,    76,    79,     2,    64,
    65,    74,    70,    69,    71,    68,    75,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,    83,    87,    77,
    84,    78,    82,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
    66,     2,    67,    80,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,    85,    81,    86,    72,     2,     2,     2,     2,
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
    56,    57,    58,    59,    60,    61,    62,    63
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     2,     4,     6,     8,    10,    12,    14,    16,    18,
    20,    24,    26,    31,    35,    39,    41,    44,    47,    50,
    55,    59,    62,    66,    68,    72,    74,    76,    78,    81,
    84,    86,    89,    92,    95,    98,   101,   104,   107,   110,
   113,   115,   119,   123,   127,   129,   133,   137,   139,   143,
   147,   151,   153,   157,   161,   165,   169,   173,   175,   179,
   183,   187,   191,   193,   197,   199,   203,   205,   209,   211,
   215,   217,   221,   223,   229,   231,   235,   237,   239,   241,
   243,   245,   247,   249,   251,   253,   255,   257,   259,   261,
   265,   267,   269,   271,   273,   275,   277,   279,   281,   283,
   285,   287,   289,   291,   293,   298,   301,   305,   307,   310,
   314,   316,   320,   322,   325,   328,   330,   333,   339,   347,
   354,   360,   370,   381,   389,   398,   408,   409,   411,   414,
   418,   421,   425,   428,   432,   438,   444,   448,   454,   455,
   457,   459,   462,   466,   471,   474,   478,   482,   486,   490,
   494,   499,   505,   508,   514,   521,   523,   527,   529,   531,
   534,   536
};

static const short yyrhs[] = {     4,
     0,     5,     0,     6,     0,     9,     0,    62,     0,     7,
     0,    10,     0,    23,     0,    63,     0,    88,     0,    64,
   111,    65,     0,    89,     0,    90,    66,   111,    67,     0,
    90,    68,    63,     0,    15,    90,    93,     0,    90,     0,
    15,    91,     0,    90,    93,     0,    92,    93,     0,    92,
    66,   111,    67,     0,    92,    68,    63,     0,    64,    65,
     0,    64,    94,    65,     0,   109,     0,    94,    69,   109,
     0,    91,     0,    92,     0,    95,     0,    95,    46,     0,
    95,    47,     0,    96,     0,    21,    97,     0,    20,    97,
     0,    29,    97,     0,    46,    97,     0,    47,    97,     0,
    70,    97,     0,    71,    97,     0,    72,    97,     0,    73,
    97,     0,    97,     0,    98,    74,    97,     0,    98,    75,
    97,     0,    98,    76,    97,     0,    98,     0,    99,    70,
    98,     0,    99,    71,    98,     0,    99,     0,   100,    48,
    99,     0,   100,    49,    99,     0,   100,    50,    99,     0,
   100,     0,   101,    77,   100,     0,   101,    78,   100,     0,
   101,    42,   100,     0,   101,    43,   100,     0,   101,    28,
   100,     0,   101,     0,   102,    38,   101,     0,   102,    39,
   101,     0,   102,    40,   101,     0,   102,    41,   101,     0,
   102,     0,   103,    79,   102,     0,   103,     0,   104,    80,
   102,     0,   104,     0,   105,    81,   102,     0,   105,     0,
   106,    45,   105,     0,   106,     0,   107,    44,   103,     0,
   107,     0,   107,    82,   109,    83,   109,     0,   108,     0,
    95,   110,   109,     0,    84,     0,    51,     0,    52,     0,
    53,     0,    54,     0,    55,     0,    56,     0,    57,     0,
    58,     0,    60,     0,    61,     0,    59,     0,   109,     0,
   111,    69,   109,     0,   113,     0,   115,     0,   119,     0,
   120,     0,   121,     0,   122,     0,   124,     0,   125,     0,
   126,     0,   127,     0,   128,     0,   134,     0,   135,     0,
   136,     0,    37,    64,   111,    65,     0,    85,    86,     0,
    85,   114,    86,     0,   112,     0,   114,   112,     0,    16,
   116,    87,     0,   117,     0,   116,    69,   117,     0,    63,
     0,    63,   118,     0,    84,   109,     0,    87,     0,   111,
    87,     0,    22,    64,   111,    65,   112,     0,    22,    64,
   111,    65,   112,    26,   112,     0,    24,   112,    25,    64,
   111,    65,     0,    25,    64,   111,    65,   112,     0,    14,
    64,   123,    87,   123,    87,   123,    65,   112,     0,    14,
    64,    16,   116,    87,   123,    87,   123,    65,   112,     0,
    14,    64,    95,    27,   111,    65,   112,     0,    14,    64,
    16,    63,    27,   111,    65,   112,     0,    14,    64,    16,
    63,   118,    27,   111,    65,   112,     0,     0,   111,     0,
    17,    87,     0,    17,    63,    87,     0,    11,    87,     0,
    11,    63,    87,     0,    19,    87,     0,    19,   111,    87,
     0,    31,    64,   111,    65,   112,     0,    30,    64,   111,
    65,   129,     0,    85,   130,    86,     0,    85,   130,   133,
   130,    86,     0,     0,   131,     0,   132,     0,   131,   132,
     0,    12,   111,    83,     0,    12,   111,    83,   114,     0,
    13,    83,     0,    13,    83,   114,     0,    63,    83,   112,
     0,    33,   111,    87,     0,    34,   113,   137,     0,    34,
   113,   138,     0,    34,   113,   137,   138,     0,    35,    64,
    63,    65,   113,     0,    36,   113,     0,    18,    63,    64,
    65,   113,     0,    18,    63,    64,   140,    65,   113,     0,
    63,     0,   140,    69,    63,     0,   142,     0,   143,     0,
   142,   143,     0,   112,     0,   139,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
   141,   143,   144,   145,   146,   147,   148,   151,   153,   155,
   156,   159,   161,   162,   164,   167,   169,   172,   174,   175,
   176,   179,   181,   184,   186,   189,   191,   194,   196,   197,
   200,   202,   203,   204,   205,   206,   207,   208,   209,   210,
   213,   215,   216,   217,   220,   222,   223,   226,   228,   229,
   230,   233,   235,   237,   239,   241,   243,   247,   249,   250,
   251,   252,   255,   257,   260,   262,   265,   267,   270,   272,
   276,   278,   282,   284,   288,   290,   294,   296,   297,   298,
   299,   300,   301,   302,   303,   304,   305,   306,   309,   311,
   314,   316,   317,   318,   319,   320,   321,   322,   323,   324,
   325,   326,   327,   328,   329,   332,   334,   337,   339,   342,
   346,   348,   352,   354,   357,   361,   365,   369,   371,   375,
   377,   378,   380,   382,   384,   387,   392,   394,   397,   399,
   402,   404,   407,   409,   412,   416,   420,   422,   426,   428,
   431,   433,   436,   438,   441,   443,   446,   450,   454,   456,
   457,   460,   464,   468,   471,   476,   478,   482,   486,   488,
   491,   493
};
#endif


#if YYDEBUG != 0 || defined (YYERROR_VERBOSE)

static const char * const yytname[] = {   "$","error","$undefined.","LF","NULLTOKEN",
"TRUETOKEN","FALSETOKEN","STRING","DECIMAL","INTEGER","REGEXP","BREAK","CASE",
"DEFAULT","FOR","NEW","VAR","CONTINUE","FUNCTION","RETURN","VOID","DELETE","IF",
"THIS","DO","WHILE","ELSE","IN","INSTANCEOF","TYPEOF","SWITCH","WITH","RESERVED",
"THROW","TRY","CATCH","FINALLY","DEBUG","EQEQ","NE","STREQ","STRNEQ","LE","GE",
"OR","AND","PLUSPLUS","MINUSMINUS","LSHIFT","RSHIFT","URSHIFT","PLUSEQUAL","MINUSEQUAL",
"MULTEQUAL","DIVEQUAL","LSHIFTEQUAL","RSHIFTEQUAL","URSHIFTEQUAL","ANDEQUAL",
"MODEQUAL","XOREQUAL","OREQUAL","DOUBLE","IDENT","'('","')'","'['","']'","'.'",
"','","'+'","'-'","'~'","'!'","'*'","'/'","'%'","'<'","'>'","'&'","'^'","'|'",
"'?'","':'","'='","'{'","'}'","';'","Literal","PrimaryExpr","MemberExpr","NewExpr",
"CallExpr","Arguments","ArgumentList","LeftHandSideExpr","PostfixExpr","UnaryExpr",
"MultiplicativeExpr","AdditiveExpr","ShiftExpr","RelationalExpr","EqualityExpr",
"BitwiseANDExpr","BitwiseXORExpr","BitwiseORExpr","LogicalANDExpr","LogicalORExpr",
"ConditionalExpr","AssignmentExpr","AssignmentOperator","Expr","Statement","Block",
"StatementList","VariableStatement","VariableDeclarationList","VariableDeclaration",
"Initializer","EmptyStatement","ExprStatement","IfStatement","IterationStatement",
"ExprOpt","ContinueStatement","BreakStatement","ReturnStatement","WithStatement",
"SwitchStatement","CaseBlock","CaseClausesOpt","CaseClauses","CaseClause","DefaultClause",
"LabelledStatement","ThrowStatement","TryStatement","Catch","Finally","FunctionDeclaration",
"FormalParameterList","Program","SourceElements","SourceElement", NULL
};
#endif

static const short yyr1[] = {     0,
    88,    88,    88,    88,    88,    88,    88,    89,    89,    89,
    89,    90,    90,    90,    90,    91,    91,    92,    92,    92,
    92,    93,    93,    94,    94,    95,    95,    96,    96,    96,
    97,    97,    97,    97,    97,    97,    97,    97,    97,    97,
    98,    98,    98,    98,    99,    99,    99,   100,   100,   100,
   100,   101,   101,   101,   101,   101,   101,   102,   102,   102,
   102,   102,   103,   103,   104,   104,   105,   105,   106,   106,
   107,   107,   108,   108,   109,   109,   110,   110,   110,   110,
   110,   110,   110,   110,   110,   110,   110,   110,   111,   111,
   112,   112,   112,   112,   112,   112,   112,   112,   112,   112,
   112,   112,   112,   112,   112,   113,   113,   114,   114,   115,
   116,   116,   117,   117,   118,   119,   120,   121,   121,   122,
   122,   122,   122,   122,   122,   122,   123,   123,   124,   124,
   125,   125,   126,   126,   127,   128,   129,   129,   130,   130,
   131,   131,   132,   132,   133,   133,   134,   135,   136,   136,
   136,   137,   138,   139,   139,   140,   140,   141,   142,   142,
   143,   143
};

static const short yyr2[] = {     0,
     1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
     3,     1,     4,     3,     3,     1,     2,     2,     2,     4,
     3,     2,     3,     1,     3,     1,     1,     1,     2,     2,
     1,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     1,     3,     3,     3,     1,     3,     3,     1,     3,     3,
     3,     1,     3,     3,     3,     3,     3,     1,     3,     3,
     3,     3,     1,     3,     1,     3,     1,     3,     1,     3,
     1,     3,     1,     5,     1,     3,     1,     1,     1,     1,
     1,     1,     1,     1,     1,     1,     1,     1,     1,     3,
     1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
     1,     1,     1,     1,     4,     2,     3,     1,     2,     3,
     1,     3,     1,     2,     2,     1,     2,     5,     7,     6,
     5,     9,    10,     7,     8,     9,     0,     1,     2,     3,
     2,     3,     2,     3,     5,     5,     3,     5,     0,     1,
     1,     2,     3,     4,     2,     3,     3,     3,     3,     3,
     4,     5,     2,     5,     6,     1,     3,     1,     1,     2,
     1,     1
};

static const short yydefact[] = {     0,
     1,     2,     3,     6,     4,     7,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     8,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     5,     9,     0,
     0,     0,     0,     0,     0,   116,    10,    12,    16,    26,
    27,    28,    31,    41,    45,    48,    52,    58,    63,    65,
    67,    69,    71,    73,    75,    89,     0,   161,    91,    92,
    93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
   103,   104,   162,   158,   159,     0,   131,   127,     9,    16,
    17,   113,     0,   111,     0,   129,     0,   133,     0,    28,
    33,    32,     0,     0,     0,    34,     0,     0,     0,     0,
     0,    35,    36,     0,     0,    37,    38,    39,    40,   106,
   108,     0,     0,     0,     0,    18,     0,     0,    19,    29,
    30,    78,    79,    80,    81,    82,    83,    84,    85,    88,
    86,    87,    77,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,   117,   160,
   132,     0,    28,   128,     0,    15,     0,   114,     0,   110,
   130,     0,   134,     0,     0,     0,     0,     0,   148,     0,
     0,   149,   150,     0,   147,    11,   107,   109,    22,     0,
    24,     0,    14,     0,    21,    76,    42,    43,    44,    46,
    47,    49,    50,    51,    57,    55,    56,    53,    54,    59,
    60,    61,    62,    64,    66,    68,    70,    72,     0,    90,
   113,     0,     0,   127,   115,   112,   156,     0,     0,     0,
     0,     0,     0,     0,     0,   153,   151,   105,    23,     0,
    13,    20,     0,     0,   114,   127,     0,     0,   154,     0,
     0,   118,     0,   121,   139,   136,   135,     0,    25,    74,
     0,     0,     0,     0,   127,   155,   157,     0,   120,     0,
     0,   140,   141,     0,     0,     0,   127,   124,     0,   119,
     0,     0,   137,   139,   142,   152,   125,     0,     0,     0,
   143,   145,     0,   126,     0,   122,   144,   146,   138,   123,
     0,     0,     0
};

static const short yydefgoto[] = {    37,
    38,    39,    40,    41,   116,   190,    42,    43,    44,    45,
    46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
    56,   134,    57,   111,    59,   112,    60,    83,    84,   168,
    61,    62,    63,    64,   165,    65,    66,    67,    68,    69,
   256,   271,   272,   273,   284,    70,    71,    72,   182,   183,
    73,   229,   301,    74,    75
};

static const short yypact[] = {   295,
-32768,-32768,-32768,-32768,-32768,-32768,   -54,   -44,    87,   -48,
   -49,   -20,   175,   666,   666,   -33,-32768,   547,    -8,   666,
    -5,     2,   666,   -51,     8,   666,   666,-32768,     5,   666,
   666,   666,   666,   666,   379,-32768,-32768,-32768,     7,-32768,
    41,   115,-32768,-32768,   149,    51,   178,   -15,   170,    22,
    28,   105,   146,   -18,-32768,-32768,   -58,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,   295,-32768,   107,-32768,   592,-32768,     7,
-32768,   116,   -37,-32768,   131,-32768,   139,-32768,   -30,   101,
-32768,-32768,   666,   180,   666,-32768,   666,   666,   -27,   129,
   666,-32768,-32768,   547,   -25,-32768,-32768,-32768,-32768,-32768,
-32768,   463,   620,   666,   156,-32768,   666,   168,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,   666,   666,   666,   666,   666,   666,   666,
   666,   666,   666,   666,   666,   666,   666,   666,   666,   666,
   666,   666,   666,   666,   666,   666,   666,   666,-32768,-32768,
-32768,   173,   650,   174,   157,-32768,   666,-32768,   -48,-32768,
-32768,    13,-32768,    12,   185,    20,    30,    54,-32768,   186,
   -51,   216,-32768,    55,-32768,-32768,-32768,-32768,-32768,    94,
-32768,    31,-32768,    44,-32768,-32768,-32768,-32768,-32768,   149,
   149,    51,    51,    51,   178,   178,   178,   178,   178,   -15,
   -15,   -15,   -15,   170,   170,   170,   105,    22,   177,-32768,
   -26,    -4,   666,   666,-32768,-32768,-32768,   -51,   118,   547,
   666,   547,   176,   547,   195,-32768,-32768,-32768,-32768,   666,
-32768,-32768,   666,   666,   236,   666,   123,   182,-32768,   -51,
   202,   241,   124,-32768,   258,-32768,-32768,   206,-32768,-32768,
   132,   666,   187,   547,   666,-32768,-32768,   547,-32768,   666,
    -6,   258,-32768,   -51,   547,   137,   666,-32768,   207,-32768,
   -32,   190,-32768,   258,-32768,-32768,-32768,   547,   211,   547,
   547,   547,   191,-32768,   547,-32768,   547,   547,-32768,-32768,
   278,   279,-32768
};

static const short yypgoto[] = {-32768,
-32768,   271,   272,-32768,   -35,-32768,   -10,-32768,    21,    15,
   100,   -29,    66,   103,   126,-32768,   128,-32768,-32768,-32768,
   -88,-32768,   -11,     0,   -21,  -114,-32768,   122,   117,    64,
-32768,-32768,-32768,-32768,  -216,-32768,-32768,-32768,-32768,-32768,
-32768,     3,-32768,    17,-32768,-32768,-32768,-32768,-32768,   109,
-32768,-32768,-32768,-32768,   218
};


#define	YYLAST		739


static const short yytable[] = {    58,
   244,    89,   100,    90,    90,   119,   282,   248,    76,    90,
   158,    99,   143,    85,    82,    90,    90,    94,   105,    78,
    90,    90,    90,    90,   191,   156,   144,   145,   159,   263,
    93,   169,    77,    35,    91,    92,   158,    86,   158,   186,
    96,   158,    87,   158,   166,   196,   102,   103,   279,   170,
   291,   106,   107,   108,   109,    95,   173,   167,    97,   179,
   289,   146,   147,   157,   169,    98,   164,   163,   219,   220,
   113,   101,   114,    58,   115,   227,   230,   228,   225,   283,
   158,   174,   246,   176,   232,   177,   178,   104,   158,   184,
     1,     2,     3,     4,   233,     5,     6,   241,   158,   158,
   152,     9,   192,   185,   113,   194,   117,   153,   118,    17,
   242,   188,   158,   205,   206,   207,   208,   209,   234,   238,
   138,   139,   158,   158,    90,    90,    90,    90,    90,    90,
    90,    90,    90,    90,    90,    90,    90,    90,    90,    90,
    90,    90,    90,    90,    90,    90,   120,   121,    28,    79,
    30,   259,   200,   201,   260,   197,   198,   199,   239,   236,
   120,   121,   240,   180,   181,   122,   123,   124,   125,   126,
   127,   128,   129,   130,   131,   132,   297,   298,     1,     2,
     3,     4,   250,     5,     6,   154,   251,   264,   269,     9,
   155,   158,   158,   161,    14,    15,   275,    17,   133,   167,
   158,   288,   172,    20,   175,   158,   249,   148,   149,   150,
   151,   247,   164,   210,   211,   212,   213,   171,   193,   253,
    26,    27,   135,   136,   137,   140,   141,   142,   266,   252,
   195,   254,   261,   257,   164,   221,    28,    79,    30,   202,
   203,   204,   158,   224,    31,    32,    33,    34,   231,   235,
   276,   181,   286,   164,   214,   215,   216,   258,   281,   243,
   255,    88,   262,   278,   267,   164,   268,   280,   265,   270,
   274,   290,   292,   277,   287,   295,   299,   302,   303,    80,
    81,   218,   217,   222,   245,   226,   293,   294,   285,   296,
   237,   160,     0,     0,   300,     0,   188,   188,     1,     2,
     3,     4,     0,     5,     6,     7,     0,     0,     8,     9,
    10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
     0,     0,     0,    20,    21,    22,     0,    23,    24,     0,
     0,    25,     0,     0,     0,     0,     0,     0,     0,     0,
    26,    27,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,    28,    29,    30,     0,
     0,     0,     0,     0,    31,    32,    33,    34,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,    35,
     0,    36,     1,     2,     3,     4,     0,     5,     6,     7,
     0,     0,     8,     9,    10,    11,     0,    13,    14,    15,
    16,    17,    18,    19,     0,     0,     0,    20,    21,    22,
     0,    23,    24,     0,     0,    25,     0,     0,     0,     0,
     0,     0,     0,     0,    26,    27,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    28,    29,    30,     0,     0,     0,     0,     0,    31,    32,
    33,    34,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,    35,   110,    36,     1,     2,     3,     4,
     0,     5,     6,     7,     0,     0,     8,     9,    10,    11,
     0,    13,    14,    15,    16,    17,    18,    19,     0,     0,
     0,    20,    21,    22,     0,    23,    24,     0,     0,    25,
     0,     0,     0,     0,     0,     0,     0,     0,    26,    27,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,    28,    29,    30,     0,     0,     0,
     0,     0,    31,    32,    33,    34,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,    35,   187,    36,
     1,     2,     3,     4,     0,     5,     6,     7,     0,     0,
     8,     9,    10,    11,     0,    13,    14,    15,    16,    17,
    18,    19,     0,     0,     0,    20,    21,    22,     0,    23,
    24,     0,     0,    25,     0,     0,     0,     0,     0,     0,
     0,     0,    26,    27,     0,     1,     2,     3,     4,     0,
     5,     6,     0,     0,     0,     0,     9,   162,    28,    29,
    30,    14,    15,     0,    17,     0,    31,    32,    33,    34,
    20,     0,     0,     1,     2,     3,     4,     0,     5,     6,
     0,    35,     0,    36,     9,     0,     0,    26,    27,    14,
    15,     0,    17,     0,     0,     0,     0,     0,    20,     0,
     0,     0,     0,    28,    79,    30,     0,     0,     0,     0,
     0,    31,    32,    33,    34,    26,    27,     0,     0,     1,
     2,     3,     4,     0,     5,     6,   223,     0,     0,     0,
     9,    28,    79,    30,   189,    14,    15,     0,    17,    31,
    32,    33,    34,     0,    20,   120,   121,     0,     0,     0,
   122,   123,   124,   125,   126,   127,   128,   129,   130,   131,
   132,    26,    27,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,    28,    79,    30,
     0,     0,     0,   133,     0,    31,    32,    33,    34
};

static const short yycheck[] = {     0,
    27,    13,    24,    14,    15,    41,    13,   224,    63,    20,
    69,    23,    28,    63,    63,    26,    27,    18,    30,    64,
    31,    32,    33,    34,   113,    44,    42,    43,    87,   246,
    64,    69,    87,    85,    14,    15,    69,    87,    69,    65,
    20,    69,    63,    69,    80,   134,    26,    27,   265,    87,
    83,    31,    32,    33,    34,    64,    87,    84,    64,    87,
   277,    77,    78,    82,    69,    64,    78,    78,   157,   158,
    64,    64,    66,    74,    68,    63,    65,    65,   167,    86,
    69,    93,    87,    95,    65,    97,    98,    83,    69,   101,
     4,     5,     6,     7,    65,     9,    10,    67,    69,    69,
    79,    15,   114,   104,    64,   117,    66,    80,    68,    23,
    67,   112,    69,   143,   144,   145,   146,   147,    65,    65,
    70,    71,    69,    69,   135,   136,   137,   138,   139,   140,
   141,   142,   143,   144,   145,   146,   147,   148,   149,   150,
   151,   152,   153,   154,   155,   156,    46,    47,    62,    63,
    64,   240,   138,   139,   243,   135,   136,   137,    65,   181,
    46,    47,    69,    35,    36,    51,    52,    53,    54,    55,
    56,    57,    58,    59,    60,    61,   291,   292,     4,     5,
     6,     7,    65,     9,    10,    81,    69,    65,    65,    15,
    45,    69,    69,    87,    20,    21,    65,    23,    84,    84,
    69,    65,    64,    29,    25,    69,   228,    38,    39,    40,
    41,   223,   224,   148,   149,   150,   151,    87,    63,   231,
    46,    47,    74,    75,    76,    48,    49,    50,   250,   230,
    63,   232,   244,   234,   246,    63,    62,    63,    64,   140,
   141,   142,    69,    87,    70,    71,    72,    73,    64,    64,
   262,    36,   274,   265,   152,   153,   154,    63,   270,    83,
    85,    87,    27,   264,    63,   277,    26,   268,    87,    12,
    65,    65,    83,    87,   275,    65,    86,     0,     0,     9,
     9,   156,   155,   162,   221,   169,   284,   288,   272,   290,
   182,    74,    -1,    -1,   295,    -1,   297,   298,     4,     5,
     6,     7,    -1,     9,    10,    11,    -1,    -1,    14,    15,
    16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
    -1,    -1,    -1,    29,    30,    31,    -1,    33,    34,    -1,
    -1,    37,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    46,    47,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    62,    63,    64,    -1,
    -1,    -1,    -1,    -1,    70,    71,    72,    73,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    85,
    -1,    87,     4,     5,     6,     7,    -1,     9,    10,    11,
    -1,    -1,    14,    15,    16,    17,    -1,    19,    20,    21,
    22,    23,    24,    25,    -1,    -1,    -1,    29,    30,    31,
    -1,    33,    34,    -1,    -1,    37,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    46,    47,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    62,    63,    64,    -1,    -1,    -1,    -1,    -1,    70,    71,
    72,    73,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    85,    86,    87,     4,     5,     6,     7,
    -1,     9,    10,    11,    -1,    -1,    14,    15,    16,    17,
    -1,    19,    20,    21,    22,    23,    24,    25,    -1,    -1,
    -1,    29,    30,    31,    -1,    33,    34,    -1,    -1,    37,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,    47,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    62,    63,    64,    -1,    -1,    -1,
    -1,    -1,    70,    71,    72,    73,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    85,    86,    87,
     4,     5,     6,     7,    -1,     9,    10,    11,    -1,    -1,
    14,    15,    16,    17,    -1,    19,    20,    21,    22,    23,
    24,    25,    -1,    -1,    -1,    29,    30,    31,    -1,    33,
    34,    -1,    -1,    37,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    46,    47,    -1,     4,     5,     6,     7,    -1,
     9,    10,    -1,    -1,    -1,    -1,    15,    16,    62,    63,
    64,    20,    21,    -1,    23,    -1,    70,    71,    72,    73,
    29,    -1,    -1,     4,     5,     6,     7,    -1,     9,    10,
    -1,    85,    -1,    87,    15,    -1,    -1,    46,    47,    20,
    21,    -1,    23,    -1,    -1,    -1,    -1,    -1,    29,    -1,
    -1,    -1,    -1,    62,    63,    64,    -1,    -1,    -1,    -1,
    -1,    70,    71,    72,    73,    46,    47,    -1,    -1,     4,
     5,     6,     7,    -1,     9,    10,    27,    -1,    -1,    -1,
    15,    62,    63,    64,    65,    20,    21,    -1,    23,    70,
    71,    72,    73,    -1,    29,    46,    47,    -1,    -1,    -1,
    51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
    61,    46,    47,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    62,    63,    64,
    -1,    -1,    -1,    84,    -1,    70,    71,    72,    73
};
/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
#line 3 "/usr/lib/bison.simple"

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

/* Prevent warning if -Wstrict-prototypes.  */
#ifdef __GNUC__
int yyparse (void);
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

#line 196 "/usr/lib/bison.simple"

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
#line 142 "grammar.y"
{ yyval.node = new NullNode(); ;
    break;}
case 2:
#line 143 "grammar.y"
{ yyval.node = new BooleanNode(true); ;
    break;}
case 3:
#line 144 "grammar.y"
{ yyval.node = new BooleanNode(false); ;
    break;}
case 4:
#line 145 "grammar.y"
{ yyval.node = new NumberNode(yyvsp[0].ival); ;
    break;}
case 5:
#line 146 "grammar.y"
{ yyval.node = new NumberNode(yyvsp[0].dval); ;
    break;}
case 6:
#line 147 "grammar.y"
{ yyval.node = new StringNode(yyvsp[0].ustr); delete yyvsp[0].ustr; ;
    break;}
case 7:
#line 148 "grammar.y"
{ yyval.node = new NullNode(); /* TODO */ ;
    break;}
case 8:
#line 152 "grammar.y"
{ yyval.node = new ThisNode(); ;
    break;}
case 9:
#line 153 "grammar.y"
{ yyval.node = new ResolveNode(yyvsp[0].cstr);
                                     delete yyvsp[0].cstr; ;
    break;}
case 11:
#line 156 "grammar.y"
{ yyval.node = new GroupNode(yyvsp[-1].node); ;
    break;}
case 13:
#line 161 "grammar.y"
{ yyval.node = new AccessorNode1(yyvsp[-3].node, yyvsp[-1].node); ;
    break;}
case 14:
#line 162 "grammar.y"
{ yyval.node = new AccessorNode2(yyvsp[-2].node, yyvsp[0].cstr);
                                     delete yyvsp[0].cstr; ;
    break;}
case 15:
#line 164 "grammar.y"
{ yyval.node = new NewExprNode(yyvsp[-1].node, yyvsp[0].args); ;
    break;}
case 17:
#line 169 "grammar.y"
{ yyval.node = new NewExprNode(yyvsp[0].node); ;
    break;}
case 18:
#line 173 "grammar.y"
{ yyval.node = new FunctionCallNode(yyvsp[-1].node, yyvsp[0].args); ;
    break;}
case 19:
#line 174 "grammar.y"
{ yyval.node = new FunctionCallNode(yyvsp[-1].node, yyvsp[0].args); ;
    break;}
case 20:
#line 175 "grammar.y"
{ yyval.node = new AccessorNode1(yyvsp[-3].node, yyvsp[-1].node); ;
    break;}
case 21:
#line 176 "grammar.y"
{ yyval.node = new AccessorNode2(yyvsp[-2].node, yyvsp[0].cstr); ;
    break;}
case 22:
#line 180 "grammar.y"
{ yyval.args = new ArgumentsNode(0L); ;
    break;}
case 23:
#line 181 "grammar.y"
{ yyval.args = new ArgumentsNode(yyvsp[-1].alist); ;
    break;}
case 24:
#line 185 "grammar.y"
{ yyval.alist = new ArgumentListNode(yyvsp[0].node); ;
    break;}
case 25:
#line 186 "grammar.y"
{ yyval.alist = new ArgumentListNode(yyvsp[-2].alist, yyvsp[0].node); ;
    break;}
case 29:
#line 196 "grammar.y"
{ yyval.node = new PostfixNode(yyvsp[-1].node, OpPlusPlus); ;
    break;}
case 30:
#line 197 "grammar.y"
{ yyval.node = new PostfixNode(yyvsp[-1].node, OpMinusMinus); ;
    break;}
case 32:
#line 202 "grammar.y"
{ yyval.node = new DeleteNode(yyvsp[0].node); ;
    break;}
case 33:
#line 203 "grammar.y"
{ yyval.node = new VoidNode(yyvsp[0].node); ;
    break;}
case 34:
#line 204 "grammar.y"
{ yyval.node = new TypeOfNode(yyvsp[0].node); ;
    break;}
case 35:
#line 205 "grammar.y"
{ yyval.node = new PrefixNode(OpPlusPlus, yyvsp[0].node); ;
    break;}
case 36:
#line 206 "grammar.y"
{ yyval.node = new PrefixNode(OpMinusMinus, yyvsp[0].node); ;
    break;}
case 37:
#line 207 "grammar.y"
{ yyval.node = new UnaryPlusNode(yyvsp[0].node); ;
    break;}
case 38:
#line 208 "grammar.y"
{ yyval.node = new NegateNode(yyvsp[0].node); ;
    break;}
case 39:
#line 209 "grammar.y"
{ yyval.node = new BitwiseNotNode(yyvsp[0].node); ;
    break;}
case 40:
#line 210 "grammar.y"
{ yyval.node = new LogicalNotNode(yyvsp[0].node); ;
    break;}
case 42:
#line 215 "grammar.y"
{ yyval.node = new MultNode(yyvsp[-2].node, yyvsp[0].node, '*'); ;
    break;}
case 43:
#line 216 "grammar.y"
{ yyval.node = new MultNode(yyvsp[-2].node, yyvsp[0].node, '/'); ;
    break;}
case 44:
#line 217 "grammar.y"
{ yyval.node = new MultNode(yyvsp[-2].node,yyvsp[0].node,'%'); ;
    break;}
case 46:
#line 222 "grammar.y"
{ yyval.node = new AddNode(yyvsp[-2].node, yyvsp[0].node, '+'); ;
    break;}
case 47:
#line 223 "grammar.y"
{ yyval.node = new AddNode(yyvsp[-2].node, yyvsp[0].node, '-'); ;
    break;}
case 49:
#line 228 "grammar.y"
{ yyval.node = new ShiftNode(yyvsp[-2].node, OpLShift, yyvsp[0].node); ;
    break;}
case 50:
#line 229 "grammar.y"
{ yyval.node = new ShiftNode(yyvsp[-2].node, OpRShift, yyvsp[0].node); ;
    break;}
case 51:
#line 230 "grammar.y"
{ yyval.node = new ShiftNode(yyvsp[-2].node, OpURShift, yyvsp[0].node); ;
    break;}
case 53:
#line 236 "grammar.y"
{ yyval.node = new RelationalNode(yyvsp[-2].node, OpLess, yyvsp[0].node); ;
    break;}
case 54:
#line 238 "grammar.y"
{ yyval.node = new RelationalNode(yyvsp[-2].node, OpGreater, yyvsp[0].node); ;
    break;}
case 55:
#line 240 "grammar.y"
{ yyval.node = new RelationalNode(yyvsp[-2].node, OpLessEq, yyvsp[0].node); ;
    break;}
case 56:
#line 242 "grammar.y"
{ yyval.node = new RelationalNode(yyvsp[-2].node, OpGreaterEq, yyvsp[0].node); ;
    break;}
case 57:
#line 244 "grammar.y"
{ yyval.node = new RelationalNode(yyvsp[-2].node, OpInstanceOf, yyvsp[0].node); ;
    break;}
case 59:
#line 249 "grammar.y"
{ yyval.node = new EqualNode(yyvsp[-2].node, OpEqEq, yyvsp[0].node); ;
    break;}
case 60:
#line 250 "grammar.y"
{ yyval.node = new EqualNode(yyvsp[-2].node, OpNotEq, yyvsp[0].node); ;
    break;}
case 61:
#line 251 "grammar.y"
{ yyval.node = new EqualNode(yyvsp[-2].node, OpStrEq, yyvsp[0].node); ;
    break;}
case 62:
#line 252 "grammar.y"
{ yyval.node = new EqualNode(yyvsp[-2].node, OpStrNEq, yyvsp[0].node);;
    break;}
case 64:
#line 257 "grammar.y"
{ yyval.node = new BitOperNode(yyvsp[-2].node, OpBitAnd, yyvsp[0].node); ;
    break;}
case 66:
#line 262 "grammar.y"
{ yyval.node = new BitOperNode(yyvsp[-2].node, OpBitXOr, yyvsp[0].node); ;
    break;}
case 68:
#line 267 "grammar.y"
{ yyval.node = new BitOperNode(yyvsp[-2].node, OpBitOr, yyvsp[0].node); ;
    break;}
case 70:
#line 273 "grammar.y"
{ yyval.node = new BinaryLogicalNode(yyvsp[-2].node, OpAnd, yyvsp[0].node); ;
    break;}
case 72:
#line 279 "grammar.y"
{ yyval.node = new BinaryLogicalNode(yyvsp[-2].node, OpOr, yyvsp[0].node); ;
    break;}
case 74:
#line 285 "grammar.y"
{ yyval.node = new ConditionalNode(yyvsp[-4].node, yyvsp[-2].node, yyvsp[0].node); ;
    break;}
case 76:
#line 291 "grammar.y"
{ yyval.node = new AssignNode(yyvsp[-2].node, yyvsp[-1].op, yyvsp[0].node);;
    break;}
case 77:
#line 295 "grammar.y"
{ yyval.op = OpEqual; ;
    break;}
case 78:
#line 296 "grammar.y"
{ yyval.op = OpPlusEq; ;
    break;}
case 79:
#line 297 "grammar.y"
{ yyval.op = OpMinusEq; ;
    break;}
case 80:
#line 298 "grammar.y"
{ yyval.op = OpMultEq; ;
    break;}
case 81:
#line 299 "grammar.y"
{ yyval.op = OpDivEq; ;
    break;}
case 82:
#line 300 "grammar.y"
{ yyval.op = OpLShift; ;
    break;}
case 83:
#line 301 "grammar.y"
{ yyval.op = OpRShift; ;
    break;}
case 84:
#line 302 "grammar.y"
{ yyval.op = OpURShift; ;
    break;}
case 85:
#line 303 "grammar.y"
{ yyval.op = OpAndEq; ;
    break;}
case 86:
#line 304 "grammar.y"
{ yyval.op = OpXOrEq; ;
    break;}
case 87:
#line 305 "grammar.y"
{ yyval.op = OpOrEq; ;
    break;}
case 88:
#line 306 "grammar.y"
{ yyval.op = OpModEq; ;
    break;}
case 90:
#line 311 "grammar.y"
{ yyval.node = new CommaNode(yyvsp[-2].node, yyvsp[0].node); ;
    break;}
case 105:
#line 329 "grammar.y"
{ yyval.stat = new DebugNode(yyvsp[-1].node); ;
    break;}
case 106:
#line 333 "grammar.y"
{ yyval.stat = new BlockNode(0L); ;
    break;}
case 107:
#line 334 "grammar.y"
{ yyval.stat = new BlockNode(yyvsp[-1].slist); ;
    break;}
case 108:
#line 338 "grammar.y"
{ yyval.slist = new StatListNode(yyvsp[0].stat); ;
    break;}
case 109:
#line 339 "grammar.y"
{ yyval.slist = new StatListNode(yyvsp[-1].slist, yyvsp[0].stat); ;
    break;}
case 110:
#line 343 "grammar.y"
{ yyval.stat = new VarStatementNode(yyvsp[-1].vlist); ;
    break;}
case 111:
#line 347 "grammar.y"
{ yyval.vlist = new VarDeclListNode(yyvsp[0].decl); ;
    break;}
case 112:
#line 349 "grammar.y"
{ yyval.vlist = new VarDeclListNode(yyvsp[-2].vlist, yyvsp[0].decl); ;
    break;}
case 113:
#line 353 "grammar.y"
{ yyval.decl = new VarDeclNode(yyvsp[0].cstr, 0); delete yyvsp[0].cstr; ;
    break;}
case 114:
#line 354 "grammar.y"
{ yyval.decl = new VarDeclNode(yyvsp[-1].cstr, yyvsp[0].init); delete yyvsp[-1].cstr; ;
    break;}
case 115:
#line 358 "grammar.y"
{ yyval.init = new AssignExprNode(yyvsp[0].node); ;
    break;}
case 116:
#line 362 "grammar.y"
{ yyval.stat = new EmptyStatementNode(); ;
    break;}
case 117:
#line 366 "grammar.y"
{ yyval.stat = new ExprStatementNode(yyvsp[-1].node); ;
    break;}
case 118:
#line 370 "grammar.y"
{ yyval.stat = new IfNode(yyvsp[-2].node, yyvsp[0].stat, 0L); ;
    break;}
case 119:
#line 372 "grammar.y"
{ yyval.stat = new IfNode(yyvsp[-4].node, yyvsp[-2].stat, yyvsp[0].stat); ;
    break;}
case 120:
#line 376 "grammar.y"
{ yyval.stat = new DoWhileNode(yyvsp[-4].stat, yyvsp[-1].node); ;
    break;}
case 121:
#line 377 "grammar.y"
{ yyval.stat = new WhileNode(yyvsp[-2].node, yyvsp[0].stat); ;
    break;}
case 122:
#line 379 "grammar.y"
{ yyval.stat = new ForNode(yyvsp[-6].node, yyvsp[-4].node, yyvsp[-2].node, yyvsp[0].stat); ;
    break;}
case 123:
#line 381 "grammar.y"
{ yyval.stat = new ForNode(yyvsp[-6].vlist, yyvsp[-4].node, yyvsp[-2].node, yyvsp[0].stat); ;
    break;}
case 124:
#line 383 "grammar.y"
{ yyval.stat = new ForInNode(yyvsp[-4].node, yyvsp[-2].node, yyvsp[0].stat); ;
    break;}
case 125:
#line 385 "grammar.y"
{ yyval.stat = new ForInNode(yyvsp[-4].cstr, 0L, yyvsp[-2].node, yyvsp[0].stat);
                                     delete yyvsp[-4].cstr; ;
    break;}
case 126:
#line 388 "grammar.y"
{ yyval.stat = new ForInNode(yyvsp[-5].cstr, yyvsp[-4].init, yyvsp[-2].node, yyvsp[0].stat);
                                     delete yyvsp[-5].cstr; ;
    break;}
case 127:
#line 393 "grammar.y"
{ yyval.node = 0L; ;
    break;}
case 129:
#line 398 "grammar.y"
{ yyval.stat = new ContinueNode(); ;
    break;}
case 130:
#line 399 "grammar.y"
{ yyval.stat = new ContinueNode(yyvsp[-1].cstr); delete yyvsp[-1].cstr; ;
    break;}
case 131:
#line 403 "grammar.y"
{ yyval.stat = new BreakNode(); ;
    break;}
case 132:
#line 404 "grammar.y"
{ yyval.stat = new BreakNode(yyvsp[-1].cstr); delete yyvsp[-1].cstr; ;
    break;}
case 133:
#line 408 "grammar.y"
{ yyval.stat = new ReturnNode(0L); ;
    break;}
case 134:
#line 409 "grammar.y"
{ yyval.stat = new ReturnNode(yyvsp[-1].node); ;
    break;}
case 135:
#line 413 "grammar.y"
{ yyval.stat = new WithNode(yyvsp[-2].node, yyvsp[0].stat); ;
    break;}
case 136:
#line 417 "grammar.y"
{ yyval.stat = new SwitchNode(yyvsp[-2].node, yyvsp[0].cblk); ;
    break;}
case 137:
#line 421 "grammar.y"
{ yyval.cblk = new CaseBlockNode(yyvsp[-1].clist, 0L, 0L); ;
    break;}
case 138:
#line 423 "grammar.y"
{ yyval.cblk = new CaseBlockNode(yyvsp[-3].clist, yyvsp[-2].ccl, yyvsp[-1].clist); ;
    break;}
case 139:
#line 427 "grammar.y"
{ yyval.clist = 0L; ;
    break;}
case 141:
#line 432 "grammar.y"
{ yyval.clist = new ClauseListNode(yyvsp[0].ccl); ;
    break;}
case 142:
#line 433 "grammar.y"
{ yyval.clist = yyvsp[-1].clist->append(yyvsp[0].ccl); ;
    break;}
case 143:
#line 437 "grammar.y"
{ yyval.ccl = new CaseClauseNode(yyvsp[-1].node, 0L); ;
    break;}
case 144:
#line 438 "grammar.y"
{ yyval.ccl = new CaseClauseNode(yyvsp[-2].node, yyvsp[0].slist); ;
    break;}
case 145:
#line 442 "grammar.y"
{ yyval.ccl = new CaseClauseNode(0L, 0L);; ;
    break;}
case 146:
#line 443 "grammar.y"
{ yyval.ccl = new CaseClauseNode(0L, yyvsp[0].slist); ;
    break;}
case 147:
#line 447 "grammar.y"
{ yyval.stat = new LabelNode(yyvsp[-2].cstr, yyvsp[0].stat); delete yyvsp[-2].cstr; ;
    break;}
case 148:
#line 451 "grammar.y"
{ yyval.stat = new ThrowNode(yyvsp[-1].node); ;
    break;}
case 149:
#line 455 "grammar.y"
{ yyval.stat = new TryNode(); /* TODO */ ;
    break;}
case 150:
#line 456 "grammar.y"
{ yyval.stat = new TryNode(); /* TODO */ ;
    break;}
case 151:
#line 457 "grammar.y"
{ yyval.stat = new TryNode(); /* TODO */ ;
    break;}
case 152:
#line 461 "grammar.y"
{ yyval.node = new CatchNode(); /* TODO */ ;
    break;}
case 153:
#line 465 "grammar.y"
{ yyval.node = new FinallyNode(yyvsp[0].stat); ;
    break;}
case 154:
#line 469 "grammar.y"
{ yyval.func = new FuncDeclNode(yyvsp[-3].cstr, 0L, yyvsp[0].stat);
                                     delete yyvsp[-3].cstr; ;
    break;}
case 155:
#line 472 "grammar.y"
{ yyval.func = new FuncDeclNode(yyvsp[-4].cstr, yyvsp[-2].param, yyvsp[0].stat);
                                     delete yyvsp[-4].cstr; ;
    break;}
case 156:
#line 477 "grammar.y"
{ yyval.param = new ParameterNode(yyvsp[0].cstr); delete yyvsp[0].cstr; ;
    break;}
case 157:
#line 478 "grammar.y"
{ yyval.param = new ParameterNode(yyvsp[-2].param, yyvsp[0].cstr);
	                             delete yyvsp[0].cstr; ;
    break;}
case 158:
#line 483 "grammar.y"
{ yyval.prog = new ProgramNode(yyvsp[0].srcs); ;
    break;}
case 159:
#line 487 "grammar.y"
{ yyval.srcs = new SourceElementsNode(yyvsp[0].src); ;
    break;}
case 160:
#line 488 "grammar.y"
{ yyval.srcs = new SourceElementsNode(yyvsp[-1].srcs, yyvsp[0].src); ;
    break;}
case 161:
#line 492 "grammar.y"
{ yyval.src = new SourceElementNode(yyvsp[0].stat); ;
    break;}
case 162:
#line 493 "grammar.y"
{ yyval.src = new SourceElementNode(yyvsp[0].func); ;
    break;}
}
   /* the action file gets copied in in place of this dollarsign */
#line 498 "/usr/lib/bison.simple"

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
#line 496 "grammar.y"


int yyerror (const char *s)  /* Called by yyparse on error */
{
  fprintf(stderr, "ERROR: %s at line %d\n",
	  s, KJScript::lexer()->lineNo());
  return 1;
}
