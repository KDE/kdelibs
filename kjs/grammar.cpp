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
     ELSE = 279,
     IN = 280,
     INSTANCEOF = 281,
     TYPEOF = 282,
     SWITCH = 283,
     WITH = 284,
     RESERVED = 285,
     THROW = 286,
     TRY = 287,
     CATCH = 288,
     FINALLY = 289,
     EQEQ = 290,
     NE = 291,
     STREQ = 292,
     STRNEQ = 293,
     LE = 294,
     GE = 295,
     OR = 296,
     AND = 297,
     PLUSPLUS = 298,
     MINUSMINUS = 299,
     LSHIFT = 300,
     RSHIFT = 301,
     URSHIFT = 302,
     PLUSEQUAL = 303,
     MINUSEQUAL = 304,
     MULTEQUAL = 305,
     DIVEQUAL = 306,
     LSHIFTEQUAL = 307,
     RSHIFTEQUAL = 308,
     URSHIFTEQUAL = 309,
     ANDEQUAL = 310,
     MODEQUAL = 311,
     XOREQUAL = 312,
     OREQUAL = 313,
     IDENT = 314,
     AUTOPLUSPLUS = 315,
     AUTOMINUSMINUS = 316
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
#define ELSE 279
#define IN 280
#define INSTANCEOF 281
#define TYPEOF 282
#define SWITCH 283
#define WITH 284
#define RESERVED 285
#define THROW 286
#define TRY 287
#define CATCH 288
#define FINALLY 289
#define EQEQ 290
#define NE 291
#define STREQ 292
#define STRNEQ 293
#define LE 294
#define GE 295
#define OR 296
#define AND 297
#define PLUSPLUS 298
#define MINUSMINUS 299
#define LSHIFT 300
#define RSHIFT 301
#define URSHIFT 302
#define PLUSEQUAL 303
#define MINUSEQUAL 304
#define MULTEQUAL 305
#define DIVEQUAL 306
#define LSHIFTEQUAL 307
#define RSHIFTEQUAL 308
#define URSHIFTEQUAL 309
#define ANDEQUAL 310
#define MODEQUAL 311
#define XOREQUAL 312
#define OREQUAL 313
#define IDENT 314
#define AUTOPLUSPLUS 315
#define AUTOMINUSMINUS 316




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

/* default values for bison */
#define YYDEBUG 0
#define YYMAXDEPTH 0
#define YYERROR_VERBOSE
#define DBG(l, s, e) { l->setLoc(s.first_line, e.last_line, Parser::source); } // location

extern int yylex();
static int yyerror (const char *);
static bool automatic();

using namespace KJS;



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
#line 50 "grammar.y"
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
  FunctionBodyNode    *prog;
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
  CatchNode           *cnode;
  FinallyNode         *fnode;
} YYSTYPE;
/* Line 196 of yacc.c.  */
#line 293 "grammar.tab.c"
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
#line 317 "grammar.tab.c"

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
#define YYFINAL  191
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1387

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  86
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  69
/* YYNRULES -- Number of rules. */
#define YYNRULES  207
/* YYNRULES -- Number of states. */
#define YYNSTATES  373

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   316

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    75,     2,     2,     2,    77,    80,     2,
      63,    64,    76,    72,    67,    73,    71,    62,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    70,    85,
      78,    84,    79,    83,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    68,     2,    69,    81,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    65,    82,    66,    74,     2,     2,     2,
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
      55,    56,    57,    58,    59,    60,    61
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short int yyprhs[] =
{
       0,     0,     3,     5,     7,     9,    11,    13,    15,    17,
      19,    21,    23,    25,    29,    32,    36,    41,    45,    49,
      55,    58,    63,    64,    66,    68,    71,    75,    81,    83,
      85,    87,    89,    91,    96,   100,   104,   106,   109,   112,
     115,   120,   124,   127,   131,   133,   137,   139,   141,   143,
     146,   149,   151,   154,   157,   160,   163,   166,   169,   172,
     175,   178,   181,   184,   186,   190,   194,   198,   200,   204,
     208,   210,   214,   218,   222,   224,   228,   232,   236,   240,
     244,   248,   250,   254,   258,   262,   266,   268,   272,   274,
     278,   280,   284,   286,   290,   292,   296,   298,   304,   306,
     310,   312,   314,   316,   318,   320,   322,   324,   326,   328,
     330,   332,   334,   336,   340,   342,   344,   346,   348,   350,
     352,   354,   356,   358,   360,   362,   364,   366,   368,   370,
     373,   377,   379,   382,   386,   390,   392,   396,   398,   401,
     405,   409,   411,   415,   417,   420,   423,   425,   428,   431,
     437,   445,   452,   458,   468,   479,   487,   496,   506,   507,
     509,   512,   515,   519,   523,   526,   529,   533,   537,   540,
     543,   547,   551,   557,   563,   567,   573,   574,   576,   578,
     581,   585,   590,   593,   597,   601,   605,   609,   613,   617,
     622,   628,   631,   633,   636,   642,   649,   654,   660,   662,
     666,   669,   673,   674,   676,   678,   681,   683
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short int yyrhs[] =
{
     152,     0,    -1,     3,    -1,     4,    -1,     5,    -1,     7,
      -1,     6,    -1,    62,    -1,    51,    -1,    21,    -1,    59,
      -1,    87,    -1,    89,    -1,    63,   116,    64,    -1,    65,
      66,    -1,    65,    93,    66,    -1,    65,    93,    67,    66,
      -1,    68,    91,    69,    -1,    68,    90,    69,    -1,    68,
      90,    67,    91,    69,    -1,    91,   114,    -1,    90,    67,
      91,   114,    -1,    -1,    92,    -1,    67,    -1,    92,    67,
      -1,    94,    70,   114,    -1,    93,    67,    94,    70,   114,
      -1,    59,    -1,     6,    -1,     7,    -1,    88,    -1,   149,
      -1,    95,    68,   116,    69,    -1,    95,    71,    59,    -1,
      12,    95,    98,    -1,    95,    -1,    12,    96,    -1,    95,
      98,    -1,    97,    98,    -1,    97,    68,   116,    69,    -1,
      97,    71,    59,    -1,    63,    64,    -1,    63,    99,    64,
      -1,   114,    -1,    99,    67,   114,    -1,    96,    -1,    97,
      -1,   100,    -1,   100,    43,    -1,   100,    44,    -1,   101,
      -1,    19,   102,    -1,    18,   102,    -1,    27,   102,    -1,
      43,   102,    -1,    60,   102,    -1,    44,   102,    -1,    61,
     102,    -1,    72,   102,    -1,    73,   102,    -1,    74,   102,
      -1,    75,   102,    -1,   102,    -1,   103,    76,   102,    -1,
     103,    62,   102,    -1,   103,    77,   102,    -1,   103,    -1,
     104,    72,   103,    -1,   104,    73,   103,    -1,   104,    -1,
     105,    45,   104,    -1,   105,    46,   104,    -1,   105,    47,
     104,    -1,   105,    -1,   106,    78,   105,    -1,   106,    79,
     105,    -1,   106,    39,   105,    -1,   106,    40,   105,    -1,
     106,    26,   105,    -1,   106,    25,   105,    -1,   106,    -1,
     107,    35,   106,    -1,   107,    36,   106,    -1,   107,    37,
     106,    -1,   107,    38,   106,    -1,   107,    -1,   108,    80,
     107,    -1,   108,    -1,   109,    81,   108,    -1,   109,    -1,
     110,    82,   109,    -1,   110,    -1,   111,    42,   110,    -1,
     111,    -1,   112,    41,   111,    -1,   112,    -1,   112,    83,
     114,    70,   114,    -1,   113,    -1,   100,   115,   114,    -1,
      84,    -1,    48,    -1,    49,    -1,    50,    -1,    51,    -1,
      52,    -1,    53,    -1,    54,    -1,    55,    -1,    57,    -1,
      58,    -1,    56,    -1,   114,    -1,   116,    67,   114,    -1,
     118,    -1,   120,    -1,   123,    -1,   127,    -1,   128,    -1,
     129,    -1,   130,    -1,   132,    -1,   133,    -1,   134,    -1,
     135,    -1,   136,    -1,   142,    -1,   143,    -1,   144,    -1,
      65,    66,    -1,    65,   153,    66,    -1,   117,    -1,   119,
     117,    -1,    13,   121,    85,    -1,    13,   121,     1,    -1,
     122,    -1,   121,    67,   122,    -1,    59,    -1,    59,   126,
      -1,    14,   124,    85,    -1,    14,   124,     1,    -1,   125,
      -1,   124,    67,   122,    -1,    59,    -1,    59,   126,    -1,
      84,   114,    -1,    85,    -1,   116,    85,    -1,   116,     1,
      -1,    20,    63,   116,    64,   117,    -1,    20,    63,   116,
      64,   117,    24,   117,    -1,    22,   117,    23,    63,   116,
      64,    -1,    23,    63,   116,    64,   117,    -1,    11,    63,
     131,    85,   131,    85,   131,    64,   117,    -1,    11,    63,
      13,   121,    85,   131,    85,   131,    64,   117,    -1,    11,
      63,   100,    25,   116,    64,   117,    -1,    11,    63,    13,
      59,    25,   116,    64,   117,    -1,    11,    63,    13,    59,
     126,    25,   116,    64,   117,    -1,    -1,   116,    -1,    15,
      85,    -1,    15,     1,    -1,    15,    59,    85,    -1,    15,
      59,     1,    -1,     8,    85,    -1,     8,     1,    -1,     8,
      59,    85,    -1,     8,    59,     1,    -1,    17,    85,    -1,
      17,     1,    -1,    17,   116,    85,    -1,    17,   116,     1,
      -1,    29,    63,   116,    64,   117,    -1,    28,    63,   116,
      64,   137,    -1,    65,   138,    66,    -1,    65,   138,   141,
     138,    66,    -1,    -1,   139,    -1,   140,    -1,   139,   140,
      -1,     9,   116,    70,    -1,     9,   116,    70,   119,    -1,
      10,    70,    -1,    10,    70,   119,    -1,    59,    70,   117,
      -1,    31,   116,    85,    -1,    31,   116,     1,    -1,    32,
     118,   145,    -1,    32,   118,   146,    -1,    32,   118,   145,
     146,    -1,    33,    63,    59,    64,   118,    -1,    34,   118,
      -1,   148,    -1,    18,   148,    -1,    16,    59,    63,    64,
     151,    -1,    16,    59,    63,   150,    64,   151,    -1,    16,
      63,    64,   151,    -1,    16,    63,   150,    64,   151,    -1,
      59,    -1,   150,    67,    59,    -1,    65,    66,    -1,    65,
     153,    66,    -1,    -1,   153,    -1,   154,    -1,   153,   154,
      -1,   117,    -1,   147,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short int yyrline[] =
{
       0,   165,   165,   166,   167,   168,   169,   170,   173,   180,
     181,   182,   183,   184,   185,   186,   187,   191,   192,   193,
     197,   198,   203,   204,   208,   209,   213,   214,   219,   220,
     221,   225,   226,   227,   228,   229,   233,   234,   238,   239,
     240,   241,   245,   246,   250,   251,   255,   256,   260,   261,
     262,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   281,   282,   283,   284,   288,   289,   290,
     294,   295,   296,   297,   301,   302,   304,   306,   308,   310,
     312,   317,   318,   319,   320,   321,   325,   326,   330,   331,
     335,   336,   340,   341,   346,   347,   352,   353,   358,   359,
     364,   365,   366,   367,   368,   369,   370,   371,   372,   373,
     374,   375,   379,   380,   384,   385,   386,   387,   388,   389,
     390,   391,   392,   393,   394,   395,   396,   397,   398,   402,
     403,   407,   408,   412,   414,   424,   425,   430,   431,   435,
     437,   447,   448,   453,   454,   458,   462,   466,   468,   476,
     477,   482,   483,   484,   487,   490,   493,   496,   502,   503,
     507,   508,   512,   513,   520,   521,   525,   526,   534,   535,
     539,   540,   548,   553,   558,   559,   564,   565,   569,   570,
     574,   575,   579,   580,   584,   589,   590,   597,   598,   599,
     603,   608,   612,   614,   618,   619,   624,   625,   631,   632,
     636,   638,   643,   646,   651,   652,   656,   657
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
  "THIS", "DO", "WHILE", "ELSE", "IN", "INSTANCEOF", "TYPEOF", "SWITCH",
  "WITH", "RESERVED", "THROW", "TRY", "CATCH", "FINALLY", "EQEQ", "NE",
  "STREQ", "STRNEQ", "LE", "GE", "OR", "AND", "PLUSPLUS", "MINUSMINUS",
  "LSHIFT", "RSHIFT", "URSHIFT", "PLUSEQUAL", "MINUSEQUAL", "MULTEQUAL",
  "DIVEQUAL", "LSHIFTEQUAL", "RSHIFTEQUAL", "URSHIFTEQUAL", "ANDEQUAL",
  "MODEQUAL", "XOREQUAL", "OREQUAL", "IDENT", "AUTOPLUSPLUS",
  "AUTOMINUSMINUS", "'/'", "'('", "')'", "'{'", "'}'", "','", "'['", "']'",
  "':'", "'.'", "'+'", "'-'", "'~'", "'!'", "'*'", "'%'", "'<'", "'>'",
  "'&'", "'^'", "'|'", "'?'", "'='", "';'", "$accept", "Literal",
  "PrimaryExpr", "ArrayLiteral", "ElementList", "ElisionOpt", "Elision",
  "PropertyNameAndValueList", "PropertyName", "MemberExpr", "NewExpr",
  "CallExpr", "Arguments", "ArgumentList", "LeftHandSideExpr",
  "PostfixExpr", "UnaryExpr", "MultiplicativeExpr", "AdditiveExpr",
  "ShiftExpr", "RelationalExpr", "EqualityExpr", "BitwiseANDExpr",
  "BitwiseXORExpr", "BitwiseORExpr", "LogicalANDExpr", "LogicalORExpr",
  "ConditionalExpr", "AssignmentExpr", "AssignmentOperator", "Expr",
  "Statement", "Block", "StatementList", "VariableStatement",
  "VariableDeclarationList", "VariableDeclaration", "ConstStatement",
  "ConstDeclarationList", "ConstDeclaration", "Initializer",
  "EmptyStatement", "ExprStatement", "IfStatement", "IterationStatement",
  "ExprOpt", "ContinueStatement", "BreakStatement", "ReturnStatement",
  "WithStatement", "SwitchStatement", "CaseBlock", "CaseClausesOpt",
  "CaseClauses", "CaseClause", "DefaultClause", "LabelledStatement",
  "ThrowStatement", "TryStatement", "Catch", "Finally",
  "FunctionDeclaration", "FunctionDeclarationInternal", "FunctionExpr",
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
     315,   316,    47,    40,    41,   123,   125,    44,    91,    93,
      58,    46,    43,    45,   126,    33,    42,    37,    60,    62,
      38,    94,   124,    63,    61,    59
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    86,    87,    87,    87,    87,    87,    87,    87,    88,
      88,    88,    88,    88,    88,    88,    88,    89,    89,    89,
      90,    90,    91,    91,    92,    92,    93,    93,    94,    94,
      94,    95,    95,    95,    95,    95,    96,    96,    97,    97,
      97,    97,    98,    98,    99,    99,   100,   100,   101,   101,
     101,   102,   102,   102,   102,   102,   102,   102,   102,   102,
     102,   102,   102,   103,   103,   103,   103,   104,   104,   104,
     105,   105,   105,   105,   106,   106,   106,   106,   106,   106,
     106,   107,   107,   107,   107,   107,   108,   108,   109,   109,
     110,   110,   111,   111,   112,   112,   113,   113,   114,   114,
     115,   115,   115,   115,   115,   115,   115,   115,   115,   115,
     115,   115,   116,   116,   117,   117,   117,   117,   117,   117,
     117,   117,   117,   117,   117,   117,   117,   117,   117,   118,
     118,   119,   119,   120,   120,   121,   121,   122,   122,   123,
     123,   124,   124,   125,   125,   126,   127,   128,   128,   129,
     129,   130,   130,   130,   130,   130,   130,   130,   131,   131,
     132,   132,   132,   132,   133,   133,   133,   133,   134,   134,
     134,   134,   135,   136,   137,   137,   138,   138,   139,   139,
     140,   140,   141,   141,   142,   143,   143,   144,   144,   144,
     145,   146,   147,   147,   148,   148,   149,   149,   150,   150,
     151,   151,   152,   152,   153,   153,   154,   154
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     3,     2,     3,     4,     3,     3,     5,
       2,     4,     0,     1,     1,     2,     3,     5,     1,     1,
       1,     1,     1,     4,     3,     3,     1,     2,     2,     2,
       4,     3,     2,     3,     1,     3,     1,     1,     1,     2,
       2,     1,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     1,     3,     3,     3,     1,     3,     3,
       1,     3,     3,     3,     1,     3,     3,     3,     3,     3,
       3,     1,     3,     3,     3,     3,     1,     3,     1,     3,
       1,     3,     1,     3,     1,     3,     1,     5,     1,     3,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     3,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     2,
       3,     1,     2,     3,     3,     1,     3,     1,     2,     3,
       3,     1,     3,     1,     2,     2,     1,     2,     2,     5,
       7,     6,     5,     9,    10,     7,     8,     9,     0,     1,
       2,     2,     3,     3,     2,     2,     3,     3,     2,     2,
       3,     3,     5,     5,     3,     5,     0,     1,     1,     2,
       3,     4,     2,     3,     3,     3,     3,     3,     3,     4,
       5,     2,     1,     2,     5,     6,     4,     5,     1,     3,
       2,     3,     0,     1,     1,     2,     1,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
     202,     2,     3,     4,     6,     5,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     9,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     8,    10,     0,
       0,     7,     0,     0,    22,     0,     0,     0,     0,   146,
      11,    31,    12,    36,    46,    47,    48,    51,    63,    67,
      70,    74,    81,    86,    88,    90,    92,    94,    96,    98,
     112,     0,   206,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,   207,   192,
      32,     0,   203,   204,   165,     0,   164,   158,     0,    10,
       0,    36,    37,   137,     0,   135,   143,     0,   141,   161,
       0,   160,     0,     0,   169,     0,   168,     0,    48,    53,
     193,    52,     0,     0,     0,    54,     0,     0,     0,     0,
       0,    55,    57,     0,    56,    58,     0,     6,     5,    10,
      14,     0,     0,     0,    24,     0,     0,    23,    59,    60,
      61,    62,     0,     0,     0,    38,     0,     0,    39,    49,
      50,   101,   102,   103,   104,   105,   106,   107,   108,   111,
     109,   110,   100,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   148,     0,
     147,     1,   205,   167,   166,     0,    48,   159,     0,    29,
      30,    28,    14,    35,     0,   138,   134,     0,   133,   144,
     140,     0,   139,   163,   162,     0,   198,     0,     0,   171,
     170,     0,     0,     0,     0,     0,   186,   185,   129,     0,
       0,   187,   188,   184,    13,    15,     0,     0,   130,    22,
      18,    17,    20,    25,    42,     0,    44,     0,    34,     0,
      41,    99,    65,    64,    66,    68,    69,    71,    72,    73,
      80,    79,    77,    78,    75,    76,    82,    83,    84,    85,
      87,    89,    91,    93,    95,     0,   113,   137,     0,     0,
     158,   145,   136,   142,     0,     0,     0,   196,     0,     0,
       0,     0,     0,     0,     0,     0,   191,   189,    16,     0,
      26,     0,    43,     0,    33,    40,     0,     0,   138,   158,
       0,     0,   194,     0,   200,     0,   197,   199,   149,     0,
     152,   176,   173,   172,     0,     0,    19,    21,    45,    97,
       0,     0,     0,     0,   158,   195,   201,     0,   151,     0,
       0,   177,   178,     0,    27,     0,     0,   158,   155,     0,
     150,     0,     0,   174,   176,   179,   190,   156,     0,     0,
       0,   180,   182,     0,   157,     0,   153,   131,   181,   183,
     175,   154,   132
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short int yydefgoto[] =
{
      -1,    40,    41,    42,   135,   136,   137,   131,   132,    43,
      44,    45,   145,   245,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,   163,
      61,    62,    63,   368,    64,    94,    95,    65,    97,    98,
     205,    66,    67,    68,    69,   198,    70,    71,    72,    73,
      74,   322,   340,   341,   342,   354,    75,    76,    77,   231,
     232,    78,    79,    80,   218,   287,    81,   133,    83
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -233
static const short int yypact[] =
{
     747,  -233,  -233,  -233,  -233,  -233,     5,   -44,  1280,   -33,
     -25,     6,    34,    67,  1188,  1261,     3,  -233,   822,    45,
    1261,    54,    60,  1261,    -7,  1261,  1261,  -233,    55,  1261,
    1261,  -233,  1261,   372,    31,  1261,  1261,  1261,  1261,  -233,
    -233,  -233,  -233,   -20,  -233,   -14,   876,  -233,  -233,   -35,
     139,   154,   131,     9,    51,   -21,    85,    96,    -3,  -233,
    -233,    11,  -233,  -233,  -233,  -233,  -233,  -233,  -233,  -233,
    -233,  -233,  -233,  -233,  -233,  -233,  -233,  -233,  -233,  -233,
    -233,   233,   747,  -233,  -233,    16,  -233,   896,   109,  -233,
      33,   -20,  -233,   150,    28,  -233,   150,    36,  -233,  -233,
      27,  -233,   173,    23,  -233,  1261,  -233,    49,   172,  -233,
    -233,  -233,  1261,   237,  1261,  -233,  1261,  1261,    52,   447,
     194,  -233,  -233,   822,  -233,  -233,    86,   191,   195,    55,
     235,   163,   198,   522,  -233,   127,   969,   204,  -233,  -233,
    -233,  -233,  1042,  1261,   214,  -233,  1261,   218,  -233,  -233,
    -233,  -233,  -233,  -233,  -233,  -233,  -233,  -233,  -233,  -233,
    -233,  -233,  -233,  1261,  1261,  1261,  1261,  1261,  1261,  1261,
    1261,  1261,  1261,  1261,  1261,  1261,  1261,  1261,  1261,  1261,
    1261,  1261,  1261,  1261,  1261,  1261,  1261,  1261,  -233,  1261,
    -233,  -233,  -233,  -233,  -233,   219,  1303,   212,   197,  -233,
    -233,  -233,  -233,  -233,  1261,  -233,  -233,   -33,  -233,  -233,
    -233,   -33,  -233,  -233,  -233,    25,  -233,   215,    87,  -233,
    -233,    91,   220,   102,   110,   112,  -233,  -233,  -233,   221,
      -7,   251,  -233,  -233,  -233,  -233,    56,  1261,  -233,    31,
    -233,  -233,  -233,  -233,  -233,   116,  -233,   152,  -233,   153,
    -233,  -233,  -233,  -233,  -233,   -35,   -35,   139,   139,   139,
     154,   154,   154,   154,   154,   154,   131,   131,   131,   131,
       9,    51,   -21,    85,    96,   216,  -233,    -9,   -49,  1261,
    1261,  -233,  -233,  -233,   215,   118,   597,  -233,   215,   228,
     822,  1261,   822,   224,   822,   231,  -233,  -233,  -233,   222,
    -233,  1115,  -233,  1261,  -233,  -233,  1261,  1261,   266,  1261,
     120,   213,  -233,   215,  -233,   672,  -233,  -233,   278,   124,
    -233,   294,  -233,  -233,   240,  1261,  -233,  -233,  -233,  -233,
     125,  1261,   223,   822,  1261,  -233,  -233,   822,  -233,  1261,
       1,   294,  -233,    -7,  -233,   822,   126,  1261,  -233,   241,
    -233,   128,   236,  -233,   294,  -233,  -233,  -233,   822,   243,
     822,   822,   822,   245,  -233,   822,  -233,  -233,   822,   822,
    -233,  -233,  -233
};

/* YYPGOTO[NTERM-NUM].  */
static const short int yypgoto[] =
{
    -233,  -233,  -233,  -233,  -233,    73,  -233,  -233,    77,   306,
     308,  -233,   -22,  -233,   188,  -233,    -5,    64,    35,   -28,
     -16,   135,   140,   134,   137,   142,  -233,  -233,  -128,  -233,
     -10,   -18,   -23,   -37,  -233,   136,   -87,  -233,  -233,  -233,
     -91,  -233,  -233,  -233,  -233,  -166,  -233,  -233,  -233,  -233,
    -233,  -233,   -24,  -233,   -15,  -233,  -233,  -233,  -233,  -233,
     101,  -233,   319,  -233,   119,  -232,  -233,     2,   -78
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -130
static const short int yytable[] =
{
     113,   120,    82,   107,   192,   209,    84,    99,   242,   109,
     111,   352,   188,   118,   246,   115,   307,   193,   207,    87,
     121,   122,   126,   148,   124,   125,    93,   164,   213,   206,
     138,   139,   140,   141,    96,   251,   309,   210,   186,   199,
     200,   165,   166,   142,   178,   179,   180,   181,   143,   142,
     219,   144,   312,   226,   146,   192,   316,   147,   119,   275,
     183,   276,   199,   200,    85,   100,   112,   353,   104,   203,
       1,     2,     3,     4,     5,   204,   281,   197,   189,     8,
     187,   335,   216,    88,   216,   105,    15,   217,    17,   284,
      86,   101,   201,   102,    20,   207,   190,   103,   134,   202,
     109,   194,   221,   211,   223,   233,   224,   225,   114,   300,
      25,    26,   214,   208,   311,   201,   189,   116,    27,   189,
     282,   212,   298,   117,   283,   123,    89,    29,    30,    31,
      32,   182,    90,   247,   220,    34,   249,   227,   185,    35,
      36,    37,    38,   332,   260,   261,   262,   263,   264,   265,
     234,   288,   106,   189,   289,   290,   172,   173,   189,   252,
     253,   254,   266,   267,   268,   269,   292,   184,   349,   189,
     174,   175,   103,   327,   293,   328,   294,   189,   329,   189,
     302,   359,   313,   303,   333,   289,   308,   189,   338,   345,
     358,   189,   189,   189,   239,   189,   240,   344,   361,   169,
     170,   171,   108,   108,   257,   258,   259,   296,   108,   176,
     177,   167,   168,   108,   108,   149,   150,   108,   108,   189,
     189,   304,   305,   108,   108,   108,   108,   229,   230,   235,
     236,   255,   256,   191,   204,  -129,   215,   192,  -129,  -129,
    -129,  -129,  -129,  -129,  -129,  -129,  -129,  -129,  -129,  -129,
    -129,  -129,  -129,  -129,  -129,  -129,  -129,  -129,  -129,  -129,
     222,   -29,  -129,  -129,  -129,   -30,  -129,  -129,   237,   310,
     197,   243,   318,   248,   320,   196,   323,   250,   277,   189,
     286,   319,   280,   291,   295,   230,   306,   317,   315,   321,
     324,   331,   325,   108,  -129,  -129,  -129,   330,   334,   197,
    -129,  -129,   337,   339,   343,   360,   362,   365,   347,  -129,
    -129,   370,   301,   299,    91,   348,    92,   270,   272,   350,
     356,   346,   273,   271,   197,   369,   355,   357,   274,   351,
     363,   278,   297,   110,   285,     0,     0,   197,     0,     0,
     364,     0,   366,   367,   367,     0,     0,   371,     0,     0,
     372,   372,   108,   108,   108,   108,   108,   108,   108,   108,
     108,   108,   108,   108,   108,   108,   108,   108,   108,   108,
     108,   108,   108,   108,   108,     1,     2,     3,   127,   128,
       6,     0,     0,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,     0,     0,     0,    20,
      21,    22,     0,    23,    24,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    25,    26,     0,     0,     0,
       0,     0,     0,    27,     0,     0,     0,     0,     0,     0,
       0,   129,    29,    30,    31,    32,     0,    33,   130,     0,
      34,     0,     0,     0,    35,    36,    37,    38,     0,     0,
       1,     2,     3,     4,     5,     6,     0,    39,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,     0,     0,     0,    20,    21,    22,     0,    23,    24,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      25,    26,     0,     0,     0,     0,     0,     0,    27,     0,
       0,     0,     0,     0,     0,     0,    28,    29,    30,    31,
      32,     0,    33,   228,     0,    34,     0,     0,     0,    35,
      36,    37,    38,     0,     0,     1,     2,     3,     4,     5,
       6,     0,    39,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,     0,     0,     0,    20,
      21,    22,     0,    23,    24,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    25,    26,     0,     0,     0,
       0,     0,     0,    27,     0,     0,     0,     0,     0,     0,
       0,    28,    29,    30,    31,    32,     0,    33,   238,     0,
      34,     0,     0,     0,    35,    36,    37,    38,     0,     0,
       1,     2,     3,     4,     5,     6,     0,    39,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,     0,     0,     0,    20,    21,    22,     0,    23,    24,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      25,    26,     0,     0,     0,     0,     0,     0,    27,     0,
       0,     0,     0,     0,     0,     0,    28,    29,    30,    31,
      32,     0,    33,   314,     0,    34,     0,     0,     0,    35,
      36,    37,    38,     0,     0,     1,     2,     3,     4,     5,
       6,     0,    39,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,     0,     0,     0,    20,
      21,    22,     0,    23,    24,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    25,    26,     0,     0,     0,
       0,     0,     0,    27,     0,     0,     0,     0,     0,     0,
       0,    28,    29,    30,    31,    32,     0,    33,   336,     0,
      34,     0,     0,     0,    35,    36,    37,    38,     0,     0,
       1,     2,     3,     4,     5,     6,     0,    39,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,     0,     0,     0,    20,    21,    22,     0,    23,    24,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      25,    26,     0,     0,     0,     0,     0,     0,    27,     0,
       0,     0,     0,     0,     0,     0,    28,    29,    30,    31,
      32,     0,    33,     0,     0,    34,     0,     0,     0,    35,
      36,    37,    38,     0,     0,     1,     2,     3,     4,     5,
       6,     0,    39,     7,     8,     9,    10,    11,    88,    13,
     105,    15,    16,    17,    18,    19,     0,     0,     0,    20,
      21,    22,     0,    23,    24,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    25,    26,     0,     0,     0,
       0,     0,     0,    27,     0,     0,     0,     0,     0,     0,
       0,    28,    29,    30,    31,    32,     0,    33,     0,     0,
      34,     0,     0,     0,    35,    36,    37,    38,     0,     1,
       2,     3,     4,     5,     0,     0,     0,    39,     8,   195,
       0,     0,    88,     0,   105,    15,     0,    17,     0,   149,
     150,     0,     0,    20,   151,   152,   153,   154,   155,   156,
     157,   158,   159,   160,   161,     0,     0,     0,     0,    25,
      26,     0,     0,     0,     0,     0,     0,    27,     0,     0,
       0,     0,     0,     0,     0,    89,    29,    30,    31,    32,
     162,    90,     0,     0,    34,     0,     0,     0,    35,    36,
      37,    38,     1,     2,     3,     4,     5,     0,     0,     0,
       0,     8,     0,     0,     0,    88,     0,   105,    15,     0,
      17,     0,     0,     0,     0,     0,    20,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    25,    26,     0,     0,     0,     0,     0,     0,
      27,     0,     0,     0,     0,     0,     0,     0,    89,    29,
      30,    31,    32,     0,    90,     0,     0,    34,   241,     0,
       0,    35,    36,    37,    38,     1,     2,     3,     4,     5,
       0,     0,     0,     0,     8,     0,     0,     0,    88,     0,
     105,    15,     0,    17,     0,     0,     0,     0,     0,    20,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    25,    26,     0,     0,     0,
       0,     0,     0,    27,     0,     0,     0,     0,     0,     0,
       0,    89,    29,    30,    31,    32,   244,    90,     0,     0,
      34,     0,     0,     0,    35,    36,    37,    38,     1,     2,
       3,     4,     5,     0,     0,     0,     0,     8,     0,     0,
       0,    88,     0,   105,    15,     0,    17,     0,     0,     0,
       0,     0,    20,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    25,    26,
       0,     0,     0,     0,     0,     0,    27,     0,     0,     0,
       0,     0,     0,     0,    89,    29,    30,    31,    32,     0,
      90,     0,     0,    34,   326,     0,     0,    35,    36,    37,
      38,     1,     2,     3,     4,     5,     0,     0,     0,     0,
       8,     0,     0,     0,    12,     0,   105,    15,     0,    17,
       0,     0,     0,     0,     0,    20,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    25,    26,     0,     0,     0,     0,     0,     0,    27,
       0,     0,     0,     0,     0,     0,     0,    89,    29,    30,
      31,    32,     0,    90,     0,     0,    34,     0,     0,     0,
      35,    36,    37,    38,     1,     2,     3,     4,     5,     0,
       0,     0,     0,     8,     0,     0,     0,    88,     0,   105,
      15,     0,    17,     1,     2,     3,     4,     5,    20,     0,
       0,     0,     8,     0,     0,     0,    88,     0,     0,     0,
       0,    17,     0,     0,    25,    26,     0,     0,     0,     0,
       0,     0,    27,     0,     0,     0,     0,     0,     0,     0,
      89,    29,    30,    31,    32,     0,    90,     0,   279,    34,
       0,    27,     0,    35,    36,    37,    38,     0,     0,    89,
       0,     0,    31,    32,     0,    90,   149,   150,    34,     0,
       0,   151,   152,   153,   154,   155,   156,   157,   158,   159,
     160,   161,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   162
};

static const short int yycheck[] =
{
      18,    24,     0,    13,    82,    96,     1,     1,   136,    14,
      15,    10,     1,    23,   142,    20,    25,     1,    67,    63,
      25,    26,    32,    45,    29,    30,    59,    62,     1,     1,
      35,    36,    37,    38,    59,   163,    85,     1,    41,     6,
       7,    76,    77,    63,    35,    36,    37,    38,    68,    63,
       1,    71,   284,     1,    68,   133,   288,    71,    65,   187,
      81,   189,     6,     7,    59,    59,    63,    66,     1,    91,
       3,     4,     5,     6,     7,    84,   204,    87,    67,    12,
      83,   313,    59,    16,    59,    18,    19,    64,    21,    64,
      85,    85,    59,    59,    27,    67,    85,    63,    67,    66,
     105,    85,   112,    67,   114,   123,   116,   117,    63,   237,
      43,    44,    85,    85,   280,    59,    67,    63,    51,    67,
     207,    85,    66,    63,   211,    70,    59,    60,    61,    62,
      63,    80,    65,   143,    85,    68,   146,    85,    42,    72,
      73,    74,    75,   309,   172,   173,   174,   175,   176,   177,
      64,    64,    85,    67,    67,    64,    25,    26,    67,   164,
     165,   166,   178,   179,   180,   181,    64,    82,   334,    67,
      39,    40,    63,   301,    64,   303,    64,    67,   306,    67,
      64,   347,    64,    67,    64,    67,   277,    67,    64,    64,
      64,    67,    67,    67,    67,    67,    69,   325,    70,    45,
      46,    47,    14,    15,   169,   170,   171,   230,    20,    78,
      79,    72,    73,    25,    26,    43,    44,    29,    30,    67,
      67,    69,    69,    35,    36,    37,    38,    33,    34,    66,
      67,   167,   168,     0,    84,     0,    63,   315,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      23,    70,    27,    28,    29,    70,    31,    32,    70,   279,
     280,    67,   290,    59,   292,    87,   294,    59,    59,    67,
      65,   291,    85,    63,    63,    34,    70,    59,   286,    65,
      59,    25,    70,   105,    59,    60,    61,   307,    85,   309,
      65,    66,    24,     9,    64,    64,    70,    64,    85,    74,
      75,    66,   239,   236,     8,   333,     8,   182,   184,   337,
     343,   331,   185,   183,   334,   362,   341,   345,   186,   339,
     354,   195,   231,    14,   215,    -1,    -1,   347,    -1,    -1,
     358,    -1,   360,   361,   362,    -1,    -1,   365,    -1,    -1,
     368,   369,   164,   165,   166,   167,   168,   169,   170,   171,
     172,   173,   174,   175,   176,   177,   178,   179,   180,   181,
     182,   183,   184,   185,   186,     3,     4,     5,     6,     7,
       8,    -1,    -1,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    -1,    -1,    -1,    27,
      28,    29,    -1,    31,    32,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    43,    44,    -1,    -1,    -1,
      -1,    -1,    -1,    51,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    59,    60,    61,    62,    63,    -1,    65,    66,    -1,
      68,    -1,    -1,    -1,    72,    73,    74,    75,    -1,    -1,
       3,     4,     5,     6,     7,     8,    -1,    85,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    -1,    -1,    -1,    27,    28,    29,    -1,    31,    32,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      43,    44,    -1,    -1,    -1,    -1,    -1,    -1,    51,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    59,    60,    61,    62,
      63,    -1,    65,    66,    -1,    68,    -1,    -1,    -1,    72,
      73,    74,    75,    -1,    -1,     3,     4,     5,     6,     7,
       8,    -1,    85,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    -1,    -1,    -1,    27,
      28,    29,    -1,    31,    32,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    43,    44,    -1,    -1,    -1,
      -1,    -1,    -1,    51,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    59,    60,    61,    62,    63,    -1,    65,    66,    -1,
      68,    -1,    -1,    -1,    72,    73,    74,    75,    -1,    -1,
       3,     4,     5,     6,     7,     8,    -1,    85,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    -1,    -1,    -1,    27,    28,    29,    -1,    31,    32,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      43,    44,    -1,    -1,    -1,    -1,    -1,    -1,    51,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    59,    60,    61,    62,
      63,    -1,    65,    66,    -1,    68,    -1,    -1,    -1,    72,
      73,    74,    75,    -1,    -1,     3,     4,     5,     6,     7,
       8,    -1,    85,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    -1,    -1,    -1,    27,
      28,    29,    -1,    31,    32,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    43,    44,    -1,    -1,    -1,
      -1,    -1,    -1,    51,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    59,    60,    61,    62,    63,    -1,    65,    66,    -1,
      68,    -1,    -1,    -1,    72,    73,    74,    75,    -1,    -1,
       3,     4,     5,     6,     7,     8,    -1,    85,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    -1,    -1,    -1,    27,    28,    29,    -1,    31,    32,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      43,    44,    -1,    -1,    -1,    -1,    -1,    -1,    51,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    59,    60,    61,    62,
      63,    -1,    65,    -1,    -1,    68,    -1,    -1,    -1,    72,
      73,    74,    75,    -1,    -1,     3,     4,     5,     6,     7,
       8,    -1,    85,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    -1,    -1,    -1,    27,
      28,    29,    -1,    31,    32,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    43,    44,    -1,    -1,    -1,
      -1,    -1,    -1,    51,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    59,    60,    61,    62,    63,    -1,    65,    -1,    -1,
      68,    -1,    -1,    -1,    72,    73,    74,    75,    -1,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    85,    12,    13,
      -1,    -1,    16,    -1,    18,    19,    -1,    21,    -1,    43,
      44,    -1,    -1,    27,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    -1,    -1,    -1,    -1,    43,
      44,    -1,    -1,    -1,    -1,    -1,    -1,    51,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    59,    60,    61,    62,    63,
      84,    65,    -1,    -1,    68,    -1,    -1,    -1,    72,    73,
      74,    75,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    12,    -1,    -1,    -1,    16,    -1,    18,    19,    -1,
      21,    -1,    -1,    -1,    -1,    -1,    27,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    43,    44,    -1,    -1,    -1,    -1,    -1,    -1,
      51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    59,    60,
      61,    62,    63,    -1,    65,    -1,    -1,    68,    69,    -1,
      -1,    72,    73,    74,    75,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    12,    -1,    -1,    -1,    16,    -1,
      18,    19,    -1,    21,    -1,    -1,    -1,    -1,    -1,    27,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    43,    44,    -1,    -1,    -1,
      -1,    -1,    -1,    51,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    59,    60,    61,    62,    63,    64,    65,    -1,    -1,
      68,    -1,    -1,    -1,    72,    73,    74,    75,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    12,    -1,    -1,
      -1,    16,    -1,    18,    19,    -1,    21,    -1,    -1,    -1,
      -1,    -1,    27,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    43,    44,
      -1,    -1,    -1,    -1,    -1,    -1,    51,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    59,    60,    61,    62,    63,    -1,
      65,    -1,    -1,    68,    69,    -1,    -1,    72,    73,    74,
      75,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      12,    -1,    -1,    -1,    16,    -1,    18,    19,    -1,    21,
      -1,    -1,    -1,    -1,    -1,    27,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    43,    44,    -1,    -1,    -1,    -1,    -1,    -1,    51,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    59,    60,    61,
      62,    63,    -1,    65,    -1,    -1,    68,    -1,    -1,    -1,
      72,    73,    74,    75,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    12,    -1,    -1,    -1,    16,    -1,    18,
      19,    -1,    21,     3,     4,     5,     6,     7,    27,    -1,
      -1,    -1,    12,    -1,    -1,    -1,    16,    -1,    -1,    -1,
      -1,    21,    -1,    -1,    43,    44,    -1,    -1,    -1,    -1,
      -1,    -1,    51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      59,    60,    61,    62,    63,    -1,    65,    -1,    25,    68,
      -1,    51,    -1,    72,    73,    74,    75,    -1,    -1,    59,
      -1,    -1,    62,    63,    -1,    65,    43,    44,    68,    -1,
      -1,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    84
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,     3,     4,     5,     6,     7,     8,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      27,    28,    29,    31,    32,    43,    44,    51,    59,    60,
      61,    62,    63,    65,    68,    72,    73,    74,    75,    85,
      87,    88,    89,    95,    96,    97,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   116,   117,   118,   120,   123,   127,   128,   129,   130,
     132,   133,   134,   135,   136,   142,   143,   144,   147,   148,
     149,   152,   153,   154,     1,    59,    85,    63,    16,    59,
      65,    95,    96,    59,   121,   122,    59,   124,   125,     1,
      59,    85,    59,    63,     1,    18,    85,   116,   100,   102,
     148,   102,    63,   117,    63,   102,    63,    63,   116,    65,
     118,   102,   102,    70,   102,   102,   116,     6,     7,    59,
      66,    93,    94,   153,    67,    90,    91,    92,   102,   102,
     102,   102,    63,    68,    71,    98,    68,    71,    98,    43,
      44,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    84,   115,    62,    76,    77,    72,    73,    45,
      46,    47,    25,    26,    39,    40,    78,    79,    35,    36,
      37,    38,    80,    81,    82,    42,    41,    83,     1,    67,
      85,     0,   154,     1,    85,    13,   100,   116,   131,     6,
       7,    59,    66,    98,    84,   126,     1,    67,    85,   126,
       1,    67,    85,     1,    85,    63,    59,    64,   150,     1,
      85,   116,    23,   116,   116,   116,     1,    85,    66,    33,
      34,   145,   146,   117,    64,    66,    67,    70,    66,    67,
      69,    69,   114,    67,    64,    99,   114,   116,    59,   116,
      59,   114,   102,   102,   102,   103,   103,   104,   104,   104,
     105,   105,   105,   105,   105,   105,   106,   106,   106,   106,
     107,   108,   109,   110,   111,   114,   114,    59,   121,    25,
      85,   114,   122,   122,    64,   150,    65,   151,    64,    67,
      64,    63,    64,    64,    64,    63,   118,   146,    66,    94,
     114,    91,    64,    67,    69,    69,    70,    25,   126,    85,
     116,   131,   151,    64,    66,   153,   151,    59,   117,   116,
     117,    65,   137,   117,    59,    70,    69,   114,   114,   114,
     116,    25,   131,    64,    85,   151,    66,    24,    64,     9,
     138,   139,   140,    64,   114,    64,   116,    85,   117,   131,
     117,   116,    10,    66,   141,   140,   118,   117,    64,   131,
      64,    70,    70,   138,   117,    64,   117,   117,   119,   119,
      66,   117,   117
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
#line 165 "grammar.y"
    { (yyval.node) = new NullNode(); ;}
    break;

  case 3:
#line 166 "grammar.y"
    { (yyval.node) = new BooleanNode(true); ;}
    break;

  case 4:
#line 167 "grammar.y"
    { (yyval.node) = new BooleanNode(false); ;}
    break;

  case 5:
#line 168 "grammar.y"
    { (yyval.node) = new NumberNode((yyvsp[0].dval)); ;}
    break;

  case 6:
#line 169 "grammar.y"
    { (yyval.node) = new StringNode((yyvsp[0].ustr)); ;}
    break;

  case 7:
#line 170 "grammar.y"
    { Lexer *l = Lexer::curr();
                                     if (!l->scanRegExp()) YYABORT;
                                     (yyval.node) = new RegExpNode(l->pattern,l->flags);;}
    break;

  case 8:
#line 174 "grammar.y"
    { Lexer *l = Lexer::curr();
                                     if (!l->scanRegExp()) YYABORT;
                                     (yyval.node) = new RegExpNode(UString('=')+l->pattern,l->flags);;}
    break;

  case 9:
#line 180 "grammar.y"
    { (yyval.node) = new ThisNode(); ;}
    break;

  case 10:
#line 181 "grammar.y"
    { (yyval.node) = new ResolveNode(*(yyvsp[0].ident)); ;}
    break;

  case 13:
#line 184 "grammar.y"
    { (yyval.node) = new GroupNode((yyvsp[-1].node)); ;}
    break;

  case 14:
#line 185 "grammar.y"
    { (yyval.node) = new ObjectLiteralNode(); ;}
    break;

  case 15:
#line 186 "grammar.y"
    { (yyval.node) = new ObjectLiteralNode((yyvsp[-1].plist)); ;}
    break;

  case 16:
#line 187 "grammar.y"
    { (yyval.node) = new ObjectLiteralNode((yyvsp[-2].plist)); ;}
    break;

  case 17:
#line 191 "grammar.y"
    { (yyval.node) = new ArrayNode((yyvsp[-1].ival)); ;}
    break;

  case 18:
#line 192 "grammar.y"
    { (yyval.node) = new ArrayNode((yyvsp[-1].elm)); ;}
    break;

  case 19:
#line 193 "grammar.y"
    { (yyval.node) = new ArrayNode((yyvsp[-1].ival), (yyvsp[-3].elm)); ;}
    break;

  case 20:
#line 197 "grammar.y"
    { (yyval.elm) = new ElementNode((yyvsp[-1].ival), (yyvsp[0].node)); ;}
    break;

  case 21:
#line 199 "grammar.y"
    { (yyval.elm) = new ElementNode((yyvsp[-3].elm), (yyvsp[-1].ival), (yyvsp[0].node)); ;}
    break;

  case 22:
#line 203 "grammar.y"
    { (yyval.ival) = 0; ;}
    break;

  case 24:
#line 208 "grammar.y"
    { (yyval.ival) = 1; ;}
    break;

  case 25:
#line 209 "grammar.y"
    { (yyval.ival) = (yyvsp[-1].ival) + 1; ;}
    break;

  case 26:
#line 213 "grammar.y"
    { (yyval.plist) = new PropertyValueNode((yyvsp[-2].pnode), (yyvsp[0].node)); ;}
    break;

  case 27:
#line 215 "grammar.y"
    { (yyval.plist) = new PropertyValueNode((yyvsp[-2].pnode), (yyvsp[0].node), (yyvsp[-4].plist)); ;}
    break;

  case 28:
#line 219 "grammar.y"
    { (yyval.pnode) = new PropertyNode(*(yyvsp[0].ident)); ;}
    break;

  case 29:
#line 220 "grammar.y"
    { (yyval.pnode) = new PropertyNode(Identifier(*(yyvsp[0].ustr))); ;}
    break;

  case 30:
#line 221 "grammar.y"
    { (yyval.pnode) = new PropertyNode((yyvsp[0].dval)); ;}
    break;

  case 33:
#line 227 "grammar.y"
    { (yyval.node) = new AccessorNode1((yyvsp[-3].node), (yyvsp[-1].node)); ;}
    break;

  case 34:
#line 228 "grammar.y"
    { (yyval.node) = new AccessorNode2((yyvsp[-2].node), *(yyvsp[0].ident)); ;}
    break;

  case 35:
#line 229 "grammar.y"
    { (yyval.node) = new NewExprNode((yyvsp[-1].node), (yyvsp[0].args)); ;}
    break;

  case 37:
#line 234 "grammar.y"
    { (yyval.node) = new NewExprNode((yyvsp[0].node)); ;}
    break;

  case 38:
#line 238 "grammar.y"
    { (yyval.node) = new FunctionCallNode((yyvsp[-1].node), (yyvsp[0].args)); ;}
    break;

  case 39:
#line 239 "grammar.y"
    { (yyval.node) = new FunctionCallNode((yyvsp[-1].node), (yyvsp[0].args)); ;}
    break;

  case 40:
#line 240 "grammar.y"
    { (yyval.node) = new AccessorNode1((yyvsp[-3].node), (yyvsp[-1].node)); ;}
    break;

  case 41:
#line 241 "grammar.y"
    { (yyval.node) = new AccessorNode2((yyvsp[-2].node), *(yyvsp[0].ident)); ;}
    break;

  case 42:
#line 245 "grammar.y"
    { (yyval.args) = new ArgumentsNode(); ;}
    break;

  case 43:
#line 246 "grammar.y"
    { (yyval.args) = new ArgumentsNode((yyvsp[-1].alist)); ;}
    break;

  case 44:
#line 250 "grammar.y"
    { (yyval.alist) = new ArgumentListNode((yyvsp[0].node)); ;}
    break;

  case 45:
#line 251 "grammar.y"
    { (yyval.alist) = new ArgumentListNode((yyvsp[-2].alist), (yyvsp[0].node)); ;}
    break;

  case 49:
#line 261 "grammar.y"
    { (yyval.node) = new PostfixNode((yyvsp[-1].node), OpPlusPlus); ;}
    break;

  case 50:
#line 262 "grammar.y"
    { (yyval.node) = new PostfixNode((yyvsp[-1].node), OpMinusMinus); ;}
    break;

  case 52:
#line 267 "grammar.y"
    { (yyval.node) = new DeleteNode((yyvsp[0].node)); ;}
    break;

  case 53:
#line 268 "grammar.y"
    { (yyval.node) = new VoidNode((yyvsp[0].node)); ;}
    break;

  case 54:
#line 269 "grammar.y"
    { (yyval.node) = new TypeOfNode((yyvsp[0].node)); ;}
    break;

  case 55:
#line 270 "grammar.y"
    { (yyval.node) = new PrefixNode(OpPlusPlus, (yyvsp[0].node)); ;}
    break;

  case 56:
#line 271 "grammar.y"
    { (yyval.node) = new PrefixNode(OpPlusPlus, (yyvsp[0].node)); ;}
    break;

  case 57:
#line 272 "grammar.y"
    { (yyval.node) = new PrefixNode(OpMinusMinus, (yyvsp[0].node)); ;}
    break;

  case 58:
#line 273 "grammar.y"
    { (yyval.node) = new PrefixNode(OpMinusMinus, (yyvsp[0].node)); ;}
    break;

  case 59:
#line 274 "grammar.y"
    { (yyval.node) = new UnaryPlusNode((yyvsp[0].node)); ;}
    break;

  case 60:
#line 275 "grammar.y"
    { (yyval.node) = new NegateNode((yyvsp[0].node)); ;}
    break;

  case 61:
#line 276 "grammar.y"
    { (yyval.node) = new BitwiseNotNode((yyvsp[0].node)); ;}
    break;

  case 62:
#line 277 "grammar.y"
    { (yyval.node) = new LogicalNotNode((yyvsp[0].node)); ;}
    break;

  case 64:
#line 282 "grammar.y"
    { (yyval.node) = new MultNode((yyvsp[-2].node), (yyvsp[0].node), '*'); ;}
    break;

  case 65:
#line 283 "grammar.y"
    { (yyval.node) = new MultNode((yyvsp[-2].node), (yyvsp[0].node), '/'); ;}
    break;

  case 66:
#line 284 "grammar.y"
    { (yyval.node) = new MultNode((yyvsp[-2].node),(yyvsp[0].node),'%'); ;}
    break;

  case 68:
#line 289 "grammar.y"
    { (yyval.node) = AddNode::create((yyvsp[-2].node), (yyvsp[0].node), '+'); ;}
    break;

  case 69:
#line 290 "grammar.y"
    { (yyval.node) = AddNode::create((yyvsp[-2].node), (yyvsp[0].node), '-'); ;}
    break;

  case 71:
#line 295 "grammar.y"
    { (yyval.node) = new ShiftNode((yyvsp[-2].node), OpLShift, (yyvsp[0].node)); ;}
    break;

  case 72:
#line 296 "grammar.y"
    { (yyval.node) = new ShiftNode((yyvsp[-2].node), OpRShift, (yyvsp[0].node)); ;}
    break;

  case 73:
#line 297 "grammar.y"
    { (yyval.node) = new ShiftNode((yyvsp[-2].node), OpURShift, (yyvsp[0].node)); ;}
    break;

  case 75:
#line 303 "grammar.y"
    { (yyval.node) = new RelationalNode((yyvsp[-2].node), OpLess, (yyvsp[0].node)); ;}
    break;

  case 76:
#line 305 "grammar.y"
    { (yyval.node) = new RelationalNode((yyvsp[-2].node), OpGreater, (yyvsp[0].node)); ;}
    break;

  case 77:
#line 307 "grammar.y"
    { (yyval.node) = new RelationalNode((yyvsp[-2].node), OpLessEq, (yyvsp[0].node)); ;}
    break;

  case 78:
#line 309 "grammar.y"
    { (yyval.node) = new RelationalNode((yyvsp[-2].node), OpGreaterEq, (yyvsp[0].node)); ;}
    break;

  case 79:
#line 311 "grammar.y"
    { (yyval.node) = new RelationalNode((yyvsp[-2].node), OpInstanceOf, (yyvsp[0].node)); ;}
    break;

  case 80:
#line 313 "grammar.y"
    { (yyval.node) = new RelationalNode((yyvsp[-2].node), OpIn, (yyvsp[0].node)); ;}
    break;

  case 82:
#line 318 "grammar.y"
    { (yyval.node) = new EqualNode((yyvsp[-2].node), OpEqEq, (yyvsp[0].node)); ;}
    break;

  case 83:
#line 319 "grammar.y"
    { (yyval.node) = new EqualNode((yyvsp[-2].node), OpNotEq, (yyvsp[0].node)); ;}
    break;

  case 84:
#line 320 "grammar.y"
    { (yyval.node) = new EqualNode((yyvsp[-2].node), OpStrEq, (yyvsp[0].node)); ;}
    break;

  case 85:
#line 321 "grammar.y"
    { (yyval.node) = new EqualNode((yyvsp[-2].node), OpStrNEq, (yyvsp[0].node));;}
    break;

  case 87:
#line 326 "grammar.y"
    { (yyval.node) = new BitOperNode((yyvsp[-2].node), OpBitAnd, (yyvsp[0].node)); ;}
    break;

  case 89:
#line 331 "grammar.y"
    { (yyval.node) = new BitOperNode((yyvsp[-2].node), OpBitXOr, (yyvsp[0].node)); ;}
    break;

  case 91:
#line 336 "grammar.y"
    { (yyval.node) = new BitOperNode((yyvsp[-2].node), OpBitOr, (yyvsp[0].node)); ;}
    break;

  case 93:
#line 342 "grammar.y"
    { (yyval.node) = new BinaryLogicalNode((yyvsp[-2].node), OpAnd, (yyvsp[0].node)); ;}
    break;

  case 95:
#line 348 "grammar.y"
    { (yyval.node) = new BinaryLogicalNode((yyvsp[-2].node), OpOr, (yyvsp[0].node)); ;}
    break;

  case 97:
#line 354 "grammar.y"
    { (yyval.node) = new ConditionalNode((yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node)); ;}
    break;

  case 99:
#line 360 "grammar.y"
    { (yyval.node) = new AssignNode((yyvsp[-2].node), (yyvsp[-1].op), (yyvsp[0].node));;}
    break;

  case 100:
#line 364 "grammar.y"
    { (yyval.op) = OpEqual; ;}
    break;

  case 101:
#line 365 "grammar.y"
    { (yyval.op) = OpPlusEq; ;}
    break;

  case 102:
#line 366 "grammar.y"
    { (yyval.op) = OpMinusEq; ;}
    break;

  case 103:
#line 367 "grammar.y"
    { (yyval.op) = OpMultEq; ;}
    break;

  case 104:
#line 368 "grammar.y"
    { (yyval.op) = OpDivEq; ;}
    break;

  case 105:
#line 369 "grammar.y"
    { (yyval.op) = OpLShift; ;}
    break;

  case 106:
#line 370 "grammar.y"
    { (yyval.op) = OpRShift; ;}
    break;

  case 107:
#line 371 "grammar.y"
    { (yyval.op) = OpURShift; ;}
    break;

  case 108:
#line 372 "grammar.y"
    { (yyval.op) = OpAndEq; ;}
    break;

  case 109:
#line 373 "grammar.y"
    { (yyval.op) = OpXOrEq; ;}
    break;

  case 110:
#line 374 "grammar.y"
    { (yyval.op) = OpOrEq; ;}
    break;

  case 111:
#line 375 "grammar.y"
    { (yyval.op) = OpModEq; ;}
    break;

  case 113:
#line 380 "grammar.y"
    { (yyval.node) = new CommaNode((yyvsp[-2].node), (yyvsp[0].node)); ;}
    break;

  case 129:
#line 402 "grammar.y"
    { (yyval.stat) = new BlockNode(0); DBG((yyval.stat), (yylsp[0]), (yylsp[0])); ;}
    break;

  case 130:
#line 403 "grammar.y"
    { (yyval.stat) = new BlockNode((yyvsp[-1].srcs)); DBG((yyval.stat), (yylsp[0]), (yylsp[0])); ;}
    break;

  case 131:
#line 407 "grammar.y"
    { (yyval.slist) = new StatListNode((yyvsp[0].stat)); ;}
    break;

  case 132:
#line 408 "grammar.y"
    { (yyval.slist) = new StatListNode((yyvsp[-1].slist), (yyvsp[0].stat)); ;}
    break;

  case 133:
#line 412 "grammar.y"
    { (yyval.stat) = new VarStatementNode((yyvsp[-1].vlist));
                                      DBG((yyval.stat), (yylsp[-2]), (yylsp[0])); ;}
    break;

  case 134:
#line 414 "grammar.y"
    { if (automatic()) {
                                          (yyval.stat) = new VarStatementNode((yyvsp[-1].vlist));
					  DBG((yyval.stat), (yylsp[-2]), (yylsp[-1]));
                                        } else {
					  YYABORT;
					}
                                      ;}
    break;

  case 135:
#line 424 "grammar.y"
    { (yyval.vlist) = new VarDeclListNode((yyvsp[0].decl)); ;}
    break;

  case 136:
#line 426 "grammar.y"
    { (yyval.vlist) = new VarDeclListNode((yyvsp[-2].vlist), (yyvsp[0].decl)); ;}
    break;

  case 137:
#line 430 "grammar.y"
    { (yyval.decl) = new VarDeclNode(*(yyvsp[0].ident), 0, VarDeclNode::Variable); ;}
    break;

  case 138:
#line 431 "grammar.y"
    { (yyval.decl) = new VarDeclNode(*(yyvsp[-1].ident), (yyvsp[0].init), VarDeclNode::Variable); ;}
    break;

  case 139:
#line 435 "grammar.y"
    { (yyval.stat) = new VarStatementNode((yyvsp[-1].vlist));
                                      DBG((yyval.stat), (yylsp[-2]), (yylsp[0])); ;}
    break;

  case 140:
#line 437 "grammar.y"
    { if (automatic()) {
                                          (yyval.stat) = new VarStatementNode((yyvsp[-1].vlist));
					  DBG((yyval.stat), (yylsp[-2]), (yylsp[-1]));
                                        } else {
					  YYABORT;
					}
                                      ;}
    break;

  case 141:
#line 447 "grammar.y"
    { (yyval.vlist) = new VarDeclListNode((yyvsp[0].decl)); ;}
    break;

  case 142:
#line 449 "grammar.y"
    { (yyval.vlist) = new VarDeclListNode((yyvsp[-2].vlist), (yyvsp[0].decl)); ;}
    break;

  case 143:
#line 453 "grammar.y"
    { (yyval.decl) = new VarDeclNode(*(yyvsp[0].ident), 0, VarDeclNode::Constant); ;}
    break;

  case 144:
#line 454 "grammar.y"
    { (yyval.decl) = new VarDeclNode(*(yyvsp[-1].ident), (yyvsp[0].init), VarDeclNode::Constant); ;}
    break;

  case 145:
#line 458 "grammar.y"
    { (yyval.init) = new AssignExprNode((yyvsp[0].node)); ;}
    break;

  case 146:
#line 462 "grammar.y"
    { (yyval.stat) = new EmptyStatementNode(); DBG((yyval.stat), (yylsp[0]), (yylsp[0])); ;}
    break;

  case 147:
#line 466 "grammar.y"
    { (yyval.stat) = new ExprStatementNode((yyvsp[-1].node));
                                     DBG((yyval.stat), (yylsp[-1]), (yylsp[0])); ;}
    break;

  case 148:
#line 468 "grammar.y"
    { if (automatic()) {
                                       (yyval.stat) = new ExprStatementNode((yyvsp[-1].node));
				       DBG((yyval.stat), (yylsp[-1]), (yylsp[-1]));
                                     } else
				       YYABORT; ;}
    break;

  case 149:
#line 476 "grammar.y"
    { (yyval.stat) = new IfNode((yyvsp[-2].node),(yyvsp[0].stat),0);DBG((yyval.stat),(yylsp[-4]),(yylsp[-1])); ;}
    break;

  case 150:
#line 478 "grammar.y"
    { (yyval.stat) = new IfNode((yyvsp[-4].node),(yyvsp[-2].stat),(yyvsp[0].stat));DBG((yyval.stat),(yylsp[-6]),(yylsp[-3])); ;}
    break;

  case 151:
#line 482 "grammar.y"
    { (yyval.stat)=new DoWhileNode((yyvsp[-4].stat),(yyvsp[-1].node));DBG((yyval.stat),(yylsp[-5]),(yylsp[-3]));;}
    break;

  case 152:
#line 483 "grammar.y"
    { (yyval.stat) = new WhileNode((yyvsp[-2].node),(yyvsp[0].stat));DBG((yyval.stat),(yylsp[-4]),(yylsp[-1])); ;}
    break;

  case 153:
#line 485 "grammar.y"
    { (yyval.stat) = new ForNode((yyvsp[-6].node),(yyvsp[-4].node),(yyvsp[-2].node),(yyvsp[0].stat));
	                             DBG((yyval.stat),(yylsp[-8]),(yylsp[-1])); ;}
    break;

  case 154:
#line 488 "grammar.y"
    { (yyval.stat) = new ForNode((yyvsp[-6].vlist),(yyvsp[-4].node),(yyvsp[-2].node),(yyvsp[0].stat));
	                             DBG((yyval.stat),(yylsp[-9]),(yylsp[-1])); ;}
    break;

  case 155:
#line 491 "grammar.y"
    { (yyval.stat) = new ForInNode((yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].stat));
	                             DBG((yyval.stat),(yylsp[-6]),(yylsp[-1])); ;}
    break;

  case 156:
#line 494 "grammar.y"
    { (yyval.stat) = new ForInNode(*(yyvsp[-4].ident),0,(yyvsp[-2].node),(yyvsp[0].stat));
	                             DBG((yyval.stat),(yylsp[-7]),(yylsp[-1])); ;}
    break;

  case 157:
#line 497 "grammar.y"
    { (yyval.stat) = new ForInNode(*(yyvsp[-5].ident),(yyvsp[-4].init),(yyvsp[-2].node),(yyvsp[0].stat));
	                             DBG((yyval.stat),(yylsp[-8]),(yylsp[-1])); ;}
    break;

  case 158:
#line 502 "grammar.y"
    { (yyval.node) = 0; ;}
    break;

  case 160:
#line 507 "grammar.y"
    { (yyval.stat) = new ContinueNode(); DBG((yyval.stat),(yylsp[-1]),(yylsp[0])); ;}
    break;

  case 161:
#line 508 "grammar.y"
    { if (automatic()) {
                                       (yyval.stat) = new ContinueNode(); DBG((yyval.stat),(yylsp[-1]),(yylsp[0]));
                                     } else
				       YYABORT; ;}
    break;

  case 162:
#line 512 "grammar.y"
    { (yyval.stat) = new ContinueNode(*(yyvsp[-1].ident)); DBG((yyval.stat),(yylsp[-2]),(yylsp[0])); ;}
    break;

  case 163:
#line 513 "grammar.y"
    { if (automatic()) {
                                       (yyval.stat) = new ContinueNode(*(yyvsp[-1].ident));DBG((yyval.stat),(yylsp[-2]),(yylsp[-1]));
                                     } else
				       YYABORT; ;}
    break;

  case 164:
#line 520 "grammar.y"
    { (yyval.stat) = new BreakNode();DBG((yyval.stat),(yylsp[-1]),(yylsp[0])); ;}
    break;

  case 165:
#line 521 "grammar.y"
    { if (automatic()) {
                                       (yyval.stat) = new BreakNode(); DBG((yyval.stat),(yylsp[-1]),(yylsp[-1]));
                                     } else
				       YYABORT; ;}
    break;

  case 166:
#line 525 "grammar.y"
    { (yyval.stat) = new BreakNode(*(yyvsp[-1].ident)); DBG((yyval.stat),(yylsp[-2]),(yylsp[0])); ;}
    break;

  case 167:
#line 526 "grammar.y"
    { if (automatic()) {
                                       (yyval.stat) = new BreakNode(*(yyvsp[-1].ident)); DBG((yyval.stat),(yylsp[-2]),(yylsp[-1]));
                                     } else
				       YYABORT;
                                   ;}
    break;

  case 168:
#line 534 "grammar.y"
    { (yyval.stat) = new ReturnNode(0); DBG((yyval.stat),(yylsp[-1]),(yylsp[0])); ;}
    break;

  case 169:
#line 535 "grammar.y"
    { if (automatic()) {
                                       (yyval.stat) = new ReturnNode(0); DBG((yyval.stat),(yylsp[-1]),(yylsp[-1]));
                                     } else
				       YYABORT; ;}
    break;

  case 170:
#line 539 "grammar.y"
    { (yyval.stat) = new ReturnNode((yyvsp[-1].node)); DBG((yyval.stat),(yylsp[-2]),(yylsp[0])); ;}
    break;

  case 171:
#line 540 "grammar.y"
    { if (automatic()) {
                                       (yyval.stat) = new ReturnNode((yyvsp[-1].node)); DBG((yyval.stat),(yylsp[-2]),(yylsp[-2]));
                                     }
                                     else
				       YYABORT; ;}
    break;

  case 172:
#line 548 "grammar.y"
    { (yyval.stat) = new WithNode((yyvsp[-2].node),(yyvsp[0].stat));
                                     DBG((yyval.stat), (yylsp[-4]), (yylsp[-1])); ;}
    break;

  case 173:
#line 553 "grammar.y"
    { (yyval.stat) = new SwitchNode((yyvsp[-2].node), (yyvsp[0].cblk));
                                     DBG((yyval.stat), (yylsp[-4]), (yylsp[-1])); ;}
    break;

  case 174:
#line 558 "grammar.y"
    { (yyval.cblk) = new CaseBlockNode((yyvsp[-1].clist), 0, 0); ;}
    break;

  case 175:
#line 560 "grammar.y"
    { (yyval.cblk) = new CaseBlockNode((yyvsp[-3].clist), (yyvsp[-2].ccl), (yyvsp[-1].clist)); ;}
    break;

  case 176:
#line 564 "grammar.y"
    { (yyval.clist) = 0; ;}
    break;

  case 178:
#line 569 "grammar.y"
    { (yyval.clist) = new ClauseListNode((yyvsp[0].ccl)); ;}
    break;

  case 179:
#line 570 "grammar.y"
    { (yyval.clist) = new ClauseListNode((yyvsp[-1].clist), (yyvsp[0].ccl)); ;}
    break;

  case 180:
#line 574 "grammar.y"
    { (yyval.ccl) = new CaseClauseNode((yyvsp[-1].node)); ;}
    break;

  case 181:
#line 575 "grammar.y"
    { (yyval.ccl) = new CaseClauseNode((yyvsp[-2].node), (yyvsp[0].slist)); ;}
    break;

  case 182:
#line 579 "grammar.y"
    { (yyval.ccl) = new CaseClauseNode(0); ;}
    break;

  case 183:
#line 580 "grammar.y"
    { (yyval.ccl) = new CaseClauseNode(0, (yyvsp[0].slist)); ;}
    break;

  case 184:
#line 584 "grammar.y"
    { (yyvsp[0].stat)->pushLabel(*(yyvsp[-2].ident));
                                     (yyval.stat) = new LabelNode(*(yyvsp[-2].ident), (yyvsp[0].stat)); DBG((yyval.stat),(yylsp[-2]),(yylsp[-1])); ;}
    break;

  case 185:
#line 589 "grammar.y"
    { (yyval.stat) = new ThrowNode((yyvsp[-1].node)); DBG((yyval.stat),(yylsp[-2]),(yylsp[0])); ;}
    break;

  case 186:
#line 590 "grammar.y"
    { if (automatic()) {
                                       (yyval.stat) = new ThrowNode((yyvsp[-1].node)); DBG((yyval.stat),(yylsp[-2]),(yylsp[-2]));
                                     } else {
				       YYABORT; } ;}
    break;

  case 187:
#line 597 "grammar.y"
    { (yyval.stat) = new TryNode((yyvsp[-1].stat), (yyvsp[0].cnode)); DBG((yyval.stat),(yylsp[-2]),(yylsp[-2])); ;}
    break;

  case 188:
#line 598 "grammar.y"
    { (yyval.stat) = new TryNode((yyvsp[-1].stat), (yyvsp[0].fnode)); DBG((yyval.stat),(yylsp[-2]),(yylsp[-2])); ;}
    break;

  case 189:
#line 599 "grammar.y"
    { (yyval.stat) = new TryNode((yyvsp[-2].stat), (yyvsp[-1].cnode), (yyvsp[0].fnode)); DBG((yyval.stat),(yylsp[-3]),(yylsp[-3])); ;}
    break;

  case 190:
#line 603 "grammar.y"
    { CatchNode *c; (yyval.cnode) = c = new CatchNode(*(yyvsp[-2].ident), (yyvsp[0].stat));
				     DBG(c,(yylsp[-4]),(yylsp[-1])); ;}
    break;

  case 191:
#line 608 "grammar.y"
    { FinallyNode *f; (yyval.fnode) = f = new FinallyNode((yyvsp[0].stat)); DBG(f,(yylsp[-1]),(yylsp[-1])); ;}
    break;

  case 193:
#line 614 "grammar.y"
    { (yyval.func) = (yyvsp[0].func); ;}
    break;

  case 194:
#line 618 "grammar.y"
    { (yyval.func) = new FuncDeclNode(*(yyvsp[-3].ident), (yyvsp[0].body)); DBG((yyval.func),(yylsp[-4]),(yylsp[-1])); ;}
    break;

  case 195:
#line 620 "grammar.y"
    { (yyval.func) = new FuncDeclNode(*(yyvsp[-4].ident), (yyvsp[-2].param), (yyvsp[0].body)); DBG((yyval.func),(yylsp[-5]),(yylsp[-1])); ;}
    break;

  case 196:
#line 624 "grammar.y"
    { (yyval.node) = new FuncExprNode((yyvsp[0].body)); ;}
    break;

  case 197:
#line 626 "grammar.y"
    { (yyval.node) = new FuncExprNode((yyvsp[-2].param), (yyvsp[0].body)); ;}
    break;

  case 198:
#line 631 "grammar.y"
    { (yyval.param) = new ParameterNode(*(yyvsp[0].ident)); ;}
    break;

  case 199:
#line 632 "grammar.y"
    { (yyval.param) = new ParameterNode((yyvsp[-2].param), *(yyvsp[0].ident)); ;}
    break;

  case 200:
#line 636 "grammar.y"
    { (yyval.body) = new FunctionBodyNode(0);
	                             DBG((yyval.body), (yylsp[-1]), (yylsp[0]));;}
    break;

  case 201:
#line 638 "grammar.y"
    { (yyval.body) = new FunctionBodyNode((yyvsp[-1].srcs));
	                             DBG((yyval.body), (yylsp[-2]), (yylsp[0]));;}
    break;

  case 202:
#line 643 "grammar.y"
    { (yyval.prog) = new FunctionBodyNode(0);
                                     (yyval.prog)->setLoc(0, 0, Parser::source);
                                     Parser::progNode = (yyval.prog); ;}
    break;

  case 203:
#line 646 "grammar.y"
    { (yyval.prog) = new FunctionBodyNode((yyvsp[0].srcs));
                                     Parser::progNode = (yyval.prog); ;}
    break;

  case 204:
#line 651 "grammar.y"
    { (yyval.srcs) = new SourceElementsNode((yyvsp[0].stat)); ;}
    break;

  case 205:
#line 652 "grammar.y"
    { (yyval.srcs) = new SourceElementsNode((yyvsp[-1].srcs), (yyvsp[0].stat)); ;}
    break;

  case 206:
#line 656 "grammar.y"
    { (yyval.stat) = (yyvsp[0].stat); ;}
    break;

  case 207:
#line 657 "grammar.y"
    { (yyval.stat) = (yyvsp[0].func); ;}
    break;


      default: break;
    }

/* Line 1126 of yacc.c.  */
#line 2786 "grammar.tab.c"

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


#line 660 "grammar.y"


int yyerror (const char * /* s */)  /* Called by yyparse on error */
{
  //  fprintf(stderr, "ERROR: %s at line %d\n",
  //	  s, KJS::Lexer::curr()->lineNo());
  return 1;
}

/* may we automatically insert a semicolon ? */
bool automatic()
{
  if (Lexer::curr()->hadError())
    return false;
  if (yychar == '}' || yychar == 0)
    return true;
  else if (Lexer::curr()->prevTerminator())
    return true;

  return false;
}

