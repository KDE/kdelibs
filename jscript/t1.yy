%{

#include "bison2cpp.h"

void initFlex( const char *_code );

%}

%union
{
     int vali;
     double vald;
     char *name;
     void *ptr;
}

%token FUNCTION     
%token IF
%token ELSE
%token IN
%token WITH
%token WHILE
%token FOR
%token SHIFT_LEFT
%token SHIFT_RIGHT
%token EQ
%token NEQ
%token OR
%token AND
%token THIS
%token B_NULL
%token FLOAT
%token B_TRUE
%token B_FALSE
%token NEW
%token DELETE
%token BREAK
%token CONTINUE
%token RETURN
%token VAR
%token PP
%token MM
%token <name> STRING
%token LEQ
%token GEQ
%token MAS
%token DAS
%token AAS
%token SAS
%token PAS
%token RAS
%token BAAS
%token BOAS
%token <vali>  NUM        /* number   */
%token <vald>  FLOAT
%token <name>  IDENTIFIER   /* Variable and Function  */
%type <ptr> primaryExpression
%type <ptr> expression
%type <ptr> assignmentExpression
%type <ptr> orExpression
%type <ptr> andExpression
%type <ptr> bitwiseOrExpression
%type <ptr> bitwiseAndExpression
%type <ptr> bitwiseXorExpression
%type <ptr> unaryExpression
%type <ptr> additiveExpression     
%type <ptr> multiplicativeExpression
%type <ptr> shiftExpression
%type <ptr> relationalExpression
%type <ptr> equalityExpression
%type <ptr> conditionalExpression
%type <ptr> expressionOpt
%type <ptr> memberExpression
%type <ptr> simpleExpression
%type <ptr> statement
%type <ptr> statements
%type <ptr> compoundStatement
%type <ptr> variablesOrExpression
%type <ptr> element
%type <ptr> parameterListOpt
%type <ptr> parameterList
%type <ptr> argumentList
%type <ptr> argumentListOpt
%type <ptr> constructor
%type <ptr> constructorCall

/* Grammar follows */
     
%%

input:   /* empty */ 
     | input element { printf("!!!!! Adding code !!!!!\n"); jsAppendCode( $<ptr>2 ); }
;
     
element:       FUNCTION IDENTIFIER '(' parameterListOpt ')' compoundStatement { printf("Function: '%s'\n",$<name>2 ); $$ = newJSFunction( $<name>2, $<ptr>4, $<ptr>6 ); }
             | statements { printf("Statement\n"); $$ = $<ptr>1; }
             | error '\n' { yyerrok; $$ = 0L; }
;
     
parameterListOpt: /* empty */ { $$ = 0L; }
                | parameterList { $$ = $<ptr>1; }
;

parameterList: IDENTIFIER { printf("param '%s'\n", $<name>1); $$ = newJSParameter( $<name>1, 0L ); }
             | IDENTIFIER ',' parameterList { printf("param '%s\n", $<name>1); $$ = newJSParameter( $<name>1, $<ptr>3 ); }
;

compoundStatement: '{' statements '}' { $$ = $<ptr>2; }
;

statements: /* empty */ { $$ = 0L; }
             | statement statements { printf(""); $$ = newJSStatement( $<ptr>1, $<ptr>2 ); }
             | compoundStatement { printf(""); $$ = $<ptr>1; }
;

statement:     ';' { printf(""); $$ = 0L; }
             | IF condition statement { printf("Simple IF\n"); $$ = 0L; }
             | IF condition statement ELSE statement { printf("Complex IF\n"); $$ = 0L; }
             | WHILE condition statement { printf(""); $$ = 0L; }
             | forParen ';' expressionOpt ';' expressionOpt ')' statement { printf(""); $$ = 0L; }
             | forBegin ';' expressionOpt ';' expressionOpt ')' statement { printf(""); $$ = 0L; }
             | forBegin IN expression ')' statement { printf(""); $$ = 0L; }
             | BREAK semiOpt { printf(""); $$ = 0L; }
             | CONTINUE semiOpt { printf(""); $$ = 0L; }
             | WITH '(' expression ')' statement { printf(""); $$ = 0L; }
             | RETURN expressionOpt semiOpt { printf(""); $$ = 0L; }
             | compoundStatement { printf(""); $$ = $<ptr>1; }
             | variablesOrExpression semiOpt { printf(""); $$ = $<ptr>1; }
;

semiOpt:
       | ';' { printf(""); }
;

condition:     '(' expression ')' { printf("Condition\n"); }
;

forParen: FOR '(' { printf(""); }
;

forBegin: forParen variablesOrExpression { printf(""); }
;

variablesOrExpression: VAR variables { printf(""); $$ = 0L; }
                     | assignmentExpression { printf(""); $$ = $<ptr>1; }
;

variables: variable { printf(""); }
         | variable ',' variables { printf(""); }
;

variable: IDENTIFIER { printf("Var: '%s'\n", $<name>1); }
        | IDENTIFIER '=' assignmentExpression { printf("Var with Assignment: '%s'\n", $<name>1); }
;

expressionOpt: /* empty */ { $$ = 0L; }
             | expression { printf(""); $$ = $<ptr>1; }
;

expression:    assignmentExpression { printf(""); $$ = $<ptr>1; }
             | assignmentExpression expression { printf(""); }
;

assignmentExpression: conditionalExpression { printf(""); $$ = $<ptr>1; }
                    | conditionalExpression '=' assignmentExpression { printf("Assignment ( = )\n"); $$ = newJSAssignment( OP_ASSIGN, $<ptr>1, $<ptr>3 ); }
                    | conditionalExpression MAS assignmentExpression { printf("Assignment ( *= )\n"); }
                    | conditionalExpression DAS assignmentExpression { printf("Assignment ( /= )\n"); }
                    | conditionalExpression AAS assignmentExpression { printf("Assignment ( += )\n"); }
                    | conditionalExpression SAS assignmentExpression { printf("Assignment ( -= )\n"); }
                    | conditionalExpression PAS assignmentExpression { printf("Assignment ( ^= )\n"); }
                    | conditionalExpression RAS assignmentExpression { printf("Assignment ( %= )\n"); }
                    | conditionalExpression BAAS assignmentExpression { printf("Assignment ( &= )\n"); }
                    | conditionalExpression BOAS assignmentExpression { printf("Assignment ( |= )\n"); }
;

conditionalExpression: orExpression { printf(""); $$ = $<ptr>1; }
                     | orExpression '?' assignmentExpression ':' assignmentExpression { printf(""); }
;

orExpression: andExpression { printf(""); $$ = $<ptr>1; }
            | andExpression OR orExpression { printf(""); $$ = newJSBinaryOperator( OP_OR, $<ptr>1, $<ptr>3 ); }
;

andExpression: bitwiseOrExpression { printf(""); $$ = $<ptr>1; }
             | bitwiseOrExpression AND andExpression { printf(""); $$ = newJSBinaryOperator( OP_AND, $<ptr>1, $<ptr>3 ); }
;

bitwiseOrExpression: bitwiseXorExpression { printf(""); $$ = $<ptr>1; }
                   | bitwiseXorExpression '|' bitwiseOrExpression { printf(""); $$ = newJSBinaryOperator( OP_BOR, $<ptr>1, $<ptr>3 ); }
;

bitwiseXorExpression: bitwiseAndExpression { printf(""); $$ = $<ptr>1; }
                    | bitwiseAndExpression '^' bitwiseXorExpression { printf(""); $$ = newJSBinaryOperator( OP_BXOR, $<ptr>1, $<ptr>3 ); }
;

bitwiseAndExpression: equalityExpression { printf(""); $$ = $<ptr>1; }
                    | equalityExpression '&' bitwiseAndExpression { printf(""); $$ = newJSBinaryOperator( OP_BAND, $<ptr>1, $<ptr>3 ); }
;

equalityExpression: relationalExpression { printf(""); $$ = $<ptr>1; }
                  | relationalExpression EQ equalityExpression { printf(""); $$ = newJSBinaryOperator( OP_EQ, $<ptr>1, $<ptr>3 ); }
                  | relationalExpression NEQ equalityExpression { printf(""); $$ = newJSBinaryOperator( OP_NEQ, $<ptr>1, $<ptr>3 ); }
;

relationalExpression: shiftExpression { printf(""); $$ = $<ptr>1; }
                    | relationalExpression '<' shiftExpression { printf(""); $$ = newJSBinaryOperator( OP_LT, $<ptr>1, $<ptr>3 ); }
                    | relationalExpression '>' shiftExpression { printf(""); $$ = newJSBinaryOperator( OP_GT, $<ptr>1, $<ptr>3 ); }
                    | relationalExpression LEQ shiftExpression { printf(""); $$ = newJSBinaryOperator( OP_LEQ, $<ptr>1, $<ptr>3 ); }
                    | relationalExpression GEQ shiftExpression { printf(""); $$ = newJSBinaryOperator( OP_GEQ, $<ptr>1, $<ptr>3 ); }
;

shiftExpression: additiveExpression { printf(""); $$ = $<ptr>1; }
               | additiveExpression  SHIFT_LEFT shiftExpression { printf(""); $$ = newJSBinaryOperator( OP_SL, $<ptr>1, $<ptr>3 ); }
               | additiveExpression  SHIFT_RIGHT shiftExpression { printf(""); $$ = newJSBinaryOperator( OP_SR, $<ptr>1, $<ptr>3 );}
;

additiveExpression: multiplicativeExpression { printf(""); $$ = $<ptr>1; }
                  | multiplicativeExpression '+' additiveExpression { printf("Add ( + )\n"); $$ = newJSBinaryOperator( OP_ADD, $<ptr>1, $<ptr>3 ); }
                  | multiplicativeExpression '-' additiveExpression { printf("Sub ( - )\n"); $$ = newJSBinaryOperator( OP_SUB, $<ptr>1, $<ptr>3 ); }
;

multiplicativeExpression: unaryExpression { printf(""); $$ = $<ptr>1; }
                        | unaryExpression '*' multiplicativeExpression { printf("Mul ( * )\n"); $$ = newJSBinaryOperator( OP_MUL, $<ptr>1, $<ptr>3 ); }
                        | unaryExpression '/' multiplicativeExpression { printf("Div ( / )\n"); $$ = newJSBinaryOperator( OP_DIV, $<ptr>1, $<ptr>3 ); }
;

unaryExpression: simpleExpression { printf(""); $$ = $<ptr>1; }
               | '-' unaryExpression { printf("Unary Minus\n"); }
               | PP simpleExpression { printf("++ Prefix\n"); }
               | MM simpleExpression { printf("-- Prefix\n"); }
	       | simpleExpression PP { printf("Postfix ++\n"); }
	       | simpleExpression MM { printf("Postfix --\n"); }
               | NEW constructor { printf("new\n"); $$ = $<ptr>2; }
               | DELETE simpleExpression { printf("delete\n"); }
;

constructor: THIS '.' constructorCall { printf(""); $$ = 0L; }
           | constructorCall { printf(""); $$ = $<ptr>1; }
;

constructorCall: IDENTIFIER { printf(""); $$ = newJSConstructorCall( newJSIdentifier( $<name>1 ) , 0L ); }
               | IDENTIFIER '(' argumentListOpt ')' { printf(""); $$ = newJSConstructorCall( newJSIdentifier( $<name>1 ), $<ptr>3 ); }
               | IDENTIFIER '.' constructorCall { printf(""); $$ = 0L; }
;

simpleExpression: memberExpression { printf(""); $$ = $<ptr>1; }
                | memberExpression '[' expression ']' { printf("[ ]\n"); $$ = newJSArrayAccess( $<ptr>1, $<ptr>3 ); }
                | memberExpression '(' argumentListOpt ')' { printf("Function Call\n"); $$ = newJSFunctionCall( $<ptr>1, $<ptr>3 ); }
;

memberExpression: primaryExpression { printf(""); $$ = $<ptr>1; }
                | simpleExpression '.' IDENTIFIER { printf("Member ( '%s' )\n", $<name>3 ); $$ = newJSMember( $<ptr>1, $<name>3 ); }
;

argumentListOpt: /* empty */ { $$ = 0L; }
	       | argumentList { printf("ArgumentList\n"); $$ = $<ptr>1; }
;

argumentList: assignmentExpression { printf("Argument\n"); $$ = newJSArgument( $<ptr>1, 0L ); }
            | assignmentExpression ',' argumentList { printf("Argument (cont)\n"); $$ = newJSArgument( $<ptr>1, $<ptr>3 ); }
;

primaryExpression: '(' expression ')' { printf("Paranthesis\n"); $$ = $<ptr>2; }
                 | IDENTIFIER { printf("ID '%s'\n",$<name>1); $$ = newJSIdentifier( $<name>1 ); }
                 | NUM { printf("NUM\n"); $$ = newJSInteger( $<vali>1 ); }
                 | FLOAT { printf(""); $$ = newJSFloat( $<vald>1 ); }
                 | STRING { printf(""); $$ = newJSString( $<name>1 ); }
                 | B_FALSE { printf(""); $$ = newJSBool( 0 ); }
                 | B_TRUE { printf(""); $$ = newJSBool( 1 ); }
                 | B_NULL { printf(""); $$ = newJSNull(); }
                 | THIS { printf(""); $$ = newJSThis(); }
;

/* End of grammar */

%%

yyerror ( char *s )  /* Called by yyparse on error */
{
    printf ("ERROR: %s\n", s);
}

void mainParse( const char *_code )
{
    initFlex( _code );
    yyparse();
}






