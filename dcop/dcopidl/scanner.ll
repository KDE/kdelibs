%{

#define YY_NO_UNPUT
#include <stdlib.h>
#include <ctype.h>

class QString;

#ifndef KDE_USE_FINAL
#include "yacc.cc.h"
#endif

extern int idl_line_no;
int comment_mode;

#include <qstring.h>

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

/*--------------------------------------------------------------------------*/

Digits                  [0-9]+
Oct_Digit               [0-7]
Hex_Digit               [a-fA-F0-9]
Int_Literal		[1-9][0-9]*
Oct_Literal		0{Oct_Digit}*
Hex_Literal		(0x|0X){Hex_Digit}*
Esc_Sequence1           "\\"[ntvbrfa\\\?\'\"]
Esc_Sequence2           "\\"{Oct_Digit}{1,3}
Esc_Sequence3           "\\"(x|X){Hex_Digit}{1,2}
Esc_Sequence            ({Esc_Sequence1}|{Esc_Sequence2}|{Esc_Sequence3})
Char                    ([^\n\t\"\'\\]|{Esc_Sequence})
Char_Literal            "'"({Char}|\")"'"
String_Literal		\"({Char}|"'")*\"
Float_Literal1		{Digits}"."{Digits}(e|E)("+"|"-")?{Digits}
Float_Literal2		{Digits}(e|E)("+"|"-")?{Digits}
Float_Literal3          {Digits}"."{Digits}
Float_Literal4		"."{Digits}
Float_Literal5		"."{Digits}(e|E)("+"|"-")?{Digits}

KDCOP			([^k]|k[^_]|k_[^d]|k_d[^c]|k_dc[^o]|k_dco[^p])*

/*--------------------------------------------------------------------------*/

Kidl_Identifier	[_a-zA-Z][a-zA-Z0-9_]*

/*--------------------------------------------------------------------------*/



%%

[ \t]			;
[\n]			{ idl_line_no++; }

"/*"[^\n]*		{ comment_mode = 1; }
"*/"[^\n]*		{ comment_mode = 0; }
[^\n]*			{ if (!comment_mode) { REJECT; } }

"//"[^\n]*		;
"#!"[^\n]*		{
                          exit( 1 );
                        }
"#include <"[^>]*">"\s*\n {
			  QString s( yytext );
			  yylval._str = new QString( s.mid( 10, s.stripWhiteSpace().length() - 11 ) );
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
","			return T_COMMA;
";"			return T_SEMICOLON;
"&"			return T_AMPERSAND;
"<"			return T_LESS;
">"			return T_GREATER;
"::"			return T_SCOPE;
"="			return T_EQUAL;

const			return T_CONST;
enum			return T_ENUM;
class			return T_CLASS;
virtual			return T_VIRTUAL;
public			return T_PUBLIC;
"k_dcop"		return T_DCOP_AREA;
typedef			return T_TYPEDEF;
K_DCOP			return T_DCOP;
"0"			return T_NULL;

"private:"{KDCOP}"k_dcop"	return T_DCOP_AREA;
"private:"{KDCOP}"}"		return T_RIGHT_CURLY_BRACKET;
"public:"{KDCOP}"k_dcop"	return T_DCOP_AREA;
"public:"{KDCOP}"}"		return T_RIGHT_CURLY_BRACKET;

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
.                       {
                          return T_UNKNOWN;
                        }

%%

void dcopidlInitFlex( const char *_code )
{
   comment_mode = 0;
   yy_switch_to_buffer( yy_scan_string( _code ) );
}
