/* A Bison parser, made by GNU Bison 1.875.  */

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
     S = 258,
     SGML_CD = 259,
     INCLUDES = 260,
     DASHMATCH = 261,
     BEGINSWITH = 262,
     ENDSWITH = 263,
     CONTAINS = 264,
     STRING = 265,
     IDENT = 266,
     HASH = 267,
     IMPORT_SYM = 268,
     PAGE_SYM = 269,
     MEDIA_SYM = 270,
     FONT_FACE_SYM = 271,
     CHARSET_SYM = 272,
     NAMESPACE_SYM = 273,
     KHTML_RULE_SYM = 274,
     KHTML_DECLS_SYM = 275,
     KHTML_VALUE_SYM = 276,
     IMPORTANT_SYM = 277,
     QEMS = 278,
     EMS = 279,
     EXS = 280,
     PXS = 281,
     CMS = 282,
     MMS = 283,
     INS = 284,
     PTS = 285,
     PCS = 286,
     DEGS = 287,
     RADS = 288,
     GRADS = 289,
     MSECS = 290,
     SECS = 291,
     HERZ = 292,
     KHERZ = 293,
     DIMEN = 294,
     PERCENTAGE = 295,
     NUMBER = 296,
     URI = 297,
     FUNCTION = 298,
     UNICODERANGE = 299
   };
#endif
#define S 258
#define SGML_CD 259
#define INCLUDES 260
#define DASHMATCH 261
#define BEGINSWITH 262
#define ENDSWITH 263
#define CONTAINS 264
#define STRING 265
#define IDENT 266
#define HASH 267
#define IMPORT_SYM 268
#define PAGE_SYM 269
#define MEDIA_SYM 270
#define FONT_FACE_SYM 271
#define CHARSET_SYM 272
#define NAMESPACE_SYM 273
#define KHTML_RULE_SYM 274
#define KHTML_DECLS_SYM 275
#define KHTML_VALUE_SYM 276
#define IMPORTANT_SYM 277
#define QEMS 278
#define EMS 279
#define EXS 280
#define PXS 281
#define CMS 282
#define MMS 283
#define INS 284
#define PTS 285
#define PCS 286
#define DEGS 287
#define RADS 288
#define GRADS 289
#define MSECS 290
#define SECS 291
#define HERZ 292
#define KHERZ 293
#define DIMEN 294
#define PERCENTAGE 295
#define NUMBER 296
#define URI 297
#define FUNCTION 298
#define UNICODERANGE 299




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
#define YYLAST   416

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  62
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  57
/* YYNRULES -- Number of rules. */
#define YYNRULES  154
/* YYNRULES -- Number of states. */
#define YYNSTATES  281

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   299

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    59,    16,    54,    53,    56,    14,    60,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    13,    52,
       2,    58,    55,     2,    61,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    15,     2,    57,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    50,    17,    51,     2,     2,     2,     2,
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
       5,     6,     7,     8,     9,    10,    11,    12,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49
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
      63,     0,    -1,    69,    68,    70,    72,    76,    -1,    64,
      67,    -1,    65,    67,    -1,    66,    67,    -1,    24,    50,
      67,    88,    67,    51,    -1,    25,   104,    -1,    26,    50,
      67,   109,    51,    -1,    -1,    67,     3,    -1,    -1,    68,
       4,    -1,    68,     3,    -1,    -1,    22,    67,    10,    67,
      52,    -1,    22,     1,   117,    -1,    22,     1,    52,    -1,
      -1,    70,    71,    68,    -1,    18,    67,    78,    67,    79,
      52,    -1,    18,     1,   117,    -1,    18,     1,    52,    -1,
      -1,    73,    68,    -1,    23,    67,    75,    78,    67,    52,
      -1,    11,    -1,    -1,    74,    67,    -1,    -1,    76,    77,
      68,    -1,    88,    -1,    81,    -1,    84,    -1,    85,    -1,
     116,    -1,   115,    -1,    71,     1,    -1,    10,    -1,    47,
      -1,    -1,    80,    -1,    83,    -1,    80,    53,    67,    83,
      -1,    80,     1,    -1,    20,    67,    80,    50,    67,    82,
      51,    -1,    -1,    82,    88,    67,    -1,    11,    67,    -1,
      19,     1,   117,    -1,    19,     1,    52,    -1,    21,     1,
     117,    -1,    21,     1,    52,    -1,    54,    67,    -1,    55,
      67,    -1,    -1,    56,    -1,    54,    -1,    89,   104,    -1,
      90,    -1,    89,    53,    67,    90,    -1,    89,     1,    -1,
      91,    -1,    90,    86,    91,    -1,    90,     1,    -1,    92,
      67,    -1,    92,    95,    67,    -1,    95,    67,    -1,    93,
      94,    -1,    94,    -1,    17,    -1,    11,    17,    -1,    16,
      17,    -1,    11,    -1,    16,    -1,    96,    -1,    95,    96,
      -1,    95,     1,    -1,    12,    -1,    97,    -1,   100,    -1,
     103,    -1,    14,    11,    -1,    93,    99,    -1,    99,    -1,
      11,    67,    -1,    15,    67,    98,    57,    -1,    15,    67,
      98,   101,    67,   102,    67,    57,    -1,    58,    -1,     5,
      -1,     6,    -1,     7,    -1,     8,    -1,     9,    -1,    11,
      -1,    10,    -1,    13,    11,    -1,    13,    13,    11,    -1,
      13,    48,    67,    91,    59,    -1,    50,    67,   106,    51,
      -1,    50,    67,     1,    51,    -1,    50,    67,   105,    51,
      -1,    50,    67,   105,   106,    51,    -1,    50,    67,   105,
       1,    51,    -1,   106,    52,    67,    -1,     1,    52,    67,
      -1,   105,   106,    52,    67,    -1,   105,     1,    52,    67,
      -1,   107,    13,    67,   109,   108,    -1,     1,   117,    -1,
      11,    67,    -1,    27,    67,    -1,    -1,   111,    -1,   109,
     110,   111,    -1,    60,    67,    -1,    53,    67,    -1,    -1,
     112,    -1,    87,   112,    -1,    44,    67,    -1,    10,    67,
      -1,    11,    67,    -1,    47,    67,    -1,    49,    67,    -1,
     114,    -1,   113,    -1,    46,    67,    -1,    45,    67,    -1,
      31,    67,    -1,    32,    67,    -1,    33,    67,    -1,    34,
      67,    -1,    35,    67,    -1,    36,    67,    -1,    37,    67,
      -1,    38,    67,    -1,    39,    67,    -1,    40,    67,    -1,
      41,    67,    -1,    42,    67,    -1,    43,    67,    -1,    29,
      67,    -1,    28,    67,    -1,    30,    67,    -1,    48,    67,
     109,    59,    67,    -1,    48,    67,     1,    -1,    12,    67,
      -1,    61,     1,   117,    -1,    61,     1,    52,    -1,     1,
     117,    -1,    50,     1,   118,     1,    51,    -1,    50,     1,
      51,    -1,   117,    -1,   118,     1,   117,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   252,   252,   253,   254,   255,   259,   266,   272,   296,
     298,   301,   303,   304,   307,   309,   314,   315,   318,   320,
     331,   341,   344,   349,   351,   355,   359,   363,   364,   367,
     369,   380,   381,   382,   383,   384,   385,   386,   390,   391,
     395,   398,   403,   407,   412,   419,   433,   434,   444,   466,
     469,   475,   478,   484,   485,   486,   490,   491,   495,   515,
     528,   542,   549,   552,   573,   580,   584,   589,   597,   598,
     607,   608,   609,   614,   634,   638,   642,   652,   659,   665,
     666,   667,   671,   680,   681,   688,   712,   717,   726,   729,
     732,   735,   738,   741,   747,   748,   752,   758,   763,   772,
     775,   778,   781,   786,   792,   796,   799,   804,   810,   832,
     838,   845,   846,   850,   854,   870,   873,   876,   882,   883,
     885,   886,   887,   893,   894,   895,   897,   903,   904,   905,
     906,   907,   908,   909,   910,   911,   912,   913,   914,   915,
     916,   917,   918,   919,   920,   925,   933,   949,   956,   962,
     971,   997,   998,  1002,  1003
};
#endif

#if YYDEBUG || YYERROR_VERBOSE
/* YYTNME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "S", "SGML_CD", "INCLUDES", "DASHMATCH", 
  "BEGINSWITH", "ENDSWITH", "CONTAINS", "STRING", "IDENT", "HASH", "':'", 
  "'.'", "'['", "'*'", "'|'", "IMPORT_SYM", "PAGE_SYM", "MEDIA_SYM", 
  "FONT_FACE_SYM", "CHARSET_SYM", "NAMESPACE_SYM", "KHTML_RULE_SYM", 
  "KHTML_DECLS_SYM", "KHTML_VALUE_SYM", "IMPORTANT_SYM", "QEMS", "EMS", 
  "EXS", "PXS", "CMS", "MMS", "INS", "PTS", "PCS", "DEGS", "RADS", 
  "GRADS", "MSECS", "SECS", "HERZ", "KHERZ", "DIMEN", "PERCENTAGE", 
  "NUMBER", "URI", "FUNCTION", "UNICODERANGE", "'{'", "'}'", "';'", "','", 
  "'+'", "'>'", "'-'", "']'", "'='", "')'", "'/'", "'@'", "$accept", 
  "stylesheet", "khtml_rule", "khtml_decls", "khtml_value", "maybe_space", 
  "maybe_sgml", "maybe_charset", "import_list", "import", 
  "maybe_namespace", "namespace", "ns_prefix", "maybe_ns_prefix", 
  "rule_list", "rule", "string_or_uri", "maybe_media_list", "media_list", 
  "media", "ruleset_list", "medium", "page", "font_face", "combinator", 
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
     265,   266,   267,    58,    46,    91,    42,   124,   268,   269,
     270,   271,   272,   273,   274,   275,   276,   277,   278,   279,
     280,   281,   282,   283,   284,   285,   286,   287,   288,   289,
     290,   291,   292,   293,   294,   295,   296,   297,   298,   299,
     123,   125,    59,    44,    43,    62,    45,    93,    61,    41,
      47,    64
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    62,    63,    63,    63,    63,    64,    65,    66,    67,
      67,    68,    68,    68,    69,    69,    69,    69,    70,    70,
      71,    71,    71,    72,    72,    73,    74,    75,    75,    76,
      76,    77,    77,    77,    77,    77,    77,    77,    78,    78,
      79,    79,    80,    80,    80,    81,    82,    82,    83,    84,
      84,    85,    85,    86,    86,    86,    87,    87,    88,    89,
      89,    89,    90,    90,    90,    91,    91,    91,    92,    92,
      93,    93,    93,    94,    94,    95,    95,    95,    96,    96,
      96,    96,    97,    98,    98,    99,   100,   100,   101,   101,
     101,   101,   101,   101,   102,   102,   103,   103,   103,   104,
     104,   104,   104,   104,   105,   105,   105,   105,   106,   106,
     107,   108,   108,   109,   109,   110,   110,   110,   111,   111,
     111,   111,   111,   111,   111,   111,   111,   112,   112,   112,
     112,   112,   112,   112,   112,   112,   112,   112,   112,   112,
     112,   112,   112,   112,   112,   113,   113,   114,   115,   115,
     116,   117,   117,   118,   118
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
     228,   144,   -37,   -12,    -8,    94,  -159,  -159,  -159,  -159,
     -16,    13,  -159,  -159,  -159,  -159,  -159,    14,    14,    14,
     101,   112,  -159,  -159,  -159,  -159,   205,    18,   301,  -159,
    -159,   165,   194,    36,    81,  -159,    -3,   114,  -159,   135,
    -159,  -159,    32,   160,  -159,   246,   214,  -159,   152,  -159,
    -159,  -159,  -159,   221,  -159,    45,   196,   116,  -159,  -159,
    -159,  -159,  -159,  -159,  -159,  -159,  -159,  -159,  -159,  -159,
    -159,  -159,  -159,  -159,  -159,  -159,  -159,  -159,  -159,  -159,
    -159,  -159,  -159,  -159,  -159,   370,   127,  -159,  -159,  -159,
    -159,    96,  -159,  -159,  -159,  -159,  -159,  -159,   143,  -159,
    -159,  -159,   159,  -159,  -159,    15,  -159,    38,  -159,  -159,
    -159,  -159,  -159,  -159,   303,    14,   152,  -159,  -159,  -159,
    -159,    14,  -159,  -159,  -159,  -159,    14,   255,  -159,   223,
    -159,  -159,  -159,    14,    14,    14,    14,    14,    14,    14,
      14,    14,    14,    14,    14,    14,    14,    14,    14,    14,
      14,    14,    14,    14,    14,    14,   254,    14,  -159,  -159,
    -159,  -159,   341,   129,   104,    17,   101,   121,   101,   271,
    -159,   205,    81,   135,   198,    43,  -159,  -159,   205,    14,
      14,  -159,    14,    14,  -159,  -159,  -159,  -159,    14,   301,
    -159,    57,    14,    14,  -159,  -159,  -159,  -159,  -159,  -159,
    -159,  -159,    99,   108,   184,  -159,   211,   222,   230,  -159,
    -159,  -159,  -159,  -159,  -159,  -159,  -159,  -159,   192,    14,
    -159,  -159,  -159,  -159,  -159,  -159,  -159,  -159,  -159,  -159,
     226,    14,    14,    97,  -159,    19,    14,  -159,  -159,   147,
      19,   151,   174,  -159,   101,  -159,   117,  -159,  -159,    14,
    -159,   204,    34,  -159,    51,  -159,  -159,    42,  -159,  -159,
    -159,  -159,  -159,  -159,  -159,    14,    14,  -159,  -159,  -159,
    -159,  -159,    -2,    19,    14,  -159,  -159,   178,  -159,  -159,
      14
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -159,  -159,  -159,  -159,  -159,    -1,   -91,  -159,  -159,    95,
    -159,  -159,  -159,  -159,  -159,  -159,    65,  -159,    29,  -159,
    -159,     4,  -159,  -159,  -159,  -159,  -158,  -159,   131,   -87,
    -159,   218,   278,   280,   -33,  -159,  -159,   153,  -159,  -159,
    -159,  -159,   284,  -159,   273,  -159,  -159,   -63,  -159,   197,
     269,  -159,  -159,  -159,  -159,    -7,  -159
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -113
static const short yytable[] =
{
      11,    24,   166,    23,   168,    17,    18,    19,   101,   213,
     102,    26,    27,    12,    28,   122,    24,    24,    24,    53,
      24,    24,    24,    25,    33,    97,   172,   181,   200,    54,
     250,   173,    40,   108,    21,   268,    22,   105,    13,    24,
     107,    24,    15,   268,   115,   103,   127,   121,   222,   223,
     224,   225,   226,   126,    24,   275,    54,   133,   134,   135,
     136,   137,   138,   139,   140,   141,   142,   143,   144,   145,
     146,   147,   148,   149,   150,   151,   152,   153,   154,   155,
     156,   157,    13,   122,   218,   109,   -41,   269,    99,   177,
     164,   165,   271,   191,    16,   269,   128,   163,   100,    -9,
     227,   228,   171,   270,    29,    30,    -9,    24,   178,   197,
     160,   179,   180,    32,   197,   182,   234,   161,   244,   279,
      24,    -2,   203,   183,   247,   104,   233,   262,   263,   132,
     188,   189,    34,    35,    36,    37,    38,    39,    40,    91,
     204,   205,   206,    -9,   169,    10,   198,    -9,  -112,  -112,
     160,   198,   106,   120,    -9,    -9,   196,   161,    21,   192,
     193,   111,   217,    -9,    35,    36,    37,    38,    -9,    -9,
     170,   -55,   -55,   -55,   -55,   -55,   -55,   -55,   159,    21,
     160,   195,   207,    91,   231,   239,   232,   161,    92,    34,
      35,    36,    37,    38,    39,    40,   238,    21,   235,   255,
     236,    21,    -9,   258,   240,    -9,    -9,    -9,    24,   220,
     -59,    -9,   241,   -59,   112,   113,    34,    35,    36,    37,
      38,    39,    40,   242,    21,   117,   260,   111,   246,   278,
     118,   243,   256,   249,   259,   261,   254,   -55,   -55,   -55,
     -55,   -55,   -55,   -55,    21,    96,   265,   130,   131,   266,
       1,   245,     2,     3,     4,   190,   267,    24,    35,    36,
      37,    38,   208,   272,    58,    59,    60,   237,   273,   257,
     274,    21,   123,   124,   186,   187,   -60,   276,   280,   -60,
     112,   113,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    24,    21,   184,   185,    83,   230,
      84,    58,    59,    60,    34,    35,    36,    37,    38,    39,
      40,    21,   216,   174,   119,   116,   110,   221,   129,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    58,    59,    60,   158,    83,     0,    84,     0,   194,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,     0,     0,     0,     0,    83,     0,    84,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,     0,    78,    79
};

static const short yycheck[] =
{
       1,     3,    93,    10,    95,     6,     7,     8,    11,   167,
      13,    12,    13,    50,    15,    48,     3,     3,     3,     1,
       3,     3,     3,    10,    25,    32,    11,   114,    11,    11,
      11,    16,    17,     1,    50,     1,    52,    38,    50,     3,
      41,     3,    50,     1,    45,    48,     1,    48,     5,     6,
       7,     8,     9,    54,     3,    57,    11,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    50,   116,   171,    53,    52,    53,    52,    51,
      91,    92,    50,   156,     0,    53,    51,     1,    17,     3,
      57,    58,   103,    52,     3,     4,    10,     3,   109,    10,
      53,   112,   113,     1,    10,   116,    59,    60,   209,   277,
       3,     0,     1,   124,    27,    11,   189,    10,    11,    13,
     131,   132,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    47,     1,     1,    47,     3,    51,    52,
      53,    47,    17,     1,    10,     3,   163,    60,    50,   160,
     161,     1,   169,    11,    12,    13,    14,    15,    16,    17,
      11,    11,    12,    13,    14,    15,    16,    17,    51,    50,
      53,    52,    61,    18,   185,     1,   187,    60,    23,    11,
      12,    13,    14,    15,    16,    17,   203,    50,   199,    52,
     201,    50,    50,    52,   205,    53,    54,    55,     3,    11,
      50,    59,     1,    53,    54,    55,    11,    12,    13,    14,
      15,    16,    17,     1,    50,    11,    52,     1,   229,    51,
      16,     1,   239,   234,   241,   242,   237,    11,    12,    13,
      14,    15,    16,    17,    50,    51,   247,    51,    52,   250,
      22,    59,    24,    25,    26,     1,    52,     3,    12,    13,
      14,    15,   167,   264,    10,    11,    12,   202,   269,   240,
     271,    50,    51,    52,    51,    52,    50,   273,   279,    53,
      54,    55,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,     3,    50,    51,    52,    54,   178,
      56,    10,    11,    12,    11,    12,    13,    14,    15,    16,
      17,    50,    51,   105,    46,    45,    42,   174,    55,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    10,    11,    12,    85,    54,    -1,    56,    -1,   162,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    -1,    -1,    -1,    -1,    54,    -1,    56,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    -1,    45,    46
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,    22,    24,    25,    26,    63,    64,    65,    66,    69,
       1,    67,    50,    50,   104,    50,     0,    67,    67,    67,
      68,    50,    52,   117,     3,    10,    67,    67,    67,     3,
       4,    70,     1,    67,    11,    12,    13,    14,    15,    16,
      17,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,   100,   103,     1,    11,   105,   106,   107,    10,    11,
      12,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    54,    56,    87,   109,   111,   112,   113,
     114,    18,    23,    71,    72,    73,    51,   117,   118,    52,
      17,    11,    13,    48,    11,    67,    17,    67,     1,    53,
     104,     1,    54,    55,    86,    67,    95,    11,    16,    94,
       1,    67,    96,    51,    52,   117,    67,     1,    51,   106,
      51,    52,    13,    67,    67,    67,    67,    67,    67,    67,
      67,    67,    67,    67,    67,    67,    67,    67,    67,    67,
      67,    67,    67,    67,    67,    67,    67,    67,   112,    51,
      53,    60,   110,     1,    67,    67,    68,    76,    68,     1,
      11,    67,    11,    16,    93,    98,    99,    51,    67,    67,
      67,    91,    67,    67,    51,    52,    51,    52,    67,    67,
       1,   109,    67,    67,   111,    52,   117,    10,    47,    78,
      11,    74,    75,     1,    19,    20,    21,    61,    71,    77,
      81,    84,    85,    88,   115,   116,    51,   117,    91,    67,
      11,    99,     5,     6,     7,     8,     9,    57,    58,   101,
      90,    67,    67,   109,    59,    67,    67,    78,   117,     1,
      67,     1,     1,     1,    68,    59,    67,    27,   108,    67,
      11,    79,    80,    83,    67,    52,   117,    80,    52,   117,
      52,   117,    10,    11,   102,    67,    67,    52,     1,    53,
      52,    50,    67,    67,    67,    57,    83,    82,    51,    88,
      67
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
      case 79: /* maybe_media_list */

        (null);

        break;
      case 80: /* media_list */

        (null);

        break;
      case 82: /* ruleset_list */

        (null);

        break;
      case 89: /* selector_list */

        (null);

        break;
      case 90: /* selector */

        (null);

        break;
      case 91: /* simple_selector */

        (null);

        break;
      case 95: /* specifier_list */

        (null);

        break;
      case 96: /* specifier */

        (null);

        break;
      case 97: /* class */

        (null);

        break;
      case 100: /* attrib */

        (null);

        break;
      case 103: /* pseudo */

        (null);

        break;
      case 109: /* expr */

        (null);

        break;
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
      case 79: /* maybe_media_list */

        { delete yyvaluep->mediaList; yyvaluep->mediaList = 0; };

        break;
      case 80: /* media_list */

        { delete yyvaluep->mediaList; yyvaluep->mediaList = 0; };

        break;
      case 82: /* ruleset_list */

        { delete yyvaluep->ruleList; yyvaluep->ruleList = 0; };

        break;
      case 89: /* selector_list */

        { delete yyvaluep->selectorList; yyvaluep->selectorList = 0; };

        break;
      case 90: /* selector */

        { delete yyvaluep->selector; yyvaluep->selector = 0; };

        break;
      case 91: /* simple_selector */

        { delete yyvaluep->selector; yyvaluep->selector = 0; };

        break;
      case 95: /* specifier_list */

        { delete yyvaluep->selector; yyvaluep->selector = 0; };

        break;
      case 96: /* specifier */

        { delete yyvaluep->selector; yyvaluep->selector = 0; };

        break;
      case 97: /* class */

        { delete yyvaluep->selector; yyvaluep->selector = 0; };

        break;
      case 100: /* attrib */

        { delete yyvaluep->selector; yyvaluep->selector = 0; };

        break;
      case 103: /* pseudo */

        { delete yyvaluep->selector; yyvaluep->selector = 0; };

        break;
      case 109: /* expr */

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

    { yyval.relation = CSSSelector::Child; ;}
    break;

  case 55:

    { yyval.relation = CSSSelector::Descendant; ;}
    break;

  case 56:

    { yyval.val = -1; ;}
    break;

  case 57:

    { yyval.val = 1; ;}
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
    ;}
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
    ;}
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
    ;}
    break;

  case 61:

    {
	delete yyvsp[-1].selectorList;
	yyval.selectorList = 0;
    ;}
    break;

  case 62:

    {
	yyval.selector = yyvsp[0].selector;
    ;}
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
    ;}
    break;

  case 64:

    {
	delete yyvsp[-1].selector;
	yyval.selector = 0;
    ;}
    break;

  case 65:

    {
	yyval.selector = new CSSSelector();
	yyval.selector->tag = yyvsp[-1].element;
    ;}
    break;

  case 66:

    {
	yyval.selector = yyvsp[-1].selector;
        if ( yyval.selector )
	    yyval.selector->tag = yyvsp[-2].element;
    ;}
    break;

  case 67:

    {
	yyval.selector = yyvsp[-1].selector;
        if ( yyval.selector )
	    yyval.selector->tag = 0xffffffff;
    ;}
    break;

  case 68:

    { yyval.element = (yyvsp[-1].ns<<16) | yyvsp[0].element; ;}
    break;

  case 69:

    {
        /* according to the specs this one matches all namespaces if no
	   default namespace has been specified otherwise the default namespace */
	CSSParser *p = static_cast<CSSParser *>(parser);
	yyval.element = (p->defaultNamespace<<16) | yyvsp[0].element;
    ;}
    break;

  case 70:

    { yyval.ns = 0; ;}
    break;

  case 71:

    { yyval.ns = 1; /* #### insert correct namespace id here */ ;}
    break;

  case 72:

    { yyval.ns = 0xffff; ;}
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
    ;}
    break;

  case 74:

    { yyval.element = 0xffff; ;}
    break;

  case 75:

    {
	yyval.selector = yyvsp[0].selector;
	yyval.selector->nonCSSHint = static_cast<CSSParser *>(parser)->nonCSSHint;
    ;}
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
    ;}
    break;

  case 77:

    {
	delete yyvsp[-1].selector;
	yyval.selector = 0;
    ;}
    break;

  case 78:

    {
	yyval.selector = new CSSSelector();
	yyval.selector->match = CSSSelector::Id;
	yyval.selector->attr = ATTR_ID;
	yyval.selector->value = domString(yyvsp[0].string);
    ;}
    break;

  case 82:

    {
	yyval.selector = new CSSSelector();
	yyval.selector->match = CSSSelector::List;
	yyval.selector->attr = ATTR_CLASS;
	yyval.selector->value = domString(yyvsp[0].string);
    ;}
    break;

  case 83:

    { yyval.attribute = (yyvsp[-1].ns<<16) | yyvsp[0].attribute; ;}
    break;

  case 84:

    {
	/* opposed to elements, these only match for non namespaced attributes */
	yyval.attribute = yyvsp[0].attribute;
    ;}
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
    ;}
    break;

  case 86:

    {
	yyval.selector = new CSSSelector();
	yyval.selector->attr = yyvsp[-1].attribute;
	yyval.selector->match = CSSSelector::Set;
    ;}
    break;

  case 87:

    {
	yyval.selector = new CSSSelector();
	yyval.selector->attr = yyvsp[-5].attribute;
	yyval.selector->match = (CSSSelector::Match)yyvsp[-4].val;
	yyval.selector->value = domString(yyvsp[-2].string);
    ;}
    break;

  case 88:

    {
	yyval.val = CSSSelector::Exact;
    ;}
    break;

  case 89:

    {
	yyval.val = CSSSelector::List;
    ;}
    break;

  case 90:

    {
	yyval.val = CSSSelector::Hyphen;
    ;}
    break;

  case 91:

    {
	yyval.val = CSSSelector::Begin;
    ;}
    break;

  case 92:

    {
	yyval.val = CSSSelector::End;
    ;}
    break;

  case 93:

    {
	yyval.val = CSSSelector::Contain;
    ;}
    break;

  case 96:

    {
	yyval.selector = new CSSSelector();
	yyval.selector->match = CSSSelector::Pseudo;
	yyval.selector->value = domString(yyvsp[0].string);
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
        yyval.selector->simpleSelector = yyvsp[-1].selector;
        yyval.selector->value = domString(yyvsp[-3].string);
    ;}
    break;

  case 99:

    {
	yyval.ok = yyvsp[-1].ok;
    ;}
    break;

  case 100:

    {
	yyval.ok = false;
    ;}
    break;

  case 101:

    {
	yyval.ok = yyvsp[-1].ok;
    ;}
    break;

  case 102:

    {
	yyval.ok = yyvsp[-2].ok;
	if ( yyvsp[-1].ok )
	    yyval.ok = yyvsp[-1].ok;
    ;}
    break;

  case 103:

    {
	yyval.ok = yyvsp[-2].ok;
    ;}
    break;

  case 104:

    {
	yyval.ok = yyvsp[-2].ok;
    ;}
    break;

  case 105:

    {
        yyval.ok = false;
    ;}
    break;

  case 106:

    {
	yyval.ok = yyvsp[-3].ok;
	if ( yyvsp[-2].ok )
	    yyval.ok = yyvsp[-2].ok;
    ;}
    break;

  case 107:

    {
        yyval.ok = yyvsp[-3].ok;
    ;}
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
    ;}
    break;

  case 109:

    {
        yyval.ok = false;
    ;}
    break;

  case 110:

    {
	QString str = qString(yyvsp[-1].string);
	yyval.prop_id = getPropertyID( str.lower().latin1(), str.length() );
    ;}
    break;

  case 111:

    { yyval.b = true; ;}
    break;

  case 112:

    { yyval.b = false; ;}
    break;

  case 113:

    {
	yyval.valueList = new ValueList;
	yyval.valueList->addValue( yyvsp[0].value );
    ;}
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
    ;}
    break;

  case 115:

    {
	yyval.tok = '/';
    ;}
    break;

  case 116:

    {
	yyval.tok = ',';
    ;}
    break;

  case 117:

    {
        yyval.tok = 0;
  ;}
    break;

  case 118:

    { yyval.value = yyvsp[0].value; ;}
    break;

  case 119:

    { yyval.value = yyvsp[0].value; yyval.value.fValue *= yyvsp[-1].val; ;}
    break;

  case 120:

    { yyval.value.id = 0; yyval.value.string = yyvsp[-1].string; yyval.value.unit = CSSPrimitiveValue::CSS_DIMENSION; ;}
    break;

  case 121:

    { yyval.value.id = 0; yyval.value.string = yyvsp[-1].string; yyval.value.unit = CSSPrimitiveValue::CSS_STRING; ;}
    break;

  case 122:

    {
      QString str = qString( yyvsp[-1].string );
      yyval.value.id = getValueID( str.lower().latin1(), str.length() );
      yyval.value.unit = CSSPrimitiveValue::CSS_IDENT;
      yyval.value.string = yyvsp[-1].string;
  ;}
    break;

  case 123:

    { yyval.value.id = 0; yyval.value.string = yyvsp[-1].string; yyval.value.unit = CSSPrimitiveValue::CSS_URI; ;}
    break;

  case 124:

    { yyval.value.id = 0; yyval.value.iValue = 0; yyval.value.unit = CSSPrimitiveValue::CSS_UNKNOWN;/* ### */ ;}
    break;

  case 125:

    { yyval.value.id = 0; yyval.value.string = yyvsp[0].string; yyval.value.unit = CSSPrimitiveValue::CSS_RGBCOLOR; ;}
    break;

  case 126:

    {
      yyval.value = yyvsp[0].value;
  ;}
    break;

  case 127:

    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_NUMBER; ;}
    break;

  case 128:

    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_PERCENTAGE; ;}
    break;

  case 129:

    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_PX; ;}
    break;

  case 130:

    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_CM; ;}
    break;

  case 131:

    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_MM; ;}
    break;

  case 132:

    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_IN; ;}
    break;

  case 133:

    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_PT; ;}
    break;

  case 134:

    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_PC; ;}
    break;

  case 135:

    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_DEG; ;}
    break;

  case 136:

    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_RAD; ;}
    break;

  case 137:

    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_GRAD; ;}
    break;

  case 138:

    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_MS; ;}
    break;

  case 139:

    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_S; ;}
    break;

  case 140:

    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_HZ; ;}
    break;

  case 141:

    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_KHZ; ;}
    break;

  case 142:

    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_EMS; ;}
    break;

  case 143:

    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = Value::Q_EMS; ;}
    break;

  case 144:

    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_EXS; ;}
    break;

  case 145:

    {
      Function *f = new Function;
      f->name = yyvsp[-4].string;
      f->args = yyvsp[-2].valueList;
      yyval.value.id = 0;
      yyval.value.unit = Value::Function;
      yyval.value.function = f;
  ;}
    break;

  case 146:

    {
      Function *f = new Function;
      f->name = yyvsp[-2].string;
      f->args = 0;
      yyval.value.id = 0;
      yyval.value.unit = Value::Function;
      yyval.value.function = f;
  ;}
    break;

  case 147:

    { yyval.string = yyvsp[-1].string; ;}
    break;

  case 148:

    {
	yyval.rule = 0;
#ifdef CSS_DEBUG
	kdDebug( 6080 ) << "skipped invalid @-rule" << endl;
#endif
    ;}
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
	kdDebug( 6080 ) << "skipped invalid rule" << endl;
#endif
    ;}
    break;


    }

/* Line 991 of yacc.c.  */


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
  goto yyerrlab2;


/*---------------------------------------------------------------.
| yyerrlab2 -- pop states until the error token can be shifted.  |
`---------------------------------------------------------------*/
yyerrlab2:
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






