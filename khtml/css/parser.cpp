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
#define YYPURE 1

/* Using locations.  */
#define YYLSP_NEEDED 0

/* Substitute the variable and function names.  */
#define yyparse cssyyparse
#define yylex   cssyylex
#define yyerror cssyyerror
#define yylval  cssyylval
#define yychar  cssyychar
#define yydebug cssyydebug
#define yynerrs cssyynerrs


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     UNIMPORTANT_TOK = 258,
     S = 259,
     SGML_CD = 260,
     INCLUDES = 261,
     DASHMATCH = 262,
     BEGINSWITH = 263,
     ENDSWITH = 264,
     CONTAINS = 265,
     STRING = 266,
     IDENT = 267,
     NTH = 268,
     HASH = 269,
     IMPORT_SYM = 270,
     PAGE_SYM = 271,
     MEDIA_SYM = 272,
     FONT_FACE_SYM = 273,
     CHARSET_SYM = 274,
     NAMESPACE_SYM = 275,
     KHTML_RULE_SYM = 276,
     KHTML_DECLS_SYM = 277,
     KHTML_VALUE_SYM = 278,
     IMPORTANT_SYM = 279,
     QEMS = 280,
     EMS = 281,
     EXS = 282,
     PXS = 283,
     CMS = 284,
     MMS = 285,
     INS = 286,
     PTS = 287,
     PCS = 288,
     DEGS = 289,
     RADS = 290,
     GRADS = 291,
     MSECS = 292,
     SECS = 293,
     HERZ = 294,
     KHERZ = 295,
     DIMEN = 296,
     PERCENTAGE = 297,
     FLOAT = 298,
     INTEGER = 299,
     URI = 300,
     FUNCTION = 301,
     NOTFUNCTION = 302,
     UNICODERANGE = 303
   };
#endif
/* Tokens.  */
#define UNIMPORTANT_TOK 258
#define S 259
#define SGML_CD 260
#define INCLUDES 261
#define DASHMATCH 262
#define BEGINSWITH 263
#define ENDSWITH 264
#define CONTAINS 265
#define STRING 266
#define IDENT 267
#define NTH 268
#define HASH 269
#define IMPORT_SYM 270
#define PAGE_SYM 271
#define MEDIA_SYM 272
#define FONT_FACE_SYM 273
#define CHARSET_SYM 274
#define NAMESPACE_SYM 275
#define KHTML_RULE_SYM 276
#define KHTML_DECLS_SYM 277
#define KHTML_VALUE_SYM 278
#define IMPORTANT_SYM 279
#define QEMS 280
#define EMS 281
#define EXS 282
#define PXS 283
#define CMS 284
#define MMS 285
#define INS 286
#define PTS 287
#define PCS 288
#define DEGS 289
#define RADS 290
#define GRADS 291
#define MSECS 292
#define SECS 293
#define HERZ 294
#define KHERZ 295
#define DIMEN 296
#define PERCENTAGE 297
#define FLOAT 298
#define INTEGER 299
#define URI 300
#define FUNCTION 301
#define NOTFUNCTION 302
#define UNICODERANGE 303




/* Copy the first part of user declarations.  */



/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2002-2003 Lars Knoll (knoll@kde.org)
 *  Copyright (c) 2003 Apple Computer
 *  Copyright (C) 2003 Dirk Mueller (mueller@kde.org)
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include <config.h>
#include <string.h>
#include <stdlib.h>

#include <dom/dom_string.h>
#include <xml/dom_docimpl.h>
#include <css/cssstyleselector.h>
#include <css/css_ruleimpl.h>
#include <css/css_stylesheetimpl.h>
#include <css/css_valueimpl.h>
#include <misc/htmlhashes.h>
#include "cssparser.h"

#include <assert.h>
#include <kdebug.h>

//#define CSS_DEBUG

using namespace DOM;

//
// The following file defines the function
//     const struct props *findProp(const char *word, int len)
//
// with 'props->id' a CSS property in the range from CSS_PROP_MIN to
// (and including) CSS_PROP_TOTAL-1

// turn off inlining to void warning with newer gcc
#undef __inline
#define __inline

#include "cssproperties.c"
#include "cssvalues.c"

#undef __inline

int DOM::getPropertyID(const char *tagStr, int len)
{
    { // HTML CSS Properties
        const struct css_prop *prop = findProp(tagStr, len);
        if (!prop)
            return 0;

        return prop->id;
    }
}

int DOM::getValueID(const char *tagStr, int len)
{
    { // HTML CSS Values
        const struct css_value *val = findValue(tagStr, len);
        if (!val)
            return 0;

        return val->id;
    }
}

#define YYDEBUG 0
#undef YYMAXDEPTH
#define YYPARSE_PARAM parser


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

{
    CSSRuleImpl *rule;
    CSSSelector *selector;
    QList<CSSSelector*> *selectorList;
    bool ok;
    MediaListImpl *mediaList;
    CSSMediaRuleImpl *mediaRule;
    CSSRuleListImpl *ruleList;
    ParseString string;
    float val;
    int prop_id;
    unsigned int attribute;
    unsigned int element;
    unsigned int ns;
    CSSSelector::Relation relation;
    CSSSelector::Match match;
    bool b;
    char tok;
    Value value;
    ValueList *valueList;
}
/* Line 187 of yacc.c.  */

	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */



static inline int cssyyerror(const char *x )
{
#ifdef CSS_DEBUG
    qDebug( "%s", x );
#else
    Q_UNUSED( x );
#endif
    return 1;
}

static int cssyylex( YYSTYPE *yylval ) {
    return CSSParser::current()->lex( yylval );
}

#define null 1



/* Line 216 of yacc.c.  */


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
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

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
#define YYFINAL  17
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   477

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  67
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  56
/* YYNRULES -- Number of rules.  */
#define YYNRULES  165
/* YYNRULES -- Number of states.  */
#define YYNSTATES  308

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   303

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    64,    18,    58,    57,    61,    16,    65,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    15,    56,
       2,    63,    60,     2,    66,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    17,     2,    62,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    54,    19,    55,    59,     2,     2,     2,
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
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     9,    12,    15,    18,    20,    22,    29,
      32,    38,    39,    42,    43,    46,    49,    50,    52,    58,
      62,    66,    67,    71,    78,    82,    86,    87,    91,    98,
     102,   106,   107,   110,   111,   115,   117,   119,   121,   123,
     125,   127,   130,   132,   134,   135,   137,   139,   144,   147,
     155,   156,   160,   163,   167,   171,   175,   179,   182,   185,
     188,   189,   191,   193,   196,   198,   203,   206,   208,   212,
     215,   217,   220,   223,   226,   230,   233,   237,   242,   246,
     248,   250,   252,   255,   258,   260,   262,   264,   266,   269,
     272,   277,   286,   292,   302,   304,   306,   308,   310,   312,
     314,   316,   318,   321,   325,   330,   335,   340,   345,   351,
     356,   361,   366,   372,   378,   382,   386,   391,   396,   402,
     405,   408,   411,   412,   414,   418,   421,   424,   425,   427,
     430,   433,   436,   439,   442,   445,   447,   449,   452,   455,
     458,   461,   464,   467,   470,   473,   476,   479,   482,   485,
     488,   491,   494,   497,   500,   503,   506,   512,   516,   519,
     523,   527,   530,   536,   540,   542
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      68,     0,    -1,    75,    74,    77,    79,    82,    -1,    70,
      73,    -1,    71,    73,    -1,    72,    73,    -1,    94,    -1,
      78,    -1,    26,    54,    73,    69,    73,    55,    -1,    27,
     108,    -1,    28,    54,    73,   113,    55,    -1,    -1,    73,
       4,    -1,    -1,    74,     5,    -1,    74,     4,    -1,    -1,
      76,    -1,    24,    73,    11,    73,    56,    -1,    24,     1,
     121,    -1,    24,     1,    56,    -1,    -1,    77,    78,    74,
      -1,    20,    73,    84,    73,    85,    56,    -1,    20,     1,
     121,    -1,    20,     1,    56,    -1,    -1,    79,    80,    74,
      -1,    25,    73,    81,    84,    73,    56,    -1,    25,     1,
     121,    -1,    25,     1,    56,    -1,    -1,    12,     4,    -1,
      -1,    82,    83,    74,    -1,    94,    -1,    87,    -1,    90,
      -1,    91,    -1,   120,    -1,   119,    -1,    78,     1,    -1,
      11,    -1,    50,    -1,    -1,    86,    -1,    89,    -1,    86,
      57,    73,    89,    -1,    86,     1,    -1,    22,    73,    86,
      54,    73,    88,    55,    -1,    -1,    88,    94,    73,    -1,
      12,    73,    -1,    21,     1,   121,    -1,    21,     1,    56,
      -1,    23,     1,   121,    -1,    23,     1,    56,    -1,    58,
      73,    -1,    59,    73,    -1,    60,    73,    -1,    -1,    61,
      -1,    58,    -1,    95,   108,    -1,    96,    -1,    95,    57,
      73,    96,    -1,    95,     1,    -1,    98,    -1,    96,    92,
      98,    -1,    96,     1,    -1,    19,    -1,    18,    19,    -1,
      12,    19,    -1,    99,    73,    -1,    99,   100,    73,    -1,
     100,    73,    -1,    97,    99,    73,    -1,    97,    99,   100,
      73,    -1,    97,   100,    73,    -1,    12,    -1,    18,    -1,
     101,    -1,   100,   101,    -1,   100,     1,    -1,    14,    -1,
     102,    -1,   104,    -1,   107,    -1,    16,    12,    -1,    12,
      73,    -1,    17,    73,   103,    62,    -1,    17,    73,   103,
     105,    73,   106,    73,    62,    -1,    17,    73,    97,   103,
      62,    -1,    17,    73,    97,   103,   105,    73,   106,    73,
      62,    -1,    63,    -1,     6,    -1,     7,    -1,     8,    -1,
       9,    -1,    10,    -1,    12,    -1,    11,    -1,    15,    12,
      -1,    15,    15,    12,    -1,    15,    51,    13,    64,    -1,
      15,    51,    49,    64,    -1,    15,    51,    12,    64,    -1,
      15,    51,    11,    64,    -1,    15,    52,    73,    98,    64,
      -1,    54,    73,   110,    55,    -1,    54,    73,     1,    55,
      -1,    54,    73,   109,    55,    -1,    54,    73,   109,   110,
      55,    -1,    54,    73,   109,     1,    55,    -1,   110,    56,
      73,    -1,     1,    56,    73,    -1,   109,   110,    56,    73,
      -1,   109,     1,    56,    73,    -1,   111,    15,    73,   113,
     112,    -1,     1,   121,    -1,    12,    73,    -1,    29,    73,
      -1,    -1,   115,    -1,   113,   114,   115,    -1,    65,    73,
      -1,    57,    73,    -1,    -1,   116,    -1,    93,   116,    -1,
      46,    73,    -1,    11,    73,    -1,    12,    73,    -1,    50,
      73,    -1,    53,    73,    -1,   118,    -1,   117,    -1,    49,
      73,    -1,    48,    73,    -1,    47,    73,    -1,    33,    73,
      -1,    34,    73,    -1,    35,    73,    -1,    36,    73,    -1,
      37,    73,    -1,    38,    73,    -1,    39,    73,    -1,    40,
      73,    -1,    41,    73,    -1,    42,    73,    -1,    43,    73,
      -1,    44,    73,    -1,    45,    73,    -1,    31,    73,    -1,
      30,    73,    -1,    32,    73,    -1,    51,    73,   113,    64,
      73,    -1,    51,    73,     1,    -1,    14,    73,    -1,    66,
       1,   121,    -1,    66,     1,    56,    -1,     1,   121,    -1,
      54,     1,   122,     1,    55,    -1,    54,     1,    55,    -1,
     121,    -1,   122,     1,   121,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   263,   263,   264,   265,   266,   270,   271,   275,   282,
     288,   313,   314,   317,   319,   320,   323,   325,   328,   339,
     342,   347,   349,   360,   370,   373,   379,   380,   384,   392,
     393,   397,   398,   401,   403,   414,   415,   416,   417,   418,
     419,   420,   424,   425,   429,   432,   437,   441,   446,   453,
     467,   468,   478,   500,   503,   509,   512,   518,   519,   520,
     521,   525,   526,   530,   552,   565,   583,   592,   595,   609,
     616,   617,   618,   622,   626,   631,   636,   643,   652,   664,
     680,   686,   689,   699,   706,   712,   713,   714,   718,   727,
     751,   756,   762,   770,   782,   785,   788,   791,   794,   797,
     803,   804,   808,   814,   820,   827,   834,   841,   848,   857,
     860,   863,   866,   871,   877,   881,   884,   889,   895,   917,
     923,   930,   931,   935,   939,   955,   958,   961,   967,   968,
     970,   971,   972,   978,   979,   980,   982,   988,   989,   990,
     991,   992,   993,   994,   995,   996,   997,   998,   999,  1000,
    1001,  1002,  1003,  1004,  1005,  1006,  1011,  1019,  1035,  1042,
    1048,  1057,  1083,  1084,  1088,  1089
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "UNIMPORTANT_TOK", "S", "SGML_CD",
  "INCLUDES", "DASHMATCH", "BEGINSWITH", "ENDSWITH", "CONTAINS", "STRING",
  "IDENT", "NTH", "HASH", "':'", "'.'", "'['", "'*'", "'|'", "IMPORT_SYM",
  "PAGE_SYM", "MEDIA_SYM", "FONT_FACE_SYM", "CHARSET_SYM", "NAMESPACE_SYM",
  "KHTML_RULE_SYM", "KHTML_DECLS_SYM", "KHTML_VALUE_SYM", "IMPORTANT_SYM",
  "QEMS", "EMS", "EXS", "PXS", "CMS", "MMS", "INS", "PTS", "PCS", "DEGS",
  "RADS", "GRADS", "MSECS", "SECS", "HERZ", "KHERZ", "DIMEN", "PERCENTAGE",
  "FLOAT", "INTEGER", "URI", "FUNCTION", "NOTFUNCTION", "UNICODERANGE",
  "'{'", "'}'", "';'", "','", "'+'", "'~'", "'>'", "'-'", "']'", "'='",
  "')'", "'/'", "'@'", "$accept", "stylesheet", "ruleset_or_import",
  "khtml_rule", "khtml_decls", "khtml_value", "maybe_space", "maybe_sgml",
  "maybe_charset", "charset", "import_list", "import", "namespace_list",
  "namespace", "maybe_ns_prefix", "rule_list", "rule", "string_or_uri",
  "maybe_media_list", "media_list", "media", "ruleset_list", "medium",
  "page", "font_face", "combinator", "unary_operator", "ruleset",
  "selector_list", "selector", "namespace_selector", "simple_selector",
  "element_name", "specifier_list", "specifier", "class", "attrib_id",
  "attrib", "match", "ident_or_string", "pseudo", "declaration_block",
  "declaration_list", "declaration", "property", "prio", "expr",
  "operator", "term", "unary_term", "function", "hexcolor", "invalid_at",
  "invalid_rule", "invalid_block", "invalid_block_list", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,    58,    46,    91,    42,   124,
     270,   271,   272,   273,   274,   275,   276,   277,   278,   279,
     280,   281,   282,   283,   284,   285,   286,   287,   288,   289,
     290,   291,   292,   293,   294,   295,   296,   297,   298,   299,
     300,   301,   302,   303,   123,   125,    59,    44,    43,   126,
      62,    45,    93,    61,    41,    47,    64
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    67,    68,    68,    68,    68,    69,    69,    70,    71,
      72,    73,    73,    74,    74,    74,    75,    75,    76,    76,
      76,    77,    77,    78,    78,    78,    79,    79,    80,    80,
      80,    81,    81,    82,    82,    83,    83,    83,    83,    83,
      83,    83,    84,    84,    85,    85,    86,    86,    86,    87,
      88,    88,    89,    90,    90,    91,    91,    92,    92,    92,
      92,    93,    93,    94,    95,    95,    95,    96,    96,    96,
      97,    97,    97,    98,    98,    98,    98,    98,    98,    99,
      99,   100,   100,   100,   101,   101,   101,   101,   102,   103,
     104,   104,   104,   104,   105,   105,   105,   105,   105,   105,
     106,   106,   107,   107,   107,   107,   107,   107,   107,   108,
     108,   108,   108,   108,   109,   109,   109,   109,   110,   110,
     111,   112,   112,   113,   113,   114,   114,   114,   115,   115,
     115,   115,   115,   115,   115,   115,   115,   116,   116,   116,
     116,   116,   116,   116,   116,   116,   116,   116,   116,   116,
     116,   116,   116,   116,   116,   116,   117,   117,   118,   119,
     119,   120,   121,   121,   122,   122
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     5,     2,     2,     2,     1,     1,     6,     2,
       5,     0,     2,     0,     2,     2,     0,     1,     5,     3,
       3,     0,     3,     6,     3,     3,     0,     3,     6,     3,
       3,     0,     2,     0,     3,     1,     1,     1,     1,     1,
       1,     2,     1,     1,     0,     1,     1,     4,     2,     7,
       0,     3,     2,     3,     3,     3,     3,     2,     2,     2,
       0,     1,     1,     2,     1,     4,     2,     1,     3,     2,
       1,     2,     2,     2,     3,     2,     3,     4,     3,     1,
       1,     1,     2,     2,     1,     1,     1,     1,     2,     2,
       4,     8,     5,     9,     1,     1,     1,     1,     1,     1,
       1,     1,     2,     3,     4,     4,     4,     4,     5,     4,
       4,     4,     5,     5,     3,     3,     4,     4,     5,     2,
       2,     2,     0,     1,     3,     2,     2,     0,     1,     2,
       2,     2,     2,     2,     2,     1,     1,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     5,     3,     2,     3,
       3,     2,     5,     3,     1,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
      16,     0,     0,     0,     0,     0,    11,    11,    11,    13,
      17,     0,     0,    11,    11,     9,    11,     1,     3,     4,
       5,    21,     0,    20,    19,    12,    11,     0,     0,     0,
      15,    14,    26,     0,     0,    79,    84,     0,     0,    11,
      80,    70,     0,    11,     7,     6,     0,     0,     0,    67,
      11,     0,    81,    85,    86,    87,     0,    11,     0,     0,
       0,    11,    11,    11,    11,    11,    11,    11,    11,    11,
      11,    11,    11,    11,    11,    11,    11,    11,    11,    11,
      11,    11,    11,    11,    11,    11,    11,    62,    61,     0,
     127,   123,   128,   136,   135,    13,    33,   163,   164,     0,
      18,    72,   102,     0,     0,    11,    88,     0,    71,     0,
       0,     0,    66,    11,    63,    69,    11,    11,    11,     0,
      79,    80,    11,     0,    73,     0,    83,    75,    82,   110,
      11,   119,   120,     0,   111,     0,   109,    11,    11,   131,
     132,   158,   154,   153,   155,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   130,   139,
     138,   137,   133,     0,   134,   129,    10,    11,    11,     0,
      22,     0,    13,     0,     0,   103,     0,     0,     0,     0,
       0,    11,     0,     0,     0,    25,    24,    42,    43,    11,
       8,     0,    57,    58,    59,    68,    76,     0,    78,    74,
     115,   113,    11,   112,    11,   114,     0,   157,   127,   126,
     125,   124,     0,    31,    27,     0,     0,    11,     0,     0,
       0,    13,    36,    37,    38,    35,    40,    39,   162,   165,
     107,   106,   104,   105,     0,    89,    11,     0,    95,    96,
      97,    98,    99,    90,    94,    11,    44,     0,    77,   117,
     116,   127,    11,    30,    29,     0,     0,   161,     0,     0,
       0,     0,    41,    34,   108,    92,    11,     0,    11,     0,
       0,    46,    11,   118,   156,    32,    11,    54,    53,     0,
      56,    55,   160,   159,     0,   101,   100,    11,    52,    23,
      48,    11,   121,     0,    11,    11,     0,     0,    28,    50,
       0,    91,    47,     0,    93,    49,    11,    51
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     5,    43,     6,     7,     8,   235,    21,     9,    10,
      32,    44,    96,   172,   256,   173,   221,   189,   269,   270,
     222,   303,   271,   223,   224,   119,    89,    45,    46,    47,
      48,    49,    50,    51,    52,    53,   184,    54,   245,   287,
      55,    15,    58,    59,    60,   273,    90,   169,    91,    92,
      93,    94,   226,   227,   131,    99
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -172
static const yytype_int16 yypact[] =
{
     235,    18,   -44,   -27,    59,    23,  -172,  -172,  -172,  -172,
    -172,   135,     0,  -172,  -172,  -172,  -172,  -172,   119,   119,
     119,    32,   134,  -172,  -172,  -172,  -172,   340,    20,   335,
    -172,  -172,   121,    47,    53,   127,  -172,   195,   156,  -172,
     173,  -172,   189,  -172,  -172,  -172,    38,   168,   459,  -172,
     317,   160,  -172,  -172,  -172,  -172,   186,  -172,   102,   103,
     180,  -172,  -172,  -172,  -172,  -172,  -172,  -172,  -172,  -172,
    -172,  -172,  -172,  -172,  -172,  -172,  -172,  -172,  -172,  -172,
    -172,  -172,  -172,  -172,  -172,  -172,  -172,  -172,  -172,   413,
      63,  -172,  -172,  -172,  -172,  -172,   177,  -172,  -172,   197,
    -172,  -172,  -172,   203,   114,  -172,  -172,    16,  -172,   152,
      94,   126,  -172,  -172,  -172,  -172,  -172,  -172,  -172,   451,
    -172,  -172,   317,   160,   119,   160,  -172,   119,  -172,  -172,
    -172,  -172,   119,   194,  -172,   213,  -172,  -172,  -172,   119,
     119,   119,   119,   119,   119,   119,   119,   119,   119,   119,
     119,   119,   119,   119,   119,   119,   119,   119,   119,   119,
     119,   119,   119,   277,   119,  -172,  -172,  -172,  -172,   378,
      32,   193,  -172,   133,   219,  -172,   181,   192,   196,   206,
     383,   127,   173,   241,    37,  -172,  -172,  -172,  -172,  -172,
    -172,   383,   119,   119,   119,  -172,   119,   160,   119,   119,
     119,  -172,  -172,  -172,  -172,   119,   335,  -172,    74,   119,
     119,  -172,   157,   107,    32,   222,   276,  -172,   278,   279,
     281,  -172,  -172,  -172,  -172,  -172,  -172,  -172,  -172,  -172,
    -172,  -172,  -172,  -172,   221,   119,  -172,    45,  -172,  -172,
    -172,  -172,  -172,  -172,  -172,  -172,   161,   283,   119,   119,
     119,   377,  -172,  -172,  -172,   288,    -2,  -172,   167,   161,
     198,   210,  -172,    32,  -172,  -172,  -172,   226,  -172,   227,
       2,  -172,  -172,  -172,   119,  -172,  -172,  -172,  -172,    39,
    -172,  -172,  -172,  -172,   226,  -172,  -172,  -172,   119,  -172,
    -172,  -172,   119,   106,  -172,  -172,    27,   161,  -172,   119,
      29,  -172,  -172,   217,  -172,  -172,  -172,   119
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -172,  -172,  -172,  -172,  -172,  -172,    -1,   -78,  -172,  -172,
    -172,   -31,  -172,  -172,  -172,  -172,  -172,    31,  -172,    44,
    -172,  -172,     7,  -172,  -172,  -172,  -172,  -171,  -172,   105,
     199,   -93,   297,   -32,   -37,  -172,   146,  -172,    99,    64,
    -172,   304,  -172,   293,  -172,  -172,   -66,  -172,   184,   272,
    -172,  -172,  -172,  -172,    -3,  -172
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -123
static const yytype_int16 yytable[] =
{
      12,    95,   225,   290,    25,    18,    19,    20,    24,   187,
      13,    26,    27,    28,   128,    29,   123,   170,   125,    11,
      25,    56,   -11,    17,    25,    34,   195,    14,   181,   -11,
      98,    25,    57,    25,   182,    41,    30,    31,   107,   112,
     290,   110,   111,   238,   239,   240,   241,   242,   188,   124,
     127,   238,   239,   240,   241,   242,   132,    25,   -45,   291,
     139,   140,   141,   142,   143,   144,   145,   146,   147,   148,
     149,   150,   151,   152,   153,   154,   155,   156,   157,   158,
     159,   160,   161,   162,   163,   164,   128,   234,   128,   301,
     197,   304,    14,   294,   214,   113,   291,   208,    25,   243,
     244,    22,    97,   133,   180,   187,   186,   265,   244,   100,
      25,    25,   191,    16,    57,   192,   193,   194,   166,   255,
     167,   196,   198,    25,   199,   176,   177,   178,   168,   200,
      25,   167,   306,    -2,   215,    33,   205,   206,   252,   168,
     251,    42,   220,   263,   188,    35,   101,    36,    37,    38,
      39,    40,    41,    42,   216,   217,   218,   134,   136,   137,
     128,   126,   298,   179,   -11,    25,   209,   210,   106,   115,
     213,   229,   -11,   268,    36,    37,    38,    39,   -11,   -11,
     -60,   190,   -60,   -60,   -60,   -60,   -60,   -60,   246,    22,
     109,    23,   108,   -11,   212,   138,   248,   -11,   174,   219,
     -11,   249,   171,   250,   -11,   -11,    22,   102,   185,   254,
     103,    22,   257,   253,   -11,   175,   259,   -11,   -11,   -11,
     -11,    22,   -64,   277,   -11,   -64,   116,   117,   118,    35,
      25,    36,    37,    38,    39,    40,    41,   285,   286,   -11,
      22,   129,   130,   -11,   267,   230,   104,   105,    22,   201,
     202,   274,    22,   236,   280,   278,   231,   281,   283,     1,
     232,     2,     3,     4,    22,   284,   282,   288,   203,   204,
     233,   292,   305,    22,   228,   293,    22,   258,   207,   260,
     261,    25,   262,   289,   115,   264,   296,   276,    61,    62,
     297,    63,   275,   299,   300,   -60,   247,   -60,   -60,   -60,
     -60,   -60,   -60,   279,   302,   307,   183,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,   237,
      86,    36,    37,    38,    39,    87,   266,   -65,    88,    25,
     -65,   116,   117,   118,    25,   122,    61,    62,   295,    63,
     114,   135,    35,   211,    36,    37,    38,    39,    40,    41,
      42,   165,     0,     0,     0,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    25,    86,    61,
      62,     0,    63,    87,     0,    35,    88,    36,    37,    38,
      39,    40,    41,     0,     0,     0,   272,     0,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
       0,    86,  -122,  -122,   167,     0,    87,     0,     0,    88,
       0,     0,   168,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,     0,
      81,    82,    83,    35,     0,    36,    37,    38,    39,    40,
      41,   120,     0,    36,    37,    38,    39,   121
};

static const yytype_int16 yycheck[] =
{
       1,    32,   173,     1,     4,     6,     7,     8,    11,    11,
      54,    11,    13,    14,    51,    16,    48,    95,    50,     1,
       4,     1,     4,     0,     4,    26,   119,    54,    12,    11,
      33,     4,    12,     4,    18,    19,     4,     5,    39,     1,
       1,    42,    43,     6,     7,     8,     9,    10,    50,    50,
      51,     6,     7,     8,     9,    10,    57,     4,    56,    57,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,   123,   180,   125,    62,
     122,    62,    54,    54,   172,    57,    57,   163,     4,    62,
      63,    54,    55,     1,   105,    11,   109,    62,    63,    56,
       4,     4,   113,    54,    12,   116,   117,   118,    55,    12,
      57,   122,   123,     4,   125,    11,    12,    13,    65,   130,
       4,    57,   303,     0,     1,     1,   137,   138,    64,    65,
     206,    20,   173,   221,    50,    12,    19,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    55,    55,    56,
     197,     1,    56,    49,     4,     4,   167,   168,    12,     1,
     171,   174,    12,    12,    14,    15,    16,    17,    18,    19,
      12,    55,    14,    15,    16,    17,    18,    19,   189,    54,
       1,    56,    19,     4,     1,    15,   197,     4,     1,    66,
      11,   202,    25,   204,    11,    12,    54,    12,    56,   212,
      15,    54,   215,    56,    54,    12,   217,    57,    58,    59,
      60,    54,    54,    56,    64,    57,    58,    59,    60,    12,
       4,    14,    15,    16,    17,    18,    19,    11,    12,    50,
      54,    55,    56,    50,   245,    64,    51,    52,    54,    55,
      56,   252,    54,    12,    56,   258,    64,   260,   261,    24,
      64,    26,    27,    28,    54,   266,    56,   268,    55,    56,
      64,   272,    55,    54,    55,   276,    54,     1,     1,     1,
       1,     4,     1,    56,     1,    64,   287,   256,    11,    12,
     291,    14,     4,   294,   295,    12,   191,    14,    15,    16,
      17,    18,    19,   259,   297,   306,   107,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,   183,
      53,    14,    15,    16,    17,    58,   237,    54,    61,     4,
      57,    58,    59,    60,     4,    48,    11,    12,   284,    14,
      46,    58,    12,   169,    14,    15,    16,    17,    18,    19,
      20,    89,    -1,    -1,    -1,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,     4,    53,    11,
      12,    -1,    14,    58,    -1,    12,    61,    14,    15,    16,
      17,    18,    19,    -1,    -1,    -1,    29,    -1,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    53,    55,    56,    57,    -1,    58,    -1,    -1,    61,
      -1,    -1,    65,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    -1,
      47,    48,    49,    12,    -1,    14,    15,    16,    17,    18,
      19,    12,    -1,    14,    15,    16,    17,    18
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    24,    26,    27,    28,    68,    70,    71,    72,    75,
      76,     1,    73,    54,    54,   108,    54,     0,    73,    73,
      73,    74,    54,    56,   121,     4,    11,    73,    73,    73,
       4,     5,    77,     1,    73,    12,    14,    15,    16,    17,
      18,    19,    20,    69,    78,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   104,   107,     1,    12,   109,   110,
     111,    11,    12,    14,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    53,    58,    61,    93,
     113,   115,   116,   117,   118,    78,    79,    55,   121,   122,
      56,    19,    12,    15,    51,    52,    12,    73,    19,     1,
      73,    73,     1,    57,   108,     1,    58,    59,    60,    92,
      12,    18,    99,   100,    73,   100,     1,    73,   101,    55,
      56,   121,    73,     1,    55,   110,    55,    56,    15,    73,
      73,    73,    73,    73,    73,    73,    73,    73,    73,    73,
      73,    73,    73,    73,    73,    73,    73,    73,    73,    73,
      73,    73,    73,    73,    73,   116,    55,    57,    65,   114,
      74,    25,    80,    82,     1,    12,    11,    12,    13,    49,
      73,    12,    18,    97,   103,    56,   121,    11,    50,    84,
      55,    73,    73,    73,    73,    98,    73,   100,    73,    73,
      73,    55,    56,    55,    56,    73,    73,     1,   113,    73,
      73,   115,     1,    73,    74,     1,    21,    22,    23,    66,
      78,    83,    87,    90,    91,    94,   119,   120,    55,   121,
      64,    64,    64,    64,    98,    73,    12,   103,     6,     7,
       8,     9,    10,    62,    63,   105,    73,    96,    73,    73,
      73,   113,    64,    56,   121,    12,    81,   121,     1,    73,
       1,     1,     1,    74,    64,    62,   105,    73,    12,    85,
      86,    89,    29,   112,    73,     4,    84,    56,   121,    86,
      56,   121,    56,   121,    73,    11,    12,   106,    73,    56,
       1,    57,    73,    73,    54,   106,    73,    73,    56,    73,
      73,    62,    89,    88,    62,    55,    94,    73
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
# define YYLEX yylex (&yylval, YYLEX_PARAM)
#else
# define YYLEX yylex (&yylval)
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
		  Type, Value); \
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
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (!yyvaluep)
    return;
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
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
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
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
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
		       		       );
      fprintf (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
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
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {
      case 85: /* "maybe_media_list" */

	{ delete (yyvaluep->mediaList); (yyvaluep->mediaList) = 0; };

	break;
      case 86: /* "media_list" */

	{ delete (yyvaluep->mediaList); (yyvaluep->mediaList) = 0; };

	break;
      case 88: /* "ruleset_list" */

	{ delete (yyvaluep->ruleList); (yyvaluep->ruleList) = 0; };

	break;
      case 95: /* "selector_list" */

	{ if ((yyvaluep->selectorList)) qDeleteAll(*(yyvaluep->selectorList)); delete (yyvaluep->selectorList); (yyvaluep->selectorList) = 0; };

	break;
      case 96: /* "selector" */

	{ delete (yyvaluep->selector); (yyvaluep->selector) = 0; };

	break;
      case 98: /* "simple_selector" */

	{ delete (yyvaluep->selector); (yyvaluep->selector) = 0; };

	break;
      case 100: /* "specifier_list" */

	{ delete (yyvaluep->selector); (yyvaluep->selector) = 0; };

	break;
      case 101: /* "specifier" */

	{ delete (yyvaluep->selector); (yyvaluep->selector) = 0; };

	break;
      case 102: /* "class" */

	{ delete (yyvaluep->selector); (yyvaluep->selector) = 0; };

	break;
      case 104: /* "attrib" */

	{ delete (yyvaluep->selector); (yyvaluep->selector) = 0; };

	break;
      case 107: /* "pseudo" */

	{ delete (yyvaluep->selector); (yyvaluep->selector) = 0; };

	break;
      case 113: /* "expr" */

	{ delete (yyvaluep->valueList); (yyvaluep->valueList) = 0; };

	break;

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
  /* The look-ahead symbol.  */
int yychar;

/* The semantic value of the look-ahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;

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



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


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


	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

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

#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;


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


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 8:

    {
        CSSParser *p = static_cast<CSSParser *>(parser);
	p->rule = (yyvsp[(4) - (6)].rule);
    ;}
    break;

  case 9:

    {
	/* can be empty */
    ;}
    break;

  case 10:

    {
	CSSParser *p = static_cast<CSSParser *>(parser);
	if ( (yyvsp[(4) - (5)].valueList) ) {
	    p->valueList = (yyvsp[(4) - (5)].valueList);
#ifdef CSS_DEBUG
	    kDebug( 6080 ) << "   got property for " << p->id <<
		(p->important?" important":"");
	    bool ok =
#endif
		p->parseValue( p->id, p->important );
#ifdef CSS_DEBUG
	    if ( !ok )
		kDebug( 6080 ) << "     couldn't parse value!";
#endif
	}
#ifdef CSS_DEBUG
	else
	    kDebug( 6080 ) << "     no value found!";
#endif
	delete p->valueList;
	p->valueList = 0;
    ;}
    break;

  case 18:

    {
#ifdef CSS_DEBUG
     kDebug( 6080 ) << "charset rule: " << qString((yyvsp[(3) - (5)].string));
#endif
     CSSParser* p = static_cast<CSSParser*>(parser);
     if ((yyval.rule) && p->styleElement && p->styleElement->isCSSStyleSheet()) {
         (yyval.rule) = new CSSCharsetRuleImpl(p->styleElement, domString((yyvsp[(3) - (5)].string)));
         p->styleElement->append((yyval.rule));
     } else
         (yyval.rule) = 0;
 ;}
    break;

  case 19:

    {
      (yyval.rule) = 0;
 ;}
    break;

  case 20:

    {
      (yyval.rule) = 0;
 ;}
    break;

  case 22:

    {
     CSSParser *p = static_cast<CSSParser *>(parser);
     if ( (yyvsp[(2) - (3)].rule) && p->styleElement && p->styleElement->isCSSStyleSheet() ) {
	 p->styleElement->append( (yyvsp[(2) - (3)].rule) );
     } else {
	 delete (yyvsp[(2) - (3)].rule);
     }
 ;}
    break;

  case 23:

    {
#ifdef CSS_DEBUG
	kDebug( 6080 ) << "@import: " << qString((yyvsp[(3) - (6)].string));
#endif
	CSSParser *p = static_cast<CSSParser *>(parser);
	if ( (yyvsp[(5) - (6)].mediaList) && p->styleElement && p->styleElement->isCSSStyleSheet() )
	    (yyval.rule) = new CSSImportRuleImpl( p->styleElement, domString((yyvsp[(3) - (6)].string)), (yyvsp[(5) - (6)].mediaList) );
	else
	    (yyval.rule) = 0;
    ;}
    break;

  case 24:

    {
        (yyval.rule) = 0;
    ;}
    break;

  case 25:

    {
        (yyval.rule) = 0;
    ;}
    break;

  case 28:

    {
#ifdef CSS_DEBUG
    kDebug( 6080 ) << "@namespace: " << qString((yyvsp[(4) - (6)].string));
#endif
      CSSParser *p = static_cast<CSSParser *>(parser);
    if (p->styleElement && p->styleElement->isCSSStyleSheet())
        static_cast<CSSStyleSheetImpl*>(p->styleElement)->addNamespace(p, domString((yyvsp[(3) - (6)].string)), domString((yyvsp[(4) - (6)].string)));
 ;}
    break;

  case 31:

    { (yyval.string).string = 0; ;}
    break;

  case 32:

    { (yyval.string) = (yyvsp[(1) - (2)].string); ;}
    break;

  case 34:

    {
     CSSParser *p = static_cast<CSSParser *>(parser);
     if ( (yyvsp[(2) - (3)].rule) && p->styleElement && p->styleElement->isCSSStyleSheet() ) {
	 p->styleElement->append( (yyvsp[(2) - (3)].rule) );
     } else {
	 delete (yyvsp[(2) - (3)].rule);
     }
 ;}
    break;

  case 41:

    { delete (yyvsp[(1) - (2)].rule); (yyval.rule) = 0; ;}
    break;

  case 44:

    {
	(yyval.mediaList) = new MediaListImpl();
    ;}
    break;

  case 46:

    {
	(yyval.mediaList) = new MediaListImpl();
	(yyval.mediaList)->appendMedium( domString((yyvsp[(1) - (1)].string)).lower() );
    ;}
    break;

  case 47:

    {
	(yyval.mediaList) = (yyvsp[(1) - (4)].mediaList);
	if ((yyval.mediaList))
	    (yyval.mediaList)->appendMedium( domString((yyvsp[(4) - (4)].string)).lower() );
    ;}
    break;

  case 48:

    {
       delete (yyvsp[(1) - (2)].mediaList);
       (yyval.mediaList) = 0;
    ;}
    break;

  case 49:

    {
	CSSParser *p = static_cast<CSSParser *>(parser);
	if ( (yyvsp[(3) - (7)].mediaList) && (yyvsp[(6) - (7)].ruleList) &&
	     p->styleElement && p->styleElement->isCSSStyleSheet() ) {
	    (yyval.rule) = new CSSMediaRuleImpl( static_cast<CSSStyleSheetImpl*>(p->styleElement), (yyvsp[(3) - (7)].mediaList), (yyvsp[(6) - (7)].ruleList) );
	} else {
	    (yyval.rule) = 0;
	    delete (yyvsp[(3) - (7)].mediaList);
	    delete (yyvsp[(6) - (7)].ruleList);
	}
    ;}
    break;

  case 50:

    { (yyval.ruleList) = 0; ;}
    break;

  case 51:

    {
      (yyval.ruleList) = (yyvsp[(1) - (3)].ruleList);
      if ( (yyvsp[(2) - (3)].rule) ) {
	  if ( !(yyval.ruleList) ) (yyval.ruleList) = new CSSRuleListImpl();
	  (yyval.ruleList)->append( (yyvsp[(2) - (3)].rule) );
      }
  ;}
    break;

  case 52:

    {
      (yyval.string) = (yyvsp[(1) - (2)].string);
  ;}
    break;

  case 53:

    {
      (yyval.rule) = 0;
    ;}
    break;

  case 54:

    {
      (yyval.rule) = 0;
    ;}
    break;

  case 55:

    {
      (yyval.rule) = 0;
    ;}
    break;

  case 56:

    {
      (yyval.rule) = 0;
    ;}
    break;

  case 57:

    { (yyval.relation) = CSSSelector::DirectAdjacent; ;}
    break;

  case 58:

    { (yyval.relation) = CSSSelector::IndirectAdjacent; ;}
    break;

  case 59:

    { (yyval.relation) = CSSSelector::Child; ;}
    break;

  case 60:

    { (yyval.relation) = CSSSelector::Descendant; ;}
    break;

  case 61:

    { (yyval.val) = -1; ;}
    break;

  case 62:

    { (yyval.val) = 1; ;}
    break;

  case 63:

    {
#ifdef CSS_DEBUG
	kDebug( 6080 ) << "got ruleset" << endl << "  selector:";
#endif
	CSSParser *p = static_cast<CSSParser *>(parser);
	if ( (yyvsp[(1) - (2)].selectorList) && (yyvsp[(2) - (2)].ok) && p->numParsedProperties ) {
	    CSSStyleRuleImpl *rule = new CSSStyleRuleImpl( p->styleElement );
	    CSSStyleDeclarationImpl *decl = p->createStyleDeclaration( rule );
	    rule->setSelector( (yyvsp[(1) - (2)].selectorList) );
	    rule->setDeclaration(decl);
	    (yyval.rule) = rule;
	} else {
	    (yyval.rule) = 0;
	    if ((yyvsp[(1) - (2)].selectorList)) qDeleteAll(*(yyvsp[(1) - (2)].selectorList));
	    delete (yyvsp[(1) - (2)].selectorList);
	    (yyvsp[(1) - (2)].selectorList) = 0;
	    p->clearProperties();
	}
    ;}
    break;

  case 64:

    {
	if ( (yyvsp[(1) - (1)].selector) ) {
	    (yyval.selectorList) = new QList<CSSSelector*>;
#ifdef CSS_DEBUG
	    kDebug( 6080 ) << "   got simple selector:";
	    (yyvsp[(1) - (1)].selector)->print();
#endif
	    (yyval.selectorList)->append( (yyvsp[(1) - (1)].selector) );
	    khtml::CSSStyleSelector::precomputeAttributeDependencies(static_cast<CSSParser *>(parser)->document(), (yyvsp[(1) - (1)].selector));
	} else {
	    (yyval.selectorList) = 0;
	}
    ;}
    break;

  case 65:

    {
	if ( (yyvsp[(1) - (4)].selectorList) && (yyvsp[(4) - (4)].selector) ) {
	    (yyval.selectorList) = (yyvsp[(1) - (4)].selectorList);
	    (yyval.selectorList)->append( (yyvsp[(4) - (4)].selector) );
	    khtml::CSSStyleSelector::precomputeAttributeDependencies(static_cast<CSSParser *>(parser)->document(), (yyvsp[(4) - (4)].selector));
#ifdef CSS_DEBUG
	    kDebug( 6080 ) << "   got simple selector:";
	    (yyvsp[(4) - (4)].selector)->print();
#endif
	} else {
            if ((yyvsp[(1) - (4)].selectorList)) qDeleteAll(*(yyvsp[(1) - (4)].selectorList));
	    delete (yyvsp[(1) - (4)].selectorList);
	    (yyvsp[(1) - (4)].selectorList)=0;

	    delete (yyvsp[(4) - (4)].selector);
	    (yyval.selectorList) = 0;
	}
    ;}
    break;

  case 66:

    {
        if ((yyvsp[(1) - (2)].selectorList)) qDeleteAll(*(yyvsp[(1) - (2)].selectorList));
	delete (yyvsp[(1) - (2)].selectorList);
	(yyvsp[(1) - (2)].selectorList) = 0;
	(yyval.selectorList) = 0;
    ;}
    break;

  case 67:

    {
	(yyval.selector) = (yyvsp[(1) - (1)].selector);
    ;}
    break;

  case 68:

    {
	if ( !(yyvsp[(1) - (3)].selector) || !(yyvsp[(3) - (3)].selector) ) {
	    delete (yyvsp[(1) - (3)].selector);
	    delete (yyvsp[(3) - (3)].selector);
	    (yyval.selector) = 0;
	} else {
	    (yyval.selector) = (yyvsp[(3) - (3)].selector);
	    CSSSelector *end = (yyvsp[(3) - (3)].selector);
	    while( end->tagHistory )
		end = end->tagHistory;
	    end->relation = (yyvsp[(2) - (3)].relation);
	    end->tagHistory = (yyvsp[(1) - (3)].selector);
	}
    ;}
    break;

  case 69:

    {
	delete (yyvsp[(1) - (2)].selector);
	(yyval.selector) = 0;
    ;}
    break;

  case 70:

    { (yyval.string).string = 0; (yyval.string).length = 0; ;}
    break;

  case 71:

    { static unsigned short star = '*'; (yyval.string).string = &star; (yyval.string).length = 1; ;}
    break;

  case 72:

    { (yyval.string) = (yyvsp[(1) - (2)].string); ;}
    break;

  case 73:

    {
	(yyval.selector) = new CSSSelector();
	(yyval.selector)->tag = (yyvsp[(1) - (2)].element);
    ;}
    break;

  case 74:

    {
	(yyval.selector) = (yyvsp[(2) - (3)].selector);
        if ( (yyval.selector) )
	    (yyval.selector)->tag = (yyvsp[(1) - (3)].element);
    ;}
    break;

  case 75:

    {
	(yyval.selector) = (yyvsp[(1) - (2)].selector);
        if ( (yyval.selector) )
            (yyval.selector)->tag = makeId(static_cast<CSSParser*>(parser)->defaultNamespace(), anyLocalName);
    ;}
    break;

  case 76:

    {
        (yyval.selector) = new CSSSelector();
        (yyval.selector)->tag = (yyvsp[(2) - (3)].element);
	CSSParser *p = static_cast<CSSParser *>(parser);
        if (p->styleElement && p->styleElement->isCSSStyleSheet())
            static_cast<CSSStyleSheetImpl*>(p->styleElement)->determineNamespace((yyval.selector)->tag, domString((yyvsp[(1) - (3)].string)));
    ;}
    break;

  case 77:

    {
        (yyval.selector) = (yyvsp[(3) - (4)].selector);
        if ((yyval.selector)) {
            (yyval.selector)->tag = (yyvsp[(2) - (4)].element);
            CSSParser *p = static_cast<CSSParser *>(parser);
            if (p->styleElement && p->styleElement->isCSSStyleSheet())
                static_cast<CSSStyleSheetImpl*>(p->styleElement)->determineNamespace((yyval.selector)->tag, domString((yyvsp[(1) - (4)].string)));
        }
    ;}
    break;

  case 78:

    {
        (yyval.selector) = (yyvsp[(2) - (3)].selector);
        if ((yyval.selector)) {
            (yyval.selector)->tag = makeId(anyNamespace, anyLocalName);
            CSSParser *p = static_cast<CSSParser *>(parser);
            if (p->styleElement && p->styleElement->isCSSStyleSheet())
                static_cast<CSSStyleSheetImpl*>(p->styleElement)->determineNamespace((yyval.selector)->tag, domString((yyvsp[(1) - (3)].string)));
        }
    ;}
    break;

  case 79:

    {
	CSSParser *p = static_cast<CSSParser *>(parser);
	DOM::DocumentImpl *doc = p->document();
	QString tag = qString((yyvsp[(1) - (1)].string));
	if ( doc ) {
	    if (doc->isHTMLDocument())
		tag = tag.toLower();
	    const DOMString dtag(tag);
            (yyval.element) = makeId(p->defaultNamespace(), doc->getId(NodeImpl::ElementId, dtag.implementation(), false, true));
	} else {
	    (yyval.element) = makeId(p->defaultNamespace(), khtml::getTagID(tag.toLower().toAscii(), tag.length()));
	    // this case should never happen - only when loading
	    // the default stylesheet - which must not contain unknown tags
// 	    assert($$ != 0);
	}
    ;}
    break;

  case 80:

    {
	(yyval.element) = makeId(static_cast<CSSParser*>(parser)->defaultNamespace(), anyLocalName);
    ;}
    break;

  case 81:

    {
	(yyval.selector) = (yyvsp[(1) - (1)].selector);
    ;}
    break;

  case 82:

    {
	(yyval.selector) = (yyvsp[(1) - (2)].selector);
	if ( (yyval.selector) ) {
            CSSSelector *end = (yyvsp[(1) - (2)].selector);
            while( end->tagHistory )
                end = end->tagHistory;
            end->relation = CSSSelector::SubSelector;
            end->tagHistory = (yyvsp[(2) - (2)].selector);
	}
    ;}
    break;

  case 83:

    {
	delete (yyvsp[(1) - (2)].selector);
	(yyval.selector) = 0;
    ;}
    break;

  case 84:

    {
	(yyval.selector) = new CSSSelector();
	(yyval.selector)->match = CSSSelector::Id;
	(yyval.selector)->attr = ATTR_ID;
	(yyval.selector)->value = domString((yyvsp[(1) - (1)].string));
    ;}
    break;

  case 88:

    {
	(yyval.selector) = new CSSSelector();
	(yyval.selector)->match = CSSSelector::Class;
	(yyval.selector)->attr = ATTR_CLASS;
	(yyval.selector)->value = domString((yyvsp[(2) - (2)].string));
    ;}
    break;

  case 89:

    {
	CSSParser *p = static_cast<CSSParser *>(parser);
	DOM::DocumentImpl *doc = p->document();

	QString attr = qString((yyvsp[(1) - (2)].string));
	if ( doc ) {
	    if (doc->isHTMLDocument())
		attr = attr.toLower();
	    const DOMString dattr(attr);
#ifdef APPLE_CHANGES
            (yyval.attribute) = doc->attrId(0, dattr.implementation(), false);
#else
	    (yyval.attribute) = doc->getId(NodeImpl::AttributeId, dattr.implementation(), false, true);
#endif
	} else {
	    (yyval.attribute) = khtml::getAttrID(attr.toLower().toAscii(), attr.length());
	    // this case should never happen - only when loading
	    // the default stylesheet - which must not contain unknown attributes
	    assert((yyval.attribute) != 0);
	    }
    ;}
    break;

  case 90:

    {
	(yyval.selector) = new CSSSelector();
	(yyval.selector)->attr = (yyvsp[(3) - (4)].attribute);
	(yyval.selector)->match = CSSSelector::Set;
    ;}
    break;

  case 91:

    {
	(yyval.selector) = new CSSSelector();
	(yyval.selector)->attr = (yyvsp[(3) - (8)].attribute);
	(yyval.selector)->match = (yyvsp[(4) - (8)].match);
	(yyval.selector)->value = domString((yyvsp[(6) - (8)].string));
    ;}
    break;

  case 92:

    {
        (yyval.selector) = new CSSSelector();
        (yyval.selector)->attr = (yyvsp[(4) - (5)].attribute);
        (yyval.selector)->match = CSSSelector::Set;
        CSSParser *p = static_cast<CSSParser *>(parser);
        if (p->styleElement && p->styleElement->isCSSStyleSheet())
            static_cast<CSSStyleSheetImpl*>(p->styleElement)->determineNamespace((yyval.selector)->attr, domString((yyvsp[(3) - (5)].string)));
    ;}
    break;

  case 93:

    {
        (yyval.selector) = new CSSSelector();
        (yyval.selector)->attr = (yyvsp[(4) - (9)].attribute);
        (yyval.selector)->match = (CSSSelector::Match)(yyvsp[(5) - (9)].match);
        (yyval.selector)->value = domString((yyvsp[(7) - (9)].string));
        CSSParser *p = static_cast<CSSParser *>(parser);
        if (p->styleElement && p->styleElement->isCSSStyleSheet())
            static_cast<CSSStyleSheetImpl*>(p->styleElement)->determineNamespace((yyval.selector)->attr, domString((yyvsp[(3) - (9)].string)));
   ;}
    break;

  case 94:

    {
	(yyval.match) = CSSSelector::Exact;
    ;}
    break;

  case 95:

    {
	(yyval.match) = CSSSelector::List;
    ;}
    break;

  case 96:

    {
	(yyval.match) = CSSSelector::Hyphen;
    ;}
    break;

  case 97:

    {
	(yyval.match) = CSSSelector::Begin;
    ;}
    break;

  case 98:

    {
	(yyval.match) = CSSSelector::End;
    ;}
    break;

  case 99:

    {
	(yyval.match) = CSSSelector::Contain;
    ;}
    break;

  case 102:

    {
	(yyval.selector) = new CSSSelector();
	(yyval.selector)->match = CSSSelector::PseudoClass;
	(yyval.selector)->value = domString((yyvsp[(2) - (2)].string));
    ;}
    break;

  case 103:

    {
	(yyval.selector) = new CSSSelector();
	(yyval.selector)->match = CSSSelector::PseudoElement;
        (yyval.selector)->value = domString((yyvsp[(3) - (3)].string));
    ;}
    break;

  case 104:

    {
        (yyval.selector) = new CSSSelector();
        (yyval.selector)->match = CSSSelector::PseudoClass;
        (yyval.selector)->string_arg = domString((yyvsp[(3) - (4)].string));
        (yyval.selector)->value = domString((yyvsp[(2) - (4)].string));
    ;}
    break;

  case 105:

    {
        (yyval.selector) = new CSSSelector();
        (yyval.selector)->match = CSSSelector::PseudoClass;
        (yyval.selector)->string_arg = QString::number((yyvsp[(3) - (4)].val));
        (yyval.selector)->value = domString((yyvsp[(2) - (4)].string));
    ;}
    break;

  case 106:

    {
        (yyval.selector) = new CSSSelector();
        (yyval.selector)->match = CSSSelector::PseudoClass;
        (yyval.selector)->string_arg = domString((yyvsp[(3) - (4)].string));
        (yyval.selector)->value = domString((yyvsp[(2) - (4)].string));
    ;}
    break;

  case 107:

    {
        (yyval.selector) = new CSSSelector();
        (yyval.selector)->match = CSSSelector::PseudoClass;
        (yyval.selector)->string_arg = domString((yyvsp[(3) - (4)].string));
        (yyval.selector)->value = domString((yyvsp[(2) - (4)].string));
    ;}
    break;

  case 108:

    {
        (yyval.selector) = new CSSSelector();
        (yyval.selector)->match = CSSSelector::PseudoClass;
        (yyval.selector)->simpleSelector = (yyvsp[(4) - (5)].selector);
        (yyval.selector)->value = domString((yyvsp[(2) - (5)].string));
    ;}
    break;

  case 109:

    {
	(yyval.ok) = (yyvsp[(3) - (4)].ok);
    ;}
    break;

  case 110:

    {
	(yyval.ok) = false;
    ;}
    break;

  case 111:

    {
	(yyval.ok) = (yyvsp[(3) - (4)].ok);
    ;}
    break;

  case 112:

    {
	(yyval.ok) = (yyvsp[(3) - (5)].ok);
	if ( (yyvsp[(4) - (5)].ok) )
	    (yyval.ok) = (yyvsp[(4) - (5)].ok);
    ;}
    break;

  case 113:

    {
	(yyval.ok) = (yyvsp[(3) - (5)].ok);
    ;}
    break;

  case 114:

    {
	(yyval.ok) = (yyvsp[(1) - (3)].ok);
    ;}
    break;

  case 115:

    {
        (yyval.ok) = false;
    ;}
    break;

  case 116:

    {
	(yyval.ok) = (yyvsp[(1) - (4)].ok);
	if ( (yyvsp[(2) - (4)].ok) )
	    (yyval.ok) = (yyvsp[(2) - (4)].ok);
    ;}
    break;

  case 117:

    {
        (yyval.ok) = (yyvsp[(1) - (4)].ok);
    ;}
    break;

  case 118:

    {
	(yyval.ok) = false;
	CSSParser *p = static_cast<CSSParser *>(parser);
	if ( (yyvsp[(1) - (5)].prop_id) && (yyvsp[(4) - (5)].valueList) ) {
	    p->valueList = (yyvsp[(4) - (5)].valueList);
#ifdef CSS_DEBUG
	    kDebug( 6080 ) << "   got property: " << (yyvsp[(1) - (5)].prop_id) <<
		((yyvsp[(5) - (5)].b)?" important":"");
#endif
	        bool ok = p->parseValue( (yyvsp[(1) - (5)].prop_id), (yyvsp[(5) - (5)].b) );
                if ( ok )
		    (yyval.ok) = ok;
#ifdef CSS_DEBUG
	        else
		    kDebug( 6080 ) << "     couldn't parse value!";
#endif
	} else {
            delete (yyvsp[(4) - (5)].valueList);
        }
	delete p->valueList;
	p->valueList = 0;
    ;}
    break;

  case 119:

    {
        (yyval.ok) = false;
    ;}
    break;

  case 120:

    {
	QString str = qString((yyvsp[(1) - (2)].string));
	(yyval.prop_id) = getPropertyID( str.toLower().toLatin1(), str.length() );
    ;}
    break;

  case 121:

    { (yyval.b) = true; ;}
    break;

  case 122:

    { (yyval.b) = false; ;}
    break;

  case 123:

    {
	(yyval.valueList) = new ValueList;
	(yyval.valueList)->addValue( (yyvsp[(1) - (1)].value) );
    ;}
    break;

  case 124:

    {
	(yyval.valueList) = (yyvsp[(1) - (3)].valueList);
	if ( (yyval.valueList) ) {
	    if ( (yyvsp[(2) - (3)].tok) ) {
		Value v;
		v.id = 0;
		v.unit = Value::Operator;
		v.iValue = (yyvsp[(2) - (3)].tok);
		(yyval.valueList)->addValue( v );
	    }
	    (yyval.valueList)->addValue( (yyvsp[(3) - (3)].value) );
	}
    ;}
    break;

  case 125:

    {
	(yyval.tok) = '/';
    ;}
    break;

  case 126:

    {
	(yyval.tok) = ',';
    ;}
    break;

  case 127:

    {
        (yyval.tok) = 0;
  ;}
    break;

  case 128:

    { (yyval.value) = (yyvsp[(1) - (1)].value); ;}
    break;

  case 129:

    { (yyval.value) = (yyvsp[(2) - (2)].value); (yyval.value).fValue *= (yyvsp[(1) - (2)].val); ;}
    break;

  case 130:

    { (yyval.value).id = 0; (yyval.value).string = (yyvsp[(1) - (2)].string); (yyval.value).unit = CSSPrimitiveValue::CSS_DIMENSION; ;}
    break;

  case 131:

    { (yyval.value).id = 0; (yyval.value).string = (yyvsp[(1) - (2)].string); (yyval.value).unit = CSSPrimitiveValue::CSS_STRING; ;}
    break;

  case 132:

    {
      QString str = qString( (yyvsp[(1) - (2)].string) );
      (yyval.value).id = getValueID( str.toLower().toLatin1(), str.length() );
      (yyval.value).unit = CSSPrimitiveValue::CSS_IDENT;
      (yyval.value).string = (yyvsp[(1) - (2)].string);
  ;}
    break;

  case 133:

    { (yyval.value).id = 0; (yyval.value).string = (yyvsp[(1) - (2)].string); (yyval.value).unit = CSSPrimitiveValue::CSS_URI; ;}
    break;

  case 134:

    { (yyval.value).id = 0; (yyval.value).iValue = 0; (yyval.value).unit = CSSPrimitiveValue::CSS_UNKNOWN;/* ### */ ;}
    break;

  case 135:

    { (yyval.value).id = 0; (yyval.value).string = (yyvsp[(1) - (1)].string); (yyval.value).unit = CSSPrimitiveValue::CSS_RGBCOLOR; ;}
    break;

  case 136:

    {
      (yyval.value) = (yyvsp[(1) - (1)].value);
  ;}
    break;

  case 137:

    { (yyval.value).id = 0; (yyval.value).isInt = true; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_NUMBER; ;}
    break;

  case 138:

    { (yyval.value).id = 0; (yyval.value).isInt = false; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_NUMBER; ;}
    break;

  case 139:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_PERCENTAGE; ;}
    break;

  case 140:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_PX; ;}
    break;

  case 141:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_CM; ;}
    break;

  case 142:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_MM; ;}
    break;

  case 143:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_IN; ;}
    break;

  case 144:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_PT; ;}
    break;

  case 145:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_PC; ;}
    break;

  case 146:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_DEG; ;}
    break;

  case 147:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_RAD; ;}
    break;

  case 148:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_GRAD; ;}
    break;

  case 149:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_MS; ;}
    break;

  case 150:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_S; ;}
    break;

  case 151:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_HZ; ;}
    break;

  case 152:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_KHZ; ;}
    break;

  case 153:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_EMS; ;}
    break;

  case 154:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = Value::Q_EMS; ;}
    break;

  case 155:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_EXS; ;}
    break;

  case 156:

    {
      Function *f = new Function;
      f->name = (yyvsp[(1) - (5)].string);
      f->args = (yyvsp[(3) - (5)].valueList);
      (yyval.value).id = 0;
      (yyval.value).unit = Value::Function;
      (yyval.value).function = f;
  ;}
    break;

  case 157:

    {
      Function *f = new Function;
      f->name = (yyvsp[(1) - (3)].string);
      f->args = 0;
      (yyval.value).id = 0;
      (yyval.value).unit = Value::Function;
      (yyval.value).function = f;
  ;}
    break;

  case 158:

    { (yyval.string) = (yyvsp[(1) - (2)].string); ;}
    break;

  case 159:

    {
	(yyval.rule) = 0;
#ifdef CSS_DEBUG
	kDebug( 6080 ) << "skipped invalid @-rule";
#endif
    ;}
    break;

  case 160:

    {
	(yyval.rule) = 0;
#ifdef CSS_DEBUG
	kDebug( 6080 ) << "skipped invalid @-rule";
#endif
    ;}
    break;

  case 161:

    {
	(yyval.rule) = 0;
#ifdef CSS_DEBUG
	kDebug( 6080 ) << "skipped invalid rule";
#endif
    ;}
    break;


/* Line 1267 of yacc.c.  */

      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;


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
		      yytoken, &yylval);
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


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  *++yyvsp = yylval;


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
		 yytoken, &yylval);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
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






