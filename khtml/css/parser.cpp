
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
#define YYLAST   567

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  75
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  66
/* YYNRULES -- Number of rules.  */
#define YYNRULES  193
/* YYNRULES -- Number of states.  */
#define YYNSTATES  360

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
      64,    65,    19,    67,    66,    70,    17,    73,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    16,    63,
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
      32,    35,    41,    47,    48,    51,    52,    55,    58,    59,
      61,    67,    71,    75,    76,    80,    87,    91,    95,    96,
     100,   107,   111,   115,   116,   119,   120,   124,   126,   128,
     130,   132,   134,   136,   139,   141,   143,   145,   148,   150,
     152,   155,   156,   161,   169,   171,   177,   178,   182,   183,
     185,   187,   189,   194,   195,   197,   199,   204,   207,   215,
     222,   223,   227,   230,   234,   238,   242,   246,   250,   253,
     256,   259,   261,   263,   266,   268,   273,   276,   278,   281,
     285,   289,   292,   294,   297,   300,   302,   305,   307,   310,
     314,   317,   319,   321,   324,   327,   329,   331,   333,   336,
     339,   341,   343,   345,   347,   350,   353,   358,   367,   373,
     383,   385,   387,   389,   391,   393,   395,   397,   399,   402,
     406,   411,   416,   421,   426,   433,   438,   443,   448,   454,
     460,   464,   468,   473,   478,   484,   487,   490,   493,   494,
     496,   500,   503,   506,   507,   509,   512,   515,   518,   521,
     524,   527,   529,   531,   534,   537,   540,   543,   546,   549,
     552,   555,   558,   561,   564,   567,   570,   573,   576,   579,
     582,   585,   588,   591,   594,   600,   604,   607,   611,   615,
     618,   624,   628,   630
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
      76,     0,    -1,    84,    83,    86,    88,    91,    -1,    78,
      82,    -1,    79,    82,    -1,    80,    82,    -1,    81,    82,
      -1,   111,    -1,    87,    -1,    27,    61,    82,    77,    82,
      62,    -1,    28,   126,    -1,    29,    61,    82,   131,    62,
      -1,    30,     4,    82,   101,    62,    -1,    -1,    82,     4,
      -1,    -1,    83,     5,    -1,    83,     4,    -1,    -1,    85,
      -1,    25,    82,    11,    82,    63,    -1,    25,     1,   139,
      -1,    25,     1,    63,    -1,    -1,    86,    87,    83,    -1,
      21,    82,    94,    82,   102,    63,    -1,    21,     1,   139,
      -1,    21,     1,    63,    -1,    -1,    88,    89,    83,    -1,
      26,    82,    90,    94,    82,    63,    -1,    26,     1,   139,
      -1,    26,     1,    63,    -1,    -1,    12,     4,    -1,    -1,
      91,    92,    83,    -1,   111,    -1,   104,    -1,   107,    -1,
     108,    -1,   138,    -1,   137,    -1,    87,     1,    -1,   111,
      -1,   138,    -1,   137,    -1,    87,     1,    -1,    11,    -1,
      57,    -1,    12,    82,    -1,    -1,    16,    82,   131,    82,
      -1,    64,    82,    95,    82,    96,    65,    82,    -1,    97,
      -1,    98,    82,    34,    82,    97,    -1,    -1,    34,    82,
      98,    -1,    -1,    32,    -1,    33,    -1,    98,    -1,   100,
      82,   106,    99,    -1,    -1,   103,    -1,   101,    -1,   103,
      66,    82,   101,    -1,   103,     1,    -1,    23,    82,   103,
      61,    82,   105,    62,    -1,    23,    82,    61,    82,   105,
      62,    -1,    -1,   105,    93,    82,    -1,    12,    82,    -1,
      22,     1,   139,    -1,    22,     1,    63,    -1,    24,    82,
     126,    -1,    24,     1,   139,    -1,    24,     1,    63,    -1,
      67,    82,    -1,    68,    82,    -1,    69,    82,    -1,    70,
      -1,    67,    -1,   112,   126,    -1,   113,    -1,   112,    66,
      82,   113,    -1,   112,     1,    -1,   115,    -1,   113,     4,
      -1,   113,     4,   115,    -1,   113,   109,   115,    -1,   113,
       1,    -1,    20,    -1,    19,    20,    -1,    12,    20,    -1,
     117,    -1,   117,   118,    -1,   118,    -1,   114,   117,    -1,
     114,   117,   118,    -1,   114,   118,    -1,   117,    -1,   119,
      -1,   114,   117,    -1,   114,   119,    -1,    12,    -1,    19,
      -1,   119,    -1,   118,   119,    -1,   118,     1,    -1,    14,
      -1,   120,    -1,   122,    -1,   125,    -1,    17,    12,    -1,
      12,    82,    -1,    18,    82,   121,    71,    -1,    18,    82,
     121,   123,    82,   124,    82,    71,    -1,    18,    82,   114,
     121,    71,    -1,    18,    82,   114,   121,   123,    82,   124,
      82,    71,    -1,    72,    -1,     6,    -1,     7,    -1,     8,
      -1,     9,    -1,    10,    -1,    12,    -1,    11,    -1,    16,
      12,    -1,    16,    16,    12,    -1,    16,    58,    13,    65,
      -1,    16,    58,    56,    65,    -1,    16,    58,    12,    65,
      -1,    16,    58,    11,    65,    -1,    16,    59,    82,   116,
      82,    65,    -1,    61,    82,   128,    62,    -1,    61,    82,
       1,    62,    -1,    61,    82,   127,    62,    -1,    61,    82,
     127,   128,    62,    -1,    61,    82,   127,     1,    62,    -1,
     128,    63,    82,    -1,     1,    63,    82,    -1,   127,   128,
      63,    82,    -1,   127,     1,    63,    82,    -1,   129,    16,
      82,   131,   130,    -1,     1,   139,    -1,    12,    82,    -1,
      31,    82,    -1,    -1,   133,    -1,   131,   132,   133,    -1,
      73,    82,    -1,    66,    82,    -1,    -1,   134,    -1,   110,
     134,    -1,    53,    82,    -1,    11,    82,    -1,    12,    82,
      -1,    57,    82,    -1,    60,    82,    -1,   136,    -1,   135,
      -1,    56,    82,    -1,    55,    82,    -1,    54,    82,    -1,
      38,    82,    -1,    39,    82,    -1,    40,    82,    -1,    41,
      82,    -1,    42,    82,    -1,    43,    82,    -1,    44,    82,
      -1,    45,    82,    -1,    46,    82,    -1,    47,    82,    -1,
      48,    82,    -1,    49,    82,    -1,    50,    82,    -1,    36,
      82,    -1,    35,    82,    -1,    37,    82,    -1,    51,    82,
      -1,    52,    82,    -1,    58,    82,   131,    65,    82,    -1,
      58,    82,     1,    -1,    15,    82,    -1,    74,     1,   139,
      -1,    74,     1,    63,    -1,     1,   139,    -1,    61,     1,
     140,     1,    62,    -1,    61,     1,    62,    -1,   139,    -1,
     140,     1,   139,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   292,   292,   293,   294,   295,   296,   300,   301,   305,
     312,   318,   343,   350,   351,   354,   356,   357,   360,   362,
     365,   374,   376,   380,   382,   393,   403,   406,   412,   413,
     417,   425,   426,   430,   431,   434,   436,   447,   448,   449,
     450,   451,   452,   453,   457,   458,   459,   460,   464,   465,
     469,   475,   478,   484,   490,   494,   501,   504,   510,   513,
     516,   522,   525,   531,   534,   539,   543,   548,   555,   566,
     578,   579,   589,   607,   610,   616,   623,   626,   632,   633,
     634,   638,   639,   643,   665,   678,   696,   706,   709,   712,
     726,   740,   747,   748,   749,   753,   758,   765,   772,   780,
     790,   803,   808,   815,   823,   836,   840,   846,   849,   859,
     866,   880,   881,   882,   886,   903,   910,   916,   923,   932,
     945,   948,   951,   954,   957,   960,   966,   967,   971,   977,
     983,   990,   997,  1004,  1011,  1020,  1023,  1026,  1029,  1034,
    1040,  1044,  1047,  1052,  1058,  1080,  1086,  1093,  1094,  1098,
    1102,  1118,  1121,  1124,  1130,  1131,  1133,  1134,  1135,  1141,
    1142,  1143,  1145,  1151,  1152,  1153,  1154,  1155,  1156,  1157,
    1158,  1159,  1160,  1161,  1162,  1163,  1164,  1165,  1166,  1167,
    1168,  1169,  1170,  1171,  1176,  1184,  1200,  1207,  1213,  1222,
    1248,  1249,  1253,  1254
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
     265,   266,   267,   268,   269,   270,    58,    46,    91,    42,
     124,   271,   272,   273,   274,   275,   276,   277,   278,   279,
     280,   281,   282,   283,   284,   285,   286,   287,   288,   289,
     290,   291,   292,   293,   294,   295,   296,   297,   298,   299,
     300,   301,   302,   303,   304,   305,   306,   307,   308,   309,
     310,   123,   125,    59,    40,    41,    44,    43,   126,    62,
      45,    93,    61,    47,    64
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    75,    76,    76,    76,    76,    76,    77,    77,    78,
      79,    80,    81,    82,    82,    83,    83,    83,    84,    84,
      85,    85,    85,    86,    86,    87,    87,    87,    88,    88,
      89,    89,    89,    90,    90,    91,    91,    92,    92,    92,
      92,    92,    92,    92,    93,    93,    93,    93,    94,    94,
      95,    96,    96,    97,    98,    98,    99,    99,   100,   100,
     100,   101,   101,   102,   102,   103,   103,   103,   104,   104,
     105,   105,   106,   107,   107,   108,   108,   108,   109,   109,
     109,   110,   110,   111,   112,   112,   112,   113,   113,   113,
     113,   113,   114,   114,   114,   115,   115,   115,   115,   115,
     115,   116,   116,   116,   116,   117,   117,   118,   118,   118,
     119,   119,   119,   119,   120,   121,   122,   122,   122,   122,
     123,   123,   123,   123,   123,   123,   124,   124,   125,   125,
     125,   125,   125,   125,   125,   126,   126,   126,   126,   126,
     127,   127,   127,   127,   128,   128,   129,   130,   130,   131,
     131,   132,   132,   132,   133,   133,   133,   133,   133,   133,
     133,   133,   133,   134,   134,   134,   134,   134,   134,   134,
     134,   134,   134,   134,   134,   134,   134,   134,   134,   134,
     134,   134,   134,   134,   135,   135,   136,   137,   137,   138,
     139,   139,   140,   140
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
       0,     3,     2,     3,     3,     3,     3,     3,     2,     2,
       2,     1,     1,     2,     1,     4,     2,     1,     2,     3,
       3,     2,     1,     2,     2,     1,     2,     1,     2,     3,
       2,     1,     1,     2,     2,     1,     1,     1,     2,     2,
       1,     1,     1,     1,     2,     2,     4,     8,     5,     9,
       1,     1,     1,     1,     1,     1,     1,     1,     2,     3,
       4,     4,     4,     4,     6,     4,     4,     4,     5,     5,
       3,     3,     4,     4,     5,     2,     2,     2,     0,     1,
       3,     2,     2,     0,     1,     2,     2,     2,     2,     2,
       2,     1,     1,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     5,     3,     2,     3,     3,     2,
       5,     3,     1,     3
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
     105,   110,     0,     0,    13,   106,    92,     0,    13,     8,
       7,     0,     0,     0,    87,    95,     0,   107,   111,   112,
     113,     0,    13,     0,     0,     0,    13,    13,    13,    13,
      13,    13,    13,    13,    13,    13,    13,    13,    13,    13,
      13,    13,    13,    13,    13,    13,    13,    13,    13,    13,
      13,    13,    13,    13,    82,    81,     0,   153,   149,   154,
     162,   161,    59,    60,    13,    54,    61,    13,     0,    15,
      35,   191,   192,     0,    20,    94,   128,     0,     0,    13,
     114,     0,    93,     0,     0,     0,    86,    13,    83,    91,
      88,    13,    13,    13,     0,   105,   106,    98,     0,     0,
     109,   108,   136,    13,   145,   146,     0,   137,     0,   135,
      13,    13,   157,   158,   186,   180,   179,   181,   166,   167,
     168,   169,   170,   171,   172,   173,   174,   175,   176,   177,
     178,   182,   183,   156,   165,   164,   163,   159,     0,   160,
     155,    11,    13,    13,     0,     0,     0,     0,    12,    24,
       0,    15,     0,     0,   129,     0,     0,     0,     0,     0,
      13,     0,     0,     0,    27,    26,    48,    49,    13,     9,
       0,    89,    78,    79,    80,    90,     0,   141,   139,    13,
     138,    13,   140,     0,   185,   153,   152,   151,   150,    13,
      13,    13,    13,    56,     0,    33,    29,     0,     0,    13,
       0,     0,     0,    15,    38,    39,    40,    37,    42,    41,
     190,   193,   133,   132,   130,   131,     0,    13,   101,   102,
     115,    13,     0,   121,   122,   123,   124,   125,   116,   120,
      13,    58,     0,   143,   142,   153,    13,    50,    51,     0,
      72,    13,    62,    32,    31,     0,     0,   189,     0,    58,
       0,     0,     0,    43,    36,   103,   104,     0,   118,    13,
       0,    65,     0,     0,    13,   144,   184,    13,     0,    55,
       0,    34,    13,    74,    73,    13,     0,    77,    76,    75,
     188,   187,   134,     0,   127,   126,    13,    25,    67,    13,
     147,     0,    13,    57,     0,    70,    13,    13,     0,    58,
     153,    53,    30,     0,    70,     0,   117,    66,    52,    69,
       0,    13,    44,    46,    45,     0,   119,    47,    71,    68
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     6,    48,     7,     8,     9,    10,   186,    25,    11,
      12,    37,   350,   110,   191,   286,   192,   243,   351,   208,
     230,   308,   105,   106,   282,   107,   301,   302,   303,   244,
     343,   233,   245,   246,   134,    96,   352,    51,    52,    53,
      54,   257,    55,    56,    57,    58,   203,    59,   270,   326,
      60,    17,    63,    64,    65,   305,    97,   184,    98,    99,
     100,   101,   353,   354,   144,   113
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -166
static const yytype_int16 yypact[] =
{
     293,   172,   -36,   -16,   -10,   116,   135,  -166,  -166,  -166,
    -166,  -166,  -166,    75,    38,  -166,  -166,  -166,  -166,  -166,
    -166,   236,   236,   236,   236,   166,    27,  -166,  -166,  -166,
    -166,   490,    23,   431,   229,  -166,  -166,   118,   301,   124,
     257,  -166,   284,   234,  -166,   282,  -166,   272,  -166,  -166,
    -166,    39,   136,   433,  -166,   376,   176,  -166,  -166,  -166,
    -166,   285,  -166,   122,   112,   288,  -166,  -166,  -166,  -166,
    -166,  -166,  -166,  -166,  -166,  -166,  -166,  -166,  -166,  -166,
    -166,  -166,  -166,  -166,  -166,  -166,  -166,  -166,  -166,  -166,
    -166,  -166,  -166,  -166,  -166,  -166,   382,   235,  -166,  -166,
    -166,  -166,  -166,  -166,  -166,  -166,    29,  -166,   255,  -166,
     319,  -166,  -166,   351,  -166,  -166,  -166,   383,   150,  -166,
    -166,    19,  -166,   156,   248,   123,  -166,  -166,  -166,  -166,
     547,  -166,  -166,  -166,   547,  -166,  -166,   376,   197,   387,
    -166,  -166,  -166,  -166,  -166,   236,   345,  -166,   247,  -166,
    -166,  -166,   236,   236,   236,   236,   236,   236,   236,   236,
     236,   236,   236,   236,   236,   236,   236,   236,   236,   236,
     236,   236,   236,   236,   236,   236,   236,   236,   329,   236,
    -166,  -166,  -166,  -166,   481,   129,    30,   148,  -166,   166,
     267,  -166,   337,   354,  -166,   333,   344,   374,   375,   538,
     257,   282,   390,   138,  -166,  -166,  -166,  -166,  -166,  -166,
     538,  -166,   236,   236,   236,  -166,   396,   236,  -166,  -166,
    -166,  -166,   236,   431,  -166,   226,   236,   236,  -166,  -166,
    -166,  -166,  -166,   407,   173,   277,   166,   397,   443,  -166,
     266,   458,   459,  -166,  -166,  -166,  -166,  -166,  -166,  -166,
    -166,  -166,  -166,  -166,  -166,  -166,   433,  -166,  -166,  -166,
     236,  -166,   241,  -166,  -166,  -166,  -166,  -166,  -166,  -166,
    -166,   163,   155,   236,   236,   253,  -166,   236,    28,   104,
     236,  -166,  -166,  -166,  -166,   457,     5,  -166,   194,   221,
     211,   139,   271,  -166,   166,  -166,  -166,    37,  -166,  -166,
     283,  -166,   427,    59,  -166,  -166,   236,  -166,   430,  -166,
     104,  -166,  -166,  -166,  -166,  -166,    58,  -166,  -166,  -166,
    -166,  -166,  -166,   283,  -166,  -166,  -166,  -166,  -166,  -166,
     236,   431,  -166,    29,   125,   236,  -166,  -166,    22,   229,
     113,   236,  -166,    36,   236,    26,  -166,  -166,   236,  -166,
     496,  -166,  -166,  -166,  -166,    95,  -166,  -166,   236,  -166
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -166,  -166,  -166,  -166,  -166,  -166,  -166,    -1,   -90,  -166,
    -166,  -166,   -27,  -166,  -166,  -166,  -166,  -166,  -166,   213,
    -166,  -166,   224,   190,  -166,  -166,   -32,  -166,   216,  -166,
     168,  -166,  -166,  -166,  -166,  -166,   -28,  -166,   303,  -100,
      24,  -166,   -48,   -33,   -44,  -166,   312,  -166,   278,   192,
    -166,   -50,  -166,   480,  -166,  -166,  -165,  -166,   360,   449,
    -166,  -166,   355,   357,    -2,  -166
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -149
static const yytype_int16 yytable[] =
{
      14,   128,   108,    50,    49,   137,    21,    22,    23,    24,
     109,    28,   141,   225,    31,    32,   206,    33,    34,   189,
     138,   202,   139,    29,    61,    15,    29,    29,    38,    39,
      29,   200,    29,   -13,    29,    62,   112,   237,   201,    46,
     126,    29,    29,   121,   307,    16,   124,   125,    40,    30,
      41,    18,    42,    43,    44,    45,    46,    47,   275,   328,
     328,   145,   207,   -13,   231,   152,   153,   154,   155,   156,
     157,   158,   159,   160,   161,   162,   163,   164,   165,   166,
     167,   168,   169,   170,   171,   172,   173,   174,   175,   176,
     177,   178,   179,   346,   141,   141,   237,   356,   349,   256,
      16,   236,   322,   185,   216,   127,   187,    40,    29,    41,
     241,    42,    43,    44,    45,    46,    47,   -13,   199,   336,
      19,   205,   -64,   146,   329,   329,   210,    29,    29,    29,
     212,   213,   214,    29,    62,    20,    26,   129,    27,    47,
     130,   229,   217,    29,   263,   264,   265,   266,   267,   222,
     223,   258,    29,   294,   211,   259,   129,   359,   215,   130,
     232,   195,   196,   197,   247,   242,   340,    29,   104,   241,
      35,    36,   141,    13,   149,   150,   -13,   140,   -13,   182,
     -97,   226,   227,   -13,   147,   209,   183,   114,   342,   235,
      41,   251,    42,    43,    44,   102,   103,   -84,   140,   260,
      16,  -100,   -84,   131,   132,   133,   198,   271,   295,   268,
     269,    41,   296,    42,    43,    44,   -85,    26,   273,   204,
     274,   -85,   131,   132,   133,    29,   -63,   104,   277,   278,
     279,   280,   284,    29,    26,   287,   283,   -97,   289,   291,
      29,   319,   -97,   -97,   -97,   -97,   120,   263,   264,   265,
     266,   267,    29,   102,   103,    26,   297,   313,  -100,   206,
     260,   102,   103,  -100,  -100,  -100,  -100,   290,   234,   300,
     -13,   -13,    26,   123,   317,   306,   -13,   115,   -13,   -13,
     310,    29,   315,   -13,   304,   104,   314,    29,   318,   285,
     321,   276,   182,   104,   324,   325,   116,   181,   323,   183,
     117,   182,   122,   330,   151,   207,   331,   347,   183,   220,
     221,   334,   298,   269,   335,  -148,  -148,   188,     1,   182,
       2,     3,     4,     5,   -13,   338,   183,   -13,   339,   -13,
     224,   341,    26,    29,   320,   344,   345,    -2,   237,   348,
      66,    67,   118,   119,    68,   190,    26,   142,   143,    40,
     358,    41,   193,    42,    43,    44,    45,    46,    47,   238,
     239,   240,    26,   111,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,   140,    93,
      41,   -96,    42,    43,    44,   194,    94,   140,   252,    95,
     -99,    41,   261,    42,    43,    44,    26,   218,   219,   253,
      41,   241,    42,    43,    44,    26,   250,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    29,    88,    89,    90,   254,
     255,   281,    66,    67,   288,   135,    68,    41,   -96,    42,
      43,    44,   136,   -96,   -96,   -96,   -96,   -99,    26,   292,
     293,   311,   -99,   -99,   -99,   -99,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
     327,    93,    66,    67,    29,   332,    68,   357,    94,   312,
     333,    95,    40,   309,    41,   316,    42,    43,    44,    45,
      46,    47,   355,   272,   262,   337,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
     299,    93,    29,   148,   228,   180,     0,   248,    94,   249,
      40,    95,    41,     0,    42,    43,    44,    45,    46,    40,
       0,    41,     0,    42,    43,    44,    45,    46
};

static const yytype_int16 yycheck[] =
{
       1,    51,    34,    31,    31,    53,     7,     8,     9,    10,
      37,    13,    56,   178,    15,    16,    11,    18,    19,   109,
      53,   121,    55,     4,     1,    61,     4,     4,     1,    30,
       4,    12,     4,     4,     4,    12,    38,     1,    19,    20,
       1,     4,     4,    44,    16,    61,    47,    48,    12,    11,
      14,    61,    16,    17,    18,    19,    20,    21,   223,     1,
       1,    62,    57,    34,    34,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    71,   138,   139,     1,    71,    62,   199,
      61,   191,    65,   104,   137,    66,   107,    12,     4,    14,
      74,    16,    17,    18,    19,    20,    21,     4,   119,    61,
       4,   123,    63,     1,    66,    66,   127,     4,     4,     4,
     131,   132,   133,     4,    12,     0,    61,     1,    63,    21,
       4,    12,   143,     4,     6,     7,     8,     9,    10,   150,
     151,   199,     4,   243,   130,   199,     1,    62,   134,     4,
      12,    11,    12,    13,   192,   192,   331,     4,    64,    74,
       4,     5,   216,     1,    62,    63,     4,     1,    65,    66,
       4,   182,   183,    11,    62,    62,    73,    63,    63,   190,
      14,   193,    16,    17,    18,    32,    33,    61,     1,   200,
      61,     4,    66,    67,    68,    69,    56,   208,   256,    71,
      72,    14,   256,    16,    17,    18,    61,    61,   219,    63,
     221,    66,    67,    68,    69,     4,    63,    64,   229,   230,
     231,   232,   234,     4,    61,   237,    63,    61,   239,   240,
       4,   291,    66,    67,    68,    69,    12,     6,     7,     8,
       9,    10,     4,    32,    33,    61,   257,    63,    61,    11,
     261,    32,    33,    66,    67,    68,    69,     1,     1,   270,
       4,     4,    61,     1,    63,   276,     4,    20,    11,    12,
     281,     4,    61,    11,    31,    64,   288,     4,   290,    12,
     292,    65,    66,    64,    11,    12,    12,    62,   299,    73,
      16,    66,    20,   304,    16,    57,   307,   339,    73,    62,
      63,   312,    71,    72,   315,    62,    63,    62,    25,    66,
      27,    28,    29,    30,    57,   326,    73,    61,   329,    57,
       1,   332,    61,     4,    63,   336,   337,     0,     1,   340,
      11,    12,    58,    59,    15,    26,    61,    62,    63,    12,
     351,    14,     1,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    61,    62,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,     1,    60,
      14,     4,    16,    17,    18,    12,    67,     1,    65,    70,
       4,    14,    12,    16,    17,    18,    61,    62,    63,    65,
      14,    74,    16,    17,    18,    61,    62,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,     4,    54,    55,    56,    65,
      65,    34,    11,    12,     1,    12,    15,    14,    61,    16,
      17,    18,    19,    66,    67,    68,    69,    61,    61,     1,
       1,     4,    66,    67,    68,    69,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      63,    60,    11,    12,     4,    65,    15,     1,    67,   286,
     310,    70,    12,   279,    14,   289,    16,    17,    18,    19,
      20,    21,   344,   210,   202,   323,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
     262,    60,     4,    63,   184,    96,    -1,   192,    67,   192,
      12,    70,    14,    -1,    16,    17,    18,    19,    20,    12,
      -1,    14,    -1,    16,    17,    18,    19,    20
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    25,    27,    28,    29,    30,    76,    78,    79,    80,
      81,    84,    85,     1,    82,    61,    61,   126,    61,     4,
       0,    82,    82,    82,    82,    83,    61,    63,   139,     4,
      11,    82,    82,    82,    82,     4,     5,    86,     1,    82,
      12,    14,    16,    17,    18,    19,    20,    21,    77,    87,
     111,   112,   113,   114,   115,   117,   118,   119,   120,   122,
     125,     1,    12,   127,   128,   129,    11,    12,    15,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    60,    67,    70,   110,   131,   133,   134,
     135,   136,    32,    33,    64,    97,    98,   100,   101,    87,
      88,    62,   139,   140,    63,    20,    12,    16,    58,    59,
      12,    82,    20,     1,    82,    82,     1,    66,   126,     1,
       4,    67,    68,    69,   109,    12,    19,   117,   118,   118,
       1,   119,    62,    63,   139,    82,     1,    62,   128,    62,
      63,    16,    82,    82,    82,    82,    82,    82,    82,    82,
      82,    82,    82,    82,    82,    82,    82,    82,    82,    82,
      82,    82,    82,    82,    82,    82,    82,    82,    82,    82,
     134,    62,    66,    73,   132,    82,    82,    82,    62,    83,
      26,    89,    91,     1,    12,    11,    12,    13,    56,    82,
      12,    19,   114,   121,    63,   139,    11,    57,    94,    62,
      82,   115,    82,    82,    82,   115,   118,    82,    62,    63,
      62,    63,    82,    82,     1,   131,    82,    82,   133,    12,
      95,    34,    12,   106,     1,    82,    83,     1,    22,    23,
      24,    74,    87,    92,   104,   107,   108,   111,   137,   138,
      62,   139,    65,    65,    65,    65,   114,   116,   117,   119,
      82,    12,   121,     6,     7,     8,     9,    10,    71,    72,
     123,    82,   113,    82,    82,   131,    65,    82,    82,    82,
      82,    34,    99,    63,   139,    12,    90,   139,     1,    82,
       1,    82,     1,     1,    83,   117,   119,    82,    71,   123,
      82,   101,   102,   103,    31,   130,    82,    16,    96,    97,
      82,     4,    94,    63,   139,    61,   103,    63,   139,   126,
      63,   139,    65,    82,    11,    12,   124,    63,     1,    66,
      82,    82,    65,    98,    82,    82,    61,   124,    82,    82,
     131,    82,    63,   105,    82,    82,    71,   101,    82,    62,
      87,    93,   111,   137,   138,   105,    71,     1,    82,    62
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
      case 98: /* "media_query_exp_list" */

	{ delete (yyvaluep->mediaQueryExpList); (yyvaluep->mediaQueryExpList) = 0; };

	break;
      case 99: /* "maybe_and_media_query_exp_list" */

	{ delete (yyvaluep->mediaQueryExpList); (yyvaluep->mediaQueryExpList) = 0; };

	break;
      case 102: /* "maybe_media_list" */

	{ delete (yyvaluep->mediaList); (yyvaluep->mediaList) = 0; };

	break;
      case 103: /* "media_list" */

	{ delete (yyvaluep->mediaList); (yyvaluep->mediaList) = 0; };

	break;
      case 105: /* "ruleset_list" */

	{ delete (yyvaluep->ruleList); (yyvaluep->ruleList) = 0; };

	break;
      case 112: /* "selector_list" */

	{ if ((yyvaluep->selectorList)) qDeleteAll(*(yyvaluep->selectorList)); delete (yyvaluep->selectorList); (yyvaluep->selectorList) = 0; };

	break;
      case 113: /* "selector" */

	{ delete (yyvaluep->selector); (yyvaluep->selector) = 0; };

	break;
      case 115: /* "simple_selector" */

	{ delete (yyvaluep->selector); (yyvaluep->selector) = 0; };

	break;
      case 116: /* "simple_css3_selector" */

	{ delete (yyvaluep->selector); (yyvaluep->selector) = 0; };

	break;
      case 118: /* "specifier_list" */

	{ delete (yyvaluep->selector); (yyvaluep->selector) = 0; };

	break;
      case 119: /* "specifier" */

	{ delete (yyvaluep->selector); (yyvaluep->selector) = 0; };

	break;
      case 120: /* "class" */

	{ delete (yyvaluep->selector); (yyvaluep->selector) = 0; };

	break;
      case 122: /* "attrib" */

	{ delete (yyvaluep->selector); (yyvaluep->selector) = 0; };

	break;
      case 125: /* "pseudo" */

	{ delete (yyvaluep->selector); (yyvaluep->selector) = 0; };

	break;
      case 131: /* "expr" */

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

  case 20:

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

  case 21:

    {
 ;}
    break;

  case 22:

    {
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
      CSSParser *p = static_cast<CSSParser *>(parser);
      CSSFontFaceRuleImpl *rule = new CSSFontFaceRuleImpl( p->styleElement );
      CSSStyleDeclarationImpl *decl = p->createFontFaceStyleDeclaration( rule );
      rule->setDeclaration(decl);
      (yyval.rule) = rule;
    ;}
    break;

  case 76:

    {
      (yyval.rule) = 0;
    ;}
    break;

  case 77:

    {
      (yyval.rule) = 0;
    ;}
    break;

  case 78:

    { (yyval.relation) = CSSSelector::DirectAdjacent; ;}
    break;

  case 79:

    { (yyval.relation) = CSSSelector::IndirectAdjacent; ;}
    break;

  case 80:

    { (yyval.relation) = CSSSelector::Child; ;}
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
        (yyval.selector) = (yyvsp[(1) - (2)].selector);
    ;}
    break;

  case 89:

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

  case 90:

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

  case 91:

    {
	delete (yyvsp[(1) - (2)].selector);
	(yyval.selector) = 0;
    ;}
    break;

  case 92:

    { (yyval.string).string = 0; (yyval.string).length = 0; ;}
    break;

  case 93:

    { static unsigned short star = '*'; (yyval.string).string = &star; (yyval.string).length = 1; ;}
    break;

  case 94:

    { (yyval.string) = (yyvsp[(1) - (2)].string); ;}
    break;

  case 95:

    {
	(yyval.selector) = new CSSSelector();
        (yyval.selector)->tagLocalName = LocalName::fromId(localNamePart((yyvsp[(1) - (1)].element)));
        (yyval.selector)->tagNamespace = NamespaceName::fromId(namespacePart((yyvsp[(1) - (1)].element)));
    ;}
    break;

  case 96:

    {
	(yyval.selector) = (yyvsp[(2) - (2)].selector);
        if ( (yyval.selector) ) {
            (yyval.selector)->tagLocalName = LocalName::fromId(localNamePart((yyvsp[(1) - (2)].element)));
            (yyval.selector)->tagNamespace = NamespaceName::fromId(namespacePart((yyvsp[(1) - (2)].element)));
        }
    ;}
    break;

  case 97:

    {
	(yyval.selector) = (yyvsp[(1) - (1)].selector);
        if ( (yyval.selector) ) {
            (yyval.selector)->tagLocalName = LocalName::fromId(anyLocalName);
            (yyval.selector)->tagNamespace = NamespaceName::fromId(static_cast<CSSParser*>(parser)->defaultNamespace());
        }
    ;}
    break;

  case 98:

    {
        (yyval.selector) = new CSSSelector();
        (yyval.selector)->tagLocalName = LocalName::fromId(localNamePart((yyvsp[(2) - (2)].element)));
        (yyval.selector)->tagNamespace = NamespaceName::fromId(namespacePart((yyvsp[(2) - (2)].element)));
	CSSParser *p = static_cast<CSSParser *>(parser);
        if (p->styleElement && p->styleElement->isCSSStyleSheet())
            static_cast<CSSStyleSheetImpl*>(p->styleElement)->determineNamespace((yyval.selector)->tagNamespace, domString((yyvsp[(1) - (2)].string)));
    ;}
    break;

  case 99:

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

  case 100:

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

  case 101:

    {
	(yyval.selector) = new CSSSelector();
        (yyval.selector)->tagLocalName = LocalName::fromId(localNamePart((yyvsp[(1) - (1)].element)));
        (yyval.selector)->tagNamespace = NamespaceName::fromId(namespacePart((yyvsp[(1) - (1)].element)));
    ;}
    break;

  case 102:

    {
	(yyval.selector) = (yyvsp[(1) - (1)].selector);
        if ( (yyval.selector) ) {
            (yyval.selector)->tagLocalName = LocalName::fromId(anyLocalName);
            (yyval.selector)->tagNamespace = NamespaceName::fromId(static_cast<CSSParser*>(parser)->defaultNamespace());
        }
    ;}
    break;

  case 103:

    {
        (yyval.selector) = new CSSSelector();
        (yyval.selector)->tagLocalName = LocalName::fromId(localNamePart((yyvsp[(2) - (2)].element)));
        (yyval.selector)->tagNamespace = NamespaceName::fromId(namespacePart((yyvsp[(2) - (2)].element)));
	CSSParser *p = static_cast<CSSParser *>(parser);
        if (p->styleElement && p->styleElement->isCSSStyleSheet())
            static_cast<CSSStyleSheetImpl*>(p->styleElement)->determineNamespace((yyval.selector)->tagNamespace, domString((yyvsp[(1) - (2)].string)));
    ;}
    break;

  case 104:

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

  case 105:

    {
      CSSParser *p = static_cast<CSSParser *>(parser);
      (yyval.element) = makeId(p->defaultNamespace(), p->getLocalNameId(domString((yyvsp[(1) - (1)].string))));
    ;}
    break;

  case 106:

    {
	(yyval.element) = makeId(static_cast<CSSParser*>(parser)->defaultNamespace(), anyLocalName);
    ;}
    break;

  case 107:

    {
	(yyval.selector) = (yyvsp[(1) - (1)].selector);
    ;}
    break;

  case 108:

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

  case 109:

    {
	delete (yyvsp[(1) - (2)].selector);
	(yyval.selector) = 0;
    ;}
    break;

  case 110:

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

  case 114:

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

  case 115:

    {
      CSSParser *p = static_cast<CSSParser *>(parser);
      (yyval.attribute) = makeId(emptyNamespace, p->getLocalNameId(domString((yyvsp[(1) - (2)].string))));
    ;}
    break;

  case 116:

    {
	(yyval.selector) = new CSSSelector();
        (yyval.selector)->attrLocalName = LocalName::fromId(localNamePart((yyvsp[(3) - (4)].attribute)));
        (yyval.selector)->attrNamespace = NamespaceName::fromId(namespacePart((yyvsp[(3) - (4)].attribute)));
	(yyval.selector)->match = CSSSelector::Set;
    ;}
    break;

  case 117:

    {
	(yyval.selector) = new CSSSelector();
        (yyval.selector)->attrLocalName = LocalName::fromId(localNamePart((yyvsp[(3) - (8)].attribute)));
        (yyval.selector)->attrNamespace = NamespaceName::fromId(namespacePart((yyvsp[(3) - (8)].attribute)));
	(yyval.selector)->match = (yyvsp[(4) - (8)].match);
	(yyval.selector)->value = domString((yyvsp[(6) - (8)].string));
    ;}
    break;

  case 118:

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

  case 119:

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

  case 120:

    {
	(yyval.match) = CSSSelector::Exact;
    ;}
    break;

  case 121:

    {
	(yyval.match) = CSSSelector::List;
    ;}
    break;

  case 122:

    {
	(yyval.match) = CSSSelector::Hyphen;
    ;}
    break;

  case 123:

    {
	(yyval.match) = CSSSelector::Begin;
    ;}
    break;

  case 124:

    {
	(yyval.match) = CSSSelector::End;
    ;}
    break;

  case 125:

    {
	(yyval.match) = CSSSelector::Contain;
    ;}
    break;

  case 128:

    {
	(yyval.selector) = new CSSSelector();
	(yyval.selector)->match = CSSSelector::PseudoClass;
	(yyval.selector)->value = domString((yyvsp[(2) - (2)].string));
    ;}
    break;

  case 129:

    {
	(yyval.selector) = new CSSSelector();
	(yyval.selector)->match = CSSSelector::PseudoElement;
        (yyval.selector)->value = domString((yyvsp[(3) - (3)].string));
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
        (yyval.selector)->string_arg = QString::number((yyvsp[(3) - (4)].val));
        (yyval.selector)->value = domString((yyvsp[(2) - (4)].string));
    ;}
    break;

  case 132:

    {
        (yyval.selector) = new CSSSelector();
        (yyval.selector)->match = CSSSelector::PseudoClass;
        (yyval.selector)->string_arg = domString((yyvsp[(3) - (4)].string));
        (yyval.selector)->value = domString((yyvsp[(2) - (4)].string));
    ;}
    break;

  case 133:

    {
        (yyval.selector) = new CSSSelector();
        (yyval.selector)->match = CSSSelector::PseudoClass;
        (yyval.selector)->string_arg = domString((yyvsp[(3) - (4)].string));
        (yyval.selector)->value = domString((yyvsp[(2) - (4)].string));
    ;}
    break;

  case 134:

    {
        (yyval.selector) = new CSSSelector();
        (yyval.selector)->match = CSSSelector::PseudoClass;
        (yyval.selector)->simpleSelector = (yyvsp[(4) - (6)].selector);
        (yyval.selector)->value = domString((yyvsp[(2) - (6)].string));
    ;}
    break;

  case 135:

    {
	(yyval.ok) = (yyvsp[(3) - (4)].ok);
    ;}
    break;

  case 136:

    {
	(yyval.ok) = false;
    ;}
    break;

  case 137:

    {
	(yyval.ok) = (yyvsp[(3) - (4)].ok);
    ;}
    break;

  case 138:

    {
	(yyval.ok) = (yyvsp[(3) - (5)].ok);
	if ( (yyvsp[(4) - (5)].ok) )
	    (yyval.ok) = (yyvsp[(4) - (5)].ok);
    ;}
    break;

  case 139:

    {
	(yyval.ok) = (yyvsp[(3) - (5)].ok);
    ;}
    break;

  case 140:

    {
	(yyval.ok) = (yyvsp[(1) - (3)].ok);
    ;}
    break;

  case 141:

    {
        (yyval.ok) = false;
    ;}
    break;

  case 142:

    {
	(yyval.ok) = (yyvsp[(1) - (4)].ok);
	if ( (yyvsp[(2) - (4)].ok) )
	    (yyval.ok) = (yyvsp[(2) - (4)].ok);
    ;}
    break;

  case 143:

    {
        (yyval.ok) = (yyvsp[(1) - (4)].ok);
    ;}
    break;

  case 144:

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

  case 145:

    {
        (yyval.ok) = false;
    ;}
    break;

  case 146:

    {
	QString str = qString((yyvsp[(1) - (2)].string));
	(yyval.prop_id) = getPropertyID( str.toLower().toLatin1(), str.length() );
    ;}
    break;

  case 147:

    { (yyval.b) = true; ;}
    break;

  case 148:

    { (yyval.b) = false; ;}
    break;

  case 149:

    {
	(yyval.valueList) = new ValueList;
	(yyval.valueList)->addValue( (yyvsp[(1) - (1)].value) );
    ;}
    break;

  case 150:

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

  case 151:

    {
	(yyval.tok) = '/';
    ;}
    break;

  case 152:

    {
	(yyval.tok) = ',';
    ;}
    break;

  case 153:

    {
        (yyval.tok) = 0;
  ;}
    break;

  case 154:

    { (yyval.value) = (yyvsp[(1) - (1)].value); ;}
    break;

  case 155:

    { (yyval.value) = (yyvsp[(2) - (2)].value); (yyval.value).fValue *= (yyvsp[(1) - (2)].val); ;}
    break;

  case 156:

    { (yyval.value).id = 0; (yyval.value).string = (yyvsp[(1) - (2)].string); (yyval.value).unit = CSSPrimitiveValue::CSS_DIMENSION; ;}
    break;

  case 157:

    { (yyval.value).id = 0; (yyval.value).string = (yyvsp[(1) - (2)].string); (yyval.value).unit = CSSPrimitiveValue::CSS_STRING; ;}
    break;

  case 158:

    {
      QString str = qString( (yyvsp[(1) - (2)].string) );
      (yyval.value).id = getValueID( str.toLower().toLatin1(), str.length() );
      (yyval.value).unit = CSSPrimitiveValue::CSS_IDENT;
      (yyval.value).string = (yyvsp[(1) - (2)].string);
  ;}
    break;

  case 159:

    { (yyval.value).id = 0; (yyval.value).string = (yyvsp[(1) - (2)].string); (yyval.value).unit = CSSPrimitiveValue::CSS_URI; ;}
    break;

  case 160:

    { (yyval.value).id = 0; (yyval.value).iValue = 0; (yyval.value).unit = CSSPrimitiveValue::CSS_UNKNOWN;/* ### */ ;}
    break;

  case 161:

    { (yyval.value).id = 0; (yyval.value).string = (yyvsp[(1) - (1)].string); (yyval.value).unit = CSSPrimitiveValue::CSS_RGBCOLOR; ;}
    break;

  case 162:

    {
      (yyval.value) = (yyvsp[(1) - (1)].value);
  ;}
    break;

  case 163:

    { (yyval.value).id = 0; (yyval.value).isInt = true; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_NUMBER; ;}
    break;

  case 164:

    { (yyval.value).id = 0; (yyval.value).isInt = false; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_NUMBER; ;}
    break;

  case 165:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_PERCENTAGE; ;}
    break;

  case 166:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_PX; ;}
    break;

  case 167:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_CM; ;}
    break;

  case 168:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_MM; ;}
    break;

  case 169:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_IN; ;}
    break;

  case 170:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_PT; ;}
    break;

  case 171:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_PC; ;}
    break;

  case 172:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_DEG; ;}
    break;

  case 173:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_RAD; ;}
    break;

  case 174:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_GRAD; ;}
    break;

  case 175:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_MS; ;}
    break;

  case 176:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_S; ;}
    break;

  case 177:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_HZ; ;}
    break;

  case 178:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_KHZ; ;}
    break;

  case 179:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_EMS; ;}
    break;

  case 180:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = Value::Q_EMS; ;}
    break;

  case 181:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_EXS; ;}
    break;

  case 182:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_DPI; ;}
    break;

  case 183:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_DPCM; ;}
    break;

  case 184:

    {
      Function *f = new Function;
      f->name = (yyvsp[(1) - (5)].string);
      f->args = (yyvsp[(3) - (5)].valueList);
      (yyval.value).id = 0;
      (yyval.value).unit = Value::Function;
      (yyval.value).function = f;
  ;}
    break;

  case 185:

    {
      Function *f = new Function;
      f->name = (yyvsp[(1) - (3)].string);
      f->args = 0;
      (yyval.value).id = 0;
      (yyval.value).unit = Value::Function;
      (yyval.value).function = f;
  ;}
    break;

  case 186:

    { (yyval.string) = (yyvsp[(1) - (2)].string); ;}
    break;

  case 187:

    {
	(yyval.rule) = 0;
#ifdef CSS_DEBUG
	kDebug( 6080 ) << "skipped invalid @-rule";
#endif
    ;}
    break;

  case 188:

    {
	(yyval.rule) = 0;
#ifdef CSS_DEBUG
	kDebug( 6080 ) << "skipped invalid @-rule";
#endif
    ;}
    break;

  case 189:

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






