
/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

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
#define YYBISON_VERSION "2.4.1"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Using locations.  */
#define YYLSP_NEEDED 0

/* Substitute the variable and function names.  */
#define yyparse         cssyyparse
#define yylex           cssyylex
#define yyerror         cssyyerror
#define yylval          cssyylval
#define yychar          cssyychar
#define yydebug         cssyydebug
#define yynerrs         cssyynerrs


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


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     REDUCE = 258,
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
     HEXCOLOR = 270,
     IMPORT_SYM = 271,
     PAGE_SYM = 272,
     MEDIA_SYM = 273,
     FONT_FACE_SYM = 274,
     CHARSET_SYM = 275,
     NAMESPACE_SYM = 276,
     KHTML_RULE_SYM = 277,
     KHTML_DECLS_SYM = 278,
     KHTML_VALUE_SYM = 279,
     KHTML_MEDIAQUERY_SYM = 280,
     IMPORTANT_SYM = 281,
     MEDIA_ONLY = 282,
     MEDIA_NOT = 283,
     MEDIA_AND = 284,
     QEMS = 285,
     EMS = 286,
     EXS = 287,
     PXS = 288,
     CMS = 289,
     MMS = 290,
     INS = 291,
     PTS = 292,
     PCS = 293,
     DEGS = 294,
     RADS = 295,
     GRADS = 296,
     MSECS = 297,
     SECS = 298,
     HERZ = 299,
     KHERZ = 300,
     DPI = 301,
     DPCM = 302,
     DIMEN = 303,
     PERCENTAGE = 304,
     FLOAT = 305,
     INTEGER = 306,
     URI = 307,
     FUNCTION = 308,
     NOTFUNCTION = 309,
     UNICODERANGE = 310
   };
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



} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
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
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
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
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
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
# define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
	Stack = &yyptr->Stack_alloc;					\
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
#define YYNTOKENS  75
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  67
/* YYNRULES -- Number of rules.  */
#define YYNRULES  195
/* YYNRULES -- Number of states.  */
#define YYNSTATES  367

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   310

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      65,    66,    19,    63,    67,    70,    17,    73,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    16,    64,
       2,    72,    69,     2,    74,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    18,     2,    71,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    61,    20,    62,    68,     2,     2,     2,
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
      15,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     9,    12,    15,    18,    21,    23,    25,
      32,    35,    41,    47,    48,    50,    51,    54,    55,    58,
      61,    62,    64,    70,    74,    78,    79,    83,    90,    94,
      98,    99,   103,   110,   114,   118,   119,   122,   123,   127,
     129,   131,   133,   135,   137,   139,   142,   144,   146,   148,
     151,   153,   155,   158,   159,   164,   172,   174,   180,   181,
     185,   186,   188,   190,   192,   197,   198,   200,   202,   207,
     210,   218,   225,   226,   230,   233,   237,   241,   245,   249,
     253,   256,   259,   262,   264,   266,   269,   271,   276,   279,
     281,   284,   288,   292,   295,   297,   300,   303,   305,   308,
     310,   313,   317,   320,   322,   324,   327,   330,   332,   334,
     336,   339,   342,   344,   346,   348,   350,   353,   356,   361,
     370,   376,   386,   388,   390,   392,   394,   396,   398,   400,
     402,   405,   409,   416,   424,   431,   438,   445,   450,   455,
     460,   466,   472,   476,   480,   485,   490,   496,   499,   502,
     505,   506,   508,   512,   515,   518,   519,   521,   524,   527,
     530,   533,   536,   539,   541,   543,   546,   549,   552,   555,
     558,   561,   564,   567,   570,   573,   576,   579,   582,   585,
     588,   591,   594,   597,   600,   603,   606,   612,   616,   619,
     623,   627,   630,   636,   640,   642
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
      76,     0,    -1,    85,    84,    87,    89,    92,    -1,    78,
      83,    -1,    79,    83,    -1,    80,    83,    -1,    81,    83,
      -1,   112,    -1,    88,    -1,    27,    61,    83,    77,    83,
      62,    -1,    28,   127,    -1,    29,    61,    83,   132,    62,
      -1,    30,     4,    83,   102,    62,    -1,    -1,    63,    -1,
      -1,    83,     4,    -1,    -1,    84,     5,    -1,    84,     4,
      -1,    -1,    86,    -1,    25,    83,    11,    83,    64,    -1,
      25,     1,   140,    -1,    25,     1,    64,    -1,    -1,    87,
      88,    84,    -1,    21,    83,    95,    83,   103,    64,    -1,
      21,     1,   140,    -1,    21,     1,    64,    -1,    -1,    89,
      90,    84,    -1,    26,    83,    91,    95,    83,    64,    -1,
      26,     1,   140,    -1,    26,     1,    64,    -1,    -1,    12,
       4,    -1,    -1,    92,    93,    84,    -1,   112,    -1,   105,
      -1,   108,    -1,   109,    -1,   139,    -1,   138,    -1,    88,
       1,    -1,   112,    -1,   139,    -1,   138,    -1,    88,     1,
      -1,    11,    -1,    57,    -1,    12,    83,    -1,    -1,    16,
      83,   132,    83,    -1,    65,    83,    96,    83,    97,    66,
      83,    -1,    98,    -1,    99,    83,    34,    83,    98,    -1,
      -1,    34,    83,    99,    -1,    -1,    32,    -1,    33,    -1,
      99,    -1,   101,    83,   107,   100,    -1,    -1,   104,    -1,
     102,    -1,   104,    67,    83,   102,    -1,   104,     1,    -1,
      23,    83,   104,    61,    83,   106,    62,    -1,    23,    83,
      61,    83,   106,    62,    -1,    -1,   106,    94,    83,    -1,
      12,    83,    -1,    22,     1,   140,    -1,    22,     1,    64,
      -1,    24,    83,   127,    -1,    24,     1,   140,    -1,    24,
       1,    64,    -1,    63,    83,    -1,    68,    83,    -1,    69,
      83,    -1,    70,    -1,    63,    -1,   113,   127,    -1,   114,
      -1,   113,    67,    83,   114,    -1,   113,     1,    -1,   116,
      -1,   114,     4,    -1,   114,     4,   116,    -1,   114,   110,
     116,    -1,   114,     1,    -1,    20,    -1,    19,    20,    -1,
      12,    20,    -1,   118,    -1,   118,   119,    -1,   119,    -1,
     115,   118,    -1,   115,   118,   119,    -1,   115,   119,    -1,
     118,    -1,   120,    -1,   115,   118,    -1,   115,   120,    -1,
      12,    -1,    19,    -1,   120,    -1,   119,   120,    -1,   119,
       1,    -1,    14,    -1,   121,    -1,   123,    -1,   126,    -1,
      17,    12,    -1,    12,    83,    -1,    18,    83,   122,    71,
      -1,    18,    83,   122,   124,    83,   125,    83,    71,    -1,
      18,    83,   115,   122,    71,    -1,    18,    83,   115,   122,
     124,    83,   125,    83,    71,    -1,    72,    -1,     6,    -1,
       7,    -1,     8,    -1,     9,    -1,    10,    -1,    12,    -1,
      11,    -1,    16,    12,    -1,    16,    16,    12,    -1,    16,
      58,    83,    13,    83,    66,    -1,    16,    58,    83,    82,
      56,    83,    66,    -1,    16,    58,    83,    12,    83,    66,
      -1,    16,    58,    83,    11,    83,    66,    -1,    16,    59,
      83,   117,    83,    66,    -1,    61,    83,   129,    62,    -1,
      61,    83,     1,    62,    -1,    61,    83,   128,    62,    -1,
      61,    83,   128,   129,    62,    -1,    61,    83,   128,     1,
      62,    -1,   129,    64,    83,    -1,     1,    64,    83,    -1,
     128,   129,    64,    83,    -1,   128,     1,    64,    83,    -1,
     130,    16,    83,   132,   131,    -1,     1,   140,    -1,    12,
      83,    -1,    31,    83,    -1,    -1,   134,    -1,   132,   133,
     134,    -1,    73,    83,    -1,    67,    83,    -1,    -1,   135,
      -1,   111,   135,    -1,    53,    83,    -1,    11,    83,    -1,
      12,    83,    -1,    57,    83,    -1,    60,    83,    -1,   137,
      -1,   136,    -1,    56,    83,    -1,    55,    83,    -1,    54,
      83,    -1,    38,    83,    -1,    39,    83,    -1,    40,    83,
      -1,    41,    83,    -1,    42,    83,    -1,    43,    83,    -1,
      44,    83,    -1,    45,    83,    -1,    46,    83,    -1,    47,
      83,    -1,    48,    83,    -1,    49,    83,    -1,    50,    83,
      -1,    36,    83,    -1,    35,    83,    -1,    37,    83,    -1,
      51,    83,    -1,    52,    83,    -1,    58,    83,   132,    66,
      83,    -1,    58,    83,     1,    -1,    15,    83,    -1,    74,
       1,   140,    -1,    74,     1,    64,    -1,     1,   140,    -1,
      61,     1,   141,     1,    62,    -1,    61,     1,    62,    -1,
     140,    -1,   141,     1,   140,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   292,   292,   293,   294,   295,   296,   300,   301,   305,
     312,   318,   343,   349,   351,   355,   356,   359,   361,   362,
     365,   367,   370,   379,   381,   385,   387,   398,   408,   411,
     417,   418,   422,   430,   431,   435,   436,   439,   441,   452,
     453,   454,   455,   456,   457,   458,   462,   463,   464,   465,
     469,   470,   474,   480,   483,   489,   495,   499,   506,   509,
     515,   518,   521,   527,   530,   536,   539,   544,   548,   553,
     560,   571,   583,   584,   594,   612,   615,   621,   628,   631,
     637,   638,   639,   643,   644,   648,   670,   683,   701,   711,
     714,   717,   731,   745,   752,   753,   754,   758,   763,   770,
     777,   785,   795,   808,   813,   820,   828,   841,   845,   851,
     854,   864,   871,   885,   886,   887,   891,   908,   915,   921,
     928,   937,   950,   953,   956,   959,   962,   965,   971,   972,
     976,   982,   988,   995,  1002,  1009,  1016,  1025,  1028,  1031,
    1034,  1039,  1045,  1049,  1052,  1057,  1063,  1085,  1091,  1098,
    1099,  1103,  1107,  1123,  1126,  1129,  1135,  1136,  1138,  1139,
    1140,  1146,  1147,  1148,  1150,  1156,  1157,  1158,  1159,  1160,
    1161,  1162,  1163,  1164,  1165,  1166,  1167,  1168,  1169,  1170,
    1171,  1172,  1173,  1174,  1175,  1176,  1181,  1189,  1205,  1212,
    1218,  1227,  1253,  1254,  1258,  1259
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "REDUCE", "S", "SGML_CD", "INCLUDES",
  "DASHMATCH", "BEGINSWITH", "ENDSWITH", "CONTAINS", "STRING", "IDENT",
  "NTH", "HASH", "HEXCOLOR", "':'", "'.'", "'['", "'*'", "'|'",
  "IMPORT_SYM", "PAGE_SYM", "MEDIA_SYM", "FONT_FACE_SYM", "CHARSET_SYM",
  "NAMESPACE_SYM", "KHTML_RULE_SYM", "KHTML_DECLS_SYM", "KHTML_VALUE_SYM",
  "KHTML_MEDIAQUERY_SYM", "IMPORTANT_SYM", "MEDIA_ONLY", "MEDIA_NOT",
  "MEDIA_AND", "QEMS", "EMS", "EXS", "PXS", "CMS", "MMS", "INS", "PTS",
  "PCS", "DEGS", "RADS", "GRADS", "MSECS", "SECS", "HERZ", "KHERZ", "DPI",
  "DPCM", "DIMEN", "PERCENTAGE", "FLOAT", "INTEGER", "URI", "FUNCTION",
  "NOTFUNCTION", "UNICODERANGE", "'{'", "'}'", "'+'", "';'", "'('", "')'",
  "','", "'~'", "'>'", "'-'", "']'", "'='", "'/'", "'@'", "$accept",
  "stylesheet", "ruleset_or_import", "khtml_rule", "khtml_decls",
  "khtml_value", "khtml_mediaquery", "maybe_plus", "maybe_space",
  "maybe_sgml", "maybe_charset", "charset", "import_list", "import",
  "namespace_list", "namespace", "maybe_ns_prefix", "rule_list", "rule",
  "safe_ruleset", "string_or_uri", "media_feature", "maybe_media_value",
  "media_query_exp", "media_query_exp_list",
  "maybe_and_media_query_exp_list", "maybe_media_restrictor",
  "media_query", "maybe_media_list", "media_list", "media", "ruleset_list",
  "medium", "page", "font_face", "combinator", "unary_operator", "ruleset",
  "selector_list", "selector", "namespace_selector", "simple_selector",
  "simple_css3_selector", "element_name", "specifier_list", "specifier",
  "class", "attrib_id", "attrib", "match", "ident_or_string", "pseudo",
  "declaration_block", "declaration_list", "declaration", "property",
  "prio", "expr", "operator", "term", "unary_term", "function", "hexcolor",
  "invalid_at", "invalid_rule", "invalid_block", "invalid_block_list", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,    58,    46,    91,    42,
     124,   271,   272,   273,   274,   275,   276,   277,   278,   279,
     280,   281,   282,   283,   284,   285,   286,   287,   288,   289,
     290,   291,   292,   293,   294,   295,   296,   297,   298,   299,
     300,   301,   302,   303,   304,   305,   306,   307,   308,   309,
     310,   123,   125,    43,    59,    40,    41,    44,   126,    62,
      45,    93,    61,    47,    64
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    75,    76,    76,    76,    76,    76,    77,    77,    78,
      79,    80,    81,    82,    82,    83,    83,    84,    84,    84,
      85,    85,    86,    86,    86,    87,    87,    88,    88,    88,
      89,    89,    90,    90,    90,    91,    91,    92,    92,    93,
      93,    93,    93,    93,    93,    93,    94,    94,    94,    94,
      95,    95,    96,    97,    97,    98,    99,    99,   100,   100,
     101,   101,   101,   102,   102,   103,   103,   104,   104,   104,
     105,   105,   106,   106,   107,   108,   108,   109,   109,   109,
     110,   110,   110,   111,   111,   112,   113,   113,   113,   114,
     114,   114,   114,   114,   115,   115,   115,   116,   116,   116,
     116,   116,   116,   117,   117,   117,   117,   118,   118,   119,
     119,   119,   120,   120,   120,   120,   121,   122,   123,   123,
     123,   123,   124,   124,   124,   124,   124,   124,   125,   125,
     126,   126,   126,   126,   126,   126,   126,   127,   127,   127,
     127,   127,   128,   128,   128,   128,   129,   129,   130,   131,
     131,   132,   132,   133,   133,   133,   134,   134,   134,   134,
     134,   134,   134,   134,   134,   135,   135,   135,   135,   135,
     135,   135,   135,   135,   135,   135,   135,   135,   135,   135,
     135,   135,   135,   135,   135,   135,   136,   136,   137,   138,
     138,   139,   140,   140,   141,   141
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     5,     2,     2,     2,     2,     1,     1,     6,
       2,     5,     5,     0,     1,     0,     2,     0,     2,     2,
       0,     1,     5,     3,     3,     0,     3,     6,     3,     3,
       0,     3,     6,     3,     3,     0,     2,     0,     3,     1,
       1,     1,     1,     1,     1,     2,     1,     1,     1,     2,
       1,     1,     2,     0,     4,     7,     1,     5,     0,     3,
       0,     1,     1,     1,     4,     0,     1,     1,     4,     2,
       7,     6,     0,     3,     2,     3,     3,     3,     3,     3,
       2,     2,     2,     1,     1,     2,     1,     4,     2,     1,
       2,     3,     3,     2,     1,     2,     2,     1,     2,     1,
       2,     3,     2,     1,     1,     2,     2,     1,     1,     1,
       2,     2,     1,     1,     1,     1,     2,     2,     4,     8,
       5,     9,     1,     1,     1,     1,     1,     1,     1,     1,
       2,     3,     6,     7,     6,     6,     6,     4,     4,     4,
       5,     5,     3,     3,     4,     4,     5,     2,     2,     2,
       0,     1,     3,     2,     2,     0,     1,     2,     2,     2,
       2,     2,     2,     1,     1,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     5,     3,     2,     3,
       3,     2,     5,     3,     1,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
      20,     0,     0,     0,     0,     0,     0,    15,    15,    15,
      15,    17,    21,     0,     0,    15,    15,    10,    15,    15,
       1,     3,     4,     5,     6,    25,     0,    24,    23,    16,
      15,     0,     0,     0,    60,    19,    18,    30,     0,     0,
     107,   112,     0,     0,    15,   108,    94,     0,    15,     8,
       7,     0,     0,     0,    89,    97,     0,   109,   113,   114,
     115,     0,    15,     0,     0,     0,    15,    15,    15,    15,
      15,    15,    15,    15,    15,    15,    15,    15,    15,    15,
      15,    15,    15,    15,    15,    15,    15,    15,    15,    15,
      15,    15,    15,    15,    84,    83,     0,   155,   151,   156,
     164,   163,    61,    62,    15,    56,    63,    15,     0,    17,
      37,   193,   194,     0,    22,    96,   130,     0,    15,    15,
     116,     0,    95,     0,     0,     0,    88,    15,    85,    93,
      90,    15,    15,    15,     0,   107,   108,   100,     0,     0,
     111,   110,   138,    15,   147,   148,     0,   139,     0,   137,
      15,    15,   159,   160,   188,   182,   181,   183,   168,   169,
     170,   171,   172,   173,   174,   175,   176,   177,   178,   179,
     180,   184,   185,   158,   167,   166,   165,   161,     0,   162,
     157,    11,    15,    15,     0,     0,     0,     0,    12,    26,
       0,    17,     0,     0,   131,    13,     0,    15,     0,     0,
       0,    29,    28,    50,    51,    15,     9,     0,    91,    80,
      81,    82,    92,     0,   143,   141,    15,   140,    15,   142,
       0,   187,   155,   154,   153,   152,    15,    15,    15,    15,
      58,     0,    35,    31,     0,     0,    15,     0,     0,     0,
      17,    40,    41,    42,    39,    44,    43,   192,   195,    15,
      15,    15,    14,     0,     0,    15,   103,   104,   117,    15,
       0,   123,   124,   125,   126,   127,   118,   122,    15,    60,
       0,   145,   144,   155,    15,    52,    53,     0,    74,    15,
      64,    34,    33,     0,     0,   191,     0,    60,     0,     0,
       0,    45,    38,     0,     0,     0,    15,   105,   106,     0,
     120,    15,     0,    67,     0,     0,    15,   146,   186,    15,
       0,    57,     0,    36,    15,    76,    75,    15,     0,    79,
      78,    77,   190,   189,   135,   134,   132,     0,   136,     0,
     129,   128,    15,    27,    69,    15,   149,     0,    15,    59,
       0,    72,    15,   133,    15,     0,    60,   155,    55,    32,
       0,    72,     0,   119,    68,    54,    71,     0,    15,    46,
      48,    47,     0,   121,    49,    73,    70
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     6,    48,     7,     8,     9,    10,   253,   186,    25,
      11,    12,    37,   357,   110,   191,   284,   192,   240,   358,
     205,   227,   310,   105,   106,   280,   107,   303,   304,   305,
     241,   350,   230,   242,   243,   134,    96,   359,    51,    52,
      53,    54,   255,    55,    56,    57,    58,   200,    59,   268,
     332,    60,    17,    63,    64,    65,   307,    97,   184,    98,
      99,   100,   101,   360,   361,   144,   113
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -166
static const yytype_int16 yypact[] =
{
     282,    33,   -56,   -39,   -33,    60,    59,  -166,  -166,  -166,
    -166,  -166,  -166,   129,    49,  -166,  -166,  -166,  -166,  -166,
    -166,   121,   121,   121,   121,   226,   144,  -166,  -166,  -166,
    -166,   457,    29,   453,   264,  -166,  -166,   152,   199,    45,
     141,  -166,   243,   185,  -166,   193,  -166,   349,  -166,  -166,
    -166,    44,    47,   567,  -166,   263,   139,  -166,  -166,  -166,
    -166,   278,  -166,   162,   -22,   217,  -166,  -166,  -166,  -166,
    -166,  -166,  -166,  -166,  -166,  -166,  -166,  -166,  -166,  -166,
    -166,  -166,  -166,  -166,  -166,  -166,  -166,  -166,  -166,  -166,
    -166,  -166,  -166,  -166,  -166,  -166,   404,    92,  -166,  -166,
    -166,  -166,  -166,  -166,  -166,  -166,    20,  -166,   244,  -166,
     220,  -166,  -166,   317,  -166,  -166,  -166,   320,  -166,  -166,
    -166,   517,  -166,   201,   135,   124,  -166,  -166,  -166,  -166,
     558,  -166,  -166,  -166,   558,  -166,  -166,   263,   348,   357,
    -166,  -166,  -166,  -166,  -166,   121,   469,  -166,   207,  -166,
    -166,  -166,   121,   121,   121,   121,   121,   121,   121,   121,
     121,   121,   121,   121,   121,   121,   121,   121,   121,   121,
     121,   121,   121,   121,   121,   121,   121,   121,   344,   121,
    -166,  -166,  -166,  -166,   503,   154,    23,   158,  -166,   226,
     271,  -166,   303,   237,  -166,   417,   508,   141,   193,   326,
     127,  -166,  -166,  -166,  -166,  -166,  -166,   508,  -166,   121,
     121,   121,  -166,   418,   121,  -166,  -166,  -166,  -166,   121,
     453,  -166,   190,   121,   121,  -166,  -166,  -166,  -166,  -166,
     310,   269,   230,   226,   302,   367,  -166,   140,   369,   375,
    -166,  -166,  -166,  -166,  -166,  -166,  -166,  -166,  -166,  -166,
    -166,  -166,  -166,   347,   567,  -166,  -166,  -166,   121,  -166,
     169,  -166,  -166,  -166,  -166,  -166,  -166,  -166,  -166,   179,
     184,   121,   121,   305,  -166,   121,    15,    48,   121,  -166,
    -166,  -166,  -166,   401,   137,  -166,   290,   205,   402,   134,
     406,  -166,   226,    34,    35,    46,  -166,  -166,  -166,    54,
    -166,  -166,   156,  -166,   346,    55,  -166,  -166,   121,  -166,
     342,  -166,    48,  -166,  -166,  -166,  -166,  -166,    62,  -166,
    -166,  -166,  -166,  -166,  -166,  -166,  -166,    58,  -166,   156,
    -166,  -166,  -166,  -166,  -166,  -166,   121,   453,  -166,    20,
     123,   121,  -166,  -166,  -166,    22,   264,    31,   121,  -166,
     202,   121,    28,  -166,  -166,   121,  -166,   411,  -166,  -166,
    -166,  -166,   273,  -166,  -166,   121,  -166
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -166,  -166,  -166,  -166,  -166,  -166,  -166,  -166,    -1,   -89,
    -166,  -166,  -166,   -21,  -166,  -166,  -166,  -166,  -166,  -166,
     143,  -166,  -166,   136,   111,  -166,  -166,   -32,  -166,   146,
    -166,    80,  -166,  -166,  -166,  -166,  -166,   -28,  -166,   231,
    -100,    50,  -166,   -49,   -30,   -44,  -166,   238,  -166,   212,
     133,  -166,   -50,  -166,   419,  -166,  -166,  -165,  -166,   299,
     388,  -166,  -166,   325,   327,    -2,  -166
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -151
static const yytype_int16 yytable[] =
{
      14,   128,   108,    50,   137,    15,    21,    22,    23,    24,
      49,    28,   141,   222,    31,    32,   109,    33,    34,    29,
     189,   199,    16,   138,   -15,   139,    29,    29,    18,    39,
      61,   309,    29,    29,    13,   -15,   112,   -15,    29,    29,
     149,    62,   150,   121,   -15,   126,   124,   125,   129,    29,
      29,   130,    29,    29,   -15,   273,   334,   228,    29,    20,
      30,   145,    29,   334,    19,   152,   153,   154,   155,   156,
     157,   158,   159,   160,   161,   162,   163,   164,   165,   166,
     167,   168,   169,   170,   171,   172,   173,   174,   175,   176,
     177,   178,   179,   353,   141,   141,   254,   -15,   182,   363,
     324,   325,   233,   185,   183,    16,   187,   213,   -86,   114,
     131,   127,   326,   104,   -86,   132,   133,   195,   196,   -66,
     328,   202,   335,   342,   343,    29,   207,    29,    29,   335,
     209,   210,   211,   261,   262,   263,   264,   265,    29,    29,
     140,   288,   214,   -99,   -15,    38,   203,   256,   203,   219,
     220,   292,   257,    41,   181,    42,    43,    44,    29,   182,
      29,   115,    29,   146,   244,   183,   226,   330,   331,   141,
     229,   239,   347,    47,    62,   261,   262,   263,   264,   265,
     208,   223,   224,    29,   212,   129,   206,   349,   130,   232,
      26,   248,   204,    27,   204,    16,   258,   120,   266,   267,
     -99,   -15,   -99,   234,   269,   297,   -99,   -99,   -99,    29,
     298,   102,   103,   122,    40,   271,    41,   272,    42,    43,
      44,    45,    46,    47,   147,   275,   276,   277,   278,   282,
      35,    36,   285,   151,    29,   287,   289,   102,   103,   321,
     300,   267,   283,   -65,   104,   -87,   190,   131,   293,   294,
     295,   -87,   132,   133,   299,   116,   274,   182,   258,   117,
      26,   111,    26,   183,   356,   201,   317,   302,    29,   217,
     104,   218,   231,   308,   234,   -15,   238,    41,   312,    42,
      43,    44,   -15,   -15,   316,    40,   320,    41,   323,    42,
      43,    44,    45,    46,    47,   327,   102,   103,    26,   247,
     329,   118,   119,    -2,   234,   336,   188,     1,   337,     2,
       3,     4,     5,   340,   354,    40,   341,    41,   193,    42,
      43,    44,    45,    46,    47,   235,   236,   237,   -15,   104,
      26,   345,   194,   281,   346,   366,   306,   348,   259,    26,
     142,   351,   143,   352,   279,   221,   355,   238,    29,   140,
     123,    26,  -102,   -15,   315,    66,    67,   365,   140,    68,
     -15,   -98,    41,    26,    42,    43,    44,  -150,   286,  -150,
     290,    41,   182,    42,    43,    44,   291,   238,   183,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,   296,    93,   313,   -15,    94,   338,  -102,
     333,  -102,   364,   311,    95,  -102,  -102,  -102,   -98,   140,
     -98,    29,  -101,   339,   -98,   -98,   -98,   314,   249,   250,
     251,   362,    41,   318,    42,    43,    44,   260,   270,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    29,    88,    89,
      90,    29,   344,    26,    66,    67,   319,    26,    68,    40,
     322,    41,   301,    42,    43,    44,    45,    46,    47,  -101,
     252,  -101,   148,   225,   180,  -101,  -101,  -101,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    29,    93,    66,    67,    94,   245,    68,   246,
      40,    29,    41,    95,    42,    43,    44,    45,    46,   197,
      26,   215,     0,   216,     0,     0,   198,    46,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,     0,    93,     0,     0,    94,     0,     0,     0,
      40,     0,    41,    95,    42,    43,    44,    45,    46,   135,
       0,    41,     0,    42,    43,    44,   136
};

static const yytype_int16 yycheck[] =
{
       1,    51,    34,    31,    53,    61,     7,     8,     9,    10,
      31,    13,    56,   178,    15,    16,    37,    18,    19,     4,
     109,   121,    61,    53,     4,    55,     4,     4,    61,    30,
       1,    16,     4,     4,     1,     4,    38,     4,     4,     4,
      62,    12,    64,    44,    11,     1,    47,    48,     1,     4,
       4,     4,     4,     4,    34,   220,     1,    34,     4,     0,
      11,    62,     4,     1,     4,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    71,   138,   139,   196,    66,    67,    71,
      66,    66,   191,   104,    73,    61,   107,   137,    61,    64,
      63,    67,    66,    65,    67,    68,    69,   118,   119,    64,
      66,   123,    67,    61,    66,     4,   127,     4,     4,    67,
     131,   132,   133,     6,     7,     8,     9,    10,     4,     4,
       1,     1,   143,     4,     4,     1,    11,   196,    11,   150,
     151,   240,   196,    14,    62,    16,    17,    18,     4,    67,
       4,    20,     4,     1,   192,    73,    12,    11,    12,   213,
      12,   192,   337,    21,    12,     6,     7,     8,     9,    10,
     130,   182,   183,     4,   134,     1,    62,    64,     4,   190,
      61,   193,    57,    64,    57,    61,   197,    12,    71,    72,
      61,    61,    63,     1,   205,   254,    67,    68,    69,     4,
     254,    32,    33,    20,    12,   216,    14,   218,    16,    17,
      18,    19,    20,    21,    62,   226,   227,   228,   229,   231,
       4,     5,   234,    16,     4,   236,   237,    32,    33,   289,
      71,    72,    12,    64,    65,    61,    26,    63,   249,   250,
     251,    67,    68,    69,   255,    12,    66,    67,   259,    16,
      61,    62,    61,    73,    62,    64,    61,   268,     4,    62,
      65,    64,     1,   274,     1,     4,    74,    14,   279,    16,
      17,    18,    11,    12,   286,    12,   288,    14,   290,    16,
      17,    18,    19,    20,    21,   296,    32,    33,    61,    62,
     301,    58,    59,     0,     1,   306,    62,    25,   309,    27,
      28,    29,    30,   314,   346,    12,   317,    14,     1,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    57,    65,
      61,   332,    12,    64,   335,    62,    31,   338,    12,    61,
      62,   342,    64,   344,    34,     1,   347,    74,     4,     1,
       1,    61,     4,     4,    64,    11,    12,   358,     1,    15,
      11,     4,    14,    61,    16,    17,    18,    62,     1,    64,
       1,    14,    67,    16,    17,    18,     1,    74,    73,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    56,    60,     4,    57,    63,    66,    61,
      64,    63,     1,   277,    70,    67,    68,    69,    61,     1,
      63,     4,     4,   312,    67,    68,    69,   284,    11,    12,
      13,   351,    14,   287,    16,    17,    18,   199,   207,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,     4,    54,    55,
      56,     4,   329,    61,    11,    12,    64,    61,    15,    12,
      64,    14,   260,    16,    17,    18,    19,    20,    21,    61,
      63,    63,    63,   184,    96,    67,    68,    69,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,     4,    60,    11,    12,    63,   192,    15,   192,
      12,     4,    14,    70,    16,    17,    18,    19,    20,    12,
      61,    62,    -1,    64,    -1,    -1,    19,    20,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    -1,    60,    -1,    -1,    63,    -1,    -1,    -1,
      12,    -1,    14,    70,    16,    17,    18,    19,    20,    12,
      -1,    14,    -1,    16,    17,    18,    19
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    25,    27,    28,    29,    30,    76,    78,    79,    80,
      81,    85,    86,     1,    83,    61,    61,   127,    61,     4,
       0,    83,    83,    83,    83,    84,    61,    64,   140,     4,
      11,    83,    83,    83,    83,     4,     5,    87,     1,    83,
      12,    14,    16,    17,    18,    19,    20,    21,    77,    88,
     112,   113,   114,   115,   116,   118,   119,   120,   121,   123,
     126,     1,    12,   128,   129,   130,    11,    12,    15,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    60,    63,    70,   111,   132,   134,   135,
     136,   137,    32,    33,    65,    98,    99,   101,   102,    88,
      89,    62,   140,   141,    64,    20,    12,    16,    58,    59,
      12,    83,    20,     1,    83,    83,     1,    67,   127,     1,
       4,    63,    68,    69,   110,    12,    19,   118,   119,   119,
       1,   120,    62,    64,   140,    83,     1,    62,   129,    62,
      64,    16,    83,    83,    83,    83,    83,    83,    83,    83,
      83,    83,    83,    83,    83,    83,    83,    83,    83,    83,
      83,    83,    83,    83,    83,    83,    83,    83,    83,    83,
     135,    62,    67,    73,   133,    83,    83,    83,    62,    84,
      26,    90,    92,     1,    12,    83,    83,    12,    19,   115,
     122,    64,   140,    11,    57,    95,    62,    83,   116,    83,
      83,    83,   116,   119,    83,    62,    64,    62,    64,    83,
      83,     1,   132,    83,    83,   134,    12,    96,    34,    12,
     107,     1,    83,    84,     1,    22,    23,    24,    74,    88,
      93,   105,   108,   109,   112,   138,   139,    62,   140,    11,
      12,    13,    63,    82,   115,   117,   118,   120,    83,    12,
     122,     6,     7,     8,     9,    10,    71,    72,   124,    83,
     114,    83,    83,   132,    66,    83,    83,    83,    83,    34,
     100,    64,   140,    12,    91,   140,     1,    83,     1,    83,
       1,     1,    84,    83,    83,    83,    56,   118,   120,    83,
      71,   124,    83,   102,   103,   104,    31,   131,    83,    16,
      97,    98,    83,     4,    95,    64,   140,    61,   104,    64,
     140,   127,    64,   140,    66,    66,    66,    83,    66,    83,
      11,    12,   125,    64,     1,    67,    83,    83,    66,    99,
      83,    83,    61,    66,   125,    83,    83,   132,    83,    64,
     106,    83,    83,    71,   102,    83,    62,    88,    94,   112,
     138,   139,   106,    71,     1,    83,    62
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
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
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
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      YYFPRINTF (stderr, "\n");
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
      case 99: /* "media_query_exp_list" */

	{ delete (yyvaluep->mediaQueryExpList); (yyvaluep->mediaQueryExpList) = 0; };

	break;
      case 100: /* "maybe_and_media_query_exp_list" */

	{ delete (yyvaluep->mediaQueryExpList); (yyvaluep->mediaQueryExpList) = 0; };

	break;
      case 103: /* "maybe_media_list" */

	{ delete (yyvaluep->mediaList); (yyvaluep->mediaList) = 0; };

	break;
      case 104: /* "media_list" */

	{ delete (yyvaluep->mediaList); (yyvaluep->mediaList) = 0; };

	break;
      case 106: /* "ruleset_list" */

	{ delete (yyvaluep->ruleList); (yyvaluep->ruleList) = 0; };

	break;
      case 113: /* "selector_list" */

	{ if ((yyvaluep->selectorList)) qDeleteAll(*(yyvaluep->selectorList)); delete (yyvaluep->selectorList); (yyvaluep->selectorList) = 0; };

	break;
      case 114: /* "selector" */

	{ delete (yyvaluep->selector); (yyvaluep->selector) = 0; };

	break;
      case 116: /* "simple_selector" */

	{ delete (yyvaluep->selector); (yyvaluep->selector) = 0; };

	break;
      case 117: /* "simple_css3_selector" */

	{ delete (yyvaluep->selector); (yyvaluep->selector) = 0; };

	break;
      case 119: /* "specifier_list" */

	{ delete (yyvaluep->selector); (yyvaluep->selector) = 0; };

	break;
      case 120: /* "specifier" */

	{ delete (yyvaluep->selector); (yyvaluep->selector) = 0; };

	break;
      case 121: /* "class" */

	{ delete (yyvaluep->selector); (yyvaluep->selector) = 0; };

	break;
      case 123: /* "attrib" */

	{ delete (yyvaluep->selector); (yyvaluep->selector) = 0; };

	break;
      case 126: /* "pseudo" */

	{ delete (yyvaluep->selector); (yyvaluep->selector) = 0; };

	break;
      case 132: /* "expr" */

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





/*-------------------------.
| yyparse or yypush_parse.  |
`-------------------------*/

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
/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

    /* Number of syntax errors so far.  */
    int yynerrs;

    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.

       Refer to the stacks thru separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yytoken = 0;
  yyss = yyssa;
  yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */

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
	YYSTACK_RELOCATE (yyss_alloc, yyss);
	YYSTACK_RELOCATE (yyvs_alloc, yyvs);
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

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

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

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
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

  case 22:

    {
#ifdef CSS_DEBUG
     kDebug( 6080 ) << "charset rule: " << qString((yyvsp[(3) - (5)].string));
#endif
     CSSParser* p = static_cast<CSSParser*>(parser);
     if (p->styleElement && p->styleElement->isCSSStyleSheet()) {
         p->styleElement->append( new CSSCharsetRuleImpl(p->styleElement, domString((yyvsp[(3) - (5)].string))) );
     }
 ;}
    break;

  case 23:

    {
 ;}
    break;

  case 24:

    {
 ;}
    break;

  case 26:

    {
     CSSParser *p = static_cast<CSSParser *>(parser);
     if ( (yyvsp[(2) - (3)].rule) && p->styleElement && p->styleElement->isCSSStyleSheet() ) {
	 p->styleElement->append( (yyvsp[(2) - (3)].rule) );
     } else {
	 delete (yyvsp[(2) - (3)].rule);
     }
 ;}
    break;

  case 27:

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

  case 28:

    {
        (yyval.rule) = 0;
    ;}
    break;

  case 29:

    {
        (yyval.rule) = 0;
    ;}
    break;

  case 32:

    {
#ifdef CSS_DEBUG
    kDebug( 6080 ) << "@namespace: " << qString((yyvsp[(4) - (6)].string));
#endif
      CSSParser *p = static_cast<CSSParser *>(parser);
    if (p->styleElement && p->styleElement->isCSSStyleSheet())
        static_cast<CSSStyleSheetImpl*>(p->styleElement)->addNamespace(p, domString((yyvsp[(3) - (6)].string)), domString((yyvsp[(4) - (6)].string)));
 ;}
    break;

  case 35:

    { (yyval.string).string = 0; ;}
    break;

  case 36:

    { (yyval.string) = (yyvsp[(1) - (2)].string); ;}
    break;

  case 38:

    {
     CSSParser *p = static_cast<CSSParser *>(parser);
     if ( (yyvsp[(2) - (3)].rule) && p->styleElement && p->styleElement->isCSSStyleSheet() ) {
	 p->styleElement->append( (yyvsp[(2) - (3)].rule) );
     } else {
	 delete (yyvsp[(2) - (3)].rule);
     }
 ;}
    break;

  case 45:

    { delete (yyvsp[(1) - (2)].rule); (yyval.rule) = 0; ;}
    break;

  case 49:

    { delete (yyvsp[(1) - (2)].rule); (yyval.rule) = 0; ;}
    break;

  case 52:

    {
        (yyval.string) = (yyvsp[(1) - (2)].string);
    ;}
    break;

  case 53:

    {
        (yyval.valueList) = 0;
    ;}
    break;

  case 54:

    {
        (yyval.valueList) = (yyvsp[(3) - (4)].valueList);
    ;}
    break;

  case 55:

    {
        (yyval.mediaQueryExp) = new khtml::MediaQueryExp(domString((yyvsp[(3) - (7)].string)).lower(), (yyvsp[(5) - (7)].valueList));
    ;}
    break;

  case 56:

    {
      (yyval.mediaQueryExpList) =  new QList<khtml::MediaQueryExp*>;
      (yyval.mediaQueryExpList)->append((yyvsp[(1) - (1)].mediaQueryExp));
    ;}
    break;

  case 57:

    {
      (yyval.mediaQueryExpList) = (yyvsp[(1) - (5)].mediaQueryExpList);
      (yyval.mediaQueryExpList)->append((yyvsp[(5) - (5)].mediaQueryExp));
    ;}
    break;

  case 58:

    {
        (yyval.mediaQueryExpList) = new QList<khtml::MediaQueryExp*>;
    ;}
    break;

  case 59:

    {
        (yyval.mediaQueryExpList) = (yyvsp[(3) - (3)].mediaQueryExpList);
    ;}
    break;

  case 60:

    {
        (yyval.mediaQueryRestrictor) = khtml::MediaQuery::None;
    ;}
    break;

  case 61:

    {
        (yyval.mediaQueryRestrictor) = khtml::MediaQuery::Only;
    ;}
    break;

  case 62:

    {
        (yyval.mediaQueryRestrictor) = khtml::MediaQuery::Not;
    ;}
    break;

  case 63:

    {
        (yyval.mediaQuery) = new khtml::MediaQuery(khtml::MediaQuery::None, "all", (yyvsp[(1) - (1)].mediaQueryExpList));
    ;}
    break;

  case 64:

    {
        (yyval.mediaQuery) = new khtml::MediaQuery((yyvsp[(1) - (4)].mediaQueryRestrictor), domString((yyvsp[(3) - (4)].string)).lower(), (yyvsp[(4) - (4)].mediaQueryExpList));
    ;}
    break;

  case 65:

    {
	(yyval.mediaList) = new MediaListImpl();
    ;}
    break;

  case 67:

    {
        (yyval.mediaList) = new MediaListImpl();
        (yyval.mediaList)->appendMediaQuery((yyvsp[(1) - (1)].mediaQuery));
    ;}
    break;

  case 68:

    {
	(yyval.mediaList) = (yyvsp[(1) - (4)].mediaList);
	if ((yyval.mediaList))
	    (yyval.mediaList)->appendMediaQuery( (yyvsp[(4) - (4)].mediaQuery) );
    ;}
    break;

  case 69:

    {
       delete (yyvsp[(1) - (2)].mediaList);
       (yyval.mediaList) = 0;
    ;}
    break;

  case 70:

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

  case 71:

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

  case 72:

    { (yyval.ruleList) = 0; ;}
    break;

  case 73:

    {
      (yyval.ruleList) = (yyvsp[(1) - (3)].ruleList);
      if ( (yyvsp[(2) - (3)].rule) ) {
	  if ( !(yyval.ruleList) ) (yyval.ruleList) = new CSSRuleListImpl();
	  (yyval.ruleList)->append( (yyvsp[(2) - (3)].rule) );
      }
  ;}
    break;

  case 74:

    {
      (yyval.string) = (yyvsp[(1) - (2)].string);
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

    {
      CSSParser *p = static_cast<CSSParser *>(parser);
      CSSFontFaceRuleImpl *rule = new CSSFontFaceRuleImpl( p->styleElement );
      CSSStyleDeclarationImpl *decl = p->createFontFaceStyleDeclaration( rule );
      rule->setDeclaration(decl);
      (yyval.rule) = rule;
    ;}
    break;

  case 78:

    {
      (yyval.rule) = 0;
    ;}
    break;

  case 79:

    {
      (yyval.rule) = 0;
    ;}
    break;

  case 80:

    { (yyval.relation) = CSSSelector::DirectAdjacent; ;}
    break;

  case 81:

    { (yyval.relation) = CSSSelector::IndirectAdjacent; ;}
    break;

  case 82:

    { (yyval.relation) = CSSSelector::Child; ;}
    break;

  case 83:

    { (yyval.val) = -1; ;}
    break;

  case 84:

    { (yyval.val) = 1; ;}
    break;

  case 85:

    {
#ifdef CSS_DEBUG
	kDebug( 6080 ) << "got ruleset" << endl << "  selector:";
#endif
	CSSParser *p = static_cast<CSSParser *>(parser);
	if ( (yyvsp[(1) - (2)].selectorList)  ) {
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

  case 86:

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

  case 87:

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

  case 88:

    {
        if ((yyvsp[(1) - (2)].selectorList)) qDeleteAll(*(yyvsp[(1) - (2)].selectorList));
	delete (yyvsp[(1) - (2)].selectorList);
	(yyvsp[(1) - (2)].selectorList) = 0;
	(yyval.selectorList) = 0;
    ;}
    break;

  case 89:

    {
	(yyval.selector) = (yyvsp[(1) - (1)].selector);
    ;}
    break;

  case 90:

    {
        (yyval.selector) = (yyvsp[(1) - (2)].selector);
    ;}
    break;

  case 91:

    {
        if ( !(yyvsp[(1) - (3)].selector) || !(yyvsp[(3) - (3)].selector) ) {
	    delete (yyvsp[(1) - (3)].selector);
	    delete (yyvsp[(3) - (3)].selector);
	    (yyval.selector) = 0;
        } else {
            (yyval.selector) = (yyvsp[(3) - (3)].selector);
            CSSSelector *end = (yyval.selector);
            while( end->tagHistory )
                end = end->tagHistory;
            end->relation = CSSSelector::Descendant;
            end->tagHistory = (yyvsp[(1) - (3)].selector);
        }
    ;}
    break;

  case 92:

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

  case 93:

    {
	delete (yyvsp[(1) - (2)].selector);
	(yyval.selector) = 0;
    ;}
    break;

  case 94:

    { (yyval.string).string = 0; (yyval.string).length = 0; ;}
    break;

  case 95:

    { static unsigned short star = '*'; (yyval.string).string = &star; (yyval.string).length = 1; ;}
    break;

  case 96:

    { (yyval.string) = (yyvsp[(1) - (2)].string); ;}
    break;

  case 97:

    {
	(yyval.selector) = new CSSSelector();
        (yyval.selector)->tagLocalName = LocalName::fromId(localNamePart((yyvsp[(1) - (1)].element)));
        (yyval.selector)->tagNamespace = NamespaceName::fromId(namespacePart((yyvsp[(1) - (1)].element)));
    ;}
    break;

  case 98:

    {
	(yyval.selector) = (yyvsp[(2) - (2)].selector);
        if ( (yyval.selector) ) {
            (yyval.selector)->tagLocalName = LocalName::fromId(localNamePart((yyvsp[(1) - (2)].element)));
            (yyval.selector)->tagNamespace = NamespaceName::fromId(namespacePart((yyvsp[(1) - (2)].element)));
        }
    ;}
    break;

  case 99:

    {
	(yyval.selector) = (yyvsp[(1) - (1)].selector);
        if ( (yyval.selector) ) {
            (yyval.selector)->tagLocalName = LocalName::fromId(anyLocalName);
            (yyval.selector)->tagNamespace = NamespaceName::fromId(static_cast<CSSParser*>(parser)->defaultNamespace());
        }
    ;}
    break;

  case 100:

    {
        (yyval.selector) = new CSSSelector();
        (yyval.selector)->tagLocalName = LocalName::fromId(localNamePart((yyvsp[(2) - (2)].element)));
        (yyval.selector)->tagNamespace = NamespaceName::fromId(namespacePart((yyvsp[(2) - (2)].element)));
	CSSParser *p = static_cast<CSSParser *>(parser);
        if (p->styleElement && p->styleElement->isCSSStyleSheet())
            static_cast<CSSStyleSheetImpl*>(p->styleElement)->determineNamespace((yyval.selector)->tagNamespace, domString((yyvsp[(1) - (2)].string)));
    ;}
    break;

  case 101:

    {
        (yyval.selector) = (yyvsp[(3) - (3)].selector);
        if ((yyval.selector)) {
            (yyval.selector)->tagLocalName = LocalName::fromId(localNamePart((yyvsp[(2) - (3)].element)));
            (yyval.selector)->tagNamespace = NamespaceName::fromId(namespacePart((yyvsp[(2) - (3)].element)));
            CSSParser *p = static_cast<CSSParser *>(parser);
            if (p->styleElement && p->styleElement->isCSSStyleSheet())
                static_cast<CSSStyleSheetImpl*>(p->styleElement)->determineNamespace((yyval.selector)->tagNamespace, domString((yyvsp[(1) - (3)].string)));
        }
    ;}
    break;

  case 102:

    {
        (yyval.selector) = (yyvsp[(2) - (2)].selector);
        if ((yyval.selector)) {
            (yyval.selector)->tagLocalName = LocalName::fromId(anyLocalName);
            (yyval.selector)->tagNamespace = NamespaceName::fromId(anyNamespace);
            CSSParser *p = static_cast<CSSParser *>(parser);
            if (p->styleElement && p->styleElement->isCSSStyleSheet())
                static_cast<CSSStyleSheetImpl*>(p->styleElement)->determineNamespace((yyval.selector)->tagNamespace, domString((yyvsp[(1) - (2)].string)));
        }
    ;}
    break;

  case 103:

    {
	(yyval.selector) = new CSSSelector();
        (yyval.selector)->tagLocalName = LocalName::fromId(localNamePart((yyvsp[(1) - (1)].element)));
        (yyval.selector)->tagNamespace = NamespaceName::fromId(namespacePart((yyvsp[(1) - (1)].element)));
    ;}
    break;

  case 104:

    {
	(yyval.selector) = (yyvsp[(1) - (1)].selector);
        if ( (yyval.selector) ) {
            (yyval.selector)->tagLocalName = LocalName::fromId(anyLocalName);
            (yyval.selector)->tagNamespace = NamespaceName::fromId(static_cast<CSSParser*>(parser)->defaultNamespace());
        }
    ;}
    break;

  case 105:

    {
        (yyval.selector) = new CSSSelector();
        (yyval.selector)->tagLocalName = LocalName::fromId(localNamePart((yyvsp[(2) - (2)].element)));
        (yyval.selector)->tagNamespace = NamespaceName::fromId(namespacePart((yyvsp[(2) - (2)].element)));
	CSSParser *p = static_cast<CSSParser *>(parser);
        if (p->styleElement && p->styleElement->isCSSStyleSheet())
            static_cast<CSSStyleSheetImpl*>(p->styleElement)->determineNamespace((yyval.selector)->tagNamespace, domString((yyvsp[(1) - (2)].string)));
    ;}
    break;

  case 106:

    {
        (yyval.selector) = (yyvsp[(2) - (2)].selector);
        if ((yyval.selector)) {
            (yyval.selector)->tagLocalName = LocalName::fromId(anyLocalName);
            (yyval.selector)->tagNamespace = NamespaceName::fromId(anyNamespace);
            CSSParser *p = static_cast<CSSParser *>(parser);
            if (p->styleElement && p->styleElement->isCSSStyleSheet())
                static_cast<CSSStyleSheetImpl*>(p->styleElement)->determineNamespace((yyval.selector)->tagNamespace, domString((yyvsp[(1) - (2)].string)));
        }
    ;}
    break;

  case 107:

    {
      CSSParser *p = static_cast<CSSParser *>(parser);
      (yyval.element) = makeId(p->defaultNamespace(), p->getLocalNameId(domString((yyvsp[(1) - (1)].string))));
    ;}
    break;

  case 108:

    {
	(yyval.element) = makeId(static_cast<CSSParser*>(parser)->defaultNamespace(), anyLocalName);
    ;}
    break;

  case 109:

    {
	(yyval.selector) = (yyvsp[(1) - (1)].selector);
    ;}
    break;

  case 110:

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

  case 111:

    {
	delete (yyvsp[(1) - (2)].selector);
	(yyval.selector) = 0;
    ;}
    break;

  case 112:

    {
        CSSParser *p = static_cast<CSSParser *>(parser);

	(yyval.selector) = new CSSSelector();
	(yyval.selector)->match = CSSSelector::Id;
        (yyval.selector)->attrLocalName = LocalName::fromId(localNamePart(ATTR_ID));
        (yyval.selector)->attrNamespace = NamespaceName::fromId(namespacePart(ATTR_ID));

        bool caseSensitive = p->document()->htmlMode() == DocumentImpl::XHtml || !p->document()->inCompatMode();
        if (caseSensitive)
            (yyval.selector)->value = domString((yyvsp[(1) - (1)].string));
        else
            (yyval.selector)->value = domString((yyvsp[(1) - (1)].string)).lower();
    ;}
    break;

  case 116:

    {
        CSSParser *p = static_cast<CSSParser *>(parser);

	(yyval.selector) = new CSSSelector();
	(yyval.selector)->match = CSSSelector::Class;
        (yyval.selector)->attrLocalName = LocalName::fromId(localNamePart(ATTR_CLASS));
        (yyval.selector)->attrNamespace = NamespaceName::fromId(namespacePart(ATTR_CLASS));

        bool caseSensitive = p->document()->htmlMode() == DocumentImpl::XHtml || !p->document()->inCompatMode();
        if (caseSensitive)
            (yyval.selector)->value = domString((yyvsp[(2) - (2)].string));
        else
            (yyval.selector)->value = domString((yyvsp[(2) - (2)].string)).lower();
    ;}
    break;

  case 117:

    {
      CSSParser *p = static_cast<CSSParser *>(parser);
      (yyval.attribute) = makeId(emptyNamespace, p->getLocalNameId(domString((yyvsp[(1) - (2)].string))));
    ;}
    break;

  case 118:

    {
	(yyval.selector) = new CSSSelector();
        (yyval.selector)->attrLocalName = LocalName::fromId(localNamePart((yyvsp[(3) - (4)].attribute)));
        (yyval.selector)->attrNamespace = NamespaceName::fromId(namespacePart((yyvsp[(3) - (4)].attribute)));
	(yyval.selector)->match = CSSSelector::Set;
    ;}
    break;

  case 119:

    {
	(yyval.selector) = new CSSSelector();
        (yyval.selector)->attrLocalName = LocalName::fromId(localNamePart((yyvsp[(3) - (8)].attribute)));
        (yyval.selector)->attrNamespace = NamespaceName::fromId(namespacePart((yyvsp[(3) - (8)].attribute)));
	(yyval.selector)->match = (yyvsp[(4) - (8)].match);
	(yyval.selector)->value = domString((yyvsp[(6) - (8)].string));
    ;}
    break;

  case 120:

    {
        (yyval.selector) = new CSSSelector();
        (yyval.selector)->attrLocalName = LocalName::fromId(localNamePart((yyvsp[(4) - (5)].attribute)));
        (yyval.selector)->attrNamespace = NamespaceName::fromId(namespacePart((yyvsp[(4) - (5)].attribute)));
        (yyval.selector)->match = CSSSelector::Set;
        CSSParser *p = static_cast<CSSParser *>(parser);
        if (p->styleElement && p->styleElement->isCSSStyleSheet())
            static_cast<CSSStyleSheetImpl*>(p->styleElement)->determineNamespace((yyval.selector)->attrNamespace, domString((yyvsp[(3) - (5)].string)));
    ;}
    break;

  case 121:

    {
        (yyval.selector) = new CSSSelector();
        (yyval.selector)->attrLocalName = LocalName::fromId(localNamePart((yyvsp[(4) - (9)].attribute)));
        (yyval.selector)->attrNamespace = NamespaceName::fromId(namespacePart((yyvsp[(4) - (9)].attribute)));
        (yyval.selector)->match = (CSSSelector::Match)(yyvsp[(5) - (9)].match);
        (yyval.selector)->value = domString((yyvsp[(7) - (9)].string));
        CSSParser *p = static_cast<CSSParser *>(parser);
        if (p->styleElement && p->styleElement->isCSSStyleSheet())
            static_cast<CSSStyleSheetImpl*>(p->styleElement)->determineNamespace((yyval.selector)->attrNamespace, domString((yyvsp[(3) - (9)].string)));
   ;}
    break;

  case 122:

    {
	(yyval.match) = CSSSelector::Exact;
    ;}
    break;

  case 123:

    {
	(yyval.match) = CSSSelector::List;
    ;}
    break;

  case 124:

    {
	(yyval.match) = CSSSelector::Hyphen;
    ;}
    break;

  case 125:

    {
	(yyval.match) = CSSSelector::Begin;
    ;}
    break;

  case 126:

    {
	(yyval.match) = CSSSelector::End;
    ;}
    break;

  case 127:

    {
	(yyval.match) = CSSSelector::Contain;
    ;}
    break;

  case 130:

    {
	(yyval.selector) = new CSSSelector();
	(yyval.selector)->match = CSSSelector::PseudoClass;
	(yyval.selector)->value = domString((yyvsp[(2) - (2)].string));
    ;}
    break;

  case 131:

    {
	(yyval.selector) = new CSSSelector();
	(yyval.selector)->match = CSSSelector::PseudoElement;
        (yyval.selector)->value = domString((yyvsp[(3) - (3)].string));
    ;}
    break;

  case 132:

    {
        (yyval.selector) = new CSSSelector();
        (yyval.selector)->match = CSSSelector::PseudoClass;
        (yyval.selector)->string_arg = domString((yyvsp[(4) - (6)].string));
        (yyval.selector)->value = domString((yyvsp[(2) - (6)].string));
    ;}
    break;

  case 133:

    {
        (yyval.selector) = new CSSSelector();
        (yyval.selector)->match = CSSSelector::PseudoClass;
        (yyval.selector)->string_arg = QString::number((yyvsp[(5) - (7)].val));
        (yyval.selector)->value = domString((yyvsp[(2) - (7)].string));
    ;}
    break;

  case 134:

    {
        (yyval.selector) = new CSSSelector();
        (yyval.selector)->match = CSSSelector::PseudoClass;
        (yyval.selector)->string_arg = domString((yyvsp[(4) - (6)].string));
        (yyval.selector)->value = domString((yyvsp[(2) - (6)].string));
    ;}
    break;

  case 135:

    {
        (yyval.selector) = new CSSSelector();
        (yyval.selector)->match = CSSSelector::PseudoClass;
        (yyval.selector)->string_arg = domString((yyvsp[(4) - (6)].string));
        (yyval.selector)->value = domString((yyvsp[(2) - (6)].string));
    ;}
    break;

  case 136:

    {
        (yyval.selector) = new CSSSelector();
        (yyval.selector)->match = CSSSelector::PseudoClass;
        (yyval.selector)->simpleSelector = (yyvsp[(4) - (6)].selector);
        (yyval.selector)->value = domString((yyvsp[(2) - (6)].string));
    ;}
    break;

  case 137:

    {
	(yyval.ok) = (yyvsp[(3) - (4)].ok);
    ;}
    break;

  case 138:

    {
	(yyval.ok) = false;
    ;}
    break;

  case 139:

    {
	(yyval.ok) = (yyvsp[(3) - (4)].ok);
    ;}
    break;

  case 140:

    {
	(yyval.ok) = (yyvsp[(3) - (5)].ok);
	if ( (yyvsp[(4) - (5)].ok) )
	    (yyval.ok) = (yyvsp[(4) - (5)].ok);
    ;}
    break;

  case 141:

    {
	(yyval.ok) = (yyvsp[(3) - (5)].ok);
    ;}
    break;

  case 142:

    {
	(yyval.ok) = (yyvsp[(1) - (3)].ok);
    ;}
    break;

  case 143:

    {
        (yyval.ok) = false;
    ;}
    break;

  case 144:

    {
	(yyval.ok) = (yyvsp[(1) - (4)].ok);
	if ( (yyvsp[(2) - (4)].ok) )
	    (yyval.ok) = (yyvsp[(2) - (4)].ok);
    ;}
    break;

  case 145:

    {
        (yyval.ok) = (yyvsp[(1) - (4)].ok);
    ;}
    break;

  case 146:

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

  case 147:

    {
        (yyval.ok) = false;
    ;}
    break;

  case 148:

    {
	QString str = qString((yyvsp[(1) - (2)].string));
	(yyval.prop_id) = getPropertyID( str.toLower().toLatin1(), str.length() );
    ;}
    break;

  case 149:

    { (yyval.b) = true; ;}
    break;

  case 150:

    { (yyval.b) = false; ;}
    break;

  case 151:

    {
	(yyval.valueList) = new ValueList;
	(yyval.valueList)->addValue( (yyvsp[(1) - (1)].value) );
    ;}
    break;

  case 152:

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

  case 153:

    {
	(yyval.tok) = '/';
    ;}
    break;

  case 154:

    {
	(yyval.tok) = ',';
    ;}
    break;

  case 155:

    {
        (yyval.tok) = 0;
  ;}
    break;

  case 156:

    { (yyval.value) = (yyvsp[(1) - (1)].value); ;}
    break;

  case 157:

    { (yyval.value) = (yyvsp[(2) - (2)].value); (yyval.value).fValue *= (yyvsp[(1) - (2)].val); ;}
    break;

  case 158:

    { (yyval.value).id = 0; (yyval.value).string = (yyvsp[(1) - (2)].string); (yyval.value).unit = CSSPrimitiveValue::CSS_DIMENSION; ;}
    break;

  case 159:

    { (yyval.value).id = 0; (yyval.value).string = (yyvsp[(1) - (2)].string); (yyval.value).unit = CSSPrimitiveValue::CSS_STRING; ;}
    break;

  case 160:

    {
      QString str = qString( (yyvsp[(1) - (2)].string) );
      (yyval.value).id = getValueID( str.toLower().toLatin1(), str.length() );
      (yyval.value).unit = CSSPrimitiveValue::CSS_IDENT;
      (yyval.value).string = (yyvsp[(1) - (2)].string);
  ;}
    break;

  case 161:

    { (yyval.value).id = 0; (yyval.value).string = (yyvsp[(1) - (2)].string); (yyval.value).unit = CSSPrimitiveValue::CSS_URI; ;}
    break;

  case 162:

    { (yyval.value).id = 0; (yyval.value).iValue = 0; (yyval.value).unit = CSSPrimitiveValue::CSS_UNKNOWN;/* ### */ ;}
    break;

  case 163:

    { (yyval.value).id = 0; (yyval.value).string = (yyvsp[(1) - (1)].string); (yyval.value).unit = CSSPrimitiveValue::CSS_RGBCOLOR; ;}
    break;

  case 164:

    {
      (yyval.value) = (yyvsp[(1) - (1)].value);
  ;}
    break;

  case 165:

    { (yyval.value).id = 0; (yyval.value).isInt = true; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_NUMBER; ;}
    break;

  case 166:

    { (yyval.value).id = 0; (yyval.value).isInt = false; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_NUMBER; ;}
    break;

  case 167:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_PERCENTAGE; ;}
    break;

  case 168:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_PX; ;}
    break;

  case 169:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_CM; ;}
    break;

  case 170:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_MM; ;}
    break;

  case 171:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_IN; ;}
    break;

  case 172:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_PT; ;}
    break;

  case 173:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_PC; ;}
    break;

  case 174:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_DEG; ;}
    break;

  case 175:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_RAD; ;}
    break;

  case 176:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_GRAD; ;}
    break;

  case 177:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_MS; ;}
    break;

  case 178:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_S; ;}
    break;

  case 179:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_HZ; ;}
    break;

  case 180:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_KHZ; ;}
    break;

  case 181:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_EMS; ;}
    break;

  case 182:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = Value::Q_EMS; ;}
    break;

  case 183:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_EXS; ;}
    break;

  case 184:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_DPI; ;}
    break;

  case 185:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_DPCM; ;}
    break;

  case 186:

    {
      Function *f = new Function;
      f->name = (yyvsp[(1) - (5)].string);
      f->args = (yyvsp[(3) - (5)].valueList);
      (yyval.value).id = 0;
      (yyval.value).unit = Value::Function;
      (yyval.value).function = f;
  ;}
    break;

  case 187:

    {
      Function *f = new Function;
      f->name = (yyvsp[(1) - (3)].string);
      f->args = 0;
      (yyval.value).id = 0;
      (yyval.value).unit = Value::Function;
      (yyval.value).function = f;
  ;}
    break;

  case 188:

    { (yyval.string) = (yyvsp[(1) - (2)].string); ;}
    break;

  case 189:

    {
	(yyval.rule) = 0;
#ifdef CSS_DEBUG
	kDebug( 6080 ) << "skipped invalid @-rule";
#endif
    ;}
    break;

  case 190:

    {
	(yyval.rule) = 0;
#ifdef CSS_DEBUG
	kDebug( 6080 ) << "skipped invalid @-rule";
#endif
    ;}
    break;

  case 191:

    {
	(yyval.rule) = 0;
#ifdef CSS_DEBUG
	kDebug( 6080 ) << "skipped invalid rule";
#endif
    ;}
    break;



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
      /* If just tried and failed to reuse lookahead token after an
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

  /* Else will try to reuse lookahead token after shifting the error
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

#if !defined(yyoverflow) || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
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






