%{
#include "predicate_parser.h"
#include <string.h>
#include <stdlib.h>
#define YY_NO_UNPUT

int yywrap();
void PredicateParse_initLexer( const char *_code );
char *PredicateParse_putSymbol( char *_name );
char *PredicateParse_putString( char *_str );

%}

DIGIT [0-9]

%%

"==" { return EQ; }

"AND" { return AND; }
"OR" { return OR; }

"true" { yylval.valb = 1; return VAL_BOOL; }
"false" { yylval.valb = 0; return VAL_BOOL; }

"'"[^']*"'" { yylval.name = PredicateParse_putString( yytext ); return VAL_STRING; }

"-"{DIGIT}+ { yylval.vali = atoi( yytext ); return VAL_NUM; }
{DIGIT}+ { yylval.vali = atoi( yytext ); return VAL_NUM; }

{DIGIT}*"\."{DIGIT}+ { yylval.vald = atof( yytext ); return VAL_FLOAT; }

[a-zA-Z][a-zA-Z0-9\-]* { yylval.name = PredicateParse_putSymbol( yytext ); return VAL_ID; }

"{"|"}"|"["|"]"|","|"\." { yylval.name = 0; return (int)(*yytext); }

[ \t\n]+ /* eat up whitespace */

. { printf( "Unrecognized character: %s\n", yytext ); }

%%

char *PredicateParse_putSymbol( char *_name )
{
    char *p = (char*)malloc( strlen( _name ) + 1 );
    if (p != NULL)
    {
        strcpy( p, _name );
    }
    return p;
}

char *PredicateParse_putString( char *_str )
{
    int l = strlen( _str );
    char *p = (char*)malloc( l );
    char *s = _str + 1;
    char *d = p;

    if (p == NULL)
        return NULL;

    while ( s != _str + l - 1 )
    {
        if ( *s != '\\' )
            *d++ = *s++;
        else
        {
            s++;
            if ( s != _str + l - 1 )
            {
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
    }
    *d = 0;
    return p;
}

void PredicateParse_initLexer( const char *_code )
{
    yy_switch_to_buffer( yy_scan_string( _code ) );
}

int yywrap()
{
    yy_delete_buffer( YY_CURRENT_BUFFER );
    return 1;
}

