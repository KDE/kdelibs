/************************/
/* Internal definitions */
/************************/

D	[0-9]
L	[a-zA-Z_]

%{
#include <stdlib.h>
#include <stdio.h>
#include "matic.h"

#include "maticparser.cpp.h"

int line_count;

static void newStr(const char *_s, bool str = false)
{
	QString *s = new QString(_s);
	if (str)
		*s = s->mid(1,s->length()-2);
	maticlval.string = s;
}
%}

%%

"undef"		{ return UNDEF; }

#.*$		/* eat comments */

'([^']|\\')*'	{ newStr(matictext,true); return STRING; }
\"([^"]|\\\")*\"	{ newStr(matictext,true); return STRING; }

"-"		{ return '-'; }
">"		{ return '>'; }
"="		{ return '='; }
"{"		{ return '{'; }
"}"		{ return '}'; }
"["		{ return '['; }
"]"		{ return ']'; }
","		{ return ','; }
";"		{ return ';'; }

"$VAR1->".*$	/* eat up those assignment */
${L}({L}|{D})*	{ newStr(matictext); return VAR; }
[-[:alnum:]_]*	{ newStr(matictext); return STRING; }

[ \t]*		/* eat spaces */

.		/* eat all other characters */
"\n"		{ /* eat new lines */ line_count++; }

%%

int maticwrap()
{
	return 1;
}

void initMaticFlex(const char *s)
{
	matic_switch_to_buffer( matic_scan_string(s) );
}
