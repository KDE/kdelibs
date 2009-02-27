%{
#include <stdlib.h>
#include <stdio.h>
#include "predicateparse.h"

void Soliderror(const char *s);
int Solidlex();
int Solidlex_destroy();
void PredicateParse_initLexer( const char *s );
void PredicateParse_mainParse( const char *_code );

%}

%union
{
     char valb;
     int vali;
     double vald;
     char *name;
     void *ptr;
}

%token EQ
%token MASK

%token AND
%token OR
%token IS

%token <valb> VAL_BOOL
%token <name> VAL_STRING
%token <name> VAL_ID
%token <vali> VAL_NUM
%token <vald> VAL_FLOAT

%type <ptr> predicate
%type <ptr> predicate_atom
%type <ptr> predicate_or
%type <ptr> predicate_and
%type <ptr> string_list
%type <ptr> string_list_rec
%type <ptr> value

%destructor { PredicateParse_destroy( $$ ); } predicate
%destructor { PredicateParse_destroy( $$ ); } predicate_atom
%destructor { PredicateParse_destroy( $$ ); } predicate_or
%destructor { PredicateParse_destroy( $$ ); } predicate_and

%%

predicate: predicate_atom { PredicateParse_setResult( $<ptr>1 ); $$ = $<ptr>1; }
         | '[' predicate_or ']' { PredicateParse_setResult( $<ptr>2 ); $$ = $<ptr>2; }
         | '[' predicate_and ']' { PredicateParse_setResult( $<ptr>2 ); $$ = $<ptr>2; }

predicate_atom: VAL_ID '.' VAL_ID EQ value { $$ = PredicateParse_newAtom( $<name>1, $<name>3, $<ptr>5 ); }
              | VAL_ID '.' VAL_ID MASK value { $$ = PredicateParse_newMaskAtom( $<name>1, $<name>3, $<ptr>5 ); }
              | IS VAL_ID { $$ = PredicateParse_newIsAtom( $<name>2 ); }

predicate_or: predicate OR predicate { $$ = PredicateParse_newOr( $<ptr>1, $<ptr>3 ); }

predicate_and: predicate AND predicate { $$ = PredicateParse_newAnd( $<ptr>1, $<ptr>3 ); }

value: VAL_STRING { $$ = PredicateParse_newStringValue( $<name>1 ); }
     | VAL_BOOL { $$ = PredicateParse_newBoolValue( $<valb>1 ); }
     | VAL_NUM { $$ = PredicateParse_newNumValue( $<vali>1 ); }
     | VAL_FLOAT { $$ = PredicateParse_newDoubleValue( $<vald>1 ); }
     | string_list { $$ = $<ptr>1; }

string_list: '{' string_list_rec '}' { $$ = $<ptr>1; }

string_list_rec: /* empty */ { $$ = PredicateParse_newEmptyStringListValue(); }
               | VAL_STRING { $$ = PredicateParse_newStringListValue( $<ptr>1 ); }
               | VAL_STRING ',' string_list_rec { $$ = PredicateParse_appendStringListValue( $<name>1, $<ptr>3 ); }

%%

void Soliderror ( const char *s )  /* Called by Solidparse on error */
{
    printf ("ERROR: %s\n", s);
    PredicateParse_errorDetected();
}

void PredicateParse_mainParse( const char *_code )
{
    PredicateParse_initLexer( _code );
    Solidparse();
    Solidlex_destroy();
}

