#include "jsexec.h"
#include "jstree.h"
#include "builtin.h"

/**********************************************************
 *
 * JSIntegerObject
 *
 *********************************************************/

JSIntegerObject::JSIntegerObject( int _i ) : JSObject()
{
    value = _i;
}

/**********************************************************
 *
 * JSBoolObject
 *
 *********************************************************/

JSBoolObject::JSBoolObject( bool _i ) : JSObject()
{
    value = _i;
}

/**********************************************************
 *
 * JSFloatObject
 *
 *********************************************************/

JSFloatObject::JSFloatObject( double _i ) : JSObject()
{
    value = _i;
}

/**********************************************************
 *
 * JSStringObject
 *
 *********************************************************/

JSStringObject::JSStringObject( const char *_string )
{
    string = _string;
}

/**********************************************************
 *
 * JSVariableObject
 *
 *********************************************************/

JSVariableObject::JSVariableObject() : JSObject()
{
    value = 0L;
    bConst = FALSE;
    bDynamic = FALSE;
}

void JSVariableObject::clear()
{
    if ( bConst )
	return;
    if ( value )
	value->release();
    value = 0L;
}

JSVariableObject::~JSVariableObject()
{
    if ( value )
	delete value;
}

/**********************************************************
 *
 * JSFunctionObject
 *
 *********************************************************/

JSFunctionObject::JSFunctionObject( JSFunction * _func ) : JSObject()
{
    function = _func;
    object = 0L;
}

const char* JSFunctionObject::getName()
{
    if ( function )
	return function->getName();
    else
	return 0L;
}

/**********************************************************
 *
 * JSParameterListObject
 *
 *********************************************************/

JSParameterListObject::JSParameterListObject() : JSObject()
{
    parameterValues.setAutoDelete( TRUE );
}

/**********************************************************
 *
 * JSUserDefinedObject
 *
 *********************************************************/

JSUserDefinedObject::JSUserDefinedObject()
{
    scope.setObject( this );
    lock = 1;
}

/**********************************************************
 *
 * JSValue
 *
 *********************************************************/

JSValue::JSValue()
{
    object = 0L;
    bAutoDelete = FALSE;
    bLeftValue = FALSE;
}

JSValue::~JSValue()
{
    if ( bAutoDelete && object ) object->release(); 
}

void JSValue::setObject( JSObject *_o )
{
    if ( object && bAutoDelete )
	object->release();
    object = _o;
}

/**********************************************************
 *
 * JSScope
 *
 *********************************************************/

JSScope::JSScope()
{
    objectList.setAutoDelete( TRUE );
}
  
void JSScope::insertObject( JSObject* _obj )
{
    objectList.append( _obj );
}

JSVariableObject* JSScope::findVariable( const char *_name )
{
    JSObject *obj;
    
    for ( obj = objectList.first(); obj != 0L; obj = objectList.next() )
    {
	if ( obj->inherits( TYPE_JSVariableObject ) )
	{
	    JSVariableObject *var = (JSVariableObject*) obj;
	    if ( strcmp( var->getName(), _name ) == 0L )
		return var;
	}
    }
    
    return 0L;
}

JSFunctionObject* JSScope::findFunction( const char *_name )
{
    JSObject *obj;
    
    for ( obj = objectList.first(); obj != 0L; obj = objectList.next() )
    {
	if ( obj->inherits( TYPE_JSFunctionObject ) )
	{
	    JSFunctionObject *func = (JSFunctionObject*) obj;
	    if ( strcmp( func->getName(), _name ) == 0L )
		return func;
	}
    }
    
    return 0L;
}

/**********************************************************
 *
 * JSScopeStack
 *
 *********************************************************/

JSScopeStack::JSScopeStack( JSScope *_globalScope, JSInstanceScope *_instanceScope )
{
    globalScope = _globalScope;
    
    scopeList.setAutoDelete( TRUE );
    scopeList.append( _globalScope );

    instanceScopeStack.setAutoDelete( FALSE );
    if ( _instanceScope )
	instanceScopeStack.push( _instanceScope );
}

JSScopeStack::~JSScopeStack()
{
    // Remove the global scope without deleting it
    if ( globalScope )
    {
	scopeList.setAutoDelete( FALSE );
	scopeList.removeRef( globalScope );
	scopeList.setAutoDelete( TRUE );
    }
}

JSVariableObject* JSScopeStack::findVariable( const char *_name, bool _top_only )
{
    JSVariableObject *var;
    JSScope *scope;
    
    if ( _top_only )
	return scopeList.getLast()->findVariable( _name );
    
    for ( scope = scopeList.last(); scope != 0L; scope = scopeList.prev() )
    {
	var = scope->findVariable( _name );
	if ( var != 0L )
	    return var;
    }
    
    return 0L;
}

JSFunctionObject* JSScopeStack::findFunction( const char *_name )
{
    JSFunctionObject *func;
    JSScope *scope;
    
    for ( scope = scopeList.last(); scope != 0L; scope = scopeList.prev() )
    {
	func = scope->findFunction( _name );
	if ( func != 0L )
	    return func;
    }

    return 0L;
}

void JSScopeStack::pushScope( JSScope* _scope )
{
    scopeList.append( _scope );
}

void JSScopeStack::popScope()
{
    JSScope *s = scopeList.getLast();
    if ( s )
	scopeList.removeRef( s );
}

JSScope* JSScopeStack::topScope()
{
    return scopeList.getLast();
}

void JSScopeStack::pushInstanceScope( JSInstanceScope* _scope )
{
    instanceScopeStack.push( _scope );
}

void JSScopeStack::popInstanceScope()
{
    instanceScopeStack.pop();
}

JSInstanceScope* JSScopeStack::topInstanceScope()
{
    return instanceScopeStack.top();
}

/**********************************************************
 *
 * execJavaScript
 *
 *********************************************************/

int execJavaScript( JSCode *_code, JSScope *_global, JSInstanceScope *_instanceScope )
{
    int ret;
    
    /* JSScope *global = new JSScope();
    initBuiltin( global );
    
    if ( _builtin != 0L )
	_builtin( global ); */
    
    JSScopeStack scopes( _global, _instanceScope );
    
    JSValue rv;
    
    /* JSNode *c;
    for ( c = _code->first(); c != 0L; c = _code->next() )
    {
	if ( c->isA() == ID_JSFunction )
	{
	    JSFunction *func = (JSFunction*)c;
	    global->insertObject( new JSFunctionObject( func ) );
	}
    } */
    
    JSNode *c;
    for ( c = _code->first(); c != 0L; c = _code->next() )
    {
	printf("Exec '%i'\n",c->isA());
	
	ret = c->rightValue( &scopes, &rv );
	if ( ret )
	    return ret;
    }

    return 0;
}

/**********************************************************
 *
 * makeGlobalScope
 *
 *********************************************************/

JSScope* makeGlobalScope()
{
    JSScope *global = new JSScope();
    initBuiltin( global );
    return global;
}


