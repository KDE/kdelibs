%{

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include <qstring.h>

#define AMP_ENTITY "&amp;"
#define YYERROR_VERBOSE

extern int yylex();

// extern QString idl_lexFile;
extern int idl_line_no;

static int dcop_area = 0;

void dcopidlInitFlex( const char *_code );

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
%token <_int> T_CHAR_LITERAL
%token <_str> T_STRING_LITERAL
%token <_str> T_INCLUDE
%token T_CLASS
%token T_STRUCT
%token T_LEFT_CURLY_BRACKET
%token T_LEFT_PARANTHESIS
%token T_RIGHT_CURLY_BRACKET
%token T_RIGHT_PARANTHESIS
%token T_COLON
%token T_SEMICOLON
%token T_PUBLIC
%token T_PROTECTED
%token T_PRIVATE
%token T_VIRTUAL
%token T_CONST
%token T_RETURN
%token T_SIGNAL
%token T_SLOT
%token T_TYPEDEF
%token T_COMMA
%token T_ASTERISK
%token T_TILDE
%token T_LESS
%token T_GREATER
%token T_AMPERSAND
%token T_ACCESS
%token T_ENUM
%token T_NAMESPACE
%token T_UNKNOWN
%token T_TRUE
%token T_FALSE
%token T_STATIC
%token T_EQUAL
%token T_SCOPE
%token T_NULL
%token T_DCOP
%token T_DCOP_AREA
%token T_SIGNED
%token T_UNSIGNED
%token T_FUNOPERATOR
%token T_MISCOPERATOR

%type <_str> body
%type <_str> class_header
%type <_str> super_classes
%type <_str> super_class
%type <_str> super_class_name
%type <_str> typedef
%type <_str> function
%type <_str> function_header
%type <_str> param
%type <_str> type
%type <_str> type_name
%type <_str> type_list
%type <_str> params
%type <_str> return
%type <_int> const_qualifier
%type <_str> prequalifier
%type <_int> virtual_qualifier
%type <_str> Identifier
%type <_int> dcoptag

%%

/*1*/
main
	: includes declaration main
	  {
	     dcop_area = 0; // reset
	  }
	| /* empty */
	
includes
	: T_INCLUDE includes
          {
		printf("<INCLUDE file=\"%s\"/>\n", $1->latin1() );
	  }
        | /* empty */
          {
          }
        ;

dcoptag
	: T_DCOP { $$ = 1; }
	| /* empty */ { $$ = 0; }

declaration
	: T_CLASS Identifier class_header dcoptag body T_SEMICOLON
	  {
	 	if ($4)
		  printf("<CLASS name=\"%s\">\n%s%s</CLASS>\n", $2->latin1(), $3->latin1(), $5->latin1() );
	  }
	| T_CLASS Identifier T_SEMICOLON
	  {
	  }
	| T_STRUCT Identifier T_SEMICOLON
	  {
	  }
	| T_STRUCT Identifier T_LEFT_CURLY_BRACKET main T_RIGHT_CURLY_BRACKET  T_SEMICOLON
	  {
	  }
	| T_NAMESPACE T_IDENTIFIER T_LEFT_CURLY_BRACKET main T_RIGHT_CURLY_BRACKET T_SEMICOLON
	  {
	  }
	| T_TYPEDEF return Identifier T_SEMICOLON
	  {
	  }
	| function
	  {
	  }
	| member
	  {
	  }
	| enum
	  {
	  }

bool_value: T_TRUE | T_FALSE;

nodcop_area: T_PRIVATE | T_PROTECTED | T_PUBLIC ;

sigslot: T_SIGNAL | T_SLOT | ;

nodcop_area_begin
	: nodcop_area sigslot T_COLON
        {
	  dcop_area = 0;
	}
	| sigslot T_COLON
	{
	  dcop_area = 0;
	}

dcop_area_begin
	: T_DCOP_AREA T_COLON
	{
	  dcop_area = 1;
	}

Identifier
	: T_IDENTIFIER {
	  $$ = $1;
	}
	| T_IDENTIFIER T_SCOPE Identifier {
	   $$ = new QString( *($1) + *($3) );
	}
		
super_class_name
	: Identifier
	  {
		QString* tmp = new QString( "<SUPER name=\"%1\"/>\n" );
		*tmp = tmp->arg( *($1) );
		$$ = tmp;
	  }

super_class
	: virtual_qualifier T_PUBLIC super_class_name
	  {
		$$ = $3;
	  }
	| super_class_name
	  {
		$$ = $1;
	  }
	;

super_classes
	: super_class T_LEFT_CURLY_BRACKET
	  {
		$$ = $1;
	  }
	| super_class T_COMMA super_classes
	  {
		/* $$ = $1; */
		$$ = new QString( *($1) + *($3) );
	  }
	;

class_header
	: T_COLON super_classes
	  {
		$$ = $2;
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
	| enum body
	  {
		$$ = $2;
	  }
	| dcop_area_begin body
	  {
		$$ = $2;
	  }
	| nodcop_area_begin body
	  {	
	        $$ = $2;
	  }
	| member body {
 	        $$ = $2;
	}
	;

enum
	: T_ENUM T_IDENTIFIER T_LEFT_CURLY_BRACKET enum_list T_RIGHT_CURLY_BRACKET T_IDENTIFIER T_SEMICOLON
	| T_ENUM T_IDENTIFIER T_LEFT_CURLY_BRACKET enum_list T_RIGHT_CURLY_BRACKET T_SEMICOLON
	| T_ENUM T_LEFT_CURLY_BRACKET enum_list T_RIGHT_CURLY_BRACKET T_IDENTIFIER T_SEMICOLON

enum_list
	: enum_item T_COMMA enum_list
	| enum_item

enum_item
	: T_IDENTIFIER T_EQUAL T_INTEGER_LITERAL {}
	| T_IDENTIFIER T_EQUAL T_CHAR_LITERAL {}
	| T_IDENTIFIER {}

typedef
	: T_TYPEDEF Identifier T_LESS type_list T_GREATER Identifier T_SEMICOLON
	  {
		if (dcop_area) {
 		  QString* tmp = new QString("<TYPEDEF name=\"%1\" template=\"%2\"><PARAM %3</TYPEDEF>\n");
		  *tmp = tmp->arg( *($6) ).arg( *($2) ).arg( *($4) );
		  $$ = tmp;
		} else {
		  $$ = new QString("");
		}
	  }

const_qualifier
	: /* empty */
	  {
		$$ = 0;
	  }
	| T_CONST
	  {
		$$ = 1;
	  }
	;

prequalifier
	: T_SIGNED { $$ = new QString("signed"); }
	| T_UNSIGNED { $$ = new QString("unsigned"); }

return
	: type
	  {
		QString* tmp = new QString("<RET %1");
		*tmp = tmp->arg( *($1) );
		$$ = tmp;
	  }

asterisks
	: T_ASTERISK asterisks
	| T_ASTERISK

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

type_name
	: prequalifier Identifier { $$ = new QString( *($1) + *($2) ); }
	| Identifier { $$ = $1; }

type
	: type_name T_AMPERSAND {
	     if (dcop_area)
		yyerror("in dcop areas are only const references allowed!");
	  }
	| T_CONST type_name T_AMPERSAND {
	     if (dcop_area) {
	  	QString* tmp = new QString(" type=\"%1\" qleft=\"const\" qright=\"" AMP_ENTITY "\"/>");
		*tmp = tmp->arg( *($2) );
		$$ = tmp;
	     }
	  }
	| T_CONST Identifier T_LESS type_list T_GREATER T_AMPERSAND {
		if (dcop_area) {
		  QString* tmp = new QString(" type=\"%1<%2>\" qleft=\"const\" qright=\"" AMP_ENTITY "\"/>");
		  *tmp = tmp->arg( *($2) ).arg( *($4) );
		  $$ = tmp;
		}
	}
	| Identifier T_LESS type_list T_GREATER T_AMPERSAND {
	    if (dcop_area)
	      yyerror("in dcop areas are only const references allowed!");	
	}
	| Identifier T_LESS type_list T_GREATER asterisks {
	    if (dcop_area)
	      yyerror("in dcop areas are no pointers allowed!");
	}
	| Identifier T_LESS type_list T_GREATER {
	     QString* tmp = new QString(" type=\"%1<%2>\" qleft=\"\" qright=\"\"/>");
	     *tmp = tmp->arg( *($1) ).arg( *($3) );
	     $$ = tmp;
	}
	| type_name {
		QString* tmp = new QString(" type=\"%1\"/>");
		*tmp = tmp->arg( *($1) );
		$$ = tmp;
	}
	| type_name asterisks
  	  {
	    if (dcop_area)
	      yyerror("in dcop areas are no pointers allowed");
	  }
	| T_CONST type_name asterisks
  	  {
	    if (dcop_area)
	      yyerror("in dcop areas are no pointers allowed");
	  }

type_list
	: type T_COMMA type_list
	  {
	    if (dcop_area)
		yyerror("two argument templates in dcop area currently not supported!");

	    $$ = new QString(*($1) + "," + *($3));
	  }
	| type
  	  {
 	    $$ = $1;
	  }
	
param
	: type Identifier default
	  {
		if (dcop_area) {
		   QString* tmp = new QString("<ARG name=\"%1\"" + *($1));
  		   *tmp = tmp->arg( *($2) );
		   $$ = tmp;		
		} else $$ = new QString();
	  }
	| type default
	  {
		if (dcop_area)
		  yyerror("in dcoparea you have to specify paramater names!");
		$$ = new QString();
	  }

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
	| T_EQUAL Identifier
	  {
	  }
	| T_EQUAL bool_value
	  {
	  }
	|  T_EQUAL Identifier T_LEFT_PARANTHESIS params T_RIGHT_PARANTHESIS
	  {
	  }
	;

virtual_qualifier
	: /* empty */ { $$ = 0; }
	| T_VIRTUAL { $$ = 1; }

operator
	: T_MISCOPERATOR | T_GREATER | T_LESS | T_EQUAL ;

function_header
	: return Identifier T_LEFT_PARANTHESIS params T_RIGHT_PARANTHESIS const_qualifier
	  {
	     if (dcop_area) {
		QString* tmp = new QString("<FUNC name=\"%1\" qual=\"%4\">%2%3</FUNC>\n");
		*tmp = tmp->arg( *($2) );
		*tmp = tmp->arg( *($1) );
		*tmp = tmp->arg( *($4) );
		*tmp = tmp->arg($6 ? "const" : "");
		$$ = tmp;
   	     } else
	        $$ = new QString("");
	  }
	| return T_FUNOPERATOR operator T_LEFT_PARANTHESIS params T_RIGHT_PARANTHESIS const_qualifier
	  {
	     if (dcop_area)
		yyerror("operators aren't allowed in dcop areas!");
	     $$ = new QString("");
	  }

function
	: function_header function_body
 	  {
	        $$ = $1;
	  }
	| T_VIRTUAL function_header T_EQUAL T_NULL function_body
	  {
		$$ = $2;
	  }
	| T_VIRTUAL function_header function_body
	  {
		$$ = $2;
	  }
	| Identifier T_LEFT_PARANTHESIS params T_RIGHT_PARANTHESIS function_body
	  {
	      /* The constructor */
	      assert(!dcop_area);
              $$ = new QString("");
	  }
	| virtual_qualifier T_TILDE Identifier T_LEFT_PARANTHESIS T_RIGHT_PARANTHESIS function_body
	  {
	      /* The destructor */
  	      assert(!dcop_area);
              $$ = new QString("");
	  }
	| T_STATIC return Identifier T_LEFT_PARANTHESIS params T_RIGHT_PARANTHESIS function_body
	  {
		if (dcop_area)
		  yyerror("static is not allowed in dcop area!");
		$$ = new QString();
	  }

function_body
	: T_SEMICOLON
	| T_LEFT_CURLY_BRACKET function_lines T_RIGHT_CURLY_BRACKET
	| T_LEFT_CURLY_BRACKET function_lines T_RIGHT_CURLY_BRACKET T_SEMICOLON

function_lines
	: T_RETURN Identifier T_SEMICOLON function_lines {}
	| T_RETURN Identifier T_LEFT_PARANTHESIS params T_RIGHT_PARANTHESIS T_SEMICOLON function_lines {}
	| T_RETURN Identifier T_ACCESS T_IDENTIFIER T_SEMICOLON function_lines {}
	| T_RETURN Identifier T_ACCESS T_IDENTIFIER T_LEFT_PARANTHESIS params T_RIGHT_PARANTHESIS T_SEMICOLON function_lines {}
	| T_IDENTIFIER T_EQUAL T_IDENTIFIER T_SEMICOLON function_lines {}
	| /* empty */ {}
	;

member
	: return T_IDENTIFIER T_SEMICOLON {}
	| T_STATIC return T_IDENTIFIER T_SEMICOLON {}

%%

void dcopidlParse( const char *_code )
{
    printf("<!DOCTYPE DCOP-IDL><DCOP-IDL>\n");
    dcopidlInitFlex( _code );
    yyparse();
    printf("</DCOP-IDL>\n");
}
