
/*  A Bison parser, made from grammar.y
 by  GNU Bison version 1.25
  */

#define YYBISON 1  /* Identify Bison output.  */

#define	LF	258
#define	NULLTOKEN	259
#define	TRUETOKEN	260
#define	FALSETOKEN	261
#define	STRING	262
#define	DECIMAL	263
#define	INTEGER	264
#define	BREAK	265
#define	FOR	266
#define	NEW	267
#define	VAR	268
#define	CONTINUE	269
#define	FUNCTION	270
#define	RETURN	271
#define	VOID	272
#define	DELETE	273
#define	IF	274
#define	THIS	275
#define	WHILE	276
#define	ELSE	277
#define	IN	278
#define	TYPEOF	279
#define	WITH	280
#define	RESERVED	281
#define	PRINT	282
#define	ALERT	283
#define	EQEQ	284
#define	NE	285
#define	LE	286
#define	GE	287
#define	OR	288
#define	AND	289
#define	PLUSPLUS	290
#define	MINUSMINUS	291
#define	LSHIFT	292
#define	RSHIFT	293
#define	URSHIFT	294
#define	PLUSEQUAL	295
#define	MINUSEQUAL	296
#define	MULTEQUAL	297
#define	DIVEQUAL	298
#define	LSHIFTEQUAL	299
#define	RSHIFTEQUAL	300
#define	URSHIFTEQUAL	301
#define	ANDEQUAL	302
#define	MODEQUAL	303
#define	XOREQUAL	304
#define	OREQUAL	305
#define	DOUBLE	306
#define	IDENT	307

#line 1 "grammar.y"


#include "global.h"
#include "kjs.h"
#include "nodes.h"
#include "lexer.h"

extern int yylex();
int yyerror (char *);

using namespace KJS;


#line 15 "grammar.y"
typedef union {
  int                 ival;
  double              dval;
  CString             *cstr;
  UString             *ustr;
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
  Operator            op;
} YYSTYPE;
#include <stdio.h>

#ifndef __cplusplus
#ifndef __STDC__
#define const
#endif
#endif



#define	YYFINAL		247
#define	YYFLAG		-32768
#define	YYNTBASE	77

#define YYTRANSLATE(x) ((unsigned)(x) <= 307 ? yytranslate[x] : 122)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,    62,     2,     2,     2,    65,    68,     2,    53,
    54,    63,    59,    58,    60,    57,    64,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,    72,    76,    66,
    73,    67,    71,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
    55,     2,    56,    69,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,    74,    70,    75,    61,     2,     2,     2,     2,
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
    46,    47,    48,    49,    50,    51,    52
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     2,     4,     6,     8,    10,    12,    14,    16,    18,
    22,    24,    29,    33,    37,    39,    42,    45,    48,    53,
    57,    60,    64,    66,    70,    72,    74,    76,    79,    82,
    84,    87,    90,    93,    96,    99,   102,   105,   108,   111,
   113,   117,   121,   125,   127,   131,   135,   137,   141,   145,
   149,   151,   155,   159,   163,   167,   169,   173,   177,   179,
   183,   185,   189,   191,   195,   197,   201,   203,   207,   209,
   215,   217,   221,   223,   225,   227,   229,   231,   233,   235,
   237,   239,   241,   243,   245,   247,   251,   253,   255,   257,
   259,   261,   263,   265,   267,   269,   271,   276,   281,   284,
   288,   290,   293,   297,   299,   303,   305,   308,   311,   313,
   316,   322,   330,   336,   346,   357,   365,   374,   384,   385,
   387,   390,   393,   396,   400,   406,   412,   419,   421,   425,
   427,   429,   432,   434
};

static const short yyrhs[] = {     4,
     0,     5,     0,     6,     0,     9,     0,    51,     0,     7,
     0,    20,     0,    52,     0,    77,     0,    53,   100,    54,
     0,    78,     0,    79,    55,   100,    56,     0,    79,    57,
    52,     0,    12,    79,    82,     0,    79,     0,    12,    80,
     0,    79,    82,     0,    81,    82,     0,    81,    55,   100,
    56,     0,    81,    57,    52,     0,    53,    54,     0,    53,
    83,    54,     0,    98,     0,    83,    58,    98,     0,    80,
     0,    81,     0,    84,     0,    84,    35,     0,    84,    36,
     0,    85,     0,    18,    86,     0,    17,    86,     0,    24,
    86,     0,    35,    86,     0,    36,    86,     0,    59,    86,
     0,    60,    86,     0,    61,    86,     0,    62,    86,     0,
    86,     0,    87,    63,    86,     0,    87,    64,    86,     0,
    87,    65,    86,     0,    87,     0,    88,    59,    87,     0,
    88,    60,    87,     0,    88,     0,    89,    37,    88,     0,
    89,    38,    88,     0,    89,    39,    88,     0,    89,     0,
    90,    66,    89,     0,    90,    67,    89,     0,    90,    31,
    89,     0,    90,    32,    89,     0,    90,     0,    91,    29,
    90,     0,    91,    30,    90,     0,    91,     0,    92,    68,
    91,     0,    92,     0,    93,    69,    91,     0,    93,     0,
    94,    70,    91,     0,    94,     0,    95,    34,    94,     0,
    95,     0,    96,    33,    92,     0,    96,     0,    96,    71,
    98,    72,    98,     0,    97,     0,    84,    99,    98,     0,
    73,     0,    40,     0,    41,     0,    42,     0,    43,     0,
    44,     0,    45,     0,    46,     0,    47,     0,    49,     0,
    50,     0,    48,     0,    98,     0,   100,    58,    98,     0,
   102,     0,   104,     0,   108,     0,   109,     0,   110,     0,
   111,     0,   113,     0,   114,     0,   115,     0,   116,     0,
    27,    53,   100,    54,     0,    28,    53,   100,    54,     0,
    74,    75,     0,    74,   103,    75,     0,   101,     0,   103,
   101,     0,    13,   105,    76,     0,   106,     0,   105,    58,
   106,     0,    52,     0,    52,   107,     0,    73,    98,     0,
    76,     0,   100,    76,     0,    19,    53,   100,    54,   101,
     0,    19,    53,   100,    54,   101,    22,   101,     0,    21,
    53,   100,    54,   101,     0,    11,    53,   112,    76,   112,
    76,   112,    54,   101,     0,    11,    53,    13,   105,    76,
   112,    76,   112,    54,   101,     0,    11,    53,    84,    23,
   100,    54,   101,     0,    11,    53,    13,    52,    23,   100,
    54,   101,     0,    11,    53,    13,    52,   107,    23,   100,
    54,   101,     0,     0,   100,     0,    14,    76,     0,    10,
    76,     0,    16,    76,     0,    16,   100,    76,     0,    25,
    53,   100,    54,   101,     0,    15,    52,    53,    54,   102,
     0,    15,    52,    53,   118,    54,   102,     0,    52,     0,
   118,    58,    52,     0,   120,     0,   121,     0,   120,   121,
     0,   101,     0,   117,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
   108,   110,   111,   112,   113,   114,   117,   119,   121,   122,
   125,   127,   128,   130,   133,   135,   138,   140,   141,   142,
   145,   147,   150,   152,   155,   157,   160,   162,   163,   166,
   168,   169,   170,   171,   172,   173,   174,   175,   176,   179,
   181,   182,   183,   186,   188,   189,   192,   194,   195,   196,
   199,   201,   203,   205,   207,   211,   213,   214,   217,   219,
   222,   224,   227,   229,   232,   234,   238,   240,   244,   246,
   250,   252,   256,   258,   259,   260,   261,   262,   263,   264,
   265,   266,   267,   268,   271,   273,   276,   278,   279,   280,
   281,   282,   283,   284,   285,   286,   287,   288,   291,   293,
   296,   298,   301,   305,   307,   311,   313,   316,   320,   324,
   328,   330,   334,   336,   338,   340,   342,   345,   350,   352,
   355,   359,   363,   365,   368,   372,   375,   380,   382,   385,
   390,   392,   395,   397
};
#endif


#if YYDEBUG != 0 || defined (YYERROR_VERBOSE)

static const char * const yytname[] = {   "$","error","$undefined.","LF","NULLTOKEN",
"TRUETOKEN","FALSETOKEN","STRING","DECIMAL","INTEGER","BREAK","FOR","NEW","VAR",
"CONTINUE","FUNCTION","RETURN","VOID","DELETE","IF","THIS","WHILE","ELSE","IN",
"TYPEOF","WITH","RESERVED","PRINT","ALERT","EQEQ","NE","LE","GE","OR","AND",
"PLUSPLUS","MINUSMINUS","LSHIFT","RSHIFT","URSHIFT","PLUSEQUAL","MINUSEQUAL",
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
"FunctionDeclaration","FormalParameterList","Program","SourceElements","SourceElement", NULL
};
#endif

static const short yyr1[] = {     0,
    77,    77,    77,    77,    77,    77,    78,    78,    78,    78,
    79,    79,    79,    79,    80,    80,    81,    81,    81,    81,
    82,    82,    83,    83,    84,    84,    85,    85,    85,    86,
    86,    86,    86,    86,    86,    86,    86,    86,    86,    87,
    87,    87,    87,    88,    88,    88,    89,    89,    89,    89,
    90,    90,    90,    90,    90,    91,    91,    91,    92,    92,
    93,    93,    94,    94,    95,    95,    96,    96,    97,    97,
    98,    98,    99,    99,    99,    99,    99,    99,    99,    99,
    99,    99,    99,    99,   100,   100,   101,   101,   101,   101,
   101,   101,   101,   101,   101,   101,   101,   101,   102,   102,
   103,   103,   104,   105,   105,   106,   106,   107,   108,   109,
   110,   110,   111,   111,   111,   111,   111,   111,   112,   112,
   113,   114,   115,   115,   116,   117,   117,   118,   118,   119,
   120,   120,   121,   121
};

static const short yyr2[] = {     0,
     1,     1,     1,     1,     1,     1,     1,     1,     1,     3,
     1,     4,     3,     3,     1,     2,     2,     2,     4,     3,
     2,     3,     1,     3,     1,     1,     1,     2,     2,     1,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     1,
     3,     3,     3,     1,     3,     3,     1,     3,     3,     3,
     1,     3,     3,     3,     3,     1,     3,     3,     1,     3,
     1,     3,     1,     3,     1,     3,     1,     3,     1,     5,
     1,     3,     1,     1,     1,     1,     1,     1,     1,     1,
     1,     1,     1,     1,     1,     3,     1,     1,     1,     1,
     1,     1,     1,     1,     1,     1,     4,     4,     2,     3,
     1,     2,     3,     1,     3,     1,     2,     2,     1,     2,
     5,     7,     5,     9,    10,     7,     8,     9,     0,     1,
     2,     2,     2,     3,     5,     5,     6,     1,     3,     1,
     1,     2,     1,     1
};

static const short yydefact[] = {     0,
     1,     2,     3,     6,     4,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     7,     0,     0,     0,     0,
     0,     0,     0,     5,     8,     0,     0,     0,     0,     0,
     0,   109,     9,    11,    15,    25,    26,    27,    30,    40,
    44,    47,    51,    56,    59,    61,    63,    65,    67,    69,
    71,    85,     0,   133,    87,    88,    89,    90,    91,    92,
    93,    94,    95,    96,   134,   130,   131,   122,   119,    15,
    16,   106,     0,   104,   121,     0,   123,     0,    27,    32,
    31,     0,     0,    33,     0,     0,     0,    34,    35,     0,
    36,    37,    38,    39,    99,   101,     0,     0,     0,     0,
    17,     0,     0,    18,    28,    29,    74,    75,    76,    77,
    78,    79,    80,    81,    84,    82,    83,    73,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
   110,   132,     0,    27,   120,     0,    14,     0,   107,     0,
   103,     0,   124,     0,     0,     0,     0,     0,    10,   100,
   102,    21,     0,    23,     0,    13,     0,    20,    72,    41,
    42,    43,    45,    46,    48,    49,    50,    54,    55,    52,
    53,    57,    58,    60,    62,    64,    66,    68,     0,    86,
   106,     0,     0,   119,   108,   105,   128,     0,     0,     0,
     0,     0,    97,    98,    22,     0,    12,    19,     0,     0,
   107,   119,     0,     0,   126,     0,     0,   111,   113,   125,
    24,    70,     0,     0,     0,     0,   119,   127,   129,     0,
     0,     0,   119,   116,     0,   112,   117,     0,     0,     0,
   118,     0,   114,   115,     0,     0,     0
};

static const short yydefgoto[] = {    33,
    34,    35,    36,    37,   101,   163,    38,    39,    40,    41,
    42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
    52,   119,    53,    54,    55,    97,    56,    73,    74,   149,
    57,    58,    59,    60,   146,    61,    62,    63,    64,    65,
   199,   245,    66,    67
};

static const short yypact[] = {   213,
-32768,-32768,-32768,-32768,-32768,   -69,   -17,   136,   -31,   -44,
    14,   492,   593,   593,    33,-32768,    81,   593,    91,    96,
   139,   593,   593,-32768,-32768,   593,   593,   593,   593,   593,
   286,-32768,-32768,-32768,    -3,-32768,    12,   117,-32768,-32768,
    44,    -4,   131,    -6,   125,   -19,   130,    13,   162,   -18,
-32768,-32768,   -52,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,   213,-32768,-32768,   513,    -3,
-32768,   134,   -48,-32768,-32768,   155,-32768,   -47,   159,-32768,
-32768,   593,   593,-32768,   593,   593,   593,-32768,-32768,   -21,
-32768,-32768,-32768,-32768,-32768,-32768,   359,   572,   593,   158,
-32768,   593,   164,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,   593,   593,
   593,   593,   593,   593,   593,   593,   593,   593,   593,   593,
   593,   593,   593,   593,   593,   593,   593,   593,   593,   593,
-32768,-32768,   183,    56,   156,   160,-32768,   593,-32768,   -31,
-32768,   132,-32768,     4,    22,    23,    24,    30,-32768,-32768,
-32768,-32768,    31,-32768,   127,-32768,   135,-32768,-32768,-32768,
-32768,-32768,    44,    44,    -4,    -4,    -4,   131,   131,   131,
   131,    -6,    -6,   125,   125,   125,    13,   -19,   167,-32768,
   -22,   -42,   593,   593,-32768,-32768,-32768,   168,    77,   432,
   432,   432,-32768,-32768,-32768,   593,-32768,-32768,   593,   593,
   220,   593,    78,   169,-32768,   168,   192,   224,-32768,-32768,
-32768,-32768,    92,   593,   171,   432,   593,-32768,-32768,   432,
   432,    93,   593,-32768,   196,-32768,-32768,   432,   197,   432,
-32768,   432,-32768,-32768,   252,   253,-32768
};

static const short yypgoto[] = {-32768,
-32768,   246,   247,-32768,   -32,-32768,   -10,-32768,    17,    80,
    53,    46,    73,   -41,   118,-32768,   120,-32768,-32768,-32768,
   -76,-32768,   -12,   -29,  -175,-32768,-32768,   115,   109,    69,
-32768,-32768,-32768,-32768,  -185,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,   195
};


#define	YYLAST		655


static const short yytable[] = {    78,
   210,    96,    79,    79,   104,   140,    68,    79,   214,   150,
   140,    79,    79,    90,   138,   150,    79,    79,    79,    79,
    72,   164,   215,   141,   128,   129,   225,   151,   153,    80,
    81,    75,   159,   212,    84,    69,   140,   147,    88,    89,
   228,   235,   169,    91,    92,    93,    94,   239,   134,    98,
   148,    99,   139,   100,   123,   124,   145,   200,   144,   130,
   131,   140,   189,   190,    98,    76,   102,   161,   103,   154,
   155,   195,   156,   157,   158,   201,   202,   203,   193,   140,
   140,   140,   136,   204,   205,    82,   165,   140,   206,   167,
   105,   106,   184,   185,   186,   107,   108,   109,   110,   111,
   112,   113,   114,   115,   116,   117,   120,   121,   122,    79,
    79,    79,    79,    79,    79,    79,    79,    79,    79,    79,
    79,    79,    79,    79,    79,    79,    79,    79,   118,   221,
   216,   226,   222,    83,   217,   140,   170,   171,   172,     1,
     2,     3,     4,    85,     5,   231,   238,     8,    86,   140,
   140,   105,   106,   132,   133,    16,   107,   108,   109,   110,
   111,   112,   113,   114,   115,   116,   117,   125,   126,   127,
   218,   219,   220,   178,   179,   180,   181,   175,   176,   177,
   213,   145,   207,   197,   140,   198,    24,    25,    26,   118,
   208,    87,   140,   105,   106,   137,   234,   223,   135,   145,
   236,   237,   173,   174,   182,   183,   148,   152,   241,   166,
   243,   232,   244,   140,   145,   168,     1,     2,     3,     4,
   145,     5,     6,     7,     8,     9,    10,    11,    12,    13,
    14,    15,    16,    17,   191,   194,    18,    19,   209,    20,
    21,    31,   224,   229,   227,   230,   233,    22,    23,   240,
   242,   246,   247,    70,    71,   188,   187,   192,   196,   211,
   142,     0,     0,    24,    25,    26,     0,     0,     0,     0,
     0,    27,    28,    29,    30,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,    31,     0,    32,     1,
     2,     3,     4,     0,     5,     6,     7,     8,     9,    10,
     0,    12,    13,    14,    15,    16,    17,     0,     0,    18,
    19,     0,    20,    21,     0,     0,     0,     0,     0,     0,
    22,    23,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,    24,    25,    26,     0,
     0,     0,     0,     0,    27,    28,    29,    30,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,    31,
    95,    32,     1,     2,     3,     4,     0,     5,     6,     7,
     8,     9,    10,     0,    12,    13,    14,    15,    16,    17,
     0,     0,    18,    19,     0,    20,    21,     0,     0,     0,
     0,     0,     0,    22,    23,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,    24,
    25,    26,     0,     0,     0,     0,     0,    27,    28,    29,
    30,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,    31,   160,    32,     1,     2,     3,     4,     0,
     5,     6,     7,     8,     9,    10,     0,    12,    13,    14,
    15,    16,    17,     0,     0,    18,    19,     0,    20,    21,
     0,     0,     0,     0,     0,     0,    22,    23,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,    24,    25,    26,     0,     0,     0,     0,     0,
    27,    28,    29,    30,     0,     1,     2,     3,     4,     0,
     5,     0,     0,     8,     0,    31,     0,    32,    13,    14,
     0,    16,     0,     0,     0,    18,     1,     2,     3,     4,
     0,     5,     0,     0,     8,   143,    22,    23,     0,    13,
    14,     0,    16,     0,     0,     0,    18,     0,     0,     0,
     0,     0,    24,    25,    26,     0,     0,    22,    23,     0,
    27,    28,    29,    30,     0,     0,     0,     0,     0,     0,
     0,     0,     0,    24,    25,    26,     0,    77,     0,     0,
     0,    27,    28,    29,    30,     1,     2,     3,     4,     0,
     5,     0,     0,     8,     0,     0,     0,     0,    13,    14,
     0,    16,     0,     0,     0,    18,     1,     2,     3,     4,
     0,     5,     0,     0,     8,     0,    22,    23,     0,    13,
    14,     0,    16,     0,     0,     0,    18,     0,     0,     0,
     0,     0,    24,    25,    26,   162,     0,    22,    23,     0,
    27,    28,    29,    30,     0,     0,     0,     0,     0,     0,
     0,     0,     0,    24,    25,    26,     0,     0,     0,     0,
     0,    27,    28,    29,    30
};

static const short yycheck[] = {    12,
    23,    31,    13,    14,    37,    58,    76,    18,   194,    58,
    58,    22,    23,    26,    33,    58,    27,    28,    29,    30,
    52,    98,   198,    76,    31,    32,   212,    76,    76,    13,
    14,    76,    54,    76,    18,    53,    58,    70,    22,    23,
   216,   227,   119,    27,    28,    29,    30,   233,    68,    53,
    73,    55,    71,    57,    59,    60,    69,    54,    69,    66,
    67,    58,   139,   140,    53,    52,    55,    97,    57,    82,
    83,   148,    85,    86,    87,    54,    54,    54,    23,    58,
    58,    58,    70,    54,    54,    53,    99,    58,    58,   102,
    35,    36,   134,   135,   136,    40,    41,    42,    43,    44,
    45,    46,    47,    48,    49,    50,    63,    64,    65,   120,
   121,   122,   123,   124,   125,   126,   127,   128,   129,   130,
   131,   132,   133,   134,   135,   136,   137,   138,    73,   206,
    54,    54,   209,    53,    58,    58,   120,   121,   122,     4,
     5,     6,     7,    53,     9,    54,    54,    12,    53,    58,
    58,    35,    36,    29,    30,    20,    40,    41,    42,    43,
    44,    45,    46,    47,    48,    49,    50,    37,    38,    39,
   200,   201,   202,   128,   129,   130,   131,   125,   126,   127,
   193,   194,    56,    52,    58,    54,    51,    52,    53,    73,
    56,    53,    58,    35,    36,    34,   226,   210,    69,   212,
   230,   231,   123,   124,   132,   133,    73,    53,   238,    52,
   240,   224,   242,    58,   227,    52,     4,     5,     6,     7,
   233,     9,    10,    11,    12,    13,    14,    15,    16,    17,
    18,    19,    20,    21,    52,    76,    24,    25,    72,    27,
    28,    74,    23,    52,    76,    22,    76,    35,    36,    54,
    54,     0,     0,     8,     8,   138,   137,   143,   150,   191,
    66,    -1,    -1,    51,    52,    53,    -1,    -1,    -1,    -1,
    -1,    59,    60,    61,    62,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    74,    -1,    76,     4,
     5,     6,     7,    -1,     9,    10,    11,    12,    13,    14,
    -1,    16,    17,    18,    19,    20,    21,    -1,    -1,    24,
    25,    -1,    27,    28,    -1,    -1,    -1,    -1,    -1,    -1,
    35,    36,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    51,    52,    53,    -1,
    -1,    -1,    -1,    -1,    59,    60,    61,    62,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    74,
    75,    76,     4,     5,     6,     7,    -1,     9,    10,    11,
    12,    13,    14,    -1,    16,    17,    18,    19,    20,    21,
    -1,    -1,    24,    25,    -1,    27,    28,    -1,    -1,    -1,
    -1,    -1,    -1,    35,    36,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    51,
    52,    53,    -1,    -1,    -1,    -1,    -1,    59,    60,    61,
    62,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    74,    75,    76,     4,     5,     6,     7,    -1,
     9,    10,    11,    12,    13,    14,    -1,    16,    17,    18,
    19,    20,    21,    -1,    -1,    24,    25,    -1,    27,    28,
    -1,    -1,    -1,    -1,    -1,    -1,    35,    36,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    51,    52,    53,    -1,    -1,    -1,    -1,    -1,
    59,    60,    61,    62,    -1,     4,     5,     6,     7,    -1,
     9,    -1,    -1,    12,    -1,    74,    -1,    76,    17,    18,
    -1,    20,    -1,    -1,    -1,    24,     4,     5,     6,     7,
    -1,     9,    -1,    -1,    12,    13,    35,    36,    -1,    17,
    18,    -1,    20,    -1,    -1,    -1,    24,    -1,    -1,    -1,
    -1,    -1,    51,    52,    53,    -1,    -1,    35,    36,    -1,
    59,    60,    61,    62,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    51,    52,    53,    -1,    76,    -1,    -1,
    -1,    59,    60,    61,    62,     4,     5,     6,     7,    -1,
     9,    -1,    -1,    12,    -1,    -1,    -1,    -1,    17,    18,
    -1,    20,    -1,    -1,    -1,    24,     4,     5,     6,     7,
    -1,     9,    -1,    -1,    12,    -1,    35,    36,    -1,    17,
    18,    -1,    20,    -1,    -1,    -1,    24,    -1,    -1,    -1,
    -1,    -1,    51,    52,    53,    54,    -1,    35,    36,    -1,
    59,    60,    61,    62,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    51,    52,    53,    -1,    -1,    -1,    -1,
    -1,    59,    60,    61,    62
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
#line 109 "grammar.y"
{ yyval.node = new NullNode(); ;
    break;}
case 2:
#line 110 "grammar.y"
{ yyval.node = new BooleanNode(true); ;
    break;}
case 3:
#line 111 "grammar.y"
{ yyval.node = new BooleanNode(false); ;
    break;}
case 4:
#line 112 "grammar.y"
{ yyval.node = new NumberNode(yyvsp[0].ival); ;
    break;}
case 5:
#line 113 "grammar.y"
{ yyval.node = new NumberNode(yyvsp[0].dval); ;
    break;}
case 6:
#line 114 "grammar.y"
{ yyval.node = new StringNode(yyvsp[0].ustr); delete yyvsp[0].ustr; ;
    break;}
case 7:
#line 118 "grammar.y"
{ yyval.node = new ThisNode(); ;
    break;}
case 8:
#line 119 "grammar.y"
{ yyval.node = new ResolveNode(yyvsp[0].cstr);
                                     delete yyvsp[0].cstr; ;
    break;}
case 10:
#line 122 "grammar.y"
{ yyval.node = new GroupNode(yyvsp[-1].node); ;
    break;}
case 12:
#line 127 "grammar.y"
{ yyval.node = new AccessorNode1(yyvsp[-3].node, yyvsp[-1].node); ;
    break;}
case 13:
#line 128 "grammar.y"
{ yyval.node = new AccessorNode2(yyvsp[-2].node, yyvsp[0].cstr);
                                     delete yyvsp[0].cstr; ;
    break;}
case 14:
#line 130 "grammar.y"
{ yyval.node = new NewExprNode(yyvsp[-1].node, yyvsp[0].args); ;
    break;}
case 16:
#line 135 "grammar.y"
{ yyval.node = new NewExprNode(yyvsp[0].node); ;
    break;}
case 17:
#line 139 "grammar.y"
{ yyval.node = new FunctionCallNode(yyvsp[-1].node, yyvsp[0].args); ;
    break;}
case 18:
#line 140 "grammar.y"
{ yyval.node = new FunctionCallNode(yyvsp[-1].node, yyvsp[0].args); ;
    break;}
case 19:
#line 141 "grammar.y"
{ yyval.node = new AccessorNode1(yyvsp[-3].node, yyvsp[-1].node); ;
    break;}
case 20:
#line 142 "grammar.y"
{ yyval.node = new AccessorNode2(yyvsp[-2].node, yyvsp[0].cstr); ;
    break;}
case 21:
#line 146 "grammar.y"
{ yyval.args = new ArgumentsNode(0L); ;
    break;}
case 22:
#line 147 "grammar.y"
{ yyval.args = new ArgumentsNode(yyvsp[-1].alist); ;
    break;}
case 23:
#line 151 "grammar.y"
{ yyval.alist = new ArgumentListNode(yyvsp[0].node); ;
    break;}
case 24:
#line 152 "grammar.y"
{ yyval.alist = new ArgumentListNode(yyvsp[-2].alist, yyvsp[0].node); ;
    break;}
case 28:
#line 162 "grammar.y"
{ yyval.node = new PostfixNode(yyvsp[-1].node, OpPlusPlus); ;
    break;}
case 29:
#line 163 "grammar.y"
{ yyval.node = new PostfixNode(yyvsp[-1].node, OpMinusMinus); ;
    break;}
case 31:
#line 168 "grammar.y"
{ yyval.node = new DeleteNode(yyvsp[0].node); ;
    break;}
case 32:
#line 169 "grammar.y"
{ yyval.node = new VoidNode(yyvsp[0].node); ;
    break;}
case 33:
#line 170 "grammar.y"
{ yyval.node = new TypeOfNode(yyvsp[0].node); ;
    break;}
case 34:
#line 171 "grammar.y"
{ yyval.node = new PrefixNode(OpPlusPlus, yyvsp[0].node); ;
    break;}
case 35:
#line 172 "grammar.y"
{ yyval.node = new PrefixNode(OpMinusMinus, yyvsp[0].node); ;
    break;}
case 36:
#line 173 "grammar.y"
{ yyval.node = new UnaryPlusNode(yyvsp[0].node); ;
    break;}
case 37:
#line 174 "grammar.y"
{ yyval.node = new NegateNode(yyvsp[0].node); ;
    break;}
case 38:
#line 175 "grammar.y"
{ yyval.node = new BitwiseNotNode(yyvsp[0].node); ;
    break;}
case 39:
#line 176 "grammar.y"
{ yyval.node = new LogicalNotNode(yyvsp[0].node); ;
    break;}
case 41:
#line 181 "grammar.y"
{ yyval.node = new MultNode(yyvsp[-2].node, yyvsp[0].node, '*'); ;
    break;}
case 42:
#line 182 "grammar.y"
{ yyval.node = new MultNode(yyvsp[-2].node, yyvsp[0].node, '/'); ;
    break;}
case 43:
#line 183 "grammar.y"
{ yyval.node = new MultNode(yyvsp[-2].node,yyvsp[0].node,'%'); ;
    break;}
case 45:
#line 188 "grammar.y"
{ yyval.node = new AddNode(yyvsp[-2].node, yyvsp[0].node, '+'); ;
    break;}
case 46:
#line 189 "grammar.y"
{ yyval.node = new AddNode(yyvsp[-2].node, yyvsp[0].node, '-'); ;
    break;}
case 48:
#line 194 "grammar.y"
{ yyval.node = new ShiftNode(yyvsp[-2].node, OpLShift, yyvsp[0].node); ;
    break;}
case 49:
#line 195 "grammar.y"
{ yyval.node = new ShiftNode(yyvsp[-2].node, OpRShift, yyvsp[0].node); ;
    break;}
case 50:
#line 196 "grammar.y"
{ yyval.node = new ShiftNode(yyvsp[-2].node, OpURShift, yyvsp[0].node); ;
    break;}
case 52:
#line 202 "grammar.y"
{ yyval.node = new RelationalNode(yyvsp[-2].node, OpLess, yyvsp[0].node); ;
    break;}
case 53:
#line 204 "grammar.y"
{ yyval.node = new RelationalNode(yyvsp[-2].node, OpGreater, yyvsp[0].node); ;
    break;}
case 54:
#line 206 "grammar.y"
{ yyval.node = new RelationalNode(yyvsp[-2].node, OpLessEq, yyvsp[0].node); ;
    break;}
case 55:
#line 208 "grammar.y"
{ yyval.node = new RelationalNode(yyvsp[-2].node, OpGreaterEq, yyvsp[0].node); ;
    break;}
case 57:
#line 213 "grammar.y"
{ yyval.node = new EqualNode(yyvsp[-2].node, OpEqEq, yyvsp[0].node); ;
    break;}
case 58:
#line 214 "grammar.y"
{ yyval.node = new EqualNode(yyvsp[-2].node, OpNotEq, yyvsp[0].node); ;
    break;}
case 60:
#line 219 "grammar.y"
{ yyval.node = new BitOperNode(yyvsp[-2].node, OpBitAnd, yyvsp[0].node); ;
    break;}
case 62:
#line 224 "grammar.y"
{ yyval.node = new BitOperNode(yyvsp[-2].node, OpBitXOr, yyvsp[0].node); ;
    break;}
case 64:
#line 229 "grammar.y"
{ yyval.node = new BitOperNode(yyvsp[-2].node, OpBitOr, yyvsp[0].node); ;
    break;}
case 66:
#line 235 "grammar.y"
{ yyval.node = new BinaryLogicalNode(yyvsp[-2].node, OpAnd, yyvsp[0].node); ;
    break;}
case 68:
#line 241 "grammar.y"
{ yyval.node = new BinaryLogicalNode(yyvsp[-2].node, OpOr, yyvsp[0].node); ;
    break;}
case 70:
#line 247 "grammar.y"
{ yyval.node = new ConditionalNode(yyvsp[-4].node, yyvsp[-2].node, yyvsp[0].node); ;
    break;}
case 72:
#line 253 "grammar.y"
{ yyval.node = new AssignNode(yyvsp[-2].node, yyvsp[-1].op, yyvsp[0].node);;
    break;}
case 73:
#line 257 "grammar.y"
{ yyval.op = OpEqual; ;
    break;}
case 74:
#line 258 "grammar.y"
{ yyval.op = OpPlusEq; ;
    break;}
case 75:
#line 259 "grammar.y"
{ yyval.op = OpMinusEq; ;
    break;}
case 76:
#line 260 "grammar.y"
{ yyval.op = OpMultEq; ;
    break;}
case 77:
#line 261 "grammar.y"
{ yyval.op = OpDivEq; ;
    break;}
case 78:
#line 262 "grammar.y"
{ yyval.op = OpLShift; ;
    break;}
case 79:
#line 263 "grammar.y"
{ yyval.op = OpRShift; ;
    break;}
case 80:
#line 264 "grammar.y"
{ yyval.op = OpURShift; ;
    break;}
case 81:
#line 265 "grammar.y"
{ yyval.op = OpAndEq; ;
    break;}
case 82:
#line 266 "grammar.y"
{ yyval.op = OpXOrEq; ;
    break;}
case 83:
#line 267 "grammar.y"
{ yyval.op = OpOrEq; ;
    break;}
case 84:
#line 268 "grammar.y"
{ yyval.op = OpModEq; ;
    break;}
case 86:
#line 273 "grammar.y"
{ yyval.node = new CommaNode(yyvsp[-2].node, yyvsp[0].node); ;
    break;}
case 97:
#line 287 "grammar.y"
{ yyval.stat = new PrintNode(yyvsp[-1].node); ;
    break;}
case 98:
#line 288 "grammar.y"
{ yyval.stat = new AlertNode(yyvsp[-1].node); ;
    break;}
case 99:
#line 292 "grammar.y"
{ yyval.stat = new BlockNode(0L); ;
    break;}
case 100:
#line 293 "grammar.y"
{ yyval.stat = new BlockNode(yyvsp[-1].slist); ;
    break;}
case 101:
#line 297 "grammar.y"
{ yyval.slist = new StatListNode(yyvsp[0].stat); ;
    break;}
case 102:
#line 298 "grammar.y"
{ yyval.slist = new StatListNode(yyvsp[-1].slist, yyvsp[0].stat); ;
    break;}
case 103:
#line 302 "grammar.y"
{ yyval.stat = new VarStatementNode(yyvsp[-1].vlist); ;
    break;}
case 104:
#line 306 "grammar.y"
{ yyval.vlist = new VarDeclListNode(yyvsp[0].decl); ;
    break;}
case 105:
#line 308 "grammar.y"
{ yyval.vlist = new VarDeclListNode(yyvsp[-2].vlist, yyvsp[0].decl); ;
    break;}
case 106:
#line 312 "grammar.y"
{ yyval.decl = new VarDeclNode(yyvsp[0].cstr); delete yyvsp[0].cstr; ;
    break;}
case 107:
#line 313 "grammar.y"
{ yyval.decl = new VarDeclNode(yyvsp[-1].cstr, yyvsp[0].init); delete yyvsp[-1].cstr; ;
    break;}
case 108:
#line 317 "grammar.y"
{ yyval.init = new AssignExprNode(yyvsp[0].node); ;
    break;}
case 109:
#line 321 "grammar.y"
{ yyval.stat = new EmptyStatementNode(); ;
    break;}
case 110:
#line 325 "grammar.y"
{ yyval.stat = new ExprStatementNode(yyvsp[-1].node); ;
    break;}
case 111:
#line 329 "grammar.y"
{ yyval.stat = new IfNode(yyvsp[-2].node, yyvsp[0].stat, 0L); ;
    break;}
case 112:
#line 331 "grammar.y"
{ yyval.stat = new IfNode(yyvsp[-4].node, yyvsp[-2].stat, yyvsp[0].stat); ;
    break;}
case 113:
#line 335 "grammar.y"
{ yyval.stat = new WhileNode(yyvsp[-2].node, yyvsp[0].stat); ;
    break;}
case 114:
#line 337 "grammar.y"
{ yyval.stat = new ForNode(yyvsp[-6].node, yyvsp[-4].node, yyvsp[-2].node, yyvsp[0].stat); ;
    break;}
case 115:
#line 339 "grammar.y"
{ yyval.stat = new ForNode(yyvsp[-6].vlist, yyvsp[-4].node, yyvsp[-2].node, yyvsp[0].stat); ;
    break;}
case 116:
#line 341 "grammar.y"
{ yyval.stat = new ForNode(yyvsp[-4].node, yyvsp[-2].node, yyvsp[0].stat); ;
    break;}
case 117:
#line 343 "grammar.y"
{ yyval.stat = new ForNode(yyvsp[-4].cstr, 0L, yyvsp[-2].node, yyvsp[0].stat);
                                     delete yyvsp[-4].cstr; ;
    break;}
case 118:
#line 346 "grammar.y"
{ yyval.stat = new ForNode(yyvsp[-5].cstr, yyvsp[-4].init, yyvsp[-2].node, yyvsp[0].stat);
                                     delete yyvsp[-5].cstr; ;
    break;}
case 119:
#line 351 "grammar.y"
{ yyval.node = 0L; ;
    break;}
case 121:
#line 356 "grammar.y"
{ yyval.stat = new ContinueNode(); ;
    break;}
case 122:
#line 360 "grammar.y"
{ yyval.stat = new BreakNode(); ;
    break;}
case 123:
#line 364 "grammar.y"
{ yyval.stat = new ReturnNode(0L); ;
    break;}
case 124:
#line 365 "grammar.y"
{ yyval.stat = new ReturnNode(yyvsp[-1].node); ;
    break;}
case 125:
#line 369 "grammar.y"
{ yyval.stat = new WithNode(yyvsp[-2].node, yyvsp[0].stat); ;
    break;}
case 126:
#line 373 "grammar.y"
{ yyval.func = new FuncDeclNode(yyvsp[-3].cstr, 0L, yyvsp[0].stat);
                                     delete yyvsp[-3].cstr; ;
    break;}
case 127:
#line 376 "grammar.y"
{ yyval.func = new FuncDeclNode(yyvsp[-4].cstr, yyvsp[-2].param, yyvsp[0].stat);
                                     delete yyvsp[-4].cstr; ;
    break;}
case 128:
#line 381 "grammar.y"
{ yyval.param = new ParameterNode(yyvsp[0].cstr); delete yyvsp[0].cstr; ;
    break;}
case 129:
#line 382 "grammar.y"
{ yyval.param = yyvsp[-2].param->append(yyvsp[0].cstr); delete yyvsp[0].cstr; ;
    break;}
case 130:
#line 386 "grammar.y"
{ yyval.prog = KJSWorld::prog
					= new ProgramNode(yyvsp[0].srcs); ;
    break;}
case 131:
#line 391 "grammar.y"
{ yyval.srcs = new SourceElementsNode(yyvsp[0].src); ;
    break;}
case 132:
#line 392 "grammar.y"
{ yyval.srcs = new SourceElementsNode(yyvsp[-1].srcs, yyvsp[0].src); ;
    break;}
case 133:
#line 396 "grammar.y"
{ yyval.src = new SourceElementNode(yyvsp[0].stat); ;
    break;}
case 134:
#line 397 "grammar.y"
{ yyval.src = new SourceElementNode(yyvsp[0].func); ;
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
#line 400 "grammar.y"


int yyerror (char *s)  /* Called by yyparse on error */
{
  fprintf (stderr, "ERROR: %s at line %d\n", s, KJSWorld::lexer->lineNo());
  return 1;
}
