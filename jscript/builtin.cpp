/* This file is part of the KDE libraries
    Copyright (C) 1997 Torben Weis (weis@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#include "builtin.h"

#include <stdlib.h>
#include <stdio.h>

JSPrintFunction *jsPrint = 0L;

void initBuiltin( JSScope *_scope )
{
    if ( jsPrint == 0L )
	jsPrint = new JSPrintFunction();
    
    _scope->insertObject( new JSFunctionObject( jsPrint ) );
}

JSPrintFunction::JSPrintFunction() : JSFunction( "print", 0L, 0L )
{
}

int JSPrintFunction::rightValue( JSScopeStack* , JSValue *rv, JSParameterListObject *_param )
{
    // int ret = 0;
    
    if ( _param )
    {
	JSValue *v;
	for ( v = _param->firstValue(); v != 0L; v = _param->nextValue() )
	{
	    if ( v->getObject()->isA() == TYPE_JSIntegerObject )
		printf( "%i ", ((JSIntegerObject*)(v->getObject()))->getValue() );
	    else if ( v->getObject()->isA() == TYPE_JSStringObject )
		printf( "%s ", ((JSStringObject*)(v->getObject()))->getString() );
	    else if ( v->getObject()->isA() == TYPE_JSBoolObject )
	    {
		if ( ((JSBoolObject*)(v->getObject()))->getValue() )
		    printf( "TRUE " );
		else
		    printf( "FALSE " );
	    }
	    else if ( v->getObject()->isA() == TYPE_JSFloatObject )
		printf( "%f ", ((JSFloatObject*)(v->getObject()))->getValue() );	    
	}
    }
    
    rv->setObject( new JSObject() );
    rv->setAutoDelete( TRUE );
    rv->setLeftValue( FALSE );

    return 0;
}
