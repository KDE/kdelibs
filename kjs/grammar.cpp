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
     BREAK = 261,
     CASE = 262,
     DEFAULT = 263,
     FOR = 264,
     NEW = 265,
     VAR = 266,
     CONST = 267,
     CONTINUE = 268,
     FUNCTION = 269,
     RETURN = 270,
     VOID = 271,
     DELETE = 272,
     IF = 273,
     THIS = 274,
     DO = 275,
     WHILE = 276,
     IN = 277,
     INSTANCEOF = 278,
     TYPEOF = 279,
     SWITCH = 280,
     WITH = 281,
     RESERVED = 282,
     THROW = 283,
     TRY = 284,
     CATCH = 285,
     FINALLY = 286,
     DEBUGGER = 287,
     IMPORT = 288,
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
     NUMBER = 315,
     STRING = 316,
     IDENT = 317,
     AUTOPLUSPLUS = 318,
     AUTOMINUSMINUS = 319
   };
#endif
/* Tokens.  */
#define NULLTOKEN 258
#define TRUETOKEN 259
#define FALSETOKEN 260
#define BREAK 261
#define CASE 262
#define DEFAULT 263
#define FOR 264
#define NEW 265
#define VAR 266
#define CONST 267
#define CONTINUE 268
#define FUNCTION 269
#define RETURN 270
#define VOID 271
#define DELETE 272
#define IF 273
#define THIS 274
#define DO 275
#define WHILE 276
#define IN 277
#define INSTANCEOF 278
#define TYPEOF 279
#define SWITCH 280
#define WITH 281
#define RESERVED 282
#define THROW 283
#define TRY 284
#define CATCH 285
#define FINALLY 286
#define DEBUGGER 287
#define IMPORT 288
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
#define NUMBER 315
#define STRING 316
#define IDENT 317
#define AUTOPLUSPLUS 318
#define AUTOMINUSMINUS 319




/* Copy the first part of user declarations.  */
#line 1 "grammar.y"


/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999-2000 Harri Porten (porten@kde.org)
 *  Copyright (C) 2006 Apple Computer, Inc.
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
#if !PLATFORM(DARWIN)
    // avoid triggering warnings in older bison
#define YYERROR_VERBOSE
#endif

extern int kjsyylex();
int kjsyyerror(const char *);
static bool allowAutomaticSemicolon();

#define AUTO_SEMICOLON do { if (!allowAutomaticSemicolon()) YYABORT; } while (0)
#define DBG(l, s, e) (l)->setLoc((s).first_line, (e).last_line)

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
#line 65 "grammar.y"
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
  PackageNameNode     *pkgn;
  PackageIdentNode     *pkgi;
} YYSTYPE;
/* Line 196 of yacc.c.  */
#line 315 "grammar.tab.c"
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
#line 339 "grammar.tab.c"

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
#define YYFINAL  214
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1487

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  89
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  105
/* YYNRULES -- Number of rules. */
#define YYNRULES  308
/* YYNRULES -- Number of states. */
#define YYNSTATES  538

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   319

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    78,     2,     2,     2,    80,    83,     2,
      67,    68,    79,    75,    69,    76,    74,    65,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    66,    88,
      81,    87,    82,    86,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    72,     2,    73,    84,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    70,    85,    71,    77,     2,     2,     2,
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
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64
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
     646,   648,   650,   652,   654,   656,   658,   660,   662,   664,
     667,   671,   675,   679,   681,   685,   687,   691,   693,   696,
     698,   701,   705,   709,   711,   715,   717,   720,   723,   726,
     728,   731,   734,   740,   748,   755,   761,   771,   782,   790,
     799,   809,   810,   812,   813,   815,   818,   821,   825,   829,
     832,   835,   839,   843,   846,   849,   853,   857,   863,   869,
     873,   879,   880,   882,   884,   887,   891,   896,   899,   903,
     907,   911,   915,   920,   928,   938,   941,   944,   946,   950,
     952,   954,   958,   962,   968,   975,   980,   986,   992,   999,
    1001,  1005,  1008,  1012,  1013,  1015,  1017,  1020,  1022
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short int yyrhs[] =
{
     191,     0,    -1,     3,    -1,     4,    -1,     5,    -1,    60,
      -1,    61,    -1,    65,    -1,    52,    -1,    62,    -1,    61,
      -1,    60,    -1,    91,    66,   145,    -1,    62,    62,    67,
      68,   190,    -1,    62,    62,    67,   189,    68,   190,    -1,
      92,    -1,    93,    69,    92,    -1,    95,    -1,    70,    71,
      -1,    70,    93,    71,    -1,    70,    93,    69,    71,    -1,
      19,    -1,    90,    -1,    96,    -1,    62,    -1,    67,   149,
      68,    -1,    72,    98,    73,    -1,    72,    97,    73,    -1,
      72,    97,    69,    98,    73,    -1,    98,   145,    -1,    97,
      69,    98,   145,    -1,    -1,    99,    -1,    69,    -1,    99,
      69,    -1,    94,    -1,   188,    -1,   100,    72,   149,    73,
      -1,   100,    74,    62,    -1,    10,   100,   106,    -1,    95,
      -1,   101,    72,   149,    73,    -1,   101,    74,    62,    -1,
      10,   100,   106,    -1,   100,    -1,    10,   102,    -1,   101,
      -1,    10,   102,    -1,   100,   106,    -1,   104,   106,    -1,
     104,    72,   149,    73,    -1,   104,    74,    62,    -1,   101,
     106,    -1,   105,   106,    -1,   105,    72,   149,    73,    -1,
     105,    74,    62,    -1,    67,    68,    -1,    67,   107,    68,
      -1,   145,    -1,   107,    69,   145,    -1,   102,    -1,   104,
      -1,   103,    -1,   105,    -1,   108,    -1,   108,    44,    -1,
     108,    45,    -1,   109,    -1,   109,    44,    -1,   109,    45,
      -1,    17,   113,    -1,    16,   113,    -1,    24,   113,    -1,
      44,   113,    -1,    63,   113,    -1,    45,   113,    -1,    64,
     113,    -1,    75,   113,    -1,    76,   113,    -1,    77,   113,
      -1,    78,   113,    -1,   110,    -1,   112,    -1,   111,    -1,
     112,    -1,   113,    -1,   115,    79,   113,    -1,   115,    65,
     113,    -1,   115,    80,   113,    -1,   114,    -1,   116,    79,
     113,    -1,   116,    65,   113,    -1,   116,    80,   113,    -1,
     115,    -1,   117,    75,   115,    -1,   117,    76,   115,    -1,
     116,    -1,   118,    75,   115,    -1,   118,    76,   115,    -1,
     117,    -1,   119,    46,   117,    -1,   119,    47,   117,    -1,
     119,    48,   117,    -1,   118,    -1,   120,    46,   117,    -1,
     120,    47,   117,    -1,   120,    48,   117,    -1,   119,    -1,
     121,    81,   119,    -1,   121,    82,   119,    -1,   121,    40,
     119,    -1,   121,    41,   119,    -1,   121,    23,   119,    -1,
     121,    22,   119,    -1,   119,    -1,   122,    81,   119,    -1,
     122,    82,   119,    -1,   122,    40,   119,    -1,   122,    41,
     119,    -1,   122,    23,   119,    -1,   120,    -1,   123,    81,
     119,    -1,   123,    82,   119,    -1,   123,    40,   119,    -1,
     123,    41,   119,    -1,   123,    23,   119,    -1,   123,    22,
     119,    -1,   121,    -1,   124,    36,   121,    -1,   124,    37,
     121,    -1,   124,    38,   121,    -1,   124,    39,   121,    -1,
     122,    -1,   125,    36,   122,    -1,   125,    37,   122,    -1,
     125,    38,   122,    -1,   125,    39,   122,    -1,   123,    -1,
     126,    36,   121,    -1,   126,    37,   121,    -1,   126,    38,
     121,    -1,   126,    39,   121,    -1,   124,    -1,   127,    83,
     124,    -1,   125,    -1,   128,    83,   125,    -1,   126,    -1,
     129,    83,   124,    -1,   127,    -1,   130,    84,   127,    -1,
     128,    -1,   131,    84,   128,    -1,   129,    -1,   132,    84,
     127,    -1,   130,    -1,   133,    85,   130,    -1,   131,    -1,
     134,    85,   131,    -1,   132,    -1,   135,    85,   130,    -1,
     133,    -1,   136,    43,   133,    -1,   134,    -1,   137,    43,
     134,    -1,   135,    -1,   138,    43,   133,    -1,   136,    -1,
     139,    42,   136,    -1,   137,    -1,   140,    42,   137,    -1,
     138,    -1,   141,    42,   136,    -1,   139,    -1,   139,    86,
     145,    66,   145,    -1,   140,    -1,   140,    86,   146,    66,
     146,    -1,   141,    -1,   141,    86,   145,    66,   145,    -1,
     142,    -1,   108,   148,   145,    -1,   143,    -1,   108,   148,
     146,    -1,   144,    -1,   109,   148,   145,    -1,    87,    -1,
      49,    -1,    50,    -1,    51,    -1,    52,    -1,    53,    -1,
      54,    -1,    55,    -1,    56,    -1,    58,    -1,    59,    -1,
      57,    -1,   145,    -1,   149,    69,   145,    -1,   146,    -1,
     150,    69,   146,    -1,   147,    -1,   151,    69,   145,    -1,
     153,    -1,   154,    -1,   159,    -1,   164,    -1,   165,    -1,
     166,    -1,   167,    -1,   170,    -1,   171,    -1,   172,    -1,
     173,    -1,   174,    -1,   180,    -1,   181,    -1,   182,    -1,
     183,    -1,   186,    -1,    70,    71,    -1,    70,   192,    71,
      -1,    11,   155,    88,    -1,    11,   155,     1,    -1,   157,
      -1,   155,    69,   157,    -1,   158,    -1,   156,    69,   158,
      -1,    62,    -1,    62,   162,    -1,    62,    -1,    62,   163,
      -1,    12,   160,    88,    -1,    12,   160,     1,    -1,   161,
      -1,   160,    69,   161,    -1,    62,    -1,    62,   162,    -1,
      87,   145,    -1,    87,   146,    -1,    88,    -1,   151,    88,
      -1,   151,     1,    -1,    18,    67,   149,    68,   152,    -1,
      18,    67,   149,    68,   152,    35,   152,    -1,    20,   152,
      21,    67,   149,    68,    -1,    21,    67,   149,    68,   152,
      -1,     9,    67,   169,    88,   168,    88,   168,    68,   152,
      -1,     9,    67,    11,   156,    88,   168,    88,   168,    68,
     152,    -1,     9,    67,   108,    22,   149,    68,   152,    -1,
       9,    67,    11,    62,    22,   149,    68,   152,    -1,     9,
      67,    11,    62,   163,    22,   149,    68,   152,    -1,    -1,
     149,    -1,    -1,   150,    -1,    13,    88,    -1,    13,     1,
      -1,    13,    62,    88,    -1,    13,    62,     1,    -1,     6,
      88,    -1,     6,     1,    -1,     6,    62,    88,    -1,     6,
      62,     1,    -1,    15,    88,    -1,    15,     1,    -1,    15,
     149,    88,    -1,    15,   149,     1,    -1,    26,    67,   149,
      68,   152,    -1,    25,    67,   149,    68,   175,    -1,    70,
     176,    71,    -1,    70,   176,   179,   176,    71,    -1,    -1,
     177,    -1,   178,    -1,   177,   178,    -1,     7,   149,    66,
      -1,     7,   149,    66,   192,    -1,     8,    66,    -1,     8,
      66,   192,    -1,    62,    66,   152,    -1,    28,   149,    88,
      -1,    28,   149,     1,    -1,    29,   153,    31,   153,    -1,
      29,   153,    30,    67,    62,    68,   153,    -1,    29,   153,
      30,    67,    62,    68,   153,    31,   153,    -1,    32,    88,
      -1,    32,     1,    -1,    62,    -1,   184,    74,    62,    -1,
      61,    -1,   184,    -1,    33,   185,    88,    -1,    33,   185,
       1,    -1,    14,    62,    67,    68,   190,    -1,    14,    62,
      67,   189,    68,   190,    -1,    14,    67,    68,   190,    -1,
      14,    67,   189,    68,   190,    -1,    14,    62,    67,    68,
     190,    -1,    14,    62,    67,   189,    68,   190,    -1,    62,
      -1,   189,    69,    62,    -1,    70,    71,    -1,    70,   192,
      71,    -1,    -1,   192,    -1,   193,    -1,   192,   193,    -1,
     187,    -1,   152,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short int yyrline[] =
{
       0,   193,   193,   194,   195,   196,   197,   198,   203,   211,
     212,   213,   217,   218,   219,   224,   225,   229,   230,   231,
     233,   237,   238,   239,   240,   241,   246,   247,   248,   252,
     253,   258,   259,   263,   264,   268,   269,   270,   271,   272,
     276,   277,   278,   279,   283,   284,   288,   289,   293,   294,
     295,   296,   300,   301,   302,   303,   307,   308,   312,   313,
     317,   318,   322,   323,   327,   328,   329,   333,   334,   335,
     339,   340,   341,   342,   343,   344,   345,   346,   347,   348,
     349,   352,   353,   357,   358,   362,   363,   364,   365,   369,
     370,   372,   374,   379,   380,   381,   385,   386,   388,   393,
     394,   395,   396,   400,   401,   402,   403,   407,   408,   409,
     410,   411,   412,   413,   417,   418,   419,   420,   421,   422,
     427,   428,   429,   430,   431,   432,   434,   438,   439,   440,
     441,   442,   446,   447,   449,   451,   453,   458,   459,   461,
     462,   464,   469,   470,   474,   475,   480,   481,   485,   486,
     490,   491,   496,   497,   502,   503,   507,   508,   513,   514,
     519,   520,   524,   525,   530,   531,   536,   537,   541,   542,
     547,   548,   552,   553,   558,   559,   564,   565,   570,   571,
     576,   577,   582,   583,   588,   589,   590,   591,   592,   593,
     594,   595,   596,   597,   598,   599,   603,   604,   608,   609,
     613,   614,   618,   619,   620,   621,   622,   623,   624,   625,
     626,   627,   628,   629,   630,   631,   632,   633,   634,   638,
     639,   643,   644,   648,   649,   654,   655,   660,   661,   665,
     666,   670,   671,   675,   676,   681,   682,   686,   690,   694,
     698,   699,   703,   705,   710,   711,   712,   714,   716,   724,
     726,   731,   732,   736,   737,   741,   742,   743,   744,   748,
     749,   750,   751,   755,   756,   757,   758,   762,   766,   770,
     771,   776,   777,   781,   782,   786,   787,   791,   792,   796,
     800,   801,   805,   806,   807,   812,   813,   817,   818,   822,
     823,   827,   828,   832,   833,   838,   839,   841,   842,   847,
     848,   852,   853,   857,   858,   862,   863,   867,   868
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "NULLTOKEN", "TRUETOKEN", "FALSETOKEN",
  "BREAK", "CASE", "DEFAULT", "FOR", "NEW", "VAR", "CONST", "CONTINUE",
  "FUNCTION", "RETURN", "VOID", "DELETE", "IF", "THIS", "DO", "WHILE",
  "IN", "INSTANCEOF", "TYPEOF", "SWITCH", "WITH", "RESERVED", "THROW",
  "TRY", "CATCH", "FINALLY", "DEBUGGER", "IMPORT", "IF_WITHOUT_ELSE",
  "ELSE", "EQEQ", "NE", "STREQ", "STRNEQ", "LE", "GE", "OR", "AND",
  "PLUSPLUS", "MINUSMINUS", "LSHIFT", "RSHIFT", "URSHIFT", "PLUSEQUAL",
  "MINUSEQUAL", "MULTEQUAL", "DIVEQUAL", "LSHIFTEQUAL", "RSHIFTEQUAL",
  "URSHIFTEQUAL", "ANDEQUAL", "MODEQUAL", "XOREQUAL", "OREQUAL", "NUMBER",
  "STRING", "IDENT", "AUTOPLUSPLUS", "AUTOMINUSMINUS", "'/'", "':'", "'('",
  "')'", "','", "'{'", "'}'", "'['", "']'", "'.'", "'+'", "'-'", "'~'",
  "'!'", "'*'", "'%'", "'<'", "'>'", "'&'", "'^'", "'|'", "'?'", "'='",
  "';'", "$accept", "Literal", "PropertyName", "Property", "PropertyList",
  "PrimaryExpr", "PrimaryExprNoBrace", "ArrayLiteral", "ElementList",
  "ElisionOpt", "Elision", "MemberExpr", "MemberExprNoBF", "NewExpr",
  "NewExprNoBF", "CallExpr", "CallExprNoBF", "Arguments", "ArgumentList",
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
  "ExprNoIn", "ExprNoBF", "Statement", "Block", "VariableStatement",
  "VariableDeclarationList", "VariableDeclarationListNoIn",
  "VariableDeclaration", "VariableDeclarationNoIn", "ConstStatement",
  "ConstDeclarationList", "ConstDeclaration", "Initializer",
  "InitializerNoIn", "EmptyStatement", "ExprStatement", "IfStatement",
  "IterationStatement", "ExprOpt", "ExprNoInOpt", "ContinueStatement",
  "BreakStatement", "ReturnStatement", "WithStatement", "SwitchStatement",
  "CaseBlock", "CaseClausesOpt", "CaseClauses", "CaseClause",
  "DefaultClause", "LabelledStatement", "ThrowStatement", "TryStatement",
  "DebuggerStatement", "PackageIdentifiers", "PackageName",
  "ImportStatement", "FunctionDeclaration", "FunctionExpr",
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
     315,   316,   317,   318,   319,    47,    58,    40,    41,    44,
     123,   125,    91,    93,    46,    43,    45,   126,    33,    42,
      37,    60,    62,    38,    94,   124,    63,    61,    59
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    89,    90,    90,    90,    90,    90,    90,    90,    91,
      91,    91,    92,    92,    92,    93,    93,    94,    94,    94,
      94,    95,    95,    95,    95,    95,    96,    96,    96,    97,
      97,    98,    98,    99,    99,   100,   100,   100,   100,   100,
     101,   101,   101,   101,   102,   102,   103,   103,   104,   104,
     104,   104,   105,   105,   105,   105,   106,   106,   107,   107,
     108,   108,   109,   109,   110,   110,   110,   111,   111,   111,
     112,   112,   112,   112,   112,   112,   112,   112,   112,   112,
     112,   113,   113,   114,   114,   115,   115,   115,   115,   116,
     116,   116,   116,   117,   117,   117,   118,   118,   118,   119,
     119,   119,   119,   120,   120,   120,   120,   121,   121,   121,
     121,   121,   121,   121,   122,   122,   122,   122,   122,   122,
     123,   123,   123,   123,   123,   123,   123,   124,   124,   124,
     124,   124,   125,   125,   125,   125,   125,   126,   126,   126,
     126,   126,   127,   127,   128,   128,   129,   129,   130,   130,
     131,   131,   132,   132,   133,   133,   134,   134,   135,   135,
     136,   136,   137,   137,   138,   138,   139,   139,   140,   140,
     141,   141,   142,   142,   143,   143,   144,   144,   145,   145,
     146,   146,   147,   147,   148,   148,   148,   148,   148,   148,
     148,   148,   148,   148,   148,   148,   149,   149,   150,   150,
     151,   151,   152,   152,   152,   152,   152,   152,   152,   152,
     152,   152,   152,   152,   152,   152,   152,   152,   152,   153,
     153,   154,   154,   155,   155,   156,   156,   157,   157,   158,
     158,   159,   159,   160,   160,   161,   161,   162,   163,   164,
     165,   165,   166,   166,   167,   167,   167,   167,   167,   167,
     167,   168,   168,   169,   169,   170,   170,   170,   170,   171,
     171,   171,   171,   172,   172,   172,   172,   173,   174,   175,
     175,   176,   176,   177,   177,   178,   178,   179,   179,   180,
     181,   181,   182,   182,   182,   183,   183,   184,   184,   185,
     185,   186,   186,   187,   187,   188,   188,   188,   188,   189,
     189,   190,   190,   191,   191,   192,   192,   193,   193
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
       1,     1,     1,     1,     1,     1,     1,     1,     1,     2,
       3,     3,     3,     1,     3,     1,     3,     1,     2,     1,
       2,     3,     3,     1,     3,     1,     2,     2,     2,     1,
       2,     2,     5,     7,     6,     5,     9,    10,     7,     8,
       9,     0,     1,     0,     1,     2,     2,     3,     3,     2,
       2,     3,     3,     2,     2,     3,     3,     5,     5,     3,
       5,     0,     1,     1,     2,     3,     4,     2,     3,     3,
       3,     3,     4,     7,     9,     2,     2,     1,     3,     1,
       1,     3,     3,     5,     6,     4,     5,     5,     6,     1,
       3,     2,     3,     0,     1,     1,     2,     1,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned short int yydefact[] =
{
     303,     2,     3,     4,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    21,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     8,     5,     6,
      24,     0,     0,     7,     0,     0,    31,     0,     0,     0,
       0,   239,    22,    40,    23,    46,    62,    63,    67,    83,
      84,    89,    96,   103,   120,   137,   146,   152,   158,   164,
     170,   176,   182,   200,     0,   308,   202,   203,   204,   205,
     206,   207,   208,   209,   210,   211,   212,   213,   214,   215,
     216,   217,   218,   307,     0,   304,   305,   260,     0,   259,
     253,     0,     0,    24,     0,    35,    17,    44,    47,    36,
     227,     0,   223,   235,     0,   233,   256,     0,   255,     0,
     264,   263,    44,    60,    61,    64,    81,    82,    85,    93,
      99,   107,   127,   142,   148,   154,   160,   166,   172,   178,
     196,     0,    64,    71,    70,     0,     0,     0,    72,     0,
       0,     0,     0,   286,   285,   289,   287,   290,     0,    73,
      75,     0,    74,    76,     0,   219,     0,    33,     0,     0,
      32,    77,    78,    79,    80,     0,     0,     0,    52,     0,
       0,    53,    68,    69,   185,   186,   187,   188,   189,   190,
     191,   192,   195,   193,   194,   184,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   241,     0,   240,     1,   306,   262,   261,     0,    64,
     114,   132,   144,   150,   156,   162,   168,   174,   180,   198,
     254,     0,    44,    45,     0,     0,    11,    10,     9,    18,
       0,    15,     0,     0,     0,    43,     0,   228,   222,     0,
     221,   236,   232,     0,   231,   258,   257,     0,    48,     0,
       0,    49,    65,    66,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   266,
       0,   265,     0,     0,     0,     0,     0,   281,   280,     0,
       0,     0,   292,   291,   279,    25,   220,    31,    27,    26,
      29,    34,    56,     0,    58,     0,    42,     0,    55,   183,
      91,    90,    92,    97,    98,   104,   105,   106,   126,   125,
     123,   124,   121,   122,   138,   139,   140,   141,   147,   153,
     159,   165,   171,     0,   201,   229,     0,   225,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   251,    39,     0,   299,
       0,     0,     0,     0,     0,    19,     0,    38,   237,   224,
     234,     0,     0,     0,    51,   179,    87,    86,    88,    94,
      95,   100,   101,   102,   113,   112,   110,   111,   108,   109,
     128,   129,   130,   131,   143,   149,   155,   161,   167,     0,
     197,     0,     0,     0,     0,     0,     0,   282,   288,     0,
      57,     0,    41,    54,     0,     0,     0,   230,     0,   251,
       0,    64,   181,   119,   117,   118,   115,   116,   133,   134,
     135,   136,   145,   151,   157,   163,   169,     0,   199,   252,
       0,     0,     0,     0,   295,     0,     0,     0,    12,    20,
      16,    37,   293,     0,    50,     0,   242,     0,   245,   271,
     268,   267,     0,    28,    30,    59,   177,     0,   238,     0,
     229,   226,     0,     0,     0,   251,   297,     0,   301,     0,
     296,   300,     0,     0,   294,   173,     0,   244,     0,     0,
     272,   273,     0,     0,     0,   230,   251,   248,   175,     0,
     298,   302,    13,     0,   243,     0,     0,   269,   271,   274,
     283,   249,     0,     0,     0,    14,   275,   277,     0,     0,
     250,     0,   246,   276,   278,   270,   284,   247
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short int yydefgoto[] =
{
      -1,    42,   240,   241,   242,    95,    96,    44,   158,   159,
     160,   112,    45,   113,    46,   114,    47,   168,   313,   132,
      48,   116,    49,   117,   118,    51,   119,    52,   120,    53,
     121,    54,   122,   221,    55,   123,   222,    56,   124,   223,
      57,   125,   224,    58,   126,   225,    59,   127,   226,    60,
     128,   227,    61,   129,   228,    62,   130,   229,    63,   349,
     449,   230,    64,    65,    66,    67,   101,   346,   102,   347,
      68,   104,   105,   247,   427,    69,    70,    71,    72,   450,
     231,    73,    74,    75,    76,    77,   470,   499,   500,   501,
     518,    78,    79,    80,    81,   147,   148,    82,    83,    99,
     371,   454,    84,    85,    86
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -424
static const short int yypact[] =
{
     949,  -424,  -424,  -424,    17,   -13,   239,    41,    53,    19,
      65,   549,  1409,  1409,    70,  -424,  1029,    98,  1409,   105,
     113,  1409,    52,    29,   -15,  1409,  1409,  -424,  -424,  -424,
     151,  1409,  1409,  -424,  1409,   629,   176,  1409,  1409,  1409,
    1409,  -424,  -424,  -424,  -424,   -11,  -424,   126,   335,  -424,
    -424,  -424,   -10,    13,   155,   147,   276,   169,   171,   172,
     225,    55,  -424,  -424,    11,  -424,  -424,  -424,  -424,  -424,
    -424,  -424,  -424,  -424,  -424,  -424,  -424,  -424,  -424,  -424,
    -424,  -424,  -424,  -424,   287,   949,  -424,  -424,    36,  -424,
    1105,   239,   157,  -424,   161,  -424,  -424,   132,  -424,  -424,
     189,    24,  -424,   189,    25,  -424,  -424,    37,  -424,   211,
    -424,  -424,   132,  -424,   167,   530,  -424,  -424,  -424,    96,
     115,   278,   156,   280,   197,   206,   210,   254,    60,  -424,
    -424,    32,   240,  -424,  -424,  1409,   286,  1409,  -424,  1409,
    1409,    35,   272,  -424,  -424,  -424,  -424,   231,    40,  -424,
    -424,  1029,  -424,  -424,   260,  -424,   709,  -424,    95,  1181,
     241,  -424,  -424,  -424,  -424,  1257,  1409,   246,  -424,  1409,
     268,  -424,  -424,  -424,  -424,  -424,  -424,  -424,  -424,  -424,
    -424,  -424,  -424,  -424,  -424,  -424,  1409,  1409,  1409,  1409,
    1409,  1409,  1409,  1409,  1409,  1409,  1409,  1409,  1409,  1409,
    1409,  1409,  1409,  1409,  1409,  1409,  1409,  1409,  1409,  1409,
    1409,  -424,  1409,  -424,  -424,  -424,  -424,  -424,   290,   309,
     278,   207,   284,   251,   271,   310,   357,    69,  -424,  -424,
     333,   326,   132,  -424,   358,    76,  -424,  -424,   369,  -424,
     378,  -424,   162,  1409,   383,  -424,  1409,  -424,  -424,    41,
    -424,  -424,  -424,    53,  -424,  -424,  -424,    91,  -424,  1409,
     384,  -424,  -424,  -424,  1409,  1409,  1409,  1409,  1409,  1409,
    1409,  1409,  1409,  1409,  1409,  1409,  1409,  1409,  1409,  1409,
    1409,  1409,  1409,  1409,  1409,  1409,  1409,  1409,  1409,  -424,
    1409,  -424,   264,   380,   301,   314,   329,  -424,  -424,   382,
      52,   390,  -424,  -424,  -424,  -424,  -424,   176,  -424,  -424,
    -424,  -424,  -424,   336,  -424,   196,  -424,   201,  -424,  -424,
    -424,  -424,  -424,    96,    96,   115,   115,   115,   278,   278,
     278,   278,   278,   278,   156,   156,   156,   156,   280,   197,
     206,   210,   254,   388,  -424,    49,   -20,  -424,  1409,  1409,
    1409,  1409,  1409,  1409,  1409,  1409,  1409,  1409,  1409,  1409,
    1409,  1409,  1409,  1409,  1409,  1409,  1409,  -424,   178,  -424,
     366,   339,   389,  1409,   165,  -424,   202,  -424,  -424,  -424,
    -424,   366,   341,   204,  -424,  -424,  -424,  -424,  -424,    96,
      96,   115,   115,   115,   278,   278,   278,   278,   278,   278,
     156,   156,   156,   156,   280,   197,   206,   210,   254,   391,
    -424,  1029,  1409,  1029,   385,  1029,   396,  -424,  -424,  1333,
    -424,  1409,  -424,  -424,  1409,  1409,  1409,   437,   398,  1409,
     348,   530,  -424,   278,   278,   278,   278,   278,   207,   207,
     207,   207,   284,   251,   271,   310,   357,   395,  -424,   393,
     375,   366,   350,   789,  -424,   366,   402,   188,  -424,  -424,
    -424,  -424,  -424,   366,  -424,  1409,   430,   355,  -424,   459,
    -424,  -424,   399,  -424,  -424,  -424,  -424,   370,  -424,  1409,
     387,  -424,   381,  1029,  1409,  1409,  -424,   366,  -424,   869,
    -424,  -424,   366,   372,  -424,  -424,  1029,  -424,  1409,    78,
     459,  -424,    52,  1029,   374,  -424,  1409,  -424,  -424,   404,
    -424,  -424,  -424,   366,  -424,    88,   409,  -424,   459,  -424,
     445,  -424,  1029,   410,  1029,  -424,   949,   949,   406,    52,
    -424,  1029,  -424,   949,   949,  -424,  -424,  -424
};

/* YYPGOTO[NTERM-NUM].  */
static const short int yypgoto[] =
{
    -424,  -424,  -424,   106,  -424,  -424,     0,  -424,  -424,   175,
    -424,     5,  -424,    18,  -424,  -424,  -424,   -37,  -424,     8,
    -424,  -424,  -424,    15,    27,  -424,  -177,  -424,   -59,  -424,
     -14,  -424,   -72,    20,  -424,  -196,   120,  -424,  -189,   125,
    -424,  -179,   127,  -424,  -176,   124,  -424,  -166,   134,  -424,
    -424,  -424,  -424,  -424,  -424,  -424,  -138,  -342,  -424,     3,
      23,  -424,  -424,   -12,   -21,  -424,  -424,  -424,   250,    62,
    -424,  -424,   247,   412,    21,  -424,  -424,  -424,  -424,  -423,
    -424,  -424,  -424,  -424,  -424,  -424,  -424,    -9,  -424,     7,
    -424,  -424,  -424,  -424,  -424,  -424,  -424,  -424,  -424,  -424,
    -252,  -373,  -424,   -32,   -83
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const unsigned short int yytable[] =
{
      43,   142,   215,   156,   136,   382,   482,   432,   462,   338,
     171,    97,   211,   323,   324,    50,    43,   339,    87,   115,
     106,   310,   447,   448,    98,   248,   252,   314,   340,   115,
     143,    50,   341,   289,   131,    43,   297,   216,   255,   133,
     134,   302,   115,   342,   141,   138,   145,   146,   319,   428,
      50,   186,   149,   150,    90,   187,   165,   154,   152,   153,
     245,   166,   509,   167,   161,   162,   163,   164,   429,   188,
     189,   425,   343,   215,   344,   258,   220,   261,   486,    88,
     212,   107,   490,   523,   478,    43,   516,   404,   190,   191,
     494,   389,   390,   249,   253,   405,   232,   209,   219,   213,
      50,   290,   287,   100,   290,    89,   406,   108,   378,   233,
     407,   363,   250,   254,   510,   103,   452,   144,   264,   512,
     291,   408,    35,   298,   217,   256,   385,   109,   303,   334,
     335,   336,   337,   325,   326,   327,   426,   135,   369,   304,
     525,   210,   508,   115,   370,   115,   288,   115,   115,   517,
     409,    43,   410,   369,   526,   364,    43,   290,   292,   381,
     294,   265,   295,   296,   307,   137,    50,   115,   308,   195,
     196,    50,   139,   115,   115,   266,   267,   115,   273,   274,
     140,   328,   329,   330,   331,   332,   333,   197,   198,   315,
     268,   269,   317,   165,   115,   367,   275,   276,   169,   165,
     170,   192,   193,   194,   243,   493,   244,   400,   401,   402,
     403,   391,   392,   393,   320,   321,   322,   151,   115,   234,
     115,   236,   237,   238,   235,   236,   237,   238,   199,   200,
     350,   374,   239,   375,   165,   458,   459,   277,   278,   259,
     369,   260,     1,     2,     3,   157,   451,   351,   352,    91,
     369,   115,   205,    92,   115,   206,   492,   207,    15,   394,
     395,   396,   397,   398,   399,   290,   376,   115,   208,   422,
     290,   290,   115,   290,   423,   461,   246,   464,   257,   417,
     283,   474,   383,   475,   262,   263,   476,   214,   353,   354,
     284,    27,   386,   387,   388,   285,   115,   286,   115,    28,
      29,    93,   299,   300,    33,   301,    34,   293,   316,    94,
     311,    36,   201,   202,   203,   204,   279,   280,   281,   282,
     355,   356,   357,   358,   270,   271,   272,   495,   305,   290,
     318,   348,   411,   290,   359,   220,   433,   434,   435,   436,
     437,   220,   220,   220,   220,   220,   220,   220,   220,   220,
     220,   220,   345,   262,   263,   360,   115,   431,   174,   175,
     176,   177,   178,   179,   180,   181,   182,   183,   184,   413,
     290,   430,   431,   431,   115,   438,   439,   440,   441,   172,
     173,   115,   414,   290,   174,   175,   176,   177,   178,   179,
     180,   181,   182,   183,   184,   361,   185,   415,   290,   466,
     362,   468,   365,   471,   420,   421,   215,   455,   456,   463,
     456,    43,   220,    43,   366,    43,   483,   290,   487,   456,
     115,   489,   185,   497,   290,   368,    50,   115,    50,   115,
      50,   372,   115,   115,   431,   467,   453,   115,   503,   290,
     513,   456,   522,   290,   373,   377,   384,   412,   477,   416,
     215,   215,   418,    43,   424,   469,   457,   465,   472,   479,
     480,   484,   290,   485,   491,   496,   498,   502,    50,   506,
     220,   507,   524,   115,   426,   527,   529,   535,   531,   442,
     460,   520,   419,    43,   514,   443,   445,   115,   444,    43,
     481,   521,   431,   115,   533,   534,    43,   446,    50,   379,
     380,   505,   504,    43,    50,     0,   115,   519,   536,   528,
     530,    50,   532,     0,   115,   251,     0,     0,    50,   537,
       0,   515,    43,     0,    43,     0,    43,    43,     0,     0,
       0,    43,     0,    43,    43,     0,     0,    50,     0,    50,
       0,    50,    50,     0,     0,     0,    50,     0,    50,    50,
     110,     0,     1,     2,     3,     0,     0,     0,     0,    91,
       0,     0,     0,    92,     0,    12,    13,     0,    15,     0,
       0,     0,     0,    18,   262,   263,     0,     0,     0,   174,
     175,   176,   177,   178,   179,   180,   181,   182,   183,   184,
       0,     0,     0,    25,    26,     0,     0,     0,     0,     0,
       0,    27,     0,     0,     0,     0,     0,     0,     0,    28,
      29,    93,    31,    32,    33,     0,    34,   185,     0,    94,
       0,    36,     0,     0,    37,    38,    39,    40,     0,     0,
       0,     0,     1,     2,     3,     4,     0,   111,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,     0,     0,    18,    19,    20,     0,    21,    22,     0,
       0,    23,    24,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    25,    26,     0,     0,     0,     0,     0,
       0,    27,     0,     0,     0,     0,     0,     0,     0,    28,
      29,    30,    31,    32,    33,     0,    34,     0,     0,    35,
     155,    36,     0,     0,    37,    38,    39,    40,     0,     0,
       0,     0,     1,     2,     3,     4,     0,    41,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,     0,     0,    18,    19,    20,     0,    21,    22,     0,
       0,    23,    24,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    25,    26,     0,     0,     0,     0,     0,
       0,    27,     0,     0,     0,     0,     0,     0,     0,    28,
      29,    30,    31,    32,    33,     0,    34,     0,     0,    35,
     306,    36,     0,     0,    37,    38,    39,    40,     0,     0,
       0,     0,     1,     2,     3,     4,     0,    41,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,     0,     0,    18,    19,    20,     0,    21,    22,     0,
       0,    23,    24,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    25,    26,     0,     0,     0,     0,     0,
       0,    27,     0,     0,     0,     0,     0,     0,     0,    28,
      29,    30,    31,    32,    33,     0,    34,     0,     0,    35,
     488,    36,     0,     0,    37,    38,    39,    40,     0,     0,
       0,     0,     1,     2,     3,     4,     0,    41,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,     0,     0,    18,    19,    20,     0,    21,    22,     0,
       0,    23,    24,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    25,    26,     0,     0,     0,     0,     0,
       0,    27,     0,     0,     0,     0,     0,     0,     0,    28,
      29,    30,    31,    32,    33,     0,    34,     0,     0,    35,
     511,    36,     0,     0,    37,    38,    39,    40,     0,     0,
       0,     0,     1,     2,     3,     4,     0,    41,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,     0,     0,    18,    19,    20,     0,    21,    22,     0,
       0,    23,    24,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    25,    26,     0,     0,     0,     0,     0,
       0,    27,     0,     0,     0,     0,     0,     0,     0,    28,
      29,    30,    31,    32,    33,     0,    34,     0,     0,    35,
       0,    36,     0,     0,    37,    38,    39,    40,     0,     0,
       0,     0,     1,     2,     3,     4,     0,    41,     5,     6,
       7,     8,     9,     0,    11,    12,    13,    14,    15,    16,
      17,     0,     0,    18,    19,    20,     0,    21,    22,     0,
       0,    23,    24,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    25,    26,     0,     0,     0,     0,     0,
       0,    27,     0,     0,     0,     0,     0,     0,     0,    28,
      29,    30,    31,    32,    33,     0,    34,     0,     0,    35,
       0,    36,     0,     0,    37,    38,    39,    40,     1,     2,
       3,     0,     0,     0,     0,    91,   218,    41,     0,    92,
       0,    12,    13,     0,    15,     0,     0,     0,     0,    18,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    25,
      26,     0,     0,     0,     0,     0,     0,    27,     0,     0,
       0,     0,     0,     0,     0,    28,    29,    93,    31,    32,
      33,     0,    34,     0,     0,    94,     0,    36,     0,     0,
      37,    38,    39,    40,     1,     2,     3,     0,     0,     0,
       0,    91,     0,     0,     0,    92,     0,    12,    13,     0,
      15,     0,     0,     0,     0,    18,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    25,    26,     0,     0,     0,
       0,     0,     0,    27,     0,     0,     0,     0,     0,     0,
       0,    28,    29,    93,    31,    32,    33,     0,    34,     0,
       0,    94,     0,    36,   309,     0,    37,    38,    39,    40,
       1,     2,     3,     0,     0,     0,     0,    91,     0,     0,
       0,    92,     0,    12,    13,     0,    15,     0,     0,     0,
       0,    18,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    25,    26,     0,     0,     0,     0,     0,     0,    27,
       0,     0,     0,     0,     0,     0,     0,    28,    29,    93,
      31,    32,    33,     0,    34,   312,     0,    94,     0,    36,
       0,     0,    37,    38,    39,    40,     1,     2,     3,     0,
       0,     0,     0,    91,     0,     0,     0,    92,     0,    12,
      13,     0,    15,     0,     0,     0,     0,    18,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    25,    26,     0,
       0,     0,     0,     0,     0,    27,     0,     0,     0,     0,
       0,     0,     0,    28,    29,    93,    31,    32,    33,     0,
      34,     0,     0,    94,     0,    36,   473,     0,    37,    38,
      39,    40,     1,     2,     3,     0,     0,     0,     0,    91,
       0,     0,     0,    92,     0,    12,    13,     0,    15,     0,
       0,     0,     0,    18,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    25,    26,     0,     0,     0,     0,     0,
       0,    27,     0,     0,     0,     0,     0,     0,     0,    28,
      29,    93,    31,    32,    33,     0,    34,     0,     0,    94,
       0,    36,     0,     0,    37,    38,    39,    40
};

static const short int yycheck[] =
{
       0,    22,    85,    35,    16,   257,   429,   349,   381,   205,
      47,     6,     1,   190,   191,     0,    16,   206,     1,    11,
       1,   159,   364,   365,     6,     1,     1,   165,   207,    21,
       1,    16,   208,     1,    11,    35,     1,     1,     1,    12,
      13,     1,    34,   209,    21,    18,    61,    62,   186,    69,
      35,    48,    25,    26,    67,    65,    67,    34,    31,    32,
      97,    72,   485,    74,    37,    38,    39,    40,    88,    79,
      80,    22,   210,   156,   212,   112,    90,   114,   451,    62,
      69,    62,   455,   506,   426,    85,     8,   283,    75,    76,
     463,   268,   269,    69,    69,   284,    91,    42,    90,    88,
      85,    69,    42,    62,    69,    88,   285,    88,   246,    91,
     286,    42,    88,    88,   487,    62,   368,    88,   115,   492,
      88,   287,    70,    88,    88,    88,   264,    62,    88,   201,
     202,   203,   204,   192,   193,   194,    87,    67,    62,   151,
     513,    86,   484,   135,    68,   137,    86,   139,   140,    71,
     288,   151,   290,    62,    66,    86,   156,    69,   135,    68,
     137,    65,   139,   140,    69,    67,   151,   159,    73,    22,
      23,   156,    67,   165,   166,    79,    80,   169,    22,    23,
      67,   195,   196,   197,   198,   199,   200,    40,    41,   166,
      75,    76,   169,    67,   186,   232,    40,    41,    72,    67,
      74,    46,    47,    48,    72,   457,    74,   279,   280,   281,
     282,   270,   271,   272,   187,   188,   189,    66,   210,    62,
     212,    60,    61,    62,    67,    60,    61,    62,    81,    82,
      23,    69,    71,    71,    67,   373,    71,    81,    82,    72,
      62,    74,     3,     4,     5,    69,    68,    40,    41,    10,
      62,   243,    83,    14,   246,    84,    68,    85,    19,   273,
     274,   275,   276,   277,   278,    69,   243,   259,    43,    73,
      69,    69,   264,    69,    73,    73,    87,    73,    67,   300,
      83,   419,   259,   421,    44,    45,   424,     0,    81,    82,
      84,    52,   265,   266,   267,    85,   288,    43,   290,    60,
      61,    62,    30,    31,    65,    74,    67,    21,    62,    70,
      69,    72,    36,    37,    38,    39,    36,    37,    38,    39,
      36,    37,    38,    39,    46,    47,    48,   465,    68,    69,
      62,    22,    68,    69,    83,   349,   350,   351,   352,   353,
     354,   355,   356,   357,   358,   359,   360,   361,   362,   363,
     364,   365,    62,    44,    45,    84,   348,   349,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    68,
      69,   348,   364,   365,   366,   355,   356,   357,   358,    44,
      45,   373,    68,    69,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    85,    87,    68,    69,   411,
      43,   413,    69,   415,    68,    69,   489,    68,    69,    68,
      69,   411,   426,   413,    88,   415,    68,    69,    68,    69,
     412,   453,    87,    68,    69,    67,   411,   419,   413,   421,
     415,    62,   424,   425,   426,   412,    70,   429,    68,    69,
      68,    69,    68,    69,    66,    62,    62,    67,   425,    67,
     533,   534,    62,   453,    66,    70,    67,    66,    62,    22,
      62,    66,    69,    88,    62,    35,     7,    68,   453,    88,
     484,   483,    68,   465,    87,    66,    31,    71,    68,   359,
     374,   502,   307,   483,   496,   360,   362,   479,   361,   489,
     428,   503,   484,   485,   526,   527,   496,   363,   483,   249,
     253,   480,   479,   503,   489,    -1,   498,   500,   529,   518,
     522,   496,   524,    -1,   506,   103,    -1,    -1,   503,   531,
      -1,   498,   522,    -1,   524,    -1,   526,   527,    -1,    -1,
      -1,   531,    -1,   533,   534,    -1,    -1,   522,    -1,   524,
      -1,   526,   527,    -1,    -1,    -1,   531,    -1,   533,   534,
       1,    -1,     3,     4,     5,    -1,    -1,    -1,    -1,    10,
      -1,    -1,    -1,    14,    -1,    16,    17,    -1,    19,    -1,
      -1,    -1,    -1,    24,    44,    45,    -1,    -1,    -1,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      -1,    -1,    -1,    44,    45,    -1,    -1,    -1,    -1,    -1,
      -1,    52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    60,
      61,    62,    63,    64,    65,    -1,    67,    87,    -1,    70,
      -1,    72,    -1,    -1,    75,    76,    77,    78,    -1,    -1,
      -1,    -1,     3,     4,     5,     6,    -1,    88,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    -1,    -1,    24,    25,    26,    -1,    28,    29,    -1,
      -1,    32,    33,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    44,    45,    -1,    -1,    -1,    -1,    -1,
      -1,    52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    60,
      61,    62,    63,    64,    65,    -1,    67,    -1,    -1,    70,
      71,    72,    -1,    -1,    75,    76,    77,    78,    -1,    -1,
      -1,    -1,     3,     4,     5,     6,    -1,    88,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    -1,    -1,    24,    25,    26,    -1,    28,    29,    -1,
      -1,    32,    33,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    44,    45,    -1,    -1,    -1,    -1,    -1,
      -1,    52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    60,
      61,    62,    63,    64,    65,    -1,    67,    -1,    -1,    70,
      71,    72,    -1,    -1,    75,    76,    77,    78,    -1,    -1,
      -1,    -1,     3,     4,     5,     6,    -1,    88,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    -1,    -1,    24,    25,    26,    -1,    28,    29,    -1,
      -1,    32,    33,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    44,    45,    -1,    -1,    -1,    -1,    -1,
      -1,    52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    60,
      61,    62,    63,    64,    65,    -1,    67,    -1,    -1,    70,
      71,    72,    -1,    -1,    75,    76,    77,    78,    -1,    -1,
      -1,    -1,     3,     4,     5,     6,    -1,    88,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    -1,    -1,    24,    25,    26,    -1,    28,    29,    -1,
      -1,    32,    33,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    44,    45,    -1,    -1,    -1,    -1,    -1,
      -1,    52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    60,
      61,    62,    63,    64,    65,    -1,    67,    -1,    -1,    70,
      71,    72,    -1,    -1,    75,    76,    77,    78,    -1,    -1,
      -1,    -1,     3,     4,     5,     6,    -1,    88,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    -1,    -1,    24,    25,    26,    -1,    28,    29,    -1,
      -1,    32,    33,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    44,    45,    -1,    -1,    -1,    -1,    -1,
      -1,    52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    60,
      61,    62,    63,    64,    65,    -1,    67,    -1,    -1,    70,
      -1,    72,    -1,    -1,    75,    76,    77,    78,    -1,    -1,
      -1,    -1,     3,     4,     5,     6,    -1,    88,     9,    10,
      11,    12,    13,    -1,    15,    16,    17,    18,    19,    20,
      21,    -1,    -1,    24,    25,    26,    -1,    28,    29,    -1,
      -1,    32,    33,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    44,    45,    -1,    -1,    -1,    -1,    -1,
      -1,    52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    60,
      61,    62,    63,    64,    65,    -1,    67,    -1,    -1,    70,
      -1,    72,    -1,    -1,    75,    76,    77,    78,     3,     4,
       5,    -1,    -1,    -1,    -1,    10,    11,    88,    -1,    14,
      -1,    16,    17,    -1,    19,    -1,    -1,    -1,    -1,    24,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    44,
      45,    -1,    -1,    -1,    -1,    -1,    -1,    52,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    60,    61,    62,    63,    64,
      65,    -1,    67,    -1,    -1,    70,    -1,    72,    -1,    -1,
      75,    76,    77,    78,     3,     4,     5,    -1,    -1,    -1,
      -1,    10,    -1,    -1,    -1,    14,    -1,    16,    17,    -1,
      19,    -1,    -1,    -1,    -1,    24,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    44,    45,    -1,    -1,    -1,
      -1,    -1,    -1,    52,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    60,    61,    62,    63,    64,    65,    -1,    67,    -1,
      -1,    70,    -1,    72,    73,    -1,    75,    76,    77,    78,
       3,     4,     5,    -1,    -1,    -1,    -1,    10,    -1,    -1,
      -1,    14,    -1,    16,    17,    -1,    19,    -1,    -1,    -1,
      -1,    24,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    44,    45,    -1,    -1,    -1,    -1,    -1,    -1,    52,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    60,    61,    62,
      63,    64,    65,    -1,    67,    68,    -1,    70,    -1,    72,
      -1,    -1,    75,    76,    77,    78,     3,     4,     5,    -1,
      -1,    -1,    -1,    10,    -1,    -1,    -1,    14,    -1,    16,
      17,    -1,    19,    -1,    -1,    -1,    -1,    24,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    44,    45,    -1,
      -1,    -1,    -1,    -1,    -1,    52,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    60,    61,    62,    63,    64,    65,    -1,
      67,    -1,    -1,    70,    -1,    72,    73,    -1,    75,    76,
      77,    78,     3,     4,     5,    -1,    -1,    -1,    -1,    10,
      -1,    -1,    -1,    14,    -1,    16,    17,    -1,    19,    -1,
      -1,    -1,    -1,    24,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    44,    45,    -1,    -1,    -1,    -1,    -1,
      -1,    52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    60,
      61,    62,    63,    64,    65,    -1,    67,    -1,    -1,    70,
      -1,    72,    -1,    -1,    75,    76,    77,    78
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,     3,     4,     5,     6,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    24,    25,
      26,    28,    29,    32,    33,    44,    45,    52,    60,    61,
      62,    63,    64,    65,    67,    70,    72,    75,    76,    77,
      78,    88,    90,    95,    96,   101,   103,   105,   109,   111,
     112,   114,   116,   118,   120,   123,   126,   129,   132,   135,
     138,   141,   144,   147,   151,   152,   153,   154,   159,   164,
     165,   166,   167,   170,   171,   172,   173,   174,   180,   181,
     182,   183,   186,   187,   191,   192,   193,     1,    62,    88,
      67,    10,    14,    62,    70,    94,    95,   100,   102,   188,
      62,   155,   157,    62,   160,   161,     1,    62,    88,    62,
       1,    88,   100,   102,   104,   108,   110,   112,   113,   115,
     117,   119,   121,   124,   127,   130,   133,   136,   139,   142,
     145,   149,   108,   113,   113,    67,   152,    67,   113,    67,
      67,   149,   153,     1,    88,    61,    62,   184,   185,   113,
     113,    66,   113,   113,   149,    71,   192,    69,    97,    98,
      99,   113,   113,   113,   113,    67,    72,    74,   106,    72,
      74,   106,    44,    45,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    87,   148,    65,    79,    80,
      75,    76,    46,    47,    48,    22,    23,    40,    41,    81,
      82,    36,    37,    38,    39,    83,    84,    85,    43,    42,
      86,     1,    69,    88,     0,   193,     1,    88,    11,   108,
     119,   122,   125,   128,   131,   134,   137,   140,   143,   146,
     150,   169,   100,   102,    62,    67,    60,    61,    62,    71,
      91,    92,    93,    72,    74,   106,    87,   162,     1,    69,
      88,   162,     1,    69,    88,     1,    88,    67,   106,    72,
      74,   106,    44,    45,   148,    65,    79,    80,    75,    76,
      46,    47,    48,    22,    23,    40,    41,    81,    82,    36,
      37,    38,    39,    83,    84,    85,    43,    42,    86,     1,
      69,    88,   149,    21,   149,   149,   149,     1,    88,    30,
      31,    74,     1,    88,   152,    68,    71,    69,    73,    73,
     145,    69,    68,   107,   145,   149,    62,   149,    62,   145,
     113,   113,   113,   115,   115,   117,   117,   117,   119,   119,
     119,   119,   119,   119,   121,   121,   121,   121,   124,   127,
     130,   133,   136,   145,   145,    62,   156,   158,    22,   148,
      23,    40,    41,    81,    82,    36,    37,    38,    39,    83,
      84,    85,    43,    42,    86,    69,    88,   106,    67,    62,
      68,   189,    62,    66,    69,    71,   149,    62,   145,   157,
     161,    68,   189,   149,    62,   145,   113,   113,   113,   115,
     115,   117,   117,   117,   119,   119,   119,   119,   119,   119,
     121,   121,   121,   121,   124,   127,   130,   133,   136,   145,
     145,    68,    67,    68,    68,    68,    67,   153,    62,    98,
      68,    69,    73,    73,    66,    22,    87,   163,    69,    88,
     149,   108,   146,   119,   119,   119,   119,   119,   122,   122,
     122,   122,   125,   128,   131,   134,   137,   146,   146,   149,
     168,    68,   189,    70,   190,    68,    69,    67,   145,    71,
      92,    73,   190,    68,    73,    66,   152,   149,   152,    70,
     175,   152,    62,    73,   145,   145,   145,   149,   146,    22,
      62,   158,   168,    68,    66,    88,   190,    68,    71,   192,
     190,    62,    68,   189,   190,   145,    35,    68,     7,   176,
     177,   178,    68,    68,   149,   163,    88,   152,   146,   168,
     190,    71,   190,    68,   152,   149,     8,    71,   179,   178,
     153,   152,    68,   168,    68,   190,    66,    66,   176,    31,
     152,    68,   152,   192,   192,    71,   153,   152
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
#line 193 "grammar.y"
    { (yyval.node) = new NullNode(); ;}
    break;

  case 3:
#line 194 "grammar.y"
    { (yyval.node) = new BooleanNode(true); ;}
    break;

  case 4:
#line 195 "grammar.y"
    { (yyval.node) = new BooleanNode(false); ;}
    break;

  case 5:
#line 196 "grammar.y"
    { (yyval.node) = new NumberNode((yyvsp[0].dval)); ;}
    break;

  case 6:
#line 197 "grammar.y"
    { (yyval.node) = new StringNode((yyvsp[0].ustr)); ;}
    break;

  case 7:
#line 198 "grammar.y"
    {
                                            Lexer *l = Lexer::curr();
                                            if (!l->scanRegExp()) YYABORT;
                                            (yyval.node) = new RegExpNode(l->pattern, l->flags);
                                        ;}
    break;

  case 8:
#line 203 "grammar.y"
    {
                                            Lexer *l = Lexer::curr();
                                            if (!l->scanRegExp()) YYABORT;
                                            (yyval.node) = new RegExpNode(UString('=') + l->pattern, l->flags);
                                        ;}
    break;

  case 9:
#line 211 "grammar.y"
    { (yyval.pname) = new PropertyNameNode(*(yyvsp[0].ident)); ;}
    break;

  case 10:
#line 212 "grammar.y"
    { (yyval.pname) = new PropertyNameNode(Identifier(*(yyvsp[0].ustr))); ;}
    break;

  case 11:
#line 213 "grammar.y"
    { (yyval.pname) = new PropertyNameNode((yyvsp[0].dval)); ;}
    break;

  case 12:
#line 217 "grammar.y"
    { (yyval.pnode) = new PropertyNode((yyvsp[-2].pname), (yyvsp[0].node), PropertyNode::Constant); ;}
    break;

  case 13:
#line 218 "grammar.y"
    { if (!makeGetterOrSetterPropertyNode((yyval.pnode), *(yyvsp[-4].ident), *(yyvsp[-3].ident), 0, (yyvsp[0].body))) YYABORT; ;}
    break;

  case 14:
#line 220 "grammar.y"
    { if (!makeGetterOrSetterPropertyNode((yyval.pnode), *(yyvsp[-5].ident), *(yyvsp[-4].ident), (yyvsp[-2].param), (yyvsp[0].body))) YYABORT; ;}
    break;

  case 15:
#line 224 "grammar.y"
    { (yyval.plist) = new PropertyListNode((yyvsp[0].pnode)); ;}
    break;

  case 16:
#line 225 "grammar.y"
    { (yyval.plist) = new PropertyListNode((yyvsp[0].pnode), (yyvsp[-2].plist)); ;}
    break;

  case 18:
#line 230 "grammar.y"
    { (yyval.node) = new ObjectLiteralNode(); ;}
    break;

  case 19:
#line 231 "grammar.y"
    { (yyval.node) = new ObjectLiteralNode((yyvsp[-1].plist)); ;}
    break;

  case 20:
#line 233 "grammar.y"
    { (yyval.node) = new ObjectLiteralNode((yyvsp[-2].plist)); ;}
    break;

  case 21:
#line 237 "grammar.y"
    { (yyval.node) = new ThisNode(); ;}
    break;

  case 24:
#line 240 "grammar.y"
    { (yyval.node) = new ResolveNode(*(yyvsp[0].ident)); ;}
    break;

  case 25:
#line 241 "grammar.y"
    { (yyval.node) = ((yyvsp[-1].node)->isResolveNode() || (yyvsp[-1].node)->isGroupNode()) ?
                                            (yyvsp[-1].node) : new GroupNode((yyvsp[-1].node)); ;}
    break;

  case 26:
#line 246 "grammar.y"
    { (yyval.node) = new ArrayNode((yyvsp[-1].ival)); ;}
    break;

  case 27:
#line 247 "grammar.y"
    { (yyval.node) = new ArrayNode((yyvsp[-1].elm)); ;}
    break;

  case 28:
#line 248 "grammar.y"
    { (yyval.node) = new ArrayNode((yyvsp[-1].ival), (yyvsp[-3].elm)); ;}
    break;

  case 29:
#line 252 "grammar.y"
    { (yyval.elm) = new ElementNode((yyvsp[-1].ival), (yyvsp[0].node)); ;}
    break;

  case 30:
#line 254 "grammar.y"
    { (yyval.elm) = new ElementNode((yyvsp[-3].elm), (yyvsp[-1].ival), (yyvsp[0].node)); ;}
    break;

  case 31:
#line 258 "grammar.y"
    { (yyval.ival) = 0; ;}
    break;

  case 33:
#line 263 "grammar.y"
    { (yyval.ival) = 1; ;}
    break;

  case 34:
#line 264 "grammar.y"
    { (yyval.ival) = (yyvsp[-1].ival) + 1; ;}
    break;

  case 36:
#line 269 "grammar.y"
    { (yyval.node) = (yyvsp[0].funcExpr); ;}
    break;

  case 37:
#line 270 "grammar.y"
    { (yyval.node) = new BracketAccessorNode((yyvsp[-3].node), (yyvsp[-1].node)); ;}
    break;

  case 38:
#line 271 "grammar.y"
    { (yyval.node) = new DotAccessorNode((yyvsp[-2].node), *(yyvsp[0].ident)); ;}
    break;

  case 39:
#line 272 "grammar.y"
    { (yyval.node) = new NewExprNode((yyvsp[-1].node), (yyvsp[0].args)); ;}
    break;

  case 41:
#line 277 "grammar.y"
    { (yyval.node) = new BracketAccessorNode((yyvsp[-3].node), (yyvsp[-1].node)); ;}
    break;

  case 42:
#line 278 "grammar.y"
    { (yyval.node) = new DotAccessorNode((yyvsp[-2].node), *(yyvsp[0].ident)); ;}
    break;

  case 43:
#line 279 "grammar.y"
    { (yyval.node) = new NewExprNode((yyvsp[-1].node), (yyvsp[0].args)); ;}
    break;

  case 45:
#line 284 "grammar.y"
    { (yyval.node) = new NewExprNode((yyvsp[0].node)); ;}
    break;

  case 47:
#line 289 "grammar.y"
    { (yyval.node) = new NewExprNode((yyvsp[0].node)); ;}
    break;

  case 48:
#line 293 "grammar.y"
    { (yyval.node) = makeFunctionCallNode((yyvsp[-1].node), (yyvsp[0].args)); ;}
    break;

  case 49:
#line 294 "grammar.y"
    { (yyval.node) = makeFunctionCallNode((yyvsp[-1].node), (yyvsp[0].args)); ;}
    break;

  case 50:
#line 295 "grammar.y"
    { (yyval.node) = new BracketAccessorNode((yyvsp[-3].node), (yyvsp[-1].node)); ;}
    break;

  case 51:
#line 296 "grammar.y"
    { (yyval.node) = new DotAccessorNode((yyvsp[-2].node), *(yyvsp[0].ident)); ;}
    break;

  case 52:
#line 300 "grammar.y"
    { (yyval.node) = makeFunctionCallNode((yyvsp[-1].node), (yyvsp[0].args)); ;}
    break;

  case 53:
#line 301 "grammar.y"
    { (yyval.node) = makeFunctionCallNode((yyvsp[-1].node), (yyvsp[0].args)); ;}
    break;

  case 54:
#line 302 "grammar.y"
    { (yyval.node) = new BracketAccessorNode((yyvsp[-3].node), (yyvsp[-1].node)); ;}
    break;

  case 55:
#line 303 "grammar.y"
    { (yyval.node) = new DotAccessorNode((yyvsp[-2].node), *(yyvsp[0].ident)); ;}
    break;

  case 56:
#line 307 "grammar.y"
    { (yyval.args) = new ArgumentsNode(); ;}
    break;

  case 57:
#line 308 "grammar.y"
    { (yyval.args) = new ArgumentsNode((yyvsp[-1].alist)); ;}
    break;

  case 58:
#line 312 "grammar.y"
    { (yyval.alist) = new ArgumentListNode((yyvsp[0].node)); ;}
    break;

  case 59:
#line 313 "grammar.y"
    { (yyval.alist) = new ArgumentListNode((yyvsp[-2].alist), (yyvsp[0].node)); ;}
    break;

  case 65:
#line 328 "grammar.y"
    { if (!makePostfixNode((yyval.node), (yyvsp[-1].node), OpPlusPlus)) YYABORT; ;}
    break;

  case 66:
#line 329 "grammar.y"
    { if (!makePostfixNode((yyval.node), (yyvsp[-1].node), OpMinusMinus)) YYABORT; ;}
    break;

  case 68:
#line 334 "grammar.y"
    { if (!makePostfixNode((yyval.node), (yyvsp[-1].node), OpPlusPlus)) YYABORT; ;}
    break;

  case 69:
#line 335 "grammar.y"
    { if (!makePostfixNode((yyval.node), (yyvsp[-1].node), OpMinusMinus)) YYABORT; ;}
    break;

  case 70:
#line 339 "grammar.y"
    { (yyval.node) = makeDeleteNode((yyvsp[0].node)); ;}
    break;

  case 71:
#line 340 "grammar.y"
    { (yyval.node) = new VoidNode((yyvsp[0].node)); ;}
    break;

  case 72:
#line 341 "grammar.y"
    { (yyval.node) = makeTypeOfNode((yyvsp[0].node)); ;}
    break;

  case 73:
#line 342 "grammar.y"
    { if (!makePrefixNode((yyval.node), (yyvsp[0].node), OpPlusPlus)) YYABORT; ;}
    break;

  case 74:
#line 343 "grammar.y"
    { if (!makePrefixNode((yyval.node), (yyvsp[0].node), OpPlusPlus)) YYABORT; ;}
    break;

  case 75:
#line 344 "grammar.y"
    { if (!makePrefixNode((yyval.node), (yyvsp[0].node), OpMinusMinus)) YYABORT; ;}
    break;

  case 76:
#line 345 "grammar.y"
    { if (!makePrefixNode((yyval.node), (yyvsp[0].node), OpMinusMinus)) YYABORT; ;}
    break;

  case 77:
#line 346 "grammar.y"
    { (yyval.node) = new UnaryPlusNode((yyvsp[0].node)); ;}
    break;

  case 78:
#line 347 "grammar.y"
    { (yyval.node) = new NegateNode((yyvsp[0].node)); ;}
    break;

  case 79:
#line 348 "grammar.y"
    { (yyval.node) = new BitwiseNotNode((yyvsp[0].node)); ;}
    break;

  case 80:
#line 349 "grammar.y"
    { (yyval.node) = new LogicalNotNode((yyvsp[0].node)); ;}
    break;

  case 86:
#line 363 "grammar.y"
    { (yyval.node) = new MultNode((yyvsp[-2].node), (yyvsp[0].node), '*'); ;}
    break;

  case 87:
#line 364 "grammar.y"
    { (yyval.node) = new MultNode((yyvsp[-2].node), (yyvsp[0].node), '/'); ;}
    break;

  case 88:
#line 365 "grammar.y"
    { (yyval.node) = new MultNode((yyvsp[-2].node), (yyvsp[0].node),'%'); ;}
    break;

  case 90:
#line 371 "grammar.y"
    { (yyval.node) = new MultNode((yyvsp[-2].node), (yyvsp[0].node), '*'); ;}
    break;

  case 91:
#line 373 "grammar.y"
    { (yyval.node) = new MultNode((yyvsp[-2].node), (yyvsp[0].node), '/'); ;}
    break;

  case 92:
#line 375 "grammar.y"
    { (yyval.node) = new MultNode((yyvsp[-2].node), (yyvsp[0].node),'%'); ;}
    break;

  case 94:
#line 380 "grammar.y"
    { (yyval.node) = new AddNode((yyvsp[-2].node), (yyvsp[0].node), '+'); ;}
    break;

  case 95:
#line 381 "grammar.y"
    { (yyval.node) = new AddNode((yyvsp[-2].node), (yyvsp[0].node), '-'); ;}
    break;

  case 97:
#line 387 "grammar.y"
    { (yyval.node) = new AddNode((yyvsp[-2].node), (yyvsp[0].node), '+'); ;}
    break;

  case 98:
#line 389 "grammar.y"
    { (yyval.node) = new AddNode((yyvsp[-2].node), (yyvsp[0].node), '-'); ;}
    break;

  case 100:
#line 394 "grammar.y"
    { (yyval.node) = new ShiftNode((yyvsp[-2].node), OpLShift, (yyvsp[0].node)); ;}
    break;

  case 101:
#line 395 "grammar.y"
    { (yyval.node) = new ShiftNode((yyvsp[-2].node), OpRShift, (yyvsp[0].node)); ;}
    break;

  case 102:
#line 396 "grammar.y"
    { (yyval.node) = new ShiftNode((yyvsp[-2].node), OpURShift, (yyvsp[0].node)); ;}
    break;

  case 104:
#line 401 "grammar.y"
    { (yyval.node) = new ShiftNode((yyvsp[-2].node), OpLShift, (yyvsp[0].node)); ;}
    break;

  case 105:
#line 402 "grammar.y"
    { (yyval.node) = new ShiftNode((yyvsp[-2].node), OpRShift, (yyvsp[0].node)); ;}
    break;

  case 106:
#line 403 "grammar.y"
    { (yyval.node) = new ShiftNode((yyvsp[-2].node), OpURShift, (yyvsp[0].node)); ;}
    break;

  case 108:
#line 408 "grammar.y"
    { (yyval.node) = new RelationalNode((yyvsp[-2].node), OpLess, (yyvsp[0].node)); ;}
    break;

  case 109:
#line 409 "grammar.y"
    { (yyval.node) = new RelationalNode((yyvsp[-2].node), OpGreater, (yyvsp[0].node)); ;}
    break;

  case 110:
#line 410 "grammar.y"
    { (yyval.node) = new RelationalNode((yyvsp[-2].node), OpLessEq, (yyvsp[0].node)); ;}
    break;

  case 111:
#line 411 "grammar.y"
    { (yyval.node) = new RelationalNode((yyvsp[-2].node), OpGreaterEq, (yyvsp[0].node)); ;}
    break;

  case 112:
#line 412 "grammar.y"
    { (yyval.node) = new RelationalNode((yyvsp[-2].node), OpInstanceOf, (yyvsp[0].node)); ;}
    break;

  case 113:
#line 413 "grammar.y"
    { (yyval.node) = new RelationalNode((yyvsp[-2].node), OpIn, (yyvsp[0].node)); ;}
    break;

  case 115:
#line 418 "grammar.y"
    { (yyval.node) = new RelationalNode((yyvsp[-2].node), OpLess, (yyvsp[0].node)); ;}
    break;

  case 116:
#line 419 "grammar.y"
    { (yyval.node) = new RelationalNode((yyvsp[-2].node), OpGreater, (yyvsp[0].node)); ;}
    break;

  case 117:
#line 420 "grammar.y"
    { (yyval.node) = new RelationalNode((yyvsp[-2].node), OpLessEq, (yyvsp[0].node)); ;}
    break;

  case 118:
#line 421 "grammar.y"
    { (yyval.node) = new RelationalNode((yyvsp[-2].node), OpGreaterEq, (yyvsp[0].node)); ;}
    break;

  case 119:
#line 423 "grammar.y"
    { (yyval.node) = new RelationalNode((yyvsp[-2].node), OpInstanceOf, (yyvsp[0].node)); ;}
    break;

  case 121:
#line 428 "grammar.y"
    { (yyval.node) = new RelationalNode((yyvsp[-2].node), OpLess, (yyvsp[0].node)); ;}
    break;

  case 122:
#line 429 "grammar.y"
    { (yyval.node) = new RelationalNode((yyvsp[-2].node), OpGreater, (yyvsp[0].node)); ;}
    break;

  case 123:
#line 430 "grammar.y"
    { (yyval.node) = new RelationalNode((yyvsp[-2].node), OpLessEq, (yyvsp[0].node)); ;}
    break;

  case 124:
#line 431 "grammar.y"
    { (yyval.node) = new RelationalNode((yyvsp[-2].node), OpGreaterEq, (yyvsp[0].node)); ;}
    break;

  case 125:
#line 433 "grammar.y"
    { (yyval.node) = new RelationalNode((yyvsp[-2].node), OpInstanceOf, (yyvsp[0].node)); ;}
    break;

  case 126:
#line 434 "grammar.y"
    { (yyval.node) = new RelationalNode((yyvsp[-2].node), OpIn, (yyvsp[0].node)); ;}
    break;

  case 128:
#line 439 "grammar.y"
    { (yyval.node) = new EqualNode((yyvsp[-2].node), OpEqEq, (yyvsp[0].node)); ;}
    break;

  case 129:
#line 440 "grammar.y"
    { (yyval.node) = new EqualNode((yyvsp[-2].node), OpNotEq, (yyvsp[0].node)); ;}
    break;

  case 130:
#line 441 "grammar.y"
    { (yyval.node) = new EqualNode((yyvsp[-2].node), OpStrEq, (yyvsp[0].node)); ;}
    break;

  case 131:
#line 442 "grammar.y"
    { (yyval.node) = new EqualNode((yyvsp[-2].node), OpStrNEq, (yyvsp[0].node));;}
    break;

  case 133:
#line 448 "grammar.y"
    { (yyval.node) = new EqualNode((yyvsp[-2].node), OpEqEq, (yyvsp[0].node)); ;}
    break;

  case 134:
#line 450 "grammar.y"
    { (yyval.node) = new EqualNode((yyvsp[-2].node), OpNotEq, (yyvsp[0].node)); ;}
    break;

  case 135:
#line 452 "grammar.y"
    { (yyval.node) = new EqualNode((yyvsp[-2].node), OpStrEq, (yyvsp[0].node)); ;}
    break;

  case 136:
#line 454 "grammar.y"
    { (yyval.node) = new EqualNode((yyvsp[-2].node), OpStrNEq, (yyvsp[0].node));;}
    break;

  case 138:
#line 460 "grammar.y"
    { (yyval.node) = new EqualNode((yyvsp[-2].node), OpEqEq, (yyvsp[0].node)); ;}
    break;

  case 139:
#line 461 "grammar.y"
    { (yyval.node) = new EqualNode((yyvsp[-2].node), OpNotEq, (yyvsp[0].node)); ;}
    break;

  case 140:
#line 463 "grammar.y"
    { (yyval.node) = new EqualNode((yyvsp[-2].node), OpStrEq, (yyvsp[0].node)); ;}
    break;

  case 141:
#line 465 "grammar.y"
    { (yyval.node) = new EqualNode((yyvsp[-2].node), OpStrNEq, (yyvsp[0].node));;}
    break;

  case 143:
#line 470 "grammar.y"
    { (yyval.node) = new BitOperNode((yyvsp[-2].node), OpBitAnd, (yyvsp[0].node)); ;}
    break;

  case 145:
#line 476 "grammar.y"
    { (yyval.node) = new BitOperNode((yyvsp[-2].node), OpBitAnd, (yyvsp[0].node)); ;}
    break;

  case 147:
#line 481 "grammar.y"
    { (yyval.node) = new BitOperNode((yyvsp[-2].node), OpBitAnd, (yyvsp[0].node)); ;}
    break;

  case 149:
#line 486 "grammar.y"
    { (yyval.node) = new BitOperNode((yyvsp[-2].node), OpBitXOr, (yyvsp[0].node)); ;}
    break;

  case 151:
#line 492 "grammar.y"
    { (yyval.node) = new BitOperNode((yyvsp[-2].node), OpBitXOr, (yyvsp[0].node)); ;}
    break;

  case 153:
#line 498 "grammar.y"
    { (yyval.node) = new BitOperNode((yyvsp[-2].node), OpBitXOr, (yyvsp[0].node)); ;}
    break;

  case 155:
#line 503 "grammar.y"
    { (yyval.node) = new BitOperNode((yyvsp[-2].node), OpBitOr, (yyvsp[0].node)); ;}
    break;

  case 157:
#line 509 "grammar.y"
    { (yyval.node) = new BitOperNode((yyvsp[-2].node), OpBitOr, (yyvsp[0].node)); ;}
    break;

  case 159:
#line 515 "grammar.y"
    { (yyval.node) = new BitOperNode((yyvsp[-2].node), OpBitOr, (yyvsp[0].node)); ;}
    break;

  case 161:
#line 520 "grammar.y"
    { (yyval.node) = new BinaryLogicalNode((yyvsp[-2].node), OpAnd, (yyvsp[0].node)); ;}
    break;

  case 163:
#line 526 "grammar.y"
    { (yyval.node) = new BinaryLogicalNode((yyvsp[-2].node), OpAnd, (yyvsp[0].node)); ;}
    break;

  case 165:
#line 532 "grammar.y"
    { (yyval.node) = new BinaryLogicalNode((yyvsp[-2].node), OpAnd, (yyvsp[0].node)); ;}
    break;

  case 167:
#line 537 "grammar.y"
    { (yyval.node) = new BinaryLogicalNode((yyvsp[-2].node), OpOr, (yyvsp[0].node)); ;}
    break;

  case 169:
#line 543 "grammar.y"
    { (yyval.node) = new BinaryLogicalNode((yyvsp[-2].node), OpOr, (yyvsp[0].node)); ;}
    break;

  case 171:
#line 548 "grammar.y"
    { (yyval.node) = new BinaryLogicalNode((yyvsp[-2].node), OpOr, (yyvsp[0].node)); ;}
    break;

  case 173:
#line 554 "grammar.y"
    { (yyval.node) = new ConditionalNode((yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node)); ;}
    break;

  case 175:
#line 560 "grammar.y"
    { (yyval.node) = new ConditionalNode((yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node)); ;}
    break;

  case 177:
#line 566 "grammar.y"
    { (yyval.node) = new ConditionalNode((yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node)); ;}
    break;

  case 179:
#line 572 "grammar.y"
    { if (!makeAssignNode((yyval.node), (yyvsp[-2].node), (yyvsp[-1].op), (yyvsp[0].node))) YYABORT; ;}
    break;

  case 181:
#line 578 "grammar.y"
    { if (!makeAssignNode((yyval.node), (yyvsp[-2].node), (yyvsp[-1].op), (yyvsp[0].node))) YYABORT; ;}
    break;

  case 183:
#line 584 "grammar.y"
    { if (!makeAssignNode((yyval.node), (yyvsp[-2].node), (yyvsp[-1].op), (yyvsp[0].node))) YYABORT; ;}
    break;

  case 184:
#line 588 "grammar.y"
    { (yyval.op) = OpEqual; ;}
    break;

  case 185:
#line 589 "grammar.y"
    { (yyval.op) = OpPlusEq; ;}
    break;

  case 186:
#line 590 "grammar.y"
    { (yyval.op) = OpMinusEq; ;}
    break;

  case 187:
#line 591 "grammar.y"
    { (yyval.op) = OpMultEq; ;}
    break;

  case 188:
#line 592 "grammar.y"
    { (yyval.op) = OpDivEq; ;}
    break;

  case 189:
#line 593 "grammar.y"
    { (yyval.op) = OpLShift; ;}
    break;

  case 190:
#line 594 "grammar.y"
    { (yyval.op) = OpRShift; ;}
    break;

  case 191:
#line 595 "grammar.y"
    { (yyval.op) = OpURShift; ;}
    break;

  case 192:
#line 596 "grammar.y"
    { (yyval.op) = OpAndEq; ;}
    break;

  case 193:
#line 597 "grammar.y"
    { (yyval.op) = OpXOrEq; ;}
    break;

  case 194:
#line 598 "grammar.y"
    { (yyval.op) = OpOrEq; ;}
    break;

  case 195:
#line 599 "grammar.y"
    { (yyval.op) = OpModEq; ;}
    break;

  case 197:
#line 604 "grammar.y"
    { (yyval.node) = new CommaNode((yyvsp[-2].node), (yyvsp[0].node)); ;}
    break;

  case 199:
#line 609 "grammar.y"
    { (yyval.node) = new CommaNode((yyvsp[-2].node), (yyvsp[0].node)); ;}
    break;

  case 201:
#line 614 "grammar.y"
    { (yyval.node) = new CommaNode((yyvsp[-2].node), (yyvsp[0].node)); ;}
    break;

  case 219:
#line 638 "grammar.y"
    { (yyval.stat) = new BlockNode(0); DBG((yyval.stat), (yylsp[0]), (yylsp[0])); ;}
    break;

  case 220:
#line 639 "grammar.y"
    { (yyval.stat) = new BlockNode((yyvsp[-1].srcs)); DBG((yyval.stat), (yylsp[0]), (yylsp[0])); ;}
    break;

  case 221:
#line 643 "grammar.y"
    { (yyval.stat) = new VarStatementNode((yyvsp[-1].vlist)); DBG((yyval.stat), (yylsp[-2]), (yylsp[0])); ;}
    break;

  case 222:
#line 644 "grammar.y"
    { (yyval.stat) = new VarStatementNode((yyvsp[-1].vlist)); DBG((yyval.stat), (yylsp[-2]), (yylsp[-1])); AUTO_SEMICOLON; ;}
    break;

  case 223:
#line 648 "grammar.y"
    { (yyval.vlist) = new VarDeclListNode((yyvsp[0].decl)); ;}
    break;

  case 224:
#line 650 "grammar.y"
    { (yyval.vlist) = new VarDeclListNode((yyvsp[-2].vlist), (yyvsp[0].decl)); ;}
    break;

  case 225:
#line 654 "grammar.y"
    { (yyval.vlist) = new VarDeclListNode((yyvsp[0].decl)); ;}
    break;

  case 226:
#line 656 "grammar.y"
    { (yyval.vlist) = new VarDeclListNode((yyvsp[-2].vlist), (yyvsp[0].decl)); ;}
    break;

  case 227:
#line 660 "grammar.y"
    { (yyval.decl) = new VarDeclNode(*(yyvsp[0].ident), 0, VarDeclNode::Variable); ;}
    break;

  case 228:
#line 661 "grammar.y"
    { (yyval.decl) = new VarDeclNode(*(yyvsp[-1].ident), (yyvsp[0].init), VarDeclNode::Variable); ;}
    break;

  case 229:
#line 665 "grammar.y"
    { (yyval.decl) = new VarDeclNode(*(yyvsp[0].ident), 0, VarDeclNode::Variable); ;}
    break;

  case 230:
#line 666 "grammar.y"
    { (yyval.decl) = new VarDeclNode(*(yyvsp[-1].ident), (yyvsp[0].init), VarDeclNode::Variable); ;}
    break;

  case 231:
#line 670 "grammar.y"
    { (yyval.stat) = new VarStatementNode((yyvsp[-1].vlist)); DBG((yyval.stat), (yylsp[-2]), (yylsp[0])); ;}
    break;

  case 232:
#line 671 "grammar.y"
    { (yyval.stat) = new VarStatementNode((yyvsp[-1].vlist)); DBG((yyval.stat), (yylsp[-2]), (yylsp[-1])); AUTO_SEMICOLON; ;}
    break;

  case 233:
#line 675 "grammar.y"
    { (yyval.vlist) = new VarDeclListNode((yyvsp[0].decl)); ;}
    break;

  case 234:
#line 677 "grammar.y"
    { (yyval.vlist) = new VarDeclListNode((yyvsp[-2].vlist), (yyvsp[0].decl)); ;}
    break;

  case 235:
#line 681 "grammar.y"
    { (yyval.decl) = new VarDeclNode(*(yyvsp[0].ident), 0, VarDeclNode::Constant); ;}
    break;

  case 236:
#line 682 "grammar.y"
    { (yyval.decl) = new VarDeclNode(*(yyvsp[-1].ident), (yyvsp[0].init), VarDeclNode::Constant); ;}
    break;

  case 237:
#line 686 "grammar.y"
    { (yyval.init) = new AssignExprNode((yyvsp[0].node)); ;}
    break;

  case 238:
#line 690 "grammar.y"
    { (yyval.init) = new AssignExprNode((yyvsp[0].node)); ;}
    break;

  case 239:
#line 694 "grammar.y"
    { (yyval.stat) = new EmptyStatementNode(); ;}
    break;

  case 240:
#line 698 "grammar.y"
    { (yyval.stat) = new ExprStatementNode((yyvsp[-1].node)); DBG((yyval.stat), (yylsp[-1]), (yylsp[0])); ;}
    break;

  case 241:
#line 699 "grammar.y"
    { (yyval.stat) = new ExprStatementNode((yyvsp[-1].node)); DBG((yyval.stat), (yylsp[-1]), (yylsp[-1])); AUTO_SEMICOLON; ;}
    break;

  case 242:
#line 704 "grammar.y"
    { (yyval.stat) = new IfNode((yyvsp[-2].node), (yyvsp[0].stat), 0); DBG((yyval.stat), (yylsp[-4]), (yylsp[-1])); ;}
    break;

  case 243:
#line 706 "grammar.y"
    { (yyval.stat) = new IfNode((yyvsp[-4].node), (yyvsp[-2].stat), (yyvsp[0].stat)); DBG((yyval.stat), (yylsp[-6]), (yylsp[-3])); ;}
    break;

  case 244:
#line 710 "grammar.y"
    { (yyval.stat) = new DoWhileNode((yyvsp[-4].stat), (yyvsp[-1].node)); DBG((yyval.stat), (yylsp[-5]), (yylsp[-3]));;}
    break;

  case 245:
#line 711 "grammar.y"
    { (yyval.stat) = new WhileNode((yyvsp[-2].node), (yyvsp[0].stat)); DBG((yyval.stat), (yylsp[-4]), (yylsp[-1])); ;}
    break;

  case 246:
#line 713 "grammar.y"
    { (yyval.stat) = new ForNode((yyvsp[-6].node), (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].stat)); DBG((yyval.stat), (yylsp[-8]), (yylsp[-1])); ;}
    break;

  case 247:
#line 715 "grammar.y"
    { (yyval.stat) = new ForNode((yyvsp[-6].vlist), (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].stat)); DBG((yyval.stat), (yylsp[-9]), (yylsp[-1])); ;}
    break;

  case 248:
#line 717 "grammar.y"
    {
                                            Node *n = (yyvsp[-4].node)->nodeInsideAllParens();
                                            if (!n->isLocation())
                                                YYABORT;
                                            (yyval.stat) = new ForInNode(n, (yyvsp[-2].node), (yyvsp[0].stat));
                                            DBG((yyval.stat), (yylsp[-6]), (yylsp[-1]));
                                        ;}
    break;

  case 249:
#line 725 "grammar.y"
    { (yyval.stat) = new ForInNode(*(yyvsp[-4].ident), 0, (yyvsp[-2].node), (yyvsp[0].stat)); DBG((yyval.stat), (yylsp[-7]), (yylsp[-1])); ;}
    break;

  case 250:
#line 727 "grammar.y"
    { (yyval.stat) = new ForInNode(*(yyvsp[-5].ident), (yyvsp[-4].init), (yyvsp[-2].node), (yyvsp[0].stat)); DBG((yyval.stat), (yylsp[-8]), (yylsp[-1])); ;}
    break;

  case 251:
#line 731 "grammar.y"
    { (yyval.node) = 0; ;}
    break;

  case 253:
#line 736 "grammar.y"
    { (yyval.node) = 0; ;}
    break;

  case 255:
#line 741 "grammar.y"
    { (yyval.stat) = new ContinueNode(); DBG((yyval.stat), (yylsp[-1]), (yylsp[0])); ;}
    break;

  case 256:
#line 742 "grammar.y"
    { (yyval.stat) = new ContinueNode(); DBG((yyval.stat), (yylsp[-1]), (yylsp[-1])); AUTO_SEMICOLON; ;}
    break;

  case 257:
#line 743 "grammar.y"
    { (yyval.stat) = new ContinueNode(*(yyvsp[-1].ident)); DBG((yyval.stat), (yylsp[-2]), (yylsp[0])); ;}
    break;

  case 258:
#line 744 "grammar.y"
    { (yyval.stat) = new ContinueNode(*(yyvsp[-1].ident)); DBG((yyval.stat), (yylsp[-2]), (yylsp[-1])); AUTO_SEMICOLON; ;}
    break;

  case 259:
#line 748 "grammar.y"
    { (yyval.stat) = new BreakNode(); DBG((yyval.stat), (yylsp[-1]), (yylsp[0])); ;}
    break;

  case 260:
#line 749 "grammar.y"
    { (yyval.stat) = new BreakNode(); DBG((yyval.stat), (yylsp[-1]), (yylsp[-1])); AUTO_SEMICOLON; ;}
    break;

  case 261:
#line 750 "grammar.y"
    { (yyval.stat) = new BreakNode(*(yyvsp[-1].ident)); DBG((yyval.stat), (yylsp[-2]), (yylsp[0])); ;}
    break;

  case 262:
#line 751 "grammar.y"
    { (yyval.stat) = new BreakNode(*(yyvsp[-1].ident)); DBG((yyval.stat), (yylsp[-2]), (yylsp[-1])); AUTO_SEMICOLON; ;}
    break;

  case 263:
#line 755 "grammar.y"
    { (yyval.stat) = new ReturnNode(0); DBG((yyval.stat), (yylsp[-1]), (yylsp[0])); ;}
    break;

  case 264:
#line 756 "grammar.y"
    { (yyval.stat) = new ReturnNode(0); DBG((yyval.stat), (yylsp[-1]), (yylsp[-1])); AUTO_SEMICOLON; ;}
    break;

  case 265:
#line 757 "grammar.y"
    { (yyval.stat) = new ReturnNode((yyvsp[-1].node)); DBG((yyval.stat), (yylsp[-2]), (yylsp[0])); ;}
    break;

  case 266:
#line 758 "grammar.y"
    { (yyval.stat) = new ReturnNode((yyvsp[-1].node)); DBG((yyval.stat), (yylsp[-2]), (yylsp[-1])); AUTO_SEMICOLON; ;}
    break;

  case 267:
#line 762 "grammar.y"
    { (yyval.stat) = new WithNode((yyvsp[-2].node), (yyvsp[0].stat)); DBG((yyval.stat), (yylsp[-4]), (yylsp[-1])); ;}
    break;

  case 268:
#line 766 "grammar.y"
    { (yyval.stat) = new SwitchNode((yyvsp[-2].node), (yyvsp[0].cblk)); DBG((yyval.stat), (yylsp[-4]), (yylsp[-1])); ;}
    break;

  case 269:
#line 770 "grammar.y"
    { (yyval.cblk) = new CaseBlockNode((yyvsp[-1].clist), 0, 0); ;}
    break;

  case 270:
#line 772 "grammar.y"
    { (yyval.cblk) = new CaseBlockNode((yyvsp[-3].clist), (yyvsp[-2].ccl), (yyvsp[-1].clist)); ;}
    break;

  case 271:
#line 776 "grammar.y"
    { (yyval.clist) = 0; ;}
    break;

  case 273:
#line 781 "grammar.y"
    { (yyval.clist) = new ClauseListNode((yyvsp[0].ccl)); ;}
    break;

  case 274:
#line 782 "grammar.y"
    { (yyval.clist) = new ClauseListNode((yyvsp[-1].clist), (yyvsp[0].ccl)); ;}
    break;

  case 275:
#line 786 "grammar.y"
    { (yyval.ccl) = new CaseClauseNode((yyvsp[-1].node)); ;}
    break;

  case 276:
#line 787 "grammar.y"
    { (yyval.ccl) = new CaseClauseNode((yyvsp[-2].node), (yyvsp[0].srcs)); ;}
    break;

  case 277:
#line 791 "grammar.y"
    { (yyval.ccl) = new CaseClauseNode(0); ;}
    break;

  case 278:
#line 792 "grammar.y"
    { (yyval.ccl) = new CaseClauseNode(0, (yyvsp[0].srcs)); ;}
    break;

  case 279:
#line 796 "grammar.y"
    { (yyvsp[0].stat)->pushLabel(*(yyvsp[-2].ident)); (yyval.stat) = new LabelNode(*(yyvsp[-2].ident), (yyvsp[0].stat)); ;}
    break;

  case 280:
#line 800 "grammar.y"
    { (yyval.stat) = new ThrowNode((yyvsp[-1].node)); DBG((yyval.stat), (yylsp[-2]), (yylsp[0])); ;}
    break;

  case 281:
#line 801 "grammar.y"
    { (yyval.stat) = new ThrowNode((yyvsp[-1].node)); DBG((yyval.stat), (yylsp[-2]), (yylsp[-1])); AUTO_SEMICOLON; ;}
    break;

  case 282:
#line 805 "grammar.y"
    { (yyval.stat) = new TryNode((yyvsp[-2].stat), Identifier::null(), 0, (yyvsp[0].stat)); DBG((yyval.stat), (yylsp[-3]), (yylsp[-2])); ;}
    break;

  case 283:
#line 806 "grammar.y"
    { (yyval.stat) = new TryNode((yyvsp[-5].stat), *(yyvsp[-2].ident), (yyvsp[0].stat), 0); DBG((yyval.stat), (yylsp[-6]), (yylsp[-5])); ;}
    break;

  case 284:
#line 808 "grammar.y"
    { (yyval.stat) = new TryNode((yyvsp[-7].stat), *(yyvsp[-4].ident), (yyvsp[-2].stat), (yyvsp[0].stat)); DBG((yyval.stat), (yylsp[-8]), (yylsp[-7])); ;}
    break;

  case 285:
#line 812 "grammar.y"
    { (yyval.stat) = new EmptyStatementNode(); DBG((yyval.stat), (yylsp[-1]), (yylsp[0])); ;}
    break;

  case 286:
#line 813 "grammar.y"
    { (yyval.stat) = new EmptyStatementNode(); DBG((yyval.stat), (yylsp[-1]), (yylsp[-1])); AUTO_SEMICOLON; ;}
    break;

  case 287:
#line 817 "grammar.y"
    { (yyval.pkgi) = new PackageIdentNode(*(yyvsp[0].ident)); ;}
    break;

  case 288:
#line 818 "grammar.y"
    { (yyval.pkgi) = new PackageIdentNode((yyvsp[-2].pkgi), *(yyvsp[0].ident)); ;}
    break;

  case 289:
#line 822 "grammar.y"
    { (yyval.pkgn) = new PackageNameNode(*(yyvsp[0].ustr)); ;}
    break;

  case 290:
#line 823 "grammar.y"
    { (yyval.pkgn) = new PackageNameNode((yyvsp[0].pkgi)); ;}
    break;

  case 291:
#line 827 "grammar.y"
    { (yyval.stat) = new ImportStatement((yyvsp[-1].pkgn)); DBG((yyval.stat), (yylsp[-2]), (yylsp[0])); ;}
    break;

  case 292:
#line 828 "grammar.y"
    { (yyval.stat) = new ImportStatement((yyvsp[-1].pkgn)); DBG((yyval.stat), (yylsp[-2]), (yylsp[-1])); AUTO_SEMICOLON; ;}
    break;

  case 293:
#line 832 "grammar.y"
    { (yyval.func) = new FuncDeclNode(*(yyvsp[-3].ident), (yyvsp[0].body)); ;}
    break;

  case 294:
#line 834 "grammar.y"
    { (yyval.func) = new FuncDeclNode(*(yyvsp[-4].ident), (yyvsp[-2].param), (yyvsp[0].body)); ;}
    break;

  case 295:
#line 838 "grammar.y"
    { (yyval.funcExpr) = new FuncExprNode(Identifier::null(), (yyvsp[0].body)); ;}
    break;

  case 296:
#line 840 "grammar.y"
    { (yyval.funcExpr) = new FuncExprNode(Identifier::null(), (yyvsp[0].body), (yyvsp[-2].param)); ;}
    break;

  case 297:
#line 841 "grammar.y"
    { (yyval.funcExpr) = new FuncExprNode(*(yyvsp[-3].ident), (yyvsp[0].body)); ;}
    break;

  case 298:
#line 843 "grammar.y"
    { (yyval.funcExpr) = new FuncExprNode(*(yyvsp[-4].ident), (yyvsp[0].body), (yyvsp[-2].param)); ;}
    break;

  case 299:
#line 847 "grammar.y"
    { (yyval.param) = new ParameterNode(*(yyvsp[0].ident)); ;}
    break;

  case 300:
#line 848 "grammar.y"
    { (yyval.param) = new ParameterNode((yyvsp[-2].param), *(yyvsp[0].ident)); ;}
    break;

  case 301:
#line 852 "grammar.y"
    { (yyval.body) = new FunctionBodyNode(0); DBG((yyval.body), (yylsp[-1]), (yylsp[0])); ;}
    break;

  case 302:
#line 853 "grammar.y"
    { (yyval.body) = new FunctionBodyNode((yyvsp[-1].srcs)); DBG((yyval.body), (yylsp[-2]), (yylsp[0])); ;}
    break;

  case 303:
#line 857 "grammar.y"
    { Parser::accept(new ProgramNode(0)); ;}
    break;

  case 304:
#line 858 "grammar.y"
    { Parser::accept(new ProgramNode((yyvsp[0].srcs))); ;}
    break;

  case 305:
#line 862 "grammar.y"
    { (yyval.srcs) = new SourceElementsNode((yyvsp[0].stat)); ;}
    break;

  case 306:
#line 863 "grammar.y"
    { (yyval.srcs) = new SourceElementsNode((yyvsp[-1].srcs), (yyvsp[0].stat)); ;}
    break;

  case 307:
#line 867 "grammar.y"
    { (yyval.stat) = (yyvsp[0].func); ;}
    break;

  case 308:
#line 868 "grammar.y"
    { (yyval.stat) = (yyvsp[0].stat); ;}
    break;


      default: break;
    }

/* Line 1126 of yacc.c.  */
#line 3265 "grammar.tab.c"

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


#line 871 "grammar.y"


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

