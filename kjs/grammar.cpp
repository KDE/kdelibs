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
/* Line 191 of yacc.c.  */
#line 284 "grammar.tab.c"
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
#line 308 "grammar.tab.c"

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
#define YYFINAL  191
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1380

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  86
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  69
/* YYNRULES -- Number of rules. */
#define YYNRULES  206
/* YYNRULES -- Number of states. */
#define YYNSTATES  372

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   316

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    75,     2,     2,     2,    77,    80,     2,
      63,    64,    76,    72,    69,    73,    71,    62,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    70,    85,
      78,    84,    79,    83,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    67,     2,    68,    81,     2,     2,     2,     2,     2,
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
static const unsigned short yyprhs[] =
{
       0,     0,     3,     5,     7,     9,    11,    13,    15,    17,
      19,    21,    23,    25,    29,    32,    36,    40,    44,    50,
      53,    58,    59,    61,    63,    66,    70,    76,    78,    80,
      82,    84,    86,    91,    95,    99,   101,   104,   107,   110,
     115,   119,   122,   126,   128,   132,   134,   136,   138,   141,
     144,   146,   149,   152,   155,   158,   161,   164,   167,   170,
     173,   176,   179,   181,   185,   189,   193,   195,   199,   203,
     205,   209,   213,   217,   219,   223,   227,   231,   235,   239,
     243,   245,   249,   253,   257,   261,   263,   267,   269,   273,
     275,   279,   281,   285,   287,   291,   293,   299,   301,   305,
     307,   309,   311,   313,   315,   317,   319,   321,   323,   325,
     327,   329,   331,   335,   337,   339,   341,   343,   345,   347,
     349,   351,   353,   355,   357,   359,   361,   363,   365,   368,
     372,   374,   377,   381,   385,   387,   391,   393,   396,   400,
     404,   406,   410,   412,   415,   418,   420,   423,   426,   432,
     440,   447,   453,   463,   474,   482,   491,   501,   502,   504,
     507,   510,   514,   518,   521,   524,   528,   532,   535,   538,
     542,   546,   552,   558,   562,   568,   569,   571,   573,   576,
     580,   585,   588,   592,   596,   600,   604,   608,   612,   617,
     623,   626,   628,   631,   637,   644,   649,   655,   657,   661,
     664,   668,   669,   671,   673,   676,   678
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short yyrhs[] =
{
     152,     0,    -1,     3,    -1,     4,    -1,     5,    -1,     7,
      -1,     6,    -1,    62,    -1,    51,    -1,    21,    -1,    59,
      -1,    87,    -1,    89,    -1,    63,   116,    64,    -1,    65,
      66,    -1,    65,    93,    66,    -1,    67,    91,    68,    -1,
      67,    90,    68,    -1,    67,    90,    69,    91,    68,    -1,
      91,   114,    -1,    90,    69,    91,   114,    -1,    -1,    92,
      -1,    69,    -1,    92,    69,    -1,    94,    70,   114,    -1,
      93,    69,    94,    70,   114,    -1,    59,    -1,     6,    -1,
       7,    -1,    88,    -1,   149,    -1,    95,    67,   116,    68,
      -1,    95,    71,    59,    -1,    12,    95,    98,    -1,    95,
      -1,    12,    96,    -1,    95,    98,    -1,    97,    98,    -1,
      97,    67,   116,    68,    -1,    97,    71,    59,    -1,    63,
      64,    -1,    63,    99,    64,    -1,   114,    -1,    99,    69,
     114,    -1,    96,    -1,    97,    -1,   100,    -1,   100,    43,
      -1,   100,    44,    -1,   101,    -1,    19,   102,    -1,    18,
     102,    -1,    27,   102,    -1,    43,   102,    -1,    60,   102,
      -1,    44,   102,    -1,    61,   102,    -1,    72,   102,    -1,
      73,   102,    -1,    74,   102,    -1,    75,   102,    -1,   102,
      -1,   103,    76,   102,    -1,   103,    62,   102,    -1,   103,
      77,   102,    -1,   103,    -1,   104,    72,   103,    -1,   104,
      73,   103,    -1,   104,    -1,   105,    45,   104,    -1,   105,
      46,   104,    -1,   105,    47,   104,    -1,   105,    -1,   106,
      78,   105,    -1,   106,    79,   105,    -1,   106,    39,   105,
      -1,   106,    40,   105,    -1,   106,    26,   105,    -1,   106,
      25,   105,    -1,   106,    -1,   107,    35,   106,    -1,   107,
      36,   106,    -1,   107,    37,   106,    -1,   107,    38,   106,
      -1,   107,    -1,   108,    80,   107,    -1,   108,    -1,   109,
      81,   108,    -1,   109,    -1,   110,    82,   109,    -1,   110,
      -1,   111,    42,   110,    -1,   111,    -1,   112,    41,   111,
      -1,   112,    -1,   112,    83,   114,    70,   114,    -1,   113,
      -1,   100,   115,   114,    -1,    84,    -1,    48,    -1,    49,
      -1,    50,    -1,    51,    -1,    52,    -1,    53,    -1,    54,
      -1,    55,    -1,    57,    -1,    58,    -1,    56,    -1,   114,
      -1,   116,    69,   114,    -1,   118,    -1,   120,    -1,   123,
      -1,   127,    -1,   128,    -1,   129,    -1,   130,    -1,   132,
      -1,   133,    -1,   134,    -1,   135,    -1,   136,    -1,   142,
      -1,   143,    -1,   144,    -1,    65,    66,    -1,    65,   153,
      66,    -1,   117,    -1,   119,   117,    -1,    13,   121,    85,
      -1,    13,   121,     1,    -1,   122,    -1,   121,    69,   122,
      -1,    59,    -1,    59,   126,    -1,    14,   124,    85,    -1,
      14,   124,     1,    -1,   125,    -1,   124,    69,   122,    -1,
      59,    -1,    59,   126,    -1,    84,   114,    -1,    85,    -1,
     116,    85,    -1,   116,     1,    -1,    20,    63,   116,    64,
     117,    -1,    20,    63,   116,    64,   117,    24,   117,    -1,
      22,   117,    23,    63,   116,    64,    -1,    23,    63,   116,
      64,   117,    -1,    11,    63,   131,    85,   131,    85,   131,
      64,   117,    -1,    11,    63,    13,   121,    85,   131,    85,
     131,    64,   117,    -1,    11,    63,   100,    25,   116,    64,
     117,    -1,    11,    63,    13,    59,    25,   116,    64,   117,
      -1,    11,    63,    13,    59,   126,    25,   116,    64,   117,
      -1,    -1,   116,    -1,    15,    85,    -1,    15,     1,    -1,
      15,    59,    85,    -1,    15,    59,     1,    -1,     8,    85,
      -1,     8,     1,    -1,     8,    59,    85,    -1,     8,    59,
       1,    -1,    17,    85,    -1,    17,     1,    -1,    17,   116,
      85,    -1,    17,   116,     1,    -1,    29,    63,   116,    64,
     117,    -1,    28,    63,   116,    64,   137,    -1,    65,   138,
      66,    -1,    65,   138,   141,   138,    66,    -1,    -1,   139,
      -1,   140,    -1,   139,   140,    -1,     9,   116,    70,    -1,
       9,   116,    70,   119,    -1,    10,    70,    -1,    10,    70,
     119,    -1,    59,    70,   117,    -1,    31,   116,    85,    -1,
      31,   116,     1,    -1,    32,   118,   145,    -1,    32,   118,
     146,    -1,    32,   118,   145,   146,    -1,    33,    63,    59,
      64,   118,    -1,    34,   118,    -1,   148,    -1,    18,   148,
      -1,    16,    59,    63,    64,   151,    -1,    16,    59,    63,
     150,    64,   151,    -1,    16,    63,    64,   151,    -1,    16,
      63,   150,    64,   151,    -1,    59,    -1,   150,    69,    59,
      -1,    65,    66,    -1,    65,   153,    66,    -1,    -1,   153,
      -1,   154,    -1,   153,   154,    -1,   117,    -1,   147,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   165,   165,   166,   167,   168,   169,   170,   173,   180,
     181,   182,   183,   184,   185,   186,   190,   191,   192,   196,
     197,   202,   203,   207,   208,   212,   213,   218,   219,   220,
     224,   225,   226,   227,   228,   232,   233,   237,   238,   239,
     240,   244,   245,   249,   250,   254,   255,   259,   260,   261,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   280,   281,   282,   283,   287,   288,   289,   293,
     294,   295,   296,   300,   301,   303,   305,   307,   309,   311,
     316,   317,   318,   319,   320,   324,   325,   329,   330,   334,
     335,   339,   340,   345,   346,   351,   352,   357,   358,   363,
     364,   365,   366,   367,   368,   369,   370,   371,   372,   373,
     374,   378,   379,   383,   384,   385,   386,   387,   388,   389,
     390,   391,   392,   393,   394,   395,   396,   397,   401,   402,
     406,   407,   411,   413,   423,   424,   429,   430,   434,   436,
     446,   447,   452,   453,   457,   461,   465,   467,   475,   476,
     481,   482,   483,   486,   489,   492,   495,   501,   502,   506,
     507,   511,   512,   519,   520,   524,   525,   533,   534,   538,
     539,   547,   552,   557,   558,   563,   564,   568,   569,   573,
     574,   578,   579,   583,   588,   589,   596,   597,   598,   602,
     607,   611,   613,   617,   618,   623,   624,   630,   631,   635,
     637,   642,   645,   650,   651,   655,   656
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
  "THIS", "DO", "WHILE", "ELSE", "IN", "INSTANCEOF", "TYPEOF", "SWITCH", 
  "WITH", "RESERVED", "THROW", "TRY", "CATCH", "FINALLY", "EQEQ", "NE", 
  "STREQ", "STRNEQ", "LE", "GE", "OR", "AND", "PLUSPLUS", "MINUSMINUS", 
  "LSHIFT", "RSHIFT", "URSHIFT", "PLUSEQUAL", "MINUSEQUAL", "MULTEQUAL", 
  "DIVEQUAL", "LSHIFTEQUAL", "RSHIFTEQUAL", "URSHIFTEQUAL", "ANDEQUAL", 
  "MODEQUAL", "XOREQUAL", "OREQUAL", "IDENT", "AUTOPLUSPLUS", 
  "AUTOMINUSMINUS", "'/'", "'('", "')'", "'{'", "'}'", "'['", "']'", 
  "','", "':'", "'.'", "'+'", "'-'", "'~'", "'!'", "'*'", "'%'", "'<'", 
  "'>'", "'&'", "'^'", "'|'", "'?'", "'='", "';'", "$accept", "Literal", 
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
static const unsigned short yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,    47,    40,    41,   123,   125,    91,    93,    44,
      58,    46,    43,    45,   126,    33,    42,    37,    60,    62,
      38,    94,   124,    63,    61,    59
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    86,    87,    87,    87,    87,    87,    87,    87,    88,
      88,    88,    88,    88,    88,    88,    89,    89,    89,    90,
      90,    91,    91,    92,    92,    93,    93,    94,    94,    94,
      95,    95,    95,    95,    95,    96,    96,    97,    97,    97,
      97,    98,    98,    99,    99,   100,   100,   101,   101,   101,
     102,   102,   102,   102,   102,   102,   102,   102,   102,   102,
     102,   102,   103,   103,   103,   103,   104,   104,   104,   105,
     105,   105,   105,   106,   106,   106,   106,   106,   106,   106,
     107,   107,   107,   107,   107,   108,   108,   109,   109,   110,
     110,   111,   111,   112,   112,   113,   113,   114,   114,   115,
     115,   115,   115,   115,   115,   115,   115,   115,   115,   115,
     115,   116,   116,   117,   117,   117,   117,   117,   117,   117,
     117,   117,   117,   117,   117,   117,   117,   117,   118,   118,
     119,   119,   120,   120,   121,   121,   122,   122,   123,   123,
     124,   124,   125,   125,   126,   127,   128,   128,   129,   129,
     130,   130,   130,   130,   130,   130,   130,   131,   131,   132,
     132,   132,   132,   133,   133,   133,   133,   134,   134,   134,
     134,   135,   136,   137,   137,   138,   138,   139,   139,   140,
     140,   141,   141,   142,   143,   143,   144,   144,   144,   145,
     146,   147,   147,   148,   148,   149,   149,   150,   150,   151,
     151,   152,   152,   153,   153,   154,   154
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     3,     2,     3,     3,     3,     5,     2,
       4,     0,     1,     1,     2,     3,     5,     1,     1,     1,
       1,     1,     4,     3,     3,     1,     2,     2,     2,     4,
       3,     2,     3,     1,     3,     1,     1,     1,     2,     2,
       1,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     1,     3,     3,     3,     1,     3,     3,     1,
       3,     3,     3,     1,     3,     3,     3,     3,     3,     3,
       1,     3,     3,     3,     3,     1,     3,     1,     3,     1,
       3,     1,     3,     1,     3,     1,     5,     1,     3,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     3,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     2,     3,
       1,     2,     3,     3,     1,     3,     1,     2,     3,     3,
       1,     3,     1,     2,     2,     1,     2,     2,     5,     7,
       6,     5,     9,    10,     7,     8,     9,     0,     1,     2,
       2,     3,     3,     2,     2,     3,     3,     2,     2,     3,
       3,     5,     5,     3,     5,     0,     1,     1,     2,     3,
       4,     2,     3,     3,     3,     3,     3,     3,     4,     5,
       2,     1,     2,     5,     6,     4,     5,     1,     3,     2,
       3,     0,     1,     1,     2,     1,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
     201,     2,     3,     4,     6,     5,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     9,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     8,    10,     0,
       0,     7,     0,     0,    21,     0,     0,     0,     0,   145,
      11,    30,    12,    35,    45,    46,    47,    50,    62,    66,
      69,    73,    80,    85,    87,    89,    91,    93,    95,    97,
     111,     0,   205,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   206,   191,
      31,     0,   202,   203,   164,     0,   163,   157,     0,    10,
       0,    35,    36,   136,     0,   134,   142,     0,   140,   160,
       0,   159,     0,     0,   168,     0,   167,     0,    47,    52,
     192,    51,     0,     0,     0,    53,     0,     0,     0,     0,
       0,    54,    56,     0,    55,    57,     0,     6,     5,    10,
      14,     0,     0,     0,    23,     0,     0,    22,    58,    59,
      60,    61,     0,     0,     0,    37,     0,     0,    38,    48,
      49,   100,   101,   102,   103,   104,   105,   106,   107,   110,
     108,   109,    99,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   147,     0,
     146,     1,   204,   166,   165,     0,    47,   158,     0,    28,
      29,    27,    14,    34,     0,   137,   133,     0,   132,   143,
     139,     0,   138,   162,   161,     0,   197,     0,     0,   170,
     169,     0,     0,     0,     0,     0,   185,   184,   128,     0,
       0,   186,   187,   183,    13,    15,     0,     0,   129,    17,
      21,    16,    19,    24,    41,     0,    43,     0,    33,     0,
      40,    98,    64,    63,    65,    67,    68,    70,    71,    72,
      79,    78,    76,    77,    74,    75,    81,    82,    83,    84,
      86,    88,    90,    92,    94,     0,   112,   136,     0,     0,
     157,   144,   135,   141,     0,     0,     0,   195,     0,     0,
       0,     0,     0,     0,     0,     0,   190,   188,     0,    25,
       0,    42,     0,    32,    39,     0,     0,   137,   157,     0,
       0,   193,     0,   199,     0,   196,   198,   148,     0,   151,
     175,   172,   171,     0,     0,    18,    20,    44,    96,     0,
       0,     0,     0,   157,   194,   200,     0,   150,     0,     0,
     176,   177,     0,    26,     0,     0,   157,   154,     0,   149,
       0,     0,   173,   175,   178,   189,   155,     0,     0,     0,
     179,   181,     0,   156,     0,   152,   130,   180,   182,   174,
     153,   131
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,    40,    41,    42,   135,   136,   137,   131,   132,    43,
      44,    45,   145,   245,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,   163,
      61,    62,    63,   367,    64,    94,    95,    65,    97,    98,
     205,    66,    67,    68,    69,   198,    70,    71,    72,    73,
      74,   321,   339,   340,   341,   353,    75,    76,    77,   231,
     232,    78,    79,    80,   218,   287,    81,   133,    83
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -265
static const short yypact[] =
{
     747,  -265,  -265,  -265,  -265,  -265,     5,   -45,   176,   -31,
      30,     6,    -3,    67,  1232,  1305,   -14,  -265,   822,    12,
    1305,    15,    74,  1305,   -13,  1305,  1305,  -265,    44,  1305,
    1305,  -265,  1305,   372,    84,  1305,  1305,  1305,  1305,  -265,
    -265,  -265,  -265,   -26,  -265,   -17,   251,  -265,  -265,   -19,
      50,   159,   131,   184,   115,   156,   158,   200,    -2,  -265,
    -265,    11,  -265,  -265,  -265,  -265,  -265,  -265,  -265,  -265,
    -265,  -265,  -265,  -265,  -265,  -265,  -265,  -265,  -265,  -265,
    -265,   244,   747,  -265,  -265,    10,  -265,   940,   182,  -265,
      92,   -26,  -265,   178,    18,  -265,   178,    28,  -265,  -265,
      16,  -265,   201,   -11,  -265,  1305,  -265,    39,    19,  -265,
    -265,  -265,  1305,   240,  1305,  -265,  1305,  1305,    46,   447,
     132,  -265,  -265,   822,  -265,  -265,    48,   195,   196,    44,
     905,    69,   197,   522,  -265,   107,  1013,   199,  -265,  -265,
    -265,  -265,  1086,  1305,   212,  -265,  1305,   214,  -265,  -265,
    -265,  -265,  -265,  -265,  -265,  -265,  -265,  -265,  -265,  -265,
    -265,  -265,  -265,  1305,  1305,  1305,  1305,  1305,  1305,  1305,
    1305,  1305,  1305,  1305,  1305,  1305,  1305,  1305,  1305,  1305,
    1305,  1305,  1305,  1305,  1305,  1305,  1305,  1305,  -265,  1305,
    -265,  -265,  -265,  -265,  -265,   218,   203,   209,   194,  -265,
    -265,  -265,  -265,  -265,  1305,  -265,  -265,   -31,  -265,  -265,
    -265,   -31,  -265,  -265,  -265,    57,  -265,   215,    56,  -265,
    -265,    85,   219,    86,    98,    99,  -265,  -265,  -265,   220,
     -13,   250,  -265,  -265,  -265,  -265,    60,  1305,  -265,  -265,
      84,  -265,  -265,  -265,  -265,   100,  -265,   143,  -265,   147,
    -265,  -265,  -265,  -265,  -265,   -19,   -19,    50,    50,    50,
     159,   159,   159,   159,   159,   159,   131,   131,   131,   131,
     184,   115,   156,   158,   200,   216,  -265,     9,   -43,  1305,
    1305,  -265,  -265,  -265,   215,   109,   597,  -265,   215,   226,
     822,  1305,   822,   224,   822,   231,  -265,  -265,   221,  -265,
    1159,  -265,  1305,  -265,  -265,  1305,  1305,   267,  1305,   120,
     225,  -265,   215,  -265,   672,  -265,  -265,   273,   121,  -265,
     302,  -265,  -265,   248,  1305,  -265,  -265,  -265,  -265,   129,
    1305,   228,   822,  1305,  -265,  -265,   822,  -265,  1305,    26,
     302,  -265,   -13,  -265,   822,   130,  1305,  -265,   252,  -265,
     164,   245,  -265,   302,  -265,  -265,  -265,   822,   253,   822,
     822,   822,   255,  -265,   822,  -265,  -265,   822,   822,  -265,
    -265,  -265
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -265,  -265,  -265,  -265,  -265,    82,  -265,  -265,    88,   317,
     319,  -265,    -7,  -265,   188,  -265,    -5,    33,    79,   -29,
      51,   148,   146,   149,   152,   145,  -265,  -265,  -128,  -265,
     -10,   -18,   -23,   -27,  -265,   137,   -20,  -265,  -265,  -265,
     -91,  -265,  -265,  -265,  -265,  -264,  -265,  -265,  -265,  -265,
    -265,  -265,   -15,  -265,     0,  -265,  -265,  -265,  -265,  -265,
     113,  -265,   331,  -265,   133,  -261,  -265,     2,   -78
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -129
static const short yytable[] =
{
     113,   120,    82,   107,   192,   209,    84,    99,   242,   109,
     111,   193,   188,   118,   246,   115,   310,   213,    87,   206,
     121,   122,   126,   311,   124,   125,   207,   315,    93,   210,
     138,   139,   140,   141,   306,   251,   351,   142,   148,   186,
     219,   143,   308,   164,   331,   144,   142,   226,   216,   112,
     146,   334,   119,   217,   147,   192,   102,   165,   166,   275,
     103,   276,   149,   150,    85,   100,   199,   200,   104,   348,
       1,     2,     3,     4,     5,   114,   281,   197,   116,     8,
     189,   187,   358,    88,   203,   105,    15,   207,    17,    96,
      86,   101,   352,   204,    20,   194,   190,   211,   199,   200,
     109,   214,   221,   208,   223,   233,   224,   225,   189,   299,
      25,    26,   234,   212,   123,   189,   216,   189,    27,   201,
     288,   284,   167,   168,   220,   289,    89,    29,    30,    31,
      32,   227,    90,   247,    34,   235,   249,   117,   236,    35,
      36,    37,    38,   260,   261,   262,   263,   264,   265,   290,
     292,   201,   106,   134,   189,   189,   172,   173,   202,   252,
     253,   254,   293,   294,   301,   229,   230,   189,   189,   302,
     174,   175,   326,   312,   327,   239,   240,   328,   289,     1,
       2,     3,     4,     5,   332,   337,   307,   282,     8,   189,
     189,   283,    88,   344,   357,   182,   343,    17,   189,   189,
     255,   256,   108,   108,   169,   170,   171,   296,   108,   176,
     177,   303,   189,   108,   108,   304,   189,   108,   108,   178,
     179,   180,   181,   108,   108,   108,   108,    27,   279,   266,
     267,   268,   269,   189,   360,    89,   192,   183,    31,    32,
     184,    90,   185,    34,   191,   103,   149,   150,   257,   258,
     259,   151,   152,   153,   154,   155,   156,   157,   158,   159,
     160,   161,   204,   222,   215,   -28,   -29,   237,   243,   309,
     197,   248,   317,   250,   319,   196,   322,   277,   189,   280,
     286,   318,   291,   295,   230,   316,   305,   162,   314,   320,
     323,   324,   330,   108,   149,   150,   329,   336,   197,   151,
     152,   153,   154,   155,   156,   157,   158,   159,   160,   161,
     333,   338,   342,   346,   347,   361,   359,   364,   349,   355,
     345,   369,   300,   197,   298,    91,   356,    92,   350,   271,
     270,   274,   278,   272,   368,   162,   197,   273,   362,   363,
     354,   365,   366,   366,   297,   110,   370,     0,   285,   371,
     371,     0,   108,   108,   108,   108,   108,   108,   108,   108,
     108,   108,   108,   108,   108,   108,   108,   108,   108,   108,
     108,   108,   108,   108,   108,     1,     2,     3,   127,   128,
       6,     0,     0,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,     0,     0,     0,    20,
      21,    22,     0,    23,    24,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    25,    26,     0,     0,     0,
       0,     0,     0,    27,     0,     0,     0,     0,     0,     0,
       0,   129,    29,    30,    31,    32,     0,    33,   130,    34,
       0,     0,     0,     0,    35,    36,    37,    38,     0,     0,
       1,     2,     3,     4,     5,     6,     0,    39,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,     0,     0,     0,    20,    21,    22,     0,    23,    24,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      25,    26,     0,     0,     0,     0,     0,     0,    27,     0,
       0,     0,     0,     0,     0,     0,    28,    29,    30,    31,
      32,     0,    33,   228,    34,     0,     0,     0,     0,    35,
      36,    37,    38,     0,     0,     1,     2,     3,     4,     5,
       6,     0,    39,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,     0,     0,     0,    20,
      21,    22,     0,    23,    24,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    25,    26,     0,     0,     0,
       0,     0,     0,    27,     0,     0,     0,     0,     0,     0,
       0,    28,    29,    30,    31,    32,     0,    33,   238,    34,
       0,     0,     0,     0,    35,    36,    37,    38,     0,     0,
       1,     2,     3,     4,     5,     6,     0,    39,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,     0,     0,     0,    20,    21,    22,     0,    23,    24,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      25,    26,     0,     0,     0,     0,     0,     0,    27,     0,
       0,     0,     0,     0,     0,     0,    28,    29,    30,    31,
      32,     0,    33,   313,    34,     0,     0,     0,     0,    35,
      36,    37,    38,     0,     0,     1,     2,     3,     4,     5,
       6,     0,    39,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,     0,     0,     0,    20,
      21,    22,     0,    23,    24,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    25,    26,     0,     0,     0,
       0,     0,     0,    27,     0,     0,     0,     0,     0,     0,
       0,    28,    29,    30,    31,    32,     0,    33,   335,    34,
       0,     0,     0,     0,    35,    36,    37,    38,     0,     0,
       1,     2,     3,     4,     5,     6,     0,    39,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,     0,     0,     0,    20,    21,    22,     0,    23,    24,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      25,    26,     0,     0,     0,     0,     0,     0,    27,     0,
       0,     0,     0,     0,     0,     0,    28,    29,    30,    31,
      32,     0,    33,     0,    34,     0,     0,     0,     0,    35,
      36,    37,    38,     0,     0,     1,     2,     3,     4,     5,
       6,     0,    39,     7,     8,     9,    10,    11,    88,    13,
     105,    15,    16,    17,    18,    19,     0,     0,     0,    20,
      21,    22,     0,    23,    24,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    25,    26,     0,     0,     0,
       0,     0,     0,    27,     0,     0,     0,     0,     0,     0,
       0,    28,    29,    30,    31,    32,     0,    33,     0,    34,
       0,     0,     0,     0,    35,    36,    37,    38,     0,     0,
       0,     0,     0,     0,     0,  -128,     0,    39,  -128,  -128,
    -128,  -128,  -128,  -128,  -128,  -128,  -128,  -128,  -128,  -128,
    -128,  -128,  -128,  -128,  -128,  -128,  -128,  -128,  -128,  -128,
       0,     0,  -128,  -128,  -128,     0,  -128,  -128,     0,     0,
       0,     0,     0,     1,     2,     3,     4,     5,     0,     0,
       0,     0,     8,   195,     0,     0,    88,     0,   105,    15,
       0,    17,     0,     0,  -128,  -128,  -128,    20,     0,     0,
    -128,  -128,     0,     0,     0,     0,     0,     0,     0,  -128,
    -128,     0,     0,    25,    26,     0,     0,     0,     0,     0,
       0,    27,     0,     0,     0,     0,     0,     0,     0,    89,
      29,    30,    31,    32,     0,    90,     0,    34,     0,     0,
       0,     0,    35,    36,    37,    38,     1,     2,     3,     4,
       5,     0,     0,     0,     0,     8,     0,     0,     0,    88,
       0,   105,    15,     0,    17,     0,     0,     0,     0,     0,
      20,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    25,    26,     0,     0,
       0,     0,     0,     0,    27,     0,     0,     0,     0,     0,
       0,     0,    89,    29,    30,    31,    32,     0,    90,     0,
      34,   241,     0,     0,     0,    35,    36,    37,    38,     1,
       2,     3,     4,     5,     0,     0,     0,     0,     8,     0,
       0,     0,    88,     0,   105,    15,     0,    17,     0,     0,
       0,     0,     0,    20,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    25,
      26,     0,     0,     0,     0,     0,     0,    27,     0,     0,
       0,     0,     0,     0,     0,    89,    29,    30,    31,    32,
     244,    90,     0,    34,     0,     0,     0,     0,    35,    36,
      37,    38,     1,     2,     3,     4,     5,     0,     0,     0,
       0,     8,     0,     0,     0,    88,     0,   105,    15,     0,
      17,     0,     0,     0,     0,     0,    20,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    25,    26,     0,     0,     0,     0,     0,     0,
      27,     0,     0,     0,     0,     0,     0,     0,    89,    29,
      30,    31,    32,     0,    90,     0,    34,   325,     0,     0,
       0,    35,    36,    37,    38,     1,     2,     3,     4,     5,
       0,     0,     0,     0,     8,     0,     0,     0,    12,     0,
     105,    15,     0,    17,     0,     0,     0,     0,     0,    20,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    25,    26,     0,     0,     0,
       0,     0,     0,    27,     0,     0,     0,     0,     0,     0,
       0,    89,    29,    30,    31,    32,     0,    90,     0,    34,
       0,     0,     0,     0,    35,    36,    37,    38,     1,     2,
       3,     4,     5,     0,     0,     0,     0,     8,     0,     0,
       0,    88,     0,   105,    15,     0,    17,     0,     0,     0,
       0,     0,    20,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    25,    26,
       0,     0,     0,     0,     0,     0,    27,     0,     0,     0,
       0,     0,     0,     0,    89,    29,    30,    31,    32,     0,
      90,     0,    34,     0,     0,     0,     0,    35,    36,    37,
      38
};

static const short yycheck[] =
{
      18,    24,     0,    13,    82,    96,     1,     1,   136,    14,
      15,     1,     1,    23,   142,    20,   280,     1,    63,     1,
      25,    26,    32,   284,    29,    30,    69,   288,    59,     1,
      35,    36,    37,    38,    25,   163,    10,    63,    45,    41,
       1,    67,    85,    62,   308,    71,    63,     1,    59,    63,
      67,   312,    65,    64,    71,   133,    59,    76,    77,   187,
      63,   189,    43,    44,    59,    59,     6,     7,     1,   333,
       3,     4,     5,     6,     7,    63,   204,    87,    63,    12,
      69,    83,   346,    16,    91,    18,    19,    69,    21,    59,
      85,    85,    66,    84,    27,    85,    85,    69,     6,     7,
     105,    85,   112,    85,   114,   123,   116,   117,    69,   237,
      43,    44,    64,    85,    70,    69,    59,    69,    51,    59,
      64,    64,    72,    73,    85,    69,    59,    60,    61,    62,
      63,    85,    65,   143,    67,    66,   146,    63,    69,    72,
      73,    74,    75,   172,   173,   174,   175,   176,   177,    64,
      64,    59,    85,    69,    69,    69,    25,    26,    66,   164,
     165,   166,    64,    64,    64,    33,    34,    69,    69,    69,
      39,    40,   300,    64,   302,    68,    69,   305,    69,     3,
       4,     5,     6,     7,    64,    64,   277,   207,    12,    69,
      69,   211,    16,    64,    64,    80,   324,    21,    69,    69,
     167,   168,    14,    15,    45,    46,    47,   230,    20,    78,
      79,    68,    69,    25,    26,    68,    69,    29,    30,    35,
      36,    37,    38,    35,    36,    37,    38,    51,    25,   178,
     179,   180,   181,    69,    70,    59,   314,    81,    62,    63,
      82,    65,    42,    67,     0,    63,    43,    44,   169,   170,
     171,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    84,    23,    63,    70,    70,    70,    69,   279,
     280,    59,   290,    59,   292,    87,   294,    59,    69,    85,
      65,   291,    63,    63,    34,    59,    70,    84,   286,    65,
      59,    70,    25,   105,    43,    44,   306,    24,   308,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      85,     9,    64,    85,   332,    70,    64,    64,   336,   342,
     330,    66,   240,   333,   236,     8,   344,     8,   338,   183,
     182,   186,   195,   184,   361,    84,   346,   185,   353,   357,
     340,   359,   360,   361,   231,    14,   364,    -1,   215,   367,
     368,    -1,   164,   165,   166,   167,   168,   169,   170,   171,
     172,   173,   174,   175,   176,   177,   178,   179,   180,   181,
     182,   183,   184,   185,   186,     3,     4,     5,     6,     7,
       8,    -1,    -1,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    -1,    -1,    -1,    27,
      28,    29,    -1,    31,    32,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    43,    44,    -1,    -1,    -1,
      -1,    -1,    -1,    51,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    59,    60,    61,    62,    63,    -1,    65,    66,    67,
      -1,    -1,    -1,    -1,    72,    73,    74,    75,    -1,    -1,
       3,     4,     5,     6,     7,     8,    -1,    85,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    -1,    -1,    -1,    27,    28,    29,    -1,    31,    32,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      43,    44,    -1,    -1,    -1,    -1,    -1,    -1,    51,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    59,    60,    61,    62,
      63,    -1,    65,    66,    67,    -1,    -1,    -1,    -1,    72,
      73,    74,    75,    -1,    -1,     3,     4,     5,     6,     7,
       8,    -1,    85,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    -1,    -1,    -1,    27,
      28,    29,    -1,    31,    32,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    43,    44,    -1,    -1,    -1,
      -1,    -1,    -1,    51,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    59,    60,    61,    62,    63,    -1,    65,    66,    67,
      -1,    -1,    -1,    -1,    72,    73,    74,    75,    -1,    -1,
       3,     4,     5,     6,     7,     8,    -1,    85,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    -1,    -1,    -1,    27,    28,    29,    -1,    31,    32,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      43,    44,    -1,    -1,    -1,    -1,    -1,    -1,    51,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    59,    60,    61,    62,
      63,    -1,    65,    66,    67,    -1,    -1,    -1,    -1,    72,
      73,    74,    75,    -1,    -1,     3,     4,     5,     6,     7,
       8,    -1,    85,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    -1,    -1,    -1,    27,
      28,    29,    -1,    31,    32,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    43,    44,    -1,    -1,    -1,
      -1,    -1,    -1,    51,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    59,    60,    61,    62,    63,    -1,    65,    66,    67,
      -1,    -1,    -1,    -1,    72,    73,    74,    75,    -1,    -1,
       3,     4,     5,     6,     7,     8,    -1,    85,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    -1,    -1,    -1,    27,    28,    29,    -1,    31,    32,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      43,    44,    -1,    -1,    -1,    -1,    -1,    -1,    51,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    59,    60,    61,    62,
      63,    -1,    65,    -1,    67,    -1,    -1,    -1,    -1,    72,
      73,    74,    75,    -1,    -1,     3,     4,     5,     6,     7,
       8,    -1,    85,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    -1,    -1,    -1,    27,
      28,    29,    -1,    31,    32,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    43,    44,    -1,    -1,    -1,
      -1,    -1,    -1,    51,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    59,    60,    61,    62,    63,    -1,    65,    -1,    67,
      -1,    -1,    -1,    -1,    72,    73,    74,    75,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,     0,    -1,    85,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      -1,    -1,    27,    28,    29,    -1,    31,    32,    -1,    -1,
      -1,    -1,    -1,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    12,    13,    -1,    -1,    16,    -1,    18,    19,
      -1,    21,    -1,    -1,    59,    60,    61,    27,    -1,    -1,
      65,    66,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    74,
      75,    -1,    -1,    43,    44,    -1,    -1,    -1,    -1,    -1,
      -1,    51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    59,
      60,    61,    62,    63,    -1,    65,    -1,    67,    -1,    -1,
      -1,    -1,    72,    73,    74,    75,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    12,    -1,    -1,    -1,    16,
      -1,    18,    19,    -1,    21,    -1,    -1,    -1,    -1,    -1,
      27,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    43,    44,    -1,    -1,
      -1,    -1,    -1,    -1,    51,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    59,    60,    61,    62,    63,    -1,    65,    -1,
      67,    68,    -1,    -1,    -1,    72,    73,    74,    75,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    12,    -1,
      -1,    -1,    16,    -1,    18,    19,    -1,    21,    -1,    -1,
      -1,    -1,    -1,    27,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    43,
      44,    -1,    -1,    -1,    -1,    -1,    -1,    51,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    59,    60,    61,    62,    63,
      64,    65,    -1,    67,    -1,    -1,    -1,    -1,    72,    73,
      74,    75,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    12,    -1,    -1,    -1,    16,    -1,    18,    19,    -1,
      21,    -1,    -1,    -1,    -1,    -1,    27,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    43,    44,    -1,    -1,    -1,    -1,    -1,    -1,
      51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    59,    60,
      61,    62,    63,    -1,    65,    -1,    67,    68,    -1,    -1,
      -1,    72,    73,    74,    75,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    12,    -1,    -1,    -1,    16,    -1,
      18,    19,    -1,    21,    -1,    -1,    -1,    -1,    -1,    27,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    43,    44,    -1,    -1,    -1,
      -1,    -1,    -1,    51,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    59,    60,    61,    62,    63,    -1,    65,    -1,    67,
      -1,    -1,    -1,    -1,    72,    73,    74,    75,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    12,    -1,    -1,
      -1,    16,    -1,    18,    19,    -1,    21,    -1,    -1,    -1,
      -1,    -1,    27,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    43,    44,
      -1,    -1,    -1,    -1,    -1,    -1,    51,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    59,    60,    61,    62,    63,    -1,
      65,    -1,    67,    -1,    -1,    -1,    -1,    72,    73,    74,
      75
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,     3,     4,     5,     6,     7,     8,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      27,    28,    29,    31,    32,    43,    44,    51,    59,    60,
      61,    62,    63,    65,    67,    72,    73,    74,    75,    85,
      87,    88,    89,    95,    96,    97,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   116,   117,   118,   120,   123,   127,   128,   129,   130,
     132,   133,   134,   135,   136,   142,   143,   144,   147,   148,
     149,   152,   153,   154,     1,    59,    85,    63,    16,    59,
      65,    95,    96,    59,   121,   122,    59,   124,   125,     1,
      59,    85,    59,    63,     1,    18,    85,   116,   100,   102,
     148,   102,    63,   117,    63,   102,    63,    63,   116,    65,
     118,   102,   102,    70,   102,   102,   116,     6,     7,    59,
      66,    93,    94,   153,    69,    90,    91,    92,   102,   102,
     102,   102,    63,    67,    71,    98,    67,    71,    98,    43,
      44,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    84,   115,    62,    76,    77,    72,    73,    45,
      46,    47,    25,    26,    39,    40,    78,    79,    35,    36,
      37,    38,    80,    81,    82,    42,    41,    83,     1,    69,
      85,     0,   154,     1,    85,    13,   100,   116,   131,     6,
       7,    59,    66,    98,    84,   126,     1,    69,    85,   126,
       1,    69,    85,     1,    85,    63,    59,    64,   150,     1,
      85,   116,    23,   116,   116,   116,     1,    85,    66,    33,
      34,   145,   146,   117,    64,    66,    69,    70,    66,    68,
      69,    68,   114,    69,    64,    99,   114,   116,    59,   116,
      59,   114,   102,   102,   102,   103,   103,   104,   104,   104,
     105,   105,   105,   105,   105,   105,   106,   106,   106,   106,
     107,   108,   109,   110,   111,   114,   114,    59,   121,    25,
      85,   114,   122,   122,    64,   150,    65,   151,    64,    69,
      64,    63,    64,    64,    64,    63,   118,   146,    94,   114,
      91,    64,    69,    68,    68,    70,    25,   126,    85,   116,
     131,   151,    64,    66,   153,   151,    59,   117,   116,   117,
      65,   137,   117,    59,    70,    68,   114,   114,   114,   116,
      25,   131,    64,    85,   151,    66,    24,    64,     9,   138,
     139,   140,    64,   114,    64,   116,    85,   117,   131,   117,
     116,    10,    66,   141,   140,   118,   117,    64,   131,    64,
      70,    70,   138,   117,    64,   117,   117,   119,   119,    66,
     117,   117
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
#line 165 "grammar.y"
    { yyval.node = new NullNode(); ;}
    break;

  case 3:
#line 166 "grammar.y"
    { yyval.node = new BooleanNode(true); ;}
    break;

  case 4:
#line 167 "grammar.y"
    { yyval.node = new BooleanNode(false); ;}
    break;

  case 5:
#line 168 "grammar.y"
    { yyval.node = new NumberNode(yyvsp[0].dval); ;}
    break;

  case 6:
#line 169 "grammar.y"
    { yyval.node = new StringNode(yyvsp[0].ustr); ;}
    break;

  case 7:
#line 170 "grammar.y"
    { Lexer *l = Lexer::curr();
                                     if (!l->scanRegExp()) YYABORT;
                                     yyval.node = new RegExpNode(l->pattern,l->flags);;}
    break;

  case 8:
#line 174 "grammar.y"
    { Lexer *l = Lexer::curr();
                                     if (!l->scanRegExp()) YYABORT;
                                     yyval.node = new RegExpNode(UString('=')+l->pattern,l->flags);;}
    break;

  case 9:
#line 180 "grammar.y"
    { yyval.node = new ThisNode(); ;}
    break;

  case 10:
#line 181 "grammar.y"
    { yyval.node = new ResolveNode(*yyvsp[0].ident); ;}
    break;

  case 13:
#line 184 "grammar.y"
    { yyval.node = new GroupNode(yyvsp[-1].node); ;}
    break;

  case 14:
#line 185 "grammar.y"
    { yyval.node = new ObjectLiteralNode(); ;}
    break;

  case 15:
#line 186 "grammar.y"
    { yyval.node = new ObjectLiteralNode(yyvsp[-1].plist); ;}
    break;

  case 16:
#line 190 "grammar.y"
    { yyval.node = new ArrayNode(yyvsp[-1].ival); ;}
    break;

  case 17:
#line 191 "grammar.y"
    { yyval.node = new ArrayNode(yyvsp[-1].elm); ;}
    break;

  case 18:
#line 192 "grammar.y"
    { yyval.node = new ArrayNode(yyvsp[-1].ival, yyvsp[-3].elm); ;}
    break;

  case 19:
#line 196 "grammar.y"
    { yyval.elm = new ElementNode(yyvsp[-1].ival, yyvsp[0].node); ;}
    break;

  case 20:
#line 198 "grammar.y"
    { yyval.elm = new ElementNode(yyvsp[-3].elm, yyvsp[-1].ival, yyvsp[0].node); ;}
    break;

  case 21:
#line 202 "grammar.y"
    { yyval.ival = 0; ;}
    break;

  case 23:
#line 207 "grammar.y"
    { yyval.ival = 1; ;}
    break;

  case 24:
#line 208 "grammar.y"
    { yyval.ival = yyvsp[-1].ival + 1; ;}
    break;

  case 25:
#line 212 "grammar.y"
    { yyval.plist = new PropertyValueNode(yyvsp[-2].pnode, yyvsp[0].node); ;}
    break;

  case 26:
#line 214 "grammar.y"
    { yyval.plist = new PropertyValueNode(yyvsp[-2].pnode, yyvsp[0].node, yyvsp[-4].plist); ;}
    break;

  case 27:
#line 218 "grammar.y"
    { yyval.pnode = new PropertyNode(*yyvsp[0].ident); ;}
    break;

  case 28:
#line 219 "grammar.y"
    { yyval.pnode = new PropertyNode(Identifier(*yyvsp[0].ustr)); ;}
    break;

  case 29:
#line 220 "grammar.y"
    { yyval.pnode = new PropertyNode(yyvsp[0].dval); ;}
    break;

  case 32:
#line 226 "grammar.y"
    { yyval.node = new AccessorNode1(yyvsp[-3].node, yyvsp[-1].node); ;}
    break;

  case 33:
#line 227 "grammar.y"
    { yyval.node = new AccessorNode2(yyvsp[-2].node, *yyvsp[0].ident); ;}
    break;

  case 34:
#line 228 "grammar.y"
    { yyval.node = new NewExprNode(yyvsp[-1].node, yyvsp[0].args); ;}
    break;

  case 36:
#line 233 "grammar.y"
    { yyval.node = new NewExprNode(yyvsp[0].node); ;}
    break;

  case 37:
#line 237 "grammar.y"
    { yyval.node = new FunctionCallNode(yyvsp[-1].node, yyvsp[0].args); ;}
    break;

  case 38:
#line 238 "grammar.y"
    { yyval.node = new FunctionCallNode(yyvsp[-1].node, yyvsp[0].args); ;}
    break;

  case 39:
#line 239 "grammar.y"
    { yyval.node = new AccessorNode1(yyvsp[-3].node, yyvsp[-1].node); ;}
    break;

  case 40:
#line 240 "grammar.y"
    { yyval.node = new AccessorNode2(yyvsp[-2].node, *yyvsp[0].ident); ;}
    break;

  case 41:
#line 244 "grammar.y"
    { yyval.args = new ArgumentsNode(); ;}
    break;

  case 42:
#line 245 "grammar.y"
    { yyval.args = new ArgumentsNode(yyvsp[-1].alist); ;}
    break;

  case 43:
#line 249 "grammar.y"
    { yyval.alist = new ArgumentListNode(yyvsp[0].node); ;}
    break;

  case 44:
#line 250 "grammar.y"
    { yyval.alist = new ArgumentListNode(yyvsp[-2].alist, yyvsp[0].node); ;}
    break;

  case 48:
#line 260 "grammar.y"
    { yyval.node = new PostfixNode(yyvsp[-1].node, OpPlusPlus); ;}
    break;

  case 49:
#line 261 "grammar.y"
    { yyval.node = new PostfixNode(yyvsp[-1].node, OpMinusMinus); ;}
    break;

  case 51:
#line 266 "grammar.y"
    { yyval.node = new DeleteNode(yyvsp[0].node); ;}
    break;

  case 52:
#line 267 "grammar.y"
    { yyval.node = new VoidNode(yyvsp[0].node); ;}
    break;

  case 53:
#line 268 "grammar.y"
    { yyval.node = new TypeOfNode(yyvsp[0].node); ;}
    break;

  case 54:
#line 269 "grammar.y"
    { yyval.node = new PrefixNode(OpPlusPlus, yyvsp[0].node); ;}
    break;

  case 55:
#line 270 "grammar.y"
    { yyval.node = new PrefixNode(OpPlusPlus, yyvsp[0].node); ;}
    break;

  case 56:
#line 271 "grammar.y"
    { yyval.node = new PrefixNode(OpMinusMinus, yyvsp[0].node); ;}
    break;

  case 57:
#line 272 "grammar.y"
    { yyval.node = new PrefixNode(OpMinusMinus, yyvsp[0].node); ;}
    break;

  case 58:
#line 273 "grammar.y"
    { yyval.node = new UnaryPlusNode(yyvsp[0].node); ;}
    break;

  case 59:
#line 274 "grammar.y"
    { yyval.node = new NegateNode(yyvsp[0].node); ;}
    break;

  case 60:
#line 275 "grammar.y"
    { yyval.node = new BitwiseNotNode(yyvsp[0].node); ;}
    break;

  case 61:
#line 276 "grammar.y"
    { yyval.node = new LogicalNotNode(yyvsp[0].node); ;}
    break;

  case 63:
#line 281 "grammar.y"
    { yyval.node = new MultNode(yyvsp[-2].node, yyvsp[0].node, '*'); ;}
    break;

  case 64:
#line 282 "grammar.y"
    { yyval.node = new MultNode(yyvsp[-2].node, yyvsp[0].node, '/'); ;}
    break;

  case 65:
#line 283 "grammar.y"
    { yyval.node = new MultNode(yyvsp[-2].node,yyvsp[0].node,'%'); ;}
    break;

  case 67:
#line 288 "grammar.y"
    { yyval.node = AddNode::create(yyvsp[-2].node, yyvsp[0].node, '+'); ;}
    break;

  case 68:
#line 289 "grammar.y"
    { yyval.node = AddNode::create(yyvsp[-2].node, yyvsp[0].node, '-'); ;}
    break;

  case 70:
#line 294 "grammar.y"
    { yyval.node = new ShiftNode(yyvsp[-2].node, OpLShift, yyvsp[0].node); ;}
    break;

  case 71:
#line 295 "grammar.y"
    { yyval.node = new ShiftNode(yyvsp[-2].node, OpRShift, yyvsp[0].node); ;}
    break;

  case 72:
#line 296 "grammar.y"
    { yyval.node = new ShiftNode(yyvsp[-2].node, OpURShift, yyvsp[0].node); ;}
    break;

  case 74:
#line 302 "grammar.y"
    { yyval.node = new RelationalNode(yyvsp[-2].node, OpLess, yyvsp[0].node); ;}
    break;

  case 75:
#line 304 "grammar.y"
    { yyval.node = new RelationalNode(yyvsp[-2].node, OpGreater, yyvsp[0].node); ;}
    break;

  case 76:
#line 306 "grammar.y"
    { yyval.node = new RelationalNode(yyvsp[-2].node, OpLessEq, yyvsp[0].node); ;}
    break;

  case 77:
#line 308 "grammar.y"
    { yyval.node = new RelationalNode(yyvsp[-2].node, OpGreaterEq, yyvsp[0].node); ;}
    break;

  case 78:
#line 310 "grammar.y"
    { yyval.node = new RelationalNode(yyvsp[-2].node, OpInstanceOf, yyvsp[0].node); ;}
    break;

  case 79:
#line 312 "grammar.y"
    { yyval.node = new RelationalNode(yyvsp[-2].node, OpIn, yyvsp[0].node); ;}
    break;

  case 81:
#line 317 "grammar.y"
    { yyval.node = new EqualNode(yyvsp[-2].node, OpEqEq, yyvsp[0].node); ;}
    break;

  case 82:
#line 318 "grammar.y"
    { yyval.node = new EqualNode(yyvsp[-2].node, OpNotEq, yyvsp[0].node); ;}
    break;

  case 83:
#line 319 "grammar.y"
    { yyval.node = new EqualNode(yyvsp[-2].node, OpStrEq, yyvsp[0].node); ;}
    break;

  case 84:
#line 320 "grammar.y"
    { yyval.node = new EqualNode(yyvsp[-2].node, OpStrNEq, yyvsp[0].node);;}
    break;

  case 86:
#line 325 "grammar.y"
    { yyval.node = new BitOperNode(yyvsp[-2].node, OpBitAnd, yyvsp[0].node); ;}
    break;

  case 88:
#line 330 "grammar.y"
    { yyval.node = new BitOperNode(yyvsp[-2].node, OpBitXOr, yyvsp[0].node); ;}
    break;

  case 90:
#line 335 "grammar.y"
    { yyval.node = new BitOperNode(yyvsp[-2].node, OpBitOr, yyvsp[0].node); ;}
    break;

  case 92:
#line 341 "grammar.y"
    { yyval.node = new BinaryLogicalNode(yyvsp[-2].node, OpAnd, yyvsp[0].node); ;}
    break;

  case 94:
#line 347 "grammar.y"
    { yyval.node = new BinaryLogicalNode(yyvsp[-2].node, OpOr, yyvsp[0].node); ;}
    break;

  case 96:
#line 353 "grammar.y"
    { yyval.node = new ConditionalNode(yyvsp[-4].node, yyvsp[-2].node, yyvsp[0].node); ;}
    break;

  case 98:
#line 359 "grammar.y"
    { yyval.node = new AssignNode(yyvsp[-2].node, yyvsp[-1].op, yyvsp[0].node);;}
    break;

  case 99:
#line 363 "grammar.y"
    { yyval.op = OpEqual; ;}
    break;

  case 100:
#line 364 "grammar.y"
    { yyval.op = OpPlusEq; ;}
    break;

  case 101:
#line 365 "grammar.y"
    { yyval.op = OpMinusEq; ;}
    break;

  case 102:
#line 366 "grammar.y"
    { yyval.op = OpMultEq; ;}
    break;

  case 103:
#line 367 "grammar.y"
    { yyval.op = OpDivEq; ;}
    break;

  case 104:
#line 368 "grammar.y"
    { yyval.op = OpLShift; ;}
    break;

  case 105:
#line 369 "grammar.y"
    { yyval.op = OpRShift; ;}
    break;

  case 106:
#line 370 "grammar.y"
    { yyval.op = OpURShift; ;}
    break;

  case 107:
#line 371 "grammar.y"
    { yyval.op = OpAndEq; ;}
    break;

  case 108:
#line 372 "grammar.y"
    { yyval.op = OpXOrEq; ;}
    break;

  case 109:
#line 373 "grammar.y"
    { yyval.op = OpOrEq; ;}
    break;

  case 110:
#line 374 "grammar.y"
    { yyval.op = OpModEq; ;}
    break;

  case 112:
#line 379 "grammar.y"
    { yyval.node = new CommaNode(yyvsp[-2].node, yyvsp[0].node); ;}
    break;

  case 128:
#line 401 "grammar.y"
    { yyval.stat = new BlockNode(0); DBG(yyval.stat, yylsp[0], yylsp[0]); ;}
    break;

  case 129:
#line 402 "grammar.y"
    { yyval.stat = new BlockNode(yyvsp[-1].srcs); DBG(yyval.stat, yylsp[0], yylsp[0]); ;}
    break;

  case 130:
#line 406 "grammar.y"
    { yyval.slist = new StatListNode(yyvsp[0].stat); ;}
    break;

  case 131:
#line 407 "grammar.y"
    { yyval.slist = new StatListNode(yyvsp[-1].slist, yyvsp[0].stat); ;}
    break;

  case 132:
#line 411 "grammar.y"
    { yyval.stat = new VarStatementNode(yyvsp[-1].vlist);
                                      DBG(yyval.stat, yylsp[-2], yylsp[0]); ;}
    break;

  case 133:
#line 413 "grammar.y"
    { if (automatic()) {
                                          yyval.stat = new VarStatementNode(yyvsp[-1].vlist);
					  DBG(yyval.stat, yylsp[-2], yylsp[-1]);
                                        } else {
					  YYABORT;
					}
                                      ;}
    break;

  case 134:
#line 423 "grammar.y"
    { yyval.vlist = new VarDeclListNode(yyvsp[0].decl); ;}
    break;

  case 135:
#line 425 "grammar.y"
    { yyval.vlist = new VarDeclListNode(yyvsp[-2].vlist, yyvsp[0].decl); ;}
    break;

  case 136:
#line 429 "grammar.y"
    { yyval.decl = new VarDeclNode(*yyvsp[0].ident, 0, VarDeclNode::Variable); ;}
    break;

  case 137:
#line 430 "grammar.y"
    { yyval.decl = new VarDeclNode(*yyvsp[-1].ident, yyvsp[0].init, VarDeclNode::Variable); ;}
    break;

  case 138:
#line 434 "grammar.y"
    { yyval.stat = new VarStatementNode(yyvsp[-1].vlist);
                                      DBG(yyval.stat, yylsp[-2], yylsp[0]); ;}
    break;

  case 139:
#line 436 "grammar.y"
    { if (automatic()) {
                                          yyval.stat = new VarStatementNode(yyvsp[-1].vlist);
					  DBG(yyval.stat, yylsp[-2], yylsp[-1]);
                                        } else {
					  YYABORT;
					}
                                      ;}
    break;

  case 140:
#line 446 "grammar.y"
    { yyval.vlist = new VarDeclListNode(yyvsp[0].decl); ;}
    break;

  case 141:
#line 448 "grammar.y"
    { yyval.vlist = new VarDeclListNode(yyvsp[-2].vlist, yyvsp[0].decl); ;}
    break;

  case 142:
#line 452 "grammar.y"
    { yyval.decl = new VarDeclNode(*yyvsp[0].ident, 0, VarDeclNode::Constant); ;}
    break;

  case 143:
#line 453 "grammar.y"
    { yyval.decl = new VarDeclNode(*yyvsp[-1].ident, yyvsp[0].init, VarDeclNode::Constant); ;}
    break;

  case 144:
#line 457 "grammar.y"
    { yyval.init = new AssignExprNode(yyvsp[0].node); ;}
    break;

  case 145:
#line 461 "grammar.y"
    { yyval.stat = new EmptyStatementNode(); DBG(yyval.stat, yylsp[0], yylsp[0]); ;}
    break;

  case 146:
#line 465 "grammar.y"
    { yyval.stat = new ExprStatementNode(yyvsp[-1].node);
                                     DBG(yyval.stat, yylsp[-1], yylsp[0]); ;}
    break;

  case 147:
#line 467 "grammar.y"
    { if (automatic()) {
                                       yyval.stat = new ExprStatementNode(yyvsp[-1].node);
				       DBG(yyval.stat, yylsp[-1], yylsp[-1]);
                                     } else
				       YYABORT; ;}
    break;

  case 148:
#line 475 "grammar.y"
    { yyval.stat = new IfNode(yyvsp[-2].node,yyvsp[0].stat,0);DBG(yyval.stat,yylsp[-4],yylsp[-1]); ;}
    break;

  case 149:
#line 477 "grammar.y"
    { yyval.stat = new IfNode(yyvsp[-4].node,yyvsp[-2].stat,yyvsp[0].stat);DBG(yyval.stat,yylsp[-6],yylsp[-3]); ;}
    break;

  case 150:
#line 481 "grammar.y"
    { yyval.stat=new DoWhileNode(yyvsp[-4].stat,yyvsp[-1].node);DBG(yyval.stat,yylsp[-5],yylsp[-3]);;}
    break;

  case 151:
#line 482 "grammar.y"
    { yyval.stat = new WhileNode(yyvsp[-2].node,yyvsp[0].stat);DBG(yyval.stat,yylsp[-4],yylsp[-1]); ;}
    break;

  case 152:
#line 484 "grammar.y"
    { yyval.stat = new ForNode(yyvsp[-6].node,yyvsp[-4].node,yyvsp[-2].node,yyvsp[0].stat);
	                             DBG(yyval.stat,yylsp[-8],yylsp[-1]); ;}
    break;

  case 153:
#line 487 "grammar.y"
    { yyval.stat = new ForNode(yyvsp[-6].vlist,yyvsp[-4].node,yyvsp[-2].node,yyvsp[0].stat);
	                             DBG(yyval.stat,yylsp[-9],yylsp[-1]); ;}
    break;

  case 154:
#line 490 "grammar.y"
    { yyval.stat = new ForInNode(yyvsp[-4].node, yyvsp[-2].node, yyvsp[0].stat);
	                             DBG(yyval.stat,yylsp[-6],yylsp[-1]); ;}
    break;

  case 155:
#line 493 "grammar.y"
    { yyval.stat = new ForInNode(*yyvsp[-4].ident,0,yyvsp[-2].node,yyvsp[0].stat);
	                             DBG(yyval.stat,yylsp[-7],yylsp[-1]); ;}
    break;

  case 156:
#line 496 "grammar.y"
    { yyval.stat = new ForInNode(*yyvsp[-5].ident,yyvsp[-4].init,yyvsp[-2].node,yyvsp[0].stat);
	                             DBG(yyval.stat,yylsp[-8],yylsp[-1]); ;}
    break;

  case 157:
#line 501 "grammar.y"
    { yyval.node = 0; ;}
    break;

  case 159:
#line 506 "grammar.y"
    { yyval.stat = new ContinueNode(); DBG(yyval.stat,yylsp[-1],yylsp[0]); ;}
    break;

  case 160:
#line 507 "grammar.y"
    { if (automatic()) {
                                       yyval.stat = new ContinueNode(); DBG(yyval.stat,yylsp[-1],yylsp[0]);
                                     } else
				       YYABORT; ;}
    break;

  case 161:
#line 511 "grammar.y"
    { yyval.stat = new ContinueNode(*yyvsp[-1].ident); DBG(yyval.stat,yylsp[-2],yylsp[0]); ;}
    break;

  case 162:
#line 512 "grammar.y"
    { if (automatic()) {
                                       yyval.stat = new ContinueNode(*yyvsp[-1].ident);DBG(yyval.stat,yylsp[-2],yylsp[-1]);
                                     } else
				       YYABORT; ;}
    break;

  case 163:
#line 519 "grammar.y"
    { yyval.stat = new BreakNode();DBG(yyval.stat,yylsp[-1],yylsp[0]); ;}
    break;

  case 164:
#line 520 "grammar.y"
    { if (automatic()) {
                                       yyval.stat = new BreakNode(); DBG(yyval.stat,yylsp[-1],yylsp[-1]);
                                     } else
				       YYABORT; ;}
    break;

  case 165:
#line 524 "grammar.y"
    { yyval.stat = new BreakNode(*yyvsp[-1].ident); DBG(yyval.stat,yylsp[-2],yylsp[0]); ;}
    break;

  case 166:
#line 525 "grammar.y"
    { if (automatic()) {
                                       yyval.stat = new BreakNode(*yyvsp[-1].ident); DBG(yyval.stat,yylsp[-2],yylsp[-1]);
                                     } else
				       YYABORT;
                                   ;}
    break;

  case 167:
#line 533 "grammar.y"
    { yyval.stat = new ReturnNode(0); DBG(yyval.stat,yylsp[-1],yylsp[0]); ;}
    break;

  case 168:
#line 534 "grammar.y"
    { if (automatic()) {
                                       yyval.stat = new ReturnNode(0); DBG(yyval.stat,yylsp[-1],yylsp[-1]);
                                     } else
				       YYABORT; ;}
    break;

  case 169:
#line 538 "grammar.y"
    { yyval.stat = new ReturnNode(yyvsp[-1].node); DBG(yyval.stat,yylsp[-2],yylsp[0]); ;}
    break;

  case 170:
#line 539 "grammar.y"
    { if (automatic()) {
                                       yyval.stat = new ReturnNode(yyvsp[-1].node); DBG(yyval.stat,yylsp[-2],yylsp[-2]);
                                     }
                                     else
				       YYABORT; ;}
    break;

  case 171:
#line 547 "grammar.y"
    { yyval.stat = new WithNode(yyvsp[-2].node,yyvsp[0].stat);
                                     DBG(yyval.stat, yylsp[-4], yylsp[-1]); ;}
    break;

  case 172:
#line 552 "grammar.y"
    { yyval.stat = new SwitchNode(yyvsp[-2].node, yyvsp[0].cblk);
                                     DBG(yyval.stat, yylsp[-4], yylsp[-1]); ;}
    break;

  case 173:
#line 557 "grammar.y"
    { yyval.cblk = new CaseBlockNode(yyvsp[-1].clist, 0, 0); ;}
    break;

  case 174:
#line 559 "grammar.y"
    { yyval.cblk = new CaseBlockNode(yyvsp[-3].clist, yyvsp[-2].ccl, yyvsp[-1].clist); ;}
    break;

  case 175:
#line 563 "grammar.y"
    { yyval.clist = 0; ;}
    break;

  case 177:
#line 568 "grammar.y"
    { yyval.clist = new ClauseListNode(yyvsp[0].ccl); ;}
    break;

  case 178:
#line 569 "grammar.y"
    { yyval.clist = new ClauseListNode(yyvsp[-1].clist, yyvsp[0].ccl); ;}
    break;

  case 179:
#line 573 "grammar.y"
    { yyval.ccl = new CaseClauseNode(yyvsp[-1].node); ;}
    break;

  case 180:
#line 574 "grammar.y"
    { yyval.ccl = new CaseClauseNode(yyvsp[-2].node, yyvsp[0].slist); ;}
    break;

  case 181:
#line 578 "grammar.y"
    { yyval.ccl = new CaseClauseNode(0); ;}
    break;

  case 182:
#line 579 "grammar.y"
    { yyval.ccl = new CaseClauseNode(0, yyvsp[0].slist); ;}
    break;

  case 183:
#line 583 "grammar.y"
    { yyvsp[0].stat->pushLabel(*yyvsp[-2].ident);
                                     yyval.stat = new LabelNode(*yyvsp[-2].ident, yyvsp[0].stat); DBG(yyval.stat,yylsp[-2],yylsp[-1]); ;}
    break;

  case 184:
#line 588 "grammar.y"
    { yyval.stat = new ThrowNode(yyvsp[-1].node); DBG(yyval.stat,yylsp[-2],yylsp[0]); ;}
    break;

  case 185:
#line 589 "grammar.y"
    { if (automatic()) {
                                       yyval.stat = new ThrowNode(yyvsp[-1].node); DBG(yyval.stat,yylsp[-2],yylsp[-2]);
                                     } else {
				       YYABORT; } ;}
    break;

  case 186:
#line 596 "grammar.y"
    { yyval.stat = new TryNode(yyvsp[-1].stat, yyvsp[0].cnode); DBG(yyval.stat,yylsp[-2],yylsp[-2]); ;}
    break;

  case 187:
#line 597 "grammar.y"
    { yyval.stat = new TryNode(yyvsp[-1].stat, yyvsp[0].fnode); DBG(yyval.stat,yylsp[-2],yylsp[-2]); ;}
    break;

  case 188:
#line 598 "grammar.y"
    { yyval.stat = new TryNode(yyvsp[-2].stat, yyvsp[-1].cnode, yyvsp[0].fnode); DBG(yyval.stat,yylsp[-3],yylsp[-3]); ;}
    break;

  case 189:
#line 602 "grammar.y"
    { CatchNode *c; yyval.cnode = c = new CatchNode(*yyvsp[-2].ident, yyvsp[0].stat);
				     DBG(c,yylsp[-4],yylsp[-1]); ;}
    break;

  case 190:
#line 607 "grammar.y"
    { FinallyNode *f; yyval.fnode = f = new FinallyNode(yyvsp[0].stat); DBG(f,yylsp[-1],yylsp[-1]); ;}
    break;

  case 192:
#line 613 "grammar.y"
    { yyval.func = yyvsp[0].func; ;}
    break;

  case 193:
#line 617 "grammar.y"
    { yyval.func = new FuncDeclNode(*yyvsp[-3].ident, yyvsp[0].body); DBG(yyval.func,yylsp[-4],yylsp[-1]); ;}
    break;

  case 194:
#line 619 "grammar.y"
    { yyval.func = new FuncDeclNode(*yyvsp[-4].ident, yyvsp[-2].param, yyvsp[0].body); DBG(yyval.func,yylsp[-5],yylsp[-1]); ;}
    break;

  case 195:
#line 623 "grammar.y"
    { yyval.node = new FuncExprNode(yyvsp[0].body); ;}
    break;

  case 196:
#line 625 "grammar.y"
    { yyval.node = new FuncExprNode(yyvsp[-2].param, yyvsp[0].body); ;}
    break;

  case 197:
#line 630 "grammar.y"
    { yyval.param = new ParameterNode(*yyvsp[0].ident); ;}
    break;

  case 198:
#line 631 "grammar.y"
    { yyval.param = new ParameterNode(yyvsp[-2].param, *yyvsp[0].ident); ;}
    break;

  case 199:
#line 635 "grammar.y"
    { yyval.body = new FunctionBodyNode(0);
	                             DBG(yyval.body, yylsp[-1], yylsp[0]);;}
    break;

  case 200:
#line 637 "grammar.y"
    { yyval.body = new FunctionBodyNode(yyvsp[-1].srcs);
	                             DBG(yyval.body, yylsp[-2], yylsp[0]);;}
    break;

  case 201:
#line 642 "grammar.y"
    { yyval.prog = new FunctionBodyNode(0);
                                     yyval.prog->setLoc(0, 0, Parser::source);
                                     Parser::progNode = yyval.prog; ;}
    break;

  case 202:
#line 645 "grammar.y"
    { yyval.prog = new FunctionBodyNode(yyvsp[0].srcs);
                                     Parser::progNode = yyval.prog; ;}
    break;

  case 203:
#line 650 "grammar.y"
    { yyval.srcs = new SourceElementsNode(yyvsp[0].stat); ;}
    break;

  case 204:
#line 651 "grammar.y"
    { yyval.srcs = new SourceElementsNode(yyvsp[-1].srcs, yyvsp[0].stat); ;}
    break;

  case 205:
#line 655 "grammar.y"
    { yyval.stat = yyvsp[0].stat; ;}
    break;

  case 206:
#line 656 "grammar.y"
    { yyval.stat = yyvsp[0].func; ;}
    break;


    }

/* Line 991 of yacc.c.  */
#line 2648 "grammar.tab.c"

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


#line 659 "grammar.y"


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

