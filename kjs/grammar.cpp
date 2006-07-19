/* A Bison parser, made by GNU Bison 2.2.  */

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
#define YYBISON_VERSION "2.2"

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
     IF_WITHOUT_ELSE = 288,
     ELSE = 289,
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
     NUMBER = 314,
     STRING = 315,
     IDENT = 316,
     AUTOPLUSPLUS = 317,
     AUTOMINUSMINUS = 318
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
#define IF_WITHOUT_ELSE 288
#define ELSE 289
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
#define NUMBER 314
#define STRING 315
#define IDENT 316
#define AUTOPLUSPLUS 317
#define AUTOMINUSMINUS 318




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

#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 65 "grammar.y"
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
}
/* Line 193 of yacc.c.  */
#line 322 "grammar.tab.c"
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
#line 347 "grammar.tab.c"

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
#define YYFINAL  208
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1465

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  88
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  102
/* YYNRULES -- Number of rules.  */
#define YYNRULES  301
/* YYNRULES -- Number of states.  */
#define YYNSTATES  528

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   318

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    77,     2,     2,     2,    79,    82,     2,
      66,    67,    78,    74,    68,    75,    73,    64,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    65,    87,
      80,    86,    81,    85,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    71,     2,    72,    83,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    69,    84,    70,    76,     2,     2,     2,
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
      55,    56,    57,    58,    59,    60,    61,    62,    63
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
     646,   648,   650,   652,   654,   656,   658,   660,   662,   665,
     669,   673,   677,   679,   683,   685,   689,   691,   694,   696,
     699,   703,   707,   709,   713,   715,   718,   721,   724,   726,
     729,   732,   738,   746,   753,   759,   769,   780,   788,   797,
     807,   808,   810,   811,   813,   816,   819,   823,   827,   830,
     833,   837,   841,   844,   847,   851,   855,   861,   867,   871,
     877,   878,   880,   882,   885,   889,   894,   897,   901,   905,
     909,   913,   918,   926,   936,   939,   942,   948,   955,   960,
     966,   972,   979,   981,   985,   988,   992,   993,   995,   997,
    1000,  1002
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     187,     0,    -1,     3,    -1,     4,    -1,     5,    -1,    59,
      -1,    60,    -1,    64,    -1,    51,    -1,    61,    -1,    60,
      -1,    59,    -1,    90,    65,   144,    -1,    61,    61,    66,
      67,   186,    -1,    61,    61,    66,   185,    67,   186,    -1,
      91,    -1,    92,    68,    91,    -1,    94,    -1,    69,    70,
      -1,    69,    92,    70,    -1,    69,    92,    68,    70,    -1,
      19,    -1,    89,    -1,    95,    -1,    61,    -1,    66,   148,
      67,    -1,    71,    97,    72,    -1,    71,    96,    72,    -1,
      71,    96,    68,    97,    72,    -1,    97,   144,    -1,    96,
      68,    97,   144,    -1,    -1,    98,    -1,    68,    -1,    98,
      68,    -1,    93,    -1,   184,    -1,    99,    71,   148,    72,
      -1,    99,    73,    61,    -1,    10,    99,   105,    -1,    94,
      -1,   100,    71,   148,    72,    -1,   100,    73,    61,    -1,
      10,    99,   105,    -1,    99,    -1,    10,   101,    -1,   100,
      -1,    10,   101,    -1,    99,   105,    -1,   103,   105,    -1,
     103,    71,   148,    72,    -1,   103,    73,    61,    -1,   100,
     105,    -1,   104,   105,    -1,   104,    71,   148,    72,    -1,
     104,    73,    61,    -1,    66,    67,    -1,    66,   106,    67,
      -1,   144,    -1,   106,    68,   144,    -1,   101,    -1,   103,
      -1,   102,    -1,   104,    -1,   107,    -1,   107,    43,    -1,
     107,    44,    -1,   108,    -1,   108,    43,    -1,   108,    44,
      -1,    17,   112,    -1,    16,   112,    -1,    24,   112,    -1,
      43,   112,    -1,    62,   112,    -1,    44,   112,    -1,    63,
     112,    -1,    74,   112,    -1,    75,   112,    -1,    76,   112,
      -1,    77,   112,    -1,   109,    -1,   111,    -1,   110,    -1,
     111,    -1,   112,    -1,   114,    78,   112,    -1,   114,    64,
     112,    -1,   114,    79,   112,    -1,   113,    -1,   115,    78,
     112,    -1,   115,    64,   112,    -1,   115,    79,   112,    -1,
     114,    -1,   116,    74,   114,    -1,   116,    75,   114,    -1,
     115,    -1,   117,    74,   114,    -1,   117,    75,   114,    -1,
     116,    -1,   118,    45,   116,    -1,   118,    46,   116,    -1,
     118,    47,   116,    -1,   117,    -1,   119,    45,   116,    -1,
     119,    46,   116,    -1,   119,    47,   116,    -1,   118,    -1,
     120,    80,   118,    -1,   120,    81,   118,    -1,   120,    39,
     118,    -1,   120,    40,   118,    -1,   120,    23,   118,    -1,
     120,    22,   118,    -1,   118,    -1,   121,    80,   118,    -1,
     121,    81,   118,    -1,   121,    39,   118,    -1,   121,    40,
     118,    -1,   121,    23,   118,    -1,   119,    -1,   122,    80,
     118,    -1,   122,    81,   118,    -1,   122,    39,   118,    -1,
     122,    40,   118,    -1,   122,    23,   118,    -1,   122,    22,
     118,    -1,   120,    -1,   123,    35,   120,    -1,   123,    36,
     120,    -1,   123,    37,   120,    -1,   123,    38,   120,    -1,
     121,    -1,   124,    35,   121,    -1,   124,    36,   121,    -1,
     124,    37,   121,    -1,   124,    38,   121,    -1,   122,    -1,
     125,    35,   120,    -1,   125,    36,   120,    -1,   125,    37,
     120,    -1,   125,    38,   120,    -1,   123,    -1,   126,    82,
     123,    -1,   124,    -1,   127,    82,   124,    -1,   125,    -1,
     128,    82,   123,    -1,   126,    -1,   129,    83,   126,    -1,
     127,    -1,   130,    83,   127,    -1,   128,    -1,   131,    83,
     126,    -1,   129,    -1,   132,    84,   129,    -1,   130,    -1,
     133,    84,   130,    -1,   131,    -1,   134,    84,   129,    -1,
     132,    -1,   135,    42,   132,    -1,   133,    -1,   136,    42,
     133,    -1,   134,    -1,   137,    42,   132,    -1,   135,    -1,
     138,    41,   135,    -1,   136,    -1,   139,    41,   136,    -1,
     137,    -1,   140,    41,   135,    -1,   138,    -1,   138,    85,
     144,    65,   144,    -1,   139,    -1,   139,    85,   145,    65,
     145,    -1,   140,    -1,   140,    85,   144,    65,   144,    -1,
     141,    -1,   107,   147,   144,    -1,   142,    -1,   107,   147,
     145,    -1,   143,    -1,   108,   147,   144,    -1,    86,    -1,
      48,    -1,    49,    -1,    50,    -1,    51,    -1,    52,    -1,
      53,    -1,    54,    -1,    55,    -1,    57,    -1,    58,    -1,
      56,    -1,   144,    -1,   148,    68,   144,    -1,   145,    -1,
     149,    68,   145,    -1,   146,    -1,   150,    68,   144,    -1,
     152,    -1,   153,    -1,   158,    -1,   163,    -1,   164,    -1,
     165,    -1,   166,    -1,   169,    -1,   170,    -1,   171,    -1,
     172,    -1,   173,    -1,   179,    -1,   180,    -1,   181,    -1,
     182,    -1,    69,    70,    -1,    69,   188,    70,    -1,    11,
     154,    87,    -1,    11,   154,     1,    -1,   156,    -1,   154,
      68,   156,    -1,   157,    -1,   155,    68,   157,    -1,    61,
      -1,    61,   161,    -1,    61,    -1,    61,   162,    -1,    12,
     159,    87,    -1,    12,   159,     1,    -1,   160,    -1,   159,
      68,   160,    -1,    61,    -1,    61,   161,    -1,    86,   144,
      -1,    86,   145,    -1,    87,    -1,   150,    87,    -1,   150,
       1,    -1,    18,    66,   148,    67,   151,    -1,    18,    66,
     148,    67,   151,    34,   151,    -1,    20,   151,    21,    66,
     148,    67,    -1,    21,    66,   148,    67,   151,    -1,     9,
      66,   168,    87,   167,    87,   167,    67,   151,    -1,     9,
      66,    11,   155,    87,   167,    87,   167,    67,   151,    -1,
       9,    66,   107,    22,   148,    67,   151,    -1,     9,    66,
      11,    61,    22,   148,    67,   151,    -1,     9,    66,    11,
      61,   162,    22,   148,    67,   151,    -1,    -1,   148,    -1,
      -1,   149,    -1,    13,    87,    -1,    13,     1,    -1,    13,
      61,    87,    -1,    13,    61,     1,    -1,     6,    87,    -1,
       6,     1,    -1,     6,    61,    87,    -1,     6,    61,     1,
      -1,    15,    87,    -1,    15,     1,    -1,    15,   148,    87,
      -1,    15,   148,     1,    -1,    26,    66,   148,    67,   151,
      -1,    25,    66,   148,    67,   174,    -1,    69,   175,    70,
      -1,    69,   175,   178,   175,    70,    -1,    -1,   176,    -1,
     177,    -1,   176,   177,    -1,     7,   148,    65,    -1,     7,
     148,    65,   188,    -1,     8,    65,    -1,     8,    65,   188,
      -1,    61,    65,   151,    -1,    28,   148,    87,    -1,    28,
     148,     1,    -1,    29,   152,    31,   152,    -1,    29,   152,
      30,    66,    61,    67,   152,    -1,    29,   152,    30,    66,
      61,    67,   152,    31,   152,    -1,    32,    87,    -1,    32,
       1,    -1,    14,    61,    66,    67,   186,    -1,    14,    61,
      66,   185,    67,   186,    -1,    14,    66,    67,   186,    -1,
      14,    66,   185,    67,   186,    -1,    14,    61,    66,    67,
     186,    -1,    14,    61,    66,   185,    67,   186,    -1,    61,
      -1,   185,    68,    61,    -1,    69,    70,    -1,    69,   188,
      70,    -1,    -1,   188,    -1,   189,    -1,   188,   189,    -1,
     183,    -1,   151,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   190,   190,   191,   192,   193,   194,   195,   200,   208,
     209,   210,   214,   215,   216,   221,   222,   226,   227,   228,
     230,   234,   235,   236,   237,   238,   242,   243,   244,   248,
     249,   254,   255,   259,   260,   264,   265,   266,   267,   268,
     272,   273,   274,   275,   279,   280,   284,   285,   289,   290,
     291,   292,   296,   297,   298,   299,   303,   304,   308,   309,
     313,   314,   318,   319,   323,   324,   325,   329,   330,   331,
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   348,   349,   353,   354,   358,   359,   360,   361,   365,
     366,   368,   370,   375,   376,   377,   381,   382,   384,   389,
     390,   391,   392,   396,   397,   398,   399,   403,   404,   405,
     406,   407,   408,   409,   413,   414,   415,   416,   417,   418,
     423,   424,   425,   426,   427,   428,   430,   434,   435,   436,
     437,   438,   442,   443,   445,   447,   449,   454,   455,   457,
     458,   460,   465,   466,   470,   471,   476,   477,   481,   482,
     486,   487,   492,   493,   498,   499,   503,   504,   509,   510,
     515,   516,   520,   521,   526,   527,   532,   533,   537,   538,
     543,   544,   548,   549,   554,   555,   560,   561,   566,   567,
     572,   573,   578,   579,   584,   585,   586,   587,   588,   589,
     590,   591,   592,   593,   594,   595,   599,   600,   604,   605,
     609,   610,   614,   615,   616,   617,   618,   619,   620,   621,
     622,   623,   624,   625,   626,   627,   628,   629,   633,   634,
     638,   639,   643,   644,   649,   650,   655,   656,   660,   661,
     665,   666,   670,   671,   676,   677,   681,   685,   689,   693,
     694,   698,   700,   705,   706,   707,   709,   711,   719,   721,
     726,   727,   731,   732,   736,   737,   738,   739,   743,   744,
     745,   746,   750,   751,   752,   753,   757,   761,   765,   766,
     771,   772,   776,   777,   781,   782,   786,   787,   791,   795,
     796,   800,   801,   802,   807,   808,   812,   813,   818,   819,
     821,   822,   827,   828,   832,   833,   837,   838,   842,   843,
     847,   848
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "NULLTOKEN", "TRUETOKEN", "FALSETOKEN",
  "BREAK", "CASE", "DEFAULT", "FOR", "NEW", "VAR", "CONST", "CONTINUE",
  "FUNCTION", "RETURN", "VOID", "DELETE", "IF", "THIS", "DO", "WHILE",
  "IN", "INSTANCEOF", "TYPEOF", "SWITCH", "WITH", "RESERVED", "THROW",
  "TRY", "CATCH", "FINALLY", "DEBUGGER", "IF_WITHOUT_ELSE", "ELSE", "EQEQ",
  "NE", "STREQ", "STRNEQ", "LE", "GE", "OR", "AND", "PLUSPLUS",
  "MINUSMINUS", "LSHIFT", "RSHIFT", "URSHIFT", "PLUSEQUAL", "MINUSEQUAL",
  "MULTEQUAL", "DIVEQUAL", "LSHIFTEQUAL", "RSHIFTEQUAL", "URSHIFTEQUAL",
  "ANDEQUAL", "MODEQUAL", "XOREQUAL", "OREQUAL", "NUMBER", "STRING",
  "IDENT", "AUTOPLUSPLUS", "AUTOMINUSMINUS", "'/'", "':'", "'('", "')'",
  "','", "'{'", "'}'", "'['", "']'", "'.'", "'+'", "'-'", "'~'", "'!'",
  "'*'", "'%'", "'<'", "'>'", "'&'", "'^'", "'|'", "'?'", "'='", "';'",
  "$accept", "Literal", "PropertyName", "Property", "PropertyList",
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
  "DebuggerStatement", "FunctionDeclaration", "FunctionExpr",
  "FormalParameterList", "FunctionBody", "Program", "SourceElements",
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
     315,   316,   317,   318,    47,    58,    40,    41,    44,   123,
     125,    91,    93,    46,    43,    45,   126,    33,    42,    37,
      60,    62,    38,    94,   124,    63,    61,    59
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    88,    89,    89,    89,    89,    89,    89,    89,    90,
      90,    90,    91,    91,    91,    92,    92,    93,    93,    93,
      93,    94,    94,    94,    94,    94,    95,    95,    95,    96,
      96,    97,    97,    98,    98,    99,    99,    99,    99,    99,
     100,   100,   100,   100,   101,   101,   102,   102,   103,   103,
     103,   103,   104,   104,   104,   104,   105,   105,   106,   106,
     107,   107,   108,   108,   109,   109,   109,   110,   110,   110,
     111,   111,   111,   111,   111,   111,   111,   111,   111,   111,
     111,   112,   112,   113,   113,   114,   114,   114,   114,   115,
     115,   115,   115,   116,   116,   116,   117,   117,   117,   118,
     118,   118,   118,   119,   119,   119,   119,   120,   120,   120,
     120,   120,   120,   120,   121,   121,   121,   121,   121,   121,
     122,   122,   122,   122,   122,   122,   122,   123,   123,   123,
     123,   123,   124,   124,   124,   124,   124,   125,   125,   125,
     125,   125,   126,   126,   127,   127,   128,   128,   129,   129,
     130,   130,   131,   131,   132,   132,   133,   133,   134,   134,
     135,   135,   136,   136,   137,   137,   138,   138,   139,   139,
     140,   140,   141,   141,   142,   142,   143,   143,   144,   144,
     145,   145,   146,   146,   147,   147,   147,   147,   147,   147,
     147,   147,   147,   147,   147,   147,   148,   148,   149,   149,
     150,   150,   151,   151,   151,   151,   151,   151,   151,   151,
     151,   151,   151,   151,   151,   151,   151,   151,   152,   152,
     153,   153,   154,   154,   155,   155,   156,   156,   157,   157,
     158,   158,   159,   159,   160,   160,   161,   162,   163,   164,
     164,   165,   165,   166,   166,   166,   166,   166,   166,   166,
     167,   167,   168,   168,   169,   169,   169,   169,   170,   170,
     170,   170,   171,   171,   171,   171,   172,   173,   174,   174,
     175,   175,   176,   176,   177,   177,   178,   178,   179,   180,
     180,   181,   181,   181,   182,   182,   183,   183,   184,   184,
     184,   184,   185,   185,   186,   186,   187,   187,   188,   188,
     189,   189
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
       1,     1,     1,     1,     1,     1,     1,     1,     2,     3,
       3,     3,     1,     3,     1,     3,     1,     2,     1,     2,
       3,     3,     1,     3,     1,     2,     2,     2,     1,     2,
       2,     5,     7,     6,     5,     9,    10,     7,     8,     9,
       0,     1,     0,     1,     2,     2,     3,     3,     2,     2,
       3,     3,     2,     2,     3,     3,     5,     5,     3,     5,
       0,     1,     1,     2,     3,     4,     2,     3,     3,     3,
       3,     4,     7,     9,     2,     2,     5,     6,     4,     5,
       5,     6,     1,     3,     2,     3,     0,     1,     1,     2,
       1,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
     296,     2,     3,     4,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    21,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     8,     5,     6,    24,
       0,     0,     7,     0,     0,    31,     0,     0,     0,     0,
     238,    22,    40,    23,    46,    62,    63,    67,    83,    84,
      89,    96,   103,   120,   137,   146,   152,   158,   164,   170,
     176,   182,   200,     0,   301,   202,   203,   204,   205,   206,
     207,   208,   209,   210,   211,   212,   213,   214,   215,   216,
     217,   300,     0,   297,   298,   259,     0,   258,   252,     0,
       0,    24,     0,    35,    17,    44,    47,    36,   226,     0,
     222,   234,     0,   232,   255,     0,   254,     0,   263,   262,
      44,    60,    61,    64,    81,    82,    85,    93,    99,   107,
     127,   142,   148,   154,   160,   166,   172,   178,   196,     0,
      64,    71,    70,     0,     0,     0,    72,     0,     0,     0,
       0,   285,   284,    73,    75,     0,    74,    76,     0,   218,
       0,    33,     0,     0,    32,    77,    78,    79,    80,     0,
       0,     0,    52,     0,     0,    53,    68,    69,   185,   186,
     187,   188,   189,   190,   191,   192,   195,   193,   194,   184,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   240,     0,   239,     1,   299,
     261,   260,     0,    64,   114,   132,   144,   150,   156,   162,
     168,   174,   180,   198,   253,     0,    44,    45,     0,     0,
      11,    10,     9,    18,     0,    15,     0,     0,     0,    43,
       0,   227,   221,     0,   220,   235,   231,     0,   230,   257,
     256,     0,    48,     0,     0,    49,    65,    66,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   265,     0,   264,     0,     0,     0,     0,
       0,   280,   279,     0,     0,   278,    25,   219,    31,    27,
      26,    29,    34,    56,     0,    58,     0,    42,     0,    55,
     183,    91,    90,    92,    97,    98,   104,   105,   106,   126,
     125,   123,   124,   121,   122,   138,   139,   140,   141,   147,
     153,   159,   165,   171,     0,   201,   228,     0,   224,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   250,    39,     0,
     292,     0,     0,     0,     0,     0,    19,     0,    38,   236,
     223,   233,     0,     0,     0,    51,   179,    87,    86,    88,
      94,    95,   100,   101,   102,   113,   112,   110,   111,   108,
     109,   128,   129,   130,   131,   143,   149,   155,   161,   167,
       0,   197,     0,     0,     0,     0,     0,     0,   281,     0,
      57,     0,    41,    54,     0,     0,     0,   229,     0,   250,
       0,    64,   181,   119,   117,   118,   115,   116,   133,   134,
     135,   136,   145,   151,   157,   163,   169,     0,   199,   251,
       0,     0,     0,     0,   288,     0,     0,     0,    12,    20,
      16,    37,   286,     0,    50,     0,   241,     0,   244,   270,
     267,   266,     0,    28,    30,    59,   177,     0,   237,     0,
     228,   225,     0,     0,     0,   250,   290,     0,   294,     0,
     289,   293,     0,     0,   287,   173,     0,   243,     0,     0,
     271,   272,     0,     0,     0,   229,   250,   247,   175,     0,
     291,   295,    13,     0,   242,     0,     0,   268,   270,   273,
     282,   248,     0,     0,     0,    14,   274,   276,     0,     0,
     249,     0,   245,   275,   277,   269,   283,   246
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    41,   234,   235,   236,    93,    94,    43,   152,   153,
     154,   110,    44,   111,    45,   112,    46,   162,   304,   130,
      47,   114,    48,   115,   116,    50,   117,    51,   118,    52,
     119,    53,   120,   215,    54,   121,   216,    55,   122,   217,
      56,   123,   218,    57,   124,   219,    58,   125,   220,    59,
     126,   221,    60,   127,   222,    61,   128,   223,    62,   340,
     439,   224,    63,    64,    65,    66,    99,   337,   100,   338,
      67,   102,   103,   241,   417,    68,    69,    70,    71,   440,
     225,    72,    73,    74,    75,    76,   460,   489,   490,   491,
     508,    77,    78,    79,    80,    81,    97,   362,   444,    82,
      83,    84
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -359
static const yytype_int16 yypact[] =
{
     934,  -359,  -359,  -359,     4,   -52,  1124,   -24,   -17,     5,
      -2,   539,  1388,  1388,     6,  -359,  1013,     8,  1388,    64,
      73,  1388,    94,    25,  1388,  1388,  -359,  -359,  -359,    83,
    1388,  1388,  -359,  1388,   618,   144,  1388,  1388,  1388,  1388,
    -359,  -359,  -359,  -359,   -11,  -359,   143,   518,  -359,  -359,
    -359,    79,   -53,   225,   184,   158,    92,   142,   179,   236,
     -14,  -359,  -359,    16,  -359,  -359,  -359,  -359,  -359,  -359,
    -359,  -359,  -359,  -359,  -359,  -359,  -359,  -359,  -359,  -359,
    -359,  -359,   323,   934,  -359,  -359,    31,  -359,  1088,  1124,
     115,  -359,   127,  -359,  -359,   170,  -359,  -359,   281,    17,
    -359,   281,    22,  -359,  -359,    34,  -359,   200,  -359,  -359,
     170,  -359,   174,  1067,  -359,  -359,  -359,   106,   163,   243,
     188,   239,   263,   267,   284,   328,    -5,  -359,  -359,    27,
     212,  -359,  -359,  1388,   351,  1388,  -359,  1388,  1388,    28,
     228,  -359,  -359,  -359,  -359,  1013,  -359,  -359,   229,  -359,
     697,  -359,   136,  1163,   307,  -359,  -359,  -359,  -359,  1238,
    1388,   315,  -359,  1388,   316,  -359,  -359,  -359,  -359,  -359,
    -359,  -359,  -359,  -359,  -359,  -359,  -359,  -359,  -359,  -359,
    1388,  1388,  1388,  1388,  1388,  1388,  1388,  1388,  1388,  1388,
    1388,  1388,  1388,  1388,  1388,  1388,  1388,  1388,  1388,  1388,
    1388,  1388,  1388,  1388,  1388,  -359,  1388,  -359,  -359,  -359,
    -359,  -359,   318,   303,   243,   180,   245,   298,   299,   300,
     341,    86,  -359,  -359,   317,   301,   170,  -359,   320,     3,
    -359,  -359,   326,  -359,   330,  -359,   224,  1388,   332,  -359,
    1388,  -359,  -359,   -24,  -359,  -359,  -359,   -17,  -359,  -359,
    -359,    12,  -359,  1388,   335,  -359,  -359,  -359,  1388,  1388,
    1388,  1388,  1388,  1388,  1388,  1388,  1388,  1388,  1388,  1388,
    1388,  1388,  1388,  1388,  1388,  1388,  1388,  1388,  1388,  1388,
    1388,  1388,  1388,  -359,  1388,  -359,   231,   325,   233,   252,
     254,  -359,  -359,   331,    94,  -359,  -359,  -359,   144,  -359,
    -359,  -359,  -359,  -359,   259,  -359,   149,  -359,   154,  -359,
    -359,  -359,  -359,  -359,   106,   106,   163,   163,   163,   243,
     243,   243,   243,   243,   243,   188,   188,   188,   188,   239,
     263,   267,   284,   328,   333,  -359,    -9,   -44,  -359,  1388,
    1388,  1388,  1388,  1388,  1388,  1388,  1388,  1388,  1388,  1388,
    1388,  1388,  1388,  1388,  1388,  1388,  1388,  1388,  -359,   100,
    -359,   334,   264,   342,  1388,   131,  -359,   167,  -359,  -359,
    -359,  -359,   334,   266,   185,  -359,  -359,  -359,  -359,  -359,
     106,   106,   163,   163,   163,   243,   243,   243,   243,   243,
     243,   188,   188,   188,   188,   239,   263,   267,   284,   328,
     336,  -359,  1013,  1388,  1013,   338,  1013,   339,  -359,  1313,
    -359,  1388,  -359,  -359,  1388,  1388,  1388,   377,   348,  1388,
     268,  1067,  -359,   243,   243,   243,   243,   243,   180,   180,
     180,   180,   245,   298,   299,   300,   341,   345,  -359,   343,
     327,   334,   270,   776,  -359,   334,   352,   187,  -359,  -359,
    -359,  -359,  -359,   334,  -359,  1388,   388,   272,  -359,   419,
    -359,  -359,   360,  -359,  -359,  -359,  -359,   274,  -359,  1388,
     344,  -359,   346,  1013,  1388,  1388,  -359,   334,  -359,   855,
    -359,  -359,   334,   276,  -359,  -359,  1013,  -359,  1388,   112,
     419,  -359,    94,  1013,   295,  -359,  1388,  -359,  -359,   362,
    -359,  -359,  -359,   334,  -359,    63,   366,  -359,   419,  -359,
     401,  -359,  1013,   367,  1013,  -359,   934,   934,   365,    94,
    -359,  1013,  -359,   934,   934,  -359,  -359,  -359
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -359,  -359,  -359,    72,  -359,  -359,     0,  -359,  -359,   140,
    -359,    13,  -359,    33,  -359,  -359,  -359,   -43,  -359,     9,
    -359,  -359,  -359,    15,    69,  -359,  -174,  -359,  -130,  -359,
     -38,  -359,  -150,   -62,  -359,  -191,    89,  -359,  -167,    90,
    -359,  -163,    88,  -359,  -151,    91,  -359,  -149,    93,  -359,
    -359,  -359,  -359,  -359,  -359,  -359,   -40,  -315,  -359,   -35,
      42,  -359,  -359,   -12,   -21,  -359,  -359,  -359,   199,    30,
    -359,  -359,   204,   353,   -18,  -359,  -359,  -359,  -359,  -358,
    -359,  -359,  -359,  -359,  -359,  -359,  -359,   -55,  -359,   -34,
    -359,  -359,  -359,  -359,  -359,  -359,  -359,  -249,  -304,  -359,
     -27,   -74
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const yytype_uint16 yytable[] =
{
      42,   140,   373,   165,   134,    85,   104,   150,   329,   209,
     314,   315,   180,   415,    88,    49,    42,   205,   242,    95,
     113,   184,   185,   246,   418,   422,   141,   203,   283,   291,
     113,    49,   210,   330,    42,   249,   281,    98,   331,    96,
     437,   438,   113,   419,   101,   325,   326,   327,   328,    49,
     214,   332,   239,   129,   333,   159,   316,   317,   318,   107,
     160,   472,   161,   139,   360,    86,   105,   252,   452,   255,
     361,   204,   133,   360,   135,   148,   209,   416,   258,   372,
     282,   131,   132,    42,   206,   243,   395,   136,   380,   381,
     247,    87,   106,   143,   144,   284,   284,   213,    49,   146,
     147,   468,   226,   207,   244,   155,   156,   157,   158,   248,
     442,   396,   142,   301,   285,   292,   397,   499,   211,   305,
     506,   250,   227,   391,   392,   393,   394,   354,   516,   398,
     137,   284,   399,   295,   382,   383,   384,   476,   513,   138,
     310,   480,   113,   181,   113,    42,   113,   113,   145,   484,
      42,   319,   320,   321,   322,   323,   324,   182,   183,   498,
      49,   360,   113,    34,   334,    49,   335,   441,   113,   113,
     259,   355,   113,   500,   199,   286,   228,   288,   502,   289,
     290,   229,   507,   358,   260,   261,   230,   231,   232,   113,
     230,   231,   232,   195,   196,   197,   198,   233,   483,   515,
     369,   449,   306,   341,   298,   308,   189,   190,   299,   159,
     267,   268,   151,   113,   163,   113,   164,   284,   376,   342,
     343,   412,   284,   191,   192,   200,   413,   269,   270,   385,
     386,   387,   388,   389,   390,   284,   159,   262,   263,   451,
     159,   237,   400,   238,   401,   253,   113,   254,   360,   113,
     311,   312,   313,   284,   482,   256,   257,   454,   293,   294,
     344,   345,   113,   201,   193,   194,   251,   113,   271,   272,
     186,   187,   188,   408,   273,   274,   275,   276,   202,   367,
     346,   347,   348,   349,   428,   429,   430,   431,   264,   265,
     266,   113,   365,   113,   366,   374,   296,   284,   402,   284,
     404,   284,   214,   423,   424,   425,   426,   427,   214,   214,
     214,   214,   214,   214,   214,   214,   214,   214,   214,   405,
     284,   406,   284,   208,   448,   339,   410,   411,   377,   378,
     379,   445,   446,   453,   446,   473,   284,   477,   446,   487,
     284,   493,   284,   503,   446,   277,   256,   257,   113,   421,
     278,   168,   169,   170,   171,   172,   173,   174,   175,   176,
     177,   178,   512,   284,   421,   421,   113,   240,   279,   464,
     280,   465,   287,   113,   466,   302,   307,   309,   214,   336,
     350,   420,   351,   353,   352,   356,   359,   363,   357,   179,
     456,   403,   458,   368,   461,   364,   375,   407,   414,   469,
     462,   455,    42,   443,    42,   209,    42,   459,   447,   470,
     474,   284,   113,   481,   475,   485,   479,    49,   113,    49,
     113,    49,   486,   113,   113,   421,   488,   492,   113,   514,
     416,   517,   519,   496,   521,   525,   214,   450,   409,   432,
     434,   433,   370,    42,   435,   457,     0,   436,   471,   209,
     209,   371,   495,   518,   245,     0,   509,   467,    49,     0,
       0,   497,     0,     0,   113,     0,     0,     0,     0,     0,
       0,   510,     0,    42,   504,     0,     0,     0,   113,    42,
       0,   511,     0,   421,   113,     0,    42,     0,    49,   523,
     524,     0,     0,    42,    49,     0,     0,   113,   526,     0,
     520,    49,   522,     0,     0,   113,     0,     0,    49,   527,
       0,   494,    42,     0,    42,     0,    42,    42,     0,     0,
       0,    42,     0,    42,    42,     0,     0,    49,     0,    49,
     505,    49,    49,     0,     0,     0,    49,     0,    49,    49,
     108,     0,     1,     2,     3,     0,     0,     0,     0,    89,
       0,     0,     0,    90,     0,    12,    13,     0,    15,     0,
       0,   166,   167,    18,     0,     0,   168,   169,   170,   171,
     172,   173,   174,   175,   176,   177,   178,     0,     0,     0,
       0,     0,    24,    25,     0,     0,     0,     0,     0,     0,
      26,     0,     0,     0,     0,     0,     0,     0,    27,    28,
      91,    30,    31,    32,   179,    33,     0,     0,    92,     0,
      35,     0,     0,    36,    37,    38,    39,     0,     0,     0,
       0,     1,     2,     3,     4,     0,   109,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
       0,     0,    18,    19,    20,     0,    21,    22,     0,     0,
      23,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    24,    25,     0,     0,     0,     0,     0,     0,    26,
       0,     0,     0,     0,     0,     0,     0,    27,    28,    29,
      30,    31,    32,     0,    33,     0,     0,    34,   149,    35,
       0,     0,    36,    37,    38,    39,     0,     0,     0,     0,
       1,     2,     3,     4,     0,    40,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,     0,
       0,    18,    19,    20,     0,    21,    22,     0,     0,    23,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      24,    25,     0,     0,     0,     0,     0,     0,    26,     0,
       0,     0,     0,     0,     0,     0,    27,    28,    29,    30,
      31,    32,     0,    33,     0,     0,    34,   297,    35,     0,
       0,    36,    37,    38,    39,     0,     0,     0,     0,     1,
       2,     3,     4,     0,    40,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,     0,     0,
      18,    19,    20,     0,    21,    22,     0,     0,    23,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    24,
      25,     0,     0,     0,     0,     0,     0,    26,     0,     0,
       0,     0,     0,     0,     0,    27,    28,    29,    30,    31,
      32,     0,    33,     0,     0,    34,   478,    35,     0,     0,
      36,    37,    38,    39,     0,     0,     0,     0,     1,     2,
       3,     4,     0,    40,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,     0,     0,    18,
      19,    20,     0,    21,    22,     0,     0,    23,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    24,    25,
       0,     0,     0,     0,     0,     0,    26,     0,     0,     0,
       0,     0,     0,     0,    27,    28,    29,    30,    31,    32,
       0,    33,     0,     0,    34,   501,    35,     0,     0,    36,
      37,    38,    39,     0,     0,     0,     0,     1,     2,     3,
       4,     0,    40,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,     0,     0,    18,    19,
      20,     0,    21,    22,     0,     0,    23,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    24,    25,     0,
       0,     0,     0,     0,     0,    26,     0,     0,     0,     0,
       0,     0,     0,    27,    28,    29,    30,    31,    32,     0,
      33,     0,     0,    34,     0,    35,     0,     0,    36,    37,
      38,    39,     0,     0,     0,     0,     1,     2,     3,     4,
       0,    40,     5,     6,     7,     8,     9,     0,    11,    12,
      13,    14,    15,    16,    17,     0,     0,    18,    19,    20,
       0,    21,    22,     0,     0,    23,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    24,    25,     0,     0,
       0,     0,     0,     0,    26,     0,     0,     0,     0,     0,
       0,     0,    27,    28,    29,    30,    31,    32,     0,    33,
       0,     0,    34,     0,    35,     0,     0,    36,    37,    38,
      39,     1,     2,     3,     0,     0,     0,     0,    89,   212,
      40,     0,    90,     0,    12,    13,     0,    15,     0,     0,
     256,   257,    18,     0,     0,   168,   169,   170,   171,   172,
     173,   174,   175,   176,   177,   178,     0,     1,     2,     3,
       0,    24,    25,     0,    89,     0,     0,     0,    90,    26,
       0,     0,     0,    15,     0,     0,     0,    27,    28,    91,
      30,    31,    32,   179,    33,     0,     0,    92,     0,    35,
       0,     0,    36,    37,    38,    39,     1,     2,     3,     0,
       0,     0,     0,    89,     0,    26,     0,    90,     0,    12,
      13,     0,    15,    27,    28,    91,     0,    18,    32,     0,
      33,     0,     0,    92,     0,    35,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    24,    25,     0,     0,
       0,     0,     0,     0,    26,     0,     0,     0,     0,     0,
       0,     0,    27,    28,    91,    30,    31,    32,     0,    33,
       0,     0,    92,     0,    35,   300,     0,    36,    37,    38,
      39,     1,     2,     3,     0,     0,     0,     0,    89,     0,
       0,     0,    90,     0,    12,    13,     0,    15,     0,     0,
       0,     0,    18,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    24,    25,     0,     0,     0,     0,     0,     0,    26,
       0,     0,     0,     0,     0,     0,     0,    27,    28,    91,
      30,    31,    32,     0,    33,   303,     0,    92,     0,    35,
       0,     0,    36,    37,    38,    39,     1,     2,     3,     0,
       0,     0,     0,    89,     0,     0,     0,    90,     0,    12,
      13,     0,    15,     0,     0,     0,     0,    18,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    24,    25,     0,     0,
       0,     0,     0,     0,    26,     0,     0,     0,     0,     0,
       0,     0,    27,    28,    91,    30,    31,    32,     0,    33,
       0,     0,    92,     0,    35,   463,     0,    36,    37,    38,
      39,     1,     2,     3,     0,     0,     0,     0,    89,     0,
       0,     0,    90,     0,    12,    13,     0,    15,     0,     0,
       0,     0,    18,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    24,    25,     0,     0,     0,     0,     0,     0,    26,
       0,     0,     0,     0,     0,     0,     0,    27,    28,    91,
      30,    31,    32,     0,    33,     0,     0,    92,     0,    35,
       0,     0,    36,    37,    38,    39
};

static const yytype_int16 yycheck[] =
{
       0,    22,   251,    46,    16,     1,     1,    34,   199,    83,
     184,   185,    47,    22,    66,     0,    16,     1,     1,     6,
      11,    74,    75,     1,    68,   340,     1,    41,     1,     1,
      21,    16,     1,   200,    34,     1,    41,    61,   201,     6,
     355,   356,    33,    87,    61,   195,   196,   197,   198,    34,
      88,   202,    95,    11,   203,    66,   186,   187,   188,    61,
      71,   419,    73,    21,    61,    61,    61,   110,   372,   112,
      67,    85,    66,    61,    66,    33,   150,    86,   113,    67,
      85,    12,    13,    83,    68,    68,   277,    18,   262,   263,
      68,    87,    87,    24,    25,    68,    68,    88,    83,    30,
      31,   416,    89,    87,    87,    36,    37,    38,    39,    87,
     359,   278,    87,   153,    87,    87,   279,   475,    87,   159,
       8,    87,    89,   273,   274,   275,   276,    41,    65,   280,
      66,    68,   281,   145,   264,   265,   266,   441,   496,    66,
     180,   445,   133,    64,   135,   145,   137,   138,    65,   453,
     150,   189,   190,   191,   192,   193,   194,    78,    79,   474,
     145,    61,   153,    69,   204,   150,   206,    67,   159,   160,
      64,    85,   163,   477,    82,   133,    61,   135,   482,   137,
     138,    66,    70,   226,    78,    79,    59,    60,    61,   180,
      59,    60,    61,    35,    36,    37,    38,    70,   447,   503,
     240,    70,   160,    23,    68,   163,    22,    23,    72,    66,
      22,    23,    68,   204,    71,   206,    73,    68,   258,    39,
      40,    72,    68,    39,    40,    83,    72,    39,    40,   267,
     268,   269,   270,   271,   272,    68,    66,    74,    75,    72,
      66,    71,   282,    73,   284,    71,   237,    73,    61,   240,
     181,   182,   183,    68,    67,    43,    44,    72,    30,    31,
      80,    81,   253,    84,    80,    81,    66,   258,    80,    81,
      45,    46,    47,   294,    35,    36,    37,    38,    42,   237,
      35,    36,    37,    38,   346,   347,   348,   349,    45,    46,
      47,   282,    68,   284,    70,   253,    67,    68,    67,    68,
      67,    68,   340,   341,   342,   343,   344,   345,   346,   347,
     348,   349,   350,   351,   352,   353,   354,   355,   356,    67,
      68,    67,    68,     0,   364,    22,    67,    68,   259,   260,
     261,    67,    68,    67,    68,    67,    68,    67,    68,    67,
      68,    67,    68,    67,    68,    82,    43,    44,   339,   340,
      83,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    67,    68,   355,   356,   357,    86,    84,   409,
      42,   411,    21,   364,   414,    68,    61,    61,   416,    61,
      82,   339,    83,    42,    84,    68,    66,    61,    87,    86,
     402,    66,   404,    61,   406,    65,    61,    66,    65,    22,
      61,    65,   402,    69,   404,   479,   406,    69,    66,    61,
      65,    68,   403,    61,    87,   455,   443,   402,   409,   404,
     411,   406,    34,   414,   415,   416,     7,    67,   419,    67,
      86,    65,    31,    87,    67,    70,   474,   365,   298,   350,
     352,   351,   243,   443,   353,   403,    -1,   354,   418,   523,
     524,   247,   470,   508,   101,    -1,   490,   415,   443,    -1,
      -1,   473,    -1,    -1,   455,    -1,    -1,    -1,    -1,    -1,
      -1,   492,    -1,   473,   486,    -1,    -1,    -1,   469,   479,
      -1,   493,    -1,   474,   475,    -1,   486,    -1,   473,   516,
     517,    -1,    -1,   493,   479,    -1,    -1,   488,   519,    -1,
     512,   486,   514,    -1,    -1,   496,    -1,    -1,   493,   521,
      -1,   469,   512,    -1,   514,    -1,   516,   517,    -1,    -1,
      -1,   521,    -1,   523,   524,    -1,    -1,   512,    -1,   514,
     488,   516,   517,    -1,    -1,    -1,   521,    -1,   523,   524,
       1,    -1,     3,     4,     5,    -1,    -1,    -1,    -1,    10,
      -1,    -1,    -1,    14,    -1,    16,    17,    -1,    19,    -1,
      -1,    43,    44,    24,    -1,    -1,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    -1,    -1,    -1,
      -1,    -1,    43,    44,    -1,    -1,    -1,    -1,    -1,    -1,
      51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    59,    60,
      61,    62,    63,    64,    86,    66,    -1,    -1,    69,    -1,
      71,    -1,    -1,    74,    75,    76,    77,    -1,    -1,    -1,
      -1,     3,     4,     5,     6,    -1,    87,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      -1,    -1,    24,    25,    26,    -1,    28,    29,    -1,    -1,
      32,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    43,    44,    -1,    -1,    -1,    -1,    -1,    -1,    51,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    59,    60,    61,
      62,    63,    64,    -1,    66,    -1,    -1,    69,    70,    71,
      -1,    -1,    74,    75,    76,    77,    -1,    -1,    -1,    -1,
       3,     4,     5,     6,    -1,    87,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    -1,
      -1,    24,    25,    26,    -1,    28,    29,    -1,    -1,    32,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      43,    44,    -1,    -1,    -1,    -1,    -1,    -1,    51,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    59,    60,    61,    62,
      63,    64,    -1,    66,    -1,    -1,    69,    70,    71,    -1,
      -1,    74,    75,    76,    77,    -1,    -1,    -1,    -1,     3,
       4,     5,     6,    -1,    87,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    -1,    -1,
      24,    25,    26,    -1,    28,    29,    -1,    -1,    32,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    43,
      44,    -1,    -1,    -1,    -1,    -1,    -1,    51,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    59,    60,    61,    62,    63,
      64,    -1,    66,    -1,    -1,    69,    70,    71,    -1,    -1,
      74,    75,    76,    77,    -1,    -1,    -1,    -1,     3,     4,
       5,     6,    -1,    87,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    -1,    -1,    24,
      25,    26,    -1,    28,    29,    -1,    -1,    32,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    43,    44,
      -1,    -1,    -1,    -1,    -1,    -1,    51,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    59,    60,    61,    62,    63,    64,
      -1,    66,    -1,    -1,    69,    70,    71,    -1,    -1,    74,
      75,    76,    77,    -1,    -1,    -1,    -1,     3,     4,     5,
       6,    -1,    87,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    -1,    -1,    24,    25,
      26,    -1,    28,    29,    -1,    -1,    32,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    43,    44,    -1,
      -1,    -1,    -1,    -1,    -1,    51,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    59,    60,    61,    62,    63,    64,    -1,
      66,    -1,    -1,    69,    -1,    71,    -1,    -1,    74,    75,
      76,    77,    -1,    -1,    -1,    -1,     3,     4,     5,     6,
      -1,    87,     9,    10,    11,    12,    13,    -1,    15,    16,
      17,    18,    19,    20,    21,    -1,    -1,    24,    25,    26,
      -1,    28,    29,    -1,    -1,    32,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    43,    44,    -1,    -1,
      -1,    -1,    -1,    -1,    51,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    59,    60,    61,    62,    63,    64,    -1,    66,
      -1,    -1,    69,    -1,    71,    -1,    -1,    74,    75,    76,
      77,     3,     4,     5,    -1,    -1,    -1,    -1,    10,    11,
      87,    -1,    14,    -1,    16,    17,    -1,    19,    -1,    -1,
      43,    44,    24,    -1,    -1,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    -1,     3,     4,     5,
      -1,    43,    44,    -1,    10,    -1,    -1,    -1,    14,    51,
      -1,    -1,    -1,    19,    -1,    -1,    -1,    59,    60,    61,
      62,    63,    64,    86,    66,    -1,    -1,    69,    -1,    71,
      -1,    -1,    74,    75,    76,    77,     3,     4,     5,    -1,
      -1,    -1,    -1,    10,    -1,    51,    -1,    14,    -1,    16,
      17,    -1,    19,    59,    60,    61,    -1,    24,    64,    -1,
      66,    -1,    -1,    69,    -1,    71,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    43,    44,    -1,    -1,
      -1,    -1,    -1,    -1,    51,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    59,    60,    61,    62,    63,    64,    -1,    66,
      -1,    -1,    69,    -1,    71,    72,    -1,    74,    75,    76,
      77,     3,     4,     5,    -1,    -1,    -1,    -1,    10,    -1,
      -1,    -1,    14,    -1,    16,    17,    -1,    19,    -1,    -1,
      -1,    -1,    24,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    43,    44,    -1,    -1,    -1,    -1,    -1,    -1,    51,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    59,    60,    61,
      62,    63,    64,    -1,    66,    67,    -1,    69,    -1,    71,
      -1,    -1,    74,    75,    76,    77,     3,     4,     5,    -1,
      -1,    -1,    -1,    10,    -1,    -1,    -1,    14,    -1,    16,
      17,    -1,    19,    -1,    -1,    -1,    -1,    24,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    43,    44,    -1,    -1,
      -1,    -1,    -1,    -1,    51,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    59,    60,    61,    62,    63,    64,    -1,    66,
      -1,    -1,    69,    -1,    71,    72,    -1,    74,    75,    76,
      77,     3,     4,     5,    -1,    -1,    -1,    -1,    10,    -1,
      -1,    -1,    14,    -1,    16,    17,    -1,    19,    -1,    -1,
      -1,    -1,    24,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    43,    44,    -1,    -1,    -1,    -1,    -1,    -1,    51,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    59,    60,    61,
      62,    63,    64,    -1,    66,    -1,    -1,    69,    -1,    71,
      -1,    -1,    74,    75,    76,    77
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     3,     4,     5,     6,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    24,    25,
      26,    28,    29,    32,    43,    44,    51,    59,    60,    61,
      62,    63,    64,    66,    69,    71,    74,    75,    76,    77,
      87,    89,    94,    95,   100,   102,   104,   108,   110,   111,
     113,   115,   117,   119,   122,   125,   128,   131,   134,   137,
     140,   143,   146,   150,   151,   152,   153,   158,   163,   164,
     165,   166,   169,   170,   171,   172,   173,   179,   180,   181,
     182,   183,   187,   188,   189,     1,    61,    87,    66,    10,
      14,    61,    69,    93,    94,    99,   101,   184,    61,   154,
     156,    61,   159,   160,     1,    61,    87,    61,     1,    87,
      99,   101,   103,   107,   109,   111,   112,   114,   116,   118,
     120,   123,   126,   129,   132,   135,   138,   141,   144,   148,
     107,   112,   112,    66,   151,    66,   112,    66,    66,   148,
     152,     1,    87,   112,   112,    65,   112,   112,   148,    70,
     188,    68,    96,    97,    98,   112,   112,   112,   112,    66,
      71,    73,   105,    71,    73,   105,    43,    44,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    86,
     147,    64,    78,    79,    74,    75,    45,    46,    47,    22,
      23,    39,    40,    80,    81,    35,    36,    37,    38,    82,
      83,    84,    42,    41,    85,     1,    68,    87,     0,   189,
       1,    87,    11,   107,   118,   121,   124,   127,   130,   133,
     136,   139,   142,   145,   149,   168,    99,   101,    61,    66,
      59,    60,    61,    70,    90,    91,    92,    71,    73,   105,
      86,   161,     1,    68,    87,   161,     1,    68,    87,     1,
      87,    66,   105,    71,    73,   105,    43,    44,   147,    64,
      78,    79,    74,    75,    45,    46,    47,    22,    23,    39,
      40,    80,    81,    35,    36,    37,    38,    82,    83,    84,
      42,    41,    85,     1,    68,    87,   148,    21,   148,   148,
     148,     1,    87,    30,    31,   151,    67,    70,    68,    72,
      72,   144,    68,    67,   106,   144,   148,    61,   148,    61,
     144,   112,   112,   112,   114,   114,   116,   116,   116,   118,
     118,   118,   118,   118,   118,   120,   120,   120,   120,   123,
     126,   129,   132,   135,   144,   144,    61,   155,   157,    22,
     147,    23,    39,    40,    80,    81,    35,    36,    37,    38,
      82,    83,    84,    42,    41,    85,    68,    87,   105,    66,
      61,    67,   185,    61,    65,    68,    70,   148,    61,   144,
     156,   160,    67,   185,   148,    61,   144,   112,   112,   112,
     114,   114,   116,   116,   116,   118,   118,   118,   118,   118,
     118,   120,   120,   120,   120,   123,   126,   129,   132,   135,
     144,   144,    67,    66,    67,    67,    67,    66,   152,    97,
      67,    68,    72,    72,    65,    22,    86,   162,    68,    87,
     148,   107,   145,   118,   118,   118,   118,   118,   121,   121,
     121,   121,   124,   127,   130,   133,   136,   145,   145,   148,
     167,    67,   185,    69,   186,    67,    68,    66,   144,    70,
      91,    72,   186,    67,    72,    65,   151,   148,   151,    69,
     174,   151,    61,    72,   144,   144,   144,   148,   145,    22,
      61,   157,   167,    67,    65,    87,   186,    67,    70,   188,
     186,    61,    67,   185,   186,   144,    34,    67,     7,   175,
     176,   177,    67,    67,   148,   162,    87,   151,   145,   167,
     186,    70,   186,    67,   151,   148,     8,    70,   178,   177,
     152,   151,    67,   167,    67,   186,    65,    65,   175,    31,
     151,    67,   151,   188,   188,    70,   152,   151
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
yy_symbol_value_print (FILE *yyoutput, int yytype, const YYSTYPE * const yyvaluep, const YYLTYPE * const yylocationp)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep, yylocationp)
    FILE *yyoutput;
    int yytype;
    const YYSTYPE * const yyvaluep;
    const YYLTYPE * const yylocationp;
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
yy_symbol_print (FILE *yyoutput, int yytype, const YYSTYPE * const yyvaluep, const YYLTYPE * const yylocationp)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep, yylocationp)
    FILE *yyoutput;
    int yytype;
    const YYSTYPE * const yyvaluep;
    const YYLTYPE * const yylocationp;
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
yy_reduce_print (yyvsp, yylsp, yyrule
		   )
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
#line 190 "grammar.y"
    { (yyval.node) = new NullNode(); ;}
    break;

  case 3:
#line 191 "grammar.y"
    { (yyval.node) = new BooleanNode(true); ;}
    break;

  case 4:
#line 192 "grammar.y"
    { (yyval.node) = new BooleanNode(false); ;}
    break;

  case 5:
#line 193 "grammar.y"
    { (yyval.node) = new NumberNode((yyvsp[(1) - (1)].dval)); ;}
    break;

  case 6:
#line 194 "grammar.y"
    { (yyval.node) = new StringNode((yyvsp[(1) - (1)].ustr)); ;}
    break;

  case 7:
#line 195 "grammar.y"
    {
                                            Lexer *l = Lexer::curr();
                                            if (!l->scanRegExp()) YYABORT;
                                            (yyval.node) = new RegExpNode(l->pattern, l->flags);
                                        ;}
    break;

  case 8:
#line 200 "grammar.y"
    {
                                            Lexer *l = Lexer::curr();
                                            if (!l->scanRegExp()) YYABORT;
                                            (yyval.node) = new RegExpNode(UString('=') + l->pattern, l->flags);
                                        ;}
    break;

  case 9:
#line 208 "grammar.y"
    { (yyval.pname) = new PropertyNameNode(*(yyvsp[(1) - (1)].ident)); ;}
    break;

  case 10:
#line 209 "grammar.y"
    { (yyval.pname) = new PropertyNameNode(Identifier(*(yyvsp[(1) - (1)].ustr))); ;}
    break;

  case 11:
#line 210 "grammar.y"
    { (yyval.pname) = new PropertyNameNode((yyvsp[(1) - (1)].dval)); ;}
    break;

  case 12:
#line 214 "grammar.y"
    { (yyval.pnode) = new PropertyNode((yyvsp[(1) - (3)].pname), (yyvsp[(3) - (3)].node), PropertyNode::Constant); ;}
    break;

  case 13:
#line 215 "grammar.y"
    { if (!makeGetterOrSetterPropertyNode((yyval.pnode), *(yyvsp[(1) - (5)].ident), *(yyvsp[(2) - (5)].ident), 0, (yyvsp[(5) - (5)].body))) YYABORT; ;}
    break;

  case 14:
#line 217 "grammar.y"
    { if (!makeGetterOrSetterPropertyNode((yyval.pnode), *(yyvsp[(1) - (6)].ident), *(yyvsp[(2) - (6)].ident), (yyvsp[(4) - (6)].param), (yyvsp[(6) - (6)].body))) YYABORT; ;}
    break;

  case 15:
#line 221 "grammar.y"
    { (yyval.plist) = new PropertyListNode((yyvsp[(1) - (1)].pnode)); ;}
    break;

  case 16:
#line 222 "grammar.y"
    { (yyval.plist) = new PropertyListNode((yyvsp[(3) - (3)].pnode), (yyvsp[(1) - (3)].plist)); ;}
    break;

  case 18:
#line 227 "grammar.y"
    { (yyval.node) = new ObjectLiteralNode(); ;}
    break;

  case 19:
#line 228 "grammar.y"
    { (yyval.node) = new ObjectLiteralNode((yyvsp[(2) - (3)].plist)); ;}
    break;

  case 20:
#line 230 "grammar.y"
    { (yyval.node) = new ObjectLiteralNode((yyvsp[(2) - (4)].plist)); ;}
    break;

  case 21:
#line 234 "grammar.y"
    { (yyval.node) = new ThisNode(); ;}
    break;

  case 24:
#line 237 "grammar.y"
    { (yyval.node) = new ResolveNode(*(yyvsp[(1) - (1)].ident)); ;}
    break;

  case 25:
#line 238 "grammar.y"
    { (yyval.node) = (yyvsp[(2) - (3)].node)->isResolveNode() ? (yyvsp[(2) - (3)].node) : new GroupNode((yyvsp[(2) - (3)].node)); ;}
    break;

  case 26:
#line 242 "grammar.y"
    { (yyval.node) = new ArrayNode((yyvsp[(2) - (3)].ival)); ;}
    break;

  case 27:
#line 243 "grammar.y"
    { (yyval.node) = new ArrayNode((yyvsp[(2) - (3)].elm)); ;}
    break;

  case 28:
#line 244 "grammar.y"
    { (yyval.node) = new ArrayNode((yyvsp[(4) - (5)].ival), (yyvsp[(2) - (5)].elm)); ;}
    break;

  case 29:
#line 248 "grammar.y"
    { (yyval.elm) = new ElementNode((yyvsp[(1) - (2)].ival), (yyvsp[(2) - (2)].node)); ;}
    break;

  case 30:
#line 250 "grammar.y"
    { (yyval.elm) = new ElementNode((yyvsp[(1) - (4)].elm), (yyvsp[(3) - (4)].ival), (yyvsp[(4) - (4)].node)); ;}
    break;

  case 31:
#line 254 "grammar.y"
    { (yyval.ival) = 0; ;}
    break;

  case 33:
#line 259 "grammar.y"
    { (yyval.ival) = 1; ;}
    break;

  case 34:
#line 260 "grammar.y"
    { (yyval.ival) = (yyvsp[(1) - (2)].ival) + 1; ;}
    break;

  case 36:
#line 265 "grammar.y"
    { (yyval.node) = (yyvsp[(1) - (1)].funcExpr); ;}
    break;

  case 37:
#line 266 "grammar.y"
    { (yyval.node) = new BracketAccessorNode((yyvsp[(1) - (4)].node), (yyvsp[(3) - (4)].node)); ;}
    break;

  case 38:
#line 267 "grammar.y"
    { (yyval.node) = new DotAccessorNode((yyvsp[(1) - (3)].node), *(yyvsp[(3) - (3)].ident)); ;}
    break;

  case 39:
#line 268 "grammar.y"
    { (yyval.node) = new NewExprNode((yyvsp[(2) - (3)].node), (yyvsp[(3) - (3)].args)); ;}
    break;

  case 41:
#line 273 "grammar.y"
    { (yyval.node) = new BracketAccessorNode((yyvsp[(1) - (4)].node), (yyvsp[(3) - (4)].node)); ;}
    break;

  case 42:
#line 274 "grammar.y"
    { (yyval.node) = new DotAccessorNode((yyvsp[(1) - (3)].node), *(yyvsp[(3) - (3)].ident)); ;}
    break;

  case 43:
#line 275 "grammar.y"
    { (yyval.node) = new NewExprNode((yyvsp[(2) - (3)].node), (yyvsp[(3) - (3)].args)); ;}
    break;

  case 45:
#line 280 "grammar.y"
    { (yyval.node) = new NewExprNode((yyvsp[(2) - (2)].node)); ;}
    break;

  case 47:
#line 285 "grammar.y"
    { (yyval.node) = new NewExprNode((yyvsp[(2) - (2)].node)); ;}
    break;

  case 48:
#line 289 "grammar.y"
    { (yyval.node) = makeFunctionCallNode((yyvsp[(1) - (2)].node), (yyvsp[(2) - (2)].args)); ;}
    break;

  case 49:
#line 290 "grammar.y"
    { (yyval.node) = makeFunctionCallNode((yyvsp[(1) - (2)].node), (yyvsp[(2) - (2)].args)); ;}
    break;

  case 50:
#line 291 "grammar.y"
    { (yyval.node) = new BracketAccessorNode((yyvsp[(1) - (4)].node), (yyvsp[(3) - (4)].node)); ;}
    break;

  case 51:
#line 292 "grammar.y"
    { (yyval.node) = new DotAccessorNode((yyvsp[(1) - (3)].node), *(yyvsp[(3) - (3)].ident)); ;}
    break;

  case 52:
#line 296 "grammar.y"
    { (yyval.node) = makeFunctionCallNode((yyvsp[(1) - (2)].node), (yyvsp[(2) - (2)].args)); ;}
    break;

  case 53:
#line 297 "grammar.y"
    { (yyval.node) = makeFunctionCallNode((yyvsp[(1) - (2)].node), (yyvsp[(2) - (2)].args)); ;}
    break;

  case 54:
#line 298 "grammar.y"
    { (yyval.node) = new BracketAccessorNode((yyvsp[(1) - (4)].node), (yyvsp[(3) - (4)].node)); ;}
    break;

  case 55:
#line 299 "grammar.y"
    { (yyval.node) = new DotAccessorNode((yyvsp[(1) - (3)].node), *(yyvsp[(3) - (3)].ident)); ;}
    break;

  case 56:
#line 303 "grammar.y"
    { (yyval.args) = new ArgumentsNode(); ;}
    break;

  case 57:
#line 304 "grammar.y"
    { (yyval.args) = new ArgumentsNode((yyvsp[(2) - (3)].alist)); ;}
    break;

  case 58:
#line 308 "grammar.y"
    { (yyval.alist) = new ArgumentListNode((yyvsp[(1) - (1)].node)); ;}
    break;

  case 59:
#line 309 "grammar.y"
    { (yyval.alist) = new ArgumentListNode((yyvsp[(1) - (3)].alist), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 65:
#line 324 "grammar.y"
    { if (!makePostfixNode((yyval.node), (yyvsp[(1) - (2)].node), OpPlusPlus)) YYABORT; ;}
    break;

  case 66:
#line 325 "grammar.y"
    { if (!makePostfixNode((yyval.node), (yyvsp[(1) - (2)].node), OpMinusMinus)) YYABORT; ;}
    break;

  case 68:
#line 330 "grammar.y"
    { if (!makePostfixNode((yyval.node), (yyvsp[(1) - (2)].node), OpPlusPlus)) YYABORT; ;}
    break;

  case 69:
#line 331 "grammar.y"
    { if (!makePostfixNode((yyval.node), (yyvsp[(1) - (2)].node), OpMinusMinus)) YYABORT; ;}
    break;

  case 70:
#line 335 "grammar.y"
    { (yyval.node) = makeDeleteNode((yyvsp[(2) - (2)].node)); ;}
    break;

  case 71:
#line 336 "grammar.y"
    { (yyval.node) = new VoidNode((yyvsp[(2) - (2)].node)); ;}
    break;

  case 72:
#line 337 "grammar.y"
    { (yyval.node) = makeTypeOfNode((yyvsp[(2) - (2)].node)); ;}
    break;

  case 73:
#line 338 "grammar.y"
    { if (!makePrefixNode((yyval.node), (yyvsp[(2) - (2)].node), OpPlusPlus)) YYABORT; ;}
    break;

  case 74:
#line 339 "grammar.y"
    { if (!makePrefixNode((yyval.node), (yyvsp[(2) - (2)].node), OpPlusPlus)) YYABORT; ;}
    break;

  case 75:
#line 340 "grammar.y"
    { if (!makePrefixNode((yyval.node), (yyvsp[(2) - (2)].node), OpMinusMinus)) YYABORT; ;}
    break;

  case 76:
#line 341 "grammar.y"
    { if (!makePrefixNode((yyval.node), (yyvsp[(2) - (2)].node), OpMinusMinus)) YYABORT; ;}
    break;

  case 77:
#line 342 "grammar.y"
    { (yyval.node) = new UnaryPlusNode((yyvsp[(2) - (2)].node)); ;}
    break;

  case 78:
#line 343 "grammar.y"
    { (yyval.node) = new NegateNode((yyvsp[(2) - (2)].node)); ;}
    break;

  case 79:
#line 344 "grammar.y"
    { (yyval.node) = new BitwiseNotNode((yyvsp[(2) - (2)].node)); ;}
    break;

  case 80:
#line 345 "grammar.y"
    { (yyval.node) = new LogicalNotNode((yyvsp[(2) - (2)].node)); ;}
    break;

  case 86:
#line 359 "grammar.y"
    { (yyval.node) = new MultNode((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), '*'); ;}
    break;

  case 87:
#line 360 "grammar.y"
    { (yyval.node) = new MultNode((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), '/'); ;}
    break;

  case 88:
#line 361 "grammar.y"
    { (yyval.node) = new MultNode((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node),'%'); ;}
    break;

  case 90:
#line 367 "grammar.y"
    { (yyval.node) = new MultNode((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), '*'); ;}
    break;

  case 91:
#line 369 "grammar.y"
    { (yyval.node) = new MultNode((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), '/'); ;}
    break;

  case 92:
#line 371 "grammar.y"
    { (yyval.node) = new MultNode((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node),'%'); ;}
    break;

  case 94:
#line 376 "grammar.y"
    { (yyval.node) = new AddNode((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), '+'); ;}
    break;

  case 95:
#line 377 "grammar.y"
    { (yyval.node) = new AddNode((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), '-'); ;}
    break;

  case 97:
#line 383 "grammar.y"
    { (yyval.node) = new AddNode((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), '+'); ;}
    break;

  case 98:
#line 385 "grammar.y"
    { (yyval.node) = new AddNode((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), '-'); ;}
    break;

  case 100:
#line 390 "grammar.y"
    { (yyval.node) = new ShiftNode((yyvsp[(1) - (3)].node), OpLShift, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 101:
#line 391 "grammar.y"
    { (yyval.node) = new ShiftNode((yyvsp[(1) - (3)].node), OpRShift, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 102:
#line 392 "grammar.y"
    { (yyval.node) = new ShiftNode((yyvsp[(1) - (3)].node), OpURShift, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 104:
#line 397 "grammar.y"
    { (yyval.node) = new ShiftNode((yyvsp[(1) - (3)].node), OpLShift, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 105:
#line 398 "grammar.y"
    { (yyval.node) = new ShiftNode((yyvsp[(1) - (3)].node), OpRShift, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 106:
#line 399 "grammar.y"
    { (yyval.node) = new ShiftNode((yyvsp[(1) - (3)].node), OpURShift, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 108:
#line 404 "grammar.y"
    { (yyval.node) = new RelationalNode((yyvsp[(1) - (3)].node), OpLess, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 109:
#line 405 "grammar.y"
    { (yyval.node) = new RelationalNode((yyvsp[(1) - (3)].node), OpGreater, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 110:
#line 406 "grammar.y"
    { (yyval.node) = new RelationalNode((yyvsp[(1) - (3)].node), OpLessEq, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 111:
#line 407 "grammar.y"
    { (yyval.node) = new RelationalNode((yyvsp[(1) - (3)].node), OpGreaterEq, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 112:
#line 408 "grammar.y"
    { (yyval.node) = new RelationalNode((yyvsp[(1) - (3)].node), OpInstanceOf, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 113:
#line 409 "grammar.y"
    { (yyval.node) = new RelationalNode((yyvsp[(1) - (3)].node), OpIn, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 115:
#line 414 "grammar.y"
    { (yyval.node) = new RelationalNode((yyvsp[(1) - (3)].node), OpLess, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 116:
#line 415 "grammar.y"
    { (yyval.node) = new RelationalNode((yyvsp[(1) - (3)].node), OpGreater, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 117:
#line 416 "grammar.y"
    { (yyval.node) = new RelationalNode((yyvsp[(1) - (3)].node), OpLessEq, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 118:
#line 417 "grammar.y"
    { (yyval.node) = new RelationalNode((yyvsp[(1) - (3)].node), OpGreaterEq, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 119:
#line 419 "grammar.y"
    { (yyval.node) = new RelationalNode((yyvsp[(1) - (3)].node), OpInstanceOf, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 121:
#line 424 "grammar.y"
    { (yyval.node) = new RelationalNode((yyvsp[(1) - (3)].node), OpLess, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 122:
#line 425 "grammar.y"
    { (yyval.node) = new RelationalNode((yyvsp[(1) - (3)].node), OpGreater, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 123:
#line 426 "grammar.y"
    { (yyval.node) = new RelationalNode((yyvsp[(1) - (3)].node), OpLessEq, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 124:
#line 427 "grammar.y"
    { (yyval.node) = new RelationalNode((yyvsp[(1) - (3)].node), OpGreaterEq, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 125:
#line 429 "grammar.y"
    { (yyval.node) = new RelationalNode((yyvsp[(1) - (3)].node), OpInstanceOf, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 126:
#line 430 "grammar.y"
    { (yyval.node) = new RelationalNode((yyvsp[(1) - (3)].node), OpIn, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 128:
#line 435 "grammar.y"
    { (yyval.node) = new EqualNode((yyvsp[(1) - (3)].node), OpEqEq, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 129:
#line 436 "grammar.y"
    { (yyval.node) = new EqualNode((yyvsp[(1) - (3)].node), OpNotEq, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 130:
#line 437 "grammar.y"
    { (yyval.node) = new EqualNode((yyvsp[(1) - (3)].node), OpStrEq, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 131:
#line 438 "grammar.y"
    { (yyval.node) = new EqualNode((yyvsp[(1) - (3)].node), OpStrNEq, (yyvsp[(3) - (3)].node));;}
    break;

  case 133:
#line 444 "grammar.y"
    { (yyval.node) = new EqualNode((yyvsp[(1) - (3)].node), OpEqEq, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 134:
#line 446 "grammar.y"
    { (yyval.node) = new EqualNode((yyvsp[(1) - (3)].node), OpNotEq, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 135:
#line 448 "grammar.y"
    { (yyval.node) = new EqualNode((yyvsp[(1) - (3)].node), OpStrEq, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 136:
#line 450 "grammar.y"
    { (yyval.node) = new EqualNode((yyvsp[(1) - (3)].node), OpStrNEq, (yyvsp[(3) - (3)].node));;}
    break;

  case 138:
#line 456 "grammar.y"
    { (yyval.node) = new EqualNode((yyvsp[(1) - (3)].node), OpEqEq, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 139:
#line 457 "grammar.y"
    { (yyval.node) = new EqualNode((yyvsp[(1) - (3)].node), OpNotEq, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 140:
#line 459 "grammar.y"
    { (yyval.node) = new EqualNode((yyvsp[(1) - (3)].node), OpStrEq, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 141:
#line 461 "grammar.y"
    { (yyval.node) = new EqualNode((yyvsp[(1) - (3)].node), OpStrNEq, (yyvsp[(3) - (3)].node));;}
    break;

  case 143:
#line 466 "grammar.y"
    { (yyval.node) = new BitOperNode((yyvsp[(1) - (3)].node), OpBitAnd, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 145:
#line 472 "grammar.y"
    { (yyval.node) = new BitOperNode((yyvsp[(1) - (3)].node), OpBitAnd, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 147:
#line 477 "grammar.y"
    { (yyval.node) = new BitOperNode((yyvsp[(1) - (3)].node), OpBitAnd, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 149:
#line 482 "grammar.y"
    { (yyval.node) = new BitOperNode((yyvsp[(1) - (3)].node), OpBitXOr, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 151:
#line 488 "grammar.y"
    { (yyval.node) = new BitOperNode((yyvsp[(1) - (3)].node), OpBitXOr, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 153:
#line 494 "grammar.y"
    { (yyval.node) = new BitOperNode((yyvsp[(1) - (3)].node), OpBitXOr, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 155:
#line 499 "grammar.y"
    { (yyval.node) = new BitOperNode((yyvsp[(1) - (3)].node), OpBitOr, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 157:
#line 505 "grammar.y"
    { (yyval.node) = new BitOperNode((yyvsp[(1) - (3)].node), OpBitOr, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 159:
#line 511 "grammar.y"
    { (yyval.node) = new BitOperNode((yyvsp[(1) - (3)].node), OpBitOr, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 161:
#line 516 "grammar.y"
    { (yyval.node) = new BinaryLogicalNode((yyvsp[(1) - (3)].node), OpAnd, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 163:
#line 522 "grammar.y"
    { (yyval.node) = new BinaryLogicalNode((yyvsp[(1) - (3)].node), OpAnd, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 165:
#line 528 "grammar.y"
    { (yyval.node) = new BinaryLogicalNode((yyvsp[(1) - (3)].node), OpAnd, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 167:
#line 533 "grammar.y"
    { (yyval.node) = new BinaryLogicalNode((yyvsp[(1) - (3)].node), OpOr, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 169:
#line 539 "grammar.y"
    { (yyval.node) = new BinaryLogicalNode((yyvsp[(1) - (3)].node), OpOr, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 171:
#line 544 "grammar.y"
    { (yyval.node) = new BinaryLogicalNode((yyvsp[(1) - (3)].node), OpOr, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 173:
#line 550 "grammar.y"
    { (yyval.node) = new ConditionalNode((yyvsp[(1) - (5)].node), (yyvsp[(3) - (5)].node), (yyvsp[(5) - (5)].node)); ;}
    break;

  case 175:
#line 556 "grammar.y"
    { (yyval.node) = new ConditionalNode((yyvsp[(1) - (5)].node), (yyvsp[(3) - (5)].node), (yyvsp[(5) - (5)].node)); ;}
    break;

  case 177:
#line 562 "grammar.y"
    { (yyval.node) = new ConditionalNode((yyvsp[(1) - (5)].node), (yyvsp[(3) - (5)].node), (yyvsp[(5) - (5)].node)); ;}
    break;

  case 179:
#line 568 "grammar.y"
    { if (!makeAssignNode((yyval.node), (yyvsp[(1) - (3)].node), (yyvsp[(2) - (3)].op), (yyvsp[(3) - (3)].node))) YYABORT; ;}
    break;

  case 181:
#line 574 "grammar.y"
    { if (!makeAssignNode((yyval.node), (yyvsp[(1) - (3)].node), (yyvsp[(2) - (3)].op), (yyvsp[(3) - (3)].node))) YYABORT; ;}
    break;

  case 183:
#line 580 "grammar.y"
    { if (!makeAssignNode((yyval.node), (yyvsp[(1) - (3)].node), (yyvsp[(2) - (3)].op), (yyvsp[(3) - (3)].node))) YYABORT; ;}
    break;

  case 184:
#line 584 "grammar.y"
    { (yyval.op) = OpEqual; ;}
    break;

  case 185:
#line 585 "grammar.y"
    { (yyval.op) = OpPlusEq; ;}
    break;

  case 186:
#line 586 "grammar.y"
    { (yyval.op) = OpMinusEq; ;}
    break;

  case 187:
#line 587 "grammar.y"
    { (yyval.op) = OpMultEq; ;}
    break;

  case 188:
#line 588 "grammar.y"
    { (yyval.op) = OpDivEq; ;}
    break;

  case 189:
#line 589 "grammar.y"
    { (yyval.op) = OpLShift; ;}
    break;

  case 190:
#line 590 "grammar.y"
    { (yyval.op) = OpRShift; ;}
    break;

  case 191:
#line 591 "grammar.y"
    { (yyval.op) = OpURShift; ;}
    break;

  case 192:
#line 592 "grammar.y"
    { (yyval.op) = OpAndEq; ;}
    break;

  case 193:
#line 593 "grammar.y"
    { (yyval.op) = OpXOrEq; ;}
    break;

  case 194:
#line 594 "grammar.y"
    { (yyval.op) = OpOrEq; ;}
    break;

  case 195:
#line 595 "grammar.y"
    { (yyval.op) = OpModEq; ;}
    break;

  case 197:
#line 600 "grammar.y"
    { (yyval.node) = new CommaNode((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 199:
#line 605 "grammar.y"
    { (yyval.node) = new CommaNode((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 201:
#line 610 "grammar.y"
    { (yyval.node) = new CommaNode((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 218:
#line 633 "grammar.y"
    { (yyval.stat) = new BlockNode(0); DBG((yyval.stat), (yylsp[(2) - (2)]), (yylsp[(2) - (2)])); ;}
    break;

  case 219:
#line 634 "grammar.y"
    { (yyval.stat) = new BlockNode((yyvsp[(2) - (3)].srcs)); DBG((yyval.stat), (yylsp[(3) - (3)]), (yylsp[(3) - (3)])); ;}
    break;

  case 220:
#line 638 "grammar.y"
    { (yyval.stat) = new VarStatementNode((yyvsp[(2) - (3)].vlist)); DBG((yyval.stat), (yylsp[(1) - (3)]), (yylsp[(3) - (3)])); ;}
    break;

  case 221:
#line 639 "grammar.y"
    { (yyval.stat) = new VarStatementNode((yyvsp[(2) - (3)].vlist)); DBG((yyval.stat), (yylsp[(1) - (3)]), (yylsp[(2) - (3)])); AUTO_SEMICOLON; ;}
    break;

  case 222:
#line 643 "grammar.y"
    { (yyval.vlist) = new VarDeclListNode((yyvsp[(1) - (1)].decl)); ;}
    break;

  case 223:
#line 645 "grammar.y"
    { (yyval.vlist) = new VarDeclListNode((yyvsp[(1) - (3)].vlist), (yyvsp[(3) - (3)].decl)); ;}
    break;

  case 224:
#line 649 "grammar.y"
    { (yyval.vlist) = new VarDeclListNode((yyvsp[(1) - (1)].decl)); ;}
    break;

  case 225:
#line 651 "grammar.y"
    { (yyval.vlist) = new VarDeclListNode((yyvsp[(1) - (3)].vlist), (yyvsp[(3) - (3)].decl)); ;}
    break;

  case 226:
#line 655 "grammar.y"
    { (yyval.decl) = new VarDeclNode(*(yyvsp[(1) - (1)].ident), 0, VarDeclNode::Variable); ;}
    break;

  case 227:
#line 656 "grammar.y"
    { (yyval.decl) = new VarDeclNode(*(yyvsp[(1) - (2)].ident), (yyvsp[(2) - (2)].init), VarDeclNode::Variable); ;}
    break;

  case 228:
#line 660 "grammar.y"
    { (yyval.decl) = new VarDeclNode(*(yyvsp[(1) - (1)].ident), 0, VarDeclNode::Variable); ;}
    break;

  case 229:
#line 661 "grammar.y"
    { (yyval.decl) = new VarDeclNode(*(yyvsp[(1) - (2)].ident), (yyvsp[(2) - (2)].init), VarDeclNode::Variable); ;}
    break;

  case 230:
#line 665 "grammar.y"
    { (yyval.stat) = new VarStatementNode((yyvsp[(2) - (3)].vlist)); DBG((yyval.stat), (yylsp[(1) - (3)]), (yylsp[(3) - (3)])); ;}
    break;

  case 231:
#line 666 "grammar.y"
    { (yyval.stat) = new VarStatementNode((yyvsp[(2) - (3)].vlist)); DBG((yyval.stat), (yylsp[(1) - (3)]), (yylsp[(2) - (3)])); AUTO_SEMICOLON; ;}
    break;

  case 232:
#line 670 "grammar.y"
    { (yyval.vlist) = new VarDeclListNode((yyvsp[(1) - (1)].decl)); ;}
    break;

  case 233:
#line 672 "grammar.y"
    { (yyval.vlist) = new VarDeclListNode((yyvsp[(1) - (3)].vlist), (yyvsp[(3) - (3)].decl)); ;}
    break;

  case 234:
#line 676 "grammar.y"
    { (yyval.decl) = new VarDeclNode(*(yyvsp[(1) - (1)].ident), 0, VarDeclNode::Constant); ;}
    break;

  case 235:
#line 677 "grammar.y"
    { (yyval.decl) = new VarDeclNode(*(yyvsp[(1) - (2)].ident), (yyvsp[(2) - (2)].init), VarDeclNode::Constant); ;}
    break;

  case 236:
#line 681 "grammar.y"
    { (yyval.init) = new AssignExprNode((yyvsp[(2) - (2)].node)); ;}
    break;

  case 237:
#line 685 "grammar.y"
    { (yyval.init) = new AssignExprNode((yyvsp[(2) - (2)].node)); ;}
    break;

  case 238:
#line 689 "grammar.y"
    { (yyval.stat) = new EmptyStatementNode(); ;}
    break;

  case 239:
#line 693 "grammar.y"
    { (yyval.stat) = new ExprStatementNode((yyvsp[(1) - (2)].node)); DBG((yyval.stat), (yylsp[(1) - (2)]), (yylsp[(2) - (2)])); ;}
    break;

  case 240:
#line 694 "grammar.y"
    { (yyval.stat) = new ExprStatementNode((yyvsp[(1) - (2)].node)); DBG((yyval.stat), (yylsp[(1) - (2)]), (yylsp[(1) - (2)])); AUTO_SEMICOLON; ;}
    break;

  case 241:
#line 699 "grammar.y"
    { (yyval.stat) = new IfNode((yyvsp[(3) - (5)].node), (yyvsp[(5) - (5)].stat), 0); DBG((yyval.stat), (yylsp[(1) - (5)]), (yylsp[(4) - (5)])); ;}
    break;

  case 242:
#line 701 "grammar.y"
    { (yyval.stat) = new IfNode((yyvsp[(3) - (7)].node), (yyvsp[(5) - (7)].stat), (yyvsp[(7) - (7)].stat)); DBG((yyval.stat), (yylsp[(1) - (7)]), (yylsp[(4) - (7)])); ;}
    break;

  case 243:
#line 705 "grammar.y"
    { (yyval.stat) = new DoWhileNode((yyvsp[(2) - (6)].stat), (yyvsp[(5) - (6)].node)); DBG((yyval.stat), (yylsp[(1) - (6)]), (yylsp[(3) - (6)]));;}
    break;

  case 244:
#line 706 "grammar.y"
    { (yyval.stat) = new WhileNode((yyvsp[(3) - (5)].node), (yyvsp[(5) - (5)].stat)); DBG((yyval.stat), (yylsp[(1) - (5)]), (yylsp[(4) - (5)])); ;}
    break;

  case 245:
#line 708 "grammar.y"
    { (yyval.stat) = new ForNode((yyvsp[(3) - (9)].node), (yyvsp[(5) - (9)].node), (yyvsp[(7) - (9)].node), (yyvsp[(9) - (9)].stat)); DBG((yyval.stat), (yylsp[(1) - (9)]), (yylsp[(8) - (9)])); ;}
    break;

  case 246:
#line 710 "grammar.y"
    { (yyval.stat) = new ForNode((yyvsp[(4) - (10)].vlist), (yyvsp[(6) - (10)].node), (yyvsp[(8) - (10)].node), (yyvsp[(10) - (10)].stat)); DBG((yyval.stat), (yylsp[(1) - (10)]), (yylsp[(9) - (10)])); ;}
    break;

  case 247:
#line 712 "grammar.y"
    {
                                            Node *n = (yyvsp[(3) - (7)].node)->nodeInsideAllParens();
                                            if (!n->isLocation())
                                                YYABORT;
                                            (yyval.stat) = new ForInNode(n, (yyvsp[(5) - (7)].node), (yyvsp[(7) - (7)].stat));
                                            DBG((yyval.stat), (yylsp[(1) - (7)]), (yylsp[(6) - (7)]));
                                        ;}
    break;

  case 248:
#line 720 "grammar.y"
    { (yyval.stat) = new ForInNode(*(yyvsp[(4) - (8)].ident), 0, (yyvsp[(6) - (8)].node), (yyvsp[(8) - (8)].stat)); DBG((yyval.stat), (yylsp[(1) - (8)]), (yylsp[(7) - (8)])); ;}
    break;

  case 249:
#line 722 "grammar.y"
    { (yyval.stat) = new ForInNode(*(yyvsp[(4) - (9)].ident), (yyvsp[(5) - (9)].init), (yyvsp[(7) - (9)].node), (yyvsp[(9) - (9)].stat)); DBG((yyval.stat), (yylsp[(1) - (9)]), (yylsp[(8) - (9)])); ;}
    break;

  case 250:
#line 726 "grammar.y"
    { (yyval.node) = 0; ;}
    break;

  case 252:
#line 731 "grammar.y"
    { (yyval.node) = 0; ;}
    break;

  case 254:
#line 736 "grammar.y"
    { (yyval.stat) = new ContinueNode(); DBG((yyval.stat), (yylsp[(1) - (2)]), (yylsp[(2) - (2)])); ;}
    break;

  case 255:
#line 737 "grammar.y"
    { (yyval.stat) = new ContinueNode(); DBG((yyval.stat), (yylsp[(1) - (2)]), (yylsp[(1) - (2)])); AUTO_SEMICOLON; ;}
    break;

  case 256:
#line 738 "grammar.y"
    { (yyval.stat) = new ContinueNode(*(yyvsp[(2) - (3)].ident)); DBG((yyval.stat), (yylsp[(1) - (3)]), (yylsp[(3) - (3)])); ;}
    break;

  case 257:
#line 739 "grammar.y"
    { (yyval.stat) = new ContinueNode(*(yyvsp[(2) - (3)].ident)); DBG((yyval.stat), (yylsp[(1) - (3)]), (yylsp[(2) - (3)])); AUTO_SEMICOLON; ;}
    break;

  case 258:
#line 743 "grammar.y"
    { (yyval.stat) = new BreakNode(); DBG((yyval.stat), (yylsp[(1) - (2)]), (yylsp[(2) - (2)])); ;}
    break;

  case 259:
#line 744 "grammar.y"
    { (yyval.stat) = new BreakNode(); DBG((yyval.stat), (yylsp[(1) - (2)]), (yylsp[(1) - (2)])); AUTO_SEMICOLON; ;}
    break;

  case 260:
#line 745 "grammar.y"
    { (yyval.stat) = new BreakNode(*(yyvsp[(2) - (3)].ident)); DBG((yyval.stat), (yylsp[(1) - (3)]), (yylsp[(3) - (3)])); ;}
    break;

  case 261:
#line 746 "grammar.y"
    { (yyval.stat) = new BreakNode(*(yyvsp[(2) - (3)].ident)); DBG((yyval.stat), (yylsp[(1) - (3)]), (yylsp[(2) - (3)])); AUTO_SEMICOLON; ;}
    break;

  case 262:
#line 750 "grammar.y"
    { (yyval.stat) = new ReturnNode(0); DBG((yyval.stat), (yylsp[(1) - (2)]), (yylsp[(2) - (2)])); ;}
    break;

  case 263:
#line 751 "grammar.y"
    { (yyval.stat) = new ReturnNode(0); DBG((yyval.stat), (yylsp[(1) - (2)]), (yylsp[(1) - (2)])); AUTO_SEMICOLON; ;}
    break;

  case 264:
#line 752 "grammar.y"
    { (yyval.stat) = new ReturnNode((yyvsp[(2) - (3)].node)); DBG((yyval.stat), (yylsp[(1) - (3)]), (yylsp[(3) - (3)])); ;}
    break;

  case 265:
#line 753 "grammar.y"
    { (yyval.stat) = new ReturnNode((yyvsp[(2) - (3)].node)); DBG((yyval.stat), (yylsp[(1) - (3)]), (yylsp[(2) - (3)])); AUTO_SEMICOLON; ;}
    break;

  case 266:
#line 757 "grammar.y"
    { (yyval.stat) = new WithNode((yyvsp[(3) - (5)].node), (yyvsp[(5) - (5)].stat)); DBG((yyval.stat), (yylsp[(1) - (5)]), (yylsp[(4) - (5)])); ;}
    break;

  case 267:
#line 761 "grammar.y"
    { (yyval.stat) = new SwitchNode((yyvsp[(3) - (5)].node), (yyvsp[(5) - (5)].cblk)); DBG((yyval.stat), (yylsp[(1) - (5)]), (yylsp[(4) - (5)])); ;}
    break;

  case 268:
#line 765 "grammar.y"
    { (yyval.cblk) = new CaseBlockNode((yyvsp[(2) - (3)].clist), 0, 0); ;}
    break;

  case 269:
#line 767 "grammar.y"
    { (yyval.cblk) = new CaseBlockNode((yyvsp[(2) - (5)].clist), (yyvsp[(3) - (5)].ccl), (yyvsp[(4) - (5)].clist)); ;}
    break;

  case 270:
#line 771 "grammar.y"
    { (yyval.clist) = 0; ;}
    break;

  case 272:
#line 776 "grammar.y"
    { (yyval.clist) = new ClauseListNode((yyvsp[(1) - (1)].ccl)); ;}
    break;

  case 273:
#line 777 "grammar.y"
    { (yyval.clist) = new ClauseListNode((yyvsp[(1) - (2)].clist), (yyvsp[(2) - (2)].ccl)); ;}
    break;

  case 274:
#line 781 "grammar.y"
    { (yyval.ccl) = new CaseClauseNode((yyvsp[(2) - (3)].node)); ;}
    break;

  case 275:
#line 782 "grammar.y"
    { (yyval.ccl) = new CaseClauseNode((yyvsp[(2) - (4)].node), (yyvsp[(4) - (4)].srcs)); ;}
    break;

  case 276:
#line 786 "grammar.y"
    { (yyval.ccl) = new CaseClauseNode(0); ;}
    break;

  case 277:
#line 787 "grammar.y"
    { (yyval.ccl) = new CaseClauseNode(0, (yyvsp[(3) - (3)].srcs)); ;}
    break;

  case 278:
#line 791 "grammar.y"
    { (yyvsp[(3) - (3)].stat)->pushLabel(*(yyvsp[(1) - (3)].ident)); (yyval.stat) = new LabelNode(*(yyvsp[(1) - (3)].ident), (yyvsp[(3) - (3)].stat)); ;}
    break;

  case 279:
#line 795 "grammar.y"
    { (yyval.stat) = new ThrowNode((yyvsp[(2) - (3)].node)); DBG((yyval.stat), (yylsp[(1) - (3)]), (yylsp[(3) - (3)])); ;}
    break;

  case 280:
#line 796 "grammar.y"
    { (yyval.stat) = new ThrowNode((yyvsp[(2) - (3)].node)); DBG((yyval.stat), (yylsp[(1) - (3)]), (yylsp[(2) - (3)])); AUTO_SEMICOLON; ;}
    break;

  case 281:
#line 800 "grammar.y"
    { (yyval.stat) = new TryNode((yyvsp[(2) - (4)].stat), Identifier::null(), 0, (yyvsp[(4) - (4)].stat)); DBG((yyval.stat), (yylsp[(1) - (4)]), (yylsp[(2) - (4)])); ;}
    break;

  case 282:
#line 801 "grammar.y"
    { (yyval.stat) = new TryNode((yyvsp[(2) - (7)].stat), *(yyvsp[(5) - (7)].ident), (yyvsp[(7) - (7)].stat), 0); DBG((yyval.stat), (yylsp[(1) - (7)]), (yylsp[(2) - (7)])); ;}
    break;

  case 283:
#line 803 "grammar.y"
    { (yyval.stat) = new TryNode((yyvsp[(2) - (9)].stat), *(yyvsp[(5) - (9)].ident), (yyvsp[(7) - (9)].stat), (yyvsp[(9) - (9)].stat)); DBG((yyval.stat), (yylsp[(1) - (9)]), (yylsp[(2) - (9)])); ;}
    break;

  case 284:
#line 807 "grammar.y"
    { (yyval.stat) = new EmptyStatementNode(); DBG((yyval.stat), (yylsp[(1) - (2)]), (yylsp[(2) - (2)])); ;}
    break;

  case 285:
#line 808 "grammar.y"
    { (yyval.stat) = new EmptyStatementNode(); DBG((yyval.stat), (yylsp[(1) - (2)]), (yylsp[(1) - (2)])); AUTO_SEMICOLON; ;}
    break;

  case 286:
#line 812 "grammar.y"
    { (yyval.func) = new FuncDeclNode(*(yyvsp[(2) - (5)].ident), (yyvsp[(5) - (5)].body)); ;}
    break;

  case 287:
#line 814 "grammar.y"
    { (yyval.func) = new FuncDeclNode(*(yyvsp[(2) - (6)].ident), (yyvsp[(4) - (6)].param), (yyvsp[(6) - (6)].body)); ;}
    break;

  case 288:
#line 818 "grammar.y"
    { (yyval.funcExpr) = new FuncExprNode(Identifier::null(), (yyvsp[(4) - (4)].body)); ;}
    break;

  case 289:
#line 820 "grammar.y"
    { (yyval.funcExpr) = new FuncExprNode(Identifier::null(), (yyvsp[(5) - (5)].body), (yyvsp[(3) - (5)].param)); ;}
    break;

  case 290:
#line 821 "grammar.y"
    { (yyval.funcExpr) = new FuncExprNode(*(yyvsp[(2) - (5)].ident), (yyvsp[(5) - (5)].body)); ;}
    break;

  case 291:
#line 823 "grammar.y"
    { (yyval.funcExpr) = new FuncExprNode(*(yyvsp[(2) - (6)].ident), (yyvsp[(6) - (6)].body), (yyvsp[(4) - (6)].param)); ;}
    break;

  case 292:
#line 827 "grammar.y"
    { (yyval.param) = new ParameterNode(*(yyvsp[(1) - (1)].ident)); ;}
    break;

  case 293:
#line 828 "grammar.y"
    { (yyval.param) = new ParameterNode((yyvsp[(1) - (3)].param), *(yyvsp[(3) - (3)].ident)); ;}
    break;

  case 294:
#line 832 "grammar.y"
    { (yyval.body) = new FunctionBodyNode(0); DBG((yyval.body), (yylsp[(1) - (2)]), (yylsp[(2) - (2)])); ;}
    break;

  case 295:
#line 833 "grammar.y"
    { (yyval.body) = new FunctionBodyNode((yyvsp[(2) - (3)].srcs)); DBG((yyval.body), (yylsp[(1) - (3)]), (yylsp[(3) - (3)])); ;}
    break;

  case 296:
#line 837 "grammar.y"
    { Parser::accept(new ProgramNode(0)); ;}
    break;

  case 297:
#line 838 "grammar.y"
    { Parser::accept(new ProgramNode((yyvsp[(1) - (1)].srcs))); ;}
    break;

  case 298:
#line 842 "grammar.y"
    { (yyval.srcs) = new SourceElementsNode((yyvsp[(1) - (1)].stat)); ;}
    break;

  case 299:
#line 843 "grammar.y"
    { (yyval.srcs) = new SourceElementsNode((yyvsp[(1) - (2)].srcs), (yyvsp[(2) - (2)].stat)); ;}
    break;

  case 300:
#line 847 "grammar.y"
    { (yyval.stat) = (yyvsp[(1) - (1)].func); ;}
    break;

  case 301:
#line 848 "grammar.y"
    { (yyval.stat) = (yyvsp[(1) - (1)].stat); ;}
    break;


/* Line 1267 of yacc.c.  */
#line 3441 "grammar.tab.c"
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
  return yyresult;
}


#line 851 "grammar.y"


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

