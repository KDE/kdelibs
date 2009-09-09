%{
#include "predicate_parser.h"
#include "predicateparse.h"
#include <string.h>
#include <stdlib.h>
#define YY_NO_UNPUT

int Solidwrap();
void PredicateParse_initLexer( const char *_code );
char *PredicateParse_putSymbol( char *_name );
char *PredicateParse_putString( char *_str );

%}

DIGIT [0-9]

%%

"==" { return EQ; }
"&" { return MASK; }

[aA][nN][dD] { return AND; }
[oO][rR] { return OR; }
[iI][sS] { return IS; }

[tT][rR][uU][eE] { Solidlval.valb = 1; return VAL_BOOL; }
[fF][aA][lL][sS][eE] { Solidlval.valb = 0; return VAL_BOOL; }

"'"[^']*"'" { Solidlval.name = PredicateParse_putString( Solidtext ); return VAL_STRING; }

"-"{DIGIT}+ { Solidlval.vali = atoi( Solidtext ); return VAL_NUM; }
{DIGIT}+ { Solidlval.vali = atoi( Solidtext ); return VAL_NUM; }

{DIGIT}*"\."{DIGIT}+ { Solidlval.vald = atof( Solidtext ); return VAL_FLOAT; }

[a-zA-Z][a-zA-Z0-9\-]* { Solidlval.name = PredicateParse_putSymbol( Solidtext ); return VAL_ID; }

"{"|"}"|"["|"]"|","|"\." { Solidlval.name = 0; return (int)(*Solidtext); }

[ \t\n]+ /* eat up whitespace */

. { PredicateLexer_unknownToken(Solidtext); }

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
    Solid_switch_to_buffer( Solid_scan_string( _code ) );
}

int Solidwrap()
{
    Solid_delete_buffer( YY_CURRENT_BUFFER );
    return 1;
}

