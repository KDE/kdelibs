%{
/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001-2003 Michael Goffioul <kdeprint@swing.be>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#define YYSTYPE QVariant
#define YYPARSE_PARAM fooloader
#define YYDEBUG 1

#include <stdlib.h>
#include <qvariant.h>
#include "foomatic2loader.h"

void yyerror(const char*) {}
int yylex();
%}

%token VAR
%token STRING
%token NUMBER
%token UNDEF
%token POSTPIPE
%token QUOTED

%%

foo:   foodata
     | foo foodata
;

foodata:   VAR '=' '{' fieldlist '}' ';'  { static_cast<Foomatic2Loader*>(fooloader)->m_foodata.insert("VAR",$4); }
	     | POSTPIPE '=' QUOTED ';'        { static_cast<Foomatic2Loader*>(fooloader)->m_foodata.insert("POSTPIPE",$3); }
;

fieldlist:   assignment                 { $$ = $1; }
		   | fieldlist ',' assignment   { QMap<QString,QVariant>::ConstIterator it = $3.mapBegin(); $1.asMap().insert(it.key(), it.data()); $$ = $1; }
;

assignment:   STRING '=' '>' value              { $$.asMap().insert($1.toString(), $4); }
;

valuelist:   value                { $$.asList().append($1); }
		   | valuelist ',' value  { $1.asList().append($3); $$ = $1; }
;

value:   UNDEF             { $$ = QVariant(); }
	   | STRING            { $$ = $1; }
	   | NUMBER            { $$ = $1; }
	   | '[' valuelist ']' { $$ = $2; }
	   | '{' fieldlist '}' { $$ = $2; }
	   | '[' ']'           { $$ = QVariant(); }
	   | '{' '}'           { $$ = QVariant(); }
;

%%
