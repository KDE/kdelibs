    /*

    Copyright (C) 1999 Stefan Westerfeld, stefan@space.twc.de
                       Nicolas Brodu, nicolas.brodu@free.fr

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
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <string>
#include "core.h"
#include "namespace.h"

using namespace std;
using namespace Arts;

extern int idl_line_no;
extern string idl_filename;

extern int yylex();
extern void mcopidlInitFlex( const char *_code );
extern void addEnumTodo( const EnumDef& edef );
extern void addStructTodo( const TypeDef& type );
extern void addInterfaceTodo( const InterfaceDef& iface );

void yyerror( const char *s )
{
	printf( "%s:%i: %s\n", idl_filename.c_str(), idl_line_no, s );
    exit(1);
	//   theParser->parse_error( idl_lexFile, s, idl_line_no );
}

static struct ParserGlobals {
	vector<string> noHints;
} *g;

%}

%union
{
  // generic data types
  long		_int;
  char*		_str;
  unsigned short	_char;
  double	_float;

  vector<char*> *_strs;

  // types
  vector<TypeComponent> *_typeComponentSeq;
  TypeComponent* _typeComponent;

  // enums
  vector<EnumComponent> *_enumComponentSeq;

  // interfaces
  InterfaceDef *_interfaceDef;

  ParamDef* _paramDef;
  vector<ParamDef> *_paramDefSeq;

  MethodDef* _methodDef;
  vector<MethodDef> *_methodDefSeq;

  AttributeDef* _attributeDef;
  vector<AttributeDef> *_attributeDefSeq;
}

%token T_STRUCT T_ENUM T_INTERFACE T_MODULE T_VOID
%token T_LEFT_CURLY_BRACKET T_RIGHT_CURLY_BRACKET
%token T_LEFT_PARANTHESIS T_RIGHT_PARANTHESIS
%token T_LESS T_GREATER T_EQUAL
%token T_SEMICOLON T_COLON T_COMMA
%token<_str> T_IDENTIFIER T_QUALIFIED_IDENTIFIER
%type<_str> type
%type<_str> simpletype
%type<_str> enumname
%type<_str> interfacelistelem
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
%type<_strs> interfacelist
%type<_strs> identifierlist
%type<_strs> defaultdef
%type<_strs> inheritedinterfaces

%type<_int> maybereadonly
%type<_int> direction
%type<_int> maybeoneway
%type<_int> maybedefault

%token T_INTEGER_LITERAL T_UNKNOWN
%type<_int> T_INTEGER_LITERAL

%token T_BOOLEAN T_STRING T_LONG T_BYTE T_OBJECT T_SEQUENCE T_AUDIO T_FLOAT
%token T_IN T_OUT T_STREAM T_MULTI T_ATTRIBUTE T_READONLY T_ASYNC T_ONEWAY
%token T_DEFAULT

%%

aidlfile: definitions;

definitions: epsilon | definition definitions ;

definition: structdef | interfacedef | moduledef | enumdef ;

structdef:
	  T_STRUCT T_IDENTIFIER { ModuleHelper::define($2); } T_SEMICOLON
	| T_STRUCT T_IDENTIFIER { ModuleHelper::define($2); } 
	    T_LEFT_CURLY_BRACKET
	  	  structbody
	    T_RIGHT_CURLY_BRACKET
	  T_SEMICOLON
	  {
        char *qualified = ModuleHelper::qualify($2);
		addStructTodo(TypeDef(qualified,*$5,g->noHints));
		free(qualified);
	    free($2);
	  }
	;

enumdef:
	  T_ENUM enumname { ModuleHelper::define($2); }
	    T_LEFT_CURLY_BRACKET
	  	  enumbody
	    T_RIGHT_CURLY_BRACKET
	  T_SEMICOLON
	  {
	    char *qualified = ModuleHelper::qualify($2);
	  	addEnumTodo(EnumDef(qualified,*$5,g->noHints));
		free(qualified);
		free($2);
		delete $5;
	  }
	;

enumname: T_IDENTIFIER { $$ = $1; } | epsilon { $$ = strdup("_anonymous_"); };

enumbody:
      T_IDENTIFIER
	  {
	  	$$ = new vector<EnumComponent>;
		$$->push_back(EnumComponent($1,0,g->noHints));
		free($1);
	  }
	| T_IDENTIFIER T_EQUAL T_INTEGER_LITERAL
	  {
	  	$$ = new vector<EnumComponent>;
		$$->push_back(EnumComponent($1,$3,g->noHints));
		free($1);
	  }
	| enumbody T_COMMA T_IDENTIFIER
	  {
	  	EnumComponent& last = (*$1)[$1->size()-1];

		$$ = $1;
		$$->push_back(EnumComponent($3,last.value+1,g->noHints));
		free($3);
	  }
	| enumbody T_COMMA T_IDENTIFIER T_EQUAL T_INTEGER_LITERAL
	  {
		$$ = $1;
		$$->push_back(EnumComponent($3,$5,g->noHints));
		free($3);
	  };

interfacedef:
	  T_INTERFACE T_IDENTIFIER { ModuleHelper::define($2); } T_SEMICOLON
	| T_INTERFACE T_IDENTIFIER { ModuleHelper::define($2); } inheritedinterfaces
	    T_LEFT_CURLY_BRACKET
	  	  classbody
	    T_RIGHT_CURLY_BRACKET
	  T_SEMICOLON
	  {
	    vector<char *>::iterator ii;
		for(ii=$4->begin(); ii != $4->end(); ii++)
		{
			$6->inheritedInterfaces.push_back(*ii);
			free(*ii);
		}
		delete $4;
        char *qualified = ModuleHelper::qualify($2);
		$6->name = qualified;
		free(qualified);
		free($2);
	  	addInterfaceTodo(*$6);
		delete $6;
	  }
	;

inheritedinterfaces:
      epsilon { $$ = new vector<char *>; }
	| T_COLON interfacelist { $$ = $2; };

moduledef:
	  T_MODULE T_IDENTIFIER { ModuleHelper::enter($2); free($2); }
	    T_LEFT_CURLY_BRACKET
	  	  definitions
	    T_RIGHT_CURLY_BRACKET { ModuleHelper::leave(); }
	  T_SEMICOLON
	;

classbody:
      epsilon {
	  	$$ = new InterfaceDef();
	  }
	| methoddef classbody
	  {
		$$ = $2;
		$$->methods.insert($$->methods.begin(),*$1);
		delete $1;
	  }
	| attributedef classbody
	  {
	    $$ = $2;
	    $$->attributes.insert($$->attributes.begin(),$1->begin(),$1->end());
	    if ((*$1)[0].flags & streamDefault) {
	      vector<std::string> sv;
	      for (vector<AttributeDef>::iterator i=$1->begin(); i!=$1->end(); i++)
	        sv.push_back(i->name);
	      $$->defaultPorts.insert($$->defaultPorts.begin(),sv.begin(),sv.end());
	    }
	  }
	| defaultdef classbody
	  {
	    $$ = $2;
	    for (vector<char *>::iterator i=$1->begin(); i!=$1->end(); i++)
	       $$->defaultPorts.insert($$->defaultPorts.begin(), *i);
	  };

attributedef:
	  streamdef
	| maybereadonly T_ATTRIBUTE type identifierlist T_SEMICOLON
	  {
	    // 16 == attribute
		vector<char *>::iterator i;
		$$ = new vector<AttributeDef>;
		for(i=$4->begin();i != $4->end();i++)
		{
	  	  $$->push_back(AttributeDef((*i),$3,(AttributeType)($1 + 16),g->noHints));
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

maybedefault:
      epsilon { $$ = 0; }
	| T_DEFAULT { $$ = streamDefault; }
	;

streamdef: maybedefault direction type T_STREAM identifierlist T_SEMICOLON
	  {
	    // 8 == stream
		vector<char *>::iterator i;
		$$ = new vector<AttributeDef>;
		for(i=$5->begin();i != $5->end();i++)
		{
	  	  $$->push_back(AttributeDef((*i),$3,(AttributeType)(($2|$1) + 8),g->noHints));
		  free(*i);
		}
		delete $5;
	  };

defaultdef: T_DEFAULT identifierlist T_SEMICOLON
	  {
	  	$$ = $2;
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
	  	$$ = new MethodDef($3,$2,(MethodType)$1,*$5,g->noHints);
		free($3);
		free($2);
	  }
	;

paramdefs:
      epsilon
	  {
	  	$$ = new vector<ParamDef>;
	  }
	| paramdef paramdefs1
	  {
	  	$$ = $2;
		$$->insert($$->begin(),*$1);
		delete $1;
	  };

// at least one parameter (ex:  "long a" or "long a, long b, string c")
paramdefs1:
	  epsilon
	  {
	  	$$ = new vector<ParamDef>;
	  }
	| paramdefs1 T_COMMA paramdef
	  {
	  	$$ = $1;
		$$->push_back(*$3);
		delete $3;
		//$$->insert($$->begin(),$3);
	  }
	;

// one parameter (ex:  "long a")
paramdef: type T_IDENTIFIER
	  {
	  	$$ = new ParamDef(string($1),string($2),g->noHints);
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

interfacelist:
	  interfacelistelem { $$ = new vector<char *>; $$->push_back($1); }
	| interfacelist T_COMMA interfacelistelem { $$ = $1; $$->push_back($3); }

interfacelistelem: T_IDENTIFIER {
		$$ = ModuleHelper::qualify($1);
		free($1);
	  }
	| T_QUALIFIED_IDENTIFIER {
		$$ = ModuleHelper::qualify($1);
		free($1);
	  }
	;

structbody: epsilon {
		// is empty by default
		$$ = new vector<TypeComponent>;
	  }
	| type identifierlist T_SEMICOLON structbody {
	    $$ = $4;
		vector<char *>::reverse_iterator i;
		for(i = $2->rbegin();i != $2->rend();i++)
		{
		  char *identifier = *i;

		  $$->insert($$->begin(),TypeComponent($1,identifier,g->noHints));
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

		// alloc new size: add one for the null byte and one for the '*' char
	    char *result = (char *)malloc(strlen($3)+2);
		result[0] = '*';
		strcpy(&result[1],$3);
		free($3);  /* fails */

	  	$$ = result;
	  };

simpletype:
      T_BOOLEAN { $$ = strdup("boolean"); }
	| T_STRING { $$ = strdup("string"); }
	| T_LONG { $$ = strdup("long"); }
	| T_BYTE { $$ = strdup("byte"); }
	| T_OBJECT { $$ = strdup("object"); }
	| T_AUDIO { $$ = strdup("float"); }
	| T_FLOAT { $$ = strdup("float"); }
	| T_VOID { $$ = strdup("void"); }
	| T_IDENTIFIER {
		$$ = ModuleHelper::qualify($1);
		free($1);
	  }
	| T_QUALIFIED_IDENTIFIER {
		$$ = ModuleHelper::qualify($1);
		free($1);
	  };


epsilon: /* empty */ ;
%%

void mcopidlParse( const char *_code )
{
	g = new ParserGlobals;
    mcopidlInitFlex( _code );
    yyparse();
	delete g;
}
