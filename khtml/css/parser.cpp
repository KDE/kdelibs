/* A Bison parser, made from parser.y, by GNU bison 1.75.  */

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
#define YYBISON	1

/* Pure parsers.  */
#define YYPURE	1

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
#line 1 "parser.y"


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

#ifndef YYSTYPE
#line 85 "parser.y"
typedef union {
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
    CSSSelector::Match match;
    bool b;
    char tok;
    Value value;
    ValueList *valueList;
} yystype;
/* Line 193 of /usr/share/bison/yacc.c.  */
#line 276 "parser.tab.c"
# define YYSTYPE yystype
# define YYSTYPE_IS_TRIVIAL 1
#endif

#ifndef YYLTYPE
typedef struct yyltype
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
} yyltype;
# define YYLTYPE yyltype
# define YYLTYPE_IS_TRIVIAL 1
#endif

/* Copy the second part of user declarations.  */
#line 107 "parser.y"


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



/* Line 213 of /usr/share/bison/yacc.c.  */
#line 316 "parser.tab.c"

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
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAX (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short) + sizeof (YYSTYPE))				\
      + YYSTACK_GAP_MAX)

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
	    (To)[yyi] = (From)[yyi];	\
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
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAX;	\
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
#define YYLAST   447

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  63
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  57
/* YYNRULES -- Number of rules. */
#define YYNRULES  154
/* YYNRULES -- Number of states. */
#define YYNSTATES  281

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   300

#define YYTRANSLATE(X) \
  ((unsigned)(X) <= YYMAXUTOK ? yytranslate[X] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    60,    17,    55,    54,    57,    15,    61,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    14,    53,
       2,    59,    56,     2,    62,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    16,     2,    58,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    51,    18,    52,     2,     2,     2,     2,
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
     154,   158,   162,   166,   169,   172,   173,   175,   177,   180,
     182,   187,   190,   192,   196,   199,   202,   206,   209,   212,
     214,   216,   219,   222,   224,   226,   228,   231,   234,   236,
     238,   240,   242,   245,   248,   250,   253,   258,   267,   269,
     271,   273,   275,   277,   279,   281,   283,   286,   290,   296,
     301,   306,   311,   317,   323,   327,   331,   336,   341,   347,
     350,   353,   356,   357,   359,   363,   366,   369,   370,   372,
     375,   378,   381,   384,   387,   390,   392,   394,   397,   400,
     403,   406,   409,   412,   415,   418,   421,   424,   427,   430,
     433,   436,   439,   442,   445,   448,   454,   458,   461,   465,
     469,   472,   478,   482,   484
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const yysigned_char yyrhs[] =
{
      64,     0,    -1,    70,    69,    71,    73,    77,    -1,    65,
      68,    -1,    66,    68,    -1,    67,    68,    -1,    25,    51,
      68,    89,    68,    52,    -1,    26,   105,    -1,    27,    51,
      68,   110,    52,    -1,    -1,    68,     4,    -1,    -1,    69,
       5,    -1,    69,     4,    -1,    -1,    23,    68,    11,    68,
      53,    -1,    23,     1,   118,    -1,    23,     1,    53,    -1,
      -1,    71,    72,    69,    -1,    19,    68,    79,    68,    80,
      53,    -1,    19,     1,   118,    -1,    19,     1,    53,    -1,
      -1,    74,    69,    -1,    24,    68,    76,    79,    68,    53,
      -1,    12,    -1,    -1,    75,    68,    -1,    -1,    77,    78,
      69,    -1,    89,    -1,    82,    -1,    85,    -1,    86,    -1,
     117,    -1,   116,    -1,    72,     1,    -1,    11,    -1,    48,
      -1,    -1,    81,    -1,    84,    -1,    81,    54,    68,    84,
      -1,    81,     1,    -1,    21,    68,    81,    51,    68,    83,
      52,    -1,    -1,    83,    89,    68,    -1,    12,    68,    -1,
      20,     1,   118,    -1,    20,     1,    53,    -1,    22,     1,
     118,    -1,    22,     1,    53,    -1,    55,    68,    -1,    56,
      68,    -1,    -1,    57,    -1,    55,    -1,    90,   105,    -1,
      91,    -1,    90,    54,    68,    91,    -1,    90,     1,    -1,
      92,    -1,    91,    87,    92,    -1,    91,     1,    -1,    93,
      68,    -1,    93,    96,    68,    -1,    96,    68,    -1,    94,
      95,    -1,    95,    -1,    18,    -1,    12,    18,    -1,    17,
      18,    -1,    12,    -1,    17,    -1,    97,    -1,    96,    97,
      -1,    96,     1,    -1,    13,    -1,    98,    -1,   101,    -1,
     104,    -1,    15,    12,    -1,    94,   100,    -1,   100,    -1,
      12,    68,    -1,    16,    68,    99,    58,    -1,    16,    68,
      99,   102,    68,   103,    68,    58,    -1,    59,    -1,     6,
      -1,     7,    -1,     8,    -1,     9,    -1,    10,    -1,    12,
      -1,    11,    -1,    14,    12,    -1,    14,    14,    12,    -1,
      14,    49,    68,    92,    60,    -1,    51,    68,   107,    52,
      -1,    51,    68,     1,    52,    -1,    51,    68,   106,    52,
      -1,    51,    68,   106,   107,    52,    -1,    51,    68,   106,
       1,    52,    -1,   107,    53,    68,    -1,     1,    53,    68,
      -1,   106,   107,    53,    68,    -1,   106,     1,    53,    68,
      -1,   108,    14,    68,   110,   109,    -1,     1,   118,    -1,
      12,    68,    -1,    28,    68,    -1,    -1,   112,    -1,   110,
     111,   112,    -1,    61,    68,    -1,    54,    68,    -1,    -1,
     113,    -1,    88,   113,    -1,    45,    68,    -1,    11,    68,
      -1,    12,    68,    -1,    48,    68,    -1,    50,    68,    -1,
     115,    -1,   114,    -1,    47,    68,    -1,    46,    68,    -1,
      32,    68,    -1,    33,    68,    -1,    34,    68,    -1,    35,
      68,    -1,    36,    68,    -1,    37,    68,    -1,    38,    68,
      -1,    39,    68,    -1,    40,    68,    -1,    41,    68,    -1,
      42,    68,    -1,    43,    68,    -1,    44,    68,    -1,    30,
      68,    -1,    29,    68,    -1,    31,    68,    -1,    49,    68,
     110,    60,    68,    -1,    49,    68,     1,    -1,    13,    68,
      -1,    62,     1,   118,    -1,    62,     1,    53,    -1,     1,
     118,    -1,    51,     1,   119,     1,    52,    -1,    51,     1,
      52,    -1,   118,    -1,   119,     1,   118,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   253,   253,   255,   256,   257,   260,   267,   273,   298,
     300,   303,   305,   306,   309,   311,   316,   317,   320,   322,
     332,   343,   346,   352,   353,   357,   361,   364,   366,   369,
     371,   381,   383,   384,   385,   386,   387,   388,   391,   393,
     396,   400,   404,   409,   414,   420,   434,   436,   445,   453,
     457,   462,   466,   471,   473,   474,   477,   479,   482,   502,
     516,   530,   536,   540,   561,   567,   572,   577,   584,   586,
     594,   596,   597,   601,   622,   625,   630,   640,   646,   653,
     654,   655,   658,   667,   669,   675,   699,   705,   713,   717,
     720,   723,   726,   729,   734,   736,   739,   745,   751,   759,
     763,   766,   769,   774,   779,   783,   787,   792,   797,   820,
     825,   832,   834,   837,   842,   857,   861,   864,   869,   871,
     873,   874,   875,   881,   882,   883,   885,   890,   892,   893,
     894,   895,   896,   897,   898,   899,   900,   901,   902,   903,
     904,   905,   906,   907,   908,   912,   921,   932,   939,   946,
     954,   964,   966,   969,   971
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
  "'{'", "'}'", "';'", "','", "'+'", "'>'", "'-'", "']'", "'='", "')'", 
  "'/'", "'@'", "$accept", "stylesheet", "khtml_rule", "khtml_decls", 
  "khtml_value", "maybe_space", "maybe_sgml", "maybe_charset", 
  "import_list", "import", "maybe_namespace", "namespace", "ns_prefix", 
  "maybe_ns_prefix", "rule_list", "rule", "string_or_uri", 
  "maybe_media_list", "media_list", "media", "ruleset_list", "medium", 
  "page", "font_face", "combinator", "unary_operator", "ruleset", 
  "selector_list", "selector", "simple_selector", "ns_element", 
  "ns_selector", "element_name", "specifier_list", "specifier", "class", 
  "ns_attrib_id", "attrib_id", "attrib", "match", "ident_or_string", 
  "pseudo", "declaration_block", "declaration_list", "declaration", 
  "property", "prio", "expr", "operator", "term", "unary_term", 
  "function", "hexcolor", "invalid_at", "invalid_rule", "invalid_block", 
  "invalid_block_list", 0
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
     300,   123,   125,    59,    44,    43,    62,    45,    93,    61,
      41,    47,    64
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    63,    64,    64,    64,    64,    65,    66,    67,    68,
      68,    69,    69,    69,    70,    70,    70,    70,    71,    71,
      72,    72,    72,    73,    73,    74,    75,    76,    76,    77,
      77,    78,    78,    78,    78,    78,    78,    78,    79,    79,
      80,    80,    81,    81,    81,    82,    83,    83,    84,    85,
      85,    86,    86,    87,    87,    87,    88,    88,    89,    90,
      90,    90,    91,    91,    91,    92,    92,    92,    93,    93,
      94,    94,    94,    95,    95,    96,    96,    96,    97,    97,
      97,    97,    98,    99,    99,   100,   101,   101,   102,   102,
     102,   102,   102,   102,   103,   103,   104,   104,   104,   105,
     105,   105,   105,   105,   106,   106,   106,   106,   107,   107,
     108,   109,   109,   110,   110,   111,   111,   111,   112,   112,
     112,   112,   112,   112,   112,   112,   112,   113,   113,   113,
     113,   113,   113,   113,   113,   113,   113,   113,   113,   113,
     113,   113,   113,   113,   113,   114,   114,   115,   116,   116,
     117,   118,   118,   119,   119
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     5,     2,     2,     2,     6,     2,     5,     0,
       2,     0,     2,     2,     0,     5,     3,     3,     0,     3,
       6,     3,     3,     0,     2,     6,     1,     0,     2,     0,
       3,     1,     1,     1,     1,     1,     1,     2,     1,     1,
       0,     1,     1,     4,     2,     7,     0,     3,     2,     3,
       3,     3,     3,     2,     2,     0,     1,     1,     2,     1,
       4,     2,     1,     3,     2,     2,     3,     2,     2,     1,
       1,     2,     2,     1,     1,     1,     2,     2,     1,     1,
       1,     1,     2,     2,     1,     2,     4,     8,     1,     1,
       1,     1,     1,     1,     1,     1,     2,     3,     5,     4,
       4,     4,     5,     5,     3,     3,     4,     4,     5,     2,
       2,     2,     0,     1,     3,     2,     2,     0,     1,     2,
       2,     2,     2,     2,     2,     1,     1,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     5,     3,     2,     3,     3,
       2,     5,     3,     1,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
      14,     0,     0,     0,     0,     0,     9,     9,     9,    11,
       0,     0,     9,     9,     7,     9,     1,     3,     4,     5,
      18,     0,    17,    16,    10,     9,     0,     0,     0,    13,
      12,    23,     0,     0,    73,    78,     0,     0,     9,    74,
      70,     9,     0,     0,    62,     9,     0,    69,     0,    75,
      79,    80,    81,     0,     9,     0,     0,     0,     9,     9,
       9,     9,     9,     9,     9,     9,     9,     9,     9,     9,
       9,     9,     9,     9,     9,     9,     9,     9,     9,     9,
       9,     9,     9,    57,    56,     0,   117,   113,   118,   126,
     125,     0,     9,    11,    29,    11,   152,   153,     0,    15,
      71,    96,     0,     9,    82,     0,    72,     0,    61,     9,
      58,    64,     9,     9,     0,    65,     0,    73,    74,    68,
      77,    67,    76,   100,     9,   109,   110,     0,   101,     0,
      99,     9,     9,   121,   122,   147,   143,   142,   144,   129,
     130,   131,   132,   133,   134,   135,   136,   137,   138,   139,
     140,   141,   120,   128,   127,   123,     0,   124,   119,     8,
       9,     9,     0,     0,     0,    27,    19,     0,    24,     0,
      97,     0,     9,     0,     0,     0,    84,     6,     0,    53,
      54,    63,    66,   105,   103,     9,   102,     9,   104,     0,
     146,   117,   116,   115,   114,    22,    21,    38,    39,     9,
      26,     9,     0,     0,     0,     9,     0,     0,     0,    11,
      32,    33,    34,    31,    36,    35,   151,   154,     0,    85,
       9,    83,    89,    90,    91,    92,    93,    86,    88,     9,
       0,   107,   106,   117,     9,    40,    28,     9,   150,     0,
       0,     0,     0,    37,    30,    98,     0,     9,   108,   145,
       9,     0,     0,    42,     0,    50,    49,     0,    52,    51,
     149,   148,    95,    94,     9,   111,    48,    20,    44,     9,
      25,     9,     0,     0,    46,    87,    43,     0,    45,     9,
      47
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,     5,     6,     7,     8,   219,    20,     9,    31,    93,
      94,    95,   201,   202,   167,   209,   199,   251,   252,   210,
     277,   253,   211,   212,   114,    85,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,   175,   176,    51,   229,
     264,    52,    14,    55,    56,    57,   248,    86,   162,    87,
      88,    89,    90,   214,   215,   125,    98
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -159
static const short yypact[] =
{
     214,    15,   -34,   -18,    -5,    95,  -159,  -159,  -159,  -159,
     127,    94,  -159,  -159,  -159,  -159,  -159,    89,    89,    89,
     251,    98,  -159,  -159,  -159,  -159,   310,     9,   331,  -159,
    -159,   129,    -9,    50,    92,  -159,   112,   115,  -159,   126,
    -159,  -159,    31,   175,  -159,   229,     6,  -159,   151,  -159,
    -159,  -159,  -159,   122,  -159,    44,   206,   133,  -159,  -159,
    -159,  -159,  -159,  -159,  -159,  -159,  -159,  -159,  -159,  -159,
    -159,  -159,  -159,  -159,  -159,  -159,  -159,  -159,  -159,  -159,
    -159,  -159,  -159,  -159,  -159,   400,   -23,  -159,  -159,  -159,
    -159,   103,  -159,  -159,  -159,  -159,  -159,  -159,   153,  -159,
    -159,  -159,   146,  -159,  -159,    18,  -159,    93,  -159,  -159,
    -159,  -159,  -159,  -159,   207,    89,   151,  -159,  -159,  -159,
    -159,    89,  -159,  -159,  -159,  -159,    89,   157,  -159,   208,
    -159,  -159,  -159,    89,    89,    89,    89,    89,    89,    89,
      89,    89,    89,    89,    89,    89,    89,    89,    89,    89,
      89,    89,    89,    89,    89,    89,   253,    89,  -159,  -159,
    -159,  -159,   371,   130,   102,    16,   251,   120,   251,   220,
    -159,   310,    92,   126,   165,    42,  -159,  -159,   310,    89,
      89,  -159,    89,    89,  -159,  -159,  -159,  -159,    89,   331,
    -159,    68,    89,    89,  -159,  -159,  -159,  -159,  -159,  -159,
    -159,  -159,    77,   134,   193,  -159,   202,   226,   237,  -159,
    -159,  -159,  -159,  -159,  -159,  -159,  -159,  -159,   187,    89,
    -159,  -159,  -159,  -159,  -159,  -159,  -159,  -159,  -159,  -159,
     303,    89,    89,   118,  -159,   145,    89,  -159,  -159,   144,
     145,   148,   197,  -159,   251,  -159,   105,  -159,  -159,    89,
    -159,   198,    33,  -159,    90,  -159,  -159,    38,  -159,  -159,
    -159,  -159,  -159,  -159,  -159,    89,    89,  -159,  -159,  -159,
    -159,  -159,    -3,   145,    89,  -159,  -159,   200,  -159,  -159,
      89
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -159,  -159,  -159,  -159,  -159,    -1,   -91,  -159,  -159,    86,
    -159,  -159,  -159,  -159,  -159,  -159,    60,  -159,    27,  -159,
    -159,    -4,  -159,  -159,  -159,  -159,  -158,  -159,    96,   -87,
    -159,   168,   230,   232,   -33,  -159,  -159,   101,  -159,  -159,
    -159,  -159,   238,  -159,   224,  -159,  -159,  -148,  -159,   119,
     221,  -159,  -159,  -159,  -159,    -7,  -159
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, parse error.  */
#define YYTABLE_NINF -113
static const short yytable[] =
{
      11,    24,   166,    23,   168,    17,    18,    19,   191,   213,
      53,    26,    27,    24,    28,   122,    10,    12,   117,    -9,
      24,    54,    24,   118,    33,    97,    -9,   181,   200,   159,
     172,   160,   108,    13,   268,   173,    40,   105,   161,   268,
     107,   233,    21,    96,   115,   127,    15,   121,   222,   223,
     224,   225,   226,   126,    24,   275,    54,   133,   134,   135,
     136,   137,   138,   139,   140,   141,   142,   143,   144,   145,
     146,   147,   148,   149,   150,   151,   152,   153,   154,   155,
     156,   157,    13,   122,   218,   109,   -41,   269,   197,   271,
     164,   165,   269,    24,    24,    16,   128,    24,    24,    32,
     227,   228,   171,    99,   163,    25,    24,    -9,   178,    24,
     100,   179,   180,   197,    -9,   182,   262,   263,   244,   279,
      -2,   203,   160,   183,   101,   198,   102,   104,   234,   161,
     188,   189,    34,    35,    36,    37,    38,    39,    40,    91,
     204,   205,   206,   270,   106,   177,   247,   132,    91,    24,
     198,    -9,   120,    92,   169,    -9,   196,   250,   170,   192,
     193,   103,   217,    -9,    35,    36,    37,    38,    -9,    -9,
    -112,  -112,   160,    21,   123,   124,   111,   220,    21,   161,
      22,    21,   207,   195,   231,    21,   232,   -55,   -55,   -55,
     -55,   -55,   -55,   -55,   239,    21,   238,   255,   235,    21,
     236,   258,    -9,   241,   240,    -9,    -9,    -9,    21,   184,
     185,    -9,    34,    35,    36,    37,    38,    39,    40,    34,
      35,    36,    37,    38,    39,    40,   -59,   242,   246,   -59,
     112,   113,   256,   249,   259,   261,   254,     1,   243,     2,
       3,     4,    35,    36,    37,    38,   265,   245,    21,   266,
     260,   267,   278,   208,   190,    29,    30,    24,   130,   131,
     186,   187,   237,   272,    58,    59,    60,   257,   273,   276,
     274,    21,   216,   174,   230,   221,   119,   116,   280,   129,
     110,   194,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,   111,     0,   158,     0,    83,     0,
      84,     0,     0,     0,    24,   -55,   -55,   -55,   -55,   -55,
     -55,   -55,    34,    35,    36,    37,    38,    39,    40,     0,
       0,     0,     0,     0,     0,    24,     0,     0,     0,     0,
       0,     0,    58,    59,    60,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   -60,     0,     0,   -60,   112,   113,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    58,    59,    60,     0,    83,     0,    84,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,     0,     0,     0,     0,    83,     0,    84,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,     0,    78,    79
};

static const short yycheck[] =
{
       1,     4,    93,    10,    95,     6,     7,     8,   156,   167,
       1,    12,    13,     4,    15,    48,     1,    51,    12,     4,
       4,    12,     4,    17,    25,    32,    11,   114,    12,    52,
      12,    54,     1,    51,     1,    17,    18,    38,    61,     1,
      41,   189,    51,    52,    45,     1,    51,    48,     6,     7,
       8,     9,    10,    54,     4,    58,    12,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    51,   116,   171,    54,    53,    54,    11,    51,
      91,    92,    54,     4,     4,     0,    52,     4,     4,     1,
      58,    59,   103,    53,     1,    11,     4,     4,   109,     4,
      18,   112,   113,    11,    11,   116,    11,    12,   209,   277,
       0,     1,    54,   124,    12,    48,    14,    12,    60,    61,
     131,   132,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    53,    18,    52,    28,    14,    19,     4,
      48,    48,     1,    24,     1,     4,   163,    12,    12,   160,
     161,    49,   169,    12,    13,    14,    15,    16,    17,    18,
      52,    53,    54,    51,    52,    53,     1,    12,    51,    61,
      53,    51,    62,    53,   185,    51,   187,    12,    13,    14,
      15,    16,    17,    18,     1,    51,   203,    53,   199,    51,
     201,    53,    51,     1,   205,    54,    55,    56,    51,    52,
      53,    60,    12,    13,    14,    15,    16,    17,    18,    12,
      13,    14,    15,    16,    17,    18,    51,     1,   229,    54,
      55,    56,   239,   234,   241,   242,   237,    23,     1,    25,
      26,    27,    13,    14,    15,    16,   247,    60,    51,   250,
      53,    53,    52,   167,     1,     4,     5,     4,    52,    53,
      52,    53,   202,   264,    11,    12,    13,   240,   269,   273,
     271,    51,    52,   105,   178,   174,    46,    45,   279,    55,
      42,   162,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,     1,    -1,    85,    -1,    55,    -1,
      57,    -1,    -1,    -1,     4,    12,    13,    14,    15,    16,
      17,    18,    12,    13,    14,    15,    16,    17,    18,    -1,
      -1,    -1,    -1,    -1,    -1,     4,    -1,    -1,    -1,    -1,
      -1,    -1,    11,    12,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    51,    -1,    -1,    54,    55,    56,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    11,    12,    13,    -1,    55,    -1,    57,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    -1,    -1,    -1,    -1,    55,    -1,    57,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    -1,    46,    47
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,    23,    25,    26,    27,    64,    65,    66,    67,    70,
       1,    68,    51,    51,   105,    51,     0,    68,    68,    68,
      69,    51,    53,   118,     4,    11,    68,    68,    68,     4,
       5,    71,     1,    68,    12,    13,    14,    15,    16,    17,
      18,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,   101,   104,     1,    12,   106,   107,   108,    11,    12,
      13,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    55,    57,    88,   110,   112,   113,   114,
     115,    19,    24,    72,    73,    74,    52,   118,   119,    53,
      18,    12,    14,    49,    12,    68,    18,    68,     1,    54,
     105,     1,    55,    56,    87,    68,    96,    12,    17,    95,
       1,    68,    97,    52,    53,   118,    68,     1,    52,   107,
      52,    53,    14,    68,    68,    68,    68,    68,    68,    68,
      68,    68,    68,    68,    68,    68,    68,    68,    68,    68,
      68,    68,    68,    68,    68,    68,    68,    68,   113,    52,
      54,    61,   111,     1,    68,    68,    69,    77,    69,     1,
      12,    68,    12,    17,    94,    99,   100,    52,    68,    68,
      68,    92,    68,    68,    52,    53,    52,    53,    68,    68,
       1,   110,    68,    68,   112,    53,   118,    11,    48,    79,
      12,    75,    76,     1,    20,    21,    22,    62,    72,    78,
      82,    85,    86,    89,   116,   117,    52,   118,    92,    68,
      12,   100,     6,     7,     8,     9,    10,    58,    59,   102,
      91,    68,    68,   110,    60,    68,    68,    79,   118,     1,
      68,     1,     1,     1,    69,    60,    68,    28,   109,    68,
      12,    80,    81,    84,    68,    53,   118,    81,    53,   118,
      53,   118,    11,    12,   103,    68,    68,    53,     1,    54,
      53,    51,    68,    68,    68,    58,    84,    83,    52,    89,
      68
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
#define YYEMPTY		-2
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
      yychar1 = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { 								\
      yyerror ("syntax error: cannot back up");			\
      YYERROR;							\
    }								\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

/* YYLLOC_DEFAULT -- Compute the default location (before the actions
   are run).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)           \
  Current.first_line   = Rhs[1].first_line;      \
  Current.first_column = Rhs[1].first_column;    \
  Current.last_line    = Rhs[N].last_line;       \
  Current.last_column  = Rhs[N].last_column;
#endif

/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX	yylex (&yylval, YYLEX_PARAM)
#else
# define YYLEX	yylex (&yylval)
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
/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YYDSYMPRINT(Args)
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
/*-----------------------------.
| Print this symbol on YYOUT.  |
`-----------------------------*/

static void
#if defined (__STDC__) || defined (__cplusplus)
yysymprint (FILE* yyout, int yytype, YYSTYPE yyvalue)
#else
yysymprint (yyout, yytype, yyvalue)
    FILE* yyout;
    int yytype;
    YYSTYPE yyvalue;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvalue;

  if (yytype < YYNTOKENS)
    {
      YYFPRINTF (yyout, "token %s (", yytname[yytype]);
# ifdef YYPRINT
      YYPRINT (yyout, yytoknum[yytype], yyvalue);
# endif
    }
  else
    YYFPRINTF (yyout, "nterm %s (", yytname[yytype]);

  switch (yytype)
    {
      case 80: /* maybe_media_list */
#line 0 "parser.y"
        (null);
#line 1094 "parser.tab.c"
        break;
      case 81: /* media_list */
#line 0 "parser.y"
        (null);
#line 1099 "parser.tab.c"
        break;
      case 83: /* ruleset_list */
#line 0 "parser.y"
        (null);
#line 1104 "parser.tab.c"
        break;
      case 90: /* selector_list */
#line 0 "parser.y"
        (null);
#line 1109 "parser.tab.c"
        break;
      case 91: /* selector */
#line 0 "parser.y"
        (null);
#line 1114 "parser.tab.c"
        break;
      case 92: /* simple_selector */
#line 0 "parser.y"
        (null);
#line 1119 "parser.tab.c"
        break;
      case 96: /* specifier_list */
#line 0 "parser.y"
        (null);
#line 1124 "parser.tab.c"
        break;
      case 97: /* specifier */
#line 0 "parser.y"
        (null);
#line 1129 "parser.tab.c"
        break;
      case 98: /* class */
#line 0 "parser.y"
        (null);
#line 1134 "parser.tab.c"
        break;
      case 101: /* attrib */
#line 0 "parser.y"
        (null);
#line 1139 "parser.tab.c"
        break;
      case 104: /* pseudo */
#line 0 "parser.y"
        (null);
#line 1144 "parser.tab.c"
        break;
      case 110: /* expr */
#line 0 "parser.y"
        (null);
#line 1149 "parser.tab.c"
        break;
      default:
        break;
    }
  YYFPRINTF (yyout, ")");
}
#endif /* YYDEBUG. */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
#if defined (__STDC__) || defined (__cplusplus)
yydestruct (int yytype, YYSTYPE yyvalue)
#else
yydestruct (yytype, yyvalue)
    int yytype;
    YYSTYPE yyvalue;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvalue;

  switch (yytype)
    {
      case 80: /* maybe_media_list */
#line 128 "parser.y"
        { delete yyvalue.mediaList; yyvalue.mediaList = 0; };
#line 1180 "parser.tab.c"
        break;
      case 81: /* media_list */
#line 128 "parser.y"
        { delete yyvalue.mediaList; yyvalue.mediaList = 0; };
#line 1185 "parser.tab.c"
        break;
      case 83: /* ruleset_list */
#line 130 "parser.y"
        { delete yyvalue.ruleList; yyvalue.ruleList = 0; };
#line 1190 "parser.tab.c"
        break;
      case 90: /* selector_list */
#line 129 "parser.y"
        { delete yyvalue.selectorList; yyvalue.selectorList = 0; };
#line 1195 "parser.tab.c"
        break;
      case 91: /* selector */
#line 131 "parser.y"
        { delete yyvalue.selector; yyvalue.selector = 0; };
#line 1200 "parser.tab.c"
        break;
      case 92: /* simple_selector */
#line 131 "parser.y"
        { delete yyvalue.selector; yyvalue.selector = 0; };
#line 1205 "parser.tab.c"
        break;
      case 96: /* specifier_list */
#line 131 "parser.y"
        { delete yyvalue.selector; yyvalue.selector = 0; };
#line 1210 "parser.tab.c"
        break;
      case 97: /* specifier */
#line 131 "parser.y"
        { delete yyvalue.selector; yyvalue.selector = 0; };
#line 1215 "parser.tab.c"
        break;
      case 98: /* class */
#line 131 "parser.y"
        { delete yyvalue.selector; yyvalue.selector = 0; };
#line 1220 "parser.tab.c"
        break;
      case 101: /* attrib */
#line 131 "parser.y"
        { delete yyvalue.selector; yyvalue.selector = 0; };
#line 1225 "parser.tab.c"
        break;
      case 104: /* pseudo */
#line 131 "parser.y"
        { delete yyvalue.selector; yyvalue.selector = 0; };
#line 1230 "parser.tab.c"
        break;
      case 110: /* expr */
#line 127 "parser.y"
        { delete yyvalue.valueList; yyvalue.valueList = 0; };
#line 1235 "parser.tab.c"
        break;
      default:
        break;
    }
}



/* The user can define YYPARSE_PARAM as the name of an argument to be passed
   into yyparse.  The argument should have type void *.
   It should actually point to an object.
   Grammar actions can access the variable by casting it
   to the proper pointer type.  */

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
#  define YYPARSE_PARAM_ARG void *YYPARSE_PARAM
#  define YYPARSE_PARAM_DECL
# else
#  define YYPARSE_PARAM_ARG YYPARSE_PARAM
#  define YYPARSE_PARAM_DECL void *YYPARSE_PARAM;
# endif
#else /* !YYPARSE_PARAM */
# define YYPARSE_PARAM_ARG
# define YYPARSE_PARAM_DECL
#endif /* !YYPARSE_PARAM */

/* Prevent warning if -Wstrict-prototypes.  */
#ifdef __GNUC__
# ifdef YYPARSE_PARAM
int yyparse (void *);
# else
int yyparse (void);
# endif
#endif




int
yyparse (YYPARSE_PARAM_ARG)
     YYPARSE_PARAM_DECL
{
  /* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of parse errors so far.  */
int yynerrs;

  register int yystate;
  register int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Lookahead token as an internal (translated) token number.  */
  int yychar1 = 0;

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

  if (yyssp >= yyss + yystacksize - 1)
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
      if (yystacksize >= YYMAXDEPTH)
	goto yyoverflowlab;
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
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

      if (yyssp >= yyss + yystacksize - 1)
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

  /* yychar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with.  */

  if (yychar <= 0)		/* This means end of input.  */
    {
      yychar1 = 0;
      yychar = YYEOF;		/* Don't call YYLEX any more.  */

      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yychar1 = YYTRANSLATE (yychar);

      /* We have to keep this `#if YYDEBUG', since we use variables
	 which are defined only if `YYDEBUG' is set.  */
      YYDPRINTF ((stderr, "Next token is "));
      YYDSYMPRINT ((stderr, yychar1, yylval));
      YYDPRINTF ((stderr, "\n"));
    }

  /* If the proper action on seeing token YYCHAR1 is to reduce or to
     detect an error, take that action.  */
  yyn += yychar1;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yychar1)
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
  YYDPRINTF ((stderr, "Shifting token %d (%s), ",
	      yychar, yytname[yychar1]));

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



#if YYDEBUG
  /* We have to keep this `#if YYDEBUG', since we use variables which
     are defined only if `YYDEBUG' is set.  */
  if (yydebug)
    {
      int yyi;

      YYFPRINTF (stderr, "Reducing via rule %d (line %d), ",
		 yyn - 1, yyrline[yyn]);

      /* Print the symbols being reduced, and their result.  */
      for (yyi = yyprhs[yyn]; yyrhs[yyi] >= 0; yyi++)
	YYFPRINTF (stderr, "%s ", yytname[yyrhs[yyi]]);
      YYFPRINTF (stderr, " -> %s\n", yytname[yyr1[yyn]]);
    }
#endif
  switch (yyn)
    {
        case 6:
    {
        CSSParser *p = static_cast<CSSParser *>(parser);
	p->rule = yyvsp[-2].rule;
    }
    break;

  case 7:
    {
	/* can be empty */
    }
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
    }
    break;

  case 15:
    {
#ifdef CSS_DEBUG
     kdDebug( 6080 ) << "charset rule: " << qString(yyvsp[-2].string) << endl;
#endif
 }
    break;

  case 19:
    {
     CSSParser *p = static_cast<CSSParser *>(parser);
     if ( yyvsp[-1].rule && p->styleElement && p->styleElement->isCSSStyleSheet() ) {
	 p->styleElement->append( yyvsp[-1].rule );
     } else {
	 delete yyvsp[-1].rule;
     }
 }
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
    }
    break;

  case 21:
    {
        yyval.rule = 0;
    }
    break;

  case 22:
    {
        yyval.rule = 0;
    }
    break;

  case 27:
    { yyval.string.string = 0; yyval.string.length = 0; }
    break;

  case 30:
    {
     CSSParser *p = static_cast<CSSParser *>(parser);
     if ( yyvsp[-1].rule && p->styleElement && p->styleElement->isCSSStyleSheet() ) {
	 p->styleElement->append( yyvsp[-1].rule );
     } else {
	 delete yyvsp[-1].rule;
     }
 }
    break;

  case 37:
    { delete yyvsp[-1].rule; yyval.rule = 0; }
    break;

  case 40:
    {
	yyval.mediaList = new MediaListImpl();
    }
    break;

  case 42:
    {
	yyval.mediaList = new MediaListImpl();
	yyval.mediaList->appendMedium( domString(yyvsp[0].string).lower() );
    }
    break;

  case 43:
    {
	yyval.mediaList = yyvsp[-3].mediaList;
	if (yyval.mediaList)
	    yyval.mediaList->appendMedium( domString(yyvsp[0].string).lower() );
    }
    break;

  case 44:
    {
       delete yyvsp[-1].mediaList;
       yyval.mediaList = 0;
    }
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
    }
    break;

  case 46:
    { yyval.ruleList = 0; }
    break;

  case 47:
    {
      yyval.ruleList = yyvsp[-2].ruleList;
      if ( yyvsp[-1].rule ) {
	  if ( !yyval.ruleList ) yyval.ruleList = new CSSRuleListImpl();
	  yyval.ruleList->append( yyvsp[-1].rule );
      }
  }
    break;

  case 48:
    {
      yyval.string = yyvsp[-1].string;
  }
    break;

  case 49:
    {
      yyval.rule = 0;
    }
    break;

  case 50:
    {
      yyval.rule = 0;
    }
    break;

  case 51:
    {
      yyval.rule = 0;
    }
    break;

  case 52:
    {
      yyval.rule = 0;
    }
    break;

  case 53:
    { yyval.relation = CSSSelector::Sibling; }
    break;

  case 54:
    { yyval.relation = CSSSelector::Child; }
    break;

  case 55:
    { yyval.relation = CSSSelector::Descendant; }
    break;

  case 56:
    { yyval.val = -1; }
    break;

  case 57:
    { yyval.val = 1; }
    break;

  case 58:
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
    }
    break;

  case 59:
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
    }
    break;

  case 60:
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
    }
    break;

  case 61:
    {
	delete yyvsp[-1].selectorList;
	yyval.selectorList = 0;
    }
    break;

  case 62:
    {
	yyval.selector = yyvsp[0].selector;
    }
    break;

  case 63:
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
    }
    break;

  case 64:
    {
	delete yyvsp[-1].selector;
	yyval.selector = 0;
    }
    break;

  case 65:
    {
	yyval.selector = new CSSSelector();
	yyval.selector->tag = yyvsp[-1].element;
    }
    break;

  case 66:
    {
	yyval.selector = yyvsp[-1].selector;
        if ( yyval.selector )
	    yyval.selector->tag = yyvsp[-2].element;
    }
    break;

  case 67:
    {
	yyval.selector = yyvsp[-1].selector;
        if ( yyval.selector )
	    yyval.selector->tag = 0xffffffff;
    }
    break;

  case 68:
    { yyval.element = (yyvsp[-1].ns<<16) | yyvsp[0].element; }
    break;

  case 69:
    {
        /* according to the specs this one matches all namespaces if no
	   default namespace has been specified otherwise the default namespace */
	CSSParser *p = static_cast<CSSParser *>(parser);
	yyval.element = (p->defaultNamespace<<16) | yyvsp[0].element;
    }
    break;

  case 70:
    { yyval.ns = 0; }
    break;

  case 71:
    { yyval.ns = 1; /* #### insert correct namespace id here */ }
    break;

  case 72:
    { yyval.ns = 0xffff; }
    break;

  case 73:
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
    }
    break;

  case 74:
    { yyval.element = 0xffff; }
    break;

  case 75:
    {
	yyval.selector = yyvsp[0].selector;
	yyval.selector->nonCSSHint = static_cast<CSSParser *>(parser)->nonCSSHint;
    }
    break;

  case 76:
    {
	yyval.selector = yyvsp[-1].selector;
	if ( yyval.selector ) {
            CSSSelector *end = yyvsp[-1].selector;
            while( end->tagHistory )
                end = end->tagHistory;
            end->relation = CSSSelector::SubSelector;
            end->tagHistory = yyvsp[0].selector;
	}
    }
    break;

  case 77:
    {
	delete yyvsp[-1].selector;
	yyval.selector = 0;
    }
    break;

  case 78:
    {
	yyval.selector = new CSSSelector();
	yyval.selector->match = CSSSelector::Id;
	yyval.selector->attr = ATTR_ID;
	yyval.selector->value = domString(yyvsp[0].string);
    }
    break;

  case 82:
    {
	yyval.selector = new CSSSelector();
	yyval.selector->match = CSSSelector::List;
	yyval.selector->attr = ATTR_CLASS;
	yyval.selector->value = domString(yyvsp[0].string);
    }
    break;

  case 83:
    { yyval.attribute = (yyvsp[-1].ns<<16) | yyvsp[0].attribute; }
    break;

  case 84:
    {
	/* opposed to elements, these only match for non namespaced attributes */
	yyval.attribute = yyvsp[0].attribute;
    }
    break;

  case 85:
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
    }
    break;

  case 86:
    {
	yyval.selector = new CSSSelector();
	yyval.selector->attr = yyvsp[-1].attribute;
	yyval.selector->match = CSSSelector::Set;
    }
    break;

  case 87:
    {
	yyval.selector = new CSSSelector();
	yyval.selector->attr = yyvsp[-5].attribute;
	yyval.selector->match = yyvsp[-4].match;
	yyval.selector->value = domString(yyvsp[-2].string);
    }
    break;

  case 88:
    {
	yyval.match = CSSSelector::Exact;
    }
    break;

  case 89:
    {
	yyval.match = CSSSelector::List;
    }
    break;

  case 90:
    {
	yyval.match = CSSSelector::Hyphen;
    }
    break;

  case 91:
    {
	yyval.match = CSSSelector::Begin;
    }
    break;

  case 92:
    {
	yyval.match = CSSSelector::End;
    }
    break;

  case 93:
    {
	yyval.match = CSSSelector::Contain;
    }
    break;

  case 96:
    {
	yyval.selector = new CSSSelector();
	yyval.selector->match = CSSSelector::Pseudo;
	yyval.selector->value = domString(yyvsp[0].string);
    }
    break;

  case 97:
    {
	yyval.selector = new CSSSelector();
	yyval.selector->match = CSSSelector::Pseudo;
        yyval.selector->value = domString(yyvsp[0].string);
    }
    break;

  case 98:
    {
        yyval.selector = new CSSSelector();
        yyval.selector->match = CSSSelector::Pseudo;
        yyval.selector->simpleSelector = yyvsp[-1].selector;
        yyval.selector->value = domString(yyvsp[-3].string);
    }
    break;

  case 99:
    {
	yyval.ok = yyvsp[-1].ok;
    }
    break;

  case 100:
    {
	yyval.ok = false;
    }
    break;

  case 101:
    {
	yyval.ok = yyvsp[-1].ok;
    }
    break;

  case 102:
    {
	yyval.ok = yyvsp[-2].ok;
	if ( yyvsp[-1].ok )
	    yyval.ok = yyvsp[-1].ok;
    }
    break;

  case 103:
    {
	yyval.ok = yyvsp[-2].ok;
    }
    break;

  case 104:
    {
	yyval.ok = yyvsp[-2].ok;
    }
    break;

  case 105:
    {
        yyval.ok = false;
    }
    break;

  case 106:
    {
	yyval.ok = yyvsp[-3].ok;
	if ( yyvsp[-2].ok )
	    yyval.ok = yyvsp[-2].ok;
    }
    break;

  case 107:
    {
        yyval.ok = yyvsp[-3].ok;
    }
    break;

  case 108:
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
    }
    break;

  case 109:
    {
        yyval.ok = false;
    }
    break;

  case 110:
    {
	QString str = qString(yyvsp[-1].string);
	yyval.prop_id = getPropertyID( str.lower().latin1(), str.length() );
    }
    break;

  case 111:
    { yyval.b = true; }
    break;

  case 112:
    { yyval.b = false; }
    break;

  case 113:
    {
	yyval.valueList = new ValueList;
	yyval.valueList->addValue( yyvsp[0].value );
    }
    break;

  case 114:
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
    }
    break;

  case 115:
    {
	yyval.tok = '/';
    }
    break;

  case 116:
    {
	yyval.tok = ',';
    }
    break;

  case 117:
    {
        yyval.tok = 0;
  }
    break;

  case 118:
    { yyval.value = yyvsp[0].value; }
    break;

  case 119:
    { yyval.value = yyvsp[0].value; yyval.value.fValue *= yyvsp[-1].val; }
    break;

  case 120:
    { yyval.value.id = 0; yyval.value.string = yyvsp[-1].string; yyval.value.unit = CSSPrimitiveValue::CSS_DIMENSION; }
    break;

  case 121:
    { yyval.value.id = 0; yyval.value.string = yyvsp[-1].string; yyval.value.unit = CSSPrimitiveValue::CSS_STRING; }
    break;

  case 122:
    {
      QString str = qString( yyvsp[-1].string );
      yyval.value.id = getValueID( str.lower().latin1(), str.length() );
      yyval.value.unit = CSSPrimitiveValue::CSS_IDENT;
      yyval.value.string = yyvsp[-1].string;
  }
    break;

  case 123:
    { yyval.value.id = 0; yyval.value.string = yyvsp[-1].string; yyval.value.unit = CSSPrimitiveValue::CSS_URI; }
    break;

  case 124:
    { yyval.value.id = 0; yyval.value.iValue = 0; yyval.value.unit = CSSPrimitiveValue::CSS_UNKNOWN;/* ### */ }
    break;

  case 125:
    { yyval.value.id = 0; yyval.value.string = yyvsp[0].string; yyval.value.unit = CSSPrimitiveValue::CSS_RGBCOLOR; }
    break;

  case 126:
    {
      yyval.value = yyvsp[0].value;
  }
    break;

  case 127:
    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_NUMBER; }
    break;

  case 128:
    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_PERCENTAGE; }
    break;

  case 129:
    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_PX; }
    break;

  case 130:
    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_CM; }
    break;

  case 131:
    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_MM; }
    break;

  case 132:
    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_IN; }
    break;

  case 133:
    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_PT; }
    break;

  case 134:
    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_PC; }
    break;

  case 135:
    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_DEG; }
    break;

  case 136:
    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_RAD; }
    break;

  case 137:
    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_GRAD; }
    break;

  case 138:
    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_MS; }
    break;

  case 139:
    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_S; }
    break;

  case 140:
    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_HZ; }
    break;

  case 141:
    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_KHZ; }
    break;

  case 142:
    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_EMS; }
    break;

  case 143:
    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = Value::Q_EMS; }
    break;

  case 144:
    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_EXS; }
    break;

  case 145:
    {
      Function *f = new Function;
      f->name = yyvsp[-4].string;
      f->args = yyvsp[-2].valueList;
      yyval.value.id = 0;
      yyval.value.unit = Value::Function;
      yyval.value.function = f;
  }
    break;

  case 146:
    {
      Function *f = new Function;
      f->name = yyvsp[-2].string;
      f->args = 0;
      yyval.value.id = 0;
      yyval.value.unit = Value::Function;
      yyval.value.function = f;
  }
    break;

  case 147:
    { yyval.string = yyvsp[-1].string; }
    break;

  case 148:
    {
	yyval.rule = 0;
#ifdef CSS_DEBUG
	kdDebug( 6080 ) << "skipped invalid @-rule" << endl;
#endif
    }
    break;

  case 149:
    {
	yyval.rule = 0;
#ifdef CSS_DEBUG
	kdDebug( 6080 ) << "skipped invalid @-rule" << endl;
#endif
    }
    break;

  case 150:
    {
	yyval.rule = 0;
#ifdef CSS_DEBUG
	kdDebug( 6080 ) << "skipped invalid rule" << endl;
#endif
    }
    break;


    }

/* Line 1016 of /usr/share/bison/yacc.c.  */
#line 2411 "parser.tab.c"

  yyvsp -= yylen;
  yyssp -= yylen;


#if YYDEBUG
  if (yydebug)
    {
      short *yyssp1 = yyss - 1;
      YYFPRINTF (stderr, "state stack now");
      while (yyssp1 != yyssp)
	YYFPRINTF (stderr, " %d", *++yyssp1);
      YYFPRINTF (stderr, "\n");
    }
#endif

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
	  yysize += yystrlen ("parse error, unexpected ") + 1;
	  yysize += yystrlen (yytname[yytype]);
	  yymsg = (char *) YYSTACK_ALLOC (yysize);
	  if (yymsg != 0)
	    {
	      char *yyp = yystpcpy (yymsg, "parse error, unexpected ");
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
	    yyerror ("parse error; also virtual memory exhausted");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror ("parse error");
    }
  goto yyerrlab1;


/*----------------------------------------------------.
| yyerrlab1 -- error raised explicitly by an action.  |
`----------------------------------------------------*/
yyerrlab1:
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
	  while (yyssp > yyss)
	    {
	      YYDPRINTF ((stderr, "Error: popping "));
	      YYDSYMPRINT ((stderr,
			    yystos[*yyssp],
			    *yyvsp));
	      YYDPRINTF ((stderr, "\n"));
	      yydestruct (yystos[*yyssp], *yyvsp);
	      YYPOPSTACK;
	    }
	  YYABORT;
        }

      YYDPRINTF ((stderr, "Discarding token %d (%s).\n",
		  yychar, yytname[yychar1]));
      yydestruct (yychar1, yylval);
      yychar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */

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

      YYDPRINTF ((stderr, "Error: popping "));
      YYDSYMPRINT ((stderr,
		    yystos[*yyssp], *yyvsp));
      YYDPRINTF ((stderr, "\n"));

      yydestruct (yystos[yystate], *yyvsp);
      yyvsp--;
      yystate = *--yyssp;


#if YYDEBUG
      if (yydebug)
	{
	  short *yyssp1 = yyss - 1;
	  YYFPRINTF (stderr, "Error: state stack now");
	  while (yyssp1 != yyssp)
	    YYFPRINTF (stderr, " %d", *++yyssp1);
	  YYFPRINTF (stderr, "\n");
	}
#endif
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





