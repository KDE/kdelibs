#include <stdlib.h>
#include <stdio.h>

#include "jscript.h"

/**********************************************************
 *
 * JSEnvironment
 *
 *********************************************************/

JSEnvironment::JSEnvironment( KHTMLWidget *_view )
{
    view = _view;

    globalScope = makeGlobalScope();
    
    printf("MAKING BULILTIN STUFF\n");

    jsWindowObject = new JSWindowObject( this );

    JSVariableObject *var = new JSVariableObject();
    var->setValue( jsWindowObject );
    var->setName( "window" );
    var->setConst( TRUE );

    globalScope->insertObject( var );

    // Ugly Trick: Netscape allows to drop the "window." stuff. So we make every
    // variable/function of window available to the global scope
    /* JSInstanceScope* scope = jsWindowObject->getScope();
    QList<JSObject>* objectList = scope->getObjectList();
    JSObject *o;
    for ( o = objectList->first(); o != 0L; o = objectList->next() )
	globalScope->insertObject( o ); */
}

JSCode* JSEnvironment::parse( const char *_script )
{
    JSCode* code = new JSCode();
    int ret = parseJavaScript( _script, code, globalScope );
    
    if ( ret == 0 )
	return code;

    delete code;
    return 0L;
}

int JSEnvironment::exec( JSCode *_code, JSInstanceScope *_scope )
{
    output.setStr( "" );
    
    if ( _scope == 0L )
	_scope = jsWindowObject->getScope();
    
    int ret = execJavaScript( _code, globalScope, _scope );
    return ret;
}

KHTMLView* JSEnvironment::getKHTMLView()
{
    return view->getView();
}

JSEnvironment::~JSEnvironment()
{
    printf("===================================== DEL JSEnv 1 =============================\n");
    delete globalScope;
    printf("===================================== DEL JSEnv 2 =============================\n");
}

/**********************************************************
 *
 * JSWindowObject
 *
 *********************************************************/

JSWindowObject::JSWindowObject( JSEnvironment *_jsenv ) : JSUserDefinedObject()
{
    jsEnvironment = _jsenv;

    // Variable 'status'
    varStatus = new JSWindowVarObject( this, KIND_JSWindowStatus );
    getScope()->insertObject( varStatus );
    // Variable 'parent'
    varParent = new JSWindowVarObject( this, KIND_JSWindowParent );
    getScope()->insertObject( varParent );
    // Variable 'top'
    varTop = new JSWindowVarObject( this, KIND_JSWindowTop );
    getScope()->insertObject( varTop );
    // Variable 'name'
    varName = new JSWindowVarObject( this, KIND_JSWindowName );
    getScope()->insertObject( varName );

    // Variable 'document'
    jsDocumentObject = new JSDocumentObject( this );
    jsDocumentVar = new JSVariableObject();
    jsDocumentVar->setValue( jsDocumentObject );
    jsDocumentVar->setName( "document" );
    jsDocumentVar->setConst( TRUE );
    getScope()->insertObject( jsDocumentVar );

    // Variable for array 'frames'
    jsFramesObject = new JSWindowArrayObject( this, KIND_JSWindowFrames );
    jsFramesVar = new JSVariableObject();
    jsFramesVar->setValue( jsFramesObject );
    jsFramesVar->setName( "frames" );
    jsFramesVar->setConst( TRUE );
    getScope()->insertObject( jsFramesVar );

}

JSWindowObject::~JSWindowObject()
{
    printf("===================================== DEL JSWindowObject =============================\n");
}

/**********************************************************
 *
 * JSWindowArrayObject
 *
 *********************************************************/

JSWindowArrayObject::JSWindowArrayObject( JSWindowObject *_win, int _kind )
{
    kind = _kind;
    object = _win;
}

int JSWindowArrayObject::rightValue( JSObject *_index, JSValue *_rv )
{
    printf("Accessing array\n");
    
    int ret = 0;
    
    switch( kind )
    {
    case KIND_JSWindowFrames:
	{
	    if ( !_index->inherits( TYPE_JSIntegerObject ) )
		return ERROR_JSNotAInteger;
	    
	    int index = ((JSIntegerObject*)_index)->getValue();
	    	    
	    KHTMLWidget *w = object->getJSEnvironment()->getKHTMLWidget();
	    if ( index < 0 )
		return ERROR_JSIndexOutOfRange;
	    
	    QList<KHTMLWidget> *list = w->getFrameList();
	    
	    if ( index >= (int)list->count() )
		return ERROR_JSIndexOutOfRange;
	    
	    _rv->setObject( list->at( index )->getJSWindowObject() );
	    _rv->setLeftValue( FALSE );
	    _rv->setAutoDelete( FALSE );
	    
	    return ret;
	}
    }
    
    return ERROR_JSInternal;
}

JSWindowArrayObject::~JSWindowArrayObject()
{
    printf("===================================== DEL JSWindowArrayObject =============================\n");
}

/**********************************************************
 *
 * JSWindowVarObject
 *
 *********************************************************/

JSWindowVarObject::JSWindowVarObject( JSWindowObject *_obj, int _kind )
{
    kind = _kind;
    object = _obj;

    switch ( kind )
    {
    case KIND_JSWindowStatus:
	setName( "status" );
	setDynamic( TRUE );
	break;
    case KIND_JSWindowTop:
	setName( "top" );
	setConst( TRUE );
	break;
    case KIND_JSWindowParent:
	setName( "parent" );
	setConst( TRUE );
	break;
    case KIND_JSWindowName:
	setName( "name" );
	setConst( TRUE );
	setDynamic( TRUE );
	break;
    }
}

JSObject* JSWindowVarObject::getValue()
{
    switch ( kind )
    {
    case KIND_JSWindowStatus:
	return new JSStringObject( "Statusbar content" );
    case KIND_JSWindowParent:
	{
	    KHTMLView* v = object->getJSEnvironment()->getKHTMLView()->getParentView();
	    if ( v == 0L )
	    {
		return new JSObject();
	    }
	    return v->getKHTMLWidget()->getJSWindowObject();
	}
    case KIND_JSWindowTop:
	{
	    KHTMLView* v = object->getJSEnvironment()->getKHTMLView()->getParentView();
	    if ( v == 0L )
		return object;
	    while ( v->getParentView() )
		v = v->getParentView();
	    return v->getKHTMLWidget()->getJSWindowObject();
	}
    case KIND_JSWindowName:
	{
	    const char *p = object->getJSEnvironment()->getKHTMLView()->getFrameName();
	    if ( p )
		return new JSStringObject( p );
	    else
		return new JSStringObject( "" );
	}
    }
 
    return new JSObject();
}

void JSWindowVarObject::setValue( JSObject* _val )
{
    switch ( kind )
    {
    case KIND_JSWindowStatus:
	if ( _val->inherits( TYPE_JSStringObject ) )
	{
	    printf("STATUS BAR->'%s'\n",((JSStringObject*)_val)->getString());
	}
	break;
    }
}

void JSWindowVarObject::clear()
{
    switch ( kind )
    {
    case KIND_JSWindowStatus:
	printf("STATUS BAR->\"\"\n");
	break;
    }
}

JSWindowVarObject::~JSWindowVarObject()
{
    printf("===================================== DEL JSWindowVarObject =============================\n");
}

/**********************************************************
 *
 * JSDocumentObject
 *
 *********************************************************/

JSDocumentObject::JSDocumentObject( JSWindowObject *_window ) : JSUserDefinedObject()
{
    window = _window;
    
    jsDocumentWrite = new JSDocumentWriteFunction( this );
    getScope()->insertObject( new JSFunctionObject( jsDocumentWrite ) );
}

JSDocumentObject::~JSDocumentObject()
{
    printf("===================================== DEL JSDocumentObject =============================\n");
    // delete jsDocumentWrite;
}

/**********************************************************
 *
 * JSDocumentWriteFunction
 *
 *********************************************************/

JSDocumentWriteFunction::JSDocumentWriteFunction( JSDocumentObject *_obj ) : JSFunction( "write", 0L, 0L )
{
    object = _obj;
}

int JSDocumentWriteFunction::rightValue( JSScopeStack*, JSValue *rv, JSParameterListObject *_param )
{
    int ret = 0;

    QString out;
    
    if ( _param )
    {
	JSValue *v;
	int i = 0;
	for ( v = _param->firstValue(); v != 0L; v = _param->nextValue() )
	{
	    i++;
	    if ( i > 1 )
		object->getJSWindowObject()->getJSEnvironment()->writeOutput( " " );
	    
	    if ( v->getObject()->isA() == TYPE_JSIntegerObject )
	    {
		out.sprintf("%i",((JSIntegerObject*)(v->getObject()))->getValue() );
		object->getJSWindowObject()->getJSEnvironment()->writeOutput( out.data() );
		printf( "%i ", ((JSIntegerObject*)(v->getObject()))->getValue() );
	    }
	    else if ( v->getObject()->isA() == TYPE_JSStringObject )
	    {
		out = ((JSStringObject*)(v->getObject()))->getString();
		object->getJSWindowObject()->getJSEnvironment()->writeOutput( out.data() );
		printf( "%s ", ((JSStringObject*)(v->getObject()))->getString() );
	    }
	    else if ( v->getObject()->isA() == TYPE_JSBoolObject )
	    {
		if ( ((JSBoolObject*)(v->getObject()))->getValue() )
		{
		    out = "TRUE";
		    object->getJSWindowObject()->getJSEnvironment()->writeOutput( out.data() );
		    printf( "TRUE " );
		}
		else
		{
		    out = "FALSE";
		    object->getJSWindowObject()->getJSEnvironment()->writeOutput( out.data() );
		    printf( "FALSE " );
		}
	    }
	    else if ( v->getObject()->isA() == TYPE_JSFloatObject )
	    {
		out.sprintf( "%f", ((JSFloatObject*)(v->getObject()))->getValue() );	    
		object->getJSWindowObject()->getJSEnvironment()->writeOutput( out.data() );
		printf( "%f ", ((JSFloatObject*)(v->getObject()))->getValue() );	    
	    }
	}
    }
    
    rv->setObject( new JSObject() );
    rv->setAutoDelete( TRUE );
    rv->setLeftValue( FALSE );
    
    return ret;
}

JSDocumentWriteFunction::~JSDocumentWriteFunction()
{
    printf("===================================== DEL JSDocumentWriteFunction =============================\n");
}

/**********************************************************
 *
 * JSEventHandler
 *
 *********************************************************/

JSEventHandler::JSEventHandler( JSEnvironment *_env, const char *_name, const char *_script )
{
    name = _name;
    jsEnvironment = _env;
    code = jsEnvironment->parse( _script );
}

JSEventHandler::~JSEventHandler()
{
    printf("===================================== DEL JSEventHandler =============================\n");
    if ( code )
	delete code;
}

int JSEventHandler::exec( JSInstanceScope *_scope )
{
    if ( !code )
	return ERROR_JSInternal;
    
    int ret = jsEnvironment->exec( code, _scope );
    printf("RETURN '%i'\n",ret);
    return ret;
}

