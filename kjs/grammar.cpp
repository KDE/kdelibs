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
#define YYLAST   1479

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  89
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  104
/* YYNRULES -- Number of rules.  */
#define YYNRULES  311
/* YYNRULES -- Number of states.  */
#define YYNSTATES  546

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
     728,   731,   734,   740,   748,   756,   763,   769,   779,   790,
     798,   807,   817,   818,   820,   821,   823,   826,   829,   833,
     837,   840,   843,   847,   851,   854,   857,   861,   865,   871,
     877,   881,   887,   888,   890,   892,   895,   899,   904,   907,
     911,   915,   919,   923,   928,   936,   946,   949,   952,   954,
     958,   964,   970,   974,   978,   984,   990,   996,  1003,  1008,
    1014,  1020,  1027,  1029,  1033,  1036,  1040,  1041,  1043,  1045,
    1048,  1050
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     190,     0,    -1,     3,    -1,     4,    -1,     5,    -1,    60,
      -1,    61,    -1,    65,    -1,    52,    -1,    62,    -1,    61,
      -1,    60,    -1,    91,    66,   145,    -1,    62,    62,    67,
      68,   189,    -1,    62,    62,    67,   188,    68,   189,    -1,
      92,    -1,    93,    69,    92,    -1,    95,    -1,    70,    71,
      -1,    70,    93,    71,    -1,    70,    93,    69,    71,    -1,
      19,    -1,    90,    -1,    96,    -1,    62,    -1,    67,   149,
      68,    -1,    72,    98,    73,    -1,    72,    97,    73,    -1,
      72,    97,    69,    98,    73,    -1,    98,   145,    -1,    97,
      69,    98,   145,    -1,    -1,    99,    -1,    69,    -1,    99,
      69,    -1,    94,    -1,   187,    -1,   100,    72,   149,    73,
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
     183,    -1,   185,    -1,    70,    71,    -1,    70,   191,    71,
      -1,    11,   155,    88,    -1,    11,   155,     1,    -1,   157,
      -1,   155,    69,   157,    -1,   158,    -1,   156,    69,   158,
      -1,    62,    -1,    62,   162,    -1,    62,    -1,    62,   163,
      -1,    12,   160,    88,    -1,    12,   160,     1,    -1,   161,
      -1,   160,    69,   161,    -1,    62,    -1,    62,   162,    -1,
      87,   145,    -1,    87,   146,    -1,    88,    -1,   151,    88,
      -1,   151,     1,    -1,    18,    67,   149,    68,   152,    -1,
      18,    67,   149,    68,   152,    35,   152,    -1,    20,   152,
      21,    67,   149,    68,    88,    -1,    20,   152,    21,    67,
     149,    68,    -1,    21,    67,   149,    68,   152,    -1,     9,
      67,   169,    88,   168,    88,   168,    68,   152,    -1,     9,
      67,    11,   156,    88,   168,    88,   168,    68,   152,    -1,
       9,    67,   108,    22,   149,    68,   152,    -1,     9,    67,
      11,    62,    22,   149,    68,   152,    -1,     9,    67,    11,
      62,   163,    22,   149,    68,   152,    -1,    -1,   149,    -1,
      -1,   150,    -1,    13,    88,    -1,    13,     1,    -1,    13,
      62,    88,    -1,    13,    62,     1,    -1,     6,    88,    -1,
       6,     1,    -1,     6,    62,    88,    -1,     6,    62,     1,
      -1,    15,    88,    -1,    15,     1,    -1,    15,   149,    88,
      -1,    15,   149,     1,    -1,    26,    67,   149,    68,   152,
      -1,    25,    67,   149,    68,   175,    -1,    70,   176,    71,
      -1,    70,   176,   179,   176,    71,    -1,    -1,   177,    -1,
     178,    -1,   177,   178,    -1,     7,   149,    66,    -1,     7,
     149,    66,   191,    -1,     8,    66,    -1,     8,    66,   191,
      -1,    62,    66,   152,    -1,    28,   149,    88,    -1,    28,
     149,     1,    -1,    29,   153,    31,   153,    -1,    29,   153,
      30,    67,    62,    68,   153,    -1,    29,   153,    30,    67,
      62,    68,   153,    31,   153,    -1,    32,    88,    -1,    32,
       1,    -1,    62,    -1,   184,    74,    62,    -1,    33,   184,
      74,    79,    88,    -1,    33,   184,    74,    79,     1,    -1,
      33,   184,    88,    -1,    33,   184,     1,    -1,    33,    62,
      87,   184,    88,    -1,    33,    62,    87,   184,     1,    -1,
      14,    62,    67,    68,   189,    -1,    14,    62,    67,   188,
      68,   189,    -1,    14,    67,    68,   189,    -1,    14,    67,
     188,    68,   189,    -1,    14,    62,    67,    68,   189,    -1,
      14,    62,    67,   188,    68,   189,    -1,    62,    -1,   188,
      69,    62,    -1,    70,    71,    -1,    70,   191,    71,    -1,
      -1,   191,    -1,   192,    -1,   191,   192,    -1,   186,    -1,
     152,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   185,   185,   186,   187,   188,   189,   190,   196,   205,
     206,   207,   211,   212,   213,   218,   219,   223,   224,   225,
     227,   231,   232,   233,   234,   235,   240,   241,   242,   246,
     247,   252,   253,   257,   258,   262,   263,   264,   265,   266,
     270,   271,   272,   273,   277,   278,   282,   283,   287,   288,
     289,   290,   294,   295,   296,   297,   301,   302,   306,   307,
     311,   312,   316,   317,   321,   322,   323,   327,   328,   329,
     333,   334,   335,   336,   337,   338,   339,   340,   341,   342,
     343,   346,   347,   351,   352,   356,   357,   358,   359,   363,
     364,   366,   368,   373,   374,   375,   379,   380,   382,   387,
     388,   389,   390,   394,   395,   396,   397,   401,   402,   403,
     404,   405,   406,   407,   411,   412,   413,   414,   415,   416,
     421,   422,   423,   424,   425,   426,   428,   432,   433,   434,
     435,   436,   440,   441,   443,   445,   447,   452,   453,   455,
     456,   458,   463,   464,   468,   469,   474,   475,   479,   480,
     484,   485,   490,   491,   496,   497,   501,   502,   507,   508,
     513,   514,   518,   519,   524,   525,   530,   531,   535,   536,
     541,   542,   546,   547,   552,   553,   558,   559,   564,   565,
     570,   571,   576,   577,   582,   583,   584,   585,   586,   587,
     588,   589,   590,   591,   592,   593,   597,   598,   602,   603,
     607,   608,   612,   613,   614,   615,   616,   617,   618,   619,
     620,   621,   622,   623,   624,   625,   626,   627,   628,   632,
     633,   637,   638,   642,   643,   648,   649,   654,   655,   659,
     660,   664,   665,   670,   671,   676,   677,   681,   685,   689,
     693,   694,   698,   700,   705,   706,   707,   708,   710,   712,
     720,   722,   727,   728,   732,   733,   737,   738,   739,   740,
     744,   745,   746,   747,   751,   752,   753,   754,   758,   762,
     766,   767,   772,   773,   777,   778,   782,   783,   787,   788,
     792,   796,   797,   801,   802,   803,   808,   809,   813,   814,
     818,   820,   822,   824,   826,   828,   833,   834,   839,   840,
     842,   843,   848,   849,   853,   854,   858,   859,   863,   864,
     868,   869
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
  "PropertyName", "Property", "PropertyList", "PrimaryExpr",
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
  "FunctionDeclaration", "FunctionExpr", "FormalParameterList",
  "FunctionBody", "Program", "SourceElements", "SourceElement", 0
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
     167,   167,   168,   168,   169,   169,   170,   170,   170,   170,
     171,   171,   171,   171,   172,   172,   172,   172,   173,   174,
     175,   175,   176,   176,   177,   177,   178,   178,   179,   179,
     180,   181,   181,   182,   182,   182,   183,   183,   184,   184,
     185,   185,   185,   185,   185,   185,   186,   186,   187,   187,
     187,   187,   188,   188,   189,   189,   190,   190,   191,   191,
     192,   192
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
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
       2,     2,     5,     7,     7,     6,     5,     9,    10,     7,
       8,     9,     0,     1,     0,     1,     2,     2,     3,     3,
       2,     2,     3,     3,     2,     2,     3,     3,     5,     5,
       3,     5,     0,     1,     1,     2,     3,     4,     2,     3,
       3,     3,     3,     4,     7,     9,     2,     2,     1,     3,
       5,     5,     3,     3,     5,     5,     5,     6,     4,     5,
       5,     6,     1,     3,     2,     3,     0,     1,     1,     2,
       1,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
     306,     2,     3,     4,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    21,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     8,     5,     6,
      24,     0,     0,     7,     0,     0,    31,     0,     0,     0,
       0,   239,    22,    40,    23,    46,    62,    63,    67,    83,
      84,    89,    96,   103,   120,   137,   146,   152,   158,   164,
     170,   176,   182,   200,     0,   311,   202,   203,   204,   205,
     206,   207,   208,   209,   210,   211,   212,   213,   214,   215,
     216,   217,   218,   310,     0,   307,   308,   261,     0,   260,
     254,     0,     0,    24,     0,    35,    17,    44,    47,    36,
     227,     0,   223,   235,     0,   233,   257,     0,   256,     0,
     265,   264,    44,    60,    61,    64,    81,    82,    85,    93,
      99,   107,   127,   142,   148,   154,   160,   166,   172,   178,
     196,     0,    64,    71,    70,     0,     0,     0,    72,     0,
       0,     0,     0,   287,   286,   288,     0,    73,    75,     0,
      74,    76,     0,   219,     0,    33,     0,     0,    32,    77,
      78,    79,    80,     0,     0,     0,    52,     0,     0,    53,
      68,    69,   185,   186,   187,   188,   189,   190,   191,   192,
     195,   193,   194,   184,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   241,
       0,   240,     1,   309,   263,   262,     0,    64,   114,   132,
     144,   150,   156,   162,   168,   174,   180,   198,   255,     0,
      44,    45,     0,     0,    11,    10,     9,    18,     0,    15,
       0,     0,     0,    43,     0,   228,   222,     0,   221,   236,
     232,     0,   231,   259,   258,     0,    48,     0,     0,    49,
      65,    66,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   267,     0,   266,
       0,     0,     0,     0,     0,   282,   281,     0,     0,     0,
     293,     0,   292,   280,    25,   220,    31,    27,    26,    29,
      34,    56,     0,    58,     0,    42,     0,    55,   183,    91,
      90,    92,    97,    98,   104,   105,   106,   126,   125,   123,
     124,   121,   122,   138,   139,   140,   141,   147,   153,   159,
     165,   171,     0,   201,   229,     0,   225,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   252,    39,     0,   302,     0,
       0,     0,     0,     0,    19,     0,    38,   237,   224,   234,
       0,     0,     0,    51,   179,    87,    86,    88,    94,    95,
     100,   101,   102,   113,   112,   110,   111,   108,   109,   128,
     129,   130,   131,   143,   149,   155,   161,   167,     0,   197,
       0,     0,     0,     0,     0,     0,   283,   288,     0,   289,
       0,     0,    57,     0,    41,    54,     0,     0,     0,   230,
       0,   252,     0,    64,   181,   119,   117,   118,   115,   116,
     133,   134,   135,   136,   145,   151,   157,   163,   169,     0,
     199,   253,     0,     0,     0,     0,   298,     0,     0,     0,
      12,    20,    16,    37,   296,     0,    50,     0,   242,     0,
     246,   272,   269,   268,     0,   295,     0,   294,   291,   290,
      28,    30,    59,   177,     0,   238,     0,   229,   226,     0,
       0,     0,   252,   300,     0,   304,     0,   299,   303,     0,
       0,   297,   173,     0,   245,     0,     0,   273,   274,     0,
       0,     0,   230,   252,   249,   175,     0,   301,   305,    13,
       0,   243,   244,     0,     0,   270,   272,   275,   284,   250,
       0,     0,     0,    14,   276,   278,     0,     0,   251,     0,
     247,   277,   279,   271,   285,   248
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    42,   238,   239,   240,    95,    96,    44,   156,   157,
     158,   112,    45,   113,    46,   114,    47,   166,   312,   132,
      48,   116,    49,   117,   118,    51,   119,    52,   120,    53,
     121,    54,   122,   219,    55,   123,   220,    56,   124,   221,
      57,   125,   222,    58,   126,   223,    59,   127,   224,    60,
     128,   225,    61,   129,   226,    62,   130,   227,    63,   348,
     451,   228,    64,    65,    66,    67,   101,   345,   102,   346,
      68,   104,   105,   245,   429,    69,    70,    71,    72,   452,
     229,    73,    74,    75,    76,    77,   472,   506,   507,   508,
     526,    78,    79,    80,    81,   146,    82,    83,    99,   370,
     456,    84,    85,    86
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -379
static const yytype_int16 yypact[] =
{
     941,  -379,  -379,  -379,    11,   -11,   309,     7,    98,    16,
     108,   180,  1401,  1401,   112,  -379,  1021,   142,  1401,   162,
     168,  1401,   169,     8,   184,  1401,  1401,  -379,  -379,  -379,
     -17,  1401,  1401,  -379,  1401,   568,    14,  1401,  1401,  1401,
    1401,  -379,  -379,  -379,  -379,   128,  -379,   131,   615,  -379,
    -379,  -379,    93,   116,   246,   326,   288,   165,   167,   190,
     220,    71,  -379,  -379,    10,  -379,  -379,  -379,  -379,  -379,
    -379,  -379,  -379,  -379,  -379,  -379,  -379,  -379,  -379,  -379,
    -379,  -379,  -379,  -379,   284,   941,  -379,  -379,    27,  -379,
    1097,   309,   -26,  -379,   166,  -379,  -379,   134,  -379,  -379,
     182,    13,  -379,   182,    17,  -379,  -379,    29,  -379,   263,
    -379,  -379,   134,  -379,   164,   641,  -379,  -379,  -379,    96,
     178,   274,   395,   304,   204,   250,   267,   311,    80,  -379,
    -379,    20,   228,  -379,  -379,  1401,   335,  1401,  -379,  1401,
    1401,    21,   248,  -379,  -379,   245,    19,  -379,  -379,  1021,
    -379,  -379,   213,  -379,   701,  -379,    -2,  1173,   293,  -379,
    -379,  -379,  -379,  1249,  1401,   306,  -379,  1401,   310,  -379,
    -379,  -379,  -379,  -379,  -379,  -379,  -379,  -379,  -379,  -379,
    -379,  -379,  -379,  -379,  1401,  1401,  1401,  1401,  1401,  1401,
    1401,  1401,  1401,  1401,  1401,  1401,  1401,  1401,  1401,  1401,
    1401,  1401,  1401,  1401,  1401,  1401,  1401,  1401,  1401,  -379,
    1401,  -379,  -379,  -379,  -379,  -379,   334,   333,   274,   236,
     308,   290,   314,   317,   362,    88,  -379,  -379,   340,   323,
     134,  -379,   346,   -14,  -379,  -379,   353,  -379,   360,  -379,
      84,  1401,   359,  -379,  1401,  -379,  -379,     7,  -379,  -379,
    -379,    98,  -379,  -379,  -379,    35,  -379,  1401,   366,  -379,
    -379,  -379,  1401,  1401,  1401,  1401,  1401,  1401,  1401,  1401,
    1401,  1401,  1401,  1401,  1401,  1401,  1401,  1401,  1401,  1401,
    1401,  1401,  1401,  1401,  1401,  1401,  1401,  -379,  1401,  -379,
     247,   367,   269,   282,   289,  -379,  -379,   372,   169,   389,
    -379,    -3,  -379,  -379,  -379,  -379,    14,  -379,  -379,  -379,
    -379,  -379,   296,  -379,    22,  -379,    94,  -379,  -379,  -379,
    -379,  -379,    96,    96,   178,   178,   178,   274,   274,   274,
     274,   274,   274,   395,   395,   395,   395,   304,   204,   250,
     267,   311,   386,  -379,    65,    -7,  -379,  1401,  1401,  1401,
    1401,  1401,  1401,  1401,  1401,  1401,  1401,  1401,  1401,  1401,
    1401,  1401,  1401,  1401,  1401,  1401,  -379,   100,  -379,   384,
     325,   390,  1401,   200,  -379,   109,  -379,  -379,  -379,  -379,
     384,   332,   201,  -379,  -379,  -379,  -379,  -379,    96,    96,
     178,   178,   178,   274,   274,   274,   274,   274,   274,   395,
     395,   395,   395,   304,   204,   250,   267,   311,   393,  -379,
    1021,  1401,  1021,   394,  1021,   398,  -379,  -379,    28,  -379,
      31,  1325,  -379,  1401,  -379,  -379,  1401,  1401,  1401,   434,
     401,  1401,   355,   641,  -379,   274,   274,   274,   274,   274,
     236,   236,   236,   236,   308,   290,   314,   317,   362,   399,
    -379,   397,   380,   384,   373,   781,  -379,   384,   405,   103,
    -379,  -379,  -379,  -379,  -379,   384,  -379,  1401,   436,   375,
    -379,   462,  -379,  -379,   404,  -379,   411,  -379,  -379,  -379,
    -379,  -379,  -379,  -379,   377,  -379,  1401,   387,  -379,   391,
    1021,  1401,  1401,  -379,   384,  -379,   861,  -379,  -379,   384,
     379,  -379,  -379,  1021,   392,  1401,    39,   462,  -379,   169,
    1021,   381,  -379,  1401,  -379,  -379,   413,  -379,  -379,  -379,
     384,  -379,  -379,   111,   417,  -379,   462,  -379,   454,  -379,
    1021,   418,  1021,  -379,   941,   941,   416,   169,  -379,  1021,
    -379,   941,   941,  -379,  -379,  -379
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -379,  -379,  -379,   119,  -379,  -379,     0,  -379,  -379,   183,
    -379,    34,  -379,    37,  -379,  -379,  -379,   -37,  -379,   196,
    -379,  -379,  -379,    15,    26,  -379,  -155,  -379,   -54,  -379,
     -53,  -379,   -13,    76,  -379,  -161,   135,  -379,  -159,   139,
    -379,  -150,   140,  -379,  -145,   133,  -379,  -139,   137,  -379,
    -379,  -379,  -379,  -379,  -379,  -379,  -138,  -340,  -379,   -21,
      -8,  -379,  -379,   -15,   -18,  -379,  -379,  -379,   257,    77,
    -379,  -379,   255,   406,    25,  -379,  -379,  -379,  -379,  -378,
    -379,  -379,  -379,  -379,  -379,  -379,  -379,   -12,  -379,     1,
    -379,  -379,  -379,  -379,  -379,   214,  -379,  -379,  -379,  -249,
    -373,  -379,   -33,   -80
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const yytype_uint16 yytable[] =
{
      43,   136,   154,   131,   142,   213,   381,   464,   434,   143,
     169,   209,    87,   141,   246,    50,    43,   106,   250,   309,
     300,   287,   295,   449,   450,   313,   152,   184,   214,   475,
     253,    50,   478,   322,   323,    43,   232,   218,   133,   134,
      97,   233,   337,    98,   138,   338,   318,   524,   368,   149,
      50,   147,   148,   489,   369,   339,    90,   150,   151,   419,
     243,   340,   430,   159,   160,   161,   162,   306,   341,   100,
     342,   307,   343,    88,   213,   256,   420,   259,   107,   210,
     493,   431,   247,   155,   497,    43,   251,   427,   485,   288,
     288,   288,   501,   301,   262,   424,   144,   368,   211,    89,
      50,   248,   476,   380,   108,   252,   377,   302,   289,   296,
     525,   388,   389,   207,   516,   215,   477,   254,   454,   479,
     403,   517,   285,   404,   384,   230,   519,   290,   231,   292,
     362,   293,   294,   405,   303,   531,   324,   325,   326,   406,
     327,   328,   329,   330,   331,   332,   407,   533,   408,    43,
     409,   515,   428,   373,    43,   374,   314,   208,   185,   316,
     103,   263,   368,   288,    50,   368,   286,   425,   453,    50,
     109,   499,   186,   187,   363,   264,   265,   534,   288,   135,
     288,   110,   463,     1,     2,     3,   333,   334,   335,   336,
      91,   188,   189,   366,    92,   163,    12,    13,   163,    15,
     164,   163,   165,   167,    18,   168,   241,   115,   242,   137,
     500,   319,   320,   321,   390,   391,   392,   115,   393,   394,
     395,   396,   397,   398,    25,    26,   234,   235,   236,   139,
     115,   163,    27,   375,   460,   140,   257,   237,   258,    35,
      28,    29,    93,    31,    32,    33,   145,    34,   203,   382,
      94,   204,    36,   266,   267,    37,    38,    39,    40,   349,
     234,   235,   236,   206,   399,   400,   401,   402,   111,   244,
     288,   461,   260,   261,   466,   205,   350,   351,   297,   298,
     416,   304,   288,   481,   212,   482,   217,   281,   483,   385,
     386,   387,   190,   191,   192,   218,   435,   436,   437,   438,
     439,   218,   218,   218,   218,   218,   218,   218,   218,   218,
     218,   218,     1,     2,     3,   410,   288,   352,   353,    91,
     268,   269,   270,    92,   199,   200,   201,   202,    15,   502,
     255,   115,   299,   115,   282,   115,   115,   412,   288,   432,
     277,   278,   279,   280,   354,   355,   356,   357,   193,   194,
     413,   288,   283,   115,   284,   347,   291,   414,   288,   115,
     115,    27,   310,   115,   422,   423,   195,   196,   315,    28,
      29,    93,   317,   358,    33,   218,    34,   260,   261,    94,
     115,    36,   172,   173,   174,   175,   176,   177,   178,   179,
     180,   181,   182,   457,   458,   468,   344,   470,   359,   473,
     465,   458,   360,   469,   115,   361,   115,   197,   198,   364,
      43,   365,    43,   367,    43,   371,   213,   271,   272,   484,
     183,   376,   496,   490,   288,    50,   372,    50,   383,    50,
     440,   441,   442,   443,   411,   273,   274,   115,   218,   415,
     115,   494,   458,   504,   288,   510,   288,   520,   458,   530,
     288,   417,   426,   115,   455,    43,   486,   459,   115,   467,
     474,   213,   213,   487,   471,   491,   288,   498,   492,   505,
      50,   503,   509,   419,   428,   514,   275,   276,   511,   513,
     522,   532,   115,   535,   115,   537,   539,   543,   521,   421,
      43,   528,   462,   444,   447,   529,    43,   523,   445,   448,
     446,   541,   542,    43,   378,    50,   379,   488,   527,   249,
      43,    50,   512,   418,   536,   538,     0,   540,    50,   544,
       0,     0,     0,     0,   545,    50,     0,     0,     0,     0,
      43,     0,    43,     0,    43,    43,     0,     0,     0,    43,
       0,    43,    43,   115,   433,    50,     0,    50,     0,    50,
      50,     0,     0,     0,    50,     0,    50,    50,     0,   433,
     433,   115,     0,     0,     0,     0,     0,     0,   115,     0,
       0,     1,     2,     3,     4,     0,     0,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
       0,     0,    18,    19,    20,     0,    21,    22,     0,     0,
      23,    24,     0,     0,     0,     0,     0,   115,     0,     0,
       0,     0,    25,    26,     0,     0,     0,   115,     0,   115,
      27,     0,   115,   115,   433,     0,     0,   115,    28,    29,
      30,    31,    32,    33,     0,    34,     0,     0,    35,   153,
      36,     0,     0,    37,    38,    39,    40,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    41,     0,     0,   170,
     171,     0,     0,   115,   172,   173,   174,   175,   176,   177,
     178,   179,   180,   181,   182,     0,     0,     0,     0,     0,
       0,     0,   115,     0,     0,   260,   261,   433,   115,     0,
     172,   173,   174,   175,   176,   177,   178,   179,   180,   181,
     182,   115,   183,     0,     1,     2,     3,     4,     0,   115,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,     0,     0,    18,    19,    20,   183,    21,
      22,     0,     0,    23,    24,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    25,    26,     0,     0,     0,
       0,     0,     0,    27,     0,     0,     0,     0,     0,     0,
       0,    28,    29,    30,    31,    32,    33,     0,    34,     0,
       0,    35,   305,    36,     0,     0,    37,    38,    39,    40,
       0,     0,     0,     0,     1,     2,     3,     4,     0,    41,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,     0,     0,    18,    19,    20,     0,    21,
      22,     0,     0,    23,    24,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    25,    26,     0,     0,     0,
       0,     0,     0,    27,     0,     0,     0,     0,     0,     0,
       0,    28,    29,    30,    31,    32,    33,     0,    34,     0,
       0,    35,   495,    36,     0,     0,    37,    38,    39,    40,
       0,     0,     0,     0,     1,     2,     3,     4,     0,    41,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,     0,     0,    18,    19,    20,     0,    21,
      22,     0,     0,    23,    24,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    25,    26,     0,     0,     0,
       0,     0,     0,    27,     0,     0,     0,     0,     0,     0,
       0,    28,    29,    30,    31,    32,    33,     0,    34,     0,
       0,    35,   518,    36,     0,     0,    37,    38,    39,    40,
       0,     0,     0,     0,     1,     2,     3,     4,     0,    41,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,     0,     0,    18,    19,    20,     0,    21,
      22,     0,     0,    23,    24,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    25,    26,     0,     0,     0,
       0,     0,     0,    27,     0,     0,     0,     0,     0,     0,
       0,    28,    29,    30,    31,    32,    33,     0,    34,     0,
       0,    35,     0,    36,     0,     0,    37,    38,    39,    40,
       0,     0,     0,     0,     1,     2,     3,     4,     0,    41,
       5,     6,     7,     8,     9,     0,    11,    12,    13,    14,
      15,    16,    17,     0,     0,    18,    19,    20,     0,    21,
      22,     0,     0,    23,    24,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    25,    26,     0,     0,     0,
       0,     0,     0,    27,     0,     0,     0,     0,     0,     0,
       0,    28,    29,    30,    31,    32,    33,     0,    34,     0,
       0,    35,     0,    36,     0,     0,    37,    38,    39,    40,
       1,     2,     3,     0,     0,     0,     0,    91,   216,    41,
       0,    92,     0,    12,    13,     0,    15,     0,     0,     0,
       0,    18,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    25,    26,     0,     0,     0,     0,     0,     0,    27,
       0,     0,     0,     0,     0,     0,     0,    28,    29,    93,
      31,    32,    33,     0,    34,     0,     0,    94,     0,    36,
       0,     0,    37,    38,    39,    40,     1,     2,     3,     0,
       0,     0,     0,    91,     0,     0,     0,    92,     0,    12,
      13,     0,    15,     0,     0,     0,     0,    18,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    25,    26,     0,
       0,     0,     0,     0,     0,    27,     0,     0,     0,     0,
       0,     0,     0,    28,    29,    93,    31,    32,    33,     0,
      34,     0,     0,    94,     0,    36,   308,     0,    37,    38,
      39,    40,     1,     2,     3,     0,     0,     0,     0,    91,
       0,     0,     0,    92,     0,    12,    13,     0,    15,     0,
       0,     0,     0,    18,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    25,    26,     0,     0,     0,     0,     0,
       0,    27,     0,     0,     0,     0,     0,     0,     0,    28,
      29,    93,    31,    32,    33,     0,    34,   311,     0,    94,
       0,    36,     0,     0,    37,    38,    39,    40,     1,     2,
       3,     0,     0,     0,     0,    91,     0,     0,     0,    92,
       0,    12,    13,     0,    15,     0,     0,     0,     0,    18,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    25,
      26,     0,     0,     0,     0,     0,     0,    27,     0,     0,
       0,     0,     0,     0,     0,    28,    29,    93,    31,    32,
      33,     0,    34,     0,     0,    94,     0,    36,   480,     0,
      37,    38,    39,    40,     1,     2,     3,     0,     0,     0,
       0,    91,     0,     0,     0,    92,     0,    12,    13,     0,
      15,     0,     0,     0,     0,    18,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    25,    26,     0,     0,     0,
       0,     0,     0,    27,     0,     0,     0,     0,     0,     0,
       0,    28,    29,    93,    31,    32,    33,     0,    34,     0,
       0,    94,     0,    36,     0,     0,    37,    38,    39,    40
};

static const yytype_int16 yycheck[] =
{
       0,    16,    35,    11,    22,    85,   255,   380,   348,     1,
      47,     1,     1,    21,     1,     0,    16,     1,     1,   157,
       1,     1,     1,   363,   364,   163,    34,    48,     1,     1,
       1,    16,     1,   188,   189,    35,    62,    90,    12,    13,
       6,    67,   203,     6,    18,   204,   184,     8,    62,    66,
      35,    25,    26,   431,    68,   205,    67,    31,    32,    62,
      97,   206,    69,    37,    38,    39,    40,    69,   207,    62,
     208,    73,   210,    62,   154,   112,    79,   114,    62,    69,
     453,    88,    69,    69,   457,    85,    69,    22,   428,    69,
      69,    69,   465,    74,   115,    73,    88,    62,    88,    88,
      85,    88,    74,    68,    88,    88,   244,    88,    88,    88,
      71,   266,   267,    42,   492,    88,    88,    88,   367,    88,
     281,   494,    42,   282,   262,    91,   499,   135,    91,   137,
      42,   139,   140,   283,   149,   513,   190,   191,   192,   284,
     193,   194,   195,   196,   197,   198,   285,   520,   286,   149,
     288,   491,    87,    69,   154,    71,   164,    86,    65,   167,
      62,    65,    62,    69,   149,    62,    86,    73,    68,   154,
      62,    68,    79,    80,    86,    79,    80,    66,    69,    67,
      69,     1,    73,     3,     4,     5,   199,   200,   201,   202,
      10,    75,    76,   230,    14,    67,    16,    17,    67,    19,
      72,    67,    74,    72,    24,    74,    72,    11,    74,    67,
     459,   185,   186,   187,   268,   269,   270,    21,   271,   272,
     273,   274,   275,   276,    44,    45,    60,    61,    62,    67,
      34,    67,    52,   241,   372,    67,    72,    71,    74,    70,
      60,    61,    62,    63,    64,    65,    62,    67,    83,   257,
      70,    84,    72,    75,    76,    75,    76,    77,    78,    23,
      60,    61,    62,    43,   277,   278,   279,   280,    88,    87,
      69,    71,    44,    45,    73,    85,    40,    41,    30,    31,
     298,    68,    69,   421,     0,   423,    90,    83,   426,   263,
     264,   265,    46,    47,    48,   348,   349,   350,   351,   352,
     353,   354,   355,   356,   357,   358,   359,   360,   361,   362,
     363,   364,     3,     4,     5,    68,    69,    81,    82,    10,
      46,    47,    48,    14,    36,    37,    38,    39,    19,   467,
      67,   135,    87,   137,    84,   139,   140,    68,    69,   347,
      36,    37,    38,    39,    36,    37,    38,    39,    22,    23,
      68,    69,    85,   157,    43,    22,    21,    68,    69,   163,
     164,    52,    69,   167,    68,    69,    40,    41,    62,    60,
      61,    62,    62,    83,    65,   428,    67,    44,    45,    70,
     184,    72,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    68,    69,   410,    62,   412,    84,   414,
      68,    69,    85,   411,   208,    43,   210,    81,    82,    69,
     410,    88,   412,    67,   414,    62,   496,    22,    23,   427,
      87,    62,   455,    68,    69,   410,    66,   412,    62,   414,
     354,   355,   356,   357,    67,    40,    41,   241,   491,    67,
     244,    68,    69,    68,    69,    68,    69,    68,    69,    68,
      69,    62,    66,   257,    70,   455,    22,    67,   262,    66,
      62,   541,   542,    62,    70,    66,    69,    62,    88,     7,
     455,    35,    68,    62,    87,   490,    81,    82,   486,    88,
      88,    68,   286,    66,   288,    31,    68,    71,   503,   306,
     490,   509,   373,   358,   361,   510,   496,   505,   359,   362,
     360,   534,   535,   503,   247,   490,   251,   430,   507,   103,
     510,   496,   487,   299,   526,   530,    -1,   532,   503,   537,
      -1,    -1,    -1,    -1,   539,   510,    -1,    -1,    -1,    -1,
     530,    -1,   532,    -1,   534,   535,    -1,    -1,    -1,   539,
      -1,   541,   542,   347,   348,   530,    -1,   532,    -1,   534,
     535,    -1,    -1,    -1,   539,    -1,   541,   542,    -1,   363,
     364,   365,    -1,    -1,    -1,    -1,    -1,    -1,   372,    -1,
      -1,     3,     4,     5,     6,    -1,    -1,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      -1,    -1,    24,    25,    26,    -1,    28,    29,    -1,    -1,
      32,    33,    -1,    -1,    -1,    -1,    -1,   411,    -1,    -1,
      -1,    -1,    44,    45,    -1,    -1,    -1,   421,    -1,   423,
      52,    -1,   426,   427,   428,    -1,    -1,   431,    60,    61,
      62,    63,    64,    65,    -1,    67,    -1,    -1,    70,    71,
      72,    -1,    -1,    75,    76,    77,    78,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    88,    -1,    -1,    44,
      45,    -1,    -1,   467,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   486,    -1,    -1,    44,    45,   491,   492,    -1,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,   505,    87,    -1,     3,     4,     5,     6,    -1,   513,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    -1,    -1,    24,    25,    26,    87,    28,
      29,    -1,    -1,    32,    33,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    44,    45,    -1,    -1,    -1,
      -1,    -1,    -1,    52,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    60,    61,    62,    63,    64,    65,    -1,    67,    -1,
      -1,    70,    71,    72,    -1,    -1,    75,    76,    77,    78,
      -1,    -1,    -1,    -1,     3,     4,     5,     6,    -1,    88,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    -1,    -1,    24,    25,    26,    -1,    28,
      29,    -1,    -1,    32,    33,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    44,    45,    -1,    -1,    -1,
      -1,    -1,    -1,    52,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    60,    61,    62,    63,    64,    65,    -1,    67,    -1,
      -1,    70,    71,    72,    -1,    -1,    75,    76,    77,    78,
      -1,    -1,    -1,    -1,     3,     4,     5,     6,    -1,    88,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    -1,    -1,    24,    25,    26,    -1,    28,
      29,    -1,    -1,    32,    33,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    44,    45,    -1,    -1,    -1,
      -1,    -1,    -1,    52,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    60,    61,    62,    63,    64,    65,    -1,    67,    -1,
      -1,    70,    71,    72,    -1,    -1,    75,    76,    77,    78,
      -1,    -1,    -1,    -1,     3,     4,     5,     6,    -1,    88,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    -1,    -1,    24,    25,    26,    -1,    28,
      29,    -1,    -1,    32,    33,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    44,    45,    -1,    -1,    -1,
      -1,    -1,    -1,    52,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    60,    61,    62,    63,    64,    65,    -1,    67,    -1,
      -1,    70,    -1,    72,    -1,    -1,    75,    76,    77,    78,
      -1,    -1,    -1,    -1,     3,     4,     5,     6,    -1,    88,
       9,    10,    11,    12,    13,    -1,    15,    16,    17,    18,
      19,    20,    21,    -1,    -1,    24,    25,    26,    -1,    28,
      29,    -1,    -1,    32,    33,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    44,    45,    -1,    -1,    -1,
      -1,    -1,    -1,    52,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    60,    61,    62,    63,    64,    65,    -1,    67,    -1,
      -1,    70,    -1,    72,    -1,    -1,    75,    76,    77,    78,
       3,     4,     5,    -1,    -1,    -1,    -1,    10,    11,    88,
      -1,    14,    -1,    16,    17,    -1,    19,    -1,    -1,    -1,
      -1,    24,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    44,    45,    -1,    -1,    -1,    -1,    -1,    -1,    52,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    60,    61,    62,
      63,    64,    65,    -1,    67,    -1,    -1,    70,    -1,    72,
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
      61,    62,    63,    64,    65,    -1,    67,    68,    -1,    70,
      -1,    72,    -1,    -1,    75,    76,    77,    78,     3,     4,
       5,    -1,    -1,    -1,    -1,    10,    -1,    -1,    -1,    14,
      -1,    16,    17,    -1,    19,    -1,    -1,    -1,    -1,    24,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    44,
      45,    -1,    -1,    -1,    -1,    -1,    -1,    52,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    60,    61,    62,    63,    64,
      65,    -1,    67,    -1,    -1,    70,    -1,    72,    73,    -1,
      75,    76,    77,    78,     3,     4,     5,    -1,    -1,    -1,
      -1,    10,    -1,    -1,    -1,    14,    -1,    16,    17,    -1,
      19,    -1,    -1,    -1,    -1,    24,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    44,    45,    -1,    -1,    -1,
      -1,    -1,    -1,    52,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    60,    61,    62,    63,    64,    65,    -1,    67,    -1,
      -1,    70,    -1,    72,    -1,    -1,    75,    76,    77,    78
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     3,     4,     5,     6,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    24,    25,
      26,    28,    29,    32,    33,    44,    45,    52,    60,    61,
      62,    63,    64,    65,    67,    70,    72,    75,    76,    77,
      78,    88,    90,    95,    96,   101,   103,   105,   109,   111,
     112,   114,   116,   118,   120,   123,   126,   129,   132,   135,
     138,   141,   144,   147,   151,   152,   153,   154,   159,   164,
     165,   166,   167,   170,   171,   172,   173,   174,   180,   181,
     182,   183,   185,   186,   190,   191,   192,     1,    62,    88,
      67,    10,    14,    62,    70,    94,    95,   100,   102,   187,
      62,   155,   157,    62,   160,   161,     1,    62,    88,    62,
       1,    88,   100,   102,   104,   108,   110,   112,   113,   115,
     117,   119,   121,   124,   127,   130,   133,   136,   139,   142,
     145,   149,   108,   113,   113,    67,   152,    67,   113,    67,
      67,   149,   153,     1,    88,    62,   184,   113,   113,    66,
     113,   113,   149,    71,   191,    69,    97,    98,    99,   113,
     113,   113,   113,    67,    72,    74,   106,    72,    74,   106,
      44,    45,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    87,   148,    65,    79,    80,    75,    76,
      46,    47,    48,    22,    23,    40,    41,    81,    82,    36,
      37,    38,    39,    83,    84,    85,    43,    42,    86,     1,
      69,    88,     0,   192,     1,    88,    11,   108,   119,   122,
     125,   128,   131,   134,   137,   140,   143,   146,   150,   169,
     100,   102,    62,    67,    60,    61,    62,    71,    91,    92,
      93,    72,    74,   106,    87,   162,     1,    69,    88,   162,
       1,    69,    88,     1,    88,    67,   106,    72,    74,   106,
      44,    45,   148,    65,    79,    80,    75,    76,    46,    47,
      48,    22,    23,    40,    41,    81,    82,    36,    37,    38,
      39,    83,    84,    85,    43,    42,    86,     1,    69,    88,
     149,    21,   149,   149,   149,     1,    88,    30,    31,    87,
       1,    74,    88,   152,    68,    71,    69,    73,    73,   145,
      69,    68,   107,   145,   149,    62,   149,    62,   145,   113,
     113,   113,   115,   115,   117,   117,   117,   119,   119,   119,
     119,   119,   119,   121,   121,   121,   121,   124,   127,   130,
     133,   136,   145,   145,    62,   156,   158,    22,   148,    23,
      40,    41,    81,    82,    36,    37,    38,    39,    83,    84,
      85,    43,    42,    86,    69,    88,   106,    67,    62,    68,
     188,    62,    66,    69,    71,   149,    62,   145,   157,   161,
      68,   188,   149,    62,   145,   113,   113,   113,   115,   115,
     117,   117,   117,   119,   119,   119,   119,   119,   119,   121,
     121,   121,   121,   124,   127,   130,   133,   136,   145,   145,
      68,    67,    68,    68,    68,    67,   153,    62,   184,    62,
      79,    98,    68,    69,    73,    73,    66,    22,    87,   163,
      69,    88,   149,   108,   146,   119,   119,   119,   119,   119,
     122,   122,   122,   122,   125,   128,   131,   134,   137,   146,
     146,   149,   168,    68,   188,    70,   189,    68,    69,    67,
     145,    71,    92,    73,   189,    68,    73,    66,   152,   149,
     152,    70,   175,   152,    62,     1,    74,    88,     1,    88,
      73,   145,   145,   145,   149,   146,    22,    62,   158,   168,
      68,    66,    88,   189,    68,    71,   191,   189,    62,    68,
     188,   189,   145,    35,    68,     7,   176,   177,   178,    68,
      68,   149,   163,    88,   152,   146,   168,   189,    71,   189,
      68,   152,    88,   149,     8,    71,   179,   178,   153,   152,
      68,   168,    68,   189,    66,    66,   176,    31,   152,    68,
     152,   191,   191,    71,   153,   152
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
    { if (!makeGetterOrSetterPropertyNode((yyval.pnode), *(yyvsp[(1) - (5)].ident), *(yyvsp[(2) - (5)].ident), 0, (yyvsp[(5) - (5)].body))) YYABORT; ;}
    break;

  case 14:
#line 214 "grammar.y"
    { if (!makeGetterOrSetterPropertyNode((yyval.pnode), *(yyvsp[(1) - (6)].ident), *(yyvsp[(2) - (6)].ident), (yyvsp[(4) - (6)].param), (yyvsp[(6) - (6)].body))) YYABORT; ;}
    break;

  case 15:
#line 218 "grammar.y"
    { (yyval.plist) = new PropertyListNode((yyvsp[(1) - (1)].pnode)); ;}
    break;

  case 16:
#line 219 "grammar.y"
    { (yyval.plist) = new PropertyListNode((yyvsp[(3) - (3)].pnode), (yyvsp[(1) - (3)].plist)); ;}
    break;

  case 18:
#line 224 "grammar.y"
    { (yyval.node) = new ObjectLiteralNode(); ;}
    break;

  case 19:
#line 225 "grammar.y"
    { (yyval.node) = new ObjectLiteralNode((yyvsp[(2) - (3)].plist)); ;}
    break;

  case 20:
#line 227 "grammar.y"
    { (yyval.node) = new ObjectLiteralNode((yyvsp[(2) - (4)].plist)); ;}
    break;

  case 21:
#line 231 "grammar.y"
    { (yyval.node) = new ThisNode(); ;}
    break;

  case 24:
#line 234 "grammar.y"
    { (yyval.node) = new VarAccessNode(*(yyvsp[(1) - (1)].ident)); ;}
    break;

  case 25:
#line 235 "grammar.y"
    { (yyval.node) = makeGroupNode((yyvsp[(2) - (3)].node)); ;}
    break;

  case 26:
#line 240 "grammar.y"
    { (yyval.node) = new ArrayNode((yyvsp[(2) - (3)].ival)); ;}
    break;

  case 27:
#line 241 "grammar.y"
    { (yyval.node) = new ArrayNode((yyvsp[(2) - (3)].elm)); ;}
    break;

  case 28:
#line 242 "grammar.y"
    { (yyval.node) = new ArrayNode((yyvsp[(4) - (5)].ival), (yyvsp[(2) - (5)].elm)); ;}
    break;

  case 29:
#line 246 "grammar.y"
    { (yyval.elm) = new ElementNode((yyvsp[(1) - (2)].ival), (yyvsp[(2) - (2)].node)); ;}
    break;

  case 30:
#line 248 "grammar.y"
    { (yyval.elm) = new ElementNode((yyvsp[(1) - (4)].elm), (yyvsp[(3) - (4)].ival), (yyvsp[(4) - (4)].node)); ;}
    break;

  case 31:
#line 252 "grammar.y"
    { (yyval.ival) = 0; ;}
    break;

  case 33:
#line 257 "grammar.y"
    { (yyval.ival) = 1; ;}
    break;

  case 34:
#line 258 "grammar.y"
    { (yyval.ival) = (yyvsp[(1) - (2)].ival) + 1; ;}
    break;

  case 36:
#line 263 "grammar.y"
    { (yyval.node) = (yyvsp[(1) - (1)].funcExpr); ;}
    break;

  case 37:
#line 264 "grammar.y"
    { (yyval.node) = new BracketAccessorNode((yyvsp[(1) - (4)].node), (yyvsp[(3) - (4)].node)); ;}
    break;

  case 38:
#line 265 "grammar.y"
    { (yyval.node) = new DotAccessorNode((yyvsp[(1) - (3)].node), *(yyvsp[(3) - (3)].ident)); ;}
    break;

  case 39:
#line 266 "grammar.y"
    { (yyval.node) = new NewExprNode((yyvsp[(2) - (3)].node), (yyvsp[(3) - (3)].args)); ;}
    break;

  case 41:
#line 271 "grammar.y"
    { (yyval.node) = new BracketAccessorNode((yyvsp[(1) - (4)].node), (yyvsp[(3) - (4)].node)); ;}
    break;

  case 42:
#line 272 "grammar.y"
    { (yyval.node) = new DotAccessorNode((yyvsp[(1) - (3)].node), *(yyvsp[(3) - (3)].ident)); ;}
    break;

  case 43:
#line 273 "grammar.y"
    { (yyval.node) = new NewExprNode((yyvsp[(2) - (3)].node), (yyvsp[(3) - (3)].args)); ;}
    break;

  case 45:
#line 278 "grammar.y"
    { (yyval.node) = new NewExprNode((yyvsp[(2) - (2)].node)); ;}
    break;

  case 47:
#line 283 "grammar.y"
    { (yyval.node) = new NewExprNode((yyvsp[(2) - (2)].node)); ;}
    break;

  case 48:
#line 287 "grammar.y"
    { (yyval.node) = makeFunctionCallNode((yyvsp[(1) - (2)].node), (yyvsp[(2) - (2)].args)); ;}
    break;

  case 49:
#line 288 "grammar.y"
    { (yyval.node) = makeFunctionCallNode((yyvsp[(1) - (2)].node), (yyvsp[(2) - (2)].args)); ;}
    break;

  case 50:
#line 289 "grammar.y"
    { (yyval.node) = new BracketAccessorNode((yyvsp[(1) - (4)].node), (yyvsp[(3) - (4)].node)); ;}
    break;

  case 51:
#line 290 "grammar.y"
    { (yyval.node) = new DotAccessorNode((yyvsp[(1) - (3)].node), *(yyvsp[(3) - (3)].ident)); ;}
    break;

  case 52:
#line 294 "grammar.y"
    { (yyval.node) = makeFunctionCallNode((yyvsp[(1) - (2)].node), (yyvsp[(2) - (2)].args)); ;}
    break;

  case 53:
#line 295 "grammar.y"
    { (yyval.node) = makeFunctionCallNode((yyvsp[(1) - (2)].node), (yyvsp[(2) - (2)].args)); ;}
    break;

  case 54:
#line 296 "grammar.y"
    { (yyval.node) = new BracketAccessorNode((yyvsp[(1) - (4)].node), (yyvsp[(3) - (4)].node)); ;}
    break;

  case 55:
#line 297 "grammar.y"
    { (yyval.node) = new DotAccessorNode((yyvsp[(1) - (3)].node), *(yyvsp[(3) - (3)].ident)); ;}
    break;

  case 56:
#line 301 "grammar.y"
    { (yyval.args) = new ArgumentsNode(); ;}
    break;

  case 57:
#line 302 "grammar.y"
    { (yyval.args) = new ArgumentsNode((yyvsp[(2) - (3)].alist)); ;}
    break;

  case 58:
#line 306 "grammar.y"
    { (yyval.alist) = new ArgumentListNode((yyvsp[(1) - (1)].node)); ;}
    break;

  case 59:
#line 307 "grammar.y"
    { (yyval.alist) = new ArgumentListNode((yyvsp[(1) - (3)].alist), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 65:
#line 322 "grammar.y"
    { (yyval.node) = makePostfixNode((yyvsp[(1) - (2)].node), OpPlusPlus); ;}
    break;

  case 66:
#line 323 "grammar.y"
    { (yyval.node) = makePostfixNode((yyvsp[(1) - (2)].node), OpMinusMinus); ;}
    break;

  case 68:
#line 328 "grammar.y"
    { (yyval.node) = makePostfixNode((yyvsp[(1) - (2)].node), OpPlusPlus); ;}
    break;

  case 69:
#line 329 "grammar.y"
    { (yyval.node) = makePostfixNode((yyvsp[(1) - (2)].node), OpMinusMinus); ;}
    break;

  case 70:
#line 333 "grammar.y"
    { (yyval.node) = makeDeleteNode((yyvsp[(2) - (2)].node)); ;}
    break;

  case 71:
#line 334 "grammar.y"
    { (yyval.node) = new VoidNode((yyvsp[(2) - (2)].node)); ;}
    break;

  case 72:
#line 335 "grammar.y"
    { (yyval.node) = makeTypeOfNode((yyvsp[(2) - (2)].node)); ;}
    break;

  case 73:
#line 336 "grammar.y"
    { (yyval.node) = makePrefixNode((yyvsp[(2) - (2)].node), OpPlusPlus); ;}
    break;

  case 74:
#line 337 "grammar.y"
    { (yyval.node) = makePrefixNode((yyvsp[(2) - (2)].node), OpPlusPlus); ;}
    break;

  case 75:
#line 338 "grammar.y"
    { (yyval.node) = makePrefixNode((yyvsp[(2) - (2)].node), OpMinusMinus); ;}
    break;

  case 76:
#line 339 "grammar.y"
    { (yyval.node) = makePrefixNode((yyvsp[(2) - (2)].node), OpMinusMinus); ;}
    break;

  case 77:
#line 340 "grammar.y"
    { (yyval.node) = makeUnaryPlusNode((yyvsp[(2) - (2)].node)); ;}
    break;

  case 78:
#line 341 "grammar.y"
    { (yyval.node) = makeNegateNode((yyvsp[(2) - (2)].node)); ;}
    break;

  case 79:
#line 342 "grammar.y"
    { (yyval.node) = makeBitwiseNotNode((yyvsp[(2) - (2)].node)); ;}
    break;

  case 80:
#line 343 "grammar.y"
    { (yyval.node) = makeLogicalNotNode((yyvsp[(2) - (2)].node)); ;}
    break;

  case 86:
#line 357 "grammar.y"
    { (yyval.node) = makeMultNode((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), OpMult); ;}
    break;

  case 87:
#line 358 "grammar.y"
    { (yyval.node) = makeMultNode((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), OpDiv); ;}
    break;

  case 88:
#line 359 "grammar.y"
    { (yyval.node) = makeMultNode((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), OpMod); ;}
    break;

  case 90:
#line 365 "grammar.y"
    { (yyval.node) = makeMultNode((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), OpMult); ;}
    break;

  case 91:
#line 367 "grammar.y"
    { (yyval.node) = makeMultNode((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), OpDiv); ;}
    break;

  case 92:
#line 369 "grammar.y"
    { (yyval.node) = makeMultNode((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), OpMod); ;}
    break;

  case 94:
#line 374 "grammar.y"
    { (yyval.node) = makeAddNode((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), OpPlus); ;}
    break;

  case 95:
#line 375 "grammar.y"
    { (yyval.node) = makeAddNode((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), OpMinus); ;}
    break;

  case 97:
#line 381 "grammar.y"
    { (yyval.node) = makeAddNode((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), OpPlus); ;}
    break;

  case 98:
#line 383 "grammar.y"
    { (yyval.node) = makeAddNode((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), OpMinus); ;}
    break;

  case 100:
#line 388 "grammar.y"
    { (yyval.node) = makeShiftNode((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), OpLShift); ;}
    break;

  case 101:
#line 389 "grammar.y"
    { (yyval.node) = makeShiftNode((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), OpRShift); ;}
    break;

  case 102:
#line 390 "grammar.y"
    { (yyval.node) = makeShiftNode((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), OpURShift); ;}
    break;

  case 104:
#line 395 "grammar.y"
    { (yyval.node) = makeShiftNode((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), OpLShift); ;}
    break;

  case 105:
#line 396 "grammar.y"
    { (yyval.node) = makeShiftNode((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), OpRShift); ;}
    break;

  case 106:
#line 397 "grammar.y"
    { (yyval.node) = makeShiftNode((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), OpURShift); ;}
    break;

  case 108:
#line 402 "grammar.y"
    { (yyval.node) = makeRelationalNode((yyvsp[(1) - (3)].node), OpLess, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 109:
#line 403 "grammar.y"
    { (yyval.node) = makeRelationalNode((yyvsp[(1) - (3)].node), OpGreater, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 110:
#line 404 "grammar.y"
    { (yyval.node) = makeRelationalNode((yyvsp[(1) - (3)].node), OpLessEq, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 111:
#line 405 "grammar.y"
    { (yyval.node) = makeRelationalNode((yyvsp[(1) - (3)].node), OpGreaterEq, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 112:
#line 406 "grammar.y"
    { (yyval.node) = makeRelationalNode((yyvsp[(1) - (3)].node), OpInstanceOf, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 113:
#line 407 "grammar.y"
    { (yyval.node) = makeRelationalNode((yyvsp[(1) - (3)].node), OpIn, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 115:
#line 412 "grammar.y"
    { (yyval.node) = makeRelationalNode((yyvsp[(1) - (3)].node), OpLess, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 116:
#line 413 "grammar.y"
    { (yyval.node) = makeRelationalNode((yyvsp[(1) - (3)].node), OpGreater, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 117:
#line 414 "grammar.y"
    { (yyval.node) = makeRelationalNode((yyvsp[(1) - (3)].node), OpLessEq, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 118:
#line 415 "grammar.y"
    { (yyval.node) = makeRelationalNode((yyvsp[(1) - (3)].node), OpGreaterEq, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 119:
#line 417 "grammar.y"
    { (yyval.node) = makeRelationalNode((yyvsp[(1) - (3)].node), OpInstanceOf, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 121:
#line 422 "grammar.y"
    { (yyval.node) = makeRelationalNode((yyvsp[(1) - (3)].node), OpLess, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 122:
#line 423 "grammar.y"
    { (yyval.node) = makeRelationalNode((yyvsp[(1) - (3)].node), OpGreater, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 123:
#line 424 "grammar.y"
    { (yyval.node) = makeRelationalNode((yyvsp[(1) - (3)].node), OpLessEq, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 124:
#line 425 "grammar.y"
    { (yyval.node) = makeRelationalNode((yyvsp[(1) - (3)].node), OpGreaterEq, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 125:
#line 427 "grammar.y"
    { (yyval.node) = makeRelationalNode((yyvsp[(1) - (3)].node), OpInstanceOf, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 126:
#line 428 "grammar.y"
    { (yyval.node) = makeRelationalNode((yyvsp[(1) - (3)].node), OpIn, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 128:
#line 433 "grammar.y"
    { (yyval.node) = makeEqualNode((yyvsp[(1) - (3)].node), OpEqEq, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 129:
#line 434 "grammar.y"
    { (yyval.node) = makeEqualNode((yyvsp[(1) - (3)].node), OpNotEq, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 130:
#line 435 "grammar.y"
    { (yyval.node) = makeEqualNode((yyvsp[(1) - (3)].node), OpStrEq, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 131:
#line 436 "grammar.y"
    { (yyval.node) = makeEqualNode((yyvsp[(1) - (3)].node), OpStrNEq, (yyvsp[(3) - (3)].node));;}
    break;

  case 133:
#line 442 "grammar.y"
    { (yyval.node) = makeEqualNode((yyvsp[(1) - (3)].node), OpEqEq, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 134:
#line 444 "grammar.y"
    { (yyval.node) = makeEqualNode((yyvsp[(1) - (3)].node), OpNotEq, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 135:
#line 446 "grammar.y"
    { (yyval.node) = makeEqualNode((yyvsp[(1) - (3)].node), OpStrEq, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 136:
#line 448 "grammar.y"
    { (yyval.node) = makeEqualNode((yyvsp[(1) - (3)].node), OpStrNEq, (yyvsp[(3) - (3)].node));;}
    break;

  case 138:
#line 454 "grammar.y"
    { (yyval.node) = makeEqualNode((yyvsp[(1) - (3)].node), OpEqEq, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 139:
#line 455 "grammar.y"
    { (yyval.node) = makeEqualNode((yyvsp[(1) - (3)].node), OpNotEq, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 140:
#line 457 "grammar.y"
    { (yyval.node) = makeEqualNode((yyvsp[(1) - (3)].node), OpStrEq, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 141:
#line 459 "grammar.y"
    { (yyval.node) = makeEqualNode((yyvsp[(1) - (3)].node), OpStrNEq, (yyvsp[(3) - (3)].node));;}
    break;

  case 143:
#line 464 "grammar.y"
    { (yyval.node) = makeBitOperNode((yyvsp[(1) - (3)].node), OpBitAnd, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 145:
#line 470 "grammar.y"
    { (yyval.node) = makeBitOperNode((yyvsp[(1) - (3)].node), OpBitAnd, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 147:
#line 475 "grammar.y"
    { (yyval.node) = makeBitOperNode((yyvsp[(1) - (3)].node), OpBitAnd, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 149:
#line 480 "grammar.y"
    { (yyval.node) = makeBitOperNode((yyvsp[(1) - (3)].node), OpBitXOr, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 151:
#line 486 "grammar.y"
    { (yyval.node) = makeBitOperNode((yyvsp[(1) - (3)].node), OpBitXOr, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 153:
#line 492 "grammar.y"
    { (yyval.node) = makeBitOperNode((yyvsp[(1) - (3)].node), OpBitXOr, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 155:
#line 497 "grammar.y"
    { (yyval.node) = makeBitOperNode((yyvsp[(1) - (3)].node), OpBitOr, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 157:
#line 503 "grammar.y"
    { (yyval.node) = makeBitOperNode((yyvsp[(1) - (3)].node), OpBitOr, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 159:
#line 509 "grammar.y"
    { (yyval.node) = makeBitOperNode((yyvsp[(1) - (3)].node), OpBitOr, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 161:
#line 514 "grammar.y"
    { (yyval.node) = makeBinaryLogicalNode((yyvsp[(1) - (3)].node), OpAnd, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 163:
#line 520 "grammar.y"
    { (yyval.node) = makeBinaryLogicalNode((yyvsp[(1) - (3)].node), OpAnd, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 165:
#line 526 "grammar.y"
    { (yyval.node) = makeBinaryLogicalNode((yyvsp[(1) - (3)].node), OpAnd, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 167:
#line 531 "grammar.y"
    { (yyval.node) = makeBinaryLogicalNode((yyvsp[(1) - (3)].node), OpOr, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 169:
#line 537 "grammar.y"
    { (yyval.node) = makeBinaryLogicalNode((yyvsp[(1) - (3)].node), OpOr, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 171:
#line 542 "grammar.y"
    { (yyval.node) = makeBinaryLogicalNode((yyvsp[(1) - (3)].node), OpOr, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 173:
#line 548 "grammar.y"
    { (yyval.node) = makeConditionalNode((yyvsp[(1) - (5)].node), (yyvsp[(3) - (5)].node), (yyvsp[(5) - (5)].node)); ;}
    break;

  case 175:
#line 554 "grammar.y"
    { (yyval.node) = makeConditionalNode((yyvsp[(1) - (5)].node), (yyvsp[(3) - (5)].node), (yyvsp[(5) - (5)].node)); ;}
    break;

  case 177:
#line 560 "grammar.y"
    { (yyval.node) = makeConditionalNode((yyvsp[(1) - (5)].node), (yyvsp[(3) - (5)].node), (yyvsp[(5) - (5)].node)); ;}
    break;

  case 179:
#line 566 "grammar.y"
    { (yyval.node) = makeAssignNode((yyvsp[(1) - (3)].node), (yyvsp[(2) - (3)].op), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 181:
#line 572 "grammar.y"
    { (yyval.node) = makeAssignNode((yyvsp[(1) - (3)].node), (yyvsp[(2) - (3)].op), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 183:
#line 578 "grammar.y"
    { (yyval.node) = makeAssignNode((yyvsp[(1) - (3)].node), (yyvsp[(2) - (3)].op), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 184:
#line 582 "grammar.y"
    { (yyval.op) = OpEqual; ;}
    break;

  case 185:
#line 583 "grammar.y"
    { (yyval.op) = OpPlusEq; ;}
    break;

  case 186:
#line 584 "grammar.y"
    { (yyval.op) = OpMinusEq; ;}
    break;

  case 187:
#line 585 "grammar.y"
    { (yyval.op) = OpMultEq; ;}
    break;

  case 188:
#line 586 "grammar.y"
    { (yyval.op) = OpDivEq; ;}
    break;

  case 189:
#line 587 "grammar.y"
    { (yyval.op) = OpLShift; ;}
    break;

  case 190:
#line 588 "grammar.y"
    { (yyval.op) = OpRShift; ;}
    break;

  case 191:
#line 589 "grammar.y"
    { (yyval.op) = OpURShift; ;}
    break;

  case 192:
#line 590 "grammar.y"
    { (yyval.op) = OpAndEq; ;}
    break;

  case 193:
#line 591 "grammar.y"
    { (yyval.op) = OpXOrEq; ;}
    break;

  case 194:
#line 592 "grammar.y"
    { (yyval.op) = OpOrEq; ;}
    break;

  case 195:
#line 593 "grammar.y"
    { (yyval.op) = OpModEq; ;}
    break;

  case 197:
#line 598 "grammar.y"
    { (yyval.node) = new CommaNode((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 199:
#line 603 "grammar.y"
    { (yyval.node) = new CommaNode((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 201:
#line 608 "grammar.y"
    { (yyval.node) = new CommaNode((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 219:
#line 632 "grammar.y"
    { (yyval.stat) = new BlockNode(0); DBG((yyval.stat), (yylsp[(2) - (2)]), (yylsp[(2) - (2)])); ;}
    break;

  case 220:
#line 633 "grammar.y"
    { (yyval.stat) = new BlockNode((yyvsp[(2) - (3)].srcs)); DBG((yyval.stat), (yylsp[(3) - (3)]), (yylsp[(3) - (3)])); ;}
    break;

  case 221:
#line 637 "grammar.y"
    { (yyval.stat) = new VarStatementNode((yyvsp[(2) - (3)].vlist)); DBG((yyval.stat), (yylsp[(1) - (3)]), (yylsp[(3) - (3)])); ;}
    break;

  case 222:
#line 638 "grammar.y"
    { (yyval.stat) = new VarStatementNode((yyvsp[(2) - (3)].vlist)); DBG((yyval.stat), (yylsp[(1) - (3)]), (yylsp[(2) - (3)])); AUTO_SEMICOLON; ;}
    break;

  case 223:
#line 642 "grammar.y"
    { (yyval.vlist) = new VarDeclListNode((yyvsp[(1) - (1)].decl)); ;}
    break;

  case 224:
#line 644 "grammar.y"
    { (yyval.vlist) = new VarDeclListNode((yyvsp[(1) - (3)].vlist), (yyvsp[(3) - (3)].decl)); ;}
    break;

  case 225:
#line 648 "grammar.y"
    { (yyval.vlist) = new VarDeclListNode((yyvsp[(1) - (1)].decl)); ;}
    break;

  case 226:
#line 650 "grammar.y"
    { (yyval.vlist) = new VarDeclListNode((yyvsp[(1) - (3)].vlist), (yyvsp[(3) - (3)].decl)); ;}
    break;

  case 227:
#line 654 "grammar.y"
    { (yyval.decl) = new VarDeclNode(*(yyvsp[(1) - (1)].ident), 0, VarDeclNode::Variable); ;}
    break;

  case 228:
#line 655 "grammar.y"
    { (yyval.decl) = new VarDeclNode(*(yyvsp[(1) - (2)].ident), (yyvsp[(2) - (2)].init), VarDeclNode::Variable); ;}
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
    { (yyval.stat) = new VarStatementNode((yyvsp[(2) - (3)].vlist)); DBG((yyval.stat), (yylsp[(1) - (3)]), (yylsp[(3) - (3)])); ;}
    break;

  case 232:
#line 666 "grammar.y"
    { (yyval.stat) = new VarStatementNode((yyvsp[(2) - (3)].vlist)); DBG((yyval.stat), (yylsp[(1) - (3)]), (yylsp[(2) - (3)])); AUTO_SEMICOLON; ;}
    break;

  case 233:
#line 670 "grammar.y"
    { (yyval.vlist) = new VarDeclListNode((yyvsp[(1) - (1)].decl)); ;}
    break;

  case 234:
#line 672 "grammar.y"
    { (yyval.vlist) = new VarDeclListNode((yyvsp[(1) - (3)].vlist), (yyvsp[(3) - (3)].decl)); ;}
    break;

  case 235:
#line 676 "grammar.y"
    { (yyval.decl) = new VarDeclNode(*(yyvsp[(1) - (1)].ident), 0, VarDeclNode::Constant); ;}
    break;

  case 236:
#line 677 "grammar.y"
    { (yyval.decl) = new VarDeclNode(*(yyvsp[(1) - (2)].ident), (yyvsp[(2) - (2)].init), VarDeclNode::Constant); ;}
    break;

  case 237:
#line 681 "grammar.y"
    { (yyval.init) = new AssignExprNode((yyvsp[(2) - (2)].node)); ;}
    break;

  case 238:
#line 685 "grammar.y"
    { (yyval.init) = new AssignExprNode((yyvsp[(2) - (2)].node)); ;}
    break;

  case 239:
#line 689 "grammar.y"
    { (yyval.stat) = new EmptyStatementNode(); ;}
    break;

  case 240:
#line 693 "grammar.y"
    { (yyval.stat) = new ExprStatementNode((yyvsp[(1) - (2)].node)); DBG((yyval.stat), (yylsp[(1) - (2)]), (yylsp[(2) - (2)])); ;}
    break;

  case 241:
#line 694 "grammar.y"
    { (yyval.stat) = new ExprStatementNode((yyvsp[(1) - (2)].node)); DBG((yyval.stat), (yylsp[(1) - (2)]), (yylsp[(1) - (2)])); AUTO_SEMICOLON; ;}
    break;

  case 242:
#line 699 "grammar.y"
    { (yyval.stat) = makeIfNode((yyvsp[(3) - (5)].node), (yyvsp[(5) - (5)].stat), 0); DBG((yyval.stat), (yylsp[(1) - (5)]), (yylsp[(4) - (5)])); ;}
    break;

  case 243:
#line 701 "grammar.y"
    { (yyval.stat) = makeIfNode((yyvsp[(3) - (7)].node), (yyvsp[(5) - (7)].stat), (yyvsp[(7) - (7)].stat)); DBG((yyval.stat), (yylsp[(1) - (7)]), (yylsp[(4) - (7)])); ;}
    break;

  case 244:
#line 705 "grammar.y"
    { (yyval.stat) = new DoWhileNode((yyvsp[(2) - (7)].stat), (yyvsp[(5) - (7)].node)); DBG((yyval.stat), (yylsp[(1) - (7)]), (yylsp[(3) - (7)]));;}
    break;

  case 245:
#line 706 "grammar.y"
    { (yyval.stat) = new DoWhileNode((yyvsp[(2) - (6)].stat), (yyvsp[(5) - (6)].node)); DBG((yyval.stat), (yylsp[(1) - (6)]), (yylsp[(3) - (6)])); ;}
    break;

  case 246:
#line 707 "grammar.y"
    { (yyval.stat) = new WhileNode((yyvsp[(3) - (5)].node), (yyvsp[(5) - (5)].stat)); DBG((yyval.stat), (yylsp[(1) - (5)]), (yylsp[(4) - (5)])); ;}
    break;

  case 247:
#line 709 "grammar.y"
    { (yyval.stat) = new ForNode((yyvsp[(3) - (9)].node), (yyvsp[(5) - (9)].node), (yyvsp[(7) - (9)].node), (yyvsp[(9) - (9)].stat)); DBG((yyval.stat), (yylsp[(1) - (9)]), (yylsp[(8) - (9)])); ;}
    break;

  case 248:
#line 711 "grammar.y"
    { (yyval.stat) = new ForNode((yyvsp[(4) - (10)].vlist), (yyvsp[(6) - (10)].node), (yyvsp[(8) - (10)].node), (yyvsp[(10) - (10)].stat)); DBG((yyval.stat), (yylsp[(1) - (10)]), (yylsp[(9) - (10)])); ;}
    break;

  case 249:
#line 713 "grammar.y"
    {
                                            Node *n = (yyvsp[(3) - (7)].node)->nodeInsideAllParens();
                                            if (!n->isLocation())
                                                YYABORT;
                                            (yyval.stat) = new ForInNode(n, (yyvsp[(5) - (7)].node), (yyvsp[(7) - (7)].stat));
                                            DBG((yyval.stat), (yylsp[(1) - (7)]), (yylsp[(6) - (7)]));
                                        ;}
    break;

  case 250:
#line 721 "grammar.y"
    { (yyval.stat) = new ForInNode(*(yyvsp[(4) - (8)].ident), 0, (yyvsp[(6) - (8)].node), (yyvsp[(8) - (8)].stat)); DBG((yyval.stat), (yylsp[(1) - (8)]), (yylsp[(7) - (8)])); ;}
    break;

  case 251:
#line 723 "grammar.y"
    { (yyval.stat) = new ForInNode(*(yyvsp[(4) - (9)].ident), (yyvsp[(5) - (9)].init), (yyvsp[(7) - (9)].node), (yyvsp[(9) - (9)].stat)); DBG((yyval.stat), (yylsp[(1) - (9)]), (yylsp[(8) - (9)])); ;}
    break;

  case 252:
#line 727 "grammar.y"
    { (yyval.node) = 0; ;}
    break;

  case 254:
#line 732 "grammar.y"
    { (yyval.node) = 0; ;}
    break;

  case 256:
#line 737 "grammar.y"
    { (yyval.stat) = new ContinueNode(); DBG((yyval.stat), (yylsp[(1) - (2)]), (yylsp[(2) - (2)])); ;}
    break;

  case 257:
#line 738 "grammar.y"
    { (yyval.stat) = new ContinueNode(); DBG((yyval.stat), (yylsp[(1) - (2)]), (yylsp[(1) - (2)])); AUTO_SEMICOLON; ;}
    break;

  case 258:
#line 739 "grammar.y"
    { (yyval.stat) = new ContinueNode(*(yyvsp[(2) - (3)].ident)); DBG((yyval.stat), (yylsp[(1) - (3)]), (yylsp[(3) - (3)])); ;}
    break;

  case 259:
#line 740 "grammar.y"
    { (yyval.stat) = new ContinueNode(*(yyvsp[(2) - (3)].ident)); DBG((yyval.stat), (yylsp[(1) - (3)]), (yylsp[(2) - (3)])); AUTO_SEMICOLON; ;}
    break;

  case 260:
#line 744 "grammar.y"
    { (yyval.stat) = new BreakNode(); DBG((yyval.stat), (yylsp[(1) - (2)]), (yylsp[(2) - (2)])); ;}
    break;

  case 261:
#line 745 "grammar.y"
    { (yyval.stat) = new BreakNode(); DBG((yyval.stat), (yylsp[(1) - (2)]), (yylsp[(1) - (2)])); AUTO_SEMICOLON; ;}
    break;

  case 262:
#line 746 "grammar.y"
    { (yyval.stat) = new BreakNode(*(yyvsp[(2) - (3)].ident)); DBG((yyval.stat), (yylsp[(1) - (3)]), (yylsp[(3) - (3)])); ;}
    break;

  case 263:
#line 747 "grammar.y"
    { (yyval.stat) = new BreakNode(*(yyvsp[(2) - (3)].ident)); DBG((yyval.stat), (yylsp[(1) - (3)]), (yylsp[(2) - (3)])); AUTO_SEMICOLON; ;}
    break;

  case 264:
#line 751 "grammar.y"
    { (yyval.stat) = new ReturnNode(0); DBG((yyval.stat), (yylsp[(1) - (2)]), (yylsp[(2) - (2)])); ;}
    break;

  case 265:
#line 752 "grammar.y"
    { (yyval.stat) = new ReturnNode(0); DBG((yyval.stat), (yylsp[(1) - (2)]), (yylsp[(1) - (2)])); AUTO_SEMICOLON; ;}
    break;

  case 266:
#line 753 "grammar.y"
    { (yyval.stat) = new ReturnNode((yyvsp[(2) - (3)].node)); DBG((yyval.stat), (yylsp[(1) - (3)]), (yylsp[(3) - (3)])); ;}
    break;

  case 267:
#line 754 "grammar.y"
    { (yyval.stat) = new ReturnNode((yyvsp[(2) - (3)].node)); DBG((yyval.stat), (yylsp[(1) - (3)]), (yylsp[(2) - (3)])); AUTO_SEMICOLON; ;}
    break;

  case 268:
#line 758 "grammar.y"
    { (yyval.stat) = new WithNode((yyvsp[(3) - (5)].node), (yyvsp[(5) - (5)].stat)); DBG((yyval.stat), (yylsp[(1) - (5)]), (yylsp[(4) - (5)])); ;}
    break;

  case 269:
#line 762 "grammar.y"
    { (yyval.stat) = new SwitchNode((yyvsp[(3) - (5)].node), (yyvsp[(5) - (5)].cblk)); DBG((yyval.stat), (yylsp[(1) - (5)]), (yylsp[(4) - (5)])); ;}
    break;

  case 270:
#line 766 "grammar.y"
    { (yyval.cblk) = new CaseBlockNode((yyvsp[(2) - (3)].clist), 0, 0); ;}
    break;

  case 271:
#line 768 "grammar.y"
    { (yyval.cblk) = new CaseBlockNode((yyvsp[(2) - (5)].clist), (yyvsp[(3) - (5)].ccl), (yyvsp[(4) - (5)].clist)); ;}
    break;

  case 272:
#line 772 "grammar.y"
    { (yyval.clist) = 0; ;}
    break;

  case 274:
#line 777 "grammar.y"
    { (yyval.clist) = new ClauseListNode((yyvsp[(1) - (1)].ccl)); ;}
    break;

  case 275:
#line 778 "grammar.y"
    { (yyval.clist) = new ClauseListNode((yyvsp[(1) - (2)].clist), (yyvsp[(2) - (2)].ccl)); ;}
    break;

  case 276:
#line 782 "grammar.y"
    { (yyval.ccl) = new CaseClauseNode((yyvsp[(2) - (3)].node)); ;}
    break;

  case 277:
#line 783 "grammar.y"
    { (yyval.ccl) = new CaseClauseNode((yyvsp[(2) - (4)].node), (yyvsp[(4) - (4)].srcs)); ;}
    break;

  case 278:
#line 787 "grammar.y"
    { (yyval.ccl) = new CaseClauseNode(0); ;}
    break;

  case 279:
#line 788 "grammar.y"
    { (yyval.ccl) = new CaseClauseNode(0, (yyvsp[(3) - (3)].srcs)); ;}
    break;

  case 280:
#line 792 "grammar.y"
    { (yyval.stat) = makeLabelNode(*(yyvsp[(1) - (3)].ident), (yyvsp[(3) - (3)].stat)); ;}
    break;

  case 281:
#line 796 "grammar.y"
    { (yyval.stat) = new ThrowNode((yyvsp[(2) - (3)].node)); DBG((yyval.stat), (yylsp[(1) - (3)]), (yylsp[(3) - (3)])); ;}
    break;

  case 282:
#line 797 "grammar.y"
    { (yyval.stat) = new ThrowNode((yyvsp[(2) - (3)].node)); DBG((yyval.stat), (yylsp[(1) - (3)]), (yylsp[(2) - (3)])); AUTO_SEMICOLON; ;}
    break;

  case 283:
#line 801 "grammar.y"
    { (yyval.stat) = new TryNode((yyvsp[(2) - (4)].stat), CommonIdentifiers::shared()->nullIdentifier, 0, (yyvsp[(4) - (4)].stat)); DBG((yyval.stat), (yylsp[(1) - (4)]), (yylsp[(2) - (4)])); ;}
    break;

  case 284:
#line 802 "grammar.y"
    { (yyval.stat) = new TryNode((yyvsp[(2) - (7)].stat), *(yyvsp[(5) - (7)].ident), (yyvsp[(7) - (7)].stat), 0); DBG((yyval.stat), (yylsp[(1) - (7)]), (yylsp[(2) - (7)])); ;}
    break;

  case 285:
#line 804 "grammar.y"
    { (yyval.stat) = new TryNode((yyvsp[(2) - (9)].stat), *(yyvsp[(5) - (9)].ident), (yyvsp[(7) - (9)].stat), (yyvsp[(9) - (9)].stat)); DBG((yyval.stat), (yylsp[(1) - (9)]), (yylsp[(2) - (9)])); ;}
    break;

  case 286:
#line 808 "grammar.y"
    { (yyval.stat) = new EmptyStatementNode(); DBG((yyval.stat), (yylsp[(1) - (2)]), (yylsp[(2) - (2)])); ;}
    break;

  case 287:
#line 809 "grammar.y"
    { (yyval.stat) = new EmptyStatementNode(); DBG((yyval.stat), (yylsp[(1) - (2)]), (yylsp[(1) - (2)])); AUTO_SEMICOLON; ;}
    break;

  case 288:
#line 813 "grammar.y"
    { (yyval.pkgn) = new PackageNameNode(*(yyvsp[(1) - (1)].ident)); ;}
    break;

  case 289:
#line 814 "grammar.y"
    { (yyval.pkgn) = new PackageNameNode((yyvsp[(1) - (3)].pkgn), *(yyvsp[(3) - (3)].ident)); ;}
    break;

  case 290:
#line 818 "grammar.y"
    { (yyval.stat) = makeImportNode((yyvsp[(2) - (5)].pkgn), true, 0);
                                          DBG((yyval.stat), (yylsp[(1) - (5)]), (yylsp[(5) - (5)])); ;}
    break;

  case 291:
#line 820 "grammar.y"
    { (yyval.stat) = makeImportNode((yyvsp[(2) - (5)].pkgn), true, 0);
                                          DBG((yyval.stat), (yylsp[(1) - (5)]), (yylsp[(5) - (5)])); AUTO_SEMICOLON; ;}
    break;

  case 292:
#line 822 "grammar.y"
    { (yyval.stat) = makeImportNode((yyvsp[(2) - (3)].pkgn), false, 0);
                                          DBG((yyval.stat), (yylsp[(1) - (3)]), (yylsp[(3) - (3)])); ;}
    break;

  case 293:
#line 824 "grammar.y"
    { (yyval.stat) = makeImportNode((yyvsp[(2) - (3)].pkgn), false, 0);
                                          DBG((yyval.stat), (yylsp[(1) - (3)]), (yylsp[(3) - (3)])); AUTO_SEMICOLON; ;}
    break;

  case 294:
#line 826 "grammar.y"
    { (yyval.stat) = makeImportNode((yyvsp[(4) - (5)].pkgn), false, *(yyvsp[(2) - (5)].ident));
                                          DBG((yyval.stat), (yylsp[(1) - (5)]), (yylsp[(5) - (5)])); ;}
    break;

  case 295:
#line 828 "grammar.y"
    { (yyval.stat) = makeImportNode((yyvsp[(4) - (5)].pkgn), false, *(yyvsp[(2) - (5)].ident));
                                          DBG((yyval.stat), (yylsp[(1) - (5)]), (yylsp[(5) - (5)])); AUTO_SEMICOLON; ;}
    break;

  case 296:
#line 833 "grammar.y"
    { (yyval.func) = new FuncDeclNode(*(yyvsp[(2) - (5)].ident), (yyvsp[(5) - (5)].body)); ;}
    break;

  case 297:
#line 835 "grammar.y"
    { (yyval.func) = new FuncDeclNode(*(yyvsp[(2) - (6)].ident), (yyvsp[(4) - (6)].param), (yyvsp[(6) - (6)].body)); ;}
    break;

  case 298:
#line 839 "grammar.y"
    { (yyval.funcExpr) = new FuncExprNode(CommonIdentifiers::shared()->nullIdentifier, (yyvsp[(4) - (4)].body)); ;}
    break;

  case 299:
#line 841 "grammar.y"
    { (yyval.funcExpr) = new FuncExprNode(CommonIdentifiers::shared()->nullIdentifier, (yyvsp[(5) - (5)].body), (yyvsp[(3) - (5)].param)); ;}
    break;

  case 300:
#line 842 "grammar.y"
    { (yyval.funcExpr) = new FuncExprNode(*(yyvsp[(2) - (5)].ident), (yyvsp[(5) - (5)].body)); ;}
    break;

  case 301:
#line 844 "grammar.y"
    { (yyval.funcExpr) = new FuncExprNode(*(yyvsp[(2) - (6)].ident), (yyvsp[(6) - (6)].body), (yyvsp[(4) - (6)].param)); ;}
    break;

  case 302:
#line 848 "grammar.y"
    { (yyval.param) = new ParameterNode(*(yyvsp[(1) - (1)].ident)); ;}
    break;

  case 303:
#line 849 "grammar.y"
    { (yyval.param) = new ParameterNode((yyvsp[(1) - (3)].param), *(yyvsp[(3) - (3)].ident)); ;}
    break;

  case 304:
#line 853 "grammar.y"
    { (yyval.body) = new FunctionBodyNode(0); DBG((yyval.body), (yylsp[(1) - (2)]), (yylsp[(2) - (2)])); ;}
    break;

  case 305:
#line 854 "grammar.y"
    { (yyval.body) = new FunctionBodyNode((yyvsp[(2) - (3)].srcs)); DBG((yyval.body), (yylsp[(1) - (3)]), (yylsp[(3) - (3)])); ;}
    break;

  case 306:
#line 858 "grammar.y"
    { parser().didFinishParsing(new ProgramNode(0)); ;}
    break;

  case 307:
#line 859 "grammar.y"
    { parser().didFinishParsing(new ProgramNode((yyvsp[(1) - (1)].srcs))); ;}
    break;

  case 308:
#line 863 "grammar.y"
    { (yyval.srcs) = new SourceElementsNode((yyvsp[(1) - (1)].stat)); ;}
    break;

  case 309:
#line 864 "grammar.y"
    { (yyval.srcs) = new SourceElementsNode((yyvsp[(1) - (2)].srcs), (yyvsp[(2) - (2)].stat)); ;}
    break;

  case 310:
#line 868 "grammar.y"
    { (yyval.stat) = (yyvsp[(1) - (1)].func); ;}
    break;

  case 311:
#line 869 "grammar.y"
    { (yyval.stat) = (yyvsp[(1) - (1)].stat); ;}
    break;


/* Line 1267 of yacc.c.  */
#line 3508 "grammar.tab.c"
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


#line 872 "grammar.y"


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

