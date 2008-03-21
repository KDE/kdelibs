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
#define YYLAST   543

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  74
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  64
/* YYNRULES -- Number of rules.  */
#define YYNRULES  182
/* YYNRULES -- Number of states.  */
#define YYNSTATES  347

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
     130,   132,   134,   136,   139,   141,   143,   146,   147,   152,
     162,   164,   167,   168,   170,   171,   173,   175,   180,   181,
     183,   185,   190,   193,   201,   208,   209,   213,   216,   220,
     224,   228,   232,   235,   238,   241,   242,   244,   246,   249,
     251,   256,   259,   261,   265,   268,   270,   273,   276,   279,
     283,   286,   290,   295,   299,   301,   303,   305,   308,   311,
     313,   315,   317,   319,   322,   325,   330,   339,   345,   355,
     357,   359,   361,   363,   365,   367,   369,   371,   374,   378,
     383,   388,   393,   398,   404,   409,   414,   419,   425,   431,
     435,   439,   444,   449,   455,   458,   461,   464,   465,   467,
     471,   474,   477,   478,   480,   483,   486,   489,   492,   495,
     498,   500,   502,   505,   508,   511,   514,   517,   520,   523,
     526,   529,   532,   535,   538,   541,   544,   547,   550,   553,
     556,   559,   562,   565,   571,   575,   578,   582,   586,   589,
     595,   599,   601
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
      75,     0,    -1,    83,    82,    85,    87,    90,    -1,    77,
      81,    -1,    78,    81,    -1,    79,    81,    -1,    80,    81,
      -1,   109,    -1,    86,    -1,    26,    60,    81,    76,    81,
      61,    -1,    27,   123,    -1,    28,    60,    81,   128,    61,
      -1,    29,     4,    81,    99,    61,    -1,    -1,    81,     4,
      -1,    -1,    82,     5,    -1,    82,     4,    -1,    -1,    84,
      -1,    24,    81,    11,    81,    62,    -1,    24,     1,   136,
      -1,    24,     1,    62,    -1,    -1,    85,    86,    82,    -1,
      20,    81,    92,    81,   100,    62,    -1,    20,     1,   136,
      -1,    20,     1,    62,    -1,    -1,    87,    88,    82,    -1,
      25,    81,    89,    92,    81,    62,    -1,    25,     1,   136,
      -1,    25,     1,    62,    -1,    -1,    12,     4,    -1,    -1,
      90,    91,    82,    -1,   109,    -1,   102,    -1,   105,    -1,
     106,    -1,   135,    -1,   134,    -1,    86,     1,    -1,    11,
      -1,    56,    -1,    12,    81,    -1,    -1,    15,    81,   128,
      81,    -1,    33,    81,    63,    81,    93,    81,    94,    64,
      81,    -1,    95,    -1,    96,    95,    -1,    -1,    96,    -1,
      -1,    31,    -1,    32,    -1,    98,    81,   104,    97,    -1,
      -1,   101,    -1,    99,    -1,   101,    65,    81,    99,    -1,
     101,     1,    -1,    22,    81,   101,    60,    81,   103,    61,
      -1,    22,    81,    60,    81,   103,    61,    -1,    -1,   103,
     109,    81,    -1,    12,    81,    -1,    21,     1,   136,    -1,
      21,     1,    62,    -1,    23,     1,   136,    -1,    23,     1,
      62,    -1,    66,    81,    -1,    67,    81,    -1,    68,    81,
      -1,    -1,    69,    -1,    66,    -1,   110,   123,    -1,   111,
      -1,   110,    65,    81,   111,    -1,   110,     1,    -1,   113,
      -1,   111,   107,   113,    -1,   111,     1,    -1,    19,    -1,
      18,    19,    -1,    12,    19,    -1,   114,    81,    -1,   114,
     115,    81,    -1,   115,    81,    -1,   112,   114,    81,    -1,
     112,   114,   115,    81,    -1,   112,   115,    81,    -1,    12,
      -1,    18,    -1,   116,    -1,   115,   116,    -1,   115,     1,
      -1,    14,    -1,   117,    -1,   119,    -1,   122,    -1,    16,
      12,    -1,    12,    81,    -1,    17,    81,   118,    70,    -1,
      17,    81,   118,   120,    81,   121,    81,    70,    -1,    17,
      81,   112,   118,    70,    -1,    17,    81,   112,   118,   120,
      81,   121,    81,    70,    -1,    71,    -1,     6,    -1,     7,
      -1,     8,    -1,     9,    -1,    10,    -1,    12,    -1,    11,
      -1,    15,    12,    -1,    15,    15,    12,    -1,    15,    57,
      13,    64,    -1,    15,    57,    55,    64,    -1,    15,    57,
      12,    64,    -1,    15,    57,    11,    64,    -1,    15,    58,
      81,   113,    64,    -1,    60,    81,   125,    61,    -1,    60,
      81,     1,    61,    -1,    60,    81,   124,    61,    -1,    60,
      81,   124,   125,    61,    -1,    60,    81,   124,     1,    61,
      -1,   125,    62,    81,    -1,     1,    62,    81,    -1,   124,
     125,    62,    81,    -1,   124,     1,    62,    81,    -1,   126,
      15,    81,   128,   127,    -1,     1,   136,    -1,    12,    81,
      -1,    30,    81,    -1,    -1,   130,    -1,   128,   129,   130,
      -1,    72,    81,    -1,    65,    81,    -1,    -1,   131,    -1,
     108,   131,    -1,    52,    81,    -1,    11,    81,    -1,    12,
      81,    -1,    56,    81,    -1,    59,    81,    -1,   133,    -1,
     132,    -1,    55,    81,    -1,    54,    81,    -1,    53,    81,
      -1,    37,    81,    -1,    38,    81,    -1,    39,    81,    -1,
      40,    81,    -1,    41,    81,    -1,    42,    81,    -1,    43,
      81,    -1,    44,    81,    -1,    45,    81,    -1,    46,    81,
      -1,    47,    81,    -1,    48,    81,    -1,    49,    81,    -1,
      35,    81,    -1,    34,    81,    -1,    36,    81,    -1,    50,
      81,    -1,    51,    81,    -1,    57,    81,   128,    64,    81,
      -1,    57,    81,     1,    -1,    14,    81,    -1,    73,     1,
     136,    -1,    73,     1,    62,    -1,     1,   136,    -1,    60,
       1,   137,     1,    61,    -1,    60,     1,    61,    -1,   136,
      -1,   137,     1,   136,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   289,   289,   290,   291,   292,   293,   297,   298,   302,
     309,   315,   340,   347,   348,   351,   353,   354,   357,   359,
     362,   373,   376,   381,   383,   394,   404,   407,   413,   414,
     418,   426,   427,   431,   432,   435,   437,   448,   449,   450,
     451,   452,   453,   454,   458,   459,   463,   469,   472,   478,
     484,   488,   495,   498,   502,   505,   508,   514,   520,   523,
     528,   532,   537,   544,   555,   567,   568,   578,   600,   603,
     609,   612,   618,   619,   620,   621,   625,   626,   630,   652,
     665,   683,   692,   695,   709,   716,   717,   718,   722,   726,
     731,   736,   743,   752,   764,   780,   786,   789,   799,   806,
     812,   813,   814,   818,   827,   851,   856,   862,   870,   882,
     885,   888,   891,   894,   897,   903,   904,   908,   914,   920,
     927,   934,   941,   948,   957,   960,   963,   966,   971,   977,
     981,   984,   989,   995,  1017,  1023,  1030,  1031,  1035,  1039,
    1055,  1058,  1061,  1067,  1068,  1070,  1071,  1072,  1078,  1079,
    1080,  1082,  1088,  1089,  1090,  1091,  1092,  1093,  1094,  1095,
    1096,  1097,  1098,  1099,  1100,  1101,  1102,  1103,  1104,  1105,
    1106,  1107,  1108,  1113,  1121,  1137,  1144,  1150,  1159,  1185,
    1186,  1190,  1191
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
  "namespace", "maybe_ns_prefix", "rule_list", "rule", "string_or_uri",
  "media_feature", "maybe_media_value", "media_query_exp",
  "media_query_exp_list", "maybe_media_query_exp_list",
  "maybe_media_restrictor", "media_query", "maybe_media_list",
  "media_list", "media", "ruleset_list", "medium", "page", "font_face",
  "combinator", "unary_operator", "ruleset", "selector_list", "selector",
  "namespace_selector", "simple_selector", "element_name",
  "specifier_list", "specifier", "class", "attrib_id", "attrib", "match",
  "ident_or_string", "pseudo", "declaration_block", "declaration_list",
  "declaration", "property", "prio", "expr", "operator", "term",
  "unary_term", "function", "hexcolor", "invalid_at", "invalid_rule",
  "invalid_block", "invalid_block_list", 0
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
      91,    91,    91,    91,    92,    92,    93,    94,    94,    95,
      96,    96,    97,    97,    98,    98,    98,    99,   100,   100,
     101,   101,   101,   102,   102,   103,   103,   104,   105,   105,
     106,   106,   107,   107,   107,   107,   108,   108,   109,   110,
     110,   110,   111,   111,   111,   112,   112,   112,   113,   113,
     113,   113,   113,   113,   114,   114,   115,   115,   115,   116,
     116,   116,   116,   117,   118,   119,   119,   119,   119,   120,
     120,   120,   120,   120,   120,   121,   121,   122,   122,   122,
     122,   122,   122,   122,   123,   123,   123,   123,   123,   124,
     124,   124,   124,   125,   125,   126,   127,   127,   128,   128,
     129,   129,   129,   130,   130,   130,   130,   130,   130,   130,
     130,   130,   131,   131,   131,   131,   131,   131,   131,   131,
     131,   131,   131,   131,   131,   131,   131,   131,   131,   131,
     131,   131,   131,   132,   132,   133,   134,   134,   135,   136,
     136,   137,   137
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     5,     2,     2,     2,     2,     1,     1,     6,
       2,     5,     5,     0,     2,     0,     2,     2,     0,     1,
       5,     3,     3,     0,     3,     6,     3,     3,     0,     3,
       6,     3,     3,     0,     2,     0,     3,     1,     1,     1,
       1,     1,     1,     2,     1,     1,     2,     0,     4,     9,
       1,     2,     0,     1,     0,     1,     1,     4,     0,     1,
       1,     4,     2,     7,     6,     0,     3,     2,     3,     3,
       3,     3,     2,     2,     2,     0,     1,     1,     2,     1,
       4,     2,     1,     3,     2,     1,     2,     2,     2,     3,
       2,     3,     4,     3,     1,     1,     1,     2,     2,     1,
       1,     1,     1,     2,     2,     4,     8,     5,     9,     1,
       1,     1,     1,     1,     1,     1,     1,     2,     3,     4,
       4,     4,     4,     5,     4,     4,     4,     5,     5,     3,
       3,     4,     4,     5,     2,     2,     2,     0,     1,     3,
       2,     2,     0,     1,     2,     2,     2,     2,     2,     2,
       1,     1,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     5,     3,     2,     3,     3,     2,     5,
       3,     1,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
      18,     0,     0,     0,     0,     0,     0,    13,    13,    13,
      13,    15,    19,     0,     0,    13,    13,    10,    13,    13,
       1,     3,     4,     5,     6,    23,     0,    22,    21,    14,
      13,     0,     0,     0,    54,    17,    16,    28,     0,     0,
      94,    99,     0,     0,    13,    95,    85,     0,    13,     8,
       7,     0,     0,     0,    82,    13,     0,    96,   100,   101,
     102,     0,    13,     0,     0,     0,    13,    13,    13,    13,
      13,    13,    13,    13,    13,    13,    13,    13,    13,    13,
      13,    13,    13,    13,    13,    13,    13,    13,    13,    13,
      13,    13,    13,    13,    77,    76,     0,   142,   138,   143,
     151,   150,    55,    56,    13,     0,    15,    35,   180,   181,
       0,    20,    87,   117,     0,     0,    13,   103,     0,    86,
       0,     0,     0,    81,    13,    78,    84,    13,    13,    13,
       0,    94,    95,    13,     0,    88,     0,    98,    90,    97,
     125,    13,   134,   135,     0,   126,     0,   124,    13,    13,
     146,   147,   175,   169,   168,   170,   155,   156,   157,   158,
     159,   160,   161,   162,   163,   164,   165,   166,   167,   171,
     172,   145,   154,   153,   152,   148,     0,   149,   144,    11,
      13,    13,     0,     0,    12,    24,     0,    15,     0,     0,
     118,     0,     0,     0,     0,     0,    13,     0,     0,     0,
      27,    26,    44,    45,    13,     9,     0,    72,    73,    74,
      83,    91,     0,    93,    89,   130,   128,    13,   127,    13,
     129,     0,   174,   142,   141,   140,   139,    13,    52,     0,
      33,    29,     0,     0,    13,     0,     0,     0,    15,    38,
      39,    40,    37,    42,    41,   179,   182,   122,   121,   119,
     120,     0,   104,    13,     0,   110,   111,   112,   113,   114,
     105,   109,    13,    54,     0,    92,   132,   131,   142,    13,
      67,    13,    50,    53,    57,    32,    31,     0,     0,   178,
       0,    54,     0,     0,    43,    36,   123,   107,    13,     0,
      60,     0,     0,    13,   133,   173,     0,    51,    34,    13,
      69,    68,    13,     0,    71,    70,   177,   176,     0,   116,
     115,    13,    25,    62,    13,   136,    13,     0,    65,    13,
      13,     0,    54,     0,    30,     0,    65,     0,   106,    61,
      13,    13,    64,    13,     0,   108,    46,    47,    66,    63,
      13,     0,     0,    13,   142,    49,    48
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     6,    48,     7,     8,     9,    10,   252,    25,    11,
      12,    37,    49,   107,   187,   278,   188,   238,   204,   331,
     341,   272,   273,   274,   104,   290,   291,   292,   239,   325,
     228,   240,   241,   130,    96,   333,    51,    52,    53,    54,
      55,    56,    57,    58,   199,    59,   262,   311,    60,    17,
      63,    64,    65,   294,    97,   182,    98,    99,   100,   101,
     243,   244,   142,   110
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -174
static const yytype_int16 yypact[] =
{
     216,   135,   -49,    51,    90,   160,   171,  -174,  -174,  -174,
    -174,  -174,  -174,   -18,    34,  -174,  -174,  -174,  -174,  -174,
    -174,   180,   180,   180,   180,    23,   177,  -174,  -174,  -174,
    -174,   464,    19,   404,     9,  -174,  -174,   167,   141,    60,
     179,  -174,   142,   192,  -174,   191,  -174,   235,  -174,  -174,
    -174,   105,   237,   525,  -174,   243,   213,  -174,  -174,  -174,
    -174,   222,  -174,   106,   161,   226,  -174,  -174,  -174,  -174,
    -174,  -174,  -174,  -174,  -174,  -174,  -174,  -174,  -174,  -174,
    -174,  -174,  -174,  -174,  -174,  -174,  -174,  -174,  -174,  -174,
    -174,  -174,  -174,  -174,  -174,  -174,   356,   -39,  -174,  -174,
    -174,  -174,  -174,  -174,  -174,   187,  -174,   242,  -174,  -174,
     270,  -174,  -174,  -174,   274,   195,  -174,  -174,   281,  -174,
      83,   127,    58,  -174,  -174,  -174,  -174,  -174,  -174,  -174,
     517,  -174,  -174,   243,   213,   180,   213,  -174,   180,  -174,
    -174,  -174,  -174,   180,   234,  -174,   201,  -174,  -174,  -174,
     180,   180,   180,   180,   180,   180,   180,   180,   180,   180,
     180,   180,   180,   180,   180,   180,   180,   180,   180,   180,
     180,   180,   180,   180,   180,   180,   310,   180,  -174,  -174,
    -174,  -174,   453,    20,  -174,    23,   208,  -174,   174,   246,
    -174,   224,   244,   245,   248,   509,   179,   191,   278,    43,
    -174,  -174,  -174,  -174,  -174,  -174,   509,   180,   180,   180,
    -174,   180,   213,   180,   180,   180,  -174,  -174,  -174,  -174,
     180,   404,  -174,    65,   180,   180,  -174,  -174,   283,    96,
     104,    23,   257,   322,  -174,   324,   325,   326,  -174,  -174,
    -174,  -174,  -174,  -174,  -174,  -174,  -174,  -174,  -174,  -174,
    -174,   256,   180,  -174,    50,  -174,  -174,  -174,  -174,  -174,
    -174,  -174,  -174,   120,   369,   180,   180,   180,   204,  -174,
     180,  -174,  -174,   283,  -174,  -174,  -174,   336,    99,  -174,
     100,   145,   153,   175,  -174,    23,  -174,  -174,  -174,   130,
    -174,   266,   101,  -174,  -174,   180,     0,  -174,  -174,  -174,
    -174,  -174,  -174,   108,  -174,  -174,  -174,  -174,   130,  -174,
    -174,  -174,  -174,  -174,  -174,   180,  -174,    91,   180,  -174,
    -174,    30,     9,   113,  -174,   319,   180,    31,  -174,  -174,
    -174,  -174,  -174,  -174,   405,  -174,   180,    21,   180,  -174,
    -174,   277,   404,  -174,    33,   180,   180
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -174,  -174,  -174,  -174,  -174,  -174,  -174,    -1,   -94,  -174,
    -174,  -174,   -27,  -174,  -174,  -174,  -174,  -174,    93,  -174,
    -174,    95,  -174,  -174,  -174,   -33,  -174,    92,  -174,    46,
    -174,  -174,  -174,  -174,  -174,   -29,  -174,   168,   259,   -91,
     329,   -34,   -40,  -174,   214,  -174,   121,    70,  -174,   338,
    -174,   350,  -174,  -174,  -173,  -174,   232,   330,  -174,  -174,
    -174,  -174,    -8,  -174
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -138
static const yytype_int16 yytable[] =
{
      14,   105,    50,   223,    29,    28,    21,    22,    23,    24,
     106,    15,   185,    29,    31,    32,   139,    33,    34,   134,
      61,   136,   179,    29,    29,    29,   180,    35,    36,    39,
     109,    62,   227,   181,    29,    29,   340,   -13,    29,   210,
     102,   103,    26,   118,    27,    30,   121,   122,   268,   255,
     256,   257,   258,   259,   135,   138,   255,   256,   257,   258,
     259,   143,    29,   316,    29,   150,   151,   152,   153,   154,
     155,   156,   157,   158,   159,   160,   161,   162,   163,   164,
     165,   166,   167,   168,   169,   170,   171,   172,   173,   174,
     175,   176,   177,   231,   139,    29,   139,   -13,   180,   212,
     328,   335,   313,   183,   251,   181,   123,   144,    29,   313,
     202,    16,   201,   260,   261,   195,   277,    29,    62,   205,
     287,   261,   111,   206,    29,   330,   207,   208,   209,   269,
     180,    29,   211,   213,    29,   214,    13,   181,   202,   -13,
     215,   309,   310,    26,   285,   200,   -13,   220,   221,    29,
      18,   102,   103,   324,   113,   203,    26,   114,   275,   242,
      26,   237,   300,   -59,    19,    16,   314,   145,   319,   344,
     124,    20,   139,   314,    -2,   232,   102,   103,    38,   224,
     225,   246,   -58,   203,    29,   230,    40,    47,    41,    42,
      43,    44,    45,    46,    47,   233,   234,   235,   112,   115,
     116,    26,   108,   263,   117,   302,   191,   192,   193,   229,
     119,   265,   -13,    26,   137,   304,   266,   -13,   267,   -13,
     -13,   276,   147,   148,   279,   -13,   270,    41,    42,    43,
      44,   -13,   -13,   281,   293,    26,   120,   306,   126,   -13,
       1,   149,     2,     3,     4,     5,   -13,   236,   184,   -75,
     194,   -75,   -75,   -75,   -75,   -75,   -75,    41,    42,    43,
      44,   289,   218,   219,   -13,  -137,  -137,   186,   295,   180,
     296,   189,   301,   -13,   305,   307,   181,   -13,   -13,   -13,
     -13,   -13,    26,   140,   141,    29,   190,   308,   247,   329,
     253,   -13,   315,   196,    26,   216,   217,   -79,   317,   197,
      46,   318,   -79,   127,   128,   129,    26,   245,   248,   249,
     321,   222,   250,   322,    29,   323,   271,    26,   326,   327,
     286,    66,    67,   280,    68,   282,   283,   284,   312,   336,
     337,    40,   338,    41,    42,    43,    44,    45,    46,   342,
     298,   343,   345,   346,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,   297,    93,
     126,   299,   334,   303,   264,   288,    94,   198,   320,    95,
     332,   -75,   133,   -75,   -75,   -75,   -75,   -75,   -75,   125,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    29,    88,
      89,    90,   254,   146,   226,    66,    67,    40,    68,    41,
      42,    43,    44,    45,    46,     0,   178,     0,     0,   -80,
       0,     0,     0,     0,   -80,   127,   128,   129,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,     0,    93,    66,    67,   339,    68,    29,     0,
      94,     0,     0,    95,     0,     0,    40,     0,    41,    42,
      43,    44,    45,    46,    47,     0,     0,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,     0,    93,    29,     0,     0,     0,     0,     0,    94,
       0,    40,    95,    41,    42,    43,    44,    45,    46,    40,
       0,    41,    42,    43,    44,    45,    46,   131,     0,    41,
      42,    43,    44,   132
};

static const yytype_int16 yycheck[] =
{
       1,    34,    31,   176,     4,    13,     7,     8,     9,    10,
      37,    60,   106,     4,    15,    16,    56,    18,    19,    53,
       1,    55,    61,     4,     4,     4,    65,     4,     5,    30,
      38,    12,    12,    72,     4,     4,    15,     4,     4,   130,
      31,    32,    60,    44,    62,    11,    47,    48,   221,     6,
       7,     8,     9,    10,    55,    56,     6,     7,     8,     9,
      10,    62,     4,    63,     4,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,   187,   134,     4,   136,    64,    65,   133,
      70,    70,     1,   104,   195,    72,     1,     1,     4,     1,
      11,    60,   120,    70,    71,   116,    12,     4,    12,    61,
      70,    71,    62,   124,     4,    12,   127,   128,   129,    64,
      65,     4,   133,   134,     4,   136,     1,    72,    11,     4,
     141,    11,    12,    60,   238,    62,    11,   148,   149,     4,
      60,    31,    32,    62,    12,    56,    60,    15,    62,   188,
      60,   188,    62,    62,     4,    60,    65,    61,    60,   342,
      65,     0,   212,    65,     0,     1,    31,    32,     1,   180,
     181,   189,    62,    56,     4,   186,    12,    20,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    19,    57,
      58,    60,    61,   204,    12,    60,    11,    12,    13,     1,
      19,   212,     4,    60,     1,    62,   217,     4,   219,    11,
      12,   229,    61,    62,   232,    12,   227,    14,    15,    16,
      17,    18,    19,   234,    30,    60,     1,    62,     1,     4,
      24,    15,    26,    27,    28,    29,    11,    73,    61,    12,
      55,    14,    15,    16,    17,    18,    19,    14,    15,    16,
      17,   262,    61,    62,    56,    61,    62,    25,   269,    65,
     271,     1,   280,    60,   282,   283,    72,    64,    65,    66,
      67,    68,    60,    61,    62,     4,    12,   288,    64,   322,
      12,    56,   293,    12,    60,    61,    62,    60,   299,    18,
      19,   302,    65,    66,    67,    68,    60,    61,    64,    64,
     311,     1,    64,   314,     4,   316,    33,    60,   319,   320,
      64,    11,    12,     1,    14,     1,     1,     1,    62,   330,
     331,    12,   333,    14,    15,    16,    17,    18,    19,   340,
       4,    64,   343,   344,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,   273,    59,
       1,   278,   326,   281,   206,   254,    66,   118,   308,    69,
      61,    12,    53,    14,    15,    16,    17,    18,    19,    51,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,     4,    53,
      54,    55,   198,    63,   182,    11,    12,    12,    14,    14,
      15,    16,    17,    18,    19,    -1,    96,    -1,    -1,    60,
      -1,    -1,    -1,    -1,    65,    66,    67,    68,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    -1,    59,    11,    12,    61,    14,     4,    -1,
      66,    -1,    -1,    69,    -1,    -1,    12,    -1,    14,    15,
      16,    17,    18,    19,    20,    -1,    -1,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    -1,    59,     4,    -1,    -1,    -1,    -1,    -1,    66,
      -1,    12,    69,    14,    15,    16,    17,    18,    19,    12,
      -1,    14,    15,    16,    17,    18,    19,    12,    -1,    14,
      15,    16,    17,    18
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    24,    26,    27,    28,    29,    75,    77,    78,    79,
      80,    83,    84,     1,    81,    60,    60,   123,    60,     4,
       0,    81,    81,    81,    81,    82,    60,    62,   136,     4,
      11,    81,    81,    81,    81,     4,     5,    85,     1,    81,
      12,    14,    15,    16,    17,    18,    19,    20,    76,    86,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   119,
     122,     1,    12,   124,   125,   126,    11,    12,    14,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    59,    66,    69,   108,   128,   130,   131,
     132,   133,    31,    32,    98,    99,    86,    87,    61,   136,
     137,    62,    19,    12,    15,    57,    58,    12,    81,    19,
       1,    81,    81,     1,    65,   123,     1,    66,    67,    68,
     107,    12,    18,   114,   115,    81,   115,     1,    81,   116,
      61,    62,   136,    81,     1,    61,   125,    61,    62,    15,
      81,    81,    81,    81,    81,    81,    81,    81,    81,    81,
      81,    81,    81,    81,    81,    81,    81,    81,    81,    81,
      81,    81,    81,    81,    81,    81,    81,    81,   131,    61,
      65,    72,   129,    81,    61,    82,    25,    88,    90,     1,
      12,    11,    12,    13,    55,    81,    12,    18,   112,   118,
      62,   136,    11,    56,    92,    61,    81,    81,    81,    81,
     113,    81,   115,    81,    81,    81,    61,    62,    61,    62,
      81,    81,     1,   128,    81,    81,   130,    12,   104,     1,
      81,    82,     1,    21,    22,    23,    73,    86,    91,   102,
     105,   106,   109,   134,   135,    61,   136,    64,    64,    64,
      64,   113,    81,    12,   118,     6,     7,     8,     9,    10,
      70,    71,   120,    81,   111,    81,    81,    81,   128,    64,
      81,    33,    95,    96,    97,    62,   136,    12,    89,   136,
       1,    81,     1,     1,     1,    82,    64,    70,   120,    81,
      99,   100,   101,    30,   127,    81,    81,    95,     4,    92,
      62,   136,    60,   101,    62,   136,    62,   136,    81,    11,
      12,   121,    62,     1,    65,    81,    63,    81,    81,    60,
     121,    81,    81,    81,    62,   103,    81,    81,    70,    99,
      12,    93,    61,   109,   103,    70,    81,    81,    81,    61,
      15,    94,    81,    64,   128,    81,    81
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
      case 96: /* "media_query_exp_list" */

	{ delete (yyvaluep->mediaQueryExpList); (yyvaluep->mediaQueryExpList) = 0; };

	break;
      case 97: /* "maybe_media_query_exp_list" */

	{ delete (yyvaluep->mediaQueryExpList); (yyvaluep->mediaQueryExpList) = 0; };

	break;
      case 100: /* "maybe_media_list" */

	{ delete (yyvaluep->mediaList); (yyvaluep->mediaList) = 0; };

	break;
      case 101: /* "media_list" */

	{ delete (yyvaluep->mediaList); (yyvaluep->mediaList) = 0; };

	break;
      case 103: /* "ruleset_list" */

	{ delete (yyvaluep->ruleList); (yyvaluep->ruleList) = 0; };

	break;
      case 110: /* "selector_list" */

	{ if ((yyvaluep->selectorList)) qDeleteAll(*(yyvaluep->selectorList)); delete (yyvaluep->selectorList); (yyvaluep->selectorList) = 0; };

	break;
      case 111: /* "selector" */

	{ delete (yyvaluep->selector); (yyvaluep->selector) = 0; };

	break;
      case 113: /* "simple_selector" */

	{ delete (yyvaluep->selector); (yyvaluep->selector) = 0; };

	break;
      case 115: /* "specifier_list" */

	{ delete (yyvaluep->selector); (yyvaluep->selector) = 0; };

	break;
      case 116: /* "specifier" */

	{ delete (yyvaluep->selector); (yyvaluep->selector) = 0; };

	break;
      case 117: /* "class" */

	{ delete (yyvaluep->selector); (yyvaluep->selector) = 0; };

	break;
      case 119: /* "attrib" */

	{ delete (yyvaluep->selector); (yyvaluep->selector) = 0; };

	break;
      case 122: /* "pseudo" */

	{ delete (yyvaluep->selector); (yyvaluep->selector) = 0; };

	break;
      case 128: /* "expr" */

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

  case 46:

    {
        (yyval.string) = (yyvsp[(1) - (2)].string);
    ;}
    break;

  case 47:

    {
        (yyval.valueList) = 0;
    ;}
    break;

  case 48:

    {
        (yyval.valueList) = (yyvsp[(3) - (4)].valueList);
    ;}
    break;

  case 49:

    {
        (yyval.mediaQueryExp) = new khtml::MediaQueryExp(domString((yyvsp[(5) - (9)].string)).lower(), (yyvsp[(7) - (9)].valueList));
    ;}
    break;

  case 50:

    {
      (yyval.mediaQueryExpList) =  new QList<khtml::MediaQueryExp*>;
      (yyval.mediaQueryExpList)->append((yyvsp[(1) - (1)].mediaQueryExp));
    ;}
    break;

  case 51:

    {
      (yyval.mediaQueryExpList) = (yyvsp[(1) - (2)].mediaQueryExpList);
      (yyval.mediaQueryExpList)->append((yyvsp[(2) - (2)].mediaQueryExp));
    ;}
    break;

  case 52:

    {
        (yyval.mediaQueryExpList) = new QList<khtml::MediaQueryExp*>;
    ;}
    break;

  case 54:

    {
        (yyval.mediaQueryRestrictor) = khtml::MediaQuery::None;
    ;}
    break;

  case 55:

    {
        (yyval.mediaQueryRestrictor) = khtml::MediaQuery::Only;
    ;}
    break;

  case 56:

    {
        (yyval.mediaQueryRestrictor) = khtml::MediaQuery::Not;
    ;}
    break;

  case 57:

    {
        (yyval.mediaQuery) = new khtml::MediaQuery((yyvsp[(1) - (4)].mediaQueryRestrictor), domString((yyvsp[(3) - (4)].string)).lower(), (yyvsp[(4) - (4)].mediaQueryExpList));
    ;}
    break;

  case 58:

    {
	(yyval.mediaList) = new MediaListImpl();
    ;}
    break;

  case 60:

    {
        (yyval.mediaList) = new MediaListImpl();
        (yyval.mediaList)->appendMediaQuery((yyvsp[(1) - (1)].mediaQuery));
    ;}
    break;

  case 61:

    {
	(yyval.mediaList) = (yyvsp[(1) - (4)].mediaList);
	if ((yyval.mediaList))
	    (yyval.mediaList)->appendMediaQuery( (yyvsp[(4) - (4)].mediaQuery) );
    ;}
    break;

  case 62:

    {
       delete (yyvsp[(1) - (2)].mediaList);
       (yyval.mediaList) = 0;
    ;}
    break;

  case 63:

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

  case 64:

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

  case 65:

    { (yyval.ruleList) = 0; ;}
    break;

  case 66:

    {
      (yyval.ruleList) = (yyvsp[(1) - (3)].ruleList);
      if ( (yyvsp[(2) - (3)].rule) ) {
	  if ( !(yyval.ruleList) ) (yyval.ruleList) = new CSSRuleListImpl();
	  (yyval.ruleList)->append( (yyvsp[(2) - (3)].rule) );
      }
  ;}
    break;

  case 67:

    {
      (yyval.string) = (yyvsp[(1) - (2)].string);
  ;}
    break;

  case 68:

    {
      (yyval.rule) = 0;
    ;}
    break;

  case 69:

    {
      (yyval.rule) = 0;
    ;}
    break;

  case 70:

    {
      (yyval.rule) = 0;
    ;}
    break;

  case 71:

    {
      (yyval.rule) = 0;
    ;}
    break;

  case 72:

    { (yyval.relation) = CSSSelector::DirectAdjacent; ;}
    break;

  case 73:

    { (yyval.relation) = CSSSelector::IndirectAdjacent; ;}
    break;

  case 74:

    { (yyval.relation) = CSSSelector::Child; ;}
    break;

  case 75:

    { (yyval.relation) = CSSSelector::Descendant; ;}
    break;

  case 76:

    { (yyval.val) = -1; ;}
    break;

  case 77:

    { (yyval.val) = 1; ;}
    break;

  case 78:

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

  case 79:

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

  case 80:

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

  case 81:

    {
        if ((yyvsp[(1) - (2)].selectorList)) qDeleteAll(*(yyvsp[(1) - (2)].selectorList));
	delete (yyvsp[(1) - (2)].selectorList);
	(yyvsp[(1) - (2)].selectorList) = 0;
	(yyval.selectorList) = 0;
    ;}
    break;

  case 82:

    {
	(yyval.selector) = (yyvsp[(1) - (1)].selector);
    ;}
    break;

  case 83:

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

  case 84:

    {
	delete (yyvsp[(1) - (2)].selector);
	(yyval.selector) = 0;
    ;}
    break;

  case 85:

    { (yyval.string).string = 0; (yyval.string).length = 0; ;}
    break;

  case 86:

    { static unsigned short star = '*'; (yyval.string).string = &star; (yyval.string).length = 1; ;}
    break;

  case 87:

    { (yyval.string) = (yyvsp[(1) - (2)].string); ;}
    break;

  case 88:

    {
	(yyval.selector) = new CSSSelector();
	(yyval.selector)->tag = (yyvsp[(1) - (2)].element);
    ;}
    break;

  case 89:

    {
	(yyval.selector) = (yyvsp[(2) - (3)].selector);
        if ( (yyval.selector) )
	    (yyval.selector)->tag = (yyvsp[(1) - (3)].element);
    ;}
    break;

  case 90:

    {
	(yyval.selector) = (yyvsp[(1) - (2)].selector);
        if ( (yyval.selector) )
            (yyval.selector)->tag = makeId(static_cast<CSSParser*>(parser)->defaultNamespace(), anyLocalName);
    ;}
    break;

  case 91:

    {
        (yyval.selector) = new CSSSelector();
        (yyval.selector)->tag = (yyvsp[(2) - (3)].element);
	CSSParser *p = static_cast<CSSParser *>(parser);
        if (p->styleElement && p->styleElement->isCSSStyleSheet())
            static_cast<CSSStyleSheetImpl*>(p->styleElement)->determineNamespace((yyval.selector)->tag, domString((yyvsp[(1) - (3)].string)));
    ;}
    break;

  case 92:

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

  case 93:

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

  case 94:

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

  case 95:

    {
	(yyval.element) = makeId(static_cast<CSSParser*>(parser)->defaultNamespace(), anyLocalName);
    ;}
    break;

  case 96:

    {
	(yyval.selector) = (yyvsp[(1) - (1)].selector);
    ;}
    break;

  case 97:

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

  case 98:

    {
	delete (yyvsp[(1) - (2)].selector);
	(yyval.selector) = 0;
    ;}
    break;

  case 99:

    {
	(yyval.selector) = new CSSSelector();
	(yyval.selector)->match = CSSSelector::Id;
	(yyval.selector)->attr = ATTR_ID;
	(yyval.selector)->value = domString((yyvsp[(1) - (1)].string));
    ;}
    break;

  case 103:

    {
	(yyval.selector) = new CSSSelector();
	(yyval.selector)->match = CSSSelector::Class;
	(yyval.selector)->attr = ATTR_CLASS;
	(yyval.selector)->value = domString((yyvsp[(2) - (2)].string));
    ;}
    break;

  case 104:

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

  case 105:

    {
	(yyval.selector) = new CSSSelector();
	(yyval.selector)->attr = (yyvsp[(3) - (4)].attribute);
	(yyval.selector)->match = CSSSelector::Set;
    ;}
    break;

  case 106:

    {
	(yyval.selector) = new CSSSelector();
	(yyval.selector)->attr = (yyvsp[(3) - (8)].attribute);
	(yyval.selector)->match = (yyvsp[(4) - (8)].match);
	(yyval.selector)->value = domString((yyvsp[(6) - (8)].string));
    ;}
    break;

  case 107:

    {
        (yyval.selector) = new CSSSelector();
        (yyval.selector)->attr = (yyvsp[(4) - (5)].attribute);
        (yyval.selector)->match = CSSSelector::Set;
        CSSParser *p = static_cast<CSSParser *>(parser);
        if (p->styleElement && p->styleElement->isCSSStyleSheet())
            static_cast<CSSStyleSheetImpl*>(p->styleElement)->determineNamespace((yyval.selector)->attr, domString((yyvsp[(3) - (5)].string)));
    ;}
    break;

  case 108:

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

  case 109:

    {
	(yyval.match) = CSSSelector::Exact;
    ;}
    break;

  case 110:

    {
	(yyval.match) = CSSSelector::List;
    ;}
    break;

  case 111:

    {
	(yyval.match) = CSSSelector::Hyphen;
    ;}
    break;

  case 112:

    {
	(yyval.match) = CSSSelector::Begin;
    ;}
    break;

  case 113:

    {
	(yyval.match) = CSSSelector::End;
    ;}
    break;

  case 114:

    {
	(yyval.match) = CSSSelector::Contain;
    ;}
    break;

  case 117:

    {
	(yyval.selector) = new CSSSelector();
	(yyval.selector)->match = CSSSelector::PseudoClass;
	(yyval.selector)->value = domString((yyvsp[(2) - (2)].string));
    ;}
    break;

  case 118:

    {
	(yyval.selector) = new CSSSelector();
	(yyval.selector)->match = CSSSelector::PseudoElement;
        (yyval.selector)->value = domString((yyvsp[(3) - (3)].string));
    ;}
    break;

  case 119:

    {
        (yyval.selector) = new CSSSelector();
        (yyval.selector)->match = CSSSelector::PseudoClass;
        (yyval.selector)->string_arg = domString((yyvsp[(3) - (4)].string));
        (yyval.selector)->value = domString((yyvsp[(2) - (4)].string));
    ;}
    break;

  case 120:

    {
        (yyval.selector) = new CSSSelector();
        (yyval.selector)->match = CSSSelector::PseudoClass;
        (yyval.selector)->string_arg = QString::number((yyvsp[(3) - (4)].val));
        (yyval.selector)->value = domString((yyvsp[(2) - (4)].string));
    ;}
    break;

  case 121:

    {
        (yyval.selector) = new CSSSelector();
        (yyval.selector)->match = CSSSelector::PseudoClass;
        (yyval.selector)->string_arg = domString((yyvsp[(3) - (4)].string));
        (yyval.selector)->value = domString((yyvsp[(2) - (4)].string));
    ;}
    break;

  case 122:

    {
        (yyval.selector) = new CSSSelector();
        (yyval.selector)->match = CSSSelector::PseudoClass;
        (yyval.selector)->string_arg = domString((yyvsp[(3) - (4)].string));
        (yyval.selector)->value = domString((yyvsp[(2) - (4)].string));
    ;}
    break;

  case 123:

    {
        (yyval.selector) = new CSSSelector();
        (yyval.selector)->match = CSSSelector::PseudoClass;
        (yyval.selector)->simpleSelector = (yyvsp[(4) - (5)].selector);
        (yyval.selector)->value = domString((yyvsp[(2) - (5)].string));
    ;}
    break;

  case 124:

    {
	(yyval.ok) = (yyvsp[(3) - (4)].ok);
    ;}
    break;

  case 125:

    {
	(yyval.ok) = false;
    ;}
    break;

  case 126:

    {
	(yyval.ok) = (yyvsp[(3) - (4)].ok);
    ;}
    break;

  case 127:

    {
	(yyval.ok) = (yyvsp[(3) - (5)].ok);
	if ( (yyvsp[(4) - (5)].ok) )
	    (yyval.ok) = (yyvsp[(4) - (5)].ok);
    ;}
    break;

  case 128:

    {
	(yyval.ok) = (yyvsp[(3) - (5)].ok);
    ;}
    break;

  case 129:

    {
	(yyval.ok) = (yyvsp[(1) - (3)].ok);
    ;}
    break;

  case 130:

    {
        (yyval.ok) = false;
    ;}
    break;

  case 131:

    {
	(yyval.ok) = (yyvsp[(1) - (4)].ok);
	if ( (yyvsp[(2) - (4)].ok) )
	    (yyval.ok) = (yyvsp[(2) - (4)].ok);
    ;}
    break;

  case 132:

    {
        (yyval.ok) = (yyvsp[(1) - (4)].ok);
    ;}
    break;

  case 133:

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

  case 134:

    {
        (yyval.ok) = false;
    ;}
    break;

  case 135:

    {
	QString str = qString((yyvsp[(1) - (2)].string));
	(yyval.prop_id) = getPropertyID( str.toLower().toLatin1(), str.length() );
    ;}
    break;

  case 136:

    { (yyval.b) = true; ;}
    break;

  case 137:

    { (yyval.b) = false; ;}
    break;

  case 138:

    {
	(yyval.valueList) = new ValueList;
	(yyval.valueList)->addValue( (yyvsp[(1) - (1)].value) );
    ;}
    break;

  case 139:

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

  case 140:

    {
	(yyval.tok) = '/';
    ;}
    break;

  case 141:

    {
	(yyval.tok) = ',';
    ;}
    break;

  case 142:

    {
        (yyval.tok) = 0;
  ;}
    break;

  case 143:

    { (yyval.value) = (yyvsp[(1) - (1)].value); ;}
    break;

  case 144:

    { (yyval.value) = (yyvsp[(2) - (2)].value); (yyval.value).fValue *= (yyvsp[(1) - (2)].val); ;}
    break;

  case 145:

    { (yyval.value).id = 0; (yyval.value).string = (yyvsp[(1) - (2)].string); (yyval.value).unit = CSSPrimitiveValue::CSS_DIMENSION; ;}
    break;

  case 146:

    { (yyval.value).id = 0; (yyval.value).string = (yyvsp[(1) - (2)].string); (yyval.value).unit = CSSPrimitiveValue::CSS_STRING; ;}
    break;

  case 147:

    {
      QString str = qString( (yyvsp[(1) - (2)].string) );
      (yyval.value).id = getValueID( str.toLower().toLatin1(), str.length() );
      (yyval.value).unit = CSSPrimitiveValue::CSS_IDENT;
      (yyval.value).string = (yyvsp[(1) - (2)].string);
  ;}
    break;

  case 148:

    { (yyval.value).id = 0; (yyval.value).string = (yyvsp[(1) - (2)].string); (yyval.value).unit = CSSPrimitiveValue::CSS_URI; ;}
    break;

  case 149:

    { (yyval.value).id = 0; (yyval.value).iValue = 0; (yyval.value).unit = CSSPrimitiveValue::CSS_UNKNOWN;/* ### */ ;}
    break;

  case 150:

    { (yyval.value).id = 0; (yyval.value).string = (yyvsp[(1) - (1)].string); (yyval.value).unit = CSSPrimitiveValue::CSS_RGBCOLOR; ;}
    break;

  case 151:

    {
      (yyval.value) = (yyvsp[(1) - (1)].value);
  ;}
    break;

  case 152:

    { (yyval.value).id = 0; (yyval.value).isInt = true; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_NUMBER; ;}
    break;

  case 153:

    { (yyval.value).id = 0; (yyval.value).isInt = false; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_NUMBER; ;}
    break;

  case 154:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_PERCENTAGE; ;}
    break;

  case 155:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_PX; ;}
    break;

  case 156:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_CM; ;}
    break;

  case 157:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_MM; ;}
    break;

  case 158:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_IN; ;}
    break;

  case 159:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_PT; ;}
    break;

  case 160:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_PC; ;}
    break;

  case 161:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_DEG; ;}
    break;

  case 162:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_RAD; ;}
    break;

  case 163:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_GRAD; ;}
    break;

  case 164:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_MS; ;}
    break;

  case 165:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_S; ;}
    break;

  case 166:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_HZ; ;}
    break;

  case 167:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_KHZ; ;}
    break;

  case 168:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_EMS; ;}
    break;

  case 169:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = Value::Q_EMS; ;}
    break;

  case 170:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_EXS; ;}
    break;

  case 171:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_DPI; ;}
    break;

  case 172:

    { (yyval.value).id = 0; (yyval.value).fValue = (yyvsp[(1) - (2)].val); (yyval.value).unit = CSSPrimitiveValue::CSS_DPCM; ;}
    break;

  case 173:

    {
      Function *f = new Function;
      f->name = (yyvsp[(1) - (5)].string);
      f->args = (yyvsp[(3) - (5)].valueList);
      (yyval.value).id = 0;
      (yyval.value).unit = Value::Function;
      (yyval.value).function = f;
  ;}
    break;

  case 174:

    {
      Function *f = new Function;
      f->name = (yyvsp[(1) - (3)].string);
      f->args = 0;
      (yyval.value).id = 0;
      (yyval.value).unit = Value::Function;
      (yyval.value).function = f;
  ;}
    break;

  case 175:

    { (yyval.string) = (yyvsp[(1) - (2)].string); ;}
    break;

  case 176:

    {
	(yyval.rule) = 0;
#ifdef CSS_DEBUG
	kDebug( 6080 ) << "skipped invalid @-rule";
#endif
    ;}
    break;

  case 177:

    {
	(yyval.rule) = 0;
#ifdef CSS_DEBUG
	kDebug( 6080 ) << "skipped invalid @-rule";
#endif
    ;}
    break;

  case 178:

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






