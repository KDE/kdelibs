%{

#include "kurl.parse.h"

void yyerror(char *s);
int yylex();
void initFlex( const char *s );

%}

%union
{
     char *name;
}

%token <name> STRING
%token <name> REF
%token <name> PATH
%token DSLASH

/* Grammar follows */
     
%%

start: /* empty */
     | STRING DSLASH host { kurl_setProtocol( $<name>1 );  }
     | STRING DSLASH host path { kurl_setProtocol( $<name>1 ); }
     | STRING ':' path { kurl_setProtocol( $<name>1 ); }
     | path { }
;

host: STRING { kurl_setHost( $<name>1 ); }
    | STRING '@' STRING { kurl_setUser( $<name>1 ); kurl_setHost( $<name>3 ); }
    | STRING ':' STRING '@' STRING { kurl_setUser( $<name>1 ); kurl_setPass( $<name>3 ); kurl_setHost( $<name>5 ); }
    | STRING ':' STRING { kurl_setHost( $<name>1 ); kurl_setPort( $<name>3 ); }
    | STRING '@' STRING ':' STRING { kurl_setUser( $<name>1 ); kurl_setHost( $<name>3 ); kurl_setPort( $<name>5 ); }
    | STRING ':' STRING '@' STRING ':' STRING { kurl_setUser( $<name>1 ); kurl_setPass( $<name>3 ); kurl_setHost( $<name>5 ); kurl_setPort( $<name>7 ); }
;

path: PATH { kurl_setPath( $<name>1 ); }
    | PATH REF { kurl_setPath( $<name>1 ); kurl_setRef( $<name>2 ); }
    | REF { kurl_setRef( $<name>1 ); }
;

/* End of grammar */

%%

void kurlerror ( char *s )  /* Called by yyparse on error */
{
  // printf ("ERROR: %s\n", s);
  kurl_setError();
}

void kurl_mainParse( char *_code )
{
  kurl_initFlex( _code );
  kurlparse();
}
