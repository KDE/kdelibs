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
     KHTML_MEDIAQUERY_SYM = 279,
     IMPORTANT_SYM = 280,
     MEDIA_ONLY = 281,
     MEDIA_NOT = 282,
     MEDIA_AND = 283,
     QEMS = 284,
     EMS = 285,
     EXS = 286,
     PXS = 287,
     CMS = 288,
     MMS = 289,
     INS = 290,
     PTS = 291,
     PCS = 292,
     DEGS = 293,
     RADS = 294,
     GRADS = 295,
     MSECS = 296,
     SECS = 297,
     HERZ = 298,
     KHERZ = 299,
     DPI = 300,
     DPCM = 301,
     DIMEN = 302,
     PERCENTAGE = 303,
     FLOAT = 304,
     INTEGER = 305,
     URI = 306,
     FUNCTION = 307,
     NOTFUNCTION = 308,
     UNICODERANGE = 309
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
#define KHTML_MEDIAQUERY_SYM 279
#define IMPORTANT_SYM 280
#define MEDIA_ONLY 281
#define MEDIA_NOT 282
#define MEDIA_AND 283
#define QEMS 284
#define EMS 285
#define EXS 286
#define PXS 287
#define CMS 288
#define MMS 289
#define INS 290
#define PTS 291
#define PCS 292
#define DEGS 293
#define RADS 294
#define GRADS 295
#define MSECS 296
#define SECS 297
#define HERZ 298
#define KHERZ 299
#define DPI 300
#define DPCM 301
#define DIMEN 302
#define PERCENTAGE 303
#define FLOAT 304
#define INTEGER 305
#define URI 306
#define FUNCTION 307
#define NOTFUNCTION 308
#define UNICODERANGE 309




/* Copy the first part of user declarations.  */



/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2002-2003 Lars Knoll (knoll@kde.org)
 *  Copyright (c) 2003 Apple Computer
 *  Copyright (C) 2003 Dirk Mueller (mueller@kde.org)
 *  Copyright (C) 2008 Germain Garand (germain@ebooksfrance.org)
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
#include <css/css_mediaquery.h>
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
#define YYMAXDEPTH 10000
#define YYPARSE_PARAM parser
#define YYENABLE_NLS 0
#define YYLTYPE_IS_TRIVIAL 1


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

    khtml::MediaQuery* mediaQuery;
    khtml::MediaQueryExp* mediaQueryExp;
    QList<khtml::MediaQueryExp*>* mediaQueryExpList;
    khtml::MediaQuery::Restrictor mediaQueryRestrictor;
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
#define YYFINAL  20
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   586

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  74
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  66
/* YYNRULES -- Number of rules.  */
#define YYNRULES  191
/* YYNRULES -- Number of states.  */
#define YYNSTATES  365

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   309

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      63,    64,    18,    66,    65,    69,    16,    72,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    15,    62,
       2,    71,    68,     2,    73,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    17,     2,    70,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    60,    19,    61,    67,     2,     2,     2,
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
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     9,    12,    15,    18,    21,    23,    25,
      32,    35,    41,    47,    48,    51,    52,    55,    58,    59,
      61,    67,    71,    75,    76,    80,    87,    91,    95,    96,
     100,   107,   111,   115,   116,   119,   120,   124,   126,   128,
     130,   132,   134,   136,   139,   141,   143,   145,   148,   150,
     152,   155,   156,   161,   169,   171,   177,   178,   182,   183,
     185,   187,   189,   194,   195,   197,   199,   204,   207,   215,
     222,   223,   227,   230,   234,   238,   242,   246,   249,   252,
     255,   256,   258,   260,   263,   265,   270,   273,   275,   279,
     282,   284,   287,   290,   293,   297,   300,   304,   309,   313,
     316,   319,   323,   327,   329,   331,   333,   336,   339,   341,
     343,   345,   347,   350,   353,   358,   367,   373,   383,   385,
     387,   389,   391,   393,   395,   397,   399,   402,   406,   411,
     416,   421,   426,   432,   437,   442,   447,   453,   459,   463,
     467,   472,   477,   483,   486,   489,   492,   493,   495,   499,
     502,   505,   506,   508,   511,   514,   517,   520,   523,   526,
     528,   530,   533,   536,   539,   542,   545,   548,   551,   554,
     557,   560,   563,   566,   569,   572,   575,   578,   581,   584,
     587,   590,   593,   599,   603,   606,   610,   614,   617,   623,
     627,   629
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
      75,     0,    -1,    83,    82,    85,    87,    90,    -1,    77,
      81,    -1,    78,    81,    -1,    79,    81,    -1,    80,    81,
      -1,   110,    -1,    86,    -1,    26,    60,    81,    76,    81,
      61,    -1,    27,   125,    -1,    28,    60,    81,   130,    61,
      -1,    29,     4,    81,   100,    61,    -1,    -1,    81,     4,
      -1,    -1,    82,     5,    -1,    82,     4,    -1,    -1,    84,
      -1,    24,    81,    11,    81,    62,    -1,    24,     1,   138,
      -1,    24,     1,    62,    -1,    -1,    85,    86,    82,    -1,
      20,    81,    93,    81,   101,    62,    -1,    20,     1,   138,
      -1,    20,     1,    62,    -1,    -1,    87,    88,    82,    -1,
      25,    81,    89,    93,    81,    62,    -1,    25,     1,   138,
      -1,    25,     1,    62,    -1,    -1,    12,     4,    -1,    -1,
      90,    91,    82,    -1,   110,    -1,   103,    -1,   106,    -1,
     107,    -1,   137,    -1,   136,    -1,    86,     1,    -1,   110,
      -1,   137,    -1,   136,    -1,    86,     1,    -1,    11,    -1,
      56,    -1,    12,    81,    -1,    -1,    15,    81,   130,    81,
      -1,    63,    81,    94,    81,    95,    64,    81,    -1,    96,
      -1,    97,    81,    33,    81,    96,    -1,    -1,    33,    81,
      97,    -1,    -1,    31,    -1,    32,    -1,    97,    -1,    99,
      81,   105,    98,    -1,    -1,   102,    -1,   100,    -1,   102,
      65,    81,   100,    -1,   102,     1,    -1,    22,    81,   102,
      60,    81,   104,    61,    -1,    22,    81,    60,    81,   104,
      61,    -1,    -1,   104,    92,    81,    -1,    12,    81,    -1,
      21,     1,   138,    -1,    21,     1,    62,    -1,    23,     1,
     138,    -1,    23,     1,    62,    -1,    66,    81,    -1,    67,
      81,    -1,    68,    81,    -1,    -1,    69,    -1,    66,    -1,
     111,   125,    -1,   112,    -1,   111,    65,    81,   112,    -1,
     111,     1,    -1,   114,    -1,   112,   108,   114,    -1,   112,
       1,    -1,    19,    -1,    18,    19,    -1,    12,    19,    -1,
     116,    81,    -1,   116,   117,    81,    -1,   117,    81,    -1,
     113,   116,    81,    -1,   113,   116,   117,    81,    -1,   113,
     117,    81,    -1,   116,    81,    -1,   118,    81,    -1,   113,
     116,    81,    -1,   113,   118,    81,    -1,    12,    -1,    18,
      -1,   118,    -1,   117,   118,    -1,   117,     1,    -1,    14,
      -1,   119,    -1,   121,    -1,   124,    -1,    16,    12,    -1,
      12,    81,    -1,    17,    81,   120,    70,    -1,    17,    81,
     120,   122,    81,   123,    81,    70,    -1,    17,    81,   113,
     120,    70,    -1,    17,    81,   113,   120,   122,    81,   123,
      81,    70,    -1,    71,    -1,     6,    -1,     7,    -1,     8,
      -1,     9,    -1,    10,    -1,    12,    -1,    11,    -1,    15,
      12,    -1,    15,    15,    12,    -1,    15,    57,    13,    64,
      -1,    15,    57,    55,    64,    -1,    15,    57,    12,    64,
      -1,    15,    57,    11,    64,    -1,    15,    58,    81,   115,
      64,    -1,    60,    81,   127,    61,    -1,    60,    81,     1,
      61,    -1,    60,    81,   126,    61,    -1,    60,    81,   126,
     127,    61,    -1,    60,    81,   126,     1,    61,    -1,   127,
      62,    81,    -1,     1,    62,    81,    -1,   126,   127,    62,
      81,    -1,   126,     1,    62,    81,    -1,   128,    15,    81,
     130,   129,    -1,     1,   138,    -1,    12,    81,    -1,    30,
      81,    -1,    -1,   132,    -1,   130,   131,   132,    -1,    72,
      81,    -1,    65,    81,    -1,    -1,   133,    -1,   109,   133,
      -1,    52,    81,    -1,    11,    81,    -1,    12,    81,    -1,
      56,    81,    -1,    59,    81,    -1,   135,    -1,   134,    -1,
      55,    81,    -1,    54,    81,    -1,    53,    81,    -1,    37,
      81,    -1,    38,    81,    -1,    39,    81,    -1,    40,    81,
      -1,    41,    81,    -1,    42,    81,    -1,    43,    81,    -1,
      44,    81,    -1,    45,    81,    -1,    46,    81,    -1,    47,
      81,    -1,    48,    81,    -1,    49,    81,    -1,    35,    81,
      -1,    34,    81,    -1,    36,    81,    -1,    50,    81,    -1,
      51,    81,    -1,    57,    81,   130,    64,    81,    -1,    57,
      81,     1,    -1,    14,    81,    -1,    73,     1,   138,    -1,
      73,     1,    62,    -1,     1,   138,    -1,    60,     1,   139,
       1,    61,    -1,    60,     1,    61,    -1,   138,    -1,   139,
       1,   138,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   292,   292,   293,   294,   295,   296,   300,   301,   305,
     312,   318,   343,   350,   351,   354,   356,   357,   360,   362,
     365,   376,   379,   384,   386,   397,   407,   410,   416,   417,
     421,   429,   430,   434,   435,   438,   440,   451,   452,   453,
     454,   455,   456,   457,   461,   462,   463,   464,   468,   469,
     473,   479,   482,   488,   494,   498,   505,   508,   514,   517,
     520,   526,   529,   535,   538,   543,   547,   552,   559,   570,
     582,   583,   593,   615,   618,   624,   627,   633,   634,   635,
     636,   640,   641,   645,   667,   680,   698,   707,   710,   724,
     731,   732,   733,   737,   741,   746,   751,   758,   767,   779,
     783,   788,   795,   807,   823,   829,   832,   842,   849,   855,
     856,   857,   861,   870,   894,   899,   905,   913,   925,   928,
     931,   934,   937,   940,   946,   947,   951,   957,   963,   970,
     977,   984,   991,  1000,  1003,  1006,  1009,  1014,  1020,  1024,
    1027,  1032,  1038,  1060,  1066,  1073,  1074,  1078,  1082,  1098,
    1101,  1104,  1110,  1111,  1113,  1114,  1115,  1121,  1122,  1123,
    1125,  1131,  1132,  1133,  1134,  1135,  1136,  1137,  1138,  1139,
    1140,  1141,  1142,  1143,  1144,  1145,  1146,  1147,  1148,  1149,
    1150,  1151,  1156,  1164,  1180,  1187,  1193,  1202,  1228,  1229,
    1233,  1234
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
  "KHTML_RULE_SYM", "KHTML_DECLS_SYM", "KHTML_VALUE_SYM",
  "KHTML_MEDIAQUERY_SYM", "IMPORTANT_SYM", "MEDIA_ONLY", "MEDIA_NOT",
  "MEDIA_AND", "QEMS", "EMS", "EXS", "PXS", "CMS", "MMS", "INS", "PTS",
  "PCS", "DEGS", "RADS", "GRADS", "MSECS", "SECS", "HERZ", "KHERZ", "DPI",
  "DPCM", "DIMEN", "PERCENTAGE", "FLOAT", "INTEGER", "URI", "FUNCTION",
  "NOTFUNCTION", "UNICODERANGE", "'{'", "'}'", "';'", "'('", "')'", "','",
  "'+'", "'~'", "'>'", "'-'", "']'", "'='", "'/'", "'@'", "$accept",
  "stylesheet", "ruleset_or_import", "khtml_rule", "khtml_decls",
  "khtml_value", "khtml_mediaquery", "maybe_space", "maybe_sgml",
  "maybe_charset", "charset", "import_list", "import", "namespace_list",
  "namespace", "maybe_ns_prefix", "rule_list", "rule", "safe_ruleset",
  "string_or_uri", "media_feature", "maybe_media_value", "media_query_exp",
  "media_query_exp_list", "maybe_and_media_query_exp_list",
  "maybe_media_restrictor", "media_query", "maybe_media_list",
  "media_list", "media", "ruleset_list", "medium", "page", "font_face",
  "combinator", "unary_operator", "ruleset", "selector_list", "selector",
  "namespace_selector", "simple_selector", "simple_css3_selector",
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
     300,   301,   302,   303,   304,   305,   306,   307,   308,   309,
     123,   125,    59,    40,    41,    44,    43,   126,    62,    45,
      93,    61,    47,    64
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    74,    75,    75,    75,    75,    75,    76,    76,    77,
      78,    79,    80,    81,    81,    82,    82,    82,    83,    83,
      84,    84,    84,    85,    85,    86,    86,    86,    87,    87,
      88,    88,    88,    89,    89,    90,    90,    91,    91,    91,
      91,    91,    91,    91,    92,    92,    92,    92,    93,    93,
      94,    95,    95,    96,    97,    97,    98,    98,    99,    99,
      99,   100,   100,   101,   101,   102,   102,   102,   103,   103,
     104,   104,   105,   106,   106,   107,   107,   108,   108,   108,
     108,   109,   109,   110,   111,   111,   111,   112,   112,   112,
     113,   113,   113,   114,   114,   114,   114,   114,   114,   115,
     115,   115,   115,   116,   116,   117,   117,   117,   118,   118,
     118,   118,   119,   120,   121,   121,   121,   121,   122,   122,
     122,   122,   122,   122,   123,   123,   124,   124,   124,   124,
     124,   124,   124,   125,   125,   125,   125,   125,   126,   126,
     126,   126,   127,   127,   128,   129,   129,   130,   130,   131,
     131,   131,   132,   132,   132,   132,   132,   132,   132,   132,
     132,   133,   133,   133,   133,   133,   133,   133,   133,   133,
     133,   133,   133,   133,   133,   133,   133,   133,   133,   133,
     133,   133,   134,   134,   135,   136,   136,   137,   138,   138,
     139,   139
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     5,     2,     2,     2,     2,     1,     1,     6,
       2,     5,     5,     0,     2,     0,     2,     2,     0,     1,
       5,     3,     3,     0,     3,     6,     3,     3,     0,     3,
       6,     3,     3,     0,     2,     0,     3,     1,     1,     1,
       1,     1,     1,     2,     1,     1,     1,     2,     1,     1,
       2,     0,     4,     7,     1,     5,     0,     3,     0,     1,
       1,     1,     4,     0,     1,     1,     4,     2,     7,     6,
       0,     3,     2,     3,     3,     3,     3,     2,     2,     2,
       0,     1,     1,     2,     1,     4,     2,     1,     3,     2,
       1,     2,     2,     2,     3,     2,     3,     4,     3,     2,
       2,     3,     3,     1,     1,     1,     2,     2,     1,     1,
       1,     1,     2,     2,     4,     8,     5,     9,     1,     1,
       1,     1,     1,     1,     1,     1,     2,     3,     4,     4,
       4,     4,     5,     4,     4,     4,     5,     5,     3,     3,
       4,     4,     5,     2,     2,     2,     0,     1,     3,     2,
       2,     0,     1,     2,     2,     2,     2,     2,     2,     1,
       1,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     5,     3,     2,     3,     3,     2,     5,     3,
       1,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
      18,     0,     0,     0,     0,     0,     0,    13,    13,    13,
      13,    15,    19,     0,     0,    13,    13,    10,    13,    13,
       1,     3,     4,     5,     6,    23,     0,    22,    21,    14,
      13,     0,     0,     0,    58,    17,    16,    28,     0,     0,
     103,   108,     0,     0,    13,   104,    90,     0,    13,     8,
       7,     0,     0,     0,    87,    13,     0,   105,   109,   110,
     111,     0,    13,     0,     0,     0,    13,    13,    13,    13,
      13,    13,    13,    13,    13,    13,    13,    13,    13,    13,
      13,    13,    13,    13,    13,    13,    13,    13,    13,    13,
      13,    13,    13,    13,    82,    81,     0,   151,   147,   152,
     160,   159,    59,    60,    13,    54,    61,    13,     0,    15,
      35,   189,   190,     0,    20,    92,   126,     0,     0,    13,
     112,     0,    91,     0,     0,     0,    86,    13,    83,    89,
      13,    13,    13,     0,   103,   104,    13,     0,    93,     0,
     107,    95,   106,   134,    13,   143,   144,     0,   135,     0,
     133,    13,    13,   155,   156,   184,   178,   177,   179,   164,
     165,   166,   167,   168,   169,   170,   171,   172,   173,   174,
     175,   176,   180,   181,   154,   163,   162,   161,   157,     0,
     158,   153,    11,    13,    13,     0,     0,     0,     0,    12,
      24,     0,    15,     0,     0,   127,     0,     0,     0,     0,
       0,    13,     0,     0,     0,    27,    26,    48,    49,    13,
       9,     0,    77,    78,    79,    88,    96,     0,    98,    94,
     139,   137,    13,   136,    13,   138,     0,   183,   151,   150,
     149,   148,    13,    13,    13,    13,    56,     0,    33,    29,
       0,     0,    13,     0,     0,     0,    15,    38,    39,    40,
      37,    42,    41,   188,   191,   131,   130,   128,   129,     0,
       0,    13,    13,   113,    13,     0,   119,   120,   121,   122,
     123,   114,   118,    13,    58,     0,    97,   141,   140,   151,
      13,    50,    51,     0,    72,    13,    62,    32,    31,     0,
       0,   187,     0,    58,     0,     0,    43,    36,    13,    13,
     132,    99,   100,   116,    13,     0,    65,     0,     0,    13,
     142,   182,    13,     0,    55,     0,    34,    13,    74,    73,
      13,     0,    76,    75,   186,   185,   101,   102,     0,   125,
     124,    13,    25,    67,    13,   145,     0,    13,    57,     0,
      70,    13,    13,     0,    58,   151,    53,    30,     0,    70,
       0,   115,    66,    52,    69,     0,    13,    44,    46,    45,
       0,   117,    47,    71,    68
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     6,    48,     7,     8,     9,    10,   187,    25,    11,
      12,    37,   355,   110,   192,   290,   193,   246,   356,   209,
     233,   313,   105,   106,   286,   107,   306,   307,   308,   247,
     348,   236,   248,   249,   133,    96,   357,    51,    52,    53,
      54,   260,    55,    56,    57,    58,   204,    59,   273,   331,
      60,    17,    63,    64,    65,   310,    97,   185,    98,    99,
     100,   101,   358,   359,   145,   113
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -175
static const yytype_int16 yypact[] =
{
     325,    31,   -26,   -16,   -12,    47,    57,  -175,  -175,  -175,
    -175,  -175,  -175,     0,   207,  -175,  -175,  -175,  -175,  -175,
    -175,   120,   120,   120,   120,   305,   132,  -175,  -175,  -175,
    -175,   520,    19,   462,   136,  -175,  -175,   117,   274,   119,
     139,  -175,   127,   137,  -175,   142,  -175,   213,  -175,  -175,
    -175,    40,   415,   271,  -175,   259,   358,  -175,  -175,  -175,
    -175,   220,  -175,    37,   270,   140,  -175,  -175,  -175,  -175,
    -175,  -175,  -175,  -175,  -175,  -175,  -175,  -175,  -175,  -175,
    -175,  -175,  -175,  -175,  -175,  -175,  -175,  -175,  -175,  -175,
    -175,  -175,  -175,  -175,  -175,  -175,   414,   165,  -175,  -175,
    -175,  -175,  -175,  -175,  -175,  -175,    20,  -175,   116,  -175,
     149,  -175,  -175,   186,  -175,  -175,  -175,   177,   167,  -175,
    -175,    21,  -175,   193,   130,    46,  -175,  -175,  -175,  -175,
    -175,  -175,  -175,   392,  -175,  -175,   259,   358,   120,   358,
    -175,   120,  -175,  -175,  -175,  -175,   120,   234,  -175,   276,
    -175,  -175,  -175,   120,   120,   120,   120,   120,   120,   120,
     120,   120,   120,   120,   120,   120,   120,   120,   120,   120,
     120,   120,   120,   120,   120,   120,   120,   120,   120,   346,
     120,  -175,  -175,  -175,  -175,   511,   160,    23,   194,  -175,
     305,   208,  -175,   228,   282,  -175,   161,   163,   175,   188,
     567,   139,   142,   201,   187,  -175,  -175,  -175,  -175,  -175,
    -175,   567,   120,   120,   120,  -175,   120,   358,   120,   120,
     120,  -175,  -175,  -175,  -175,   120,   462,  -175,    82,   120,
     120,  -175,  -175,  -175,  -175,  -175,   223,   240,   250,   305,
     199,   269,  -175,   290,   298,   304,  -175,  -175,  -175,  -175,
    -175,  -175,  -175,  -175,  -175,  -175,  -175,  -175,  -175,   271,
     264,  -175,  -175,   120,  -175,   195,  -175,  -175,  -175,  -175,
    -175,  -175,  -175,  -175,   113,   427,   120,   120,   120,   206,
    -175,   120,    22,    59,   120,  -175,  -175,  -175,  -175,   273,
       2,  -175,   244,   128,   253,   267,  -175,   305,  -175,  -175,
    -175,   120,   120,  -175,  -175,   159,  -175,   252,    63,  -175,
    -175,   120,  -175,   275,  -175,    59,  -175,  -175,  -175,  -175,
    -175,    44,  -175,  -175,  -175,  -175,   120,   120,   159,  -175,
    -175,  -175,  -175,  -175,  -175,   120,   462,  -175,    20,   148,
     120,  -175,  -175,    24,   136,    55,   120,  -175,    96,   120,
      26,  -175,  -175,   120,  -175,   316,  -175,  -175,  -175,  -175,
     306,  -175,  -175,   120,  -175
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -175,  -175,  -175,  -175,  -175,  -175,  -175,    -1,   -93,  -175,
    -175,  -175,   -27,  -175,  -175,  -175,  -175,  -175,  -175,    56,
    -175,  -175,    62,    33,  -175,  -175,   -32,  -175,    68,  -175,
       7,  -175,  -175,  -175,  -175,  -175,   -28,  -175,   152,   -99,
     231,  -175,   -52,   -34,   -44,  -175,   162,  -175,   101,    41,
    -175,   317,  -175,   308,  -175,  -175,  -174,  -175,   229,   321,
    -175,  -175,   185,   226,    -2,  -175
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -147
static const yytype_int16 yytable[] =
{
      14,   136,   108,    50,    49,   228,    21,    22,    23,    24,
     109,    28,   142,   207,    31,    32,   190,    33,    34,   137,
      61,   139,   203,    29,   -13,    29,    29,    29,    29,    39,
      29,    62,    13,   201,    15,   -13,   112,   312,   147,   202,
      46,   126,   -13,   121,    16,   333,   124,   125,    18,    62,
      29,    19,   279,   -13,   138,   141,   234,    20,   208,   -13,
      26,   146,    27,    29,   333,   153,   154,   155,   156,   157,
     158,   159,   160,   161,   162,   163,   164,   165,   166,   167,
     168,   169,   170,   171,   172,   173,   174,   175,   176,   177,
     178,   179,   180,   142,   351,   142,   361,   240,   148,   239,
      16,   259,   217,   186,   341,   127,   188,   210,    40,   334,
      41,    42,    43,    44,    45,    46,    47,    29,   200,   -13,
     183,   206,   104,    29,    29,   -64,   211,   184,   334,   212,
     213,   214,    29,    38,    29,   216,   218,    47,   219,   116,
      29,   207,   117,   220,   102,   103,   280,   183,   261,   120,
     225,   226,    29,   297,   184,   152,   262,   354,   115,   102,
     103,   122,   345,    29,    29,   250,   245,   102,   103,   244,
     329,   330,   232,   142,   191,   -63,   104,   189,   196,   197,
     198,   114,   229,   230,   118,   119,   208,   194,   320,   195,
     238,   104,   254,   266,   267,   268,   269,   270,    29,   104,
     263,   266,   267,   268,   269,   270,   235,   298,   274,   237,
     347,    29,   -13,   264,   123,   299,   276,   -13,    30,   -13,
     -13,   277,   199,   278,   -13,   255,   182,   256,    -2,   240,
     183,   281,   282,   283,   284,   288,   309,   184,   291,   257,
      40,   293,    41,    42,    43,    44,    45,    46,    47,   241,
     242,   243,   258,    26,    29,   205,   285,   271,   272,    26,
     301,   302,   289,   263,   -13,   303,   272,  -146,  -146,   -13,
     292,   183,   305,    41,    42,    43,    44,   316,   184,   311,
      26,   143,   144,   134,   315,    41,    42,    43,    44,   135,
     319,   294,   323,   325,    26,   221,   222,   326,   327,   295,
      26,   244,   287,   328,    26,   296,   318,   240,   335,    35,
      36,   336,   352,    26,   332,   322,   339,   362,    40,   340,
      41,    42,    43,    44,    45,    46,    47,    26,   300,   324,
     343,   150,   151,   344,    26,   111,   346,   223,   224,   337,
     349,   350,    26,   253,   353,   314,   317,   227,   338,     1,
      29,     2,     3,     4,     5,   363,   360,    66,    67,   140,
      68,   321,   -13,   275,   215,   265,   304,   364,   128,   342,
     -13,   149,    41,    42,    43,    44,   -13,   -13,   251,   244,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    40,    93,    41,    42,    43,    44,
      45,    46,    94,     0,   231,    95,   129,   181,   -13,   252,
       0,     0,     0,   -13,   -13,   -13,   -13,   -80,   129,   -80,
     -80,   -80,   -80,   -80,   -80,     0,     0,     0,     0,   -80,
       0,   -80,   -80,   -80,   -80,   -80,   -80,     0,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    29,    88,    89,    90,
       0,     0,     0,    66,    67,   -84,    68,     0,     0,     0,
     -84,   130,   131,   132,     0,     0,     0,   -85,     0,     0,
       0,     0,   -85,   130,   131,   132,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
       0,    93,    66,    67,    29,    68,     0,     0,    94,     0,
       0,    95,    40,     0,    41,    42,    43,    44,    45,    46,
      47,     0,     0,     0,     0,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,     0,
      93,    29,     0,     0,     0,     0,     0,    94,     0,    40,
      95,    41,    42,    43,    44,    45,    46
};

static const yytype_int16 yycheck[] =
{
       1,    53,    34,    31,    31,   179,     7,     8,     9,    10,
      37,    13,    56,    11,    15,    16,   109,    18,    19,    53,
       1,    55,   121,     4,     4,     4,     4,     4,     4,    30,
       4,    12,     1,    12,    60,     4,    38,    15,     1,    18,
      19,     1,    11,    44,    60,     1,    47,    48,    60,    12,
       4,     4,   226,    33,    55,    56,    33,     0,    56,     4,
      60,    62,    62,     4,     1,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,   137,    70,   139,    70,     1,    61,   192,
      60,   200,   136,   104,    60,    65,   107,    61,    12,    65,
      14,    15,    16,    17,    18,    19,    20,     4,   119,    64,
      65,   123,    63,     4,     4,    62,   127,    72,    65,   130,
     131,   132,     4,     1,     4,   136,   137,    20,   139,    12,
       4,    11,    15,   144,    31,    32,    64,    65,   200,    12,
     151,   152,     4,   246,    72,    15,   200,    61,    19,    31,
      32,    19,   336,     4,     4,   193,   193,    31,    32,    73,
      11,    12,    12,   217,    25,    62,    63,    61,    11,    12,
      13,    62,   183,   184,    57,    58,    56,     1,    60,    12,
     191,    63,   194,     6,     7,     8,     9,    10,     4,    63,
     201,     6,     7,     8,     9,    10,    12,   259,   209,     1,
      62,     4,     4,    12,     1,   259,   217,     4,    11,    11,
      12,   222,    55,   224,    11,    64,    61,    64,     0,     1,
      65,   232,   233,   234,   235,   237,    30,    72,   240,    64,
      12,   242,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    64,    60,     4,    62,    33,    70,    71,    60,
     261,   262,    12,   264,    56,    70,    71,    61,    62,    56,
       1,    65,   273,    14,    15,    16,    17,     4,    72,   280,
      60,    61,    62,    12,   285,    14,    15,    16,    17,    18,
     292,     1,   294,   295,    60,    61,    62,   298,   299,     1,
      60,    73,    62,   304,    60,     1,    62,     1,   309,     4,
       5,   312,   344,    60,    62,    62,   317,     1,    12,   320,
      14,    15,    16,    17,    18,    19,    20,    60,    64,    62,
     331,    61,    62,   334,    60,    61,   337,    61,    62,    64,
     341,   342,    60,    61,   345,   283,   290,     1,   315,    24,
       4,    26,    27,    28,    29,   356,   349,    11,    12,     1,
      14,   293,     4,   211,   133,   203,   265,    61,    51,   328,
      12,    63,    14,    15,    16,    17,    18,    19,   193,    73,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    12,    59,    14,    15,    16,    17,
      18,    19,    66,    -1,   185,    69,     1,    96,    60,   193,
      -1,    -1,    -1,    65,    66,    67,    68,    12,     1,    14,
      15,    16,    17,    18,    19,    -1,    -1,    -1,    -1,    12,
      -1,    14,    15,    16,    17,    18,    19,    -1,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,     4,    53,    54,    55,
      -1,    -1,    -1,    11,    12,    60,    14,    -1,    -1,    -1,
      65,    66,    67,    68,    -1,    -1,    -1,    60,    -1,    -1,
      -1,    -1,    65,    66,    67,    68,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      -1,    59,    11,    12,     4,    14,    -1,    -1,    66,    -1,
      -1,    69,    12,    -1,    14,    15,    16,    17,    18,    19,
      20,    -1,    -1,    -1,    -1,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    -1,
      59,     4,    -1,    -1,    -1,    -1,    -1,    66,    -1,    12,
      69,    14,    15,    16,    17,    18,    19
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    24,    26,    27,    28,    29,    75,    77,    78,    79,
      80,    83,    84,     1,    81,    60,    60,   125,    60,     4,
       0,    81,    81,    81,    81,    82,    60,    62,   138,     4,
      11,    81,    81,    81,    81,     4,     5,    85,     1,    81,
      12,    14,    15,    16,    17,    18,    19,    20,    76,    86,
     110,   111,   112,   113,   114,   116,   117,   118,   119,   121,
     124,     1,    12,   126,   127,   128,    11,    12,    14,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    59,    66,    69,   109,   130,   132,   133,
     134,   135,    31,    32,    63,    96,    97,    99,   100,    86,
      87,    61,   138,   139,    62,    19,    12,    15,    57,    58,
      12,    81,    19,     1,    81,    81,     1,    65,   125,     1,
      66,    67,    68,   108,    12,    18,   116,   117,    81,   117,
       1,    81,   118,    61,    62,   138,    81,     1,    61,   127,
      61,    62,    15,    81,    81,    81,    81,    81,    81,    81,
      81,    81,    81,    81,    81,    81,    81,    81,    81,    81,
      81,    81,    81,    81,    81,    81,    81,    81,    81,    81,
      81,   133,    61,    65,    72,   131,    81,    81,    81,    61,
      82,    25,    88,    90,     1,    12,    11,    12,    13,    55,
      81,    12,    18,   113,   120,    62,   138,    11,    56,    93,
      61,    81,    81,    81,    81,   114,    81,   117,    81,    81,
      81,    61,    62,    61,    62,    81,    81,     1,   130,    81,
      81,   132,    12,    94,    33,    12,   105,     1,    81,    82,
       1,    21,    22,    23,    73,    86,    91,   103,   106,   107,
     110,   136,   137,    61,   138,    64,    64,    64,    64,   113,
     115,   116,   118,    81,    12,   120,     6,     7,     8,     9,
      10,    70,    71,   122,    81,   112,    81,    81,    81,   130,
      64,    81,    81,    81,    81,    33,    98,    62,   138,    12,
      89,   138,     1,    81,     1,     1,     1,    82,   116,   118,
      64,    81,    81,    70,   122,    81,   100,   101,   102,    30,
     129,    81,    15,    95,    96,    81,     4,    93,    62,   138,
      60,   102,    62,   138,    62,   138,    81,    81,    81,    11,
      12,   123,    62,     1,    65,    81,    81,    64,    97,    81,
      81,    60,   123,    81,    81,   130,    81,    62,   104,    81,
      81,    70,   100,    81,    61,    86,    92,   110,   136,   137,
     104,    70,     1,    81,    61
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
      case 97: /* "media_query_exp_list" */

	{ delete (yyvaluep->mediaQueryExpList); (yyvaluep->mediaQueryExpList) = 0; };

	break;
      case 98: /* "maybe_and_media_query_exp_list" */

	{ delete (yyvaluep->mediaQueryExpList); (yyvaluep->mediaQueryExpList) = 0; };

	break;
      case 101: /* "maybe_media_list" */

	{ delete (yyvaluep->mediaList); (yyvaluep->mediaList) = 0; };

	break;
      case 102: /* "media_list" */

	{ delete (yyvaluep->mediaList); (yyvaluep->mediaList) = 0; };

	break;
      case 104: /* "ruleset_list" */

	{ delete (yyvaluep->ruleList); (yyvaluep->ruleList) = 0; };

	break;
      case 111: /* "selector_list" */

	{ if ((yyvaluep->selectorList)) qDeleteAll(*(yyvaluep->selectorList)); delete (yyvaluep->selectorList); (yyvaluep->selectorList) = 0; };

	break;
      case 112: /* "selector" */

	{ delete (yyvaluep->selector); (yyvaluep->selector) = 0; };

	break;
      case 114: /* "simple_selector" */

	{ delete (yyvaluep->selector); (yyvaluep->selector) = 0; };

	break;
      case 115: /* "simple_css3_selector" */

	{ delete (yyvaluep->selector); (yyvaluep->selector) = 0; };

	break;
      case 117: /* "specifier_list" */

	{ delete (yyvaluep->selector); (yyvaluep->selector) = 0; };

	break;
      case 118: /* "specifier" */

	{ delete (yyvaluep->selector); (yyvaluep->selector) = 0; };

	break;
      case 119: /* "class" */

	{ delete (yyvaluep->selector); (yyvaluep->selector) = 0; };

	break;
      case 121: /* "attrib" */

	{ delete (yyvaluep->selector); (yyvaluep->selector) = 0; };

	break;
      case 124: /* "pseudo" */

	{ delete (yyvaluep->selector); (yyvaluep->selector) = 0; };

	break;
      case 130: /* "expr" */

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
        case 9:

    {
        CSSParser *p = static_cast<CSSParser *>(parser);
	p->rule = (yyvsp[(4) - (6)].rule);
    ;}
    break;

  case 10:

    {
	/* can be empty */
    ;}
    break;

  case 11:

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

  case 12:

    {
         CSSParser *p = static_cast<CSSParser *>(parser);
         p->mediaQuery = (yyvsp[(4) - (5)].mediaQuery);
     ;}
    break;

  case 20:

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

  case 21:

    {
      (yyval.rule) = 0;
 ;}
    break;

  case 22:

    {
      (yyval.rule) = 0;
 ;}
    break;

  case 24:

    {
     CSSParser *p = static_cast<CSSParser *>(parser);
     if ( (yyvsp[(2) - (3)].rule) && p->styleElement && p->styleElement->isCSSStyleSheet() ) {
	 p->styleElement->append( (yyvsp[(2) - (3)].rule) );
     } else {
	 delete (yyvsp[(2) - (3)].rule);
     }
 ;}
    break;

  case 25:

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

  case 26:

    {
        (yyval.rule) = 0;
    ;}
    break;

  case 27:

    {
        (yyval.rule) = 0;
    ;}
    break;

  case 30:

    {
#ifdef CSS_DEBUG
    kDebug( 6080 ) << "@namespace: " << qString((yyvsp[(4) - (6)].string));
#endif
      CSSParser *p = static_cast<CSSParser *>(parser);
    if (p->styleElement && p->styleElement->isCSSStyleSheet())
        static_cast<CSSStyleSheetImpl*>(p->styleElement)->addNamespace(p, domString((yyvsp[(3) - (6)].string)), domString((yyvsp[(4) - (6)].string)));
 ;}
    break;

  case 33:

    { (yyval.string).string = 0; ;}
    break;

  case 34:

    { (yyval.string) = (yyvsp[(1) - (2)].string); ;}
    break;

  case 36:

    {
     CSSParser *p = static_cast<CSSParser *>(parser);
     if ( (yyvsp[(2) - (3)].rule) && p->styleElement && p->styleElement->isCSSStyleSheet() ) {
	 p->styleElement->append( (yyvsp[(2) - (3)].rule) );
     } else {
	 delete (yyvsp[(2) - (3)].rule);
     }
 ;}
    break;

  case 43:

    { delete (yyvsp[(1) - (2)].rule); (yyval.rule) = 0; ;}
    break;

  case 47:

    { delete (yyvsp[(1) - (2)].rule); (yyval.rule) = 0; ;}
    break;

  case 50:

    {
        (yyval.string) = (yyvsp[(1) - (2)].string);
    ;}
    break;

  case 51:

    {
        (yyval.valueList) = 0;
    ;}
    break;

  case 52:

    {
        (yyval.valueList) = (yyvsp[(3) - (4)].valueList);
    ;}
    break;

  case 53:

    {
        (yyval.mediaQueryExp) = new khtml::MediaQueryExp(domString((yyvsp[(3) - (7)].string)).lower(), (yyvsp[(5) - (7)].valueList));
    ;}
    break;

  case 54:

    {
      (yyval.mediaQueryExpList) =  new QList<khtml::MediaQueryExp*>;
      (yyval.mediaQueryExpList)->append((yyvsp[(1) - (1)].mediaQueryExp));
    ;}
    break;

  case 55:

    {
      (yyval.mediaQueryExpList) = (yyvsp[(1) - (5)].mediaQueryExpList);
      (yyval.mediaQueryExpList)->append((yyvsp[(5) - (5)].mediaQueryExp));
    ;}
    break;

  case 56:

    {
        (yyval.mediaQueryExpList) = new QList<khtml::MediaQueryExp*>;
    ;}
    break;

  case 57:

    {
        (yyval.mediaQueryExpList) = (yyvsp[(3) - (3)].mediaQueryExpList);
    ;}
    break;

  case 58:

    {
        (yyval.mediaQueryRestrictor) = khtml::MediaQuery::None;
    ;}
    break;

  case 59:

    {
        (yyval.mediaQueryRestrictor) = khtml::MediaQuery::Only;
    ;}
    break;

  case 60:

    {
        (yyval.mediaQueryRestrictor) = khtml::MediaQuery::Not;
    ;}
    break;

  case 61:

    {
        (yyval.mediaQuery) = new khtml::MediaQuery(khtml::MediaQuery::None, "all", (yyvsp[(1) - (1)].mediaQueryExpList));
    ;}
    break;

  case 62:

    {
        (yyval.mediaQuery) = new khtml::MediaQuery((yyvsp[(1) - (4)].mediaQueryRestrictor), domString((yyvsp[(3) - (4)].string)).lower(), (yyvsp[(4) - (4)].mediaQueryExpList));
    ;}
    break;

  case 63:

    {
	(yyval.mediaList) = new MediaListImpl();
    ;}
    break;

  case 65:

    {
        (yyval.mediaList) = new MediaListImpl();
        (yyval.mediaList)->appendMediaQuery((yyvsp[(1) - (1)].mediaQuery));
    ;}
    break;

  case 66:

    {
	(yyval.mediaList) = (yyvsp[(1) - (4)].mediaList);
	if ((yyval.mediaList))
	    (yyval.mediaList)->appendMediaQuery( (yyvsp[(4) - (4)].mediaQuery) );
    ;}
    break;

  case 67:

    {
       delete (yyvsp[(1) - (2)].mediaList);
       (yyval.mediaList) = 0;
    ;}
    break;

  case 68:

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

  case 69:

    {
        CSSParser *p = static_cast<CSSParser *>(parser);
        if ((yyvsp[(5) - (6)].ruleList) && p->styleElement && p->styleElement->isCSSStyleSheet() ) {
            (yyval.rule) = new CSSMediaRuleImpl( static_cast<CSSStyleSheetImpl*>(p->styleElement), 0, (yyvsp[(5) - (6)].ruleList));
        } else {
            (yyval.rule) = 0;
            delete (yyvsp[(5) - (6)].ruleList);
        }
    ;}
    break;

  case 70:

    { (yyval.ruleList) = 0; ;}
    break;

  case 71:

    {
      (yyval.ruleList) = (yyvsp[(1) - (3)].ruleList);
      if ( (yyvsp[(2) - (3)].rule) ) {
	  if ( !(yyval.ruleList) ) (yyval.ruleList) = new CSSRuleListImpl();
	  (yyval.ruleList)->append( (yyvsp[(2) - (3)].rule) );
      }
  ;}
    break;

  case 72:

    {
      (yyval.string) = (yyvsp[(1) - (2)].string);
  ;}
    break;

  case 73:

    {
      (yyval.rule) = 0;
    ;}
    break;

  case 74:

    {
      (yyval.rule) = 0;
    ;}
    break;

  case 75:

    {
      (yyval.rule) = 0;
    ;}
    break;

  case 76:

    {
      (yyval.rule) = 0;
    ;}
    break;

  case 77:

    { (yyval.relation) = CSSSelector::DirectAdjacent; ;}
    break;

  case 78:

    { (yyval.relation) = CSSSelector::IndirectAdjacent; ;}
    break;

  case 79:

    { (yyval.relation) = CSSSelector::Child; ;}
    break;

  case 80:

    { (yyval.relation) = CSSSelector::Descendant; ;}
    break;

  case 81:

    { (yyval.val) = -1; ;}
    break;

  case 82:

    { (yyval.val) = 1; ;}
    break;

  case 83:

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

  case 84:

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

  case 85:

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

  case 86:

    {
        if ((yyvsp[(1) - (2)].selectorList)) qDeleteAll(*(yyvsp[(1) - (2)].selectorList));
	delete (yyvsp[(1) - (2)].selectorList);
	(yyvsp[(1) - (2)].selectorList) = 0;
	(yyval.selectorList) = 0;
    ;}
    break;

  case 87:

    {
	(yyval.selector) = (yyvsp[(1) - (1)].selector);
    ;}
    break;

  case 88:

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

  case 89:

    {
	delete (yyvsp[(1) - (2)].selector);
	(yyval.selector) = 0;
    ;}
    break;

  case 90:

    { (yyval.string).string = 0; (yyval.string).length = 0; ;}
    break;

  case 91:

    { static unsigned short star = '*'; (yyval.string).string = &star; (yyval.string).length = 1; ;}
    break;

  case 92:

    { (yyval.string) = (yyvsp[(1) - (2)].string); ;}
    break;

  case 93:

    {
	(yyval.selector) = new CSSSelector();
	(yyval.selector)->tag = (yyvsp[(1) - (2)].element);
    ;}
    break;

  case 94:

    {
	(yyval.selector) = (yyvsp[(2) - (3)].selector);
        if ( (yyval.selector) )
	    (yyval.selector)->tag = (yyvsp[(1) - (3)].element);
    ;}
    break;

  case 95:

    {
	(yyval.selector) = (yyvsp[(1) - (2)].selector);
        if ( (yyval.selector) )
            (yyval.selector)->tag = makeId(static_cast<CSSParser*>(parser)->defaultNamespace(), anyLocalName);
    ;}
    break;

  case 96:

    {
        (yyval.selector) = new CSSSelector();
        (yyval.selector)->tag = (yyvsp[(2) - (3)].element);
	CSSParser *p = static_cast<CSSParser *>(parser);
        if (p->styleElement && p->styleElement->isCSSStyleSheet())
            static_cast<CSSStyleSheetImpl*>(p->styleElement)->determineNamespace((yyval.selector)->tag, domString((yyvsp[(1) - (3)].string)));
    ;}
    break;

  case 97:

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

  case 98:

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

  case 99:

    {
	(yyval.selector) = new CSSSelector();
	(yyval.selector)->tag = (yyvsp[(1) - (2)].element);
    ;}
    break;

  case 100:

    {
	(yyval.selector) = (yyvsp[(1) - (2)].selector);
        if ( (yyval.selector) )
            (yyval.selector)->tag = makeId(static_cast<CSSParser*>(parser)->defaultNamespace(), anyLocalName);
    ;}
    break;

  case 101:

    {
        (yyval.selector) = new CSSSelector();
        (yyval.selector)->tag = (yyvsp[(2) - (3)].element);
	CSSParser *p = static_cast<CSSParser *>(parser);
        if (p->styleElement && p->styleElement->isCSSStyleSheet())
            static_cast<CSSStyleSheetImpl*>(p->styleElement)->determineNamespace((yyval.selector)->tag, domString((yyvsp[(1) - (3)].string)));
    ;}
    break;

  case 102:

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

  case 103:

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

  case 104:

    {
	(yyval.element) = makeId(static_cast<CSSParser*>(parser)->defaultNamespace(), anyLocalName);
    ;}
    break;

  case 105:

    {
	(yyval.selector) = (yyvsp[(1) - (1)].selector);
    ;}
    break;

  case 106:

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

  case 107:

    {
	delete (yyvsp[(1) - (2)].selector);
	(yyval.selector) = 0;
    ;}
    break;

  case 108:

    {
	(yyval.selector) = new CSSSelector();
	(yyval.selector)->match = CSSSelector::Id;
	(yyval.selector)->attr = ATTR_ID;
	(yyval.selector)->value = domString((yyvsp[(1) - (1)].string));
    ;}
    break;

  case 112:

    {
	(yyval.selector) = new CSSSelector();
	(yyval.selector)->match = CSSSelector::Class;
	(yyval.selector)->attr = ATTR_CLASS;
	(yyval.selector)->value = domString((yyvsp[(2) - (2)].string));
    ;}
    break;

  case 113:

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

  case 114:

    {
	(yyval.selector) = new CSSSelector();
	(yyval.selector)->attr = (yyvsp[(3) - (4)].attribute);
	(yyval.selector)->match = CSSSelector::Set;
    ;}
    break;

  case 115:

    {
	(yyval.selector) = new CSSSelector();
	(yyval.selector)->attr = (yyvsp[(3) - (8)].attribute);
	(yyval.selector)->match = (yyvsp[(4) - (8)].match);
	(yyval.selector)->value = domString((yyvsp[(6) - (8)].string));
    ;}
    break;

  case 116:

    {
        (yyval.selector) = new CSSSelector();
        (yyval.selector)->attr = (yyvsp[(4) - (5)].attribute);
        (yyval.selector)->match = CSSSelector::Set;
        CSSParser *p = static_cast<CSSParser *>(parser);
        if (p->styleElement && p->styleElement->isCSSStyleSheet())
            static_cast<CSSStyleSheetImpl*>(p->styleElement)->determineNamespace((yyval.selector)->attr, domString((yyvsp[(3) - (5)].string)));
    ;}
    break;

  case 117:

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

  case 118:

    {
	(yyval.match) = CSSSelector::Exact;
    ;}
    break;

  case 119:

    {
	(yyval.match) = CSSSelector::List;
    ;}
    break;

  case 120:

    {
	(yyval.match) = CSSSelector::Hyphen;
    ;}
    break;

  case 121:

    {
	(yyval.match) = CSSSelector::Begin;
    ;}
    break;

  case 122:

    {
	(yyval.match) = CSSSelector::End;
    ;}
    break;

  case 123:

    {
	(yyval.match) = CSSSelector::Contain;
    ;}
    break;

  case 126:

    {
	(yyval.selector) = new CSSSelector();
	(yyval.selector)->match = CSSSelector::PseudoClass;
	(yyval.selector)->value = domString((yyvsp[(2) - (2)].string));
    ;}
    break;

  case 127:

    {
	(yyval.selector) = new CSSSelector();
	(yyval.selector)->match = CSSSelector::PseudoElement;
        (yyval.selector)->value = domString((yyvsp[(3) - (3)].string));
    ;}
    break;

  case 128:

    {
        (yyval.selector) = new CSSSelector();
        (yyval.selector)->match = CSSSelector::PseudoClass;
        (yyval.selector)->string_arg = domString((yyvsp[(3) - (4)].string));
        (yyval.selector)->value = domString((yyvsp[(2) - (4)].string));
    ;}
    break;

  case 129:

    {
        (yyval.selector) = new CSSSelector();
        (yyval.selector)->match = CSSSelector::PseudoClass;
        (yyval.selector)->string_arg = QString::number((yyvsp[(3) - (4)].val));
        (yyval.selector)->value = domString((yyvsp[(2) - (4)].string));
    ;}
    break;

  case 130:

    {
        (yyval.selector) = new CSSSelector();
        (yyval.selector)->match = CSSSelector::PseudoClass;
        (yyval.selector)->string_arg = domString((yyvsp[(3) - (4)].string));
        (yyval.selector)->value = domString((yyvsp[(2) - (4)].string));
    ;}
    break;

  case 131:

    {
        (yyval.selector) = new CSSSelector();
        (yyval.selector)->match = CSSSelector::PseudoClass;
        (yyval.selector)->string_arg = domString((yyvsp[(3) - (4)].string));
        (yyval.selector)->value = domString((yyvsp[(2) - (4)].string));
    ;}
    break;

  case 132:

    {
        (yyval.selector) = new CSSSelector();
        (yyval.selector)->match = CSSSelector::PseudoClass;
        (yyval.selector)->simpleSelector = (yyvsp[(4) - (5)].selector);
        (yyval.selector)->value = domString((yyvsp[(2) - (5)].string));
    ;}
    break;

  case 133:

    {
	(yyval.ok) = (yyvsp[(3) - (4)].ok);
    ;}
    break;

  case 134:

    {
	(yyval.ok) = false;
    ;}
    break;

  case 135:

    {
	(yyval.ok) = (yyvsp[(3) - (4)].ok);
    ;}
    break;

  case 136:

    {
	(yyval.ok) = (yyvsp[(3) - (5)].ok);
	if ( (yyvsp[(4) - (5)].ok) )
	    (yyval.ok) = (yyvsp[(4) - (5)].ok);
    ;}
    break;

  case 137:

    {
	(yyval.ok) = (yyvsp[(3) - (5)].ok);
    ;}
    break;

  case 138:

    {
	(yyval.ok) = (yyvsp[(1) - (3)].ok);
    ;}
    break;

  case 139:

    {
        (yyval.ok) = false;
    ;}
    break;

  case 140:

    {
	(yyval.ok) = (yyvsp[(1) - (4)].ok);
	if ( (yyvsp[(2) - (4)].ok) )
	    (yyval.ok) = (yyvsp[(2) - (4)].ok);
    ;}
    break;

  case 141:

    {
        (yyval.ok) = (yyvsp[(1) - (4)].ok);
    ;}
    break;

  case 142:

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

  case 143:

    {
        (yyval.ok) = false;
    ;}
    break;

  case 144:

    {
	QString str = qString((yyvsp[(1) - (2)].string));
	(yyval.prop_id) = getPropertyID( str.toLower().toLatin1(), str.length() );
    ;}
    break;

  case 145:

    { (yyval.b) = true; ;}
    break;

  case 146:

    { (yyval.b) = false; ;}
    break;

  case 147:

    {
	(yyval.valueList) = new ValueList;
	(yyval.valueList)->addValue( (yyvsp[(1) - (1)].value) );
    ;}
    break;

  case 148:

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

  case 149:

    {
	(yyval.tok) = '/';
    ;}
    break;

  case 150:

    {
	(yyval.tok) = ',';
    ;}
    break;

  case 151:

    {
        (yyval.tok) = 0;
  ;}
    break;

  case 152:

    { (yyval.value) = (yyvsp[(1) - (1)].value); ;}
    break;

  case 153:

    { (yyval.value) = (yyvsp[(2) - (2)].value); (yyval.value).fValue *= (yyvsp[(1) - (2)].val); ;}
    break;

  case 154:

    { (yyval.value).id = 0; (yyval.value).string = (yyvsp[(1) - (2)].string); (yyval.value).unit = CSSPrimitiveValue::CSS_DIMENSION; ;}
    break;

  case 155:

    { (yyval.value).id = 0; (yyval.value).string = (yyvsp[(1) - (2)].string); (yyval.value).unit = CSSPrimitiveValue::CSS_STRING; ;}
    break;

  case 156:

    {
      QString str = qString( (yyvsp[(1) - (2)].string) );
      (yyval.value).id = getValueID( str.toLower().toLatin1(), str.length() );
      (yyval.value).unit = CSSPrimitiveValue::CSS_IDENT;
      (yyval.value).string = (yyvsp[(1) - (2)].string);
  ;}
    break;

  case 157:

    { (yyval.value).id = 0; (yyval.value).string = (yyvsp[(1) - (2)].string); (yyval.value).unit = CSSPrimitiveValue::CSS_URI; ;}
    break;

  case 158:

    { (yyval.value).id = 0; (yyval.value).iValue = 0; (yyval.value).unit = CSSPrimitiveValue::CSS_UNKNOWN;/* ### */ ;}
    break;

  case 159:

    { (yyval.value).id = 0; (yyval.value).string = (yyvsp[(1) - (1)].string); (yyval.value).unit = CSSPrimitiveValue::CSS_RGBCOLOR; ;}
    break;

  case 160:

    {
      (yyval.value) = (yyvsp[(1) - (1)].value);
  ;}
    break;

  case 161:

    { (yyval.value).id = 0; (yyval.value).isInt = true; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_NUMBER; ;}
    break;

  case 162:

    { (yyval.value).id = 0; (yyval.value).isInt = false; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_NUMBER; ;}
    break;

  case 163:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_PERCENTAGE; ;}
    break;

  case 164:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_PX; ;}
    break;

  case 165:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_CM; ;}
    break;

  case 166:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_MM; ;}
    break;

  case 167:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_IN; ;}
    break;

  case 168:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_PT; ;}
    break;

  case 169:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_PC; ;}
    break;

  case 170:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_DEG; ;}
    break;

  case 171:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_RAD; ;}
    break;

  case 172:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_GRAD; ;}
    break;

  case 173:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_MS; ;}
    break;

  case 174:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_S; ;}
    break;

  case 175:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_HZ; ;}
    break;

  case 176:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_KHZ; ;}
    break;

  case 177:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_EMS; ;}
    break;

  case 178:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = Value::Q_EMS; ;}
    break;

  case 179:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_EXS; ;}
    break;

  case 180:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_DPI; ;}
    break;

  case 181:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_DPCM; ;}
    break;

  case 182:

    {
      Function *f = new Function;
      f->name = (yyvsp[(1) - (5)].string);
      f->args = (yyvsp[(3) - (5)].valueList);
      (yyval.value).id = 0;
      (yyval.value).unit = Value::Function;
      (yyval.value).function = f;
  ;}
    break;

  case 183:

    {
      Function *f = new Function;
      f->name = (yyvsp[(1) - (3)].string);
      f->args = 0;
      (yyval.value).id = 0;
      (yyval.value).unit = Value::Function;
      (yyval.value).function = f;
  ;}
    break;

  case 184:

    { (yyval.string) = (yyvsp[(1) - (2)].string); ;}
    break;

  case 185:

    {
	(yyval.rule) = 0;
#ifdef CSS_DEBUG
	kDebug( 6080 ) << "skipped invalid @-rule";
#endif
    ;}
    break;

  case 186:

    {
	(yyval.rule) = 0;
#ifdef CSS_DEBUG
	kDebug( 6080 ) << "skipped invalid @-rule";
#endif
    ;}
    break;

  case 187:

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






