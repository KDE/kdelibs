%{

#include <stdlib.h>
#include <stdio.h>

#include <qstring.h>

extern int yylex();

// extern QString idl_lexFile;
extern int idl_line_no;

void kidlInitFlex( const char *_code );

void yyerror( const char *s )
{
	qDebug( "In line %i : %s", idl_line_no, s );
        exit(1);
	//   theParser->parse_error( idl_lexFile, s, idl_line_no );
}

%}


%union
{
  long   _int;
  QString        *_str;
  ushort          _char;
  double _float;
}

%token <_char> T_CHARACTER_LITERAL
%token <_float> T_DOUBLE_LITERAL
%token <_str> T_IDENTIFIER
%token <_int> T_INTEGER_LITERAL
%token <_str> T_STRING_LITERAL
%token <_str> T_INCLUDE
%token T_CLASS
%token T_LEFT_CURLY_BRACKET
%token T_LEFT_PARANTHESIS
%token T_RIGHT_CURLY_BRACKET
%token T_RIGHT_PARANTHESIS
%token T_COLON
%token T_SEMICOLON
%token T_PUBLIC
%token T_VIRTUAL
%token T_CONST
%token T_TYPEDEF
%token T_COMMA
%token T_LESS
%token T_GREATER
%token T_AMPERSAND
%token T_ENUM
%token T_UNKNOWN
%token T_EQUAL
%token T_SCOPE
%token T_NULL
%token T_DCOP
%token T_DCOP_AREA

%type <_str> body
%type <_str> class_header
%type <_str> super_classes
%type <_str> typedef
%type <_str> typedef_params
%type <_str> function
%type <_str> param
%type <_str> params
%type <_str> return
%type <_str> return_params
%type <_str> qualifier

%%

/*1*/
main
	: includes rest
	  {
	  }
	;
	
includes
	: T_INCLUDE includes
          {
		printf("<INCLUDE file=\"%s\"/>\n", $1->latin1() );
	  }
        |
          {
          }
        ;

rest
	: T_CLASS T_IDENTIFIER class_header T_DCOP body T_SEMICOLON
	  {
		printf("<CLASS name=\"%s\">\n%s\n%s</CLASS>\n", $2->latin1(), $3->latin1(), $5->latin1() );
	  }
	| T_CLASS T_IDENTIFIER T_SEMICOLON includes
	  {
	  }
	;

super_classes
	: T_IDENTIFIER T_LEFT_CURLY_BRACKET
	  {
		QString* tmp = new QString( "<SUPER name=\"%1\"/>" );
		*tmp = tmp->arg( *($1) );
		$$ = tmp;
	  }
	| T_IDENTIFIER T_COMMA super_classes
	  {
		QString* tmp = new QString( "<SUPER name=\"%1\"/>%2" );
		*tmp = tmp->arg( *($1) ).arg( *($3) );
		$$ = tmp;
	  }
	;

class_header
	: T_COLON T_VIRTUAL T_PUBLIC super_classes
	  {
		$$ = $4;
	  }
	| T_COLON T_PUBLIC super_classes
	  {
		$$ = $3;
	  }
	| T_LEFT_CURLY_BRACKET
	  {
		$$ = new QString( "" );
	  }
	;

body
	: T_RIGHT_CURLY_BRACKET
	  {
		$$ = new QString( "" );
	  }
	| typedef body
	  {
		$$ = new QString( *($1) + *($2) );
	  }
	| function body
	  {
		$$ = new QString( *($1) + *($2) );
	  }
	| T_DCOP_AREA T_COLON  body
	  {
		$$ = $3;
	  }
	;

typedef
	: T_TYPEDEF T_IDENTIFIER T_LESS typedef_params T_GREATER T_IDENTIFIER T_SEMICOLON
	  {
		QString* tmp = new QString("<TYPEDEF name=\"%1\" template=\"%2\">%3</TYPEDEF>\n");
		*tmp = tmp->arg( *($6) ).arg( *($2) ).arg( *($4) );
		$$ = tmp;
	  }
	;

typedef_params
	: T_IDENTIFIER
	  {
		QString* tmp = new QString("<PARAM type=\"%1\"/>");
		*tmp = tmp->arg( *($1) );
		$$ = tmp;
	  }
	| T_IDENTIFIER T_COMMA typedef_params
	  {
		QString* tmp = new QString("<PARAM type=\"%1\"/>%2");
		*tmp = tmp->arg( *($1) ).arg( *($3) );
		$$ = tmp;
	  }
	;

qualifier
	: /* empty */
	  {
		$$ = new QString( "" );
	  }
	| T_CONST
	  {
		$$ = new QString( "const" );
	  }
	;

return_params
	: T_IDENTIFIER
	  {
		$$ = $1;
	  }
	| T_IDENTIFIER T_COMMA typedef_params
	  {
		$$ = new QString( *($1) + *($3) );
	  }
	;

return
	: T_IDENTIFIER
	  {
		QString* tmp = new QString("<RET type=\"%1\"/>");
		*tmp = tmp->arg( *($1) );
		$$ = tmp;		
	  }
	| T_CONST T_IDENTIFIER T_AMPERSAND
	  {
		QString* tmp = new QString("<RET type=\"%1\" qleft=\"const\" qright=\"&\"/>");
		*tmp = tmp->arg( *($2) );
		$$ = tmp;		
	  }
	| T_IDENTIFIER T_LESS return_params T_GREATER
	  {
		QString* tmp = new QString("<RET type=\"%1<%2>\"/>");
		*tmp = tmp->arg( *($1) ).arg( *($3) );
		$$ = tmp;		
	  }
	| T_CONST T_IDENTIFIER T_LESS return_params T_GREATER T_AMPERSAND
	  {
		QString* tmp = new QString("<RET type=\"%1<%2>\" qleft=\"const\" qright=\"&\"/>");
		*tmp = tmp->arg( *($2) ).arg( *($4) );
		$$ = tmp;		
	  }
	;

params
	: /* empty */
	  {
		$$ = new QString( "" );
	  }
	| param
	  {
		$$ = $1;
	  }
	| params T_COMMA param
	  {
		$$ = new QString( *($1) + *($3) );
	  }
	;

param
	: T_CONST T_IDENTIFIER T_AMPERSAND T_IDENTIFIER default
	  {
		QString* tmp = new QString("<ARG name=\"%1\" type=\"%2\" qleft=\"const\" qright=\"&\"/>");
		*tmp = tmp->arg( *($4) ).arg( *($2) );
		$$ = tmp;		
	  }
	| T_IDENTIFIER T_IDENTIFIER default
	  {
		QString* tmp = new QString("<ARG name=\"%1\" type=\"%2\"/>");
		*tmp = tmp->arg( *($2) ).arg( *($1) );
		$$ = tmp;		
	  }
	;

default
	: /* empty */
	  {
	  }
	| T_EQUAL T_STRING_LITERAL
	  {
	  }
	| T_EQUAL T_CHARACTER_LITERAL
	  {
	  }
	| T_EQUAL T_DOUBLE_LITERAL
	  {
	  }
	| T_EQUAL T_INTEGER_LITERAL
	  {
	  }
	| T_EQUAL T_NULL
	  {
	  }
	| T_EQUAL T_IDENTIFIER T_SCOPE T_IDENTIFIER
	  {
	  }
	;

function
	: T_VIRTUAL return T_IDENTIFIER T_LEFT_PARANTHESIS params T_RIGHT_PARANTHESIS qualifier T_EQUAL T_NULL T_SEMICOLON
	  {
		QString* tmp = new QString("<FUNC name=\"%1\" qual=\"%4\">%2%3</FUNC>\n");
		*tmp = tmp->arg( *($3) ).arg( *($2) ).arg( *($5) ).arg( *($7) );
		$$ = tmp;
	  }
	| return T_IDENTIFIER T_LEFT_PARANTHESIS params T_RIGHT_PARANTHESIS qualifier T_EQUAL T_NULL T_SEMICOLON
	  {
		QString* tmp = new QString("<FUNC name=\"%1\" qual=\"%4\">%2%3</FUNC>\n");
		*tmp = tmp->arg( *($2) ).arg( *($1) ).arg( *($4) ).arg( *($6) );
		$$ = tmp;
	  }
	| T_VIRTUAL return T_IDENTIFIER T_LEFT_PARANTHESIS params T_RIGHT_PARANTHESIS qualifier T_SEMICOLON
	  {
		QString* tmp = new QString("<FUNC name=\"%1\" qual=\"%4\">%2%3</FUNC>\n");
		*tmp = tmp->arg( *($3) ).arg( *($2) ).arg( *($5) ).arg( *($7) );
		$$ = tmp;
	  }
	| return T_IDENTIFIER T_LEFT_PARANTHESIS params T_RIGHT_PARANTHESIS qualifier T_SEMICOLON
	  {
		QString* tmp = new QString("<FUNC name=\"%1\" qual=\"%4\">%2%3</FUNC>\n");
		*tmp = tmp->arg( *($2) ).arg( *($1) ).arg( *($4) ).arg( *($6) );
		$$ = tmp;
	  }
	;

%%

void kidlParse( const char *_code )
{
    printf("<!DOCTYPE DCOP-IDL><DCOP-IDL>\n");
    kidlInitFlex( _code );
    yyparse();
    printf("</DCOP-IDL>\n");
}
