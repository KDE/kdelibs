%{

#include "t1.h"
#include <string.h>
#include <stdlib.h>

char* putSymbol( char *_name );
char* putString( char *_name );
int yywrap();
void initFlex( const char *_code );

%}

DIGIT    [0-9]
ID       [a-z][a-z0-9]*

%%

"function" { return FUNCTION; }
"if" { return IF; }
"else" { return ELSE; }
"in" { return IN; }
"with" { return WITH; }
"while" { return WHILE; }
"for" { return FOR; }
"<<" { return SHIFT_LEFT; }
">>" { return SHIFT_RIGHT; }
"==" { return EQ; }
"!=" { return NEQ; }
"||" { return OR; }
"&&" { return AND; }
"this" { return THIS; }
"null" { return B_NULL; }
"true" { return B_TRUE; }
"false" { return B_FALSE; }
"new" { return NEW; }
"delete" { return DELETE; }
"break" { return BREAK; }
"continue" { return CONTINUE; }
"return" { return RETURN; }
"var" { return VAR; }
"++" { return PP; }
"--" { return MM; }
"<=" { return LEQ; }
">=" { return GEQ; }
"*=" { return MAS; }
"/=" { return DAS; }
"+=" { return AAS; }
"-=" { return SAS; }
"^=" { return PAS; }
"%=" { return RAS; }
"&=" { return BAAS; }
"|=" { return BOAS; }

"\""[^\"]*"\"" { yylval.name = putString( yytext ); return STRING; }
"'"[^']*"'" { yylval.name = putString( yytext ); return STRING; }

{DIGIT}+  { yylval.vali = atoi( yytext ); return NUM; }

{DIGIT}*"\."{DIGIT}+ { yylval.vald = atof( yytext ); return FLOAT; }

{ID}  { yylval.name = putSymbol( yytext ); return IDENTIFIER; }

">"|"<"|"^"|"&"|"\|"|"="|";"|"\."|"+"|"-"|"*"|"/"|"("|")"|"{"|"}"|"["|"]"|","  { yylval.name = 0L; return (int)(*yytext); }

[ \t\n]+          /* eat up whitespace */

. { printf( "Unrecognized character: %s\n", yytext ); }

%%

char* putSymbol( char *_name )
{
  char *p = (char*)malloc( strlen( _name ) + 1 );
  strcpy( p, _name );
  return p;
}

char* putString( char *_str )
{
  int l = strlen( _str );
  char *p = (char*)malloc( l );
  char *s = _str + 1;
  char *d = p;
  while ( s != _str + l - 1 )
  {
     if ( *s != '\\' )
        *d++ = *s++;
     else
     {
        s++;
        if ( *s == '\\' )
          *d++ = '\\';
        else if ( *s == 'n' )
          *d++ = '\n';
        else if ( *s == 'r' )
          *d++ = '\r';
        else if ( *s == 't' )
          *d++ = '\t';
        s++;
     }
  }
  *d = 0;
  return p;
}

int yywrap()
{
  yy_delete_buffer( YY_CURRENT_BUFFER );
  return 1;
}

void initFlex( const char *_code )
{
   yy_switch_to_buffer( yy_scan_string( _code ) );
}
