    /*

    Copyright (C) 1999 Stefan Westerfeld
                       stefan@space.twc.de

    Copyright (C) 1999 Torben Weis <weis@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    */

%{

/*
    This is the lex file for mcopidl. It is based on the dcopidl (kidl)
    lex file, which was written by Torben Weis.
*/

#define YY_NO_UNPUT
#define YY_NEVER_INTERACTIVE 1

#include <cstdlib>
#include <cstring>
#include <cctype>
#include "core.h"

using namespace std;
using namespace Arts;

#ifndef KDE_USE_FINAL
#include "yacc.cc.h"
#endif

extern int idl_line_no;
int comment_mode;

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

extern void startInclude(const char *line);
extern void endInclude();

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

/*--------------------------------------------------------------------------*/

Kidl_Identifier	[_a-zA-Z][a-zA-Z0-9_]*

Qualified_Identifier	("::")?[_a-zA-Z](("::")?[a-zA-Z0-9_])*

/*--------------------------------------------------------------------------*/

%%

[ \t]			;
[\n]			{ idl_line_no++; }

"/\*"			{ comment_mode = 1; }
"\*/"			{ if (!comment_mode) { REJECT; } else { comment_mode = 0; } }
[^\n*]*			{ if (!comment_mode) { REJECT; } }
"*"				{ if (!comment_mode) { REJECT; } }

"//"[^\n]*		;

"#startinclude"[^\n]*    { startInclude(yytext); }
"#endinclude"[^\n]* { endInclude(); }


"{"			return T_LEFT_CURLY_BRACKET;
"}"			return T_RIGHT_CURLY_BRACKET;
"("			return T_LEFT_PARANTHESIS;
")"			return T_RIGHT_PARANTHESIS;
"<"			return T_LESS;
">"			return T_GREATER;
","			return T_COMMA;
";"			return T_SEMICOLON;
":"			return T_COLON;
"="			return T_EQUAL;

void		return T_VOID;
byte		return T_BYTE;
long		return T_LONG;
boolean		return T_BOOLEAN;
string		return T_STRING;
object		return T_OBJECT;
struct		return T_STRUCT;
interface	return T_INTERFACE;
module		return T_MODULE;
"enum"		return T_ENUM;
attribute	return T_ATTRIBUTE;
sequence	return T_SEQUENCE;
readonly	return T_READONLY;
in			return T_IN;
out			return T_OUT;
audio		return T_AUDIO;
float		return T_FLOAT;
stream		return T_STREAM;
multi		return T_MULTI;
async		return T_ASYNC;
oneway		return T_ONEWAY;
default		return T_DEFAULT;

{Kidl_Identifier}	{
			  yylval._str = strdup(yytext);		// TAKE CARE: free that thing
			  return T_IDENTIFIER;
			}

{Qualified_Identifier} {
			  yylval._str = strdup(yytext);		// TAKE CARE: free that thing
			  return T_QUALIFIED_IDENTIFIER;
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

.                       {
                          return T_UNKNOWN;
                        }

%%

void mcopidlInitFlex( const char *_code )
{
   comment_mode = 0;
   yy_switch_to_buffer( yy_scan_string( _code ) );
}
