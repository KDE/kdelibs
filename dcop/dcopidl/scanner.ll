%{
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

#define YY_NO_UNPUT
#include <stdlib.h>
#include <ctype.h>

class QString;

#ifndef KDE_USE_FINAL
#include "yacc.cc.h"
#endif

extern int idl_line_no;
int comment_mode;
int function_mode = 0;

#include <qstring.h>
#include <qregexp.h>

static long ascii_to_longlong( long base, const char *s )
{
  long ll = 0;
  while( *s != '\0' ) {
    char c = *s++;
    if( c >= 'a' )
      c -= 'a' - 'A';
    c -= '0';
    if( c > 9 )
      c -= 'A' - '0' - 10;
    ll = ll * base + c;
  }
  return ll;
}

static double ascii_to_longdouble (const char *s)
{
  double d;
#ifdef HAVE_SCANF_LF
  sscanf (s, "%Lf", &d);
#else
  /*
   * this is only an approximation and will probably break fixed<>
   * parameter calculation on systems where
   * sizeof(double) < sizeof(long double). but fortunately all
   * systems where scanf("%Lf") is known to be broken (Linux/Alpha
   * and HPUX) have sizeof(double) == sizeof(long double).
   */
  d = strtod (s, NULL);
#endif
  return d;
}

static char translate_char( const char *s )
{
  char c = *s++;

  if( c != '\\' )
    return c;
  c = *s++;
  switch( c ) {
  case 'n':
    return '\n';
  case 't':
    return '\t';
  case 'v':
    return '\v';
  case 'b':
    return '\b';
  case 'r':
    return '\r';
  case 'f':
    return '\f';
  case 'a':
    return '\a';
  case '\\':
    return '\\';
  case '?':
    return '\?';
  case '\'':
    return '\'';
  case '"':
    return '"';
  case 'x':
  case 'X':
    return (char) ascii_to_longlong( 16, s );
  default:
    // Gotta be an octal
    return (char) ascii_to_longlong( 8, s );
  }
}


%}

%option noyywrap
%option never-interactive
/*--------------------------------------------------------------------------*/

Digits                  [0-9]+
Oct_Digit               [0-7]
Hex_Digit               [a-fA-F0-9]
Int_Literal		[1-9][0-9]*L?
Oct_Literal		0{Oct_Digit}*
Hex_Literal		(0x|0X){Hex_Digit}*
Esc_Sequence1           "\\"[ntvbrfa\\\?\'\"]
Esc_Sequence2           "\\"{Oct_Digit}{1,3}
Esc_Sequence3           "\\"(x|X){Hex_Digit}{1,2}
Esc_Sequence            ({Esc_Sequence1}|{Esc_Sequence2}|{Esc_Sequence3})
Char                    ([^\n\t\"\'\\]|{Esc_Sequence})
Char_Literal            "'"({Char}|\"|\\)"'"
String_Literal		\"({Char}|"'")*\"
Float_Literal1		{Digits}"."{Digits}(e|E)("+"|"-")?{Digits}
Float_Literal2		{Digits}(e|E)("+"|"-")?{Digits}
Float_Literal3          {Digits}"."{Digits}
Float_Literal4		"."{Digits}
Float_Literal5		"."{Digits}(e|E)("+"|"-")?{Digits}

/*--------------------------------------------------------------------------*/

Kidl_Identifier		[_a-zA-Z][a-zA-Z0-9_]*

/*--------------------------------------------------------------------------*/



%%

[ \t]			;
[\n]			{ idl_line_no++; }

"/\*"           { comment_mode = 1; }
"\*/"           { if (!comment_mode) { REJECT; } else { comment_mode = 0; } }
"}"		{   
		    if (! comment_mode) {
		    if (!function_mode) { REJECT; } else {
			function_mode = 0;
			return T_RIGHT_CURLY_BRACKET;
		    }
		    }
		}
[^\n}*]*        { if (!comment_mode && !function_mode) { REJECT; } }
"*"             { if (!comment_mode) { REJECT; } }

"//"[^\n]*		;
"#!"[^\n]*		{
                          exit( 1 );
                        }
"#include"[ \t]*[<\"][^>"]*[>\"]\s*\n {
			  QString s( yytext );
                          int i = s.find(QRegExp("[\"<]"))+1;
                          int j = s.find(QRegExp("[\">]"), i);
			  yylval._str = new QString( s.mid( i, j - i ) );
                          idl_line_no++;
                          return T_INCLUDE;
                        }
"#"[^\n]*\n             {
                          idl_line_no++;
                        }
"{"			return T_LEFT_CURLY_BRACKET;
"}"			return T_RIGHT_CURLY_BRACKET;
"("			return T_LEFT_PARANTHESIS;
")"			return T_RIGHT_PARANTHESIS;
":"			return T_COLON;
"+"			return T_PLUS;
"-"			return T_MINUS;
"~"			return T_TILDE;
","			return T_COMMA;
"*"			return T_ASTERISK;
";"			return T_SEMICOLON;
"&"			return T_AMPERSAND;
"<"			return T_LESS;
">"			return T_GREATER;
"::"			return T_SCOPE;
"="			return T_EQUAL;
"."			return T_ACCESS;
"..."			return T_TRIPLE_DOT;
"["			return T_ARRAY_OPEN;
"]"			return T_ARRAY_CLOSE;
"->"			return T_ACCESS;
(">>"|"<<")		return T_SHIFT;
(">="|"<="|"!="|"==")	return T_MISCOPERATOR;


const			return T_CONST;
enum			return T_ENUM;
namespace		return T_NAMESPACE;
using			return T_USING;
class			return T_CLASS;
struct			return T_STRUCT;
operator		return T_FUNOPERATOR;
virtual			return T_VIRTUAL;
public			return T_PUBLIC;
inline			return T_INLINE;
static			return T_STATIC;
signed			return T_SIGNED;
unsigned		return T_UNSIGNED;
short			return T_SHORT;
long			return T_LONG;
friend			return T_FRIEND;
protected		return T_PROTECTED;
private			return T_PRIVATE;
signals			return T_SIGNAL;
return			return T_RETURN;
slots			return T_SLOT;
true			return T_TRUE;
int			return T_INT;
char			return T_CHAR;
false			return T_FALSE;
TRUE			return T_TRUE;
FALSE			return T_FALSE;
"k_dcop"		return T_DCOP_AREA;
"k_dcop_signals"		return T_DCOP_SIGNAL_AREA;
typedef			return T_TYPEDEF;
K_DCOP			return T_DCOP;
Q_OBJECT		;
("0"|"0L")		return T_NULL;
"extern "[A-Za-z0-9_ \t*]+	return T_EXTERN;
"extern \"C\""			return T_EXTERN_C;

{Kidl_Identifier}	{
			  yylval._str = new QString( yytext );
			  return T_IDENTIFIER;
			}

{Float_Literal1}	|
{Float_Literal2}	|
{Float_Literal3}	|
{Float_Literal4}	|
{Float_Literal5}	{
			  yylval._float = ascii_to_longdouble( yytext );
			  return T_DOUBLE_LITERAL;
			}
{Int_Literal}		{
			  yylval._int = ascii_to_longlong( 10, yytext );
			  return T_INTEGER_LITERAL;
			}
{Oct_Literal}		{
			  yylval._int = ascii_to_longlong( 8, yytext );
			  return T_INTEGER_LITERAL;
			}
{Hex_Literal}		{
			  yylval._int = ascii_to_longlong( 16, yytext + 2 );
			  return T_INTEGER_LITERAL;
			}
{Char_Literal}		{
                          QCString s( yytext );
	                  s = s.mid( 1, s.length() - 2 );
			  yylval._char = translate_char( s );
			  return T_CHARACTER_LITERAL;
			}
{String_Literal}	{
                          QString s( yytext );
                          yylval._str = new QString( s.mid( 1, s.length() - 2 ) );
			  return T_STRING_LITERAL;
			}
.			{
			  if (function_mode)
				return yylex(); // call once again

			  qFatal("could not parse %c(%d) at line %d" , yytext[0], yytext[0], idl_line_no);
			  return T_UNKNOWN;
			}

%%

void dcopidlInitFlex( const char *_code )
{
   comment_mode = 0;
   yy_switch_to_buffer( yy_scan_string( _code ) );
}
