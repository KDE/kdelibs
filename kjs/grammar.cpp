
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
#define	NULLTOKEN	258
#define	TRUETOKEN	259
#define	FALSETOKEN	260
#define	STRING	261
#define	DECIMAL	262
#define	INTEGER	263
#define	REGEXP	264
#define	BREAK	265
#define	CASE	266
#define	DEFAULT	267
#define	FOR	268
#define	NEW	269
#define	VAR	270
#define	CONTINUE	271
#define	FUNCTION	272
#define	RETURN	273
#define	VOID	274
#define	DELETE	275
#define	IF	276
#define	THIS	277
#define	DO	278
#define	WHILE	279
#define	ELSE	280
#define	IN	281
#define	INSTANCEOF	282
#define	TYPEOF	283
#define	SWITCH	284
#define	WITH	285
#define	RESERVED	286
#define	THROW	287
#define	TRY	288
#define	CATCH	289
#define	FINALLY	290
#define	EQEQ	291
#define	NE	292
#define	STREQ	293
#define	STRNEQ	294
#define	LE	295
#define	GE	296
#define	OR	297
#define	AND	298
#define	PLUSPLUS	299
#define	MINUSMINUS	300
#define	LSHIFT	301
#define	RSHIFT	302
#define	URSHIFT	303
#define	PLUSEQUAL	304
#define	MINUSEQUAL	305
#define	MULTEQUAL	306
#define	DIVEQUAL	307
#define	LSHIFTEQUAL	308
#define	RSHIFTEQUAL	309
#define	URSHIFTEQUAL	310
#define	ANDEQUAL	311
#define	MODEQUAL	312
#define	XOREQUAL	313
#define	OREQUAL	314
#define	DOUBLE	315
#define	IDENT	316
#define	AUTO	317

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

#include "kjs.h"
#include "nodes.h"
#include "lexer.h"

extern int yylex();
int yyerror (const char *);
bool automatic();

using namespace KJS;


#line 34 "grammar.y"
typedef union {
  int                 ival;
  double              dval;
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
  ElementNode         *elm;
  ElisionNode         *eli;
  Operator            op;
} YYSTYPE;
#include <stdio.h>

#ifndef __cplusplus
#ifndef __STDC__
#define const
#endif
#endif



#define	YYFINAL		321
#define	YYFLAG		-32768
#define	YYNTBASE	87

#define YYTRANSLATE(x) ((unsigned)(x) <= 317 ? yytranslate[x] : 147)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,    72,     2,     2,     2,    75,    78,     2,    63,
    64,    73,    69,    67,    70,    68,    74,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,    82,    86,    76,
    83,    77,    81,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
    65,     2,    66,    79,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,    84,    80,    85,    71,     2,     2,     2,     2,
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
    56,    57,    58,    59,    60,    61,    62
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     2,     4,     6,     8,    10,    12,    14,    16,    18,
    20,    22,    26,    30,    34,    40,    43,    48,    49,    51,
    53,    56,    58,    63,    67,    71,    73,    76,    79,    82,
    87,    91,    94,    98,   100,   104,   106,   108,   110,   113,
   116,   118,   121,   124,   127,   130,   134,   137,   141,   144,
   147,   150,   153,   155,   159,   163,   167,   169,   173,   177,
   179,   183,   187,   191,   193,   197,   201,   205,   209,   213,
   215,   219,   223,   227,   231,   233,   237,   239,   243,   245,
   249,   251,   255,   257,   261,   263,   269,   271,   275,   277,
   279,   281,   283,   285,   287,   289,   291,   293,   295,   297,
   299,   301,   305,   307,   309,   311,   313,   315,   317,   319,
   321,   323,   325,   327,   329,   331,   333,   336,   340,   342,
   345,   349,   353,   355,   359,   361,   364,   367,   369,   372,
   375,   381,   389,   396,   402,   412,   423,   431,   440,   450,
   451,   453,   456,   460,   463,   467,   470,   474,   480,   486,
   490,   496,   497,   499,   501,   504,   508,   513,   516,   520,
   524,   527,   531,   535,   539,   544,   550,   553,   559,   566,
   568,   572,   574,   576,   579,   581
};

static const short yyrhs[] = {     3,
     0,     4,     0,     5,     0,     8,     0,    60,     0,     6,
     0,     9,     0,    22,     0,    61,     0,    87,     0,    89,
     0,    63,   114,    64,     0,    65,    91,    66,     0,    65,
    90,    66,     0,    65,    90,    67,    91,    66,     0,    91,
   112,     0,    90,    67,    91,   112,     0,     0,    92,     0,
    67,     0,    92,    67,     0,    88,     0,    93,    65,   114,
    66,     0,    93,    68,    61,     0,    14,    93,    96,     0,
    93,     0,    14,    94,     0,    93,    96,     0,    95,    96,
     0,    95,    65,   114,    66,     0,    95,    68,    61,     0,
    63,    64,     0,    63,    97,    64,     0,   112,     0,    97,
    67,   112,     0,    94,     0,    95,     0,    98,     0,    98,
    44,     0,    98,    45,     0,    99,     0,    20,   100,     0,
    19,   100,     0,    28,   100,     0,    44,   100,     0,    62,
    44,   100,     0,    45,   100,     0,    62,    45,   100,     0,
    69,   100,     0,    70,   100,     0,    71,   100,     0,    72,
   100,     0,   100,     0,   101,    73,   100,     0,   101,    74,
   100,     0,   101,    75,   100,     0,   101,     0,   102,    69,
   101,     0,   102,    70,   101,     0,   102,     0,   103,    46,
   102,     0,   103,    47,   102,     0,   103,    48,   102,     0,
   103,     0,   104,    76,   103,     0,   104,    77,   103,     0,
   104,    40,   103,     0,   104,    41,   103,     0,   104,    27,
   103,     0,   104,     0,   105,    36,   104,     0,   105,    37,
   104,     0,   105,    38,   104,     0,   105,    39,   104,     0,
   105,     0,   106,    78,   105,     0,   106,     0,   107,    79,
   105,     0,   107,     0,   108,    80,   105,     0,   108,     0,
   109,    43,   108,     0,   109,     0,   110,    42,   106,     0,
   110,     0,   110,    81,   112,    82,   112,     0,   111,     0,
    98,   113,   112,     0,    83,     0,    49,     0,    50,     0,
    51,     0,    52,     0,    53,     0,    54,     0,    55,     0,
    56,     0,    58,     0,    59,     0,    57,     0,   112,     0,
   114,    67,   112,     0,   116,     0,   118,     0,   122,     0,
   123,     0,   124,     0,   125,     0,   127,     0,   128,     0,
   129,     0,   130,     0,   131,     0,   137,     0,   138,     0,
   139,     0,    84,    85,     0,    84,   117,    85,     0,   115,
     0,   117,   115,     0,    15,   119,    86,     0,    15,   119,
     1,     0,   120,     0,   119,    67,   120,     0,    61,     0,
    61,   121,     0,    83,   112,     0,    86,     0,   114,    86,
     0,   114,     1,     0,    21,    63,   114,    64,   115,     0,
    21,    63,   114,    64,   115,    25,   115,     0,    23,   115,
    24,    63,   114,    64,     0,    24,    63,   114,    64,   115,
     0,    13,    63,   126,    86,   126,    86,   126,    64,   115,
     0,    13,    63,    15,   119,    86,   126,    86,   126,    64,
   115,     0,    13,    63,    98,    26,   114,    64,   115,     0,
    13,    63,    15,    61,    26,   114,    64,   115,     0,    13,
    63,    15,    61,   121,    26,   114,    64,   115,     0,     0,
   114,     0,    16,    86,     0,    16,    61,    86,     0,    10,
    86,     0,    10,    61,    86,     0,    18,    86,     0,    18,
   114,    86,     0,    30,    63,   114,    64,   115,     0,    29,
    63,   114,    64,   132,     0,    84,   133,    85,     0,    84,
   133,   136,   133,    85,     0,     0,   134,     0,   135,     0,
   134,   135,     0,    11,   114,    82,     0,    11,   114,    82,
   117,     0,    12,    82,     0,    12,    82,   117,     0,    61,
    82,   115,     0,    32,    86,     0,    32,   114,    86,     0,
    33,   116,   140,     0,    33,   116,   141,     0,    33,   116,
   140,   141,     0,    34,    63,    61,    64,   116,     0,    35,
   116,     0,    17,    61,    63,    64,   116,     0,    17,    61,
    63,   143,    64,   116,     0,    61,     0,   143,    67,    61,
     0,   145,     0,   146,     0,   145,   146,     0,   115,     0,
   142,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
   143,   145,   146,   147,   148,   149,   150,   153,   155,   157,
   158,   159,   162,   164,   165,   168,   170,   174,   176,   179,
   181,   184,   186,   187,   189,   192,   194,   197,   199,   200,
   201,   204,   206,   209,   211,   214,   216,   219,   221,   222,
   225,   227,   228,   229,   230,   231,   232,   233,   234,   235,
   236,   237,   240,   242,   243,   244,   247,   249,   250,   253,
   255,   256,   257,   260,   262,   264,   266,   268,   270,   274,
   276,   277,   278,   279,   282,   284,   287,   289,   292,   294,
   297,   299,   303,   305,   309,   311,   315,   317,   321,   323,
   324,   325,   326,   327,   328,   329,   330,   331,   332,   333,
   336,   338,   341,   343,   344,   345,   346,   347,   348,   349,
   350,   351,   352,   353,   354,   355,   358,   360,   363,   365,
   368,   370,   376,   378,   382,   384,   387,   391,   395,   397,
   403,   405,   409,   411,   412,   414,   416,   418,   421,   426,
   428,   431,   433,   436,   438,   441,   443,   446,   450,   454,
   456,   460,   462,   465,   467,   470,   472,   475,   477,   480,
   484,   486,   489,   491,   492,   495,   499,   503,   506,   511,
   513,   517,   521,   523,   526,   528
};
#endif


#if YYDEBUG != 0 || defined (YYERROR_VERBOSE)

static const char * const yytname[] = {   "$","error","$undefined.","NULLTOKEN",
"TRUETOKEN","FALSETOKEN","STRING","DECIMAL","INTEGER","REGEXP","BREAK","CASE",
"DEFAULT","FOR","NEW","VAR","CONTINUE","FUNCTION","RETURN","VOID","DELETE","IF",
"THIS","DO","WHILE","ELSE","IN","INSTANCEOF","TYPEOF","SWITCH","WITH","RESERVED",
"THROW","TRY","CATCH","FINALLY","EQEQ","NE","STREQ","STRNEQ","LE","GE","OR",
"AND","PLUSPLUS","MINUSMINUS","LSHIFT","RSHIFT","URSHIFT","PLUSEQUAL","MINUSEQUAL",
"MULTEQUAL","DIVEQUAL","LSHIFTEQUAL","RSHIFTEQUAL","URSHIFTEQUAL","ANDEQUAL",
"MODEQUAL","XOREQUAL","OREQUAL","DOUBLE","IDENT","AUTO","'('","')'","'['","']'",
"','","'.'","'+'","'-'","'~'","'!'","'*'","'/'","'%'","'<'","'>'","'&'","'^'",
"'|'","'?'","':'","'='","'{'","'}'","';'","Literal","PrimaryExpr","ArrayLiteral",
"ElementList","ElisionOpt","Elision","MemberExpr","NewExpr","CallExpr","Arguments",
"ArgumentList","LeftHandSideExpr","PostfixExpr","UnaryExpr","MultiplicativeExpr",
"AdditiveExpr","ShiftExpr","RelationalExpr","EqualityExpr","BitwiseANDExpr",
"BitwiseXORExpr","BitwiseORExpr","LogicalANDExpr","LogicalORExpr","ConditionalExpr",
"AssignmentExpr","AssignmentOperator","Expr","Statement","Block","StatementList",
"VariableStatement","VariableDeclarationList","VariableDeclaration","Initializer",
"EmptyStatement","ExprStatement","IfStatement","IterationStatement","ExprOpt",
"ContinueStatement","BreakStatement","ReturnStatement","WithStatement","SwitchStatement",
"CaseBlock","CaseClausesOpt","CaseClauses","CaseClause","DefaultClause","LabelledStatement",
"ThrowStatement","TryStatement","Catch","Finally","FunctionDeclaration","FormalParameterList",
"Program","SourceElements","SourceElement", NULL
};
#endif

static const short yyr1[] = {     0,
    87,    87,    87,    87,    87,    87,    87,    88,    88,    88,
    88,    88,    89,    89,    89,    90,    90,    91,    91,    92,
    92,    93,    93,    93,    93,    94,    94,    95,    95,    95,
    95,    96,    96,    97,    97,    98,    98,    99,    99,    99,
   100,   100,   100,   100,   100,   100,   100,   100,   100,   100,
   100,   100,   101,   101,   101,   101,   102,   102,   102,   103,
   103,   103,   103,   104,   104,   104,   104,   104,   104,   105,
   105,   105,   105,   105,   106,   106,   107,   107,   108,   108,
   109,   109,   110,   110,   111,   111,   112,   112,   113,   113,
   113,   113,   113,   113,   113,   113,   113,   113,   113,   113,
   114,   114,   115,   115,   115,   115,   115,   115,   115,   115,
   115,   115,   115,   115,   115,   115,   116,   116,   117,   117,
   118,   118,   119,   119,   120,   120,   121,   122,   123,   123,
   124,   124,   125,   125,   125,   125,   125,   125,   125,   126,
   126,   127,   127,   128,   128,   129,   129,   130,   131,   132,
   132,   133,   133,   134,   134,   135,   135,   136,   136,   137,
   138,   138,   139,   139,   139,   140,   141,   142,   142,   143,
   143,   144,   145,   145,   146,   146
};

static const short yyr2[] = {     0,
     1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
     1,     3,     3,     3,     5,     2,     4,     0,     1,     1,
     2,     1,     4,     3,     3,     1,     2,     2,     2,     4,
     3,     2,     3,     1,     3,     1,     1,     1,     2,     2,
     1,     2,     2,     2,     2,     3,     2,     3,     2,     2,
     2,     2,     1,     3,     3,     3,     1,     3,     3,     1,
     3,     3,     3,     1,     3,     3,     3,     3,     3,     1,
     3,     3,     3,     3,     1,     3,     1,     3,     1,     3,
     1,     3,     1,     3,     1,     5,     1,     3,     1,     1,
     1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
     1,     3,     1,     1,     1,     1,     1,     1,     1,     1,
     1,     1,     1,     1,     1,     1,     2,     3,     1,     2,
     3,     3,     1,     3,     1,     2,     2,     1,     2,     2,
     5,     7,     6,     5,     9,    10,     7,     8,     9,     0,
     1,     2,     3,     2,     3,     2,     3,     5,     5,     3,
     5,     0,     1,     1,     2,     3,     4,     2,     3,     3,
     2,     3,     3,     3,     4,     5,     2,     5,     6,     1,
     3,     1,     1,     2,     1,     1
};

static const short yydefact[] = {     0,
     1,     2,     3,     6,     4,     7,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     8,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     5,     9,     0,     0,
    18,     0,     0,     0,     0,     0,   128,    10,    22,    11,
    26,    36,    37,    38,    41,    53,    57,    60,    64,    70,
    75,    77,    79,    81,    83,    85,    87,   101,     0,   175,
   103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
   113,   114,   115,   116,   176,   172,   173,     0,   144,   140,
     9,    26,    27,   125,     0,   123,     0,   142,     0,   146,
     0,    38,    43,    42,     0,     0,     0,    44,     0,     0,
   161,     0,     0,    45,    47,     0,     0,     0,     0,    20,
     0,     0,    19,    49,    50,    51,    52,   117,   119,     0,
     0,     0,     0,    28,     0,     0,    29,    39,    40,    90,
    91,    92,    93,    94,    95,    96,    97,   100,    98,    99,
    89,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,   130,     0,   129,   174,   145,
     0,    38,   141,     0,    25,     0,   126,   122,     0,   121,
   143,     0,   147,     0,     0,     0,     0,     0,   162,     0,
     0,   163,   164,   160,    46,    48,    12,    14,    18,    13,
    16,    21,   118,   120,    32,     0,    34,     0,    24,     0,
    31,    88,    54,    55,    56,    58,    59,    61,    62,    63,
    69,    67,    68,    65,    66,    71,    72,    73,    74,    76,
    78,    80,    82,    84,     0,   102,   125,     0,     0,   140,
   127,   124,   170,     0,     0,     0,     0,     0,     0,     0,
     0,   167,   165,     0,    33,     0,    23,    30,     0,     0,
   126,   140,     0,     0,   168,     0,     0,   131,     0,   134,
   152,   149,   148,     0,    15,    17,    35,    86,     0,     0,
     0,     0,   140,   169,   171,     0,   133,     0,     0,   153,
   154,     0,     0,     0,   140,   137,     0,   132,     0,     0,
   150,   152,   155,   166,   138,     0,     0,     0,   156,   158,
     0,   139,     0,   135,   157,   159,   151,   136,     0,     0,
     0
};

static const short yydefgoto[] = {    38,
    39,    40,   111,   112,   113,    41,    42,    43,   124,   206,
    44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
    54,    55,    56,    57,    58,   142,    59,   119,    61,   120,
    62,    85,    86,   177,    63,    64,    65,    66,   174,    67,
    68,    69,    70,    71,   272,   289,   290,   291,   302,    72,
    73,    74,   192,   193,    75,   245,   319,    76,    77
};

static const short yypact[] = {   172,
-32768,-32768,-32768,-32768,-32768,-32768,   -48,   -42,   205,   -24,
   -41,     2,   553,   850,   850,   -37,-32768,   482,    -8,   850,
    -2,    18,   582,   -54,   850,   850,-32768,     1,    33,   850,
    38,   850,   850,   850,   850,   314,-32768,-32768,-32768,-32768,
    -3,-32768,    22,   311,-32768,-32768,    93,    25,    -5,    26,
    71,   101,    21,   127,   175,   -31,-32768,-32768,     6,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,   172,-32768,   140,-32768,   652,
-32768,    -3,-32768,   157,     7,-32768,   161,-32768,   189,-32768,
   -53,    68,-32768,-32768,   850,   229,   850,-32768,   850,   850,
-32768,   -35,   121,-32768,-32768,   482,   850,   850,     4,-32768,
   106,   681,   187,-32768,-32768,-32768,-32768,-32768,-32768,   398,
   751,   850,   194,-32768,   850,   196,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,   850,   850,   850,   850,   850,   850,   850,   850,   850,
   850,   850,   850,   850,   850,   850,   850,   850,   850,   850,
   850,   850,   850,   850,   850,-32768,   850,-32768,-32768,-32768,
   198,   831,   193,   176,-32768,   850,-32768,-32768,   -24,-32768,
-32768,    11,-32768,    32,   200,    37,    63,    64,-32768,   201,
   -54,   226,-32768,-32768,-32768,-32768,-32768,-32768,    38,-32768,
-32768,-32768,-32768,-32768,-32768,    65,-32768,   117,-32768,   158,
-32768,-32768,-32768,-32768,-32768,    93,    93,    25,    25,    25,
    -5,    -5,    -5,    -5,    -5,    26,    26,    26,    26,    71,
    71,    71,   127,   101,   185,-32768,   -25,   -32,   850,   850,
-32768,-32768,-32768,   -54,    96,   482,   850,   482,   190,   482,
   212,-32768,-32768,   780,-32768,   850,-32768,-32768,   850,   850,
   249,   850,    97,   192,-32768,   -54,   215,   254,    98,-32768,
   269,-32768,-32768,   217,-32768,-32768,-32768,-32768,   139,   850,
   197,   482,   850,-32768,-32768,   482,-32768,   850,    -6,   269,
-32768,   -54,   482,   148,   850,-32768,   221,-32768,   -23,   206,
-32768,   269,-32768,-32768,-32768,   482,   223,   482,   482,   482,
   204,-32768,   482,-32768,   482,   482,-32768,-32768,   290,   291,
-32768
};

static const short yypgoto[] = {-32768,
-32768,-32768,-32768,    95,-32768,   283,   286,-32768,   -26,-32768,
   -10,-32768,    14,    84,    49,   -36,   -33,    60,   132,-32768,
   134,-32768,-32768,-32768,   -85,-32768,   -11,     0,   -21,   -71,
-32768,   128,   119,    66,-32768,-32768,-32768,-32768,  -231,-32768,
-32768,-32768,-32768,-32768,-32768,    -1,-32768,    10,-32768,-32768,
-32768,-32768,-32768,   110,-32768,-32768,-32768,-32768,   228
};


#define	YYLAST		922


static const short yytable[] = {    60,
   260,    91,   103,    92,    92,   300,   166,   178,   264,    92,
   164,   102,    78,   167,    92,    92,   127,    96,   109,    87,
    80,    92,    92,    92,    92,    95,   201,    93,    94,    36,
   281,   167,   183,    98,   179,   207,    84,    79,   104,   105,
   148,   149,   150,   167,    88,   114,   115,   116,   117,   165,
   189,   297,   151,   262,    97,   175,   212,   176,   309,   121,
    99,   122,    89,   307,   123,   152,   153,   197,   173,   172,
   167,   243,   167,   179,   244,    60,   107,   108,   301,   235,
   100,   236,   106,   184,   121,   186,   125,   187,   188,   126,
   241,   168,   180,   146,   147,   246,    92,    92,   167,   161,
   248,   154,   155,   167,   110,   194,   156,   157,   158,   159,
   208,   128,   129,   210,   221,   222,   223,   224,   225,   204,
   195,   196,   226,   227,   228,   229,   249,   250,   255,   167,
   167,   256,    92,    92,    92,    92,    92,    92,    92,    92,
    92,    92,    92,    92,    92,    92,    92,    92,    92,    92,
    92,    92,    92,    92,   190,   191,   213,   214,   215,   266,
   282,   287,   267,   167,   167,   143,   144,   145,   276,   252,
   277,   198,   199,   278,     1,     2,     3,     4,   160,     5,
     6,     7,   257,   167,     8,     9,    10,    11,    12,    13,
    14,    15,    16,    17,    18,    19,   218,   219,   220,    20,
    21,    22,   293,    23,    24,   167,   162,     1,     2,     3,
     4,   306,     5,     6,   167,    25,    26,   163,     9,   230,
   231,   232,   265,   258,   167,   170,    17,   263,   173,   216,
   217,    27,    28,    29,    30,   269,    31,   315,   316,   176,
    32,    33,    34,    35,   284,   268,   181,   270,   279,   273,
   173,   182,   185,   202,   209,    36,   211,    37,   237,   167,
   191,   240,   247,   251,    27,    81,   259,    30,   294,    31,
   304,   173,   274,   271,   280,   285,   299,   283,   286,   288,
   292,   296,   295,   173,   308,   298,   313,   310,   317,   320,
   321,    82,   305,   254,    83,   234,   233,   242,   238,   303,
   311,   253,   261,   169,     0,   312,     0,   314,     0,     0,
     0,     0,   318,     0,   204,   204,     1,     2,     3,     4,
     0,     5,     6,     7,     0,     0,     8,     9,    10,    11,
     0,    13,    14,    15,    16,    17,    18,    19,     0,     0,
     0,    20,    21,    22,     0,    23,    24,     0,     0,     0,
     0,     0,     0,     0,   128,   129,     0,    25,    26,   130,
   131,   132,   133,   134,   135,   136,   137,   138,   139,   140,
     0,     0,     0,    27,    28,    29,    30,     0,    31,     0,
     0,     0,    32,    33,    34,    35,     0,     0,     0,     0,
     0,     0,     0,   141,     0,     0,     0,    36,   118,    37,
     1,     2,     3,     4,     0,     5,     6,     7,     0,     0,
     8,     9,    10,    11,     0,    13,    14,    15,    16,    17,
    18,    19,     0,     0,     0,    20,    21,    22,     0,    23,
    24,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,    25,    26,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,    27,    28,    29,
    30,     0,    31,     0,     0,     0,    32,    33,    34,    35,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,    36,   203,    37,     1,     2,     3,     4,     0,     5,
     6,     7,     0,     0,     8,     9,    10,    11,     0,    13,
    14,    15,    16,    17,    18,    19,     0,     0,     0,    20,
    21,    22,     0,    23,    24,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,    25,    26,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,    27,    28,    29,    30,     0,    31,     0,     0,     0,
    32,    33,    34,    35,     0,     1,     2,     3,     4,     0,
     5,     6,     0,     0,     0,    36,     9,    37,     0,     0,
     0,    14,    15,     0,    17,     0,     0,     0,     0,     0,
    20,     0,     0,     0,     1,     2,     3,     4,     0,     5,
     6,     0,     0,     0,     0,     9,    25,    26,     0,     0,
    14,    15,     0,    17,     0,     0,     0,     0,     0,    20,
     0,     0,    27,    81,    29,    30,     0,    31,     0,     0,
     0,    32,    33,    34,    35,    25,    26,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,    90,     0,
     0,    27,    81,    29,    30,     0,    31,     0,     0,     0,
    32,    33,    34,    35,     1,     2,     3,     4,     0,     5,
     6,     0,     0,     0,     0,     9,   171,   101,     0,     0,
    14,    15,     0,    17,     0,     0,     0,     0,     0,    20,
     0,     0,     0,     1,     2,     3,     4,     0,     5,     6,
     0,     0,     0,     0,     9,    25,    26,     0,     0,    14,
    15,     0,    17,     0,     0,     0,     0,     0,    20,     0,
     0,    27,    81,    29,    30,     0,    31,     0,     0,     0,
    32,    33,    34,    35,    25,    26,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    27,    81,    29,    30,     0,    31,   200,     0,     0,    32,
    33,    34,    35,     1,     2,     3,     4,     0,     5,     6,
     0,     0,     0,     0,     9,     0,     0,     0,     0,    14,
    15,     0,    17,     0,     0,     0,     0,     0,    20,     0,
     0,     0,     1,     2,     3,     4,     0,     5,     6,     0,
     0,     0,     0,     9,    25,    26,     0,     0,    14,    15,
     0,    17,     0,     0,     0,     0,     0,    20,     0,     0,
    27,    81,    29,    30,   205,    31,     0,     0,     0,    32,
    33,    34,    35,    25,    26,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,    27,
    81,    29,    30,     0,    31,   275,     0,     0,    32,    33,
    34,    35,     1,     2,     3,     4,   239,     5,     6,     0,
     0,     0,     0,     9,     0,     0,     0,     0,    14,    15,
     0,    17,     0,     0,   128,   129,     0,    20,     0,   130,
   131,   132,   133,   134,   135,   136,   137,   138,   139,   140,
     0,     0,     0,    25,    26,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,    27,
    81,    29,    30,   141,    31,     0,     0,     0,    32,    33,
    34,    35
};

static const short yycheck[] = {     0,
    26,    13,    24,    14,    15,    12,     1,     1,   240,    20,
    42,    23,    61,    67,    25,    26,    43,    18,    30,    61,
    63,    32,    33,    34,    35,    63,   112,    14,    15,    84,
   262,    67,    86,    20,    67,   121,    61,    86,    25,    26,
    46,    47,    48,    67,    86,    32,    33,    34,    35,    81,
    86,   283,    27,    86,    63,    82,   142,    83,    82,    63,
    63,    65,    61,   295,    68,    40,    41,    64,    80,    80,
    67,    61,    67,    67,    64,    76,    44,    45,    85,   165,
    63,   167,    82,    95,    63,    97,    65,    99,   100,    68,
   176,    86,    86,    69,    70,    64,   107,   108,    67,    79,
    64,    76,    77,    67,    67,   106,    36,    37,    38,    39,
   122,    44,    45,   125,   151,   152,   153,   154,   155,   120,
   107,   108,   156,   157,   158,   159,    64,    64,    64,    67,
    67,    67,   143,   144,   145,   146,   147,   148,   149,   150,
   151,   152,   153,   154,   155,   156,   157,   158,   159,   160,
   161,   162,   163,   164,    34,    35,   143,   144,   145,    64,
    64,    64,    67,    67,    67,    73,    74,    75,   254,   191,
   256,    66,    67,   259,     3,     4,     5,     6,    78,     8,
     9,    10,    66,    67,    13,    14,    15,    16,    17,    18,
    19,    20,    21,    22,    23,    24,   148,   149,   150,    28,
    29,    30,    64,    32,    33,    67,    80,     3,     4,     5,
     6,    64,     8,     9,    67,    44,    45,    43,    14,   160,
   161,   162,   244,    66,    67,    86,    22,   239,   240,   146,
   147,    60,    61,    62,    63,   247,    65,   309,   310,    83,
    69,    70,    71,    72,   266,   246,    86,   248,   260,   250,
   262,    63,    24,    67,    61,    84,    61,    86,    61,    67,
    35,    86,    63,    63,    60,    61,    82,    63,   280,    65,
   292,   283,    61,    84,    26,    61,   288,    86,    25,    11,
    64,   282,    86,   295,    64,   286,    64,    82,    85,     0,
     0,     9,   293,   199,     9,   164,   163,   179,   171,   290,
   302,   192,   237,    76,    -1,   306,    -1,   308,    -1,    -1,
    -1,    -1,   313,    -1,   315,   316,     3,     4,     5,     6,
    -1,     8,     9,    10,    -1,    -1,    13,    14,    15,    16,
    -1,    18,    19,    20,    21,    22,    23,    24,    -1,    -1,
    -1,    28,    29,    30,    -1,    32,    33,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    44,    45,    -1,    44,    45,    49,
    50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
    -1,    -1,    -1,    60,    61,    62,    63,    -1,    65,    -1,
    -1,    -1,    69,    70,    71,    72,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    83,    -1,    -1,    -1,    84,    85,    86,
     3,     4,     5,     6,    -1,     8,     9,    10,    -1,    -1,
    13,    14,    15,    16,    -1,    18,    19,    20,    21,    22,
    23,    24,    -1,    -1,    -1,    28,    29,    30,    -1,    32,
    33,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    44,    45,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    60,    61,    62,
    63,    -1,    65,    -1,    -1,    -1,    69,    70,    71,    72,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    84,    85,    86,     3,     4,     5,     6,    -1,     8,
     9,    10,    -1,    -1,    13,    14,    15,    16,    -1,    18,
    19,    20,    21,    22,    23,    24,    -1,    -1,    -1,    28,
    29,    30,    -1,    32,    33,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    44,    45,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    60,    61,    62,    63,    -1,    65,    -1,    -1,    -1,
    69,    70,    71,    72,    -1,     3,     4,     5,     6,    -1,
     8,     9,    -1,    -1,    -1,    84,    14,    86,    -1,    -1,
    -1,    19,    20,    -1,    22,    -1,    -1,    -1,    -1,    -1,
    28,    -1,    -1,    -1,     3,     4,     5,     6,    -1,     8,
     9,    -1,    -1,    -1,    -1,    14,    44,    45,    -1,    -1,
    19,    20,    -1,    22,    -1,    -1,    -1,    -1,    -1,    28,
    -1,    -1,    60,    61,    62,    63,    -1,    65,    -1,    -1,
    -1,    69,    70,    71,    72,    44,    45,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    86,    -1,
    -1,    60,    61,    62,    63,    -1,    65,    -1,    -1,    -1,
    69,    70,    71,    72,     3,     4,     5,     6,    -1,     8,
     9,    -1,    -1,    -1,    -1,    14,    15,    86,    -1,    -1,
    19,    20,    -1,    22,    -1,    -1,    -1,    -1,    -1,    28,
    -1,    -1,    -1,     3,     4,     5,     6,    -1,     8,     9,
    -1,    -1,    -1,    -1,    14,    44,    45,    -1,    -1,    19,
    20,    -1,    22,    -1,    -1,    -1,    -1,    -1,    28,    -1,
    -1,    60,    61,    62,    63,    -1,    65,    -1,    -1,    -1,
    69,    70,    71,    72,    44,    45,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    60,    61,    62,    63,    -1,    65,    66,    -1,    -1,    69,
    70,    71,    72,     3,     4,     5,     6,    -1,     8,     9,
    -1,    -1,    -1,    -1,    14,    -1,    -1,    -1,    -1,    19,
    20,    -1,    22,    -1,    -1,    -1,    -1,    -1,    28,    -1,
    -1,    -1,     3,     4,     5,     6,    -1,     8,     9,    -1,
    -1,    -1,    -1,    14,    44,    45,    -1,    -1,    19,    20,
    -1,    22,    -1,    -1,    -1,    -1,    -1,    28,    -1,    -1,
    60,    61,    62,    63,    64,    65,    -1,    -1,    -1,    69,
    70,    71,    72,    44,    45,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    60,
    61,    62,    63,    -1,    65,    66,    -1,    -1,    69,    70,
    71,    72,     3,     4,     5,     6,    26,     8,     9,    -1,
    -1,    -1,    -1,    14,    -1,    -1,    -1,    -1,    19,    20,
    -1,    22,    -1,    -1,    44,    45,    -1,    28,    -1,    49,
    50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
    -1,    -1,    -1,    44,    45,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    60,
    61,    62,    63,    83,    65,    -1,    -1,    -1,    69,    70,
    71,    72
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
#line 144 "grammar.y"
{ yyval.node = new NullNode(); ;
    break;}
case 2:
#line 145 "grammar.y"
{ yyval.node = new BooleanNode(true); ;
    break;}
case 3:
#line 146 "grammar.y"
{ yyval.node = new BooleanNode(false); ;
    break;}
case 4:
#line 147 "grammar.y"
{ yyval.node = new NumberNode(yyvsp[0].ival); ;
    break;}
case 5:
#line 148 "grammar.y"
{ yyval.node = new NumberNode(yyvsp[0].dval); ;
    break;}
case 6:
#line 149 "grammar.y"
{ yyval.node = new StringNode(yyvsp[0].ustr); delete yyvsp[0].ustr; ;
    break;}
case 7:
#line 150 "grammar.y"
{ yyval.node = new NullNode(); /* TODO */ ;
    break;}
case 8:
#line 154 "grammar.y"
{ yyval.node = new ThisNode(); ;
    break;}
case 9:
#line 155 "grammar.y"
{ yyval.node = new ResolveNode(yyvsp[0].ustr);
                                     delete yyvsp[0].ustr; ;
    break;}
case 12:
#line 159 "grammar.y"
{ yyval.node = new GroupNode(yyvsp[-1].node); ;
    break;}
case 13:
#line 163 "grammar.y"
{ yyval.node = new ArrayNode(yyvsp[-1].eli); ;
    break;}
case 14:
#line 164 "grammar.y"
{ yyval.node = new ArrayNode(yyvsp[-1].elm, 0L); ;
    break;}
case 15:
#line 165 "grammar.y"
{ yyval.node = new ArrayNode(yyvsp[-3].elm, yyvsp[-1].eli); ;
    break;}
case 16:
#line 169 "grammar.y"
{ yyval.elm = new ElementNode(yyvsp[-1].eli, yyvsp[0].node); ;
    break;}
case 17:
#line 171 "grammar.y"
{ yyval.elm = yyvsp[-3].elm->append(yyvsp[-1].eli, yyvsp[0].node); ;
    break;}
case 18:
#line 175 "grammar.y"
{ yyval.eli = 0L; ;
    break;}
case 20:
#line 180 "grammar.y"
{ yyval.eli = new ElisionNode(0L); ;
    break;}
case 21:
#line 181 "grammar.y"
{ yyval.eli = new ElisionNode(yyvsp[-1].eli); ;
    break;}
case 23:
#line 186 "grammar.y"
{ yyval.node = new AccessorNode1(yyvsp[-3].node, yyvsp[-1].node); ;
    break;}
case 24:
#line 187 "grammar.y"
{ yyval.node = new AccessorNode2(yyvsp[-2].node, yyvsp[0].ustr);
                                     delete yyvsp[0].ustr; ;
    break;}
case 25:
#line 189 "grammar.y"
{ yyval.node = new NewExprNode(yyvsp[-1].node, yyvsp[0].args); ;
    break;}
case 27:
#line 194 "grammar.y"
{ yyval.node = new NewExprNode(yyvsp[0].node); ;
    break;}
case 28:
#line 198 "grammar.y"
{ yyval.node = new FunctionCallNode(yyvsp[-1].node, yyvsp[0].args); ;
    break;}
case 29:
#line 199 "grammar.y"
{ yyval.node = new FunctionCallNode(yyvsp[-1].node, yyvsp[0].args); ;
    break;}
case 30:
#line 200 "grammar.y"
{ yyval.node = new AccessorNode1(yyvsp[-3].node, yyvsp[-1].node); ;
    break;}
case 31:
#line 201 "grammar.y"
{ yyval.node = new AccessorNode2(yyvsp[-2].node, yyvsp[0].ustr); ;
    break;}
case 32:
#line 205 "grammar.y"
{ yyval.args = new ArgumentsNode(0L); ;
    break;}
case 33:
#line 206 "grammar.y"
{ yyval.args = new ArgumentsNode(yyvsp[-1].alist); ;
    break;}
case 34:
#line 210 "grammar.y"
{ yyval.alist = new ArgumentListNode(yyvsp[0].node); ;
    break;}
case 35:
#line 211 "grammar.y"
{ yyval.alist = new ArgumentListNode(yyvsp[-2].alist, yyvsp[0].node); ;
    break;}
case 39:
#line 221 "grammar.y"
{ yyval.node = new PostfixNode(yyvsp[-1].node, OpPlusPlus); ;
    break;}
case 40:
#line 222 "grammar.y"
{ yyval.node = new PostfixNode(yyvsp[-1].node, OpMinusMinus); ;
    break;}
case 42:
#line 227 "grammar.y"
{ yyval.node = new DeleteNode(yyvsp[0].node); ;
    break;}
case 43:
#line 228 "grammar.y"
{ yyval.node = new VoidNode(yyvsp[0].node); ;
    break;}
case 44:
#line 229 "grammar.y"
{ yyval.node = new TypeOfNode(yyvsp[0].node); ;
    break;}
case 45:
#line 230 "grammar.y"
{ yyval.node = new PrefixNode(OpPlusPlus, yyvsp[0].node); ;
    break;}
case 46:
#line 231 "grammar.y"
{ yyval.node = new PrefixNode(OpPlusPlus, yyvsp[0].node); ;
    break;}
case 47:
#line 232 "grammar.y"
{ yyval.node = new PrefixNode(OpMinusMinus, yyvsp[0].node); ;
    break;}
case 48:
#line 233 "grammar.y"
{ yyval.node = new PrefixNode(OpMinusMinus, yyvsp[0].node); ;
    break;}
case 49:
#line 234 "grammar.y"
{ yyval.node = new UnaryPlusNode(yyvsp[0].node); ;
    break;}
case 50:
#line 235 "grammar.y"
{ yyval.node = new NegateNode(yyvsp[0].node); ;
    break;}
case 51:
#line 236 "grammar.y"
{ yyval.node = new BitwiseNotNode(yyvsp[0].node); ;
    break;}
case 52:
#line 237 "grammar.y"
{ yyval.node = new LogicalNotNode(yyvsp[0].node); ;
    break;}
case 54:
#line 242 "grammar.y"
{ yyval.node = new MultNode(yyvsp[-2].node, yyvsp[0].node, '*'); ;
    break;}
case 55:
#line 243 "grammar.y"
{ yyval.node = new MultNode(yyvsp[-2].node, yyvsp[0].node, '/'); ;
    break;}
case 56:
#line 244 "grammar.y"
{ yyval.node = new MultNode(yyvsp[-2].node,yyvsp[0].node,'%'); ;
    break;}
case 58:
#line 249 "grammar.y"
{ yyval.node = new AddNode(yyvsp[-2].node, yyvsp[0].node, '+'); ;
    break;}
case 59:
#line 250 "grammar.y"
{ yyval.node = new AddNode(yyvsp[-2].node, yyvsp[0].node, '-'); ;
    break;}
case 61:
#line 255 "grammar.y"
{ yyval.node = new ShiftNode(yyvsp[-2].node, OpLShift, yyvsp[0].node); ;
    break;}
case 62:
#line 256 "grammar.y"
{ yyval.node = new ShiftNode(yyvsp[-2].node, OpRShift, yyvsp[0].node); ;
    break;}
case 63:
#line 257 "grammar.y"
{ yyval.node = new ShiftNode(yyvsp[-2].node, OpURShift, yyvsp[0].node); ;
    break;}
case 65:
#line 263 "grammar.y"
{ yyval.node = new RelationalNode(yyvsp[-2].node, OpLess, yyvsp[0].node); ;
    break;}
case 66:
#line 265 "grammar.y"
{ yyval.node = new RelationalNode(yyvsp[-2].node, OpGreater, yyvsp[0].node); ;
    break;}
case 67:
#line 267 "grammar.y"
{ yyval.node = new RelationalNode(yyvsp[-2].node, OpLessEq, yyvsp[0].node); ;
    break;}
case 68:
#line 269 "grammar.y"
{ yyval.node = new RelationalNode(yyvsp[-2].node, OpGreaterEq, yyvsp[0].node); ;
    break;}
case 69:
#line 271 "grammar.y"
{ yyval.node = new RelationalNode(yyvsp[-2].node, OpInstanceOf, yyvsp[0].node); ;
    break;}
case 71:
#line 276 "grammar.y"
{ yyval.node = new EqualNode(yyvsp[-2].node, OpEqEq, yyvsp[0].node); ;
    break;}
case 72:
#line 277 "grammar.y"
{ yyval.node = new EqualNode(yyvsp[-2].node, OpNotEq, yyvsp[0].node); ;
    break;}
case 73:
#line 278 "grammar.y"
{ yyval.node = new EqualNode(yyvsp[-2].node, OpStrEq, yyvsp[0].node); ;
    break;}
case 74:
#line 279 "grammar.y"
{ yyval.node = new EqualNode(yyvsp[-2].node, OpStrNEq, yyvsp[0].node);;
    break;}
case 76:
#line 284 "grammar.y"
{ yyval.node = new BitOperNode(yyvsp[-2].node, OpBitAnd, yyvsp[0].node); ;
    break;}
case 78:
#line 289 "grammar.y"
{ yyval.node = new BitOperNode(yyvsp[-2].node, OpBitXOr, yyvsp[0].node); ;
    break;}
case 80:
#line 294 "grammar.y"
{ yyval.node = new BitOperNode(yyvsp[-2].node, OpBitOr, yyvsp[0].node); ;
    break;}
case 82:
#line 300 "grammar.y"
{ yyval.node = new BinaryLogicalNode(yyvsp[-2].node, OpAnd, yyvsp[0].node); ;
    break;}
case 84:
#line 306 "grammar.y"
{ yyval.node = new BinaryLogicalNode(yyvsp[-2].node, OpOr, yyvsp[0].node); ;
    break;}
case 86:
#line 312 "grammar.y"
{ yyval.node = new ConditionalNode(yyvsp[-4].node, yyvsp[-2].node, yyvsp[0].node); ;
    break;}
case 88:
#line 318 "grammar.y"
{ yyval.node = new AssignNode(yyvsp[-2].node, yyvsp[-1].op, yyvsp[0].node);;
    break;}
case 89:
#line 322 "grammar.y"
{ yyval.op = OpEqual; ;
    break;}
case 90:
#line 323 "grammar.y"
{ yyval.op = OpPlusEq; ;
    break;}
case 91:
#line 324 "grammar.y"
{ yyval.op = OpMinusEq; ;
    break;}
case 92:
#line 325 "grammar.y"
{ yyval.op = OpMultEq; ;
    break;}
case 93:
#line 326 "grammar.y"
{ yyval.op = OpDivEq; ;
    break;}
case 94:
#line 327 "grammar.y"
{ yyval.op = OpLShift; ;
    break;}
case 95:
#line 328 "grammar.y"
{ yyval.op = OpRShift; ;
    break;}
case 96:
#line 329 "grammar.y"
{ yyval.op = OpURShift; ;
    break;}
case 97:
#line 330 "grammar.y"
{ yyval.op = OpAndEq; ;
    break;}
case 98:
#line 331 "grammar.y"
{ yyval.op = OpXOrEq; ;
    break;}
case 99:
#line 332 "grammar.y"
{ yyval.op = OpOrEq; ;
    break;}
case 100:
#line 333 "grammar.y"
{ yyval.op = OpModEq; ;
    break;}
case 102:
#line 338 "grammar.y"
{ yyval.node = new CommaNode(yyvsp[-2].node, yyvsp[0].node); ;
    break;}
case 117:
#line 359 "grammar.y"
{ yyval.stat = new BlockNode(0L); ;
    break;}
case 118:
#line 360 "grammar.y"
{ yyval.stat = new BlockNode(yyvsp[-1].slist); ;
    break;}
case 119:
#line 364 "grammar.y"
{ yyval.slist = new StatListNode(yyvsp[0].stat); ;
    break;}
case 120:
#line 365 "grammar.y"
{ yyval.slist = new StatListNode(yyvsp[-1].slist, yyvsp[0].stat); ;
    break;}
case 121:
#line 369 "grammar.y"
{ yyval.stat = new VarStatementNode(yyvsp[-1].vlist); ;
    break;}
case 122:
#line 370 "grammar.y"
{ if (automatic())
                                          yyval.stat = new VarStatementNode(yyvsp[-1].vlist);
                                        else
					  YYABORT; ;
    break;}
case 123:
#line 377 "grammar.y"
{ yyval.vlist = new VarDeclListNode(yyvsp[0].decl); ;
    break;}
case 124:
#line 379 "grammar.y"
{ yyval.vlist = new VarDeclListNode(yyvsp[-2].vlist, yyvsp[0].decl); ;
    break;}
case 125:
#line 383 "grammar.y"
{ yyval.decl = new VarDeclNode(yyvsp[0].ustr, 0); delete yyvsp[0].ustr; ;
    break;}
case 126:
#line 384 "grammar.y"
{ yyval.decl = new VarDeclNode(yyvsp[-1].ustr, yyvsp[0].init); delete yyvsp[-1].ustr; ;
    break;}
case 127:
#line 388 "grammar.y"
{ yyval.init = new AssignExprNode(yyvsp[0].node); ;
    break;}
case 128:
#line 392 "grammar.y"
{ yyval.stat = new EmptyStatementNode(); ;
    break;}
case 129:
#line 396 "grammar.y"
{ yyval.stat = new ExprStatementNode(yyvsp[-1].node); ;
    break;}
case 130:
#line 397 "grammar.y"
{ if (automatic())
                                       yyval.stat = new ExprStatementNode(yyvsp[-1].node);
                                     else
				       YYABORT; ;
    break;}
case 131:
#line 404 "grammar.y"
{ yyval.stat = new IfNode(yyvsp[-2].node, yyvsp[0].stat, 0L); ;
    break;}
case 132:
#line 406 "grammar.y"
{ yyval.stat = new IfNode(yyvsp[-4].node, yyvsp[-2].stat, yyvsp[0].stat); ;
    break;}
case 133:
#line 410 "grammar.y"
{ yyval.stat = new DoWhileNode(yyvsp[-4].stat, yyvsp[-1].node); ;
    break;}
case 134:
#line 411 "grammar.y"
{ yyval.stat = new WhileNode(yyvsp[-2].node, yyvsp[0].stat); ;
    break;}
case 135:
#line 413 "grammar.y"
{ yyval.stat = new ForNode(yyvsp[-6].node, yyvsp[-4].node, yyvsp[-2].node, yyvsp[0].stat); ;
    break;}
case 136:
#line 415 "grammar.y"
{ yyval.stat = new ForNode(yyvsp[-6].vlist, yyvsp[-4].node, yyvsp[-2].node, yyvsp[0].stat); ;
    break;}
case 137:
#line 417 "grammar.y"
{ yyval.stat = new ForInNode(yyvsp[-4].node, yyvsp[-2].node, yyvsp[0].stat); ;
    break;}
case 138:
#line 419 "grammar.y"
{ yyval.stat = new ForInNode(yyvsp[-4].ustr, 0L, yyvsp[-2].node, yyvsp[0].stat);
                                     delete yyvsp[-4].ustr; ;
    break;}
case 139:
#line 422 "grammar.y"
{ yyval.stat = new ForInNode(yyvsp[-5].ustr, yyvsp[-4].init, yyvsp[-2].node, yyvsp[0].stat);
                                     delete yyvsp[-5].ustr; ;
    break;}
case 140:
#line 427 "grammar.y"
{ yyval.node = 0L; ;
    break;}
case 142:
#line 432 "grammar.y"
{ yyval.stat = new ContinueNode(); ;
    break;}
case 143:
#line 433 "grammar.y"
{ yyval.stat = new ContinueNode(yyvsp[-1].ustr); delete yyvsp[-1].ustr; ;
    break;}
case 144:
#line 437 "grammar.y"
{ yyval.stat = new BreakNode(); ;
    break;}
case 145:
#line 438 "grammar.y"
{ yyval.stat = new BreakNode(yyvsp[-1].ustr); delete yyvsp[-1].ustr; ;
    break;}
case 146:
#line 442 "grammar.y"
{ yyval.stat = new ReturnNode(0L); ;
    break;}
case 147:
#line 443 "grammar.y"
{ yyval.stat = new ReturnNode(yyvsp[-1].node); ;
    break;}
case 148:
#line 447 "grammar.y"
{ yyval.stat = new WithNode(yyvsp[-2].node, yyvsp[0].stat); ;
    break;}
case 149:
#line 451 "grammar.y"
{ yyval.stat = new SwitchNode(yyvsp[-2].node, yyvsp[0].cblk); ;
    break;}
case 150:
#line 455 "grammar.y"
{ yyval.cblk = new CaseBlockNode(yyvsp[-1].clist, 0L, 0L); ;
    break;}
case 151:
#line 457 "grammar.y"
{ yyval.cblk = new CaseBlockNode(yyvsp[-3].clist, yyvsp[-2].ccl, yyvsp[-1].clist); ;
    break;}
case 152:
#line 461 "grammar.y"
{ yyval.clist = 0L; ;
    break;}
case 154:
#line 466 "grammar.y"
{ yyval.clist = new ClauseListNode(yyvsp[0].ccl); ;
    break;}
case 155:
#line 467 "grammar.y"
{ yyval.clist = yyvsp[-1].clist->append(yyvsp[0].ccl); ;
    break;}
case 156:
#line 471 "grammar.y"
{ yyval.ccl = new CaseClauseNode(yyvsp[-1].node, 0L); ;
    break;}
case 157:
#line 472 "grammar.y"
{ yyval.ccl = new CaseClauseNode(yyvsp[-2].node, yyvsp[0].slist); ;
    break;}
case 158:
#line 476 "grammar.y"
{ yyval.ccl = new CaseClauseNode(0L, 0L);; ;
    break;}
case 159:
#line 477 "grammar.y"
{ yyval.ccl = new CaseClauseNode(0L, yyvsp[0].slist); ;
    break;}
case 160:
#line 481 "grammar.y"
{ yyval.stat = new LabelNode(yyvsp[-2].ustr, yyvsp[0].stat); delete yyvsp[-2].ustr; ;
    break;}
case 161:
#line 485 "grammar.y"
{ yyval.stat = new ThrowNode(0L); ;
    break;}
case 162:
#line 486 "grammar.y"
{ yyval.stat = new ThrowNode(yyvsp[-1].node); ;
    break;}
case 163:
#line 490 "grammar.y"
{ yyval.stat = new TryNode(); /* TODO */ ;
    break;}
case 164:
#line 491 "grammar.y"
{ yyval.stat = new TryNode(); /* TODO */ ;
    break;}
case 165:
#line 492 "grammar.y"
{ yyval.stat = new TryNode(); /* TODO */ ;
    break;}
case 166:
#line 496 "grammar.y"
{ yyval.node = new CatchNode(); /* TODO */ ;
    break;}
case 167:
#line 500 "grammar.y"
{ yyval.node = new FinallyNode(yyvsp[0].stat); ;
    break;}
case 168:
#line 504 "grammar.y"
{ yyval.func = new FuncDeclNode(yyvsp[-3].ustr, 0L, yyvsp[0].stat);
                                     delete yyvsp[-3].ustr; ;
    break;}
case 169:
#line 507 "grammar.y"
{ yyval.func = new FuncDeclNode(yyvsp[-4].ustr, yyvsp[-2].param, yyvsp[0].stat);
                                     delete yyvsp[-4].ustr; ;
    break;}
case 170:
#line 512 "grammar.y"
{ yyval.param = new ParameterNode(yyvsp[0].ustr); delete yyvsp[0].ustr; ;
    break;}
case 171:
#line 513 "grammar.y"
{ yyval.param = new ParameterNode(yyvsp[-2].param, yyvsp[0].ustr);
	                             delete yyvsp[0].ustr; ;
    break;}
case 172:
#line 518 "grammar.y"
{ yyval.prog = new ProgramNode(yyvsp[0].srcs); ;
    break;}
case 173:
#line 522 "grammar.y"
{ yyval.srcs = new SourceElementsNode(yyvsp[0].src); ;
    break;}
case 174:
#line 523 "grammar.y"
{ yyval.srcs = new SourceElementsNode(yyvsp[-1].srcs, yyvsp[0].src); ;
    break;}
case 175:
#line 527 "grammar.y"
{ yyval.src = new SourceElementNode(yyvsp[0].stat); ;
    break;}
case 176:
#line 528 "grammar.y"
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
#line 531 "grammar.y"


int yyerror (const char *)  /* Called by yyparse on error */
{
//  fprintf(stderr, "ERROR: %s at line %d\n",
//	  s, KJScript::lexer()->lineNo());
  return 1;
}

/* may we automatically insert a semicolon ? */
bool automatic()
{
  if (yychar == '}')
    return true;
  else if (Lexer::curr()->prevTerminator())
    return true;

  return false;
}
