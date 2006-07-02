/* A Bison parser, made by GNU Bison 1.875d.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004 Free Software Foundation, Inc.

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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
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
    CSSSelector::Match match;
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

# ifndef YYFREE
#  define YYFREE free
# endif
# ifndef YYMALLOC
#  define YYMALLOC malloc
# endif

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   define YYSTACK_ALLOC alloca
#  endif
# else
#  if defined (alloca) || defined (_ALLOCA_H)
#   define YYSTACK_ALLOC alloca
#  else
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
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
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
# endif
#endif /* ! defined (yyoverflow) || YYERROR_VERBOSE */


#if (! defined (yyoverflow) \
     && (! defined (__cplusplus) \
	 || (defined (YYSTYPE_IS_TRIVIAL) && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  short int yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short int) + sizeof (YYSTYPE))			\
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined (__GNUC__) && 1 < __GNUC__
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
   typedef short int yysigned_char;
#endif

/* YYFINAL -- State number of the termination state. */
#define YYFINAL  16
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   479

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  67
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  54
/* YYNRULES -- Number of rules. */
#define YYNRULES  162
/* YYNRULES -- Number of states. */
#define YYNSTATES  305

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   303

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
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
static const unsigned short int yyprhs[] =
{
       0,     0,     3,     9,    12,    15,    18,    25,    28,    34,
      35,    38,    39,    42,    45,    46,    52,    56,    60,    61,
      65,    72,    76,    80,    81,    85,    92,    96,   100,   101,
     104,   105,   109,   111,   113,   115,   117,   119,   121,   124,
     126,   128,   129,   131,   133,   138,   141,   149,   150,   154,
     157,   161,   165,   169,   173,   176,   179,   182,   183,   185,
     187,   190,   192,   197,   200,   202,   206,   209,   211,   214,
     217,   220,   224,   227,   231,   236,   240,   242,   244,   246,
     249,   252,   254,   256,   258,   260,   263,   266,   271,   280,
     286,   296,   298,   300,   302,   304,   306,   308,   310,   312,
     315,   319,   324,   329,   334,   339,   345,   350,   355,   360,
     366,   372,   376,   380,   385,   390,   396,   399,   402,   405,
     406,   408,   412,   415,   418,   419,   421,   424,   427,   430,
     433,   436,   439,   441,   443,   446,   449,   452,   455,   458,
     461,   464,   467,   470,   473,   476,   479,   482,   485,   488,
     491,   494,   497,   500,   506,   510,   513,   517,   521,   524,
     530,   534,   536
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const yysigned_char yyrhs[] =
{
      68,     0,    -1,    74,    73,    75,    77,    80,    -1,    69,
      72,    -1,    70,    72,    -1,    71,    72,    -1,    26,    54,
      72,    92,    72,    55,    -1,    27,   106,    -1,    28,    54,
      72,   111,    55,    -1,    -1,    72,     4,    -1,    -1,    73,
       5,    -1,    73,     4,    -1,    -1,    24,    72,    11,    72,
      56,    -1,    24,     1,   119,    -1,    24,     1,    56,    -1,
      -1,    75,    76,    73,    -1,    20,    72,    82,    72,    83,
      56,    -1,    20,     1,   119,    -1,    20,     1,    56,    -1,
      -1,    77,    78,    73,    -1,    25,    72,    79,    82,    72,
      56,    -1,    25,     1,   119,    -1,    25,     1,    56,    -1,
      -1,    12,     4,    -1,    -1,    80,    81,    73,    -1,    92,
      -1,    85,    -1,    88,    -1,    89,    -1,   118,    -1,   117,
      -1,    76,     1,    -1,    11,    -1,    50,    -1,    -1,    84,
      -1,    87,    -1,    84,    57,    72,    87,    -1,    84,     1,
      -1,    22,    72,    84,    54,    72,    86,    55,    -1,    -1,
      86,    92,    72,    -1,    12,    72,    -1,    21,     1,   119,
      -1,    21,     1,    56,    -1,    23,     1,   119,    -1,    23,
       1,    56,    -1,    58,    72,    -1,    59,    72,    -1,    60,
      72,    -1,    -1,    61,    -1,    58,    -1,    93,   106,    -1,
      94,    -1,    93,    57,    72,    94,    -1,    93,     1,    -1,
      96,    -1,    94,    90,    96,    -1,    94,     1,    -1,    19,
      -1,    18,    19,    -1,    12,    19,    -1,    97,    72,    -1,
      97,    98,    72,    -1,    98,    72,    -1,    95,    97,    72,
      -1,    95,    97,    98,    72,    -1,    95,    98,    72,    -1,
      12,    -1,    18,    -1,    99,    -1,    98,    99,    -1,    98,
       1,    -1,    14,    -1,   100,    -1,   102,    -1,   105,    -1,
      16,    12,    -1,    12,    72,    -1,    17,    72,   101,    62,
      -1,    17,    72,   101,   103,    72,   104,    72,    62,    -1,
      17,    72,    95,   101,    62,    -1,    17,    72,    95,   101,
     103,    72,   104,    72,    62,    -1,    63,    -1,     6,    -1,
       7,    -1,     8,    -1,     9,    -1,    10,    -1,    12,    -1,
      11,    -1,    15,    12,    -1,    15,    15,    12,    -1,    15,
      51,    13,    64,    -1,    15,    51,    49,    64,    -1,    15,
      51,    12,    64,    -1,    15,    51,    11,    64,    -1,    15,
      52,    72,    96,    64,    -1,    54,    72,   108,    55,    -1,
      54,    72,     1,    55,    -1,    54,    72,   107,    55,    -1,
      54,    72,   107,   108,    55,    -1,    54,    72,   107,     1,
      55,    -1,   108,    56,    72,    -1,     1,    56,    72,    -1,
     107,   108,    56,    72,    -1,   107,     1,    56,    72,    -1,
     109,    15,    72,   111,   110,    -1,     1,   119,    -1,    12,
      72,    -1,    29,    72,    -1,    -1,   113,    -1,   111,   112,
     113,    -1,    65,    72,    -1,    57,    72,    -1,    -1,   114,
      -1,    91,   114,    -1,    46,    72,    -1,    11,    72,    -1,
      12,    72,    -1,    50,    72,    -1,    53,    72,    -1,   116,
      -1,   115,    -1,    49,    72,    -1,    48,    72,    -1,    47,
      72,    -1,    33,    72,    -1,    34,    72,    -1,    35,    72,
      -1,    36,    72,    -1,    37,    72,    -1,    38,    72,    -1,
      39,    72,    -1,    40,    72,    -1,    41,    72,    -1,    42,
      72,    -1,    43,    72,    -1,    44,    72,    -1,    45,    72,
      -1,    31,    72,    -1,    30,    72,    -1,    32,    72,    -1,
      51,    72,   111,    64,    72,    -1,    51,    72,     1,    -1,
      14,    72,    -1,    66,     1,   119,    -1,    66,     1,    56,
      -1,     1,   119,    -1,    54,     1,   120,     1,    55,    -1,
      54,     1,    55,    -1,   119,    -1,   120,     1,   119,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short int yyrline[] =
{
       0,   257,   257,   258,   259,   260,   264,   271,   277,   302,
     303,   306,   308,   309,   312,   314,   319,   320,   323,   325,
     336,   346,   349,   355,   356,   360,   368,   369,   373,   374,
     377,   379,   390,   391,   392,   393,   394,   395,   396,   400,
     401,   405,   408,   413,   417,   422,   429,   443,   444,   454,
     476,   479,   485,   488,   494,   495,   496,   497,   501,   502,
     506,   526,   540,   555,   562,   565,   579,   586,   587,   588,
     592,   596,   601,   606,   613,   622,   634,   650,   656,   660,
     670,   677,   683,   684,   685,   689,   698,   722,   727,   733,
     741,   753,   756,   759,   762,   765,   768,   774,   775,   779,
     785,   791,   798,   805,   812,   819,   828,   831,   834,   837,
     842,   848,   852,   855,   860,   866,   888,   894,   901,   902,
     906,   910,   926,   929,   932,   938,   939,   941,   942,   943,
     949,   950,   951,   953,   959,   960,   961,   962,   963,   964,
     965,   966,   967,   968,   969,   970,   971,   972,   973,   974,
     975,   976,   977,   982,   990,  1006,  1013,  1019,  1028,  1054,
    1055,  1059,  1060
};
#endif

#if YYDEBUG || YYERROR_VERBOSE
/* YYTNME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
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
  "')'", "'/'", "'@'", "$accept", "stylesheet", "khtml_rule",
  "khtml_decls", "khtml_value", "maybe_space", "maybe_sgml",
  "maybe_charset", "import_list", "import", "namespace_list", "namespace",
  "maybe_ns_prefix", "rule_list", "rule", "string_or_uri",
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
static const unsigned short int yytoknum[] =
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
static const unsigned char yyr1[] =
{
       0,    67,    68,    68,    68,    68,    69,    70,    71,    72,
      72,    73,    73,    73,    74,    74,    74,    74,    75,    75,
      76,    76,    76,    77,    77,    78,    78,    78,    79,    79,
      80,    80,    81,    81,    81,    81,    81,    81,    81,    82,
      82,    83,    83,    84,    84,    84,    85,    86,    86,    87,
      88,    88,    89,    89,    90,    90,    90,    90,    91,    91,
      92,    93,    93,    93,    94,    94,    94,    95,    95,    95,
      96,    96,    96,    96,    96,    96,    97,    97,    98,    98,
      98,    99,    99,    99,    99,   100,   101,   102,   102,   102,
     102,   103,   103,   103,   103,   103,   103,   104,   104,   105,
     105,   105,   105,   105,   105,   105,   106,   106,   106,   106,
     106,   107,   107,   107,   107,   108,   108,   109,   110,   110,
     111,   111,   112,   112,   112,   113,   113,   113,   113,   113,
     113,   113,   113,   113,   114,   114,   114,   114,   114,   114,
     114,   114,   114,   114,   114,   114,   114,   114,   114,   114,
     114,   114,   114,   115,   115,   116,   117,   117,   118,   119,
     119,   120,   120
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     5,     2,     2,     2,     6,     2,     5,     0,
       2,     0,     2,     2,     0,     5,     3,     3,     0,     3,
       6,     3,     3,     0,     3,     6,     3,     3,     0,     2,
       0,     3,     1,     1,     1,     1,     1,     1,     2,     1,
       1,     0,     1,     1,     4,     2,     7,     0,     3,     2,
       3,     3,     3,     3,     2,     2,     2,     0,     1,     1,
       2,     1,     4,     2,     1,     3,     2,     1,     2,     2,
       2,     3,     2,     3,     4,     3,     1,     1,     1,     2,
       2,     1,     1,     1,     1,     2,     2,     4,     8,     5,
       9,     1,     1,     1,     1,     1,     1,     1,     1,     2,
       3,     4,     4,     4,     4,     5,     4,     4,     4,     5,
       5,     3,     3,     4,     4,     5,     2,     2,     2,     0,
       1,     3,     2,     2,     0,     1,     2,     2,     2,     2,
       2,     2,     1,     1,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     5,     3,     2,     3,     3,     2,     5,
       3,     1,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
      14,     0,     0,     0,     0,     0,     9,     9,     9,    11,
       0,     0,     9,     9,     7,     9,     1,     3,     4,     5,
      18,     0,    17,    16,    10,     9,     0,     0,     0,    13,
      12,    23,     0,     0,    76,    81,     0,     0,     9,    77,
      67,     9,     0,     0,     0,    64,     9,     0,    78,    82,
      83,    84,     0,     9,     0,     0,     0,     9,     9,     9,
       9,     9,     9,     9,     9,     9,     9,     9,     9,     9,
       9,     9,     9,     9,     9,     9,     9,     9,     9,     9,
       9,     9,     9,    59,    58,     0,   124,   120,   125,   133,
     132,     0,    11,    30,   160,   161,     0,    15,    69,    99,
       0,     0,     9,    85,     0,    68,     0,    63,     9,    60,
      66,     9,     9,     9,     0,    76,    77,     9,     0,    70,
       0,    80,    72,    79,   107,     9,   116,   117,     0,   108,
       0,   106,     9,     9,   128,   129,   155,   151,   150,   152,
     137,   138,   139,   140,   141,   142,   143,   144,   145,   146,
     147,   148,   149,   127,   136,   135,   134,   130,     0,   131,
     126,     8,     9,     9,     0,     0,     0,    19,     0,    11,
       0,     0,   100,     0,     0,     0,     0,     0,     9,     0,
       0,     0,     6,     0,    54,    55,    56,    65,    73,     0,
      75,    71,   112,   110,     9,   109,     9,   111,     0,   154,
     124,   123,   122,   121,    22,    21,    39,    40,     9,     0,
      28,    24,     0,     0,     9,     0,     0,     0,    11,    33,
      34,    35,    32,    37,    36,   159,   162,   104,   103,   101,
     102,     0,    86,     9,     0,    92,    93,    94,    95,    96,
      87,    91,     9,     0,    74,   114,   113,   124,     9,    41,
      27,    26,     0,     0,   158,     0,     0,     0,     0,    38,
      31,   105,    89,     9,     0,     9,   115,   153,     9,     0,
       0,    43,    29,     9,    51,    50,     0,    53,    52,   157,
     156,     0,    98,    97,     9,   118,    49,    20,    45,     9,
       0,     9,     9,     0,     0,    25,    47,     0,    88,    44,
       0,    90,    46,     9,    48
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short int yydefgoto[] =
{
      -1,     5,     6,     7,     8,   232,    20,     9,    31,    92,
      93,   169,   253,   170,   218,   208,   269,   270,   219,   300,
     271,   220,   221,   114,    85,    41,    42,    43,    44,    45,
      46,    47,    48,    49,   181,    50,   242,   284,    51,    14,
      54,    55,    56,   266,    86,   164,    87,    88,    89,    90,
     223,   224,   126,    96
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -167
static const short int yypact[] =
{
     252,    19,   -46,   -32,   -20,    44,  -167,  -167,  -167,  -167,
     147,   183,  -167,  -167,  -167,  -167,  -167,    96,    96,    96,
      34,   114,  -167,  -167,  -167,  -167,   333,   156,   355,  -167,
    -167,    85,    98,    -2,    99,  -167,   158,   109,  -167,   152,
    -167,  -167,    52,   241,   222,  -167,   229,   162,  -167,  -167,
    -167,  -167,   215,  -167,   101,   168,   118,  -167,  -167,  -167,
    -167,  -167,  -167,  -167,  -167,  -167,  -167,  -167,  -167,  -167,
    -167,  -167,  -167,  -167,  -167,  -167,  -167,  -167,  -167,  -167,
    -167,  -167,  -167,  -167,  -167,   430,   127,  -167,  -167,  -167,
    -167,   119,  -167,   160,  -167,  -167,   189,  -167,  -167,  -167,
     202,   116,  -167,  -167,    14,  -167,   104,  -167,  -167,  -167,
    -167,  -167,  -167,  -167,   346,  -167,  -167,   229,   162,    96,
     162,  -167,    96,  -167,  -167,  -167,  -167,    96,   238,  -167,
     210,  -167,  -167,  -167,    96,    96,    96,    96,    96,    96,
      96,    96,    96,    96,    96,    96,    96,    96,    96,    96,
      96,    96,    96,    96,    96,    96,    96,    96,   273,    96,
    -167,  -167,  -167,  -167,   398,   150,     5,    34,   185,  -167,
     125,   157,  -167,   154,   169,   217,   218,   333,    99,   152,
     237,    41,  -167,   333,    96,    96,    96,  -167,    96,   162,
      96,    96,    96,  -167,  -167,  -167,  -167,    96,   355,  -167,
     -36,    96,    96,  -167,  -167,  -167,  -167,  -167,  -167,   161,
     110,    34,   232,   288,  -167,   295,   296,   326,  -167,  -167,
    -167,  -167,  -167,  -167,  -167,  -167,  -167,  -167,  -167,  -167,
    -167,   264,    96,  -167,    87,  -167,  -167,  -167,  -167,  -167,
    -167,  -167,  -167,   324,    96,    96,    96,   143,  -167,   171,
    -167,  -167,   325,    88,  -167,   198,   171,   207,   219,  -167,
      34,  -167,  -167,  -167,    31,  -167,  -167,    96,  -167,   274,
      35,  -167,  -167,  -167,  -167,  -167,    97,  -167,  -167,  -167,
    -167,    31,  -167,  -167,  -167,    96,    96,  -167,  -167,  -167,
      79,  -167,  -167,    23,   171,  -167,    96,    27,  -167,  -167,
     213,  -167,  -167,  -167,    96
};

/* YYPGOTO[NTERM-NUM].  */
static const short int yypgoto[] =
{
    -167,  -167,  -167,  -167,  -167,    -1,   -82,  -167,  -167,   163,
    -167,  -167,  -167,  -167,  -167,    82,  -167,    76,  -167,  -167,
      50,  -167,  -167,  -167,  -167,  -166,  -167,   170,   242,   -95,
     310,   -29,   -34,  -167,   175,  -167,   122,    89,  -167,   315,
    -167,   314,  -167,  -167,  -157,  -167,   208,   286,  -167,  -167,
    -167,  -167,    -7,  -167
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -120
static const short int yytable[] =
{
      11,   200,    24,    23,   222,    17,    18,    19,    12,    24,
     167,    26,    27,   123,    28,   118,   206,   120,    24,   187,
      10,   162,    13,    -9,    33,    95,   178,    24,   248,   163,
      -9,    24,   179,    40,    15,    24,   288,   104,    29,    30,
     106,   247,   282,   283,    16,   119,   122,   235,   236,   237,
     238,   239,   127,   107,    97,   207,   134,   135,   136,   137,
     138,   139,   140,   141,   142,   143,   144,   145,   146,   147,
     148,   149,   150,   151,   152,   153,   154,   155,   156,   157,
     158,   159,   231,    24,   123,   298,   123,   211,   189,   301,
     166,   -42,   289,   235,   236,   237,   238,   239,   288,   206,
      24,   177,   128,   240,   241,    91,    13,   183,    24,   108,
     184,   185,   186,    53,    24,    32,   188,   190,    98,   191,
     165,   103,   252,    -9,   192,    -2,   212,   173,   174,   175,
      -9,   197,   198,   133,   303,   295,   260,    34,   207,    35,
      36,    37,    38,    39,    40,    91,   213,   214,   215,   262,
     241,   291,    21,    94,   289,   123,   129,    52,   205,   182,
      24,   201,   202,   121,   226,   176,    -9,   210,    53,    -9,
      99,   105,   265,   100,    -9,    24,    35,    36,    37,    38,
      -9,    -9,   161,   268,   162,   168,   209,    24,   244,    -9,
     171,   216,   163,   245,    25,   246,    -9,    -9,  -119,  -119,
     162,    21,   251,    22,    21,   254,   204,   249,   163,   101,
     102,    21,   225,   256,   172,    21,    -9,   250,   227,    -9,
      -9,    -9,    -9,   131,   132,    34,    -9,    35,    36,    37,
      38,    39,    40,   228,   115,    -9,    35,    36,    37,    38,
     116,   264,   110,    35,    36,    37,    38,   267,   275,   233,
     278,   280,    21,   -57,   274,   -57,   -57,   -57,   -57,   -57,
     -57,    21,   281,   277,   285,   195,   196,   286,   302,    21,
     124,   125,   290,    21,   199,   279,     1,    24,     2,     3,
       4,   229,   230,   293,    57,    58,    21,    59,   294,   255,
     296,   297,    21,   193,   194,   -61,   257,   258,   -61,   111,
     112,   113,   304,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,   110,    82,   259,   261,   272,
     287,    83,   276,   217,    84,   273,   -57,    24,   -57,   -57,
     -57,   -57,   -57,   -57,   299,    34,   180,    35,    36,    37,
      38,    39,    40,   243,   117,   234,   263,   109,    34,    24,
      35,    36,    37,    38,    39,    40,    57,    58,   130,    59,
     292,   160,   203,     0,     0,     0,     0,     0,   -62,     0,
       0,   -62,   111,   112,   113,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,     0,    82,    57,
      58,     0,    59,    83,     0,     0,    84,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
       0,    82,     0,     0,     0,     0,    83,     0,     0,    84,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,     0,    77,    78,    79
};

static const short int yycheck[] =
{
       1,   158,     4,    10,   170,     6,     7,     8,    54,     4,
      92,    12,    13,    47,    15,    44,    11,    46,     4,   114,
       1,    57,    54,     4,    25,    32,    12,     4,    64,    65,
      11,     4,    18,    19,    54,     4,     1,    38,     4,     5,
      41,   198,    11,    12,     0,    46,    47,     6,     7,     8,
       9,    10,    53,     1,    56,    50,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,   177,     4,   118,    62,   120,   169,   117,    62,
      91,    56,    57,     6,     7,     8,     9,    10,     1,    11,
       4,   102,     1,    62,    63,    20,    54,   108,     4,    57,
     111,   112,   113,    12,     4,     1,   117,   118,    19,   120,
       1,    12,    12,     4,   125,     0,     1,    11,    12,    13,
      11,   132,   133,    15,   300,    56,   218,    12,    50,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    62,
      63,    54,    54,    55,    57,   189,    55,     1,   165,    55,
       4,   162,   163,     1,   171,    49,     4,   168,    12,    50,
      12,    19,    29,    15,    12,     4,    14,    15,    16,    17,
      18,    19,    55,    12,    57,    25,     1,     4,   189,     4,
       1,    66,    65,   194,    11,   196,    11,    12,    55,    56,
      57,    54,   209,    56,    54,   212,    56,   208,    65,    51,
      52,    54,    55,   214,    12,    54,    54,    56,    64,    57,
      58,    59,    60,    55,    56,    12,    64,    14,    15,    16,
      17,    18,    19,    64,    12,    50,    14,    15,    16,    17,
      18,   242,     1,    14,    15,    16,    17,   248,   255,    12,
     257,   258,    54,    12,    56,    14,    15,    16,    17,    18,
      19,    54,   263,    56,   265,    55,    56,   268,    55,    54,
      55,    56,   273,    54,     1,    56,    24,     4,    26,    27,
      28,    64,    64,   284,    11,    12,    54,    14,   289,     1,
     291,   292,    54,    55,    56,    54,     1,     1,    57,    58,
      59,    60,   303,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,     1,    53,     1,    64,     4,
      56,    58,   256,   170,    61,   253,    12,     4,    14,    15,
      16,    17,    18,    19,   294,    12,   104,    14,    15,    16,
      17,    18,    19,   183,    44,   180,   234,    42,    12,     4,
      14,    15,    16,    17,    18,    19,    11,    12,    54,    14,
     281,    85,   164,    -1,    -1,    -1,    -1,    -1,    54,    -1,
      -1,    57,    58,    59,    60,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    53,    11,
      12,    -1,    14,    58,    -1,    -1,    61,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    53,    -1,    -1,    -1,    -1,    58,    -1,    -1,    61,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    -1,    47,    48,    49
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,    24,    26,    27,    28,    68,    69,    70,    71,    74,
       1,    72,    54,    54,   106,    54,     0,    72,    72,    72,
      73,    54,    56,   119,     4,    11,    72,    72,    72,     4,
       5,    75,     1,    72,    12,    14,    15,    16,    17,    18,
      19,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     102,   105,     1,    12,   107,   108,   109,    11,    12,    14,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    53,    58,    61,    91,   111,   113,   114,   115,
     116,    20,    76,    77,    55,   119,   120,    56,    19,    12,
      15,    51,    52,    12,    72,    19,    72,     1,    57,   106,
       1,    58,    59,    60,    90,    12,    18,    97,    98,    72,
      98,     1,    72,    99,    55,    56,   119,    72,     1,    55,
     108,    55,    56,    15,    72,    72,    72,    72,    72,    72,
      72,    72,    72,    72,    72,    72,    72,    72,    72,    72,
      72,    72,    72,    72,    72,    72,    72,    72,    72,    72,
     114,    55,    57,    65,   112,     1,    72,    73,    25,    78,
      80,     1,    12,    11,    12,    13,    49,    72,    12,    18,
      95,   101,    55,    72,    72,    72,    72,    96,    72,    98,
      72,    72,    72,    55,    56,    55,    56,    72,    72,     1,
     111,    72,    72,   113,    56,   119,    11,    50,    82,     1,
      72,    73,     1,    21,    22,    23,    66,    76,    81,    85,
      88,    89,    92,   117,   118,    55,   119,    64,    64,    64,
      64,    96,    72,    12,   101,     6,     7,     8,     9,    10,
      62,    63,   103,    94,    72,    72,    72,   111,    64,    72,
      56,   119,    12,    79,   119,     1,    72,     1,     1,     1,
      73,    64,    62,   103,    72,    29,   110,    72,    12,    83,
      84,    87,     4,    82,    56,   119,    84,    56,   119,    56,
     119,    72,    11,    12,   104,    72,    72,    56,     1,    57,
      72,    54,   104,    72,    72,    56,    72,    72,    62,    87,
      86,    62,    55,    92,    72
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
# define YYLLOC_DEFAULT(Current, Rhs, N)		\
   ((Current).first_line   = (Rhs)[1].first_line,	\
    (Current).first_column = (Rhs)[1].first_column,	\
    (Current).last_line    = (Rhs)[N].last_line,	\
    (Current).last_column  = (Rhs)[N].last_column)
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
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_stack_print (short int *bottom, short int *top)
#else
static void
yy_stack_print (bottom, top)
    short int *bottom;
    short int *top;
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
  unsigned int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %u), ",
             yyrule - 1, yylno);
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

#if defined (YYMAXDEPTH) && YYMAXDEPTH == 0
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
      case 83: /* maybe_media_list */

        { delete yyvaluep->mediaList; yyvaluep->mediaList = 0; };

        break;
      case 84: /* media_list */

        { delete yyvaluep->mediaList; yyvaluep->mediaList = 0; };

        break;
      case 86: /* ruleset_list */

        { delete yyvaluep->ruleList; yyvaluep->ruleList = 0; };

        break;
      case 93: /* selector_list */

        { delete yyvaluep->selectorList; yyvaluep->selectorList = 0; };

        break;
      case 94: /* selector */

        { delete yyvaluep->selector; yyvaluep->selector = 0; };

        break;
      case 96: /* simple_selector */

        { delete yyvaluep->selector; yyvaluep->selector = 0; };

        break;
      case 98: /* specifier_list */

        { delete yyvaluep->selector; yyvaluep->selector = 0; };

        break;
      case 99: /* specifier */

        { delete yyvaluep->selector; yyvaluep->selector = 0; };

        break;
      case 100: /* class */

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
  short int yyssa[YYINITDEPTH];
  short int *yyss = yyssa;
  register short int *yyssp;

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
	short int *yyss1 = yyss;


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
	short int *yyss1 = yyss;
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

  case 25:

    {
#ifdef CSS_DEBUG
    kdDebug( 6080 ) << "@namespace: " << qString(yyvsp[-2].string) << endl;
#endif
      CSSParser *p = static_cast<CSSParser *>(parser);
    if (p->styleElement && p->styleElement->isCSSStyleSheet())
        static_cast<CSSStyleSheetImpl*>(p->styleElement)->addNamespace(p, domString(yyvsp[-3].string), domString(yyvsp[-2].string));
 ;}
    break;

  case 28:

    { yyval.string.string = 0; ;}
    break;

  case 29:

    { yyval.string = yyvsp[-1].string; ;}
    break;

  case 31:

    {
     CSSParser *p = static_cast<CSSParser *>(parser);
     if ( yyvsp[-1].rule && p->styleElement && p->styleElement->isCSSStyleSheet() ) {
	 p->styleElement->append( yyvsp[-1].rule );
     } else {
	 delete yyvsp[-1].rule;
     }
 ;}
    break;

  case 38:

    { delete yyvsp[-1].rule; yyval.rule = 0; ;}
    break;

  case 41:

    {
	yyval.mediaList = new MediaListImpl();
    ;}
    break;

  case 43:

    {
	yyval.mediaList = new MediaListImpl();
	yyval.mediaList->appendMedium( domString(yyvsp[0].string).lower() );
    ;}
    break;

  case 44:

    {
	yyval.mediaList = yyvsp[-3].mediaList;
	if (yyval.mediaList)
	    yyval.mediaList->appendMedium( domString(yyvsp[0].string).lower() );
    ;}
    break;

  case 45:

    {
       delete yyvsp[-1].mediaList;
       yyval.mediaList = 0;
    ;}
    break;

  case 46:

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

  case 47:

    { yyval.ruleList = 0; ;}
    break;

  case 48:

    {
      yyval.ruleList = yyvsp[-2].ruleList;
      if ( yyvsp[-1].rule ) {
	  if ( !yyval.ruleList ) yyval.ruleList = new CSSRuleListImpl();
	  yyval.ruleList->append( yyvsp[-1].rule );
      }
  ;}
    break;

  case 49:

    {
      yyval.string = yyvsp[-1].string;
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

    {
      yyval.rule = 0;
    ;}
    break;

  case 54:

    { yyval.relation = CSSSelector::DirectAdjacent; ;}
    break;

  case 55:

    { yyval.relation = CSSSelector::IndirectAdjacent; ;}
    break;

  case 56:

    { yyval.relation = CSSSelector::Child; ;}
    break;

  case 57:

    { yyval.relation = CSSSelector::Descendant; ;}
    break;

  case 58:

    { yyval.val = -1; ;}
    break;

  case 59:

    { yyval.val = 1; ;}
    break;

  case 60:

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

  case 61:

    {
	if ( yyvsp[0].selector ) {
	    yyval.selectorList = new QPtrList<CSSSelector>;
            yyval.selectorList->setAutoDelete( true );
#ifdef CSS_DEBUG
	    kdDebug( 6080 ) << "   got simple selector:" << endl;
	    yyvsp[0].selector->print();
#endif
	    yyval.selectorList->append( yyvsp[0].selector );
	    khtml::CSSStyleSelector::precomputeAttributeDependencies(static_cast<CSSParser *>(parser)->document(), yyvsp[0].selector);
	} else {
	    yyval.selectorList = 0;
	}
    ;}
    break;

  case 62:

    {
	if ( yyvsp[-3].selectorList && yyvsp[0].selector ) {
	    yyval.selectorList = yyvsp[-3].selectorList;
	    yyval.selectorList->append( yyvsp[0].selector );
	    khtml::CSSStyleSelector::precomputeAttributeDependencies(static_cast<CSSParser *>(parser)->document(), yyvsp[0].selector);
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

  case 63:

    {
	delete yyvsp[-1].selectorList;
	yyval.selectorList = 0;
    ;}
    break;

  case 64:

    {
	yyval.selector = yyvsp[0].selector;
    ;}
    break;

  case 65:

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
	}
    ;}
    break;

  case 66:

    {
	delete yyvsp[-1].selector;
	yyval.selector = 0;
    ;}
    break;

  case 67:

    { yyval.string.string = 0; yyval.string.length = 0; ;}
    break;

  case 68:

    { static unsigned short star = '*'; yyval.string.string = &star; yyval.string.length = 1; ;}
    break;

  case 69:

    { yyval.string = yyvsp[-1].string; ;}
    break;

  case 70:

    {
	yyval.selector = new CSSSelector();
	yyval.selector->tag = yyvsp[-1].element;
    ;}
    break;

  case 71:

    {
	yyval.selector = yyvsp[-1].selector;
        if ( yyval.selector )
	    yyval.selector->tag = yyvsp[-2].element;
    ;}
    break;

  case 72:

    {
	yyval.selector = yyvsp[-1].selector;
        if ( yyval.selector )
            yyval.selector->tag = makeId(static_cast<CSSParser*>(parser)->defaultNamespace(), anyLocalName);
    ;}
    break;

  case 73:

    {
        yyval.selector = new CSSSelector();
        yyval.selector->tag = yyvsp[-1].element;
	CSSParser *p = static_cast<CSSParser *>(parser);
        if (p->styleElement && p->styleElement->isCSSStyleSheet())
            static_cast<CSSStyleSheetImpl*>(p->styleElement)->determineNamespace(yyval.selector->tag, domString(yyvsp[-2].string));
    ;}
    break;

  case 74:

    {
        yyval.selector = yyvsp[-1].selector;
        if (yyval.selector) {
            yyval.selector->tag = yyvsp[-2].element;
            CSSParser *p = static_cast<CSSParser *>(parser);
            if (p->styleElement && p->styleElement->isCSSStyleSheet())
                static_cast<CSSStyleSheetImpl*>(p->styleElement)->determineNamespace(yyval.selector->tag, domString(yyvsp[-3].string));
        }
    ;}
    break;

  case 75:

    {
        yyval.selector = yyvsp[-1].selector;
        if (yyval.selector) {
            yyval.selector->tag = makeId(anyNamespace, anyLocalName);
            CSSParser *p = static_cast<CSSParser *>(parser);
            if (p->styleElement && p->styleElement->isCSSStyleSheet())
                static_cast<CSSStyleSheetImpl*>(p->styleElement)->determineNamespace(yyval.selector->tag, domString(yyvsp[-2].string));
        }
    ;}
    break;

  case 76:

    {
	CSSParser *p = static_cast<CSSParser *>(parser);
	DOM::DocumentImpl *doc = p->document();
	QString tag = qString(yyvsp[0].string);
	if ( doc ) {
	    if (doc->isHTMLDocument())
		tag = tag.lower();
	    const DOMString dtag(tag);
            yyval.element = makeId(p->defaultNamespace(), doc->getId(NodeImpl::ElementId, dtag.implementation(), false, true));
	} else {
	    yyval.element = makeId(p->defaultNamespace(), khtml::getTagID(tag.lower().ascii(), tag.length()));
	    // this case should never happen - only when loading
	    // the default stylesheet - which must not contain unknown tags
// 	    assert($$ != 0);
	}
    ;}
    break;

  case 77:

    {
	yyval.element = makeId(static_cast<CSSParser*>(parser)->defaultNamespace(), anyLocalName);
    ;}
    break;

  case 78:

    {
	yyval.selector = yyvsp[0].selector;
	yyval.selector->nonCSSHint = static_cast<CSSParser *>(parser)->nonCSSHint;
    ;}
    break;

  case 79:

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

  case 80:

    {
	delete yyvsp[-1].selector;
	yyval.selector = 0;
    ;}
    break;

  case 81:

    {
	yyval.selector = new CSSSelector();
	yyval.selector->match = CSSSelector::Id;
	yyval.selector->attr = ATTR_ID;
	yyval.selector->value = domString(yyvsp[0].string);
    ;}
    break;

  case 85:

    {
	yyval.selector = new CSSSelector();
	yyval.selector->match = CSSSelector::Class;
	yyval.selector->attr = ATTR_CLASS;
	yyval.selector->value = domString(yyvsp[0].string);
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
	yyval.selector->match = yyvsp[-4].match;
	yyval.selector->value = domString(yyvsp[-2].string);
    ;}
    break;

  case 89:

    {
        yyval.selector = new CSSSelector();
        yyval.selector->attr = yyvsp[-1].attribute;
        yyval.selector->match = CSSSelector::Set;
        CSSParser *p = static_cast<CSSParser *>(parser);
        if (p->styleElement && p->styleElement->isCSSStyleSheet())
            static_cast<CSSStyleSheetImpl*>(p->styleElement)->determineNamespace(yyval.selector->attr, domString(yyvsp[-2].string));
    ;}
    break;

  case 90:

    {
        yyval.selector = new CSSSelector();
        yyval.selector->attr = yyvsp[-5].attribute;
        yyval.selector->match = (CSSSelector::Match)yyvsp[-4].match;
        yyval.selector->value = domString(yyvsp[-2].string);
        CSSParser *p = static_cast<CSSParser *>(parser);
        if (p->styleElement && p->styleElement->isCSSStyleSheet())
            static_cast<CSSStyleSheetImpl*>(p->styleElement)->determineNamespace(yyval.selector->attr, domString(yyvsp[-6].string));
   ;}
    break;

  case 91:

    {
	yyval.match = CSSSelector::Exact;
    ;}
    break;

  case 92:

    {
	yyval.match = CSSSelector::List;
    ;}
    break;

  case 93:

    {
	yyval.match = CSSSelector::Hyphen;
    ;}
    break;

  case 94:

    {
	yyval.match = CSSSelector::Begin;
    ;}
    break;

  case 95:

    {
	yyval.match = CSSSelector::End;
    ;}
    break;

  case 96:

    {
	yyval.match = CSSSelector::Contain;
    ;}
    break;

  case 99:

    {
	yyval.selector = new CSSSelector();
	yyval.selector->match = CSSSelector::PseudoClass;
	yyval.selector->value = domString(yyvsp[0].string);
    ;}
    break;

  case 100:

    {
	yyval.selector = new CSSSelector();
	yyval.selector->match = CSSSelector::PseudoElement;
        yyval.selector->value = domString(yyvsp[0].string);
    ;}
    break;

  case 101:

    {
        yyval.selector = new CSSSelector();
        yyval.selector->match = CSSSelector::PseudoClass;
        yyval.selector->string_arg = domString(yyvsp[-1].string);
        yyval.selector->value = domString(yyvsp[-2].string);
    ;}
    break;

  case 102:

    {
        yyval.selector = new CSSSelector();
        yyval.selector->match = CSSSelector::PseudoClass;
        yyval.selector->string_arg = QString::number(yyvsp[-1].val);
        yyval.selector->value = domString(yyvsp[-2].string);
    ;}
    break;

  case 103:

    {
        yyval.selector = new CSSSelector();
        yyval.selector->match = CSSSelector::PseudoClass;
        yyval.selector->string_arg = domString(yyvsp[-1].string);
        yyval.selector->value = domString(yyvsp[-2].string);
    ;}
    break;

  case 104:

    {
        yyval.selector = new CSSSelector();
        yyval.selector->match = CSSSelector::PseudoClass;
        yyval.selector->string_arg = domString(yyvsp[-1].string);
        yyval.selector->value = domString(yyvsp[-2].string);
    ;}
    break;

  case 105:

    {
        yyval.selector = new CSSSelector();
        yyval.selector->match = CSSSelector::PseudoClass;
        yyval.selector->simpleSelector = yyvsp[-1].selector;
        yyval.selector->value = domString(yyvsp[-3].string);
    ;}
    break;

  case 106:

    {
	yyval.ok = yyvsp[-1].ok;
    ;}
    break;

  case 107:

    {
	yyval.ok = false;
    ;}
    break;

  case 108:

    {
	yyval.ok = yyvsp[-1].ok;
    ;}
    break;

  case 109:

    {
	yyval.ok = yyvsp[-2].ok;
	if ( yyvsp[-1].ok )
	    yyval.ok = yyvsp[-1].ok;
    ;}
    break;

  case 110:

    {
	yyval.ok = yyvsp[-2].ok;
    ;}
    break;

  case 111:

    {
	yyval.ok = yyvsp[-2].ok;
    ;}
    break;

  case 112:

    {
        yyval.ok = false;
    ;}
    break;

  case 113:

    {
	yyval.ok = yyvsp[-3].ok;
	if ( yyvsp[-2].ok )
	    yyval.ok = yyvsp[-2].ok;
    ;}
    break;

  case 114:

    {
        yyval.ok = yyvsp[-3].ok;
    ;}
    break;

  case 115:

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

  case 116:

    {
        yyval.ok = false;
    ;}
    break;

  case 117:

    {
	QString str = qString(yyvsp[-1].string);
	yyval.prop_id = getPropertyID( str.lower().latin1(), str.length() );
    ;}
    break;

  case 118:

    { yyval.b = true; ;}
    break;

  case 119:

    { yyval.b = false; ;}
    break;

  case 120:

    {
	yyval.valueList = new ValueList;
	yyval.valueList->addValue( yyvsp[0].value );
    ;}
    break;

  case 121:

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

  case 122:

    {
	yyval.tok = '/';
    ;}
    break;

  case 123:

    {
	yyval.tok = ',';
    ;}
    break;

  case 124:

    {
        yyval.tok = 0;
  ;}
    break;

  case 125:

    { yyval.value = yyvsp[0].value; ;}
    break;

  case 126:

    { yyval.value = yyvsp[0].value; yyval.value.fValue *= yyvsp[-1].val; ;}
    break;

  case 127:

    { yyval.value.id = 0; yyval.value.string = yyvsp[-1].string; yyval.value.unit = CSSPrimitiveValue::CSS_DIMENSION; ;}
    break;

  case 128:

    { yyval.value.id = 0; yyval.value.string = yyvsp[-1].string; yyval.value.unit = CSSPrimitiveValue::CSS_STRING; ;}
    break;

  case 129:

    {
      QString str = qString( yyvsp[-1].string );
      yyval.value.id = getValueID( str.lower().latin1(), str.length() );
      yyval.value.unit = CSSPrimitiveValue::CSS_IDENT;
      yyval.value.string = yyvsp[-1].string;
  ;}
    break;

  case 130:

    { yyval.value.id = 0; yyval.value.string = yyvsp[-1].string; yyval.value.unit = CSSPrimitiveValue::CSS_URI; ;}
    break;

  case 131:

    { yyval.value.id = 0; yyval.value.iValue = 0; yyval.value.unit = CSSPrimitiveValue::CSS_UNKNOWN;/* ### */ ;}
    break;

  case 132:

    { yyval.value.id = 0; yyval.value.string = yyvsp[0].string; yyval.value.unit = CSSPrimitiveValue::CSS_RGBCOLOR; ;}
    break;

  case 133:

    {
      yyval.value = yyvsp[0].value;
  ;}
    break;

  case 134:

    { yyval.value.id = 0; yyval.value.isInt = true; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_NUMBER; ;}
    break;

  case 135:

    { yyval.value.id = 0; yyval.value.isInt = false; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_NUMBER; ;}
    break;

  case 136:

    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_PERCENTAGE; ;}
    break;

  case 137:

    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_PX; ;}
    break;

  case 138:

    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_CM; ;}
    break;

  case 139:

    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_MM; ;}
    break;

  case 140:

    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_IN; ;}
    break;

  case 141:

    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_PT; ;}
    break;

  case 142:

    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_PC; ;}
    break;

  case 143:

    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_DEG; ;}
    break;

  case 144:

    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_RAD; ;}
    break;

  case 145:

    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_GRAD; ;}
    break;

  case 146:

    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_MS; ;}
    break;

  case 147:

    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_S; ;}
    break;

  case 148:

    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_HZ; ;}
    break;

  case 149:

    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_KHZ; ;}
    break;

  case 150:

    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_EMS; ;}
    break;

  case 151:

    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = Value::Q_EMS; ;}
    break;

  case 152:

    { yyval.value.id = 0; yyval.value.fValue = yyvsp[-1].val; yyval.value.unit = CSSPrimitiveValue::CSS_EXS; ;}
    break;

  case 153:

    {
      Function *f = new Function;
      f->name = yyvsp[-4].string;
      f->args = yyvsp[-2].valueList;
      yyval.value.id = 0;
      yyval.value.unit = Value::Function;
      yyval.value.function = f;
  ;}
    break;

  case 154:

    {
      Function *f = new Function;
      f->name = yyvsp[-2].string;
      f->args = 0;
      yyval.value.id = 0;
      yyval.value.unit = Value::Function;
      yyval.value.function = f;
  ;}
    break;

  case 155:

    { yyval.string = yyvsp[-1].string; ;}
    break;

  case 156:

    {
	yyval.rule = 0;
#ifdef CSS_DEBUG
	kdDebug( 6080 ) << "skipped invalid @-rule" << endl;
#endif
    ;}
    break;

  case 157:

    {
	yyval.rule = 0;
#ifdef CSS_DEBUG
	kdDebug( 6080 ) << "skipped invalid @-rule" << endl;
#endif
    ;}
    break;

  case 158:

    {
	yyval.rule = 0;
#ifdef CSS_DEBUG
	kdDebug( 6080 ) << "skipped invalid rule" << endl;
#endif
    ;}
    break;


    }

/* Line 1010 of yacc.c.  */


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
	  const char* yyprefix;
	  char *yymsg;
	  int yyx;

	  /* Start YYX at -YYN if negative to avoid negative indexes in
	     YYCHECK.  */
	  int yyxbegin = yyn < 0 ? -yyn : 0;

	  /* Stay within bounds of both yycheck and yytname.  */
	  int yychecklim = YYLAST - yyn;
	  int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
	  int yycount = 0;

	  yyprefix = ", expecting ";
	  for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	      {
		yysize += yystrlen (yyprefix) + yystrlen (yytname [yyx]);
		yycount += 1;
		if (yycount == 5)
		  {
		    yysize = 0;
		    break;
		  }
	      }
	  yysize += (sizeof ("syntax error, unexpected ")
		     + yystrlen (yytname[yytype]));
	  yymsg = (char *) YYSTACK_ALLOC (yysize);
	  if (yymsg != 0)
	    {
	      char *yyp = yystpcpy (yymsg, "syntax error, unexpected ");
	      yyp = yystpcpy (yyp, yytname[yytype]);

	      if (yycount < 5)
		{
		  yyprefix = ", expecting ";
		  for (yyx = yyxbegin; yyx < yyxend; ++yyx)
		    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
		      {
			yyp = yystpcpy (yyp, yyprefix);
			yyp = yystpcpy (yyp, yytname[yyx]);
			yyprefix = " or ";
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

      if (yychar <= YYEOF)
        {
          /* If at end of input, pop the error token,
	     then the rest of the stack, then return failure.  */
	  if (yychar == YYEOF)
	     for (;;)
	       {
		 YYPOPSTACK;
		 if (yyssp == yyss)
		   YYABORT;
		 YYDSYMPRINTF ("Error: popping", yystos[*yyssp], yyvsp, yylsp);
		 yydestruct (yystos[*yyssp], yyvsp);
	       }
        }
      else
	{
	  YYDSYMPRINTF ("Error: discarding", yytoken, &yylval, &yylloc);
	  yydestruct (yytoken, &yylval);
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

#ifdef __GNUC__
  /* Pacify GCC when the user code never invokes YYERROR and the label
     yyerrorlab therefore never appears in user code.  */
  if (0)
     goto yyerrorlab;
#endif

  yyvsp -= yylen;
  yyssp -= yylen;
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

      YYDSYMPRINTF ("Error: popping", yystos[*yyssp], yyvsp, yylsp);
      yydestruct (yystos[yystate], yyvsp);
      YYPOPSTACK;
      yystate = *yyssp;
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






