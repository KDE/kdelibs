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

#define YYSTYPE QStringList
#define YYPARSE_PARAM ppdloader
#define YYDEBUG 1
#define YYERROR_VERBOSE 1
#include <stdlib.h>
#include <qstringlist.h>

#include "ppdloader.h"

#define builder static_cast<PPDLoader*>( ppdloader )

int kdeprint_ppdlex();
#define kdeprint_ppderror(msg) static_cast<PPDLoader*>( ppdloader )->setErrorMsg( msg )
%}

%token TRANSLATION
%token OPENUI
%token CLOSEUI
%token OPENGROUP
%token CLOSEGROUP
%token DEFAULT
%token KEYWORD
%token OPTION
%token STRINGPART
%token QUOTED
%token CONSTRAINT
%token PAPERDIM
%token IMGAREA
%token FOODATA
%token COMMENT

%%

ppdfile:   ppdelement
	     | ppdfile ppdelement
;

string:   STRINGPART            { $$ = $1; }
	    | string STRINGPART     { $1 += $2; $$ = $1; }
	    | string QUOTED         { $1 += $2; $$ = $1; }
;

value:   string                  { $$ = $1; }
	   | QUOTED                  { $$ = QStringList($1[0].mid(1,$1[0].length()-2)); }
	   | QUOTED '/' TRANSLATION  { $$ = QStringList($1[0].mid(1,$1[0].length()-2)); }
	   | string '/' TRANSLATION  { $$ = $1; }
;

paperdim:   PAPERDIM OPTION ':' QUOTED                 { builder->putPaperDimension($2[0], $4[0]); }
		  | PAPERDIM OPTION '/' TRANSLATION ':' QUOTED { builder->putPaperDimension($2[0], $6[0]); }
;

imgarea:   IMGAREA OPTION ':' QUOTED                 { builder->putImageableArea($2[0], $4[0]); }
	     | IMGAREA OPTION '/' TRANSLATION ':' QUOTED { builder->putImageableArea($2[0], $6[0]); }
;

openui:   OPENUI OPTION ':' string                  { builder->openUi($2[0], QString::null, $4[0]); }
	    | OPENUI OPTION '/' TRANSLATION ':' string  { builder->openUi($2[0], $4[0], $6[0]); }
;

endui:   CLOSEUI ':' string     { builder->endUi($3[0]); }
	   | CLOSEUI string         { builder->endUi($2[0]); }
;

opengroup:   OPENGROUP ':' string                  { builder->openGroup($3.join(" "), QString::null); }
		   | OPENGROUP ':' string '/' TRANSLATION  { builder->openGroup($3.join(" "), $5[0]); }
;

endgroup:   CLOSEGROUP ':' string                  { builder->endGroup($3.join(" ")); }
		  | CLOSEGROUP ':' string '/' TRANSLATION  { builder->endGroup($3.join(" ")); }
;

constraint:   CONSTRAINT ':' KEYWORD OPTION KEYWORD OPTION { builder->putConstraint($3[0], $5[0], $4[0], $6[0]); }
		    | CONSTRAINT ':' KEYWORD OPTION KEYWORD        { builder->putConstraint($3[0], $5[0], $4[0], QString::null); }
			| CONSTRAINT ':' KEYWORD KEYWORD OPTION        { builder->putConstraint($3[0], $4[0], QString::null, $5[0]); }
			| CONSTRAINT ':' KEYWORD KEYWORD               { builder->putConstraint($3[0], $4[0], QString::null, QString::null); }
;

ppdelement:   KEYWORD ':' value                          { builder->putStatement2($1[0], $3[0]); }
		    | KEYWORD OPTION ':' value                   { builder->putStatement($1[0], $2[0], QString::null, $4); }
			| KEYWORD OPTION '/' TRANSLATION ':' value   { builder->putStatement($1[0], $2[0], $4[0], $6); }
			| KEYWORD OPTION '/' ':' value               { builder->putStatement($1[0], $2[0], QString::null, $4); }
			| DEFAULT ':' string                         { builder->putDefault($1[0], $3[0]); }
			| DEFAULT ':' string '/' TRANSLATION         { builder->putDefault($1[0], $3[0]); }
			| openui
			| endui
			| opengroup
			| endgroup
			| paperdim
			| imgarea
			| constraint
			| COMMENT
			| FOODATA                                    { builder->putFooData($1[0]); }
;

%%

#undef builder
