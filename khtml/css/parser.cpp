/* A Bison parser, made by GNU Bison 1.875a.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003 Free Software Foundation, Inc.

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
#define YYPURE 1

/* Using locations.  */
#define YYLSP_NEEDED 0

/* If NAME_PREFIX is specified substitute the variables and functions
   names.  */
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
     HASH = 268,
     IMPORT_SYM = 269,
     PAGE_SYM = 270,
     MEDIA_SYM = 271,
     FONT_FACE_SYM = 272,
     CHARSET_SYM = 273,
     NAMESPACE_SYM = 274,
     KHTML_RULE_SYM = 275,
     KHTML_DECLS_SYM = 276,
     KHTML_VALUE_SYM = 277,
     IMPORTANT_SYM = 278,
     QEMS = 279,
     EMS = 280,
     EXS = 281,
     PXS = 282,
     CMS = 283,
     MMS = 284,
     INS = 285,
     PTS = 286,
     PCS = 287,
     DEGS = 288,
     RADS = 289,
     GRADS = 290,
     MSECS = 291,
     SECS = 292,
     HERZ = 293,
     KHERZ = 294,
     DIMEN = 295,
     PERCENTAGE = 296,
     NUMBER = 297,
     URI = 298,
     FUNCTION = 299,
     UNICODERANGE = 300
   };
#endif
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
#define HASH 268
#define IMPORT_SYM 269
#define PAGE_SYM 270
#define MEDIA_SYM 271
#define FONT_FACE_SYM 272
#define CHARSET_SYM 273
#define NAMESPACE_SYM 274
#define KHTML_RULE_SYM 275
#define KHTML_DECLS_SYM 276
#define KHTML_VALUE_SYM 277
#define IMPORTANT_SYM 278
#define QEMS 279
#define EMS 280
#define EXS 281
#define PXS 282
#define CMS 283
#define MMS 284
#define INS 285
#define PTS 286
#define PCS 287
#define DEGS 288
#define RADS 289
#define GRADS 290
#define MSECS 291
#define SECS 292
#define HERZ 293
#define KHERZ 294
#define DIMEN 295
#define PERCENTAGE 296
#define NUMBER 297
#define URI 298
#define FUNCTION 299
#define UNICODERANGE 300




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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <string.h>
#include <stdlib.h>

#include <dom/dom_string.h>
#include <xml/dom_docimpl.h>
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
    const struct props *propsPtr = findProp(tagStr, len);
    if (!propsPtr)
        return 0;

    return propsPtr->id;
}

static inline int getValueID(const char *tagStr, int len)
{
    const struct css_value *val = findValue(tagStr, len);
    if (!val)
        return 0;

    return val->id;
}


#define YYDEBUG 0
#define YYMAXDEPTH 0
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

#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)

typedef union YYSTYPE {
    CSSRuleImpl *rule;
    CSSSelector *selector;
    QPtrList<CSSSelector> *selectorList;
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
    bool b;
    char tok;
    Value value;
    ValueList *valueList;
} YYSTYPE;
/* Line 191 of yacc.c.  */

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



/* Line 214 of yacc.c.  */


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
	 || (YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  short yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short) + sizeof (YYSTYPE))				\
      + YYSTACK_GAP_MAXIMUM)

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
#define YYFINAL  16
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   427

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  64
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  57
/* YYNRULES -- Number of rules. */
#define YYNRULES  155
/* YYNRULES -- Number of states. */
#define YYNSTATES  283

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   300

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    61,    17,    55,    54,    58,    15,    62,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    14,    53,
       2,    60,    57,     2,    63,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    16,     2,    59,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    51,    18,    52,    56,     2,     2,     2,
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
       5,     6,     7,     8,     9,    10,    11,    12,    13,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short yyprhs[] =
{
       0,     0,     3,     9,    12,    15,    18,    25,    28,    34,
      35,    38,    39,    42,    45,    46,    52,    56,    60,    61,
      65,    72,    76,    80,    81,    84,    91,    93,    94,    97,
      98,   102,   104,   106,   108,   110,   112,   114,   117,   119,
     121,   122,   124,   126,   131,   134,   142,   143,   147,   150,
     154,   158,   162,   166,   169,   172,   175,   176,   178,   180,
     183,   185,   190,   193,   195,   199,   202,   205,   209,   212,
     215,   217,   219,   222,   225,   227,   229,   231,   234,   237,
     239,   241,   243,   245,   248,   251,   253,   256,   261,   270,
     272,   274,   276,   278,   280,   282,   284,   286,   289,   293,
     299,   304,   309,   314,   320,   326,   330,   334,   339,   344,
     350,   353,   356,   359,   360,   362,   366,   369,   372,   373,
     375,   378,   381,   384,   387,   390,   393,   395,   397,   400,
     403,   406,   409,   412,   415,   418,   421,   424,   427,   430,
     433,   436,   439,   442,   445,   448,   451,   457,   461,   464,
     468,   472,   475,   481,   485,   487
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const yysigned_char yyrhs[] =
{
      65,     0,    -1,    71,    70,    72,    74,    78,    -1,    66,
      69,    -1,    67,    69,    -1,    68,    69,    -1,    25,    51,
      69,    90,    69,    52,    -1,    26,   106,    -1,    27,    51,
      69,   111,    52,    -1,    -1,    69,     4,    -1,    -1,    70,
       5,    -1,    70,     4,    -1,    -1,    23,    69,    11,    69,
      53,    -1,    23,     1,   119,    -1,    23,     1,    53,    -1,
      -1,    72,    73,    70,    -1,    19,    69,    80,    69,    81,
      53,    -1,    19,     1,   119,    -1,    19,     1,    53,    -1,
      -1,    75,    70,    -1,    24,    69,    77,    80,    69,    53,
      -1,    12,    -1,    -1,    76,    69,    -1,    -1,    78,    79,
      70,    -1,    90,    -1,    83,    -1,    86,    -1,    87,    -1,
     118,    -1,   117,    -1,    73,     1,    -1,    11,    -1,    48,
      -1,    -1,    82,    -1,    85,    -1,    82,    54,    69,    85,
      -1,    82,     1,    -1,    21,    69,    82,    51,    69,    84,
      52,    -1,    -1,    84,    90,    69,    -1,    12,    69,    -1,
      20,     1,   119,    -1,    20,     1,    53,    -1,    22,     1,
     119,    -1,    22,     1,    53,    -1,    55,    69,    -1,    56,
      69,    -1,    57,    69,    -1,    -1,    58,    -1,    55,    -1,
      91,   106,    -1,    92,    -1,    91,    54,    69,    92,    -1,
      91,     1,    -1,    93,    -1,    92,    88,    93,    -1,    92,
       1,    -1,    94,    69,    -1,    94,    97,    69,    -1,    97,
      69,    -1,    95,    96,    -1,    96,    -1,    18,    -1,    12,
      18,    -1,    17,    18,    -1,    12,    -1,    17,    -1,    98,
      -1,    97,    98,    -1,    97,     1,    -1,    13,    -1,    99,
      -1,   102,    -1,   105,    -1,    15,    12,    -1,    95,   101,
      -1,   101,    -1,    12,    69,    -1,    16,    69,   100,    59,
      -1,    16,    69,   100,   103,    69,   104,    69,    59,    -1,
      60,    -1,     6,    -1,     7,    -1,     8,    -1,     9,    -1,
      10,    -1,    12,    -1,    11,    -1,    14,    12,    -1,    14,
      14,    12,    -1,    14,    49,    69,    93,    61,    -1,    51,
      69,   108,    52,    -1,    51,    69,     1,    52,    -1,    51,
      69,   107,    52,    -1,    51,    69,   107,   108,    52,    -1,
      51,    69,   107,     1,    52,    -1,   108,    53,    69,    -1,
       1,    53,    69,    -1,   107,   108,    53,    69,    -1,   107,
       1,    53,    69,    -1,   109,    14,    69,   111,   110,    -1,
       1,   119,    -1,    12,    69,    -1,    28,    69,    -1,    -1,
     113,    -1,   111,   112,   113,    -1,    62,    69,    -1,    54,
      69,    -1,    -1,   114,    -1,    89,   114,    -1,    45,    69,
      -1,    11,    69,    -1,    12,    69,    -1,    48,    69,    -1,
      50,    69,    -1,   116,    -1,   115,    -1,    47,    69,    -1,
      46,    69,    -1,    32,    69,    -1,    33,    69,    -1,    34,
      69,    -1,    35,    69,    -1,    36,    69,    -1,    37,    69,
      -1,    38,    69,    -1,    39,    69,    -1,    40,    69,    -1,
      41,    69,    -1,    42,    69,    -1,    43,    69,    -1,    44,
      69,    -1,    30,    69,    -1,    29,    69,    -1,    31,    69,
      -1,    49,    69,   111,    61,    69,    -1,    49,    69,     1,
      -1,    13,    69,    -1,    63,     1,   119,    -1,    63,     1,
      53,    -1,     1,   119,    -1,    51,     1,   120,     1,    52,
      -1,    51,     1,    52,    -1,   119,    -1,   120,     1,   119,
      -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   253,   253,   254,   255,   256,   260,   267,   273,   298,
     299,   302,   304,   305,   308,   310,   315,   316,   319,   321,
     332,   342,   345,   351,   352,   356,   360,   364,   365,   368,
     370,   381,   382,   383,   384,   385,   386,   387,   391,   392,
     396,   399,   404,   408,   413,   420,   434,   435,   445,   467,
     470,   476,   479,   485,   486,   487,   488,   492,   493,   497,
     517,   530,   544,   551,   554,   575,   582,   586,   591,   599,
     600,   609,   610,   611,   616,   636,   640,   644,   654,   661,
     667,   668,   669,   673,   682,   683,   690,   714,   719,   728,
     731,   734,   737,   740,   743,   749,   750,   754,   760,   765,
     774,   777,   780,   783,   788,   794,   798,   801,   806,   812,
     834,   840,   847,   848,   852,   856,   872,   875,   878,   884,
     885,   887,   888,   889,   895,   896,   897,   899,   905,   906,
     907,   908,   909,   910,   911,   912,   913,   914,   915,   916,
     917,   918,   919,   920,   921,   922,   927,   935,   951,   958,
     964,   973,   999,  1000,  1004,  1005
};
#endif

#if YYDEBUG || YYERROR_VERBOSE
/* YYTNME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "UNIMPORTANT_TOK", "S", "SGML_CD", 
  "INCLUDES", "DASHMATCH", "BEGINSWITH", "ENDSWITH", "CONTAINS", "STRING", 
  "IDENT", "HASH", "':'", "'.'", "'['", "'*'", "'|'", "IMPORT_SYM", 
  "PAGE_SYM", "MEDIA_SYM", "FONT_FACE_SYM", "CHARSET_SYM", 
  "NAMESPACE_SYM", "KHTML_RULE_SYM", "KHTML_DECLS_SYM", "KHTML_VALUE_SYM", 
  "IMPORTANT_SYM", "QEMS", "EMS", "EXS", "PXS", "CMS", "MMS", "INS", 
  "PTS", "PCS", "DEGS", "RADS", "GRADS", "MSECS", "SECS", "HERZ", "KHERZ", 
  "DIMEN", "PERCENTAGE", "NUMBER", "URI", "FUNCTION", "UNICODERANGE", 
  "'{'", "'}'", "';'", "','", "'+'", "'~'", "'>'", "'-'", "']'", "'='", 
  "')'", "'/'", "'@'", "$accept", "stylesheet", "khtml_rule", 
  "khtml_decls", "khtml_value", "maybe_space", "maybe_sgml", 
  "maybe_charset", "import_list", "import", "maybe_namespace", 
  "namespace", "ns_prefix", "maybe_ns_prefix", "rule_list", "rule", 
  "string_or_uri", "maybe_media_list", "media_list", "media", 
  "ruleset_list", "medium", "page", "font_face", "combinator", 
  "unary_operator", "ruleset", "selector_list", "selector", 
  "simple_selector", "ns_element", "ns_selector", "element_name", 
  "specifier_list", "specifier", "class", "ns_attrib_id", "attrib_id", 
  "attrib", "match", "ident_or_string", "pseudo", "declaration_block", 
  "declaration_list", "declaration", "property", "prio", "expr", 
  "operator", "term", "unary_term", "function", "hexcolor", "invalid_at", 
  "invalid_rule", "invalid_block", "invalid_block_list", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const unsigned short yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,    58,    46,    91,    42,   124,   269,
     270,   271,   272,   273,   274,   275,   276,   277,   278,   279,
     280,   281,   282,   283,   284,   285,   286,   287,   288,   289,
     290,   291,   292,   293,   294,   295,   296,   297,   298,   299,
     300,   123,   125,    59,    44,    43,   126,    62,    45,    93,
      61,    41,    47,    64
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    64,    65,    65,    65,    65,    66,    67,    68,    69,
      69,    70,    70,    70,    71,    71,    71,    71,    72,    72,
      73,    73,    73,    74,    74,    75,    76,    77,    77,    78,
      78,    79,    79,    79,    79,    79,    79,    79,    80,    80,
      81,    81,    82,    82,    82,    83,    84,    84,    85,    86,
      86,    87,    87,    88,    88,    88,    88,    89,    89,    90,
      91,    91,    91,    92,    92,    92,    93,    93,    93,    94,
      94,    95,    95,    95,    96,    96,    97,    97,    97,    98,
      98,    98,    98,    99,   100,   100,   101,   102,   102,   103,
     103,   103,   103,   103,   103,   104,   104,   105,   105,   105,
     106,   106,   106,   106,   106,   107,   107,   107,   107,   108,
     108,   109,   110,   110,   111,   111,   112,   112,   112,   113,
     113,   113,   113,   113,   113,   113,   113,   113,   114,   114,
     114,   114,   114,   114,   114,   114,   114,   114,   114,   114,
     114,   114,   114,   114,   114,   114,   115,   115,   116,   117,
     117,   118,   119,   119,   120,   120
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     5,     2,     2,     2,     6,     2,     5,     0,
       2,     0,     2,     2,     0,     5,     3,     3,     0,     3,
       6,     3,     3,     0,     2,     6,     1,     0,     2,     0,
       3,     1,     1,     1,     1,     1,     1,     2,     1,     1,
       0,     1,     1,     4,     2,     7,     0,     3,     2,     3,
       3,     3,     3,     2,     2,     2,     0,     1,     1,     2,
       1,     4,     2,     1,     3,     2,     2,     3,     2,     2,
       1,     1,     2,     2,     1,     1,     1,     2,     2,     1,
       1,     1,     1,     2,     2,     1,     2,     4,     8,     1,
       1,     1,     1,     1,     1,     1,     1,     2,     3,     5,
       4,     4,     4,     5,     5,     3,     3,     4,     4,     5,
       2,     2,     2,     0,     1,     3,     2,     2,     0,     1,
       2,     2,     2,     2,     2,     2,     1,     1,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     5,     3,     2,     3,
       3,     2,     5,     3,     1,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
      14,     0,     0,     0,     0,     0,     9,     9,     9,    11,
       0,     0,     9,     9,     7,     9,     1,     3,     4,     5,
      18,     0,    17,    16,    10,     9,     0,     0,     0,    13,
      12,    23,     0,     0,    74,    79,     0,     0,     9,    75,
      71,     9,     0,     0,    63,     9,     0,    70,     0,    76,
      80,    81,    82,     0,     9,     0,     0,     0,     9,     9,
       9,     9,     9,     9,     9,     9,     9,     9,     9,     9,
       9,     9,     9,     9,     9,     9,     9,     9,     9,     9,
       9,     9,     9,    58,    57,     0,   118,   114,   119,   127,
     126,     0,     9,    11,    29,    11,   153,   154,     0,    15,
      72,    97,     0,     9,    83,     0,    73,     0,    62,     9,
      59,    65,     9,     9,     9,     0,    66,     0,    74,    75,
      69,    78,    68,    77,   101,     9,   110,   111,     0,   102,
       0,   100,     9,     9,   122,   123,   148,   144,   143,   145,
     130,   131,   132,   133,   134,   135,   136,   137,   138,   139,
     140,   141,   142,   121,   129,   128,   124,     0,   125,   120,
       8,     9,     9,     0,     0,     0,    27,    19,     0,    24,
       0,    98,     0,     9,     0,     0,     0,    85,     6,     0,
      53,    54,    55,    64,    67,   106,   104,     9,   103,     9,
     105,     0,   147,   118,   117,   116,   115,    22,    21,    38,
      39,     9,    26,     9,     0,     0,     0,     9,     0,     0,
       0,    11,    32,    33,    34,    31,    36,    35,   152,   155,
       0,    86,     9,    84,    90,    91,    92,    93,    94,    87,
      89,     9,     0,   108,   107,   118,     9,    40,    28,     9,
     151,     0,     0,     0,     0,    37,    30,    99,     0,     9,
     109,   146,     9,     0,     0,    42,     0,    50,    49,     0,
      52,    51,   150,   149,    96,    95,     9,   112,    48,    20,
      44,     9,    25,     9,     0,     0,    46,    88,    43,     0,
      45,     9,    47
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,     5,     6,     7,     8,   221,    20,     9,    31,    93,
      94,    95,   203,   204,   168,   211,   201,   253,   254,   212,
     279,   255,   213,   214,   115,    85,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,   176,   177,    51,   231,
     266,    52,    14,    55,    56,    57,   250,    86,   163,    87,
      88,    89,    90,   216,   217,   126,    98
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -161
static const short yypact[] =
{
     201,    17,   -28,   -22,   -12,    45,  -161,  -161,  -161,  -161,
      76,    99,  -161,  -161,  -161,  -161,  -161,    48,    48,    48,
     144,    54,  -161,  -161,  -161,  -161,   228,    15,   310,  -161,
    -161,    80,   122,    34,    70,  -161,   116,    94,  -161,    96,
    -161,  -161,    35,   163,  -161,   239,    88,  -161,   154,  -161,
    -161,  -161,  -161,   230,  -161,    95,   211,   103,  -161,  -161,
    -161,  -161,  -161,  -161,  -161,  -161,  -161,  -161,  -161,  -161,
    -161,  -161,  -161,  -161,  -161,  -161,  -161,  -161,  -161,  -161,
    -161,  -161,  -161,  -161,  -161,   380,   -32,  -161,  -161,  -161,
    -161,   114,  -161,  -161,  -161,  -161,  -161,  -161,   125,  -161,
    -161,  -161,   111,  -161,  -161,   195,  -161,    46,  -161,  -161,
    -161,  -161,  -161,  -161,  -161,   312,    48,   154,  -161,  -161,
    -161,  -161,    48,  -161,  -161,  -161,  -161,    48,   256,  -161,
     267,  -161,  -161,  -161,    48,    48,    48,    48,    48,    48,
      48,    48,    48,    48,    48,    48,    48,    48,    48,    48,
      48,    48,    48,    48,    48,    48,    48,   255,    48,  -161,
    -161,  -161,  -161,   350,   132,     6,    39,   144,   121,   144,
     280,  -161,   228,    70,    96,   133,    25,  -161,  -161,   228,
      48,    48,    48,  -161,    48,    48,  -161,  -161,  -161,  -161,
      48,   310,  -161,    90,    48,    48,  -161,  -161,  -161,  -161,
    -161,  -161,  -161,  -161,    -2,   105,   149,  -161,   158,   181,
     186,  -161,  -161,  -161,  -161,  -161,  -161,  -161,  -161,  -161,
     135,    48,  -161,  -161,  -161,  -161,  -161,  -161,  -161,  -161,
    -161,  -161,   261,    48,    48,   169,  -161,    97,    48,  -161,
    -161,   150,    97,   196,   207,  -161,   144,  -161,   142,  -161,
    -161,    48,  -161,   151,    40,  -161,    44,  -161,  -161,    41,
    -161,  -161,  -161,  -161,  -161,  -161,  -161,    48,    48,  -161,
    -161,  -161,  -161,  -161,    -3,    97,    48,  -161,  -161,   177,
    -161,  -161,    48
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -161,  -161,  -161,  -161,  -161,    -1,   -91,  -161,  -161,    57,
    -161,  -161,  -161,  -161,  -161,  -161,    12,  -161,    -9,  -161,
    -161,   -36,  -161,  -161,  -161,  -161,  -160,  -161,    71,   -89,
    -161,   152,   215,   224,   -35,  -161,  -161,   131,  -161,  -161,
    -161,  -161,   229,  -161,   278,  -161,  -161,  -142,  -161,   148,
     249,  -161,  -161,  -161,  -161,    -7,  -161
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -114
static const short yytable[] =
{
      11,    24,   167,    23,   169,    17,    18,    19,   215,   199,
      24,    26,    27,   123,    28,   193,    53,   199,    10,    24,
     160,    -9,   161,    12,    33,    97,   183,    54,    -9,    13,
     162,   224,   225,   226,   227,   228,   108,   105,    24,    15,
     107,   270,   270,    24,   116,    16,   200,   122,    24,   235,
      24,   202,    24,   127,   200,    32,   277,   134,   135,   136,
     137,   138,   139,   140,   141,   142,   143,   144,   145,   146,
     147,   148,   149,   150,   151,   152,   153,   154,   155,   156,
     157,   158,   123,   220,   229,   230,    13,    99,   100,   109,
     165,   166,   273,   -41,   271,   271,   128,   272,   178,    91,
     118,    24,   172,    24,    92,   119,   104,    54,   179,   252,
      25,   180,   181,   182,   106,   164,   184,   133,    -9,   281,
     246,    -2,   205,   171,   185,    -9,   170,    21,   101,    22,
     102,   190,   191,    34,    35,    36,    37,    38,    39,    40,
      91,   206,   207,   208,   161,   222,    24,   129,    29,    30,
     241,   236,   162,   264,   265,   121,    21,   198,    -9,   243,
     194,   195,    -9,   219,   111,   103,    -9,    35,    36,    37,
      38,    -9,    -9,    21,    96,   -56,   -56,   -56,   -56,   -56,
     -56,   -56,   244,    21,   209,   197,   233,   245,   234,    34,
      35,    36,    37,    38,    39,    40,   247,   249,   240,    24,
     237,    21,   238,   257,   269,    -9,   242,   173,    -9,    -9,
      -9,    -9,   174,    40,   -60,    -9,   239,   -60,   112,   113,
     114,  -113,  -113,   161,     1,   210,     2,     3,     4,   280,
     248,   162,    24,   259,   258,   251,   261,   263,   256,   278,
      34,    35,    36,    37,    38,    39,    40,    21,   267,   260,
     232,   268,    35,    36,    37,    38,   192,   175,    21,    24,
     262,   120,   111,   131,   132,   274,    58,    59,    60,   117,
     275,   110,   276,   -56,   -56,   -56,   -56,   -56,   -56,   -56,
     282,    21,   124,   125,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,   223,    21,   186,   187,
      83,   196,   -61,    84,    24,   -61,   112,   113,   114,   188,
     189,    58,    59,    60,    34,    35,    36,    37,    38,    39,
      40,    21,   218,   130,   159,     0,     0,     0,     0,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    58,    59,    60,     0,    83,     0,     0,    84,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,     0,     0,     0,     0,    83,     0,     0,    84,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,     0,    78,    79
};

static const short yycheck[] =
{
       1,     4,    93,    10,    95,     6,     7,     8,   168,    11,
       4,    12,    13,    48,    15,   157,     1,    11,     1,     4,
      52,     4,    54,    51,    25,    32,   115,    12,    11,    51,
      62,     6,     7,     8,     9,    10,     1,    38,     4,    51,
      41,     1,     1,     4,    45,     0,    48,    48,     4,   191,
       4,    12,     4,    54,    48,     1,    59,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,   117,   172,    59,    60,    51,    53,    18,    54,
      91,    92,    51,    53,    54,    54,     1,    53,    52,    19,
      12,     4,   103,     4,    24,    17,    12,    12,   109,    12,
      11,   112,   113,   114,    18,     1,   117,    14,     4,   279,
     211,     0,     1,    12,   125,    11,     1,    51,    12,    53,
      14,   132,   133,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    54,    12,     4,    52,     4,     5,
       1,    61,    62,    11,    12,     1,    51,   164,     4,     1,
     161,   162,    48,   170,     1,    49,    12,    13,    14,    15,
      16,    17,    18,    51,    52,    12,    13,    14,    15,    16,
      17,    18,     1,    51,    63,    53,   187,     1,   189,    12,
      13,    14,    15,    16,    17,    18,    61,    28,   205,     4,
     201,    51,   203,    53,    53,    51,   207,    12,    54,    55,
      56,    57,    17,    18,    51,    61,   204,    54,    55,    56,
      57,    52,    53,    54,    23,   168,    25,    26,    27,    52,
     231,    62,     4,   242,   241,   236,   243,   244,   239,   275,
      12,    13,    14,    15,    16,    17,    18,    51,   249,    53,
     179,   252,    13,    14,    15,    16,     1,   105,    51,     4,
      53,    46,     1,    52,    53,   266,    11,    12,    13,    45,
     271,    42,   273,    12,    13,    14,    15,    16,    17,    18,
     281,    51,    52,    53,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,   175,    51,    52,    53,
      55,   163,    51,    58,     4,    54,    55,    56,    57,    52,
      53,    11,    12,    13,    12,    13,    14,    15,    16,    17,
      18,    51,    52,    55,    85,    -1,    -1,    -1,    -1,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    11,    12,    13,    -1,    55,    -1,    -1,    58,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    -1,    -1,    -1,    -1,    55,    -1,    -1,    58,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    -1,    46,    47
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,    23,    25,    26,    27,    65,    66,    67,    68,    71,
       1,    69,    51,    51,   106,    51,     0,    69,    69,    69,
      70,    51,    53,   119,     4,    11,    69,    69,    69,     4,
       5,    72,     1,    69,    12,    13,    14,    15,    16,    17,
      18,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   102,   105,     1,    12,   107,   108,   109,    11,    12,
      13,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    55,    58,    89,   111,   113,   114,   115,
     116,    19,    24,    73,    74,    75,    52,   119,   120,    53,
      18,    12,    14,    49,    12,    69,    18,    69,     1,    54,
     106,     1,    55,    56,    57,    88,    69,    97,    12,    17,
      96,     1,    69,    98,    52,    53,   119,    69,     1,    52,
     108,    52,    53,    14,    69,    69,    69,    69,    69,    69,
      69,    69,    69,    69,    69,    69,    69,    69,    69,    69,
      69,    69,    69,    69,    69,    69,    69,    69,    69,   114,
      52,    54,    62,   112,     1,    69,    69,    70,    78,    70,
       1,    12,    69,    12,    17,    95,   100,   101,    52,    69,
      69,    69,    69,    93,    69,    69,    52,    53,    52,    53,
      69,    69,     1,   111,    69,    69,   113,    53,   119,    11,
      48,    80,    12,    76,    77,     1,    20,    21,    22,    63,
      73,    79,    83,    86,    87,    90,   117,   118,    52,   119,
      93,    69,    12,   101,     6,     7,     8,     9,    10,    59,
      60,   103,    92,    69,    69,   111,    61,    69,    69,    80,
     119,     1,    69,     1,     1,     1,    70,    61,    69,    28,
     110,    69,    12,    81,    82,    85,    69,    53,   119,    82,
      53,   119,    53,   119,    11,    12,   104,    69,    69,    53,
       1,    54,    53,    51,    69,    69,    69,    59,    85,    84,
      52,    90,    69
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
                  Token, Value);	\
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
yysymprint (FILE *yyoutput, int yytype, YYSTYPE *yyvaluep)
#else
static void
yysymprint (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

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
yydestruct (int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yytype, yyvaluep)
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  switch (yytype)
    {
      case 81: /* maybe_media_list */

        { delete yyvaluep->mediaList; yyvaluep->mediaList = 0; };

        break;
      case 82: /* media_list */

        { delete yyvaluep->mediaList; yyvaluep->mediaList = 0; };

        break;
      case 84: /* ruleset_list */

        { delete yyvaluep->ruleList; yyvaluep->ruleList = 0; };

        break;
      case 91: /* selector_list */

        { delete yyvaluep->selectorList; yyvaluep->selectorList = 0; };

        break;
      case 92: /* selector */

        { delete yyvaluep->selector; yyvaluep->selector = 0; };

        break;
      case 93: /* simple_selector */

        { delete yyvaluep->selector; yyvaluep->selector = 0; };

        break;
      case 97: /* specifier_list */

        { delete yyvaluep->selector; yyvaluep->selector = 0; };

        break;
      case 98: /* specifier */

        { delete yyvaluep->selector; yyvaluep->selector = 0; };

        break;
      case 99: /* class */

        { delete yyvaluep->selector; yyvaluep->selector = 0; };

        break;
      case 102: /* attrib */

        { delete yyvaluep->selector; yyvaluep->selector = 0; };

        break;
      case 105: /* pseudo */

        { delete yyvaluep->selector; yyvaluep->selector = 0; };

        break;
      case 111: /* expr */

        { delete yyvaluep->valueList; yyvaluep->valueList = 0; };

        break;

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
  /* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;

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



#define YYPOPSTACK   (yyvsp--, yyssp--)

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


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


	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow ("parser stack overflow",
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

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


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 6:

    {
        CSSParser *p = static_cast<CSSParser *>(parser);
	p->rule = yyvsp[-2].rule;
    ;}
    break;

  case 7:

    {
	/* can be empty */
    ;}
    break;

  case 8:

    {
	CSSParser *p = static_cast<CSSParser *>(parser);
	if ( yyvsp[-1].valueList ) {
	    p->valueList = yyvsp[-1].valueList;
#ifdef CSS_DEBUG
	    kdDebug( 6080 ) << "   got property for " << p->id <<
		(p->important?" important":"")<< endl;
	    bool ok =
#endif
		p->parseValue( p->id, p->important );
#ifdef CSS_DEBUG
	    if ( !ok )
		kdDebug( 6080 ) << "     couldn't parse value!" << endl;
#endif
	}
#ifdef CSS_DEBUG
	else
	    kdDebug( 6080 ) << "     no value found!" << endl;
#endif
	delete p->valueList;
	p->valueList = 0;
    ;}
    break;

  case 15:

    {
#ifdef CSS_DEBUG
     kdDebug( 6080 ) << "charset rule: " << qString(yyvsp[-2].string) << endl;
#endif
 ;}
    break;

  case 19:

    {
     CSSParser *p = static_cast<CSSParser *>(parser);
     if ( yyvsp[-1].rule && p->styleElement && p->styleElement->isCSSStyleSheet() ) {
	 p->styleElement->append( yyvsp[-1].rule );
     } else {
	 delete yyvsp[-1].rule;
     }
 ;}
    break;

  case 20:

    {
#ifdef CSS_DEBUG
	kdDebug( 6080 ) << "@import: " << qString(yyvsp[-3].string) << endl;
#endif
	CSSParser *p = static_cast<CSSParser *>(parser);
	if ( yyvsp[-1].mediaList && p->styleElement && p->styleElement->isCSSStyleSheet() )
	    yyval.rule = new CSSImportRuleImpl( p->styleElement, domString(yyvsp[-3].string), yyvsp[-1].mediaList );
	else
	    yyval.rule = 0;
    ;}
    break;

  case 21:

    {
        yyval.rule = 0;
    ;}
    break;

  case 22:

    {
        yyval.rule = 0;
    ;}
    break;

  case 27:

    { yyval.string.string = 0; yyval.string.length = 0; ;}
    break;

  case 30:

    {
     CSSParser *p = static_cast<CSSParser *>(parser);
     if ( yyvsp[-1].rule && p->styleElement && p->styleElement->isCSSStyleSheet() ) {
	 p->styleElement->append( yyvsp[-1].rule );
     } else {
	 delete yyvsp[-1].rule;
     }
 ;}
    break;

  case 37:

    { delete yyvsp[-1].rule; yyval.rule = 0; ;}
    break;

  case 40:

    {
	yyval.mediaList = new MediaListImpl();
    ;}
    break;

  case 42:

    {
	yyval.mediaList = new MediaListImpl();
	yyval.mediaList->appendMedium( domString(yyvsp[0].string).lower() );
    ;}
    break;

  case 43:

    {
	yyval.mediaList = yyvsp[-3].mediaList;
	if (yyval.mediaList)
	    yyval.mediaList->appendMedium( domString(yyvsp[0].string).lower() );
    ;}
    break;

  case 44:

    {
       delete yyvsp[-1].mediaList;
       yyval.mediaList = 0;
    ;}
    break;

  case 45:

    {
	CSSParser *p = static_cast<CSSParser *>(parser);
	if ( yyvsp[-4].mediaList && yyvsp[-1].ruleList &&
	     p->styleElement && p->styleElement->isCSSStyleSheet() ) {
	    yyval.rule = new CSSMediaRuleImpl( static_cast<CSSStyleSheetImpl*>(p->styleElement), yyvsp[-4].mediaList, yyvsp[-1].ruleList );
	} else {
	    yyval.rule = 0;
	    delete yyvsp[-4].mediaList;
	    delete yyvsp[-1].ruleList;
	}
    ;}
    break;

  case 46:

    { yyval.ruleList = 0; ;}
    break;

  case 47:

    {
      yyval.ruleList = yyvsp[-2].ruleList;
      if ( yyvsp[-1].rule ) {
	  if ( !yyval.ruleList ) yyval.ruleList = new CSSRuleListImpl();
	  yyval.ruleList->append( yyvsp[-1].rule );
      }
  ;}
    break;

  case 48:

    {
      yyval.string = yyvsp[-1].string;
  ;}
    break;

  case 49:

    {
      yyval.rule = 0;
    ;}
    break;

  case 50:

    {
      yyval.rule = 0;
    ;}
    break;

  case 51:

    {
      yyval.rule = 0;
    ;}
    break;

  case 52:

    {
      yyval.rule = 0;
    ;}
    break;

  case 53:

    { yyval.relation = CSSSelector::Sibling; ;}
    break;

  case 54:

    { yyval.relation = CSSSelector::Cousin; ;}
    break;

  case 55:

    { yyval.relation = CSSSelector::Child; ;}
    break;

  case 56:

    { yyval.relation = CSSSelector::Descendant; ;}
    break;

  case 57:

    { yyval.val = -1; ;}
    break;

  case 58:

    { yyval.val = 1; ;}
    break;

  case 59:

    {
#ifdef CSS_DEBUG
	kdDebug( 6080 ) << "got ruleset" << endl << "  selector:" << endl;
#endif
	CSSParser *p = static_cast<CSSParser *>(parser);
	if ( yyvsp[-1].selectorList && yyvsp[0].ok && p->numParsedProperties ) {
	    CSSStyleRuleImpl *rule = new CSSStyleRuleImpl( p->styleElement );
	    CSSStyleDeclarationImpl *decl = p->createStyleDeclaration( rule );
	    rule->setSelector( yyvsp[-1].selectorList );
	    rule->setDeclaration(decl);
	    yyval.rule = rule;
	} else {
	    yyval.rule = 0;
	    delete yyvsp[-1].selectorList;
	    p->clearProperties();
	}
    ;}
    break;

  case 60:

    {
	if ( yyvsp[0].selector ) {
	    yyval.selectorList = new QPtrList<CSSSelector>;
            yyval.selectorList->setAutoDelete( true );
#ifdef CSS_DEBUG
	    kdDebug( 6080 ) << "   got simple selector:" << endl;
	    yyvsp[0].selector->print();
#endif
	    yyval.selectorList->append( yyvsp[0].selector );
	} else {
	    yyval.selectorList = 0;
	}
    ;}
    break;

  case 61:

    {
	if ( yyvsp[-3].selectorList && yyvsp[0].selector ) {
	    yyval.selectorList = yyvsp[-3].selectorList;
	    yyval.selectorList->append( yyvsp[0].selector );
#ifdef CSS_DEBUG
	    kdDebug( 6080 ) << "   got simple selector:" << endl;
	    yyvsp[0].selector->print();
#endif
	} else {
	    delete yyvsp[-3].selectorList;
	    delete yyvsp[0].selector;
	    yyval.selectorList = 0;
	}
    ;}
    break;

  case 62:

    {
	delete yyvsp[-1].selectorList;
	yyval.selectorList = 0;
    ;}
    break;

  case 63:

    {
	yyval.selector = yyvsp[0].selector;
    ;}
    break;

  case 64:

    {
	if ( !yyvsp[-2].selector || !yyvsp[0].selector ) {
	    delete yyvsp[-2].selector;
	    delete yyvsp[0].selector;
	    yyval.selector = 0;
	} else {
	    yyval.selector = yyvsp[0].selector;
	    CSSSelector *end = yyvsp[0].selector;
	    while( end->tagHistory )
		end = end->tagHistory;
	    end->relation = yyvsp[-1].relation;
	    end->tagHistory = yyvsp[-2].selector;
	    if ( yyvsp[-1].relation == CSSSelector::Descendant ||
		 yyvsp[-1].relation == CSSSelector::Child ) {
		CSSParser *p = static_cast<CSSParser *>(parser);
		DOM::DocumentImpl *doc = p->document();
		if ( doc )
		    doc->setUsesDescendantRules(true);
	    }
	}
    ;}
    break;

  case 65:

    {
	delete yyvsp[-1].selector;
	yyval.selector = 0;
    ;}
    break;

  case 66:

    {
	yyval.selector = new CSSSelector();
	yyval.selector->tag = yyvsp[-1].element;
    ;}
    break;

  case 67:

    {
	yyval.selector = yyvsp[-1].selector;
        if ( yyval.selector )
	    yyval.selector->tag = yyvsp[-2].element;
    ;}
    break;

  case 68:

    {
	yyval.selector = yyvsp[-1].selector;
        if ( yyval.selector )
	    yyval.selector->tag = 0xffffffff;
    ;}
    break;

  case 69:

    { yyval.element = (yyvsp[-1].ns<<16) | yyvsp[0].element; ;}
    break;

  case 70:

    {
        /* according to the specs this one matches all namespaces if no
	   default namespace has been specified otherwise the default namespace */
	CSSParser *p = static_cast<CSSParser *>(parser);
	yyval.element = (p->defaultNamespace<<16) | yyvsp[0].element;
    ;}
    break;

  case 71:

    { yyval.ns = 0; ;}
    break;

  case 72:

    { yyval.ns = 1; /* #### insert correct namespace id here */ ;}
    break;

  case 73:

    { yyval.ns = 0xffff; ;}
    break;

  case 74:

    {
	CSSParser *p = static_cast<CSSParser *>(parser);
	DOM::DocumentImpl *doc = p->document();
	QString tag = qString(yyvsp[0].string);
	if ( doc ) {
	    if (doc->isHTMLDocument())
		tag = tag.lower();
	    const DOMString dtag(tag);
#ifdef APPLE_CHANGES
            yyval.element = doc->tagId(0, dtag.implementation(), false);
#else
	    yyval.element = doc->getId(NodeImpl::ElementId, dtag.implementation(), false, true);
#endif
	} else {
	    yyval.element = khtml::getTagID(tag.lower().ascii(), tag.length());
	    // this case should never happen - only when loading
	    // the default stylesheet - which must not contain unknown tags
// 	    assert($$ != 0);
	}
    ;}
    break;

  case 75:

    { yyval.element = 0xffff; ;}
    break;

  case 76:

    {
	yyval.selector = yyvsp[0].selector;
	yyval.selector->nonCSSHint = static_cast<CSSParser *>(parser)->nonCSSHint;
    ;}
    break;

  case 77:

    {
	yyval.selector = yyvsp[-1].selector;
	if ( yyval.selector ) {
            CSSSelector *end = yyvsp[-1].selector;
            while( end->tagHistory )
                end = end->tagHistory;
            end->relation = CSSSelector::SubSelector;
            end->tagHistory = yyvsp[0].selector;
	}
    ;}
    break;

  case 78:

    {
	delete yyvsp[-1].selector;
	yyval.selector = 0;
    ;}
    break;

  case 79:

    {
	yyval.selector = new CSSSelector();
	yyval.selector->match = CSSSelector::Id;
	yyval.selector->attr = ATTR_ID;
	yyval.selector->value = domString(yyvsp[0].string);
    ;}
    break;

  case 83:

    {
	yyval.selector = new CSSSelector();
	yyval.selector->match = CSSSelector::List;
	yyval.selector->attr = ATTR_CLASS;
	yyval.selector->value = domString(yyvsp[0].string);
    ;}
    break;

  case 84:

    { yyval.attribute = (yyvsp[-1].ns<<16) | yyvsp[0].attribute; ;}
    break;

  case 85:

    {
	/* opposed to elements, these only match for non namespaced attributes */
	yyval.attribute = yyvsp[0].attribute;
    ;}
    break;

  case 86:

    {
	CSSParser *p = static_cast<CSSParser *>(parser);
	DOM::DocumentImpl *doc = p->document();

	QString attr = qString(yyvsp[-1].string);
	if ( doc ) {
	    if (doc->isHTMLDocument())
		attr = attr.lower();
	    const DOMString dattr(attr);
#ifdef APPLE_CHANGES
            yyval.attribute = doc->attrId(0, dattr.implementation(), false);
#else
	    yyval.attribute = doc->getId(NodeImpl::AttributeId, dattr.implementation(), false, true);
#endif
	} else {
	    yyval.attribute = khtml::getAttrID(attr.lower().ascii(), attr.length());
	    // this case should never happen - only when loading
	    // the default stylesheet - which must not contain unknown attributes
	    assert(yyval.attribute != 0);
	    }
    ;}
    break;

  case 87:

    {
	yyval.selector = new CSSSelector();
	yyval.selector->attr = yyvsp[-1].attribute;
	yyval.selector->match = CSSSelector::Set;
    ;}
    break;

  case 88:

    {
	yyval.selector = new CSSSelector();
	yyval.selector->attr = yyvsp[-5].attribute;
	yyval.selector->match = (CSSSelector::Match)yyvsp[-4].val;
	yyval.selector->value = domString(yyvsp[-2].string);
    ;}
    break;

  case 89:

    {
	yyval.val = CSSSelector::Exact;
    ;}
    break;

  case 90:

    {
	yyval.val = CSSSelector::List;
    ;}
    break;

  case 91:

    {
	yyval.val = CSSSelector::Hyphen;
    ;}
    break;

  case 92:

    {
	yyval.val = CSSSelector::Begin;
    ;}
    break;

  case 93:

    {
	yyval.val = CSSSelector::End;
    ;}
    break;

  case 94:

    {
	yyval.val = CSSSelector::Contain;
    ;}
    break;

  case 97:

    {
	yyval.selector = new CSSSelector();
	yyval.selector->match = CSSSelector::Pseudo;
	yyval.selector->value = domString(yyvsp[0].string);
    ;}
    break;

  case 98:

    {
	yyval.selector = new CSSSelector();
	yyval.selector->match = CSSSelector::Pseudo;
        yyval.selector->value = domString(yyvsp[0].string);
    ;}
    break;

  case 99:

    {
        yyval.selector = new CSSSelector();
        yyval.selector->match = CSSSelector::Pseudo;
        yyval.selector->simpleSelector = yyvsp[-1].selector;
        yyval.selector->value = domString(yyvsp[-3].string);
    ;}
    break;

  case 100:

    {
	yyval.ok = yyvsp[-1].ok;
    ;}
    break;

  case 101:

    {
	yyval.ok = false;
    ;}
    break;

  case 102:

    {
	yyval.ok = yyvsp[-1].ok;
    ;}
    break;

  case 103:

    {
	yyval.ok = yyvsp[-2].ok;
	if ( yyvsp[-1].ok )
	    yyval.ok = yyvsp[-1].ok;
    ;}
    break;

  case 104:

    {
	yyval.ok = yyvsp[-2].ok;
    ;}
    break;

  case 105:

    {
	yyval.ok = yyvsp[-2].ok;
    ;}
    break;

  case 106:

    {
        yyval.ok = false;
    ;}
    break;

  case 107:

    {
	yyval.ok = yyvsp[-3].ok;
	if ( yyvsp[-2].ok )
	    yyval.ok = yyvsp[-2].ok;
    ;}
    break;

  case 108:

    {
        yyval.ok = yyvsp[-3].ok;
    ;}
    break;

  case 109:

    {
	yyval.ok = false;
	CSSParser *p = static_cast<CSSParser *>(parser);
	if ( yyvsp[-4].prop_id && yyvsp[-1].valueList ) {
	    p->valueList = yyvsp[-1].valueList;
#ifdef CSS_DEBUG
	    kdDebug( 6080 ) << "   got property: " << yyvsp[-4].prop_id <<
		(yyvsp[0].b?" important":"")<< endl;
#endif
	        bool ok = p->parseValue( yyvsp[-4].prop_id, yyvsp[0].b );
                if ( ok )
		    yyval.ok = ok;
#ifdef CSS_DEBUG
	        else
		    kdDebug( 6080 ) << "     couldn't parse value!" << endl;
#endif
	} else {
            delete yyvsp[-1].valueList;
        }
	delete p->valueList;
	p->valueList = 0;
    ;}
    break;

  case 110:

    {
        yyval.ok = false;
    ;}
    break;

  case 111:

    {
	QString str = qString(yyvsp[-1].string);
	yyval.prop_id = getPropertyID( str.lower().latin1(), str.length() );
    ;}
    break;

  case 112:

    { yyval.b = true; ;}
    break;

  case 113:

    { yyval.b = false; ;}
    break;

  case 114:

    {
	yyval.valueList = new ValueList;
	yyval.valueList->addValue( yyvsp[0].value );
    ;}
    break;

  case 115:

    {
	yyval.valueList = yyvsp[-2].valueList;
	if ( yyval.valueList ) {
	    if ( yyvsp[-1].tok ) {
		Value v;
		v.id = 0;
		v.unit = Value::Operator;
		v.iValue = yyvsp[-1].tok;
		yyval.valueList->addValue( v );
	    }
	    yyval.valueList->addValue( yyvsp[0].value );
	}
    ;}
    break;

  case 116:

    {
	yyval.tok = '/';
    ;}
    break;

  case 117:

    {
	yyval.tok = ',';
    ;}
    break;

  case 118:

    {
        yyval.tok = 0;
  ;}
    break;

  case 119:

    { yyval.value = yyvsp[0].value; ;}
    break;

  case 120:

    { yyval.value = yyvsp[0].value; yyval.value.fValue *= yyvsp[-1].val; ;}
    break;

  case 121:

    { yyval.value.id = 0; yyval.value.string = yyvsp[-1].string; yyval.value.unit = CSSPrimitiveValue::CSS_DIMENSION; ;}
    break;

  case 122:

    { yyval.value.id = 0; yyval.value.string = yyvsp[-1].string; yyval.value.unit = CSSPrimitiveValue::CSS_STRING; ;}
    break;

  case 123:

    {
      QString str = qString( yyvsp[-1].string );
      yyval.value.id = getValueID( str.lower().latin1(), str.length() );
      yyval.value.unit = CSSPrimitiveValue::CSS_IDENT;
      yyval.value.string = yyvsp[-1].string;
  ;}
    break;

  case 124:

    { yyval.value.id = 0; yyval.value.string = yyvsp[-1].string; yyval.value.unit = CSSPrimitiveValue::CSS_URI; ;}
    break;

  case 125:

    { yyval.value.id = 0; yyval.value.iValue = 0; yyval.value.unit = CSSPrimitiveValue::CSS_UNKNOWN;/* ### */ ;}
    break;

  case 126:

    { yyval.value.id = 0; yyval.value.string = yyvsp[0].string; yyval.value.unit = CSSPrimitiveValue::CSS_RGBCOLOR; ;}
    break;

  case 127:

    {
      yyval.value = yyvsp[0].value;
  ;}
    break;

  case 128:

    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_NUMBER; ;}
    break;

  case 129:

    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_PERCENTAGE; ;}
    break;

  case 130:

    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_PX; ;}
    break;

  case 131:

    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_CM; ;}
    break;

  case 132:

    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_MM; ;}
    break;

  case 133:

    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_IN; ;}
    break;

  case 134:

    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_PT; ;}
    break;

  case 135:

    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_PC; ;}
    break;

  case 136:

    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_DEG; ;}
    break;

  case 137:

    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_RAD; ;}
    break;

  case 138:

    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_GRAD; ;}
    break;

  case 139:

    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_MS; ;}
    break;

  case 140:

    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_S; ;}
    break;

  case 141:

    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_HZ; ;}
    break;

  case 142:

    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_KHZ; ;}
    break;

  case 143:

    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_EMS; ;}
    break;

  case 144:

    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = Value::Q_EMS; ;}
    break;

  case 145:

    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_EXS; ;}
    break;

  case 146:

    {
      Function *f = new Function;
      f->name = yyvsp[-4].string;
      f->args = yyvsp[-2].valueList;
      yyval.value.id = 0;
      yyval.value.unit = Value::Function;
      yyval.value.function = f;
  ;}
    break;

  case 147:

    {
      Function *f = new Function;
      f->name = yyvsp[-2].string;
      f->args = 0;
      yyval.value.id = 0;
      yyval.value.unit = Value::Function;
      yyval.value.function = f;
  ;}
    break;

  case 148:

    { yyval.string = yyvsp[-1].string; ;}
    break;

  case 149:

    {
	yyval.rule = 0;
#ifdef CSS_DEBUG
	kdDebug( 6080 ) << "skipped invalid @-rule" << endl;
#endif
    ;}
    break;

  case 150:

    {
	yyval.rule = 0;
#ifdef CSS_DEBUG
	kdDebug( 6080 ) << "skipped invalid @-rule" << endl;
#endif
    ;}
    break;

  case 151:

    {
	yyval.rule = 0;
#ifdef CSS_DEBUG
	kdDebug( 6080 ) << "skipped invalid rule" << endl;
#endif
    ;}
    break;


    }

/* Line 999 of yacc.c.  */


  yyvsp -= yylen;
  yyssp -= yylen;


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
	      yydestruct (yystos[*yyssp], yyvsp);
	      YYPOPSTACK;
	    }
	  YYABORT;
        }

      YYDSYMPRINTF ("Error: discarding", yytoken, &yylval, &yylloc);
      yydestruct (yytoken, &yylval);
      yychar = YYEMPTY;

    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*----------------------------------------------------.
| yyerrlab1 -- error raised explicitly by an action.  |
`----------------------------------------------------*/
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

      YYDSYMPRINTF ("Error: popping", yystos[*yyssp], yyvsp, yylsp);
      yydestruct (yystos[yystate], yyvsp);
      yyvsp--;
      yystate = *--yyssp;

      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  YYDPRINTF ((stderr, "Shifting error token, "));

  *++yyvsp = yylval;


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






