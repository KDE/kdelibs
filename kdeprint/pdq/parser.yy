/******************/
/* C declarations */
/******************/
%{
#include <stdio.h>
#include <stdlib.h>
#include "pdq.h"

#define	YYDEBUG 0

static QList<QString>* split_string(QString* str);
static BlockT	*main_block = 0;

extern int yylex();
int yyerror(char *s);

QString	BlockT::prefix_ = "";
%}

/**********************/
/* Bison declarations */
/**********************/
%union {
	QList<QString>	*list;
	QString		*string;
	BlockT		*block;
}

%token <string> PRINTER
%token <string> DRIVER
%token <string> INTERFACE
%token <string> LANGUAGE_DRIVER
%token <string> LOCATION
%token <string> MODEL
%token <string> DRIVER_OPTS
%token <string> DRIVER_ARGS
%token <string> INTERFACE_OPTS
%token <string> INTERFACE_ARGS
%token <string> JOB_DIR
%token <string> HELP
%token <string> OPTION
%token <string> ARGUMENT
%token <string> VAR
%token <string> DESC
%token <string> CHOICE
%token <string> REQUIRES
%token <string> REQUIRED_ARGS
%token <string> DEF_VALUE
%token <string> DEFAULT_CHOICE
%token <string> VALUE
%token <string> FILETYPE_REGX
%token <string> DRIVER_COMMAND_PATH
%token <string> INTERFACE_COMMAND_PATH
%token <string> TRY_INCLUDE
%token <string> INCLUDE
%token <string> DEFAULT_PRINTER
%token <string> STRING IDENTIFIER
%token <string> SCRIPT_START
%token <string> SCRIPT_BODY
%token <block> COMMENT

%type <block> input
%type <block> main_statement
%type <block> preprocessor_statement
%type <block> printer_block
%type <block> printer_body
%type <block> printer_body_item
%type <block> interface_block
%type <block> interface_body
%type <block> interface_body_item
%type <block> argument_block
%type <block> argument_body
%type <block> argument_body_item
%type <block> option_block
%type <block> option_body
%type <block> option_body_item
%type <block> choice_block
%type <block> scalar_assignment
%type <block> list_assignment
%type <block> pair_assignment
%type <block> driver_block
%type <block> script_block
%type <block> language_driver_block
%type <string> preprocessor_keyword
%type <string> scalar_keyword
%type <string> string_ident
%type <string> list_keyword
%type <string> pair_keyword
%type <list> string_list
%type <list> pair_item
%type <list> pair_list

%%

input:		  main_statement	{ main_block = new BlockT(BlockT::Block); main_block->name_ = new QString("Main"); main_block->addBlock($1); $$ = main_block; }
		| input main_statement	{ $1->addBlock($2); $$ = $1; }
;

main_statement:	  COMMENT			{ $$ = new BlockT(BlockT::Null); }
		| preprocessor_statement	{ $$ = $1; }
		| printer_block			{ $$ = $1; }
		| interface_block		{ $$ = $1; }
		| driver_block			{ $$ = $1; }
;

preprocessor_keyword:	  DRIVER_COMMAND_PATH		{ $$ = $1; }
			| INTERFACE_COMMAND_PATH	{ $$ = $1; }
			| TRY_INCLUDE			{ $$ = $1; }
			| INCLUDE			{ $$ = $1; }
			| DEFAULT_PRINTER		{ $$ = $1; }
			| JOB_DIR			{ $$ = $1; }
;

preprocessor_statement:	preprocessor_keyword string_ident	{ BlockT *blk = new BlockT(BlockT::String); blk->name_ = $1; blk->addValue($2); $$ = blk; }
;

string_list:	  /* empty item */		{ $$ = NULL; }
		| STRING			{ QList<QString> *l = new QList<QString>; l->setAutoDelete(true); l->append($1); $$ = l; }
		| string_list STRING		{ $1->append($2); $$ = $1; }
		| string_list ',' STRING	{ $1->append($3); $$ = $1; }
;

pair_item:	STRING '=' STRING		{ QList<QString> *l = new QList<QString>; l->append($1); l->append($3); $$ = l; }
;

pair_list:	  /* empty item */		{ $$ = NULL; }
		| pair_item			{ $1->setAutoDelete(true); $$ = $1; }
		| pair_list ',' pair_item	{ $1->append($3->at(0)); $1->append($3->at(1)); $3->setAutoDelete(false); delete $3; $$ = $1; }
;

printer_block:	PRINTER string_ident '{' printer_body '}'	{ $4->name_ = $1; $4->addValue($2); $$ = $4; }
;

printer_body:	  printer_body_item			{ BlockT *blk = new BlockT(BlockT::Block); blk->addBlock($1); $$ = blk; }
		| printer_body printer_body_item	{ $1->addBlock($2); $$ = $1; }
;

printer_body_item:
		  COMMENT		{ $$ = new BlockT(BlockT::Null); }
		| scalar_assignment	{ $$ = $1; }
		| list_assignment	{ $$ = $1; }
		| pair_assignment	{ $$ = $1; }
;

interface_block:	INTERFACE string_ident '{' interface_body '}'	{ $4->name_ = $1; $4->addValue($2); $$ = $4; }
;

interface_body:	  interface_body_item			{ BlockT *blk = new BlockT(BlockT::Block); blk->addBlock($1); $$ = blk; }
		| interface_body interface_body_item	{ $1->addBlock($2); $$ = $1; }
;

interface_body_item:	  COMMENT			{ $$ = new BlockT(BlockT::Null); }
			| scalar_assignment		{ $$ = $1; }
			| list_assignment		{ $$ = $1; }
			| pair_assignment		{ $$ = $1; }
			| argument_block		{ $$ = $1; }
			| option_block			{ $$ = $1; }
			| script_block			{ $$ = $1; }
			| language_driver_block		{ $$ = $1; }
;

argument_block:	ARGUMENT '{' argument_body '}'	{ $3->name_ = $1; $$ = $3; }
;

argument_body:	  argument_body_item			{ BlockT *blk = new BlockT(BlockT::Block); blk->addBlock($1); $$ = blk; }
		| argument_body argument_body_item	{ $1->addBlock($2); $$ = $1; }
;

argument_body_item:	  COMMENT		{ $$ = new BlockT(BlockT::Null); }
			| scalar_assignment	{ $$ = $1; }
;

option_block:	OPTION '{' option_body '}'	{ $3->name_ = $1; $$ = $3; }
;

option_body:	  option_body_item  		{ BlockT *blk = new BlockT(BlockT::Block); blk->addBlock($1); $$ = blk; }
		| option_body option_body_item	{ $1->addBlock($2); $$ = $1; }
;

option_body_item:	COMMENT			{ $$ = new BlockT(BlockT::Null); }
			| scalar_assignment	{ $$ = $1; }
			| choice_block		{ $$ = $1; }
;

choice_block:	CHOICE string_ident '{' argument_body '}'	{ $4->name_ = $1; $4->addValue($2); $$ = $4; }
;

scalar_keyword:	  LOCATION		{ $$ = $1; }
		| MODEL			{ $$ = $1; }
		| DRIVER		{ $$ = $1; }
		| INTERFACE		{ $$ = $1; }
		| HELP			{ $$ = $1; }
		| VAR			{ $$ = $1; }
		| DESC			{ $$ = $1; }
		| DEF_VALUE		{ $$ = $1; }
		| VALUE			{ $$ = $1; }
		| DEFAULT_CHOICE	{ $$ = $1; }
		| FILETYPE_REGX		{ $$ = $1; }
;

string_ident:	  STRING	{ $$ = $1; }
		| IDENTIFIER	{ $$ = $1; }
;

scalar_assignment:	  scalar_keyword string_ident			{ BlockT *blk = new BlockT(BlockT::String); blk->name_ = $1; blk->addValue($2); $$ = blk; }
			| scalar_keyword '=' string_ident		{ BlockT *blk = new BlockT(BlockT::String); blk->name_ = $1; blk->addValue($3); $$ = blk; }


list_keyword:	  DRIVER_OPTS		{ $$ = $1; }
		| INTERFACE_OPTS	{ $$ = $1; }
		| REQUIRED_ARGS		{ $$ = $1; }
		| REQUIRES		{ $$ = $1; }
;

list_assignment:	  list_keyword STRING				{ BlockT *blk = new BlockT(BlockT::StringList); blk->name_ = $1; blk->values_ = split_string($2); $$ = blk; }
			| list_keyword '{' string_list '}'		{ BlockT *blk = new BlockT(BlockT::StringList); blk->name_ = $1; blk->values_ = $3; $$ = blk; }
			| list_keyword '=' '{' string_list '}'		{ BlockT *blk = new BlockT(BlockT::StringList); blk->name_ = $1; blk->values_ = $4; $$ = blk; }
;

pair_keyword:	  DRIVER_ARGS		{ $$ = $1; }
		| INTERFACE_ARGS	{ $$ = $1; }
;

pair_assignment:	  pair_keyword '{' pair_list '}'		{ BlockT *blk = new BlockT(BlockT::PairList); blk->name_ = $1; blk->values_ = $3; $$ = blk; }
			| pair_keyword '=' '{' pair_list '}'		{ BlockT *blk = new BlockT(BlockT::PairList); blk->name_ = $1; blk->values_ = $4; $$ = blk; }
;

script_block:	  SCRIPT_START '{' SCRIPT_BODY '}'	{ BlockT *blk = new BlockT(BlockT::String); blk->name_ = $1; blk->addValue($3); $$ = blk; }
		| SCRIPT_START '=' '{' SCRIPT_BODY '}'	{ BlockT *blk = new BlockT(BlockT::String); blk->name_ = $1; blk->addValue($4); $$ = blk; }
;

driver_block:	DRIVER string_ident '{' interface_body '}'	{ $4->name_ = $1; $4->addValue($2); $$ = $4; }
;

language_driver_block:	  LANGUAGE_DRIVER string_ident '{' interface_body '}'	{ $4->name_ = $1; $4->addValue($2); $$ = $4; }
			| LANGUAGE_DRIVER string_ident '{' '}'			{ BlockT *blk = new BlockT(BlockT::Block); blk->name_ = $1; blk->addValue($2); $$ = blk; }
;

%%

/*******************/
/* Additional code */
/*******************/

int yyerror(char *s)
{
	fprintf(stderr, "%s\n", s);
}

QList<QString>* split_string(QString* str)
{
	QStringList	l = QStringList::split(',',*str,false);
	QList<QString>	*ll = new QList<QString>;
	ll->setAutoDelete(true);
	for (QStringList::ConstIterator it=l.begin(); it!=l.end(); ++it)
		ll->append(new QString((*it).stripWhiteSpace()));
	return ll;
}

BlockT* pdqParse(const QString& filename, bool expandincl)
{
	if (main_block)
	{
		delete main_block;
		main_block = 0;
	}

#if (YYDEBUG == 1)
        yydebug = 1;
#endif
	initPdqFlex(filename, expandincl);
	yyparse();

	return main_block;
}
