/******************/
/* C declarations */
/******************/

%{
#include <stdio.h>
#include <stdlib.h>

#define YYDEBUG 1
#include "matic.h"

extern int	line_count;
extern MHash	*main_hash;
%}

/**********************/
/* Bison declarations */
/**********************/
%{
#ifndef maticparser_cpp_h
%}
%union {
	QString	*string;
	MHash	*hash;
	int	num;
}
%{
#endif
%}

%token <string> STRING
%token <string> UNDEF
%token <string> VAR
%token <num> NUM

%type <hash> assignment
%type <hash> block
%type <hash> block_body
%type <hash> block_body_item
%type <hash> var_assignment

%%

input:	  var_assignment	{ main_hash = new MHash(new QString("Driver"),$1); }
	| input var_assignment	{ main_hash->addHash($2); }
;

assignment:	  STRING '=' '>' NUM		{ $$ = new MHash($1,$4); }
		| STRING '=' '>' STRING		{ $$ = new MHash($1,$4); }
		| STRING '=' '>' UNDEF		{ $$ = new MHash($1,(QString*)0); }
		| STRING '=' '>' block		{ $4->name = $1; $$ = $4; }
;

block:	  '{' '}'		{ $$ = new MHash((QString*)0,(QString*)0); }
	| '[' ']'		{ $$ = new MHash((QString*)0,(QString*)0); }
	| '{' block_body '}'	{ $$ = $2; }
	| '[' block_body ']'	{ $$ = $2; }
;

block_body_item:	  assignment	{ $$ = $1; }
			| block		{ $$ = $1; }
;

block_body:	  block_body_item			{ $$ = new MHash((QString*)0,$1); }
		| block_body ',' block_body_item	{ $1->addHash($3); $$ = $1; }
;

var_assignment:	  VAR '=' block ';'	{ $3->name = $1; $$ = $3; }
		| VAR '=' STRING ';'	{ $$ = new MHash($1,$3); }
;

%%

/********/
/* Code */
/********/

void maticerror(const char *s)
{
	fprintf(stderr, "Line %d: %s\n",line_count,s);
}

void initMaticParser(const char *s)
{
	maticdebug = 0;
	initMaticFlex(s);
}
