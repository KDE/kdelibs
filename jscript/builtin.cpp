#include "builtin.h"

#include <stdlib.h>

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

int JSPrintFunction::rightValue( JSScopeStack* _s, JSValue *rv, JSParameterListObject *_param )
{
    int ret = 0;
    
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
