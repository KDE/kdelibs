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
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     T_CHARACTER_LITERAL = 258,
     T_DOUBLE_LITERAL = 259,
     T_IDENTIFIER = 260,
     T_INTEGER_LITERAL = 261,
     T_STRING_LITERAL = 262,
     T_INCLUDE = 263,
     T_CLASS = 264,
     T_STRUCT = 265,
     T_LEFT_CURLY_BRACKET = 266,
     T_LEFT_PARANTHESIS = 267,
     T_RIGHT_CURLY_BRACKET = 268,
     T_RIGHT_PARANTHESIS = 269,
     T_COLON = 270,
     T_SEMICOLON = 271,
     T_PUBLIC = 272,
     T_PROTECTED = 273,
     T_TRIPE_DOT = 274,
     T_PRIVATE = 275,
     T_VIRTUAL = 276,
     T_CONST = 277,
     T_INLINE = 278,
     T_FRIEND = 279,
     T_RETURN = 280,
     T_SIGNAL = 281,
     T_SLOT = 282,
     T_TYPEDEF = 283,
     T_PLUS = 284,
     T_MINUS = 285,
     T_COMMA = 286,
     T_ASTERISK = 287,
     T_TILDE = 288,
     T_LESS = 289,
     T_GREATER = 290,
     T_AMPERSAND = 291,
     T_EXTERN = 292,
     T_EXTERN_C = 293,
     T_ACCESS = 294,
     T_ENUM = 295,
     T_NAMESPACE = 296,
     T_USING = 297,
     T_UNKNOWN = 298,
     T_TRIPLE_DOT = 299,
     T_TRUE = 300,
     T_FALSE = 301,
     T_STATIC = 302,
     T_MUTABLE = 303,
     T_EQUAL = 304,
     T_SCOPE = 305,
     T_NULL = 306,
     T_INT = 307,
     T_ARRAY_OPEN = 308,
     T_ARRAY_CLOSE = 309,
     T_CHAR = 310,
     T_DCOP = 311,
     T_DCOP_AREA = 312,
     T_DCOP_SIGNAL_AREA = 313,
     T_SIGNED = 314,
     T_UNSIGNED = 315,
     T_LONG = 316,
     T_SHORT = 317,
     T_FUNOPERATOR = 318,
     T_MISCOPERATOR = 319,
     T_SHIFT = 320
   };
#endif
#define T_CHARACTER_LITERAL 258
#define T_DOUBLE_LITERAL 259
#define T_IDENTIFIER 260
#define T_INTEGER_LITERAL 261
#define T_STRING_LITERAL 262
#define T_INCLUDE 263
#define T_CLASS 264
#define T_STRUCT 265
#define T_LEFT_CURLY_BRACKET 266
#define T_LEFT_PARANTHESIS 267
#define T_RIGHT_CURLY_BRACKET 268
#define T_RIGHT_PARANTHESIS 269
#define T_COLON 270
#define T_SEMICOLON 271
#define T_PUBLIC 272
#define T_PROTECTED 273
#define T_TRIPE_DOT 274
#define T_PRIVATE 275
#define T_VIRTUAL 276
#define T_CONST 277
#define T_INLINE 278
#define T_FRIEND 279
#define T_RETURN 280
#define T_SIGNAL 281
#define T_SLOT 282
#define T_TYPEDEF 283
#define T_PLUS 284
#define T_MINUS 285
#define T_COMMA 286
#define T_ASTERISK 287
#define T_TILDE 288
#define T_LESS 289
#define T_GREATER 290
#define T_AMPERSAND 291
#define T_EXTERN 292
#define T_EXTERN_C 293
#define T_ACCESS 294
#define T_ENUM 295
#define T_NAMESPACE 296
#define T_USING 297
#define T_UNKNOWN 298
#define T_TRIPLE_DOT 299
#define T_TRUE 300
#define T_FALSE 301
#define T_STATIC 302
#define T_MUTABLE 303
#define T_EQUAL 304
#define T_SCOPE 305
#define T_NULL 306
#define T_INT 307
#define T_ARRAY_OPEN 308
#define T_ARRAY_CLOSE 309
#define T_CHAR 310
#define T_DCOP 311
#define T_DCOP_AREA 312
#define T_DCOP_SIGNAL_AREA 313
#define T_SIGNED 314
#define T_UNSIGNED 315
#define T_LONG 316
#define T_SHORT 317
#define T_FUNOPERATOR 318
#define T_MISCOPERATOR 319
#define T_SHIFT 320




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
#line 277 "yacc.cc"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 214 of yacc.c.  */
#line 289 "yacc.cc"

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
#define YYLAST   556

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  66
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  53
/* YYNRULES -- Number of rules. */
#define YYNRULES  182
/* YYNRULES -- Number of states. */
#define YYNSTATES  369

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   320

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
      65
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
     417,   421,   423,   426,   428,   432,   436,   439,   442,   444,
     447,   451,   453,   457,   460,   462,   463,   466,   472,   474,
     476,   478,   480,   485,   486,   488,   490,   492,   494,   496,
     498,   505,   513,   515,   519,   520,   525,   527,   531,   534,
     540,   544,   550,   558,   565,   569,   571,   573,   577,   582,
     585,   586,   588,   591,   592,   594,   598,   601,   604,   608,
     614,   620,   626
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const yysigned_char yyrhs[] =
{
      67,     0,    -1,    68,    70,    67,    -1,    -1,     8,    68,
      -1,    38,    11,    67,    13,    -1,    -1,    56,    -1,    -1,
       9,    79,    83,    69,    85,    16,    -1,     9,     5,    79,
      83,    69,    85,    16,    -1,     9,    79,    16,    -1,    10,
      79,    16,    -1,    10,    79,    83,    85,    16,    -1,    -1,
      41,     5,    11,    71,    67,    13,    84,    -1,    42,    41,
       5,    16,    -1,    42,     5,    50,     5,    16,    -1,    37,
      16,    -1,    28,    99,    79,    16,    -1,    28,    10,    11,
      72,    13,    79,    16,    -1,    28,    10,    79,    11,    72,
      13,    79,    16,    -1,    23,   110,    -1,   110,    -1,   118,
      -1,    86,    -1,   118,    72,    -1,    -1,    45,    -1,    46,
      -1,    20,    -1,    18,    -1,    17,    -1,    26,    -1,    27,
      -1,    -1,    74,    75,    15,    -1,    75,    15,    -1,    57,
      15,    -1,    58,    15,    -1,     5,    -1,     5,    50,    79,
      -1,    79,    -1,    79,    34,   100,    35,    -1,   104,    17,
      80,    -1,    80,    -1,    81,    11,    -1,    81,    31,    82,
      -1,    15,    82,    -1,    11,    -1,    -1,    16,    -1,    13,
      -1,    91,    85,    -1,    23,   110,    85,    -1,   110,    85,
      -1,    78,    85,    -1,    86,    85,    -1,    77,    85,    -1,
      76,    85,    -1,   118,    85,    -1,    24,     9,    79,    16,
      85,    -1,    24,    79,    16,    85,    -1,    24,   106,    16,
      85,    -1,     9,    79,    16,    85,    -1,     9,    79,    83,
      85,    16,    85,    -1,    10,    79,    16,    85,    -1,    10,
      79,    83,    85,    16,    85,    -1,    42,     5,    50,     5,
      16,    85,    -1,    40,     5,    11,    87,    13,     5,    16,
      -1,    40,     5,    11,    87,    13,    16,    -1,    40,    11,
      87,    13,     5,    16,    -1,    40,    11,    87,    13,    16,
      -1,    88,    31,    87,    -1,    88,    -1,     5,    49,    90,
      -1,     5,    -1,     3,    -1,     6,    -1,    30,     6,    -1,
      51,    -1,    79,    -1,    89,    -1,    89,    29,    89,    -1,
      89,    65,    89,    -1,    28,    79,    34,   100,    35,    79,
      16,    -1,    28,    79,    34,   100,    35,    50,     5,    79,
      16,    -1,    -1,    22,    -1,    59,    -1,    59,    52,    -1,
      60,    -1,    60,    52,    -1,    59,    62,    -1,    59,    62,
      52,    -1,    59,    61,    -1,    59,    61,    52,    -1,    60,
      62,    -1,    60,    62,    52,    -1,    60,    61,    -1,    60,
      61,    52,    -1,    52,    -1,    61,    -1,    61,    52,    -1,
      62,    -1,    62,    52,    -1,    55,    -1,    59,    55,    -1,
      60,    55,    -1,    32,    94,    -1,    32,    -1,    -1,   101,
      -1,    95,    31,   101,    -1,    93,    -1,    79,    -1,    10,
      79,    -1,     9,    79,    -1,    79,    34,    97,    35,    -1,
      79,    34,    97,    35,    50,    79,    -1,    98,    31,    97,
      -1,    98,    -1,    96,    94,    -1,    96,    -1,    22,    96,
      94,    -1,    22,    96,    36,    -1,    22,    96,    -1,    96,
      36,    -1,    96,    -1,    96,    94,    -1,    99,    31,   100,
      -1,    99,    -1,    99,    79,   102,    -1,    99,   102,    -1,
      44,    -1,    -1,    49,   103,    -1,    49,    12,    99,    14,
     103,    -1,     7,    -1,    90,    -1,     4,    -1,    73,    -1,
      79,    12,    95,    14,    -1,    -1,    21,    -1,    64,    -1,
      65,    -1,    35,    -1,    34,    -1,    49,    -1,    99,    79,
      12,    95,    14,    92,    -1,    99,    63,   105,    12,    95,
      14,    92,    -1,   103,    -1,   103,    31,   107,    -1,    -1,
       5,    12,   107,    14,    -1,   108,    -1,   108,    31,   109,
      -1,   106,   112,    -1,    21,   106,    49,    51,   112,    -1,
      21,   106,   112,    -1,    79,    12,    95,    14,   112,    -1,
      79,    12,    95,    14,    15,   109,   112,    -1,   104,    33,
      79,    12,    14,   112,    -1,    47,   106,   112,    -1,    11,
      -1,    16,    -1,   111,   113,    13,    -1,   111,   113,    13,
      16,    -1,   114,   113,    -1,    -1,    16,    -1,    31,   117,
      -1,    -1,     5,    -1,     5,    49,   103,    -1,    94,     5,
      -1,   116,   115,    -1,    99,   117,    16,    -1,    99,    79,
      15,     6,    16,    -1,    47,    99,     5,   102,    16,    -1,
      48,    99,     5,   102,    16,    -1,    99,     5,    53,    90,
      54,    16,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   163,   163,   166,   170,   174,   178,   183,   184,   188,
     197,   206,   209,   212,   216,   215,   227,   230,   233,   236,
     239,   242,   245,   248,   251,   254,   260,   261,   264,   264,
     266,   266,   266,   268,   268,   268,   271,   276,   284,   292,
     304,   307,   315,   321,   330,   334,   341,   345,   353,   357,
     365,   367,   371,   375,   379,   383,   387,   391,   395,   399,
     403,   407,   411,   415,   419,   423,   427,   431,   435,   442,
     443,   444,   445,   449,   450,   454,   455,   459,   460,   461,
     462,   463,   467,   468,   469,   473,   483,   492,   495,   502,
     503,   504,   505,   506,   507,   508,   509,   510,   511,   512,
     513,   514,   515,   516,   517,   518,   519,   520,   521,   525,
     526,   531,   534,   535,   543,   544,   545,   546,   547,   553,
     564,   568,   576,   581,   590,   595,   602,   607,   612,   617,
     625,   629,   636,   645,   653,   663,   665,   668,   675,   678,
     681,   684,   687,   693,   694,   698,   698,   698,   698,   698,
     702,   724,   735,   736,   737,   742,   747,   748,   752,   756,
     760,   764,   770,   776,   782,   795,   802,   803,   804,   808,
     809,   813,   817,   818,   821,   822,   823,   826,   830,   831,
     832,   833,   834
};
#endif

#if YYDEBUG || YYERROR_VERBOSE
/* YYTNME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "T_CHARACTER_LITERAL", "T_DOUBLE_LITERAL", 
  "T_IDENTIFIER", "T_INTEGER_LITERAL", "T_STRING_LITERAL", "T_INCLUDE", 
  "T_CLASS", "T_STRUCT", "T_LEFT_CURLY_BRACKET", "T_LEFT_PARANTHESIS", 
  "T_RIGHT_CURLY_BRACKET", "T_RIGHT_PARANTHESIS", "T_COLON", 
  "T_SEMICOLON", "T_PUBLIC", "T_PROTECTED", "T_TRIPE_DOT", "T_PRIVATE", 
  "T_VIRTUAL", "T_CONST", "T_INLINE", "T_FRIEND", "T_RETURN", "T_SIGNAL", 
  "T_SLOT", "T_TYPEDEF", "T_PLUS", "T_MINUS", "T_COMMA", "T_ASTERISK", 
  "T_TILDE", "T_LESS", "T_GREATER", "T_AMPERSAND", "T_EXTERN", 
  "T_EXTERN_C", "T_ACCESS", "T_ENUM", "T_NAMESPACE", "T_USING", 
  "T_UNKNOWN", "T_TRIPLE_DOT", "T_TRUE", "T_FALSE", "T_STATIC", 
  "T_MUTABLE", "T_EQUAL", "T_SCOPE", "T_NULL", "T_INT", "T_ARRAY_OPEN", 
  "T_ARRAY_CLOSE", "T_CHAR", "T_DCOP", "T_DCOP_AREA", 
  "T_DCOP_SIGNAL_AREA", "T_SIGNED", "T_UNSIGNED", "T_LONG", "T_SHORT", 
  "T_FUNOPERATOR", "T_MISCOPERATOR", "T_SHIFT", "$accept", "main", 
  "includes", "dcoptag", "declaration", "@1", "member_list", "bool_value", 
  "nodcop_area", "sigslot", "nodcop_area_begin", "dcop_area_begin", 
  "dcop_signal_area_begin", "Identifier", "super_class_name", 
  "super_class", "super_classes", "class_header", "opt_semicolon", "body", 
  "enum", "enum_list", "enum_item", "number", "int_expression", "typedef", 
  "const_qualifier", "int_type", "asterisks", "params", "type_name", 
  "templ_type_list", "templ_type", "type", "type_list", "param", 
  "default", "value", "virtual_qualifier", "operator", "function_header", 
  "values", "init_item", "init_list", "function", "function_begin", 
  "function_body", "function_lines", "function_line", 
  "Identifier_list_rest", "Identifier_list_entry", "Identifier_list", 
  "member", 0
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
     315,   316,   317,   318,   319,   320
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    66,    67,    67,    68,    68,    68,    69,    69,    70,
      70,    70,    70,    70,    71,    70,    70,    70,    70,    70,
      70,    70,    70,    70,    70,    70,    72,    72,    73,    73,
      74,    74,    74,    75,    75,    75,    76,    76,    77,    78,
      79,    79,    80,    80,    81,    81,    82,    82,    83,    83,
      84,    84,    85,    85,    85,    85,    85,    85,    85,    85,
      85,    85,    85,    85,    85,    85,    85,    85,    85,    86,
      86,    86,    86,    87,    87,    88,    88,    89,    89,    89,
      89,    89,    90,    90,    90,    91,    91,    92,    92,    93,
      93,    93,    93,    93,    93,    93,    93,    93,    93,    93,
      93,    93,    93,    93,    93,    93,    93,    93,    93,    94,
      94,    95,    95,    95,    96,    96,    96,    96,    96,    96,
      97,    97,    98,    98,    99,    99,    99,    99,    99,    99,
     100,   100,   101,   101,   101,   102,   102,   102,   103,   103,
     103,   103,   103,   104,   104,   105,   105,   105,   105,   105,
     106,   106,   107,   107,   107,   108,   109,   109,   110,   110,
     110,   110,   110,   110,   110,   111,   112,   112,   112,   113,
     113,   114,   115,   115,   116,   116,   116,   117,   118,   118,
     118,   118,   118
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
       3,     1,     2,     1,     3,     3,     2,     2,     1,     2,
       3,     1,     3,     2,     1,     0,     2,     5,     1,     1,
       1,     1,     4,     0,     1,     1,     1,     1,     1,     1,
       6,     7,     1,     3,     0,     4,     1,     3,     2,     5,
       3,     5,     7,     6,     3,     1,     1,     3,     4,     2,
       0,     1,     2,     0,     1,     3,     2,     2,     3,     5,
       5,     5,     6
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       6,     6,     0,     0,   143,     4,     6,     1,    40,     0,
       0,   144,     0,   143,     0,     0,     0,     0,     0,     0,
       0,   101,   106,    89,    91,   102,   104,     6,   115,    25,
     114,   128,     0,     0,     0,    23,    24,     0,     0,    40,
     117,   116,     0,     0,   115,     0,     0,   126,     0,    22,
       0,     0,    18,     0,     0,     0,     0,     0,     0,     0,
       0,    90,   107,    95,    93,    92,   108,    99,    97,   103,
     105,     2,   111,     0,   110,   127,   129,    40,     0,     0,
       0,   173,     0,     0,   165,   166,   170,   158,     5,    41,
       0,    49,   143,    11,     8,    12,    35,   117,   116,     0,
       0,   160,   125,   124,    27,   116,     0,     0,    76,     0,
      74,    14,     0,     0,    40,   164,   135,    96,    94,   100,
      98,   134,     0,   135,   112,   123,     0,   121,   109,     0,
       0,   148,   147,   149,   145,   146,     0,   111,     0,   176,
       0,   177,   178,     0,   171,     0,   170,     8,   144,    42,
      45,     0,    48,     0,     7,    35,     0,     0,    52,    32,
      31,    30,   143,     0,    33,    34,     0,     0,     0,     0,
      35,     0,    35,    35,    35,     0,    35,    35,    35,    35,
       0,     0,     0,     0,    27,    27,    19,     0,     0,     0,
       0,     6,     0,    16,     0,     0,     0,     0,     0,   135,
     133,   122,   118,     0,    77,   140,    78,   138,     0,    28,
      29,    80,   141,    81,    82,   139,   175,    81,     0,   111,
       0,     0,   174,   172,     0,   167,   169,    35,     0,    46,
     143,     0,     0,   117,   116,    35,     0,   115,     0,     0,
       0,    38,    39,     0,    37,    59,    58,    56,    13,    57,
      53,    55,    60,   159,     0,     0,     0,    26,     0,     0,
      75,     0,    72,    73,     0,    17,     0,   136,   180,   181,
       0,   161,   113,   132,     0,   120,    79,   111,     0,     0,
       0,     0,    87,   179,     0,   168,     0,   131,     0,    47,
      44,     9,    35,    35,    35,    35,    54,   117,    35,    35,
       0,     0,    36,   135,     0,     0,     0,    70,    71,    50,
       0,     0,   156,     0,   119,     0,    83,    84,   182,    87,
      88,   150,   163,    10,     0,    43,    64,     0,    66,     0,
      35,    62,    63,     0,     0,    20,     0,    69,    51,    15,
       0,   154,     0,   162,   142,   151,   130,    35,    35,    61,
       0,    35,    21,   137,   152,     0,   157,    65,    67,     0,
       0,    68,   154,   155,     0,    85,   153,     0,    86
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,     3,     4,   155,    27,   191,   182,   212,   170,   171,
     172,   173,   174,    28,   150,   151,   152,    94,   339,   175,
     176,   109,   110,   214,   215,   177,   321,    30,    80,   122,
      31,   126,   127,    32,   288,   124,   195,   354,    33,   136,
      34,   355,   312,   313,   178,    86,    87,   145,   146,   141,
      81,    82,   179
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -185
static const short yypact[] =
{
      99,    95,    89,   145,   219,  -185,   121,  -185,    92,   165,
     179,   391,   435,   103,   405,   136,   172,   203,    46,   391,
     391,  -185,  -185,   232,   237,   174,   185,    98,   107,  -185,
    -185,   182,    30,   190,   177,  -185,  -185,   225,   179,    41,
     228,   247,   179,   179,   214,    22,    37,   213,   391,  -185,
     175,   179,  -185,   241,   250,   253,   226,   270,    33,   177,
     280,  -185,  -185,   234,   236,  -185,  -185,   238,   243,  -185,
    -185,  -185,   376,   435,   277,  -185,  -185,    73,   171,   120,
     305,   281,   295,   179,  -185,  -185,   297,  -185,  -185,  -185,
     262,  -185,    47,  -185,   258,  -185,   316,  -185,  -185,   304,
     266,  -185,  -185,  -185,   357,   307,   303,   250,   271,   309,
     293,  -185,   318,   311,    21,  -185,   279,  -185,  -185,  -185,
    -185,  -185,   125,    45,  -185,   277,   300,   314,  -185,   505,
      54,  -185,  -185,  -185,  -185,  -185,   334,   376,   341,  -185,
      85,  -185,  -185,   336,  -185,   337,   297,   258,  -185,   317,
    -185,   142,  -185,   335,  -185,   316,   179,   179,  -185,  -185,
    -185,  -185,   103,   420,  -185,  -185,   179,   349,   342,   344,
     155,   345,   316,   316,   316,   353,   316,   316,   316,   316,
     177,   391,   348,   139,   357,   357,  -185,   352,    54,    50,
     250,   121,   354,  -185,   495,   356,   364,   286,   376,   279,
    -185,  -185,   332,   435,  -185,  -185,  -185,  -185,   377,  -185,
    -185,  -185,  -185,   372,    36,  -185,  -185,  -185,   333,   376,
     147,   373,   339,  -185,   378,   374,  -185,   316,   391,  -185,
      47,   179,   375,   289,   292,   316,   179,    93,   379,   359,
     347,  -185,  -185,   384,  -185,  -185,  -185,  -185,  -185,  -185,
    -185,  -185,  -185,  -185,   389,   179,   387,  -185,   390,    56,
    -185,   392,  -185,  -185,   393,  -185,   391,  -185,  -185,  -185,
     402,  -185,  -185,  -185,   179,  -185,  -185,   376,    54,    54,
     395,   178,   399,  -185,   177,  -185,   406,   401,   388,  -185,
    -185,  -185,   316,   316,   316,   316,  -185,   408,   316,   316,
     391,   421,  -185,   279,   417,   179,   418,  -185,  -185,   423,
     427,   436,   416,   177,  -185,   199,  -185,  -185,  -185,   399,
    -185,  -185,  -185,  -185,   391,  -185,  -185,   433,  -185,   438,
     316,  -185,  -185,   424,   439,  -185,   440,  -185,  -185,  -185,
     505,   505,   402,  -185,  -185,  -185,  -185,   316,   316,  -185,
      42,   316,  -185,  -185,   430,   444,  -185,  -185,  -185,   457,
     447,  -185,   505,  -185,   179,  -185,  -185,   452,  -185
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -185,    16,   468,   323,  -185,  -185,    32,  -185,  -185,   301,
    -185,  -185,  -185,    -9,   242,  -185,   244,   -34,  -185,   -97,
     472,   -75,  -185,   -47,   -86,  -185,   158,  -185,    -7,  -131,
       0,   275,  -185,     6,  -184,   285,  -102,  -125,   -79,  -185,
      -3,   122,  -185,   143,     5,  -185,   -31,   340,  -185,  -185,
    -185,   351,    27
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -175
static const short yytable[] =
{
      40,    41,    44,    44,   216,    44,   220,    96,    46,    35,
      44,    44,    47,   153,   196,   101,    59,    45,    49,    45,
      51,   200,    37,    79,    76,    58,    60,     8,   115,    89,
      90,    36,   187,    97,    98,    77,    99,  -135,   114,    44,
     103,   105,   106,    71,   218,    59,     8,     8,    84,    99,
       8,    56,     8,    85,    45,   261,   147,   204,   232,     8,
     206,   306,    74,    44,    44,   278,   262,   128,   148,   267,
     194,    38,   307,   125,   143,   245,   246,   247,   123,   249,
     250,   251,   252,   149,   208,    78,   100,    57,   281,  -174,
     222,    38,   359,    78,   194,    44,    78,   273,    -3,    -3,
       6,   279,   260,     1,  -174,   211,     1,     1,     8,   298,
     183,    -3,    42,    43,   199,   263,   333,    74,   201,    72,
     213,   217,   129,    38,    11,    12,   130,    73,    44,     1,
     286,   184,   137,     2,    -3,   138,     2,     2,   296,   197,
     346,    73,    38,   123,    77,     7,   315,   233,   234,   253,
      48,   153,    52,   229,   237,    21,   198,   239,    22,     2,
     238,   282,    23,    24,    25,    26,   271,   235,    45,    45,
      39,    74,    44,   230,   256,    44,    44,    53,   198,   217,
       8,   164,   165,    54,     8,   213,   104,   254,    84,    44,
     183,   183,   319,    85,    44,   326,   327,   328,   329,   293,
     295,   331,   332,   125,   123,   131,   132,   264,    55,   198,
      44,   184,   184,   344,    74,   353,   257,   258,    75,    44,
     133,   149,   149,    83,     8,   123,    69,   297,     9,    10,
     198,   316,   317,   349,   287,   134,   135,    70,    88,    91,
      11,    12,    13,    92,    93,    74,   304,    14,    73,   102,
     357,   358,   107,   322,   361,   108,    15,    44,    91,    16,
      17,    18,    92,    95,   111,   314,    19,    20,    44,   217,
     217,    21,   310,    91,    22,   113,   112,    92,    23,    24,
      25,    26,   343,   123,    61,   116,   117,    62,   118,    65,
     119,    44,    66,    63,    64,   120,   336,    84,    67,    68,
      91,   270,    85,    91,    92,   292,   287,    92,   294,    74,
     139,   142,   140,   144,   154,    44,   137,   180,   185,   186,
     188,     8,   189,   192,   190,   156,   157,   193,   194,   158,
     287,   213,   213,   159,   160,   202,   161,    11,    12,   162,
     163,   360,   164,   165,   166,   203,   219,   221,   224,  -143,
     225,   228,   231,   213,   240,   367,    16,   241,   167,   242,
     244,   255,     8,    19,    20,   259,    42,    43,    21,   248,
     265,    22,   268,   168,   169,    23,    24,    25,    26,    12,
     269,     8,   274,   276,   277,    42,    43,   280,   129,   283,
     285,   291,   284,   300,   303,   299,     8,   301,    12,   302,
      42,    43,   138,   305,   181,    20,   309,   311,   308,    21,
       8,   318,    22,    12,    42,    50,    23,    24,    25,    26,
     121,   320,   323,   325,   330,     8,   334,    12,    21,   236,
      43,    22,   324,   335,   337,    23,    24,    25,    26,   338,
       8,   340,    12,    21,    42,    43,    22,   342,   341,   347,
      23,    24,    25,    26,   348,   351,   352,    21,   363,   350,
      22,   362,   364,   365,    23,    24,    25,    26,   368,     5,
     227,   243,    21,   290,   289,    22,    29,   345,   275,    23,
      24,    25,    26,   272,   366,   356,   226,    21,     0,     0,
      22,   223,     0,     0,    23,    24,    25,    26,   204,   205,
       8,   206,   207,     0,     0,     0,     0,   266,   204,   205,
       8,   206,   207,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   208,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   208,     0,     0,     0,     0,
     209,   210,     0,     0,     0,     0,   211,     0,     0,     0,
     209,   210,     0,     0,     0,     0,   211
};

static const short yycheck[] =
{
       9,    10,    11,    12,   129,    14,   137,    41,    11,     4,
      19,    20,    12,    92,   116,    46,    19,    11,    13,    13,
      14,   123,     6,    32,    31,    19,    20,     5,    59,    38,
      39,     4,   107,    42,    43,     5,    45,    16,     5,    48,
      47,    50,    51,    27,   130,    48,     5,     5,    11,    58,
       5,     5,     5,    16,    48,     5,    90,     3,   155,     5,
       6,     5,    32,    72,    73,    29,    16,    74,    21,   194,
      49,    50,    16,    73,    83,   172,   173,   174,    72,   176,
     177,   178,   179,    92,    30,    63,    49,    41,   219,    16,
       5,    50,    50,    63,    49,   104,    63,   199,     0,     0,
      11,    65,   188,     8,    31,    51,     8,     8,     5,    16,
     104,    13,     9,    10,   123,   190,   300,    32,   125,    12,
     129,   130,    49,    50,    21,    22,    53,    34,   137,     8,
     227,   104,    12,    38,    13,    15,    38,    38,   235,    14,
     324,    34,    50,   137,     5,     0,   277,   156,   157,   180,
      47,   230,    16,    11,   163,    52,    31,   166,    55,    38,
     163,    14,    59,    60,    61,    62,   197,   162,   162,   163,
       5,    32,   181,    31,   183,   184,   185,     5,    31,   188,
       5,    26,    27,    11,     5,   194,    11,   181,    11,   198,
     184,   185,    14,    16,   203,   292,   293,   294,   295,   233,
     234,   298,   299,   203,   198,    34,    35,   191,     5,    31,
     219,   184,   185,    14,    32,   340,   184,   185,    36,   228,
      49,   230,   231,    33,     5,   219,    52,   236,     9,    10,
      31,   278,   279,   330,   228,    64,    65,    52,    13,    11,
      21,    22,    23,    15,    16,    32,   255,    28,    34,    36,
     347,   348,    11,   284,   351,     5,    37,   266,    11,    40,
      41,    42,    15,    16,    11,   274,    47,    48,   277,   278,
     279,    52,   266,    11,    55,     5,    50,    15,    59,    60,
      61,    62,   313,   277,    52,     5,    52,    55,    52,    52,
      52,   300,    55,    61,    62,    52,   305,    11,    61,    62,
      11,    15,    16,    11,    15,    16,   300,    15,    16,    32,
       5,    16,    31,    16,    56,   324,    12,    51,    11,    16,
      49,     5,    13,     5,    31,     9,    10,    16,    49,    13,
     324,   340,   341,    17,    18,    35,    20,    21,    22,    23,
      24,   350,    26,    27,    28,    31,    12,     6,    12,    33,
      13,    34,    17,   362,     5,   364,    40,    15,    42,    15,
      15,    13,     5,    47,    48,    13,     9,    10,    52,    16,
      16,    55,    16,    57,    58,    59,    60,    61,    62,    22,
      16,     5,    50,     6,    12,     9,    10,    54,    49,    16,
      16,    16,    14,    34,     5,    16,     5,    50,    22,    15,
       9,    10,    15,    13,    47,    48,    13,     5,    16,    52,
       5,    16,    55,    22,     9,    10,    59,    60,    61,    62,
      44,    22,    16,    35,    16,     5,     5,    22,    52,     9,
      10,    55,    31,    16,    16,    59,    60,    61,    62,    16,
       5,    14,    22,    52,     9,    10,    55,    31,    12,    16,
      59,    60,    61,    62,    16,    16,    16,    52,    14,    35,
      55,    31,     5,    16,    59,    60,    61,    62,    16,     1,
     147,   170,    52,   231,   230,    55,     4,   319,   203,    59,
      60,    61,    62,   198,   362,   342,   146,    52,    -1,    -1,
      55,   140,    -1,    -1,    59,    60,    61,    62,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    12,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    30,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    30,    -1,    -1,    -1,    -1,
      45,    46,    -1,    -1,    -1,    -1,    51,    -1,    -1,    -1,
      45,    46,    -1,    -1,    -1,    -1,    51
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,     8,    38,    67,    68,    68,    11,     0,     5,     9,
      10,    21,    22,    23,    28,    37,    40,    41,    42,    47,
      48,    52,    55,    59,    60,    61,    62,    70,    79,    86,
      93,    96,    99,   104,   106,   110,   118,    67,    50,     5,
      79,    79,     9,    10,    79,    99,   106,    96,    47,   110,
      10,    99,    16,     5,    11,     5,     5,    41,    99,   106,
      99,    52,    55,    61,    62,    52,    55,    61,    62,    52,
      52,    67,    12,    34,    32,    36,    94,     5,    63,    79,
      94,   116,   117,    33,    11,    16,   111,   112,    13,    79,
      79,    11,    15,    16,    83,    16,    83,    79,    79,    79,
      49,   112,    36,    94,    11,    79,    79,    11,     5,    87,
      88,    11,    50,     5,     5,   112,     5,    52,    52,    52,
      52,    44,    95,    99,   101,    96,    97,    98,    94,    49,
      53,    34,    35,    49,    64,    65,   105,    12,    15,     5,
      31,   115,    16,    79,    16,   113,   114,    83,    21,    79,
      80,    81,    82,   104,    56,    69,     9,    10,    13,    17,
      18,    20,    23,    24,    26,    27,    28,    42,    57,    58,
      74,    75,    76,    77,    78,    85,    86,    91,   110,   118,
      51,    47,    72,    99,   118,    11,    16,    87,    49,    13,
      31,    71,     5,    16,    49,   102,   102,    14,    31,    79,
     102,    94,    35,    31,     3,     4,     6,     7,    30,    45,
      46,    51,    73,    79,    89,    90,   103,    79,    90,    12,
      95,     6,     5,   117,    12,    13,   113,    69,    34,    11,
      31,    17,    85,    79,    79,   110,     9,    79,   106,    79,
       5,    15,    15,    75,    15,    85,    85,    85,    16,    85,
      85,    85,    85,   112,    99,    13,    79,    72,    72,    13,
      90,     5,    16,    87,    67,    16,    12,   103,    16,    16,
      15,   112,   101,   102,    50,    97,     6,    12,    29,    65,
      54,    95,    14,    16,    14,    16,    85,    99,   100,    82,
      80,    16,    16,    83,    16,    83,    85,    79,    16,    16,
      34,    50,    15,     5,    79,    13,     5,    16,    16,    13,
      99,     5,   108,   109,    79,    95,    89,    89,    16,    14,
      22,    92,   112,    16,    31,    35,    85,    85,    85,    85,
      16,    85,    85,   100,     5,    16,    79,    16,    16,    84,
      14,    12,    31,   112,    14,    92,   100,    16,    16,    85,
      35,    16,    16,   103,   103,   107,   109,    85,    85,    50,
      79,    85,    31,    14,     5,    16,   107,    79,    16
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
#line 164 "yacc.yy"
    {
	  ;}
    break;

  case 4:
#line 171 "yacc.yy"
    {
		printf("<INCLUDE>%s</INCLUDE>\n", yyvsp[-1]._str->latin1() );
	  ;}
    break;

  case 5:
#line 175 "yacc.yy"
    {
	  ;}
    break;

  case 6:
#line 178 "yacc.yy"
    {
          ;}
    break;

  case 7:
#line 183 "yacc.yy"
    { yyval._int = 1; ;}
    break;

  case 8:
#line 184 "yacc.yy"
    { yyval._int = 0; ;}
    break;

  case 9:
#line 189 "yacc.yy"
    {
	 	if (yyvsp[-2]._int)
			  printf("<CLASS>\n    <NAME>%s</NAME>\n%s%s</CLASS>\n", ( in_namespace + *yyvsp[-4]._str ).latin1(), yyvsp[-3]._str->latin1(), yyvsp[-1]._str->latin1() );
		// default C++ visibility specifier is 'private'
		dcop_area = 0;
		dcop_signal_area = 0;

	  ;}
    break;

  case 10:
#line 198 "yacc.yy"
    {
	 	if (yyvsp[-2]._int)
			  printf("<CLASS>\n    <NAME>%s</NAME>\n    <LINK_SCOPE>%s</LINK_SCOPE>\n%s%s</CLASS>\n", ( in_namespace + *yyvsp[-4]._str ).latin1(),yyvsp[-5]._str->latin1(),  yyvsp[-3]._str->latin1(), yyvsp[-1]._str->latin1() );
		// default C++ visibility specifier is 'private'
		dcop_area = 0;
		dcop_signal_area = 0;

	  ;}
    break;

  case 11:
#line 207 "yacc.yy"
    {
	  ;}
    break;

  case 12:
#line 210 "yacc.yy"
    {
	  ;}
    break;

  case 13:
#line 213 "yacc.yy"
    {
	  ;}
    break;

  case 14:
#line 216 "yacc.yy"
    {
                      in_namespace += *yyvsp[-1]._str; in_namespace += "::";
                  ;}
    break;

  case 15:
#line 220 "yacc.yy"
    {
                      int pos = in_namespace.findRev( "::", -3 );
                      if( pos >= 0 )
                          in_namespace = in_namespace.left( pos + 2 );
                      else
                          in_namespace = "";
                  ;}
    break;

  case 16:
#line 228 "yacc.yy"
    {
          ;}
    break;

  case 17:
#line 231 "yacc.yy"
    {
          ;}
    break;

  case 18:
#line 234 "yacc.yy"
    {
	  ;}
    break;

  case 19:
#line 237 "yacc.yy"
    {
	  ;}
    break;

  case 20:
#line 240 "yacc.yy"
    {
	  ;}
    break;

  case 21:
#line 243 "yacc.yy"
    {
	  ;}
    break;

  case 22:
#line 246 "yacc.yy"
    {
	  ;}
    break;

  case 23:
#line 249 "yacc.yy"
    {
	  ;}
    break;

  case 24:
#line 252 "yacc.yy"
    {
	  ;}
    break;

  case 25:
#line 255 "yacc.yy"
    {
	  ;}
    break;

  case 36:
#line 272 "yacc.yy"
    {
	  dcop_area = 0;
	  dcop_signal_area = 0;
	;}
    break;

  case 37:
#line 277 "yacc.yy"
    {
	  dcop_area = 0;
	  dcop_signal_area = 0;
	;}
    break;

  case 38:
#line 285 "yacc.yy"
    {
	  dcop_area = 1;
	  dcop_signal_area = 0;
	;}
    break;

  case 39:
#line 293 "yacc.yy"
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
#line 304 "yacc.yy"
    {
	  yyval._str = yyvsp[0]._str;
	;}
    break;

  case 41:
#line 307 "yacc.yy"
    {
	   QString* tmp = new QString( "%1::%2" );
           *tmp = tmp->arg(*(yyvsp[-2]._str)).arg(*(yyvsp[0]._str));
           yyval._str = tmp;
	;}
    break;

  case 42:
#line 316 "yacc.yy"
    {
		QString* tmp = new QString( "    <SUPER>%1</SUPER>\n" );
		*tmp = tmp->arg( *(yyvsp[0]._str) );
		yyval._str = tmp;
	  ;}
    break;

  case 43:
#line 322 "yacc.yy"
    {
		QString* tmp = new QString( "    <SUPER>%1</SUPER>\n" );
		*tmp = tmp->arg( *(yyvsp[-3]._str) + "&lt;" + *(yyvsp[-1]._str) + "&gt;" );
		yyval._str = tmp;
	  ;}
    break;

  case 44:
#line 331 "yacc.yy"
    {
		yyval._str = yyvsp[0]._str;
	  ;}
    break;

  case 45:
#line 335 "yacc.yy"
    {
		yyval._str = yyvsp[0]._str;
	  ;}
    break;

  case 46:
#line 342 "yacc.yy"
    {
		yyval._str = yyvsp[-1]._str;
	  ;}
    break;

  case 47:
#line 346 "yacc.yy"
    {
		/* $$ = $1; */
		yyval._str = new QString( *(yyvsp[-2]._str) + *(yyvsp[0]._str) );
	  ;}
    break;

  case 48:
#line 354 "yacc.yy"
    {
		yyval._str = yyvsp[0]._str;
	  ;}
    break;

  case 49:
#line 358 "yacc.yy"
    {
		yyval._str = new QString( "" );
	  ;}
    break;

  case 50:
#line 365 "yacc.yy"
    {
          ;}
    break;

  case 52:
#line 372 "yacc.yy"
    {
		yyval._str = new QString( "" );
	  ;}
    break;

  case 53:
#line 376 "yacc.yy"
    {
		yyval._str = new QString( *(yyvsp[-1]._str) + *(yyvsp[0]._str) );
	  ;}
    break;

  case 54:
#line 380 "yacc.yy"
    {
		yyval._str = new QString( *(yyvsp[-1]._str) + *(yyvsp[0]._str) );
	  ;}
    break;

  case 55:
#line 384 "yacc.yy"
    {
		yyval._str = new QString( *(yyvsp[-1]._str) + *(yyvsp[0]._str) );
	  ;}
    break;

  case 56:
#line 388 "yacc.yy"
    {
		yyval._str = yyvsp[0]._str;
	  ;}
    break;

  case 57:
#line 392 "yacc.yy"
    {
		yyval._str = yyvsp[0]._str;
	  ;}
    break;

  case 58:
#line 396 "yacc.yy"
    {
		yyval._str = yyvsp[0]._str;
	  ;}
    break;

  case 59:
#line 400 "yacc.yy"
    {	
	        yyval._str = yyvsp[0]._str;
	  ;}
    break;

  case 60:
#line 404 "yacc.yy"
    {
 	        yyval._str = yyvsp[0]._str;
	  ;}
    break;

  case 61:
#line 408 "yacc.yy"
    {
		yyval._str = yyvsp[0]._str;
	  ;}
    break;

  case 62:
#line 412 "yacc.yy"
    {
		yyval._str = yyvsp[0]._str;
	  ;}
    break;

  case 63:
#line 416 "yacc.yy"
    {
		yyval._str = yyvsp[0]._str;
	  ;}
    break;

  case 64:
#line 420 "yacc.yy"
    {
		yyval._str = yyvsp[0]._str;
	  ;}
    break;

  case 65:
#line 424 "yacc.yy"
    {
                yyval._str = yyvsp[0]._str;
          ;}
    break;

  case 66:
#line 428 "yacc.yy"
    {
		yyval._str = yyvsp[0]._str;
	  ;}
    break;

  case 67:
#line 432 "yacc.yy"
    {
                yyval._str = yyvsp[0]._str;
          ;}
    break;

  case 68:
#line 436 "yacc.yy"
    {
                yyval._str = yyvsp[0]._str;
          ;}
    break;

  case 75:
#line 454 "yacc.yy"
    {;}
    break;

  case 76:
#line 455 "yacc.yy"
    {;}
    break;

  case 77:
#line 459 "yacc.yy"
    {;}
    break;

  case 78:
#line 460 "yacc.yy"
    {;}
    break;

  case 79:
#line 461 "yacc.yy"
    {;}
    break;

  case 80:
#line 462 "yacc.yy"
    {;}
    break;

  case 81:
#line 463 "yacc.yy"
    {;}
    break;

  case 82:
#line 467 "yacc.yy"
    {;}
    break;

  case 83:
#line 468 "yacc.yy"
    {;}
    break;

  case 84:
#line 469 "yacc.yy"
    {;}
    break;

  case 85:
#line 474 "yacc.yy"
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
#line 484 "yacc.yy"
    {
		if (dcop_area)
		  yyerror("scoped template typedefs are not supported in dcop areas!");
	  ;}
    break;

  case 87:
#line 492 "yacc.yy"
    {
		yyval._int = 0;
	  ;}
    break;

  case 88:
#line 496 "yacc.yy"
    {
		yyval._int = 1;
	  ;}
    break;

  case 89:
#line 502 "yacc.yy"
    { yyval._str = new QString("signed int"); ;}
    break;

  case 90:
#line 503 "yacc.yy"
    { yyval._str = new QString("signed int"); ;}
    break;

  case 91:
#line 504 "yacc.yy"
    { yyval._str = new QString("unsigned int"); ;}
    break;

  case 92:
#line 505 "yacc.yy"
    { yyval._str = new QString("unsigned int"); ;}
    break;

  case 93:
#line 506 "yacc.yy"
    { yyval._str = new QString("signed short int"); ;}
    break;

  case 94:
#line 507 "yacc.yy"
    { yyval._str = new QString("signed short int"); ;}
    break;

  case 95:
#line 508 "yacc.yy"
    { yyval._str = new QString("signed long int"); ;}
    break;

  case 96:
#line 509 "yacc.yy"
    { yyval._str = new QString("signed long int"); ;}
    break;

  case 97:
#line 510 "yacc.yy"
    { yyval._str = new QString("unsigned short int"); ;}
    break;

  case 98:
#line 511 "yacc.yy"
    { yyval._str = new QString("unsigned short int"); ;}
    break;

  case 99:
#line 512 "yacc.yy"
    { yyval._str = new QString("unsigned long int"); ;}
    break;

  case 100:
#line 513 "yacc.yy"
    { yyval._str = new QString("unsigned long int"); ;}
    break;

  case 101:
#line 514 "yacc.yy"
    { yyval._str = new QString("int"); ;}
    break;

  case 102:
#line 515 "yacc.yy"
    { yyval._str = new QString("long int"); ;}
    break;

  case 103:
#line 516 "yacc.yy"
    { yyval._str = new QString("long int"); ;}
    break;

  case 104:
#line 517 "yacc.yy"
    { yyval._str = new QString("short int"); ;}
    break;

  case 105:
#line 518 "yacc.yy"
    { yyval._str = new QString("short int"); ;}
    break;

  case 106:
#line 519 "yacc.yy"
    { yyval._str = new QString("char"); ;}
    break;

  case 107:
#line 520 "yacc.yy"
    { yyval._str = new QString("signed char"); ;}
    break;

  case 108:
#line 521 "yacc.yy"
    { yyval._str = new QString("unsigned char"); ;}
    break;

  case 111:
#line 531 "yacc.yy"
    {
		yyval._str = new QString( "" );
	  ;}
    break;

  case 113:
#line 536 "yacc.yy"
    {
		yyval._str = new QString( *(yyvsp[-2]._str) + *(yyvsp[0]._str) );
	  ;}
    break;

  case 115:
#line 544 "yacc.yy"
    { yyval._str = yyvsp[0]._str; ;}
    break;

  case 116:
#line 545 "yacc.yy"
    { yyval._str = yyvsp[0]._str; ;}
    break;

  case 117:
#line 546 "yacc.yy"
    { yyval._str = yyvsp[0]._str; ;}
    break;

  case 118:
#line 547 "yacc.yy"
    {
		QString *tmp = new QString("%1&lt;%2&gt;");
		*tmp = tmp->arg(*(yyvsp[-3]._str));
		*tmp = tmp->arg(*(yyvsp[-1]._str));
		yyval._str = tmp;
	 ;}
    break;

  case 119:
#line 553 "yacc.yy"
    {
		QString *tmp = new QString("%1&lt;%2&gt;::%3");
		*tmp = tmp->arg(*(yyvsp[-5]._str));
		*tmp = tmp->arg(*(yyvsp[-3]._str));
		*tmp = tmp->arg(*(yyvsp[0]._str));
		yyval._str = tmp;
	 ;}
    break;

  case 120:
#line 565 "yacc.yy"
    {
	    yyval._str = new QString(*(yyvsp[-2]._str) + "," + *(yyvsp[0]._str));
	  ;}
    break;

  case 121:
#line 569 "yacc.yy"
    {
 	    yyval._str = yyvsp[0]._str;
	  ;}
    break;

  case 122:
#line 577 "yacc.yy"
    {
	    if (dcop_area)
	      yyerror("in dcop areas are no pointers allowed");
	  ;}
    break;

  case 123:
#line 582 "yacc.yy"
    {
 	    yyval._str = yyvsp[0]._str;
	  ;}
    break;

  case 124:
#line 591 "yacc.yy"
    {
	    if (dcop_area)
	      yyerror("in dcop areas are no pointers allowed");
	  ;}
    break;

  case 125:
#line 595 "yacc.yy"
    {
	     if (dcop_area) {
	  	QString* tmp = new QString("<TYPE  qleft=\"const\" qright=\"" AMP_ENTITY "\">%1</TYPE>");
		*tmp = tmp->arg( *(yyvsp[-1]._str) );
		yyval._str = tmp;
	     }
	  ;}
    break;

  case 126:
#line 602 "yacc.yy"
    {
		QString* tmp = new QString("<TYPE>%1</TYPE>");
		*tmp = tmp->arg( *(yyvsp[0]._str) );
		yyval._str = tmp;
	;}
    break;

  case 127:
#line 607 "yacc.yy"
    {
	     if (dcop_area)
		yyerror("in dcop areas are only const references allowed!");
	  ;}
    break;

  case 128:
#line 612 "yacc.yy"
    {
		QString* tmp = new QString("<TYPE>%1</TYPE>");
		*tmp = tmp->arg( *(yyvsp[0]._str) );
		yyval._str = tmp;
	;}
    break;

  case 129:
#line 618 "yacc.yy"
    {
	    if (dcop_area)
	      yyerror("in dcop areas are no pointers allowed");
	  ;}
    break;

  case 130:
#line 626 "yacc.yy"
    {
	    yyval._str = new QString(*(yyvsp[-2]._str) + "," + *(yyvsp[0]._str));
	  ;}
    break;

  case 131:
#line 630 "yacc.yy"
    {
 	    yyval._str = yyvsp[0]._str;
	  ;}
    break;

  case 132:
#line 637 "yacc.yy"
    {
		if (dcop_area) {
		   QString* tmp = new QString("\n        <ARG>%1<NAME>%2</NAME></ARG>");
  		   *tmp = tmp->arg( *(yyvsp[-2]._str) );
  		   *tmp = tmp->arg( *(yyvsp[-1]._str) );
		   yyval._str = tmp;		
		} else yyval._str = new QString();
	  ;}
    break;

  case 133:
#line 646 "yacc.yy"
    {
		if (dcop_area) {
		   QString* tmp = new QString("\n        <ARG>%1</ARG>");
  		   *tmp = tmp->arg( *(yyvsp[-1]._str) );
		   yyval._str = tmp;		
		} else yyval._str = new QString();
	  ;}
    break;

  case 134:
#line 654 "yacc.yy"
    {
		if (dcop_area)
			yyerror("variable arguments not supported in dcop area.");
		yyval._str = new QString("");
	  ;}
    break;

  case 135:
#line 663 "yacc.yy"
    {
	  ;}
    break;

  case 136:
#line 666 "yacc.yy"
    {
	  ;}
    break;

  case 137:
#line 669 "yacc.yy"
    {
	  ;}
    break;

  case 138:
#line 676 "yacc.yy"
    {
          ;}
    break;

  case 139:
#line 679 "yacc.yy"
    {
          ;}
    break;

  case 140:
#line 682 "yacc.yy"
    {
          ;}
    break;

  case 141:
#line 685 "yacc.yy"
    {
          ;}
    break;

  case 142:
#line 688 "yacc.yy"
    {
          ;}
    break;

  case 143:
#line 693 "yacc.yy"
    { yyval._int = 0; ;}
    break;

  case 144:
#line 694 "yacc.yy"
    { yyval._int = 1; ;}
    break;

  case 150:
#line 703 "yacc.yy"
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

  case 151:
#line 725 "yacc.yy"
    {
	     if (dcop_area)
		yyerror("operators aren't allowed in dcop areas!");
	     yyval._str = new QString("");
	  ;}
    break;

  case 152:
#line 735 "yacc.yy"
    {;}
    break;

  case 153:
#line 736 "yacc.yy"
    {;}
    break;

  case 154:
#line 737 "yacc.yy"
    {;}
    break;

  case 155:
#line 742 "yacc.yy"
    {;}
    break;

  case 156:
#line 747 "yacc.yy"
    {;}
    break;

  case 157:
#line 748 "yacc.yy"
    {;}
    break;

  case 158:
#line 753 "yacc.yy"
    {
	        yyval._str = yyvsp[-1]._str;
	  ;}
    break;

  case 159:
#line 757 "yacc.yy"
    {
		yyval._str = yyvsp[-3]._str;
	  ;}
    break;

  case 160:
#line 761 "yacc.yy"
    {
		yyval._str = yyvsp[-1]._str;
	  ;}
    break;

  case 161:
#line 765 "yacc.yy"
    {
	      /* The constructor */
	      assert(!dcop_area);
              yyval._str = new QString("");
	  ;}
    break;

  case 162:
#line 771 "yacc.yy"
    {
	      /* The constructor */
	      assert(!dcop_area);
              yyval._str = new QString("");
	  ;}
    break;

  case 163:
#line 777 "yacc.yy"
    {
	      /* The destructor */
  	      assert(!dcop_area);
              yyval._str = new QString("");
	  ;}
    break;

  case 164:
#line 783 "yacc.yy"
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

  case 165:
#line 796 "yacc.yy"
    {
		function_mode = 1;
	;}
    break;

  case 169:
#line 808 "yacc.yy"
    {;}
    break;

  case 170:
#line 809 "yacc.yy"
    {;}
    break;

  case 174:
#line 821 "yacc.yy"
    {;}
    break;

  case 175:
#line 822 "yacc.yy"
    {;}
    break;

  case 176:
#line 823 "yacc.yy"
    {;}
    break;

  case 177:
#line 826 "yacc.yy"
    {;}
    break;

  case 178:
#line 830 "yacc.yy"
    {;}
    break;

  case 179:
#line 831 "yacc.yy"
    {;}
    break;

  case 180:
#line 832 "yacc.yy"
    {;}
    break;

  case 181:
#line 833 "yacc.yy"
    {;}
    break;

  case 182:
#line 834 "yacc.yy"
    {;}
    break;


    }

/* Line 999 of yacc.c.  */
#line 2552 "yacc.cc"

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


#line 837 "yacc.yy"


void dcopidlParse( const char *_code )
{
    dcopidlInitFlex( _code );
    yyparse();
}

