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
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     T_UNIMPORTANT = 258,
     T_CHARACTER_LITERAL = 259,
     T_DOUBLE_LITERAL = 260,
     T_IDENTIFIER = 261,
     T_INTEGER_LITERAL = 262,
     T_STRING_LITERAL = 263,
     T_INCLUDE = 264,
     T_CLASS = 265,
     T_STRUCT = 266,
     T_LEFT_CURLY_BRACKET = 267,
     T_LEFT_PARANTHESIS = 268,
     T_RIGHT_CURLY_BRACKET = 269,
     T_RIGHT_PARANTHESIS = 270,
     T_COLON = 271,
     T_SEMICOLON = 272,
     T_PUBLIC = 273,
     T_PROTECTED = 274,
     T_TRIPE_DOT = 275,
     T_PRIVATE = 276,
     T_VIRTUAL = 277,
     T_CONST = 278,
     T_INLINE = 279,
     T_FRIEND = 280,
     T_RETURN = 281,
     T_SIGNAL = 282,
     T_SLOT = 283,
     T_TYPEDEF = 284,
     T_PLUS = 285,
     T_MINUS = 286,
     T_COMMA = 287,
     T_ASTERISK = 288,
     T_TILDE = 289,
     T_LESS = 290,
     T_GREATER = 291,
     T_AMPERSAND = 292,
     T_EXTERN = 293,
     T_EXTERN_C = 294,
     T_ACCESS = 295,
     T_ENUM = 296,
     T_NAMESPACE = 297,
     T_USING = 298,
     T_UNKNOWN = 299,
     T_TRIPLE_DOT = 300,
     T_TRUE = 301,
     T_FALSE = 302,
     T_STATIC = 303,
     T_MUTABLE = 304,
     T_EQUAL = 305,
     T_SCOPE = 306,
     T_NULL = 307,
     T_INT = 308,
     T_ARRAY_OPEN = 309,
     T_ARRAY_CLOSE = 310,
     T_CHAR = 311,
     T_DCOP = 312,
     T_DCOP_AREA = 313,
     T_DCOP_SIGNAL_AREA = 314,
     T_SIGNED = 315,
     T_UNSIGNED = 316,
     T_LONG = 317,
     T_SHORT = 318,
     T_FUNOPERATOR = 319,
     T_MISCOPERATOR = 320,
     T_SHIFT = 321
   };
#endif
#define T_UNIMPORTANT 258
#define T_CHARACTER_LITERAL 259
#define T_DOUBLE_LITERAL 260
#define T_IDENTIFIER 261
#define T_INTEGER_LITERAL 262
#define T_STRING_LITERAL 263
#define T_INCLUDE 264
#define T_CLASS 265
#define T_STRUCT 266
#define T_LEFT_CURLY_BRACKET 267
#define T_LEFT_PARANTHESIS 268
#define T_RIGHT_CURLY_BRACKET 269
#define T_RIGHT_PARANTHESIS 270
#define T_COLON 271
#define T_SEMICOLON 272
#define T_PUBLIC 273
#define T_PROTECTED 274
#define T_TRIPE_DOT 275
#define T_PRIVATE 276
#define T_VIRTUAL 277
#define T_CONST 278
#define T_INLINE 279
#define T_FRIEND 280
#define T_RETURN 281
#define T_SIGNAL 282
#define T_SLOT 283
#define T_TYPEDEF 284
#define T_PLUS 285
#define T_MINUS 286
#define T_COMMA 287
#define T_ASTERISK 288
#define T_TILDE 289
#define T_LESS 290
#define T_GREATER 291
#define T_AMPERSAND 292
#define T_EXTERN 293
#define T_EXTERN_C 294
#define T_ACCESS 295
#define T_ENUM 296
#define T_NAMESPACE 297
#define T_USING 298
#define T_UNKNOWN 299
#define T_TRIPLE_DOT 300
#define T_TRUE 301
#define T_FALSE 302
#define T_STATIC 303
#define T_MUTABLE 304
#define T_EQUAL 305
#define T_SCOPE 306
#define T_NULL 307
#define T_INT 308
#define T_ARRAY_OPEN 309
#define T_ARRAY_CLOSE 310
#define T_CHAR 311
#define T_DCOP 312
#define T_DCOP_AREA 313
#define T_DCOP_SIGNAL_AREA 314
#define T_SIGNED 315
#define T_UNSIGNED 316
#define T_LONG 317
#define T_SHORT 318
#define T_FUNOPERATOR 319
#define T_MISCOPERATOR 320
#define T_SHIFT 321




/* Copy the first part of user declarations.  */
#line 1 "yacc.yy"

/*****************************************************************
Copyright (c) 1999 Torben Weis <weis@kde.org>
Copyright (c) 2000 Matthias Ettrich <ettrich@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#include <config.h>

// Workaround for a bison issue:
// bison.simple concludes from _GNU_SOURCE that stpcpy is available,
// while GNU string.h only exposes it if __USE_GNU is set.
#ifdef _GNU_SOURCE
#define __USE_GNU 1
#endif

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include <qstring.h>

#define AMP_ENTITY "&amp;"
#define YYERROR_VERBOSE

extern int yylex();

// extern QString idl_lexFile;
extern int idl_line_no;
extern int function_mode;

static int dcop_area = 0;
static int dcop_signal_area = 0;

static QString in_namespace( "" );

void dcopidlInitFlex( const char *_code );

void yyerror( const char *s )
{
	qDebug( "In line %i : %s", idl_line_no, s );
        exit(1);
	//   theParser->parse_error( idl_lexFile, s, idl_line_no );
}



/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
#line 67 "yacc.yy"
typedef union YYSTYPE {
  long   _int;
  QString        *_str;
  unsigned short          _char;
  double _float;
} YYSTYPE;
/* Line 191 of yacc.c.  */
#line 278 "yacc.cc"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 214 of yacc.c.  */
#line 290 "yacc.cc"

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
#define YYFINAL  7
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   514

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  67
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  53
/* YYNRULES -- Number of rules. */
#define YYNRULES  184
/* YYNRULES -- Number of states. */
#define YYNSTATES  371

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   321

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
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
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short yyprhs[] =
{
       0,     0,     3,     7,     8,    11,    16,    17,    19,    20,
      27,    35,    39,    43,    49,    50,    58,    63,    69,    72,
      77,    85,    94,    97,    99,   101,   103,   106,   107,   109,
     111,   113,   115,   117,   119,   121,   122,   126,   129,   132,
     135,   137,   141,   143,   148,   152,   154,   157,   161,   164,
     166,   167,   169,   171,   174,   178,   181,   184,   187,   190,
     193,   196,   202,   207,   212,   217,   224,   229,   236,   243,
     251,   258,   265,   271,   275,   277,   281,   283,   285,   287,
     290,   292,   294,   296,   300,   304,   312,   322,   323,   325,
     327,   330,   332,   335,   338,   342,   345,   349,   352,   356,
     359,   363,   365,   367,   370,   372,   375,   377,   380,   383,
     386,   388,   389,   391,   395,   397,   399,   402,   405,   410,
     417,   421,   423,   426,   428,   432,   436,   439,   442,   446,
     449,   451,   454,   458,   460,   464,   467,   469,   470,   473,
     479,   481,   483,   485,   487,   492,   493,   495,   497,   499,
     501,   503,   505,   512,   520,   522,   526,   527,   532,   534,
     538,   541,   547,   551,   557,   565,   572,   576,   578,   580,
     584,   589,   592,   593,   595,   598,   599,   601,   605,   608,
     611,   615,   621,   627,   633
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const yysigned_char yyrhs[] =
{
      68,     0,    -1,    69,    71,    68,    -1,    -1,     9,    69,
      -1,    39,    12,    68,    14,    -1,    -1,    57,    -1,    -1,
      10,    80,    84,    70,    86,    17,    -1,    10,     6,    80,
      84,    70,    86,    17,    -1,    10,    80,    17,    -1,    11,
      80,    17,    -1,    11,    80,    84,    86,    17,    -1,    -1,
      42,     6,    12,    72,    68,    14,    85,    -1,    43,    42,
       6,    17,    -1,    43,     6,    51,     6,    17,    -1,    38,
      17,    -1,    29,   100,    80,    17,    -1,    29,    11,    12,
      73,    14,    80,    17,    -1,    29,    11,    80,    12,    73,
      14,    80,    17,    -1,    24,   111,    -1,   111,    -1,   119,
      -1,    87,    -1,   119,    73,    -1,    -1,    46,    -1,    47,
      -1,    21,    -1,    19,    -1,    18,    -1,    27,    -1,    28,
      -1,    -1,    75,    76,    16,    -1,    76,    16,    -1,    58,
      16,    -1,    59,    16,    -1,     6,    -1,     6,    51,    80,
      -1,    80,    -1,    80,    35,   101,    36,    -1,   105,    18,
      81,    -1,    81,    -1,    82,    12,    -1,    82,    32,    83,
      -1,    16,    83,    -1,    12,    -1,    -1,    17,    -1,    14,
      -1,    92,    86,    -1,    24,   111,    86,    -1,   111,    86,
      -1,    79,    86,    -1,    87,    86,    -1,    78,    86,    -1,
      77,    86,    -1,   119,    86,    -1,    25,    10,    80,    17,
      86,    -1,    25,    80,    17,    86,    -1,    25,   107,    17,
      86,    -1,    10,    80,    17,    86,    -1,    10,    80,    84,
      86,    17,    86,    -1,    11,    80,    17,    86,    -1,    11,
      80,    84,    86,    17,    86,    -1,    43,     6,    51,     6,
      17,    86,    -1,    41,     6,    12,    88,    14,     6,    17,
      -1,    41,     6,    12,    88,    14,    17,    -1,    41,    12,
      88,    14,     6,    17,    -1,    41,    12,    88,    14,    17,
      -1,    89,    32,    88,    -1,    89,    -1,     6,    50,    91,
      -1,     6,    -1,     4,    -1,     7,    -1,    31,     7,    -1,
      52,    -1,    80,    -1,    90,    -1,    90,    30,    90,    -1,
      90,    66,    90,    -1,    29,    80,    35,   101,    36,    80,
      17,    -1,    29,    80,    35,   101,    36,    51,     6,    80,
      17,    -1,    -1,    23,    -1,    60,    -1,    60,    53,    -1,
      61,    -1,    61,    53,    -1,    60,    63,    -1,    60,    63,
      53,    -1,    60,    62,    -1,    60,    62,    53,    -1,    61,
      63,    -1,    61,    63,    53,    -1,    61,    62,    -1,    61,
      62,    53,    -1,    53,    -1,    62,    -1,    62,    53,    -1,
      63,    -1,    63,    53,    -1,    56,    -1,    60,    56,    -1,
      61,    56,    -1,    33,    95,    -1,    33,    -1,    -1,   102,
      -1,    96,    32,   102,    -1,    94,    -1,    80,    -1,    11,
      80,    -1,    10,    80,    -1,    80,    35,    98,    36,    -1,
      80,    35,    98,    36,    51,    80,    -1,    99,    32,    98,
      -1,    99,    -1,    97,    95,    -1,    97,    -1,    23,    97,
      95,    -1,    23,    97,    37,    -1,    23,    97,    -1,    97,
      23,    -1,    97,    23,    37,    -1,    97,    37,    -1,    97,
      -1,    97,    95,    -1,   100,    32,   101,    -1,   100,    -1,
     100,    80,   103,    -1,   100,   103,    -1,    45,    -1,    -1,
      50,   104,    -1,    50,    13,   100,    15,   104,    -1,     8,
      -1,    91,    -1,     5,    -1,    74,    -1,    80,    13,    96,
      15,    -1,    -1,    22,    -1,    65,    -1,    66,    -1,    36,
      -1,    35,    -1,    50,    -1,   100,    80,    13,    96,    15,
      93,    -1,   100,    64,   106,    13,    96,    15,    93,    -1,
     104,    -1,   104,    32,   108,    -1,    -1,     6,    13,   108,
      15,    -1,   109,    -1,   109,    32,   110,    -1,   107,   113,
      -1,    22,   107,    50,    52,   113,    -1,    22,   107,   113,
      -1,    80,    13,    96,    15,   113,    -1,    80,    13,    96,
      15,    16,   110,   113,    -1,   105,    34,    80,    13,    15,
     113,    -1,    48,   107,   113,    -1,    12,    -1,    17,    -1,
     112,   114,    14,    -1,   112,   114,    14,    17,    -1,   115,
     114,    -1,    -1,    17,    -1,    32,   118,    -1,    -1,     6,
      -1,     6,    50,   104,    -1,    95,     6,    -1,   117,   116,
      -1,   100,   118,    17,    -1,   100,    80,    16,     7,    17,
      -1,    48,   100,     6,   103,    17,    -1,    49,   100,     6,
     103,    17,    -1,   100,     6,    54,    91,    55,    17,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   164,   164,   167,   171,   175,   179,   184,   185,   189,
     198,   207,   210,   213,   217,   216,   228,   231,   234,   237,
     240,   243,   246,   249,   252,   255,   261,   262,   265,   265,
     267,   267,   267,   269,   269,   269,   272,   277,   285,   293,
     305,   308,   316,   322,   331,   335,   342,   346,   354,   358,
     366,   368,   372,   376,   380,   384,   388,   392,   396,   400,
     404,   408,   412,   416,   420,   424,   428,   432,   436,   443,
     444,   445,   446,   450,   451,   455,   456,   460,   461,   462,
     463,   464,   468,   469,   470,   474,   484,   493,   496,   503,
     504,   505,   506,   507,   508,   509,   510,   511,   512,   513,
     514,   515,   516,   517,   518,   519,   520,   521,   522,   526,
     527,   532,   535,   536,   544,   545,   546,   547,   548,   554,
     565,   569,   577,   582,   591,   596,   603,   608,   613,   620,
     625,   630,   638,   642,   649,   658,   666,   676,   678,   681,
     688,   691,   694,   697,   700,   706,   707,   711,   711,   711,
     711,   711,   715,   737,   748,   749,   750,   755,   760,   761,
     765,   769,   773,   777,   783,   789,   795,   808,   815,   816,
     817,   821,   822,   826,   830,   831,   834,   835,   836,   839,
     843,   844,   845,   846,   847
};
#endif

#if YYDEBUG || YYERROR_VERBOSE
/* YYTNME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "T_UNIMPORTANT", "T_CHARACTER_LITERAL", 
  "T_DOUBLE_LITERAL", "T_IDENTIFIER", "T_INTEGER_LITERAL", 
  "T_STRING_LITERAL", "T_INCLUDE", "T_CLASS", "T_STRUCT", 
  "T_LEFT_CURLY_BRACKET", "T_LEFT_PARANTHESIS", "T_RIGHT_CURLY_BRACKET", 
  "T_RIGHT_PARANTHESIS", "T_COLON", "T_SEMICOLON", "T_PUBLIC", 
  "T_PROTECTED", "T_TRIPE_DOT", "T_PRIVATE", "T_VIRTUAL", "T_CONST", 
  "T_INLINE", "T_FRIEND", "T_RETURN", "T_SIGNAL", "T_SLOT", "T_TYPEDEF", 
  "T_PLUS", "T_MINUS", "T_COMMA", "T_ASTERISK", "T_TILDE", "T_LESS", 
  "T_GREATER", "T_AMPERSAND", "T_EXTERN", "T_EXTERN_C", "T_ACCESS", 
  "T_ENUM", "T_NAMESPACE", "T_USING", "T_UNKNOWN", "T_TRIPLE_DOT", 
  "T_TRUE", "T_FALSE", "T_STATIC", "T_MUTABLE", "T_EQUAL", "T_SCOPE", 
  "T_NULL", "T_INT", "T_ARRAY_OPEN", "T_ARRAY_CLOSE", "T_CHAR", "T_DCOP", 
  "T_DCOP_AREA", "T_DCOP_SIGNAL_AREA", "T_SIGNED", "T_UNSIGNED", "T_LONG", 
  "T_SHORT", "T_FUNOPERATOR", "T_MISCOPERATOR", "T_SHIFT", "$accept", 
  "main", "includes", "dcoptag", "declaration", "@1", "member_list", 
  "bool_value", "nodcop_area", "sigslot", "nodcop_area_begin", 
  "dcop_area_begin", "dcop_signal_area_begin", "Identifier", 
  "super_class_name", "super_class", "super_classes", "class_header", 
  "opt_semicolon", "body", "enum", "enum_list", "enum_item", "number", 
  "int_expression", "typedef", "const_qualifier", "int_type", "asterisks", 
  "params", "type_name", "templ_type_list", "templ_type", "type", 
  "type_list", "param", "default", "value", "virtual_qualifier", 
  "operator", "function_header", "values", "init_item", "init_list", 
  "function", "function_begin", "function_body", "function_lines", 
  "function_line", "Identifier_list_rest", "Identifier_list_entry", 
  "Identifier_list", "member", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const unsigned short yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    67,    68,    68,    69,    69,    69,    70,    70,    71,
      71,    71,    71,    71,    72,    71,    71,    71,    71,    71,
      71,    71,    71,    71,    71,    71,    73,    73,    74,    74,
      75,    75,    75,    76,    76,    76,    77,    77,    78,    79,
      80,    80,    81,    81,    82,    82,    83,    83,    84,    84,
      85,    85,    86,    86,    86,    86,    86,    86,    86,    86,
      86,    86,    86,    86,    86,    86,    86,    86,    86,    87,
      87,    87,    87,    88,    88,    89,    89,    90,    90,    90,
      90,    90,    91,    91,    91,    92,    92,    93,    93,    94,
      94,    94,    94,    94,    94,    94,    94,    94,    94,    94,
      94,    94,    94,    94,    94,    94,    94,    94,    94,    95,
      95,    96,    96,    96,    97,    97,    97,    97,    97,    97,
      98,    98,    99,    99,   100,   100,   100,   100,   100,   100,
     100,   100,   101,   101,   102,   102,   102,   103,   103,   103,
     104,   104,   104,   104,   104,   105,   105,   106,   106,   106,
     106,   106,   107,   107,   108,   108,   108,   109,   110,   110,
     111,   111,   111,   111,   111,   111,   111,   112,   113,   113,
     113,   114,   114,   115,   116,   116,   117,   117,   117,   118,
     119,   119,   119,   119,   119
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     3,     0,     2,     4,     0,     1,     0,     6,
       7,     3,     3,     5,     0,     7,     4,     5,     2,     4,
       7,     8,     2,     1,     1,     1,     2,     0,     1,     1,
       1,     1,     1,     1,     1,     0,     3,     2,     2,     2,
       1,     3,     1,     4,     3,     1,     2,     3,     2,     1,
       0,     1,     1,     2,     3,     2,     2,     2,     2,     2,
       2,     5,     4,     4,     4,     6,     4,     6,     6,     7,
       6,     6,     5,     3,     1,     3,     1,     1,     1,     2,
       1,     1,     1,     3,     3,     7,     9,     0,     1,     1,
       2,     1,     2,     2,     3,     2,     3,     2,     3,     2,
       3,     1,     1,     2,     1,     2,     1,     2,     2,     2,
       1,     0,     1,     3,     1,     1,     2,     2,     4,     6,
       3,     1,     2,     1,     3,     3,     2,     2,     3,     2,
       1,     2,     3,     1,     3,     2,     1,     0,     2,     5,
       1,     1,     1,     1,     4,     0,     1,     1,     1,     1,
       1,     1,     6,     7,     1,     3,     0,     4,     1,     3,
       2,     5,     3,     5,     7,     6,     3,     1,     1,     3,
       4,     2,     0,     1,     2,     0,     1,     3,     2,     2,
       3,     5,     5,     5,     6
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       6,     6,     0,     0,   145,     4,     6,     1,    40,     0,
       0,   146,     0,   145,     0,     0,     0,     0,     0,     0,
       0,   101,   106,    89,    91,   102,   104,     6,   115,    25,
     114,   130,     0,     0,     0,    23,    24,     0,     0,    40,
     117,   116,     0,     0,   115,     0,     0,   126,     0,    22,
       0,     0,    18,     0,     0,     0,     0,     0,     0,     0,
       0,    90,   107,    95,    93,    92,   108,    99,    97,   103,
     105,     2,   111,     0,   127,   110,   129,   131,    40,     0,
       0,     0,   175,     0,     0,   167,   168,   172,   160,     5,
      41,     0,    49,   145,    11,     8,    12,    35,   117,   116,
       0,     0,   162,   125,   124,    27,   116,     0,     0,    76,
       0,    74,    14,     0,     0,    40,   166,   137,    96,    94,
     100,    98,   136,     0,   137,   112,   123,     0,   121,   128,
     109,     0,     0,   150,   149,   151,   147,   148,     0,   111,
       0,   178,     0,   179,   180,     0,   173,     0,   172,     8,
     146,    42,    45,     0,    48,     0,     7,    35,     0,     0,
      52,    32,    31,    30,   145,     0,    33,    34,     0,     0,
       0,     0,    35,     0,    35,    35,    35,     0,    35,    35,
      35,    35,     0,     0,     0,     0,    27,    27,    19,     0,
       0,     0,     0,     6,     0,    16,     0,     0,     0,     0,
       0,   137,   135,   122,   118,     0,    77,   142,    78,   140,
       0,    28,    29,    80,   143,    81,    82,   141,   177,    81,
       0,   111,     0,     0,   176,   174,     0,   169,   171,    35,
       0,    46,   145,     0,     0,   117,   116,    35,     0,   115,
       0,     0,     0,    38,    39,     0,    37,    59,    58,    56,
      13,    57,    53,    55,    60,   161,     0,     0,     0,    26,
       0,     0,    75,     0,    72,    73,     0,    17,     0,   138,
     182,   183,     0,   163,   113,   134,     0,   120,    79,   111,
       0,     0,     0,     0,    87,   181,     0,   170,     0,   133,
       0,    47,    44,     9,    35,    35,    35,    35,    54,   117,
      35,    35,     0,     0,    36,   137,     0,     0,     0,    70,
      71,    50,     0,     0,   158,     0,   119,     0,    83,    84,
     184,    87,    88,   152,   165,    10,     0,    43,    64,     0,
      66,     0,    35,    62,    63,     0,     0,    20,     0,    69,
      51,    15,     0,   156,     0,   164,   144,   153,   132,    35,
      35,    61,     0,    35,    21,   139,   154,     0,   159,    65,
      67,     0,     0,    68,   156,   157,     0,    85,   155,     0,
      86
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,     3,     4,   157,    27,   193,   184,   214,   172,   173,
     174,   175,   176,    28,   152,   153,   154,    95,   341,   177,
     178,   110,   111,   216,   217,   179,   323,    30,    81,   123,
      31,   127,   128,    32,   290,   125,   197,   356,    33,   138,
      34,   357,   314,   315,   180,    87,    88,   147,   148,   143,
      82,    83,   181
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -272
static const short yypact[] =
{
      74,    48,    41,    22,   239,  -272,   125,  -272,    43,   121,
     155,   389,   405,   361,   423,   218,   151,   169,    59,   389,
     389,  -272,  -272,   177,   181,   100,   185,    68,    86,  -272,
    -272,   195,    12,   207,   148,  -272,  -272,   242,   155,    40,
     230,   248,   155,   155,   231,    26,   108,   174,   389,  -272,
     192,   155,  -272,   271,   288,   284,   246,   298,    29,   148,
     306,  -272,  -272,   250,   252,  -272,  -272,   265,   266,  -272,
    -272,  -272,   375,   405,   283,   289,  -272,  -272,   163,   119,
     113,   315,   292,   308,   155,  -272,  -272,   309,  -272,  -272,
    -272,   257,  -272,    75,  -272,   272,  -272,   317,  -272,  -272,
     324,   278,  -272,  -272,  -272,    56,   335,   331,   288,   300,
     338,   321,  -272,   343,   337,    21,  -272,   311,  -272,  -272,
    -272,  -272,  -272,   147,    42,  -272,   289,   320,   327,  -272,
    -272,   136,    54,  -272,  -272,  -272,  -272,  -272,   349,   375,
     356,  -272,    53,  -272,  -272,   351,  -272,   354,   309,   272,
    -272,   334,  -272,    76,  -272,   364,  -272,   317,   155,   155,
    -272,  -272,  -272,  -272,   361,   437,  -272,  -272,   155,   368,
     371,   372,   198,   373,   317,   317,   317,   374,   317,   317,
     317,   317,   148,   389,   376,    67,    56,    56,  -272,   378,
      54,   202,   288,   125,   377,  -272,   449,   379,   380,   274,
     375,   311,  -272,  -272,   342,   405,  -272,  -272,  -272,  -272,
     394,  -272,  -272,  -272,  -272,   390,    80,  -272,  -272,  -272,
     347,   375,   158,   387,   355,  -272,   391,   393,  -272,   317,
     389,  -272,    75,   155,   396,   297,   299,   317,   155,   131,
     401,   384,   357,  -272,  -272,   409,  -272,  -272,  -272,  -272,
    -272,  -272,  -272,  -272,  -272,  -272,   420,   155,   411,  -272,
     416,   214,  -272,   415,  -272,  -272,   425,  -272,   389,  -272,
    -272,  -272,   434,  -272,  -272,  -272,   155,  -272,  -272,   375,
      54,    54,   424,   184,   421,  -272,   148,  -272,   442,   431,
     428,  -272,  -272,  -272,   317,   317,   317,   317,  -272,   452,
     317,   317,   389,   464,  -272,   311,   454,   155,   455,  -272,
    -272,   456,   392,   461,   443,   148,  -272,   190,  -272,  -272,
    -272,   421,  -272,  -272,  -272,  -272,   389,  -272,  -272,   460,
    -272,   465,   317,  -272,  -272,   445,   470,  -272,   471,  -272,
    -272,  -272,   136,   136,   434,  -272,  -272,  -272,  -272,   317,
     317,  -272,    44,   317,  -272,  -272,   446,   474,  -272,  -272,
    -272,   485,   475,  -272,   136,  -272,   155,  -272,  -272,   477,
    -272
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -272,    10,   501,   358,  -272,  -272,    89,  -272,  -272,   332,
    -272,  -272,  -272,    -9,   270,  -272,   273,   -35,  -272,   -43,
     502,   -94,  -272,    -2,   -88,  -272,   187,  -272,   -23,  -132,
      -3,   304,  -272,     6,  -271,   310,   -77,  -127,   -80,  -272,
      32,   149,  -272,   167,     8,  -272,   -31,   366,  -272,  -272,
    -272,   370,    23
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -177
static const short yytable[] =
{
      40,    41,    44,    44,   218,    44,    97,   222,    77,    47,
      44,    44,    35,   155,   189,   102,    37,    45,    78,    45,
      51,    49,     7,    80,   104,    58,    60,    36,   116,    90,
      91,   335,     8,    98,    99,   115,   100,    71,  -137,    44,
     198,   106,   107,    46,   220,    75,     8,   202,     8,   100,
       8,    59,   130,     6,    45,   348,   149,     1,   206,   224,
       8,   208,     8,    44,    44,    56,    42,    43,    -3,   269,
     126,   196,    38,    78,    -3,   145,    79,     1,   124,    12,
      59,     8,    -3,     1,   151,   210,    75,     2,   231,   283,
      79,    38,   196,    79,    38,   361,    44,   150,   265,    72,
      75,    57,   262,   203,   183,    20,   213,     2,   232,    21,
     280,   185,    22,     2,   234,   201,    23,    24,    25,    26,
      85,    73,   215,   219,   275,    86,   139,    39,   186,   140,
      44,   247,   248,   249,     1,   251,   252,   253,   254,    -3,
     206,   207,     8,   208,   209,   124,   281,   317,   300,   235,
     236,   255,   155,    69,   133,   134,   239,    53,   101,   241,
      85,     8,   199,    54,     2,    86,    73,   210,   273,   135,
      45,    45,   237,   284,    44,    55,   258,    44,    44,   200,
    -176,   219,   211,   212,   136,   137,   288,   215,   213,   256,
     200,    44,   185,   185,   298,  -176,    44,   240,     8,   321,
     295,   297,   126,   266,   105,   346,   124,    75,   263,   186,
     186,   103,    44,   131,    38,   355,   200,   132,    74,   264,
     308,    44,   200,   151,   151,   166,   167,   124,    75,   299,
      61,   309,    76,    62,    65,    52,   289,    66,    70,    63,
      64,    84,    92,    67,    68,     8,    93,    94,   306,     9,
      10,   328,   329,   330,   331,   324,    89,   333,   334,    44,
      92,    11,    12,    13,    93,    96,    73,   316,    14,    92,
      44,   219,   219,    93,   312,   259,   260,    15,   318,   319,
      16,    17,    18,   108,   345,   124,    85,    19,    20,   351,
     272,    86,    21,    44,   109,    22,   112,   113,   338,    23,
      24,    25,    26,   118,   114,   119,   359,   360,   289,    92,
     363,    92,   117,    93,   294,    93,   296,    44,   120,   121,
     129,   141,    75,     8,   142,   144,   146,   158,   159,   156,
     182,   160,   289,   215,   215,   161,   162,   139,   163,    11,
      12,   164,   165,   362,   166,   167,   168,   187,   188,   194,
     190,  -145,   191,   192,   195,   215,   204,   369,    16,   205,
     169,   196,   221,   223,   226,    19,    20,     8,   227,   230,
      21,    42,    43,    22,   242,   170,   171,    23,    24,    25,
      26,     8,   233,    11,    12,    42,    43,   243,   244,   246,
     257,   250,   261,   276,   267,     8,   270,   271,    12,    42,
      43,   278,   282,   279,   285,   131,   286,   342,   303,    48,
     287,     8,    12,   293,    21,    42,    43,    22,   301,   302,
     122,    23,    24,    25,    26,   304,   305,   140,    21,     8,
     307,    22,   310,    42,    50,    23,    24,    25,    26,   311,
     313,   320,    21,     8,   322,    22,    12,   238,    43,    23,
      24,    25,    26,   206,   207,     8,   208,   209,    21,   325,
      12,    22,   268,   326,   327,    23,    24,    25,    26,   332,
     336,   337,   339,   340,   343,   344,    21,   349,   364,    22,
     210,   352,   350,    23,    24,    25,    26,   353,   354,   365,
      21,   366,   367,    22,   370,   211,   212,    23,    24,    25,
      26,   213,     5,   292,   245,   291,    29,   229,   347,   277,
     274,   358,   225,   368,   228
};

static const unsigned short yycheck[] =
{
       9,    10,    11,    12,   131,    14,    41,   139,    31,    12,
      19,    20,     4,    93,   108,    46,     6,    11,     6,    13,
      14,    13,     0,    32,    47,    19,    20,     4,    59,    38,
      39,   302,     6,    42,    43,     6,    45,    27,    17,    48,
     117,    50,    51,    11,   132,    33,     6,   124,     6,    58,
       6,    19,    75,    12,    48,   326,    91,     9,     4,     6,
       6,     7,     6,    72,    73,     6,    10,    11,     0,   196,
      73,    50,    51,     6,     0,    84,    64,     9,    72,    23,
      48,     6,    14,     9,    93,    31,    33,    39,    12,   221,
      64,    51,    50,    64,    51,    51,   105,    22,   192,    13,
      33,    42,   190,   126,    48,    49,    52,    39,    32,    53,
      30,   105,    56,    39,   157,   124,    60,    61,    62,    63,
      12,    35,   131,   132,   201,    17,    13,     6,   105,    16,
     139,   174,   175,   176,     9,   178,   179,   180,   181,    14,
       4,     5,     6,     7,     8,   139,    66,   279,    17,   158,
     159,   182,   232,    53,    35,    36,   165,     6,    50,   168,
      12,     6,    15,    12,    39,    17,    35,    31,   199,    50,
     164,   165,   164,    15,   183,     6,   185,   186,   187,    32,
      17,   190,    46,    47,    65,    66,   229,   196,    52,   183,
      32,   200,   186,   187,   237,    32,   205,   165,     6,    15,
     235,   236,   205,   193,    12,    15,   200,    33,     6,   186,
     187,    37,   221,    50,    51,   342,    32,    54,    23,    17,
       6,   230,    32,   232,   233,    27,    28,   221,    33,   238,
      53,    17,    37,    56,    53,    17,   230,    56,    53,    62,
      63,    34,    12,    62,    63,     6,    16,    17,   257,    10,
      11,   294,   295,   296,   297,   286,    14,   300,   301,   268,
      12,    22,    23,    24,    16,    17,    35,   276,    29,    12,
     279,   280,   281,    16,   268,   186,   187,    38,   280,   281,
      41,    42,    43,    12,   315,   279,    12,    48,    49,   332,
      16,    17,    53,   302,     6,    56,    12,    51,   307,    60,
      61,    62,    63,    53,     6,    53,   349,   350,   302,    12,
     353,    12,     6,    16,    17,    16,    17,   326,    53,    53,
      37,     6,    33,     6,    32,    17,    17,    10,    11,    57,
      52,    14,   326,   342,   343,    18,    19,    13,    21,    22,
      23,    24,    25,   352,    27,    28,    29,    12,    17,     6,
      50,    34,    14,    32,    17,   364,    36,   366,    41,    32,
      43,    50,    13,     7,    13,    48,    49,     6,    14,    35,
      53,    10,    11,    56,     6,    58,    59,    60,    61,    62,
      63,     6,    18,    22,    23,    10,    11,    16,    16,    16,
      14,    17,    14,    51,    17,     6,    17,    17,    23,    10,
      11,     7,    55,    13,    17,    50,    15,    15,    51,    48,
      17,     6,    23,    17,    53,    10,    11,    56,    17,    35,
      45,    60,    61,    62,    63,    16,     6,    16,    53,     6,
      14,    56,    17,    10,    11,    60,    61,    62,    63,    14,
       6,    17,    53,     6,    23,    56,    23,    10,    11,    60,
      61,    62,    63,     4,     5,     6,     7,     8,    53,    17,
      23,    56,    13,    32,    36,    60,    61,    62,    63,    17,
       6,    17,    17,    17,    13,    32,    53,    17,    32,    56,
      31,    36,    17,    60,    61,    62,    63,    17,    17,    15,
      53,     6,    17,    56,    17,    46,    47,    60,    61,    62,
      63,    52,     1,   233,   172,   232,     4,   149,   321,   205,
     200,   344,   142,   364,   148
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,     9,    39,    68,    69,    69,    12,     0,     6,    10,
      11,    22,    23,    24,    29,    38,    41,    42,    43,    48,
      49,    53,    56,    60,    61,    62,    63,    71,    80,    87,
      94,    97,   100,   105,   107,   111,   119,    68,    51,     6,
      80,    80,    10,    11,    80,   100,   107,    97,    48,   111,
      11,   100,    17,     6,    12,     6,     6,    42,   100,   107,
     100,    53,    56,    62,    63,    53,    56,    62,    63,    53,
      53,    68,    13,    35,    23,    33,    37,    95,     6,    64,
      80,    95,   117,   118,    34,    12,    17,   112,   113,    14,
      80,    80,    12,    16,    17,    84,    17,    84,    80,    80,
      80,    50,   113,    37,    95,    12,    80,    80,    12,     6,
      88,    89,    12,    51,     6,     6,   113,     6,    53,    53,
      53,    53,    45,    96,   100,   102,    97,    98,    99,    37,
      95,    50,    54,    35,    36,    50,    65,    66,   106,    13,
      16,     6,    32,   116,    17,    80,    17,   114,   115,    84,
      22,    80,    81,    82,    83,   105,    57,    70,    10,    11,
      14,    18,    19,    21,    24,    25,    27,    28,    29,    43,
      58,    59,    75,    76,    77,    78,    79,    86,    87,    92,
     111,   119,    52,    48,    73,   100,   119,    12,    17,    88,
      50,    14,    32,    72,     6,    17,    50,   103,   103,    15,
      32,    80,   103,    95,    36,    32,     4,     5,     7,     8,
      31,    46,    47,    52,    74,    80,    90,    91,   104,    80,
      91,    13,    96,     7,     6,   118,    13,    14,   114,    70,
      35,    12,    32,    18,    86,    80,    80,   111,    10,    80,
     107,    80,     6,    16,    16,    76,    16,    86,    86,    86,
      17,    86,    86,    86,    86,   113,   100,    14,    80,    73,
      73,    14,    91,     6,    17,    88,    68,    17,    13,   104,
      17,    17,    16,   113,   102,   103,    51,    98,     7,    13,
      30,    66,    55,    96,    15,    17,    15,    17,    86,   100,
     101,    83,    81,    17,    17,    84,    17,    84,    86,    80,
      17,    17,    35,    51,    16,     6,    80,    14,     6,    17,
      17,    14,   100,     6,   109,   110,    80,    96,    90,    90,
      17,    15,    23,    93,   113,    17,    32,    36,    86,    86,
      86,    86,    17,    86,    86,   101,     6,    17,    80,    17,
      17,    85,    15,    13,    32,   113,    15,    93,   101,    17,
      17,    86,    36,    17,    17,   104,   104,   108,   110,    86,
      86,    51,    80,    86,    32,    15,     6,    17,   108,    80,
      17
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



/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



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
        case 2:
#line 165 "yacc.yy"
    {
	  ;}
    break;

  case 4:
#line 172 "yacc.yy"
    {
		printf("<INCLUDE>%s</INCLUDE>\n", yyvsp[-1]._str->latin1() );
	  ;}
    break;

  case 5:
#line 176 "yacc.yy"
    {
	  ;}
    break;

  case 6:
#line 179 "yacc.yy"
    {
          ;}
    break;

  case 7:
#line 184 "yacc.yy"
    { yyval._int = 1; ;}
    break;

  case 8:
#line 185 "yacc.yy"
    { yyval._int = 0; ;}
    break;

  case 9:
#line 190 "yacc.yy"
    {
	 	if (yyvsp[-2]._int)
			  printf("<CLASS>\n    <NAME>%s</NAME>\n%s%s</CLASS>\n", ( in_namespace + *yyvsp[-4]._str ).latin1(), yyvsp[-3]._str->latin1(), yyvsp[-1]._str->latin1() );
		// default C++ visibility specifier is 'private'
		dcop_area = 0;
		dcop_signal_area = 0;

	  ;}
    break;

  case 10:
#line 199 "yacc.yy"
    {
	 	if (yyvsp[-2]._int)
			  printf("<CLASS>\n    <NAME>%s</NAME>\n    <LINK_SCOPE>%s</LINK_SCOPE>\n%s%s</CLASS>\n", ( in_namespace + *yyvsp[-4]._str ).latin1(),yyvsp[-5]._str->latin1(),  yyvsp[-3]._str->latin1(), yyvsp[-1]._str->latin1() );
		// default C++ visibility specifier is 'private'
		dcop_area = 0;
		dcop_signal_area = 0;

	  ;}
    break;

  case 11:
#line 208 "yacc.yy"
    {
	  ;}
    break;

  case 12:
#line 211 "yacc.yy"
    {
	  ;}
    break;

  case 13:
#line 214 "yacc.yy"
    {
	  ;}
    break;

  case 14:
#line 217 "yacc.yy"
    {
                      in_namespace += *yyvsp[-1]._str; in_namespace += "::";
                  ;}
    break;

  case 15:
#line 221 "yacc.yy"
    {
                      int pos = in_namespace.findRev( "::", -3 );
                      if( pos >= 0 )
                          in_namespace = in_namespace.left( pos + 2 );
                      else
                          in_namespace = "";
                  ;}
    break;

  case 16:
#line 229 "yacc.yy"
    {
          ;}
    break;

  case 17:
#line 232 "yacc.yy"
    {
          ;}
    break;

  case 18:
#line 235 "yacc.yy"
    {
	  ;}
    break;

  case 19:
#line 238 "yacc.yy"
    {
	  ;}
    break;

  case 20:
#line 241 "yacc.yy"
    {
	  ;}
    break;

  case 21:
#line 244 "yacc.yy"
    {
	  ;}
    break;

  case 22:
#line 247 "yacc.yy"
    {
	  ;}
    break;

  case 23:
#line 250 "yacc.yy"
    {
	  ;}
    break;

  case 24:
#line 253 "yacc.yy"
    {
	  ;}
    break;

  case 25:
#line 256 "yacc.yy"
    {
	  ;}
    break;

  case 36:
#line 273 "yacc.yy"
    {
	  dcop_area = 0;
	  dcop_signal_area = 0;
	;}
    break;

  case 37:
#line 278 "yacc.yy"
    {
	  dcop_area = 0;
	  dcop_signal_area = 0;
	;}
    break;

  case 38:
#line 286 "yacc.yy"
    {
	  dcop_area = 1;
	  dcop_signal_area = 0;
	;}
    break;

  case 39:
#line 294 "yacc.yy"
    {
	  /*
	  A dcop signals area needs all dcop area capabilities,
	  e.g. parsing of function parameters.
	  */
	  dcop_area = 1;
	  dcop_signal_area = 1;
	;}
    break;

  case 40:
#line 305 "yacc.yy"
    {
	  yyval._str = yyvsp[0]._str;
	;}
    break;

  case 41:
#line 308 "yacc.yy"
    {
	   QString* tmp = new QString( "%1::%2" );
           *tmp = tmp->arg(*(yyvsp[-2]._str)).arg(*(yyvsp[0]._str));
           yyval._str = tmp;
	;}
    break;

  case 42:
#line 317 "yacc.yy"
    {
		QString* tmp = new QString( "    <SUPER>%1</SUPER>\n" );
		*tmp = tmp->arg( *(yyvsp[0]._str) );
		yyval._str = tmp;
	  ;}
    break;

  case 43:
#line 323 "yacc.yy"
    {
		QString* tmp = new QString( "    <SUPER>%1</SUPER>\n" );
		*tmp = tmp->arg( *(yyvsp[-3]._str) + "&lt;" + *(yyvsp[-1]._str) + "&gt;" );
		yyval._str = tmp;
	  ;}
    break;

  case 44:
#line 332 "yacc.yy"
    {
		yyval._str = yyvsp[0]._str;
	  ;}
    break;

  case 45:
#line 336 "yacc.yy"
    {
		yyval._str = yyvsp[0]._str;
	  ;}
    break;

  case 46:
#line 343 "yacc.yy"
    {
		yyval._str = yyvsp[-1]._str;
	  ;}
    break;

  case 47:
#line 347 "yacc.yy"
    {
		/* $$ = $1; */
		yyval._str = new QString( *(yyvsp[-2]._str) + *(yyvsp[0]._str) );
	  ;}
    break;

  case 48:
#line 355 "yacc.yy"
    {
		yyval._str = yyvsp[0]._str;
	  ;}
    break;

  case 49:
#line 359 "yacc.yy"
    {
		yyval._str = new QString( "" );
	  ;}
    break;

  case 50:
#line 366 "yacc.yy"
    {
          ;}
    break;

  case 52:
#line 373 "yacc.yy"
    {
		yyval._str = new QString( "" );
	  ;}
    break;

  case 53:
#line 377 "yacc.yy"
    {
		yyval._str = new QString( *(yyvsp[-1]._str) + *(yyvsp[0]._str) );
	  ;}
    break;

  case 54:
#line 381 "yacc.yy"
    {
		yyval._str = new QString( *(yyvsp[-1]._str) + *(yyvsp[0]._str) );
	  ;}
    break;

  case 55:
#line 385 "yacc.yy"
    {
		yyval._str = new QString( *(yyvsp[-1]._str) + *(yyvsp[0]._str) );
	  ;}
    break;

  case 56:
#line 389 "yacc.yy"
    {
		yyval._str = yyvsp[0]._str;
	  ;}
    break;

  case 57:
#line 393 "yacc.yy"
    {
		yyval._str = yyvsp[0]._str;
	  ;}
    break;

  case 58:
#line 397 "yacc.yy"
    {
		yyval._str = yyvsp[0]._str;
	  ;}
    break;

  case 59:
#line 401 "yacc.yy"
    {	
	        yyval._str = yyvsp[0]._str;
	  ;}
    break;

  case 60:
#line 405 "yacc.yy"
    {
 	        yyval._str = yyvsp[0]._str;
	  ;}
    break;

  case 61:
#line 409 "yacc.yy"
    {
		yyval._str = yyvsp[0]._str;
	  ;}
    break;

  case 62:
#line 413 "yacc.yy"
    {
		yyval._str = yyvsp[0]._str;
	  ;}
    break;

  case 63:
#line 417 "yacc.yy"
    {
		yyval._str = yyvsp[0]._str;
	  ;}
    break;

  case 64:
#line 421 "yacc.yy"
    {
		yyval._str = yyvsp[0]._str;
	  ;}
    break;

  case 65:
#line 425 "yacc.yy"
    {
                yyval._str = yyvsp[0]._str;
          ;}
    break;

  case 66:
#line 429 "yacc.yy"
    {
		yyval._str = yyvsp[0]._str;
	  ;}
    break;

  case 67:
#line 433 "yacc.yy"
    {
                yyval._str = yyvsp[0]._str;
          ;}
    break;

  case 68:
#line 437 "yacc.yy"
    {
                yyval._str = yyvsp[0]._str;
          ;}
    break;

  case 75:
#line 455 "yacc.yy"
    {;}
    break;

  case 76:
#line 456 "yacc.yy"
    {;}
    break;

  case 77:
#line 460 "yacc.yy"
    {;}
    break;

  case 78:
#line 461 "yacc.yy"
    {;}
    break;

  case 79:
#line 462 "yacc.yy"
    {;}
    break;

  case 80:
#line 463 "yacc.yy"
    {;}
    break;

  case 81:
#line 464 "yacc.yy"
    {;}
    break;

  case 82:
#line 468 "yacc.yy"
    {;}
    break;

  case 83:
#line 469 "yacc.yy"
    {;}
    break;

  case 84:
#line 470 "yacc.yy"
    {;}
    break;

  case 85:
#line 475 "yacc.yy"
    {
		if (dcop_area) {
 		  QString* tmp = new QString("<TYPEDEF name=\"%1\" template=\"%2\"><PARAM %3</TYPEDEF>\n");
		  *tmp = tmp->arg( *(yyvsp[-1]._str) ).arg( *(yyvsp[-5]._str) ).arg( *(yyvsp[-3]._str) );
		  yyval._str = tmp;
		} else {
		  yyval._str = new QString("");
		}
	  ;}
    break;

  case 86:
#line 485 "yacc.yy"
    {
		if (dcop_area)
		  yyerror("scoped template typedefs are not supported in dcop areas!");
	  ;}
    break;

  case 87:
#line 493 "yacc.yy"
    {
		yyval._int = 0;
	  ;}
    break;

  case 88:
#line 497 "yacc.yy"
    {
		yyval._int = 1;
	  ;}
    break;

  case 89:
#line 503 "yacc.yy"
    { yyval._str = new QString("signed int"); ;}
    break;

  case 90:
#line 504 "yacc.yy"
    { yyval._str = new QString("signed int"); ;}
    break;

  case 91:
#line 505 "yacc.yy"
    { yyval._str = new QString("unsigned int"); ;}
    break;

  case 92:
#line 506 "yacc.yy"
    { yyval._str = new QString("unsigned int"); ;}
    break;

  case 93:
#line 507 "yacc.yy"
    { yyval._str = new QString("signed short int"); ;}
    break;

  case 94:
#line 508 "yacc.yy"
    { yyval._str = new QString("signed short int"); ;}
    break;

  case 95:
#line 509 "yacc.yy"
    { yyval._str = new QString("signed long int"); ;}
    break;

  case 96:
#line 510 "yacc.yy"
    { yyval._str = new QString("signed long int"); ;}
    break;

  case 97:
#line 511 "yacc.yy"
    { yyval._str = new QString("unsigned short int"); ;}
    break;

  case 98:
#line 512 "yacc.yy"
    { yyval._str = new QString("unsigned short int"); ;}
    break;

  case 99:
#line 513 "yacc.yy"
    { yyval._str = new QString("unsigned long int"); ;}
    break;

  case 100:
#line 514 "yacc.yy"
    { yyval._str = new QString("unsigned long int"); ;}
    break;

  case 101:
#line 515 "yacc.yy"
    { yyval._str = new QString("int"); ;}
    break;

  case 102:
#line 516 "yacc.yy"
    { yyval._str = new QString("long int"); ;}
    break;

  case 103:
#line 517 "yacc.yy"
    { yyval._str = new QString("long int"); ;}
    break;

  case 104:
#line 518 "yacc.yy"
    { yyval._str = new QString("short int"); ;}
    break;

  case 105:
#line 519 "yacc.yy"
    { yyval._str = new QString("short int"); ;}
    break;

  case 106:
#line 520 "yacc.yy"
    { yyval._str = new QString("char"); ;}
    break;

  case 107:
#line 521 "yacc.yy"
    { yyval._str = new QString("signed char"); ;}
    break;

  case 108:
#line 522 "yacc.yy"
    { yyval._str = new QString("unsigned char"); ;}
    break;

  case 111:
#line 532 "yacc.yy"
    {
		yyval._str = new QString( "" );
	  ;}
    break;

  case 113:
#line 537 "yacc.yy"
    {
		yyval._str = new QString( *(yyvsp[-2]._str) + *(yyvsp[0]._str) );
	  ;}
    break;

  case 115:
#line 545 "yacc.yy"
    { yyval._str = yyvsp[0]._str; ;}
    break;

  case 116:
#line 546 "yacc.yy"
    { yyval._str = yyvsp[0]._str; ;}
    break;

  case 117:
#line 547 "yacc.yy"
    { yyval._str = yyvsp[0]._str; ;}
    break;

  case 118:
#line 548 "yacc.yy"
    {
		QString *tmp = new QString("%1&lt;%2&gt;");
		*tmp = tmp->arg(*(yyvsp[-3]._str));
		*tmp = tmp->arg(*(yyvsp[-1]._str));
		yyval._str = tmp;
	 ;}
    break;

  case 119:
#line 554 "yacc.yy"
    {
		QString *tmp = new QString("%1&lt;%2&gt;::%3");
		*tmp = tmp->arg(*(yyvsp[-5]._str));
		*tmp = tmp->arg(*(yyvsp[-3]._str));
		*tmp = tmp->arg(*(yyvsp[0]._str));
		yyval._str = tmp;
	 ;}
    break;

  case 120:
#line 566 "yacc.yy"
    {
	    yyval._str = new QString(*(yyvsp[-2]._str) + "," + *(yyvsp[0]._str));
	  ;}
    break;

  case 121:
#line 570 "yacc.yy"
    {
 	    yyval._str = yyvsp[0]._str;
	  ;}
    break;

  case 122:
#line 578 "yacc.yy"
    {
	    if (dcop_area)
	      yyerror("in dcop areas are no pointers allowed");
	  ;}
    break;

  case 123:
#line 583 "yacc.yy"
    {
 	    yyval._str = yyvsp[0]._str;
	  ;}
    break;

  case 124:
#line 592 "yacc.yy"
    {
	    if (dcop_area)
	      yyerror("in dcop areas are no pointers allowed");
	  ;}
    break;

  case 125:
#line 596 "yacc.yy"
    {
	     if (dcop_area) {
	  	QString* tmp = new QString("<TYPE  qleft=\"const\" qright=\"" AMP_ENTITY "\">%1</TYPE>");
		*tmp = tmp->arg( *(yyvsp[-1]._str) );
		yyval._str = tmp;
	     }
	  ;}
    break;

  case 126:
#line 603 "yacc.yy"
    {
		QString* tmp = new QString("<TYPE>%1</TYPE>");
		*tmp = tmp->arg( *(yyvsp[0]._str) );
		yyval._str = tmp;
	;}
    break;

  case 127:
#line 608 "yacc.yy"
    {
        QString* tmp = new QString("<TYPE>%1</TYPE>");
        *tmp = tmp->arg( *(yyvsp[-1]._str) );
        yyval._str = tmp;
    ;}
    break;

  case 128:
#line 613 "yacc.yy"
    { 
        if (dcop_area) { 
           QString* tmp = new QString("<TYPE  qleft=\"const\" qright=\"" AMP_ENTITY "\">%1</TYPE>"); 
           *tmp = tmp->arg( *(yyvsp[-2]._str) ); 
           yyval._str = tmp; 
        } 
    ;}
    break;

  case 129:
#line 620 "yacc.yy"
    {
	     if (dcop_area)
		yyerror("in dcop areas are only const references allowed!");
	  ;}
    break;

  case 130:
#line 625 "yacc.yy"
    {
		QString* tmp = new QString("<TYPE>%1</TYPE>");
		*tmp = tmp->arg( *(yyvsp[0]._str) );
		yyval._str = tmp;
	;}
    break;

  case 131:
#line 631 "yacc.yy"
    {
	    if (dcop_area)
	      yyerror("in dcop areas are no pointers allowed");
	  ;}
    break;

  case 132:
#line 639 "yacc.yy"
    {
	    yyval._str = new QString(*(yyvsp[-2]._str) + "," + *(yyvsp[0]._str));
	  ;}
    break;

  case 133:
#line 643 "yacc.yy"
    {
 	    yyval._str = yyvsp[0]._str;
	  ;}
    break;

  case 134:
#line 650 "yacc.yy"
    {
		if (dcop_area) {
		   QString* tmp = new QString("\n        <ARG>%1<NAME>%2</NAME></ARG>");
  		   *tmp = tmp->arg( *(yyvsp[-2]._str) );
  		   *tmp = tmp->arg( *(yyvsp[-1]._str) );
		   yyval._str = tmp;		
		} else yyval._str = new QString();
	  ;}
    break;

  case 135:
#line 659 "yacc.yy"
    {
		if (dcop_area) {
		   QString* tmp = new QString("\n        <ARG>%1</ARG>");
  		   *tmp = tmp->arg( *(yyvsp[-1]._str) );
		   yyval._str = tmp;		
		} else yyval._str = new QString();
	  ;}
    break;

  case 136:
#line 667 "yacc.yy"
    {
		if (dcop_area)
			yyerror("variable arguments not supported in dcop area.");
		yyval._str = new QString("");
	  ;}
    break;

  case 137:
#line 676 "yacc.yy"
    {
	  ;}
    break;

  case 138:
#line 679 "yacc.yy"
    {
	  ;}
    break;

  case 139:
#line 682 "yacc.yy"
    {
	  ;}
    break;

  case 140:
#line 689 "yacc.yy"
    {
          ;}
    break;

  case 141:
#line 692 "yacc.yy"
    {
          ;}
    break;

  case 142:
#line 695 "yacc.yy"
    {
          ;}
    break;

  case 143:
#line 698 "yacc.yy"
    {
          ;}
    break;

  case 144:
#line 701 "yacc.yy"
    {
          ;}
    break;

  case 145:
#line 706 "yacc.yy"
    { yyval._int = 0; ;}
    break;

  case 146:
#line 707 "yacc.yy"
    { yyval._int = 1; ;}
    break;

  case 152:
#line 716 "yacc.yy"
    {
	     if (dcop_area || dcop_signal_area) {
		QString* tmp = 0;
                tmp = new QString(
                        "    <%4>\n"
                        "        %2\n"
                        "        <NAME>%1</NAME>"
                        "%3\n"
                        "     </%5>\n");
		*tmp = tmp->arg( *(yyvsp[-4]._str) );
		*tmp = tmp->arg( *(yyvsp[-5]._str) );
                *tmp = tmp->arg( *(yyvsp[-2]._str) );
                
                QString tagname = (dcop_signal_area) ? "SIGNAL" : "FUNC";
                QString attr = (yyvsp[0]._int) ? " qual=\"const\"" : "";
                *tmp = tmp->arg( QString("%1%2").arg(tagname).arg(attr) );
                *tmp = tmp->arg( QString("%1").arg(tagname) );
		yyval._str = tmp;
   	     } else
	        yyval._str = new QString("");
	  ;}
    break;

  case 153:
#line 738 "yacc.yy"
    {
	     if (dcop_area)
		yyerror("operators aren't allowed in dcop areas!");
	     yyval._str = new QString("");
	  ;}
    break;

  case 154:
#line 748 "yacc.yy"
    {;}
    break;

  case 155:
#line 749 "yacc.yy"
    {;}
    break;

  case 156:
#line 750 "yacc.yy"
    {;}
    break;

  case 157:
#line 755 "yacc.yy"
    {;}
    break;

  case 158:
#line 760 "yacc.yy"
    {;}
    break;

  case 159:
#line 761 "yacc.yy"
    {;}
    break;

  case 160:
#line 766 "yacc.yy"
    {
	        yyval._str = yyvsp[-1]._str;
	  ;}
    break;

  case 161:
#line 770 "yacc.yy"
    {
		yyval._str = yyvsp[-3]._str;
	  ;}
    break;

  case 162:
#line 774 "yacc.yy"
    {
		yyval._str = yyvsp[-1]._str;
	  ;}
    break;

  case 163:
#line 778 "yacc.yy"
    {
	      /* The constructor */
	      assert(!dcop_area);
              yyval._str = new QString("");
	  ;}
    break;

  case 164:
#line 784 "yacc.yy"
    {
	      /* The constructor */
	      assert(!dcop_area);
              yyval._str = new QString("");
	  ;}
    break;

  case 165:
#line 790 "yacc.yy"
    {
	      /* The destructor */
  	      assert(!dcop_area);
              yyval._str = new QString("");
	  ;}
    break;

  case 166:
#line 796 "yacc.yy"
    {
              if (dcop_area) {
                 if (dcop_signal_area)
                     yyerror("DCOP signals cannot be static");
                 else
                     yyerror("DCOP functions cannot be static");
              } else {
                 yyval._str = new QString();
              }  
	  ;}
    break;

  case 167:
#line 809 "yacc.yy"
    {
		function_mode = 1;
	;}
    break;

  case 171:
#line 821 "yacc.yy"
    {;}
    break;

  case 172:
#line 822 "yacc.yy"
    {;}
    break;

  case 176:
#line 834 "yacc.yy"
    {;}
    break;

  case 177:
#line 835 "yacc.yy"
    {;}
    break;

  case 178:
#line 836 "yacc.yy"
    {;}
    break;

  case 179:
#line 839 "yacc.yy"
    {;}
    break;

  case 180:
#line 843 "yacc.yy"
    {;}
    break;

  case 181:
#line 844 "yacc.yy"
    {;}
    break;

  case 182:
#line 845 "yacc.yy"
    {;}
    break;

  case 183:
#line 846 "yacc.yy"
    {;}
    break;

  case 184:
#line 847 "yacc.yy"
    {;}
    break;


    }

/* Line 991 of yacc.c.  */
#line 2567 "yacc.cc"

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


/*----------------------------------------------------.
| yyerrlab1 -- error raised explicitly by an action.  |
`----------------------------------------------------*/
yyerrlab1:

  /* Suppress GCC warning that yyerrlab1 is unused when no action
     invokes YYERROR.  */
#if defined (__GNUC_MINOR__) && 2093 <= (__GNUC__ * 1000 + __GNUC_MINOR__) \
    && !defined __cplusplus
  __attribute__ ((__unused__))
#endif


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


#line 850 "yacc.yy"


void dcopidlParse( const char *_code )
{
    dcopidlInitFlex( _code );
    yyparse();
}

