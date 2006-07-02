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
     DEBUGGER = 290,
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
#define DEBUGGER 290
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
#ifdef YYMAXDEPTH
#undef YYMAXDEPTH
#endif
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
#line 52 "grammar.y"
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
#line 297 "grammar.tab.c"
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
#line 321 "grammar.tab.c"

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
#define YYFINAL  195
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1398

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  87
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  70
/* YYNRULES -- Number of rules. */
#define YYNRULES  210
/* YYNRULES -- Number of states. */
#define YYNSTATES  377

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
      64,    65,    77,    73,    68,    74,    72,    63,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    71,    86,
      79,    85,    80,    84,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    69,     2,    70,    82,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    66,    83,    67,    75,     2,     2,     2,
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
     372,   375,   379,   381,   384,   388,   392,   394,   398,   400,
     403,   407,   411,   413,   417,   419,   422,   425,   427,   430,
     433,   439,   447,   454,   460,   470,   481,   489,   498,   508,
     509,   511,   514,   517,   521,   525,   528,   531,   535,   539,
     542,   545,   549,   553,   559,   565,   569,   575,   576,   578,
     580,   583,   587,   592,   595,   599,   603,   607,   611,   615,
     619,   624,   627,   630,   636,   639,   641,   644,   650,   657,
     662,   668,   670,   674,   677,   681,   682,   684,   686,   689,
     691
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short int yyrhs[] =
{
     154,     0,    -1,     3,    -1,     4,    -1,     5,    -1,     7,
      -1,     6,    -1,    63,    -1,    52,    -1,    21,    -1,    60,
      -1,    88,    -1,    90,    -1,    64,   117,    65,    -1,    66,
      67,    -1,    66,    94,    67,    -1,    66,    94,    68,    67,
      -1,    69,    92,    70,    -1,    69,    91,    70,    -1,    69,
      91,    68,    92,    70,    -1,    92,   115,    -1,    91,    68,
      92,   115,    -1,    -1,    93,    -1,    68,    -1,    93,    68,
      -1,    95,    71,   115,    -1,    94,    68,    95,    71,   115,
      -1,    60,    -1,     6,    -1,     7,    -1,    89,    -1,   151,
      -1,    96,    69,   117,    70,    -1,    96,    72,    60,    -1,
      12,    96,    99,    -1,    96,    -1,    12,    97,    -1,    96,
      99,    -1,    98,    99,    -1,    98,    69,   117,    70,    -1,
      98,    72,    60,    -1,    64,    65,    -1,    64,   100,    65,
      -1,   115,    -1,   100,    68,   115,    -1,    97,    -1,    98,
      -1,   101,    -1,   101,    44,    -1,   101,    45,    -1,   102,
      -1,    19,   103,    -1,    18,   103,    -1,    27,   103,    -1,
      44,   103,    -1,    61,   103,    -1,    45,   103,    -1,    62,
     103,    -1,    73,   103,    -1,    74,   103,    -1,    75,   103,
      -1,    76,   103,    -1,   103,    -1,   104,    77,   103,    -1,
     104,    63,   103,    -1,   104,    78,   103,    -1,   104,    -1,
     105,    73,   104,    -1,   105,    74,   104,    -1,   105,    -1,
     106,    46,   105,    -1,   106,    47,   105,    -1,   106,    48,
     105,    -1,   106,    -1,   107,    79,   106,    -1,   107,    80,
     106,    -1,   107,    40,   106,    -1,   107,    41,   106,    -1,
     107,    26,   106,    -1,   107,    25,   106,    -1,   107,    -1,
     108,    36,   107,    -1,   108,    37,   107,    -1,   108,    38,
     107,    -1,   108,    39,   107,    -1,   108,    -1,   109,    81,
     108,    -1,   109,    -1,   110,    82,   109,    -1,   110,    -1,
     111,    83,   110,    -1,   111,    -1,   112,    43,   111,    -1,
     112,    -1,   113,    42,   112,    -1,   113,    -1,   113,    84,
     115,    71,   115,    -1,   114,    -1,   101,   116,   115,    -1,
      85,    -1,    49,    -1,    50,    -1,    51,    -1,    52,    -1,
      53,    -1,    54,    -1,    55,    -1,    56,    -1,    58,    -1,
      59,    -1,    57,    -1,   115,    -1,   117,    68,   115,    -1,
     119,    -1,   121,    -1,   124,    -1,   128,    -1,   129,    -1,
     130,    -1,   131,    -1,   133,    -1,   134,    -1,   135,    -1,
     136,    -1,   137,    -1,   143,    -1,   144,    -1,   145,    -1,
     146,    -1,    66,    67,    -1,    66,   155,    67,    -1,   118,
      -1,   120,   118,    -1,    13,   122,    86,    -1,    13,   122,
       1,    -1,   123,    -1,   122,    68,   123,    -1,    60,    -1,
      60,   127,    -1,    14,   125,    86,    -1,    14,   125,     1,
      -1,   126,    -1,   125,    68,   123,    -1,    60,    -1,    60,
     127,    -1,    85,   115,    -1,    86,    -1,   117,    86,    -1,
     117,     1,    -1,    20,    64,   117,    65,   118,    -1,    20,
      64,   117,    65,   118,    24,   118,    -1,    22,   118,    23,
      64,   117,    65,    -1,    23,    64,   117,    65,   118,    -1,
      11,    64,   132,    86,   132,    86,   132,    65,   118,    -1,
      11,    64,    13,   122,    86,   132,    86,   132,    65,   118,
      -1,    11,    64,   101,    25,   117,    65,   118,    -1,    11,
      64,    13,    60,    25,   117,    65,   118,    -1,    11,    64,
      13,    60,   127,    25,   117,    65,   118,    -1,    -1,   117,
      -1,    15,    86,    -1,    15,     1,    -1,    15,    60,    86,
      -1,    15,    60,     1,    -1,     8,    86,    -1,     8,     1,
      -1,     8,    60,    86,    -1,     8,    60,     1,    -1,    17,
      86,    -1,    17,     1,    -1,    17,   117,    86,    -1,    17,
     117,     1,    -1,    29,    64,   117,    65,   118,    -1,    28,
      64,   117,    65,   138,    -1,    66,   139,    67,    -1,    66,
     139,   142,   139,    67,    -1,    -1,   140,    -1,   141,    -1,
     140,   141,    -1,     9,   117,    71,    -1,     9,   117,    71,
     120,    -1,    10,    71,    -1,    10,    71,   120,    -1,    60,
      71,   118,    -1,    31,   117,    86,    -1,    31,   117,     1,
      -1,    32,   119,   147,    -1,    32,   119,   148,    -1,    32,
     119,   147,   148,    -1,    35,    86,    -1,    35,     1,    -1,
      33,    64,    60,    65,   119,    -1,    34,   119,    -1,   150,
      -1,    18,   150,    -1,    16,    60,    64,    65,   153,    -1,
      16,    60,    64,   152,    65,   153,    -1,    16,    64,    65,
     153,    -1,    16,    64,   152,    65,   153,    -1,    60,    -1,
     152,    68,    60,    -1,    66,    67,    -1,    66,   155,    67,
      -1,    -1,   155,    -1,   156,    -1,   155,   156,    -1,   118,
      -1,   149,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short int yyrline[] =
{
       0,   169,   169,   170,   171,   172,   173,   174,   177,   184,
     185,   186,   187,   188,   189,   190,   191,   195,   196,   197,
     201,   202,   207,   208,   212,   213,   217,   218,   223,   224,
     225,   229,   230,   231,   232,   233,   237,   238,   242,   243,
     244,   245,   249,   250,   254,   255,   259,   260,   264,   265,
     266,   270,   271,   272,   273,   274,   275,   276,   277,   278,
     279,   280,   281,   285,   286,   287,   288,   292,   293,   294,
     298,   299,   300,   301,   305,   306,   308,   310,   312,   314,
     316,   321,   322,   323,   324,   325,   329,   330,   334,   335,
     339,   340,   344,   345,   350,   351,   356,   357,   362,   363,
     368,   369,   370,   371,   372,   373,   374,   375,   376,   377,
     378,   379,   383,   384,   388,   389,   390,   391,   392,   393,
     394,   395,   396,   397,   398,   399,   400,   401,   402,   403,
     407,   408,   412,   413,   417,   419,   429,   430,   435,   436,
     440,   442,   452,   453,   458,   459,   463,   467,   471,   473,
     481,   482,   487,   488,   489,   492,   495,   498,   501,   507,
     508,   512,   513,   517,   518,   525,   526,   530,   531,   539,
     540,   544,   545,   553,   558,   563,   564,   569,   570,   574,
     575,   579,   580,   584,   585,   589,   594,   595,   602,   603,
     604,   608,   609,   618,   623,   627,   629,   633,   634,   639,
     640,   646,   647,   651,   653,   658,   661,   666,   667,   671,
     672
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
  "WITH", "RESERVED", "THROW", "TRY", "CATCH", "FINALLY", "DEBUGGER",
  "EQEQ", "NE", "STREQ", "STRNEQ", "LE", "GE", "OR", "AND", "PLUSPLUS",
  "MINUSMINUS", "LSHIFT", "RSHIFT", "URSHIFT", "PLUSEQUAL", "MINUSEQUAL",
  "MULTEQUAL", "DIVEQUAL", "LSHIFTEQUAL", "RSHIFTEQUAL", "URSHIFTEQUAL",
  "ANDEQUAL", "MODEQUAL", "XOREQUAL", "OREQUAL", "IDENT", "AUTOPLUSPLUS",
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
  "ThrowStatement", "TryStatement", "DebuggerStatement", "Catch",
  "Finally", "FunctionDeclaration", "FunctionDeclarationInternal",
  "FunctionExpr", "FormalParameterList", "FunctionBody", "Program",
  "SourceElements", "SourceElement", 0
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
     315,   316,   317,    47,    40,    41,   123,   125,    44,    91,
      93,    58,    46,    43,    45,   126,    33,    42,    37,    60,
      62,    38,    94,   124,    63,    61,    59
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    87,    88,    88,    88,    88,    88,    88,    88,    89,
      89,    89,    89,    89,    89,    89,    89,    90,    90,    90,
      91,    91,    92,    92,    93,    93,    94,    94,    95,    95,
      95,    96,    96,    96,    96,    96,    97,    97,    98,    98,
      98,    98,    99,    99,   100,   100,   101,   101,   102,   102,
     102,   103,   103,   103,   103,   103,   103,   103,   103,   103,
     103,   103,   103,   104,   104,   104,   104,   105,   105,   105,
     106,   106,   106,   106,   107,   107,   107,   107,   107,   107,
     107,   108,   108,   108,   108,   108,   109,   109,   110,   110,
     111,   111,   112,   112,   113,   113,   114,   114,   115,   115,
     116,   116,   116,   116,   116,   116,   116,   116,   116,   116,
     116,   116,   117,   117,   118,   118,   118,   118,   118,   118,
     118,   118,   118,   118,   118,   118,   118,   118,   118,   118,
     119,   119,   120,   120,   121,   121,   122,   122,   123,   123,
     124,   124,   125,   125,   126,   126,   127,   128,   129,   129,
     130,   130,   131,   131,   131,   131,   131,   131,   131,   132,
     132,   133,   133,   133,   133,   134,   134,   134,   134,   135,
     135,   135,   135,   136,   137,   138,   138,   139,   139,   140,
     140,   141,   141,   142,   142,   143,   144,   144,   145,   145,
     145,   146,   146,   147,   148,   149,   149,   150,   150,   151,
     151,   152,   152,   153,   153,   154,   154,   155,   155,   156,
     156
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
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       2,     3,     1,     2,     3,     3,     1,     3,     1,     2,
       3,     3,     1,     3,     1,     2,     2,     1,     2,     2,
       5,     7,     6,     5,     9,    10,     7,     8,     9,     0,
       1,     2,     2,     3,     3,     2,     2,     3,     3,     2,
       2,     3,     3,     5,     5,     3,     5,     0,     1,     1,
       2,     3,     4,     2,     3,     3,     3,     3,     3,     3,
       4,     2,     2,     5,     2,     1,     2,     5,     6,     4,
       5,     1,     3,     2,     3,     0,     1,     1,     2,     1,
       1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
     205,     2,     3,     4,     6,     5,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     9,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     8,    10,
       0,     0,     7,     0,     0,    22,     0,     0,     0,     0,
     147,    11,    31,    12,    36,    46,    47,    48,    51,    63,
      67,    70,    74,    81,    86,    88,    90,    92,    94,    96,
      98,   112,     0,   209,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,   129,
     210,   195,    32,     0,   206,   207,   166,     0,   165,   159,
       0,    10,     0,    36,    37,   138,     0,   136,   144,     0,
     142,   162,     0,   161,     0,     0,   170,     0,   169,     0,
      48,    53,   196,    52,     0,     0,     0,    54,     0,     0,
       0,     0,     0,   192,   191,    55,    57,     0,    56,    58,
       0,     6,     5,    10,    14,     0,     0,     0,    24,     0,
       0,    23,    59,    60,    61,    62,     0,     0,     0,    38,
       0,     0,    39,    49,    50,   101,   102,   103,   104,   105,
     106,   107,   108,   111,   109,   110,   100,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   149,     0,   148,     1,   208,   168,   167,     0,
      48,   160,     0,    29,    30,    28,    14,    35,     0,   139,
     135,     0,   134,   145,   141,     0,   140,   164,   163,     0,
     201,     0,     0,   172,   171,     0,     0,     0,     0,     0,
     187,   186,   130,     0,     0,   188,   189,   185,    13,    15,
       0,     0,   131,    22,    18,    17,    20,    25,    42,     0,
      44,     0,    34,     0,    41,    99,    65,    64,    66,    68,
      69,    71,    72,    73,    80,    79,    77,    78,    75,    76,
      82,    83,    84,    85,    87,    89,    91,    93,    95,     0,
     113,   138,     0,     0,   159,   146,   137,   143,     0,     0,
       0,   199,     0,     0,     0,     0,     0,     0,     0,     0,
     194,   190,    16,     0,    26,     0,    43,     0,    33,    40,
       0,     0,   139,   159,     0,     0,   197,     0,   203,     0,
     200,   202,   150,     0,   153,   177,   174,   173,     0,     0,
      19,    21,    45,    97,     0,     0,     0,     0,   159,   198,
     204,     0,   152,     0,     0,   178,   179,     0,    27,     0,
       0,   159,   156,     0,   151,     0,     0,   175,   177,   180,
     193,   157,     0,     0,     0,   181,   183,     0,   158,     0,
     154,   132,   182,   184,   176,   155,   133
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short int yydefgoto[] =
{
      -1,    41,    42,    43,   139,   140,   141,   135,   136,    44,
      45,    46,   149,   249,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,   167,
      62,    63,    64,   372,    65,    96,    97,    66,    99,   100,
     209,    67,    68,    69,    70,   202,    71,    72,    73,    74,
      75,   326,   344,   345,   346,   358,    76,    77,    78,    79,
     235,   236,    80,    81,    82,   222,   291,    83,   137,    85
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -271
static const short int yypact[] =
{
     756,  -271,  -271,  -271,  -271,  -271,     6,    21,   180,   -19,
      -7,     7,    -9,    83,  1248,  1322,    84,  -271,   832,   110,
    1322,   117,   135,  1322,   165,    26,  1322,  1322,  -271,     3,
    1322,  1322,  -271,  1322,   376,    51,  1322,  1322,  1322,  1322,
    -271,  -271,  -271,  -271,   -10,  -271,    53,   254,  -271,  -271,
     -14,   143,   107,   125,    94,   126,   163,   167,   208,   -32,
    -271,  -271,     8,  -271,  -271,  -271,  -271,  -271,  -271,  -271,
    -271,  -271,  -271,  -271,  -271,  -271,  -271,  -271,  -271,  -271,
    -271,  -271,  -271,   263,   756,  -271,  -271,    27,  -271,   952,
     200,  -271,    10,   -10,  -271,   181,    12,  -271,   181,    14,
    -271,  -271,    32,  -271,   201,   -23,  -271,  1322,  -271,    29,
     176,  -271,  -271,  -271,  1322,   244,  1322,  -271,  1322,  1322,
      43,   452,   189,  -271,  -271,  -271,  -271,   832,  -271,  -271,
     -34,   197,   198,     3,   916,   162,   199,   528,  -271,    56,
    1026,   206,  -271,  -271,  -271,  -271,  1100,  1322,   211,  -271,
    1322,   215,  -271,  -271,  -271,  -271,  -271,  -271,  -271,  -271,
    -271,  -271,  -271,  -271,  -271,  -271,  -271,  1322,  1322,  1322,
    1322,  1322,  1322,  1322,  1322,  1322,  1322,  1322,  1322,  1322,
    1322,  1322,  1322,  1322,  1322,  1322,  1322,  1322,  1322,  1322,
    1322,  1322,  -271,  1322,  -271,  -271,  -271,  -271,  -271,   219,
     203,   213,   196,  -271,  -271,  -271,  -271,  -271,  1322,  -271,
    -271,   -19,  -271,  -271,  -271,   -19,  -271,  -271,  -271,    31,
    -271,   217,     4,  -271,  -271,    55,   221,    69,    73,    99,
    -271,  -271,  -271,   222,   165,   253,  -271,  -271,  -271,  -271,
      54,  1322,  -271,    51,  -271,  -271,  -271,  -271,  -271,   103,
    -271,    72,  -271,   112,  -271,  -271,  -271,  -271,  -271,   -14,
     -14,   143,   143,   143,   107,   107,   107,   107,   107,   107,
     125,   125,   125,   125,    94,   126,   163,   167,   208,   218,
    -271,    -6,   -48,  1322,  1322,  -271,  -271,  -271,   217,   108,
     604,  -271,   217,   230,   832,  1322,   832,   226,   832,   233,
    -271,  -271,  -271,   223,  -271,  1174,  -271,  1322,  -271,  -271,
    1322,  1322,   271,  1322,   123,   228,  -271,   217,  -271,   680,
    -271,  -271,   273,   130,  -271,   292,  -271,  -271,   250,  1322,
    -271,  -271,  -271,  -271,   132,  1322,   231,   832,  1322,  -271,
    -271,   832,  -271,  1322,    16,   292,  -271,   165,  -271,   832,
     141,  1322,  -271,   251,  -271,   122,   247,  -271,   292,  -271,
    -271,  -271,   832,   255,   832,   832,   832,   258,  -271,   832,
    -271,  -271,   832,   832,  -271,  -271,  -271
};

/* YYPGOTO[NTERM-NUM].  */
static const short int yypgoto[] =
{
    -271,  -271,  -271,  -271,  -271,    78,  -271,  -271,    82,   320,
     321,  -271,   -28,  -271,   188,  -271,     9,    70,    37,    57,
     -22,   144,   146,   147,   145,   148,  -271,  -271,  -135,  -271,
     -11,   -18,   -21,   -30,  -271,   138,  -140,  -271,  -271,  -271,
     -92,  -271,  -271,  -271,  -271,  -270,  -271,  -271,  -271,  -271,
    -271,  -271,   -17,  -271,    -3,  -271,  -271,  -271,  -271,  -271,
    -271,   114,  -271,   329,  -271,   131,  -267,  -271,     1,   -80
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -131
static const short int yytable[] =
{
     115,    84,   109,   122,   196,   246,   213,    86,   101,   192,
     190,   250,   120,   210,   315,   214,   203,   204,   152,   311,
     211,   316,   130,   111,   113,   320,   356,   123,   197,   117,
     223,   238,   255,   217,   193,   125,   126,   220,   313,   128,
     129,    95,   221,   336,   230,   142,   143,   144,   145,   168,
     339,   104,   191,    98,   146,   105,   279,   196,   280,   147,
     203,   204,   148,   169,   170,   207,    87,   102,   353,   292,
     205,   286,   293,   285,   127,   287,   193,   206,   201,   208,
     211,   363,   215,   357,   106,    89,     1,     2,     3,     4,
       5,   220,    88,   103,   194,     8,   288,   193,   212,    90,
     216,   107,    15,   225,    17,   227,   304,   228,   229,   237,
      20,   193,   124,   198,   205,   224,   111,   146,   218,   138,
     294,   302,   150,   193,   243,   151,   244,    26,    27,   231,
     182,   183,   184,   185,   296,    28,   251,   193,   297,   253,
     193,   193,   308,    91,    30,    31,    32,    33,   114,    92,
     176,   177,    35,   173,   174,   175,    36,    37,    38,    39,
     270,   271,   272,   273,   298,   178,   179,   193,   306,   108,
     331,   307,   332,   317,   116,   333,   293,   256,   257,   258,
     193,   118,   309,     1,     2,     3,     4,     5,   337,   312,
     193,   193,     8,   365,   348,   342,    90,   349,   193,   119,
     193,    17,   110,   110,   180,   181,   362,   186,   110,   193,
     261,   262,   263,   300,   110,   110,   171,   172,   110,   110,
     153,   154,   233,   234,   110,   110,   110,   110,   283,   239,
     240,   121,    28,   264,   265,   266,   267,   268,   269,   196,
      91,   259,   260,    32,    33,   187,    92,   153,   154,    35,
     188,   189,   155,   156,   157,   158,   159,   160,   161,   162,
     163,   164,   165,   195,   105,   219,   208,   226,   -29,   -30,
     241,   252,   314,   201,   247,   254,   322,   200,   324,   281,
     327,   193,   284,   290,   323,   295,   299,   234,   166,   310,
     321,   319,   325,   328,   329,   110,   335,   341,   153,   154,
     334,   343,   201,   155,   156,   157,   158,   159,   160,   161,
     162,   163,   164,   165,   338,   347,   364,   351,   366,   352,
     369,   305,   303,   354,   350,   374,   360,   201,    93,    94,
     274,   361,   355,   275,   277,   276,   373,   282,   278,   166,
     201,   367,   359,   112,   368,     0,   370,   371,   371,   301,
     289,   375,     0,     0,   376,   376,   110,   110,   110,   110,
     110,   110,   110,   110,   110,   110,   110,   110,   110,   110,
     110,   110,   110,   110,   110,   110,   110,   110,   110,     1,
       2,     3,   131,   132,     6,     0,     0,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
       0,     0,     0,    20,    21,    22,     0,    23,    24,     0,
       0,    25,     0,     0,     0,     0,     0,     0,     0,     0,
      26,    27,     0,     0,     0,     0,     0,     0,    28,     0,
       0,     0,     0,     0,     0,     0,   133,    30,    31,    32,
      33,     0,    34,   134,     0,    35,     0,     0,     0,    36,
      37,    38,    39,     0,     0,     1,     2,     3,     4,     5,
       6,     0,    40,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,     0,     0,     0,    20,
      21,    22,     0,    23,    24,     0,     0,    25,     0,     0,
       0,     0,     0,     0,     0,     0,    26,    27,     0,     0,
       0,     0,     0,     0,    28,     0,     0,     0,     0,     0,
       0,     0,    29,    30,    31,    32,    33,     0,    34,   232,
       0,    35,     0,     0,     0,    36,    37,    38,    39,     0,
       0,     1,     2,     3,     4,     5,     6,     0,    40,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,     0,     0,     0,    20,    21,    22,     0,    23,
      24,     0,     0,    25,     0,     0,     0,     0,     0,     0,
       0,     0,    26,    27,     0,     0,     0,     0,     0,     0,
      28,     0,     0,     0,     0,     0,     0,     0,    29,    30,
      31,    32,    33,     0,    34,   242,     0,    35,     0,     0,
       0,    36,    37,    38,    39,     0,     0,     1,     2,     3,
       4,     5,     6,     0,    40,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,     0,     0,
       0,    20,    21,    22,     0,    23,    24,     0,     0,    25,
       0,     0,     0,     0,     0,     0,     0,     0,    26,    27,
       0,     0,     0,     0,     0,     0,    28,     0,     0,     0,
       0,     0,     0,     0,    29,    30,    31,    32,    33,     0,
      34,   318,     0,    35,     0,     0,     0,    36,    37,    38,
      39,     0,     0,     1,     2,     3,     4,     5,     6,     0,
      40,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,     0,     0,     0,    20,    21,    22,
       0,    23,    24,     0,     0,    25,     0,     0,     0,     0,
       0,     0,     0,     0,    26,    27,     0,     0,     0,     0,
       0,     0,    28,     0,     0,     0,     0,     0,     0,     0,
      29,    30,    31,    32,    33,     0,    34,   340,     0,    35,
       0,     0,     0,    36,    37,    38,    39,     0,     0,     1,
       2,     3,     4,     5,     6,     0,    40,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
       0,     0,     0,    20,    21,    22,     0,    23,    24,     0,
       0,    25,     0,     0,     0,     0,     0,     0,     0,     0,
      26,    27,     0,     0,     0,     0,     0,     0,    28,     0,
       0,     0,     0,     0,     0,     0,    29,    30,    31,    32,
      33,     0,    34,     0,     0,    35,     0,     0,     0,    36,
      37,    38,    39,     0,     0,     1,     2,     3,     4,     5,
       6,     0,    40,     7,     8,     9,    10,    11,    90,    13,
     107,    15,    16,    17,    18,    19,     0,     0,     0,    20,
      21,    22,     0,    23,    24,     0,     0,    25,     0,     0,
       0,     0,     0,     0,     0,     0,    26,    27,     0,     0,
       0,     0,     0,     0,    28,     0,     0,     0,     0,     0,
       0,     0,    29,    30,    31,    32,    33,     0,    34,     0,
       0,    35,     0,     0,     0,    36,    37,    38,    39,     0,
       0,     0,     0,     0,     0,     0,  -130,     0,    40,  -130,
    -130,  -130,  -130,  -130,  -130,  -130,  -130,  -130,  -130,  -130,
    -130,  -130,  -130,  -130,  -130,  -130,  -130,  -130,  -130,  -130,
    -130,     0,     0,  -130,  -130,  -130,     0,  -130,  -130,     0,
       0,  -130,     0,     0,     0,     1,     2,     3,     4,     5,
       0,     0,     0,     0,     8,   199,     0,     0,    90,     0,
     107,    15,     0,    17,     0,     0,  -130,  -130,  -130,    20,
       0,     0,  -130,  -130,     0,     0,     0,     0,     0,     0,
       0,  -130,  -130,     0,     0,     0,    26,    27,     0,     0,
       0,     0,     0,     0,    28,     0,     0,     0,     0,     0,
       0,     0,    91,    30,    31,    32,    33,     0,    92,     0,
       0,    35,     0,     0,     0,    36,    37,    38,    39,     1,
       2,     3,     4,     5,     0,     0,     0,     0,     8,     0,
       0,     0,    90,     0,   107,    15,     0,    17,     0,     0,
       0,     0,     0,    20,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      26,    27,     0,     0,     0,     0,     0,     0,    28,     0,
       0,     0,     0,     0,     0,     0,    91,    30,    31,    32,
      33,     0,    92,     0,     0,    35,   245,     0,     0,    36,
      37,    38,    39,     1,     2,     3,     4,     5,     0,     0,
       0,     0,     8,     0,     0,     0,    90,     0,   107,    15,
       0,    17,     0,     0,     0,     0,     0,    20,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    26,    27,     0,     0,     0,     0,
       0,     0,    28,     0,     0,     0,     0,     0,     0,     0,
      91,    30,    31,    32,    33,   248,    92,     0,     0,    35,
       0,     0,     0,    36,    37,    38,    39,     1,     2,     3,
       4,     5,     0,     0,     0,     0,     8,     0,     0,     0,
      90,     0,   107,    15,     0,    17,     0,     0,     0,     0,
       0,    20,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    26,    27,
       0,     0,     0,     0,     0,     0,    28,     0,     0,     0,
       0,     0,     0,     0,    91,    30,    31,    32,    33,     0,
      92,     0,     0,    35,   330,     0,     0,    36,    37,    38,
      39,     1,     2,     3,     4,     5,     0,     0,     0,     0,
       8,     0,     0,     0,    12,     0,   107,    15,     0,    17,
       0,     0,     0,     0,     0,    20,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    26,    27,     0,     0,     0,     0,     0,     0,
      28,     0,     0,     0,     0,     0,     0,     0,    91,    30,
      31,    32,    33,     0,    92,     0,     0,    35,     0,     0,
       0,    36,    37,    38,    39,     1,     2,     3,     4,     5,
       0,     0,     0,     0,     8,     0,     0,     0,    90,     0,
     107,    15,     0,    17,     0,     0,     0,     0,     0,    20,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    26,    27,     0,     0,
       0,     0,     0,     0,    28,     0,     0,     0,     0,     0,
       0,     0,    91,    30,    31,    32,    33,     0,    92,     0,
       0,    35,     0,     0,     0,    36,    37,    38,    39
};

static const short int yycheck[] =
{
      18,     0,    13,    24,    84,   140,    98,     1,     1,     1,
      42,   146,    23,     1,   284,     1,     6,     7,    46,    25,
      68,   288,    33,    14,    15,   292,    10,     1,     1,    20,
       1,    65,   167,     1,    68,    26,    27,    60,    86,    30,
      31,    60,    65,   313,     1,    36,    37,    38,    39,    63,
     317,    60,    84,    60,    64,    64,   191,   137,   193,    69,
       6,     7,    72,    77,    78,    93,    60,    60,   338,    65,
      60,   211,    68,   208,    71,   215,    68,    67,    89,    85,
      68,   351,    68,    67,     1,    64,     3,     4,     5,     6,
       7,    60,    86,    86,    86,    12,    65,    68,    86,    16,
      86,    18,    19,   114,    21,   116,   241,   118,   119,   127,
      27,    68,    86,    86,    60,    86,   107,    64,    86,    68,
      65,    67,    69,    68,    68,    72,    70,    44,    45,    86,
      36,    37,    38,    39,    65,    52,   147,    68,    65,   150,
      68,    68,    70,    60,    61,    62,    63,    64,    64,    66,
      25,    26,    69,    46,    47,    48,    73,    74,    75,    76,
     182,   183,   184,   185,    65,    40,    41,    68,    65,    86,
     305,    68,   307,    65,    64,   310,    68,   168,   169,   170,
      68,    64,    70,     3,     4,     5,     6,     7,    65,   281,
      68,    68,    12,    71,   329,    65,    16,    65,    68,    64,
      68,    21,    14,    15,    79,    80,    65,    81,    20,    68,
     173,   174,   175,   234,    26,    27,    73,    74,    30,    31,
      44,    45,    33,    34,    36,    37,    38,    39,    25,    67,
      68,    66,    52,   176,   177,   178,   179,   180,   181,   319,
      60,   171,   172,    63,    64,    82,    66,    44,    45,    69,
      83,    43,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,     0,    64,    64,    85,    23,    71,    71,
      71,    60,   283,   284,    68,    60,   294,    89,   296,    60,
     298,    68,    86,    66,   295,    64,    64,    34,    85,    71,
      60,   290,    66,    60,    71,   107,    25,    24,    44,    45,
     311,     9,   313,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    86,    65,    65,    86,    71,   337,
      65,   243,   240,   341,   335,    67,   347,   338,     8,     8,
     186,   349,   343,   187,   189,   188,   366,   199,   190,    85,
     351,   358,   345,    14,   362,    -1,   364,   365,   366,   235,
     219,   369,    -1,    -1,   372,   373,   168,   169,   170,   171,
     172,   173,   174,   175,   176,   177,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,     3,
       4,     5,     6,     7,     8,    -1,    -1,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      -1,    -1,    -1,    27,    28,    29,    -1,    31,    32,    -1,
      -1,    35,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      44,    45,    -1,    -1,    -1,    -1,    -1,    -1,    52,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    60,    61,    62,    63,
      64,    -1,    66,    67,    -1,    69,    -1,    -1,    -1,    73,
      74,    75,    76,    -1,    -1,     3,     4,     5,     6,     7,
       8,    -1,    86,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    -1,    -1,    -1,    27,
      28,    29,    -1,    31,    32,    -1,    -1,    35,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    44,    45,    -1,    -1,
      -1,    -1,    -1,    -1,    52,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    60,    61,    62,    63,    64,    -1,    66,    67,
      -1,    69,    -1,    -1,    -1,    73,    74,    75,    76,    -1,
      -1,     3,     4,     5,     6,     7,     8,    -1,    86,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    -1,    -1,    -1,    27,    28,    29,    -1,    31,
      32,    -1,    -1,    35,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    44,    45,    -1,    -1,    -1,    -1,    -1,    -1,
      52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    60,    61,
      62,    63,    64,    -1,    66,    67,    -1,    69,    -1,    -1,
      -1,    73,    74,    75,    76,    -1,    -1,     3,     4,     5,
       6,     7,     8,    -1,    86,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    -1,    -1,
      -1,    27,    28,    29,    -1,    31,    32,    -1,    -1,    35,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    44,    45,
      -1,    -1,    -1,    -1,    -1,    -1,    52,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    60,    61,    62,    63,    64,    -1,
      66,    67,    -1,    69,    -1,    -1,    -1,    73,    74,    75,
      76,    -1,    -1,     3,     4,     5,     6,     7,     8,    -1,
      86,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    -1,    -1,    -1,    27,    28,    29,
      -1,    31,    32,    -1,    -1,    35,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    44,    45,    -1,    -1,    -1,    -1,
      -1,    -1,    52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      60,    61,    62,    63,    64,    -1,    66,    67,    -1,    69,
      -1,    -1,    -1,    73,    74,    75,    76,    -1,    -1,     3,
       4,     5,     6,     7,     8,    -1,    86,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      -1,    -1,    -1,    27,    28,    29,    -1,    31,    32,    -1,
      -1,    35,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      44,    45,    -1,    -1,    -1,    -1,    -1,    -1,    52,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    60,    61,    62,    63,
      64,    -1,    66,    -1,    -1,    69,    -1,    -1,    -1,    73,
      74,    75,    76,    -1,    -1,     3,     4,     5,     6,     7,
       8,    -1,    86,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    -1,    -1,    -1,    27,
      28,    29,    -1,    31,    32,    -1,    -1,    35,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    44,    45,    -1,    -1,
      -1,    -1,    -1,    -1,    52,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    60,    61,    62,    63,    64,    -1,    66,    -1,
      -1,    69,    -1,    -1,    -1,    73,    74,    75,    76,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,     0,    -1,    86,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    -1,    -1,    27,    28,    29,    -1,    31,    32,    -1,
      -1,    35,    -1,    -1,    -1,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    12,    13,    -1,    -1,    16,    -1,
      18,    19,    -1,    21,    -1,    -1,    60,    61,    62,    27,
      -1,    -1,    66,    67,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    75,    76,    -1,    -1,    -1,    44,    45,    -1,    -1,
      -1,    -1,    -1,    -1,    52,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    60,    61,    62,    63,    64,    -1,    66,    -1,
      -1,    69,    -1,    -1,    -1,    73,    74,    75,    76,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    12,    -1,
      -1,    -1,    16,    -1,    18,    19,    -1,    21,    -1,    -1,
      -1,    -1,    -1,    27,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      44,    45,    -1,    -1,    -1,    -1,    -1,    -1,    52,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    60,    61,    62,    63,
      64,    -1,    66,    -1,    -1,    69,    70,    -1,    -1,    73,
      74,    75,    76,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    12,    -1,    -1,    -1,    16,    -1,    18,    19,
      -1,    21,    -1,    -1,    -1,    -1,    -1,    27,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    44,    45,    -1,    -1,    -1,    -1,
      -1,    -1,    52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      60,    61,    62,    63,    64,    65,    66,    -1,    -1,    69,
      -1,    -1,    -1,    73,    74,    75,    76,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    12,    -1,    -1,    -1,
      16,    -1,    18,    19,    -1,    21,    -1,    -1,    -1,    -1,
      -1,    27,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    44,    45,
      -1,    -1,    -1,    -1,    -1,    -1,    52,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    60,    61,    62,    63,    64,    -1,
      66,    -1,    -1,    69,    70,    -1,    -1,    73,    74,    75,
      76,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      12,    -1,    -1,    -1,    16,    -1,    18,    19,    -1,    21,
      -1,    -1,    -1,    -1,    -1,    27,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    44,    45,    -1,    -1,    -1,    -1,    -1,    -1,
      52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    60,    61,
      62,    63,    64,    -1,    66,    -1,    -1,    69,    -1,    -1,
      -1,    73,    74,    75,    76,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    12,    -1,    -1,    -1,    16,    -1,
      18,    19,    -1,    21,    -1,    -1,    -1,    -1,    -1,    27,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    44,    45,    -1,    -1,
      -1,    -1,    -1,    -1,    52,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    60,    61,    62,    63,    64,    -1,    66,    -1,
      -1,    69,    -1,    -1,    -1,    73,    74,    75,    76
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,     3,     4,     5,     6,     7,     8,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      27,    28,    29,    31,    32,    35,    44,    45,    52,    60,
      61,    62,    63,    64,    66,    69,    73,    74,    75,    76,
      86,    88,    89,    90,    96,    97,    98,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   117,   118,   119,   121,   124,   128,   129,   130,
     131,   133,   134,   135,   136,   137,   143,   144,   145,   146,
     149,   150,   151,   154,   155,   156,     1,    60,    86,    64,
      16,    60,    66,    96,    97,    60,   122,   123,    60,   125,
     126,     1,    60,    86,    60,    64,     1,    18,    86,   117,
     101,   103,   150,   103,    64,   118,    64,   103,    64,    64,
     117,    66,   119,     1,    86,   103,   103,    71,   103,   103,
     117,     6,     7,    60,    67,    94,    95,   155,    68,    91,
      92,    93,   103,   103,   103,   103,    64,    69,    72,    99,
      69,    72,    99,    44,    45,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    85,   116,    63,    77,
      78,    73,    74,    46,    47,    48,    25,    26,    40,    41,
      79,    80,    36,    37,    38,    39,    81,    82,    83,    43,
      42,    84,     1,    68,    86,     0,   156,     1,    86,    13,
     101,   117,   132,     6,     7,    60,    67,    99,    85,   127,
       1,    68,    86,   127,     1,    68,    86,     1,    86,    64,
      60,    65,   152,     1,    86,   117,    23,   117,   117,   117,
       1,    86,    67,    33,    34,   147,   148,   118,    65,    67,
      68,    71,    67,    68,    70,    70,   115,    68,    65,   100,
     115,   117,    60,   117,    60,   115,   103,   103,   103,   104,
     104,   105,   105,   105,   106,   106,   106,   106,   106,   106,
     107,   107,   107,   107,   108,   109,   110,   111,   112,   115,
     115,    60,   122,    25,    86,   115,   123,   123,    65,   152,
      66,   153,    65,    68,    65,    64,    65,    65,    65,    64,
     119,   148,    67,    95,   115,    92,    65,    68,    70,    70,
      71,    25,   127,    86,   117,   132,   153,    65,    67,   155,
     153,    60,   118,   117,   118,    66,   138,   118,    60,    71,
      70,   115,   115,   115,   117,    25,   132,    65,    86,   153,
      67,    24,    65,     9,   139,   140,   141,    65,   115,    65,
     117,    86,   118,   132,   118,   117,    10,    67,   142,   141,
     119,   118,    65,   132,    65,    71,    71,   139,   118,    65,
     118,   118,   120,   120,    67,   118,   118
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
#line 169 "grammar.y"
    { (yyval.node) = new NullNode(); ;}
    break;

  case 3:
#line 170 "grammar.y"
    { (yyval.node) = new BooleanNode(true); ;}
    break;

  case 4:
#line 171 "grammar.y"
    { (yyval.node) = new BooleanNode(false); ;}
    break;

  case 5:
#line 172 "grammar.y"
    { (yyval.node) = new NumberNode((yyvsp[0].dval)); ;}
    break;

  case 6:
#line 173 "grammar.y"
    { (yyval.node) = new StringNode((yyvsp[0].ustr)); ;}
    break;

  case 7:
#line 174 "grammar.y"
    { Lexer *l = Lexer::curr();
                                     if (!l->scanRegExp()) YYABORT;
                                     (yyval.node) = new RegExpNode(l->pattern,l->flags);;}
    break;

  case 8:
#line 178 "grammar.y"
    { Lexer *l = Lexer::curr();
                                     if (!l->scanRegExp()) YYABORT;
                                     (yyval.node) = new RegExpNode(UString('=')+l->pattern,l->flags);;}
    break;

  case 9:
#line 184 "grammar.y"
    { (yyval.node) = new ThisNode(); ;}
    break;

  case 10:
#line 185 "grammar.y"
    { (yyval.node) = new ResolveNode(*(yyvsp[0].ident)); ;}
    break;

  case 13:
#line 188 "grammar.y"
    { (yyval.node) = new GroupNode((yyvsp[-1].node)); ;}
    break;

  case 14:
#line 189 "grammar.y"
    { (yyval.node) = new ObjectLiteralNode(); ;}
    break;

  case 15:
#line 190 "grammar.y"
    { (yyval.node) = new ObjectLiteralNode((yyvsp[-1].plist)); ;}
    break;

  case 16:
#line 191 "grammar.y"
    { (yyval.node) = new ObjectLiteralNode((yyvsp[-2].plist)); ;}
    break;

  case 17:
#line 195 "grammar.y"
    { (yyval.node) = new ArrayNode((yyvsp[-1].ival)); ;}
    break;

  case 18:
#line 196 "grammar.y"
    { (yyval.node) = new ArrayNode((yyvsp[-1].elm)); ;}
    break;

  case 19:
#line 197 "grammar.y"
    { (yyval.node) = new ArrayNode((yyvsp[-1].ival), (yyvsp[-3].elm)); ;}
    break;

  case 20:
#line 201 "grammar.y"
    { (yyval.elm) = new ElementNode((yyvsp[-1].ival), (yyvsp[0].node)); ;}
    break;

  case 21:
#line 203 "grammar.y"
    { (yyval.elm) = new ElementNode((yyvsp[-3].elm), (yyvsp[-1].ival), (yyvsp[0].node)); ;}
    break;

  case 22:
#line 207 "grammar.y"
    { (yyval.ival) = 0; ;}
    break;

  case 24:
#line 212 "grammar.y"
    { (yyval.ival) = 1; ;}
    break;

  case 25:
#line 213 "grammar.y"
    { (yyval.ival) = (yyvsp[-1].ival) + 1; ;}
    break;

  case 26:
#line 217 "grammar.y"
    { (yyval.plist) = new PropertyValueNode((yyvsp[-2].pnode), (yyvsp[0].node)); ;}
    break;

  case 27:
#line 219 "grammar.y"
    { (yyval.plist) = new PropertyValueNode((yyvsp[-2].pnode), (yyvsp[0].node), (yyvsp[-4].plist)); ;}
    break;

  case 28:
#line 223 "grammar.y"
    { (yyval.pnode) = new PropertyNode(*(yyvsp[0].ident)); ;}
    break;

  case 29:
#line 224 "grammar.y"
    { (yyval.pnode) = new PropertyNode(Identifier(*(yyvsp[0].ustr))); ;}
    break;

  case 30:
#line 225 "grammar.y"
    { (yyval.pnode) = new PropertyNode((yyvsp[0].dval)); ;}
    break;

  case 33:
#line 231 "grammar.y"
    { (yyval.node) = new AccessorNode1((yyvsp[-3].node), (yyvsp[-1].node)); ;}
    break;

  case 34:
#line 232 "grammar.y"
    { (yyval.node) = new AccessorNode2((yyvsp[-2].node), *(yyvsp[0].ident)); ;}
    break;

  case 35:
#line 233 "grammar.y"
    { (yyval.node) = new NewExprNode((yyvsp[-1].node), (yyvsp[0].args)); ;}
    break;

  case 37:
#line 238 "grammar.y"
    { (yyval.node) = new NewExprNode((yyvsp[0].node)); ;}
    break;

  case 38:
#line 242 "grammar.y"
    { (yyval.node) = new FunctionCallNode((yyvsp[-1].node), (yyvsp[0].args)); ;}
    break;

  case 39:
#line 243 "grammar.y"
    { (yyval.node) = new FunctionCallNode((yyvsp[-1].node), (yyvsp[0].args)); ;}
    break;

  case 40:
#line 244 "grammar.y"
    { (yyval.node) = new AccessorNode1((yyvsp[-3].node), (yyvsp[-1].node)); ;}
    break;

  case 41:
#line 245 "grammar.y"
    { (yyval.node) = new AccessorNode2((yyvsp[-2].node), *(yyvsp[0].ident)); ;}
    break;

  case 42:
#line 249 "grammar.y"
    { (yyval.args) = new ArgumentsNode(); ;}
    break;

  case 43:
#line 250 "grammar.y"
    { (yyval.args) = new ArgumentsNode((yyvsp[-1].alist)); ;}
    break;

  case 44:
#line 254 "grammar.y"
    { (yyval.alist) = new ArgumentListNode((yyvsp[0].node)); ;}
    break;

  case 45:
#line 255 "grammar.y"
    { (yyval.alist) = new ArgumentListNode((yyvsp[-2].alist), (yyvsp[0].node)); ;}
    break;

  case 49:
#line 265 "grammar.y"
    { (yyval.node) = new PostfixNode((yyvsp[-1].node), OpPlusPlus); ;}
    break;

  case 50:
#line 266 "grammar.y"
    { (yyval.node) = new PostfixNode((yyvsp[-1].node), OpMinusMinus); ;}
    break;

  case 52:
#line 271 "grammar.y"
    { (yyval.node) = new DeleteNode((yyvsp[0].node)); ;}
    break;

  case 53:
#line 272 "grammar.y"
    { (yyval.node) = new VoidNode((yyvsp[0].node)); ;}
    break;

  case 54:
#line 273 "grammar.y"
    { (yyval.node) = new TypeOfNode((yyvsp[0].node)); ;}
    break;

  case 55:
#line 274 "grammar.y"
    { (yyval.node) = new PrefixNode(OpPlusPlus, (yyvsp[0].node)); ;}
    break;

  case 56:
#line 275 "grammar.y"
    { (yyval.node) = new PrefixNode(OpPlusPlus, (yyvsp[0].node)); ;}
    break;

  case 57:
#line 276 "grammar.y"
    { (yyval.node) = new PrefixNode(OpMinusMinus, (yyvsp[0].node)); ;}
    break;

  case 58:
#line 277 "grammar.y"
    { (yyval.node) = new PrefixNode(OpMinusMinus, (yyvsp[0].node)); ;}
    break;

  case 59:
#line 278 "grammar.y"
    { (yyval.node) = new UnaryPlusNode((yyvsp[0].node)); ;}
    break;

  case 60:
#line 279 "grammar.y"
    { (yyval.node) = new NegateNode((yyvsp[0].node)); ;}
    break;

  case 61:
#line 280 "grammar.y"
    { (yyval.node) = new BitwiseNotNode((yyvsp[0].node)); ;}
    break;

  case 62:
#line 281 "grammar.y"
    { (yyval.node) = new LogicalNotNode((yyvsp[0].node)); ;}
    break;

  case 64:
#line 286 "grammar.y"
    { (yyval.node) = new MultNode((yyvsp[-2].node), (yyvsp[0].node), '*'); ;}
    break;

  case 65:
#line 287 "grammar.y"
    { (yyval.node) = new MultNode((yyvsp[-2].node), (yyvsp[0].node), '/'); ;}
    break;

  case 66:
#line 288 "grammar.y"
    { (yyval.node) = new MultNode((yyvsp[-2].node),(yyvsp[0].node),'%'); ;}
    break;

  case 68:
#line 293 "grammar.y"
    { (yyval.node) = AddNode::create((yyvsp[-2].node), (yyvsp[0].node), '+'); ;}
    break;

  case 69:
#line 294 "grammar.y"
    { (yyval.node) = AddNode::create((yyvsp[-2].node), (yyvsp[0].node), '-'); ;}
    break;

  case 71:
#line 299 "grammar.y"
    { (yyval.node) = new ShiftNode((yyvsp[-2].node), OpLShift, (yyvsp[0].node)); ;}
    break;

  case 72:
#line 300 "grammar.y"
    { (yyval.node) = new ShiftNode((yyvsp[-2].node), OpRShift, (yyvsp[0].node)); ;}
    break;

  case 73:
#line 301 "grammar.y"
    { (yyval.node) = new ShiftNode((yyvsp[-2].node), OpURShift, (yyvsp[0].node)); ;}
    break;

  case 75:
#line 307 "grammar.y"
    { (yyval.node) = new RelationalNode((yyvsp[-2].node), OpLess, (yyvsp[0].node)); ;}
    break;

  case 76:
#line 309 "grammar.y"
    { (yyval.node) = new RelationalNode((yyvsp[-2].node), OpGreater, (yyvsp[0].node)); ;}
    break;

  case 77:
#line 311 "grammar.y"
    { (yyval.node) = new RelationalNode((yyvsp[-2].node), OpLessEq, (yyvsp[0].node)); ;}
    break;

  case 78:
#line 313 "grammar.y"
    { (yyval.node) = new RelationalNode((yyvsp[-2].node), OpGreaterEq, (yyvsp[0].node)); ;}
    break;

  case 79:
#line 315 "grammar.y"
    { (yyval.node) = new RelationalNode((yyvsp[-2].node), OpInstanceOf, (yyvsp[0].node)); ;}
    break;

  case 80:
#line 317 "grammar.y"
    { (yyval.node) = new RelationalNode((yyvsp[-2].node), OpIn, (yyvsp[0].node)); ;}
    break;

  case 82:
#line 322 "grammar.y"
    { (yyval.node) = new EqualNode((yyvsp[-2].node), OpEqEq, (yyvsp[0].node)); ;}
    break;

  case 83:
#line 323 "grammar.y"
    { (yyval.node) = new EqualNode((yyvsp[-2].node), OpNotEq, (yyvsp[0].node)); ;}
    break;

  case 84:
#line 324 "grammar.y"
    { (yyval.node) = new EqualNode((yyvsp[-2].node), OpStrEq, (yyvsp[0].node)); ;}
    break;

  case 85:
#line 325 "grammar.y"
    { (yyval.node) = new EqualNode((yyvsp[-2].node), OpStrNEq, (yyvsp[0].node));;}
    break;

  case 87:
#line 330 "grammar.y"
    { (yyval.node) = new BitOperNode((yyvsp[-2].node), OpBitAnd, (yyvsp[0].node)); ;}
    break;

  case 89:
#line 335 "grammar.y"
    { (yyval.node) = new BitOperNode((yyvsp[-2].node), OpBitXOr, (yyvsp[0].node)); ;}
    break;

  case 91:
#line 340 "grammar.y"
    { (yyval.node) = new BitOperNode((yyvsp[-2].node), OpBitOr, (yyvsp[0].node)); ;}
    break;

  case 93:
#line 346 "grammar.y"
    { (yyval.node) = new BinaryLogicalNode((yyvsp[-2].node), OpAnd, (yyvsp[0].node)); ;}
    break;

  case 95:
#line 352 "grammar.y"
    { (yyval.node) = new BinaryLogicalNode((yyvsp[-2].node), OpOr, (yyvsp[0].node)); ;}
    break;

  case 97:
#line 358 "grammar.y"
    { (yyval.node) = new ConditionalNode((yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node)); ;}
    break;

  case 99:
#line 364 "grammar.y"
    { (yyval.node) = new AssignNode((yyvsp[-2].node), (yyvsp[-1].op), (yyvsp[0].node));;}
    break;

  case 100:
#line 368 "grammar.y"
    { (yyval.op) = OpEqual; ;}
    break;

  case 101:
#line 369 "grammar.y"
    { (yyval.op) = OpPlusEq; ;}
    break;

  case 102:
#line 370 "grammar.y"
    { (yyval.op) = OpMinusEq; ;}
    break;

  case 103:
#line 371 "grammar.y"
    { (yyval.op) = OpMultEq; ;}
    break;

  case 104:
#line 372 "grammar.y"
    { (yyval.op) = OpDivEq; ;}
    break;

  case 105:
#line 373 "grammar.y"
    { (yyval.op) = OpLShift; ;}
    break;

  case 106:
#line 374 "grammar.y"
    { (yyval.op) = OpRShift; ;}
    break;

  case 107:
#line 375 "grammar.y"
    { (yyval.op) = OpURShift; ;}
    break;

  case 108:
#line 376 "grammar.y"
    { (yyval.op) = OpAndEq; ;}
    break;

  case 109:
#line 377 "grammar.y"
    { (yyval.op) = OpXOrEq; ;}
    break;

  case 110:
#line 378 "grammar.y"
    { (yyval.op) = OpOrEq; ;}
    break;

  case 111:
#line 379 "grammar.y"
    { (yyval.op) = OpModEq; ;}
    break;

  case 113:
#line 384 "grammar.y"
    { (yyval.node) = new CommaNode((yyvsp[-2].node), (yyvsp[0].node)); ;}
    break;

  case 130:
#line 407 "grammar.y"
    { (yyval.stat) = new BlockNode(0); DBG((yyval.stat), (yylsp[0]), (yylsp[0])); ;}
    break;

  case 131:
#line 408 "grammar.y"
    { (yyval.stat) = new BlockNode((yyvsp[-1].srcs)); DBG((yyval.stat), (yylsp[0]), (yylsp[0])); ;}
    break;

  case 132:
#line 412 "grammar.y"
    { (yyval.slist) = new StatListNode((yyvsp[0].stat)); ;}
    break;

  case 133:
#line 413 "grammar.y"
    { (yyval.slist) = new StatListNode((yyvsp[-1].slist), (yyvsp[0].stat)); ;}
    break;

  case 134:
#line 417 "grammar.y"
    { (yyval.stat) = new VarStatementNode((yyvsp[-1].vlist));
                                      DBG((yyval.stat), (yylsp[-2]), (yylsp[0])); ;}
    break;

  case 135:
#line 419 "grammar.y"
    { if (automatic()) {
                                          (yyval.stat) = new VarStatementNode((yyvsp[-1].vlist));
					  DBG((yyval.stat), (yylsp[-2]), (yylsp[-1]));
                                        } else {
					  YYABORT;
					}
                                      ;}
    break;

  case 136:
#line 429 "grammar.y"
    { (yyval.vlist) = new VarDeclListNode((yyvsp[0].decl)); ;}
    break;

  case 137:
#line 431 "grammar.y"
    { (yyval.vlist) = new VarDeclListNode((yyvsp[-2].vlist), (yyvsp[0].decl)); ;}
    break;

  case 138:
#line 435 "grammar.y"
    { (yyval.decl) = new VarDeclNode(*(yyvsp[0].ident), 0, VarDeclNode::Variable); ;}
    break;

  case 139:
#line 436 "grammar.y"
    { (yyval.decl) = new VarDeclNode(*(yyvsp[-1].ident), (yyvsp[0].init), VarDeclNode::Variable); ;}
    break;

  case 140:
#line 440 "grammar.y"
    { (yyval.stat) = new VarStatementNode((yyvsp[-1].vlist));
                                      DBG((yyval.stat), (yylsp[-2]), (yylsp[0])); ;}
    break;

  case 141:
#line 442 "grammar.y"
    { if (automatic()) {
                                          (yyval.stat) = new VarStatementNode((yyvsp[-1].vlist));
					  DBG((yyval.stat), (yylsp[-2]), (yylsp[-1]));
                                        } else {
					  YYABORT;
					}
                                      ;}
    break;

  case 142:
#line 452 "grammar.y"
    { (yyval.vlist) = new VarDeclListNode((yyvsp[0].decl)); ;}
    break;

  case 143:
#line 454 "grammar.y"
    { (yyval.vlist) = new VarDeclListNode((yyvsp[-2].vlist), (yyvsp[0].decl)); ;}
    break;

  case 144:
#line 458 "grammar.y"
    { (yyval.decl) = new VarDeclNode(*(yyvsp[0].ident), 0, VarDeclNode::Constant); ;}
    break;

  case 145:
#line 459 "grammar.y"
    { (yyval.decl) = new VarDeclNode(*(yyvsp[-1].ident), (yyvsp[0].init), VarDeclNode::Constant); ;}
    break;

  case 146:
#line 463 "grammar.y"
    { (yyval.init) = new AssignExprNode((yyvsp[0].node)); ;}
    break;

  case 147:
#line 467 "grammar.y"
    { (yyval.stat) = new EmptyStatementNode(); DBG((yyval.stat), (yylsp[0]), (yylsp[0])); ;}
    break;

  case 148:
#line 471 "grammar.y"
    { (yyval.stat) = new ExprStatementNode((yyvsp[-1].node));
                                     DBG((yyval.stat), (yylsp[-1]), (yylsp[0])); ;}
    break;

  case 149:
#line 473 "grammar.y"
    { if (automatic()) {
                                       (yyval.stat) = new ExprStatementNode((yyvsp[-1].node));
				       DBG((yyval.stat), (yylsp[-1]), (yylsp[-1]));
                                     } else
				       YYABORT; ;}
    break;

  case 150:
#line 481 "grammar.y"
    { (yyval.stat) = new IfNode((yyvsp[-2].node),(yyvsp[0].stat),0);DBG((yyval.stat),(yylsp[-4]),(yylsp[-1])); ;}
    break;

  case 151:
#line 483 "grammar.y"
    { (yyval.stat) = new IfNode((yyvsp[-4].node),(yyvsp[-2].stat),(yyvsp[0].stat));DBG((yyval.stat),(yylsp[-6]),(yylsp[-3])); ;}
    break;

  case 152:
#line 487 "grammar.y"
    { (yyval.stat)=new DoWhileNode((yyvsp[-4].stat),(yyvsp[-1].node));DBG((yyval.stat),(yylsp[-5]),(yylsp[-3]));;}
    break;

  case 153:
#line 488 "grammar.y"
    { (yyval.stat) = new WhileNode((yyvsp[-2].node),(yyvsp[0].stat));DBG((yyval.stat),(yylsp[-4]),(yylsp[-1])); ;}
    break;

  case 154:
#line 490 "grammar.y"
    { (yyval.stat) = new ForNode((yyvsp[-6].node),(yyvsp[-4].node),(yyvsp[-2].node),(yyvsp[0].stat));
	                             DBG((yyval.stat),(yylsp[-8]),(yylsp[-1])); ;}
    break;

  case 155:
#line 493 "grammar.y"
    { (yyval.stat) = new ForNode((yyvsp[-6].vlist),(yyvsp[-4].node),(yyvsp[-2].node),(yyvsp[0].stat));
	                             DBG((yyval.stat),(yylsp[-9]),(yylsp[-1])); ;}
    break;

  case 156:
#line 496 "grammar.y"
    { (yyval.stat) = new ForInNode((yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].stat));
	                             DBG((yyval.stat),(yylsp[-6]),(yylsp[-1])); ;}
    break;

  case 157:
#line 499 "grammar.y"
    { (yyval.stat) = new ForInNode(*(yyvsp[-4].ident),0,(yyvsp[-2].node),(yyvsp[0].stat));
	                             DBG((yyval.stat),(yylsp[-7]),(yylsp[-1])); ;}
    break;

  case 158:
#line 502 "grammar.y"
    { (yyval.stat) = new ForInNode(*(yyvsp[-5].ident),(yyvsp[-4].init),(yyvsp[-2].node),(yyvsp[0].stat));
	                             DBG((yyval.stat),(yylsp[-8]),(yylsp[-1])); ;}
    break;

  case 159:
#line 507 "grammar.y"
    { (yyval.node) = 0; ;}
    break;

  case 161:
#line 512 "grammar.y"
    { (yyval.stat) = new ContinueNode(); DBG((yyval.stat),(yylsp[-1]),(yylsp[0])); ;}
    break;

  case 162:
#line 513 "grammar.y"
    { if (automatic()) {
                                       (yyval.stat) = new ContinueNode(); DBG((yyval.stat),(yylsp[-1]),(yylsp[0]));
                                     } else
				       YYABORT; ;}
    break;

  case 163:
#line 517 "grammar.y"
    { (yyval.stat) = new ContinueNode(*(yyvsp[-1].ident)); DBG((yyval.stat),(yylsp[-2]),(yylsp[0])); ;}
    break;

  case 164:
#line 518 "grammar.y"
    { if (automatic()) {
                                       (yyval.stat) = new ContinueNode(*(yyvsp[-1].ident));DBG((yyval.stat),(yylsp[-2]),(yylsp[-1]));
                                     } else
				       YYABORT; ;}
    break;

  case 165:
#line 525 "grammar.y"
    { (yyval.stat) = new BreakNode();DBG((yyval.stat),(yylsp[-1]),(yylsp[0])); ;}
    break;

  case 166:
#line 526 "grammar.y"
    { if (automatic()) {
                                       (yyval.stat) = new BreakNode(); DBG((yyval.stat),(yylsp[-1]),(yylsp[-1]));
                                     } else
				       YYABORT; ;}
    break;

  case 167:
#line 530 "grammar.y"
    { (yyval.stat) = new BreakNode(*(yyvsp[-1].ident)); DBG((yyval.stat),(yylsp[-2]),(yylsp[0])); ;}
    break;

  case 168:
#line 531 "grammar.y"
    { if (automatic()) {
                                       (yyval.stat) = new BreakNode(*(yyvsp[-1].ident)); DBG((yyval.stat),(yylsp[-2]),(yylsp[-1]));
                                     } else
				       YYABORT;
                                   ;}
    break;

  case 169:
#line 539 "grammar.y"
    { (yyval.stat) = new ReturnNode(0); DBG((yyval.stat),(yylsp[-1]),(yylsp[0])); ;}
    break;

  case 170:
#line 540 "grammar.y"
    { if (automatic()) {
                                       (yyval.stat) = new ReturnNode(0); DBG((yyval.stat),(yylsp[-1]),(yylsp[-1]));
                                     } else
				       YYABORT; ;}
    break;

  case 171:
#line 544 "grammar.y"
    { (yyval.stat) = new ReturnNode((yyvsp[-1].node)); DBG((yyval.stat),(yylsp[-2]),(yylsp[0])); ;}
    break;

  case 172:
#line 545 "grammar.y"
    { if (automatic()) {
                                       (yyval.stat) = new ReturnNode((yyvsp[-1].node)); DBG((yyval.stat),(yylsp[-2]),(yylsp[-2]));
                                     }
                                     else
				       YYABORT; ;}
    break;

  case 173:
#line 553 "grammar.y"
    { (yyval.stat) = new WithNode((yyvsp[-2].node),(yyvsp[0].stat));
                                     DBG((yyval.stat), (yylsp[-4]), (yylsp[-1])); ;}
    break;

  case 174:
#line 558 "grammar.y"
    { (yyval.stat) = new SwitchNode((yyvsp[-2].node), (yyvsp[0].cblk));
                                     DBG((yyval.stat), (yylsp[-4]), (yylsp[-1])); ;}
    break;

  case 175:
#line 563 "grammar.y"
    { (yyval.cblk) = new CaseBlockNode((yyvsp[-1].clist), 0, 0); ;}
    break;

  case 176:
#line 565 "grammar.y"
    { (yyval.cblk) = new CaseBlockNode((yyvsp[-3].clist), (yyvsp[-2].ccl), (yyvsp[-1].clist)); ;}
    break;

  case 177:
#line 569 "grammar.y"
    { (yyval.clist) = 0; ;}
    break;

  case 179:
#line 574 "grammar.y"
    { (yyval.clist) = new ClauseListNode((yyvsp[0].ccl)); ;}
    break;

  case 180:
#line 575 "grammar.y"
    { (yyval.clist) = new ClauseListNode((yyvsp[-1].clist), (yyvsp[0].ccl)); ;}
    break;

  case 181:
#line 579 "grammar.y"
    { (yyval.ccl) = new CaseClauseNode((yyvsp[-1].node)); ;}
    break;

  case 182:
#line 580 "grammar.y"
    { (yyval.ccl) = new CaseClauseNode((yyvsp[-2].node), (yyvsp[0].slist)); ;}
    break;

  case 183:
#line 584 "grammar.y"
    { (yyval.ccl) = new CaseClauseNode(0); ;}
    break;

  case 184:
#line 585 "grammar.y"
    { (yyval.ccl) = new CaseClauseNode(0, (yyvsp[0].slist)); ;}
    break;

  case 185:
#line 589 "grammar.y"
    { (yyvsp[0].stat)->pushLabel(*(yyvsp[-2].ident));
                                     (yyval.stat) = new LabelNode(*(yyvsp[-2].ident), (yyvsp[0].stat)); DBG((yyval.stat),(yylsp[-2]),(yylsp[-1])); ;}
    break;

  case 186:
#line 594 "grammar.y"
    { (yyval.stat) = new ThrowNode((yyvsp[-1].node)); DBG((yyval.stat),(yylsp[-2]),(yylsp[0])); ;}
    break;

  case 187:
#line 595 "grammar.y"
    { if (automatic()) {
                                       (yyval.stat) = new ThrowNode((yyvsp[-1].node)); DBG((yyval.stat),(yylsp[-2]),(yylsp[-2]));
                                     } else {
				       YYABORT; } ;}
    break;

  case 188:
#line 602 "grammar.y"
    { (yyval.stat) = new TryNode((yyvsp[-1].stat), (yyvsp[0].cnode)); DBG((yyval.stat),(yylsp[-2]),(yylsp[-2])); ;}
    break;

  case 189:
#line 603 "grammar.y"
    { (yyval.stat) = new TryNode((yyvsp[-1].stat), (yyvsp[0].fnode)); DBG((yyval.stat),(yylsp[-2]),(yylsp[-2])); ;}
    break;

  case 190:
#line 604 "grammar.y"
    { (yyval.stat) = new TryNode((yyvsp[-2].stat), (yyvsp[-1].cnode), (yyvsp[0].fnode)); DBG((yyval.stat),(yylsp[-3]),(yylsp[-3])); ;}
    break;

  case 191:
#line 608 "grammar.y"
    { (yyval.stat) = new EmptyStatementNode(); DBG((yyval.stat), (yylsp[-1]), (yylsp[0])); ;}
    break;

  case 192:
#line 609 "grammar.y"
    { if (automatic()) {
                                                (yyval.stat) = new EmptyStatementNode(); 
                                                DBG((yyval.stat), (yylsp[-1]), (yylsp[-1])); 
                                             } else {
                                                YYABORT; } ;}
    break;

  case 193:
#line 618 "grammar.y"
    { CatchNode *c; (yyval.cnode) = c = new CatchNode(*(yyvsp[-2].ident), (yyvsp[0].stat));
				     DBG(c,(yylsp[-4]),(yylsp[-1])); ;}
    break;

  case 194:
#line 623 "grammar.y"
    { FinallyNode *f; (yyval.fnode) = f = new FinallyNode((yyvsp[0].stat)); DBG(f,(yylsp[-1]),(yylsp[-1])); ;}
    break;

  case 196:
#line 629 "grammar.y"
    { (yyval.func) = (yyvsp[0].func); ;}
    break;

  case 197:
#line 633 "grammar.y"
    { (yyval.func) = new FuncDeclNode(*(yyvsp[-3].ident), (yyvsp[0].body)); DBG((yyval.func),(yylsp[-4]),(yylsp[-1])); ;}
    break;

  case 198:
#line 635 "grammar.y"
    { (yyval.func) = new FuncDeclNode(*(yyvsp[-4].ident), (yyvsp[-2].param), (yyvsp[0].body)); DBG((yyval.func),(yylsp[-5]),(yylsp[-1])); ;}
    break;

  case 199:
#line 639 "grammar.y"
    { (yyval.node) = new FuncExprNode((yyvsp[0].body)); ;}
    break;

  case 200:
#line 641 "grammar.y"
    { (yyval.node) = new FuncExprNode((yyvsp[-2].param), (yyvsp[0].body)); ;}
    break;

  case 201:
#line 646 "grammar.y"
    { (yyval.param) = new ParameterNode(*(yyvsp[0].ident)); ;}
    break;

  case 202:
#line 647 "grammar.y"
    { (yyval.param) = new ParameterNode((yyvsp[-2].param), *(yyvsp[0].ident)); ;}
    break;

  case 203:
#line 651 "grammar.y"
    { (yyval.body) = new FunctionBodyNode(0);
	                             DBG((yyval.body), (yylsp[-1]), (yylsp[0]));;}
    break;

  case 204:
#line 653 "grammar.y"
    { (yyval.body) = new FunctionBodyNode((yyvsp[-1].srcs));
	                             DBG((yyval.body), (yylsp[-2]), (yylsp[0]));;}
    break;

  case 205:
#line 658 "grammar.y"
    { (yyval.prog) = new FunctionBodyNode(0);
                                     (yyval.prog)->setLoc(0, 0, Parser::source);
                                     Parser::progNode = (yyval.prog); ;}
    break;

  case 206:
#line 661 "grammar.y"
    { (yyval.prog) = new FunctionBodyNode((yyvsp[0].srcs));
                                     Parser::progNode = (yyval.prog); ;}
    break;

  case 207:
#line 666 "grammar.y"
    { (yyval.srcs) = new SourceElementsNode((yyvsp[0].stat)); ;}
    break;

  case 208:
#line 667 "grammar.y"
    { (yyval.srcs) = new SourceElementsNode((yyvsp[-1].srcs), (yyvsp[0].stat)); ;}
    break;

  case 209:
#line 671 "grammar.y"
    { (yyval.stat) = (yyvsp[0].stat); ;}
    break;

  case 210:
#line 672 "grammar.y"
    { (yyval.stat) = (yyvsp[0].func); ;}
    break;


      default: break;
    }

/* Line 1126 of yacc.c.  */
#line 2811 "grammar.tab.c"

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


#line 675 "grammar.y"


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

