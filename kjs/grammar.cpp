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
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

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
     CONTINUE = 269,
     FUNCTION = 270,
     RETURN = 271,
     VOID = 272,
     DELETE = 273,
     IF = 274,
     THIS = 275,
     DO = 276,
     WHILE = 277,
     ELSE = 278,
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
     EQEQ = 289,
     NE = 290,
     STREQ = 291,
     STRNEQ = 292,
     LE = 293,
     GE = 294,
     OR = 295,
     AND = 296,
     PLUSPLUS = 297,
     MINUSMINUS = 298,
     LSHIFT = 299,
     RSHIFT = 300,
     URSHIFT = 301,
     PLUSEQUAL = 302,
     MINUSEQUAL = 303,
     MULTEQUAL = 304,
     DIVEQUAL = 305,
     LSHIFTEQUAL = 306,
     RSHIFTEQUAL = 307,
     URSHIFTEQUAL = 308,
     ANDEQUAL = 309,
     MODEQUAL = 310,
     XOREQUAL = 311,
     OREQUAL = 312,
     IDENT = 313,
     AUTOPLUSPLUS = 314,
     AUTOMINUSMINUS = 315
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
#define CONTINUE 269
#define FUNCTION 270
#define RETURN 271
#define VOID 272
#define DELETE 273
#define IF 274
#define THIS 275
#define DO 276
#define WHILE 277
#define ELSE 278
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
#define EQEQ 289
#define NE 290
#define STREQ 291
#define STRNEQ 292
#define LE 293
#define GE 294
#define OR 295
#define AND 296
#define PLUSPLUS 297
#define MINUSMINUS 298
#define LSHIFT 299
#define RSHIFT 300
#define URSHIFT 301
#define PLUSEQUAL 302
#define MINUSEQUAL 303
#define MULTEQUAL 304
#define DIVEQUAL 305
#define LSHIFTEQUAL 306
#define RSHIFTEQUAL 307
#define URSHIFTEQUAL 308
#define ANDEQUAL 309
#define MODEQUAL 310
#define XOREQUAL 311
#define OREQUAL 312
#define IDENT 313
#define AUTOPLUSPLUS 314
#define AUTOMINUSMINUS 315




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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
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
#line 282 "grammar.tab.c"
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
#line 306 "grammar.tab.c"

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
#define YYFINAL  186
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1369

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  85
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  66
/* YYNRULES -- Number of rules. */
#define YYNRULES  198
/* YYNRULES -- Number of states. */
#define YYNSTATES  361

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   315

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    74,     2,     2,     2,    76,    79,     2,
      62,    63,    75,    71,    68,    72,    70,    61,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    69,    84,
      77,    83,    78,    82,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    66,     2,    67,    80,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    64,    81,    65,    73,     2,     2,     2,
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
      55,    56,    57,    58,    59,    60
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
     349,   351,   353,   355,   357,   359,   361,   363,   366,   370,
     372,   375,   379,   383,   385,   389,   391,   394,   397,   399,
     402,   405,   411,   419,   426,   432,   442,   453,   461,   470,
     480,   481,   483,   486,   489,   493,   497,   500,   503,   507,
     511,   514,   517,   521,   525,   531,   537,   541,   547,   548,
     550,   552,   555,   559,   564,   567,   571,   575,   579,   583,
     587,   592,   598,   601,   603,   606,   612,   619,   624,   630,
     632,   636,   639,   643,   644,   646,   648,   651,   653
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short yyrhs[] =
{
     148,     0,    -1,     3,    -1,     4,    -1,     5,    -1,     7,
      -1,     6,    -1,    61,    -1,    50,    -1,    20,    -1,    58,
      -1,    86,    -1,    88,    -1,    62,   115,    63,    -1,    64,
      65,    -1,    64,    92,    65,    -1,    66,    90,    67,    -1,
      66,    89,    67,    -1,    66,    89,    68,    90,    67,    -1,
      90,   113,    -1,    89,    68,    90,   113,    -1,    -1,    91,
      -1,    68,    -1,    91,    68,    -1,    93,    69,   113,    -1,
      92,    68,    93,    69,   113,    -1,    58,    -1,     6,    -1,
       7,    -1,    87,    -1,   145,    -1,    94,    66,   115,    67,
      -1,    94,    70,    58,    -1,    12,    94,    97,    -1,    94,
      -1,    12,    95,    -1,    94,    97,    -1,    96,    97,    -1,
      96,    66,   115,    67,    -1,    96,    70,    58,    -1,    62,
      63,    -1,    62,    98,    63,    -1,   113,    -1,    98,    68,
     113,    -1,    95,    -1,    96,    -1,    99,    -1,    99,    42,
      -1,    99,    43,    -1,   100,    -1,    18,   101,    -1,    17,
     101,    -1,    26,   101,    -1,    42,   101,    -1,    59,   101,
      -1,    43,   101,    -1,    60,   101,    -1,    71,   101,    -1,
      72,   101,    -1,    73,   101,    -1,    74,   101,    -1,   101,
      -1,   102,    75,   101,    -1,   102,    61,   101,    -1,   102,
      76,   101,    -1,   102,    -1,   103,    71,   102,    -1,   103,
      72,   102,    -1,   103,    -1,   104,    44,   103,    -1,   104,
      45,   103,    -1,   104,    46,   103,    -1,   104,    -1,   105,
      77,   104,    -1,   105,    78,   104,    -1,   105,    38,   104,
      -1,   105,    39,   104,    -1,   105,    25,   104,    -1,   105,
      24,   104,    -1,   105,    -1,   106,    34,   105,    -1,   106,
      35,   105,    -1,   106,    36,   105,    -1,   106,    37,   105,
      -1,   106,    -1,   107,    79,   106,    -1,   107,    -1,   108,
      80,   107,    -1,   108,    -1,   109,    81,   108,    -1,   109,
      -1,   110,    41,   109,    -1,   110,    -1,   111,    40,   110,
      -1,   111,    -1,   111,    82,   113,    69,   113,    -1,   112,
      -1,    99,   114,   113,    -1,    83,    -1,    47,    -1,    48,
      -1,    49,    -1,    50,    -1,    51,    -1,    52,    -1,    53,
      -1,    54,    -1,    56,    -1,    57,    -1,    55,    -1,   113,
      -1,   115,    68,   113,    -1,   117,    -1,   119,    -1,   123,
      -1,   124,    -1,   125,    -1,   126,    -1,   128,    -1,   129,
      -1,   130,    -1,   131,    -1,   132,    -1,   138,    -1,   139,
      -1,   140,    -1,    64,    65,    -1,    64,   149,    65,    -1,
     116,    -1,   118,   116,    -1,    13,   120,    84,    -1,    13,
     120,     1,    -1,   121,    -1,   120,    68,   121,    -1,    58,
      -1,    58,   122,    -1,    83,   113,    -1,    84,    -1,   115,
      84,    -1,   115,     1,    -1,    19,    62,   115,    63,   116,
      -1,    19,    62,   115,    63,   116,    23,   116,    -1,    21,
     116,    22,    62,   115,    63,    -1,    22,    62,   115,    63,
     116,    -1,    11,    62,   127,    84,   127,    84,   127,    63,
     116,    -1,    11,    62,    13,   120,    84,   127,    84,   127,
      63,   116,    -1,    11,    62,    99,    24,   115,    63,   116,
      -1,    11,    62,    13,    58,    24,   115,    63,   116,    -1,
      11,    62,    13,    58,   122,    24,   115,    63,   116,    -1,
      -1,   115,    -1,    14,    84,    -1,    14,     1,    -1,    14,
      58,    84,    -1,    14,    58,     1,    -1,     8,    84,    -1,
       8,     1,    -1,     8,    58,    84,    -1,     8,    58,     1,
      -1,    16,    84,    -1,    16,     1,    -1,    16,   115,    84,
      -1,    16,   115,     1,    -1,    28,    62,   115,    63,   116,
      -1,    27,    62,   115,    63,   133,    -1,    64,   134,    65,
      -1,    64,   134,   137,   134,    65,    -1,    -1,   135,    -1,
     136,    -1,   135,   136,    -1,     9,   115,    69,    -1,     9,
     115,    69,   118,    -1,    10,    69,    -1,    10,    69,   118,
      -1,    58,    69,   116,    -1,    30,   115,    84,    -1,    31,
     117,   141,    -1,    31,   117,   142,    -1,    31,   117,   141,
     142,    -1,    32,    62,    58,    63,   117,    -1,    33,   117,
      -1,   144,    -1,    17,   144,    -1,    15,    58,    62,    63,
     147,    -1,    15,    58,    62,   146,    63,   147,    -1,    15,
      62,    63,   147,    -1,    15,    62,   146,    63,   147,    -1,
      58,    -1,   146,    68,    58,    -1,    64,    65,    -1,    64,
     149,    65,    -1,    -1,   149,    -1,   150,    -1,   149,   150,
      -1,   116,    -1,   143,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   166,   166,   167,   168,   169,   170,   171,   174,   181,
     182,   184,   185,   186,   187,   188,   192,   193,   194,   198,
     199,   204,   205,   209,   210,   214,   215,   220,   221,   222,
     226,   227,   228,   229,   230,   234,   235,   239,   240,   241,
     242,   247,   248,   252,   253,   257,   258,   262,   263,   264,
     268,   269,   270,   271,   272,   273,   274,   275,   276,   277,
     278,   279,   283,   284,   285,   286,   290,   291,   292,   296,
     297,   298,   299,   303,   304,   306,   308,   310,   312,   314,
     319,   320,   321,   322,   323,   327,   328,   332,   333,   337,
     338,   342,   343,   348,   349,   354,   355,   360,   361,   366,
     367,   368,   369,   370,   371,   372,   373,   374,   375,   376,
     377,   381,   382,   386,   387,   388,   389,   390,   391,   392,
     393,   394,   395,   396,   397,   398,   399,   403,   404,   408,
     409,   413,   415,   425,   426,   431,   432,   436,   440,   444,
     446,   454,   455,   460,   461,   462,   465,   468,   471,   475,
     482,   483,   487,   488,   492,   494,   502,   503,   507,   509,
     518,   519,   523,   524,   532,   537,   542,   543,   548,   549,
     553,   554,   558,   559,   563,   564,   568,   574,   578,   579,
     580,   584,   589,   593,   595,   599,   601,   607,   608,   614,
     615,   619,   621,   626,   629,   634,   635,   639,   640
};
#endif

#if YYDEBUG || YYERROR_VERBOSE
/* YYTNME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "NULLTOKEN", "TRUETOKEN", "FALSETOKEN", 
  "STRING", "NUMBER", "BREAK", "CASE", "DEFAULT", "FOR", "NEW", "VAR", 
  "CONTINUE", "FUNCTION", "RETURN", "VOID", "DELETE", "IF", "THIS", "DO", 
  "WHILE", "ELSE", "IN", "INSTANCEOF", "TYPEOF", "SWITCH", "WITH", 
  "RESERVED", "THROW", "TRY", "CATCH", "FINALLY", "EQEQ", "NE", "STREQ", 
  "STRNEQ", "LE", "GE", "OR", "AND", "PLUSPLUS", "MINUSMINUS", "LSHIFT", 
  "RSHIFT", "URSHIFT", "PLUSEQUAL", "MINUSEQUAL", "MULTEQUAL", "DIVEQUAL", 
  "LSHIFTEQUAL", "RSHIFTEQUAL", "URSHIFTEQUAL", "ANDEQUAL", "MODEQUAL", 
  "XOREQUAL", "OREQUAL", "IDENT", "AUTOPLUSPLUS", "AUTOMINUSMINUS", "'/'", 
  "'('", "')'", "'{'", "'}'", "'['", "']'", "','", "':'", "'.'", "'+'", 
  "'-'", "'~'", "'!'", "'*'", "'%'", "'<'", "'>'", "'&'", "'^'", "'|'", 
  "'?'", "'='", "';'", "$accept", "Literal", "PrimaryExpr", 
  "ArrayLiteral", "ElementList", "ElisionOpt", "Elision", 
  "PropertyNameAndValueList", "PropertyName", "MemberExpr", "NewExpr", 
  "CallExpr", "Arguments", "ArgumentList", "LeftHandSideExpr", 
  "PostfixExpr", "UnaryExpr", "MultiplicativeExpr", "AdditiveExpr", 
  "ShiftExpr", "RelationalExpr", "EqualityExpr", "BitwiseANDExpr", 
  "BitwiseXORExpr", "BitwiseORExpr", "LogicalANDExpr", "LogicalORExpr", 
  "ConditionalExpr", "AssignmentExpr", "AssignmentOperator", "Expr", 
  "Statement", "Block", "StatementList", "VariableStatement", 
  "VariableDeclarationList", "VariableDeclaration", "Initializer", 
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
     315,    47,    40,    41,   123,   125,    91,    93,    44,    58,
      46,    43,    45,   126,    33,    42,    37,    60,    62,    38,
      94,   124,    63,    61,    59
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    85,    86,    86,    86,    86,    86,    86,    86,    87,
      87,    87,    87,    87,    87,    87,    88,    88,    88,    89,
      89,    90,    90,    91,    91,    92,    92,    93,    93,    93,
      94,    94,    94,    94,    94,    95,    95,    96,    96,    96,
      96,    97,    97,    98,    98,    99,    99,   100,   100,   100,
     101,   101,   101,   101,   101,   101,   101,   101,   101,   101,
     101,   101,   102,   102,   102,   102,   103,   103,   103,   104,
     104,   104,   104,   105,   105,   105,   105,   105,   105,   105,
     106,   106,   106,   106,   106,   107,   107,   108,   108,   109,
     109,   110,   110,   111,   111,   112,   112,   113,   113,   114,
     114,   114,   114,   114,   114,   114,   114,   114,   114,   114,
     114,   115,   115,   116,   116,   116,   116,   116,   116,   116,
     116,   116,   116,   116,   116,   116,   116,   117,   117,   118,
     118,   119,   119,   120,   120,   121,   121,   122,   123,   124,
     124,   125,   125,   126,   126,   126,   126,   126,   126,   126,
     127,   127,   128,   128,   128,   128,   129,   129,   129,   129,
     130,   130,   130,   130,   131,   132,   133,   133,   134,   134,
     135,   135,   136,   136,   137,   137,   138,   139,   140,   140,
     140,   141,   142,   143,   143,   144,   144,   145,   145,   146,
     146,   147,   147,   148,   148,   149,   149,   150,   150
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
       1,     1,     1,     1,     1,     1,     1,     2,     3,     1,
       2,     3,     3,     1,     3,     1,     2,     2,     1,     2,
       2,     5,     7,     6,     5,     9,    10,     7,     8,     9,
       0,     1,     2,     2,     3,     3,     2,     2,     3,     3,
       2,     2,     3,     3,     5,     5,     3,     5,     0,     1,
       1,     2,     3,     4,     2,     3,     3,     3,     3,     3,
       4,     5,     2,     1,     2,     5,     6,     4,     5,     1,
       3,     2,     3,     0,     1,     1,     2,     1,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
     193,     2,     3,     4,     6,     5,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     9,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     8,    10,     0,     0,
       7,     0,     0,    21,     0,     0,     0,     0,   138,    11,
      30,    12,    35,    45,    46,    47,    50,    62,    66,    69,
      73,    80,    85,    87,    89,    91,    93,    95,    97,   111,
       0,   197,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   198,   183,    31,     0,
     194,   195,   157,     0,   156,   150,     0,    10,     0,    35,
      36,   135,     0,   133,   153,     0,   152,     0,     0,   161,
       0,   160,     0,    47,    52,   184,    51,     0,     0,     0,
      53,     0,     0,     0,     0,     0,    54,    56,     0,    55,
      57,     0,     6,     5,    10,    14,     0,     0,     0,    23,
       0,     0,    22,    58,    59,    60,    61,     0,     0,     0,
      37,     0,     0,    38,    48,    49,   100,   101,   102,   103,
     104,   105,   106,   107,   110,   108,   109,    99,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   140,     0,   139,     1,   196,   159,   158,
       0,    47,   151,     0,    28,    29,    27,    14,    34,     0,
     136,   132,     0,   131,   155,   154,     0,   189,     0,     0,
     163,   162,     0,     0,     0,     0,     0,   177,   127,     0,
       0,   178,   179,   176,    13,    15,     0,     0,   128,    17,
      21,    16,    19,    24,    41,     0,    43,     0,    33,     0,
      40,    98,    64,    63,    65,    67,    68,    70,    71,    72,
      79,    78,    76,    77,    74,    75,    81,    82,    83,    84,
      86,    88,    90,    92,    94,     0,   112,   135,     0,     0,
     150,   137,   134,     0,     0,     0,   187,     0,     0,     0,
       0,     0,     0,     0,     0,   182,   180,     0,    25,     0,
      42,     0,    32,    39,     0,     0,   136,   150,     0,     0,
     185,     0,   191,     0,   188,   190,   141,     0,   144,   168,
     165,   164,     0,     0,    18,    20,    44,    96,     0,     0,
       0,     0,   150,   186,   192,     0,   143,     0,     0,   169,
     170,     0,    26,     0,     0,   150,   147,     0,   142,     0,
       0,   166,   168,   171,   181,   148,     0,     0,     0,   172,
     174,     0,   149,     0,   145,   129,   173,   175,   167,   146,
     130
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,    39,    40,    41,   130,   131,   132,   126,   127,    42,
      43,    44,   140,   235,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,   158,
      60,    61,    62,   356,    63,    92,    93,   200,    64,    65,
      66,    67,   193,    68,    69,    70,    71,    72,   310,   328,
     329,   330,   342,    73,    74,    75,   221,   222,    76,    77,
      78,   209,   276,    79,   128,    81
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -252
static const short yypact[] =
{
     708,  -252,  -252,  -252,  -252,  -252,     7,   -48,   229,    -3,
       9,   -23,   116,  1181,  1253,    10,  -252,   782,    46,  1253,
      52,    54,  1253,    74,  1253,  1253,  -252,     6,  1253,  1253,
    -252,  1253,   338,    50,  1253,  1253,  1253,  1253,  -252,  -252,
    -252,  -252,    73,  -252,    84,   945,  -252,  -252,    51,    16,
     103,    86,   194,   100,    91,   122,   158,   -33,  -252,  -252,
      22,  -252,  -252,  -252,  -252,  -252,  -252,  -252,  -252,  -252,
    -252,  -252,  -252,  -252,  -252,  -252,  -252,  -252,  -252,   240,
     708,  -252,  -252,    11,  -252,   893,   181,  -252,    79,    73,
    -252,   173,    29,  -252,  -252,    23,  -252,   197,   -25,  -252,
    1253,  -252,    31,    98,  -252,  -252,  -252,  1253,   241,  1253,
    -252,  1253,  1253,   -43,   412,   169,  -252,  -252,   782,  -252,
    -252,   -18,   196,   198,     6,   864,   -11,   199,   486,  -252,
     171,   965,   201,  -252,  -252,  -252,  -252,  1037,  1253,   212,
    -252,  1253,   214,  -252,  -252,  -252,  -252,  -252,  -252,  -252,
    -252,  -252,  -252,  -252,  -252,  -252,  -252,  -252,  1253,  1253,
    1253,  1253,  1253,  1253,  1253,  1253,  1253,  1253,  1253,  1253,
    1253,  1253,  1253,  1253,  1253,  1253,  1253,  1253,  1253,  1253,
    1253,  1253,  1253,  -252,  1253,  -252,  -252,  -252,  -252,  -252,
     215,  1286,   206,   191,  -252,  -252,  -252,  -252,  -252,  1253,
    -252,  -252,    -3,  -252,  -252,  -252,   -10,  -252,   216,   -12,
    -252,  -252,    -2,   219,    15,    41,    92,  -252,  -252,   220,
      74,   243,  -252,  -252,  -252,  -252,    36,  1253,  -252,  -252,
      50,  -252,  -252,  -252,  -252,    93,  -252,   183,  -252,   185,
    -252,  -252,  -252,  -252,  -252,    51,    51,    16,    16,    16,
     103,   103,   103,   103,   103,   103,    86,    86,    86,    86,
     194,   100,    91,   122,   158,   209,  -252,    13,   -37,  1253,
    1253,  -252,  -252,   216,    94,   560,  -252,   216,   225,   782,
    1253,   782,   221,   782,   226,  -252,  -252,   223,  -252,  1109,
    -252,  1253,  -252,  -252,  1253,  1253,   265,  1253,   102,   210,
    -252,   216,  -252,   634,  -252,  -252,   273,   105,  -252,   288,
    -252,  -252,   235,  1253,  -252,  -252,  -252,  -252,   118,  1253,
     217,   782,  1253,  -252,  -252,   782,  -252,  1253,    24,   288,
    -252,    74,  -252,   782,   174,  1253,  -252,   236,  -252,   186,
     231,  -252,   288,  -252,  -252,  -252,   782,   239,   782,   782,
     782,   238,  -252,   782,  -252,  -252,   782,   782,  -252,  -252,
    -252
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -252,  -252,  -252,  -252,  -252,    75,  -252,  -252,    80,   299,
     303,  -252,   -26,  -252,    45,  -252,    -8,    95,    19,    25,
      72,   135,   136,   138,   139,   134,  -252,  -252,  -122,  -252,
      -9,   -17,   -22,   -30,  -252,   131,   120,    56,  -252,  -252,
    -252,  -252,  -251,  -252,  -252,  -252,  -252,  -252,  -252,   -15,
    -252,    -5,  -252,  -252,  -252,  -252,  -252,   104,  -252,   315,
    -252,   124,  -233,  -252,     2,   -76
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -128
static const short yytable[] =
{
     108,   115,    80,   102,   187,   104,   106,   181,    82,   232,
      94,   110,   188,   113,    85,   236,   116,   117,   143,   299,
     119,   120,   121,   183,   204,   184,   133,   134,   135,   136,
     201,   202,   210,   207,   340,    97,   241,   295,   208,    98,
     300,   217,   194,   195,   304,   224,   320,   297,   207,   182,
     184,   277,   187,   273,   225,    91,   278,   226,   103,   103,
     265,   279,   266,   198,   103,    83,   184,    95,   323,   103,
     103,   337,   107,   103,   103,   118,   192,   271,   281,   103,
     103,   103,   103,   184,   347,   194,   195,   162,   163,   341,
     184,    84,   104,    96,   196,   189,   199,   202,   212,   184,
     214,   223,   215,   216,   282,   288,   185,   205,   109,   184,
     167,   168,   159,   203,   111,   211,   112,    99,   129,     1,
       2,     3,     4,     5,   169,   170,   160,   161,     8,   237,
     191,    86,   239,   100,    14,   137,    16,   196,   114,   138,
     144,   145,    19,   139,   197,   103,   137,   164,   165,   166,
     141,   242,   243,   244,   142,   283,   290,   301,    24,    25,
     184,   291,   278,   171,   172,   321,    26,   315,   326,   316,
     184,   178,   317,   184,    87,    28,    29,    30,    31,   177,
      88,   333,    33,   247,   248,   249,   184,    34,    35,    36,
      37,   332,   250,   251,   252,   253,   254,   255,   285,   180,
     101,   219,   220,   179,   103,   103,   103,   103,   103,   103,
     103,   103,   103,   103,   103,   103,   103,   103,   103,   103,
     103,   103,   103,   103,   103,   103,   103,   187,   173,   174,
     175,   176,     1,     2,     3,     4,     5,   346,   229,   230,
     186,     8,   184,    98,    86,   256,   257,   258,   259,    16,
     292,   184,   293,   184,   184,   349,   199,   245,   246,   206,
     298,   192,   306,   213,   308,   -28,   311,   -29,   227,   233,
     238,   307,   240,   267,   184,   270,   220,   303,   294,    26,
     275,   280,   284,   305,   312,   309,   318,    87,   192,   319,
      30,    31,   313,    88,   322,    33,   325,   327,   331,   348,
     350,   335,   353,   358,   336,   289,   287,    89,   338,   344,
     334,    90,   260,   192,   261,   264,   345,   262,   339,   263,
     357,   268,   272,   296,   343,   286,   192,   351,   105,   352,
     274,   354,   355,   355,     0,     0,   359,     0,     0,   360,
     360,     1,     2,     3,   122,   123,     6,     0,     0,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,     0,     0,     0,    19,    20,    21,     0,    22,    23,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      24,    25,     0,     0,     0,     0,     0,     0,    26,     0,
       0,     0,     0,     0,     0,     0,   124,    28,    29,    30,
      31,     0,    32,   125,    33,     0,     0,     0,     0,    34,
      35,    36,    37,     0,     0,     1,     2,     3,     4,     5,
       6,     0,    38,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,     0,     0,     0,    19,    20,
      21,     0,    22,    23,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    24,    25,     0,     0,     0,     0,
       0,     0,    26,     0,     0,     0,     0,     0,     0,     0,
      27,    28,    29,    30,    31,     0,    32,   218,    33,     0,
       0,     0,     0,    34,    35,    36,    37,     0,     0,     1,
       2,     3,     4,     5,     6,     0,    38,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,     0,
       0,     0,    19,    20,    21,     0,    22,    23,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    24,    25,
       0,     0,     0,     0,     0,     0,    26,     0,     0,     0,
       0,     0,     0,     0,    27,    28,    29,    30,    31,     0,
      32,   228,    33,     0,     0,     0,     0,    34,    35,    36,
      37,     0,     0,     1,     2,     3,     4,     5,     6,     0,
      38,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,     0,     0,     0,    19,    20,    21,     0,
      22,    23,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    24,    25,     0,     0,     0,     0,     0,     0,
      26,     0,     0,     0,     0,     0,     0,     0,    27,    28,
      29,    30,    31,     0,    32,   302,    33,     0,     0,     0,
       0,    34,    35,    36,    37,     0,     0,     1,     2,     3,
       4,     5,     6,     0,    38,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,     0,     0,     0,
      19,    20,    21,     0,    22,    23,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    24,    25,     0,     0,
       0,     0,     0,     0,    26,     0,     0,     0,     0,     0,
       0,     0,    27,    28,    29,    30,    31,     0,    32,   324,
      33,     0,     0,     0,     0,    34,    35,    36,    37,     0,
       0,     1,     2,     3,     4,     5,     6,     0,    38,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,     0,     0,     0,    19,    20,    21,     0,    22,    23,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      24,    25,     0,     0,     0,     0,     0,     0,    26,     0,
       0,     0,     0,     0,     0,     0,    27,    28,    29,    30,
      31,     0,    32,     0,    33,     0,     0,     0,     0,    34,
      35,    36,    37,     0,     0,     1,     2,     3,     4,     5,
       6,     0,    38,     7,     8,     9,    10,    86,    12,   100,
      14,    15,    16,    17,    18,     0,     0,     0,    19,    20,
      21,     0,    22,    23,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    24,    25,     0,     0,     0,     0,
       0,     0,    26,     0,     0,     0,     0,     0,     0,     0,
      27,    28,    29,    30,    31,     0,    32,     0,    33,     0,
       0,     0,     0,    34,    35,    36,    37,     0,     0,     0,
       0,     0,     0,     0,  -127,     0,    38,  -127,  -127,  -127,
    -127,  -127,  -127,  -127,  -127,  -127,  -127,  -127,  -127,  -127,
    -127,  -127,  -127,  -127,  -127,  -127,  -127,  -127,     0,     0,
    -127,  -127,  -127,     0,  -127,  -127,     1,     2,     3,     4,
       5,     0,     0,     0,     0,     8,   190,     0,    86,     0,
     100,    14,     0,    16,     0,     0,     0,     0,     0,    19,
       0,     0,  -127,  -127,  -127,     0,     0,     0,  -127,  -127,
       0,     0,     0,     0,     0,    24,    25,  -127,  -127,     0,
       0,     0,     0,    26,     0,     0,     0,     0,     0,     0,
       0,    87,    28,    29,    30,    31,     0,    88,     0,    33,
       0,     0,     0,     0,    34,    35,    36,    37,     1,     2,
       3,     4,     5,     0,     0,     0,     0,     8,     0,     0,
      86,     0,   100,    14,     0,    16,     0,   144,   145,     0,
       0,    19,   146,   147,   148,   149,   150,   151,   152,   153,
     154,   155,   156,     0,     0,     0,     0,    24,    25,     0,
       0,     0,     0,     0,     0,    26,     0,     0,     0,     0,
       0,     0,     0,    87,    28,    29,    30,    31,   157,    88,
       0,    33,   231,     0,     0,     0,    34,    35,    36,    37,
       1,     2,     3,     4,     5,     0,     0,     0,     0,     8,
       0,     0,    86,     0,   100,    14,     0,    16,     0,     0,
       0,     0,     0,    19,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    24,
      25,     0,     0,     0,     0,     0,     0,    26,     0,     0,
       0,     0,     0,     0,     0,    87,    28,    29,    30,    31,
     234,    88,     0,    33,     0,     0,     0,     0,    34,    35,
      36,    37,     1,     2,     3,     4,     5,     0,     0,     0,
       0,     8,     0,     0,    86,     0,   100,    14,     0,    16,
       0,     0,     0,     0,     0,    19,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    24,    25,     0,     0,     0,     0,     0,     0,    26,
       0,     0,     0,     0,     0,     0,     0,    87,    28,    29,
      30,    31,     0,    88,     0,    33,   314,     0,     0,     0,
      34,    35,    36,    37,     1,     2,     3,     4,     5,     0,
       0,     0,     0,     8,     0,     0,    11,     0,   100,    14,
       0,    16,     0,     0,     0,     0,     0,    19,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    24,    25,     0,     0,     0,     0,     0,
       0,    26,     0,     0,     0,     0,     0,     0,     0,    87,
      28,    29,    30,    31,     0,    88,     0,    33,     0,     0,
       0,     0,    34,    35,    36,    37,     1,     2,     3,     4,
       5,     0,     0,     0,     0,     8,     0,     0,    86,     0,
     100,    14,     0,    16,     0,     0,     0,     0,     0,    19,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    24,    25,     0,     0,     0,
       0,     0,     0,    26,     0,     0,     0,     0,     0,     0,
     269,    87,    28,    29,    30,    31,     0,    88,     0,    33,
       0,     0,     0,     0,    34,    35,    36,    37,   144,   145,
       0,     0,     0,   146,   147,   148,   149,   150,   151,   152,
     153,   154,   155,   156,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   157
};

static const short yycheck[] =
{
      17,    23,     0,    12,    80,    13,    14,    40,     1,   131,
       1,    19,     1,    22,    62,   137,    24,    25,    44,   270,
      28,    29,    31,     1,     1,    68,    34,    35,    36,    37,
       1,    68,     1,    58,    10,    58,   158,    24,    63,    62,
     273,    84,     6,     7,   277,    63,   297,    84,    58,    82,
      68,    63,   128,    63,    65,    58,    68,    68,    13,    14,
     182,    63,   184,    89,    19,    58,    68,    58,   301,    24,
      25,   322,    62,    28,    29,    69,    85,   199,    63,    34,
      35,    36,    37,    68,   335,     6,     7,    71,    72,    65,
      68,    84,   100,    84,    58,    84,    83,    68,   107,    68,
     109,   118,   111,   112,    63,   227,    84,    84,    62,    68,
      24,    25,    61,    84,    62,    84,    62,     1,    68,     3,
       4,     5,     6,     7,    38,    39,    75,    76,    12,   138,
      85,    15,   141,    17,    18,    62,    20,    58,    64,    66,
      42,    43,    26,    70,    65,   100,    62,    44,    45,    46,
      66,   159,   160,   161,    70,    63,    63,    63,    42,    43,
      68,    68,    68,    77,    78,    63,    50,   289,    63,   291,
      68,    80,   294,    68,    58,    59,    60,    61,    62,    79,
      64,    63,    66,   164,   165,   166,    68,    71,    72,    73,
      74,   313,   167,   168,   169,   170,   171,   172,   220,    41,
      84,    32,    33,    81,   159,   160,   161,   162,   163,   164,
     165,   166,   167,   168,   169,   170,   171,   172,   173,   174,
     175,   176,   177,   178,   179,   180,   181,   303,    34,    35,
      36,    37,     3,     4,     5,     6,     7,    63,    67,    68,
       0,    12,    68,    62,    15,   173,   174,   175,   176,    20,
      67,    68,    67,    68,    68,    69,    83,   162,   163,    62,
     269,   270,   279,    22,   281,    69,   283,    69,    69,    68,
      58,   280,    58,    58,    68,    84,    33,   275,    69,    50,
      64,    62,    62,    58,    58,    64,   295,    58,   297,    24,
      61,    62,    69,    64,    84,    66,    23,     9,    63,    63,
      69,    84,    63,    65,   321,   230,   226,     8,   325,   331,
     319,     8,   177,   322,   178,   181,   333,   179,   327,   180,
     350,   190,   202,   267,   329,   221,   335,   342,    13,   346,
     206,   348,   349,   350,    -1,    -1,   353,    -1,    -1,   356,
     357,     3,     4,     5,     6,     7,     8,    -1,    -1,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    -1,    -1,    -1,    26,    27,    28,    -1,    30,    31,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      42,    43,    -1,    -1,    -1,    -1,    -1,    -1,    50,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    58,    59,    60,    61,
      62,    -1,    64,    65,    66,    -1,    -1,    -1,    -1,    71,
      72,    73,    74,    -1,    -1,     3,     4,     5,     6,     7,
       8,    -1,    84,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    -1,    -1,    -1,    26,    27,
      28,    -1,    30,    31,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    42,    43,    -1,    -1,    -1,    -1,
      -1,    -1,    50,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      58,    59,    60,    61,    62,    -1,    64,    65,    66,    -1,
      -1,    -1,    -1,    71,    72,    73,    74,    -1,    -1,     3,
       4,     5,     6,     7,     8,    -1,    84,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    -1,
      -1,    -1,    26,    27,    28,    -1,    30,    31,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,    43,
      -1,    -1,    -1,    -1,    -1,    -1,    50,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    58,    59,    60,    61,    62,    -1,
      64,    65,    66,    -1,    -1,    -1,    -1,    71,    72,    73,
      74,    -1,    -1,     3,     4,     5,     6,     7,     8,    -1,
      84,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    -1,    -1,    -1,    26,    27,    28,    -1,
      30,    31,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    42,    43,    -1,    -1,    -1,    -1,    -1,    -1,
      50,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    58,    59,
      60,    61,    62,    -1,    64,    65,    66,    -1,    -1,    -1,
      -1,    71,    72,    73,    74,    -1,    -1,     3,     4,     5,
       6,     7,     8,    -1,    84,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    -1,    -1,    -1,
      26,    27,    28,    -1,    30,    31,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    42,    43,    -1,    -1,
      -1,    -1,    -1,    -1,    50,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    58,    59,    60,    61,    62,    -1,    64,    65,
      66,    -1,    -1,    -1,    -1,    71,    72,    73,    74,    -1,
      -1,     3,     4,     5,     6,     7,     8,    -1,    84,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    -1,    -1,    -1,    26,    27,    28,    -1,    30,    31,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      42,    43,    -1,    -1,    -1,    -1,    -1,    -1,    50,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    58,    59,    60,    61,
      62,    -1,    64,    -1,    66,    -1,    -1,    -1,    -1,    71,
      72,    73,    74,    -1,    -1,     3,     4,     5,     6,     7,
       8,    -1,    84,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    -1,    -1,    -1,    26,    27,
      28,    -1,    30,    31,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    42,    43,    -1,    -1,    -1,    -1,
      -1,    -1,    50,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      58,    59,    60,    61,    62,    -1,    64,    -1,    66,    -1,
      -1,    -1,    -1,    71,    72,    73,    74,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,     0,    -1,    84,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    -1,    -1,
      26,    27,    28,    -1,    30,    31,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    12,    13,    -1,    15,    -1,
      17,    18,    -1,    20,    -1,    -1,    -1,    -1,    -1,    26,
      -1,    -1,    58,    59,    60,    -1,    -1,    -1,    64,    65,
      -1,    -1,    -1,    -1,    -1,    42,    43,    73,    74,    -1,
      -1,    -1,    -1,    50,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    58,    59,    60,    61,    62,    -1,    64,    -1,    66,
      -1,    -1,    -1,    -1,    71,    72,    73,    74,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    12,    -1,    -1,
      15,    -1,    17,    18,    -1,    20,    -1,    42,    43,    -1,
      -1,    26,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    -1,    -1,    -1,    -1,    42,    43,    -1,
      -1,    -1,    -1,    -1,    -1,    50,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    58,    59,    60,    61,    62,    83,    64,
      -1,    66,    67,    -1,    -1,    -1,    71,    72,    73,    74,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    12,
      -1,    -1,    15,    -1,    17,    18,    -1,    20,    -1,    -1,
      -1,    -1,    -1,    26,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,
      43,    -1,    -1,    -1,    -1,    -1,    -1,    50,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    58,    59,    60,    61,    62,
      63,    64,    -1,    66,    -1,    -1,    -1,    -1,    71,    72,
      73,    74,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    12,    -1,    -1,    15,    -1,    17,    18,    -1,    20,
      -1,    -1,    -1,    -1,    -1,    26,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    42,    43,    -1,    -1,    -1,    -1,    -1,    -1,    50,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    58,    59,    60,
      61,    62,    -1,    64,    -1,    66,    67,    -1,    -1,    -1,
      71,    72,    73,    74,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    12,    -1,    -1,    15,    -1,    17,    18,
      -1,    20,    -1,    -1,    -1,    -1,    -1,    26,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    42,    43,    -1,    -1,    -1,    -1,    -1,
      -1,    50,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    58,
      59,    60,    61,    62,    -1,    64,    -1,    66,    -1,    -1,
      -1,    -1,    71,    72,    73,    74,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    12,    -1,    -1,    15,    -1,
      17,    18,    -1,    20,    -1,    -1,    -1,    -1,    -1,    26,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    42,    43,    -1,    -1,    -1,
      -1,    -1,    -1,    50,    -1,    -1,    -1,    -1,    -1,    -1,
      24,    58,    59,    60,    61,    62,    -1,    64,    -1,    66,
      -1,    -1,    -1,    -1,    71,    72,    73,    74,    42,    43,
      -1,    -1,    -1,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    83
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,     3,     4,     5,     6,     7,     8,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    26,
      27,    28,    30,    31,    42,    43,    50,    58,    59,    60,
      61,    62,    64,    66,    71,    72,    73,    74,    84,    86,
      87,    88,    94,    95,    96,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     115,   116,   117,   119,   123,   124,   125,   126,   128,   129,
     130,   131,   132,   138,   139,   140,   143,   144,   145,   148,
     149,   150,     1,    58,    84,    62,    15,    58,    64,    94,
      95,    58,   120,   121,     1,    58,    84,    58,    62,     1,
      17,    84,   115,    99,   101,   144,   101,    62,   116,    62,
     101,    62,    62,   115,    64,   117,   101,   101,    69,   101,
     101,   115,     6,     7,    58,    65,    92,    93,   149,    68,
      89,    90,    91,   101,   101,   101,   101,    62,    66,    70,
      97,    66,    70,    97,    42,    43,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    83,   114,    61,
      75,    76,    71,    72,    44,    45,    46,    24,    25,    38,
      39,    77,    78,    34,    35,    36,    37,    79,    80,    81,
      41,    40,    82,     1,    68,    84,     0,   150,     1,    84,
      13,    99,   115,   127,     6,     7,    58,    65,    97,    83,
     122,     1,    68,    84,     1,    84,    62,    58,    63,   146,
       1,    84,   115,    22,   115,   115,   115,    84,    65,    32,
      33,   141,   142,   116,    63,    65,    68,    69,    65,    67,
      68,    67,   113,    68,    63,    98,   113,   115,    58,   115,
      58,   113,   101,   101,   101,   102,   102,   103,   103,   103,
     104,   104,   104,   104,   104,   104,   105,   105,   105,   105,
     106,   107,   108,   109,   110,   113,   113,    58,   120,    24,
      84,   113,   121,    63,   146,    64,   147,    63,    68,    63,
      62,    63,    63,    63,    62,   117,   142,    93,   113,    90,
      63,    68,    67,    67,    69,    24,   122,    84,   115,   127,
     147,    63,    65,   149,   147,    58,   116,   115,   116,    64,
     133,   116,    58,    69,    67,   113,   113,   113,   115,    24,
     127,    63,    84,   147,    65,    23,    63,     9,   134,   135,
     136,    63,   113,    63,   115,    84,   116,   127,   116,   115,
      10,    65,   137,   136,   117,   116,    63,   127,    63,    69,
      69,   134,   116,    63,   116,   116,   118,   118,    65,   116,
     116
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
#line 166 "grammar.y"
    { yyval.node = new NullNode(); ;}
    break;

  case 3:
#line 167 "grammar.y"
    { yyval.node = new BooleanNode(true); ;}
    break;

  case 4:
#line 168 "grammar.y"
    { yyval.node = new BooleanNode(false); ;}
    break;

  case 5:
#line 169 "grammar.y"
    { yyval.node = new NumberNode(yyvsp[0].dval); ;}
    break;

  case 6:
#line 170 "grammar.y"
    { yyval.node = new StringNode(yyvsp[0].ustr); delete yyvsp[0].ustr; ;}
    break;

  case 7:
#line 171 "grammar.y"
    { Lexer *l = Lexer::curr();
                                     if (!l->scanRegExp()) YYABORT;
                                     yyval.node = new RegExpNode(l->pattern,l->flags);;}
    break;

  case 8:
#line 175 "grammar.y"
    { Lexer *l = Lexer::curr();
                                     if (!l->scanRegExp()) YYABORT;
                                     yyval.node = new RegExpNode(UString('=')+l->pattern,l->flags);;}
    break;

  case 9:
#line 181 "grammar.y"
    { yyval.node = new ThisNode(); ;}
    break;

  case 10:
#line 182 "grammar.y"
    { yyval.node = new ResolveNode(*yyvsp[0].ident);
                                     delete yyvsp[0].ident; ;}
    break;

  case 13:
#line 186 "grammar.y"
    { yyval.node = new GroupNode(yyvsp[-1].node); ;}
    break;

  case 14:
#line 187 "grammar.y"
    { yyval.node = new ObjectLiteralNode(); ;}
    break;

  case 15:
#line 188 "grammar.y"
    { yyval.node = new ObjectLiteralNode(yyvsp[-1].plist); ;}
    break;

  case 16:
#line 192 "grammar.y"
    { yyval.node = new ArrayNode(yyvsp[-1].ival); ;}
    break;

  case 17:
#line 193 "grammar.y"
    { yyval.node = new ArrayNode(yyvsp[-1].elm); ;}
    break;

  case 18:
#line 194 "grammar.y"
    { yyval.node = new ArrayNode(yyvsp[-1].ival, yyvsp[-3].elm); ;}
    break;

  case 19:
#line 198 "grammar.y"
    { yyval.elm = new ElementNode(yyvsp[-1].ival, yyvsp[0].node); ;}
    break;

  case 20:
#line 200 "grammar.y"
    { yyval.elm = new ElementNode(yyvsp[-3].elm, yyvsp[-1].ival, yyvsp[0].node); ;}
    break;

  case 21:
#line 204 "grammar.y"
    { yyval.ival = 0; ;}
    break;

  case 23:
#line 209 "grammar.y"
    { yyval.ival = 1; ;}
    break;

  case 24:
#line 210 "grammar.y"
    { yyval.ival = yyvsp[-1].ival + 1; ;}
    break;

  case 25:
#line 214 "grammar.y"
    { yyval.plist = new PropertyValueNode(yyvsp[-2].pnode, yyvsp[0].node); ;}
    break;

  case 26:
#line 216 "grammar.y"
    { yyval.plist = new PropertyValueNode(yyvsp[-2].pnode, yyvsp[0].node, yyvsp[-4].plist); ;}
    break;

  case 27:
#line 220 "grammar.y"
    { yyval.pnode = new PropertyNode(*yyvsp[0].ident); delete yyvsp[0].ident; ;}
    break;

  case 28:
#line 221 "grammar.y"
    { yyval.pnode = new PropertyNode(Identifier(*yyvsp[0].ustr)); delete yyvsp[0].ustr; ;}
    break;

  case 29:
#line 222 "grammar.y"
    { yyval.pnode = new PropertyNode(yyvsp[0].dval); ;}
    break;

  case 32:
#line 228 "grammar.y"
    { yyval.node = new AccessorNode1(yyvsp[-3].node, yyvsp[-1].node); ;}
    break;

  case 33:
#line 229 "grammar.y"
    { yyval.node = new AccessorNode2(yyvsp[-2].node, *yyvsp[0].ident); delete yyvsp[0].ident; ;}
    break;

  case 34:
#line 230 "grammar.y"
    { yyval.node = new NewExprNode(yyvsp[-1].node, yyvsp[0].args); ;}
    break;

  case 36:
#line 235 "grammar.y"
    { yyval.node = new NewExprNode(yyvsp[0].node); ;}
    break;

  case 37:
#line 239 "grammar.y"
    { yyval.node = new FunctionCallNode(yyvsp[-1].node, yyvsp[0].args); ;}
    break;

  case 38:
#line 240 "grammar.y"
    { yyval.node = new FunctionCallNode(yyvsp[-1].node, yyvsp[0].args); ;}
    break;

  case 39:
#line 241 "grammar.y"
    { yyval.node = new AccessorNode1(yyvsp[-3].node, yyvsp[-1].node); ;}
    break;

  case 40:
#line 242 "grammar.y"
    { yyval.node = new AccessorNode2(yyvsp[-2].node, *yyvsp[0].ident);
                                     delete yyvsp[0].ident; ;}
    break;

  case 41:
#line 247 "grammar.y"
    { yyval.args = new ArgumentsNode(); ;}
    break;

  case 42:
#line 248 "grammar.y"
    { yyval.args = new ArgumentsNode(yyvsp[-1].alist); ;}
    break;

  case 43:
#line 252 "grammar.y"
    { yyval.alist = new ArgumentListNode(yyvsp[0].node); ;}
    break;

  case 44:
#line 253 "grammar.y"
    { yyval.alist = new ArgumentListNode(yyvsp[-2].alist, yyvsp[0].node); ;}
    break;

  case 48:
#line 263 "grammar.y"
    { yyval.node = new PostfixNode(yyvsp[-1].node, OpPlusPlus); ;}
    break;

  case 49:
#line 264 "grammar.y"
    { yyval.node = new PostfixNode(yyvsp[-1].node, OpMinusMinus); ;}
    break;

  case 51:
#line 269 "grammar.y"
    { yyval.node = new DeleteNode(yyvsp[0].node); ;}
    break;

  case 52:
#line 270 "grammar.y"
    { yyval.node = new VoidNode(yyvsp[0].node); ;}
    break;

  case 53:
#line 271 "grammar.y"
    { yyval.node = new TypeOfNode(yyvsp[0].node); ;}
    break;

  case 54:
#line 272 "grammar.y"
    { yyval.node = new PrefixNode(OpPlusPlus, yyvsp[0].node); ;}
    break;

  case 55:
#line 273 "grammar.y"
    { yyval.node = new PrefixNode(OpPlusPlus, yyvsp[0].node); ;}
    break;

  case 56:
#line 274 "grammar.y"
    { yyval.node = new PrefixNode(OpMinusMinus, yyvsp[0].node); ;}
    break;

  case 57:
#line 275 "grammar.y"
    { yyval.node = new PrefixNode(OpMinusMinus, yyvsp[0].node); ;}
    break;

  case 58:
#line 276 "grammar.y"
    { yyval.node = new UnaryPlusNode(yyvsp[0].node); ;}
    break;

  case 59:
#line 277 "grammar.y"
    { yyval.node = new NegateNode(yyvsp[0].node); ;}
    break;

  case 60:
#line 278 "grammar.y"
    { yyval.node = new BitwiseNotNode(yyvsp[0].node); ;}
    break;

  case 61:
#line 279 "grammar.y"
    { yyval.node = new LogicalNotNode(yyvsp[0].node); ;}
    break;

  case 63:
#line 284 "grammar.y"
    { yyval.node = new MultNode(yyvsp[-2].node, yyvsp[0].node, '*'); ;}
    break;

  case 64:
#line 285 "grammar.y"
    { yyval.node = new MultNode(yyvsp[-2].node, yyvsp[0].node, '/'); ;}
    break;

  case 65:
#line 286 "grammar.y"
    { yyval.node = new MultNode(yyvsp[-2].node,yyvsp[0].node,'%'); ;}
    break;

  case 67:
#line 291 "grammar.y"
    { yyval.node = AddNode::create(yyvsp[-2].node, yyvsp[0].node, '+'); ;}
    break;

  case 68:
#line 292 "grammar.y"
    { yyval.node = AddNode::create(yyvsp[-2].node, yyvsp[0].node, '-'); ;}
    break;

  case 70:
#line 297 "grammar.y"
    { yyval.node = new ShiftNode(yyvsp[-2].node, OpLShift, yyvsp[0].node); ;}
    break;

  case 71:
#line 298 "grammar.y"
    { yyval.node = new ShiftNode(yyvsp[-2].node, OpRShift, yyvsp[0].node); ;}
    break;

  case 72:
#line 299 "grammar.y"
    { yyval.node = new ShiftNode(yyvsp[-2].node, OpURShift, yyvsp[0].node); ;}
    break;

  case 74:
#line 305 "grammar.y"
    { yyval.node = new RelationalNode(yyvsp[-2].node, OpLess, yyvsp[0].node); ;}
    break;

  case 75:
#line 307 "grammar.y"
    { yyval.node = new RelationalNode(yyvsp[-2].node, OpGreater, yyvsp[0].node); ;}
    break;

  case 76:
#line 309 "grammar.y"
    { yyval.node = new RelationalNode(yyvsp[-2].node, OpLessEq, yyvsp[0].node); ;}
    break;

  case 77:
#line 311 "grammar.y"
    { yyval.node = new RelationalNode(yyvsp[-2].node, OpGreaterEq, yyvsp[0].node); ;}
    break;

  case 78:
#line 313 "grammar.y"
    { yyval.node = new RelationalNode(yyvsp[-2].node, OpInstanceOf, yyvsp[0].node); ;}
    break;

  case 79:
#line 315 "grammar.y"
    { yyval.node = new RelationalNode(yyvsp[-2].node, OpIn, yyvsp[0].node); ;}
    break;

  case 81:
#line 320 "grammar.y"
    { yyval.node = new EqualNode(yyvsp[-2].node, OpEqEq, yyvsp[0].node); ;}
    break;

  case 82:
#line 321 "grammar.y"
    { yyval.node = new EqualNode(yyvsp[-2].node, OpNotEq, yyvsp[0].node); ;}
    break;

  case 83:
#line 322 "grammar.y"
    { yyval.node = new EqualNode(yyvsp[-2].node, OpStrEq, yyvsp[0].node); ;}
    break;

  case 84:
#line 323 "grammar.y"
    { yyval.node = new EqualNode(yyvsp[-2].node, OpStrNEq, yyvsp[0].node);;}
    break;

  case 86:
#line 328 "grammar.y"
    { yyval.node = new BitOperNode(yyvsp[-2].node, OpBitAnd, yyvsp[0].node); ;}
    break;

  case 88:
#line 333 "grammar.y"
    { yyval.node = new BitOperNode(yyvsp[-2].node, OpBitXOr, yyvsp[0].node); ;}
    break;

  case 90:
#line 338 "grammar.y"
    { yyval.node = new BitOperNode(yyvsp[-2].node, OpBitOr, yyvsp[0].node); ;}
    break;

  case 92:
#line 344 "grammar.y"
    { yyval.node = new BinaryLogicalNode(yyvsp[-2].node, OpAnd, yyvsp[0].node); ;}
    break;

  case 94:
#line 350 "grammar.y"
    { yyval.node = new BinaryLogicalNode(yyvsp[-2].node, OpOr, yyvsp[0].node); ;}
    break;

  case 96:
#line 356 "grammar.y"
    { yyval.node = new ConditionalNode(yyvsp[-4].node, yyvsp[-2].node, yyvsp[0].node); ;}
    break;

  case 98:
#line 362 "grammar.y"
    { yyval.node = new AssignNode(yyvsp[-2].node, yyvsp[-1].op, yyvsp[0].node);;}
    break;

  case 99:
#line 366 "grammar.y"
    { yyval.op = OpEqual; ;}
    break;

  case 100:
#line 367 "grammar.y"
    { yyval.op = OpPlusEq; ;}
    break;

  case 101:
#line 368 "grammar.y"
    { yyval.op = OpMinusEq; ;}
    break;

  case 102:
#line 369 "grammar.y"
    { yyval.op = OpMultEq; ;}
    break;

  case 103:
#line 370 "grammar.y"
    { yyval.op = OpDivEq; ;}
    break;

  case 104:
#line 371 "grammar.y"
    { yyval.op = OpLShift; ;}
    break;

  case 105:
#line 372 "grammar.y"
    { yyval.op = OpRShift; ;}
    break;

  case 106:
#line 373 "grammar.y"
    { yyval.op = OpURShift; ;}
    break;

  case 107:
#line 374 "grammar.y"
    { yyval.op = OpAndEq; ;}
    break;

  case 108:
#line 375 "grammar.y"
    { yyval.op = OpXOrEq; ;}
    break;

  case 109:
#line 376 "grammar.y"
    { yyval.op = OpOrEq; ;}
    break;

  case 110:
#line 377 "grammar.y"
    { yyval.op = OpModEq; ;}
    break;

  case 112:
#line 382 "grammar.y"
    { yyval.node = new CommaNode(yyvsp[-2].node, yyvsp[0].node); ;}
    break;

  case 127:
#line 403 "grammar.y"
    { yyval.stat = new BlockNode(0); DBG(yyval.stat, yylsp[0], yylsp[0]); ;}
    break;

  case 128:
#line 404 "grammar.y"
    { yyval.stat = new BlockNode(yyvsp[-1].srcs); DBG(yyval.stat, yylsp[0], yylsp[0]); ;}
    break;

  case 129:
#line 408 "grammar.y"
    { yyval.slist = new StatListNode(yyvsp[0].stat); ;}
    break;

  case 130:
#line 409 "grammar.y"
    { yyval.slist = new StatListNode(yyvsp[-1].slist, yyvsp[0].stat); ;}
    break;

  case 131:
#line 413 "grammar.y"
    { yyval.stat = new VarStatementNode(yyvsp[-1].vlist);
                                      DBG(yyval.stat, yylsp[-2], yylsp[0]); ;}
    break;

  case 132:
#line 415 "grammar.y"
    { if (automatic()) {
                                          yyval.stat = new VarStatementNode(yyvsp[-1].vlist);
					  DBG(yyval.stat, yylsp[-2], yylsp[-1]);
                                        } else {
					  YYABORT;
					}
                                      ;}
    break;

  case 133:
#line 425 "grammar.y"
    { yyval.vlist = new VarDeclListNode(yyvsp[0].decl); ;}
    break;

  case 134:
#line 427 "grammar.y"
    { yyval.vlist = new VarDeclListNode(yyvsp[-2].vlist, yyvsp[0].decl); ;}
    break;

  case 135:
#line 431 "grammar.y"
    { yyval.decl = new VarDeclNode(*yyvsp[0].ident, 0); delete yyvsp[0].ident; ;}
    break;

  case 136:
#line 432 "grammar.y"
    { yyval.decl = new VarDeclNode(*yyvsp[-1].ident, yyvsp[0].init); delete yyvsp[-1].ident; ;}
    break;

  case 137:
#line 436 "grammar.y"
    { yyval.init = new AssignExprNode(yyvsp[0].node); ;}
    break;

  case 138:
#line 440 "grammar.y"
    { yyval.stat = new EmptyStatementNode(); DBG(yyval.stat, yylsp[0], yylsp[0]); ;}
    break;

  case 139:
#line 444 "grammar.y"
    { yyval.stat = new ExprStatementNode(yyvsp[-1].node);
                                     DBG(yyval.stat, yylsp[-1], yylsp[0]); ;}
    break;

  case 140:
#line 446 "grammar.y"
    { if (automatic()) {
                                       yyval.stat = new ExprStatementNode(yyvsp[-1].node);
				       DBG(yyval.stat, yylsp[-1], yylsp[-1]);
                                     } else
				       YYABORT; ;}
    break;

  case 141:
#line 454 "grammar.y"
    { yyval.stat = new IfNode(yyvsp[-2].node,yyvsp[0].stat,0);DBG(yyval.stat,yylsp[-4],yylsp[-1]); ;}
    break;

  case 142:
#line 456 "grammar.y"
    { yyval.stat = new IfNode(yyvsp[-4].node,yyvsp[-2].stat,yyvsp[0].stat);DBG(yyval.stat,yylsp[-6],yylsp[-3]); ;}
    break;

  case 143:
#line 460 "grammar.y"
    { yyval.stat=new DoWhileNode(yyvsp[-4].stat,yyvsp[-1].node);DBG(yyval.stat,yylsp[-5],yylsp[-3]);;}
    break;

  case 144:
#line 461 "grammar.y"
    { yyval.stat = new WhileNode(yyvsp[-2].node,yyvsp[0].stat);DBG(yyval.stat,yylsp[-4],yylsp[-1]); ;}
    break;

  case 145:
#line 463 "grammar.y"
    { yyval.stat = new ForNode(yyvsp[-6].node,yyvsp[-4].node,yyvsp[-2].node,yyvsp[0].stat);
	                             DBG(yyval.stat,yylsp[-8],yylsp[-1]); ;}
    break;

  case 146:
#line 466 "grammar.y"
    { yyval.stat = new ForNode(yyvsp[-6].vlist,yyvsp[-4].node,yyvsp[-2].node,yyvsp[0].stat);
	                             DBG(yyval.stat,yylsp[-9],yylsp[-1]); ;}
    break;

  case 147:
#line 469 "grammar.y"
    { yyval.stat = new ForInNode(yyvsp[-4].node, yyvsp[-2].node, yyvsp[0].stat);
	                             DBG(yyval.stat,yylsp[-6],yylsp[-1]); ;}
    break;

  case 148:
#line 472 "grammar.y"
    { yyval.stat = new ForInNode(*yyvsp[-4].ident,0,yyvsp[-2].node,yyvsp[0].stat);
	                             DBG(yyval.stat,yylsp[-7],yylsp[-1]);
                                     delete yyvsp[-4].ident; ;}
    break;

  case 149:
#line 476 "grammar.y"
    { yyval.stat = new ForInNode(*yyvsp[-5].ident,yyvsp[-4].init,yyvsp[-2].node,yyvsp[0].stat);
	                             DBG(yyval.stat,yylsp[-8],yylsp[-1]);
                                     delete yyvsp[-5].ident; ;}
    break;

  case 150:
#line 482 "grammar.y"
    { yyval.node = 0; ;}
    break;

  case 152:
#line 487 "grammar.y"
    { yyval.stat = new ContinueNode(); DBG(yyval.stat,yylsp[-1],yylsp[0]); ;}
    break;

  case 153:
#line 488 "grammar.y"
    { if (automatic()) {
                                       yyval.stat = new ContinueNode(); DBG(yyval.stat,yylsp[-1],yylsp[0]);
                                     } else
				       YYABORT; ;}
    break;

  case 154:
#line 492 "grammar.y"
    { yyval.stat = new ContinueNode(*yyvsp[-1].ident); DBG(yyval.stat,yylsp[-2],yylsp[0]);
                                     delete yyvsp[-1].ident; ;}
    break;

  case 155:
#line 494 "grammar.y"
    { if (automatic()) {
                                       yyval.stat = new ContinueNode(*yyvsp[-1].ident);DBG(yyval.stat,yylsp[-2],yylsp[-1]);
				       delete yyvsp[-1].ident;
                                     } else
				       YYABORT; ;}
    break;

  case 156:
#line 502 "grammar.y"
    { yyval.stat = new BreakNode();DBG(yyval.stat,yylsp[-1],yylsp[0]); ;}
    break;

  case 157:
#line 503 "grammar.y"
    { if (automatic()) {
                                       yyval.stat = new BreakNode(); DBG(yyval.stat,yylsp[-1],yylsp[-1]);
                                     } else
				       YYABORT; ;}
    break;

  case 158:
#line 507 "grammar.y"
    { yyval.stat = new BreakNode(*yyvsp[-1].ident); DBG(yyval.stat,yylsp[-2],yylsp[0]);
                                     delete yyvsp[-1].ident; ;}
    break;

  case 159:
#line 509 "grammar.y"
    { if (automatic()) {
                                       yyval.stat = new BreakNode(*yyvsp[-1].ident); DBG(yyval.stat,yylsp[-2],yylsp[-1]);
				       delete yyvsp[-1].ident;
                                     } else
				       YYABORT;
                                   ;}
    break;

  case 160:
#line 518 "grammar.y"
    { yyval.stat = new ReturnNode(0); DBG(yyval.stat,yylsp[-1],yylsp[0]); ;}
    break;

  case 161:
#line 519 "grammar.y"
    { if (automatic()) {
                                       yyval.stat = new ReturnNode(0); DBG(yyval.stat,yylsp[-1],yylsp[-1]);
                                     } else
				       YYABORT; ;}
    break;

  case 162:
#line 523 "grammar.y"
    { yyval.stat = new ReturnNode(yyvsp[-1].node); DBG(yyval.stat,yylsp[-2],yylsp[0]); ;}
    break;

  case 163:
#line 524 "grammar.y"
    { if (automatic()) {
                                       yyval.stat = new ReturnNode(yyvsp[-1].node); DBG(yyval.stat,yylsp[-2],yylsp[-2]);
                                     }
                                     else
				       YYABORT; ;}
    break;

  case 164:
#line 532 "grammar.y"
    { yyval.stat = new WithNode(yyvsp[-2].node,yyvsp[0].stat);
                                     DBG(yyval.stat, yylsp[-4], yylsp[-1]); ;}
    break;

  case 165:
#line 537 "grammar.y"
    { yyval.stat = new SwitchNode(yyvsp[-2].node, yyvsp[0].cblk);
                                     DBG(yyval.stat, yylsp[-4], yylsp[-1]); ;}
    break;

  case 166:
#line 542 "grammar.y"
    { yyval.cblk = new CaseBlockNode(yyvsp[-1].clist, 0, 0); ;}
    break;

  case 167:
#line 544 "grammar.y"
    { yyval.cblk = new CaseBlockNode(yyvsp[-3].clist, yyvsp[-2].ccl, yyvsp[-1].clist); ;}
    break;

  case 168:
#line 548 "grammar.y"
    { yyval.clist = 0; ;}
    break;

  case 170:
#line 553 "grammar.y"
    { yyval.clist = new ClauseListNode(yyvsp[0].ccl); ;}
    break;

  case 171:
#line 554 "grammar.y"
    { yyval.clist = new ClauseListNode(yyvsp[-1].clist, yyvsp[0].ccl); ;}
    break;

  case 172:
#line 558 "grammar.y"
    { yyval.ccl = new CaseClauseNode(yyvsp[-1].node); ;}
    break;

  case 173:
#line 559 "grammar.y"
    { yyval.ccl = new CaseClauseNode(yyvsp[-2].node, yyvsp[0].slist); ;}
    break;

  case 174:
#line 563 "grammar.y"
    { yyval.ccl = new CaseClauseNode(0); ;}
    break;

  case 175:
#line 564 "grammar.y"
    { yyval.ccl = new CaseClauseNode(0, yyvsp[0].slist); ;}
    break;

  case 176:
#line 568 "grammar.y"
    { yyvsp[0].stat->pushLabel(*yyvsp[-2].ident);
                                     yyval.stat = new LabelNode(*yyvsp[-2].ident, yyvsp[0].stat); DBG(yyval.stat,yylsp[-2],yylsp[-1]);
                                     delete yyvsp[-2].ident; ;}
    break;

  case 177:
#line 574 "grammar.y"
    { yyval.stat = new ThrowNode(yyvsp[-1].node); DBG(yyval.stat,yylsp[-2],yylsp[0]); ;}
    break;

  case 178:
#line 578 "grammar.y"
    { yyval.stat = new TryNode(yyvsp[-1].stat, yyvsp[0].cnode); DBG(yyval.stat,yylsp[-2],yylsp[-2]); ;}
    break;

  case 179:
#line 579 "grammar.y"
    { yyval.stat = new TryNode(yyvsp[-1].stat, yyvsp[0].fnode); DBG(yyval.stat,yylsp[-2],yylsp[-2]); ;}
    break;

  case 180:
#line 580 "grammar.y"
    { yyval.stat = new TryNode(yyvsp[-2].stat, yyvsp[-1].cnode, yyvsp[0].fnode); DBG(yyval.stat,yylsp[-3],yylsp[-3]); ;}
    break;

  case 181:
#line 584 "grammar.y"
    { CatchNode *c; yyval.cnode = c = new CatchNode(*yyvsp[-2].ident, yyvsp[0].stat);
				     delete yyvsp[-2].ident; DBG(c,yylsp[-4],yylsp[-1]); ;}
    break;

  case 182:
#line 589 "grammar.y"
    { FinallyNode *f; yyval.fnode = f = new FinallyNode(yyvsp[0].stat); DBG(f,yylsp[-1],yylsp[-1]); ;}
    break;

  case 184:
#line 595 "grammar.y"
    { yyval.func = yyvsp[0].func; ;}
    break;

  case 185:
#line 599 "grammar.y"
    { yyval.func = new FuncDeclNode(*yyvsp[-3].ident, yyvsp[0].body); DBG(yyval.func,yylsp[-4],yylsp[-1]);
                                             delete yyvsp[-3].ident; ;}
    break;

  case 186:
#line 602 "grammar.y"
    { yyval.func = new FuncDeclNode(*yyvsp[-4].ident, yyvsp[-2].param, yyvsp[0].body); DBG(yyval.func,yylsp[-5],yylsp[-1]);
                                     delete yyvsp[-4].ident; ;}
    break;

  case 187:
#line 607 "grammar.y"
    { yyval.node = new FuncExprNode(yyvsp[0].body); ;}
    break;

  case 188:
#line 609 "grammar.y"
    { yyval.node = new FuncExprNode(yyvsp[-2].param, yyvsp[0].body); ;}
    break;

  case 189:
#line 614 "grammar.y"
    { yyval.param = new ParameterNode(*yyvsp[0].ident); delete yyvsp[0].ident; ;}
    break;

  case 190:
#line 615 "grammar.y"
    { yyval.param = new ParameterNode(yyvsp[-2].param, *yyvsp[0].ident); delete yyvsp[0].ident; ;}
    break;

  case 191:
#line 619 "grammar.y"
    { yyval.body = new FunctionBodyNode(0);
	                             DBG(yyval.body, yylsp[-1], yylsp[0]);;}
    break;

  case 192:
#line 621 "grammar.y"
    { yyval.body = new FunctionBodyNode(yyvsp[-1].srcs);
	                             DBG(yyval.body, yylsp[-2], yylsp[0]);;}
    break;

  case 193:
#line 626 "grammar.y"
    { yyval.prog = new FunctionBodyNode(0);
                                     yyval.prog->setLoc(0, 0, Parser::source);
                                     Parser::progNode = yyval.prog; ;}
    break;

  case 194:
#line 629 "grammar.y"
    { yyval.prog = new FunctionBodyNode(yyvsp[0].srcs);
                                     Parser::progNode = yyval.prog; ;}
    break;

  case 195:
#line 634 "grammar.y"
    { yyval.srcs = new SourceElementsNode(yyvsp[0].stat); ;}
    break;

  case 196:
#line 635 "grammar.y"
    { yyval.srcs = new SourceElementsNode(yyvsp[-1].srcs, yyvsp[0].stat); ;}
    break;

  case 197:
#line 639 "grammar.y"
    { yyval.stat = yyvsp[0].stat; ;}
    break;

  case 198:
#line 640 "grammar.y"
    { yyval.stat = yyvsp[0].func; ;}
    break;


    }

/* Line 991 of yacc.c.  */
#line 2598 "grammar.tab.c"

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


#line 643 "grammar.y"


int yyerror (const char *)  /* Called by yyparse on error */
{
//  fprintf(stderr, "ERROR: %s at line %d\n",
//	  s, KJScript::lexer()->lineNo());
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

