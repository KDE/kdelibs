    /*

    Copyright (C) 1999 Stefan Westerfeld
                       stefan@space.twc.de

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    */

%{
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "core.h"

using namespace std;

extern int idl_line_no;
extern string idl_filename;

extern int yylex();
extern void mcopidlInitFlex( const char *_code );
extern void addEnumTodo( EnumDef *edef );
extern void addStructTodo( TypeDef *type );
extern void addInterfaceTodo( InterfaceDef *iface );

void yyerror( const char *s )
{
	printf( "%s:%i: %s\n", idl_filename.c_str(), idl_line_no, s );
    exit(1);
	//   theParser->parse_error( idl_lexFile, s, idl_line_no );
}

%}

%union
{
  // generic data types
  long		_int;
  char*		_str;
  unsigned short	_char;
  double	_float;

  vector<char *> *_strs;

  // types
  vector<TypeComponent *> *_typeComponentSeq;
  TypeComponent* _typeComponent;

  // enums
  vector<EnumComponent *> *_enumComponentSeq;

  // interfaces
  InterfaceDef *_interfaceDef;

  ParamDef* _paramDef;
  vector<ParamDef *> *_paramDefSeq;

  MethodDef* _methodDef;
  vector<MethodDef *> *_methodDefSeq;

  AttributeDef* _attributeDef;
  vector<AttributeDef *> *_attributeDefSeq;
}

%token T_STRUCT T_ENUM T_INTERFACE T_MODULE
%token T_LEFT_CURLY_BRACKET T_RIGHT_CURLY_BRACKET
%token T_LEFT_PARANTHESIS T_RIGHT_PARANTHESIS
%token T_LESS T_GREATER T_EQUAL
%token T_SEMICOLON T_COLON T_COMMA
%token<_str> T_IDENTIFIER
%type<_str> type
%type<_str> simpletype
%type<_str> maybe_identifier
/*%type<_typeComponent> typecomponentdef*/
%type<_typeComponentSeq> structbody
%type<_paramDef> paramdef
%type<_paramDefSeq> paramdefs
%type<_paramDefSeq> paramdefs1
%type<_methodDef> methoddef
%type<_attributeDefSeq> attributedef
%type<_attributeDefSeq> streamdef
%type<_interfaceDef> classbody
%type<_enumComponentSeq> enumbody
%type<_strs> identifierlist
%type<_strs> inheritedinterfaces

%type<_int> maybereadonly
%type<_int> direction
%type<_int> maybeoneway

%token T_INTEGER_LITERAL T_UNKNOWN
%type<_int> T_INTEGER_LITERAL

%token T_BOOLEAN T_STRING T_LONG T_BYTE T_OBJECT T_SEQUENCE T_AUDIO
%token T_IN T_OUT T_STREAM T_MULTI T_ATTRIBUTE T_READONLY T_ASYNC T_ONEWAY

%%

aidlfile: definitions;

definitions: epsilon | definition definitions ;

definition: structdef | interfacedef | moduledef | enumdef ;

structdef:
	  T_STRUCT T_IDENTIFIER
	    T_LEFT_CURLY_BRACKET
	  	  structbody
	    T_RIGHT_CURLY_BRACKET
	  T_SEMICOLON
	  {
		addStructTodo(new TypeDef($2,*$4));
	    free($2);
	  }
	;

enumdef:
	  T_ENUM maybe_identifier
	    T_LEFT_CURLY_BRACKET
	  	  enumbody
	    T_RIGHT_CURLY_BRACKET
	  T_SEMICOLON
	  {
	  	addEnumTodo(new EnumDef($2,*$4));
		free($2);
		delete $4;
	  }
	;

maybe_identifier: T_IDENTIFIER { $$ = $1; } | epsilon { $$ = strdup(""); };

enumbody:
      T_IDENTIFIER
	  {
	  	$$ = new vector<EnumComponent *>;
		$$->push_back(new EnumComponent($1,0));
		free($1);
	  }
	| T_IDENTIFIER T_EQUAL T_INTEGER_LITERAL
	  {
	  	$$ = new vector<EnumComponent *>;
		$$->push_back(new EnumComponent($1,$3));
		free($1);
	  }
	| enumbody T_COMMA T_IDENTIFIER
	  {
	  	EnumComponent *last = (*$1)[$1->size()-1];

		$$ = $1;
		$$->push_back(new EnumComponent($3,last->value+1));
		free($3);
	  }
	| enumbody T_COMMA T_IDENTIFIER T_EQUAL T_INTEGER_LITERAL
	  {
		$$ = $1;
		$$->push_back(new EnumComponent($3,$5));
		free($3);
	  };

interfacedef:
	  T_INTERFACE T_IDENTIFIER inheritedinterfaces
	    T_LEFT_CURLY_BRACKET
	  	  classbody
	    T_RIGHT_CURLY_BRACKET
	  T_SEMICOLON
	  {
	    vector<char *>::iterator ii;
		for(ii=$3->begin(); ii != $3->end(); ii++)
		{
			$5->inheritedInterfaces.push_back(*ii);
			free(*ii);
		}
		delete $3;

	  	//addInterfaceTodo(new InterfaceDef($2,*$5));
		$5->name = $2;
		free($2);
	  	addInterfaceTodo($5);
	  }
	;

inheritedinterfaces:
      epsilon { $$ = new vector<char *>; }
	| T_COLON identifierlist { $$ = $2; };

moduledef:
	  T_MODULE T_IDENTIFIER
	    T_LEFT_CURLY_BRACKET
	  	  definitions
	    T_RIGHT_CURLY_BRACKET
	  T_SEMICOLON
	;

classbody:
      epsilon {
	  	$$ = new InterfaceDef();
	  }
	| methoddef classbody
	  {
		$$ = $2;
		$$->methods.insert($$->methods.begin(),$1);
	  }
	| attributedef classbody
	  {
	    $$ = $2;
	    $$->attributes.insert($$->attributes.begin(),$1->begin(),$1->end());
	  };

attributedef:
	  streamdef
	| maybereadonly T_ATTRIBUTE type identifierlist T_SEMICOLON
	  {
	    // 16 == attribute
		vector<char *>::iterator i;
		$$ = new vector<AttributeDef *>;
		for(i=$4->begin();i != $4->end();i++)
		{
	  	  $$->push_back(new AttributeDef((*i),$3,(AttributeType)($1 + 16)));
		  free(*i);
		}
		delete $4;
	  }	
	;

maybereadonly:
      epsilon { $$ = 1+2; /* in&out (read & write) */ }
	| T_READONLY { $$ = 2; /* out (readonly) */ }
	;

maybeoneway:
      epsilon { $$ = methodTwoway; }
	| T_ONEWAY { $$ = methodOneway; }
	;

streamdef: direction type T_STREAM identifierlist T_SEMICOLON
	  {
	    // 8 == stream
		vector<char *>::iterator i;
		$$ = new vector<AttributeDef *>;
		for(i=$4->begin();i != $4->end();i++)
		{
	  	  $$->push_back(new AttributeDef((*i),$2,(AttributeType)($1 + 8)));
		  free(*i);
		}
		delete $4;
	  };

direction:
		T_IN { $$ = streamIn; }
	  | T_IN T_MULTI { $$ = streamIn|streamMulti; }
	  | T_OUT { $$ = streamOut; }
	  | T_OUT T_MULTI { $$ = streamOut|streamMulti; }
	  |	T_ASYNC T_IN { $$ = streamAsync|streamIn; }
	  | T_ASYNC T_IN T_MULTI { $$ =streamAsync|streamIn|streamMulti  }
	  | T_ASYNC T_OUT { $$ = streamAsync|streamOut; }
	  | T_ASYNC T_OUT T_MULTI { $$ = streamAsync|streamOut|streamMulti; }
	;

// CacheElement getFromCache(string name, bool hit)
methoddef:
	  maybeoneway type T_IDENTIFIER
	  	T_LEFT_PARANTHESIS paramdefs T_RIGHT_PARANTHESIS T_SEMICOLON
	  {
	  	$$ = new MethodDef($3,$2,(MethodType)$1,*$5);
		free($3);
		free($2);
	  }
	;

paramdefs:
      epsilon
	  {
	  	$$ = new vector<ParamDef *>;
	  }
	| paramdef paramdefs1
	  {
	  	$$ = $2;
		$$->insert($$->begin(),$1);
	  };

// at least one parameter (ex:  "long a" or "long a, long b, string c")
paramdefs1:
	  epsilon
	  {
	  	$$ = new vector<ParamDef *>;
	  }
	| paramdefs1 T_COMMA paramdef
	  {
	  	$$ = $1;
		$$->push_back($3);
		//$$->insert($$->begin(),$3);
	  }
	;

// one parameter (ex:  "long a")
paramdef: type T_IDENTIFIER
	  {
	  	$$ = new ParamDef(string($1),string($2));
		free($1);
		free($2);
	  };

/*
typecomponentdef: type T_IDENTIFIER T_SEMICOLON
	  {
	    $$ = new TypeComponent($1,string($2));
		free($2);
	  };
*/

identifierlist:
	  T_IDENTIFIER { $$ = new vector<char *>; $$->push_back($1); }
	| identifierlist T_COMMA T_IDENTIFIER { $$ = $1; $$->push_back($3); }

structbody: epsilon {
		// is empty by default
		$$ = new vector<TypeComponent *>;
	  }
	| type identifierlist T_SEMICOLON structbody {
	    $$ = $4;
		vector<char *>::reverse_iterator i;
		for(i = $2->rbegin();i != $2->rend();i++)
		{
		  char *identifier = *i;

		  $$->insert($$->begin(),new TypeComponent($1,identifier));
		  free(identifier);
		}
		delete $2;
	  };
	  /*
	| typecomponentdef structbody {
		$$ = $2;
		$$->insert($$->begin(),$1);
	  };
	  */

type: simpletype
    | T_SEQUENCE T_LESS simpletype T_GREATER
	  {
	  	// a sequence<long> is for instance coded as *long

	    char *result = (char *)malloc(strlen($3)+1);
		result[0] = '*';
		strcpy(&result[1],$3);
		free($3);

	  	$$ = result;
	  };

simpletype:
      T_BOOLEAN { $$ = strdup("boolean"); }
	| T_STRING { $$ = strdup("string"); }
	| T_LONG { $$ = strdup("long"); }
	| T_BYTE { $$ = strdup("byte"); }
	| T_OBJECT { $$ = strdup("object"); }
	| T_AUDIO { $$ = strdup("float"); }
	| T_IDENTIFIER {
		$$ = $1;
	  };

epsilon: /* empty */ ;
%%

void mcopidlParse( const char *_code )
{
    mcopidlInitFlex( _code );
    yyparse();
}
