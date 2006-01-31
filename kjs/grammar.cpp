/* A Bison parser, made by GNU Bison 2.1.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005 Free Software Foundation, Inc.

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
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

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

/* Bison version.  */
#define YYBISON_VERSION "2.1"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 1

/* Substitute the variable and function names.  */
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
/* Tokens.  */
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "config.h"

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
#if !APPLE_CHANGES /* work around the fact that YYERROR_VERBOSE causes a compiler warning in bison code */
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
static bool makeGetterOrSetterPropertyNode(PropertyNode*& result, Identifier &getOrSet, Identifier& name, ParameterNode *params, FunctionBodyNode *body);
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

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
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
  PropertyListNode   *plist;
  PropertyNode       *pnode;
  PropertyNameNode   *pname;
} YYSTYPE;
/* Line 196 of yacc.c.  */
#line 308 "grammar.tab.c"
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


/* Line 219 of yacc.c.  */
#line 332 "grammar.tab.c"

#if ! defined (YYSIZE_T) && defined (__SIZE_TYPE__)
# define YYSIZE_T __SIZE_TYPE__
#endif
#if ! defined (YYSIZE_T) && defined (size_t)
# define YYSIZE_T size_t
#endif
#if ! defined (YYSIZE_T) && (defined (__STDC__) || defined (__cplusplus))
# include <stddef.h> /* INFRINGES ON USER NAME SPACE */
# define YYSIZE_T size_t
#endif
#if ! defined (YYSIZE_T)
# define YYSIZE_T unsigned int
#endif

#ifndef YY_
# if YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

#if ! defined (yyoverflow) || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if defined (__STDC__) || defined (__cplusplus)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     define YYINCLUDED_STDLIB_H
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning. */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2005 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM ((YYSIZE_T) -1)
#  endif
#  ifdef __cplusplus
extern "C" {
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if (! defined (malloc) && ! defined (YYINCLUDED_STDLIB_H) \
	&& (defined (__STDC__) || defined (__cplusplus)))
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if (! defined (free) && ! defined (YYINCLUDED_STDLIB_H) \
	&& (defined (__STDC__) || defined (__cplusplus)))
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifdef __cplusplus
}
#  endif
# endif
#endif /* ! defined (yyoverflow) || YYERROR_VERBOSE */


#if (! defined (yyoverflow) \
     && (! defined (__cplusplus) \
	 || (defined (YYLTYPE_IS_TRIVIAL) && YYLTYPE_IS_TRIVIAL \
             && defined (YYSTYPE_IS_TRIVIAL) && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  short int yyss;
  YYSTYPE yyvs;
    YYLTYPE yyls;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short int) + sizeof (YYSTYPE) + sizeof (YYLTYPE))	\
      + 2 * YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined (__GNUC__) && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
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
   typedef short int yysigned_char;
#endif

/* YYFINAL -- State number of the termination state. */
#define YYFINAL  204
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1440

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  87
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  102
/* YYNRULES -- Number of rules. */
#define YYNRULES  300
/* YYNRULES -- Number of states. */
#define YYNSTATES  526

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   317

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    76,     2,     2,     2,    78,    81,     2,
      65,    66,    77,    73,    67,    74,    72,    63,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    64,    86,
      79,    85,    80,    84,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    70,     2,    71,    82,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    68,    83,    69,    75,     2,     2,     2,
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
static const unsigned short int yyprhs[] =
{
       0,     0,     3,     5,     7,     9,    11,    13,    15,    17,
      19,    21,    23,    27,    33,    40,    42,    46,    48,    51,
      55,    60,    62,    64,    66,    68,    72,    76,    80,    86,
      89,    94,    95,    97,    99,   102,   104,   106,   111,   115,
     119,   121,   126,   130,   134,   136,   139,   141,   144,   147,
     150,   155,   159,   162,   165,   170,   174,   177,   181,   183,
     187,   189,   191,   193,   195,   197,   200,   203,   205,   208,
     211,   214,   217,   220,   223,   226,   229,   232,   235,   238,
     241,   244,   246,   248,   250,   252,   254,   258,   262,   266,
     268,   272,   276,   280,   282,   286,   290,   292,   296,   300,
     302,   306,   310,   314,   316,   320,   324,   328,   330,   334,
     338,   342,   346,   350,   354,   356,   360,   364,   368,   372,
     376,   378,   382,   386,   390,   394,   398,   402,   404,   408,
     412,   416,   420,   422,   426,   430,   434,   438,   440,   444,
     448,   452,   456,   458,   462,   464,   468,   470,   474,   476,
     480,   482,   486,   488,   492,   494,   498,   500,   504,   506,
     510,   512,   516,   518,   522,   524,   528,   530,   534,   536,
     540,   542,   546,   548,   554,   556,   562,   564,   570,   572,
     576,   578,   582,   584,   588,   590,   592,   594,   596,   598,
     600,   602,   604,   606,   608,   610,   612,   614,   618,   620,
     624,   626,   630,   632,   634,   636,   638,   640,   642,   644,
     646,   648,   650,   652,   654,   656,   658,   660,   663,   667,
     669,   672,   676,   680,   682,   686,   688,   692,   694,   697,
     699,   702,   706,   710,   712,   716,   718,   721,   724,   727,
     729,   732,   735,   741,   749,   756,   762,   772,   783,   791,
     800,   810,   811,   813,   814,   816,   819,   822,   826,   830,
     833,   836,   840,   844,   847,   850,   854,   858,   864,   870,
     874,   880,   881,   883,   885,   888,   892,   897,   900,   904,
     908,   912,   916,   921,   929,   939,   945,   952,   957,   963,
     969,   976,   978,   982,   985,   989,   990,   992,   994,   997,
     999
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short int yyrhs[] =
{
     186,     0,    -1,     3,    -1,     4,    -1,     5,    -1,     7,
      -1,     6,    -1,    63,    -1,    52,    -1,    60,    -1,     6,
      -1,     7,    -1,    89,    64,   143,    -1,    60,    60,    65,
      66,   185,    -1,    60,    60,    65,   184,    66,   185,    -1,
      90,    -1,    91,    67,    90,    -1,    93,    -1,    68,    69,
      -1,    68,    91,    69,    -1,    68,    91,    67,    69,    -1,
      21,    -1,    88,    -1,    94,    -1,    60,    -1,    65,   147,
      66,    -1,    70,    96,    71,    -1,    70,    95,    71,    -1,
      70,    95,    67,    96,    71,    -1,    96,   143,    -1,    95,
      67,    96,   143,    -1,    -1,    97,    -1,    67,    -1,    97,
      67,    -1,    92,    -1,   183,    -1,    98,    70,   147,    71,
      -1,    98,    72,    60,    -1,    12,    98,   104,    -1,    93,
      -1,    99,    70,   147,    71,    -1,    99,    72,    60,    -1,
      12,    98,   104,    -1,    98,    -1,    12,   100,    -1,    99,
      -1,    12,   100,    -1,    98,   104,    -1,   102,   104,    -1,
     102,    70,   147,    71,    -1,   102,    72,    60,    -1,    99,
     104,    -1,   103,   104,    -1,   103,    70,   147,    71,    -1,
     103,    72,    60,    -1,    65,    66,    -1,    65,   105,    66,
      -1,   143,    -1,   105,    67,   143,    -1,   100,    -1,   102,
      -1,   101,    -1,   103,    -1,   106,    -1,   106,    44,    -1,
     106,    45,    -1,   107,    -1,   107,    44,    -1,   107,    45,
      -1,    19,   111,    -1,    18,   111,    -1,    26,   111,    -1,
      44,   111,    -1,    61,   111,    -1,    45,   111,    -1,    62,
     111,    -1,    73,   111,    -1,    74,   111,    -1,    75,   111,
      -1,    76,   111,    -1,   108,    -1,   110,    -1,   109,    -1,
     110,    -1,   111,    -1,   113,    77,   111,    -1,   113,    63,
     111,    -1,   113,    78,   111,    -1,   112,    -1,   114,    77,
     111,    -1,   114,    63,   111,    -1,   114,    78,   111,    -1,
     113,    -1,   115,    73,   113,    -1,   115,    74,   113,    -1,
     114,    -1,   116,    73,   113,    -1,   116,    74,   113,    -1,
     115,    -1,   117,    46,   115,    -1,   117,    47,   115,    -1,
     117,    48,   115,    -1,   116,    -1,   118,    46,   115,    -1,
     118,    47,   115,    -1,   118,    48,   115,    -1,   117,    -1,
     119,    79,   117,    -1,   119,    80,   117,    -1,   119,    40,
     117,    -1,   119,    41,   117,    -1,   119,    25,   117,    -1,
     119,    24,   117,    -1,   117,    -1,   120,    79,   117,    -1,
     120,    80,   117,    -1,   120,    40,   117,    -1,   120,    41,
     117,    -1,   120,    25,   117,    -1,   118,    -1,   121,    79,
     117,    -1,   121,    80,   117,    -1,   121,    40,   117,    -1,
     121,    41,   117,    -1,   121,    25,   117,    -1,   121,    24,
     117,    -1,   119,    -1,   122,    36,   119,    -1,   122,    37,
     119,    -1,   122,    38,   119,    -1,   122,    39,   119,    -1,
     120,    -1,   123,    36,   120,    -1,   123,    37,   120,    -1,
     123,    38,   120,    -1,   123,    39,   120,    -1,   121,    -1,
     124,    36,   119,    -1,   124,    37,   119,    -1,   124,    38,
     119,    -1,   124,    39,   119,    -1,   122,    -1,   125,    81,
     122,    -1,   123,    -1,   126,    81,   123,    -1,   124,    -1,
     127,    81,   122,    -1,   125,    -1,   128,    82,   125,    -1,
     126,    -1,   129,    82,   126,    -1,   127,    -1,   130,    82,
     125,    -1,   128,    -1,   131,    83,   128,    -1,   129,    -1,
     132,    83,   129,    -1,   130,    -1,   133,    83,   128,    -1,
     131,    -1,   134,    43,   131,    -1,   132,    -1,   135,    43,
     132,    -1,   133,    -1,   136,    43,   131,    -1,   134,    -1,
     137,    42,   134,    -1,   135,    -1,   138,    42,   135,    -1,
     136,    -1,   139,    42,   134,    -1,   137,    -1,   137,    84,
     143,    64,   143,    -1,   138,    -1,   138,    84,   144,    64,
     144,    -1,   139,    -1,   139,    84,   143,    64,   143,    -1,
     140,    -1,   106,   146,   143,    -1,   141,    -1,   106,   146,
     144,    -1,   142,    -1,   107,   146,   143,    -1,    85,    -1,
      49,    -1,    50,    -1,    51,    -1,    52,    -1,    53,    -1,
      54,    -1,    55,    -1,    56,    -1,    58,    -1,    59,    -1,
      57,    -1,   143,    -1,   147,    67,   143,    -1,   144,    -1,
     148,    67,   144,    -1,   145,    -1,   149,    67,   143,    -1,
     151,    -1,   153,    -1,   158,    -1,   163,    -1,   164,    -1,
     165,    -1,   166,    -1,   169,    -1,   170,    -1,   171,    -1,
     172,    -1,   173,    -1,   179,    -1,   180,    -1,   181,    -1,
      68,    69,    -1,    68,   187,    69,    -1,   150,    -1,   152,
     150,    -1,    13,   154,    86,    -1,    13,   154,     1,    -1,
     156,    -1,   154,    67,   156,    -1,   157,    -1,   155,    67,
     157,    -1,    60,    -1,    60,   161,    -1,    60,    -1,    60,
     162,    -1,    14,   159,    86,    -1,    14,   159,     1,    -1,
     160,    -1,   159,    67,   160,    -1,    60,    -1,    60,   161,
      -1,    85,   143,    -1,    85,   144,    -1,    86,    -1,   149,
      86,    -1,   149,     1,    -1,    20,    65,   147,    66,   150,
      -1,    20,    65,   147,    66,   150,    35,   150,    -1,    22,
     150,    23,    65,   147,    66,    -1,    23,    65,   147,    66,
     150,    -1,    11,    65,   168,    86,   167,    86,   167,    66,
     150,    -1,    11,    65,    13,   155,    86,   167,    86,   167,
      66,   150,    -1,    11,    65,   106,    24,   147,    66,   150,
      -1,    11,    65,    13,    60,    24,   147,    66,   150,    -1,
      11,    65,    13,    60,   162,    24,   147,    66,   150,    -1,
      -1,   147,    -1,    -1,   148,    -1,    15,    86,    -1,    15,
       1,    -1,    15,    60,    86,    -1,    15,    60,     1,    -1,
       8,    86,    -1,     8,     1,    -1,     8,    60,    86,    -1,
       8,    60,     1,    -1,    17,    86,    -1,    17,     1,    -1,
      17,   147,    86,    -1,    17,   147,     1,    -1,    28,    65,
     147,    66,   150,    -1,    27,    65,   147,    66,   174,    -1,
      68,   175,    69,    -1,    68,   175,   178,   175,    69,    -1,
      -1,   176,    -1,   177,    -1,   176,   177,    -1,     9,   147,
      64,    -1,     9,   147,    64,   152,    -1,    10,    64,    -1,
      10,    64,   152,    -1,    60,    64,   150,    -1,    30,   147,
      86,    -1,    30,   147,     1,    -1,    31,   151,    33,   151,
      -1,    31,   151,    32,    65,    60,    66,   151,    -1,    31,
     151,    32,    65,    60,    66,   151,    33,   151,    -1,    16,
      60,    65,    66,   185,    -1,    16,    60,    65,   184,    66,
     185,    -1,    16,    65,    66,   185,    -1,    16,    65,   184,
      66,   185,    -1,    16,    60,    65,    66,   185,    -1,    16,
      60,    65,   184,    66,   185,    -1,    60,    -1,   184,    67,
      60,    -1,    68,    69,    -1,    68,   187,    69,    -1,    -1,
     187,    -1,   188,    -1,   187,   188,    -1,   182,    -1,   150,
      -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short int yyrline[] =
{
       0,   189,   189,   190,   191,   192,   193,   194,   199,   207,
     208,   209,   213,   214,   215,   220,   221,   225,   226,   227,
     229,   233,   234,   235,   236,   237,   241,   242,   243,   247,
     248,   253,   254,   258,   259,   263,   264,   265,   266,   267,
     271,   272,   273,   274,   278,   279,   283,   284,   288,   289,
     290,   291,   295,   296,   297,   298,   302,   303,   307,   308,
     312,   313,   317,   318,   322,   323,   324,   328,   329,   330,
     334,   335,   336,   337,   338,   339,   340,   341,   342,   343,
     344,   347,   348,   352,   353,   357,   358,   359,   360,   364,
     365,   367,   369,   374,   375,   376,   380,   381,   383,   388,
     389,   390,   391,   395,   396,   397,   398,   402,   403,   404,
     405,   406,   407,   408,   412,   413,   414,   415,   416,   417,
     422,   423,   424,   425,   426,   427,   429,   433,   434,   435,
     436,   437,   441,   442,   444,   446,   448,   453,   454,   456,
     457,   459,   464,   465,   469,   470,   475,   476,   480,   481,
     485,   486,   491,   492,   497,   498,   502,   503,   508,   509,
     514,   515,   519,   520,   525,   526,   531,   532,   536,   537,
     542,   543,   547,   548,   553,   554,   559,   560,   565,   566,
     571,   572,   577,   578,   583,   584,   585,   586,   587,   588,
     589,   590,   591,   592,   593,   594,   598,   599,   603,   604,
     608,   609,   613,   614,   615,   616,   617,   618,   619,   620,
     621,   622,   623,   624,   625,   626,   627,   631,   632,   636,
     637,   641,   642,   646,   647,   652,   653,   658,   659,   663,
     664,   668,   669,   673,   674,   679,   680,   684,   688,   692,
     696,   697,   701,   703,   708,   709,   710,   712,   714,   722,
     724,   729,   730,   734,   735,   739,   740,   741,   742,   746,
     747,   748,   749,   753,   754,   755,   756,   760,   764,   768,
     769,   774,   775,   779,   780,   784,   785,   789,   790,   794,
     798,   799,   803,   804,   805,   810,   811,   816,   817,   819,
     820,   825,   826,   830,   831,   835,   836,   840,   841,   845,
     846
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
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
  "AUTOPLUSPLUS", "AUTOMINUSMINUS", "'/'", "':'", "'('", "')'", "','",
  "'{'", "'}'", "'['", "']'", "'.'", "'+'", "'-'", "'~'", "'!'", "'*'",
  "'%'", "'<'", "'>'", "'&'", "'^'", "'|'", "'?'", "'='", "';'", "$accept",
  "Literal", "PropertyName", "Property", "PropertyList", "PrimaryExpr",
  "PrimaryExprNoBrace", "ArrayLiteral", "ElementList", "ElisionOpt",
  "Elision", "MemberExpr", "MemberExprNoBF", "NewExpr", "NewExprNoBF",
  "CallExpr", "CallExprNoBF", "Arguments", "ArgumentList",
  "LeftHandSideExpr", "LeftHandSideExprNoBF", "PostfixExpr",
  "PostfixExprNoBF", "UnaryExprCommon", "UnaryExpr", "UnaryExprNoBF",
  "MultiplicativeExpr", "MultiplicativeExprNoBF", "AdditiveExpr",
  "AdditiveExprNoBF", "ShiftExpr", "ShiftExprNoBF", "RelationalExpr",
  "RelationalExprNoIn", "RelationalExprNoBF", "EqualityExpr",
  "EqualityExprNoIn", "EqualityExprNoBF", "BitwiseANDExpr",
  "BitwiseANDExprNoIn", "BitwiseANDExprNoBF", "BitwiseXORExpr",
  "BitwiseXORExprNoIn", "BitwiseXORExprNoBF", "BitwiseORExpr",
  "BitwiseORExprNoIn", "BitwiseORExprNoBF", "LogicalANDExpr",
  "LogicalANDExprNoIn", "LogicalANDExprNoBF", "LogicalORExpr",
  "LogicalORExprNoIn", "LogicalORExprNoBF", "ConditionalExpr",
  "ConditionalExprNoIn", "ConditionalExprNoBF", "AssignmentExpr",
  "AssignmentExprNoIn", "AssignmentExprNoBF", "AssignmentOperator", "Expr",
  "ExprNoIn", "ExprNoBF", "Statement", "Block", "StatementList",
  "VariableStatement", "VariableDeclarationList",
  "VariableDeclarationListNoIn", "VariableDeclaration",
  "VariableDeclarationNoIn", "ConstStatement", "ConstDeclarationList",
  "ConstDeclaration", "Initializer", "InitializerNoIn", "EmptyStatement",
  "ExprStatement", "IfStatement", "IterationStatement", "ExprOpt",
  "ExprNoInOpt", "ContinueStatement", "BreakStatement", "ReturnStatement",
  "WithStatement", "SwitchStatement", "CaseBlock", "CaseClausesOpt",
  "CaseClauses", "CaseClause", "DefaultClause", "LabelledStatement",
  "ThrowStatement", "TryStatement", "FunctionDeclaration", "FunctionExpr",
  "FormalParameterList", "FunctionBody", "Program", "SourceElements",
  "SourceElement", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const unsigned short int yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,    47,    58,    40,    41,    44,   123,   125,
      91,    93,    46,    43,    45,   126,    33,    42,    37,    60,
      62,    38,    94,   124,    63,    61,    59
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    87,    88,    88,    88,    88,    88,    88,    88,    89,
      89,    89,    90,    90,    90,    91,    91,    92,    92,    92,
      92,    93,    93,    93,    93,    93,    94,    94,    94,    95,
      95,    96,    96,    97,    97,    98,    98,    98,    98,    98,
      99,    99,    99,    99,   100,   100,   101,   101,   102,   102,
     102,   102,   103,   103,   103,   103,   104,   104,   105,   105,
     106,   106,   107,   107,   108,   108,   108,   109,   109,   109,
     110,   110,   110,   110,   110,   110,   110,   110,   110,   110,
     110,   111,   111,   112,   112,   113,   113,   113,   113,   114,
     114,   114,   114,   115,   115,   115,   116,   116,   116,   117,
     117,   117,   117,   118,   118,   118,   118,   119,   119,   119,
     119,   119,   119,   119,   120,   120,   120,   120,   120,   120,
     121,   121,   121,   121,   121,   121,   121,   122,   122,   122,
     122,   122,   123,   123,   123,   123,   123,   124,   124,   124,
     124,   124,   125,   125,   126,   126,   127,   127,   128,   128,
     129,   129,   130,   130,   131,   131,   132,   132,   133,   133,
     134,   134,   135,   135,   136,   136,   137,   137,   138,   138,
     139,   139,   140,   140,   141,   141,   142,   142,   143,   143,
     144,   144,   145,   145,   146,   146,   146,   146,   146,   146,
     146,   146,   146,   146,   146,   146,   147,   147,   148,   148,
     149,   149,   150,   150,   150,   150,   150,   150,   150,   150,
     150,   150,   150,   150,   150,   150,   150,   151,   151,   152,
     152,   153,   153,   154,   154,   155,   155,   156,   156,   157,
     157,   158,   158,   159,   159,   160,   160,   161,   162,   163,
     164,   164,   165,   165,   166,   166,   166,   166,   166,   166,
     166,   167,   167,   168,   168,   169,   169,   169,   169,   170,
     170,   170,   170,   171,   171,   171,   171,   172,   173,   174,
     174,   175,   175,   176,   176,   177,   177,   178,   178,   179,
     180,   180,   181,   181,   181,   182,   182,   183,   183,   183,
     183,   184,   184,   185,   185,   186,   186,   187,   187,   188,
     188
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     3,     5,     6,     1,     3,     1,     2,     3,
       4,     1,     1,     1,     1,     3,     3,     3,     5,     2,
       4,     0,     1,     1,     2,     1,     1,     4,     3,     3,
       1,     4,     3,     3,     1,     2,     1,     2,     2,     2,
       4,     3,     2,     2,     4,     3,     2,     3,     1,     3,
       1,     1,     1,     1,     1,     2,     2,     1,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     1,     1,     1,     1,     1,     3,     3,     3,     1,
       3,     3,     3,     1,     3,     3,     1,     3,     3,     1,
       3,     3,     3,     1,     3,     3,     3,     1,     3,     3,
       3,     3,     3,     3,     1,     3,     3,     3,     3,     3,
       1,     3,     3,     3,     3,     3,     3,     1,     3,     3,
       3,     3,     1,     3,     3,     3,     3,     1,     3,     3,
       3,     3,     1,     3,     1,     3,     1,     3,     1,     3,
       1,     3,     1,     3,     1,     3,     1,     3,     1,     3,
       1,     3,     1,     3,     1,     3,     1,     3,     1,     3,
       1,     3,     1,     5,     1,     5,     1,     5,     1,     3,
       1,     3,     1,     3,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     3,     1,     3,
       1,     3,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     2,     3,     1,
       2,     3,     3,     1,     3,     1,     3,     1,     2,     1,
       2,     3,     3,     1,     3,     1,     2,     2,     2,     1,
       2,     2,     5,     7,     6,     5,     9,    10,     7,     8,
       9,     0,     1,     0,     1,     2,     2,     3,     3,     2,
       2,     3,     3,     2,     2,     3,     3,     5,     5,     3,
       5,     0,     1,     1,     2,     3,     4,     2,     3,     3,
       3,     3,     4,     7,     9,     5,     6,     4,     5,     5,
       6,     1,     3,     2,     3,     0,     1,     1,     2,     1,
       1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned short int yydefact[] =
{
     295,     2,     3,     4,     6,     5,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    21,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     8,    24,     0,
       0,     7,     0,     0,    31,     0,     0,     0,     0,   239,
      22,    40,    23,    46,    62,    63,    67,    83,    84,    89,
      96,   103,   120,   137,   146,   152,   158,   164,   170,   176,
     182,   200,     0,   300,   202,   203,   204,   205,   206,   207,
     208,   209,   210,   211,   212,   213,   214,   215,   216,   299,
       0,   296,   297,   260,     0,   259,   253,     0,     0,    24,
       0,    35,    17,    44,    47,    36,   227,     0,   223,   235,
       0,   233,   256,     0,   255,     0,   264,   263,    44,    60,
      61,    64,    81,    82,    85,    93,    99,   107,   127,   142,
     148,   154,   160,   166,   172,   178,   196,     0,    64,    71,
      70,     0,     0,     0,    72,     0,     0,     0,     0,    73,
      75,     0,    74,    76,     0,   217,     0,    33,     0,     0,
      32,    77,    78,    79,    80,     0,     0,     0,    52,     0,
       0,    53,    68,    69,   185,   186,   187,   188,   189,   190,
     191,   192,   195,   193,   194,   184,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   241,     0,   240,     1,   298,   262,   261,     0,    64,
     114,   132,   144,   150,   156,   162,   168,   174,   180,   198,
     254,     0,    44,    45,     0,     0,    10,    11,     9,    18,
       0,    15,     0,     0,     0,    43,     0,   228,   222,     0,
     221,   236,   232,     0,   231,   258,   257,     0,    48,     0,
       0,    49,    65,    66,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   266,
       0,   265,     0,     0,     0,     0,     0,   281,   280,     0,
       0,   279,    25,   218,    31,    27,    26,    29,    34,    56,
       0,    58,     0,    42,     0,    55,   183,    91,    90,    92,
      97,    98,   104,   105,   106,   126,   125,   123,   124,   121,
     122,   138,   139,   140,   141,   147,   153,   159,   165,   171,
       0,   201,   229,     0,   225,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   251,    39,     0,   291,     0,     0,     0,
       0,     0,    19,     0,    38,   237,   224,   234,     0,     0,
       0,    51,   179,    87,    86,    88,    94,    95,   100,   101,
     102,   113,   112,   110,   111,   108,   109,   128,   129,   130,
     131,   143,   149,   155,   161,   167,     0,   197,     0,     0,
       0,     0,     0,     0,   282,     0,    57,     0,    41,    54,
       0,     0,     0,   230,     0,   251,     0,    64,   181,   119,
     117,   118,   115,   116,   133,   134,   135,   136,   145,   151,
     157,   163,   169,     0,   199,   252,     0,     0,     0,     0,
     287,     0,     0,     0,    12,    20,    16,    37,   285,     0,
      50,     0,   242,     0,   245,   271,   268,   267,     0,    28,
      30,    59,   177,     0,   238,     0,   229,   226,     0,     0,
       0,   251,   289,     0,   293,     0,   288,   292,     0,     0,
     286,   173,     0,   244,     0,     0,   272,   273,     0,     0,
       0,   230,   251,   248,   175,     0,   290,   294,    13,     0,
     243,     0,     0,   269,   271,   274,   283,   249,     0,     0,
       0,    14,   275,   277,     0,     0,   250,     0,   246,   219,
     276,   278,   270,   284,   247,   220
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short int yydefgoto[] =
{
      -1,    40,   230,   231,   232,    91,    92,    42,   148,   149,
     150,   108,    43,   109,    44,   110,    45,   158,   300,   128,
      46,   112,    47,   113,   114,    49,   115,    50,   116,    51,
     117,    52,   118,   211,    53,   119,   212,    54,   120,   213,
      55,   121,   214,    56,   122,   215,    57,   123,   216,    58,
     124,   217,    59,   125,   218,    60,   126,   219,    61,   336,
     435,   220,    62,    63,    64,   520,    65,    97,   333,    98,
     334,    66,   100,   101,   237,   413,    67,    68,    69,    70,
     436,   221,    71,    72,    73,    74,    75,   456,   485,   486,
     487,   504,    76,    77,    78,    79,    95,   358,   440,    80,
      81,    82
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -399
static const short int yypact[] =
{
     917,  -399,  -399,  -399,  -399,  -399,     6,   -51,   254,   -36,
     -14,    12,     8,   537,  1364,  1364,   -11,  -399,   993,    -5,
    1364,    26,    48,  1364,     9,  1364,  1364,  -399,    57,  1364,
    1364,  -399,  1364,   613,    94,  1364,  1364,  1364,  1364,  -399,
    -399,  -399,  -399,   125,  -399,   140,  1047,  -399,  -399,  -399,
     106,   113,   256,   134,   217,    85,    96,   116,   163,   -15,
    -399,  -399,     7,  -399,  -399,  -399,  -399,  -399,  -399,  -399,
    -399,  -399,  -399,  -399,  -399,  -399,  -399,  -399,  -399,  -399,
     237,   917,  -399,  -399,    29,  -399,  1068,   254,    11,  -399,
     147,  -399,  -399,   169,  -399,  -399,   132,    34,  -399,   132,
      36,  -399,  -399,    31,  -399,   175,  -399,  -399,   169,  -399,
     199,  1121,  -399,  -399,  -399,   131,   157,   282,   211,   241,
     165,   181,   185,   262,   101,  -399,  -399,    42,   228,  -399,
    -399,  1364,   285,  1364,  -399,  1364,  1364,    47,   278,  -399,
    -399,   993,  -399,  -399,   222,  -399,   689,  -399,    93,  1142,
     246,  -399,  -399,  -399,  -399,  1216,  1364,   258,  -399,  1364,
     260,  -399,  -399,  -399,  -399,  -399,  -399,  -399,  -399,  -399,
    -399,  -399,  -399,  -399,  -399,  -399,  1364,  1364,  1364,  1364,
    1364,  1364,  1364,  1364,  1364,  1364,  1364,  1364,  1364,  1364,
    1364,  1364,  1364,  1364,  1364,  1364,  1364,  1364,  1364,  1364,
    1364,  -399,  1364,  -399,  -399,  -399,  -399,  -399,   261,   521,
     282,   203,   248,   242,   270,   280,   321,   108,  -399,  -399,
     298,   300,   169,  -399,   301,   -24,  -399,  -399,   308,  -399,
     323,  -399,   -46,  1364,   328,  -399,  1364,  -399,  -399,   -36,
    -399,  -399,  -399,   -14,  -399,  -399,  -399,   -22,  -399,  1364,
     329,  -399,  -399,  -399,  1364,  1364,  1364,  1364,  1364,  1364,
    1364,  1364,  1364,  1364,  1364,  1364,  1364,  1364,  1364,  1364,
    1364,  1364,  1364,  1364,  1364,  1364,  1364,  1364,  1364,  -399,
    1364,  -399,   249,   325,   267,   269,   271,  -399,  -399,   326,
       9,  -399,  -399,  -399,    94,  -399,  -399,  -399,  -399,  -399,
     273,  -399,   110,  -399,   122,  -399,  -399,  -399,  -399,  -399,
     131,   131,   157,   157,   157,   282,   282,   282,   282,   282,
     282,   211,   211,   211,   211,   241,   165,   181,   185,   262,
     333,  -399,    -2,   -47,  -399,  1364,  1364,  1364,  1364,  1364,
    1364,  1364,  1364,  1364,  1364,  1364,  1364,  1364,  1364,  1364,
    1364,  1364,  1364,  1364,  -399,   -19,  -399,   336,   275,   340,
    1364,   164,  -399,   144,  -399,  -399,  -399,  -399,   336,   277,
     171,  -399,  -399,  -399,  -399,  -399,   131,   131,   157,   157,
     157,   282,   282,   282,   282,   282,   282,   211,   211,   211,
     211,   241,   165,   181,   185,   262,   346,  -399,   993,  1364,
     993,   344,   993,   353,  -399,  1290,  -399,  1364,  -399,  -399,
    1364,  1364,  1364,   369,   355,  1364,   279,  1121,  -399,   282,
     282,   282,   282,   282,   203,   203,   203,   203,   248,   242,
     270,   280,   321,   356,  -399,   350,   335,   336,   284,   765,
    -399,   336,   359,    -7,  -399,  -399,  -399,  -399,  -399,   336,
    -399,  1364,   387,   288,  -399,   414,  -399,  -399,   358,  -399,
    -399,  -399,  -399,   291,  -399,  1364,   341,  -399,   339,   993,
    1364,  1364,  -399,   336,  -399,   841,  -399,  -399,   336,   293,
    -399,  -399,   993,  -399,  1364,    19,   414,  -399,     9,   993,
     295,  -399,  1364,  -399,  -399,   361,  -399,  -399,  -399,   336,
    -399,   109,   364,  -399,   414,  -399,   396,  -399,   993,   366,
     993,  -399,   993,   993,   365,     9,  -399,   993,  -399,  -399,
     993,   993,  -399,  -399,  -399,  -399
};

/* YYPGOTO[NTERM-NUM].  */
static const short int yypgoto[] =
{
    -399,  -399,  -399,    69,  -399,  -399,     0,  -399,  -399,   139,
    -399,    23,  -399,    55,  -399,  -399,  -399,   -43,  -399,    -4,
    -399,  -399,  -399,    16,    70,  -399,  -119,  -399,   -35,  -399,
      33,  -399,   -68,   -50,  -399,  -184,    89,  -399,  -156,    90,
    -399,  -145,    88,  -399,  -142,    91,  -399,  -141,    92,  -399,
    -399,  -399,  -399,  -399,  -399,  -399,   -98,  -326,  -399,   -31,
      32,  -399,  -399,    -6,   -23,   -75,  -399,  -399,  -399,   202,
      30,  -399,  -399,   205,   347,   -17,  -399,  -399,  -399,  -399,
    -398,  -399,  -399,  -399,  -399,  -399,  -399,  -399,   -54,  -399,
     -34,  -399,  -399,  -399,  -399,  -399,  -399,  -243,  -362,  -399,
     -30,   -76
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const unsigned short int yytable[] =
{
      41,   138,   161,   146,   369,   205,   448,    83,   201,   111,
     418,   325,   132,   102,    86,   176,    48,   468,    41,   111,
     414,   361,   411,   362,    96,   433,   434,   199,   111,   502,
     206,    93,   245,    41,    48,   238,   356,   242,   356,   415,
     326,   356,   357,   279,   368,   127,    99,   437,   287,    48,
     235,   297,   327,   356,   131,   137,   328,   301,   329,   478,
     133,   310,   311,    94,   144,   248,    84,   251,   105,   200,
     205,   224,   103,   495,   202,   472,   225,    33,   306,   476,
     254,    41,   209,   412,   129,   130,   464,   480,   503,   391,
     134,   135,    85,   203,   509,   139,   140,    48,   104,   142,
     143,   239,   330,   243,   331,   151,   152,   153,   154,   280,
     222,   496,   438,   136,   280,   207,   498,   246,   392,   210,
     240,   141,   244,   321,   322,   323,   324,   111,   281,   111,
     393,   111,   111,   288,   394,   291,   395,   511,   365,   376,
     377,    41,   223,   277,   494,   111,    41,   312,   313,   314,
     350,   111,   111,   226,   227,   111,   372,    48,   185,   186,
     294,   147,    48,   282,   295,   284,   195,   285,   286,   177,
     226,   227,   111,   512,   187,   188,   280,   280,   196,   354,
     396,   408,   397,   178,   179,   278,   180,   181,   302,   280,
     155,   304,   351,   409,   255,   156,   111,   157,   111,   197,
     479,   387,   388,   389,   390,   155,   198,   228,   256,   257,
     159,   280,   160,   189,   190,   447,   229,   236,   315,   316,
     317,   318,   319,   320,   228,   378,   379,   380,   337,   111,
     258,   259,   111,   445,   155,   263,   264,   204,   280,   233,
     247,   234,   450,   338,   339,   111,   273,   307,   308,   309,
     111,   265,   266,   191,   192,   193,   194,     1,     2,     3,
       4,     5,   444,   274,   155,   363,    87,   404,   275,   249,
      88,   250,   252,   253,   111,    17,   111,   269,   270,   271,
     272,   370,   340,   341,   342,   343,   344,   345,   292,   280,
     267,   268,   424,   425,   426,   427,   381,   382,   383,   384,
     385,   386,   182,   183,   184,   276,    27,   460,   283,   461,
     289,   290,   462,   298,    89,   398,   280,    31,   303,    32,
     305,   332,    90,   346,    34,   373,   374,   375,   260,   261,
     262,   111,   417,   400,   280,   401,   280,   402,   280,   406,
     407,   441,   442,   449,   442,   469,   280,   417,   417,   111,
     473,   442,   347,   481,   483,   280,   111,   489,   280,   499,
     442,   508,   280,   348,   349,   352,   355,   416,   359,   210,
     419,   420,   421,   422,   423,   210,   210,   210,   210,   210,
     210,   210,   210,   210,   210,   210,   353,   360,   364,   371,
     399,   403,   452,   465,   454,   111,   457,   410,    41,   205,
      41,   111,    41,   111,   439,   443,   111,   111,   417,   475,
     451,   111,   455,   458,    48,   466,    48,   280,    48,   477,
     470,   471,   482,   484,   488,   492,   412,   510,   513,   515,
     446,   453,   517,   405,   522,   428,   430,   429,   521,    41,
     431,   366,   432,   463,   467,   210,   241,   111,   367,   491,
     514,     0,   505,     0,     0,    48,     0,     0,     0,     0,
       0,   111,     0,   493,     0,   506,   417,   111,     0,    41,
       0,     0,     0,     0,     0,    41,   500,     0,     0,     0,
     111,     0,    41,   507,     0,    48,     0,     0,   111,    41,
       0,    48,   523,     0,     0,     0,     0,   490,    48,     0,
       0,     0,   516,   210,   518,    48,   519,   519,    41,     0,
      41,   524,    41,    41,   525,   525,   501,    41,     0,     0,
      41,    41,     0,     0,    48,     0,    48,     0,    48,    48,
       0,     0,     0,    48,     0,     0,    48,    48,   106,     0,
       1,     2,     3,     4,     5,   335,     0,     0,     0,    87,
       0,     0,     0,    88,     0,    14,    15,     0,    17,     0,
       0,     0,     0,    20,     0,   252,   253,     0,     0,     0,
     164,   165,   166,   167,   168,   169,   170,   171,   172,   173,
     174,    25,    26,     0,     0,     0,     0,     0,     0,    27,
       0,     0,     0,     0,     0,     0,     0,    89,    29,    30,
      31,     0,    32,     0,     0,    90,   175,    34,     0,     0,
      35,    36,    37,    38,     0,     0,     1,     2,     3,     4,
       5,     6,     0,   107,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,     0,     0,    20,
      21,    22,     0,    23,    24,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    25,    26,     0,
       0,     0,     0,     0,     0,    27,     0,     0,     0,     0,
       0,     0,     0,    28,    29,    30,    31,     0,    32,     0,
       0,    33,   145,    34,     0,     0,    35,    36,    37,    38,
       0,     0,     1,     2,     3,     4,     5,     6,     0,    39,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,     0,     0,    20,    21,    22,     0,    23,
      24,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    25,    26,     0,     0,     0,     0,     0,
       0,    27,     0,     0,     0,     0,     0,     0,     0,    28,
      29,    30,    31,     0,    32,     0,     0,    33,   293,    34,
       0,     0,    35,    36,    37,    38,     0,     0,     1,     2,
       3,     4,     5,     6,     0,    39,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,     0,
       0,    20,    21,    22,     0,    23,    24,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    25,
      26,     0,     0,     0,     0,     0,     0,    27,     0,     0,
       0,     0,     0,     0,     0,    28,    29,    30,    31,     0,
      32,     0,     0,    33,   474,    34,     0,     0,    35,    36,
      37,    38,     0,     0,     1,     2,     3,     4,     5,     6,
       0,    39,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,     0,     0,    20,    21,    22,
       0,    23,    24,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    25,    26,     0,     0,     0,
       0,     0,     0,    27,     0,     0,     0,     0,     0,     0,
       0,    28,    29,    30,    31,     0,    32,     0,     0,    33,
     497,    34,     0,     0,    35,    36,    37,    38,     0,     0,
       1,     2,     3,     4,     5,     6,     0,    39,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,     0,     0,    20,    21,    22,     0,    23,    24,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    25,    26,     0,     0,     0,     0,     0,     0,    27,
       0,     0,     0,     0,     0,     0,     0,    28,    29,    30,
      31,     0,    32,     0,     0,    33,     0,    34,     0,     0,
      35,    36,    37,    38,     0,     0,     1,     2,     3,     4,
       5,     6,     0,    39,     7,     8,     9,    10,    11,     0,
      13,    14,    15,    16,    17,    18,    19,     0,     0,    20,
      21,    22,     0,    23,    24,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    25,    26,     0,
       0,     0,     0,     0,     0,    27,     0,     0,     0,     0,
       0,     0,     0,    28,    29,    30,    31,     0,    32,     0,
       0,    33,     0,    34,     0,     0,    35,    36,    37,    38,
       0,     1,     2,     3,     4,     5,     0,     0,     0,    39,
      87,   208,     0,     0,    88,     0,    14,    15,     0,    17,
       0,   162,   163,     0,    20,     0,   164,   165,   166,   167,
     168,   169,   170,   171,   172,   173,   174,     0,     0,     0,
       0,     0,    25,    26,     0,     0,     0,     0,     0,     0,
      27,     0,     0,     0,     0,     0,     0,     0,    89,    29,
      30,    31,   175,    32,     0,     0,    90,     0,    34,     0,
       0,    35,    36,    37,    38,     1,     2,     3,     4,     5,
       0,     0,     0,     0,    87,     0,     0,     0,    88,     0,
      14,    15,     0,    17,     0,   252,   253,     0,    20,     0,
     164,   165,   166,   167,   168,   169,   170,   171,   172,   173,
     174,     0,     0,     0,     0,     0,    25,    26,     0,     0,
       0,     0,     0,     0,    27,     0,     0,     0,     0,     0,
       0,     0,    89,    29,    30,    31,   175,    32,     0,     0,
      90,     0,    34,   296,     0,    35,    36,    37,    38,     1,
       2,     3,     4,     5,     0,     0,     0,     0,    87,     0,
       0,     0,    88,     0,    14,    15,     0,    17,     0,     0,
       0,     0,    20,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      25,    26,     0,     0,     0,     0,     0,     0,    27,     0,
       0,     0,     0,     0,     0,     0,    89,    29,    30,    31,
       0,    32,   299,     0,    90,     0,    34,     0,     0,    35,
      36,    37,    38,     1,     2,     3,     4,     5,     0,     0,
       0,     0,    87,     0,     0,     0,    88,     0,    14,    15,
       0,    17,     0,     0,     0,     0,    20,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    25,    26,     0,     0,     0,     0,
       0,     0,    27,     0,     0,     0,     0,     0,     0,     0,
      89,    29,    30,    31,     0,    32,     0,     0,    90,     0,
      34,   459,     0,    35,    36,    37,    38,     1,     2,     3,
       4,     5,     0,     0,     0,     0,    87,     0,     0,     0,
      88,     0,    14,    15,     0,    17,     0,     0,     0,     0,
      20,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    25,    26,
       0,     0,     0,     0,     0,     0,    27,     0,     0,     0,
       0,     0,     0,     0,    89,    29,    30,    31,     0,    32,
       0,     0,    90,     0,    34,     0,     0,    35,    36,    37,
      38
};

static const short int yycheck[] =
{
       0,    24,    45,    33,   247,    81,   368,     1,     1,    13,
     336,   195,    18,     1,    65,    46,     0,   415,    18,    23,
      67,    67,    24,    69,    60,   351,   352,    42,    32,    10,
       1,     8,     1,    33,    18,     1,    60,     1,    60,    86,
     196,    60,    66,     1,    66,    13,    60,    66,     1,    33,
      93,   149,   197,    60,    65,    23,   198,   155,   199,    66,
      65,   180,   181,     8,    32,   108,    60,   110,    60,    84,
     146,    60,    60,   471,    67,   437,    65,    68,   176,   441,
     111,    81,    86,    85,    14,    15,   412,   449,    69,   273,
      20,    65,    86,    86,   492,    25,    26,    81,    86,    29,
      30,    67,   200,    67,   202,    35,    36,    37,    38,    67,
      87,   473,   355,    65,    67,    86,   478,    86,   274,    86,
      86,    64,    86,   191,   192,   193,   194,   131,    86,   133,
     275,   135,   136,    86,   276,   141,   277,   499,   236,   258,
     259,   141,    87,    42,   470,   149,   146,   182,   183,   184,
      42,   155,   156,     6,     7,   159,   254,   141,    24,    25,
      67,    67,   146,   131,    71,   133,    81,   135,   136,    63,
       6,     7,   176,    64,    40,    41,    67,    67,    82,   222,
     278,    71,   280,    77,    78,    84,    73,    74,   156,    67,
      65,   159,    84,    71,    63,    70,   200,    72,   202,    83,
     443,   269,   270,   271,   272,    65,    43,    60,    77,    78,
      70,    67,    72,    79,    80,    71,    69,    85,   185,   186,
     187,   188,   189,   190,    60,   260,   261,   262,    25,   233,
      73,    74,   236,    69,    65,    24,    25,     0,    67,    70,
      65,    72,    71,    40,    41,   249,    81,   177,   178,   179,
     254,    40,    41,    36,    37,    38,    39,     3,     4,     5,
       6,     7,   360,    82,    65,   233,    12,   290,    83,    70,
      16,    72,    44,    45,   278,    21,   280,    36,    37,    38,
      39,   249,    79,    80,    36,    37,    38,    39,    66,    67,
      79,    80,   342,   343,   344,   345,   263,   264,   265,   266,
     267,   268,    46,    47,    48,    43,    52,   405,    23,   407,
      32,    33,   410,    67,    60,    66,    67,    63,    60,    65,
      60,    60,    68,    81,    70,   255,   256,   257,    46,    47,
      48,   335,   336,    66,    67,    66,    67,    66,    67,    66,
      67,    66,    67,    66,    67,    66,    67,   351,   352,   353,
      66,    67,    82,   451,    66,    67,   360,    66,    67,    66,
      67,    66,    67,    83,    43,    67,    65,   335,    60,   336,
     337,   338,   339,   340,   341,   342,   343,   344,   345,   346,
     347,   348,   349,   350,   351,   352,    86,    64,    60,    60,
      65,    65,   398,    24,   400,   399,   402,    64,   398,   475,
     400,   405,   402,   407,    68,    65,   410,   411,   412,   439,
      64,   415,    68,    60,   398,    60,   400,    67,   402,    60,
      64,    86,    35,     9,    66,    86,    85,    66,    64,    33,
     361,   399,    66,   294,    69,   346,   348,   347,   513,   439,
     349,   239,   350,   411,   414,   412,    99,   451,   243,   466,
     504,    -1,   486,    -1,    -1,   439,    -1,    -1,    -1,    -1,
      -1,   465,    -1,   469,    -1,   488,   470,   471,    -1,   469,
      -1,    -1,    -1,    -1,    -1,   475,   482,    -1,    -1,    -1,
     484,    -1,   482,   489,    -1,   469,    -1,    -1,   492,   489,
      -1,   475,   515,    -1,    -1,    -1,    -1,   465,   482,    -1,
      -1,    -1,   508,   470,   510,   489,   512,   513,   508,    -1,
     510,   517,   512,   513,   520,   521,   484,   517,    -1,    -1,
     520,   521,    -1,    -1,   508,    -1,   510,    -1,   512,   513,
      -1,    -1,    -1,   517,    -1,    -1,   520,   521,     1,    -1,
       3,     4,     5,     6,     7,    24,    -1,    -1,    -1,    12,
      -1,    -1,    -1,    16,    -1,    18,    19,    -1,    21,    -1,
      -1,    -1,    -1,    26,    -1,    44,    45,    -1,    -1,    -1,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    44,    45,    -1,    -1,    -1,    -1,    -1,    -1,    52,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    60,    61,    62,
      63,    -1,    65,    -1,    -1,    68,    85,    70,    -1,    -1,
      73,    74,    75,    76,    -1,    -1,     3,     4,     5,     6,
       7,     8,    -1,    86,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    -1,    -1,    26,
      27,    28,    -1,    30,    31,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    44,    45,    -1,
      -1,    -1,    -1,    -1,    -1,    52,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    60,    61,    62,    63,    -1,    65,    -1,
      -1,    68,    69,    70,    -1,    -1,    73,    74,    75,    76,
      -1,    -1,     3,     4,     5,     6,     7,     8,    -1,    86,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    -1,    -1,    26,    27,    28,    -1,    30,
      31,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    44,    45,    -1,    -1,    -1,    -1,    -1,
      -1,    52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    60,
      61,    62,    63,    -1,    65,    -1,    -1,    68,    69,    70,
      -1,    -1,    73,    74,    75,    76,    -1,    -1,     3,     4,
       5,     6,     7,     8,    -1,    86,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    -1,
      -1,    26,    27,    28,    -1,    30,    31,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    44,
      45,    -1,    -1,    -1,    -1,    -1,    -1,    52,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    60,    61,    62,    63,    -1,
      65,    -1,    -1,    68,    69,    70,    -1,    -1,    73,    74,
      75,    76,    -1,    -1,     3,     4,     5,     6,     7,     8,
      -1,    86,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    -1,    -1,    26,    27,    28,
      -1,    30,    31,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    44,    45,    -1,    -1,    -1,
      -1,    -1,    -1,    52,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    60,    61,    62,    63,    -1,    65,    -1,    -1,    68,
      69,    70,    -1,    -1,    73,    74,    75,    76,    -1,    -1,
       3,     4,     5,     6,     7,     8,    -1,    86,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    -1,    -1,    26,    27,    28,    -1,    30,    31,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    44,    45,    -1,    -1,    -1,    -1,    -1,    -1,    52,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    60,    61,    62,
      63,    -1,    65,    -1,    -1,    68,    -1,    70,    -1,    -1,
      73,    74,    75,    76,    -1,    -1,     3,     4,     5,     6,
       7,     8,    -1,    86,    11,    12,    13,    14,    15,    -1,
      17,    18,    19,    20,    21,    22,    23,    -1,    -1,    26,
      27,    28,    -1,    30,    31,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    44,    45,    -1,
      -1,    -1,    -1,    -1,    -1,    52,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    60,    61,    62,    63,    -1,    65,    -1,
      -1,    68,    -1,    70,    -1,    -1,    73,    74,    75,    76,
      -1,     3,     4,     5,     6,     7,    -1,    -1,    -1,    86,
      12,    13,    -1,    -1,    16,    -1,    18,    19,    -1,    21,
      -1,    44,    45,    -1,    26,    -1,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    -1,    -1,    -1,
      -1,    -1,    44,    45,    -1,    -1,    -1,    -1,    -1,    -1,
      52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    60,    61,
      62,    63,    85,    65,    -1,    -1,    68,    -1,    70,    -1,
      -1,    73,    74,    75,    76,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    12,    -1,    -1,    -1,    16,    -1,
      18,    19,    -1,    21,    -1,    44,    45,    -1,    26,    -1,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    -1,    -1,    -1,    -1,    -1,    44,    45,    -1,    -1,
      -1,    -1,    -1,    -1,    52,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    60,    61,    62,    63,    85,    65,    -1,    -1,
      68,    -1,    70,    71,    -1,    73,    74,    75,    76,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    12,    -1,
      -1,    -1,    16,    -1,    18,    19,    -1,    21,    -1,    -1,
      -1,    -1,    26,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      44,    45,    -1,    -1,    -1,    -1,    -1,    -1,    52,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    60,    61,    62,    63,
      -1,    65,    66,    -1,    68,    -1,    70,    -1,    -1,    73,
      74,    75,    76,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    12,    -1,    -1,    -1,    16,    -1,    18,    19,
      -1,    21,    -1,    -1,    -1,    -1,    26,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    44,    45,    -1,    -1,    -1,    -1,
      -1,    -1,    52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      60,    61,    62,    63,    -1,    65,    -1,    -1,    68,    -1,
      70,    71,    -1,    73,    74,    75,    76,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    12,    -1,    -1,    -1,
      16,    -1,    18,    19,    -1,    21,    -1,    -1,    -1,    -1,
      26,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    44,    45,
      -1,    -1,    -1,    -1,    -1,    -1,    52,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    60,    61,    62,    63,    -1,    65,
      -1,    -1,    68,    -1,    70,    -1,    -1,    73,    74,    75,
      76
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,     3,     4,     5,     6,     7,     8,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      26,    27,    28,    30,    31,    44,    45,    52,    60,    61,
      62,    63,    65,    68,    70,    73,    74,    75,    76,    86,
      88,    93,    94,    99,   101,   103,   107,   109,   110,   112,
     114,   116,   118,   121,   124,   127,   130,   133,   136,   139,
     142,   145,   149,   150,   151,   153,   158,   163,   164,   165,
     166,   169,   170,   171,   172,   173,   179,   180,   181,   182,
     186,   187,   188,     1,    60,    86,    65,    12,    16,    60,
      68,    92,    93,    98,   100,   183,    60,   154,   156,    60,
     159,   160,     1,    60,    86,    60,     1,    86,    98,   100,
     102,   106,   108,   110,   111,   113,   115,   117,   119,   122,
     125,   128,   131,   134,   137,   140,   143,   147,   106,   111,
     111,    65,   150,    65,   111,    65,    65,   147,   151,   111,
     111,    64,   111,   111,   147,    69,   187,    67,    95,    96,
      97,   111,   111,   111,   111,    65,    70,    72,   104,    70,
      72,   104,    44,    45,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    85,   146,    63,    77,    78,
      73,    74,    46,    47,    48,    24,    25,    40,    41,    79,
      80,    36,    37,    38,    39,    81,    82,    83,    43,    42,
      84,     1,    67,    86,     0,   188,     1,    86,    13,   106,
     117,   120,   123,   126,   129,   132,   135,   138,   141,   144,
     148,   168,    98,   100,    60,    65,     6,     7,    60,    69,
      89,    90,    91,    70,    72,   104,    85,   161,     1,    67,
      86,   161,     1,    67,    86,     1,    86,    65,   104,    70,
      72,   104,    44,    45,   146,    63,    77,    78,    73,    74,
      46,    47,    48,    24,    25,    40,    41,    79,    80,    36,
      37,    38,    39,    81,    82,    83,    43,    42,    84,     1,
      67,    86,   147,    23,   147,   147,   147,     1,    86,    32,
      33,   150,    66,    69,    67,    71,    71,   143,    67,    66,
     105,   143,   147,    60,   147,    60,   143,   111,   111,   111,
     113,   113,   115,   115,   115,   117,   117,   117,   117,   117,
     117,   119,   119,   119,   119,   122,   125,   128,   131,   134,
     143,   143,    60,   155,   157,    24,   146,    25,    40,    41,
      79,    80,    36,    37,    38,    39,    81,    82,    83,    43,
      42,    84,    67,    86,   104,    65,    60,    66,   184,    60,
      64,    67,    69,   147,    60,   143,   156,   160,    66,   184,
     147,    60,   143,   111,   111,   111,   113,   113,   115,   115,
     115,   117,   117,   117,   117,   117,   117,   119,   119,   119,
     119,   122,   125,   128,   131,   134,   143,   143,    66,    65,
      66,    66,    66,    65,   151,    96,    66,    67,    71,    71,
      64,    24,    85,   162,    67,    86,   147,   106,   144,   117,
     117,   117,   117,   117,   120,   120,   120,   120,   123,   126,
     129,   132,   135,   144,   144,   147,   167,    66,   184,    68,
     185,    66,    67,    65,   143,    69,    90,    71,   185,    66,
      71,    64,   150,   147,   150,    68,   174,   150,    60,    71,
     143,   143,   143,   147,   144,    24,    60,   157,   167,    66,
      64,    86,   185,    66,    69,   187,   185,    60,    66,   184,
     185,   143,    35,    66,     9,   175,   176,   177,    66,    66,
     147,   162,    86,   150,   144,   167,   185,    69,   185,    66,
     150,   147,    10,    69,   178,   177,   151,   150,    66,   167,
      66,   185,    64,    64,   175,    33,   150,    66,   150,   150,
     152,   152,    69,   151,   150,   150
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


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
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (0)


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (N)								\
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (0)
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
              (Loc).first_line, (Loc).first_column,	\
              (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
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

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)		\
do {								\
  if (yydebug)							\
    {								\
      YYFPRINTF (stderr, "%s ", Title);				\
      yysymprint (stderr,					\
                  Type, Value, Location);	\
      YYFPRINTF (stderr, "\n");					\
    }								\
} while (0)

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_stack_print (short int *bottom, short int *top)
#else
static void
yy_stack_print (bottom, top)
    short int *bottom;
    short int *top;
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
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu), ",
             yyrule - 1, yylno);
  /* Print the symbols being reduced, and their result.  */
  for (yyi = yyprhs[yyrule]; 0 <= yyrhs[yyi]; yyi++)
    YYFPRINTF (stderr, "%s ", yytname[yyrhs[yyi]]);
  YYFPRINTF (stderr, "-> %s\n", yytname[yyr1[yyrule]]);
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
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
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
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

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
  const char *yys = yystr;

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
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      size_t yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

#endif /* YYERROR_VERBOSE */



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
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  YY_LOCATION_PRINT (yyoutput, *yylocationp);
  YYFPRINTF (yyoutput, ": ");

# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
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
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, YYLTYPE *yylocationp)
#else
static void
yydestruct (yymsg, yytype, yyvaluep, yylocationp)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
    YYLTYPE *yylocationp;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;
  (void) yylocationp;

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

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



/* The look-ahead symbol.  */
int yychar;

/* The semantic value of the look-ahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;
/* Location data for the look-ahead symbol.  */
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
    ;
#endif
#endif
{
  
  int yystate;
  int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Look-ahead token as an internal (translated) token number.  */
  int yytoken = 0;

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  short int yyssa[YYINITDEPTH];
  short int *yyss = yyssa;
  short int *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  YYSTYPE *yyvsp;

  /* The location stack.  */
  YYLTYPE yylsa[YYINITDEPTH];
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;
  /* The locations where the error started and ended. */
  YYLTYPE yyerror_range[2];

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
#if YYLTYPE_IS_TRIVIAL
  /* Initialize the default location before parsing starts.  */
  yylloc.first_line   = yylloc.last_line   = 1;
  yylloc.first_column = yylloc.last_column = 0;
#endif

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
	short int *yyss1 = yyss;
	YYLTYPE *yyls1 = yyls;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
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
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	short int *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
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
/* Read a look-ahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to look-ahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a look-ahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid look-ahead symbol.  */
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
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
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

  /* Shift the look-ahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

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
  YYLLOC_DEFAULT (yyloc, yylsp - yylen, yylen);
  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 189 "grammar.y"
    { (yyval.node) = new NullNode(); ;}
    break;

  case 3:
#line 190 "grammar.y"
    { (yyval.node) = new BooleanNode(true); ;}
    break;

  case 4:
#line 191 "grammar.y"
    { (yyval.node) = new BooleanNode(false); ;}
    break;

  case 5:
#line 192 "grammar.y"
    { (yyval.node) = new NumberNode((yyvsp[0].dval)); ;}
    break;

  case 6:
#line 193 "grammar.y"
    { (yyval.node) = new StringNode((yyvsp[0].ustr)); ;}
    break;

  case 7:
#line 194 "grammar.y"
    {
                                            Lexer *l = Lexer::curr();
                                            if (!l->scanRegExp()) YYABORT;
                                            (yyval.node) = new RegExpNode(l->pattern, l->flags);
                                        ;}
    break;

  case 8:
#line 199 "grammar.y"
    {
                                            Lexer *l = Lexer::curr();
                                            if (!l->scanRegExp()) YYABORT;
                                            (yyval.node) = new RegExpNode(UString('=') + l->pattern, l->flags);
                                        ;}
    break;

  case 9:
#line 207 "grammar.y"
    { (yyval.pname) = new PropertyNameNode(*(yyvsp[0].ident)); ;}
    break;

  case 10:
#line 208 "grammar.y"
    { (yyval.pname) = new PropertyNameNode(Identifier(*(yyvsp[0].ustr))); ;}
    break;

  case 11:
#line 209 "grammar.y"
    { (yyval.pname) = new PropertyNameNode((yyvsp[0].dval)); ;}
    break;

  case 12:
#line 213 "grammar.y"
    { (yyval.pnode) = new PropertyNode((yyvsp[-2].pname), (yyvsp[0].node), PropertyNode::Constant); ;}
    break;

  case 13:
#line 214 "grammar.y"
    { if (!makeGetterOrSetterPropertyNode((yyval.pnode), *(yyvsp[-4].ident), *(yyvsp[-3].ident), 0, (yyvsp[0].body))) YYABORT; ;}
    break;

  case 14:
#line 216 "grammar.y"
    { if (!makeGetterOrSetterPropertyNode((yyval.pnode), *(yyvsp[-5].ident), *(yyvsp[-4].ident), (yyvsp[-2].param), (yyvsp[0].body))) YYABORT; ;}
    break;

  case 15:
#line 220 "grammar.y"
    { (yyval.plist) = new PropertyListNode((yyvsp[0].pnode)); ;}
    break;

  case 16:
#line 221 "grammar.y"
    { (yyval.plist) = new PropertyListNode((yyvsp[0].pnode), (yyvsp[-2].plist)); ;}
    break;

  case 18:
#line 226 "grammar.y"
    { (yyval.node) = new ObjectLiteralNode(); ;}
    break;

  case 19:
#line 227 "grammar.y"
    { (yyval.node) = new ObjectLiteralNode((yyvsp[-1].plist)); ;}
    break;

  case 20:
#line 229 "grammar.y"
    { (yyval.node) = new ObjectLiteralNode((yyvsp[-2].plist)); ;}
    break;

  case 21:
#line 233 "grammar.y"
    { (yyval.node) = new ThisNode(); ;}
    break;

  case 24:
#line 236 "grammar.y"
    { (yyval.node) = new ResolveNode(*(yyvsp[0].ident)); ;}
    break;

  case 25:
#line 237 "grammar.y"
    { (yyval.node) = (yyvsp[-1].node)->isResolveNode() ? (yyvsp[-1].node) : new GroupNode((yyvsp[-1].node)); ;}
    break;

  case 26:
#line 241 "grammar.y"
    { (yyval.node) = new ArrayNode((yyvsp[-1].ival)); ;}
    break;

  case 27:
#line 242 "grammar.y"
    { (yyval.node) = new ArrayNode((yyvsp[-1].elm)); ;}
    break;

  case 28:
#line 243 "grammar.y"
    { (yyval.node) = new ArrayNode((yyvsp[-1].ival), (yyvsp[-3].elm)); ;}
    break;

  case 29:
#line 247 "grammar.y"
    { (yyval.elm) = new ElementNode((yyvsp[-1].ival), (yyvsp[0].node)); ;}
    break;

  case 30:
#line 249 "grammar.y"
    { (yyval.elm) = new ElementNode((yyvsp[-3].elm), (yyvsp[-1].ival), (yyvsp[0].node)); ;}
    break;

  case 31:
#line 253 "grammar.y"
    { (yyval.ival) = 0; ;}
    break;

  case 33:
#line 258 "grammar.y"
    { (yyval.ival) = 1; ;}
    break;

  case 34:
#line 259 "grammar.y"
    { (yyval.ival) = (yyvsp[-1].ival) + 1; ;}
    break;

  case 36:
#line 264 "grammar.y"
    { (yyval.node) = (yyvsp[0].funcExpr); ;}
    break;

  case 37:
#line 265 "grammar.y"
    { (yyval.node) = new BracketAccessorNode((yyvsp[-3].node), (yyvsp[-1].node)); ;}
    break;

  case 38:
#line 266 "grammar.y"
    { (yyval.node) = new DotAccessorNode((yyvsp[-2].node), *(yyvsp[0].ident)); ;}
    break;

  case 39:
#line 267 "grammar.y"
    { (yyval.node) = new NewExprNode((yyvsp[-1].node), (yyvsp[0].args)); ;}
    break;

  case 41:
#line 272 "grammar.y"
    { (yyval.node) = new BracketAccessorNode((yyvsp[-3].node), (yyvsp[-1].node)); ;}
    break;

  case 42:
#line 273 "grammar.y"
    { (yyval.node) = new DotAccessorNode((yyvsp[-2].node), *(yyvsp[0].ident)); ;}
    break;

  case 43:
#line 274 "grammar.y"
    { (yyval.node) = new NewExprNode((yyvsp[-1].node), (yyvsp[0].args)); ;}
    break;

  case 45:
#line 279 "grammar.y"
    { (yyval.node) = new NewExprNode((yyvsp[0].node)); ;}
    break;

  case 47:
#line 284 "grammar.y"
    { (yyval.node) = new NewExprNode((yyvsp[0].node)); ;}
    break;

  case 48:
#line 288 "grammar.y"
    { (yyval.node) = makeFunctionCallNode((yyvsp[-1].node), (yyvsp[0].args)); ;}
    break;

  case 49:
#line 289 "grammar.y"
    { (yyval.node) = makeFunctionCallNode((yyvsp[-1].node), (yyvsp[0].args)); ;}
    break;

  case 50:
#line 290 "grammar.y"
    { (yyval.node) = new BracketAccessorNode((yyvsp[-3].node), (yyvsp[-1].node)); ;}
    break;

  case 51:
#line 291 "grammar.y"
    { (yyval.node) = new DotAccessorNode((yyvsp[-2].node), *(yyvsp[0].ident)); ;}
    break;

  case 52:
#line 295 "grammar.y"
    { (yyval.node) = makeFunctionCallNode((yyvsp[-1].node), (yyvsp[0].args)); ;}
    break;

  case 53:
#line 296 "grammar.y"
    { (yyval.node) = makeFunctionCallNode((yyvsp[-1].node), (yyvsp[0].args)); ;}
    break;

  case 54:
#line 297 "grammar.y"
    { (yyval.node) = new BracketAccessorNode((yyvsp[-3].node), (yyvsp[-1].node)); ;}
    break;

  case 55:
#line 298 "grammar.y"
    { (yyval.node) = new DotAccessorNode((yyvsp[-2].node), *(yyvsp[0].ident)); ;}
    break;

  case 56:
#line 302 "grammar.y"
    { (yyval.args) = new ArgumentsNode(); ;}
    break;

  case 57:
#line 303 "grammar.y"
    { (yyval.args) = new ArgumentsNode((yyvsp[-1].alist)); ;}
    break;

  case 58:
#line 307 "grammar.y"
    { (yyval.alist) = new ArgumentListNode((yyvsp[0].node)); ;}
    break;

  case 59:
#line 308 "grammar.y"
    { (yyval.alist) = new ArgumentListNode((yyvsp[-2].alist), (yyvsp[0].node)); ;}
    break;

  case 65:
#line 323 "grammar.y"
    { if (!makePostfixNode((yyval.node), (yyvsp[-1].node), OpPlusPlus)) YYABORT; ;}
    break;

  case 66:
#line 324 "grammar.y"
    { if (!makePostfixNode((yyval.node), (yyvsp[-1].node), OpMinusMinus)) YYABORT; ;}
    break;

  case 68:
#line 329 "grammar.y"
    { if (!makePostfixNode((yyval.node), (yyvsp[-1].node), OpPlusPlus)) YYABORT; ;}
    break;

  case 69:
#line 330 "grammar.y"
    { if (!makePostfixNode((yyval.node), (yyvsp[-1].node), OpMinusMinus)) YYABORT; ;}
    break;

  case 70:
#line 334 "grammar.y"
    { (yyval.node) = makeDeleteNode((yyvsp[0].node)); ;}
    break;

  case 71:
#line 335 "grammar.y"
    { (yyval.node) = new VoidNode((yyvsp[0].node)); ;}
    break;

  case 72:
#line 336 "grammar.y"
    { (yyval.node) = makeTypeOfNode((yyvsp[0].node)); ;}
    break;

  case 73:
#line 337 "grammar.y"
    { if (!makePrefixNode((yyval.node), (yyvsp[0].node), OpPlusPlus)) YYABORT; ;}
    break;

  case 74:
#line 338 "grammar.y"
    { if (!makePrefixNode((yyval.node), (yyvsp[0].node), OpPlusPlus)) YYABORT; ;}
    break;

  case 75:
#line 339 "grammar.y"
    { if (!makePrefixNode((yyval.node), (yyvsp[0].node), OpMinusMinus)) YYABORT; ;}
    break;

  case 76:
#line 340 "grammar.y"
    { if (!makePrefixNode((yyval.node), (yyvsp[0].node), OpMinusMinus)) YYABORT; ;}
    break;

  case 77:
#line 341 "grammar.y"
    { (yyval.node) = new UnaryPlusNode((yyvsp[0].node)); ;}
    break;

  case 78:
#line 342 "grammar.y"
    { (yyval.node) = new NegateNode((yyvsp[0].node)); ;}
    break;

  case 79:
#line 343 "grammar.y"
    { (yyval.node) = new BitwiseNotNode((yyvsp[0].node)); ;}
    break;

  case 80:
#line 344 "grammar.y"
    { (yyval.node) = new LogicalNotNode((yyvsp[0].node)); ;}
    break;

  case 86:
#line 358 "grammar.y"
    { (yyval.node) = new MultNode((yyvsp[-2].node), (yyvsp[0].node), '*'); ;}
    break;

  case 87:
#line 359 "grammar.y"
    { (yyval.node) = new MultNode((yyvsp[-2].node), (yyvsp[0].node), '/'); ;}
    break;

  case 88:
#line 360 "grammar.y"
    { (yyval.node) = new MultNode((yyvsp[-2].node), (yyvsp[0].node),'%'); ;}
    break;

  case 90:
#line 366 "grammar.y"
    { (yyval.node) = new MultNode((yyvsp[-2].node), (yyvsp[0].node), '*'); ;}
    break;

  case 91:
#line 368 "grammar.y"
    { (yyval.node) = new MultNode((yyvsp[-2].node), (yyvsp[0].node), '/'); ;}
    break;

  case 92:
#line 370 "grammar.y"
    { (yyval.node) = new MultNode((yyvsp[-2].node), (yyvsp[0].node),'%'); ;}
    break;

  case 94:
#line 375 "grammar.y"
    { (yyval.node) = new AddNode((yyvsp[-2].node), (yyvsp[0].node), '+'); ;}
    break;

  case 95:
#line 376 "grammar.y"
    { (yyval.node) = new AddNode((yyvsp[-2].node), (yyvsp[0].node), '-'); ;}
    break;

  case 97:
#line 382 "grammar.y"
    { (yyval.node) = new AddNode((yyvsp[-2].node), (yyvsp[0].node), '+'); ;}
    break;

  case 98:
#line 384 "grammar.y"
    { (yyval.node) = new AddNode((yyvsp[-2].node), (yyvsp[0].node), '-'); ;}
    break;

  case 100:
#line 389 "grammar.y"
    { (yyval.node) = new ShiftNode((yyvsp[-2].node), OpLShift, (yyvsp[0].node)); ;}
    break;

  case 101:
#line 390 "grammar.y"
    { (yyval.node) = new ShiftNode((yyvsp[-2].node), OpRShift, (yyvsp[0].node)); ;}
    break;

  case 102:
#line 391 "grammar.y"
    { (yyval.node) = new ShiftNode((yyvsp[-2].node), OpURShift, (yyvsp[0].node)); ;}
    break;

  case 104:
#line 396 "grammar.y"
    { (yyval.node) = new ShiftNode((yyvsp[-2].node), OpLShift, (yyvsp[0].node)); ;}
    break;

  case 105:
#line 397 "grammar.y"
    { (yyval.node) = new ShiftNode((yyvsp[-2].node), OpRShift, (yyvsp[0].node)); ;}
    break;

  case 106:
#line 398 "grammar.y"
    { (yyval.node) = new ShiftNode((yyvsp[-2].node), OpURShift, (yyvsp[0].node)); ;}
    break;

  case 108:
#line 403 "grammar.y"
    { (yyval.node) = new RelationalNode((yyvsp[-2].node), OpLess, (yyvsp[0].node)); ;}
    break;

  case 109:
#line 404 "grammar.y"
    { (yyval.node) = new RelationalNode((yyvsp[-2].node), OpGreater, (yyvsp[0].node)); ;}
    break;

  case 110:
#line 405 "grammar.y"
    { (yyval.node) = new RelationalNode((yyvsp[-2].node), OpLessEq, (yyvsp[0].node)); ;}
    break;

  case 111:
#line 406 "grammar.y"
    { (yyval.node) = new RelationalNode((yyvsp[-2].node), OpGreaterEq, (yyvsp[0].node)); ;}
    break;

  case 112:
#line 407 "grammar.y"
    { (yyval.node) = new RelationalNode((yyvsp[-2].node), OpInstanceOf, (yyvsp[0].node)); ;}
    break;

  case 113:
#line 408 "grammar.y"
    { (yyval.node) = new RelationalNode((yyvsp[-2].node), OpIn, (yyvsp[0].node)); ;}
    break;

  case 115:
#line 413 "grammar.y"
    { (yyval.node) = new RelationalNode((yyvsp[-2].node), OpLess, (yyvsp[0].node)); ;}
    break;

  case 116:
#line 414 "grammar.y"
    { (yyval.node) = new RelationalNode((yyvsp[-2].node), OpGreater, (yyvsp[0].node)); ;}
    break;

  case 117:
#line 415 "grammar.y"
    { (yyval.node) = new RelationalNode((yyvsp[-2].node), OpLessEq, (yyvsp[0].node)); ;}
    break;

  case 118:
#line 416 "grammar.y"
    { (yyval.node) = new RelationalNode((yyvsp[-2].node), OpGreaterEq, (yyvsp[0].node)); ;}
    break;

  case 119:
#line 418 "grammar.y"
    { (yyval.node) = new RelationalNode((yyvsp[-2].node), OpInstanceOf, (yyvsp[0].node)); ;}
    break;

  case 121:
#line 423 "grammar.y"
    { (yyval.node) = new RelationalNode((yyvsp[-2].node), OpLess, (yyvsp[0].node)); ;}
    break;

  case 122:
#line 424 "grammar.y"
    { (yyval.node) = new RelationalNode((yyvsp[-2].node), OpGreater, (yyvsp[0].node)); ;}
    break;

  case 123:
#line 425 "grammar.y"
    { (yyval.node) = new RelationalNode((yyvsp[-2].node), OpLessEq, (yyvsp[0].node)); ;}
    break;

  case 124:
#line 426 "grammar.y"
    { (yyval.node) = new RelationalNode((yyvsp[-2].node), OpGreaterEq, (yyvsp[0].node)); ;}
    break;

  case 125:
#line 428 "grammar.y"
    { (yyval.node) = new RelationalNode((yyvsp[-2].node), OpInstanceOf, (yyvsp[0].node)); ;}
    break;

  case 126:
#line 429 "grammar.y"
    { (yyval.node) = new RelationalNode((yyvsp[-2].node), OpIn, (yyvsp[0].node)); ;}
    break;

  case 128:
#line 434 "grammar.y"
    { (yyval.node) = new EqualNode((yyvsp[-2].node), OpEqEq, (yyvsp[0].node)); ;}
    break;

  case 129:
#line 435 "grammar.y"
    { (yyval.node) = new EqualNode((yyvsp[-2].node), OpNotEq, (yyvsp[0].node)); ;}
    break;

  case 130:
#line 436 "grammar.y"
    { (yyval.node) = new EqualNode((yyvsp[-2].node), OpStrEq, (yyvsp[0].node)); ;}
    break;

  case 131:
#line 437 "grammar.y"
    { (yyval.node) = new EqualNode((yyvsp[-2].node), OpStrNEq, (yyvsp[0].node));;}
    break;

  case 133:
#line 443 "grammar.y"
    { (yyval.node) = new EqualNode((yyvsp[-2].node), OpEqEq, (yyvsp[0].node)); ;}
    break;

  case 134:
#line 445 "grammar.y"
    { (yyval.node) = new EqualNode((yyvsp[-2].node), OpNotEq, (yyvsp[0].node)); ;}
    break;

  case 135:
#line 447 "grammar.y"
    { (yyval.node) = new EqualNode((yyvsp[-2].node), OpStrEq, (yyvsp[0].node)); ;}
    break;

  case 136:
#line 449 "grammar.y"
    { (yyval.node) = new EqualNode((yyvsp[-2].node), OpStrNEq, (yyvsp[0].node));;}
    break;

  case 138:
#line 455 "grammar.y"
    { (yyval.node) = new EqualNode((yyvsp[-2].node), OpEqEq, (yyvsp[0].node)); ;}
    break;

  case 139:
#line 456 "grammar.y"
    { (yyval.node) = new EqualNode((yyvsp[-2].node), OpNotEq, (yyvsp[0].node)); ;}
    break;

  case 140:
#line 458 "grammar.y"
    { (yyval.node) = new EqualNode((yyvsp[-2].node), OpStrEq, (yyvsp[0].node)); ;}
    break;

  case 141:
#line 460 "grammar.y"
    { (yyval.node) = new EqualNode((yyvsp[-2].node), OpStrNEq, (yyvsp[0].node));;}
    break;

  case 143:
#line 465 "grammar.y"
    { (yyval.node) = new BitOperNode((yyvsp[-2].node), OpBitAnd, (yyvsp[0].node)); ;}
    break;

  case 145:
#line 471 "grammar.y"
    { (yyval.node) = new BitOperNode((yyvsp[-2].node), OpBitAnd, (yyvsp[0].node)); ;}
    break;

  case 147:
#line 476 "grammar.y"
    { (yyval.node) = new BitOperNode((yyvsp[-2].node), OpBitAnd, (yyvsp[0].node)); ;}
    break;

  case 149:
#line 481 "grammar.y"
    { (yyval.node) = new BitOperNode((yyvsp[-2].node), OpBitXOr, (yyvsp[0].node)); ;}
    break;

  case 151:
#line 487 "grammar.y"
    { (yyval.node) = new BitOperNode((yyvsp[-2].node), OpBitXOr, (yyvsp[0].node)); ;}
    break;

  case 153:
#line 493 "grammar.y"
    { (yyval.node) = new BitOperNode((yyvsp[-2].node), OpBitXOr, (yyvsp[0].node)); ;}
    break;

  case 155:
#line 498 "grammar.y"
    { (yyval.node) = new BitOperNode((yyvsp[-2].node), OpBitOr, (yyvsp[0].node)); ;}
    break;

  case 157:
#line 504 "grammar.y"
    { (yyval.node) = new BitOperNode((yyvsp[-2].node), OpBitOr, (yyvsp[0].node)); ;}
    break;

  case 159:
#line 510 "grammar.y"
    { (yyval.node) = new BitOperNode((yyvsp[-2].node), OpBitOr, (yyvsp[0].node)); ;}
    break;

  case 161:
#line 515 "grammar.y"
    { (yyval.node) = new BinaryLogicalNode((yyvsp[-2].node), OpAnd, (yyvsp[0].node)); ;}
    break;

  case 163:
#line 521 "grammar.y"
    { (yyval.node) = new BinaryLogicalNode((yyvsp[-2].node), OpAnd, (yyvsp[0].node)); ;}
    break;

  case 165:
#line 527 "grammar.y"
    { (yyval.node) = new BinaryLogicalNode((yyvsp[-2].node), OpAnd, (yyvsp[0].node)); ;}
    break;

  case 167:
#line 532 "grammar.y"
    { (yyval.node) = new BinaryLogicalNode((yyvsp[-2].node), OpOr, (yyvsp[0].node)); ;}
    break;

  case 169:
#line 538 "grammar.y"
    { (yyval.node) = new BinaryLogicalNode((yyvsp[-2].node), OpOr, (yyvsp[0].node)); ;}
    break;

  case 171:
#line 543 "grammar.y"
    { (yyval.node) = new BinaryLogicalNode((yyvsp[-2].node), OpOr, (yyvsp[0].node)); ;}
    break;

  case 173:
#line 549 "grammar.y"
    { (yyval.node) = new ConditionalNode((yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node)); ;}
    break;

  case 175:
#line 555 "grammar.y"
    { (yyval.node) = new ConditionalNode((yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node)); ;}
    break;

  case 177:
#line 561 "grammar.y"
    { (yyval.node) = new ConditionalNode((yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node)); ;}
    break;

  case 179:
#line 567 "grammar.y"
    { if (!makeAssignNode((yyval.node), (yyvsp[-2].node), (yyvsp[-1].op), (yyvsp[0].node))) YYABORT; ;}
    break;

  case 181:
#line 573 "grammar.y"
    { if (!makeAssignNode((yyval.node), (yyvsp[-2].node), (yyvsp[-1].op), (yyvsp[0].node))) YYABORT; ;}
    break;

  case 183:
#line 579 "grammar.y"
    { if (!makeAssignNode((yyval.node), (yyvsp[-2].node), (yyvsp[-1].op), (yyvsp[0].node))) YYABORT; ;}
    break;

  case 184:
#line 583 "grammar.y"
    { (yyval.op) = OpEqual; ;}
    break;

  case 185:
#line 584 "grammar.y"
    { (yyval.op) = OpPlusEq; ;}
    break;

  case 186:
#line 585 "grammar.y"
    { (yyval.op) = OpMinusEq; ;}
    break;

  case 187:
#line 586 "grammar.y"
    { (yyval.op) = OpMultEq; ;}
    break;

  case 188:
#line 587 "grammar.y"
    { (yyval.op) = OpDivEq; ;}
    break;

  case 189:
#line 588 "grammar.y"
    { (yyval.op) = OpLShift; ;}
    break;

  case 190:
#line 589 "grammar.y"
    { (yyval.op) = OpRShift; ;}
    break;

  case 191:
#line 590 "grammar.y"
    { (yyval.op) = OpURShift; ;}
    break;

  case 192:
#line 591 "grammar.y"
    { (yyval.op) = OpAndEq; ;}
    break;

  case 193:
#line 592 "grammar.y"
    { (yyval.op) = OpXOrEq; ;}
    break;

  case 194:
#line 593 "grammar.y"
    { (yyval.op) = OpOrEq; ;}
    break;

  case 195:
#line 594 "grammar.y"
    { (yyval.op) = OpModEq; ;}
    break;

  case 197:
#line 599 "grammar.y"
    { (yyval.node) = new CommaNode((yyvsp[-2].node), (yyvsp[0].node)); ;}
    break;

  case 199:
#line 604 "grammar.y"
    { (yyval.node) = new CommaNode((yyvsp[-2].node), (yyvsp[0].node)); ;}
    break;

  case 201:
#line 609 "grammar.y"
    { (yyval.node) = new CommaNode((yyvsp[-2].node), (yyvsp[0].node)); ;}
    break;

  case 217:
#line 631 "grammar.y"
    { (yyval.stat) = new BlockNode(0); DBG((yyval.stat), (yylsp[0]), (yylsp[0])); ;}
    break;

  case 218:
#line 632 "grammar.y"
    { (yyval.stat) = new BlockNode((yyvsp[-1].srcs)); DBG((yyval.stat), (yylsp[0]), (yylsp[0])); ;}
    break;

  case 219:
#line 636 "grammar.y"
    { (yyval.slist) = new StatListNode((yyvsp[0].stat)); ;}
    break;

  case 220:
#line 637 "grammar.y"
    { (yyval.slist) = new StatListNode((yyvsp[-1].slist), (yyvsp[0].stat)); ;}
    break;

  case 221:
#line 641 "grammar.y"
    { (yyval.stat) = new VarStatementNode((yyvsp[-1].vlist)); DBG((yyval.stat), (yylsp[-2]), (yylsp[0])); ;}
    break;

  case 222:
#line 642 "grammar.y"
    { (yyval.stat) = new VarStatementNode((yyvsp[-1].vlist)); DBG((yyval.stat), (yylsp[-2]), (yylsp[-1])); AUTO_SEMICOLON; ;}
    break;

  case 223:
#line 646 "grammar.y"
    { (yyval.vlist) = new VarDeclListNode((yyvsp[0].decl)); ;}
    break;

  case 224:
#line 648 "grammar.y"
    { (yyval.vlist) = new VarDeclListNode((yyvsp[-2].vlist), (yyvsp[0].decl)); ;}
    break;

  case 225:
#line 652 "grammar.y"
    { (yyval.vlist) = new VarDeclListNode((yyvsp[0].decl)); ;}
    break;

  case 226:
#line 654 "grammar.y"
    { (yyval.vlist) = new VarDeclListNode((yyvsp[-2].vlist), (yyvsp[0].decl)); ;}
    break;

  case 227:
#line 658 "grammar.y"
    { (yyval.decl) = new VarDeclNode(*(yyvsp[0].ident), 0, VarDeclNode::Variable); ;}
    break;

  case 228:
#line 659 "grammar.y"
    { (yyval.decl) = new VarDeclNode(*(yyvsp[-1].ident), (yyvsp[0].init), VarDeclNode::Variable); ;}
    break;

  case 229:
#line 663 "grammar.y"
    { (yyval.decl) = new VarDeclNode(*(yyvsp[0].ident), 0, VarDeclNode::Variable); ;}
    break;

  case 230:
#line 664 "grammar.y"
    { (yyval.decl) = new VarDeclNode(*(yyvsp[-1].ident), (yyvsp[0].init), VarDeclNode::Variable); ;}
    break;

  case 231:
#line 668 "grammar.y"
    { (yyval.stat) = new VarStatementNode((yyvsp[-1].vlist)); DBG((yyval.stat), (yylsp[-2]), (yylsp[0])); ;}
    break;

  case 232:
#line 669 "grammar.y"
    { (yyval.stat) = new VarStatementNode((yyvsp[-1].vlist)); DBG((yyval.stat), (yylsp[-2]), (yylsp[-1])); AUTO_SEMICOLON; ;}
    break;

  case 233:
#line 673 "grammar.y"
    { (yyval.vlist) = new VarDeclListNode((yyvsp[0].decl)); ;}
    break;

  case 234:
#line 675 "grammar.y"
    { (yyval.vlist) = new VarDeclListNode((yyvsp[-2].vlist), (yyvsp[0].decl)); ;}
    break;

  case 235:
#line 679 "grammar.y"
    { (yyval.decl) = new VarDeclNode(*(yyvsp[0].ident), 0, VarDeclNode::Constant); ;}
    break;

  case 236:
#line 680 "grammar.y"
    { (yyval.decl) = new VarDeclNode(*(yyvsp[-1].ident), (yyvsp[0].init), VarDeclNode::Constant); ;}
    break;

  case 237:
#line 684 "grammar.y"
    { (yyval.init) = new AssignExprNode((yyvsp[0].node)); ;}
    break;

  case 238:
#line 688 "grammar.y"
    { (yyval.init) = new AssignExprNode((yyvsp[0].node)); ;}
    break;

  case 239:
#line 692 "grammar.y"
    { (yyval.stat) = new EmptyStatementNode(); ;}
    break;

  case 240:
#line 696 "grammar.y"
    { (yyval.stat) = new ExprStatementNode((yyvsp[-1].node)); DBG((yyval.stat), (yylsp[-1]), (yylsp[0])); ;}
    break;

  case 241:
#line 697 "grammar.y"
    { (yyval.stat) = new ExprStatementNode((yyvsp[-1].node)); DBG((yyval.stat), (yylsp[-1]), (yylsp[-1])); AUTO_SEMICOLON; ;}
    break;

  case 242:
#line 702 "grammar.y"
    { (yyval.stat) = new IfNode((yyvsp[-2].node), (yyvsp[0].stat), 0); DBG((yyval.stat), (yylsp[-4]), (yylsp[-1])); ;}
    break;

  case 243:
#line 704 "grammar.y"
    { (yyval.stat) = new IfNode((yyvsp[-4].node), (yyvsp[-2].stat), (yyvsp[0].stat)); DBG((yyval.stat), (yylsp[-6]), (yylsp[-3])); ;}
    break;

  case 244:
#line 708 "grammar.y"
    { (yyval.stat) = new DoWhileNode((yyvsp[-4].stat), (yyvsp[-1].node)); DBG((yyval.stat), (yylsp[-5]), (yylsp[-3]));;}
    break;

  case 245:
#line 709 "grammar.y"
    { (yyval.stat) = new WhileNode((yyvsp[-2].node), (yyvsp[0].stat)); DBG((yyval.stat), (yylsp[-4]), (yylsp[-1])); ;}
    break;

  case 246:
#line 711 "grammar.y"
    { (yyval.stat) = new ForNode((yyvsp[-6].node), (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].stat)); DBG((yyval.stat), (yylsp[-8]), (yylsp[-1])); ;}
    break;

  case 247:
#line 713 "grammar.y"
    { (yyval.stat) = new ForNode((yyvsp[-6].vlist), (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].stat)); DBG((yyval.stat), (yylsp[-9]), (yylsp[-1])); ;}
    break;

  case 248:
#line 715 "grammar.y"
    {
                                            Node *n = (yyvsp[-4].node)->nodeInsideAllParens();
                                            if (!n->isLocation())
                                                YYABORT;
                                            (yyval.stat) = new ForInNode(n, (yyvsp[-2].node), (yyvsp[0].stat));
                                            DBG((yyval.stat), (yylsp[-6]), (yylsp[-1]));
                                        ;}
    break;

  case 249:
#line 723 "grammar.y"
    { (yyval.stat) = new ForInNode(*(yyvsp[-4].ident), 0, (yyvsp[-2].node), (yyvsp[0].stat)); DBG((yyval.stat), (yylsp[-7]), (yylsp[-1])); ;}
    break;

  case 250:
#line 725 "grammar.y"
    { (yyval.stat) = new ForInNode(*(yyvsp[-5].ident), (yyvsp[-4].init), (yyvsp[-2].node), (yyvsp[0].stat)); DBG((yyval.stat), (yylsp[-8]), (yylsp[-1])); ;}
    break;

  case 251:
#line 729 "grammar.y"
    { (yyval.node) = 0; ;}
    break;

  case 253:
#line 734 "grammar.y"
    { (yyval.node) = 0; ;}
    break;

  case 255:
#line 739 "grammar.y"
    { (yyval.stat) = new ContinueNode(); DBG((yyval.stat), (yylsp[-1]), (yylsp[0])); ;}
    break;

  case 256:
#line 740 "grammar.y"
    { (yyval.stat) = new ContinueNode(); DBG((yyval.stat), (yylsp[-1]), (yylsp[-1])); AUTO_SEMICOLON; ;}
    break;

  case 257:
#line 741 "grammar.y"
    { (yyval.stat) = new ContinueNode(*(yyvsp[-1].ident)); DBG((yyval.stat), (yylsp[-2]), (yylsp[0])); ;}
    break;

  case 258:
#line 742 "grammar.y"
    { (yyval.stat) = new ContinueNode(*(yyvsp[-1].ident)); DBG((yyval.stat), (yylsp[-2]), (yylsp[-1])); AUTO_SEMICOLON; ;}
    break;

  case 259:
#line 746 "grammar.y"
    { (yyval.stat) = new BreakNode(); DBG((yyval.stat), (yylsp[-1]), (yylsp[0])); ;}
    break;

  case 260:
#line 747 "grammar.y"
    { (yyval.stat) = new BreakNode(); DBG((yyval.stat), (yylsp[-1]), (yylsp[-1])); AUTO_SEMICOLON; ;}
    break;

  case 261:
#line 748 "grammar.y"
    { (yyval.stat) = new BreakNode(*(yyvsp[-1].ident)); DBG((yyval.stat), (yylsp[-2]), (yylsp[0])); ;}
    break;

  case 262:
#line 749 "grammar.y"
    { (yyval.stat) = new BreakNode(*(yyvsp[-1].ident)); DBG((yyval.stat), (yylsp[-2]), (yylsp[-1])); AUTO_SEMICOLON; ;}
    break;

  case 263:
#line 753 "grammar.y"
    { (yyval.stat) = new ReturnNode(0); DBG((yyval.stat), (yylsp[-1]), (yylsp[0])); ;}
    break;

  case 264:
#line 754 "grammar.y"
    { (yyval.stat) = new ReturnNode(0); DBG((yyval.stat), (yylsp[-1]), (yylsp[-1])); AUTO_SEMICOLON; ;}
    break;

  case 265:
#line 755 "grammar.y"
    { (yyval.stat) = new ReturnNode((yyvsp[-1].node)); DBG((yyval.stat), (yylsp[-2]), (yylsp[0])); ;}
    break;

  case 266:
#line 756 "grammar.y"
    { (yyval.stat) = new ReturnNode((yyvsp[-1].node)); DBG((yyval.stat), (yylsp[-2]), (yylsp[-1])); AUTO_SEMICOLON; ;}
    break;

  case 267:
#line 760 "grammar.y"
    { (yyval.stat) = new WithNode((yyvsp[-2].node), (yyvsp[0].stat)); DBG((yyval.stat), (yylsp[-4]), (yylsp[-1])); ;}
    break;

  case 268:
#line 764 "grammar.y"
    { (yyval.stat) = new SwitchNode((yyvsp[-2].node), (yyvsp[0].cblk)); DBG((yyval.stat), (yylsp[-4]), (yylsp[-1])); ;}
    break;

  case 269:
#line 768 "grammar.y"
    { (yyval.cblk) = new CaseBlockNode((yyvsp[-1].clist), 0, 0); ;}
    break;

  case 270:
#line 770 "grammar.y"
    { (yyval.cblk) = new CaseBlockNode((yyvsp[-3].clist), (yyvsp[-2].ccl), (yyvsp[-1].clist)); ;}
    break;

  case 271:
#line 774 "grammar.y"
    { (yyval.clist) = 0; ;}
    break;

  case 273:
#line 779 "grammar.y"
    { (yyval.clist) = new ClauseListNode((yyvsp[0].ccl)); ;}
    break;

  case 274:
#line 780 "grammar.y"
    { (yyval.clist) = new ClauseListNode((yyvsp[-1].clist), (yyvsp[0].ccl)); ;}
    break;

  case 275:
#line 784 "grammar.y"
    { (yyval.ccl) = new CaseClauseNode((yyvsp[-1].node)); ;}
    break;

  case 276:
#line 785 "grammar.y"
    { (yyval.ccl) = new CaseClauseNode((yyvsp[-2].node), (yyvsp[0].slist)); ;}
    break;

  case 277:
#line 789 "grammar.y"
    { (yyval.ccl) = new CaseClauseNode(0); ;}
    break;

  case 278:
#line 790 "grammar.y"
    { (yyval.ccl) = new CaseClauseNode(0, (yyvsp[0].slist)); ;}
    break;

  case 279:
#line 794 "grammar.y"
    { (yyvsp[0].stat)->pushLabel(*(yyvsp[-2].ident)); (yyval.stat) = new LabelNode(*(yyvsp[-2].ident), (yyvsp[0].stat)); ;}
    break;

  case 280:
#line 798 "grammar.y"
    { (yyval.stat) = new ThrowNode((yyvsp[-1].node)); DBG((yyval.stat), (yylsp[-2]), (yylsp[0])); ;}
    break;

  case 281:
#line 799 "grammar.y"
    { (yyval.stat) = new ThrowNode((yyvsp[-1].node)); DBG((yyval.stat), (yylsp[-2]), (yylsp[-1])); AUTO_SEMICOLON; ;}
    break;

  case 282:
#line 803 "grammar.y"
    { (yyval.stat) = new TryNode((yyvsp[-2].stat), Identifier::null(), 0, (yyvsp[0].stat)); DBG((yyval.stat), (yylsp[-3]), (yylsp[-2])); ;}
    break;

  case 283:
#line 804 "grammar.y"
    { (yyval.stat) = new TryNode((yyvsp[-5].stat), *(yyvsp[-2].ident), (yyvsp[0].stat), 0); DBG((yyval.stat), (yylsp[-6]), (yylsp[-5])); ;}
    break;

  case 284:
#line 806 "grammar.y"
    { (yyval.stat) = new TryNode((yyvsp[-7].stat), *(yyvsp[-4].ident), (yyvsp[-2].stat), (yyvsp[0].stat)); DBG((yyval.stat), (yylsp[-8]), (yylsp[-7])); ;}
    break;

  case 285:
#line 810 "grammar.y"
    { (yyval.func) = new FuncDeclNode(*(yyvsp[-3].ident), (yyvsp[0].body)); ;}
    break;

  case 286:
#line 812 "grammar.y"
    { (yyval.func) = new FuncDeclNode(*(yyvsp[-4].ident), (yyvsp[-2].param), (yyvsp[0].body)); ;}
    break;

  case 287:
#line 816 "grammar.y"
    { (yyval.funcExpr) = new FuncExprNode(Identifier::null(), (yyvsp[0].body)); ;}
    break;

  case 288:
#line 818 "grammar.y"
    { (yyval.funcExpr) = new FuncExprNode(Identifier::null(), (yyvsp[0].body), (yyvsp[-2].param)); ;}
    break;

  case 289:
#line 819 "grammar.y"
    { (yyval.funcExpr) = new FuncExprNode(*(yyvsp[-3].ident), (yyvsp[0].body)); ;}
    break;

  case 290:
#line 821 "grammar.y"
    { (yyval.funcExpr) = new FuncExprNode(*(yyvsp[-4].ident), (yyvsp[0].body), (yyvsp[-2].param)); ;}
    break;

  case 291:
#line 825 "grammar.y"
    { (yyval.param) = new ParameterNode(*(yyvsp[0].ident)); ;}
    break;

  case 292:
#line 826 "grammar.y"
    { (yyval.param) = new ParameterNode((yyvsp[-2].param), *(yyvsp[0].ident)); ;}
    break;

  case 293:
#line 830 "grammar.y"
    { (yyval.body) = new FunctionBodyNode(0); DBG((yyval.body), (yylsp[-1]), (yylsp[0])); ;}
    break;

  case 294:
#line 831 "grammar.y"
    { (yyval.body) = new FunctionBodyNode((yyvsp[-1].srcs)); DBG((yyval.body), (yylsp[-2]), (yylsp[0])); ;}
    break;

  case 295:
#line 835 "grammar.y"
    { Parser::accept(new ProgramNode(0)); ;}
    break;

  case 296:
#line 836 "grammar.y"
    { Parser::accept(new ProgramNode((yyvsp[0].srcs))); ;}
    break;

  case 297:
#line 840 "grammar.y"
    { (yyval.srcs) = new SourceElementsNode((yyvsp[0].stat)); ;}
    break;

  case 298:
#line 841 "grammar.y"
    { (yyval.srcs) = new SourceElementsNode((yyvsp[-1].srcs), (yyvsp[0].stat)); ;}
    break;

  case 299:
#line 845 "grammar.y"
    { (yyval.stat) = (yyvsp[0].func); ;}
    break;

  case 300:
#line 846 "grammar.y"
    { (yyval.stat) = (yyvsp[0].stat); ;}
    break;


      default: break;
    }

/* Line 1126 of yacc.c.  */
#line 3213 "grammar.tab.c"

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
	  int yytype = YYTRANSLATE (yychar);
	  YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
	  YYSIZE_T yysize = yysize0;
	  YYSIZE_T yysize1;
	  int yysize_overflow = 0;
	  char *yymsg = 0;
#	  define YYERROR_VERBOSE_ARGS_MAXIMUM 5
	  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
	  int yyx;

#if 0
	  /* This is so xgettext sees the translatable formats that are
	     constructed on the fly.  */
	  YY_("syntax error, unexpected %s");
	  YY_("syntax error, unexpected %s, expecting %s");
	  YY_("syntax error, unexpected %s, expecting %s or %s");
	  YY_("syntax error, unexpected %s, expecting %s or %s or %s");
	  YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
#endif
	  char *yyfmt;
	  char const *yyf;
	  static char const yyunexpected[] = "syntax error, unexpected %s";
	  static char const yyexpecting[] = ", expecting %s";
	  static char const yyor[] = " or %s";
	  char yyformat[sizeof yyunexpected
			+ sizeof yyexpecting - 1
			+ ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
			   * (sizeof yyor - 1))];
	  char const *yyprefix = yyexpecting;

	  /* Start YYX at -YYN if negative to avoid negative indexes in
	     YYCHECK.  */
	  int yyxbegin = yyn < 0 ? -yyn : 0;

	  /* Stay within bounds of both yycheck and yytname.  */
	  int yychecklim = YYLAST - yyn;
	  int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
	  int yycount = 1;

	  yyarg[0] = yytname[yytype];
	  yyfmt = yystpcpy (yyformat, yyunexpected);

	  for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	      {
		if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
		  {
		    yycount = 1;
		    yysize = yysize0;
		    yyformat[sizeof yyunexpected - 1] = '\0';
		    break;
		  }
		yyarg[yycount++] = yytname[yyx];
		yysize1 = yysize + yytnamerr (0, yytname[yyx]);
		yysize_overflow |= yysize1 < yysize;
		yysize = yysize1;
		yyfmt = yystpcpy (yyfmt, yyprefix);
		yyprefix = yyor;
	      }

	  yyf = YY_(yyformat);
	  yysize1 = yysize + yystrlen (yyf);
	  yysize_overflow |= yysize1 < yysize;
	  yysize = yysize1;

	  if (!yysize_overflow && yysize <= YYSTACK_ALLOC_MAXIMUM)
	    yymsg = (char *) YYSTACK_ALLOC (yysize);
	  if (yymsg)
	    {
	      /* Avoid sprintf, as that infringes on the user's name space.
		 Don't have undefined behavior even if the translation
		 produced a string with the wrong number of "%s"s.  */
	      char *yyp = yymsg;
	      int yyi = 0;
	      while ((*yyp = *yyf))
		{
		  if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		    {
		      yyp += yytnamerr (yyp, yyarg[yyi++]);
		      yyf += 2;
		    }
		  else
		    {
		      yyp++;
		      yyf++;
		    }
		}
	      yyerror (yymsg);
	      YYSTACK_FREE (yymsg);
	    }
	  else
	    {
	      yyerror (YY_("syntax error"));
	      goto yyexhaustedlab;
	    }
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror (YY_("syntax error"));
    }

  yyerror_range[0] = yylloc;

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse look-ahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
        {
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
        }
      else
	{
	  yydestruct ("Error: discarding", yytoken, &yylval, &yylloc);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse look-ahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (0)
     goto yyerrorlab;

  yyerror_range[0] = yylsp[1-yylen];
  yylsp -= yylen;
  yyvsp -= yylen;
  yyssp -= yylen;
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
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

      yyerror_range[0] = *yylsp;
      yydestruct ("Error: popping", yystos[yystate], yyvsp, yylsp);
      YYPOPSTACK;
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  *++yyvsp = yylval;

  yyerror_range[1] = yylloc;
  /* Using YYLLOC is tempting, but would change the location of
     the look-ahead.  YYLOC is available though. */
  YYLLOC_DEFAULT (yyloc, yyerror_range - 1, 2);
  *++yylsp = yyloc;

  /* Shift the error token. */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

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
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEOF && yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval, &yylloc);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp, yylsp);
      YYPOPSTACK;
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  return yyresult;
}


#line 849 "grammar.y"


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

static bool makeGetterOrSetterPropertyNode(PropertyNode*& result, Identifier& getOrSet, Identifier& name, ParameterNode *params, FunctionBodyNode *body)
{
    PropertyNode::Type type;
    
    if (getOrSet == "get")
        type = PropertyNode::Getter;
    else if (getOrSet == "set")
        type = PropertyNode::Setter;
    else
        return false;
    
    result = new PropertyNode(new PropertyNameNode(name), 
                              new FuncExprNode(Identifier::null(), body, params), type);

    return true;
}

/* called by yyparse on error */
int yyerror(const char *)
{
// fprintf(stderr, "ERROR: %s at line %d\n", s, KJS::Lexer::curr()->lineNo());
    return 1;
}

/* may we automatically insert a semicolon ? */
static bool allowAutomaticSemicolon()
{
    return yychar == '}' || yychar == 0 || Lexer::curr()->prevTerminator();
}

