%{

#include "kurl.tab.h"
#include <string.h>
#include <stdlib.h>

char* kurlPutSymbol( char *_name );
int yywrap();

%}

%%

"://" { return DSLASH; }

"@"|":"|"#" { kurllval.name = 0L; return (int)(*yytext); }

"/"[^@#:]* { kurllval.name = kurlPutSymbol( yytext ); return PATH; }
"#".* { kurllval.name = kurlPutSymbol( yytext + 1 ); return REF; }
[^/@#:]+ { kurllval.name = kurlPutSymbol( yytext ); return STRING; }

%%

char* kurlPutSymbol( char *_name )
{
  char *p = (char*)malloc( strlen( _name ) + 1 );
  strcpy( p, _name );
  return p;
}

void kurl_initFlex( char *_code )
{
  kurl_switch_to_buffer( kurl_scan_string( _code ) );
}

int kurlwrap()
{
  kurl_delete_buffer( YY_CURRENT_BUFFER );
  return 1;
}
