/* A Bison parser, made by GNU Bison 1.875.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002 Free Software Foundation, Inc.

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
   Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301,
   USA.
   */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* Written by Richard Stallman by simplifying the original so called
   ``semantic'' parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 1

/* If NAME_PREFIX is specified substitute the variables and functions
   names.  */
#define yyparse kjsyyparse
#define yylex   kjsyylex
#define yyerror kjsyyerror
#define yylval  kjsyylval
#define yychar  kjsyychar
#define yydebug kjsyydebug
#define yynerrs kjsyynerrs
#define yylloc kjsyylloc

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     NULLTOKEN = 258,
     TRUETOKEN = 259,
     FALSETOKEN = 260,
     STRING = 261,
     NUMBER = 262,
     BREAK = 263,
     CASE = 264,
     DEFAULT = 265,
     FOR = 266,
     NEW = 267,
     VAR = 268,
     CONST = 269,
     CONTINUE = 270,
     FUNCTION = 271,
     RETURN = 272,
     VOID = 273,
     DELETE = 274,
     IF = 275,
     THIS = 276,
     DO = 277,
     WHILE = 278,
     IN = 279,
     INSTANCEOF = 280,
     TYPEOF = 281,
     SWITCH = 282,
     WITH = 283,
     RESERVED = 284,
     THROW = 285,
     TRY = 286,
     CATCH = 287,
     FINALLY = 288,
     IF_WITHOUT_ELSE = 289,
     ELSE = 290,
     EQEQ = 291,
     NE = 292,
     STREQ = 293,
     STRNEQ = 294,
     LE = 295,
     GE = 296,
     OR = 297,
     AND = 298,
     PLUSPLUS = 299,
     MINUSMINUS = 300,
     LSHIFT = 301,
     RSHIFT = 302,
     URSHIFT = 303,
     PLUSEQUAL = 304,
     MINUSEQUAL = 305,
     MULTEQUAL = 306,
     DIVEQUAL = 307,
     LSHIFTEQUAL = 308,
     RSHIFTEQUAL = 309,
     URSHIFTEQUAL = 310,
     ANDEQUAL = 311,
     MODEQUAL = 312,
     XOREQUAL = 313,
     OREQUAL = 314,
     IDENT = 315,
     AUTOPLUSPLUS = 316,
     AUTOMINUSMINUS = 317
   };
#endif
#define NULLTOKEN 258
#define TRUETOKEN 259
#define FALSETOKEN 260
#define STRING 261
#define NUMBER 262
#define BREAK 263
#define CASE 264
#define DEFAULT 265
#define FOR 266
#define NEW 267
#define VAR 268
#define CONST 269
#define CONTINUE 270
#define FUNCTION 271
#define RETURN 272
#define VOID 273
#define DELETE 274
#define IF 275
#define THIS 276
#define DO 277
#define WHILE 278
#define IN 279
#define INSTANCEOF 280
#define TYPEOF 281
#define SWITCH 282
#define WITH 283
#define RESERVED 284
#define THROW 285
#define TRY 286
#define CATCH 287
#define FINALLY 288
#define IF_WITHOUT_ELSE 289
#define ELSE 290
#define EQEQ 291
#define NE 292
#define STREQ 293
#define STRNEQ 294
#define LE 295
#define GE 296
#define OR 297
#define AND 298
#define PLUSPLUS 299
#define MINUSMINUS 300
#define LSHIFT 301
#define RSHIFT 302
#define URSHIFT 303
#define PLUSEQUAL 304
#define MINUSEQUAL 305
#define MULTEQUAL 306
#define DIVEQUAL 307
#define LSHIFTEQUAL 308
#define RSHIFTEQUAL 309
#define URSHIFTEQUAL 310
#define ANDEQUAL 311
#define MODEQUAL 312
#define XOREQUAL 313
#define OREQUAL 314
#define IDENT 315
#define AUTOPLUSPLUS 316
#define AUTOMINUSMINUS 317




/* Copy the first part of user declarations.  */
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
 *  Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <string.h>
#include <stdlib.h>
#include "value.h"
#include "object.h"
#include "types.h"
#include "interpreter.h"
#include "nodes.h"
#include "lexer.h"
#include "internal.h"

// Not sure why, but yacc doesn't add this define along with the others.
#define yylloc kjsyylloc

/* default values for bison */
#define YYDEBUG 0
#if !APPLE_CHANGES
#define YYERROR_VERBOSE
#endif

extern int kjsyylex();
int kjsyyerror(const char *);
static bool allowAutomaticSemicolon();

#define AUTO_SEMICOLON do { if (!allowAutomaticSemicolon()) YYABORT; } while (0)
#define DBG(l, s, e) (l)->setLoc((s).first_line, (e).last_line, Parser::sid)

using namespace KJS;

static bool makeAssignNode(Node*& result, Node *loc, Operator op, Node *expr);
static bool makePrefixNode(Node*& result, Node *expr, Operator op);
static bool makePostfixNode(Node*& result, Node *expr, Operator op);
static Node *makeFunctionCallNode(Node *func, ArgumentsNode *args);
static Node *makeTypeOfNode(Node *expr);
static Node *makeDeleteNode(Node *expr);



/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
#line 63 "grammar.y"
typedef union YYSTYPE {
  int                 ival;
  double              dval;
  UString             *ustr;
  Identifier          *ident;
  Node                *node;
  StatementNode       *stat;
  ParameterNode       *param;
  FunctionBodyNode    *body;
  FuncDeclNode        *func;
  FuncExprNode        *funcExpr;
  ProgramNode         *prog;
  AssignExprNode      *init;
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
  Operator            op;
  PropertyValueNode   *plist;
  PropertyNode        *pnode;
} YYSTYPE;
/* Line 191 of yacc.c.  */
#line 298 "grammar.tab.c"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

#if ! defined (YYLTYPE) && ! defined (YYLTYPE_IS_DECLARED)
typedef struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
} YYLTYPE;
# define yyltype YYLTYPE /* obsolescent; will be withdrawn */
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif


/* Copy the second part of user declarations.  */


/* Line 214 of yacc.c.  */
#line 322 "grammar.tab.c"

#if ! defined (yyoverflow) || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# if YYSTACK_USE_ALLOCA
#  define YYSTACK_ALLOC alloca
# else
#  ifndef YYSTACK_USE_ALLOCA
#   if defined (alloca) || defined (_ALLOCA_H)
#    define YYSTACK_ALLOC alloca
#   else
#    ifdef __GNUC__
#     define YYSTACK_ALLOC __builtin_alloca
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning. */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
# else
#  if defined (__STDC__) || defined (__cplusplus)
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   define YYSIZE_T size_t
#  endif
#  define YYSTACK_ALLOC malloc
#  define YYSTACK_FREE free
# endif
#endif /* ! defined (yyoverflow) || YYERROR_VERBOSE */


#if (! defined (yyoverflow) \
     && (! defined (__cplusplus) \
	 || (YYLTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  short yyss;
  YYSTYPE yyvs;
    YYLTYPE yyls;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short) + sizeof (YYSTYPE) + sizeof (YYLTYPE))	\
      + 2 * YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  register YYSIZE_T yyi;		\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (0)
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (0)

#endif

#if defined (__STDC__) || defined (__cplusplus)
   typedef signed char yysigned_char;
#else
   typedef short yysigned_char;
#endif

/* YYFINAL -- State number of the termination state. */
#define YYFINAL  204
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1501

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  87
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  101
/* YYNRULES -- Number of rules. */
#define YYNRULES  296
/* YYNRULES -- Number of states. */
#define YYNSTATES  519

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   317

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    76,     2,     2,     2,    78,    81,     2,
      66,    67,    77,    73,    70,    74,    72,    63,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    71,    86,
      79,    85,    80,    84,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    68,     2,    69,    82,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    64,    83,    65,    75,     2,     2,     2,
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
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short yyprhs[] =
{
       0,     0,     3,     5,     7,     9,    11,    13,    15,    17,
      19,    22,    26,    28,    30,    32,    34,    38,    42,    46,
      52,    55,    60,    61,    63,    65,    68,    72,    78,    80,
      82,    84,    86,    88,    93,    97,   101,   103,   108,   112,
     116,   118,   121,   123,   126,   129,   132,   137,   141,   144,
     147,   152,   156,   159,   163,   165,   169,   171,   173,   175,
     177,   179,   182,   185,   187,   190,   193,   196,   199,   202,
     205,   208,   211,   214,   217,   220,   223,   226,   228,   230,
     232,   234,   236,   240,   244,   248,   250,   254,   258,   262,
     264,   268,   272,   274,   278,   282,   284,   288,   292,   296,
     298,   302,   306,   310,   312,   316,   320,   324,   328,   332,
     336,   338,   342,   346,   350,   354,   358,   360,   364,   368,
     372,   376,   380,   384,   386,   390,   394,   398,   402,   404,
     408,   412,   416,   420,   422,   426,   430,   434,   438,   440,
     444,   446,   450,   452,   456,   458,   462,   464,   468,   470,
     474,   476,   480,   482,   486,   488,   492,   494,   498,   500,
     504,   506,   510,   512,   516,   518,   522,   524,   528,   530,
     536,   538,   544,   546,   552,   554,   558,   560,   564,   566,
     570,   572,   574,   576,   578,   580,   582,   584,   586,   588,
     590,   592,   594,   596,   600,   602,   606,   608,   612,   614,
     616,   618,   620,   622,   624,   626,   628,   630,   632,   634,
     636,   638,   640,   642,   645,   649,   651,   654,   658,   662,
     664,   668,   670,   674,   676,   679,   681,   684,   688,   692,
     694,   698,   700,   703,   706,   709,   711,   714,   717,   723,
     731,   738,   744,   754,   765,   773,   782,   792,   793,   795,
     796,   798,   801,   804,   808,   812,   815,   818,   822,   826,
     829,   832,   836,   840,   846,   852,   856,   862,   863,   865,
     867,   870,   874,   879,   882,   886,   890,   894,   898,   903,
     911,   921,   927,   934,   939,   945,   951,   958,   960,   964,
     967,   971,   972,   974,   976,   979,   981
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short yyrhs[] =
{
     185,     0,    -1,     3,    -1,     4,    -1,     5,    -1,     7,
      -1,     6,    -1,    63,    -1,    52,    -1,    90,    -1,    64,
      65,    -1,    64,    95,    65,    -1,    21,    -1,    88,    -1,
      91,    -1,    60,    -1,    66,   146,    67,    -1,    68,    93,
      69,    -1,    68,    92,    69,    -1,    68,    92,    70,    93,
      69,    -1,    93,   142,    -1,    92,    70,    93,   142,    -1,
      -1,    94,    -1,    70,    -1,    94,    70,    -1,    96,    71,
     142,    -1,    95,    70,    96,    71,   142,    -1,    60,    -1,
       6,    -1,     7,    -1,    89,    -1,   182,    -1,    97,    68,
     146,    69,    -1,    97,    72,    60,    -1,    12,    97,   103,
      -1,    90,    -1,    98,    68,   146,    69,    -1,    98,    72,
      60,    -1,    12,    97,   103,    -1,    97,    -1,    12,    99,
      -1,    98,    -1,    12,    99,    -1,    97,   103,    -1,   101,
     103,    -1,   101,    68,   146,    69,    -1,   101,    72,    60,
      -1,    98,   103,    -1,   102,   103,    -1,   102,    68,   146,
      69,    -1,   102,    72,    60,    -1,    66,    67,    -1,    66,
     104,    67,    -1,   142,    -1,   104,    70,   142,    -1,    99,
      -1,   101,    -1,   100,    -1,   102,    -1,   105,    -1,   105,
      44,    -1,   105,    45,    -1,   106,    -1,   106,    44,    -1,
     106,    45,    -1,    19,   110,    -1,    18,   110,    -1,    26,
     110,    -1,    44,   110,    -1,    61,   110,    -1,    45,   110,
      -1,    62,   110,    -1,    73,   110,    -1,    74,   110,    -1,
      75,   110,    -1,    76,   110,    -1,   107,    -1,   109,    -1,
     108,    -1,   109,    -1,   110,    -1,   112,    77,   110,    -1,
     112,    63,   110,    -1,   112,    78,   110,    -1,   111,    -1,
     113,    77,   110,    -1,   113,    63,   110,    -1,   113,    78,
     110,    -1,   112,    -1,   114,    73,   112,    -1,   114,    74,
     112,    -1,   113,    -1,   115,    73,   112,    -1,   115,    74,
     112,    -1,   114,    -1,   116,    46,   114,    -1,   116,    47,
     114,    -1,   116,    48,   114,    -1,   115,    -1,   117,    46,
     114,    -1,   117,    47,   114,    -1,   117,    48,   114,    -1,
     116,    -1,   118,    79,   116,    -1,   118,    80,   116,    -1,
     118,    40,   116,    -1,   118,    41,   116,    -1,   118,    25,
     116,    -1,   118,    24,   116,    -1,   116,    -1,   119,    79,
     116,    -1,   119,    80,   116,    -1,   119,    40,   116,    -1,
     119,    41,   116,    -1,   119,    25,   116,    -1,   117,    -1,
     120,    79,   116,    -1,   120,    80,   116,    -1,   120,    40,
     116,    -1,   120,    41,   116,    -1,   120,    25,   116,    -1,
     120,    24,   116,    -1,   118,    -1,   121,    36,   118,    -1,
     121,    37,   118,    -1,   121,    38,   118,    -1,   121,    39,
     118,    -1,   119,    -1,   122,    36,   119,    -1,   122,    37,
     119,    -1,   122,    38,   119,    -1,   122,    39,   119,    -1,
     120,    -1,   123,    36,   118,    -1,   123,    37,   118,    -1,
     123,    38,   118,    -1,   123,    39,   118,    -1,   121,    -1,
     124,    81,   121,    -1,   122,    -1,   125,    81,   122,    -1,
     123,    -1,   126,    81,   121,    -1,   124,    -1,   127,    82,
     124,    -1,   125,    -1,   128,    82,   125,    -1,   126,    -1,
     129,    82,   124,    -1,   127,    -1,   130,    83,   127,    -1,
     128,    -1,   131,    83,   128,    -1,   129,    -1,   132,    83,
     127,    -1,   130,    -1,   133,    43,   130,    -1,   131,    -1,
     134,    43,   131,    -1,   132,    -1,   135,    43,   130,    -1,
     133,    -1,   136,    42,   133,    -1,   134,    -1,   137,    42,
     134,    -1,   135,    -1,   138,    42,   133,    -1,   136,    -1,
     136,    84,   142,    71,   142,    -1,   137,    -1,   137,    84,
     143,    71,   143,    -1,   138,    -1,   138,    84,   142,    71,
     142,    -1,   139,    -1,   105,   145,   142,    -1,   140,    -1,
     105,   145,   143,    -1,   141,    -1,   106,   145,   142,    -1,
      85,    -1,    49,    -1,    50,    -1,    51,    -1,    52,    -1,
      53,    -1,    54,    -1,    55,    -1,    56,    -1,    58,    -1,
      59,    -1,    57,    -1,   142,    -1,   146,    70,   142,    -1,
     143,    -1,   147,    70,   143,    -1,   144,    -1,   148,    70,
     142,    -1,   150,    -1,   152,    -1,   157,    -1,   162,    -1,
     163,    -1,   164,    -1,   165,    -1,   168,    -1,   169,    -1,
     170,    -1,   171,    -1,   172,    -1,   178,    -1,   179,    -1,
     180,    -1,    64,    65,    -1,    64,   186,    65,    -1,   149,
      -1,   151,   149,    -1,    13,   153,    86,    -1,    13,   153,
       1,    -1,   155,    -1,   153,    70,   155,    -1,   156,    -1,
     154,    70,   156,    -1,    60,    -1,    60,   160,    -1,    60,
      -1,    60,   161,    -1,    14,   158,    86,    -1,    14,   158,
       1,    -1,   159,    -1,   158,    70,   159,    -1,    60,    -1,
      60,   160,    -1,    85,   142,    -1,    85,   143,    -1,    86,
      -1,   148,    86,    -1,   148,     1,    -1,    20,    66,   146,
      67,   149,    -1,    20,    66,   146,    67,   149,    35,   149,
      -1,    22,   149,    23,    66,   146,    67,    -1,    23,    66,
     146,    67,   149,    -1,    11,    66,   167,    86,   166,    86,
     166,    67,   149,    -1,    11,    66,    13,   154,    86,   166,
      86,   166,    67,   149,    -1,    11,    66,   105,    24,   146,
      67,   149,    -1,    11,    66,    13,    60,    24,   146,    67,
     149,    -1,    11,    66,    13,    60,   161,    24,   146,    67,
     149,    -1,    -1,   146,    -1,    -1,   147,    -1,    15,    86,
      -1,    15,     1,    -1,    15,    60,    86,    -1,    15,    60,
       1,    -1,     8,    86,    -1,     8,     1,    -1,     8,    60,
      86,    -1,     8,    60,     1,    -1,    17,    86,    -1,    17,
       1,    -1,    17,   146,    86,    -1,    17,   146,     1,    -1,
      28,    66,   146,    67,   149,    -1,    27,    66,   146,    67,
     173,    -1,    64,   174,    65,    -1,    64,   174,   177,   174,
      65,    -1,    -1,   175,    -1,   176,    -1,   175,   176,    -1,
       9,   146,    71,    -1,     9,   146,    71,   151,    -1,    10,
      71,    -1,    10,    71,   151,    -1,    60,    71,   149,    -1,
      30,   146,    86,    -1,    30,   146,     1,    -1,    31,   150,
      33,   150,    -1,    31,   150,    32,    66,    60,    67,   150,
      -1,    31,   150,    32,    66,    60,    67,   150,    33,   150,
      -1,    16,    60,    66,    67,   184,    -1,    16,    60,    66,
     183,    67,   184,    -1,    16,    66,    67,   184,    -1,    16,
      66,   183,    67,   184,    -1,    16,    60,    66,    67,   184,
      -1,    16,    60,    66,   183,    67,   184,    -1,    60,    -1,
     183,    70,    60,    -1,    64,    65,    -1,    64,   186,    65,
      -1,    -1,   186,    -1,   187,    -1,   186,   187,    -1,   181,
      -1,   149,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   188,   188,   189,   190,   191,   192,   193,   198,   206,
     207,   208,   212,   213,   214,   215,   216,   220,   221,   222,
     226,   227,   232,   233,   237,   238,   242,   243,   248,   249,
     250,   254,   255,   256,   257,   258,   262,   263,   264,   265,
     269,   270,   274,   275,   279,   280,   281,   282,   286,   287,
     288,   289,   293,   294,   298,   299,   303,   304,   308,   309,
     313,   314,   315,   319,   320,   321,   325,   326,   327,   328,
     329,   330,   331,   332,   333,   334,   335,   338,   339,   343,
     344,   348,   349,   350,   351,   355,   356,   358,   360,   365,
     366,   367,   371,   372,   374,   379,   380,   381,   382,   386,
     387,   388,   389,   393,   394,   395,   396,   397,   398,   399,
     403,   404,   405,   406,   407,   408,   413,   414,   415,   416,
     417,   418,   420,   424,   425,   426,   427,   428,   432,   433,
     435,   437,   439,   444,   445,   447,   448,   450,   455,   456,
     460,   461,   466,   467,   471,   472,   476,   477,   482,   483,
     488,   489,   493,   494,   499,   500,   505,   506,   510,   511,
     516,   517,   522,   523,   527,   528,   533,   534,   538,   539,
     544,   545,   550,   551,   556,   557,   562,   563,   568,   569,
     574,   575,   576,   577,   578,   579,   580,   581,   582,   583,
     584,   585,   589,   590,   594,   595,   599,   600,   604,   605,
     606,   607,   608,   609,   610,   611,   612,   613,   614,   615,
     616,   617,   618,   622,   623,   627,   628,   632,   633,   637,
     638,   643,   644,   649,   650,   654,   655,   659,   660,   664,
     665,   670,   671,   675,   679,   683,   687,   688,   692,   694,
     699,   700,   701,   703,   705,   713,   715,   720,   721,   725,
     726,   730,   731,   732,   733,   737,   738,   739,   740,   744,
     745,   746,   747,   751,   755,   759,   760,   765,   766,   770,
     771,   775,   776,   780,   781,   785,   789,   790,   794,   795,
     796,   801,   802,   807,   808,   810,   811,   816,   817,   821,
     822,   826,   827,   831,   832,   836,   837
};
#endif

#if YYDEBUG || YYERROR_VERBOSE
/* YYTNME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "NULLTOKEN", "TRUETOKEN", "FALSETOKEN", 
  "STRING", "NUMBER", "BREAK", "CASE", "DEFAULT", "FOR", "NEW", "VAR", 
  "CONST", "CONTINUE", "FUNCTION", "RETURN", "VOID", "DELETE", "IF", 
  "THIS", "DO", "WHILE", "IN", "INSTANCEOF", "TYPEOF", "SWITCH", "WITH", 
  "RESERVED", "THROW", "TRY", "CATCH", "FINALLY", "IF_WITHOUT_ELSE", 
  "ELSE", "EQEQ", "NE", "STREQ", "STRNEQ", "LE", "GE", "OR", "AND", 
  "PLUSPLUS", "MINUSMINUS", "LSHIFT", "RSHIFT", "URSHIFT", "PLUSEQUAL", 
  "MINUSEQUAL", "MULTEQUAL", "DIVEQUAL", "LSHIFTEQUAL", "RSHIFTEQUAL", 
  "URSHIFTEQUAL", "ANDEQUAL", "MODEQUAL", "XOREQUAL", "OREQUAL", "IDENT", 
  "AUTOPLUSPLUS", "AUTOMINUSMINUS", "'/'", "'{'", "'}'", "'('", "')'", 
  "'['", "']'", "','", "':'", "'.'", "'+'", "'-'", "'~'", "'!'", "'*'", 
  "'%'", "'<'", "'>'", "'&'", "'^'", "'|'", "'?'", "'='", "';'", 
  "$accept", "Literal", "PrimaryExpr", "PrimaryExprNoBrace", 
  "ArrayLiteral", "ElementList", "ElisionOpt", "Elision", 
  "PropertyNameAndValueList", "PropertyName", "MemberExpr", 
  "MemberExprNoBF", "NewExpr", "NewExprNoBF", "CallExpr", "CallExprNoBF", 
  "Arguments", "ArgumentList", "LeftHandSideExpr", "LeftHandSideExprNoBF", 
  "PostfixExpr", "PostfixExprNoBF", "UnaryExprCommon", "UnaryExpr", 
  "UnaryExprNoBF", "MultiplicativeExpr", "MultiplicativeExprNoBF", 
  "AdditiveExpr", "AdditiveExprNoBF", "ShiftExpr", "ShiftExprNoBF", 
  "RelationalExpr", "RelationalExprNoIn", "RelationalExprNoBF", 
  "EqualityExpr", "EqualityExprNoIn", "EqualityExprNoBF", 
  "BitwiseANDExpr", "BitwiseANDExprNoIn", "BitwiseANDExprNoBF", 
  "BitwiseXORExpr", "BitwiseXORExprNoIn", "BitwiseXORExprNoBF", 
  "BitwiseORExpr", "BitwiseORExprNoIn", "BitwiseORExprNoBF", 
  "LogicalANDExpr", "LogicalANDExprNoIn", "LogicalANDExprNoBF", 
  "LogicalORExpr", "LogicalORExprNoIn", "LogicalORExprNoBF", 
  "ConditionalExpr", "ConditionalExprNoIn", "ConditionalExprNoBF", 
  "AssignmentExpr", "AssignmentExprNoIn", "AssignmentExprNoBF", 
  "AssignmentOperator", "Expr", "ExprNoIn", "ExprNoBF", "Statement", 
  "Block", "StatementList", "VariableStatement", 
  "VariableDeclarationList", "VariableDeclarationListNoIn", 
  "VariableDeclaration", "VariableDeclarationNoIn", "ConstStatement", 
  "ConstDeclarationList", "ConstDeclaration", "Initializer", 
  "InitializerNoIn", "EmptyStatement", "ExprStatement", "IfStatement", 
  "IterationStatement", "ExprOpt", "ExprNoInOpt", "ContinueStatement", 
  "BreakStatement", "ReturnStatement", "WithStatement", "SwitchStatement", 
  "CaseBlock", "CaseClausesOpt", "CaseClauses", "CaseClause", 
  "DefaultClause", "LabelledStatement", "ThrowStatement", "TryStatement", 
  "FunctionDeclaration", "FunctionExpr", "FormalParameterList", 
  "FunctionBody", "Program", "SourceElements", "SourceElement", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const unsigned short yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,    47,   123,   125,    40,    41,    91,    93,
      44,    58,    46,    43,    45,   126,    33,    42,    37,    60,
      62,    38,    94,   124,    63,    61,    59
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    87,    88,    88,    88,    88,    88,    88,    88,    89,
      89,    89,    90,    90,    90,    90,    90,    91,    91,    91,
      92,    92,    93,    93,    94,    94,    95,    95,    96,    96,
      96,    97,    97,    97,    97,    97,    98,    98,    98,    98,
      99,    99,   100,   100,   101,   101,   101,   101,   102,   102,
     102,   102,   103,   103,   104,   104,   105,   105,   106,   106,
     107,   107,   107,   108,   108,   108,   109,   109,   109,   109,
     109,   109,   109,   109,   109,   109,   109,   110,   110,   111,
     111,   112,   112,   112,   112,   113,   113,   113,   113,   114,
     114,   114,   115,   115,   115,   116,   116,   116,   116,   117,
     117,   117,   117,   118,   118,   118,   118,   118,   118,   118,
     119,   119,   119,   119,   119,   119,   120,   120,   120,   120,
     120,   120,   120,   121,   121,   121,   121,   121,   122,   122,
     122,   122,   122,   123,   123,   123,   123,   123,   124,   124,
     125,   125,   126,   126,   127,   127,   128,   128,   129,   129,
     130,   130,   131,   131,   132,   132,   133,   133,   134,   134,
     135,   135,   136,   136,   137,   137,   138,   138,   139,   139,
     140,   140,   141,   141,   142,   142,   143,   143,   144,   144,
     145,   145,   145,   145,   145,   145,   145,   145,   145,   145,
     145,   145,   146,   146,   147,   147,   148,   148,   149,   149,
     149,   149,   149,   149,   149,   149,   149,   149,   149,   149,
     149,   149,   149,   150,   150,   151,   151,   152,   152,   153,
     153,   154,   154,   155,   155,   156,   156,   157,   157,   158,
     158,   159,   159,   160,   161,   162,   163,   163,   164,   164,
     165,   165,   165,   165,   165,   165,   165,   166,   166,   167,
     167,   168,   168,   168,   168,   169,   169,   169,   169,   170,
     170,   170,   170,   171,   172,   173,   173,   174,   174,   175,
     175,   176,   176,   177,   177,   178,   179,   179,   180,   180,
     180,   181,   181,   182,   182,   182,   182,   183,   183,   184,
     184,   185,   185,   186,   186,   187,   187
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     1,     1,     1,     1,     1,     1,     1,     1,
       2,     3,     1,     1,     1,     1,     3,     3,     3,     5,
       2,     4,     0,     1,     1,     2,     3,     5,     1,     1,
       1,     1,     1,     4,     3,     3,     1,     4,     3,     3,
       1,     2,     1,     2,     2,     2,     4,     3,     2,     2,
       4,     3,     2,     3,     1,     3,     1,     1,     1,     1,
       1,     2,     2,     1,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     1,     1,     1,
       1,     1,     3,     3,     3,     1,     3,     3,     3,     1,
       3,     3,     1,     3,     3,     1,     3,     3,     3,     1,
       3,     3,     3,     1,     3,     3,     3,     3,     3,     3,
       1,     3,     3,     3,     3,     3,     1,     3,     3,     3,
       3,     3,     3,     1,     3,     3,     3,     3,     1,     3,
       3,     3,     3,     1,     3,     3,     3,     3,     1,     3,
       1,     3,     1,     3,     1,     3,     1,     3,     1,     3,
       1,     3,     1,     3,     1,     3,     1,     3,     1,     3,
       1,     3,     1,     3,     1,     3,     1,     3,     1,     5,
       1,     5,     1,     5,     1,     3,     1,     3,     1,     3,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     3,     1,     3,     1,     3,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     2,     3,     1,     2,     3,     3,     1,
       3,     1,     3,     1,     2,     1,     2,     3,     3,     1,
       3,     1,     2,     2,     2,     1,     2,     2,     5,     7,
       6,     5,     9,    10,     7,     8,     9,     0,     1,     0,
       1,     2,     2,     3,     3,     2,     2,     3,     3,     2,
       2,     3,     3,     5,     5,     3,     5,     0,     1,     1,
       2,     3,     4,     2,     3,     3,     3,     3,     4,     7,
       9,     5,     6,     4,     5,     5,     6,     1,     3,     2,
       3,     0,     1,     1,     2,     1,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned short yydefact[] =
{
     291,     2,     3,     4,     6,     5,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    12,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     8,    15,     0,
       0,     7,     0,     0,    22,     0,     0,     0,     0,   235,
      13,    36,    14,    42,    58,    59,    63,    79,    80,    85,
      92,    99,   116,   133,   142,   148,   154,   160,   166,   172,
     178,   196,     0,   296,   198,   199,   200,   201,   202,   203,
     204,   205,   206,   207,   208,   209,   210,   211,   212,   295,
       0,   292,   293,   256,     0,   255,   249,     0,     0,    15,
       0,    31,     9,    40,    43,    32,   223,     0,   219,   231,
       0,   229,   252,     0,   251,     0,   260,   259,    40,    56,
      57,    60,    77,    78,    81,    89,    95,   103,   123,   138,
     144,   150,   156,   162,   168,   174,   192,     0,    60,    67,
      66,     0,     0,     0,    68,     0,     0,     0,     0,    69,
      71,     0,    70,    72,   213,     0,     0,    24,     0,     0,
      23,    73,    74,    75,    76,     0,     0,     0,    48,     0,
       0,    49,    64,    65,   181,   182,   183,   184,   185,   186,
     187,   188,   191,   189,   190,   180,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   237,     0,   236,     1,   294,   258,   257,     0,    60,
     110,   128,   140,   146,   152,   158,   164,   170,   176,   194,
     250,     0,    40,    41,     0,     0,    29,    30,    28,    10,
       0,     0,     0,     0,    39,     0,   224,   218,     0,   217,
     232,   228,     0,   227,   254,   253,     0,    44,     0,     0,
      45,    61,    62,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   262,     0,
     261,     0,     0,     0,     0,     0,   277,   276,     0,     0,
     275,   214,    16,    18,    22,    17,    20,    25,    52,     0,
      54,     0,    38,     0,    51,   179,    87,    86,    88,    93,
      94,   100,   101,   102,   122,   121,   119,   120,   117,   118,
     134,   135,   136,   137,   143,   149,   155,   161,   167,     0,
     197,   225,     0,   221,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   247,    35,     0,   287,     0,     0,    11,     0,
       0,     0,    34,   233,   220,   230,     0,     0,     0,    47,
     175,    83,    82,    84,    90,    91,    96,    97,    98,   109,
     108,   106,   107,   104,   105,   124,   125,   126,   127,   139,
     145,   151,   157,   163,     0,   193,     0,     0,     0,     0,
       0,     0,   278,     0,    53,     0,    37,    50,     0,     0,
       0,   226,     0,   247,     0,    60,   177,   115,   113,   114,
     111,   112,   129,   130,   131,   132,   141,   147,   153,   159,
     165,     0,   195,   248,     0,     0,     0,     0,   283,     0,
       0,     0,    26,    33,   281,     0,    46,     0,   238,     0,
     241,   267,   264,   263,     0,    19,    21,    55,   173,     0,
     234,     0,   225,   222,     0,     0,     0,   247,   285,     0,
     289,     0,   284,   288,     0,   282,   169,     0,   240,     0,
       0,   268,   269,     0,     0,     0,   226,   247,   244,   171,
       0,   286,   290,    27,   239,     0,     0,   265,   267,   270,
     279,   245,     0,     0,     0,   271,   273,     0,     0,   246,
       0,   242,   215,   272,   274,   266,   280,   243,   216
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,    40,    91,    92,    42,   148,   149,   150,   230,   231,
     108,    43,   109,    44,   110,    45,   158,   299,   128,    46,
     112,    47,   113,   114,    49,   115,    50,   116,    51,   117,
      52,   118,   211,    53,   119,   212,    54,   120,   213,    55,
     121,   214,    56,   122,   215,    57,   123,   216,    58,   124,
     217,    59,   125,   218,    60,   126,   219,    61,   335,   433,
     220,    62,    63,    64,   513,    65,    97,   332,    98,   333,
      66,   100,   101,   236,   411,    67,    68,    69,    70,   434,
     221,    71,    72,    73,    74,    75,   452,   480,   481,   482,
     498,    76,    77,    78,    79,    95,   357,   438,    80,    81,
      82
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -356
static const short yypact[] =
{
     978,  -356,  -356,  -356,  -356,  -356,     5,   -39,   159,   -25,
      37,    16,    61,   559,  1425,  1425,    93,  -356,  1054,   110,
    1425,   121,   162,  1425,    -4,  1425,  1425,  -356,    52,  1425,
    1425,  -356,   674,  1425,    79,  1425,  1425,  1425,  1425,  -356,
    -356,  -356,  -356,   174,  -356,   185,   598,  -356,  -356,  -356,
     106,   182,   214,   223,   270,   156,   150,   158,   227,   -26,
    -356,  -356,    18,  -356,  -356,  -356,  -356,  -356,  -356,  -356,
    -356,  -356,  -356,  -356,  -356,  -356,  -356,  -356,  -356,  -356,
     268,   978,  -356,  -356,    24,  -356,  1129,   159,    47,  -356,
     166,  -356,  -356,   186,  -356,  -356,   187,    19,  -356,   187,
      29,  -356,  -356,    25,  -356,   218,  -356,  -356,   186,  -356,
     201,  1109,  -356,  -356,  -356,   161,   245,   302,   225,   278,
     229,   243,   244,   301,   -21,  -356,  -356,    30,   147,  -356,
    -356,  1425,   308,  1425,  -356,  1425,  1425,    36,   322,  -356,
    -356,  1054,  -356,  -356,  -356,   750,   -58,  -356,   288,  1203,
     281,  -356,  -356,  -356,  -356,  1277,  1425,   311,  -356,  1425,
     312,  -356,  -356,  -356,  -356,  -356,  -356,  -356,  -356,  -356,
    -356,  -356,  -356,  -356,  -356,  -356,  1425,  1425,  1425,  1425,
    1425,  1425,  1425,  1425,  1425,  1425,  1425,  1425,  1425,  1425,
    1425,  1425,  1425,  1425,  1425,  1425,  1425,  1425,  1425,  1425,
    1425,  -356,  1425,  -356,  -356,  -356,  -356,  -356,   314,   144,
     302,   165,   285,   295,   277,   297,   332,    17,  -356,  -356,
     307,   300,   186,  -356,   316,   114,  -356,  -356,  -356,  -356,
      83,   313,  1425,   318,  -356,  1425,  -356,  -356,   -25,  -356,
    -356,  -356,    37,  -356,  -356,  -356,   118,  -356,  1425,   325,
    -356,  -356,  -356,  1425,  1425,  1425,  1425,  1425,  1425,  1425,
    1425,  1425,  1425,  1425,  1425,  1425,  1425,  1425,  1425,  1425,
    1425,  1425,  1425,  1425,  1425,  1425,  1425,  1425,  -356,  1425,
    -356,   -22,   321,    -6,    91,   137,  -356,  -356,   323,    -4,
    -356,  -356,  -356,  -356,    79,  -356,  -356,  -356,  -356,   151,
    -356,   292,  -356,   294,  -356,  -356,  -356,  -356,  -356,   161,
     161,   245,   245,   245,   302,   302,   302,   302,   302,   302,
     225,   225,   225,   225,   278,   229,   243,   244,   301,   317,
    -356,    45,   -57,  -356,  1425,  1425,  1425,  1425,  1425,  1425,
    1425,  1425,  1425,  1425,  1425,  1425,  1425,  1425,  1425,  1425,
    1425,  1425,  1425,  -356,   119,  -356,   326,   204,  -356,    49,
    1425,   296,  -356,  -356,  -356,  -356,   326,   210,   298,  -356,
    -356,  -356,  -356,  -356,   161,   161,   245,   245,   245,   302,
     302,   302,   302,   302,   302,   225,   225,   225,   225,   278,
     229,   243,   244,   301,   320,  -356,  1054,  1425,  1054,   328,
    1054,   334,  -356,  1351,  -356,  1425,  -356,  -356,  1425,  1425,
    1425,   371,   341,  1425,   212,  1109,  -356,   302,   302,   302,
     302,   302,   165,   165,   165,   165,   285,   295,   277,   297,
     332,   331,  -356,   333,   319,   326,   259,   826,  -356,   326,
     344,   335,  -356,  -356,  -356,   326,  -356,  1425,   374,   271,
    -356,   401,  -356,  -356,   345,  -356,  -356,  -356,  -356,   272,
    -356,  1425,   329,  -356,   336,  1054,  1425,  1425,  -356,   326,
    -356,   902,  -356,  -356,  1425,  -356,  -356,  1054,  -356,  1425,
      -3,   401,  -356,    -4,  1054,   273,  -356,  1425,  -356,  -356,
     350,  -356,  -356,  -356,  -356,   299,   347,  -356,   401,  -356,
     386,  -356,  1054,   354,  1054,  1054,  1054,   358,    -4,  -356,
    1054,  -356,  -356,  1054,  1054,  -356,  -356,  -356,  -356
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -356,  -356,  -356,     0,  -356,  -356,   130,  -356,  -356,    66,
      38,  -356,    46,  -356,  -356,  -356,   -40,  -356,   197,  -356,
    -356,  -356,    15,    57,  -356,   -49,  -356,  -132,  -356,   -50,
    -356,  -151,    -7,  -356,  -187,    81,  -356,  -139,    82,  -356,
    -131,    80,  -356,  -124,    85,  -356,  -121,    86,  -356,  -356,
    -356,  -356,  -356,  -356,  -356,  -127,  -312,  -356,   -32,    11,
    -356,  -356,   -17,   -14,   -76,  -356,  -356,  -356,   193,    22,
    -356,  -356,   194,   339,   -23,  -356,  -356,  -356,  -356,  -310,
    -356,  -356,  -356,  -356,  -356,  -356,  -356,   -56,  -356,   -41,
    -356,  -356,  -356,  -356,  -356,  -356,  -242,  -355,  -356,   -30,
     -78
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const unsigned short yytable[] =
{
      41,   132,   145,   205,   367,   161,    83,   496,   324,   292,
     138,   444,   279,   412,   176,    48,   199,   102,    41,   201,
     237,   276,   296,   416,   127,   206,   244,    86,   300,   413,
     241,   278,    41,    48,   137,    96,   210,   286,   431,   432,
     320,   321,   322,   323,   146,   396,    93,    48,   279,   305,
     311,   312,   313,   234,    94,   226,   227,   325,   200,   349,
      32,   398,   497,   277,   279,    84,   326,   205,   247,   409,
     250,   129,   130,   329,   327,   330,   103,   134,   328,   253,
     468,    41,   139,   140,   472,   389,   142,   143,   202,   238,
     475,    85,   151,   152,   153,   154,    48,    99,   460,   242,
     279,   350,   104,   464,   203,   239,   279,   224,   363,   228,
     207,   245,   436,   225,   491,   243,   280,   385,   386,   387,
     388,   105,   287,   141,   290,   222,   370,   376,   377,   378,
     410,   309,   310,   223,   390,   314,   315,   316,   317,   318,
     319,    41,   281,   391,   283,    41,   284,   285,   358,   147,
     394,   392,   395,   359,   489,   393,    48,   490,   399,   131,
      48,   279,     1,     2,     3,     4,     5,   301,   334,   177,
     303,    87,   226,   227,   355,    88,   133,   503,   355,   355,
      17,   356,   353,   178,   179,   366,   435,   135,   251,   252,
     336,   251,   252,   164,   165,   166,   167,   168,   169,   170,
     171,   172,   173,   174,   400,   337,   338,   279,   374,   375,
     111,    27,   379,   380,   381,   382,   383,   384,   404,    89,
     111,   405,    31,    90,   254,    33,   228,    34,   136,   175,
     111,   229,   196,   442,   306,   307,   308,   195,   255,   256,
     155,   197,   156,   361,   339,   340,   157,   185,   186,   262,
     263,   155,   155,   159,   232,   180,   181,   160,   233,   368,
     182,   183,   184,   187,   188,   264,   265,   155,   204,   248,
     198,   439,   235,   249,   440,   402,   456,   445,   457,   465,
     440,   458,   279,   209,   246,   210,   417,   418,   419,   420,
     421,   210,   210,   210,   210,   210,   210,   210,   210,   210,
     210,   210,   189,   190,   266,   267,   191,   192,   193,   194,
     272,   371,   372,   373,   268,   269,   270,   271,   257,   258,
     476,   341,   342,   343,   344,   273,   469,   274,   111,   440,
     111,   282,   111,   111,   422,   423,   424,   425,   478,   484,
     502,   279,   279,   279,   275,   414,   111,   493,   259,   260,
     261,   297,   111,   111,   288,   289,   111,   293,   294,   346,
     210,   406,   279,   407,   279,   443,   279,   446,   279,   279,
     505,   302,   304,   111,   331,   348,   345,   351,   362,   448,
     347,   450,   354,   453,   360,   369,   352,   397,   408,   401,
     437,   447,   451,   205,   454,   461,    41,   111,    41,   111,
      41,   462,   466,   279,   473,   467,   474,   471,   449,   477,
     479,    48,   483,    48,   410,    48,   210,   504,   506,   508,
     459,   510,   487,   515,   403,   441,   426,   428,   427,   111,
     514,   364,   111,   429,   463,   430,   365,    41,   240,   486,
     499,     0,   507,     0,     0,   111,     0,     0,   488,     0,
     111,     0,    48,     0,     0,     0,     0,     0,     0,     0,
     494,     0,     0,     0,     0,    41,     0,   501,     0,   500,
       0,    41,   485,     0,   111,     0,   111,    41,     0,     0,
      48,     0,     0,     0,    41,   509,    48,   511,   512,   512,
     495,     0,    48,   517,   516,     0,   518,   518,     0,    48,
       0,     0,    41,     0,    41,    41,    41,     0,     0,     0,
      41,     0,     0,    41,    41,     0,     0,    48,     0,    48,
      48,    48,     0,     0,     0,    48,     0,     0,    48,    48,
       0,   111,   415,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   415,   415,   111,
       0,     0,     0,     0,     0,     0,     0,   111,     0,     0,
     106,     0,     1,     2,     3,     4,     5,     0,     0,     0,
       0,    87,     0,     0,     0,    88,     0,    14,    15,     0,
      17,     0,     0,     0,     0,    20,     0,     0,     0,     0,
       0,     0,     0,     0,   111,     0,     0,     0,     0,     0,
     111,     0,   111,    25,    26,   111,   111,   415,     0,     0,
     111,    27,     0,     0,     0,     0,     0,     0,     0,    89,
      29,    30,    31,    90,     0,    33,     0,    34,     0,     0,
       0,     0,    35,    36,    37,    38,     0,     0,     0,     0,
       0,     0,   162,   163,   111,   107,     0,   164,   165,   166,
     167,   168,   169,   170,   171,   172,   173,   174,   111,     0,
       0,     0,     0,   415,   111,     0,     0,     0,     0,     0,
       0,   111,     0,     0,     0,     0,   111,     1,     2,     3,
       4,     5,     6,   175,   111,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,     0,     0,
      20,    21,    22,     0,    23,    24,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    25,    26,
       0,     0,     0,     0,     0,     0,    27,     0,     0,     0,
       0,     0,     0,     0,    28,    29,    30,    31,    32,   144,
      33,     0,    34,     0,     0,     0,     0,    35,    36,    37,
      38,     0,     0,     1,     2,     3,     4,     5,     6,     0,
      39,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,     0,     0,    20,    21,    22,     0,
      23,    24,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    25,    26,     0,     0,     0,     0,
       0,     0,    27,     0,     0,     0,     0,     0,     0,     0,
      28,    29,    30,    31,    32,   291,    33,     0,    34,     0,
       0,     0,     0,    35,    36,    37,    38,     0,     0,     1,
       2,     3,     4,     5,     6,     0,    39,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
       0,     0,    20,    21,    22,     0,    23,    24,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      25,    26,     0,     0,     0,     0,     0,     0,    27,     0,
       0,     0,     0,     0,     0,     0,    28,    29,    30,    31,
      32,   470,    33,     0,    34,     0,     0,     0,     0,    35,
      36,    37,    38,     0,     0,     1,     2,     3,     4,     5,
       6,     0,    39,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,     0,     0,    20,    21,
      22,     0,    23,    24,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    25,    26,     0,     0,
       0,     0,     0,     0,    27,     0,     0,     0,     0,     0,
       0,     0,    28,    29,    30,    31,    32,   492,    33,     0,
      34,     0,     0,     0,     0,    35,    36,    37,    38,     0,
       0,     1,     2,     3,     4,     5,     6,     0,    39,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,     0,     0,    20,    21,    22,     0,    23,    24,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    25,    26,     0,     0,     0,     0,     0,     0,
      27,     0,     0,     0,     0,     0,     0,     0,    28,    29,
      30,    31,    32,     0,    33,     0,    34,     0,     0,     0,
       0,    35,    36,    37,    38,     0,     0,     1,     2,     3,
       4,     5,     6,     0,    39,     7,     8,     9,    10,    11,
       0,    13,    14,    15,    16,    17,    18,    19,     0,     0,
      20,    21,    22,     0,    23,    24,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    25,    26,
       0,     0,     0,     0,     0,     0,    27,     0,     0,     0,
       0,     0,     0,     0,    28,    29,    30,    31,    32,     0,
      33,     0,    34,     0,     0,     0,     0,    35,    36,    37,
      38,     0,     1,     2,     3,     4,     5,     0,     0,     0,
      39,    87,   208,     0,     0,    88,     0,    14,    15,     0,
      17,     0,     0,   251,   252,    20,     0,     0,   164,   165,
     166,   167,   168,   169,   170,   171,   172,   173,   174,     0,
       0,     0,     0,    25,    26,     0,     0,     0,     0,     0,
       0,    27,     0,     0,     0,     0,     0,     0,     0,    89,
      29,    30,    31,    90,   175,    33,     0,    34,     0,     0,
       0,     0,    35,    36,    37,    38,     1,     2,     3,     4,
       5,     0,     0,     0,     0,    87,     0,     0,     0,    88,
       0,    14,    15,     0,    17,     0,     0,     0,     0,    20,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    25,    26,     0,
       0,     0,     0,     0,     0,    27,     0,     0,     0,     0,
       0,     0,     0,    89,    29,    30,    31,    90,     0,    33,
       0,    34,   295,     0,     0,     0,    35,    36,    37,    38,
       1,     2,     3,     4,     5,     0,     0,     0,     0,    87,
       0,     0,     0,    88,     0,    14,    15,     0,    17,     0,
       0,     0,     0,    20,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    25,    26,     0,     0,     0,     0,     0,     0,    27,
       0,     0,     0,     0,     0,     0,     0,    89,    29,    30,
      31,    90,     0,    33,   298,    34,     0,     0,     0,     0,
      35,    36,    37,    38,     1,     2,     3,     4,     5,     0,
       0,     0,     0,    87,     0,     0,     0,    88,     0,    14,
      15,     0,    17,     0,     0,     0,     0,    20,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    25,    26,     0,     0,     0,
       0,     0,     0,    27,     0,     0,     0,     0,     0,     0,
       0,    89,    29,    30,    31,    90,     0,    33,     0,    34,
     455,     0,     0,     0,    35,    36,    37,    38,     1,     2,
       3,     4,     5,     0,     0,     0,     0,    87,     0,     0,
       0,    88,     0,    14,    15,     0,    17,     0,     0,     0,
       0,    20,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    25,
      26,     0,     0,     0,     0,     0,     0,    27,     0,     0,
       0,     0,     0,     0,     0,    89,    29,    30,    31,    90,
       0,    33,     0,    34,     0,     0,     0,     0,    35,    36,
      37,    38
};

static const short yycheck[] =
{
       0,    18,    32,    81,   246,    45,     1,    10,   195,    67,
      24,   366,    70,    70,    46,     0,    42,     1,    18,     1,
       1,    42,   149,   335,    13,     1,     1,    66,   155,    86,
       1,     1,    32,    18,    23,    60,    86,     1,   350,   351,
     191,   192,   193,   194,    33,    67,     8,    32,    70,   176,
     182,   183,   184,    93,     8,     6,     7,   196,    84,    42,
      64,    67,    65,    84,    70,    60,   197,   145,   108,    24,
     110,    14,    15,   200,   198,   202,    60,    20,   199,   111,
     435,    81,    25,    26,   439,   272,    29,    30,    70,    70,
     445,    86,    35,    36,    37,    38,    81,    60,   410,    70,
      70,    84,    86,   413,    86,    86,    70,    60,   235,    60,
      86,    86,   354,    66,   469,    86,    86,   268,   269,   270,
     271,    60,    86,    71,   141,    87,   253,   259,   260,   261,
      85,   180,   181,    87,   273,   185,   186,   187,   188,   189,
     190,   141,   131,   274,   133,   145,   135,   136,    65,    70,
     277,   275,   279,    70,   466,   276,   141,   467,    67,    66,
     145,    70,     3,     4,     5,     6,     7,   156,    24,    63,
     159,    12,     6,     7,    60,    16,    66,   487,    60,    60,
      21,    67,   222,    77,    78,    67,    67,    66,    44,    45,
      25,    44,    45,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    67,    40,    41,    70,   257,   258,
      13,    52,   262,   263,   264,   265,   266,   267,    67,    60,
      23,    70,    63,    64,    63,    66,    60,    68,    66,    85,
      33,    65,    82,   360,   177,   178,   179,    81,    77,    78,
      66,    83,    68,   232,    79,    80,    72,    24,    25,    24,
      25,    66,    66,    68,    68,    73,    74,    72,    72,   248,
      46,    47,    48,    40,    41,    40,    41,    66,     0,    68,
      43,    67,    85,    72,    70,   289,   403,    67,   405,    67,
      70,   408,    70,    86,    66,   335,   336,   337,   338,   339,
     340,   341,   342,   343,   344,   345,   346,   347,   348,   349,
     350,   351,    79,    80,    79,    80,    36,    37,    38,    39,
      81,   254,   255,   256,    36,    37,    38,    39,    73,    74,
     447,    36,    37,    38,    39,    82,    67,    83,   131,    70,
     133,    23,   135,   136,   341,   342,   343,   344,    67,    67,
      67,    70,    70,    70,    43,   334,   149,   474,    46,    47,
      48,    70,   155,   156,    32,    33,   159,    69,    70,    82,
     410,    69,    70,    69,    70,    69,    70,    69,    70,    70,
      71,    60,    60,   176,    60,    43,    81,    70,    60,   396,
      83,   398,    66,   400,    71,    60,    86,    66,    71,    66,
      64,    71,    64,   471,    60,    24,   396,   200,   398,   202,
     400,    60,    71,    70,    60,    86,    71,   437,   397,    35,
       9,   396,    67,   398,    85,   400,   466,    67,    71,    33,
     409,    67,    86,    65,   294,   359,   345,   347,   346,   232,
     506,   238,   235,   348,   412,   349,   242,   437,    99,   462,
     481,    -1,   498,    -1,    -1,   248,    -1,    -1,   465,    -1,
     253,    -1,   437,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     477,    -1,    -1,    -1,    -1,   465,    -1,   484,    -1,   483,
      -1,   471,   461,    -1,   277,    -1,   279,   477,    -1,    -1,
     465,    -1,    -1,    -1,   484,   502,   471,   504,   505,   506,
     479,    -1,   477,   510,   508,    -1,   513,   514,    -1,   484,
      -1,    -1,   502,    -1,   504,   505,   506,    -1,    -1,    -1,
     510,    -1,    -1,   513,   514,    -1,    -1,   502,    -1,   504,
     505,   506,    -1,    -1,    -1,   510,    -1,    -1,   513,   514,
      -1,   334,   335,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   350,   351,   352,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   360,    -1,    -1,
       1,    -1,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    12,    -1,    -1,    -1,    16,    -1,    18,    19,    -1,
      21,    -1,    -1,    -1,    -1,    26,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   397,    -1,    -1,    -1,    -1,    -1,
     403,    -1,   405,    44,    45,   408,   409,   410,    -1,    -1,
     413,    52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    60,
      61,    62,    63,    64,    -1,    66,    -1,    68,    -1,    -1,
      -1,    -1,    73,    74,    75,    76,    -1,    -1,    -1,    -1,
      -1,    -1,    44,    45,   447,    86,    -1,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,   461,    -1,
      -1,    -1,    -1,   466,   467,    -1,    -1,    -1,    -1,    -1,
      -1,   474,    -1,    -1,    -1,    -1,   479,     3,     4,     5,
       6,     7,     8,    85,   487,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    -1,    -1,
      26,    27,    28,    -1,    30,    31,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    44,    45,
      -1,    -1,    -1,    -1,    -1,    -1,    52,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    60,    61,    62,    63,    64,    65,
      66,    -1,    68,    -1,    -1,    -1,    -1,    73,    74,    75,
      76,    -1,    -1,     3,     4,     5,     6,     7,     8,    -1,
      86,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    -1,    -1,    26,    27,    28,    -1,
      30,    31,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    44,    45,    -1,    -1,    -1,    -1,
      -1,    -1,    52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      60,    61,    62,    63,    64,    65,    66,    -1,    68,    -1,
      -1,    -1,    -1,    73,    74,    75,    76,    -1,    -1,     3,
       4,     5,     6,     7,     8,    -1,    86,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      -1,    -1,    26,    27,    28,    -1,    30,    31,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      44,    45,    -1,    -1,    -1,    -1,    -1,    -1,    52,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    60,    61,    62,    63,
      64,    65,    66,    -1,    68,    -1,    -1,    -1,    -1,    73,
      74,    75,    76,    -1,    -1,     3,     4,     5,     6,     7,
       8,    -1,    86,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    -1,    -1,    26,    27,
      28,    -1,    30,    31,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    44,    45,    -1,    -1,
      -1,    -1,    -1,    -1,    52,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    60,    61,    62,    63,    64,    65,    66,    -1,
      68,    -1,    -1,    -1,    -1,    73,    74,    75,    76,    -1,
      -1,     3,     4,     5,     6,     7,     8,    -1,    86,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    -1,    -1,    26,    27,    28,    -1,    30,    31,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    44,    45,    -1,    -1,    -1,    -1,    -1,    -1,
      52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    60,    61,
      62,    63,    64,    -1,    66,    -1,    68,    -1,    -1,    -1,
      -1,    73,    74,    75,    76,    -1,    -1,     3,     4,     5,
       6,     7,     8,    -1,    86,    11,    12,    13,    14,    15,
      -1,    17,    18,    19,    20,    21,    22,    23,    -1,    -1,
      26,    27,    28,    -1,    30,    31,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    44,    45,
      -1,    -1,    -1,    -1,    -1,    -1,    52,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    60,    61,    62,    63,    64,    -1,
      66,    -1,    68,    -1,    -1,    -1,    -1,    73,    74,    75,
      76,    -1,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      86,    12,    13,    -1,    -1,    16,    -1,    18,    19,    -1,
      21,    -1,    -1,    44,    45,    26,    -1,    -1,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    -1,
      -1,    -1,    -1,    44,    45,    -1,    -1,    -1,    -1,    -1,
      -1,    52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    60,
      61,    62,    63,    64,    85,    66,    -1,    68,    -1,    -1,
      -1,    -1,    73,    74,    75,    76,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    12,    -1,    -1,    -1,    16,
      -1,    18,    19,    -1,    21,    -1,    -1,    -1,    -1,    26,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    44,    45,    -1,
      -1,    -1,    -1,    -1,    -1,    52,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    60,    61,    62,    63,    64,    -1,    66,
      -1,    68,    69,    -1,    -1,    -1,    73,    74,    75,    76,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    12,
      -1,    -1,    -1,    16,    -1,    18,    19,    -1,    21,    -1,
      -1,    -1,    -1,    26,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    44,    45,    -1,    -1,    -1,    -1,    -1,    -1,    52,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    60,    61,    62,
      63,    64,    -1,    66,    67,    68,    -1,    -1,    -1,    -1,
      73,    74,    75,    76,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    12,    -1,    -1,    -1,    16,    -1,    18,
      19,    -1,    21,    -1,    -1,    -1,    -1,    26,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    44,    45,    -1,    -1,    -1,
      -1,    -1,    -1,    52,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    60,    61,    62,    63,    64,    -1,    66,    -1,    68,
      69,    -1,    -1,    -1,    73,    74,    75,    76,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    12,    -1,    -1,
      -1,    16,    -1,    18,    19,    -1,    21,    -1,    -1,    -1,
      -1,    26,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    44,
      45,    -1,    -1,    -1,    -1,    -1,    -1,    52,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    60,    61,    62,    63,    64,
      -1,    66,    -1,    68,    -1,    -1,    -1,    -1,    73,    74,
      75,    76
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,     3,     4,     5,     6,     7,     8,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      26,    27,    28,    30,    31,    44,    45,    52,    60,    61,
      62,    63,    64,    66,    68,    73,    74,    75,    76,    86,
      88,    90,    91,    98,   100,   102,   106,   108,   109,   111,
     113,   115,   117,   120,   123,   126,   129,   132,   135,   138,
     141,   144,   148,   149,   150,   152,   157,   162,   163,   164,
     165,   168,   169,   170,   171,   172,   178,   179,   180,   181,
     185,   186,   187,     1,    60,    86,    66,    12,    16,    60,
      64,    89,    90,    97,    99,   182,    60,   153,   155,    60,
     158,   159,     1,    60,    86,    60,     1,    86,    97,    99,
     101,   105,   107,   109,   110,   112,   114,   116,   118,   121,
     124,   127,   130,   133,   136,   139,   142,   146,   105,   110,
     110,    66,   149,    66,   110,    66,    66,   146,   150,   110,
     110,    71,   110,   110,    65,   186,   146,    70,    92,    93,
      94,   110,   110,   110,   110,    66,    68,    72,   103,    68,
      72,   103,    44,    45,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    85,   145,    63,    77,    78,
      73,    74,    46,    47,    48,    24,    25,    40,    41,    79,
      80,    36,    37,    38,    39,    81,    82,    83,    43,    42,
      84,     1,    70,    86,     0,   187,     1,    86,    13,   105,
     116,   119,   122,   125,   128,   131,   134,   137,   140,   143,
     147,   167,    97,    99,    60,    66,     6,     7,    60,    65,
      95,    96,    68,    72,   103,    85,   160,     1,    70,    86,
     160,     1,    70,    86,     1,    86,    66,   103,    68,    72,
     103,    44,    45,   145,    63,    77,    78,    73,    74,    46,
      47,    48,    24,    25,    40,    41,    79,    80,    36,    37,
      38,    39,    81,    82,    83,    43,    42,    84,     1,    70,
      86,   146,    23,   146,   146,   146,     1,    86,    32,    33,
     149,    65,    67,    69,    70,    69,   142,    70,    67,   104,
     142,   146,    60,   146,    60,   142,   110,   110,   110,   112,
     112,   114,   114,   114,   116,   116,   116,   116,   116,   116,
     118,   118,   118,   118,   121,   124,   127,   130,   133,   142,
     142,    60,   154,   156,    24,   145,    25,    40,    41,    79,
      80,    36,    37,    38,    39,    81,    82,    83,    43,    42,
      84,    70,    86,   103,    66,    60,    67,   183,    65,    70,
      71,   146,    60,   142,   155,   159,    67,   183,   146,    60,
     142,   110,   110,   110,   112,   112,   114,   114,   114,   116,
     116,   116,   116,   116,   116,   118,   118,   118,   118,   121,
     124,   127,   130,   133,   142,   142,    67,    66,    67,    67,
      67,    66,   150,    93,    67,    70,    69,    69,    71,    24,
      85,   161,    70,    86,   146,   105,   143,   116,   116,   116,
     116,   116,   119,   119,   119,   119,   122,   125,   128,   131,
     134,   143,   143,   146,   166,    67,   183,    64,   184,    67,
      70,    96,   142,    69,   184,    67,    69,    71,   149,   146,
     149,    64,   173,   149,    60,    69,   142,   142,   142,   146,
     143,    24,    60,   156,   166,    67,    71,    86,   184,    67,
      65,   186,   184,    60,    71,   184,   142,    35,    67,     9,
     174,   175,   176,    67,    67,   146,   161,    86,   149,   143,
     166,   184,    65,   142,   149,   146,    10,    65,   177,   176,
     150,   149,    67,   166,    67,    71,    71,   174,    33,   149,
      67,   149,   149,   151,   151,    65,   150,   149,   149
};

#if ! defined (YYSIZE_T) && defined (__SIZE_TYPE__)
# define YYSIZE_T __SIZE_TYPE__
#endif
#if ! defined (YYSIZE_T) && defined (size_t)
# define YYSIZE_T size_t
#endif
#if ! defined (YYSIZE_T)
# if defined (__STDC__) || defined (__cplusplus)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# endif
#endif
#if ! defined (YYSIZE_T)
# define YYSIZE_T unsigned int
#endif

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrlab1

/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { 								\
      yyerror ("syntax error: cannot back up");\
      YYERROR;							\
    }								\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

/* YYLLOC_DEFAULT -- Compute the default location (before the actions
   are run).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)         \
  Current.first_line   = Rhs[1].first_line;      \
  Current.first_column = Rhs[1].first_column;    \
  Current.last_line    = Rhs[N].last_line;       \
  Current.last_column  = Rhs[N].last_column;
#endif

/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (0)

# define YYDSYMPRINT(Args)			\
do {						\
  if (yydebug)					\
    yysymprint Args;				\
} while (0)

# define YYDSYMPRINTF(Title, Token, Value, Location)		\
do {								\
  if (yydebug)							\
    {								\
      YYFPRINTF (stderr, "%s ", Title);				\
      yysymprint (stderr, 					\
                  Token, Value, Location);	\
      YYFPRINTF (stderr, "\n");					\
    }								\
} while (0)

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (cinluded).                                                   |
`------------------------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_stack_print (short *bottom, short *top)
#else
static void
yy_stack_print (bottom, top)
    short *bottom;
    short *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (/* Nothing. */; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_reduce_print (int yyrule)
#else
static void
yy_reduce_print (yyrule)
    int yyrule;
#endif
{
  int yyi;
  unsigned int yylineno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %u), ",
             yyrule - 1, yylineno);
  /* Print the symbols being reduced, and their result.  */
  for (yyi = yyprhs[yyrule]; 0 <= yyrhs[yyi]; yyi++)
    YYFPRINTF (stderr, "%s ", yytname [yyrhs[yyi]]);
  YYFPRINTF (stderr, "-> %s\n", yytname [yyr1[yyrule]]);
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (Rule);		\
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YYDSYMPRINT(Args)
# define YYDSYMPRINTF(Title, Token, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   SIZE_MAX < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#if YYMAXDEPTH == 0
# undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined (__GLIBC__) && defined (_STRING_H)
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
#   if defined (__STDC__) || defined (__cplusplus)
yystrlen (const char *yystr)
#   else
yystrlen (yystr)
     const char *yystr;
#   endif
{
  register const char *yys = yystr;

  while (*yys++ != '\0')
    continue;

  return yys - yystr - 1;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined (__GLIBC__) && defined (_STRING_H) && defined (_GNU_SOURCE)
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
#   if defined (__STDC__) || defined (__cplusplus)
yystpcpy (char *yydest, const char *yysrc)
#   else
yystpcpy (yydest, yysrc)
     char *yydest;
     const char *yysrc;
#   endif
{
  register char *yyd = yydest;
  register const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

#endif /* !YYERROR_VERBOSE */



#if YYDEBUG
/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yysymprint (FILE *yyoutput, int yytype, YYSTYPE *yyvaluep, YYLTYPE *yylocationp)
#else
static void
yysymprint (yyoutput, yytype, yyvaluep, yylocationp)
    FILE *yyoutput;
    int yytype;
    YYSTYPE *yyvaluep;
    YYLTYPE *yylocationp;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;
  (void) yylocationp;

  if (yytype < YYNTOKENS)
    {
      YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
# ifdef YYPRINT
      YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
    }
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  switch (yytype)
    {
      default:
        break;
    }
  YYFPRINTF (yyoutput, ")");
}

#endif /* ! YYDEBUG */
/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yydestruct (int yytype, YYSTYPE *yyvaluep, YYLTYPE *yylocationp)
#else
static void
yydestruct (yytype, yyvaluep, yylocationp)
    int yytype;
    YYSTYPE *yyvaluep;
    YYLTYPE *yylocationp;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;
  (void) yylocationp;

  switch (yytype)
    {

      default:
        break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM);
# else
int yyparse ();
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */



/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;
/* Location data for the lookahead symbol.  */
YYLTYPE yylloc;



/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM)
# else
int yyparse (YYPARSE_PARAM)
  void *YYPARSE_PARAM;
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  
  register int yystate;
  register int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  short	yyssa[YYINITDEPTH];
  short *yyss = yyssa;
  register short *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  register YYSTYPE *yyvsp;

  /* The location stack.  */
  YYLTYPE yylsa[YYINITDEPTH];
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;
  YYLTYPE *yylerrsp;

#define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
  YYLTYPE yyloc;

  /* When reducing, the number of symbols on the RHS of the reduced
     rule.  */
  int yylen;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;
  yylsp = yyls;
  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed. so pushing a state here evens the stacks.
     */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack. Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	short *yyss1 = yyss;
	YYLTYPE *yyls1 = yyls;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow ("parser stack overflow",
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yyls1, yysize * sizeof (*yylsp),
		    &yystacksize);
	yyls = yyls1;
	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyoverflowlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyoverflowlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	short *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyoverflowlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);
	YYSTACK_RELOCATE (yyls);
#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
      yylsp = yyls + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YYDSYMPRINTF ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */
  YYDPRINTF ((stderr, "Shifting token %s, ", yytname[yytoken]));

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;
  *++yylsp = yylloc;

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  yystate = yyn;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];

  /* Default location. */
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 188 "grammar.y"
    { yyval.node = new NullNode(); ;}
    break;

  case 3:
#line 189 "grammar.y"
    { yyval.node = new BooleanNode(true); ;}
    break;

  case 4:
#line 190 "grammar.y"
    { yyval.node = new BooleanNode(false); ;}
    break;

  case 5:
#line 191 "grammar.y"
    { yyval.node = new NumberNode(yyvsp[0].dval); ;}
    break;

  case 6:
#line 192 "grammar.y"
    { yyval.node = new StringNode(yyvsp[0].ustr); ;}
    break;

  case 7:
#line 193 "grammar.y"
    {
                                            Lexer *l = Lexer::curr();
                                            if (!l->scanRegExp()) YYABORT;
                                            yyval.node = new RegExpNode(l->pattern, l->flags);
                                        ;}
    break;

  case 8:
#line 198 "grammar.y"
    {
                                            Lexer *l = Lexer::curr();
                                            if (!l->scanRegExp()) YYABORT;
                                            yyval.node = new RegExpNode(UString('=') + l->pattern, l->flags);
                                        ;}
    break;

  case 10:
#line 207 "grammar.y"
    { yyval.node = new ObjectLiteralNode(); ;}
    break;

  case 11:
#line 208 "grammar.y"
    { yyval.node = new ObjectLiteralNode(yyvsp[-1].plist); ;}
    break;

  case 12:
#line 212 "grammar.y"
    { yyval.node = new ThisNode(); ;}
    break;

  case 15:
#line 215 "grammar.y"
    { yyval.node = new ResolveNode(*yyvsp[0].ident); ;}
    break;

  case 16:
#line 216 "grammar.y"
    { yyval.node = yyvsp[-1].node->isResolveNode() ? yyvsp[-1].node : new GroupNode(yyvsp[-1].node); ;}
    break;

  case 17:
#line 220 "grammar.y"
    { yyval.node = new ArrayNode(yyvsp[-1].ival); ;}
    break;

  case 18:
#line 221 "grammar.y"
    { yyval.node = new ArrayNode(yyvsp[-1].elm); ;}
    break;

  case 19:
#line 222 "grammar.y"
    { yyval.node = new ArrayNode(yyvsp[-1].ival, yyvsp[-3].elm); ;}
    break;

  case 20:
#line 226 "grammar.y"
    { yyval.elm = new ElementNode(yyvsp[-1].ival, yyvsp[0].node); ;}
    break;

  case 21:
#line 228 "grammar.y"
    { yyval.elm = new ElementNode(yyvsp[-3].elm, yyvsp[-1].ival, yyvsp[0].node); ;}
    break;

  case 22:
#line 232 "grammar.y"
    { yyval.ival = 0; ;}
    break;

  case 24:
#line 237 "grammar.y"
    { yyval.ival = 1; ;}
    break;

  case 25:
#line 238 "grammar.y"
    { yyval.ival = yyvsp[-1].ival + 1; ;}
    break;

  case 26:
#line 242 "grammar.y"
    { yyval.plist = new PropertyValueNode(yyvsp[-2].pnode, yyvsp[0].node); ;}
    break;

  case 27:
#line 244 "grammar.y"
    { yyval.plist = new PropertyValueNode(yyvsp[-2].pnode, yyvsp[0].node, yyvsp[-4].plist); ;}
    break;

  case 28:
#line 248 "grammar.y"
    { yyval.pnode = new PropertyNode(*yyvsp[0].ident); ;}
    break;

  case 29:
#line 249 "grammar.y"
    { yyval.pnode = new PropertyNode(Identifier(*yyvsp[0].ustr)); ;}
    break;

  case 30:
#line 250 "grammar.y"
    { yyval.pnode = new PropertyNode(yyvsp[0].dval); ;}
    break;

  case 32:
#line 255 "grammar.y"
    { yyval.node = yyvsp[0].funcExpr; ;}
    break;

  case 33:
#line 256 "grammar.y"
    { yyval.node = new BracketAccessorNode(yyvsp[-3].node, yyvsp[-1].node); ;}
    break;

  case 34:
#line 257 "grammar.y"
    { yyval.node = new DotAccessorNode(yyvsp[-2].node, *yyvsp[0].ident); ;}
    break;

  case 35:
#line 258 "grammar.y"
    { yyval.node = new NewExprNode(yyvsp[-1].node, yyvsp[0].args); ;}
    break;

  case 37:
#line 263 "grammar.y"
    { yyval.node = new BracketAccessorNode(yyvsp[-3].node, yyvsp[-1].node); ;}
    break;

  case 38:
#line 264 "grammar.y"
    { yyval.node = new DotAccessorNode(yyvsp[-2].node, *yyvsp[0].ident); ;}
    break;

  case 39:
#line 265 "grammar.y"
    { yyval.node = new NewExprNode(yyvsp[-1].node, yyvsp[0].args); ;}
    break;

  case 41:
#line 270 "grammar.y"
    { yyval.node = new NewExprNode(yyvsp[0].node); ;}
    break;

  case 43:
#line 275 "grammar.y"
    { yyval.node = new NewExprNode(yyvsp[0].node); ;}
    break;

  case 44:
#line 279 "grammar.y"
    { yyval.node = makeFunctionCallNode(yyvsp[-1].node, yyvsp[0].args); ;}
    break;

  case 45:
#line 280 "grammar.y"
    { yyval.node = makeFunctionCallNode(yyvsp[-1].node, yyvsp[0].args); ;}
    break;

  case 46:
#line 281 "grammar.y"
    { yyval.node = new BracketAccessorNode(yyvsp[-3].node, yyvsp[-1].node); ;}
    break;

  case 47:
#line 282 "grammar.y"
    { yyval.node = new DotAccessorNode(yyvsp[-2].node, *yyvsp[0].ident); ;}
    break;

  case 48:
#line 286 "grammar.y"
    { yyval.node = makeFunctionCallNode(yyvsp[-1].node, yyvsp[0].args); ;}
    break;

  case 49:
#line 287 "grammar.y"
    { yyval.node = makeFunctionCallNode(yyvsp[-1].node, yyvsp[0].args); ;}
    break;

  case 50:
#line 288 "grammar.y"
    { yyval.node = new BracketAccessorNode(yyvsp[-3].node, yyvsp[-1].node); ;}
    break;

  case 51:
#line 289 "grammar.y"
    { yyval.node = new DotAccessorNode(yyvsp[-2].node, *yyvsp[0].ident); ;}
    break;

  case 52:
#line 293 "grammar.y"
    { yyval.args = new ArgumentsNode(); ;}
    break;

  case 53:
#line 294 "grammar.y"
    { yyval.args = new ArgumentsNode(yyvsp[-1].alist); ;}
    break;

  case 54:
#line 298 "grammar.y"
    { yyval.alist = new ArgumentListNode(yyvsp[0].node); ;}
    break;

  case 55:
#line 299 "grammar.y"
    { yyval.alist = new ArgumentListNode(yyvsp[-2].alist, yyvsp[0].node); ;}
    break;

  case 61:
#line 314 "grammar.y"
    { if (!makePostfixNode(yyval.node, yyvsp[-1].node, OpPlusPlus)) YYABORT; ;}
    break;

  case 62:
#line 315 "grammar.y"
    { if (!makePostfixNode(yyval.node, yyvsp[-1].node, OpMinusMinus)) YYABORT; ;}
    break;

  case 64:
#line 320 "grammar.y"
    { if (!makePostfixNode(yyval.node, yyvsp[-1].node, OpPlusPlus)) YYABORT; ;}
    break;

  case 65:
#line 321 "grammar.y"
    { if (!makePostfixNode(yyval.node, yyvsp[-1].node, OpMinusMinus)) YYABORT; ;}
    break;

  case 66:
#line 325 "grammar.y"
    { yyval.node = makeDeleteNode(yyvsp[0].node); ;}
    break;

  case 67:
#line 326 "grammar.y"
    { yyval.node = new VoidNode(yyvsp[0].node); ;}
    break;

  case 68:
#line 327 "grammar.y"
    { yyval.node = makeTypeOfNode(yyvsp[0].node); ;}
    break;

  case 69:
#line 328 "grammar.y"
    { if (!makePrefixNode(yyval.node, yyvsp[0].node, OpPlusPlus)) YYABORT; ;}
    break;

  case 70:
#line 329 "grammar.y"
    { if (!makePrefixNode(yyval.node, yyvsp[0].node, OpPlusPlus)) YYABORT; ;}
    break;

  case 71:
#line 330 "grammar.y"
    { if (!makePrefixNode(yyval.node, yyvsp[0].node, OpMinusMinus)) YYABORT; ;}
    break;

  case 72:
#line 331 "grammar.y"
    { if (!makePrefixNode(yyval.node, yyvsp[0].node, OpMinusMinus)) YYABORT; ;}
    break;

  case 73:
#line 332 "grammar.y"
    { yyval.node = new UnaryPlusNode(yyvsp[0].node); ;}
    break;

  case 74:
#line 333 "grammar.y"
    { yyval.node = new NegateNode(yyvsp[0].node); ;}
    break;

  case 75:
#line 334 "grammar.y"
    { yyval.node = new BitwiseNotNode(yyvsp[0].node); ;}
    break;

  case 76:
#line 335 "grammar.y"
    { yyval.node = new LogicalNotNode(yyvsp[0].node); ;}
    break;

  case 82:
#line 349 "grammar.y"
    { yyval.node = new MultNode(yyvsp[-2].node, yyvsp[0].node, '*'); ;}
    break;

  case 83:
#line 350 "grammar.y"
    { yyval.node = new MultNode(yyvsp[-2].node, yyvsp[0].node, '/'); ;}
    break;

  case 84:
#line 351 "grammar.y"
    { yyval.node = new MultNode(yyvsp[-2].node, yyvsp[0].node,'%'); ;}
    break;

  case 86:
#line 357 "grammar.y"
    { yyval.node = new MultNode(yyvsp[-2].node, yyvsp[0].node, '*'); ;}
    break;

  case 87:
#line 359 "grammar.y"
    { yyval.node = new MultNode(yyvsp[-2].node, yyvsp[0].node, '/'); ;}
    break;

  case 88:
#line 361 "grammar.y"
    { yyval.node = new MultNode(yyvsp[-2].node, yyvsp[0].node,'%'); ;}
    break;

  case 90:
#line 366 "grammar.y"
    { yyval.node = new AddNode(yyvsp[-2].node, yyvsp[0].node, '+'); ;}
    break;

  case 91:
#line 367 "grammar.y"
    { yyval.node = new AddNode(yyvsp[-2].node, yyvsp[0].node, '-'); ;}
    break;

  case 93:
#line 373 "grammar.y"
    { yyval.node = new AddNode(yyvsp[-2].node, yyvsp[0].node, '+'); ;}
    break;

  case 94:
#line 375 "grammar.y"
    { yyval.node = new AddNode(yyvsp[-2].node, yyvsp[0].node, '-'); ;}
    break;

  case 96:
#line 380 "grammar.y"
    { yyval.node = new ShiftNode(yyvsp[-2].node, OpLShift, yyvsp[0].node); ;}
    break;

  case 97:
#line 381 "grammar.y"
    { yyval.node = new ShiftNode(yyvsp[-2].node, OpRShift, yyvsp[0].node); ;}
    break;

  case 98:
#line 382 "grammar.y"
    { yyval.node = new ShiftNode(yyvsp[-2].node, OpURShift, yyvsp[0].node); ;}
    break;

  case 100:
#line 387 "grammar.y"
    { yyval.node = new ShiftNode(yyvsp[-2].node, OpLShift, yyvsp[0].node); ;}
    break;

  case 101:
#line 388 "grammar.y"
    { yyval.node = new ShiftNode(yyvsp[-2].node, OpRShift, yyvsp[0].node); ;}
    break;

  case 102:
#line 389 "grammar.y"
    { yyval.node = new ShiftNode(yyvsp[-2].node, OpURShift, yyvsp[0].node); ;}
    break;

  case 104:
#line 394 "grammar.y"
    { yyval.node = new RelationalNode(yyvsp[-2].node, OpLess, yyvsp[0].node); ;}
    break;

  case 105:
#line 395 "grammar.y"
    { yyval.node = new RelationalNode(yyvsp[-2].node, OpGreater, yyvsp[0].node); ;}
    break;

  case 106:
#line 396 "grammar.y"
    { yyval.node = new RelationalNode(yyvsp[-2].node, OpLessEq, yyvsp[0].node); ;}
    break;

  case 107:
#line 397 "grammar.y"
    { yyval.node = new RelationalNode(yyvsp[-2].node, OpGreaterEq, yyvsp[0].node); ;}
    break;

  case 108:
#line 398 "grammar.y"
    { yyval.node = new RelationalNode(yyvsp[-2].node, OpInstanceOf, yyvsp[0].node); ;}
    break;

  case 109:
#line 399 "grammar.y"
    { yyval.node = new RelationalNode(yyvsp[-2].node, OpIn, yyvsp[0].node); ;}
    break;

  case 111:
#line 404 "grammar.y"
    { yyval.node = new RelationalNode(yyvsp[-2].node, OpLess, yyvsp[0].node); ;}
    break;

  case 112:
#line 405 "grammar.y"
    { yyval.node = new RelationalNode(yyvsp[-2].node, OpGreater, yyvsp[0].node); ;}
    break;

  case 113:
#line 406 "grammar.y"
    { yyval.node = new RelationalNode(yyvsp[-2].node, OpLessEq, yyvsp[0].node); ;}
    break;

  case 114:
#line 407 "grammar.y"
    { yyval.node = new RelationalNode(yyvsp[-2].node, OpGreaterEq, yyvsp[0].node); ;}
    break;

  case 115:
#line 409 "grammar.y"
    { yyval.node = new RelationalNode(yyvsp[-2].node, OpInstanceOf, yyvsp[0].node); ;}
    break;

  case 117:
#line 414 "grammar.y"
    { yyval.node = new RelationalNode(yyvsp[-2].node, OpLess, yyvsp[0].node); ;}
    break;

  case 118:
#line 415 "grammar.y"
    { yyval.node = new RelationalNode(yyvsp[-2].node, OpGreater, yyvsp[0].node); ;}
    break;

  case 119:
#line 416 "grammar.y"
    { yyval.node = new RelationalNode(yyvsp[-2].node, OpLessEq, yyvsp[0].node); ;}
    break;

  case 120:
#line 417 "grammar.y"
    { yyval.node = new RelationalNode(yyvsp[-2].node, OpGreaterEq, yyvsp[0].node); ;}
    break;

  case 121:
#line 419 "grammar.y"
    { yyval.node = new RelationalNode(yyvsp[-2].node, OpInstanceOf, yyvsp[0].node); ;}
    break;

  case 122:
#line 420 "grammar.y"
    { yyval.node = new RelationalNode(yyvsp[-2].node, OpIn, yyvsp[0].node); ;}
    break;

  case 124:
#line 425 "grammar.y"
    { yyval.node = new EqualNode(yyvsp[-2].node, OpEqEq, yyvsp[0].node); ;}
    break;

  case 125:
#line 426 "grammar.y"
    { yyval.node = new EqualNode(yyvsp[-2].node, OpNotEq, yyvsp[0].node); ;}
    break;

  case 126:
#line 427 "grammar.y"
    { yyval.node = new EqualNode(yyvsp[-2].node, OpStrEq, yyvsp[0].node); ;}
    break;

  case 127:
#line 428 "grammar.y"
    { yyval.node = new EqualNode(yyvsp[-2].node, OpStrNEq, yyvsp[0].node);;}
    break;

  case 129:
#line 434 "grammar.y"
    { yyval.node = new EqualNode(yyvsp[-2].node, OpEqEq, yyvsp[0].node); ;}
    break;

  case 130:
#line 436 "grammar.y"
    { yyval.node = new EqualNode(yyvsp[-2].node, OpNotEq, yyvsp[0].node); ;}
    break;

  case 131:
#line 438 "grammar.y"
    { yyval.node = new EqualNode(yyvsp[-2].node, OpStrEq, yyvsp[0].node); ;}
    break;

  case 132:
#line 440 "grammar.y"
    { yyval.node = new EqualNode(yyvsp[-2].node, OpStrNEq, yyvsp[0].node);;}
    break;

  case 134:
#line 446 "grammar.y"
    { yyval.node = new EqualNode(yyvsp[-2].node, OpEqEq, yyvsp[0].node); ;}
    break;

  case 135:
#line 447 "grammar.y"
    { yyval.node = new EqualNode(yyvsp[-2].node, OpNotEq, yyvsp[0].node); ;}
    break;

  case 136:
#line 449 "grammar.y"
    { yyval.node = new EqualNode(yyvsp[-2].node, OpStrEq, yyvsp[0].node); ;}
    break;

  case 137:
#line 451 "grammar.y"
    { yyval.node = new EqualNode(yyvsp[-2].node, OpStrNEq, yyvsp[0].node);;}
    break;

  case 139:
#line 456 "grammar.y"
    { yyval.node = new BitOperNode(yyvsp[-2].node, OpBitAnd, yyvsp[0].node); ;}
    break;

  case 141:
#line 462 "grammar.y"
    { yyval.node = new BitOperNode(yyvsp[-2].node, OpBitAnd, yyvsp[0].node); ;}
    break;

  case 143:
#line 467 "grammar.y"
    { yyval.node = new BitOperNode(yyvsp[-2].node, OpBitAnd, yyvsp[0].node); ;}
    break;

  case 145:
#line 472 "grammar.y"
    { yyval.node = new BitOperNode(yyvsp[-2].node, OpBitXOr, yyvsp[0].node); ;}
    break;

  case 147:
#line 478 "grammar.y"
    { yyval.node = new BitOperNode(yyvsp[-2].node, OpBitXOr, yyvsp[0].node); ;}
    break;

  case 149:
#line 484 "grammar.y"
    { yyval.node = new BitOperNode(yyvsp[-2].node, OpBitXOr, yyvsp[0].node); ;}
    break;

  case 151:
#line 489 "grammar.y"
    { yyval.node = new BitOperNode(yyvsp[-2].node, OpBitOr, yyvsp[0].node); ;}
    break;

  case 153:
#line 495 "grammar.y"
    { yyval.node = new BitOperNode(yyvsp[-2].node, OpBitOr, yyvsp[0].node); ;}
    break;

  case 155:
#line 501 "grammar.y"
    { yyval.node = new BitOperNode(yyvsp[-2].node, OpBitOr, yyvsp[0].node); ;}
    break;

  case 157:
#line 506 "grammar.y"
    { yyval.node = new BinaryLogicalNode(yyvsp[-2].node, OpAnd, yyvsp[0].node); ;}
    break;

  case 159:
#line 512 "grammar.y"
    { yyval.node = new BinaryLogicalNode(yyvsp[-2].node, OpAnd, yyvsp[0].node); ;}
    break;

  case 161:
#line 518 "grammar.y"
    { yyval.node = new BinaryLogicalNode(yyvsp[-2].node, OpAnd, yyvsp[0].node); ;}
    break;

  case 163:
#line 523 "grammar.y"
    { yyval.node = new BinaryLogicalNode(yyvsp[-2].node, OpOr, yyvsp[0].node); ;}
    break;

  case 165:
#line 529 "grammar.y"
    { yyval.node = new BinaryLogicalNode(yyvsp[-2].node, OpOr, yyvsp[0].node); ;}
    break;

  case 167:
#line 534 "grammar.y"
    { yyval.node = new BinaryLogicalNode(yyvsp[-2].node, OpOr, yyvsp[0].node); ;}
    break;

  case 169:
#line 540 "grammar.y"
    { yyval.node = new ConditionalNode(yyvsp[-4].node, yyvsp[-2].node, yyvsp[0].node); ;}
    break;

  case 171:
#line 546 "grammar.y"
    { yyval.node = new ConditionalNode(yyvsp[-4].node, yyvsp[-2].node, yyvsp[0].node); ;}
    break;

  case 173:
#line 552 "grammar.y"
    { yyval.node = new ConditionalNode(yyvsp[-4].node, yyvsp[-2].node, yyvsp[0].node); ;}
    break;

  case 175:
#line 558 "grammar.y"
    { if (!makeAssignNode(yyval.node, yyvsp[-2].node, yyvsp[-1].op, yyvsp[0].node)) YYABORT; ;}
    break;

  case 177:
#line 564 "grammar.y"
    { if (!makeAssignNode(yyval.node, yyvsp[-2].node, yyvsp[-1].op, yyvsp[0].node)) YYABORT; ;}
    break;

  case 179:
#line 570 "grammar.y"
    { if (!makeAssignNode(yyval.node, yyvsp[-2].node, yyvsp[-1].op, yyvsp[0].node)) YYABORT; ;}
    break;

  case 180:
#line 574 "grammar.y"
    { yyval.op = OpEqual; ;}
    break;

  case 181:
#line 575 "grammar.y"
    { yyval.op = OpPlusEq; ;}
    break;

  case 182:
#line 576 "grammar.y"
    { yyval.op = OpMinusEq; ;}
    break;

  case 183:
#line 577 "grammar.y"
    { yyval.op = OpMultEq; ;}
    break;

  case 184:
#line 578 "grammar.y"
    { yyval.op = OpDivEq; ;}
    break;

  case 185:
#line 579 "grammar.y"
    { yyval.op = OpLShift; ;}
    break;

  case 186:
#line 580 "grammar.y"
    { yyval.op = OpRShift; ;}
    break;

  case 187:
#line 581 "grammar.y"
    { yyval.op = OpURShift; ;}
    break;

  case 188:
#line 582 "grammar.y"
    { yyval.op = OpAndEq; ;}
    break;

  case 189:
#line 583 "grammar.y"
    { yyval.op = OpXOrEq; ;}
    break;

  case 190:
#line 584 "grammar.y"
    { yyval.op = OpOrEq; ;}
    break;

  case 191:
#line 585 "grammar.y"
    { yyval.op = OpModEq; ;}
    break;

  case 193:
#line 590 "grammar.y"
    { yyval.node = new CommaNode(yyvsp[-2].node, yyvsp[0].node); ;}
    break;

  case 195:
#line 595 "grammar.y"
    { yyval.node = new CommaNode(yyvsp[-2].node, yyvsp[0].node); ;}
    break;

  case 197:
#line 600 "grammar.y"
    { yyval.node = new CommaNode(yyvsp[-2].node, yyvsp[0].node); ;}
    break;

  case 213:
#line 622 "grammar.y"
    { yyval.stat = new BlockNode(0); DBG(yyval.stat, yylsp[0], yylsp[0]); ;}
    break;

  case 214:
#line 623 "grammar.y"
    { yyval.stat = new BlockNode(yyvsp[-1].srcs); DBG(yyval.stat, yylsp[0], yylsp[0]); ;}
    break;

  case 215:
#line 627 "grammar.y"
    { yyval.slist = new StatListNode(yyvsp[0].stat); ;}
    break;

  case 216:
#line 628 "grammar.y"
    { yyval.slist = new StatListNode(yyvsp[-1].slist, yyvsp[0].stat); ;}
    break;

  case 217:
#line 632 "grammar.y"
    { yyval.stat = new VarStatementNode(yyvsp[-1].vlist); DBG(yyval.stat, yylsp[-2], yylsp[0]); ;}
    break;

  case 218:
#line 633 "grammar.y"
    { yyval.stat = new VarStatementNode(yyvsp[-1].vlist); DBG(yyval.stat, yylsp[-2], yylsp[-1]); AUTO_SEMICOLON; ;}
    break;

  case 219:
#line 637 "grammar.y"
    { yyval.vlist = new VarDeclListNode(yyvsp[0].decl); ;}
    break;

  case 220:
#line 639 "grammar.y"
    { yyval.vlist = new VarDeclListNode(yyvsp[-2].vlist, yyvsp[0].decl); ;}
    break;

  case 221:
#line 643 "grammar.y"
    { yyval.vlist = new VarDeclListNode(yyvsp[0].decl); ;}
    break;

  case 222:
#line 645 "grammar.y"
    { yyval.vlist = new VarDeclListNode(yyvsp[-2].vlist, yyvsp[0].decl); ;}
    break;

  case 223:
#line 649 "grammar.y"
    { yyval.decl = new VarDeclNode(*yyvsp[0].ident, 0, VarDeclNode::Variable); ;}
    break;

  case 224:
#line 650 "grammar.y"
    { yyval.decl = new VarDeclNode(*yyvsp[-1].ident, yyvsp[0].init, VarDeclNode::Variable); ;}
    break;

  case 225:
#line 654 "grammar.y"
    { yyval.decl = new VarDeclNode(*yyvsp[0].ident, 0, VarDeclNode::Variable); ;}
    break;

  case 226:
#line 655 "grammar.y"
    { yyval.decl = new VarDeclNode(*yyvsp[-1].ident, yyvsp[0].init, VarDeclNode::Variable); ;}
    break;

  case 227:
#line 659 "grammar.y"
    { yyval.stat = new VarStatementNode(yyvsp[-1].vlist); DBG(yyval.stat, yylsp[-2], yylsp[0]); ;}
    break;

  case 228:
#line 660 "grammar.y"
    { yyval.stat = new VarStatementNode(yyvsp[-1].vlist); DBG(yyval.stat, yylsp[-2], yylsp[-1]); AUTO_SEMICOLON; ;}
    break;

  case 229:
#line 664 "grammar.y"
    { yyval.vlist = new VarDeclListNode(yyvsp[0].decl); ;}
    break;

  case 230:
#line 666 "grammar.y"
    { yyval.vlist = new VarDeclListNode(yyvsp[-2].vlist, yyvsp[0].decl); ;}
    break;

  case 231:
#line 670 "grammar.y"
    { yyval.decl = new VarDeclNode(*yyvsp[0].ident, 0, VarDeclNode::Constant); ;}
    break;

  case 232:
#line 671 "grammar.y"
    { yyval.decl = new VarDeclNode(*yyvsp[-1].ident, yyvsp[0].init, VarDeclNode::Constant); ;}
    break;

  case 233:
#line 675 "grammar.y"
    { yyval.init = new AssignExprNode(yyvsp[0].node); ;}
    break;

  case 234:
#line 679 "grammar.y"
    { yyval.init = new AssignExprNode(yyvsp[0].node); ;}
    break;

  case 235:
#line 683 "grammar.y"
    { yyval.stat = new EmptyStatementNode(); ;}
    break;

  case 236:
#line 687 "grammar.y"
    { yyval.stat = new ExprStatementNode(yyvsp[-1].node); DBG(yyval.stat, yylsp[-1], yylsp[0]); ;}
    break;

  case 237:
#line 688 "grammar.y"
    { yyval.stat = new ExprStatementNode(yyvsp[-1].node); DBG(yyval.stat, yylsp[-1], yylsp[-1]); AUTO_SEMICOLON; ;}
    break;

  case 238:
#line 693 "grammar.y"
    { yyval.stat = new IfNode(yyvsp[-2].node, yyvsp[0].stat, 0); DBG(yyval.stat, yylsp[-4], yylsp[-1]); ;}
    break;

  case 239:
#line 695 "grammar.y"
    { yyval.stat = new IfNode(yyvsp[-4].node, yyvsp[-2].stat, yyvsp[0].stat); DBG(yyval.stat, yylsp[-6], yylsp[-3]); ;}
    break;

  case 240:
#line 699 "grammar.y"
    { yyval.stat = new DoWhileNode(yyvsp[-4].stat, yyvsp[-1].node); DBG(yyval.stat, yylsp[-5], yylsp[-3]);;}
    break;

  case 241:
#line 700 "grammar.y"
    { yyval.stat = new WhileNode(yyvsp[-2].node, yyvsp[0].stat); DBG(yyval.stat, yylsp[-4], yylsp[-1]); ;}
    break;

  case 242:
#line 702 "grammar.y"
    { yyval.stat = new ForNode(yyvsp[-6].node, yyvsp[-4].node, yyvsp[-2].node, yyvsp[0].stat); DBG(yyval.stat, yylsp[-8], yylsp[-1]); ;}
    break;

  case 243:
#line 704 "grammar.y"
    { yyval.stat = new ForNode(yyvsp[-6].vlist, yyvsp[-4].node, yyvsp[-2].node, yyvsp[0].stat); DBG(yyval.stat, yylsp[-9], yylsp[-1]); ;}
    break;

  case 244:
#line 706 "grammar.y"
    {
                                            Node *n = yyvsp[-4].node->nodeInsideAllParens();
                                            if (!n->isLocation())
                                                YYABORT;
                                            yyval.stat = new ForInNode(n, yyvsp[-2].node, yyvsp[0].stat);
                                            DBG(yyval.stat, yylsp[-6], yylsp[-1]);
                                        ;}
    break;

  case 245:
#line 714 "grammar.y"
    { yyval.stat = new ForInNode(*yyvsp[-4].ident, 0, yyvsp[-2].node, yyvsp[0].stat); DBG(yyval.stat, yylsp[-7], yylsp[-1]); ;}
    break;

  case 246:
#line 716 "grammar.y"
    { yyval.stat = new ForInNode(*yyvsp[-5].ident, yyvsp[-4].init, yyvsp[-2].node, yyvsp[0].stat); DBG(yyval.stat, yylsp[-8], yylsp[-1]); ;}
    break;

  case 247:
#line 720 "grammar.y"
    { yyval.node = 0; ;}
    break;

  case 249:
#line 725 "grammar.y"
    { yyval.node = 0; ;}
    break;

  case 251:
#line 730 "grammar.y"
    { yyval.stat = new ContinueNode(); DBG(yyval.stat, yylsp[-1], yylsp[0]); ;}
    break;

  case 252:
#line 731 "grammar.y"
    { yyval.stat = new ContinueNode(); DBG(yyval.stat, yylsp[-1], yylsp[-1]); AUTO_SEMICOLON; ;}
    break;

  case 253:
#line 732 "grammar.y"
    { yyval.stat = new ContinueNode(*yyvsp[-1].ident); DBG(yyval.stat, yylsp[-2], yylsp[0]); ;}
    break;

  case 254:
#line 733 "grammar.y"
    { yyval.stat = new ContinueNode(*yyvsp[-1].ident); DBG(yyval.stat, yylsp[-2], yylsp[-1]); AUTO_SEMICOLON; ;}
    break;

  case 255:
#line 737 "grammar.y"
    { yyval.stat = new BreakNode(); DBG(yyval.stat, yylsp[-1], yylsp[0]); ;}
    break;

  case 256:
#line 738 "grammar.y"
    { yyval.stat = new BreakNode(); DBG(yyval.stat, yylsp[-1], yylsp[-1]); AUTO_SEMICOLON; ;}
    break;

  case 257:
#line 739 "grammar.y"
    { yyval.stat = new BreakNode(*yyvsp[-1].ident); DBG(yyval.stat, yylsp[-2], yylsp[0]); ;}
    break;

  case 258:
#line 740 "grammar.y"
    { yyval.stat = new BreakNode(*yyvsp[-1].ident); DBG(yyval.stat, yylsp[-2], yylsp[-1]); AUTO_SEMICOLON; ;}
    break;

  case 259:
#line 744 "grammar.y"
    { yyval.stat = new ReturnNode(0); DBG(yyval.stat, yylsp[-1], yylsp[0]); ;}
    break;

  case 260:
#line 745 "grammar.y"
    { yyval.stat = new ReturnNode(0); DBG(yyval.stat, yylsp[-1], yylsp[-1]); AUTO_SEMICOLON; ;}
    break;

  case 261:
#line 746 "grammar.y"
    { yyval.stat = new ReturnNode(yyvsp[-1].node); DBG(yyval.stat, yylsp[-2], yylsp[0]); ;}
    break;

  case 262:
#line 747 "grammar.y"
    { yyval.stat = new ReturnNode(yyvsp[-1].node); DBG(yyval.stat, yylsp[-2], yylsp[-1]); AUTO_SEMICOLON; ;}
    break;

  case 263:
#line 751 "grammar.y"
    { yyval.stat = new WithNode(yyvsp[-2].node, yyvsp[0].stat); DBG(yyval.stat, yylsp[-4], yylsp[-1]); ;}
    break;

  case 264:
#line 755 "grammar.y"
    { yyval.stat = new SwitchNode(yyvsp[-2].node, yyvsp[0].cblk); DBG(yyval.stat, yylsp[-4], yylsp[-1]); ;}
    break;

  case 265:
#line 759 "grammar.y"
    { yyval.cblk = new CaseBlockNode(yyvsp[-1].clist, 0, 0); ;}
    break;

  case 266:
#line 761 "grammar.y"
    { yyval.cblk = new CaseBlockNode(yyvsp[-3].clist, yyvsp[-2].ccl, yyvsp[-1].clist); ;}
    break;

  case 267:
#line 765 "grammar.y"
    { yyval.clist = 0; ;}
    break;

  case 269:
#line 770 "grammar.y"
    { yyval.clist = new ClauseListNode(yyvsp[0].ccl); ;}
    break;

  case 270:
#line 771 "grammar.y"
    { yyval.clist = new ClauseListNode(yyvsp[-1].clist, yyvsp[0].ccl); ;}
    break;

  case 271:
#line 775 "grammar.y"
    { yyval.ccl = new CaseClauseNode(yyvsp[-1].node); ;}
    break;

  case 272:
#line 776 "grammar.y"
    { yyval.ccl = new CaseClauseNode(yyvsp[-2].node, yyvsp[0].slist); ;}
    break;

  case 273:
#line 780 "grammar.y"
    { yyval.ccl = new CaseClauseNode(0); ;}
    break;

  case 274:
#line 781 "grammar.y"
    { yyval.ccl = new CaseClauseNode(0, yyvsp[0].slist); ;}
    break;

  case 275:
#line 785 "grammar.y"
    { yyvsp[0].stat->pushLabel(*yyvsp[-2].ident); yyval.stat = new LabelNode(*yyvsp[-2].ident, yyvsp[0].stat); ;}
    break;

  case 276:
#line 789 "grammar.y"
    { yyval.stat = new ThrowNode(yyvsp[-1].node); DBG(yyval.stat, yylsp[-2], yylsp[0]); ;}
    break;

  case 277:
#line 790 "grammar.y"
    { yyval.stat = new ThrowNode(yyvsp[-1].node); DBG(yyval.stat, yylsp[-2], yylsp[-1]); AUTO_SEMICOLON; ;}
    break;

  case 278:
#line 794 "grammar.y"
    { yyval.stat = new TryNode(yyvsp[-2].stat, Identifier::null(), 0, yyvsp[0].stat); DBG(yyval.stat, yylsp[-3], yylsp[-2]); ;}
    break;

  case 279:
#line 795 "grammar.y"
    { yyval.stat = new TryNode(yyvsp[-5].stat, *yyvsp[-2].ident, yyvsp[0].stat, 0); DBG(yyval.stat, yylsp[-6], yylsp[-5]); ;}
    break;

  case 280:
#line 797 "grammar.y"
    { yyval.stat = new TryNode(yyvsp[-7].stat, *yyvsp[-4].ident, yyvsp[-2].stat, yyvsp[0].stat); DBG(yyval.stat, yylsp[-8], yylsp[-7]); ;}
    break;

  case 281:
#line 801 "grammar.y"
    { yyval.func = new FuncDeclNode(*yyvsp[-3].ident, yyvsp[0].body); ;}
    break;

  case 282:
#line 803 "grammar.y"
    { yyval.func = new FuncDeclNode(*yyvsp[-4].ident, yyvsp[-2].param, yyvsp[0].body); ;}
    break;

  case 283:
#line 807 "grammar.y"
    { yyval.funcExpr = new FuncExprNode(Identifier::null(), yyvsp[0].body); ;}
    break;

  case 284:
#line 809 "grammar.y"
    { yyval.funcExpr = new FuncExprNode(Identifier::null(), yyvsp[-2].param, yyvsp[0].body); ;}
    break;

  case 285:
#line 810 "grammar.y"
    { yyval.funcExpr = new FuncExprNode(*yyvsp[-3].ident, yyvsp[0].body); ;}
    break;

  case 286:
#line 812 "grammar.y"
    { yyval.funcExpr = new FuncExprNode(*yyvsp[-4].ident, yyvsp[-2].param, yyvsp[0].body); ;}
    break;

  case 287:
#line 816 "grammar.y"
    { yyval.param = new ParameterNode(*yyvsp[0].ident); ;}
    break;

  case 288:
#line 817 "grammar.y"
    { yyval.param = new ParameterNode(yyvsp[-2].param, *yyvsp[0].ident); ;}
    break;

  case 289:
#line 821 "grammar.y"
    { yyval.body = new FunctionBodyNode(0); DBG(yyval.body, yylsp[-1], yylsp[0]); ;}
    break;

  case 290:
#line 822 "grammar.y"
    { yyval.body = new FunctionBodyNode(yyvsp[-1].srcs); DBG(yyval.body, yylsp[-2], yylsp[0]); ;}
    break;

  case 291:
#line 826 "grammar.y"
    { Parser::accept(new ProgramNode(0)); ;}
    break;

  case 292:
#line 827 "grammar.y"
    { Parser::accept(new ProgramNode(yyvsp[0].srcs)); ;}
    break;

  case 293:
#line 831 "grammar.y"
    { yyval.srcs = new SourceElementsNode(yyvsp[0].stat); ;}
    break;

  case 294:
#line 832 "grammar.y"
    { yyval.srcs = new SourceElementsNode(yyvsp[-1].srcs, yyvsp[0].stat); ;}
    break;

  case 295:
#line 836 "grammar.y"
    { yyval.stat = yyvsp[0].func; ;}
    break;

  case 296:
#line 837 "grammar.y"
    { yyval.stat = yyvsp[0].stat; ;}
    break;


    }

/* Line 991 of yacc.c.  */
#line 3063 "grammar.tab.c"

  yyvsp -= yylen;
  yyssp -= yylen;
  yylsp -= yylen;

  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;
  *++yylsp = yyloc;

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (YYPACT_NINF < yyn && yyn < YYLAST)
	{
	  YYSIZE_T yysize = 0;
	  int yytype = YYTRANSLATE (yychar);
	  char *yymsg;
	  int yyx, yycount;

	  yycount = 0;
	  /* Start YYX at -YYN if negative to avoid negative indexes in
	     YYCHECK.  */
	  for (yyx = yyn < 0 ? -yyn : 0;
	       yyx < (int) (sizeof (yytname) / sizeof (char *)); yyx++)
	    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	      yysize += yystrlen (yytname[yyx]) + 15, yycount++;
	  yysize += yystrlen ("syntax error, unexpected ") + 1;
	  yysize += yystrlen (yytname[yytype]);
	  yymsg = (char *) YYSTACK_ALLOC (yysize);
	  if (yymsg != 0)
	    {
	      char *yyp = yystpcpy (yymsg, "syntax error, unexpected ");
	      yyp = yystpcpy (yyp, yytname[yytype]);

	      if (yycount < 5)
		{
		  yycount = 0;
		  for (yyx = yyn < 0 ? -yyn : 0;
		       yyx < (int) (sizeof (yytname) / sizeof (char *));
		       yyx++)
		    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
		      {
			const char *yyq = ! yycount ? ", expecting " : " or ";
			yyp = yystpcpy (yyp, yyq);
			yyp = yystpcpy (yyp, yytname[yyx]);
			yycount++;
		      }
		}
	      yyerror (yymsg);
	      YYSTACK_FREE (yymsg);
	    }
	  else
	    yyerror ("syntax error; also virtual memory exhausted");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror ("syntax error");
    }

  yylerrsp = yylsp;

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      /* Return failure if at end of input.  */
      if (yychar == YYEOF)
        {
	  /* Pop the error token.  */
          YYPOPSTACK;
	  /* Pop the rest of the stack.  */
	  while (yyss < yyssp)
	    {
	      YYDSYMPRINTF ("Error: popping", yystos[*yyssp], yyvsp, yylsp);
	      yydestruct (yystos[*yyssp], yyvsp, yylsp);
	      YYPOPSTACK;
	    }
	  YYABORT;
        }

      YYDSYMPRINTF ("Error: discarding", yytoken, &yylval, &yylloc);
      yydestruct (yytoken, &yylval, &yylloc);
      yychar = YYEMPTY;
      *++yylerrsp = yylloc;
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab2;


/*----------------------------------------------------.
| yyerrlab1 -- error raised explicitly by an action.  |
`----------------------------------------------------*/
yyerrlab1:

  /* Suppress GCC warning that yyerrlab1 is unused when no action
     invokes YYERROR.  */
#if defined (__GNUC_MINOR__) && 2093 <= (__GNUC__ * 1000 + __GNUC_MINOR__) \
    && !defined __cplusplus
  __attribute__ ((__unused__))
#endif

  yylerrsp = yylsp;
  *++yylerrsp = yyloc;
  goto yyerrlab2;


/*---------------------------------------------------------------.
| yyerrlab2 -- pop states until the error token can be shifted.  |
`---------------------------------------------------------------*/
yyerrlab2:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;

      YYDSYMPRINTF ("Error: popping", yystos[*yyssp], yyvsp, yylsp);
      yydestruct (yystos[yystate], yyvsp, yylsp);
      yyvsp--;
      yystate = *--yyssp;
      yylsp--;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  YYDPRINTF ((stderr, "Shifting error token, "));

  *++yyvsp = yylval;
  YYLLOC_DEFAULT (yyloc, yylsp, (yylerrsp - yylsp));
  *++yylsp = yyloc;

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*----------------------------------------------.
| yyoverflowlab -- parser overflow comes here.  |
`----------------------------------------------*/
yyoverflowlab:
  yyerror ("parser stack overflow");
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  return yyresult;
}


#line 840 "grammar.y"


static bool makeAssignNode(Node*& result, Node *loc, Operator op, Node *expr)
{ 
    Node *n = loc->nodeInsideAllParens();

    if (!n->isLocation())
        return false;

    if (n->isResolveNode()) {
        ResolveNode *resolve = static_cast<ResolveNode *>(n);
        result = new AssignResolveNode(resolve->identifier(), op, expr);
    } else if (n->isBracketAccessorNode()) {
        BracketAccessorNode *bracket = static_cast<BracketAccessorNode *>(n);
        result = new AssignBracketNode(bracket->base(), bracket->subscript(), op, expr);
    } else {
        assert(n->isDotAccessorNode());
        DotAccessorNode *dot = static_cast<DotAccessorNode *>(n);
        result = new AssignDotNode(dot->base(), dot->identifier(), op, expr);
    }

    return true;
}

static bool makePrefixNode(Node*& result, Node *expr, Operator op)
{ 
    Node *n = expr->nodeInsideAllParens();

    if (!n->isLocation())
        return false;
    
    if (n->isResolveNode()) {
        ResolveNode *resolve = static_cast<ResolveNode *>(n);
        result = new PrefixResolveNode(resolve->identifier(), op);
    } else if (n->isBracketAccessorNode()) {
        BracketAccessorNode *bracket = static_cast<BracketAccessorNode *>(n);
        result = new PrefixBracketNode(bracket->base(), bracket->subscript(), op);
    } else {
        assert(n->isDotAccessorNode());
        DotAccessorNode *dot = static_cast<DotAccessorNode *>(n);
        result = new PrefixDotNode(dot->base(), dot->identifier(), op);
    }

    return true;
}

static bool makePostfixNode(Node*& result, Node *expr, Operator op)
{ 
    Node *n = expr->nodeInsideAllParens();

    if (!n->isLocation())
        return false;
    
    if (n->isResolveNode()) {
        ResolveNode *resolve = static_cast<ResolveNode *>(n);
        result = new PostfixResolveNode(resolve->identifier(), op);
    } else if (n->isBracketAccessorNode()) {
        BracketAccessorNode *bracket = static_cast<BracketAccessorNode *>(n);
        result = new PostfixBracketNode(bracket->base(), bracket->subscript(), op);
    } else {
        assert(n->isDotAccessorNode());
        DotAccessorNode *dot = static_cast<DotAccessorNode *>(n);
        result = new PostfixDotNode(dot->base(), dot->identifier(), op);
    }

    return true;
}

static Node *makeFunctionCallNode(Node *func, ArgumentsNode *args)
{
    Node *n = func->nodeInsideAllParens();
    
    if (!n->isLocation())
        return new FunctionCallValueNode(func, args);
    else if (n->isResolveNode()) {
        ResolveNode *resolve = static_cast<ResolveNode *>(n);
        return new FunctionCallResolveNode(resolve->identifier(), args);
    } else if (n->isBracketAccessorNode()) {
        BracketAccessorNode *bracket = static_cast<BracketAccessorNode *>(n);
        if (n != func)
            return new FunctionCallParenBracketNode(bracket->base(), bracket->subscript(), args);
        else
            return new FunctionCallBracketNode(bracket->base(), bracket->subscript(), args);
    } else {
        assert(n->isDotAccessorNode());
        DotAccessorNode *dot = static_cast<DotAccessorNode *>(n);
        if (n != func)
            return new FunctionCallParenDotNode(dot->base(), dot->identifier(), args);
        else
            return new FunctionCallDotNode(dot->base(), dot->identifier(), args);
    }
}

static Node *makeTypeOfNode(Node *expr)
{
    Node *n = expr->nodeInsideAllParens();

    if (n->isResolveNode()) {
        ResolveNode *resolve = static_cast<ResolveNode *>(n);
        return new TypeOfResolveNode(resolve->identifier());
    } else
        return new TypeOfValueNode(n);
}

static Node *makeDeleteNode(Node *expr)
{
    Node *n = expr->nodeInsideAllParens();
    
    if (!n->isLocation())
        return new DeleteValueNode(expr);
    else if (n->isResolveNode()) {
        ResolveNode *resolve = static_cast<ResolveNode *>(n);
        return new DeleteResolveNode(resolve->identifier());
    } else if (n->isBracketAccessorNode()) {
        BracketAccessorNode *bracket = static_cast<BracketAccessorNode *>(n);
        return new DeleteBracketNode(bracket->base(), bracket->subscript());
    } else {
        assert(n->isDotAccessorNode());
        DotAccessorNode *dot = static_cast<DotAccessorNode *>(n);
        return new DeleteDotNode(dot->base(), dot->identifier());
    }
}

int yyerror(const char * /* s */)  /* Called by yyparse on error */
{
  // fprintf(stderr, "ERROR: %s at line %d\n", s, KJS::Lexer::curr()->lineNo());
  return 1;
}

/* may we automatically insert a semicolon ? */
static bool allowAutomaticSemicolon()
{
    return yychar == '}' || yychar == 0 || Lexer::curr()->prevTerminator();
}

