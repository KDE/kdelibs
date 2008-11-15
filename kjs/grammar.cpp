/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

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

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.3"

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
     CONSTTOKEN = 267,
     CONTINUE = 268,
     FUNCTION = 269,
     RETURN = 270,
     VOIDTOKEN = 271,
     DELETETOKEN = 272,
     IF = 273,
     THISTOKEN = 274,
     DO = 275,
     WHILE = 276,
     INTOKEN = 277,
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
#define CONSTTOKEN 267
#define CONTINUE 268
#define FUNCTION 269
#define RETURN 270
#define VOIDTOKEN 271
#define DELETETOKEN 272
#define IF 273
#define THISTOKEN 274
#define DO 275
#define WHILE 276
#define INTOKEN 277
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
 *  Copyright (C) 2006, 2007 Apple Inc. All rights reserved.
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
#include "makenodes.h"
#include "lexer.h"
#include "internal.h"

// Not sure why, but yacc doesn't add this define along with the others.
#define yylloc kjsyylloc

/* default values for bison */
#define YYDEBUG 0 // Set to 1 to debug a parse error.
#define kjsyydebug 0 // Set to 1 to debug a parse error.
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

#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 59 "grammar.y"
{
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
}
/* Line 187 of yacc.c.  */
#line 319 "grammar.tab.c"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
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


/* Line 216 of yacc.c.  */
#line 344 "grammar.tab.c"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

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

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int i)
#else
static int
YYID (i)
    int i;
#endif
{
  return i;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL \
	     && defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss;
  YYSTYPE yyvs;
    YYLTYPE yyls;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE) + sizeof (YYLTYPE)) \
      + 2 * YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
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
      while (YYID (0))
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
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  212
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1503

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  89
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  112
/* YYNRULES -- Number of rules.  */
#define YYNRULES  319
/* YYNRULES -- Number of states.  */
#define YYNSTATES  554

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   319

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
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
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     5,     7,     9,    11,    13,    15,    17,
      19,    21,    23,    27,    28,    35,    36,    44,    46,    50,
      52,    55,    59,    64,    66,    68,    70,    72,    76,    80,
      84,    90,    93,    98,    99,   101,   103,   106,   108,   110,
     115,   119,   123,   125,   130,   134,   138,   140,   143,   145,
     148,   151,   154,   159,   163,   166,   169,   174,   178,   181,
     185,   187,   191,   193,   195,   197,   199,   201,   204,   207,
     209,   212,   215,   218,   221,   224,   227,   230,   233,   236,
     239,   242,   245,   248,   250,   252,   254,   256,   258,   262,
     266,   270,   272,   276,   280,   284,   286,   290,   294,   296,
     300,   304,   306,   310,   314,   318,   320,   324,   328,   332,
     334,   338,   342,   346,   350,   354,   358,   360,   364,   368,
     372,   376,   380,   382,   386,   390,   394,   398,   402,   406,
     408,   412,   416,   420,   424,   426,   430,   434,   438,   442,
     444,   448,   452,   456,   460,   462,   466,   468,   472,   474,
     478,   480,   484,   486,   490,   492,   496,   498,   502,   504,
     508,   510,   514,   516,   520,   522,   526,   528,   532,   534,
     538,   540,   544,   546,   550,   552,   558,   560,   566,   568,
     574,   576,   580,   582,   586,   588,   592,   594,   596,   598,
     600,   602,   604,   606,   608,   610,   612,   614,   616,   618,
     622,   624,   628,   630,   634,   636,   638,   640,   642,   644,
     646,   648,   650,   652,   654,   656,   658,   660,   662,   664,
     666,   668,   671,   675,   679,   683,   685,   689,   691,   695,
     697,   700,   702,   705,   709,   713,   715,   719,   721,   724,
     727,   730,   732,   735,   738,   744,   752,   760,   767,   773,
     783,   794,   802,   811,   821,   822,   824,   825,   827,   830,
     833,   837,   841,   844,   847,   851,   855,   858,   861,   865,
     869,   875,   881,   885,   891,   892,   894,   896,   899,   903,
     908,   911,   915,   919,   923,   927,   932,   940,   950,   953,
     956,   958,   962,   968,   974,   978,   982,   988,   994,   995,
    1002,  1003,  1011,  1012,  1018,  1019,  1026,  1027,  1034,  1035,
    1043,  1045,  1049,  1052,  1056,  1057,  1059,  1061,  1064,  1066
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     198,     0,    -1,     3,    -1,     4,    -1,     5,    -1,    60,
      -1,    61,    -1,    65,    -1,    52,    -1,    62,    -1,    61,
      -1,    60,    -1,    91,    66,   147,    -1,    -1,    62,    62,
      67,    68,    93,   197,    -1,    -1,    62,    62,    67,   196,
      68,    94,   197,    -1,    92,    -1,    95,    69,    92,    -1,
      97,    -1,    70,    71,    -1,    70,    95,    71,    -1,    70,
      95,    69,    71,    -1,    19,    -1,    90,    -1,    98,    -1,
      62,    -1,    67,   151,    68,    -1,    72,   100,    73,    -1,
      72,    99,    73,    -1,    72,    99,    69,   100,    73,    -1,
     100,   147,    -1,    99,    69,   100,   147,    -1,    -1,   101,
      -1,    69,    -1,   101,    69,    -1,    96,    -1,   191,    -1,
     102,    72,   151,    73,    -1,   102,    74,    62,    -1,    10,
     102,   108,    -1,    97,    -1,   103,    72,   151,    73,    -1,
     103,    74,    62,    -1,    10,   102,   108,    -1,   102,    -1,
      10,   104,    -1,   103,    -1,    10,   104,    -1,   102,   108,
      -1,   106,   108,    -1,   106,    72,   151,    73,    -1,   106,
      74,    62,    -1,   103,   108,    -1,   107,   108,    -1,   107,
      72,   151,    73,    -1,   107,    74,    62,    -1,    67,    68,
      -1,    67,   109,    68,    -1,   147,    -1,   109,    69,   147,
      -1,   104,    -1,   106,    -1,   105,    -1,   107,    -1,   110,
      -1,   110,    44,    -1,   110,    45,    -1,   111,    -1,   111,
      44,    -1,   111,    45,    -1,    17,   115,    -1,    16,   115,
      -1,    24,   115,    -1,    44,   115,    -1,    63,   115,    -1,
      45,   115,    -1,    64,   115,    -1,    75,   115,    -1,    76,
     115,    -1,    77,   115,    -1,    78,   115,    -1,   112,    -1,
     114,    -1,   113,    -1,   114,    -1,   115,    -1,   117,    79,
     115,    -1,   117,    65,   115,    -1,   117,    80,   115,    -1,
     116,    -1,   118,    79,   115,    -1,   118,    65,   115,    -1,
     118,    80,   115,    -1,   117,    -1,   119,    75,   117,    -1,
     119,    76,   117,    -1,   118,    -1,   120,    75,   117,    -1,
     120,    76,   117,    -1,   119,    -1,   121,    46,   119,    -1,
     121,    47,   119,    -1,   121,    48,   119,    -1,   120,    -1,
     122,    46,   119,    -1,   122,    47,   119,    -1,   122,    48,
     119,    -1,   121,    -1,   123,    81,   121,    -1,   123,    82,
     121,    -1,   123,    40,   121,    -1,   123,    41,   121,    -1,
     123,    23,   121,    -1,   123,    22,   121,    -1,   121,    -1,
     124,    81,   121,    -1,   124,    82,   121,    -1,   124,    40,
     121,    -1,   124,    41,   121,    -1,   124,    23,   121,    -1,
     122,    -1,   125,    81,   121,    -1,   125,    82,   121,    -1,
     125,    40,   121,    -1,   125,    41,   121,    -1,   125,    23,
     121,    -1,   125,    22,   121,    -1,   123,    -1,   126,    36,
     123,    -1,   126,    37,   123,    -1,   126,    38,   123,    -1,
     126,    39,   123,    -1,   124,    -1,   127,    36,   124,    -1,
     127,    37,   124,    -1,   127,    38,   124,    -1,   127,    39,
     124,    -1,   125,    -1,   128,    36,   123,    -1,   128,    37,
     123,    -1,   128,    38,   123,    -1,   128,    39,   123,    -1,
     126,    -1,   129,    83,   126,    -1,   127,    -1,   130,    83,
     127,    -1,   128,    -1,   131,    83,   126,    -1,   129,    -1,
     132,    84,   129,    -1,   130,    -1,   133,    84,   130,    -1,
     131,    -1,   134,    84,   129,    -1,   132,    -1,   135,    85,
     132,    -1,   133,    -1,   136,    85,   133,    -1,   134,    -1,
     137,    85,   132,    -1,   135,    -1,   138,    43,   135,    -1,
     136,    -1,   139,    43,   136,    -1,   137,    -1,   140,    43,
     135,    -1,   138,    -1,   141,    42,   138,    -1,   139,    -1,
     142,    42,   139,    -1,   140,    -1,   143,    42,   138,    -1,
     141,    -1,   141,    86,   147,    66,   147,    -1,   142,    -1,
     142,    86,   148,    66,   148,    -1,   143,    -1,   143,    86,
     147,    66,   147,    -1,   144,    -1,   110,   150,   147,    -1,
     145,    -1,   110,   150,   148,    -1,   146,    -1,   111,   150,
     147,    -1,    87,    -1,    49,    -1,    50,    -1,    51,    -1,
      52,    -1,    53,    -1,    54,    -1,    55,    -1,    56,    -1,
      58,    -1,    59,    -1,    57,    -1,   147,    -1,   151,    69,
     147,    -1,   148,    -1,   152,    69,   148,    -1,   149,    -1,
     153,    69,   147,    -1,   155,    -1,   156,    -1,   161,    -1,
     166,    -1,   167,    -1,   168,    -1,   169,    -1,   172,    -1,
     173,    -1,   174,    -1,   175,    -1,   176,    -1,   182,    -1,
     183,    -1,   184,    -1,   185,    -1,   187,    -1,    70,    71,
      -1,    70,   199,    71,    -1,    11,   157,    88,    -1,    11,
     157,     1,    -1,   159,    -1,   157,    69,   159,    -1,   160,
      -1,   158,    69,   160,    -1,    62,    -1,    62,   164,    -1,
      62,    -1,    62,   165,    -1,    12,   162,    88,    -1,    12,
     162,     1,    -1,   163,    -1,   162,    69,   163,    -1,    62,
      -1,    62,   164,    -1,    87,   147,    -1,    87,   148,    -1,
      88,    -1,   153,    88,    -1,   153,     1,    -1,    18,    67,
     151,    68,   154,    -1,    18,    67,   151,    68,   154,    35,
     154,    -1,    20,   154,    21,    67,   151,    68,    88,    -1,
      20,   154,    21,    67,   151,    68,    -1,    21,    67,   151,
      68,   154,    -1,     9,    67,   171,    88,   170,    88,   170,
      68,   154,    -1,     9,    67,    11,   158,    88,   170,    88,
     170,    68,   154,    -1,     9,    67,   110,    22,   151,    68,
     154,    -1,     9,    67,    11,    62,    22,   151,    68,   154,
      -1,     9,    67,    11,    62,   165,    22,   151,    68,   154,
      -1,    -1,   151,    -1,    -1,   152,    -1,    13,    88,    -1,
      13,     1,    -1,    13,    62,    88,    -1,    13,    62,     1,
      -1,     6,    88,    -1,     6,     1,    -1,     6,    62,    88,
      -1,     6,    62,     1,    -1,    15,    88,    -1,    15,     1,
      -1,    15,   151,    88,    -1,    15,   151,     1,    -1,    26,
      67,   151,    68,   154,    -1,    25,    67,   151,    68,   177,
      -1,    70,   178,    71,    -1,    70,   178,   181,   178,    71,
      -1,    -1,   179,    -1,   180,    -1,   179,   180,    -1,     7,
     151,    66,    -1,     7,   151,    66,   199,    -1,     8,    66,
      -1,     8,    66,   199,    -1,    62,    66,   154,    -1,    28,
     151,    88,    -1,    28,   151,     1,    -1,    29,   155,    31,
     155,    -1,    29,   155,    30,    67,    62,    68,   155,    -1,
      29,   155,    30,    67,    62,    68,   155,    31,   155,    -1,
      32,    88,    -1,    32,     1,    -1,    62,    -1,   186,    74,
      62,    -1,    33,   186,    74,    79,    88,    -1,    33,   186,
      74,    79,     1,    -1,    33,   186,    88,    -1,    33,   186,
       1,    -1,    33,    62,    87,   186,    88,    -1,    33,    62,
      87,   186,     1,    -1,    -1,    14,    62,    67,    68,   189,
     197,    -1,    -1,    14,    62,    67,   196,    68,   190,   197,
      -1,    -1,    14,    67,    68,   192,   197,    -1,    -1,    14,
      67,   196,    68,   193,   197,    -1,    -1,    14,    62,    67,
      68,   194,   197,    -1,    -1,    14,    62,    67,   196,    68,
     195,   197,    -1,    62,    -1,   196,    69,    62,    -1,    70,
      71,    -1,    70,   199,    71,    -1,    -1,   199,    -1,   200,
      -1,   199,   200,    -1,   188,    -1,   154,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   185,   185,   186,   187,   188,   189,   190,   196,   205,
     206,   207,   211,   212,   212,   216,   216,   223,   224,   228,
     229,   230,   232,   236,   237,   238,   239,   240,   245,   246,
     247,   251,   252,   257,   258,   262,   263,   267,   268,   269,
     270,   271,   275,   276,   277,   278,   282,   283,   287,   288,
     292,   293,   294,   295,   299,   300,   301,   302,   306,   307,
     311,   312,   316,   317,   321,   322,   326,   327,   328,   332,
     333,   334,   338,   339,   340,   341,   342,   343,   344,   345,
     346,   347,   348,   351,   352,   356,   357,   361,   362,   363,
     364,   368,   369,   371,   373,   378,   379,   380,   384,   385,
     387,   392,   393,   394,   395,   399,   400,   401,   402,   406,
     407,   408,   409,   410,   411,   412,   416,   417,   418,   419,
     420,   421,   426,   427,   428,   429,   430,   431,   433,   437,
     438,   439,   440,   441,   445,   446,   448,   450,   452,   457,
     458,   460,   461,   463,   468,   469,   473,   474,   479,   480,
     484,   485,   489,   490,   495,   496,   501,   502,   506,   507,
     512,   513,   518,   519,   523,   524,   529,   530,   535,   536,
     540,   541,   546,   547,   551,   552,   557,   558,   563,   564,
     569,   570,   575,   576,   581,   582,   587,   588,   589,   590,
     591,   592,   593,   594,   595,   596,   597,   598,   602,   603,
     607,   608,   612,   613,   617,   618,   619,   620,   621,   622,
     623,   624,   625,   626,   627,   628,   629,   630,   631,   632,
     633,   637,   638,   642,   643,   647,   648,   653,   654,   659,
     660,   664,   665,   669,   670,   675,   676,   681,   682,   686,
     690,   694,   698,   699,   703,   705,   710,   711,   712,   713,
     715,   717,   725,   727,   732,   733,   737,   738,   742,   743,
     744,   745,   749,   750,   751,   752,   756,   757,   758,   759,
     763,   767,   771,   772,   777,   778,   782,   783,   787,   788,
     792,   793,   797,   801,   802,   806,   807,   808,   813,   814,
     818,   819,   823,   825,   827,   829,   831,   833,   838,   838,
     839,   839,   844,   844,   847,   847,   850,   850,   851,   851,
     857,   858,   862,   863,   867,   868,   872,   873,   877,   878
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "NULLTOKEN", "TRUETOKEN", "FALSETOKEN",
  "BREAK", "CASE", "DEFAULT", "FOR", "NEW", "VAR", "CONSTTOKEN",
  "CONTINUE", "FUNCTION", "RETURN", "VOIDTOKEN", "DELETETOKEN", "IF",
  "THISTOKEN", "DO", "WHILE", "INTOKEN", "INSTANCEOF", "TYPEOF", "SWITCH",
  "WITH", "RESERVED", "THROW", "TRY", "CATCH", "FINALLY", "DEBUGGER",
  "IMPORT", "IF_WITHOUT_ELSE", "ELSE", "EQEQ", "NE", "STREQ", "STRNEQ",
  "LE", "GE", "OR", "AND", "PLUSPLUS", "MINUSMINUS", "LSHIFT", "RSHIFT",
  "URSHIFT", "PLUSEQUAL", "MINUSEQUAL", "MULTEQUAL", "DIVEQUAL",
  "LSHIFTEQUAL", "RSHIFTEQUAL", "URSHIFTEQUAL", "ANDEQUAL", "MODEQUAL",
  "XOREQUAL", "OREQUAL", "NUMBER", "STRING", "IDENT", "AUTOPLUSPLUS",
  "AUTOMINUSMINUS", "'/'", "':'", "'('", "')'", "','", "'{'", "'}'", "'['",
  "']'", "'.'", "'+'", "'-'", "'~'", "'!'", "'*'", "'%'", "'<'", "'>'",
  "'&'", "'^'", "'|'", "'?'", "'='", "';'", "$accept", "Literal",
  "PropertyName", "Property", "@1", "@2", "PropertyList", "PrimaryExpr",
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
  "ExprNoIn", "ExprNoBF", "Statement", "Block", "VariableStatement",
  "VariableDeclarationList", "VariableDeclarationListNoIn",
  "VariableDeclaration", "VariableDeclarationNoIn", "ConstStatement",
  "ConstDeclarationList", "ConstDeclaration", "Initializer",
  "InitializerNoIn", "EmptyStatement", "ExprStatement", "IfStatement",
  "IterationStatement", "ExprOpt", "ExprNoInOpt", "ContinueStatement",
  "BreakStatement", "ReturnStatement", "WithStatement", "SwitchStatement",
  "CaseBlock", "CaseClausesOpt", "CaseClauses", "CaseClause",
  "DefaultClause", "LabelledStatement", "ThrowStatement", "TryStatement",
  "DebuggerStatement", "PackageName", "ImportStatement",
  "FunctionDeclaration", "@3", "@4", "FunctionExpr", "@5", "@6", "@7",
  "@8", "FormalParameterList", "FunctionBody", "Program", "SourceElements",
  "SourceElement", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
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
static const yytype_uint8 yyr1[] =
{
       0,    89,    90,    90,    90,    90,    90,    90,    90,    91,
      91,    91,    92,    93,    92,    94,    92,    95,    95,    96,
      96,    96,    96,    97,    97,    97,    97,    97,    98,    98,
      98,    99,    99,   100,   100,   101,   101,   102,   102,   102,
     102,   102,   103,   103,   103,   103,   104,   104,   105,   105,
     106,   106,   106,   106,   107,   107,   107,   107,   108,   108,
     109,   109,   110,   110,   111,   111,   112,   112,   112,   113,
     113,   113,   114,   114,   114,   114,   114,   114,   114,   114,
     114,   114,   114,   115,   115,   116,   116,   117,   117,   117,
     117,   118,   118,   118,   118,   119,   119,   119,   120,   120,
     120,   121,   121,   121,   121,   122,   122,   122,   122,   123,
     123,   123,   123,   123,   123,   123,   124,   124,   124,   124,
     124,   124,   125,   125,   125,   125,   125,   125,   125,   126,
     126,   126,   126,   126,   127,   127,   127,   127,   127,   128,
     128,   128,   128,   128,   129,   129,   130,   130,   131,   131,
     132,   132,   133,   133,   134,   134,   135,   135,   136,   136,
     137,   137,   138,   138,   139,   139,   140,   140,   141,   141,
     142,   142,   143,   143,   144,   144,   145,   145,   146,   146,
     147,   147,   148,   148,   149,   149,   150,   150,   150,   150,
     150,   150,   150,   150,   150,   150,   150,   150,   151,   151,
     152,   152,   153,   153,   154,   154,   154,   154,   154,   154,
     154,   154,   154,   154,   154,   154,   154,   154,   154,   154,
     154,   155,   155,   156,   156,   157,   157,   158,   158,   159,
     159,   160,   160,   161,   161,   162,   162,   163,   163,   164,
     165,   166,   167,   167,   168,   168,   169,   169,   169,   169,
     169,   169,   169,   169,   170,   170,   171,   171,   172,   172,
     172,   172,   173,   173,   173,   173,   174,   174,   174,   174,
     175,   176,   177,   177,   178,   178,   179,   179,   180,   180,
     181,   181,   182,   183,   183,   184,   184,   184,   185,   185,
     186,   186,   187,   187,   187,   187,   187,   187,   189,   188,
     190,   188,   192,   191,   193,   191,   194,   191,   195,   191,
     196,   196,   197,   197,   198,   198,   199,   199,   200,   200
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     3,     0,     6,     0,     7,     1,     3,     1,
       2,     3,     4,     1,     1,     1,     1,     3,     3,     3,
       5,     2,     4,     0,     1,     1,     2,     1,     1,     4,
       3,     3,     1,     4,     3,     3,     1,     2,     1,     2,
       2,     2,     4,     3,     2,     2,     4,     3,     2,     3,
       1,     3,     1,     1,     1,     1,     1,     2,     2,     1,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     1,     1,     1,     1,     1,     3,     3,
       3,     1,     3,     3,     3,     1,     3,     3,     1,     3,
       3,     1,     3,     3,     3,     1,     3,     3,     3,     1,
       3,     3,     3,     3,     3,     3,     1,     3,     3,     3,
       3,     3,     1,     3,     3,     3,     3,     3,     3,     1,
       3,     3,     3,     3,     1,     3,     3,     3,     3,     1,
       3,     3,     3,     3,     1,     3,     1,     3,     1,     3,
       1,     3,     1,     3,     1,     3,     1,     3,     1,     3,
       1,     3,     1,     3,     1,     3,     1,     3,     1,     3,
       1,     3,     1,     3,     1,     5,     1,     5,     1,     5,
       1,     3,     1,     3,     1,     3,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     3,
       1,     3,     1,     3,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     2,     3,     3,     3,     1,     3,     1,     3,     1,
       2,     1,     2,     3,     3,     1,     3,     1,     2,     2,
       2,     1,     2,     2,     5,     7,     7,     6,     5,     9,
      10,     7,     8,     9,     0,     1,     0,     1,     2,     2,
       3,     3,     2,     2,     3,     3,     2,     2,     3,     3,
       5,     5,     3,     5,     0,     1,     1,     2,     3,     4,
       2,     3,     3,     3,     3,     4,     7,     9,     2,     2,
       1,     3,     5,     5,     3,     3,     5,     5,     0,     6,
       0,     7,     0,     5,     0,     6,     0,     6,     0,     7,
       1,     3,     2,     3,     0,     1,     1,     2,     1,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
     314,     2,     3,     4,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    23,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     8,     5,     6,
      26,     0,     0,     7,     0,     0,    33,     0,     0,     0,
       0,   241,    24,    42,    25,    48,    64,    65,    69,    85,
      86,    91,    98,   105,   122,   139,   148,   154,   160,   166,
     172,   178,   184,   202,     0,   319,   204,   205,   206,   207,
     208,   209,   210,   211,   212,   213,   214,   215,   216,   217,
     218,   219,   220,   318,     0,   315,   316,   263,     0,   262,
     256,     0,     0,    26,     0,    37,    19,    46,    49,    38,
     229,     0,   225,   237,     0,   235,   259,     0,   258,     0,
     267,   266,    46,    62,    63,    66,    83,    84,    87,    95,
     101,   109,   129,   144,   150,   156,   162,   168,   174,   180,
     198,     0,    66,    73,    72,     0,     0,     0,    74,     0,
       0,     0,     0,   289,   288,   290,     0,    75,    77,     0,
      76,    78,     0,   221,     0,    35,     0,     0,    34,    79,
      80,    81,    82,     0,     0,     0,    54,     0,     0,    55,
      70,    71,   187,   188,   189,   190,   191,   192,   193,   194,
     197,   195,   196,   186,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   243,
       0,   242,     1,   317,   265,   264,     0,    66,   116,   134,
     146,   152,   158,   164,   170,   176,   182,   200,   257,     0,
      46,    47,     0,     0,    11,    10,     9,    20,     0,    17,
       0,     0,     0,    45,     0,   230,   224,     0,   223,   238,
     234,     0,   233,   261,   260,     0,    50,     0,     0,    51,
      67,    68,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   269,     0,   268,
       0,     0,     0,     0,     0,   284,   283,     0,     0,     0,
     295,     0,   294,   282,    27,   222,    33,    29,    28,    31,
      36,    58,     0,    60,     0,    44,     0,    57,   185,    93,
      92,    94,    99,   100,   106,   107,   108,   128,   127,   125,
     126,   123,   124,   140,   141,   142,   143,   149,   155,   161,
     167,   173,     0,   203,   231,     0,   227,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   254,    41,     0,   310,   302,
       0,     0,     0,     0,    21,     0,    40,   239,   226,   236,
     298,     0,     0,    53,   181,    89,    88,    90,    96,    97,
     102,   103,   104,   115,   114,   112,   113,   110,   111,   130,
     131,   132,   133,   145,   151,   157,   163,   169,     0,   199,
       0,     0,     0,     0,     0,     0,   285,   290,     0,   291,
       0,     0,    59,     0,    43,    56,     0,     0,     0,   232,
       0,   254,     0,    66,   183,   121,   119,   120,   117,   118,
     135,   136,   137,   138,   147,   153,   159,   165,   171,     0,
     201,   255,     0,   306,     0,     0,   304,     0,     0,    12,
      22,    18,    39,     0,   300,    52,     0,   244,     0,   248,
     274,   271,   270,     0,   297,     0,   296,   293,   292,    30,
      32,    61,   179,     0,   240,     0,   231,   228,     0,     0,
       0,   254,     0,   308,     0,   303,     0,   311,    13,     0,
     299,     0,   175,     0,   247,     0,     0,   275,   276,     0,
       0,     0,   232,   254,   251,   177,     0,   307,     0,   312,
       0,   305,     0,    15,   301,   245,   246,     0,     0,   272,
     274,   277,   286,   252,     0,     0,     0,   309,   313,    14,
       0,   278,   280,     0,     0,   253,     0,   249,    16,   279,
     281,   273,   287,   250
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    42,   238,   239,   522,   540,   240,    95,    96,    44,
     156,   157,   158,   112,    45,   113,    46,   114,    47,   166,
     312,   132,    48,   116,    49,   117,   118,    51,   119,    52,
     120,    53,   121,    54,   122,   219,    55,   123,   220,    56,
     124,   221,    57,   125,   222,    58,   126,   223,    59,   127,
     224,    60,   128,   225,    61,   129,   226,    62,   130,   227,
      63,   348,   451,   228,    64,    65,    66,    67,   101,   345,
     102,   346,    68,   104,   105,   245,   429,    69,    70,    71,
      72,   452,   229,    73,    74,    75,    76,    77,   471,   506,
     507,   508,   530,    78,    79,    80,    81,   146,    82,    83,
     463,   501,    99,   455,   496,   492,   518,   370,   495,    84,
      85,    86
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -415
static const yytype_int16 yypact[] =
{
     965,  -415,  -415,  -415,     9,   -17,   256,     3,    78,    13,
      91,   565,  1425,  1425,     6,  -415,  1045,    26,  1425,   110,
     139,  1425,    20,    17,   146,  1425,  1425,  -415,  -415,  -415,
     153,  1425,  1425,  -415,  1425,   645,   157,  1425,  1425,  1425,
    1425,  -415,  -415,  -415,  -415,    67,  -415,   123,  1178,  -415,
    -415,  -415,    71,   159,   266,   148,   214,   171,   188,   189,
     233,   -35,  -415,  -415,    19,  -415,  -415,  -415,  -415,  -415,
    -415,  -415,  -415,  -415,  -415,  -415,  -415,  -415,  -415,  -415,
    -415,  -415,  -415,  -415,   281,   965,  -415,  -415,    27,  -415,
    1121,   256,   -10,  -415,   122,  -415,  -415,   127,  -415,  -415,
     191,    22,  -415,   191,    23,  -415,  -415,    31,  -415,   219,
    -415,  -415,   127,  -415,   133,  1255,  -415,  -415,  -415,    94,
     167,   283,   181,   263,   213,   223,   225,   272,    -4,  -415,
    -415,    25,   239,  -415,  -415,  1425,   303,  1425,  -415,  1425,
    1425,    29,   289,  -415,  -415,   238,     7,  -415,  -415,  1045,
    -415,  -415,   264,  -415,   725,  -415,   -20,  1197,   258,  -415,
    -415,  -415,  -415,  1273,  1425,   273,  -415,  1425,   279,  -415,
    -415,  -415,  -415,  -415,  -415,  -415,  -415,  -415,  -415,  -415,
    -415,  -415,  -415,  -415,  1425,  1425,  1425,  1425,  1425,  1425,
    1425,  1425,  1425,  1425,  1425,  1425,  1425,  1425,  1425,  1425,
    1425,  1425,  1425,  1425,  1425,  1425,  1425,  1425,  1425,  -415,
    1425,  -415,  -415,  -415,  -415,  -415,   323,  1105,   283,   216,
     267,   251,   302,   305,   345,    18,  -415,  -415,   322,   304,
     127,  -415,   326,    21,  -415,  -415,   332,  -415,   329,  -415,
     109,  1425,   334,  -415,  1425,  -415,  -415,     3,  -415,  -415,
    -415,    78,  -415,  -415,  -415,   165,  -415,  1425,   335,  -415,
    -415,  -415,  1425,  1425,  1425,  1425,  1425,  1425,  1425,  1425,
    1425,  1425,  1425,  1425,  1425,  1425,  1425,  1425,  1425,  1425,
    1425,  1425,  1425,  1425,  1425,  1425,  1425,  -415,  1425,  -415,
     268,   331,   271,   275,   278,  -415,  -415,   333,    20,   342,
    -415,     1,  -415,  -415,  -415,  -415,   157,  -415,  -415,  -415,
    -415,  -415,   280,  -415,    64,  -415,   106,  -415,  -415,  -415,
    -415,  -415,    94,    94,   167,   167,   167,   283,   283,   283,
     283,   283,   283,   181,   181,   181,   181,   263,   213,   223,
     225,   272,   339,  -415,    65,    15,  -415,  1425,  1425,  1425,
    1425,  1425,  1425,  1425,  1425,  1425,  1425,  1425,  1425,  1425,
    1425,  1425,  1425,  1425,  1425,  1425,  -415,   169,  -415,  -415,
     282,   340,  1425,   125,  -415,   198,  -415,  -415,  -415,  -415,
    -415,   284,   200,  -415,  -415,  -415,  -415,  -415,    94,    94,
     167,   167,   167,   283,   283,   283,   283,   283,   283,   181,
     181,   181,   181,   263,   213,   223,   225,   272,   343,  -415,
    1045,  1425,  1045,   336,  1045,   346,  -415,  -415,    28,  -415,
      32,  1349,  -415,  1425,  -415,  -415,  1425,  1425,  1425,   389,
     351,  1425,   286,  1255,  -415,   283,   283,   283,   283,   283,
     216,   216,   216,   216,   267,   251,   302,   305,   345,   349,
    -415,   347,   338,  -415,   288,   350,  -415,   355,   187,  -415,
    -415,  -415,  -415,   350,  -415,  -415,  1425,   384,   291,  -415,
     415,  -415,  -415,   356,  -415,   365,  -415,  -415,  -415,  -415,
    -415,  -415,  -415,   293,  -415,  1425,   341,  -415,   344,  1045,
    1425,  1425,   350,  -415,   805,  -415,   350,  -415,  -415,   313,
    -415,   350,  -415,  1045,   348,  1425,    50,   415,  -415,    20,
    1045,   315,  -415,  1425,  -415,  -415,   361,  -415,   350,  -415,
     885,  -415,   350,  -415,  -415,  -415,  -415,   -32,   364,  -415,
     415,  -415,   400,  -415,  1045,   366,  1045,  -415,  -415,  -415,
     350,   965,   965,   362,    20,  -415,  1045,  -415,  -415,   965,
     965,  -415,  -415,  -415
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -415,  -415,  -415,    62,  -415,  -415,  -415,  -415,     0,  -415,
    -415,   131,  -415,    33,  -415,    38,  -415,  -415,  -415,   -38,
    -415,   101,  -415,  -415,  -415,    11,    30,  -415,  -110,  -415,
     -45,  -415,    16,  -415,   -33,  -193,  -415,  -172,    81,  -415,
    -168,    82,  -415,  -160,    80,  -415,  -159,    84,  -415,  -153,
      85,  -415,  -415,  -415,  -415,  -415,  -415,  -415,  -144,  -342,
    -415,    -7,    -9,  -415,  -415,   -11,   -18,  -415,  -415,  -415,
     195,    24,  -415,  -415,   192,   352,   -40,  -415,  -415,  -415,
    -415,  -414,  -415,  -415,  -415,  -415,  -415,  -415,  -415,   -80,
    -415,   -56,  -415,  -415,  -415,  -415,  -415,   154,  -415,  -415,
    -415,  -415,  -415,  -415,  -415,  -415,  -415,  -240,  -320,  -415,
     -34,   -82
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const yytype_uint16 yytable[] =
{
      43,   154,   131,   213,   142,   136,   434,   207,   300,   169,
      87,    50,   141,   309,   106,   381,    43,   488,   143,   313,
     209,   449,   450,   246,   250,   152,   287,    50,   214,   474,
     295,   337,   253,   477,   541,    43,   338,   288,   285,    97,
     318,   184,   133,   134,    98,   339,    50,   340,   138,   306,
      90,   208,   232,   307,   341,   147,   148,   233,   528,   243,
     362,   150,   151,   419,   342,   100,   343,   159,   160,   161,
     162,    88,   213,   135,   256,   107,   259,   516,   322,   323,
     420,   301,   286,   368,   430,    43,   484,   427,   210,   369,
      35,   247,   251,   137,   288,   302,    50,    89,   288,   535,
     377,   108,   475,   431,   363,   144,   218,   211,   262,   403,
     248,   252,   115,   289,   404,   215,   476,   296,   384,   254,
     478,   529,   115,   405,   230,   406,   290,   454,   292,   231,
     293,   294,   407,   288,   163,   115,   185,   424,   303,   164,
     103,   165,   408,   500,   409,   324,   325,   326,   515,    43,
     186,   187,   428,   109,    43,   314,   388,   389,   316,   263,
      50,   440,   441,   442,   443,    50,   333,   334,   335,   336,
     193,   194,   517,   264,   265,   288,   521,   139,   373,   425,
     374,   524,   234,   235,   236,   234,   235,   236,   195,   196,
     163,   217,   366,   237,   163,   167,   460,   168,   537,   241,
     163,   242,   539,   271,   272,   257,   140,   258,   145,   327,
     328,   329,   330,   331,   332,   319,   320,   321,   499,   149,
     548,   273,   274,   390,   391,   392,   155,   368,   459,   197,
     198,   368,   375,   380,   188,   189,   115,   453,   115,   349,
     115,   115,   266,   267,   399,   400,   401,   402,   382,   368,
     199,   200,   201,   202,   203,   498,   350,   351,   115,     1,
       2,     3,   275,   276,   115,   115,    91,   288,   115,   288,
      92,   462,   204,   465,   205,    15,   206,   480,   244,   481,
     416,   212,   482,   260,   261,   115,   255,   393,   394,   395,
     396,   397,   398,   385,   386,   387,   281,   352,   353,   277,
     278,   279,   280,   354,   355,   356,   357,   282,    27,   115,
     283,   115,   190,   191,   192,   284,    28,    29,    93,   297,
     298,    33,   502,    34,   291,   299,    94,   310,    36,   268,
     269,   270,   304,   288,   358,   315,   410,   288,   432,   412,
     288,   317,   115,   413,   288,   115,   414,   288,   422,   423,
     456,   457,   464,   457,   489,   288,   493,   457,   115,   504,
     288,   510,   288,   115,   218,   435,   436,   437,   438,   439,
     218,   218,   218,   218,   218,   218,   218,   218,   218,   218,
     218,   523,   457,   534,   288,   344,   359,   115,   361,   115,
     360,   364,   365,   367,   371,   372,   376,   383,   411,   467,
     415,   469,   468,   472,   417,   426,   470,   458,   473,   466,
      43,   485,    43,   486,    43,   490,   288,   497,   483,   503,
     494,    50,   505,    50,   509,    50,   491,   419,   428,   536,
     542,   544,   513,   551,   546,   461,   526,   421,   213,   444,
     446,   445,   378,   379,   218,   447,   512,   448,   115,   433,
     543,   531,     0,   418,   487,   249,     0,     0,     0,     0,
     520,     0,     0,     0,   433,   433,   115,   213,   213,     0,
       0,     0,     0,   115,     0,     0,   511,     0,   514,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    43,
       0,   532,   525,     0,    43,     0,   527,     0,     0,   533,
      50,     0,     0,    43,     0,    50,   218,   549,   550,     0,
      43,     0,   115,     0,    50,     0,     0,     0,     0,     0,
      43,    50,   115,   545,   115,   547,   552,   115,   115,   433,
       0,    50,   115,     0,    43,   553,    43,     0,     0,     0,
       0,    43,    43,     0,     0,    50,    43,    50,     0,    43,
      43,     0,    50,    50,     0,     0,     0,    50,     0,     0,
      50,    50,     0,     0,     0,     0,   110,   115,     1,     2,
       3,     0,     0,     0,     0,    91,     0,     0,     0,    92,
       0,    12,    13,     0,    15,     0,   115,     0,     0,    18,
       0,   433,   115,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   115,     0,     0,    25,
      26,     0,     0,     0,   115,     0,     0,    27,     0,     0,
       0,     0,     0,     0,     0,    28,    29,    93,    31,    32,
      33,     0,    34,     0,     0,    94,     0,    36,     0,     0,
      37,    38,    39,    40,     0,     0,     0,     0,     1,     2,
       3,     4,     0,   111,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,     0,     0,    18,
      19,    20,     0,    21,    22,     0,     0,    23,    24,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    25,
      26,     0,     0,     0,     0,     0,     0,    27,     0,     0,
       0,     0,     0,     0,     0,    28,    29,    30,    31,    32,
      33,     0,    34,     0,     0,    35,   153,    36,     0,     0,
      37,    38,    39,    40,     0,     0,     0,     0,     1,     2,
       3,     4,     0,    41,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,     0,     0,    18,
      19,    20,     0,    21,    22,     0,     0,    23,    24,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    25,
      26,     0,     0,     0,     0,     0,     0,    27,     0,     0,
       0,     0,     0,     0,     0,    28,    29,    30,    31,    32,
      33,     0,    34,     0,     0,    35,   305,    36,     0,     0,
      37,    38,    39,    40,     0,     0,     0,     0,     1,     2,
       3,     4,     0,    41,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,     0,     0,    18,
      19,    20,     0,    21,    22,     0,     0,    23,    24,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    25,
      26,     0,     0,     0,     0,     0,     0,    27,     0,     0,
       0,     0,     0,     0,     0,    28,    29,    30,    31,    32,
      33,     0,    34,     0,     0,    35,   519,    36,     0,     0,
      37,    38,    39,    40,     0,     0,     0,     0,     1,     2,
       3,     4,     0,    41,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,     0,     0,    18,
      19,    20,     0,    21,    22,     0,     0,    23,    24,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    25,
      26,     0,     0,     0,     0,     0,     0,    27,     0,     0,
       0,     0,     0,     0,     0,    28,    29,    30,    31,    32,
      33,     0,    34,     0,     0,    35,   538,    36,     0,     0,
      37,    38,    39,    40,     0,     0,     0,     0,     1,     2,
       3,     4,     0,    41,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,     0,     0,    18,
      19,    20,     0,    21,    22,     0,     0,    23,    24,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    25,
      26,     0,     0,     0,     0,     0,     0,    27,     0,     0,
       0,     0,     0,     0,     0,    28,    29,    30,    31,    32,
      33,     0,    34,     0,     0,    35,     0,    36,     0,     0,
      37,    38,    39,    40,     0,     0,     0,     0,     1,     2,
       3,     4,     0,    41,     5,     6,     7,     8,     9,     0,
      11,    12,    13,    14,    15,    16,    17,     0,     0,    18,
      19,    20,     0,    21,    22,     0,     0,    23,    24,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    25,
      26,     0,     0,     0,     0,     0,     0,    27,     0,     0,
       0,     0,     0,     0,     0,    28,    29,    30,    31,    32,
      33,     0,    34,     0,     0,    35,     0,    36,     0,     0,
      37,    38,    39,    40,     1,     2,     3,   347,     0,     0,
       0,    91,   216,    41,     0,    92,     0,    12,    13,     0,
      15,     0,     0,     0,     0,    18,     0,     0,     0,   260,
     261,     0,     0,     0,   172,   173,   174,   175,   176,   177,
     178,   179,   180,   181,   182,    25,    26,     0,     0,     0,
       0,     0,     0,    27,     0,     0,     0,     0,     0,     0,
       0,    28,    29,    93,    31,    32,    33,     0,    34,     0,
       0,    94,   183,    36,     0,     0,    37,    38,    39,    40,
       1,     2,     3,     0,     0,     0,     0,    91,     0,     0,
       0,    92,     0,    12,    13,     0,    15,     0,     0,     0,
       0,    18,   170,   171,     0,     0,     0,   172,   173,   174,
     175,   176,   177,   178,   179,   180,   181,   182,     0,     0,
       0,    25,    26,     0,     0,     0,     0,     0,     0,    27,
       0,     0,     0,     0,     0,     0,     0,    28,    29,    93,
      31,    32,    33,     0,    34,   183,     0,    94,     0,    36,
     308,     0,    37,    38,    39,    40,     1,     2,     3,     0,
       0,     0,     0,    91,     0,     0,     0,    92,     0,    12,
      13,     0,    15,     0,     0,     0,     0,    18,     0,   260,
     261,     0,     0,     0,   172,   173,   174,   175,   176,   177,
     178,   179,   180,   181,   182,     0,     0,    25,    26,     0,
       0,     0,     0,     0,     0,    27,     0,     0,     0,     0,
       0,     0,     0,    28,    29,    93,    31,    32,    33,     0,
      34,   311,   183,    94,     0,    36,     0,     0,    37,    38,
      39,    40,     1,     2,     3,     0,     0,     0,     0,    91,
       0,     0,     0,    92,     0,    12,    13,     0,    15,     0,
       0,     0,     0,    18,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    25,    26,     0,     0,     0,     0,     0,
       0,    27,     0,     0,     0,     0,     0,     0,     0,    28,
      29,    93,    31,    32,    33,     0,    34,     0,     0,    94,
       0,    36,   479,     0,    37,    38,    39,    40,     1,     2,
       3,     0,     0,     0,     0,    91,     0,     0,     0,    92,
       0,    12,    13,     0,    15,     0,     0,     0,     0,    18,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    25,
      26,     0,     0,     0,     0,     0,     0,    27,     0,     0,
       0,     0,     0,     0,     0,    28,    29,    93,    31,    32,
      33,     0,    34,     0,     0,    94,     0,    36,     0,     0,
      37,    38,    39,    40
};

static const yytype_int16 yycheck[] =
{
       0,    35,    11,    85,    22,    16,   348,    42,     1,    47,
       1,     0,    21,   157,     1,   255,    16,   431,     1,   163,
       1,   363,   364,     1,     1,    34,     1,    16,     1,     1,
       1,   203,     1,     1,    66,    35,   204,    69,    42,     6,
     184,    48,    12,    13,     6,   205,    35,   206,    18,    69,
      67,    86,    62,    73,   207,    25,    26,    67,     8,    97,
      42,    31,    32,    62,   208,    62,   210,    37,    38,    39,
      40,    62,   154,    67,   112,    62,   114,   491,   188,   189,
      79,    74,    86,    62,    69,    85,   428,    22,    69,    68,
      70,    69,    69,    67,    69,    88,    85,    88,    69,   513,
     244,    88,    74,    88,    86,    88,    90,    88,   115,   281,
      88,    88,    11,    88,   282,    88,    88,    88,   262,    88,
      88,    71,    21,   283,    91,   284,   135,   367,   137,    91,
     139,   140,   285,    69,    67,    34,    65,    73,   149,    72,
      62,    74,   286,   463,   288,   190,   191,   192,   490,   149,
      79,    80,    87,    62,   154,   164,   266,   267,   167,    65,
     149,   354,   355,   356,   357,   154,   199,   200,   201,   202,
      22,    23,   492,    79,    80,    69,   496,    67,    69,    73,
      71,   501,    60,    61,    62,    60,    61,    62,    40,    41,
      67,    90,   230,    71,    67,    72,    71,    74,   518,    72,
      67,    74,   522,    22,    23,    72,    67,    74,    62,   193,
     194,   195,   196,   197,   198,   185,   186,   187,   458,    66,
     540,    40,    41,   268,   269,   270,    69,    62,   372,    81,
      82,    62,   241,    68,    75,    76,   135,    68,   137,    23,
     139,   140,    75,    76,   277,   278,   279,   280,   257,    62,
      36,    37,    38,    39,    83,    68,    40,    41,   157,     3,
       4,     5,    81,    82,   163,   164,    10,    69,   167,    69,
      14,    73,    84,    73,    85,    19,    43,   421,    87,   423,
     298,     0,   426,    44,    45,   184,    67,   271,   272,   273,
     274,   275,   276,   263,   264,   265,    83,    81,    82,    36,
      37,    38,    39,    36,    37,    38,    39,    84,    52,   208,
      85,   210,    46,    47,    48,    43,    60,    61,    62,    30,
      31,    65,   466,    67,    21,    87,    70,    69,    72,    46,
      47,    48,    68,    69,    83,    62,    68,    69,   347,    68,
      69,    62,   241,    68,    69,   244,    68,    69,    68,    69,
      68,    69,    68,    69,    68,    69,    68,    69,   257,    68,
      69,    68,    69,   262,   348,   349,   350,   351,   352,   353,
     354,   355,   356,   357,   358,   359,   360,   361,   362,   363,
     364,    68,    69,    68,    69,    62,    84,   286,    43,   288,
      85,    69,    88,    67,    62,    66,    62,    62,    67,   410,
      67,   412,   411,   414,    62,    66,    70,    67,    62,    66,
     410,    22,   412,    62,   414,    66,    69,    62,   427,    35,
      70,   410,     7,   412,    68,   414,    88,    62,    87,    68,
      66,    31,    88,    71,    68,   373,    88,   306,   520,   358,
     360,   359,   247,   251,   428,   361,   486,   362,   347,   348,
     530,   507,    -1,   299,   430,   103,    -1,    -1,    -1,    -1,
     494,    -1,    -1,    -1,   363,   364,   365,   549,   550,    -1,
      -1,    -1,    -1,   372,    -1,    -1,   485,    -1,   489,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   489,
      -1,   509,   503,    -1,   494,    -1,   505,    -1,    -1,   510,
     489,    -1,    -1,   503,    -1,   494,   490,   541,   542,    -1,
     510,    -1,   411,    -1,   503,    -1,    -1,    -1,    -1,    -1,
     520,   510,   421,   534,   423,   536,   544,   426,   427,   428,
      -1,   520,   431,    -1,   534,   546,   536,    -1,    -1,    -1,
      -1,   541,   542,    -1,    -1,   534,   546,   536,    -1,   549,
     550,    -1,   541,   542,    -1,    -1,    -1,   546,    -1,    -1,
     549,   550,    -1,    -1,    -1,    -1,     1,   466,     3,     4,
       5,    -1,    -1,    -1,    -1,    10,    -1,    -1,    -1,    14,
      -1,    16,    17,    -1,    19,    -1,   485,    -1,    -1,    24,
      -1,   490,   491,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   505,    -1,    -1,    44,
      45,    -1,    -1,    -1,   513,    -1,    -1,    52,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    60,    61,    62,    63,    64,
      65,    -1,    67,    -1,    -1,    70,    -1,    72,    -1,    -1,
      75,    76,    77,    78,    -1,    -1,    -1,    -1,     3,     4,
       5,     6,    -1,    88,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    -1,    -1,    24,
      25,    26,    -1,    28,    29,    -1,    -1,    32,    33,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    44,
      45,    -1,    -1,    -1,    -1,    -1,    -1,    52,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    60,    61,    62,    63,    64,
      65,    -1,    67,    -1,    -1,    70,    71,    72,    -1,    -1,
      75,    76,    77,    78,    -1,    -1,    -1,    -1,     3,     4,
       5,     6,    -1,    88,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    -1,    -1,    24,
      25,    26,    -1,    28,    29,    -1,    -1,    32,    33,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    44,
      45,    -1,    -1,    -1,    -1,    -1,    -1,    52,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    60,    61,    62,    63,    64,
      65,    -1,    67,    -1,    -1,    70,    71,    72,    -1,    -1,
      75,    76,    77,    78,    -1,    -1,    -1,    -1,     3,     4,
       5,     6,    -1,    88,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    -1,    -1,    24,
      25,    26,    -1,    28,    29,    -1,    -1,    32,    33,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    44,
      45,    -1,    -1,    -1,    -1,    -1,    -1,    52,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    60,    61,    62,    63,    64,
      65,    -1,    67,    -1,    -1,    70,    71,    72,    -1,    -1,
      75,    76,    77,    78,    -1,    -1,    -1,    -1,     3,     4,
       5,     6,    -1,    88,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    -1,    -1,    24,
      25,    26,    -1,    28,    29,    -1,    -1,    32,    33,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    44,
      45,    -1,    -1,    -1,    -1,    -1,    -1,    52,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    60,    61,    62,    63,    64,
      65,    -1,    67,    -1,    -1,    70,    71,    72,    -1,    -1,
      75,    76,    77,    78,    -1,    -1,    -1,    -1,     3,     4,
       5,     6,    -1,    88,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    -1,    -1,    24,
      25,    26,    -1,    28,    29,    -1,    -1,    32,    33,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    44,
      45,    -1,    -1,    -1,    -1,    -1,    -1,    52,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    60,    61,    62,    63,    64,
      65,    -1,    67,    -1,    -1,    70,    -1,    72,    -1,    -1,
      75,    76,    77,    78,    -1,    -1,    -1,    -1,     3,     4,
       5,     6,    -1,    88,     9,    10,    11,    12,    13,    -1,
      15,    16,    17,    18,    19,    20,    21,    -1,    -1,    24,
      25,    26,    -1,    28,    29,    -1,    -1,    32,    33,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    44,
      45,    -1,    -1,    -1,    -1,    -1,    -1,    52,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    60,    61,    62,    63,    64,
      65,    -1,    67,    -1,    -1,    70,    -1,    72,    -1,    -1,
      75,    76,    77,    78,     3,     4,     5,    22,    -1,    -1,
      -1,    10,    11,    88,    -1,    14,    -1,    16,    17,    -1,
      19,    -1,    -1,    -1,    -1,    24,    -1,    -1,    -1,    44,
      45,    -1,    -1,    -1,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    44,    45,    -1,    -1,    -1,
      -1,    -1,    -1,    52,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    60,    61,    62,    63,    64,    65,    -1,    67,    -1,
      -1,    70,    87,    72,    -1,    -1,    75,    76,    77,    78,
       3,     4,     5,    -1,    -1,    -1,    -1,    10,    -1,    -1,
      -1,    14,    -1,    16,    17,    -1,    19,    -1,    -1,    -1,
      -1,    24,    44,    45,    -1,    -1,    -1,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    -1,    -1,
      -1,    44,    45,    -1,    -1,    -1,    -1,    -1,    -1,    52,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    60,    61,    62,
      63,    64,    65,    -1,    67,    87,    -1,    70,    -1,    72,
      73,    -1,    75,    76,    77,    78,     3,     4,     5,    -1,
      -1,    -1,    -1,    10,    -1,    -1,    -1,    14,    -1,    16,
      17,    -1,    19,    -1,    -1,    -1,    -1,    24,    -1,    44,
      45,    -1,    -1,    -1,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    -1,    -1,    44,    45,    -1,
      -1,    -1,    -1,    -1,    -1,    52,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    60,    61,    62,    63,    64,    65,    -1,
      67,    68,    87,    70,    -1,    72,    -1,    -1,    75,    76,
      77,    78,     3,     4,     5,    -1,    -1,    -1,    -1,    10,
      -1,    -1,    -1,    14,    -1,    16,    17,    -1,    19,    -1,
      -1,    -1,    -1,    24,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    44,    45,    -1,    -1,    -1,    -1,    -1,
      -1,    52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    60,
      61,    62,    63,    64,    65,    -1,    67,    -1,    -1,    70,
      -1,    72,    73,    -1,    75,    76,    77,    78,     3,     4,
       5,    -1,    -1,    -1,    -1,    10,    -1,    -1,    -1,    14,
      -1,    16,    17,    -1,    19,    -1,    -1,    -1,    -1,    24,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    44,
      45,    -1,    -1,    -1,    -1,    -1,    -1,    52,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    60,    61,    62,    63,    64,
      65,    -1,    67,    -1,    -1,    70,    -1,    72,    -1,    -1,
      75,    76,    77,    78
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     3,     4,     5,     6,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    24,    25,
      26,    28,    29,    32,    33,    44,    45,    52,    60,    61,
      62,    63,    64,    65,    67,    70,    72,    75,    76,    77,
      78,    88,    90,    97,    98,   103,   105,   107,   111,   113,
     114,   116,   118,   120,   122,   125,   128,   131,   134,   137,
     140,   143,   146,   149,   153,   154,   155,   156,   161,   166,
     167,   168,   169,   172,   173,   174,   175,   176,   182,   183,
     184,   185,   187,   188,   198,   199,   200,     1,    62,    88,
      67,    10,    14,    62,    70,    96,    97,   102,   104,   191,
      62,   157,   159,    62,   162,   163,     1,    62,    88,    62,
       1,    88,   102,   104,   106,   110,   112,   114,   115,   117,
     119,   121,   123,   126,   129,   132,   135,   138,   141,   144,
     147,   151,   110,   115,   115,    67,   154,    67,   115,    67,
      67,   151,   155,     1,    88,    62,   186,   115,   115,    66,
     115,   115,   151,    71,   199,    69,    99,   100,   101,   115,
     115,   115,   115,    67,    72,    74,   108,    72,    74,   108,
      44,    45,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    87,   150,    65,    79,    80,    75,    76,
      46,    47,    48,    22,    23,    40,    41,    81,    82,    36,
      37,    38,    39,    83,    84,    85,    43,    42,    86,     1,
      69,    88,     0,   200,     1,    88,    11,   110,   121,   124,
     127,   130,   133,   136,   139,   142,   145,   148,   152,   171,
     102,   104,    62,    67,    60,    61,    62,    71,    91,    92,
      95,    72,    74,   108,    87,   164,     1,    69,    88,   164,
       1,    69,    88,     1,    88,    67,   108,    72,    74,   108,
      44,    45,   150,    65,    79,    80,    75,    76,    46,    47,
      48,    22,    23,    40,    41,    81,    82,    36,    37,    38,
      39,    83,    84,    85,    43,    42,    86,     1,    69,    88,
     151,    21,   151,   151,   151,     1,    88,    30,    31,    87,
       1,    74,    88,   154,    68,    71,    69,    73,    73,   147,
      69,    68,   109,   147,   151,    62,   151,    62,   147,   115,
     115,   115,   117,   117,   119,   119,   119,   121,   121,   121,
     121,   121,   121,   123,   123,   123,   123,   126,   129,   132,
     135,   138,   147,   147,    62,   158,   160,    22,   150,    23,
      40,    41,    81,    82,    36,    37,    38,    39,    83,    84,
      85,    43,    42,    86,    69,    88,   108,    67,    62,    68,
     196,    62,    66,    69,    71,   151,    62,   147,   159,   163,
      68,   196,   151,    62,   147,   115,   115,   115,   117,   117,
     119,   119,   119,   121,   121,   121,   121,   121,   121,   123,
     123,   123,   123,   126,   129,   132,   135,   138,   147,   147,
      68,    67,    68,    68,    68,    67,   155,    62,   186,    62,
      79,   100,    68,    69,    73,    73,    66,    22,    87,   165,
      69,    88,   151,   110,   148,   121,   121,   121,   121,   121,
     124,   124,   124,   124,   127,   130,   133,   136,   139,   148,
     148,   151,   170,    68,   196,   192,    68,    69,    67,   147,
      71,    92,    73,   189,    68,    73,    66,   154,   151,   154,
      70,   177,   154,    62,     1,    74,    88,     1,    88,    73,
     147,   147,   147,   151,   148,    22,    62,   160,   170,    68,
      66,    88,   194,    68,    70,   197,   193,    62,    68,   196,
     197,   190,   147,    35,    68,     7,   178,   179,   180,    68,
      68,   151,   165,    88,   154,   148,   170,   197,   195,    71,
     199,   197,    93,    68,   197,   154,    88,   151,     8,    71,
     181,   180,   155,   154,    68,   170,    68,   197,    71,   197,
      94,    66,    66,   178,    31,   154,    68,   154,   197,   199,
     199,    71,   155,   154
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
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
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
    while (YYID (0))
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
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value, Location); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep, yylocationp)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    YYLTYPE const * const yylocationp;
#endif
{
  if (!yyvaluep)
    return;
  YYUSE (yylocationp);
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep, yylocationp)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    YYLTYPE const * const yylocationp;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  YY_LOCATION_PRINT (yyoutput, *yylocationp);
  YYFPRINTF (yyoutput, ": ");
  yy_symbol_value_print (yyoutput, yytype, yyvaluep, yylocationp);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *bottom, yytype_int16 *top)
#else
static void
yy_stack_print (bottom, top)
    yytype_int16 *bottom;
    yytype_int16 *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, YYLTYPE *yylsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yylsp, yyrule)
    YYSTYPE *yyvsp;
    YYLTYPE *yylsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      fprintf (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       , &(yylsp[(yyi + 1) - (yynrhs)])		       );
      fprintf (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, yylsp, Rule); \
} while (YYID (0))

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
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
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
      YYSIZE_T yyn = 0;
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

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
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
      int yychecklim = YYLAST - yyn + 1;
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
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
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
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
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
  YYUSE (yyvaluep);
  YYUSE (yylocationp);

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
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
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
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

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
#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  yytype_int16 yyssa[YYINITDEPTH];
  yytype_int16 *yyss = yyssa;
  yytype_int16 *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  YYSTYPE *yyvsp;

  /* The location stack.  */
  YYLTYPE yylsa[YYINITDEPTH];
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;
  /* The locations where the error started and ended.  */
  YYLTYPE yyerror_range[2];

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N), yylsp -= (N))

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
  YYLTYPE yyloc;

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

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
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;
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
	yytype_int16 *yyss1 = yyss;
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

  /* Do appropriate processing given the current state.  Read a
     look-ahead token if we need one and don't already have one.  */

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

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the look-ahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;
  *++yylsp = yylloc;
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

  /* Default location.  */
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 185 "grammar.y"
    { (yyval.node) = new NullNode(); ;}
    break;

  case 3:
#line 186 "grammar.y"
    { (yyval.node) = new BooleanNode(true); ;}
    break;

  case 4:
#line 187 "grammar.y"
    { (yyval.node) = new BooleanNode(false); ;}
    break;

  case 5:
#line 188 "grammar.y"
    { (yyval.node) = new NumberNode((yyvsp[(1) - (1)].dval)); ;}
    break;

  case 6:
#line 189 "grammar.y"
    { (yyval.node) = new StringNode((yyvsp[(1) - (1)].ustr)); ;}
    break;

  case 7:
#line 190 "grammar.y"
    {
                                            Lexer& l = lexer();
                                            if (!l.scanRegExp())
                                                YYABORT;
                                            (yyval.node) = new RegExpNode(l.pattern(), l.flags());
                                        ;}
    break;

  case 8:
#line 196 "grammar.y"
    {
                                            Lexer& l = lexer();
                                            if (!l.scanRegExp())
                                                YYABORT;
                                            (yyval.node) = new RegExpNode("=" + l.pattern(), l.flags());
                                        ;}
    break;

  case 9:
#line 205 "grammar.y"
    { (yyval.pname) = new PropertyNameNode(*(yyvsp[(1) - (1)].ident)); ;}
    break;

  case 10:
#line 206 "grammar.y"
    { (yyval.pname) = new PropertyNameNode(Identifier(*(yyvsp[(1) - (1)].ustr))); ;}
    break;

  case 11:
#line 207 "grammar.y"
    { (yyval.pname) = new PropertyNameNode(Identifier(UString::from((yyvsp[(1) - (1)].dval)))); ;}
    break;

  case 12:
#line 211 "grammar.y"
    { (yyval.pnode) = new PropertyNode((yyvsp[(1) - (3)].pname), (yyvsp[(3) - (3)].node), PropertyNode::Constant); ;}
    break;

  case 13:
#line 212 "grammar.y"
    {inFuncExpr();}
    break;

  case 14:
#line 212 "grammar.y"
    {
          if (!makeGetterOrSetterPropertyNode((yyval.pnode), *(yyvsp[(1) - (6)].ident), *(yyvsp[(2) - (6)].ident), 0, (yyvsp[(6) - (6)].body)))
            YYABORT;
        ;}
    break;

  case 15:
#line 216 "grammar.y"
    {inFuncExpr();}
    break;

  case 16:
#line 216 "grammar.y"
    {
          if (!makeGetterOrSetterPropertyNode((yyval.pnode), *(yyvsp[(1) - (7)].ident), *(yyvsp[(2) - (7)].ident), (yyvsp[(4) - (7)].param), (yyvsp[(7) - (7)].body)))
            YYABORT;
        ;}
    break;

  case 17:
#line 223 "grammar.y"
    { (yyval.plist) = new PropertyListNode((yyvsp[(1) - (1)].pnode)); ;}
    break;

  case 18:
#line 224 "grammar.y"
    { (yyval.plist) = new PropertyListNode((yyvsp[(3) - (3)].pnode), (yyvsp[(1) - (3)].plist)); ;}
    break;

  case 20:
#line 229 "grammar.y"
    { (yyval.node) = new ObjectLiteralNode(); ;}
    break;

  case 21:
#line 230 "grammar.y"
    { (yyval.node) = new ObjectLiteralNode((yyvsp[(2) - (3)].plist)); ;}
    break;

  case 22:
#line 232 "grammar.y"
    { (yyval.node) = new ObjectLiteralNode((yyvsp[(2) - (4)].plist)); ;}
    break;

  case 23:
#line 236 "grammar.y"
    { (yyval.node) = new ThisNode(); ;}
    break;

  case 26:
#line 239 "grammar.y"
    { (yyval.node) = new VarAccessNode(*(yyvsp[(1) - (1)].ident)); ;}
    break;

  case 27:
#line 240 "grammar.y"
    { (yyval.node) = makeGroupNode((yyvsp[(2) - (3)].node)); ;}
    break;

  case 28:
#line 245 "grammar.y"
    { (yyval.node) = new ArrayNode((yyvsp[(2) - (3)].ival)); ;}
    break;

  case 29:
#line 246 "grammar.y"
    { (yyval.node) = new ArrayNode((yyvsp[(2) - (3)].elm)); ;}
    break;

  case 30:
#line 247 "grammar.y"
    { (yyval.node) = new ArrayNode((yyvsp[(4) - (5)].ival), (yyvsp[(2) - (5)].elm)); ;}
    break;

  case 31:
#line 251 "grammar.y"
    { (yyval.elm) = new ElementNode((yyvsp[(1) - (2)].ival), (yyvsp[(2) - (2)].node)); ;}
    break;

  case 32:
#line 253 "grammar.y"
    { (yyval.elm) = new ElementNode((yyvsp[(1) - (4)].elm), (yyvsp[(3) - (4)].ival), (yyvsp[(4) - (4)].node)); ;}
    break;

  case 33:
#line 257 "grammar.y"
    { (yyval.ival) = 0; ;}
    break;

  case 35:
#line 262 "grammar.y"
    { (yyval.ival) = 1; ;}
    break;

  case 36:
#line 263 "grammar.y"
    { (yyval.ival) = (yyvsp[(1) - (2)].ival) + 1; ;}
    break;

  case 38:
#line 268 "grammar.y"
    { (yyval.node) = (yyvsp[(1) - (1)].funcExpr); ;}
    break;

  case 39:
#line 269 "grammar.y"
    { (yyval.node) = new BracketAccessorNode((yyvsp[(1) - (4)].node), (yyvsp[(3) - (4)].node)); ;}
    break;

  case 40:
#line 270 "grammar.y"
    { (yyval.node) = new DotAccessorNode((yyvsp[(1) - (3)].node), *(yyvsp[(3) - (3)].ident)); ;}
    break;

  case 41:
#line 271 "grammar.y"
    { (yyval.node) = new NewExprNode((yyvsp[(2) - (3)].node), (yyvsp[(3) - (3)].args)); ;}
    break;

  case 43:
#line 276 "grammar.y"
    { (yyval.node) = new BracketAccessorNode((yyvsp[(1) - (4)].node), (yyvsp[(3) - (4)].node)); ;}
    break;

  case 44:
#line 277 "grammar.y"
    { (yyval.node) = new DotAccessorNode((yyvsp[(1) - (3)].node), *(yyvsp[(3) - (3)].ident)); ;}
    break;

  case 45:
#line 278 "grammar.y"
    { (yyval.node) = new NewExprNode((yyvsp[(2) - (3)].node), (yyvsp[(3) - (3)].args)); ;}
    break;

  case 47:
#line 283 "grammar.y"
    { (yyval.node) = new NewExprNode((yyvsp[(2) - (2)].node)); ;}
    break;

  case 49:
#line 288 "grammar.y"
    { (yyval.node) = new NewExprNode((yyvsp[(2) - (2)].node)); ;}
    break;

  case 50:
#line 292 "grammar.y"
    { (yyval.node) = makeFunctionCallNode((yyvsp[(1) - (2)].node), (yyvsp[(2) - (2)].args)); ;}
    break;

  case 51:
#line 293 "grammar.y"
    { (yyval.node) = makeFunctionCallNode((yyvsp[(1) - (2)].node), (yyvsp[(2) - (2)].args)); ;}
    break;

  case 52:
#line 294 "grammar.y"
    { (yyval.node) = new BracketAccessorNode((yyvsp[(1) - (4)].node), (yyvsp[(3) - (4)].node)); ;}
    break;

  case 53:
#line 295 "grammar.y"
    { (yyval.node) = new DotAccessorNode((yyvsp[(1) - (3)].node), *(yyvsp[(3) - (3)].ident)); ;}
    break;

  case 54:
#line 299 "grammar.y"
    { (yyval.node) = makeFunctionCallNode((yyvsp[(1) - (2)].node), (yyvsp[(2) - (2)].args)); ;}
    break;

  case 55:
#line 300 "grammar.y"
    { (yyval.node) = makeFunctionCallNode((yyvsp[(1) - (2)].node), (yyvsp[(2) - (2)].args)); ;}
    break;

  case 56:
#line 301 "grammar.y"
    { (yyval.node) = new BracketAccessorNode((yyvsp[(1) - (4)].node), (yyvsp[(3) - (4)].node)); ;}
    break;

  case 57:
#line 302 "grammar.y"
    { (yyval.node) = new DotAccessorNode((yyvsp[(1) - (3)].node), *(yyvsp[(3) - (3)].ident)); ;}
    break;

  case 58:
#line 306 "grammar.y"
    { (yyval.args) = new ArgumentsNode(); ;}
    break;

  case 59:
#line 307 "grammar.y"
    { (yyval.args) = new ArgumentsNode((yyvsp[(2) - (3)].alist)); ;}
    break;

  case 60:
#line 311 "grammar.y"
    { (yyval.alist) = new ArgumentListNode((yyvsp[(1) - (1)].node)); ;}
    break;

  case 61:
#line 312 "grammar.y"
    { (yyval.alist) = new ArgumentListNode((yyvsp[(1) - (3)].alist), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 67:
#line 327 "grammar.y"
    { (yyval.node) = makePostfixNode((yyvsp[(1) - (2)].node), OpPlusPlus); ;}
    break;

  case 68:
#line 328 "grammar.y"
    { (yyval.node) = makePostfixNode((yyvsp[(1) - (2)].node), OpMinusMinus); ;}
    break;

  case 70:
#line 333 "grammar.y"
    { (yyval.node) = makePostfixNode((yyvsp[(1) - (2)].node), OpPlusPlus); ;}
    break;

  case 71:
#line 334 "grammar.y"
    { (yyval.node) = makePostfixNode((yyvsp[(1) - (2)].node), OpMinusMinus); ;}
    break;

  case 72:
#line 338 "grammar.y"
    { (yyval.node) = makeDeleteNode((yyvsp[(2) - (2)].node)); ;}
    break;

  case 73:
#line 339 "grammar.y"
    { (yyval.node) = new VoidNode((yyvsp[(2) - (2)].node)); ;}
    break;

  case 74:
#line 340 "grammar.y"
    { (yyval.node) = makeTypeOfNode((yyvsp[(2) - (2)].node)); ;}
    break;

  case 75:
#line 341 "grammar.y"
    { (yyval.node) = makePrefixNode((yyvsp[(2) - (2)].node), OpPlusPlus); ;}
    break;

  case 76:
#line 342 "grammar.y"
    { (yyval.node) = makePrefixNode((yyvsp[(2) - (2)].node), OpPlusPlus); ;}
    break;

  case 77:
#line 343 "grammar.y"
    { (yyval.node) = makePrefixNode((yyvsp[(2) - (2)].node), OpMinusMinus); ;}
    break;

  case 78:
#line 344 "grammar.y"
    { (yyval.node) = makePrefixNode((yyvsp[(2) - (2)].node), OpMinusMinus); ;}
    break;

  case 79:
#line 345 "grammar.y"
    { (yyval.node) = makeUnaryPlusNode((yyvsp[(2) - (2)].node)); ;}
    break;

  case 80:
#line 346 "grammar.y"
    { (yyval.node) = makeNegateNode((yyvsp[(2) - (2)].node)); ;}
    break;

  case 81:
#line 347 "grammar.y"
    { (yyval.node) = makeBitwiseNotNode((yyvsp[(2) - (2)].node)); ;}
    break;

  case 82:
#line 348 "grammar.y"
    { (yyval.node) = makeLogicalNotNode((yyvsp[(2) - (2)].node)); ;}
    break;

  case 88:
#line 362 "grammar.y"
    { (yyval.node) = makeMultNode((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), OpMult); ;}
    break;

  case 89:
#line 363 "grammar.y"
    { (yyval.node) = makeMultNode((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), OpDiv); ;}
    break;

  case 90:
#line 364 "grammar.y"
    { (yyval.node) = makeMultNode((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), OpMod); ;}
    break;

  case 92:
#line 370 "grammar.y"
    { (yyval.node) = makeMultNode((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), OpMult); ;}
    break;

  case 93:
#line 372 "grammar.y"
    { (yyval.node) = makeMultNode((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), OpDiv); ;}
    break;

  case 94:
#line 374 "grammar.y"
    { (yyval.node) = makeMultNode((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), OpMod); ;}
    break;

  case 96:
#line 379 "grammar.y"
    { (yyval.node) = makeAddNode((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), OpPlus); ;}
    break;

  case 97:
#line 380 "grammar.y"
    { (yyval.node) = makeAddNode((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), OpMinus); ;}
    break;

  case 99:
#line 386 "grammar.y"
    { (yyval.node) = makeAddNode((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), OpPlus); ;}
    break;

  case 100:
#line 388 "grammar.y"
    { (yyval.node) = makeAddNode((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), OpMinus); ;}
    break;

  case 102:
#line 393 "grammar.y"
    { (yyval.node) = makeShiftNode((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), OpLShift); ;}
    break;

  case 103:
#line 394 "grammar.y"
    { (yyval.node) = makeShiftNode((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), OpRShift); ;}
    break;

  case 104:
#line 395 "grammar.y"
    { (yyval.node) = makeShiftNode((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), OpURShift); ;}
    break;

  case 106:
#line 400 "grammar.y"
    { (yyval.node) = makeShiftNode((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), OpLShift); ;}
    break;

  case 107:
#line 401 "grammar.y"
    { (yyval.node) = makeShiftNode((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), OpRShift); ;}
    break;

  case 108:
#line 402 "grammar.y"
    { (yyval.node) = makeShiftNode((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), OpURShift); ;}
    break;

  case 110:
#line 407 "grammar.y"
    { (yyval.node) = makeRelationalNode((yyvsp[(1) - (3)].node), OpLess, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 111:
#line 408 "grammar.y"
    { (yyval.node) = makeRelationalNode((yyvsp[(1) - (3)].node), OpGreater, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 112:
#line 409 "grammar.y"
    { (yyval.node) = makeRelationalNode((yyvsp[(1) - (3)].node), OpLessEq, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 113:
#line 410 "grammar.y"
    { (yyval.node) = makeRelationalNode((yyvsp[(1) - (3)].node), OpGreaterEq, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 114:
#line 411 "grammar.y"
    { (yyval.node) = makeRelationalNode((yyvsp[(1) - (3)].node), OpInstanceOf, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 115:
#line 412 "grammar.y"
    { (yyval.node) = makeRelationalNode((yyvsp[(1) - (3)].node), OpIn, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 117:
#line 417 "grammar.y"
    { (yyval.node) = makeRelationalNode((yyvsp[(1) - (3)].node), OpLess, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 118:
#line 418 "grammar.y"
    { (yyval.node) = makeRelationalNode((yyvsp[(1) - (3)].node), OpGreater, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 119:
#line 419 "grammar.y"
    { (yyval.node) = makeRelationalNode((yyvsp[(1) - (3)].node), OpLessEq, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 120:
#line 420 "grammar.y"
    { (yyval.node) = makeRelationalNode((yyvsp[(1) - (3)].node), OpGreaterEq, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 121:
#line 422 "grammar.y"
    { (yyval.node) = makeRelationalNode((yyvsp[(1) - (3)].node), OpInstanceOf, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 123:
#line 427 "grammar.y"
    { (yyval.node) = makeRelationalNode((yyvsp[(1) - (3)].node), OpLess, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 124:
#line 428 "grammar.y"
    { (yyval.node) = makeRelationalNode((yyvsp[(1) - (3)].node), OpGreater, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 125:
#line 429 "grammar.y"
    { (yyval.node) = makeRelationalNode((yyvsp[(1) - (3)].node), OpLessEq, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 126:
#line 430 "grammar.y"
    { (yyval.node) = makeRelationalNode((yyvsp[(1) - (3)].node), OpGreaterEq, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 127:
#line 432 "grammar.y"
    { (yyval.node) = makeRelationalNode((yyvsp[(1) - (3)].node), OpInstanceOf, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 128:
#line 433 "grammar.y"
    { (yyval.node) = makeRelationalNode((yyvsp[(1) - (3)].node), OpIn, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 130:
#line 438 "grammar.y"
    { (yyval.node) = makeEqualNode((yyvsp[(1) - (3)].node), OpEqEq, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 131:
#line 439 "grammar.y"
    { (yyval.node) = makeEqualNode((yyvsp[(1) - (3)].node), OpNotEq, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 132:
#line 440 "grammar.y"
    { (yyval.node) = makeEqualNode((yyvsp[(1) - (3)].node), OpStrEq, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 133:
#line 441 "grammar.y"
    { (yyval.node) = makeEqualNode((yyvsp[(1) - (3)].node), OpStrNEq, (yyvsp[(3) - (3)].node));;}
    break;

  case 135:
#line 447 "grammar.y"
    { (yyval.node) = makeEqualNode((yyvsp[(1) - (3)].node), OpEqEq, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 136:
#line 449 "grammar.y"
    { (yyval.node) = makeEqualNode((yyvsp[(1) - (3)].node), OpNotEq, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 137:
#line 451 "grammar.y"
    { (yyval.node) = makeEqualNode((yyvsp[(1) - (3)].node), OpStrEq, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 138:
#line 453 "grammar.y"
    { (yyval.node) = makeEqualNode((yyvsp[(1) - (3)].node), OpStrNEq, (yyvsp[(3) - (3)].node));;}
    break;

  case 140:
#line 459 "grammar.y"
    { (yyval.node) = makeEqualNode((yyvsp[(1) - (3)].node), OpEqEq, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 141:
#line 460 "grammar.y"
    { (yyval.node) = makeEqualNode((yyvsp[(1) - (3)].node), OpNotEq, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 142:
#line 462 "grammar.y"
    { (yyval.node) = makeEqualNode((yyvsp[(1) - (3)].node), OpStrEq, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 143:
#line 464 "grammar.y"
    { (yyval.node) = makeEqualNode((yyvsp[(1) - (3)].node), OpStrNEq, (yyvsp[(3) - (3)].node));;}
    break;

  case 145:
#line 469 "grammar.y"
    { (yyval.node) = makeBitOperNode((yyvsp[(1) - (3)].node), OpBitAnd, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 147:
#line 475 "grammar.y"
    { (yyval.node) = makeBitOperNode((yyvsp[(1) - (3)].node), OpBitAnd, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 149:
#line 480 "grammar.y"
    { (yyval.node) = makeBitOperNode((yyvsp[(1) - (3)].node), OpBitAnd, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 151:
#line 485 "grammar.y"
    { (yyval.node) = makeBitOperNode((yyvsp[(1) - (3)].node), OpBitXOr, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 153:
#line 491 "grammar.y"
    { (yyval.node) = makeBitOperNode((yyvsp[(1) - (3)].node), OpBitXOr, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 155:
#line 497 "grammar.y"
    { (yyval.node) = makeBitOperNode((yyvsp[(1) - (3)].node), OpBitXOr, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 157:
#line 502 "grammar.y"
    { (yyval.node) = makeBitOperNode((yyvsp[(1) - (3)].node), OpBitOr, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 159:
#line 508 "grammar.y"
    { (yyval.node) = makeBitOperNode((yyvsp[(1) - (3)].node), OpBitOr, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 161:
#line 514 "grammar.y"
    { (yyval.node) = makeBitOperNode((yyvsp[(1) - (3)].node), OpBitOr, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 163:
#line 519 "grammar.y"
    { (yyval.node) = makeBinaryLogicalNode((yyvsp[(1) - (3)].node), OpAnd, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 165:
#line 525 "grammar.y"
    { (yyval.node) = makeBinaryLogicalNode((yyvsp[(1) - (3)].node), OpAnd, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 167:
#line 531 "grammar.y"
    { (yyval.node) = makeBinaryLogicalNode((yyvsp[(1) - (3)].node), OpAnd, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 169:
#line 536 "grammar.y"
    { (yyval.node) = makeBinaryLogicalNode((yyvsp[(1) - (3)].node), OpOr, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 171:
#line 542 "grammar.y"
    { (yyval.node) = makeBinaryLogicalNode((yyvsp[(1) - (3)].node), OpOr, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 173:
#line 547 "grammar.y"
    { (yyval.node) = makeBinaryLogicalNode((yyvsp[(1) - (3)].node), OpOr, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 175:
#line 553 "grammar.y"
    { (yyval.node) = makeConditionalNode((yyvsp[(1) - (5)].node), (yyvsp[(3) - (5)].node), (yyvsp[(5) - (5)].node)); ;}
    break;

  case 177:
#line 559 "grammar.y"
    { (yyval.node) = makeConditionalNode((yyvsp[(1) - (5)].node), (yyvsp[(3) - (5)].node), (yyvsp[(5) - (5)].node)); ;}
    break;

  case 179:
#line 565 "grammar.y"
    { (yyval.node) = makeConditionalNode((yyvsp[(1) - (5)].node), (yyvsp[(3) - (5)].node), (yyvsp[(5) - (5)].node)); ;}
    break;

  case 181:
#line 571 "grammar.y"
    { (yyval.node) = makeAssignNode((yyvsp[(1) - (3)].node), (yyvsp[(2) - (3)].op), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 183:
#line 577 "grammar.y"
    { (yyval.node) = makeAssignNode((yyvsp[(1) - (3)].node), (yyvsp[(2) - (3)].op), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 185:
#line 583 "grammar.y"
    { (yyval.node) = makeAssignNode((yyvsp[(1) - (3)].node), (yyvsp[(2) - (3)].op), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 186:
#line 587 "grammar.y"
    { (yyval.op) = OpEqual; ;}
    break;

  case 187:
#line 588 "grammar.y"
    { (yyval.op) = OpPlusEq; ;}
    break;

  case 188:
#line 589 "grammar.y"
    { (yyval.op) = OpMinusEq; ;}
    break;

  case 189:
#line 590 "grammar.y"
    { (yyval.op) = OpMultEq; ;}
    break;

  case 190:
#line 591 "grammar.y"
    { (yyval.op) = OpDivEq; ;}
    break;

  case 191:
#line 592 "grammar.y"
    { (yyval.op) = OpLShift; ;}
    break;

  case 192:
#line 593 "grammar.y"
    { (yyval.op) = OpRShift; ;}
    break;

  case 193:
#line 594 "grammar.y"
    { (yyval.op) = OpURShift; ;}
    break;

  case 194:
#line 595 "grammar.y"
    { (yyval.op) = OpAndEq; ;}
    break;

  case 195:
#line 596 "grammar.y"
    { (yyval.op) = OpXOrEq; ;}
    break;

  case 196:
#line 597 "grammar.y"
    { (yyval.op) = OpOrEq; ;}
    break;

  case 197:
#line 598 "grammar.y"
    { (yyval.op) = OpModEq; ;}
    break;

  case 199:
#line 603 "grammar.y"
    { (yyval.node) = new CommaNode((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 201:
#line 608 "grammar.y"
    { (yyval.node) = new CommaNode((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 203:
#line 613 "grammar.y"
    { (yyval.node) = new CommaNode((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 221:
#line 637 "grammar.y"
    { (yyval.stat) = new BlockNode(0); DBG((yyval.stat), (yylsp[(2) - (2)]), (yylsp[(2) - (2)])); ;}
    break;

  case 222:
#line 638 "grammar.y"
    { (yyval.stat) = new BlockNode((yyvsp[(2) - (3)].srcs)); DBG((yyval.stat), (yylsp[(3) - (3)]), (yylsp[(3) - (3)])); ;}
    break;

  case 223:
#line 642 "grammar.y"
    { (yyval.stat) = new VarStatementNode((yyvsp[(2) - (3)].vlist)); DBG((yyval.stat), (yylsp[(1) - (3)]), (yylsp[(3) - (3)])); ;}
    break;

  case 224:
#line 643 "grammar.y"
    { (yyval.stat) = new VarStatementNode((yyvsp[(2) - (3)].vlist)); DBG((yyval.stat), (yylsp[(1) - (3)]), (yylsp[(2) - (3)])); AUTO_SEMICOLON; ;}
    break;

  case 225:
#line 647 "grammar.y"
    { (yyval.vlist) = new VarDeclListNode((yyvsp[(1) - (1)].decl)); ;}
    break;

  case 226:
#line 649 "grammar.y"
    { (yyval.vlist) = new VarDeclListNode((yyvsp[(1) - (3)].vlist), (yyvsp[(3) - (3)].decl)); ;}
    break;

  case 227:
#line 653 "grammar.y"
    { (yyval.vlist) = new VarDeclListNode((yyvsp[(1) - (1)].decl)); ;}
    break;

  case 228:
#line 655 "grammar.y"
    { (yyval.vlist) = new VarDeclListNode((yyvsp[(1) - (3)].vlist), (yyvsp[(3) - (3)].decl)); ;}
    break;

  case 229:
#line 659 "grammar.y"
    { (yyval.decl) = new VarDeclNode(*(yyvsp[(1) - (1)].ident), 0, VarDeclNode::Variable); ;}
    break;

  case 230:
#line 660 "grammar.y"
    { (yyval.decl) = new VarDeclNode(*(yyvsp[(1) - (2)].ident), (yyvsp[(2) - (2)].init), VarDeclNode::Variable); ;}
    break;

  case 231:
#line 664 "grammar.y"
    { (yyval.decl) = new VarDeclNode(*(yyvsp[(1) - (1)].ident), 0, VarDeclNode::Variable); ;}
    break;

  case 232:
#line 665 "grammar.y"
    { (yyval.decl) = new VarDeclNode(*(yyvsp[(1) - (2)].ident), (yyvsp[(2) - (2)].init), VarDeclNode::Variable); ;}
    break;

  case 233:
#line 669 "grammar.y"
    { (yyval.stat) = new VarStatementNode((yyvsp[(2) - (3)].vlist)); DBG((yyval.stat), (yylsp[(1) - (3)]), (yylsp[(3) - (3)])); ;}
    break;

  case 234:
#line 671 "grammar.y"
    { (yyval.stat) = new VarStatementNode((yyvsp[(2) - (3)].vlist)); DBG((yyval.stat), (yylsp[(1) - (3)]), (yylsp[(2) - (3)])); AUTO_SEMICOLON; ;}
    break;

  case 235:
#line 675 "grammar.y"
    { (yyval.vlist) = new VarDeclListNode((yyvsp[(1) - (1)].decl)); ;}
    break;

  case 236:
#line 677 "grammar.y"
    { (yyval.vlist) = new VarDeclListNode((yyvsp[(1) - (3)].vlist), (yyvsp[(3) - (3)].decl)); ;}
    break;

  case 237:
#line 681 "grammar.y"
    { (yyval.decl) = new VarDeclNode(*(yyvsp[(1) - (1)].ident), 0, VarDeclNode::Constant); ;}
    break;

  case 238:
#line 682 "grammar.y"
    { (yyval.decl) = new VarDeclNode(*(yyvsp[(1) - (2)].ident), (yyvsp[(2) - (2)].init), VarDeclNode::Constant); ;}
    break;

  case 239:
#line 686 "grammar.y"
    { (yyval.init) = new AssignExprNode((yyvsp[(2) - (2)].node)); ;}
    break;

  case 240:
#line 690 "grammar.y"
    { (yyval.init) = new AssignExprNode((yyvsp[(2) - (2)].node)); ;}
    break;

  case 241:
#line 694 "grammar.y"
    { (yyval.stat) = new EmptyStatementNode(); ;}
    break;

  case 242:
#line 698 "grammar.y"
    { (yyval.stat) = new ExprStatementNode((yyvsp[(1) - (2)].node)); DBG((yyval.stat), (yylsp[(1) - (2)]), (yylsp[(2) - (2)])); ;}
    break;

  case 243:
#line 699 "grammar.y"
    { (yyval.stat) = new ExprStatementNode((yyvsp[(1) - (2)].node)); DBG((yyval.stat), (yylsp[(1) - (2)]), (yylsp[(1) - (2)])); AUTO_SEMICOLON; ;}
    break;

  case 244:
#line 704 "grammar.y"
    { (yyval.stat) = makeIfNode((yyvsp[(3) - (5)].node), (yyvsp[(5) - (5)].stat), 0); DBG((yyval.stat), (yylsp[(1) - (5)]), (yylsp[(4) - (5)])); ;}
    break;

  case 245:
#line 706 "grammar.y"
    { (yyval.stat) = makeIfNode((yyvsp[(3) - (7)].node), (yyvsp[(5) - (7)].stat), (yyvsp[(7) - (7)].stat)); DBG((yyval.stat), (yylsp[(1) - (7)]), (yylsp[(4) - (7)])); ;}
    break;

  case 246:
#line 710 "grammar.y"
    { (yyval.stat) = new DoWhileNode((yyvsp[(2) - (7)].stat), (yyvsp[(5) - (7)].node)); DBG((yyval.stat), (yylsp[(1) - (7)]), (yylsp[(3) - (7)]));;}
    break;

  case 247:
#line 711 "grammar.y"
    { (yyval.stat) = new DoWhileNode((yyvsp[(2) - (6)].stat), (yyvsp[(5) - (6)].node)); DBG((yyval.stat), (yylsp[(1) - (6)]), (yylsp[(3) - (6)])); ;}
    break;

  case 248:
#line 712 "grammar.y"
    { (yyval.stat) = new WhileNode((yyvsp[(3) - (5)].node), (yyvsp[(5) - (5)].stat)); DBG((yyval.stat), (yylsp[(1) - (5)]), (yylsp[(4) - (5)])); ;}
    break;

  case 249:
#line 714 "grammar.y"
    { (yyval.stat) = new ForNode((yyvsp[(3) - (9)].node), (yyvsp[(5) - (9)].node), (yyvsp[(7) - (9)].node), (yyvsp[(9) - (9)].stat)); DBG((yyval.stat), (yylsp[(1) - (9)]), (yylsp[(8) - (9)])); ;}
    break;

  case 250:
#line 716 "grammar.y"
    { (yyval.stat) = new ForNode((yyvsp[(4) - (10)].vlist), (yyvsp[(6) - (10)].node), (yyvsp[(8) - (10)].node), (yyvsp[(10) - (10)].stat)); DBG((yyval.stat), (yylsp[(1) - (10)]), (yylsp[(9) - (10)])); ;}
    break;

  case 251:
#line 718 "grammar.y"
    {
                                            Node *n = (yyvsp[(3) - (7)].node)->nodeInsideAllParens();
                                            if (!n->isLocation())
                                                YYABORT;
                                            (yyval.stat) = new ForInNode(n, (yyvsp[(5) - (7)].node), (yyvsp[(7) - (7)].stat));
                                            DBG((yyval.stat), (yylsp[(1) - (7)]), (yylsp[(6) - (7)]));
                                        ;}
    break;

  case 252:
#line 726 "grammar.y"
    { (yyval.stat) = new ForInNode(*(yyvsp[(4) - (8)].ident), 0, (yyvsp[(6) - (8)].node), (yyvsp[(8) - (8)].stat)); DBG((yyval.stat), (yylsp[(1) - (8)]), (yylsp[(7) - (8)])); ;}
    break;

  case 253:
#line 728 "grammar.y"
    { (yyval.stat) = new ForInNode(*(yyvsp[(4) - (9)].ident), (yyvsp[(5) - (9)].init), (yyvsp[(7) - (9)].node), (yyvsp[(9) - (9)].stat)); DBG((yyval.stat), (yylsp[(1) - (9)]), (yylsp[(8) - (9)])); ;}
    break;

  case 254:
#line 732 "grammar.y"
    { (yyval.node) = 0; ;}
    break;

  case 256:
#line 737 "grammar.y"
    { (yyval.node) = 0; ;}
    break;

  case 258:
#line 742 "grammar.y"
    { (yyval.stat) = new ContinueNode(); DBG((yyval.stat), (yylsp[(1) - (2)]), (yylsp[(2) - (2)])); ;}
    break;

  case 259:
#line 743 "grammar.y"
    { (yyval.stat) = new ContinueNode(); DBG((yyval.stat), (yylsp[(1) - (2)]), (yylsp[(1) - (2)])); AUTO_SEMICOLON; ;}
    break;

  case 260:
#line 744 "grammar.y"
    { (yyval.stat) = new ContinueNode(*(yyvsp[(2) - (3)].ident)); DBG((yyval.stat), (yylsp[(1) - (3)]), (yylsp[(3) - (3)])); ;}
    break;

  case 261:
#line 745 "grammar.y"
    { (yyval.stat) = new ContinueNode(*(yyvsp[(2) - (3)].ident)); DBG((yyval.stat), (yylsp[(1) - (3)]), (yylsp[(2) - (3)])); AUTO_SEMICOLON; ;}
    break;

  case 262:
#line 749 "grammar.y"
    { (yyval.stat) = new BreakNode(); DBG((yyval.stat), (yylsp[(1) - (2)]), (yylsp[(2) - (2)])); ;}
    break;

  case 263:
#line 750 "grammar.y"
    { (yyval.stat) = new BreakNode(); DBG((yyval.stat), (yylsp[(1) - (2)]), (yylsp[(1) - (2)])); AUTO_SEMICOLON; ;}
    break;

  case 264:
#line 751 "grammar.y"
    { (yyval.stat) = new BreakNode(*(yyvsp[(2) - (3)].ident)); DBG((yyval.stat), (yylsp[(1) - (3)]), (yylsp[(3) - (3)])); ;}
    break;

  case 265:
#line 752 "grammar.y"
    { (yyval.stat) = new BreakNode(*(yyvsp[(2) - (3)].ident)); DBG((yyval.stat), (yylsp[(1) - (3)]), (yylsp[(2) - (3)])); AUTO_SEMICOLON; ;}
    break;

  case 266:
#line 756 "grammar.y"
    { (yyval.stat) = new ReturnNode(0); DBG((yyval.stat), (yylsp[(1) - (2)]), (yylsp[(2) - (2)])); ;}
    break;

  case 267:
#line 757 "grammar.y"
    { (yyval.stat) = new ReturnNode(0); DBG((yyval.stat), (yylsp[(1) - (2)]), (yylsp[(1) - (2)])); AUTO_SEMICOLON; ;}
    break;

  case 268:
#line 758 "grammar.y"
    { (yyval.stat) = new ReturnNode((yyvsp[(2) - (3)].node)); DBG((yyval.stat), (yylsp[(1) - (3)]), (yylsp[(3) - (3)])); ;}
    break;

  case 269:
#line 759 "grammar.y"
    { (yyval.stat) = new ReturnNode((yyvsp[(2) - (3)].node)); DBG((yyval.stat), (yylsp[(1) - (3)]), (yylsp[(2) - (3)])); AUTO_SEMICOLON; ;}
    break;

  case 270:
#line 763 "grammar.y"
    { (yyval.stat) = new WithNode((yyvsp[(3) - (5)].node), (yyvsp[(5) - (5)].stat)); DBG((yyval.stat), (yylsp[(1) - (5)]), (yylsp[(4) - (5)])); ;}
    break;

  case 271:
#line 767 "grammar.y"
    { (yyval.stat) = new SwitchNode((yyvsp[(3) - (5)].node), (yyvsp[(5) - (5)].cblk)); DBG((yyval.stat), (yylsp[(1) - (5)]), (yylsp[(4) - (5)])); ;}
    break;

  case 272:
#line 771 "grammar.y"
    { (yyval.cblk) = new CaseBlockNode((yyvsp[(2) - (3)].clist), 0, 0); ;}
    break;

  case 273:
#line 773 "grammar.y"
    { (yyval.cblk) = new CaseBlockNode((yyvsp[(2) - (5)].clist), (yyvsp[(3) - (5)].ccl), (yyvsp[(4) - (5)].clist)); ;}
    break;

  case 274:
#line 777 "grammar.y"
    { (yyval.clist) = 0; ;}
    break;

  case 276:
#line 782 "grammar.y"
    { (yyval.clist) = new ClauseListNode((yyvsp[(1) - (1)].ccl)); ;}
    break;

  case 277:
#line 783 "grammar.y"
    { (yyval.clist) = new ClauseListNode((yyvsp[(1) - (2)].clist), (yyvsp[(2) - (2)].ccl)); ;}
    break;

  case 278:
#line 787 "grammar.y"
    { (yyval.ccl) = new CaseClauseNode((yyvsp[(2) - (3)].node)); ;}
    break;

  case 279:
#line 788 "grammar.y"
    { (yyval.ccl) = new CaseClauseNode((yyvsp[(2) - (4)].node), (yyvsp[(4) - (4)].srcs)); ;}
    break;

  case 280:
#line 792 "grammar.y"
    { (yyval.ccl) = new CaseClauseNode(0); ;}
    break;

  case 281:
#line 793 "grammar.y"
    { (yyval.ccl) = new CaseClauseNode(0, (yyvsp[(3) - (3)].srcs)); ;}
    break;

  case 282:
#line 797 "grammar.y"
    { (yyval.stat) = makeLabelNode(*(yyvsp[(1) - (3)].ident), (yyvsp[(3) - (3)].stat)); ;}
    break;

  case 283:
#line 801 "grammar.y"
    { (yyval.stat) = new ThrowNode((yyvsp[(2) - (3)].node)); DBG((yyval.stat), (yylsp[(1) - (3)]), (yylsp[(3) - (3)])); ;}
    break;

  case 284:
#line 802 "grammar.y"
    { (yyval.stat) = new ThrowNode((yyvsp[(2) - (3)].node)); DBG((yyval.stat), (yylsp[(1) - (3)]), (yylsp[(2) - (3)])); AUTO_SEMICOLON; ;}
    break;

  case 285:
#line 806 "grammar.y"
    { (yyval.stat) = new TryNode((yyvsp[(2) - (4)].stat), CommonIdentifiers::shared()->nullIdentifier, 0, (yyvsp[(4) - (4)].stat)); DBG((yyval.stat), (yylsp[(1) - (4)]), (yylsp[(2) - (4)])); ;}
    break;

  case 286:
#line 807 "grammar.y"
    { (yyval.stat) = new TryNode((yyvsp[(2) - (7)].stat), *(yyvsp[(5) - (7)].ident), (yyvsp[(7) - (7)].stat), 0); DBG((yyval.stat), (yylsp[(1) - (7)]), (yylsp[(2) - (7)])); ;}
    break;

  case 287:
#line 809 "grammar.y"
    { (yyval.stat) = new TryNode((yyvsp[(2) - (9)].stat), *(yyvsp[(5) - (9)].ident), (yyvsp[(7) - (9)].stat), (yyvsp[(9) - (9)].stat)); DBG((yyval.stat), (yylsp[(1) - (9)]), (yylsp[(2) - (9)])); ;}
    break;

  case 288:
#line 813 "grammar.y"
    { (yyval.stat) = new EmptyStatementNode(); DBG((yyval.stat), (yylsp[(1) - (2)]), (yylsp[(2) - (2)])); ;}
    break;

  case 289:
#line 814 "grammar.y"
    { (yyval.stat) = new EmptyStatementNode(); DBG((yyval.stat), (yylsp[(1) - (2)]), (yylsp[(1) - (2)])); AUTO_SEMICOLON; ;}
    break;

  case 290:
#line 818 "grammar.y"
    { (yyval.pkgn) = new PackageNameNode(*(yyvsp[(1) - (1)].ident)); ;}
    break;

  case 291:
#line 819 "grammar.y"
    { (yyval.pkgn) = new PackageNameNode((yyvsp[(1) - (3)].pkgn), *(yyvsp[(3) - (3)].ident)); ;}
    break;

  case 292:
#line 823 "grammar.y"
    { (yyval.stat) = makeImportNode((yyvsp[(2) - (5)].pkgn), true, 0);
                                          DBG((yyval.stat), (yylsp[(1) - (5)]), (yylsp[(5) - (5)])); ;}
    break;

  case 293:
#line 825 "grammar.y"
    { (yyval.stat) = makeImportNode((yyvsp[(2) - (5)].pkgn), true, 0);
                                          DBG((yyval.stat), (yylsp[(1) - (5)]), (yylsp[(5) - (5)])); AUTO_SEMICOLON; ;}
    break;

  case 294:
#line 827 "grammar.y"
    { (yyval.stat) = makeImportNode((yyvsp[(2) - (3)].pkgn), false, 0);
                                          DBG((yyval.stat), (yylsp[(1) - (3)]), (yylsp[(3) - (3)])); ;}
    break;

  case 295:
#line 829 "grammar.y"
    { (yyval.stat) = makeImportNode((yyvsp[(2) - (3)].pkgn), false, 0);
                                          DBG((yyval.stat), (yylsp[(1) - (3)]), (yylsp[(3) - (3)])); AUTO_SEMICOLON; ;}
    break;

  case 296:
#line 831 "grammar.y"
    { (yyval.stat) = makeImportNode((yyvsp[(4) - (5)].pkgn), false, *(yyvsp[(2) - (5)].ident));
                                          DBG((yyval.stat), (yylsp[(1) - (5)]), (yylsp[(5) - (5)])); ;}
    break;

  case 297:
#line 833 "grammar.y"
    { (yyval.stat) = makeImportNode((yyvsp[(4) - (5)].pkgn), false, *(yyvsp[(2) - (5)].ident));
                                          DBG((yyval.stat), (yylsp[(1) - (5)]), (yylsp[(5) - (5)])); AUTO_SEMICOLON; ;}
    break;

  case 298:
#line 838 "grammar.y"
    {inFuncDecl();}
    break;

  case 299:
#line 838 "grammar.y"
    { (yyval.func) = new FuncDeclNode(*(yyvsp[(2) - (6)].ident), (yyvsp[(6) - (6)].body)); ;}
    break;

  case 300:
#line 839 "grammar.y"
    {inFuncDecl();}
    break;

  case 301:
#line 840 "grammar.y"
    { (yyval.func) = new FuncDeclNode(*(yyvsp[(2) - (7)].ident), (yyvsp[(4) - (7)].param), (yyvsp[(7) - (7)].body)); ;}
    break;

  case 302:
#line 844 "grammar.y"
    {inFuncExpr();}
    break;

  case 303:
#line 844 "grammar.y"
    {
      (yyval.funcExpr) = new FuncExprNode(CommonIdentifiers::shared()->nullIdentifier, (yyvsp[(5) - (5)].body));
    ;}
    break;

  case 304:
#line 847 "grammar.y"
    {inFuncExpr();}
    break;

  case 305:
#line 847 "grammar.y"
    {
      (yyval.funcExpr) = new FuncExprNode(CommonIdentifiers::shared()->nullIdentifier, (yyvsp[(6) - (6)].body), (yyvsp[(3) - (6)].param));
    ;}
    break;

  case 306:
#line 850 "grammar.y"
    {inFuncExpr();}
    break;

  case 307:
#line 850 "grammar.y"
    { (yyval.funcExpr) = new FuncExprNode(*(yyvsp[(2) - (6)].ident), (yyvsp[(6) - (6)].body)); ;}
    break;

  case 308:
#line 851 "grammar.y"
    {inFuncExpr();}
    break;

  case 309:
#line 851 "grammar.y"
    {
      (yyval.funcExpr) = new FuncExprNode(*(yyvsp[(2) - (7)].ident), (yyvsp[(7) - (7)].body), (yyvsp[(4) - (7)].param));
    ;}
    break;

  case 310:
#line 857 "grammar.y"
    { (yyval.param) = new ParameterNode(*(yyvsp[(1) - (1)].ident)); ;}
    break;

  case 311:
#line 858 "grammar.y"
    { (yyval.param) = new ParameterNode((yyvsp[(1) - (3)].param), *(yyvsp[(3) - (3)].ident)); ;}
    break;

  case 312:
#line 862 "grammar.y"
    { (yyval.body) = new FunctionBodyNode(0); DBG((yyval.body), (yylsp[(1) - (2)]), (yylsp[(2) - (2)])); ;}
    break;

  case 313:
#line 863 "grammar.y"
    { (yyval.body) = new FunctionBodyNode((yyvsp[(2) - (3)].srcs)); DBG((yyval.body), (yylsp[(1) - (3)]), (yylsp[(3) - (3)])); ;}
    break;

  case 314:
#line 867 "grammar.y"
    { parser().didFinishParsing(new ProgramNode(0)); ;}
    break;

  case 315:
#line 868 "grammar.y"
    { parser().didFinishParsing(new ProgramNode((yyvsp[(1) - (1)].srcs))); ;}
    break;

  case 316:
#line 872 "grammar.y"
    { (yyval.srcs) = new SourceElementsNode((yyvsp[(1) - (1)].stat)); ;}
    break;

  case 317:
#line 873 "grammar.y"
    { (yyval.srcs) = new SourceElementsNode((yyvsp[(1) - (2)].srcs), (yyvsp[(2) - (2)].stat)); ;}
    break;

  case 318:
#line 877 "grammar.y"
    { (yyval.stat) = (yyvsp[(1) - (1)].func); ;}
    break;

  case 319:
#line 878 "grammar.y"
    { (yyval.stat) = (yyvsp[(1) - (1)].stat); ;}
    break;


/* Line 1267 of yacc.c.  */
#line 3573 "grammar.tab.c"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
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
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (yymsg);
	  }
	else
	  {
	    yyerror (YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
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
	  yydestruct ("Error: discarding",
		      yytoken, &yylval, &yylloc);
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
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  yyerror_range[0] = yylsp[1-yylen];
  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
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
      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp, yylsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  *++yyvsp = yylval;

  yyerror_range[1] = yylloc;
  /* Using YYLLOC is tempting, but would change the location of
     the look-ahead.  YYLOC is available though.  */
  YYLLOC_DEFAULT (yyloc, (yyerror_range - 1), 2);
  *++yylsp = yyloc;

  /* Shift the error token.  */
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
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp, yylsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}


#line 881 "grammar.y"


/* called by yyparse on error */
int yyerror(const char *)
{
// fprintf(stderr, "ERROR: %s at line %d\n", s, KJS::Lexer::curr()->lineNo());
    return 1;
}

/* may we automatically insert a semicolon ? */
static bool allowAutomaticSemicolon()
{
    return yychar == '}' || yychar == 0 || lexer().prevTerminator();
}

// kate: indent-width 2; replace-tabs on; tab-width 4; space-indent on;
