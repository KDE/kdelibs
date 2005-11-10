%{
/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001-2003 Michael Goffioul <kdeprint@swing.be>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#include <qvariant.h>
#include <qiodevice.h>
#if __MINGW32__
// isatty
#include <io.h>
#endif 
#define YYSTYPE QVariant
#include "fooparser.cpp.h"

#define yylval kdeprint_foomatic2lval

#define YY_INPUT( buf, result, max_size ) \
	{ \
		if ( kdeprint_foomatic2scanner_device ) \
		{ \
			result = kdeprint_foomatic2scanner_device->readBlock( buf, max_size ); \
			if ( result < 0 ) \
				result = 0; \
		} \
		else \
			result = 0; \
	}

QIODevice* kdeprint_foomatic2scanner_device = NULL;
%}

%option noyywrap

D        [[:digit:]]

%%

^"$VAR1->".*$  { /* eat up those assignments */ }
^"$VAR1"       { return VAR; }
^"$postpipe"   { return POSTPIPE; }
"undef"        { return UNDEF; }
#.*            { /* eat up comment parts */ }

"="            { return '='; }
">"            { return '>'; }
","            { return ','; }
"{"            { return '{'; }
"}"            { return '}'; }
"["            { return '['; }
"]"            { return ']'; }
";"            { return ';'; }

'([^']|"\\'")*'          { yylval = QByteArray(yytext+1, strlen(yytext)-1); return STRING; }
\"[^\"]*\"               { yylval = QByteArray(yytext+1, strlen(yytext)-1); return QUOTED; }
-?{D}+                   { yylval = yytext; return NUMBER; }
-?{D}+"."{D}+            { yylval = yytext; return NUMBER; }
-?{D}+"."{D}+(e|E)-?{D}+ { yylval = yytext; return NUMBER; }

\n |
.                        { /* eat up anything else */ }

%%

void kdeprint_foomatic2scanner_init( QIODevice *d )
{
	kdeprint_foomatic2scanner_device = d;
}

void kdeprint_foomatic2scanner_terminate()
{
	kdeprint_foomatic2scanner_device = NULL;
}
